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
 * $RCSfile: RemoteMachineInfo.java,v $ $Revision: 1.46 $ $Date: 2006/08/04 11:02:08 $
 */
package org.apgrid.grpc.ng.info;

import java.util.Enumeration;
import java.util.List;
import java.util.Properties;
import java.util.Vector;

import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgGlobals;
import org.gridforum.gridrpc.GrpcException;

public class RemoteMachineInfo implements Cloneable {
	private Properties mapRemoteMachineInformation;
	private Properties mapClassPath;
	private String hostDN;
	
	/* keys for RemoteMachineInformation */
	public static final String KEY_HOSTNAME = "hostname";
	public static final String KEY_TAG = "tag";
	public static final String KEY_PORT = "port";
	public static final String KEY_MDS_HOSTNAME = "mds_hostname";
	public static final String KEY_MDS_TAG = "mds_tag";
	public static final String KEY_MPI_RUNCOMMAND = "mpi_runCommand";
	public static final String KEY_MPI_NCPUS = "mpi_runNoOfCPUs";
	public static final String KEY_GASS_SCHEME = "gass_scheme";
	public static final String KEY_CRYPT = "crypt";
	public static final String KEY_PROTOCOL = "protocol";
	public static final String KEY_FORCEXDR= "force_xdr";
	public static final String KEY_JOBMANAGER= "jobmanager";
	public static final String KEY_SUBJECT = "subject";
	public static final String KEY_QUEUE = "job_queue";
	public static final String KEY_PROJECT = "job_project";
	public static final String KEY_HOSTCOUNT = "job_hostCount";
	public static final String KEY_MINMEMORY = "job_minMemory";
	public static final String KEY_MAXMEMORY = "job_maxMemory";
	public static final String KEY_MAXTIME = "job_maxTime";
	public static final String KEY_MAXWALLTIME = "job_maxWallTime";
	public static final String KEY_MAXCPUTIME = "job_maxCpuTime";
	public static final String KEY_JOB_STARTTIMEOUT = "job_startTimeout";
	public static final String KEY_JOB_STOPTIMEOUT = "job_stopTimeout";
	public static final String KEY_HEARTBEAT = "heartbeat";
	public static final String KEY_HEARTBEAT_TIMEOUTCOUNT
									= "heartbeat_timeoutCount";
	public static final String KEY_HEARTBEAT_TIMEOUTCOUNT_ONTRANSFER
									= "heartbeat_timeoutCountOnTransfer";
	public static final String KEY_REDIRECT_OUTERR = "redirect_outerr";
	public static final String KEY_ARG_TRANS = "argument_transfer";
	public static final String KEY_COMPRESS = "compress";
	public static final String KEY_COMPRESS_THRESHOLD = "compress_threshold";
	public static final String KEY_COMMLOG_ENABLE = "commLog_enable";
	public static final String KEY_COMMLOG_FILEPATH = "commLog_filePath";
	public static final String KEY_COMMLOG_SUFFIX = "commLog_suffix";
	public static final String KEY_COMMLOG_NFILES = "commLog_nFiles";
	public static final String KEY_COMMLOG_MAXFILESIZE = "commLog_maxFileSize";
	public static final String KEY_COMMLOG_OVERWRITEDIR
									= "commLog_overwriteDirectory";
	public static final String KEY_WORK_DIR = "workDirectory";
	public static final String KEY_CORE_SIZE = "coreDumpSize";
	public static final String KEY_DEBUG = "debug";
	public static final String KEY_DEBUG_DISPLAY = "debug_display";
	public static final String KEY_DEBUG_TERM = "debug_terminal";
	public static final String KEY_DEBUG_DEBUGGER = "debug_debugger";
	public static final String KEY_DEBUG_BUSYLOOP = "debug_busyLoop";
	public static final String KEY_ENVIRONMENT = "environment";
	public static final String KEY_TCP_NODELAY = "tcp_nodelay";
	public static final String KEY_BLOCK_SIZE = "argument_blockSize";
	public static final String KEY_TCP_CONNECT_RETRY_COUNT = "tcp_connect_retryCount";
	public static final String KEY_TCP_CONNECT_RETRY_BASEINTERVAL
									= "tcp_connect_retryBaseInterval";
	public static final String KEY_TCP_CONNECT_RETRY_INCREASERATIO
									= "tcp_connect_retryIncreaseRatio";
	public static final String KEY_TCP_CONNECT_RETRY_RANDOM
									= "tcp_connect_retryRandom";
	public static final String KEY_INVOKE_SERVER = "invoke_server";
	public static final String KEY_INVOKE_SERVER_OPTION = "invoke_server_option";
	public static final String KEY_CLIENT_HOSTNAME = "client_hostname";
	public static final String KEY_JOB_RSL_EXTENSION = "job_rslExtensions";
	
