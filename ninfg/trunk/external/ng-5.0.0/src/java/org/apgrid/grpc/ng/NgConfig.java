/*
 * $RCSfile: NgConfig.java,v $ $Revision: 1.11 $ $Date: 2008/03/11 06:30:11 $
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

import java.io.IOException;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Properties;
import java.util.Vector;
import java.io.File;

import org.apgrid.grpc.ng.config.ConfigParser;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.ENVIRONMENT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.INVOKE_SERVER_OPTION;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMUNICATION_PROXY_OPTION;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.JOB_RSL_EXTENSION;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.MPI_NCPUS;

/**
 * Representation of Ninf-G Client Configuration
 */
public class NgConfig {

	private List<Properties> serverInfo   = new ArrayList<Properties>();
	private List<Properties> functionList = new ArrayList<Properties>();
	private List<String> listFiles = new ArrayList<String>();
	private List<Properties> invokeServerInfo = new ArrayList<Properties>();
	private Properties serverDefault = new Properties();
	private Properties localHostInfo_old = new Properties();
	private LocalMachineInfo localHostInfo = null;

	// configurations of <INFORMATION_SOURCE>
	private List<NgConfigSection> infoSource = new ArrayList<NgConfigSection>();

	// configurations of <CLIENT_COMMUNICATION_PROXY>
	private List<NgConfigSection> clientCommProxy = new ArrayList<NgConfigSection>();

	//private static String envLogLevel = "ninfg.logLevel";

    private static final String TAG_INCLUDE    = "INCLUDE";
    private static final String TAG_CLIENT     = "CLIENT";
    private static final String TAG_FUNCTION_INFO = "FUNCTION_INFO";
    private static final String TAG_SERVER     = "SERVER";
    private static final String TAG_SERVER_DEFAULT = "SERVER_DEFAULT";
    private static final String TAG_INVOKE_SERVER  = "INVOKE_SERVER";
	private static final String TAG_INFORMATION_SOURCE = "INFORMATION_SOURCE";
    private static final String TAG_CLIENT_COMMUNICATION_PROXY = "CLIENT_COMMUNICATION_PROXY";

	/**
	 * @param configFile
	 * @throws GrpcException
	 */
	public NgConfig(String configFile) throws GrpcException {
		if (configFile == null) {
			throw new NullPointerException("configFile is null");
		}
		readConfigFile(configFile);

		// newly added
		setupLocalMachineInfo();
	}

	private void readConfigFile(String configFile) throws GrpcException {
		if (listFiles.contains(configFile)) {
			throw new GrpcException(configFile + ": Reading the same file again");
		}
		listFiles.add(configFile);
		NgConfigParser parser = new ConfigParser();
		List<NgConfigSection> config;
		try {
			config = parser.parse(new File(configFile));
		} catch (IOException e) {
			throw new GrpcException(e.getMessage());
		} catch (NgConfigException e) {
			throw new GrpcException(e);
		}
		setData(config);
	}

	/*
	 * @deprecated
	 */
	public NgConfig(Properties prop) throws GrpcException {
		serverInfo.add(prop);
	}

	// pseudo
	private void setData(List<NgConfigSection> config) throws GrpcException {
		for (NgConfigSection section : config) {
			String name = section.getName();
			if (name.equals(TAG_CLIENT)) {
				addLocalHostInfo(section);
			} else if (name.equals(TAG_SERVER)) {
				addServerInfo(section);
			} else if (name.equals(TAG_INFORMATION_SOURCE)) {
				infoSource.add(section);
			} else if (name.equals(TAG_FUNCTION_INFO)) {
				addFunctionInfo(section);
			} else if (name.equals(TAG_INVOKE_SERVER)) {
				addInvokeServerInfo(section);
			} else if (name.equals(TAG_SERVER_DEFAULT)) {
				addServerDefaultInfo(section);
			} else if (name.equals(TAG_CLIENT_COMMUNICATION_PROXY)) {
				clientCommProxy.add(section);
			} else if (name.equals(TAG_INCLUDE)) {
				includeFile(section);
			}
		}
	}

///// Converters begin

	private Properties convertSectionToProp(NgConfigSection sect) {
		Properties prop = new Properties();
		for (NgConfigEntry ent : sect) {
			prop.setProperty(ent.getKey(), ent.getValue());
		}
		return prop;
	}

