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

import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.globus.wsrf.ResourceContext;
import org.globus.wsrf.ResourceContextException;
import org.globus.wsrf.ResourceException;
import org.naregi.rns.stubs.ACLFaultType;
import org.naregi.rns.stubs.GetACLRequestType;
import org.naregi.rns.stubs.GetACLResponseType;
import org.naregi.rns.stubs.GetCallerInfoRequestType;
import org.naregi.rns.stubs.GetCallerInfoResponseType;
import org.naregi.rns.stubs.GetServerStatusRequestType;
import org.naregi.rns.stubs.GetServerStatusResponseType;
import org.naregi.rns.stubs.NoopRequestType;
import org.naregi.rns.stubs.NoopResponseType;
import org.naregi.rns.stubs.ProfileType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.RemoveACLRequestType;
import org.naregi.rns.stubs.RemoveACLResponseType;
import org.naregi.rns.stubs.SetACLRequestType;
import org.naregi.rns.stubs.SetACLResponseType;
import org.naregi.rns.stubs.StartProfileRequestType;
import org.naregi.rns.stubs.StartProfileResponseType;
import org.naregi.rns.stubs.StopProfileRequestType;
import org.naregi.rns.stubs.StopProfileResponseType;
import org.naregi.rns.stubs.StringMapType;
import org.naregi.rns.util.RNSUtil;

/**
 * A service implementation for extension of RNS specification. (ACL operations,
 * etc.)
 */
public class ExtensionService {

	// TODO use RNSLog

	private RNSResource getResource() throws ResourceContextException,
			ResourceException {
		ResourceContext ctx = ResourceContext.getResourceContext();
		return (RNSResource) ctx.getResource();
	}

	/* --- sub operations ----------------------------------------------- */

	public NoopResponseType noop(NoopRequestType req) {
		return new NoopResponseType();
	}

	private static StartProfileResponseType startProfileResponse = new StartProfileResponseType();

	public StartProfileResponseType startProfile(StartProfileRequestType req)
			throws ReadNotPermittedFaultType {
		if (AccessControlSwitch.getType().equals(AccessControlSwitch.TYPE_VOMS)) {
			CallerInfo ci = AccessControlSwitch.getCallerInfomation();
			if (ci.isAdmin() == false) {
				throw new ReadNotPermittedFaultType();
			}
		}
		RNSProfiler.enable();
		return startProfileResponse;
	}

	public StopProfileResponseType stopProfile(StopProfileRequestType req)
			throws ReadNotPermittedFaultType {
		if (AccessControlSwitch.getType().equals(AccessControlSwitch.TYPE_VOMS)) {
			CallerInfo ci = AccessControlSwitch.getCallerInfomation();
			if (ci.isAdmin() == false) {
				throw new ReadNotPermittedFaultType();
			}
		}

		RNSProfiler.disable();
		List<ProfileType> list = RNSProfiler.getResultList();
		ProfileType[] results = null;
		if (list != null) {
			results = list.toArray(new ProfileType[0]);
		}
		StopProfileResponseType res = new StopProfileResponseType();
		res.setResults(results);
		return res;
	}

	public GetServerStatusResponseType getServerStatus(
			GetServerStatusRequestType req) throws ReadNotPermittedFaultType {
		if (AccessControlSwitch.getType().equals(AccessControlSwitch.TYPE_VOMS)) {
			CallerInfo ci = AccessControlSwitch.getCallerInfomation();
			if (ci.isAdmin() == false) {
				throw new ReadNotPermittedFaultType();
			}
		}
		Runtime rt = Runtime.getRuntime();
		GetServerStatusResponseType res = new GetServerStatusResponseType();
		ArrayList<StringMapType> al = new ArrayList<StringMapType>();

		RNSUtil.gc(); /* correct Free Memory */

		StringMapType smt;

		smt = new StringMapType();
		smt.setName("Processors");
		smt.setValue("" + rt.availableProcessors());
		al.add(smt);

		smt = new StringMapType();
		smt.setName("FreeMemory");
		smt.setValue("" + rt.freeMemory());
		al.add(smt);

		smt = new StringMapType();
		smt.setName("TotalMemory");
		smt.setValue("" + rt.totalMemory());
		al.add(smt);

		smt = new StringMapType();
		smt.setName("MaxMemory");
		smt.setValue("" + rt.maxMemory());
		al.add(smt);

		res.setMap(al.toArray(new StringMapType[0]));
		return res;
	}

	/* --- ACL operations ----------------------------------------------- */

	private ACLFaultType createACLFaultType(String message) {
		ACLFaultType f = new ACLFaultType();
		f.setFaultString(message);
		return f;
	}

