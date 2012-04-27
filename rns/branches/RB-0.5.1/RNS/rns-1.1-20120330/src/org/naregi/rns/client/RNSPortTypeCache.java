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
package org.naregi.rns.client;

import java.rmi.RemoteException;
import java.util.Map;

import javax.xml.namespace.QName;
import javax.xml.rpc.ServiceException;

import org.apache.axis.message.MessageElement;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.stubs.AddRequestType;
import org.naregi.rns.stubs.AddResponseType;
import org.naregi.rns.stubs.LookupRequestType;
import org.naregi.rns.stubs.LookupResponseType;
import org.naregi.rns.stubs.RNSEntryDoesNotExistFaultType;
import org.naregi.rns.stubs.RNSEntryExistsFaultType;
import org.naregi.rns.stubs.RNSEntryResponseType;
import org.naregi.rns.stubs.RNSPortType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.RemoveRequestType;
import org.naregi.rns.stubs.RemoveResponseType;
import org.naregi.rns.stubs.RenameRequestType;
import org.naregi.rns.stubs.RenameResponseType;
import org.naregi.rns.stubs.SetMetadataRequestType;
import org.naregi.rns.stubs.SetMetadataResponseType;
import org.naregi.rns.stubs.WriteNotPermittedFaultType;
import org.naregi.rns.util.TimeoutCacheMap;
import org.oasis.wsrf.lifetime.Destroy;
import org.oasis.wsrf.lifetime.DestroyResponse;
import org.oasis.wsrf.lifetime.ResourceNotDestroyedFaultType;
import org.oasis.wsrf.properties.GetMultipleResourcePropertiesResponse;
import org.oasis.wsrf.properties.GetMultipleResourceProperties_Element;
import org.oasis.wsrf.properties.GetResourcePropertyResponse;
import org.oasis.wsrf.properties.InvalidResourcePropertyQNameFaultType;
import org.oasis.wsrf.resource.ResourceUnavailableFaultType;
import org.oasis.wsrf.resource.ResourceUnknownFaultType;

/**
 * Cache response data of RNS operations (Lookup,
 * GetMultipleResourceProperties) (Directory cache).
 */
public class RNSPortTypeCache implements RNSPortType {

	private static final String returnNull = "null";

	private RNSPortType origPort;
	private RNSClientHome home;

	private Map<String, Object> entryCache;

	private GetMultipleResourcePropertiesResponse rp;
	private long rpTimestamp;

	private Object lookupResponse; /* list mode (if (name == null)) */
	private long lookupResponseTimestamp;

	/**
	 * Convert RNSPortType to RNSPortTypeCache.
	 *
	 * @param port RNSPortTypeCache
	 * @param home RNSClientHome
	 * @throws ServiceException
	 */
	public RNSPortTypeCache(RNSPortType port, RNSClientHome home)
			throws ServiceException {
		this.home = home;
		origPort = port;
		entryCache = new TimeoutCacheMap<String, Object>(home.getCacheTimeout());
		lookupResponse = null;
		rp = null;
	}

	/**
	 * Cache a directory entry.
	 *
	 * @param name an entry name
	 * @param ent RNSEntryResponseType
	 */
	public void putCache(String name, RNSEntryResponseType ent) {
		try {
			entryCache.put(name, ent);
		} catch (OutOfMemoryError e) {
			/* ignore */
			e.printStackTrace();
		}
	}

	/**
	 * Get an entry from this cache.
	 *
	 * @param name an entry name
	 * @return RNSEntryResponseType
	 */
	public RNSEntryResponseType getCache(String name) {
		Object o = entryCache.get(name);
		if (o == null) {
			return null;
		} else if (o instanceof RNSEntryResponseType) {
			return (RNSEntryResponseType) o;
		} else {
			return null;
		}
	}

	private void clear() {
		home.debug("clear cache", null, null);
		/* dc.entryCache.clear(); */
		entryCache = new TimeoutCacheMap<String, Object>(home.getCacheTimeout()); /* synchronized */
		rp = null;
		lookupResponse = null;
		RNSListIterator.clearResponseCache(); /* bypass */
	}

	@Override
	public synchronized AddResponseType add(AddRequestType addrequesttype)
			throws RemoteException, WriteNotPermittedFaultType {
		clear();
		return origPort.add(addrequesttype);
	}

	@Override
	public synchronized DestroyResponse destroy(Destroy destroy1)
			throws RemoteException, ResourceUnknownFaultType,
			ResourceNotDestroyedFaultType, ResourceUnavailableFaultType {
		clear();
		return origPort.destroy(destroy1);
	}

	@Override
	public synchronized GetMultipleResourcePropertiesResponse getMultipleResourceProperties(
			GetMultipleResourceProperties_Element getmultipleresourceproperties_element)
			throws RemoteException, InvalidResourcePropertyQNameFaultType,
			ResourceUnknownFaultType, ResourceUnavailableFaultType {
		if (rp != null) {
			if (System.currentTimeMillis() - rpTimestamp < home.getCacheTimeout()) {
				home.debug("getMultipleResourceProperties", null, "(use cache)");
				return rp;
			}
		}
		home.debug("getMultipleResourceProperties", null, "(enter cache)");
		rp = origPort.getMultipleResourceProperties(getmultipleresourceproperties_element);
		rpTimestamp = System.currentTimeMillis();
		return rp;
	}

	private static QName[] rps = new QName[] { RNSQNames.RP_ELEMENTCOUNT,
			RNSQNames.RP_CREATETIME, RNSQNames.RP_ACCESSTIME,
			RNSQNames.RP_MODIFICATIONTIME, RNSQNames.RP_READABLE,
			RNSQNames.RP_WRITABLE };
	private static GetMultipleResourceProperties_Element multiResReq = new GetMultipleResourceProperties_Element(
			rps);

