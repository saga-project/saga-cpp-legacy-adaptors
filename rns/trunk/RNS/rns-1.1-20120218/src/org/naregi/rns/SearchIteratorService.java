/*
 * Copyright (C) 2008-2011 Osaka University.
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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

import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.List;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.UnsignedInt;
import org.apache.axis.types.UnsignedLong;
import org.apache.commons.logging.Log;
import org.globus.wsrf.ResourceContext;
import org.globus.wsrf.ResourceContextException;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.ObjectSerializer;
import org.globus.wsrf.encoding.SerializationException;
import org.naregi.rns.stubs.IterableElementType;
import org.naregi.rns.stubs.IterateRequestType;
import org.naregi.rns.stubs.IterateResponseType;
import org.naregi.rns.stubs.RNSEntryResponseType;

/**
 * A service implementation to return results of Search operation by using
 * WS-Iterator interface.
 */
public class SearchIteratorService {

	private SearchIteratorResource getResource()
			throws ResourceContextException, ResourceException {
		ResourceContext ctx = ResourceContext.getResourceContext();
		return (SearchIteratorResource) ctx.getResource();
	}

	public IterateResponseType iterate(IterateRequestType req)
			throws RemoteException {
		long pfTotal = RNSProfiler.start();
		SearchIteratorResource r = getResource();
		List<String> results = r.getSessionResults();
		int elementCount = req.getElementCount().intValue();
		long startOffset = req.getStartOffset().longValue();
		if (startOffset > Integer.MAX_VALUE) {
			throw new RemoteException(
					"not supported: startOffset > Integer.MAX_VALUE("
							+ Integer.MAX_VALUE + ")");
		}

		Log logger = RNSLog.getLog();
		logger.debug("Search ITERATE");

		int count = 0;
		ArrayList<IterableElementType> al = new ArrayList<IterableElementType>();
		try {
			long pf;
			for (int i = (int) startOffset; i < results.size()
					&& count < elementCount; i++, count++) {
				String entStr = results.get(i);
				if (entStr == null) {
					continue;
				}

				IterableElementType iet = new IterableElementType();
				MessageElement[] mes = new MessageElement[1];

				pf = RNSProfiler.start();
				RNSEntryResponseType rert = SearchService.convertStringToRNSEntryResponseType(entStr);
				RNSProfiler.stop(RNSProfiler.TYPE.Search_Deserialize, pf);

				mes[0] = (MessageElement) ObjectSerializer.toSOAPElement(rert,
						RNSQNames.TYPE_ENTRY_RESPONSE_TYPE);
				iet.set_any(mes);
				iet.setIndex(new UnsignedInt(i));
				al.add(iet);
			}
		} catch (DeserializationException e) {
			throw new RemoteException("DeserializationException", e);
		} catch (SerializationException e) {
			throw new RemoteException("SerializationException", e);
		}

		IterableElementType[] its = (IterableElementType[]) al.toArray(new IterableElementType[0]);
		IterateResponseType res = new IterateResponseType();
		res.setIterableElement(its);
		res.setIteratorSize(new UnsignedLong(r.getElementCount()));

		RNSProfiler.stop(RNSProfiler.TYPE.Total_SearchIterator, pfTotal);
		return res;
	}
}