	public GetACLResponseType getACL(GetACLRequestType req)
			throws RemoteException, ACLFaultType {
		RNSResource r = getResource();
		CallerInfo callerInfo = AccessControlSwitch.getCallerInfomation();
		if (AccessControlSwitch.canModify(r, callerInfo) == false
				&& AccessControlSwitch.canRead(r, callerInfo) == false) {
			throw createACLFaultType("GetACL: not permitted");
		}
		/* (canModify == true || canRead == true) */
		GetACLResponseType res = new GetACLResponseType();
		res.setEntries(r.getACL().toACLEntries());
		return res;
	}

	public SetACLResponseType setACL(SetACLRequestType req)
			throws RemoteException, ACLFaultType {
		RNSResource r = getResource();
		CallerInfo callerInfo = AccessControlSwitch.getCallerInfomation();
		if (AccessControlSwitch.canModify(r, callerInfo) == false) {
			throw createACLFaultType("SetACL: not permitted");
		}
		ACL reqAcl = new ACL(req.getEntries());
		ACL currentAcl;
		try {
			currentAcl = r.getACL();
			currentAcl.autoComplete();
		} catch (ResourceException e) {
			throw e;
		}
		boolean isAdmin = AccessControlSwitch.isAdmin(r, callerInfo);
		String reqOwner = reqAcl.getOwner();
		if (reqOwner != null) { /* chown */
			if (reqOwner.equals("")) {
				reqAcl.setOwner(currentAcl.getOwner(), reqAcl.getOwnerPerm());
			} else if (isAdmin == false
					&& reqOwner.equals(currentAcl.getOwner()) == false) {
				throw createACLFaultType("SetACL: non-admin cannot change owner");
			}
		}
		String reqOwnerGroup = reqAcl.getOwnerGroup();
		if (reqOwnerGroup != null) { /* chgrp */
			if (reqOwnerGroup.equals("")) {
				reqAcl.setOwnerGroup(currentAcl.getOwnerGroup(),
						reqAcl.getOwnerGroupPerm());
			} else if (isAdmin == false
					&& reqOwnerGroup.equals(currentAcl.getOwnerGroup()) == false
					&& reqOwnerGroup.equals(callerInfo.getMainGroup()) == false) {
				if (callerInfo.getGroupList() == null) {
					throw createACLFaultType("SetACL: non-admin cannot change ownerGroup");
				}
				boolean found = false;
				for (String g : callerInfo.getGroupList()) {
					if (reqOwnerGroup.equals(g)) {
						found = true;
						break;
					}
				}
				if (found == false) {
					throw createACLFaultType("SetACL: non-admin cannot change ownerGroup");
				}
			}
		}
		/* check whether the caller belong to all groups to set as default group */
		Map<String, Short> dgm = reqAcl.getDefaultGroupMap();
		if (isAdmin == false && dgm != null) {
			Set<String> names = dgm.keySet();
			for (String name : names) {
				boolean found = false;
				for (String g : callerInfo.getGroupList()) {
					if (name.equals(g)) {
						found = true;
						break;
					}
				}
				if (found == false) {
					throw createACLFaultType("SetACL: non-admin cannot add/change defaultGroup: "
							+ name);
				}
			}
		}

		synchronized (r.getLockAndStartTransaction()) {
			try {
				/* must not do autoComplete() */
				r.setACL(reqAcl);
				r.commit();
				return new SetACLResponseType();
			} catch (ResourceException e) {
				r.rollback();
				throw e;
			}
		}
	}

	public RemoveACLResponseType removeACL(RemoveACLRequestType req)
			throws RemoteException, ACLFaultType {
		RNSResource r = getResource();
		if (AccessControlSwitch.canModify(r,
				AccessControlSwitch.getCallerInfomation()) == false) {
			throw createACLFaultType("RemoveACL: not permitted");
		}
		int type = req.getType();
		if (type == ACL.TYPE_OWNER || type == ACL.TYPE_OWNERGROUP
				|| type == ACL.TYPE_OTHER) {
			throw createACLFaultType("RemoveACL: not permitted to remove owner/ownerGroup/other");
		}
		if (type == ACL.TYPE_MASK || type == ACL.TYPE_DEFAULT_MASK
				|| type == ACL.TYPE_DEFAULT_OTHER) {
			req.setNames(null);
		}
		synchronized (r.getLockAndStartTransaction()) {
			try {
				r.removeACL(req.getType(), req.getNames());
				r.commit();
				return new RemoveACLResponseType();
			} catch (ResourceException e) {
				r.rollback();
				throw e;
			}
		}
	}

	public GetCallerInfoResponseType getCallerInfo(GetCallerInfoRequestType req) {
		GetCallerInfoResponseType res = new GetCallerInfoResponseType();
		CallerInfo ci = AccessControlSwitch.getCallerInfomation();
		res.setAdmin(ci.isAdmin());
		res.setCaller(ci.getUserName());
		res.setMainGroup(ci.getMainGroup());
		List<String> l = ci.getGroupList();
		if (l != null) {
			res.setGroups(l.toArray(new String[0]));
		}
		return res;
	}
}