	@Override
	public synchronized GetResourcePropertyResponse getResourceProperty(
			QName qname) throws RemoteException,
			InvalidResourcePropertyQNameFaultType, ResourceUnknownFaultType,
			ResourceUnavailableFaultType {
		getMultipleResourceProperties(multiResReq);
		if (rp == null) {
			/* unexpected */
			throw new ResourceUnavailableFaultType();
		}
		MessageElement[] ms = rp.get_any();
		if (ms != null) {
			MessageElement[] setms = new MessageElement[1];
			for (MessageElement m : ms) {
				if (RNSQNames.RP_ELEMENTCOUNT.equals(qname)
						|| RNSQNames.RP_CREATETIME.equals(qname)
						|| RNSQNames.RP_ACCESSTIME.equals(qname)
						|| RNSQNames.RP_MODIFICATIONTIME.equals(qname)
						|| RNSQNames.RP_READABLE.equals(qname)
						|| RNSQNames.RP_WRITABLE.equals(qname)) {
					setms[0] = m;
					GetResourcePropertyResponse res = new GetResourcePropertyResponse();
					res.set_any(setms);
					return res;
				}
			}
		}
		throw new InvalidResourcePropertyQNameFaultType();
	}

	@Override
	public synchronized LookupResponseType lookup(
			LookupRequestType lookuprequesttype) throws RemoteException,
			ReadNotPermittedFaultType {
		String[] names = lookuprequesttype.getEntryName();
		if (names != null) {
			if (names.length == 1) {
				String name = names[0];
				Object o = entryCache.get(name);
				if (o != null) {
					home.debug("lookup", null, "name=" + name + " (use cache)");
					if (o instanceof RNSEntryResponseType) {
						LookupResponseType res = new LookupResponseType();
						RNSEntryResponseType[] ents = new RNSEntryResponseType[1];
						ents[0] = (RNSEntryResponseType) o;
						res.setEntryResponse(ents);
						return res;
					} else {
						/* o instanceof RNSEntryDoesNotExistFaultType */
						throw (RNSEntryDoesNotExistFaultType) o;
					}
				}
				try {
					LookupResponseType res = origPort.lookup(lookuprequesttype);
					RNSEntryResponseType ent = null;
					if (res != null) {
						RNSEntryResponseType[] ents = res.getEntryResponse();
						if (ents != null && ents.length == 1) {
							ent = ents[0];
						}
					}
					entryCache.put(name, ent);
					home.debug("lookup", null, "name=" + name
							+ " (enter cache)");
					return res;
				} catch (RNSEntryDoesNotExistFaultType e) {
					entryCache.put(name, e);
					throw e;
				}
			} else { /* names.length > 1 : not cache */
				try {
					LookupResponseType res = origPort.lookup(lookuprequesttype);
					return res;
				} catch (RNSEntryDoesNotExistFaultType e) {
					throw e;
				}
			}
		} else { /* list */
			/* use cache */
			if (lookupResponse != null) {
				if (System.currentTimeMillis() - lookupResponseTimestamp < home.getCacheTimeout()) {
					if (lookupResponse instanceof LookupResponseType) {
						return (LookupResponseType) lookupResponse;
					} else if (lookupResponse instanceof RNSEntryDoesNotExistFaultType) {
						throw (RNSEntryDoesNotExistFaultType) lookupResponse;
					} else if (lookupResponse instanceof String
							&& lookupResponse == returnNull) {
						return null;
					}
				}
			}
			LookupResponseType res;
			try {
				res = origPort.lookup(lookuprequesttype);
			} catch (RNSEntryDoesNotExistFaultType e) {
				/* enter cache (NOENT) */
				lookupResponse = e;
				lookupResponseTimestamp = System.currentTimeMillis();
				throw e;
			}
			if (res == null
					|| ((res.getEntryResponse() == null || res.getEntryResponse().length == 0) && res.getIterator() == null)) {
				/* enter cache (null) */
				lookupResponse = returnNull;
				lookupResponseTimestamp = System.currentTimeMillis();
				return null;
			}
			if (res.getEntryResponse() != null) {
				for (RNSEntryResponseType ent : res.getEntryResponse()) {
					if (ent != null) {
						/* put entries into cache */
						home.debug("list", null, "name=" + ent.getEntryName()
								+ " (enter cache)");
						entryCache.put(ent.getEntryName(), ent);
					}
				}
			}
			home.debug("lookup", null, "listResponse (enter cache)");
			/* enter cache (raw LookupResponse) */
			lookupResponse = res;
			lookupResponseTimestamp = System.currentTimeMillis();
			return res;
		}
	}

	@Override
	public synchronized RemoveResponseType remove(
			RemoveRequestType removerequesttype) throws RemoteException,
			WriteNotPermittedFaultType {
		clear();
		return origPort.remove(removerequesttype);
	}

	@Override
	public synchronized RenameResponseType rename(
			RenameRequestType renamerequesttype) throws RemoteException,
			RNSEntryExistsFaultType, RNSEntryDoesNotExistFaultType,
			WriteNotPermittedFaultType {
		clear();
		return origPort.rename(renamerequesttype);
	}

	@Override
	public synchronized SetMetadataResponseType setMetadata(
			SetMetadataRequestType setmetadatarequesttype)
			throws RemoteException, RNSEntryDoesNotExistFaultType,
			WriteNotPermittedFaultType {
		clear();
		return origPort.setMetadata(setmetadatarequesttype);
	}
}
