/**
 * $AIST_Release: 4.2.4 $
 * $AIST_Copyright:
 *  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
 *  National Institute of Advanced Industrial Science and Technology
 *  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  $
 * $RCSfile: RemoteClassPathInfo.java,v $ $Revision: 1.11 $ $Date: 2006/08/22 10:54:33 $
 */
package org.apgrid.grpc.ng.info;

import java.util.Enumeration;
import java.util.Properties;

public class RemoteClassPathInfo implements Cloneable {
	private Properties mapRemoteClassPathInfo;
	
	/* keys */
	public static final String KEY_CLASS_PATH_HOSTNAME = "hostName";
	public static final String KEY_CLASS_PATH_CLASSNAME = "className";
	public static final String KEY_CLASS_PATH_CLASSPATH = "path";
	public static final String KEY_CLASS_PATH_STAGING = "staging";
	public static final String KEY_CLASS_PATH_BACKEND = "backend";
	public static final String KEY_CLASS_PATH_NCPUS = "mpi_nCPUs";
	public static final String KEY_CLASS_PATH_SESSION_TIMEOUT = "session_timeout";
	
	/**
	 * 
	 */
	public RemoteClassPathInfo() {
		mapRemoteClassPathInfo = new Properties();
	}
	
	/**
	 * @param hostName
	 * @param className
	 * @param path
	 */
	public RemoteClassPathInfo(String hostName,
		String className, String path) {
		mapRemoteClassPathInfo = new Properties();
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_HOSTNAME, hostName);
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_CLASSNAME, className);
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_STAGING, "false");
		/* backend is not set */
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_CLASSPATH, path);
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_SESSION_TIMEOUT, "0");
	}
	
	/**
	 * @param hostName
	 * @param className
	 * @param staging
	 * @param path
	 * @param backend
	 */
	public RemoteClassPathInfo(String hostName,
		String className, String staging, String path, String backend,
		String session_timeout) {
		mapRemoteClassPathInfo = new Properties();
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_HOSTNAME, hostName);
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_CLASSNAME, className);
		if (staging == null) {
			staging = "false";
		}
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_STAGING, staging);
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_CLASSPATH, path);
		if (backend != null) {
			/* backend is not set by default */
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_BACKEND, backend);
		}
		if (session_timeout == null) {
			session_timeout = "0";
		}
		mapRemoteClassPathInfo.put(KEY_CLASS_PATH_SESSION_TIMEOUT, session_timeout);
	}
	
	/**
	 * @param key
	 * @return
	 */
	public Object get(String key) {
		return mapRemoteClassPathInfo.get(key);
	}
	
	/**
	 * @param key
	 * @param value
	 */
	public void put(String key, Object value) {
		mapRemoteClassPathInfo.put(key, value);
	}
	
	/**
	 * @param key
	 */
	public void remove(String key) {
		mapRemoteClassPathInfo.remove(key);
	}
	
	/**
	 * @param key
	 * @return
	 */
	public boolean contains(String key) {
		return mapRemoteClassPathInfo.containsKey(key);
	}
	
	/**
	 * @param rcPathInfo
	 */
	public void overwriteParam(RemoteClassPathInfo rcPathInfo) {
		/* hostname */
		if (rcPathInfo.contains(KEY_CLASS_PATH_HOSTNAME)) {
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_HOSTNAME,
				rcPathInfo.get(KEY_CLASS_PATH_HOSTNAME));
		}
		/* classname */
		if (rcPathInfo.contains(KEY_CLASS_PATH_CLASSNAME)) {
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_CLASSNAME,
				rcPathInfo.get(KEY_CLASS_PATH_CLASSNAME));
		}
		/* path */
		if (rcPathInfo.contains(KEY_CLASS_PATH_CLASSPATH)) {
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_CLASSPATH,
				rcPathInfo.get(KEY_CLASS_PATH_CLASSPATH));
		}
		/* staging */
		if (rcPathInfo.contains(KEY_CLASS_PATH_STAGING)) {
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_STAGING,
				rcPathInfo.get(KEY_CLASS_PATH_STAGING));
		}
		/* backend */
		if (rcPathInfo.contains(KEY_CLASS_PATH_BACKEND)) {
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_BACKEND,
				rcPathInfo.get(KEY_CLASS_PATH_BACKEND));
		}
		/* nCPUs */
		if (rcPathInfo.contains(KEY_CLASS_PATH_NCPUS)) {
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_NCPUS,
				rcPathInfo.get(KEY_CLASS_PATH_NCPUS));
		}
		/* session_timeout */
		if (rcPathInfo.contains(KEY_CLASS_PATH_SESSION_TIMEOUT)) {
			mapRemoteClassPathInfo.put(KEY_CLASS_PATH_SESSION_TIMEOUT,
				rcPathInfo.get(KEY_CLASS_PATH_SESSION_TIMEOUT));
		}
	}
	
	/**
	 * @param indent
	 * @return
	 */
	public String toString(int indent) {
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < indent; i++) {
			sb.append(" ");
		}
		String indentStr = sb.toString();
		sb = new StringBuffer();
		
		/* print key & variables */
		Enumeration keys = mapRemoteClassPathInfo.keys();
		while (keys.hasMoreElements()) {
			String key = (String) keys.nextElement();
			sb.append(indentStr + "- " + key + " : " + mapRemoteClassPathInfo.get(key));
			sb.append("\n");
		}
		
		return sb.toString();
	}

	/**
	 * @return
	 */
	protected RemoteClassPathInfo getCopy() {
		try {
			return (RemoteClassPathInfo) this.clone();
		} catch (CloneNotSupportedException e) {
			e.printStackTrace();
		}
		return null;
	}
}
