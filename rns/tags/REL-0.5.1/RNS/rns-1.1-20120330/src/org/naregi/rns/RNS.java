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

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import org.apache.axis.types.URI;
import org.apache.axis.types.URI.MalformedURIException;
import org.apache.commons.logging.Log;
import org.globus.axis.message.addressing.AttributedURIType;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.wsrf.NoSuchResourceException;
import org.globus.wsrf.ResourceContext;
import org.globus.wsrf.ResourceContextException;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.ResourceKey;
import org.globus.wsrf.utils.AddressingUtils;
import org.naregi.rns.stubs.AddRequestType;
import org.naregi.rns.stubs.AddResponseType;
import org.naregi.rns.stubs.LookupRequestType;
import org.naregi.rns.stubs.LookupResponseType;
import org.naregi.rns.stubs.MetadataMappingType;
import org.naregi.rns.stubs.NameMappingType;
import org.naregi.rns.stubs.RNSEntryDoesNotExistFaultType;
import org.naregi.rns.stubs.RNSEntryExistsFaultType;
import org.naregi.rns.stubs.RNSEntryResponseType;
import org.naregi.rns.stubs.RNSEntryType;
import org.naregi.rns.stubs.RNSMetadataType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.RemoveRequestType;
import org.naregi.rns.stubs.RemoveResponseType;
import org.naregi.rns.stubs.RenameRequestType;
import org.naregi.rns.stubs.RenameResponseType;
import org.naregi.rns.stubs.SetMetadataRequestType;
import org.naregi.rns.stubs.SetMetadataResponseType;
import org.naregi.rns.stubs.WriteNotPermittedFaultType;
import org.naregi.rns.util.RNSUtil;

/**
 * A service implementation for main RNS operations.
 */
public class RNS {

	public RNS() throws RemoteException {
		initialize();
	}

	private static Log logger;

	private static ResourceContext rootCtx;
	private static RNSResourceHome rootHome;
	private static EndpointReferenceType rootEPR;

	private static String localAddress;

	private static void initialize() throws RemoteException {
		logger = RNSLog.getLog();
		logger.debug("Globus Toolkit version:"
				+ org.globus.wsrf.utils.Version.getVersion());
		logger.info("RNS version:" + RNSVersion.getVersion());
		try {
			rootCtx = ResourceContext.getResourceContext();
			rootHome = (RNSResourceHome) rootCtx.getResourceHome();
			ResourceKey key = rootHome.getRootDirKey();
			rootEPR = AddressingUtils.createEndpointReference(rootCtx, key);

			replaceLocalHostNamePort(rootEPR);

			/* force replacing ServicePath */
			AttributedURIType address = rootEPR.getAddress();
			URI uri = address.getValue();
			try {
				String svcname = RNSConfig.getRNSServicePath();
				uri.setPath(svcname);
			} catch (MalformedURIException e) {
				throw new RemoteException("unexpected", e);
			}
			address.setValue(uri);
			rootEPR.setAddress(address);

			String eprStr = RNSUtil.toXMLString(rootEPR);
			logger.info("RootEPR: " + eprStr);
			localAddress = rootEPR.getAddress().getValue().toString();
		} catch (Exception e) {
			logger.fatal("cannot create root EPR: " + e.getMessage());
			throw new RemoteException("cannot create root EPR", e);
		}
	}

	private RNSResource getResource() throws ResourceContextException,
			ResourceException, RNSEntryDoesNotExistFaultType {
		try {
			/* ResourceContextImpl BaseResourceContext */
			ResourceContext ctx = ResourceContext.getResourceContext();
			RNSResource res = (RNSResource) ctx.getResource();
			if (res != null) {
				return res;
			}
		} catch (NoSuchResourceException e) {
		}
		throw new RNSEntryDoesNotExistFaultType();
	}

	private static String getResolvedHostName(EndpointReferenceType epr) {
		AttributedURIType address = epr.getAddress();
		URI uri = address.getValue();
		try {
			return InetAddress.getByName(uri.getHost()).getCanonicalHostName();
		} catch (UnknownHostException e) {
			e.printStackTrace();
			return null;
		}
	}

