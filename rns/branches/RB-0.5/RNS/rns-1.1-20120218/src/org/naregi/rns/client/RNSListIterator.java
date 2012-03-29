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
package org.naregi.rns.client;

import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.UnsignedInt;
import org.apache.axis.types.UnsignedLong;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.ObjectDeserializer;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.stubs.IterableElementType;
import org.naregi.rns.stubs.IterateRequestType;
import org.naregi.rns.stubs.IterateResponseType;
import org.naregi.rns.stubs.RNSEntryResponseType;
import org.naregi.rns.stubs.RNSPortType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.WSIteratorPortType;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.util.TimeoutCacheMap;
import org.oasis.wsrf.properties.GetResourcePropertyResponse;

/**
 * RNS list operation client to access WS-Iterator service.
 */
public class RNSListIterator {
	private EndpointReferenceType epr;
	private WSIteratorPortType port;
	private RNSPortTypeCache rnsPortTypeCache; /* bypass... */
	private RNSClientHome clientHome;
	private String baseDir;

	private static int preferredBlockSize = 0;

	private long nextOffset;
	private ArrayList<RNSDirent> readahead;
	private int cachedOffset;
	private boolean end;
	private List<RNSEntryResponseType> firstents;

	private static class CacheKey {
		EndpointReferenceType epr;
		long offset;
		int blockSize;

		public int hashCode() {
			return epr.toString().hashCode(); // slow?
		}

		public boolean equals(Object o) {
			if (o == null) {
				return false;
			}
			if (this == o) {
				return true;
			}
			if (o instanceof CacheKey) {
				CacheKey c = (CacheKey) o;
				if (c.offset == this.offset && c.blockSize == this.blockSize) {
					if (c.epr == this.epr) {
						return true;
					} else if (c.epr.toString().equals(this.epr.toString())) {
						return true;
					}
				}
			}
			return false;
		}
	}

	private static boolean enableResponseCache = false;

	private static Map<CacheKey, IterateResponseType> responseCache = null;

	/**
	 * Initialize RNS list operation client.
	 *
	 * @param iteratorEpr WS-Iterator EPR
	 * @param clientHome RNSClientHome
	 * @param rnsPortType RNSPortType
	 * @param baseDir a directory pathaname
	 * @param firstents the entries to return firstly if the entries were gotten
	 *            previously
	 * @throws Exception
	 */
	public RNSListIterator(EndpointReferenceType iteratorEpr,
			RNSClientHome clientHome, RNSPortType rnsPortType, String baseDir,
			List<RNSEntryResponseType> firstents) throws Exception {
		this.firstents = firstents;
		this.baseDir = baseDir;

		if (rnsPortType instanceof RNSPortTypeCache) {
			rnsPortTypeCache = (RNSPortTypeCache) rnsPortType;
		} else {
			rnsPortTypeCache = null;
		}
		if (enableResponseCache == true && responseCache == null
				&& clientHome.getCacheTimeout() > 0) {
			responseCache = new TimeoutCacheMap<CacheKey, IterateResponseType>(
					clientHome.getCacheTimeout());
		}
		epr = iteratorEpr;
		port = clientHome.getWSIteratorPortType(epr, baseDir);
		this.clientHome = clientHome;
		if (preferredBlockSize == 0) {
			GetResourcePropertyResponse pb = port.getResourceProperty(RNSQNames.ITERATOR_RP_PREFERREDBLOCKSIZE);
			if (pb != null) {
				preferredBlockSize = Integer.parseInt(pb.get_any()[0].getValue());
			}
			if (preferredBlockSize <= 0) {
				preferredBlockSize = 5;
			}
		}
		nextOffset = 0;
		readahead = null;
		cachedOffset = 0;
		end = false;
	}

	/**
	 * Enable to cache response datas. (default: disable)
	 *
	 * @param enable true if caching of response data is enabled.
	 */
	public static void setResponseCache(boolean enable) {
		enableResponseCache = enable;
	}

