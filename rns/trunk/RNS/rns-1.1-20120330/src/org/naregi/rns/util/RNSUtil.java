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
package org.naregi.rns.util;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Properties;
import java.util.regex.Pattern;

import javax.xml.namespace.QName;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import net.sf.saxon.Configuration;
import net.sf.saxon.query.DynamicQueryContext;
import net.sf.saxon.query.StaticQueryContext;
import net.sf.saxon.query.XQueryExpression;
import net.sf.saxon.trans.XPathException;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.axis.message.addressing.ReferenceParametersType;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.ObjectDeserializer;
import org.globus.wsrf.encoding.ObjectSerializer;
import org.globus.wsrf.encoding.SerializationException;
import org.globus.wsrf.impl.SimpleResourceKey;
import org.globus.wsrf.impl.security.authorization.HostAuthorization;
import org.globus.wsrf.impl.security.authorization.HostOrSelfAuthorization;
import org.globus.wsrf.impl.security.authorization.IdentityAuthorization;
import org.globus.wsrf.impl.security.authorization.NoAuthorization;
import org.globus.wsrf.impl.security.authorization.SelfAuthorization;
import org.globus.wsrf.security.authorization.client.Authorization;
import org.globus.wsrf.utils.AnyHelper;
import org.naregi.rns.RNSConfig;
import org.naregi.rns.RNSLog;
import org.naregi.rns.RNSProfiler;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.client.DirnameBasename;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.stubs.RNSEntryDoesNotExistFaultType;
import org.naregi.rns.stubs.RNSEntryExistsFaultType;
import org.naregi.rns.stubs.RNSMetadataType;
import org.naregi.rns.stubs.RNSSupportType;
import org.oasis.wsrf.faults.BaseFaultType;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * RNS common utilities.
 */
public class RNSUtil {

	private RNSUtil() {
	}

	/**
	 * Convert XML String to MessegeEelement.
	 *
	 * @param xml XML String
	 * @return MessageElement
	 * @throws ParserConfigurationException
	 * @throws SAXException
	 * @throws IOException
	 */
	public static MessageElement toMessageElement(String xml)
			throws ParserConfigurationException, SAXException, IOException {
		xml = xml.replaceAll("\\r|\\n", "");
		xml = xml.replaceAll(">\\s+<", "><");

		ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
		DocumentBuilderFactory dbfactory = DocumentBuilderFactory.newInstance();
		DocumentBuilder builder;
		builder = dbfactory.newDocumentBuilder();
		Document doc = builder.parse(in);
		return new MessageElement(doc.getDocumentElement());
	}

	/**
	 * Convert indented XML String.
	 *
	 * @param me MessageElement
	 * @return XML String
	 * @throws TransformerException
	 * @throws Exception
	 */
	public static String toIndentedXML(MessageElement me)
			throws TransformerException, Exception {
		return toIndentedXML(AnyHelper.toElement(me));
	}

	/**
	 * Convert indented XML String.
	 *
	 * @param node Node or Element
	 * @return XML String
	 * @throws TransformerException
	 */
	public static String toIndentedXML(org.w3c.dom.Node node)
			throws TransformerException {
		Transformer transformer = TransformerFactory.newInstance()
				.newTransformer();
		transformer.setOutputProperty(javax.xml.transform.OutputKeys.INDENT,
				"yes");
		transformer.setOutputProperty(javax.xml.transform.OutputKeys.METHOD,
				"xml");
		transformer.setOutputProperty(
				javax.xml.transform.OutputKeys.OMIT_XML_DECLARATION, "yes");
		transformer.setOutputProperty(
				org.apache.xml.serializer.OutputPropertiesFactory.S_KEY_INDENT_AMOUNT,
				"2");
		DOMSource source = new DOMSource(node);
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		Result output = new StreamResult(baos);
		transformer.transform(source, output);

		return baos.toString();
	}

	/**
	 * Convert a File instance to EndpointReferenceType.
	 *
	 * @param file a File
	 * @return EPR
	 * @throws DeserializationException
	 * @throws IOException
	 */
	public static EndpointReferenceType toEPR(File file)
			throws DeserializationException, IOException {
		InputStream in = new FileInputStream(file);
		EndpointReferenceType epr;
		try {
			epr = toEPR(in);
		} catch (DeserializationException e) {
			throw e;
		} catch (IOException e) {
			throw e;
		} finally {
			in.close();
		}
		return epr;
	}