	private static void replaceHostName(EndpointReferenceType epr,
			String hostName, String port) {
		AttributedURIType address = epr.getAddress();
		URI uri = address.getValue();
		try {
			if (hostName != null) {
				uri.setHost(hostName);
			}
			if (port != null) {
				uri.setPort(Integer.parseInt(port));
			}
		} catch (MalformedURIException e) {
			e.printStackTrace();
			return;
		}
		address.setValue(uri);
		epr.setAddress(address);
	}

	private static String localHostName = null;
	private static String localHostPort = null;

	public static void replaceLocalHostNamePort(EndpointReferenceType epr) {
		if (localHostName == null) {
			localHostName = RNSConfig.getReplaceLocalHostName();
			if (localHostName == null) {
				localHostName = getResolvedHostName(epr);
			}
			localHostPort = RNSConfig.getReplaceLocalPort(); /* null OK */
		}
		if (localHostName != null) {
			replaceHostName(epr, localHostName, localHostPort);
		}
	}

	private static synchronized EndpointReferenceType getEPRFromLocalID(
			String id) {
		try {
			ResourceKey key = rootHome.idToResourceKey(id);
			EndpointReferenceType epr = AddressingUtils.createEndpointReference(
					rootCtx, key);
			replaceLocalHostNamePort(epr);
			return epr;
		} catch (Exception e) {
			e.printStackTrace();
			RNSLog.getLog().error(
					"cannot generate EPR from localID: " + e.getMessage());
			return null;
		}
	}

	public static RNSEntryData convertRNSEntryData(RNSEntryData ed) {
		if (ed == null) {
			return null;
		}
		if (ed.getLocalID() != null) {
			/* replace current Service URL */
			ed.setEpr(getEPRFromLocalID(ed.getLocalID()));
			if (ed.getEpr() == null) {
				return null;
			}
		}
		return ed;
	}

	public static RNSEntryResponseType setupResponseEntryType(String name,
			RNSEntryData ed) {
		ed = convertRNSEntryData(ed);
		if (ed == null) {
			return null;
		}

		RNSEntryResponseType entry = new RNSEntryResponseType();
		entry.setEntryName(name);
		entry.setEndpoint(ed.getEpr());
		entry.setMetadata(ed.getRNSMetadata());
		return entry;
	}

	private void logSuccess(String opname) {
		logger.debug(opname + ":SUCCESS");
	}

	private void logFailure(String opname, Throwable e) {
		logger.debug(opname + ":FAILURE:" + e.getClass().getSimpleName() + ":"
				+ e.getMessage());
	}

	private void logError(String opname, Throwable e) {
		logger.error(opname + ":ERROR:" + e.getClass().getSimpleName() + ":"
				+ e.getMessage());
	}

	private static final String OP_ADD = "ADD";
	private static final String OP_LOOKUP = "LOOKUP";
	private static final String OP_REMOVE = "REMOVE";
	private static final String OP_RENAME = "RENAME";
	private static final String OP_SETMETADATA = "SETMETADATA";

	/* PortType operations */
	public AddResponseType add(AddRequestType req)
			throws WriteNotPermittedFaultType, RemoteException, Throwable {
		try {
			AddResponseType res = add_main(req);
			logSuccess(OP_ADD);
			return res;
		} catch (WriteNotPermittedFaultType e) {
			logFailure(OP_ADD, e);
			throw e;
		} catch (RNSEntryDoesNotExistFaultType e) {
			logFailure(OP_ADD, e);
			RNSEntryResponseType eres = new RNSEntryResponseType();
			eres.setFault(e);
			RNSEntryResponseType[] eress = new RNSEntryResponseType[1];
			eress[0] = eres;
			AddResponseType res = new AddResponseType();
			res.setEntryResponse(eress);
			return res;
		} catch (RemoteException e) {
			e.printStackTrace();
			logError(OP_ADD, e);
			throw e;
		} catch (Throwable e) {
			e.printStackTrace();
			logError(OP_ADD, e);
			throw e;
		}
	}

