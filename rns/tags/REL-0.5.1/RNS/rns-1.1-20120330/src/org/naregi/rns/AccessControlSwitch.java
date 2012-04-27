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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Collections;
import java.util.Map;

import org.apache.commons.collections.map.ReferenceMap;
import org.naregi.rns.util.TimeoutCacheMap;

/**
 * Switch AccessControl.
 */
public class AccessControlSwitch {
	public static final String TYPE_SIMPLE = "simple";
	public static final String TYPE_VOMS = "voms";
	public static final String TYPE_NONE = "none";

	private static String type;
	private static AccessControl ac;

	static {
		type = RNSConfig.getAccessControlType();
		if (TYPE_SIMPLE.equalsIgnoreCase(type)) {
			ac = new AccessControlSimple();
			RNSLog.getLog().debug("AccessControl:type=" + type);
		} else if (TYPE_VOMS.equalsIgnoreCase(type)) {
			ac = new AccessControlVOMS();
			RNSLog.getLog().debug("AccessControl:type=" + type);
		} else if (TYPE_NONE.equalsIgnoreCase(type)) {
			ac = new AccessControlNone();
			RNSLog.getLog().debug("AccessControl:type=" + type);
		} else {
			ac = null;
			RNSLog.getLog().warn("unknown AccessControl:type=" + type);
		}
	}

	public static String getType() {
		return type;
	}

	public static CallerInfo getCallerInfomation() {
		if (ac == null) {
			return null;
		}
		return ac.getCallerInfomation();
	}

	public static boolean canRead(RNSResource resource, CallerInfo callerInfo) {
		if (ac == null) {
			return false;
		}
		return ac.canRead(resource, callerInfo);
	}

	public static boolean canWrite(RNSResource resource, CallerInfo callerInfo) {
		if (ac == null) {
			return false;
		}
		return ac.canWrite(resource, callerInfo);
	}

	public static boolean canModify(RNSResource resource, CallerInfo callerInfo) {
		if (ac == null) {
			return false;
		}
		return ac.canModify(resource, callerInfo);
	}

	@SuppressWarnings("unchecked")
	private static Map<String, Map<String, Boolean>> cache = Collections.synchronizedMap(new ReferenceMap(
			ReferenceMap.SOFT, ReferenceMap.SOFT));
	private static long cacheLifetime = 10000;

	public static boolean searchUser(String filename, String userId) {
		if (userId == null || userId.equals("")) {
			return false;
		}
		Map<String, Boolean> map = cache.get(filename);
		if (map != null) {
			Boolean b = map.get(userId);
			if (b != null) {
				return b.booleanValue();
			}
		}

		File f = new File(RNSConfig.CONFDIR_HOME + filename);
		if (!f.canRead()) {
			f = new File(RNSConfig.CONFDIR_ETC + filename);
		}
		BufferedReader br = null;
		boolean res = false;
		try {
			FileInputStream fin = new FileInputStream(f);
			InputStreamReader isr = new InputStreamReader(fin);
			br = new BufferedReader(isr);
			String line;
			while ((line = br.readLine()) != null) {
				if (userId.equals(line)) {
					res = true;
					break;
				}
			}
		} catch (FileNotFoundException e) {
		} catch (IOException e) {
			RNSLog.getLog().error(f.getAbsolutePath() + ": " + e.getMessage());
		} finally {
			try {
				if (br != null) {
					br.close();
				}
			} catch (IOException e) {
			}
		}

		if (map == null) {
			map = new TimeoutCacheMap<String, Boolean>(cacheLifetime);
			cache.put(filename, map);
		}
		map.put(userId, Boolean.valueOf(res));
		return res;
	}

	public static boolean isAdmin(RNSResource resource, CallerInfo callerInfo) {
		return ac.isAdmin(resource, callerInfo);
	}
}
