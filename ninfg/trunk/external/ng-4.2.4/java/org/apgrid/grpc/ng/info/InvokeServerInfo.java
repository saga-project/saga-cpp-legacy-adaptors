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
 * $RCSfile: InvokeServerInfo.java,v $ $Revision: 1.3 $ $Date: 2005/10/05 03:01:39 $
 */
package org.apgrid.grpc.ng.info;

import java.util.Enumeration;
import java.util.List;
import java.util.Properties;

import org.apgrid.grpc.ng.NgException;
import org.gridforum.gridrpc.GrpcException;

public class InvokeServerInfo {
	/* keys for Invoke Server Information */
	public static final String KEY_TYPE = "type";
	public static final String KEY_PATH = "path";
	public static final String KEY_LOG_FILEPATH = "log_filePath";
	public static final String KEY_MAXJOBS = "max_jobs";
	public static final String KEY_OPTION = "option";
	public static final String KEY_STATUS_POLLING = "status_polling";

	/* instance variables */
	private String type = null;
	private String path = null;
	private String log_filePath = null;
	private int maxJobs = 0;
	private List options = null;
	private int statusPolling = 0;
	
	/**
	 * @param propInvokeServerInfo
	 * @throws GrpcException
	 */
	public InvokeServerInfo(Properties propInvokeServerInfo) throws GrpcException {
		/* check if it contains key parameter */
		if (! propInvokeServerInfo.containsKey(KEY_TYPE)) {
			throw new NgException("key [" + KEY_TYPE + "] is not in InvokeServerInfo.");
		}
		
		/* set parameters from Properties */
		Enumeration keys = propInvokeServerInfo.keys();
		while (keys.hasMoreElements()) {
			String keyString = (String) keys.nextElement();
			if (keyString.equals(KEY_TYPE)) {
				/* type */
				type = (String) propInvokeServerInfo.get(keyString);
			} else if (keyString.equals(KEY_PATH)) {
				/* path */
				path = (String) propInvokeServerInfo.get(keyString);
			} else if (keyString.equals(KEY_LOG_FILEPATH)) {
				/* log_filePath */
				log_filePath = (String) propInvokeServerInfo.get(keyString);
			} else if (keyString.equals(KEY_MAXJOBS)) {
				/* max_jobs */
				maxJobs =
					Integer.parseInt((String) propInvokeServerInfo.get(KEY_MAXJOBS));
			} else if (keyString.equals(KEY_OPTION)) {
				/* option for Invoke Server */
				options = (List) propInvokeServerInfo.get(keyString);
			} else if (keyString.equals(KEY_STATUS_POLLING)) {
				/* interval of status polling */
				statusPolling =
					Integer.parseInt((String) propInvokeServerInfo.get(KEY_STATUS_POLLING));
			} else {
				/* unsupported key */
				throw new NgException("Invalid key[ " +
					keyString +	"] for InvokeServerInfo.");
			}
		}
	}
	
	/**
	 * @return
	 */
	public String getType() {
		return this.type;
	}
	
	/**
	 * @return
	 */
	public String getPath() {
		return this.path;
	}
	
	/**
	 * @return
	 */
	public String getLogFilePath() {
		return this.log_filePath;
	}
	
	/**
	 * @return
	 */
	public int getMaxJobs() {
		return this.maxJobs;
	}
	
	/**
	 * @return
	 */
	public List getOptions() {
		return this.options;
	}
	
	/**
	 * @return
	 */
	public int getStatusPolling() {
		return this.statusPolling;
	}
	
	/**
	 * @param indent
	 * @return
	 */
	public String toString(int indent) {
		StringBuffer sb = new StringBuffer();
		for (int i = 0 ; i < indent; i++) {
			sb.append(" ");
		}
		String indentStr = sb.toString();
		sb = new StringBuffer();
		
		/* print InvokeServerInfo */
		/* type */
		sb.append(indentStr + "- " + KEY_TYPE + ": " + type + "\n");
		/* path */
		if (path != null) {
			sb.append(indentStr + "- " + KEY_PATH + ": " + path + "\n");
		}
		/* log_filePath */
		if (log_filePath != null) {
			sb.append(indentStr + "- " + KEY_LOG_FILEPATH + ": " + log_filePath + "\n");
		}
		/* max_jobs */
		sb.append(indentStr + "- " + KEY_MAXJOBS + ": " + maxJobs + "\n");
		/* options */
		if (options != null) {
			for (int i = 0; i < options.size(); i++) {
				sb.append(indentStr + "- " + KEY_OPTION + ": " + options.get(i) + "\n");
			}
		}
		
		return sb.toString();
	}
}
