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
import java.util.Calendar;
import java.util.List;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.UnsignedInt;
import org.apache.axis.types.UnsignedLong;
import org.apache.commons.logging.Log;
import org.globus.wsrf.ResourceContext;
import org.globus.wsrf.ResourceContextException;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.encoding.ObjectSerializer;
import org.globus.wsrf.encoding.SerializationException;
import org.naregi.rns.stubs.IterableElementType;
import org.naregi.rns.stubs.IterateRequestType;
import org.naregi.rns.stubs.IterateResponseType;
import org.naregi.rns.stubs.RNSEntryResponseType;

/**
 * A service implementation to return entries by using WS-Iterator interface.
 */
public class ListIteratorService {

	private RNSResource getResource() throws ResourceContextException,
			ResourceException {
		ResourceContext ctx = ResourceContext.getResourceContext();
		return (RNSResource) ctx.getResource();
	}

	public IterateResponseType iterate(IterateRequestType req)
			throws RemoteException {
		long pfTotal = RNSProfiler.start();
		int elementCount = req.getElementCount().intValue();
		long startOffset = req.getStartOffset().longValue();
		RNSResource r = getResource();
		if (r.isReadable() == false) {
			throw new RemoteException("ITERATE: not permitted");
		}
		Log logger = RNSLog.getLog();
		logger.debug("ITERATE");
		List<String> list = r.getList();
		int count = 0;
		ArrayList<IterableElementType> al = new ArrayList<IterableElementType>();
		try {
			for (int i = (int) startOffset; i < list.size()
					&& count < elementCount; i++, count++) {
				String name = list.get(i);
				IterableElementType iet = new IterableElementType();
				RNSEntryData ed = r.getRNSEntryData(name);
				RNSEntryResponseType ent = RNS.setupResponseEntryType(name, ed);
				MessageElement[] mes = new MessageElement[1];
				mes[0] = (MessageElement) ObjectSerializer.toSOAPElement(ent,
						RNSQNames.TYPE_ENTRY_RESPONSE_TYPE);
				iet.set_any(mes);
				iet.setIndex(new UnsignedInt(i));
				al.add(iet);
			}
		} catch (SerializationException e) {
			throw new RemoteException("SerializationException", e);
		}
		IterableElementType[] its = (IterableElementType[]) al.toArray(new IterableElementType[0]);
		IterateResponseType res = new IterateResponseType();
		res.setIterableElement(its);
		res.setIteratorSize(new UnsignedLong(r.getElementCount()));
		synchronized (r.getLockAndStartTransaction()) {
			try {
				r.setAccessTime(Calendar.getInstance());
				if (RNSConfig.isCommitAccessTime()) {
					r.commit();
				}
			} catch (ResourceException e) {
				r.rollback();
				throw e;
			}
		}
		RNSProfiler.stop(RNSProfiler.TYPE.Total_ListIterator, pfTotal);
		return res;
	}
}
