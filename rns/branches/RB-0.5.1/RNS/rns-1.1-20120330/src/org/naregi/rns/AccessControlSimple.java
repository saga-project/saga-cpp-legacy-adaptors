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

import java.util.List;

/**
 * An implementation for rns.server.accessControlType=simple
 */
public class AccessControlSimple implements AccessControl {
	private static final String CONF_READONLY = "rns-readonly";
	private static final String CONF_READWRITE = "rns-readwrite";

	private static class SimpleCallerInfo implements CallerInfo {
		private String name;

		SimpleCallerInfo(String name) {
			if (name == null) {
				this.name = ACL.ANONYMOUS;
			} else {
				this.name = name;
			}
		}

		public String getUserName() {
			return name;
		}

		public String getMainGroup() {
			return ACL.ANONYMOUS;
		}

		public List<String> getGroupList() {
			return null;
		}

		public boolean isAdmin() {
			return false;
		}
	}

	public CallerInfo getCallerInfomation() {
		String userId = org.globus.wsrf.security.SecurityManager.getManager()
				.getCaller();
		return new SimpleCallerInfo(userId);
	}

	public boolean canRead(RNSResource resource, CallerInfo callerInfo) {
		if (callerInfo == null) {
			return false;
		}
		String userId = callerInfo.getUserName();
		boolean b = AccessControlSwitch.searchUser(CONF_READWRITE, userId);
		if (b == false) {
			b = AccessControlSwitch.searchUser(CONF_READONLY, userId);
		}
		if (b) {
			RNSLog.getLog().debug("isReadable:" + b + ":" + userId);
		} else {
			RNSLog.getLog().warn("isReadable:" + b + ":" + userId);
		}
		return b;
	}

	public boolean canWrite(RNSResource resource, CallerInfo callerInfo) {
		if (callerInfo == null) {
			return false;
		}
		String userId = callerInfo.getUserName();
		boolean b = AccessControlSwitch.searchUser(CONF_READWRITE, userId);
		if (b) {
			RNSLog.getLog().debug("isWritable:" + b + ":" + userId);
		} else {
			RNSLog.getLog().warn("isWritable:" + b + ":" + userId);
		}
		return b;
	}

	public boolean canModify(RNSResource resource, CallerInfo callerInfo) {
		/* no ACL operation */
		return false;
	}

	public boolean isAdmin(RNSResource resource, CallerInfo callerInfo) {
		return false;
	}
}