	/* value */
	public static final String VAL_PROTO_XML = "XML";
	public static final String VAL_PROTO_BIN = "binary";
	
	public static final String VAL_BACKEND_NORMAL = "normal";
	public static final String VAL_BACKEND_MPI = "mpi";
	public static final String VAL_BACKEND_BLACS = "blacs";
	
	public static final String VAL_COMPRESS_RAW = "raw";
	public static final String VAL_COMPRESS_ZLIB = "zlib";
	
	public static final String VAL_INVOKE_SERVER_NONE = "none";
	
	/**
	 * 
	 */
	public RemoteMachineInfo() throws GrpcException {
		mapRemoteMachineInformation = new Properties();
		setDefaultServerParameter(mapRemoteMachineInformation);
		mapClassPath = new Properties();
	}
	
	/**
	 * @param hostName
	 * @throws GrpcException
	 */
	public RemoteMachineInfo(String hostName) throws GrpcException {
		mapRemoteMachineInformation = new Properties();
		setDefaultServerParameter(mapRemoteMachineInformation);
		mapRemoteMachineInformation.put(KEY_HOSTNAME, hostName);
		mapClassPath = new Properties();
	}
	
	/**
	 * @param prop
	 */
	public RemoteMachineInfo(Properties prop) throws GrpcException {
		mapRemoteMachineInformation = new Properties();
		setDefaultServerParameter(mapRemoteMachineInformation);
		overwriteParameter(mapRemoteMachineInformation, prop);
		mapClassPath = new Properties();
	}
	
	/**
	 * @param key
	 * @return
	 */
	public Object get(String key) {
		return mapRemoteMachineInformation.get(key);
	}
	
	/**
	 * @param key
	 * @param value
	 */
	public void put(String key, Object value) {
		mapRemoteMachineInformation.put(key, value);
	}
	
	/**
	 * @return
	 */
	public String getHostDN() {
		return hostDN;
	}
	
	/**
	 * @param hostDN
	 */
	public void setHostDN(String hostDN) {
		this.hostDN = hostDN;
	}

	/**
	 * @param className
	 * @return
	 */
	public int getNumCPUs(String className) {
		RemoteClassPathInfo remoteClassPathInfo;

		if ((remoteClassPathInfo =
			(RemoteClassPathInfo) mapClassPath.get(className))
			!= null) {
			if (remoteClassPathInfo.contains(RemoteClassPathInfo.KEY_CLASS_PATH_NCPUS) == true) {
				return Integer.parseInt(
					(String) remoteClassPathInfo.get(
					RemoteClassPathInfo.KEY_CLASS_PATH_NCPUS));
			}
		}
		
		return Integer.parseInt(
			(String)mapRemoteMachineInformation.get(KEY_MPI_NCPUS));
	}
	
	/**
	 * @param className
	 * @return
	 */
	public RemoteClassPathInfo getRemoteClassPath(String className) {
		return (RemoteClassPathInfo) mapClassPath.get(className);
	}
	
