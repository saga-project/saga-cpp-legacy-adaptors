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

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import javax.security.auth.Subject;

import org.glite.security.voms.BasicVOMSTrustStore;
import org.glite.security.voms.FQAN;
import org.glite.security.voms.VOMSAttribute;
import org.glite.security.voms.VOMSValidator;
import org.globus.wsrf.security.SecurityManager;

/**
 * An implementation for rns.server.accessControlType=voms
 */
public class AccessControlVOMS implements AccessControl {
	private static final String CONF_ADMIN = "rns-acl-admin";

	private static class VOMSCallerInfo implements CallerInfo {
		private boolean isAdmin = false;
		private String userId = null;
		private String group = null;
		private List<String> groupList = null;

		private static boolean initializedVOMS = false;

		public VOMSCallerInfo() {

			if (initializedVOMS == false) {
				try {
					BasicVOMSTrustStore store = new BasicVOMSTrustStore();
					VOMSValidator.setTrustStore(store);
				} catch (Throwable e) {
					//e.printStackTrace();
					RNSLog.getLog().info("VOMSCallerInfo: " + e.getMessage());
				}
				initializedVOMS = true;
			}

			try {
				SecurityManager sm = org.globus.wsrf.security.SecurityManager.getManager();
				userId = sm.getCaller();
				Subject s = sm.getPeerSubject();
				if (s != null) {
					//					System.out.println("***********");
					//					System.out.println("subject: " + s);
					Set<?> publicCreds = s.getPublicCredentials(X509Certificate[].class);
					Iterator<?> iter = publicCreds.iterator();
					//					String vo = null;
					//					String hostport = null;
					while (iter.hasNext()) {
						X509Certificate certRev[] = (X509Certificate[]) iter.next();
						int size = certRev.length;
						X509Certificate cert[] = new X509Certificate[size];
						for (int i = 0; i < size; i++) {
							cert[i] = certRev[size - i - 1];
						}
						VOMSValidator validator = new VOMSValidator(cert).parse();
						List<?> vector = validator.getVOMSAttributes();
						if (vector.size() != 0) {
							for (int j = 0; j < vector.size(); j++) {
								VOMSAttribute attrib = (VOMSAttribute) vector.get(j);
								//								vo = attrib.getVO();
								//								System.out.println("VO: " + vo);
								//								hostport = attrib.getHostPort();
								//								System.out.println("hostport: " + hostport);
								List<?> fl = attrib.getListOfFQAN();
								groupList = new ArrayList<String>();
								for (int k = 0; k < fl.size(); k++) {
									FQAN f = (FQAN) fl.get(k);
									if (k == 0) {
										group = f.getFQAN();
									} else {
										groupList.add(f.getFQAN());
									}
									//									System.out.println("FQAN: " + f.getFQAN());
									//									System.out.println("group: " + f.getGroup());
									//									System.out.println("role: " + f.getRole());
									//									System.out.println("capability: "
									//											+ f.getCapability());
								}
								//								List<?> fqan = attrib.getFullyQualifiedAttributes();
								//								for (int k = 0; k < fqan.size(); k++) {
								//									String str = (String) fqan.get(k);
								//									System.out.println("attributes: " + str);
								//								}
							}
						}
					}
				}
			} catch (Throwable e) {
				RNSLog.getLog().error("" + e.getMessage());
				e.printStackTrace();
			}

			if (userId == null) {
				userId = ACL.ANONYMOUS;
			}
			if (group == null) {
				group = ACL.ANONYMOUS;
			}

			isAdmin = AccessControlSwitch.searchUser(CONF_ADMIN, userId);
		}

		public String getUserName() {
			return userId;
		}

		public String getMainGroup() {
			return group;
		}

		public List<String> getGroupList() {
			return groupList;
		}

		public boolean isAdmin() {
			return isAdmin;
		}
	}

	public CallerInfo getCallerInfomation() {
		return  new VOMSCallerInfo();
	}

	private short getCallerPerm(RNSResource resource, CallerInfo callerInfo,
			boolean checkAdmin) {
		if (callerInfo == null) {
			return 0;
		}
		if (checkAdmin && isAdmin(resource, callerInfo)) {
			return ACL.PERM_ALL;
		}
		ACL acl;
		try {
			acl = resource.getACL();
		} catch (Exception e) {
			e.printStackTrace();
			RNSLog.getLog().error(
					"AccessControlVOMS:getCallerPerm:getACL:" + e.getMessage());
			return 0;
		}
		// set anonymous and etc.
		acl.autoComplete();
		// owner
		String owner = acl.getOwner();
		if (owner != null) {
			// anonymous user implies all users
			if (owner.equals(ACL.ANONYMOUS)
					|| owner.equals(callerInfo.getUserName())) {
				return acl.getOwnerPerm();
			}
		}
		// ownerGroup
		String ownerGroup = acl.getOwnerGroup();
		if (ownerGroup != null) {
			// anonymous group implies all groups
			if (ownerGroup.equals(ACL.ANONYMOUS)
					|| ownerGroup.equals(callerInfo.getMainGroup())) {
				return acl.getOwnerGroupPerm();
			}
		}
		// user
		Short u = acl.getUserPerm(callerInfo.getUserName());
		if (u != null) { // entry exists
			return u;
		}
		// group
		if (callerInfo.getGroupList() != null) {
			for (String s : callerInfo.getGroupList()) {
				Short g = acl.getGroupPerm(s);
				if (g != null) {
					return g;
				}
			}
		}
		// other
		return acl.getOtherPerm();
	}

	public boolean canRead(RNSResource resource, CallerInfo callerInfo) {
		return ACL.canRead(getCallerPerm(resource, callerInfo, true));
	}

	public boolean canWrite(RNSResource resource, CallerInfo callerInfo) {
		return ACL.canWrite(getCallerPerm(resource, callerInfo, false));
	}

	public boolean canModify(RNSResource resource, CallerInfo callerInfo) {
		if (callerInfo == null) {
			return false;
		}
		if (isAdmin(resource, callerInfo)) {
			return true;
		}
		ACL acl;
		try {
			acl = resource.getACL();
		} catch (Exception e) {
			e.printStackTrace();
			RNSLog.getLog().error(
					"AccessControlVOMS:canModify:getACL:" + e.getMessage());
			return false;
		}
		acl.autoComplete();
		String owner = acl.getOwner();
		// isOwner?
		if (owner != null) {
			// all users can modify a directry which is owned by anonymous
			if (owner.equals(ACL.ANONYMOUS)
					|| owner.equals(callerInfo.getUserName())) {
				return true;
			}
		}
		return false;
	}

	public boolean isAdmin(RNSResource resource, CallerInfo callerInfo) {
		return callerInfo.isAdmin();
	}
}