	/**
	 * Clear cached response datas.
	 */
	public static void clearResponseCache() {
		if (responseCache != null) {
			responseCache.clear();
		}
	}

	/**
	 * Check whether the next entry exists.
	 *
	 * @return true if the next entry exists
	 * @throws RNSError
	 */
	public boolean hasNextWithError() throws RNSError {
		if (readahead != null && readahead.size() > 0
				&& cachedOffset < readahead.size()) {
			return true;
		} else if (end) {
			return false;
		}
		readahead = new ArrayList<RNSDirent>();
		cachedOffset = 0;

		if (firstents != null) {
			/* first entries in LookupResponseType */
			for (RNSEntryResponseType ent : firstents) {
				RNSDirent rd = new RNSDirent();
				rd.setName(ent.getEntryName());
				rd.setEpr(ent.getEndpoint());
				rd.setMeta(ent.getMetadata());
				if (rnsPortTypeCache != null) {
					rnsPortTypeCache.putCache(ent.getEntryName(), ent);
					clientHome.debug("RNSEntryType@WS-Iterator", null,
							"(enter cache)(bypass)");
				}
				readahead.add(rd);
				nextOffset++;
			}
			firstents = null;
			return true;
		}

		CacheKey key = null;
		IterateResponseType res = null;
		if (responseCache != null) {
			key = new CacheKey();
			key.epr = epr;
			key.offset = nextOffset;
			key.blockSize = preferredBlockSize;
			res = responseCache.get(key);
			if (res == null && responseCache.containsKey(key)) {
				/* null cache */
				return false;
			}
		}
		if (res == null) {
			IterateRequestType req = new IterateRequestType();
			req.setStartOffset(new UnsignedLong(nextOffset));
			req.setElementCount(new UnsignedInt(preferredBlockSize));
			try {
				res = port.iterate(req);
			} catch (ReadNotPermittedFaultType e) {
				throw RNSError.createEACCES(baseDir, null);
			} catch (RemoteException e) {
				throw RNSError.createENET(baseDir, e.getMessage(), e);
			}
			if (responseCache != null && res != null) {
				responseCache.put(key, res);
				clientHome.debug("WS-Iterator#iterate", null,
						"IteratorResponseCache(enter cache)");
			}
		}
		if (res == null) {
			return false;
		}
		IterableElementType[] ie = res.getIterableElement();
		if (ie == null) {
			return false;
		}
		for (int i = 0; i < ie.length; i++) {
			/* long index = ie[i].getIndex().longValue(); unused? */
			MessageElement[] me = ie[i].get_any();
			RNSEntryResponseType ent;
			try {
				ent = (RNSEntryResponseType) ObjectDeserializer.toObject(me[0],
						RNSEntryResponseType.class);
			} catch (DeserializationException e) {
				throw RNSError.createEUNEXPECTED(baseDir, null, e);
			}
			RNSDirent rd = new RNSDirent();
			rd.setName(ent.getEntryName());
			rd.setEpr(ent.getEndpoint());
			rd.setMeta(ent.getMetadata());
			String path = RNSUtil.joinPath(baseDir, ent.getEntryName());
			rd.setRNSError(RNSUtil.convertBaseFault(ent.getFault(), path, path));
			if (rnsPortTypeCache != null) {
				rnsPortTypeCache.putCache(ent.getEntryName(), ent);
				clientHome.debug("RNSEntryType@WS-Iterator", null,
						"(enter cache)(bypass)");
			}
			readahead.add(rd);
			nextOffset++;
		}
		if (ie.length < preferredBlockSize) {
			end = true;
		}
		if (readahead.size() > 0) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Get the next entry.
	 *
	 * @return a next entry
	 * @throws RNSError
	 */
	public RNSDirent nextWithError() throws RNSError {
		if (hasNextWithError()) {
			RNSDirent dent = readahead.get(cachedOffset);
			cachedOffset++;
			return dent;
		}
		return null;
	}
}