	/**
	 * @param className
	 * @param classPathInfo
	 */
	public void putRemoteClassPath(String className, RemoteClassPathInfo classPathInfo) {
		RemoteClassPathInfo rcpInfo =
			(RemoteClassPathInfo) mapClassPath.get(className);
		if (rcpInfo == null) {
			mapClassPath.put(className, classPathInfo);
		} else {
			rcpInfo.overwriteParam(classPathInfo);
			mapClassPath.put(className, rcpInfo);
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	private void setDefaultServerParameter(Properties tmpProperties) throws GrpcException {
		/* set default values */
		tmpProperties.put(KEY_GASS_SCHEME, "http");
		tmpProperties.put(KEY_CRYPT, "none");
		//tmpProperties.put(KEY_PROTOCOL, VAL_PROTO_XML);
		tmpProperties.put(KEY_PROTOCOL, VAL_PROTO_BIN);
		tmpProperties.put(KEY_FORCEXDR, "false");
		tmpProperties.put(KEY_JOB_STARTTIMEOUT, "0");
		tmpProperties.put(KEY_JOB_STOPTIMEOUT, "-1");
		tmpProperties.put(KEY_HEARTBEAT, "60");
		tmpProperties.put(KEY_HEARTBEAT_TIMEOUTCOUNT, "5");
		tmpProperties.put(KEY_HEARTBEAT_TIMEOUTCOUNT_ONTRANSFER, "-1");
		tmpProperties.put(KEY_REDIRECT_OUTERR, "true");
		tmpProperties.put(KEY_ARG_TRANS, "wait");
		tmpProperties.put(KEY_COMPRESS, VAL_COMPRESS_RAW);
		tmpProperties.put(KEY_COMPRESS_THRESHOLD, "65536");
		tmpProperties.put(KEY_COMMLOG_ENABLE, "false");
		tmpProperties.put(KEY_COMMLOG_NFILES, "1");
		tmpProperties.put(KEY_COMMLOG_OVERWRITEDIR, "false");
		tmpProperties.put(KEY_DEBUG, "false");
		tmpProperties.put(KEY_TCP_NODELAY, "false");
		tmpProperties.put(KEY_BLOCK_SIZE, "16000");
		tmpProperties.put(KEY_TCP_CONNECT_RETRY_COUNT, "4");
		tmpProperties.put(KEY_TCP_CONNECT_RETRY_BASEINTERVAL, "1");
		tmpProperties.put(KEY_TCP_CONNECT_RETRY_INCREASERATIO, "2.0");
		tmpProperties.put(KEY_TCP_CONNECT_RETRY_RANDOM, "true");
		tmpProperties.put(KEY_INVOKE_SERVER, VAL_INVOKE_SERVER_NONE);
	}

	/**
	 * @param prop
	 */
	public void overwriteParameter(Properties prop) {
		overwriteParameter(mapRemoteMachineInformation, prop);
	}

	/**
	 * @param mapRemoteMachineInformation
	 * @param prop
	 */
	private void overwriteParameter(
		Properties mapRemoteMachineInformation, Properties prop) {
		/* copy all of values into specified Properties */
		Enumeration keys = prop.keys();
		while (keys.hasMoreElements()) {
			String keyString = (String) keys.nextElement();
			/* check if it's environment variable */
			if ((keyString.equals(KEY_ENVIRONMENT)) &&
				(prop.containsKey(keyString))) {
				/* copy all of environment variables */
				List listEnvironment =
					(List) mapRemoteMachineInformation.get(keyString);
				if (listEnvironment == null) {
					listEnvironment = new Vector();
				}
				List listNewEnvironment = (List) prop.get(keyString);

				if ((listEnvironment != null) &&
					(listEnvironment != listNewEnvironment)) {
					for (int i = 0; i < listNewEnvironment.size(); i++) {
						/* get key string */
						String envNewVal = (String) listNewEnvironment.get(i);
						String envNewKey = envNewVal.split("=")[0];
						for (int j = 0; j < listEnvironment.size(); j++) {
							String envVal = (String) listEnvironment.get(j);
							String envKey = envVal.split("=")[0];
							if (envNewKey.equals(envKey)) {
								/* remove the entry before add it */
								listEnvironment.remove(j);
							}
						}
						/* insert to the list */
						listEnvironment.add(envNewVal);
					}
				}
				/* set new map of Environment variable */
				mapRemoteMachineInformation.put(keyString, listEnvironment);
			} else if (keyString.equals(KEY_MPI_NCPUS)) {
				/* get numCPUs */
				Properties propNewNumOfCPUs = (Properties) prop.get(keyString);
				/* overwrite all of numCPUs */
				setNumCPUs(propNewNumOfCPUs);

				/* check and set numCPUs for this machine */
				if (propNewNumOfCPUs.containsKey("")) {
					mapRemoteMachineInformation.put(KEY_MPI_NCPUS,
						propNewNumOfCPUs.get(""));
				}
			} else if ((keyString.equals(KEY_INVOKE_SERVER_OPTION)) &&
				(prop.containsKey(keyString))) {
				/* get the list of options */
				List listOptions = (List) mapRemoteMachineInformation.get(keyString);
				if (listOptions == null) {
					listOptions = new Vector();
				}
				List listNewOptions = (List) prop.get(keyString);
				
				/* check if they are same list */
				if (listOptions == listNewOptions) {
					continue;
				}
				
				/* copy all of variables */
				for (int i = 0; i < listNewOptions.size(); i++) {
					boolean hasSameEntry = false;
					for (int j = 0; j < listOptions.size(); j++) {
						if (listNewOptions.get(i).equals(listOptions.get(j))) {
							hasSameEntry =true;
						}
					}
					
					if (hasSameEntry == false) {
						listOptions.add(listNewOptions.get(i));
					}
				}

				/* set new map of options */
				mapRemoteMachineInformation.put(keyString, listOptions);
			} else if (keyString.equals(KEY_HOSTNAME)){
				/* nothing will be done */
			} else {
				mapRemoteMachineInformation.put(keyString, prop.get(keyString));
			}
		}
	}

	/**
	 * @param prop
	 */
	private void setNumCPUs(Properties prop) {
		/* get keys */
		Enumeration keys = prop.keys();

		/* register all elements of numCPUs */
		if (keys != null) {
			while (keys.hasMoreElements()) {
				/* get name of Class */
				String targetClass = (String) keys.nextElement();
				/* get number of CPUs */
				String numOfCPUs = (String) prop.get(targetClass);
			
				/* check if it's for default of this machine */
				if (targetClass.equals("")) {
					continue;
				}

				/* get RemoteClassPathInfo */
				RemoteClassPathInfo rcpInfo = 
					getRemoteClassPath(targetClass);
				if (rcpInfo == null) {
					rcpInfo = new RemoteClassPathInfo();
				}
				/* put information of RemoteClassPathInfo */
				rcpInfo.put(RemoteClassPathInfo.KEY_CLASS_PATH_HOSTNAME,
					get(KEY_HOSTNAME));
				rcpInfo.put(RemoteClassPathInfo.KEY_CLASS_PATH_CLASSNAME,
					targetClass);
				rcpInfo.put(RemoteClassPathInfo.KEY_CLASS_PATH_NCPUS,
					numOfCPUs);
				/* put RemoteClassPathInfo into RemoteMachineInfo */
				putRemoteClassPath(targetClass, rcpInfo);
			}
		}
	}
	
	/**
	 * 
	 */
	public void resetNumCPUs() throws GrpcException {
		/* get keys */
		Enumeration keys = mapClassPath.keys();

		/* register all elements of numCPUs */
		if (keys != null) {
			while (keys.hasMoreElements()) {
				/* get name of Class */
				String targetClass = (String) keys.nextElement();
				
				/* get RemoteClassPathInfo */
				RemoteClassPathInfo rcpInfo = 
					getRemoteClassPath(targetClass);
				if (rcpInfo == null) {
					throw new NgException("Failed to find RemoteClassPathInfo.");
				}
				
				/* remove num of CPUs */
				rcpInfo.remove(RemoteClassPathInfo.KEY_CLASS_PATH_NCPUS);
			}
		}
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
		
		/* RemoteMachineInfo */
		Enumeration keys = mapRemoteMachineInformation.keys();
		while (keys.hasMoreElements()) {
			String key = (String) keys.nextElement();
			sb.append(indentStr + "- " + key + " : " +
				mapRemoteMachineInformation.get(key));
			sb.append("\n");
		}		
		sb.append(indentStr + "+ hostDN : " + hostDN);
		sb.append("\n");
		
		/* RemoteClassPath */
		keys = mapClassPath.keys();
		while (keys.hasMoreElements()) {
			String key = (String) keys.nextElement();
			sb.append(indentStr + "+ RemoteClassPathInfo[" + key + "]");
			sb.append("\n");

			RemoteClassPathInfo rcpi = (RemoteClassPathInfo) mapClassPath.get(key);
			sb.append(rcpi.toString(indent + NgGlobals.debugIndent));
		}
		
		return sb.toString();
	}
	
	/**
	 * @return
	 */
	public RemoteMachineInfo getCopy() {
		try {
			RemoteMachineInfo tmpRemoteMachineInfo = (RemoteMachineInfo) this.clone();
			tmpRemoteMachineInfo.copyRemoteClassPath();
			return tmpRemoteMachineInfo;
		} catch (CloneNotSupportedException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * 
	 */
	private void copyRemoteClassPath() {
		/* RemoteClassPath */
		Enumeration keys = mapClassPath.keys();
		while (keys.hasMoreElements()) {
			String key = (String) keys.nextElement();
			RemoteClassPathInfo rcpi = (RemoteClassPathInfo) mapClassPath.get(key);
			RemoteClassPathInfo tmpRcpi = rcpi.getCopy();
			mapClassPath.put(key, tmpRcpi);
		}
	}

	/**
	 * @throws GrpcException
	 * 
	 */
	public void reset() throws GrpcException {
		/* reset hostDN */
		hostDN = null;
		/* reset map */
		String hostname = (String) mapRemoteMachineInformation.get(KEY_HOSTNAME);
		String tag = (String) mapRemoteMachineInformation.get(KEY_TAG);
		this.mapRemoteMachineInformation = new Properties();
		setDefaultServerParameter(this.mapRemoteMachineInformation);
		this.mapRemoteMachineInformation.put(KEY_HOSTNAME, hostname);
		if (tag != null) {
			this.mapRemoteMachineInformation.put(KEY_TAG, tag);
		}
	}
	
	/**
	 * @return
	 */
	public String makeHandleString() {
		String hostname = (String) mapRemoteMachineInformation.get(KEY_HOSTNAME);
		String tag = (String) mapRemoteMachineInformation.get(KEY_TAG);
		
		if (tag != null) {
			return hostname + "/" + tag;
		} else {
			return hostname + "/";
		}
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
}