	public LookupResponseType lookup(LookupRequestType req)
			throws ReadNotPermittedFaultType, RemoteException, Throwable {
		try {
			LookupResponseType res = lookup_main(req);
			logSuccess(OP_LOOKUP);
			return res;
		} catch (ReadNotPermittedFaultType e) {
			logFailure(OP_LOOKUP, e);
			throw e;
		} catch (RNSEntryDoesNotExistFaultType e) {
			logFailure(OP_LOOKUP, e);
			RNSEntryResponseType eres = new RNSEntryResponseType();
			eres.setFault(e);
			RNSEntryResponseType[] eress = new RNSEntryResponseType[1];
			eress[0] = eres;
			LookupResponseType res = new LookupResponseType();
			res.setEntryResponse(eress);
			return res;
		} catch (RemoteException e) {
			e.printStackTrace();
			logError(OP_LOOKUP, e);
			throw e;
		} catch (Throwable e) {
			e.printStackTrace();
			logError(OP_LOOKUP, e);
			throw e;
		}
	}

	public RemoveResponseType remove(RemoveRequestType req)
			throws WriteNotPermittedFaultType, RemoteException, Throwable {
		try {
			RemoveResponseType res = remove_main(req);
			logSuccess(OP_REMOVE);
			return res;
		} catch (WriteNotPermittedFaultType e) {
			logFailure(OP_REMOVE, e);
			throw e;
		} catch (RNSEntryDoesNotExistFaultType e) {
			logFailure(OP_REMOVE, e);
			RNSEntryResponseType eres = new RNSEntryResponseType();
			eres.setFault(e);
			RNSEntryResponseType[] eress = new RNSEntryResponseType[1];
			eress[0] = eres;
			RemoveResponseType res = new RemoveResponseType();
			res.setEntryResponse(eress);
			return res;
		} catch (RemoteException e) {
			e.printStackTrace();
			logError(OP_REMOVE, e);
			throw e;
		} catch (Throwable e) {
			e.printStackTrace();
			logError(OP_REMOVE, e);
			throw e;
		}
	}

	public RenameResponseType rename(RenameRequestType req)
			throws WriteNotPermittedFaultType, RNSEntryDoesNotExistFaultType,
			RNSEntryExistsFaultType, RemoteException, Throwable {
		try {
			RenameResponseType res = rename_main(req);
			logSuccess(OP_RENAME);
			return res;
		} catch (WriteNotPermittedFaultType e) {
			logFailure(OP_RENAME, e);
			throw e;
		} catch (RNSEntryDoesNotExistFaultType e) {
			logFailure(OP_RENAME, e);
			RNSEntryResponseType eres = new RNSEntryResponseType();
			eres.setFault(e);
			RNSEntryResponseType[] eress = new RNSEntryResponseType[1];
			eress[0] = eres;
			RenameResponseType res = new RenameResponseType();
			res.setEntryResponse(eress);
			return res;
		} catch (RNSEntryExistsFaultType e) {
			logFailure(OP_RENAME, e);
			throw e;
		} catch (RemoteException e) {
			e.printStackTrace();
			logError(OP_RENAME, e);
			throw e;
		} catch (Throwable e) {
			e.printStackTrace();
			logError(OP_RENAME, e);
			throw e;
		}
	}

	public SetMetadataResponseType setMetadata(SetMetadataRequestType req)
			throws WriteNotPermittedFaultType, RNSEntryDoesNotExistFaultType,
			RemoteException, Throwable {
		try {
			SetMetadataResponseType res = setMetadata_main(req);
			logSuccess(OP_SETMETADATA);
			return res;
		} catch (WriteNotPermittedFaultType e) {
			logFailure(OP_SETMETADATA, e);
			throw e;
		} catch (RNSEntryDoesNotExistFaultType e) {
			logFailure(OP_SETMETADATA, e);
			RNSEntryResponseType eres = new RNSEntryResponseType();
			eres.setFault(e);
			RNSEntryResponseType[] eress = new RNSEntryResponseType[1];
			eress[0] = eres;
			SetMetadataResponseType res = new SetMetadataResponseType();
			res.setEntryResponse(eress);
			return res;
		} catch (RemoteException e) {
			e.printStackTrace();
			logError(OP_SETMETADATA, e);
			throw e;
		} catch (Throwable e) {
			e.printStackTrace();
			logError(OP_SETMETADATA, e);
			throw e;
		}
	}

