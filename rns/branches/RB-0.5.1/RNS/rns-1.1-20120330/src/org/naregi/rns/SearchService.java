/*
 * Copyright (C) 2008-2012 Osaka University.
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.naregi.rns;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Properties;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import net.sf.saxon.Configuration;
import net.sf.saxon.query.DynamicQueryContext;
import net.sf.saxon.query.StaticQueryContext;
import net.sf.saxon.query.XQueryExpression;
import net.sf.saxon.trans.XPathException;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.apache.axis.types.URI.MalformedURIException;
import org.globus.axis.message.addressing.AttributedURIType;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.axis.message.addressing.ReferenceParametersType;
import org.globus.wsrf.ResourceContext;
import org.globus.wsrf.ResourceContextException;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.ObjectDeserializer;
import org.globus.wsrf.encoding.ObjectSerializer;
import org.globus.wsrf.encoding.SerializationException;
import org.globus.wsrf.utils.AddressingUtils;
import org.naregi.rns.stubs.RNSEntryResponseType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.SearchFaultType;
import org.naregi.rns.stubs.SearchRequestType;
import org.naregi.rns.stubs.SearchResponseType;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.util.TempOneFileStringList;
import org.xml.sax.InputSource;

/**
 * A service implementation for Search operation.
 */
public class SearchService {
	private RNSResource getResource() throws ResourceContextException,
			ResourceException {
		ResourceContext ctx = ResourceContext.getResourceContext();
		return (RNSResource) ctx.getResource();
	}

	private SearchFaultType createSearchFaultType(String message) {
		SearchFaultType f = new SearchFaultType();
		f.setFaultString(message);
		return f;
	}

	public static RNSEntryResponseType convertStringToRNSEntryResponseType(
			String entryResponseStr) throws DeserializationException {
		RNSEntryResponseType rnsert = (RNSEntryResponseType) ObjectDeserializer.deserialize(
				new InputSource(new StringReader(entryResponseStr)),
				RNSEntryResponseType.class);
		return rnsert;
	}

	private static final Properties props = new Properties();
	static {
		props.setProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
		props.setProperty(OutputKeys.INDENT, "no");
	}

	private static EndpointReferenceType baseIteratorEPR = null;

	private static boolean stopSearch = false;

	private static class InterruptThread extends Thread {
		public void run() {
			stopSearch = true;
		}
	}

	private Thread interruptThread;

	{ /* initialize */
		interruptThread = new InterruptThread();
		Runtime.getRuntime().addShutdownHook(interruptThread);
	}