	/**
	 * Convet a XML String to EndpointReferenceType.
	 *
	 * @param xml XML String
	 * @return EPR
	 * @throws DeserializationException
	 * @throws IOException
	 */
	public static EndpointReferenceType toEPR(String xml)
			throws DeserializationException, IOException {
		ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
		return toEPR(in);
	}

	/**
	 * Convert InputStream data to EndpointReferenceType.
	 *
	 * @param in InputStream
	 * @return EPR
	 * @throws DeserializationException
	 * @throws IOException
	 */
	public static EndpointReferenceType toEPR(InputStream in)
			throws DeserializationException, IOException {
		EndpointReferenceType epr;
		try {
			epr = (EndpointReferenceType) ObjectDeserializer.deserialize(
					new InputSource(in), EndpointReferenceType.class);
		} catch (DeserializationException e) {
			throw e;
		} finally {
			in.close();
		}
		return epr;
	}

	/**
	 * Convert URI to EndpointReferenceType of a RNS root directory for this
	 * implementation.
	 *
	 * @param uri URI
	 * @return EPR
	 */
	public static EndpointReferenceType toRNSEPR(URI uri) {
		EndpointReferenceType epr = new EndpointReferenceType();
		epr.setAddress(new Address(uri));
		QName keyName = RNSQNames.RESOURCE_ID;
		ReferenceParametersType props = new ReferenceParametersType();
		SimpleResourceKey key = new SimpleResourceKey(keyName,
				RNSConfig.getRootID());
		try {
			props.add(key.toSOAPElement());
		} catch (SerializationException e) {
			throw new RuntimeException("getRNSEPRFromURI: " + e.getMessage());
		}
		epr.setParameters(props);
		return epr;
	}

	/**
	 * Convert URI to EndpointReferenceType of normal URI string.
	 *
	 * @param uri URI
	 * @return EPR
	 */
	public static EndpointReferenceType toEPR(URI uri) {
		EndpointReferenceType epr = new EndpointReferenceType();
		epr.setAddress(new Address(uri));
		return epr;
	}

	/**
	 * Convert EPR to XML String.
	 *
	 * @param epr EPR
	 * @return XML String
	 * @throws SerializationException
	 */
	public static String toXMLString(EndpointReferenceType epr)
			throws SerializationException {
		return ObjectSerializer.toString(epr,
				EndpointReferenceType.getTypeDesc().getXmlType());
	}

	/**
	 * Get a RNS Resource ID (directory ID) from EPR.
	 *
	 * @param epr EPR
	 * @return Resource ID of the RNS directory
	 */
	public static String getRNSResourceId(EndpointReferenceType epr) {
		ReferenceParametersType rpt = epr.getParameters();
		if (rpt != null) {
			if (rpt.size() <= 0) {
				return null;
			}
			MessageElement elm = rpt.get(RNSQNames.RESOURCE_ID);
			if (elm != null) {
				/* support my RNS implement */
				return elm.getValue();
			}
		}
		return null; /* not support */
	}

	/* ------------------------------------------------------------------- */

	/**
	 * Convert XML Strings to MessageElement array.
	 *
	 * @param xmlStrs XML String
	 * @return MessageElement array
	 * @throws Exception
	 */
	public static MessageElement[] toMessageElements(String xmlStrs)
			throws Exception {
		String[] xs = Pattern.compile("^----$", Pattern.MULTILINE).split(
				xmlStrs);
		return toMessageElements(xs);
	}

	/**
	 * Convert XML String array to MessageElement array.
	 *
	 * @param xmlStrs XML String array
	 * @return MessageElement array
	 * @throws Exception
	 */
	public static MessageElement[] toMessageElements(String[] xmlStrs)
			throws Exception {
		if (xmlStrs == null) {
			return null;
		}
		int count = 0;
		ArrayList<MessageElement> ml = new ArrayList<MessageElement>();
		for (String x : xmlStrs) {
			count++;
			x = x.replaceAll("\\r|\\n", "");
			if (Pattern.matches("\\A\\s*\\z", x)) {
				continue;
			}
			try {
				ml.add(toMessageElement(x));
			} catch (Exception e) {
				throw e;
			}
		}
		return ml.toArray(new MessageElement[0]);
	}

