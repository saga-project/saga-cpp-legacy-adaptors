/*
 * $RCSfile: InvokeServerInfo.java,v $ $Revision: 1.6 $ $Date: 2007/09/26 04:14:08 $
 * $AIST_Release: 5.0.0 $
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
 */
package org.apgrid.grpc.ng.info;

import java.util.Enumeration;
import java.util.List;
import java.util.Properties;
import java.util.Map;

import org.apgrid.grpc.ng.NgException;
import org.gridforum.gridrpc.GrpcException;

public class InvokeServerInfo {
	// keys for Invoke Server Information 
	public static final String KEY_TYPE = "type";
	public static final String KEY_PATH = "path";
	public static final String KEY_LOG_FILEPATH = "log_filePath";
	public static final String KEY_MAXJOBS = "max_jobs";
	public static final String KEY_OPTION = "option";
	public static final String KEY_STATUS_POLLING = "status_polling";

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
	public InvokeServerInfo(Properties param) throws GrpcException {
		// check if it contains key parameter 
		if (! param.containsKey(KEY_TYPE)) {
			throw new NgException("key [" + KEY_TYPE 
						+ "] is not in InvokeServerInfo.");
		}

		// set parameters from Properties 
		for (Map.Entry<Object, Object>  entry :
			param.entrySet() ) {
			String key = (String) entry.getKey();
			Object value = entry.getValue();
			if (key.equals(KEY_TYPE)) {
				type = (String) value;
			} else if (key.equals(KEY_PATH)) {
				path = (String) value;
			} else if (key.equals(KEY_LOG_FILEPATH)) {
				log_filePath = (String) value;
			} else if (key.equals(KEY_MAXJOBS)) {
				maxJobs = Integer.parseInt((String)value);
			} else if (key.equals(KEY_OPTION)) {
				Object obj = param.get(key);
				options = (List) param.get(key);
			} else if (key.equals(KEY_STATUS_POLLING)) {
				// interval of status polling 
				statusPolling = Integer.parseInt((String)value);
			} else {
				// unsupported key 
				throw new NgException("Invalid key[ " + key
							+	"] for InvokeServerInfo.");
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
		char [] cbuf = new char[indent];
		for (int i = 0 ; i < cbuf.length; i++) {
			cbuf[i] = ' ';
		}
		String indentStr = new String(cbuf);
		StringBuilder sb = new StringBuilder();
		// print InvokeServerInfo 
		sb.append(indentStr).append("- ").append(KEY_TYPE)
			.append(": ").append(type).append("\n");
		if (path != null) {
			sb.append(indentStr).append("- ").append(KEY_PATH)
				.append(": ").append(path).append("\n");
		}
		if (log_filePath != null) {
			sb.append(indentStr).append("- ").append(KEY_LOG_FILEPATH)
				.append(": ").append(log_filePath).append("\n");
		}
		sb.append(indentStr).append("- ").append(KEY_MAXJOBS).append(": ")
			.append(maxJobs).append("\n");
		if (options != null) {
			for (int i = 0; i < options.size(); i++) {
				sb.append(indentStr).append("- ").append(KEY_OPTION)
					.append(": ").append(options.get(i)).append("\n");
			}
		}
		return sb.toString();
	}
}