	private Properties convertServerSectionToProp(NgConfigSection sect) {
		Properties result = new Properties();
		Map environment = new HashMap();
		List invoke_server_option = new ArrayList();
                List communication_proxy_option = new ArrayList();
		List job_rslExtensions = new ArrayList();
		List<String> mpi_ncpus = new ArrayList<String>();
		for (NgConfigEntry ent : sect) {
			if (ent.getKey().equals(ENVIRONMENT)) {
				int i = ent.getValue().indexOf('=');
				String key, val;
				if (i < 0) {
					key = ent.getValue();
					val = "";
				} else {
					key = ent.getValue().substring(0,i);
					val = ent.getValue().substring(i+1);
				}
				environment.put(key, val);
			} else if (ent.getKey().equals(INVOKE_SERVER_OPTION)) {
				invoke_server_option.add(ent.getValue());
			} else if (ent.getKey().equals(COMMUNICATION_PROXY_OPTION)) {
				communication_proxy_option.add(ent.getValue());
			} else if (ent.getKey().equals(JOB_RSL_EXTENSION)) {
				job_rslExtensions.add(ent.getValue());
			} else if (ent.getKey().equals(MPI_NCPUS)) {
				mpi_ncpus.add(ent.getValue());
			} else {
				result.setProperty(ent.getKey(), ent.getValue());
			}
		}

		if (! environment.isEmpty()) {
			result.put(ENVIRONMENT, environment);
		}
		if (! invoke_server_option.isEmpty()) {
			result.put(INVOKE_SERVER_OPTION, invoke_server_option);
		}
                if (! communication_proxy_option.isEmpty()) {
                        result.put(COMMUNICATION_PROXY_OPTION, communication_proxy_option);
                }
		if (! job_rslExtensions.isEmpty() ) {
			result.put(JOB_RSL_EXTENSION, job_rslExtensions);
		}
		if (! mpi_ncpus.isEmpty()) {
			result.put(MPI_NCPUS, mpi_ncpus);
		}
		return result;
	}

	private Properties convertInvokeServerSectionToProp(NgConfigSection sect) {
		Properties result = new Properties();
		List invoke_server_option = new ArrayList();
		for (NgConfigEntry ent : sect) {
			if (ent.getKey().equals("option")) {
				invoke_server_option.add(ent.getValue());
			}
			result.setProperty(ent.getKey(), ent.getValue());
		}
		if (! invoke_server_option.isEmpty()) {
			result.put("option", invoke_server_option);
		}
		return result;
	}

	private void addLocalHostInfo(NgConfigSection sect) {
		for (NgConfigEntry ent : sect) {
			localHostInfo_old.setProperty(ent.getKey(), ent.getValue());
		}
	}

	private void addServerInfo(NgConfigSection sect) {
		serverInfo.add(convertServerSectionToProp(sect));
	}

	private void addFunctionInfo(NgConfigSection sect) {
		functionList.add(convertSectionToProp(sect));
	}

	private void addInvokeServerInfo(NgConfigSection sect) {
		invokeServerInfo.add(convertInvokeServerSectionToProp(sect)); 
	}

	private void addServerDefaultInfo(NgConfigSection sect) {
		serverDefault = convertServerSectionToProp(sect);
	}

	private void includeFile(NgConfigSection sect) throws GrpcException {
		for (NgConfigEntry ent : sect) {
			readConfigFile(ent.getValue());
		}
	}


///// Converters end

	private void setupLocalMachineInfo() {
		this.localHostInfo = new LocalMachineInfo();

		// set log_maxFileSize
		String nFiles = localHostInfo_old.getProperty(
			LocalMachineInfo.Keys.log_nFiles.toString());
		if ((nFiles != null) && (Integer.parseInt(nFiles) > 1)) {
			localHostInfo.put(LocalMachineInfo.Keys.log_maxFileSize,
				"1048576"); // 1Mega
		}

		String _loglevel =
			getOverallLogLevel(localHostInfo_old.getProperty("loglevel"));
		if (_loglevel != null) {
        	localHostInfo.put(LocalMachineInfo.Keys.loglevel_globusToolkit, _loglevel);
        	localHostInfo.put(LocalMachineInfo.Keys.loglevel_ninfgProtocol, _loglevel);
        	localHostInfo.put(LocalMachineInfo.Keys.loglevel_ninfgInternal, _loglevel);
        	localHostInfo.put(LocalMachineInfo.Keys.loglevel_ninfgGrpc, _loglevel);
		}

		java.util.Set<Map.Entry<Object, Object>> entries =
			localHostInfo_old.entrySet();
		for (Map.Entry<Object, Object> ent : entries) {
			localHostInfo.put((String)ent.getKey(), (String)ent.getValue());
		}
	}

	private String getOverallLogLevel(String level) {
		if (level != null) return level;
		return System.getenv("NG_LOG_LEVEL");
	}



	// Getters
	public List<NgConfigSection> getInformationSource() {
		return infoSource;
	}

	public Properties getLocalMachineInfo() {
		return localHostInfo_old;
	}
	// newly added
	public LocalMachineInfo getLocalMachineInfo2() {
		return localHostInfo;
	}

	public List<Properties> getServerInfo() {
		return serverInfo;
	}

	public List<Properties> getFunctionInfo() {
		return functionList;
	}

	public List<Properties> getInvokeServerInfo() {
		return invokeServerInfo;
	}

    public Properties getServerDefault() {
        return serverDefault;
    }

	public List<NgConfigSection> getClientCommProxyInfo() { 
		return clientCommProxy;
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		if (serverInfo != null) {
			sb.append("serverInfo is ").append(serverInfo).append("\n");
		}
		if (localHostInfo_old != null) {
			sb.append("localHostInfo is ").append(localHostInfo_old).append("\n");
		}
		if (functionList != null) {
			sb.append("functionList is ").append(functionList).append("\n");
		}
		if (serverDefault != null) {
			sb.append("serverDefault is ").append(serverDefault).append("\n");
		}
		return sb.toString();
	}
}