	/**
	 * Convert InputStream to MessageElement array.
	 *
	 * @param in InputStream
	 * @return MessageElement array
	 * @throws Exception
	 */
	public static MessageElement[] toMessageElements(InputStream in)
			throws Exception {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		int bsize = 65536;
		byte[] buf = new byte[bsize];
		int len;
		while ((len = in.read(buf)) != -1) {
			baos.write(buf, 0, len);
		}
		return toMessageElements(baos.toString());
	}

	/**
	 * Convert File to MessageElement array.
	 *
	 * @param file File
	 * @return MessageElement array
	 * @throws Exception
	 */
	public static MessageElement[] toMessageElements(File file)
			throws Exception {
		FileInputStream in = new FileInputStream(file);
		MessageElement[] mes = toMessageElements(in);
		in.close();
		return mes;
	}

	/**
	 * Check whether the RNS Metadata means a directory.
	 *
	 * @param meta RNS Metadata
	 * @return true if the Metadata means a directory
	 */
	public static boolean isDirectory(RNSMetadataType meta) {
		if (meta == null) {
			return false; /* legal: nillable="true" */
		}
		if (meta.getSupportsRns() == null) {
			return false; /* illegal */
		}
		if (meta.getSupportsRns().getValue() == null) {
			return false; /* illegal */
		}

		return RNSSupportType.value1.getValue().equals(
				meta.getSupportsRns().getValue().getValue());
	}

	/**
	 * Normalize a pathname.
	 *
	 * Example: //a/../b/ -> /b
	 *
	 * @param path a pathname
	 * @return normalized pathname
	 */
	public static String normalizePath(String path) {
		String newPath = "";
		String[] p = path.split("/");
		if (p == null || p.length == 0) {
			if (path.startsWith("/")) {
				return "/";
			}
			return "";
		}
		for (int i = 0; i < p.length; i++) {
			if (p[i].equals("..")) {
				p[i] = "";
				for (int j = i - 1; j >= 0; j--) {
					if (p[j].length() != 0) {
						p[j] = "";
						break;
					}
				}
			}
		}
		boolean first = true;
		for (int i = 0; i < p.length; i++) {
			String name = p[i];
			if (name.length() == 0 || name.equals(".")) {
				/* skip */
				continue;
			}
			if (first) {
				newPath = name;
				first = false;
			} else {
				newPath = newPath + "/" + name;
			}
		}
		if (path.startsWith("/")) {
			newPath = "/" + newPath;
		}
		return newPath;
	}

	/**
	 * Join two pathname.
	 *
	 * @param parent parent directory pathname
	 * @param child child pathname
	 * @return joined pathname
	 */
	public static String joinPath(String parent, String child) {
		if (parent == null) {
			if (child == null) {
				return null;
			}
			return normalizePath(child);
		} else {
			parent = normalizePath(parent);
			if (child == null) {
				return parent;
			}
			child = normalizePath(child);
			if (parent.equals("/")) {
				if (child.startsWith("/")) {
					return child;
				} else {
					return "/" + child;
				}
			} else {
				if (child.startsWith("/")) {
					return parent + child;
				} else {
					return parent + "/" + child;
				}
			}
		}
	}

	/**
	 * Convert org.globus.wsrf.security.authorization.client.Authorization to
	 * org.globus.gsi.gssapi.auth.Authorization.
	 *
	 * @param authz org.globus.wsrf.security.authorization.client.Authorization
	 * @return org.globus.gsi.gssapi.auth.Authorization (null if authz is
	 *         unknown type)
	 */
	public static org.globus.gsi.gssapi.auth.Authorization convertGSSAPIAuthorization(
			Authorization authz) {
		if (authz instanceof NoAuthorization) {
			return org.globus.gsi.gssapi.auth.NoAuthorization.getInstance();
		} else if (authz instanceof HostAuthorization) {
			return org.globus.gsi.gssapi.auth.HostAuthorization.getInstance();
		} else if (authz instanceof HostOrSelfAuthorization) {
			return org.globus.gsi.gssapi.auth.HostOrSelfAuthorization.getInstance();
		} else if (authz instanceof SelfAuthorization) {
			return org.globus.gsi.gssapi.auth.SelfAuthorization.getInstance();
		} else if (authz instanceof IdentityAuthorization) {
			return new org.globus.gsi.gssapi.auth.IdentityAuthorization(
					((IdentityAuthorization) authz).getIdentity());
		} else {
			return null;
		}
	}