	private RNSEntryResponseType generateRNSEntryDoesNotExistFaultType(
			String name) {
		RNSEntryResponseType ent = new RNSEntryResponseType();
		ent.setEntryName(name);
		ent.setEndpoint(null);
		ent.setMetadata(null);
		RNSEntryDoesNotExistFaultType noent = new RNSEntryDoesNotExistFaultType();
		noent.setEntryName(name);
		noent.setTimestamp(Calendar.getInstance()); // need timestamp
		ent.setFault(noent);
		return ent;
	}

	private RNSEntryResponseType generateRNSEntryExistsFaultType(String name) {
		RNSEntryResponseType ent = new RNSEntryResponseType();
		ent.setEntryName(name);
		ent.setEndpoint(null);
		ent.setMetadata(null);
		RNSEntryExistsFaultType exists = new RNSEntryExistsFaultType();
		exists.setEntryName(name);
		exists.setTimestamp(Calendar.getInstance()); // need timestamp
		ent.setFault(exists);
		return ent;
	}

	private AddResponseType add_main(AddRequestType req)
			throws WriteNotPermittedFaultType, RemoteException {
		long pfTotal = RNSProfiler.start();
		RNSProfiler.TYPE type;

		RNSResource r;
		try {
			r = getResource();
		} catch (RNSEntryDoesNotExistFaultType e) {
			RNSEntryResponseType[] ents = new RNSEntryResponseType[1];
			ents[0] = generateRNSEntryDoesNotExistFaultType("(parent directory)");
			AddResponseType res = new AddResponseType();
			res.setEntryResponse(ents);
			return res;
		}
		CallerInfo callerInfo = AccessControlSwitch.getCallerInfomation();
		if (AccessControlSwitch.canWrite(r, callerInfo) == false) {
			throw new WriteNotPermittedFaultType();
		}
		RNSEntryType[] ereqs = req.getEntry();
		if (ereqs == null) {
			throw new RemoteException("no request");
		}
		ArrayList<ResourceKey> fordel = new ArrayList<ResourceKey>();
		ArrayList<RNSEntryResponseType> resl = new ArrayList<RNSEntryResponseType>();
		Object lock = r.getLockAndStartTransaction();
		synchronized (lock) {
			try {
				for (RNSEntryType ereq : ereqs) {
					String name = ereq.getEntryName();
					EndpointReferenceType epr = ereq.getEndpoint();
					RNSMetadataType meta = ereq.getMetadata();
					RNSEntryData ent = r.getRNSEntryData(name);
					RNSEntryResponseType eres;
					if (ent != null) {
						eres = generateRNSEntryExistsFaultType(name);
					} else { /* not error */
						ent = new RNSEntryData();
						if (epr == null) {
							/* create new directory into local */
							ResourceKey tmpkey;
							try {
								tmpkey = rootHome.createNewDir();
							} catch (Exception e) {
								throw new RemoteException(
										"createNewDir failed", e);
							}
							/* for rollback (destroy Resource) */
							fordel.add(tmpkey);

							ent.setLocalID((String) tmpkey.getValue());
							ent.setRNSMetadata(meta);
							ent.setDirectory(true);

							long pf = RNSProfiler.start();
							if (callerInfo != null
									&& AccessControlSwitch.getType().equals(
											AccessControlSwitch.TYPE_VOMS)) {
								/* accessControlType=voms */
								RNSResource childr = (RNSResource) rootHome.find(tmpkey);
								try {
									ACL acl = r.getACL();
									acl.setOwner(callerInfo.getUserName(),
											ACL.PERM_ALL);
									acl.setOwnerGroup(
											callerInfo.getMainGroup(),
											ACL.PERM_ALL);
									acl.setOther(ACL.PERM_ZERO);
									acl.clearUserPerm();
									acl.clearGroupPerm();
									acl.clearMask();
									acl.copyAllDefaultPermToNormal();
									acl.autoComplete();
									childr.setACL(acl);
									childr.commit();
								} catch (ResourceException e) {
									childr.rollback();
									e.printStackTrace();
									throw e;
								}
							}
							RNSProfiler.stop(RNSProfiler.TYPE.Add_SetACL_DB, pf);
						} else {
							/* adding normal junction */
							if (meta != null) {
								ent.setRNSMetadata(meta);
							}
							String id = RNSUtil.getRNSResourceId(epr);
							if (id != null && ent.isDirectory()) {
								/* This junction refers to my RNS implementation */
								String url = epr.getAddress().toString();
								if (localAddress.equals(url)) {
									/* refer to local directory */
									ent.setLocalID(id);
								} else {
									ent.setEpr(epr);
								}
							} else {
								ent.setEpr(epr);
								ent.setDirectory(false);
							}
						}
						/* localID to epr for response */
						long pf = RNSProfiler.start();
						r.insertRNSEntryData(name, ent);
						RNSProfiler.stop(RNSProfiler.TYPE.Add_InsertDB, pf);

						ent = convertRNSEntryData(ent);
						eres = new RNSEntryResponseType();
						eres.setEntryName(name);
						eres.setEndpoint(ent.getEpr());
						eres.setMetadata(meta);
					}
					resl.add(eres);
				}
				long pf = RNSProfiler.start();
				r.setModificationTime(Calendar.getInstance());
				RNSProfiler.stop(RNSProfiler.TYPE.Add_ModifyMtimeDB, pf);
				if (resl.size() == 1) {
					pf = RNSProfiler.start();
					r.commit();
					RNSProfiler.stop(RNSProfiler.TYPE.Add_CommitDB, pf);
					type = RNSProfiler.TYPE.Total_Add;
				} else {
					r.commit();
					type = RNSProfiler.TYPE.Total_Bulk_Add;
				}
			} catch (ResourceException e) {
				r.rollback();
				for (ResourceKey k : fordel) {
					RNSResource delr = (RNSResource) rootHome.find(k);
					delr.remove();
				}
				throw e;
			}
		}
		AddResponseType res = new AddResponseType();
		res.setEntryResponse(resl.toArray(new RNSEntryResponseType[0]));
		RNSProfiler.stop(type, pfTotal);
		return res;
	}