	public SearchResponseType search(SearchRequestType req)
			throws SearchFaultType, ReadNotPermittedFaultType, RemoteException {
		long pfTotal = RNSProfiler.start();
		RNSResource r = getResource();
		if (!r.isReadable()) {
			throw new ReadNotPermittedFaultType();
		}
		Collection<String> nameList;
		String[] names = req.getEntryName();
		if (names == null || names.length == 0) {
			/* all entries */
			nameList = r.getList();
		} else {
			nameList = Arrays.asList(names);
		}

		List<String> resultList;
		int listSize = nameList.size();
		if (listSize >= RNSConfig.tempFileForSearchThreshold()) {
			try {
				resultList = new TempOneFileStringList(RNSConfig.tmpdir(),
						"RNSSCH");
			} catch (IOException e) {
				e.printStackTrace();
				throw createSearchFaultType(e.getMessage());
			}
		} else {
			resultList = new ArrayList<String>(listSize);
		}

		Configuration config = new Configuration();
		StaticQueryContext sqc = config.newStaticQueryContext();
		DynamicQueryContext dynamicContext = new DynamicQueryContext(config);
		XQueryExpression exp;
		try {
			exp = sqc.compileQuery(new StringReader(req.getQuery()));
		} catch (IOException e) {
			e.printStackTrace();
			throw new RemoteException("", e);
		} catch (XPathException e) {
			throw createSearchFaultType(e.getMessage());
		}
		long pf;
		boolean first = true;
		String entryResponseString;

		/* Long process */
		for (String name : nameList) {
			if (stopSearch) {
				System.out.println("[RNS] interrupt Search process");
				throw new RemoteException("interrupt Search process");
			}
			/* recover EPR from ID */
			pf = RNSProfiler.start();
			RNSEntryData entry;
			RNSEntryData entd;
			entd = r.getRNSEntryData(name);
			entry = RNS.convertRNSEntryData(entd);
			if (entry == null) {
				/* ignore ENOENT */
				continue;
			}
			RNSProfiler.stop(RNSProfiler.TYPE.Search_GetRNSEntryFromDB, pf);

			pf = RNSProfiler.start();
			try {
				RNSEntryResponseType ent = RNS.setupResponseEntryType(name,
						entry);
				Writer out = new StringWriter(1024);
				ObjectSerializer.serialize(out, ent,
						RNSQNames.TYPE_ENTRY_RESPONSE_TYPE);
				entryResponseString = out.toString();
			} catch (SerializationException e) {
				throw new RemoteException("Serialize for Search", e);
			}
			RNSProfiler.stop(RNSProfiler.TYPE.Search_Serialize, pf);

			pf = RNSProfiler.start();
			StringWriter sw = new StringWriter(1024);
			try {
				dynamicContext.setContextItem(config.buildDocument(new StreamSource(
						new StringReader(entryResponseString))));
				exp.run(dynamicContext, new StreamResult(sw), props);
			} catch (XPathException e) {
				throw new RemoteException("Run XQuery for Search", e);
			}
			if (first) {
				RNSProfiler.stop(RNSProfiler.TYPE.Search_RunXQuery_First, pf);
				first = false;
			} else {
				RNSProfiler.stop(RNSProfiler.TYPE.Search_RunXQuery_Reuse, pf);
			}

			String result = sw.toString();
			if (result == null || result.length() == 0) {
				/* no match */
				continue;
			}
			/* protect container process */
			if (resultList instanceof ArrayList
					&& RNSUtil.checkMemory(true) == false) {
				throw createSearchFaultType("no memory to create many Search results");
			}
			if (resultList.add(result) == false) {
				throw createSearchFaultType("unexpected error in creating Search results");
			}
		}

		SearchResponseType rsp = new SearchResponseType();
		int iteratorUnit = RNSConfig.getIteratorUnit();
		if (iteratorUnit > 0 && resultList.size() > iteratorUnit) {
			/* use WS-Iterator */
			RNSEntryResponseType[] tmp = new RNSEntryResponseType[iteratorUnit];
			try {
				for (int i = 0; i < iteratorUnit; i++) {
					String entStr = resultList.get(i);
					if (entStr == null) {
						continue;
					}

					pf = RNSProfiler.start();
					tmp[i] = convertStringToRNSEntryResponseType(entStr);
					RNSProfiler.stop(RNSProfiler.TYPE.Search_Deserialize, pf);
				}
			} catch (DeserializationException e) {
				throw new RemoteException("", e);
			}
			rsp.setEntryResponse(tmp);

			String id = SearchIteratorResourceHome.setSessionResults(resultList);
			/* synchronized for recycling baseIteratorEPR */
			EndpointReferenceType epr;
			synchronized (props) { /* global lock */
				if (baseIteratorEPR == null) {
					String eprStr = "<ns1:EndpointReferenceType xsi:type=\"ns1:EndpointReferenceType\" xmlns:ns1=\"http://www.w3.org/2005/08/addressing\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
							+ "<ns1:Address xsi:type=\"ns1:AttributedURI\">"
							+ "http://temp.example.com/"
							+ "</ns1:Address><ns1:ReferenceParameters xsi:type=\"ns1:ReferenceParametersType\"><ns2:SearchSession xmlns:ns2=\""
							+ RNSQNames.NS_IMPL
							+ "\">"
							+ id
							+ "</ns2:SearchSession></ns1:ReferenceParameters></ns1:EndpointReferenceType>";

					try {
						baseIteratorEPR = RNSUtil.toEPR(eprStr);
					} catch (DeserializationException e) {
						SearchIteratorResourceHome.removeSession(id);
						e.printStackTrace();
						throw new RemoteException("unexpected", e);
					} catch (IOException e) {
						SearchIteratorResourceHome.removeSession(id);
						e.printStackTrace();
						throw new RemoteException("unexpected", e);
					}

					EndpointReferenceType tmpEPR;
					try {
						/* get own URL */
						ResourceContext ctx = ResourceContext.getResourceContext();
						tmpEPR = AddressingUtils.createEndpointReference(ctx,
								ctx.getResourceKey());
					} catch (Exception e) {
						SearchIteratorResourceHome.removeSession(id);
						throw new RemoteException("create SearchIteratorEPR", e);
					}
					AttributedURIType address = tmpEPR.getAddress();
					URI uri = address.getValue();
					try {
						String itrname = RNSConfig.getSearchIteratorServicePath();
						uri.setPath(itrname);
					} catch (MalformedURIException e) {
						SearchIteratorResourceHome.removeSession(id);
						throw new RemoteException("unexpected", e);
					}
					address.setValue(uri);
					/* replace */
					baseIteratorEPR.setAddress(address);
					RNS.replaceLocalHostNamePort(baseIteratorEPR);
					epr = baseIteratorEPR;
				} else {
					/* deep copy */
					epr = new EndpointReferenceType(baseIteratorEPR, true);
					ReferenceParametersType parameters = epr.getParameters();
					MessageElement[] mes = parameters.get_any();
					if (mes == null || (mes != null && mes.length != 1)) {
						mes = new MessageElement[1];
					}
					mes[0].setValue(id);
					parameters.set_any(mes);
					epr.setParameters(parameters);
				}
			} /* synchronized */

			System.out.println("[RNS] Start Search Iterator session: " + id);
			rsp.setIterator(epr);
			if (listSize == 1) {
				RNSProfiler.stop(RNSProfiler.TYPE.Total_Search, pfTotal);
			} else {
				RNSProfiler.stop(RNSProfiler.TYPE.Total_Bulk_Search, pfTotal);
			}
			return rsp;
		} else {
			/* not use WS-Iterator */
			RNSEntryResponseType[] tmp = new RNSEntryResponseType[resultList.size()];
			try {
				for (int i = 0; i < tmp.length; i++) {
					String entStr = resultList.get(i);
					if (entStr == null) {
						continue;
					}

					pf = RNSProfiler.start();
					tmp[i] = convertStringToRNSEntryResponseType(entStr);
					RNSProfiler.stop(RNSProfiler.TYPE.Search_Deserialize, pf);
				}
			} catch (DeserializationException e) {
				throw new RemoteException("", e);
			}
			rsp.setEntryResponse(tmp);

			if (listSize == 1) {
				RNSProfiler.stop(RNSProfiler.TYPE.Total_Search, pfTotal);
			} else {
				RNSProfiler.stop(RNSProfiler.TYPE.Total_Bulk_Search, pfTotal);
			}
			return rsp;
		}
	}
}
