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
import java.util.Iterator;

import javax.xml.rpc.ServiceException;

import org.apache.axis.types.URI;
import org.apache.axis.types.URI.MalformedURIException;
import org.apache.axis.types.UnsignedInt;
import org.apache.axis.types.UnsignedLong;
import org.globus.axis.message.addressing.AttributedURIType;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.ObjectDeserializer;
import org.naregi.rns.RNSConfig;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.stubs.IterableElementType;
import org.naregi.rns.stubs.IterateRequestType;
import org.naregi.rns.stubs.IterateResponseType;
import org.naregi.rns.stubs.RNSEntryResponseType;
import org.naregi.rns.stubs.RNSSearchPortType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.SearchFaultType;
import org.naregi.rns.stubs.SearchRequestType;
import org.naregi.rns.stubs.SearchResponseType;
import org.naregi.rns.stubs.WSIteratorPortType;
import org.naregi.rns.util.RNSUtil;
import org.oasis.wsrf.properties.GetResourcePropertyResponse;

/**
 * RNSSearchClientImpl implementation.
 */
public class RNSSearchClientImpl implements RNSSearchClient {
	private RNSClient rnsClient;
	private RNSClientHome home;

	RNSSearchClientImpl(RNSClientHome home) throws Exception {
		this.home = home;
		this.rnsClient = home.getRNSClient();
	}

	private void debug(String method, String path, String msg) {
		if (home != null) {
			home.debug(method, path, msg);
		}
	}

	@Override
	public RNSSearchResult search(String path, String xquery) throws RNSError {
		debug("search", path, null);
		DirnameBasename dbn = new DirnameBasename(path);
		if (dbn.getBasename() == null) {
			throw RNSError.createEINVAL("/", "Root directory has no metadata",
					null);
		}
		rnsClient.getEPR(path, true); /* check existence */
		String[] names = new String[1];
		names[0] = dbn.getBasename();
		RNSSearchResultHandle results = searchBulk(dbn.getDirname(), names,
				xquery);
		if (results == null) {
			return null;
		}
		RNSSearchResult ret = null;
		for (RNSSearchResult result : results) {
			if (ret == null) {
				ret = result;
			} else {
				/* unexpected */
				throw RNSError.createEUNEXPECTED(dbn.getDirname(),
						"RNSSearchResult is not one", null);
			}
		}
		RNSError e = results.getError();
		if (e != null) {
			throw e;
		}
		return ret;
	}

	private RNSSearchPortType rnsEprToRNSSearchPortType(
			final EndpointReferenceType rnsEpr, String path) throws RNSError {
		/* replace path of URL */
		/* deep copy */
		EndpointReferenceType epr = new EndpointReferenceType(rnsEpr, true);
		/* new AttributedURIType (don't modify rnsEpr) */
		AttributedURIType address = new AttributedURIType();
		URI uri = new URI(rnsEpr.getAddress().getValue());
		try {
			uri.setPath(RNSConfig.getSearchServicePath());
		} catch (MalformedURIException e) {
			throw RNSError.createEUNEXPECTED(null, null, e);
		}
		address.setValue(uri);
		epr.setAddress(address);

		try {
			return home.getRNSSearchPortType(epr, path);
		} catch (Exception e) {
			throw RNSError.createEINVAL(path, e.getMessage(), e);
		}
	}

	private String regularPath(String path) {
		if (path == null) {
			return "";
		}
		while (path.contains("//")) {
			path = path.replaceAll("//", "/");
		}
		if (path.equals("/")) {
			path = "";
		}
		return path;
	}