	private boolean availableIterator(long len) {
		int i = RNSConfig.getIteratorUnit();
		logger.debug("iteratorUnit=" + i);
		if (i > 0 && len > i) {
			return true;
		}
		return false;
	}

	private LookupResponseType list(RNSResource r)
			throws ReadNotPermittedFaultType, RemoteException {
		LookupResponseType res = new LookupResponseType();

		List<String> list = r.getList();
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

		int len = list.size();
		if (len <= 0) {
			return res;
		}

		int returnSize;
		if (availableIterator(len)) {
			/* use WS-Iterator */
			EndpointReferenceType iteratorEpr;
			try {
				iteratorEpr = AddressingUtils.createEndpointReference(rootCtx,
						rootHome.idToResourceKey((String) r.getID()));
			} catch (Exception e) {
				throw new RemoteException("create iteratorEPR", e);
			}
			AttributedURIType address = iteratorEpr.getAddress();
			URI uri = address.getValue();
			try {
				String itrname = RNSConfig.getListIteratorServicePath();
				logger.debug("IteratorServiceName: " + itrname);
				uri.setPath(itrname);
			} catch (MalformedURIException e) {
				throw new RemoteException("unexpected", e);
			}
			address.setValue(uri);
			iteratorEpr.setAddress(address);
			res.setIterator(iteratorEpr);
			replaceLocalHostNamePort(iteratorEpr);
			returnSize = RNSConfig.getIteratorUnit(); /* return arrays + WS-Iterator */
		} else {
			returnSize = len;
		}

		RNSEntryResponseType[] entries = new RNSEntryResponseType[returnSize];
		for (int i = 0; i < returnSize && i < len; i++) {
			String name = list.get(i);
			RNSEntryData ed = r.getRNSEntryData(name);
			entries[i] = setupResponseEntryType(name, ed);
		}
		res.setEntryResponse(entries);
		return res;
	}