	/**
	 * Convert authorization type name to Authorization.
	 *
	 * @param name authorization type name: none, host, hostSelf, self
	 * @return Authorization
	 */
	public static org.globus.gsi.gssapi.auth.Authorization getGSSAPIAuthorizationFromName(
			String name) {
		if ("none".equals(name)) {
			return org.globus.gsi.gssapi.auth.NoAuthorization.getInstance();
		} else if ("host".equals(name)) {
			return org.globus.gsi.gssapi.auth.HostAuthorization.getInstance();
		} else if ("hostSelf".equals(name)) {
			return org.globus.gsi.gssapi.auth.HostOrSelfAuthorization.getInstance();
		} else if ("self".equals(name)) {
			return org.globus.gsi.gssapi.auth.SelfAuthorization.getInstance();
		} else if (name != null) {
			return new org.globus.gsi.gssapi.auth.IdentityAuthorization(name);
		} else { // name == null
			return null;
		}
	}

	/**
	 * Strip dirname from pathname.
	 *
	 * @param path a pathname
	 * @return dirname
	 * @throws RNSError
	 */
	public static String getDirname(String path) throws RNSError {
		DirnameBasename dbn = new DirnameBasename(path);
		if (dbn.getDirname() == null) {
			return "/";
		} else {
			return dbn.getDirname();
		}
	}

	/**
	 * Strip basename from pathname.
	 *
	 * @param path a pathname
	 * @return basename
	 * @throws RNSError
	 */
	public static String getBasename(String path) throws RNSError {
		DirnameBasename dbn = new DirnameBasename(path);
		if (dbn.getBasename() == null) {
			return "/";
		} else {
			return dbn.getBasename();
		}
	}

	/**
	 * Unescape escaped XML String.
	 *
	 * @param str XML String
	 * @return XML String
	 */
	public static String unescapeXML(String str) {
		return str.replaceAll("&lt;", "<")
				.replaceAll("&gt;", ">")
				.replaceAll("&apos;", "'")
				.replaceAll("&quot;", "\"")
				.replaceAll("&amp;", "&");
	}

	/**
	 * Escape XML String.
	 *
	 * @param str XML String
	 * @return XML String
	 */
	public static String escapeXML(String str) {
		return str.replaceAll("&", "&amp;")
				.replaceAll("'", "&apos;")
				.replaceAll("\"", "&quot;")
				.replaceAll("<", "&lt;")
				.replaceAll(">", "&gt;");
	}

	/**
	 * Get String from File data.
	 *
	 * @param file File
	 * @return String
	 * @throws IOException
	 */
	public static String fileToString(File file) throws IOException {
		return inputStreamToString(new FileInputStream(file));
	}

	/**
	 * Get String from InputStream.
	 *
	 * @param in InputStream
	 * @return String
	 * @throws IOException
	 */
	public static String inputStreamToString(InputStream in) throws IOException {
		BufferedReader br = null;
		try {
			br = new BufferedReader(new InputStreamReader(in));
			StringBuffer sb = new StringBuffer();
			int c;
			while ((c = br.read()) != -1) {
				sb.append((char) c);
			}
			return sb.toString();
		} finally {
			br.close();
		}
	}

	/**
	 * Generate XQuery String for RNS Search operation.
	 *
	 * This is the same as RNSUtil#generateXQueryForRNSSearch(retv, null). See
	 * {@link RNSUtil#generateXQueryForRNSSearch(String, String)}.
	 *
	 * @return XQuery String
	 */
	public static String generateXQueryForRNSSearch(String retv) {
		return generateXQueryForRNSSearch(retv, null);
	}

