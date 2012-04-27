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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import javax.xml.namespace.QName;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.apache.axis.types.UnsignedLong;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.axis.message.addressing.ReferenceParametersType;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.ObjectDeserializer;
import org.naregi.rns.RNSQNames;
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
import org.naregi.rns.stubs.RNSPortType;
import org.naregi.rns.stubs.RNSSupportType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.RemoveRequestType;
import org.naregi.rns.stubs.RemoveResponseType;
import org.naregi.rns.stubs.RenameRequestType;
import org.naregi.rns.stubs.RenameResponseType;
import org.naregi.rns.stubs.SetMetadataRequestType;
import org.naregi.rns.stubs.SetMetadataResponseType;
import org.naregi.rns.stubs.SupportsRNSType;
import org.naregi.rns.stubs.WriteNotPermittedFaultType;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.util.TimeoutCacheMap;
import org.oasis.wsrf.faults.BaseFaultType;
import org.oasis.wsrf.lifetime.Destroy;
import org.oasis.wsrf.lifetime.ResourceNotDestroyedFaultType;
import org.oasis.wsrf.properties.GetMultipleResourcePropertiesResponse;
import org.oasis.wsrf.properties.GetMultipleResourceProperties_Element;
import org.oasis.wsrf.properties.InvalidResourcePropertyQNameFaultType;
import org.oasis.wsrf.resource.ResourceUnavailableFaultType;
import org.oasis.wsrf.resource.ResourceUnknownFaultType;

/**
 * RNSClient implementation.
 */
public class RNSClientImpl implements RNSClient {
	private EndpointReferenceType rootEPR;

	private RNSClientHome clientHome = null;

	private DirRef rootDirRef;
	private DirRef currentDirRef;
	private Map<String, RNSPortType> portTypeMap;

	RNSClientImpl(RNSClientHome home) throws Exception {
		EndpointReferenceType epr = home.getEPR();
		ReferenceParametersType param = epr.getParameters();
		if (param == null) {
			String uri = epr.getAddress().toString();

			epr = RNSUtil.toRNSEPR(new URI(uri));
		}

		RNSPortType port = home.getRNSPortType(epr, "/ (root)");
		rootDirRef = new DirRef("/", port);
		currentDirRef = rootDirRef;
		rootEPR = epr;
		clientHome = home;
		portTypeMap = new TimeoutCacheMap<String, RNSPortType>(
				clientHome.getCacheTimeout());
	}

	private int maxRecursive = 10;

	public int getDefaultMaxRecursiveDepth() {
		return maxRecursive;
	}

	public void setDefaultMaxRecursiveDepth(int depth) {
		maxRecursive = depth;
	}

	private boolean countTime = false; /* for debug */

	private long countTimeStart() {
		if (!countTime) {
			return 0;
		}
		return System.currentTimeMillis();
	}

	private void countTimeEnd(String name, long start) {
		if (!countTime) {
			return;
		}
		//debug(name, null, "(" + (System.currentTimeMillis() - start) + "ms)");
		System.out.println(name + " (" + (System.currentTimeMillis() - start)
				+ "ms)");
	}

	// TODO re-factoring: use bulk operations

