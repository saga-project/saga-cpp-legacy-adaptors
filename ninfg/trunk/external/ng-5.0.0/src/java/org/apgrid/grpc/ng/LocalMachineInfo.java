/*
 * $RCSfile: LocalMachineInfo.java,v $ $Revision: 1.7 $ $Date: 2008/03/25 05:39:07 $
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
package org.apgrid.grpc.ng;

import java.util.Set;
import java.util.Map;
import java.util.HashMap;
import org.apgrid.grpc.util.NgUtil;

class LocalMachineInfo {

	private Map<String, String> info = null;
	private static final Map<String, String> DEFAULT;
	private static final String DEFAULT_LOGLEVEL = "2"; // level Error

	// keys for LocalMachineInformation
	public static enum Keys {
		hostname,
		save_sessionInfo,
		loglevel,
		loglevel_globusToolkit,
		loglevel_ninfgProtocol,
		loglevel_ninfgInternal,
		loglevel_ninfgGrpc,
		log_filePath,
		log_suffix,
		log_nFiles,
		log_maxFileSize,
		log_overwriteDirectory,
		tmp_dir,
		refresh_credential,
		invoke_server_log,
		fortran_compatible,
		handling_signals,
		listen_port,
		listen_port_authonly,
		listen_port_GSI,
		listen_port_SSL,
		information_service_log,
		client_communication_proxy_log,
	}

	/*
	 * LocalMachineInfo Constructor
	 */
	public LocalMachineInfo() {
		this.info = new HashMap<String, String>(DEFAULT);
	}

	private String _get(Keys key) {
		return info.get(key.toString());
	}

	public String getHostname() {
		return _get(Keys.hostname);
	} 
	public String getListenPort() {
		return _get(Keys.listen_port);
	} 
	public String getLoglevelGlobusToolkit() {
		return _get(Keys.loglevel_globusToolkit);
	} 
	public String getLoglevelNinfgProtocol() {
		return _get(Keys.loglevel_ninfgProtocol);
	} 
	public String getLoglevelNinfgInternal() {
		return _get(Keys.loglevel_ninfgInternal);
	} 
	public String getLoglevelNinfgGrpc() {
		return _get(Keys.loglevel_ninfgGrpc);
	}
	public String getLogFilePath() {
		return _get(Keys.log_filePath);
	} 
	public String getLogSuffix() {
		return _get(Keys.log_suffix);
	}
	public String getLogNFiles() {
		return _get(Keys.log_nFiles);
	}
	public String getLogMaxFileSize() {
		return _get(Keys.log_maxFileSize);
	}
	public String getLogOverwriteDirectory() {
		return _get(Keys.log_overwriteDirectory);
	}
	public String getTmpDir() {
		return _get(Keys.tmp_dir);
	}
	public String getRefreshCredential() {
		return _get(Keys.refresh_credential);
	}
	public String getInvokeServerLog() {
		return _get(Keys.invoke_server_log);
	}
	public String getInformationServiceLog() {
		return _get(Keys.information_service_log);
	}
	public String getClientCommunicationProxyLog() {
		return _get(Keys.client_communication_proxy_log);
	} 

	public void put(LocalMachineInfo.Keys key, String val) {
		info.put(key.toString(), val);
	}
	public void put(String key, String val) {
		info.put(key, val);
	}

	public void putAll(Map<String, String> anMap) {
		info.putAll(anMap);
	}

	public Set<Map.Entry<String, String>> entrySet() {
		return info.entrySet();
	}

	public String toString() {
		return info.toString();
	}


	public static Map<String, String> getDefaultParameter() {
		return DEFAULT;
	}
	static {
		DEFAULT = new HashMap<String, String>();
		DEFAULT.put(Keys.hostname.toString(),      NgUtil.getLocalHostName());
		DEFAULT.put(Keys.save_sessionInfo.toString(),                  "256");
		DEFAULT.put(Keys.loglevel.toString(),               DEFAULT_LOGLEVEL);
		DEFAULT.put(Keys.loglevel_globusToolkit.toString(), DEFAULT_LOGLEVEL);
		DEFAULT.put(Keys.loglevel_ninfgProtocol.toString(), DEFAULT_LOGLEVEL);
		DEFAULT.put(Keys.loglevel_ninfgInternal.toString(), DEFAULT_LOGLEVEL);
		DEFAULT.put(Keys.loglevel_ninfgGrpc.toString(),     DEFAULT_LOGLEVEL);
		DEFAULT.put(Keys.log_nFiles.toString(),                          "1");
		DEFAULT.put(Keys.log_overwriteDirectory.toString(),          "false");
		DEFAULT.put(Keys.tmp_dir.toString(),                          "/tmp");
		DEFAULT.put(Keys.refresh_credential.toString(),                  "0");
		DEFAULT.put(Keys.fortran_compatible.toString(),              "false");
		DEFAULT.put(Keys.listen_port.toString(),                         "0");
		DEFAULT.put(Keys.listen_port_authonly.toString(),                "0");
		DEFAULT.put(Keys.listen_port_GSI.toString(),                     "0");
		DEFAULT.put(Keys.listen_port_SSL.toString(),                     "0");
		// please set DefaultParameter[Keys.log_maxFileSize] to client class.
		// 1M or unlimited
	}
}