	/**
	 * Generate XQuery String for RNS Search operation.
	 *
	 * <pre>
	 * --- Available variables in retv ---
	 * $ent := /ns1:RNSEntryResponseType
	 * $name := string($ent/@entry-name)
	 * $epr := $ent/ns1:endpoint
	 * $meta := $ent/ns1:metadata
	 * $sptrns := $meta/ns1:supports-rns
	 * </pre>
	 *
	 * @param retv XML (a part of &lt;ns1:metadata&gt; for return value)
	 * @param kvKey a key name of RNSKeyValue (not search RNSKeyValue if kvKey
	 *            is null)
	 * @return Generated XQuery
	 */
	public static String generateXQueryForRNSSearch(String retv, String kvKey) {
		String xq = "declare namespace ns1 = \"http://schemas.ogf.org/rns/2009/12/rns\"; "
				+ "let $ent := /ns1:RNSEntryResponseType "
				+ "let $name := string($ent/@entry-name) "
				+ "let $epr := $ent/ns1:endpoint "
				+ "let $meta := $ent/ns1:metadata "
				+ "let $sptrns := $meta/ns1:supports-rns ";
		if (kvKey != null) {
			xq += "let $rnskv := $meta/rnskv " + "let $key := $rnskv[@key=\""
					+ kvKey + "\"] " + "let $value := $key/text() ";
		}
		if (retv != null) {
			xq += "let $retv := " + retv + " ";
		}
		if (kvKey != null) {
			xq += "where exists($key) ";
		}
		xq += "return " + "<ns1:RNSEntryResponseType entry-name=\"{$name}\" "
				+ "xmlns:ns1=\"http://schemas.ogf.org/rns/2009/12/rns\" "
				+ "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
				+ "xsi:type=\"ns1:RNSEntryResponseType\">" + "{$epr}"
				+ "<ns1:metadata xsi:type=\"ns1:RNSMetadataType\">"
				+ "{$sptrns}";
		if (retv != null) {
			xq += "{$retv}";
		}
		xq += "</ns1:metadata></ns1:RNSEntryResponseType>";
		return xq;
	}

	/**
	 * Run XQuery to the XML at local machine.
	 *
	 * @param xml XML
	 * @param xquery XQuery
	 * @return result
	 * @throws XPathException
	 * @throws IOException
	 */
	public static String localXQuery(String xml, String xquery)
			throws XPathException, IOException {
		final Configuration config = new Configuration();
		final StaticQueryContext sqc = config.newStaticQueryContext();
		final Properties props = new Properties();

		props.setProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
		props.setProperty(OutputKeys.INDENT, "yes");
		final XQueryExpression exp;
		exp = sqc.compileQuery(new StringReader(xquery));

		StringWriter sw = new StringWriter();
		final DynamicQueryContext dynamicContext = new DynamicQueryContext(
				config);
		dynamicContext.setContextItem(config.buildDocument(new StreamSource(
				new StringReader(xml))));
		exp.run(dynamicContext, new StreamResult(sw), props);
		return sw.toString();
	}

	/**
	 * Force GC.
	 */
	public static void gc() {
		long pf = RNSProfiler.start();
		System.gc();
		RNSProfiler.stop(RNSProfiler.TYPE.IntentionalGC, pf);
	}

	private static long minMem = RNSConfig.minFreeMemory();
	private static Runtime runtime = Runtime.getRuntime();

	/**
	 * Force GC when the system memory is lower than rns.server.minMemory
	 *
	 * @return false if the system memory cannot be increased
	 */
	public static synchronized boolean checkMemory(boolean enableLog) {
		long nowMem = runtime.freeMemory();
		if (nowMem < minMem) {
			System.out.print("[RNS] force GC: before/after/total=" + nowMem
					+ "/");
			gc();
			nowMem = runtime.freeMemory();
			System.out.println(nowMem + "/" + runtime.totalMemory());
			if (nowMem < minMem) {
				if (enableLog) {
					String msg = "lack of memory: now/require=" + nowMem + "/"
							+ minMem;
					System.out.println(msg);
					RNSLog.getLog().error(msg);
				}
				return false; /* no memory */
			}
		}
		return true;
	}

	/**
	 * Convert BaseFaultType to RNSError.
	 *
	 * @param fault BaseFaultType
	 * @param path1 for RNSEntryDoesNotExistFaultType
	 * @param path2 for RNSEntryExistsFaultType
	 * @return RNSError
	 */
	public static RNSError convertBaseFault(BaseFaultType fault, String path1,
			String path2) {
		if (fault != null) {
			if (fault instanceof RNSEntryDoesNotExistFaultType) {
				return RNSError.createENOENT(path1, null);
			} else if (fault instanceof RNSEntryExistsFaultType) {
				return RNSError.createEEXIST(path2, null);
			} else {
				return RNSError.createEUNEXPECTED(
						path1,
						"not RNSEntryDoesNotExistFaultType and not RNSEntryExistsFaultType",
						null);
			}
		} else {
			return null;
		}
	}
}