	private LookupResponseType clientLookup(RNSPortType dirPort,
			String fullPath, String basename) throws RNSError {
		LookupRequestType lookupReq = new LookupRequestType();
		String[] names = new String[1];
		names[0] = basename;
		lookupReq.setEntryName(names);
		long s = countTimeStart();
		try {
			LookupResponseType lookupRes = dirPort.lookup(lookupReq);
			RNSEntryResponseType[] ents = lookupRes.getEntryResponse();
			if (ents == null) {
				throw RNSError.createEUNEXPECTED(fullPath,
						"lookup Response is null", null);
			}
			if (ents.length != 1) {
				throw RNSError.createEUNEXPECTED(fullPath,
						"lookup length != 1", null);
			}
			RNSError error = RNSUtil.convertBaseFault(ents[0].getFault(),
					fullPath, fullPath);
			if (error != null) {
				throw error;
			}
			return lookupRes;
		} catch (ReadNotPermittedFaultType e) {
			throw RNSError.createEACCES(fullPath, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(fullPath, e.getMessage(), e);
		} finally {
			countTimeEnd("lookup", s);
		}
	}

	private LookupResponseType clientList(RNSPortType dirPort, String baseDir,
			String[] names) throws RNSError {
		LookupRequestType lookupReq = new LookupRequestType();
		lookupReq.setEntryName(names);
		long s = countTimeStart();
		try {
			LookupResponseType lookupRes = dirPort.lookup(lookupReq);
			if (lookupRes == null) {
				return null;
			}
			if (lookupRes.getIterator() != null) {
				return lookupRes;
			}
			RNSEntryResponseType[] ents = lookupRes.getEntryResponse();
			if (ents == null || ents.length == 0) {
				return null;
			}
			RNSError error = RNSUtil.convertBaseFault(ents[0].getFault(),
					baseDir, baseDir);
			if (error != null) {
				throw error;
			}
			return lookupRes;
		} catch (ReadNotPermittedFaultType e) {
			throw RNSError.createEACCES(baseDir, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(baseDir, e.getMessage(), e);
		} finally {
			countTimeEnd("list", s);
		}
	}

	private AddResponseType clientAdd(RNSPortType dirPort, String dirPath,
			String name, boolean isDirectory, EndpointReferenceType epr,
			MessageElement[] xmls) throws RNSError {
		AddRequestType addReq = new AddRequestType();
		RNSEntryType[] entryRequests = new RNSEntryType[1];
		entryRequests[0] = new RNSEntryType();
		entryRequests[0].setEntryName(name);
		entryRequests[0].setEndpoint(epr);
		RNSMetadataType meta = new RNSMetadataType();
		SupportsRNSType st = new SupportsRNSType();
		if (isDirectory) {
			st.setValue(RNSSupportType.value1);
		} else {
			st.setValue(RNSSupportType.value2);
		}
		meta.setSupportsRns(st);

		String path = RNSUtil.joinPath(dirPath, name);

		/* nillable="true" */
		meta.set_any(xmls);
		entryRequests[0].setMetadata(meta);
		addReq.setEntry(entryRequests);
		long s = countTimeStart();
		try {
			AddResponseType addRes = dirPort.add(addReq);
			if (addRes == null) {
				throw RNSError.createEUNEXPECTED(path, "AddResponse is null",
						null);
			}
			RNSEntryResponseType[] ents = addRes.getEntryResponse();
			if (ents == null || ents.length == 0) {
				throw RNSError.createEUNEXPECTED(path,
						"AddResponse is invalid style", null);
			}
			RNSError error = RNSUtil.convertBaseFault(ents[0].getFault(),
					dirPath, path);
			if (error != null) {
				throw error;
			}
			return addRes;
		} catch (WriteNotPermittedFaultType e) {
			throw RNSError.createEACCES(dirPath, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(path, e.getMessage(), e);
		} finally {
			countTimeEnd("add", s);
		}
	}

	private RemoveResponseType clientRemove(RNSPortType dirPort,
			String dirPath, String name) throws RNSError {
		RemoveRequestType removeReq = new RemoveRequestType();
		String[] entryNames = new String[1];
		entryNames[0] = name;
		removeReq.setEntryName(entryNames);
		String path = RNSUtil.joinPath(dirPath, name);
		long s = countTimeStart();
		try {
			RemoveResponseType removeRes = dirPort.remove(removeReq);
			portTypeMap.remove(path);
			if (removeRes == null) {
				throw RNSError.createEUNEXPECTED(path,
						"RemoveResponse is null", null);
			}
			RNSEntryResponseType[] ents = removeRes.getEntryResponse();
			if (ents == null || ents.length == 0) {
				throw RNSError.createEUNEXPECTED(path,
						"RemoveResponse is invalid style", null);
			}
			RNSError error = RNSUtil.convertBaseFault(ents[0].getFault(), path,
					path);
			if (error != null) {
				throw error;
			}
			return removeRes;
		} catch (WriteNotPermittedFaultType e) {
			throw RNSError.createEACCES(dirPath, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(path, e.getMessage(), e);
		} finally {
			countTimeEnd("remove", s);
		}
	}

	private RenameResponseType clientRename(RNSPortType dirPort,
			String dirPath, String from, String to) throws RNSError {
		RenameRequestType req = new RenameRequestType();
		NameMappingType nm = new NameMappingType();

		NameMappingType[] nms = new NameMappingType[1];
		nm.setSourceName(from);
		nm.setTargetName(to);
		nms[0] = nm;

		String fromPath = RNSUtil.joinPath(dirPath, from);
		String toPath = RNSUtil.joinPath(dirPath, to);

		req.setRenameRequest(nms);
		long s = countTimeStart();
		try {
			RenameResponseType renameRes = dirPort.rename(req);
			portTypeMap.remove(fromPath);
			RNSEntryResponseType[] ents = renameRes.getEntryResponse();
			if (ents == null || ents.length == 0) {
				return null;
			}
			RNSError error = RNSUtil.convertBaseFault(ents[0].getFault(),
					fromPath, toPath);
			if (error != null) {
				throw error;
			}
			return renameRes;
		} catch (WriteNotPermittedFaultType e) {
			throw RNSError.createEACCES(dirPath, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(fromPath, e.getMessage(), e);
		} finally {
			countTimeEnd("rename", s);
		}
	}

	private SetMetadataResponseType clientSetMetadata(RNSPortType dirPort,
			String dirPath, String name, MessageElement[] xmls) throws RNSError {
		DirRef dref = new DirRef(dirPath, dirPort);
		RNSEntryResponseType ent = getEntry(dref, name, true);
		RNSMetadataType meta = ent.getMetadata();
		SupportsRNSType supportsRns = meta.getSupportsRns();

		SetMetadataRequestType req = new SetMetadataRequestType();
		MetadataMappingType mm = new MetadataMappingType();
		mm.setEntryName(name);
		mm.setSupportsRns(supportsRns); /* MUST */
		mm.set_any(xmls);

		String path = RNSUtil.joinPath(dirPath, name);

		MetadataMappingType[] mms = new MetadataMappingType[1];
		mms[0] = mm;
		req.setSetMetadataRequest(mms);
		long s = countTimeStart();
		try {
			SetMetadataResponseType setMetaRes = dref.port.setMetadata(req);
			RNSEntryResponseType[] ents = setMetaRes.getEntryResponse();
			if (ents == null || ents.length == 0) {
				return null;
			}
			RNSError error = RNSUtil.convertBaseFault(ents[0].getFault(), path,
					path);
			if (error != null) {
				throw error;
			}
			return setMetaRes;
		} catch (WriteNotPermittedFaultType e) {
			throw RNSError.createEACCES(dirPath, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(path, "setMetadata failed", e);
		} finally {
			countTimeEnd("setMetadata", s);
		}
	}

	private static class DirRef {
		private String path;
		private RNSPortType port;

		DirRef(String path, RNSPortType port) {
			this.path = path;
			this.port = port;
		}
	}

	private DirRef getDirRef(String path) throws RNSError {
		if (path == null || path.length() == 0) {
			path = RNSUtil.joinPath(currentDirRef.path, path); /* normalized */
		} else {
			path = RNSUtil.normalizePath(path);
		}
		if (path.equals("/")) {
			return rootDirRef;
		}
		RNSPortType port = portTypeMap.get(path);
		if (port != null) {
			return new DirRef(path, port);
		}
		DirnameBasename dbn = new DirnameBasename(path);
		DirRef parentDref = getDirRef(dbn.getDirname()); /* recursive call */
		return getDirRefBase(path, parentDref, dbn.getBasename());
	}

	private DirRef getDirRef(DirRef baseDref, String basename) throws RNSError {
		String fullPath = RNSUtil.joinPath(baseDref.path, basename); /* normalized */
		RNSPortType port = portTypeMap.get(fullPath);
		if (port != null) {
			debug("getDirRef", fullPath, "(use cache)");
			return new DirRef(fullPath, port);
		}
		return getDirRefBase(fullPath, baseDref, basename);
	}

	private DirRef getDirRefBase(String fullPath, DirRef baseDref,
			String basename) throws RNSError {
		if (basename == null) {
			return baseDref;
		}
		LookupResponseType lookupRes = clientLookup(baseDref.port, fullPath,
				basename);
		RNSEntryResponseType[] ents = lookupRes.getEntryResponse();
		if (RNSUtil.isDirectory(ents[0].getMetadata()) == false) {
			throw RNSError.createENOTDIR(fullPath, null);
		}
		/* RNS directory */
		EndpointReferenceType childEpr = ents[0].getEndpoint();
		if (childEpr == null) {
			throw RNSError.createEUNEXPECTED(fullPath, "lookup EPR is null",
					null);
		}

		RNSPortType port;
		try {
			port = clientHome.getRNSPortType(childEpr, fullPath);
		} catch (Exception e) {
			throw RNSError.createEINVAL(fullPath, e.getMessage(), e);
		}

		debug("getDirRef", fullPath, "(enter cache)");
		portTypeMap.put(fullPath, port);
		return new DirRef(fullPath, port);
	}

	public RNSError[] addBulk(String baseDir, RNSAddHandle handle)
			throws RNSError {
		debug("addBulk", baseDir, null);
		List<RNSEntryType> list = handle.getList();
		if (list.size() == 0) {
			return null;
		}
		DirRef dref = getDirRef(baseDir);
		AddRequestType req = new AddRequestType();
		RNSEntryType[] ereqs = list.toArray(new RNSEntryType[0]);
		req.setEntry(ereqs);
		AddResponseType res;
		try {
			res = dref.port.add(req);
		} catch (WriteNotPermittedFaultType e) {
			throw RNSError.createEACCES(baseDir, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(baseDir, e.getMessage(), e);
		}
		if (res == null) {
			throw RNSError.createEUNEXPECTED(baseDir, "AddResponse is null",
					null);
		}
		RNSEntryResponseType[] ers = res.getEntryResponse();
		if (ers == null) {
			throw RNSError.createEUNEXPECTED(baseDir,
					"AddResponse is invalid style", null);
		}
		ArrayList<RNSError> al = new ArrayList<RNSError>();
		for (RNSEntryResponseType er : ers) {
			BaseFaultType bf = er.getFault();
			if (bf != null) {
				String childPath = RNSUtil.normalizePath(baseDir + "/"
						+ er.getEntryName());
				if (bf instanceof RNSEntryExistsFaultType) {
					al.add(RNSError.createEEXIST(childPath, null));
				} else if (bf instanceof RNSEntryDoesNotExistFaultType) {
					al.add(RNSError.createENOENT(baseDir, null));
				} else {
					al.add(RNSError.createENET(childPath, bf.getMessage(), bf));
				}
			}
		}
		if (al.size() > 0) {
			return al.toArray(new RNSError[0]);
		} else {
			return null;
		}
	}

	public RNSError[] removeBulk(String baseDir, String[] names)
			throws RNSError {
		debug("removeBulk", baseDir, null);
		DirRef dref = getDirRef(baseDir);
		RNSStat st = stat(baseDir);
		if (st.isWritable() == false) {
			throw RNSError.createEACCES(baseDir, null);
		}
		ArrayList<String> oklist = new ArrayList<String>(names.length);
		ArrayList<RNSError> nglist = new ArrayList<RNSError>();
		for (String name : names) {
			if (name == null || name.length() == 0) {
				continue;
			}
			String path = RNSUtil.joinPath(baseDir, name);
			try {
				DirRef childRef = getDirRef(dref, name);
				/* check existence */
				boolean doDestroy = true;
				st = null;
				try {
					st = stat(childRef);
				} catch (RNSError e) {
					if (e.getError() == RNSError.Errno.ENOENT) {
						doDestroy = false;
					} else {
						throw e;
					}
				}
				/* check empty */
				if (st != null && st.getElementCount().longValue() > 0) {
					nglist.add(RNSError.createENOTEMPTY(path, "elementCount="
							+ st.getElementCount()));
				} else if (doDestroy) {
					try {
						destroyResource(childRef);
						oklist.add(name);
					} catch (RNSError e) {
						nglist.add(e);
					}
				}
			} catch (RNSError e) {
				if (e.getError() == RNSError.Errno.ENOTDIR) {
					/* for junction */
					oklist.add(name);
				} else {
					nglist.add(e);
				}
			}
			portTypeMap.remove(path);
		}
		if (oklist.size() > 0) {
			RemoveRequestType req = new RemoveRequestType();
			req.setEntryName(oklist.toArray(new String[0]));
			RemoveResponseType res;
			try {
				res = dref.port.remove(req);
			} catch (WriteNotPermittedFaultType e) {
				throw RNSError.createEACCES(baseDir, null);
			} catch (RemoteException e) {
				throw RNSError.createENET(baseDir, e.getMessage(), e);
			}
			if (res == null) {
				throw RNSError.createEUNEXPECTED(baseDir,
						"RemoveResponse is null", null);
			}
			RNSEntryResponseType[] ers = res.getEntryResponse();
			if (ers == null) {
				throw RNSError.createEUNEXPECTED(baseDir,
						"RemoveResponse is invalid style", null);
			}
		}
		if (nglist.size() > 0) {
			return nglist.toArray(new RNSError[0]);
		} else {
			return null;
		}
	}

	public RNSError[] renameBulk(String baseDir, Map<String, String> fromtoMap)
			throws RNSError {
		debug("renameBulk", baseDir, null);
		DirRef dref = getDirRef(baseDir);
		ArrayList<NameMappingType> list = new ArrayList<NameMappingType>();
		Set<Entry<String, String>> set = fromtoMap.entrySet();
		for (Entry<String, String> ent : set) {
			NameMappingType nm = new NameMappingType();
			String from = ent.getKey();
			if (from == null || from.length() == 0) {
				continue;
			}
			String to = ent.getValue();
			if (to == null || to.length() == 0) {
				continue;
			}
			portTypeMap.remove(RNSUtil.joinPath(baseDir, from));
			nm.setSourceName(from);
			nm.setTargetName(to);
			list.add(nm);
		}
		if (list.size() == 0) {
			return null;
		}
		RenameRequestType req = new RenameRequestType();
		req.setRenameRequest(list.toArray(new NameMappingType[0]));
		RenameResponseType res;
		try {
			res = dref.port.rename(req);
		} catch (WriteNotPermittedFaultType e) {
			throw RNSError.createEACCES(baseDir, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(baseDir, e.getMessage(), e);
		}
		if (res == null) {
			throw RNSError.createEUNEXPECTED(baseDir, "RenameResponse is null",
					null);
		}
		RNSEntryResponseType[] ers = res.getEntryResponse();
		if (ers == null) {
			throw RNSError.createEUNEXPECTED(baseDir,
					"RenameResponse is invalid style", null);
		}
		ArrayList<RNSError> al = new ArrayList<RNSError>();
		for (RNSEntryResponseType er : ers) {
			BaseFaultType bf = er.getFault();
			if (bf != null) {
				String childPath = RNSUtil.normalizePath(baseDir + "/"
						+ er.getEntryName());
				if (bf instanceof RNSEntryExistsFaultType) {
					al.add(RNSError.createEEXIST(childPath, null));
				} else if (bf instanceof RNSEntryDoesNotExistFaultType) {
					al.add(RNSError.createENOENT(childPath, null));
				} else {
					al.add(RNSError.createENET(childPath, bf.getMessage(), bf));
				}
			}
		}
		if (al.size() > 0) {
			return al.toArray(new RNSError[0]);
		} else {
			return null;
		}
	}

	public RNSError[] setMetadataBulk(String baseDir,
			Map<String, MessageElement[]> entries) throws RNSError {
		debug("setMetadataBulk", baseDir, null);
		ArrayList<String> names = new ArrayList<String>();
		Set<Entry<String, MessageElement[]>> set = entries.entrySet();
		for (Entry<String, MessageElement[]> ent : set) {
			names.add(ent.getKey());
		}
		if (names.size() == 0) {
			return null;
		}
		DirRef dref = getDirRef(baseDir);
		ArrayList<RNSError> retError = new ArrayList<RNSError>();
		ArrayList<MetadataMappingType> reqList = new ArrayList<MetadataMappingType>();
		RNSDirHandle dh = list(dref, names.toArray(new String[0]), false);

		for (RNSDirent de : dh) {
			RNSError error = de.getRNSError();
			if (error != null) {
				retError.add(error);
			} else {
				RNSMetadataType meta = de.getMeta();
				SupportsRNSType supportsRns = meta.getSupportsRns();
				MetadataMappingType mm = new MetadataMappingType();
				mm.setEntryName(de.getName());
				mm.setSupportsRns(supportsRns); /* MUST (get and set) */
				mm.set_any(entries.get(de.getName()));
				reqList.add(mm);
			}
		}
		RNSError error = dh.getError();
		if (error != null) {
			throw error;
		}
		SetMetadataRequestType req = new SetMetadataRequestType();
		MetadataMappingType[] mms = reqList.toArray(new MetadataMappingType[0]);
		req.setSetMetadataRequest(mms);
		long s = countTimeStart();
		try {
			SetMetadataResponseType setMetaRes = dref.port.setMetadata(req);
			RNSEntryResponseType[] ents = setMetaRes.getEntryResponse();
			if (ents != null && ents.length > 0) {
				for (int i = 0; i < ents.length; i++) {
					BaseFaultType fault = ents[i].getFault();
					if (fault != null) {
						String path = RNSUtil.joinPath(baseDir,
								ents[i].getEntryName());
						retError.add(RNSUtil.convertBaseFault(fault, path, path));
					}
				}
			}
		} catch (WriteNotPermittedFaultType e) {
			throw RNSError.createEACCES(baseDir, null);
		} catch (RemoteException e) {
			throw RNSError.createENET(baseDir, e.getMessage(), e);
		} finally {
			countTimeEnd("setMetadata", s);
		}
		RNSError[] ret = retError.toArray(new RNSError[0]);
		if (ret != null && ret.length > 0) {
			return ret;
		} else {
			return null;
		}
	}

	public void mkdir(String path) throws RNSError {
		mkdir(path, null);
	}

	public void mkdir(String path, MessageElement[] xmls) throws RNSError {
		DirnameBasename dbn = new DirnameBasename(path);
		DirRef dref = getDirRef(dbn.getDirname());
		if (dbn.getBasename() == null) { /* '/': root directory */
			throw RNSError.createEEXIST(dbn.getDirname(), null);
		}
		mkdir(path, dref, dbn.getBasename(), xmls);
	}

	private void mkdir(String path, DirRef dref, String name,
			MessageElement[] xmls) throws RNSError {
		debug("mkdir", path, null);
		/* EPR is null (== mkdir) */
		clientAdd(dref.port, dref.path, name, true, null, xmls);
	}

	private void walkRemoveRecursive(int depth, int maxDepth, DirRef parentRef,
			String dirname, String basename, boolean doDestroy,
			boolean isForce, boolean forDirectory, boolean forJunction,
			RNSRecursiveListener listener) throws RNSError {
		if (depth >= maxDepth) {
			throw RNSError.createELOOP(dirname, "Too deep: depth="
					+ (depth + 1) + ", specified=" + maxDepth);
		}
		DirRef childRef = null;
		try {
			childRef = getDirRef(parentRef, basename);
		} catch (RNSError e) {
			if (e.getError() != RNSError.Errno.ENOTDIR) {
				throw e;
			}
		}
		String path = RNSUtil.joinPath(dirname, basename);
		if (childRef != null) {
			RNSDirHandle dirh;
			try {
				dirh = list(childRef, null, false);
				if (dirh == null) {
					return; /* end */
				}
			} catch (RNSError e) {
				if (e.getError() == RNSError.Errno.ENOENT) {
					/* childRef is a reference only. */
					return; /* ignore */
				}
				throw e;
			}

			/* NOTE : can't remove entries while using WS-Iterator */
			/* directory list on memory temporally */
			ArrayList<RNSDirent> al = new ArrayList<RNSDirent>();
			for (RNSDirent rd : dirh) {
				if (rd == null) {
					continue;
				}
				/* need names only */
				/* saving memory: cut redundant data */
				rd.setEpr(null);
				rd.setMeta(null);
				al.add(rd);
			}
			RNSError e = dirh.getError();
			if (e != null) {
				throw e;
			}
			for (RNSDirent rd : al) {
				if (rd.isDirectory()) {
					walkRemoveRecursive(depth + 1, maxDepth, childRef, path,
							rd.getName(), doDestroy, isForce, forDirectory,
							forJunction, listener);
					remove(childRef, path, rd.getName(), doDestroy, isForce,
							forDirectory, forJunction, false, maxDepth,
							listener);
				} else {
					remove(childRef, path, rd.getName(), doDestroy, isForce,
							forDirectory, forJunction, false, maxDepth,
							listener);
				}
			}
		}
	}

	private void removeRecursive(DirRef parentRef, String dirname,
			String basename, boolean doDestroy, boolean isForce,
			boolean forDirectory, boolean forJunction, int maxDepth,
			RNSRecursiveListener listener) throws RNSError {
		RNSError e_save = null;
		try {
			walkRemoveRecursive(0, maxDepth, parentRef, dirname, basename,
					doDestroy, isForce, forDirectory, forJunction, listener);
			remove(parentRef, dirname, basename, doDestroy, isForce,
					forDirectory, forJunction, false, maxDepth, listener);
		} catch (RNSError e) {
			e_save = e;
		}
		if (e_save != null) {
			throw e_save;
		}
	}

	private void removeEntryOnlyFromList(DirRef parentRef, String basename)
			throws RNSError {
		String path = RNSUtil.joinPath(parentRef.path, basename);
		debug("removeEntryOnly", path, null);
		clientRemove(parentRef.port, parentRef.path, basename);
	}

	private void destroyResource(DirRef dref) throws RNSError {
		debug("destroy", dref.path, null);
		long s = countTimeStart();
		try {
			dref.port.destroy(new Destroy());
		} catch (ResourceNotDestroyedFaultType e) {
			throw RNSError.createEACCES(dref.path, e.getMessage());
		} catch (ResourceUnavailableFaultType e) {
			throw RNSError.createEACCES(dref.path, e.getMessage());
		} catch (ResourceUnknownFaultType e) {
			throw RNSError.createEINVAL(dref.path, e.getMessage(), e);
		} catch (RemoteException e) {
			throw RNSError.createENET(dref.path, e.getMessage(), e);
		} finally {
			countTimeEnd("destroy", s);
		}
	}

	/* cannot remove root directory */
	private void remove(String path, boolean doDestroy, boolean isForce,
			boolean forDirectory, boolean forJunction, boolean isRecursive,
			int maxDepth, RNSRecursiveListener listener) throws RNSError {
		debug("remove", path, null);
		DirnameBasename dbn = new DirnameBasename(path);
		String dirname = dbn.getDirname();
		String basename = dbn.getBasename();
		if (basename == null) {
			if (dirname == null) {
				throw RNSError.createEINVAL(path, "path is null", null);
			} else { /* '/' : root directory */
				throw RNSError.createEINVAL(path,
						"cannot remove root directory", null);
			}
		}
		DirRef parentRef = getDirRef(dirname);
		try {
			remove(parentRef, dirname, basename, doDestroy, isForce,
					forDirectory, forJunction, isRecursive, maxDepth, listener);
		} catch (RNSError e) {
			throw e;
		}
	}

	/* forDirectory : check ENOTDIR */
	/* forJunction  : check EISDIR */
	/* isForce : ignore error */
	private void remove(DirRef parentRef, String dirname, String basename,
			boolean doDestroy, boolean isForce, boolean forDirectory,
			boolean forJunction, boolean isRecursive, int maxDepth,
			RNSRecursiveListener listener) throws RNSError {
		if (isRecursive) {
			removeRecursive(parentRef, dirname, basename, doDestroy, isForce,
					forDirectory, forJunction, maxDepth, listener);
		} else {
			DirRef childRef = null;
			String delPath = RNSUtil.joinPath(parentRef.path, basename);
			try {
				/* Is the entry directory or is the entry junction ? */
				childRef = getDirRef(parentRef, basename);
				/* is directory */
				if (forJunction && !isForce) {
					throw RNSError.createEISDIR(delPath, null);
				}
			} catch (RNSError e) {
				/* is not directory */
				if (e.getError() == RNSError.Errno.ENOTDIR) {
					if (!isRecursive && forDirectory && !isForce) {
						throw e;
					}
				} else if (e.getError() == RNSError.Errno.ENOENT) {
					/* remove reference only from list */
				} else {
					throw e;
				}
			}
			if (childRef != null && doDestroy && !isForce) {
				/* is directory */
				/* check existence */
				RNSStat st = null;
				try {
					st = stat(childRef);
				} catch (RNSError e) {
					if (e.getError() == RNSError.Errno.ENOENT) {
						doDestroy = false;
					} else {
						throw e;
					}
				}
				/* check empty */
				if (st != null && st.getElementCount().longValue() > 0) {
					throw RNSError.createENOTEMPTY(delPath, "elementCount="
							+ st.getElementCount());
				}
			}
			EndpointReferenceType epr = null;
			if (childRef == null) { /* junction */
				epr = getEPR(parentRef, basename, false);
			}
			if (childRef != null && doDestroy) {
				try {
					destroyResource(childRef);
				} catch (RNSError e) {
					if (isForce == false) {
						throw e;
					}
				}
			}
			removeEntryOnlyFromList(parentRef, basename);
			if (epr != null && listener != null) {
				listener.action(epr, dirname, basename);
			}
		}
	}

	public void rmdir(String path) throws RNSError {
		debug("rmdir", path, null);
		remove(path, true, false, true, false, false, 0, null);
	}

	public void rmJunction(String path) throws RNSError {
		debug("rmJunction", path, null);
		remove(path, false, false, false, true, false, 0, null);
	}

	public void remove(String path) throws RNSError {
		debug("remove", path, null);
		remove(path, true, false, false, false, false, 0, null);
	}

	/* ignore error */
	public void removeForce(String path) throws RNSError {
		debug("removeForce", path, null);
		remove(path, true, true, false, false, false, 0, null);
	}

	public void removeReference(String path) throws RNSError {
		debug("removeReference", path, null);
		remove(path, false, false, false, false, false, 0, null);
	}

	public void removeRecursive(String path) throws RNSError {
		removeRecursive(path, getDefaultMaxRecursiveDepth(), null);
	}

	public void removeRecursive(String path, int depth) throws RNSError {
		removeRecursive(path, depth, null);
	}

	public void removeRecursive(String path, int depth,
			RNSRecursiveListener listener) throws RNSError {
		debug("rmRecursive", path, null);
		remove(path, true, false, false, false, true, depth, listener);
	}

	public RNSDirHandle list(String path, boolean withRNSStat) throws RNSError {
		debug("list", path, null);
		return list(path, null, withRNSStat);
	}

	public RNSDirHandle list(String baseDir, String[] names, boolean withRNSStat)
			throws RNSError {
		debug("listBulk", baseDir, null);
		DirRef parentRef = getDirRef(baseDir);
		try {
			return list(parentRef, names, withRNSStat);
		} catch (RNSError e) {
			throw e;
		}
	}

	private RNSDirHandle list(DirRef parentRef, String[] names,
			boolean withRNSStat) throws RNSError {
		LookupResponseType res = clientList(parentRef.port, parentRef.path,
				names);
		if (res == null) {
			/* have no children */
			return null;
		}
		EndpointReferenceType itrEpr = res.getIterator();
		RNSEntryResponseType[] ents = res.getEntryResponse();
		if (ents == null || ents.length == 0) {
			return null;
		}
		if (itrEpr == null) {
			return new RNSDirHandle(new RNSDirentArrayIterator(parentRef, ents,
					withRNSStat));
		} else {
			debug("list", parentRef.path, "use iterator service");
			try {
				RNSListIterator itr = new RNSListIterator(itrEpr, clientHome,
						parentRef.port, parentRef.path, Arrays.asList(ents));
				return new RNSDirHandle(new WSIteratorIterator(parentRef, itr,
						withRNSStat));
			} catch (RNSError e) {
				throw e;
			} catch (Exception e) {
				throw RNSError.createENET(parentRef.path, e.getMessage(), e);
			}
		}
	}

	private class RNSDirentArrayIterator implements RNSIterator<RNSDirent> {
		private DirRef parentRef;
		private RNSEntryResponseType[] ents;
		private boolean withRNSStat;
		private int position;

		private RNSDirentArrayIterator(DirRef parentRef,
				RNSEntryResponseType[] ents, boolean withRNSStat) {
			this.parentRef = parentRef;
			this.ents = ents;
			this.withRNSStat = withRNSStat;
			position = 0;
		}

		public boolean hasNext() {
			if (ents == null || position >= ents.length) {
				return false;
			}
			return true;
		}

		public RNSDirent next() {
			if (hasNext() == false) {
				return null;
			}
			RNSEntryResponseType res = ents[position];
			RNSDirent rd = new RNSDirent();
			String path = RNSUtil.joinPath(parentRef.path, res.getEntryName());
			rd.setName(res.getEntryName());
			BaseFaultType bf = res.getFault();
			if (bf != null) {
				RNSError error = RNSUtil.convertBaseFault(bf, path, path);
				rd.setRNSError(error);
			} else {
				rd.setEpr(res.getEndpoint());
				rd.setMeta(res.getMetadata());
				if (rd.isDirectory() && withRNSStat) {
					try {
						RNSStat st = stat(getDirRef(parentRef, rd.getName()));
						rd.setStat(st);
					} catch (RNSError e) {
						rd.setRNSError(e);
					}
				}
			}
			position++;
			return rd;
		}

		@Override
		public void remove() {
			/* do nothing */
		}

		@Override
		public RNSError getError() {
			return null;
		}
	}

	private class WSIteratorIterator implements RNSIterator<RNSDirent> {
		private DirRef parentRef;
		private RNSListIterator itr;
		private boolean withRNSStat;
		private RNSError saveError = null;

		private WSIteratorIterator(DirRef parentRef, RNSListIterator itr,
				boolean withRNSStat) {
			this.parentRef = parentRef;
			this.itr = itr;
			this.withRNSStat = withRNSStat;
		}

		@Override
		public boolean hasNext() {
			try {
				return itr.hasNextWithError();
			} catch (RNSError e) {
				if (clientHome.isDebugMode()) {
					e.printStackTrace();
				}
				if (saveError == null) {
					saveError = e;
				}
				return false;
			}
		}

		@Override
		public RNSDirent next() {
			try {
				RNSDirent rd = itr.nextWithError();
				RNSError error = rd.getRNSError();
				if (error == null && withRNSStat) {
					try {
						RNSStat st = stat(getDirRef(parentRef, rd.getName()));
						rd.setStat(st);
					} catch (RNSError e) {
						rd.setRNSError(e);
					}
				}
				return rd;
			} catch (RNSError e) {
				if (clientHome.isDebugMode()) {
					e.printStackTrace();
				}
				if (saveError == null) {
					saveError = e;
				}
				return null;
			}
		}

		@Override
		public void remove() {
			/* do nothing */
		}

		@Override
		public RNSError getError() {
			return saveError;
		}
	}

	public boolean isDirectory(String path) throws RNSError {
		try {
			getDirRef(path);
			return true;
		} catch (RNSError e) {
			if (e.getError() == RNSError.Errno.ENOTDIR) {
				return false;
			}
			throw e;
		}
	}

	public boolean exists(String path) throws RNSError {
		try {
			getDirRef(path);
			return true;
		} catch (RNSError e) {
			if (e.getError() == RNSError.Errno.ENOTDIR) {
				return true;
			} else if (e.getError() == RNSError.Errno.ENOENT) {
				return false;
			}
			throw e;
		}
	}

	private Calendar deserializeMessageElementToCalendar(MessageElement me) {
		Calendar ct;
		try {
			ct = (Calendar) ObjectDeserializer.toObject(me, Calendar.class);
		} catch (DeserializationException e) {
			ct = Calendar.getInstance();
		}
		return ct;
	}

	private static final QName[] getRPqnames = new QName[] {
			RNSQNames.RP_ELEMENTCOUNT, RNSQNames.RP_CREATETIME,
			RNSQNames.RP_ACCESSTIME, RNSQNames.RP_MODIFICATIONTIME,
			RNSQNames.RP_READABLE, RNSQNames.RP_WRITABLE };
	private static final GetMultipleResourceProperties_Element getRPreq = new GetMultipleResourceProperties_Element(
			getRPqnames);

	private RNSStat stat(DirRef dref) throws RNSError {
		debug("stat", dref.path, null);
		GetMultipleResourcePropertiesResponse res = null;
		{
			long s = countTimeStart();
			try {
				res = dref.port.getMultipleResourceProperties(getRPreq);
			} catch (ResourceUnknownFaultType e) {
				throw RNSError.createENOENT(dref.path, e.getMessage());
			} catch (InvalidResourcePropertyQNameFaultType e) {
				throw RNSError.createEINVAL(dref.path, e.getMessage(), e);
			} catch (ResourceUnavailableFaultType e) {
				throw RNSError.createEACCES(dref.path, e.getMessage());
			} catch (RemoteException e) {
				throw RNSError.createENET(dref.path, e.getMessage(), e);
			} finally {
				countTimeEnd("getMultipleResourceProperties", s);
			}
		}
		RNSStat st = new RNSStat();
		Calendar defaultCal = Calendar.getInstance();
		st.setCreateTime(defaultCal);
		st.setAccessTime(defaultCal);
		st.setModificationTime(defaultCal);
		if (res != null) {
			MessageElement[] ms = res.get_any();
			if (ms != null) {
				for (MessageElement m : ms) {
					QName qn = m.getQName();
					if (RNSQNames.RP_ELEMENTCOUNT.equals(qn)) {
						st.setElementCount(new UnsignedLong(m.getValue()));
					} else if (RNSQNames.RP_CREATETIME.equals(qn)) {
						st.setCreateTime(deserializeMessageElementToCalendar(m));
					} else if (RNSQNames.RP_ACCESSTIME.equals(qn)) {
						st.setAccessTime(deserializeMessageElementToCalendar(m));
					} else if (RNSQNames.RP_MODIFICATIONTIME.equals(qn)) {
						st.setModificationTime(deserializeMessageElementToCalendar(m));
					} else if (RNSQNames.RP_READABLE.equals(qn)) {
						st.setReadable(Boolean.parseBoolean(m.getValue()));
					} else if (RNSQNames.RP_WRITABLE.equals(qn)) {
						st.setWritable(Boolean.parseBoolean(m.getValue()));
					}
				}
			}
		}
		return st;
	}

	/* directory only */
	public RNSStat stat(String path) throws RNSError {
		DirRef dref = getDirRef(path);
		return stat(dref);
	}

	public void copyEntry(String from, String to) throws RNSError {
		debug("copyEntry", from, "-> " + to);
		RNSEntryResponseType ent = getEntry(from, true);
		RNSMetadataType meta = ent.getMetadata();
		if (meta != null) {
			if (RNSUtil.isDirectory(meta)) {
				addRNSEPR(to, ent.getEndpoint(), ent.getMetadata().get_any());
			} else {
				addJunction(to, ent.getEndpoint(), ent.getMetadata().get_any());
			}
		} else {
			addJunction(to, ent.getEndpoint());
		}
	}

	public void rename(String from, String to) throws RNSError {
		debug("rename", from, "-> " + to);
		DirnameBasename fromDB, toDB;
		fromDB = new DirnameBasename(from);
		toDB = new DirnameBasename(to);
		if (fromDB.getBasename() == null) {
			/* root directory */
			throw RNSError.createEACCES(from, null);
		}
		if (fromDB.getDirname() == null) {
			fromDB.setDirname("/");
		}
		if (toDB.getDirname() == null) {
			toDB.setDirname("/");
		}

		if (to.endsWith("/")) {
			/* directory mode */
			if (toDB.getBasename() != null) {
				toDB.setDirname(RNSUtil.joinPath(toDB.getDirname(),
						toDB.getBasename()));
			}
			toDB.setBasename(fromDB.getBasename());
		}
		if (toDB.getBasename() == null) {
			toDB.setBasename(fromDB.getBasename());
		}

		if (fromDB.getDirname().equals(toDB.getDirname())) {
			/* rename */
			if (fromDB.getBasename().equals(toDB.getBasename())) {
				throw RNSError.createEEXIST(to, null);
			}
			DirRef dref = getDirRef(fromDB.getDirname());
			clientRename(dref.port, dref.path, fromDB.getBasename(),
					toDB.getBasename());
		} else {
			/* move */
			String newFrom = RNSUtil.joinPath(fromDB.getDirname(),
					fromDB.getBasename());
			String newTo = RNSUtil.joinPath(toDB.getDirname(),
					toDB.getBasename());
			if (newTo.startsWith(newFrom)) {
				throw RNSError.createEINVAL(newFrom, "cannot move `" + newFrom
						+ "' to a subdirectory of itself, `" + newTo + "/"
						+ toDB.getBasename() + "'", null);
			}
			copyEntry(newFrom, newTo);
			try {
				removeReference(newFrom);
			} catch (RNSError e) {
				removeReference(newTo);
				throw e;
			}
		}
	}

	public void addRNSEPR(String path, EndpointReferenceType epr)
			throws RNSError {
		addRNSEPR(path, epr, null);
	}

	public void addRNSEPR(String path, EndpointReferenceType epr,
			MessageElement[] xmls) throws RNSError {
		debug("addRNSEPR", path, null);
		addEPR(path, epr, xmls, true);
	}

	public void addJunction(String path, EndpointReferenceType epr)
			throws RNSError {
		addJunction(path, epr, null);
	}

	public void addJunction(String path, EndpointReferenceType epr,
			MessageElement[] xmls) throws RNSError {
		debug("addJunction", path, null);
		addEPR(path, epr, xmls, false);
	}

	private void addEPR(String path, EndpointReferenceType epr,
			MessageElement[] xmls, boolean isDirectory) throws RNSError {
		debug("addEPR", path, null);
		DirnameBasename db = new DirnameBasename(path);
		if (db.getBasename() == null) {
			throw RNSError.createEINVAL(path, "basename is null", null);
		}
		DirRef dref = getDirRef(db.getDirname());
		clientAdd(dref.port, dref.path, db.getBasename(), isDirectory, epr,
				xmls);
	}

	public EndpointReferenceType getEPR(String path, boolean dirOK)
			throws RNSError {
		RNSEntryResponseType ent = getEntry(path, dirOK);
		return ent.getEndpoint();
	}

	private EndpointReferenceType getEPR(DirRef parentRef, String basename,
			boolean dirOK) throws RNSError {
		RNSEntryResponseType ent = getEntry(parentRef, basename, dirOK);
		return ent.getEndpoint();
	}

	public MessageElement[] getMetadata(String path) throws RNSError {
		RNSEntryResponseType ent = getEntry(path, true);
		RNSMetadataType rmt = ent.getMetadata();
		if (rmt == null) {
			return null;
		}
		return rmt.get_any();
	}

	private RNSEntryResponseType getEntry(String path, boolean dirOK)
			throws RNSError {
		DirnameBasename db = new DirnameBasename(path);
		if ("/".equals(db.getDirname()) && db.getBasename() == null) {
			/* response from local */
			RNSEntryResponseType ent = new RNSEntryResponseType();
			ent.setEndpoint(rootEPR);
			ent.setMetadata(null); /* root directory can not have metadata */
			ent.setEntryName("/");
			return ent;
		}
		DirRef parentRef = getDirRef(db.getDirname());
		try {
			return getEntry(parentRef, db.getBasename(), dirOK);
		} catch (RNSError e) {
			throw e;
		}
	}

	private RNSEntryResponseType getEntry(DirRef parentRef, String basename,
			boolean dirOK) throws RNSError {
		String path = RNSUtil.joinPath(parentRef.path, basename);
		debug("getEntry", path, null);
		LookupResponseType res = clientLookup(parentRef.port, path, basename);
		RNSEntryResponseType[] es = res.getEntryResponse();
		if (es.length != 1) {
			throw RNSError.createEUNEXPECTED(path, "lookup entry is not one",
					null);
		}
		if (dirOK == true || RNSUtil.isDirectory(es[0].getMetadata()) == false) {
			return es[0];
		} else {
			throw RNSError.createEISDIR(path, null);
		}
	}

	public void setMetadata(String path, MessageElement[] xmls) throws RNSError {
		debug("setXML", path, null);
		DirnameBasename db = new DirnameBasename(path);
		if ("/".equals(db.getDirname()) && db.getBasename() == null) {
			throw RNSError.createEINVAL(path,
					"root directory cannot have a metadata", null);
		}
		DirRef dref = getDirRef(db.getDirname());

		clientSetMetadata(dref.port, dref.path, db.getBasename(), xmls);
	}

	private void debug(String method, String path, String msg) {
		if (clientHome != null) {
			clientHome.debug(method, path, msg);
		}
	}

}