	private class RNSSearchResultHandleIterator implements
			RNSIterator<RNSSearchResult> {
		private String dirPath;

		private int preferredBlockSize;
		private WSIteratorPortType iteratorPort;

		private long nextOffset;
		private int cachedOffset;
		private RNSEntryResponseType[] cache;
		private boolean end;

		private RNSError saveError = null;

		RNSSearchResultHandleIterator(String dirPath, SearchResponseType srt)
				throws Exception {
			this.dirPath = dirPath;

			EndpointReferenceType iteratorEPR = srt.getIterator();
			if (iteratorEPR != null) {
				/* access SearchIteratorService */
				iteratorPort = home.getWSIteratorPortType(iteratorEPR, dirPath);

				GetResourcePropertyResponse pb = iteratorPort.getResourceProperty(RNSQNames.ITERATOR_RP_PREFERREDBLOCKSIZE);
				if (pb != null) {
					preferredBlockSize = Integer.parseInt(pb.get_any()[0].getValue());
				}
				if (preferredBlockSize <= 0) {
					preferredBlockSize = 100;
				}
				cache = srt.getEntryResponse();
				cachedOffset = 0;
				if (cache != null) {
					nextOffset = cache.length;
				}
				end = false;
			} else {
				/* not use WSIterator */
				cache = srt.getEntryResponse();
				end = true;
			}
		}

		@Override
		public boolean hasNext() {
			if (cache != null && cachedOffset < cache.length) {
				return true;
			} else if (end || cache.length == 0) {
				return false;
			}

			IterateRequestType itrReq = new IterateRequestType();
			itrReq.setStartOffset(new UnsignedLong(nextOffset));
			itrReq.setElementCount(new UnsignedInt(preferredBlockSize));
			IterateResponseType res;
			try {
				res = iteratorPort.iterate(itrReq);
			} catch (RemoteException e) {
				if (home.isDebugMode()) {
					e.printStackTrace();
				}
				if (saveError == null) {
					saveError = RNSError.createENET(dirPath, null, e);
				}
				return false;
			}
			if (res == null) {
				end = true;
				return false;
			}
			IterableElementType[] ies = res.getIterableElement();
			if (ies == null || ies.length == 0) {
				end = true;
				return false;
			}
			RNSEntryResponseType[] tmp = new RNSEntryResponseType[ies.length];
			for (int i = 0; i < ies.length; i++) {
				RNSEntryResponseType result;
				try {
					result = (RNSEntryResponseType) ObjectDeserializer.toObject(
							ies[i].get_any()[0], RNSEntryResponseType.class);
				} catch (DeserializationException e) {
					if (home.isDebugMode()) {
						e.printStackTrace();
					}
					if (saveError == null) {
						saveError = RNSError.createENET(dirPath, null, e);
					}
					end = false;
					cache = null;
					return false;
				}
				tmp[i] = result;
			}
			cache = tmp;
			cachedOffset = 0;
			if (ies.length < preferredBlockSize) {
				end = true;
			} else {
				nextOffset += preferredBlockSize;
			}
			return true;
		}

		@Override
		public RNSSearchResult next() {
			if (hasNext() == false) {
				return null;
			}
			RNSEntryResponseType rert = cache[cachedOffset];
			RNSSearchResult rsl = new RNSSearchResult();
			rsl.setPath(regularPath(dirPath) + "/" + rert.getEntryName());
			rsl.setEntryResponseType(rert);
			cachedOffset++;
			return rsl;
		}

		@Override
		public void remove() {
			throw new UnsupportedOperationException();
		}

		@Override
		public RNSError getError() {
			return saveError;
		}
	}

	@Override
	public RNSSearchResultHandle searchBulk(String dirPath, String[] names,
			String xquery) throws RNSError {
		debug("searchBulk", dirPath, null);
		if (rnsClient.isDirectory(dirPath) == false) {
			throw RNSError.createENOTDIR(dirPath,
					"searchBulk needs a directory");
		}
		EndpointReferenceType epr = rnsClient.getEPR(dirPath, true);
		RNSSearchPortType port;
		port = rnsEprToRNSSearchPortType(epr, dirPath);
		SearchRequestType req = new SearchRequestType();
		req.setEntryName(names);
		req.setQuery(xquery);
		try {
			SearchResponseType rsp = port.search(req);
			if (rsp == null) {
				return null;
			}
			return new RNSSearchResultHandle(new RNSSearchResultHandleIterator(
					dirPath, rsp));
		} catch (SearchFaultType e) {
			throw RNSError.createEINVAL(dirPath, e.getFaultString(), null);
		} catch (ServiceException e) {
			throw RNSError.createENET(dirPath, null, e);
		} catch (ReadNotPermittedFaultType e) {
			throw RNSError.createEACCES(dirPath, e.getFaultString());
		} catch (RemoteException e) {
			throw RNSError.createENET(dirPath, null, e);
		} catch (Exception e) {
			throw RNSError.createENET(dirPath, null, e);
		}
	}

