/*
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
 * $RCSfile: MDSInfo.java,v $ $Revision: 1.5 $ $Date: 2005/12/01 08:33:17 $
 */
package org.apgrid.grpc.ng.info;

import java.util.Enumeration;
import java.util.Properties;

import org.apgrid.grpc.ng.NgException;
import org.gridforum.gridrpc.GrpcException;

public class MDSInfo {
	/* keys for MDSInfo Information */
	public static final String KEY_TAG = "tag";
	public static final String KEY_HOSTNAME = "hostname";
	public static final String KEY_PORT = "port";
	public static final String KEY_PROTOCOL = "protocol";
	public static final String KEY_PATH = "path";
	public static final String KEY_SUBJECT = "subject";
	public static final String KEY_TYPE = "type";
	public static final String KEY_VONAME = "vo_name";
	public static final String KEY_CLIENT_TIMEOUT = "client_timeout";
	public static final String KEY_SERVER_TIMEOUT = "server_timeout";
	
	/* values for MDSInfo Information */
	public static final String VAL_TYPE_MDS2 = "MDS2";
	public static final String VAL_TYPE_MDS4 = "MDS4";
	
	private Properties propMDSInfo;
	boolean isLocked;

	/**
	 * 
	 */
	public MDSInfo(Properties prop) throws GrpcException {
		/* Initialize */
		propMDSInfo = new Properties();
		isLocked = false;
		/* set default variables */
		setDefaultParameter(propMDSInfo);
		/* overwrite variables */
		overwriteParameter(prop);
	}

	/**
	 * @param tmpProperties
	 * @throws GrpcException
	 */
	private void setDefaultParameter(Properties tmpProperties) throws GrpcException {
			if (tmpProperties.containsKey(KEY_PORT) != true) {
				tmpProperties.put(KEY_PORT, "0");
			}
			if (tmpProperties.containsKey(KEY_PROTOCOL) != true) {
				tmpProperties.put(KEY_PROTOCOL, "https");
			}
			if (tmpProperties.containsKey(KEY_TYPE) != true) {
				tmpProperties.put(KEY_TYPE, VAL_TYPE_MDS2);
			}
			if (tmpProperties.containsKey(KEY_VONAME) != true) {
				tmpProperties.put(KEY_VONAME, "local");
			}
			if (tmpProperties.containsKey(KEY_CLIENT_TIMEOUT) != true) {
				tmpProperties.put(KEY_CLIENT_TIMEOUT, "0");
			}
			if (tmpProperties.containsKey(KEY_SERVER_TIMEOUT) != true) {
				tmpProperties.put(KEY_SERVER_TIMEOUT, "0");
			}
	}

	/**
	 * @return
	 */
	public String getHandleString() {
		return makeHandleString(
			(String) propMDSInfo.get(KEY_HOSTNAME), propMDSInfo.getProperty(KEY_TAG));
	}
	
	/**
	 * @param hostname
	 * @param tag
	 * @return
	 */
	public static String makeHandleString(String hostname, String tag) {
		if (tag != null) {
			return hostname + "/" + tag;
		} else {
			return hostname + "/";
		}
	}

	/**
	 * @param key
	 * @return
	 */
	public Object get(String key) {
		return propMDSInfo.get(key);
	}
	
	/**
	 * @param key
	 * @param val
	 */
	public void put(String key, Object val) {
		propMDSInfo.put(key, val);
	}

	/**
	 * @param prop
	 */
	private void overwriteParameter(Properties prop) {
		/* copy all of values into specified Properties */
		Enumeration keys = prop.keys();
		while (keys.hasMoreElements()) {
			String keyString = (String) keys.nextElement();
				propMDSInfo.put(keyString, prop.get(keyString));
		}
	}

	/**
	 * @throws GrpcException
	 */
	public synchronized void lockMDS() throws GrpcException {
		/* wait for Unlocked */
		while (isLocked == true) {
			try {
				wait();
			} catch (InterruptedException e) {
				throw new NgException(e);
			}
		}
		/* lock */
		isLocked = true;
	}
	
	/**
	 * @throws GrpcException
	 */
	public synchronized void unlockMDS() throws GrpcException {
		/* check if it's locked */
		if (isLocked == false) {
			throw new NgException("Nobody lock the handle.");
		}
		/* unlock */
		isLocked = false;
		/* notifyAll */
		notifyAll();
	}
	
	/**
	 * @param indent
	 */
	public String toString(int indent) {
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < indent; i++) {
			sb.append(" ");
		}
		String indentStr = sb.toString();
		sb = new StringBuffer();

		Enumeration keys = propMDSInfo.keys();
		while (keys.hasMoreElements()) {
			String key = (String) keys.nextElement();
			sb.append(indentStr + "- " + key + " : " + propMDSInfo.get(key));
			sb.append("\n");
		}
		
		return sb.toString();
	}
}
