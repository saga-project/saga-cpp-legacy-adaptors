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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.axis.types.URI;
import org.apache.axis.types.URI.MalformedURIException;
import org.globus.axis.message.addressing.AttributedURIType;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.ACL;
import org.naregi.rns.CallerInfo;
import org.naregi.rns.RNSConfig;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.stubs.ACLEntryType;
import org.naregi.rns.stubs.ACLFaultType;
import org.naregi.rns.stubs.GetACLRequestType;
import org.naregi.rns.stubs.GetACLResponseType;
import org.naregi.rns.stubs.GetCallerInfoRequestType;
import org.naregi.rns.stubs.GetCallerInfoResponseType;
import org.naregi.rns.stubs.GetServerStatusRequestType;
import org.naregi.rns.stubs.GetServerStatusResponseType;
import org.naregi.rns.stubs.NoopRequestType;
import org.naregi.rns.stubs.ProfileType;
import org.naregi.rns.stubs.RNSExtensionPortType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.RemoveACLRequestType;
import org.naregi.rns.stubs.SetACLRequestType;
import org.naregi.rns.stubs.StartProfileRequestType;
import org.naregi.rns.stubs.StopProfileRequestType;
import org.naregi.rns.stubs.StopProfileResponseType;
import org.naregi.rns.stubs.StringMapType;
import org.naregi.rns.util.TimeoutCacheMap;
import org.oasis.wsrf.properties.GetResourcePropertyResponse;
import org.oasis.wsrf.properties.InvalidResourcePropertyQNameFaultType;
import org.oasis.wsrf.resource.ResourceUnavailableFaultType;
import org.oasis.wsrf.resource.ResourceUnknownFaultType;

/**
 * RNSExtensionClient implementation.
 */
public class RNSExtensionClientImpl implements RNSExtensionClient {
	private RNSExtensionPortType rootPort;
	private RNSClient rnsClient;
	private RNSClientHome home;

	RNSExtensionClientImpl(RNSClientHome home) throws Exception {
		this.home = home;
		this.rnsClient = home.getRNSClient();
		enableACLCache(home.getCacheTimeout());
		EndpointReferenceType epr = rnsClient.getEPR("/", true);
		rootPort = rnsEprToRnsExtensionPortType(epr, null);
	}

	private RNSExtensionPortType rnsEprToRnsExtensionPortType(
			EndpointReferenceType rnsEpr, String path) throws RNSError {
		/* replace path of URL */
		/* deep copy */
		EndpointReferenceType epr = new EndpointReferenceType(rnsEpr, true);
		/* new AttributedURIType (don't modify rnsEpr) */
		AttributedURIType address = new AttributedURIType();
		URI uri = new URI(rnsEpr.getAddress().getValue());
		try {
			uri.setPath(RNSConfig.getExtensionServicePath());
		} catch (MalformedURIException e) {
			throw RNSError.createEUNEXPECTED(null, null, e);
		}
		address.setValue(uri);
		epr.setAddress(address);

		try {
			return home.getRNSExtensionPortType(epr, path);
		} catch (Exception e) {
			throw RNSError.createEINVAL(path, e.getMessage(), e);
		}
	}

	public void noop() throws RNSError {
		NoopRequestType req = new NoopRequestType();
		try {
			rootPort.noop(req);
		} catch (RemoteException e) {
			throw RNSError.createENET(null, null, e);
		}
	}

	public String getServerVersion() throws RNSError {
		GetResourcePropertyResponse grpr;
		try {
			grpr = rootPort.getResourceProperty(RNSQNames.RP_VERSION);
		} catch (InvalidResourcePropertyQNameFaultType e) {
			throw RNSError.createEINVAL(null, null, e);
		} catch (ResourceUnknownFaultType e) {
			throw RNSError.createEINVAL(null, null, e);
		} catch (ResourceUnavailableFaultType e) {
			throw RNSError.createEBUSY(null, e.getMessage());
		} catch (RemoteException e) {
			throw RNSError.createENET(null, null, e);
		}
		return grpr.get_any()[0].getValue();
	}

	public void startProfile() throws RNSError {
		StartProfileRequestType req = new StartProfileRequestType();
		try {
			rootPort.startProfile(req);
		} catch (ReadNotPermittedFaultType e) {
			throw RNSError.createEPERM(null, "StartProfile");
		} catch (RemoteException e) {
			throw RNSError.createENET(null, null, e);
		}
	}

	public ProfileType[] stopProfile() throws RNSError {
		StopProfileRequestType req = new StopProfileRequestType();
		try {
			StopProfileResponseType res = rootPort.stopProfile(req);
			return res.getResults();
		} catch (ReadNotPermittedFaultType e) {
			throw RNSError.createEPERM(null, "StopProfile");
		} catch (RemoteException e) {
			throw RNSError.createENET(null, null, e);
		}
	}

	public Map<String, String> getServerStatus() throws RNSError {
		GetServerStatusRequestType req = new GetServerStatusRequestType();
		GetServerStatusResponseType res;
		try {
			res = rootPort.getServerStatus(req);
		} catch (ReadNotPermittedFaultType e) {
			throw RNSError.createEPERM(null, "GetServerStatus");
		} catch (RemoteException e) {
			throw RNSError.createENET(null, null, e);
		}
		Map<String, String> map = new HashMap<String, String>();
		StringMapType[] smts = res.getMap();
		if (smts != null) {
			for (StringMapType smt : smts) {
				map.put(smt.getName(), smt.getValue());
			}
		}
		return map;
	}