	private LookupResponseType lookup_main(LookupRequestType req)
			throws ReadNotPermittedFaultType, RemoteException {
		long s = RNSProfiler.start();

		RNSResource r;
		try {
			r = getResource();
		} catch (RNSEntryDoesNotExistFaultType e) {
			RNSEntryResponseType[] ents = new RNSEntryResponseType[1];
			ents[0] = generateRNSEntryDoesNotExistFaultType("(parent directory)");
			LookupResponseType res = new LookupResponseType();
			res.setEntryResponse(ents);
			res.setIterator(null);
			return res;
		}
		if (r.isReadable() == false) {
			throw new ReadNotPermittedFaultType();
		}
		if (req == null) {
			LookupResponseType res = list(r);
			RNSProfiler.stop(RNSProfiler.TYPE.Total_List, s);
			return res;
		}
		String[] names = req.getEntryName();
		if (names == null || names.length == 0 || names[0] == null) {
			LookupResponseType res = list(r);
			RNSProfiler.stop(RNSProfiler.TYPE.Total_List, s);
			return res;
		}
		RNSEntryResponseType[] ents = new RNSEntryResponseType[names.length];
		for (int i = 0; i < ents.length; i++) {
			RNSEntryResponseType ent;
			RNSEntryData ed = r.getRNSEntryData(names[i]);
			if (ed == null) {
				ent = generateRNSEntryDoesNotExistFaultType(names[i]);
			} else {
				ent = setupResponseEntryType(names[i], ed);
			}
			ents[i] = ent;
		}
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
		LookupResponseType res = new LookupResponseType();
		res.setEntryResponse(ents);
		res.setIterator(null);
		if (ents.length == 1) {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Lookup, s);
		} else {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Bulk_Lookup, s);
		}
		return res;
	}

	private RemoveResponseType remove_main(RemoveRequestType req)
			throws WriteNotPermittedFaultType, RemoteException {
		long s = RNSProfiler.start();

		RNSResource r;
		try {
			r = getResource();
		} catch (RNSEntryDoesNotExistFaultType e) {
			RNSEntryResponseType[] ents = new RNSEntryResponseType[1];
			ents[0] = generateRNSEntryDoesNotExistFaultType("(parent directory)");
			RemoveResponseType res = new RemoveResponseType();
			res.setEntryResponse(ents);
			return res;
		}
		if (r.isWritable() == false) {
			throw new WriteNotPermittedFaultType();
		}
		String[] names = req.getEntryName();
		if (names == null || names.length == 0) {
			throw new RemoteException("no entry names");
		}
		RNSEntryResponseType[] ents = new RNSEntryResponseType[names.length];
		synchronized (r.getLockAndStartTransaction()) {
			try {
				for (int i = 0; i < names.length; i++) {
					RNSEntryResponseType ent = new RNSEntryResponseType();
					RNSEntryData ed = r.getRNSEntryData(names[i]);
					ed = convertRNSEntryData(ed);
					if (ed == null) {
						ent = generateRNSEntryDoesNotExistFaultType(names[i]);
					} else {
						r.removeRNSEntryData(names[i]);
						ent.setEntryName(names[i]);
						ent.setEndpoint(ed.getEpr());
						ent.setMetadata(ed.getRNSMetadata());
					}
					ents[i] = ent;
				}
				r.setModificationTime(Calendar.getInstance());
				r.commit();
			} catch (ResourceException e) {
				r.rollback();
				throw e;
			}
		}
		RemoveResponseType res = new RemoveResponseType();
		res.setEntryResponse(ents);
		if (names.length == 1) {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Remove, s);
		} else {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Bulk_Remove, s);
		}
		return res;
	}

	private RenameResponseType rename_main(RenameRequestType req)
			throws WriteNotPermittedFaultType, RNSEntryDoesNotExistFaultType,
			RNSEntryExistsFaultType, RemoteException {
		long s = RNSProfiler.start();

		RNSResource r;
		try {
			r = getResource();
		} catch (RNSEntryDoesNotExistFaultType e) {
			RNSEntryResponseType[] ents = new RNSEntryResponseType[1];
			ents[0] = generateRNSEntryDoesNotExistFaultType("(parent directory)");
			RenameResponseType res = new RenameResponseType();
			res.setEntryResponse(ents);
			return res;
		}
		if (r.isWritable() == false) {
			throw new WriteNotPermittedFaultType();
		}
		NameMappingType[] nmap = req.getRenameRequest();
		if (nmap == null || nmap.length == 0) {
			throw new RemoteException("unexpected error");
		}

		RNSEntryResponseType[] ents = new RNSEntryResponseType[nmap.length];
		synchronized (r.getLockAndStartTransaction()) {
			try {
				for (int i = 0; i < nmap.length; i++) {
					String from = nmap[i].getSourceName();
					String to = nmap[i].getTargetName();
					if (from == null || to == null) {
						r.rollback();
						throw new RemoteException("null entry name");
					}
					RNSEntryData fromExist = r.getRNSEntryData(from);
					if (fromExist == null) {
						ents[i] = generateRNSEntryDoesNotExistFaultType(from);
						continue;
					}
					RNSEntryData toExist = r.getRNSEntryData(to);
					if (toExist != null) {
						ents[i] = generateRNSEntryExistsFaultType(to);
						continue;
					}
					r.rename(from, to);
					RNSEntryData ed = r.getRNSEntryData(to);
					RNSEntryResponseType entry = setupResponseEntryType(to, ed);
					ents[i] = entry;

				}
				r.setModificationTime(Calendar.getInstance());
				r.commit();
			} catch (ResourceException e) {
				r.rollback();
				throw e;
			}
		}
		RenameResponseType res = new RenameResponseType();
		res.setEntryResponse(ents);
		if (nmap.length == 1) {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Rename, s);
		} else {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Bulk_Rename, s);
		}

		return res;
	}

	private SetMetadataResponseType setMetadata_main(SetMetadataRequestType req)
			throws WriteNotPermittedFaultType, RNSEntryDoesNotExistFaultType,
			RemoteException {
		long s = RNSProfiler.start();

		RNSResource r;
		try {
			r = getResource();
		} catch (RNSEntryDoesNotExistFaultType e) {
			RNSEntryResponseType[] ents = new RNSEntryResponseType[1];
			ents[0] = generateRNSEntryDoesNotExistFaultType("(parent directory)");
			SetMetadataResponseType res = new SetMetadataResponseType();
			res.setEntryResponse(ents);
			return res;
		}
		if (r.isWritable() == false) {
			throw new WriteNotPermittedFaultType();
		}
		MetadataMappingType[] metamap = req.getSetMetadataRequest();
		if (metamap == null || metamap.length == 0) {
			throw new RemoteException("unexpected error");
		}
		RNSEntryResponseType[] ents = new RNSEntryResponseType[metamap.length];
		synchronized (r.getLockAndStartTransaction()) {
			try {
				for (int i = 0; i < ents.length; i++) {
					String name = metamap[i].getEntryName();
					if (name == null) {
						throw new RemoteException("unexpected: no entry name");
					}
					RNSEntryData ed = r.getRNSEntryData(name);
					if (ed == null) {
						ents[i] = generateRNSEntryDoesNotExistFaultType(name);
					} else {
						r.replaceMetadata(name, metamap[i].get_any());
						ed = r.getRNSEntryData(name);
						ents[i] = setupResponseEntryType(name, ed);
					}
				}
				r.setModificationTime(Calendar.getInstance());
				r.commit();
			} catch (ResourceException e) {
				r.rollback();
				throw e;
			}
		}
		SetMetadataResponseType res = new SetMetadataResponseType();
		res.setEntryResponse(ents);
		if (ents.length == 1) {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_SetMetadata, s);
		} else {
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Bulk_SetMetadata, s);
		}
		return res;
	}
}