	private class RNSSearchResultHandleRecursiveIterator implements
			RNSIterator<RNSSearchResult> {
		private String dirPath;
		private String xquery;
		private int depth;

		private RNSDirHandle dlist = null;
		private RNSSearchResultHandle currentHandle = null;
		private Iterator<RNSDirent> dents = null;
		private Iterator<RNSSearchResult> results = null;

		private RNSError saveError = null;

		RNSSearchResultHandleRecursiveIterator(String dirPath, String xquery,
				int depth) throws RNSError {
			this.dirPath = dirPath;
			this.xquery = xquery;
			this.depth = depth;

			dlist = rnsClient.list(dirPath, false);
			if (dlist != null) {
				dents = dlist.iterator();
			}
			currentHandle = searchBulk(dirPath, null, xquery);
			if (currentHandle != null) {
				results = currentHandle.iterator();
			}
		}

		@Override
		public boolean hasNext() {
			if (results != null) {
				if (results.hasNext()) {
					return true;
				} else {
					RNSError e = currentHandle.getError();
					if (e != null && saveError == null) {
						saveError = e;
						return false;
					}
				}
				/* next child directory */
			}
			if (dents == null) {
				return false;
			}
			if (depth == 1) {
				return false;
			}
			while (dents.hasNext()) {
				RNSDirent ent = dents.next();
				if (ent.isDirectory()) {
					try {
						currentHandle = new RNSSearchResultHandle(
								new RNSSearchResultHandleRecursiveIterator(
										dirPath + "/" + ent.getName(), xquery,
										depth - 1));
						if (currentHandle == null) {
							results = null;
							/* next child directory */
							continue;
						} else {
							results = currentHandle.iterator();
							if (results != null && results.hasNext()) {
								return true;
							}
							/* next child directory */
						}
					} catch (RNSError e) {
						if (home.isDebugMode()) {
							e.printStackTrace();
						}
						if (saveError == null) {
							saveError = e;
						}
						results = null;
						/* next child directory */
						continue;
					}
				}
			}
			RNSError e = dlist.getError();
			if (e != null && saveError == null) {
				saveError = e;
			}
			return false;
		}

		@Override
		public RNSSearchResult next() {
			if (hasNext() == false) {
				return null;
			}
			return results.next();
		}

		@Override
		public void remove() {
			if (results != null) {
				results.remove();
			}
		}

		@Override
		public RNSError getError() {
			return saveError;
		}
	}

	@Override
	public RNSSearchResultHandle searchRecursive(String dirPath, String xquery,
			int depth) throws RNSError {
		debug("searchRecursive", dirPath, null);
		if (rnsClient.isDirectory(dirPath) == false) {
			throw RNSError.createENOTDIR(dirPath,
					"searchRecursive needs a directory");
		}
		return new RNSSearchResultHandle(
				new RNSSearchResultHandleRecursiveIterator(dirPath, xquery,
						depth));
	}

	private class RNSDirHandleListBySearch implements RNSIterator<RNSDirent> {
		private RNSSearchResultHandle rh;
		private Iterator<RNSSearchResult> itr;
		private RNSError saveError = null;

		RNSDirHandleListBySearch(RNSSearchResultHandle rh) {
			this.rh = rh;
			itr = rh.iterator();
		}

		@Override
		public boolean hasNext() {
			boolean b = itr.hasNext();
			if (b == false && saveError == null) {
				saveError = rh.getError();
			}
			return b;
		}

		@Override
		public RNSDirent next() {
			RNSSearchResult result = itr.next();
			if (result == null) {
				if (saveError == null) {
					saveError = rh.getError();
				}
				return null;
			}
			String path = result.getPath();
			if (path == null) {
				return null;
			}
			DirnameBasename dbn;
			try {
				dbn = new DirnameBasename(path);
			} catch (RNSError e) {
				if (home.isDebugMode()) {
					e.printStackTrace();
				}
				if (saveError == null) {
					saveError = e;
				}
				return null;
			}
			String name = dbn.getBasename();
			if (name == null) {
				RNSError e = RNSError.createEUNEXPECTED(path,
						"child name is null", null);
				if (saveError == null) {
					saveError = e;
				}
				if (home.isDebugMode()) {
					e.printStackTrace();
				}
				return null;
			}
			RNSDirent rd = new RNSDirent();
			rd.setName(name);
			rd.setEpr(result.getEntryResponseType().getEndpoint());
			rd.setMeta(result.getEntryResponseType().getMetadata());
			return rd;
		}

		@Override
		public void remove() {
			itr.remove();
		}

		@Override
		public RNSError getError() {
			return saveError;
		}
	}

	@Override
	public RNSDirHandle listBySearch(String dirPath) throws RNSError {
		debug("listBySearch", dirPath, null);
		/* get name list without EPR and Metadata */
		String xq = RNSUtil.generateXQueryForRNSSearch(null, null);
		RNSSearchResultHandle rh = searchBulk(dirPath, null, xq);
		if (rh == null) {
			return null;
		}
		return new RNSDirHandle(new RNSDirHandleListBySearch(rh));
	}

}