	/* --- ACL operations ----------------------------------------------- */
	// TODO this cache must be removed at remove and rename
	private static Map<String, Object> aclCache = null;

	private static void enableACLCache(long timeout) {
		if (timeout <= 0) {
			aclCache = null;
		} else {
			aclCache = new TimeoutCacheMap<String, Object>(timeout);
		}
	}

	private static ACL getACLCache(String path) throws RNSError {
		if (aclCache == null || path == null) {
			return null;
		}
		Object o = aclCache.get(path);
		if (o == null) {
			return null;
		} else if (o instanceof ACL) {
			return (ACL) o;
		} else if (o instanceof RNSError) {
			throw (RNSError) o;
		} else {
			return null;
		}
	}

	private static void putACLCache(String path, Object o) {
		if (aclCache == null || path == null || o == null) {
			return;
		}
		aclCache.put(path, o);
	}

	private static void removeACLCache(String path) {
		if (aclCache == null || path == null) {
			return;
		}
		aclCache.remove(path);
	}

	private static final String errorMessage = "junction have no ACL";

	public ACL getACL(String path, boolean useCache) throws RNSError {
		ACL acl;
		if (useCache) {
			acl = getACLCache(path);
			if (acl != null) {
				return acl;
			}
		}
		boolean isDir = rnsClient.isDirectory(path);
		if (isDir == false) {
			RNSError e = RNSError.createENOTDIR(path, errorMessage);
			putACLCache(path, e);
			throw e;
		}
		EndpointReferenceType epr = rnsClient.getEPR(path, true);
		RNSExtensionPortType port = rnsEprToRnsExtensionPortType(epr, path);
		GetACLResponseType res;
		try {
			res = port.getACL(new GetACLRequestType());
		} catch (ACLFaultType e) {
			throw RNSError.createEACCES(path, e.getFaultString());
		} catch (RemoteException e) {
			throw RNSError.createENET(path, e.getMessage(), e);
		}

		try {
			ACLEntryType[] list = res.getEntries();
			acl = new ACL(list);
			acl.autoComplete();
			putACLCache(path, acl);
			return acl;
		} catch (Exception e) {
			putACLCache(path, e);
			throw RNSError.createEINVAL(path, e.getMessage(), e);
		}
	}

	public void setACL(String path, String aclSpecs) throws RNSError {
		String[] aclStrList = aclSpecs.split(",");
		setACL(path, aclStrList);
	}

	public void setACL(String path, String[] aclSpecs) throws RNSError {
		if (rnsClient.isDirectory(path) == false) {
			throw RNSError.createENOTDIR(path, errorMessage);
		}
		removeACLCache(path);
		EndpointReferenceType epr = rnsClient.getEPR(path, true);
		RNSExtensionPortType port = rnsEprToRnsExtensionPortType(epr, path);
		SetACLRequestType req = new SetACLRequestType();
		ACL acl;
		try {
			acl = new ACL(aclSpecs);
		} catch (Exception e) {
			throw RNSError.createEINVAL(path, e.getMessage(), e);
		}
		req.setEntries(acl.toACLEntries());
		try {
			port.setACL(req);
		} catch (ACLFaultType e) {
			throw RNSError.createEACCES(path, e.getFaultString());
		} catch (RemoteException e) {
			throw RNSError.createENET(path, e.getMessage(), e);
		}
	}

	public void removeACL(String path, String typeStr, String[] names)
			throws RNSError {
		removeACL(path, ACL.typeStringToShort(typeStr), names);
	}

	public void removeACL(String path, short type, String[] names)
			throws RNSError {
		if (rnsClient.isDirectory(path) == false) {
			throw RNSError.createENOTDIR(path, errorMessage);
		}
		removeACLCache(path);
		EndpointReferenceType epr = rnsClient.getEPR(path, true);
		RNSExtensionPortType port = rnsEprToRnsExtensionPortType(epr, path);
		RemoveACLRequestType req = new RemoveACLRequestType();
		req.setType(type);
		req.setNames(names);
		try {
			port.removeACL(req);
		} catch (ACLFaultType e) {
			throw RNSError.createEACCES(path, e.getFaultString());
		} catch (RemoteException e) {
			throw RNSError.createENET(path, e.getMessage(), e);
		}
	}

	private static class MyCallerInfo implements CallerInfo {
		private boolean isAdmin;
		private String name;
		private String group;
		private List<String> groups;

		MyCallerInfo(boolean isAdmin, String name, String group,
				List<String> groups) {
			this.isAdmin = isAdmin;
			this.name = name;
			this.group = group;
			this.groups = groups;
		}

		@Override
		public List<String> getGroupList() {
			return groups;
		}

		@Override
		public String getMainGroup() {
			return group;
		}

		@Override
		public String getUserName() {
			return name;
		}

		@Override
		public boolean isAdmin() {
			return isAdmin;
		}
	}

	public CallerInfo getCallerInfo() throws RNSError {
		GetCallerInfoRequestType req = new GetCallerInfoRequestType();
		GetCallerInfoResponseType res;
		try {
			res = rootPort.getCallerInfo(req);
		} catch (RemoteException e) {
			throw RNSError.createENET(null, null, e);
		}

		String[] groups = res.getGroups();
		ArrayList<String> al = null;
		if (groups != null) {
			al = new ArrayList<String>();
			for (String g : groups) {
				al.add(g);
			}
		}
		return new MyCallerInfo(res.isAdmin(), res.getCaller(),
				res.getMainGroup(), al);
	}
}
