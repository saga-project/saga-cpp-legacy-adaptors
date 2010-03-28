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
 * $RCSfile: NgInformationManager.java,v $ $Revision: 1.59 $ $Date: 2006/09/12 08:26:29 $
 */
package org.apgrid.grpc.ng;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apgrid.grpc.ng.info.InvokeServerInfo;
import org.apgrid.grpc.ng.info.MDSInfo;
import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.globus.common.CoGProperties;
import org.globus.util.Util;
import org.gridforum.gridrpc.GrpcException;

class NgInformationManager {
	private NgGrpcClient context;
	private NgConfig config;
	private Map mapRemoteMachineInfoCache = new Properties();
	private Map mapRemoteClassInfoCache = new Properties();
	private Map mapMDSInfo = new Properties();
	private Map mapInvokeServerInfo = new Properties();
	private Properties propLocalMachineInfo;
	private List listServerInfo;
	private NgMDS ngMDS = null;
	private List listMDSInfo = new ArrayList();
	private boolean isLocked;
	
	/* keys for LocalMachineInformation */
	public static final String KEY_CLIENT_HOSTNAME = "hostname";
	public static final String KEY_CLIENT_SAVE_SESSIONINFO =
		"save_sessionInfo";
	public static final String KEY_CLIENT_LOGLEVEL = "loglevel";
	public static final String KEY_CLIENT_LOGLEVEL_GT =
		"loglevel_globusToolkit";
	public static final String KEY_CLIENT_LOGLEVEL_NGPROT =
		"loglevel_ninfgProtocol";
	public static final String KEY_CLIENT_LOGLEVEL_NGINT =
		"loglevel_ninfgInternal";
	public static final String KEY_CLIENT_LOGLEVEL_NGGRPC =
		"loglevel_ninfgGrpc";
	public static final String KEY_CLIENT_LOG_FILEPATH = "log_filePath";
	public static final String KEY_CLIENT_LOG_SUFFIX = "log_suffix";
	public static final String KEY_CLIENT_LOG_NFILES = "log_nFiles";
	public static final String KEY_CLIENT_LOG_MAXFILESIZE =
		"log_maxFileSize";
	public static final String KEY_CLIENT_LOG_OVERWRITEDIR =
		"log_overwriteDirectory";
	public static final String KEY_CLIENT_REFRESH_CREDENTIAL =
		"refresh_credential";
	public static final String KEY_CLIENT_INVOKE_SERVER_LOG =
		"invoke_server_log";
	public static final String KEY_CLIENT_TMP_DIR =	"tmp_dir";
	public static final String KEY_CLIENT_FORTRAN_COMPATIBLE  =
		"fortran_compatible";
	public static final String KEY_CLIENT_HANDLING_SIGNALS  =
		"handling_signals";
	public static final String KEY_CLIENT_LISTEN_PORT_RAW =
		"listen_port";
	public static final String KEY_CLIENT_LISTEN_PORT_AUTHONLY =
		"listen_port_authonly";
	public static final String KEY_CLIENT_LISTEN_PORT_GSI =
		"listen_port_GSI";
	public static final String KEY_CLIENT_LISTEN_PORT_SSL =
		"listen_port_SSL";
	
	/* set Default loglevel(Error) */
	public static final String DEFAULT_CLIENT_LOGLEVEL = "2";
		
	/**
	 * @param context
	 */
	public NgInformationManager(NgGrpcClient context) throws GrpcException {
		this.context = context;
		this.config = context.getConfig();
		this.listServerInfo = config.getServerInfo();
		ngMDS = new NgMDS(this);
	}
	
	/**
	 * @param hostName
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfo(String hostName)
		throws GrpcException {
		putInfoLog(
			"NgInformationManager#getRemoteMachineInfo(): get information about "
			+ hostName + ".");
		
		return searchRemoteMachineInfo(hostName);
	}

	/**
	 * @param hostName
	 * @param tagName
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfo(String hostName, String tagName)
		throws GrpcException {
		putInfoLog(
			"NgInformationManager#getRemoteMachineInfo(): get information about "
			+ hostName + ", " + tagName + ".");
		
		return (RemoteMachineInfo) mapRemoteMachineInfoCache.get(
			RemoteMachineInfo.makeHandleString(hostName, tagName));
	}

	/**
	 * @param hostName
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfoCopy(String hostName)
		throws GrpcException {
		putInfoLog(
			"NgInformationManager#getRemoteMachineInfoCopy(): get information about "
			+ hostName + ".");

		return searchRemoteMachineInfo(hostName).getCopy();
	}
	
	/**
	 * @param hostName
	 * @return
	 * @throws GrpcException
	 */
	private RemoteMachineInfo searchRemoteMachineInfo(String hostName)
		throws GrpcException {
		/* search RemoteMachineInfo by tag */
		Properties propRMInfo = null;
		for (int i = (listServerInfo.size() - 1); i >= 0; i--) {
			propRMInfo = (Properties) listServerInfo.get(i);
			if ((propRMInfo.get(RemoteMachineInfo.KEY_TAG) != null) &&
				(propRMInfo.get(RemoteMachineInfo.KEY_TAG).equals(hostName))) {
				/* found the target! */
				return (RemoteMachineInfo) mapRemoteMachineInfoCache.get(
					RemoteMachineInfo.makeHandleString(
					(String)propRMInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
					(String)propRMInfo.get(RemoteMachineInfo.KEY_TAG)));
			}
			propRMInfo = null;
		}
		
		/* search RemoteMachineInfo by hostname */
		for (int i = 0; i < listServerInfo.size(); i++) {
			propRMInfo = (Properties) listServerInfo.get(i);
			if (propRMInfo.get(RemoteMachineInfo.KEY_HOSTNAME).equals(hostName)) {
				/* found the target! */
				return (RemoteMachineInfo) mapRemoteMachineInfoCache.get(
					RemoteMachineInfo.makeHandleString(
					(String)propRMInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
					(String)propRMInfo.get(RemoteMachineInfo.KEY_TAG)));
			}
			propRMInfo = null;
		}

		/* search RemoteMachineInfo by MDS */ 
		if (propRMInfo == null) {
			/* never comes here */
			 return ngMDS.getRemoteMachineInfo(hostName, null);
		}
		
		/* no information was found! */
		return null;
	}
	
	/**
	 * @param hostName
	 * @return
	 */
	protected boolean isRemoteMachineInfoRegistered(
		String hostName) throws GrpcException {
		/* search RemoteMachineInfo by tag */
		Properties propRMInfo = null;
		for (int i = (listServerInfo.size() - 1); i >= 0; i--) {
			propRMInfo = (Properties) listServerInfo.get(i);
			if ((propRMInfo.get(RemoteMachineInfo.KEY_TAG) != null) &&
				(propRMInfo.get(RemoteMachineInfo.KEY_TAG).equals(hostName))) {
				if (mapRemoteMachineInfoCache.get(
					RemoteMachineInfo.makeHandleString(
					(String)propRMInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
					(String)propRMInfo.get(RemoteMachineInfo.KEY_TAG))) != null) {
					/* found the target! */
					return true;
				}
			}
		}
		
		/* search RemoteMachineInfo by hostname */
		for (int i = (listServerInfo.size() - 1); i >= 0; i--) {
			propRMInfo = (Properties) listServerInfo.get(i);
			if (propRMInfo.get(RemoteMachineInfo.KEY_HOSTNAME).equals(hostName)) {
				if (mapRemoteMachineInfoCache.get(
					RemoteMachineInfo.makeHandleString(
					(String)propRMInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
					(String)propRMInfo.get(RemoteMachineInfo.KEY_TAG))) != null) {
					/* found the target! */
					return true;
				}
			}
		}

		/* can't find the target */
		return false;
	}

	/**
	 * @param hostName
	 * @param tagName
	 * @return
	 * @throws GrpcException
	 */
	protected boolean isRemoteMachineInfoRegistered(
			String hostName, String tagName) throws GrpcException {
		if (getRemoteMachineInfo(hostName, tagName) != null) {
			return true;
		} else {
			return false;
		}
	}

			/**
	 * @param hostName
	 * @param className
	 * @return
	 */
	protected RemoteClassPathInfo getClassPathInfo(
		String hostName, String className) throws GrpcException {
		putInfoLog(
			"NgInformationManager#getClassPathInfo(): get information about "
			+ className + " on " + hostName + ".");
		RemoteMachineInfo remoteMachineInfo = getRemoteMachineInfo(hostName);
		if(remoteMachineInfo == null) {
			throw new NgInitializeGrpcHandleException("couldn't get RemoteMachineInfo.");
		}
		
		RemoteClassPathInfo remoteClassPathInfo =
			remoteMachineInfo.getRemoteClassPath(className);
		if ((remoteClassPathInfo == null) ||
			(! remoteClassPathInfo.contains(
			RemoteClassPathInfo.KEY_CLASS_PATH_CLASSPATH))) {
			remoteClassPathInfo =
				ngMDS.getRemoteClassPathInfo(hostName, className);
		}
		return remoteClassPathInfo;
	}

	/**
	 * @param hostName
	 * @param className
	 * @return
	 */
	protected boolean isClassPathInfoRegistered(
		String hostName, String className) throws GrpcException {

		/* get RemoteMachineInfo */
		RemoteMachineInfo remoteMachineInfo = getRemoteMachineInfo(hostName);
		if(remoteMachineInfo == null) {
			return false;
		}
		
		/* get RemoteClassPathInfo */
		RemoteClassPathInfo remoteClassPathInfo =
			remoteMachineInfo.getRemoteClassPath(className);
		if (remoteClassPathInfo == null) {
			return false;
		} else if (! remoteClassPathInfo.contains(
			RemoteClassPathInfo.KEY_CLASS_PATH_CLASSPATH)) {
				return false;
		} else {
			return true;
		}
	}

	/**
	 * @param className
	 * @return
	 */
	protected RemoteClassInfo getRemoteClassInfo(
		String className) throws GrpcException {
		putInfoLog(
			"NgInformationManager#getRemoteClassInfo(): get information about "
			+ className + ".");
		RemoteClassInfo remoteClassInfo = 
				(RemoteClassInfo) mapRemoteClassInfoCache.get(className);
		if (remoteClassInfo == null) {
			putWarningLog(
				"NgInformationManager#getRemoteClassInfo(): can't find class info for "
				+ className);
		}
		return remoteClassInfo;
	}

	/**
	 * @param className
	 * @return
	 */
	protected boolean isRemoteClassInfoRegistered(
		String className) throws GrpcException {
		putInfoLog(
			"NgInformationManager#isRemoteClassInfoRegistered(): check information about "
			+ className + ".");
		RemoteClassInfo remoteClassInfo = 
			(RemoteClassInfo) mapRemoteMachineInfoCache.get(className);
		if (remoteClassInfo == null) {
			return false;
		} else {
			return true;
		}
	}

	/**
	 * @param className
	 * @return
	 */
	protected RemoteMethodInfo getRemoteMethodInfo(
		String className) throws GrpcException {
		RemoteClassInfo remoteClassInfo = getRemoteClassInfo(className);
		if (remoteClassInfo == null) {
			throw new NgException("couldn't get RemoteClassInfo.");
		}
		List listMethods = remoteClassInfo.getRemoteMethodInfoList();
		if (listMethods.size() > 0) {
			RemoteMethodInfo remoteMethodInfo =
				(RemoteMethodInfo) listMethods.get(0);
			if (remoteMethodInfo.getName().equals(
				RemoteMethodInfo.DEFAULT_METHODNAME) != true) {
				throw new NgException(
					"NgInformationManager#getRemoteMethodInfo: RemoteLibrary doesn't have method"
					+ RemoteMethodInfo.DEFAULT_METHODNAME);
			}
			return remoteMethodInfo;
		}
		return null;
	}
	
	/**
	 * @param className
	 * @param methodName
	 * @return
	 */
	protected RemoteMethodInfo getRemoteMethodInfo(
		String className, String methodName) throws GrpcException {
		RemoteClassInfo remoteClassInfo = getRemoteClassInfo(className);
		if (remoteClassInfo == null) {
			throw new NgException("couldn't get RemoteClassInfo.");
		}
		List listMethods = remoteClassInfo.getRemoteMethodInfoList();
		for (int i = 0; i < listMethods.size(); i++) {
			RemoteMethodInfo remoteMethodInfo =
				(RemoteMethodInfo) listMethods.get(i);
			if (remoteMethodInfo.getName().equals(methodName)) {
				return remoteMethodInfo;
			}
		}
		return null;
	}
	
	/**
	 * @param handleString
	 * @return
	 */
	protected MDSInfo getMDSInfoByHandleString(String handleString) {
		return (MDSInfo) mapMDSInfo.get(handleString);
	}
	
	/**
	 * @param tag
	 * @return
	 */
	protected MDSInfo getMDSInfoByTag(String tag) {
		for (int i = 0; i < listMDSInfo.size(); i++) {
			MDSInfo mdsInfo = (MDSInfo) listMDSInfo.get(i);
			if ((mdsInfo.get(MDSInfo.KEY_TAG) != null) &&
				(mdsInfo.get(MDSInfo.KEY_TAG).equals(tag))) {
				/* found the target */
				return mdsInfo;
			}
		}
		/* target is not found */
		return null;
	}
	
	/**
	 * @param type
	 * @return
	 */
	protected InvokeServerInfo getInvokeServerInfo(String type) {
		return (InvokeServerInfo) mapInvokeServerInfo.get(type);
	}
	
	/**
	 * @return
	 */
	protected Properties getLocalMachineInfo() {
		return propLocalMachineInfo;
	}
	
	/**
	 * @param hostName
	 * @param remoteMachineInfo
	 */
	protected void putRemoteMachineInfo(
		String hostName, String tagName, RemoteMachineInfo remoteMachineInfo) {
		mapRemoteMachineInfoCache.put(
			RemoteMachineInfo.makeHandleString(hostName, tagName),
			remoteMachineInfo);
	}
	
	/**
	 * @param propServerInfo
	 * @param propServerDefault
	 */
	protected void putRemoteMachineInfo(String serverName, String tagName,
		Properties propServerInfo, Properties propServerDefault)
		throws GrpcException {
		RemoteMachineInfo remoteMachineInfo;
		
		if (isRemoteMachineInfoRegistered(serverName, tagName) == true) {
			remoteMachineInfo = getRemoteMachineInfo(serverName, tagName);
		} else {
			/* create RemoteMachineInfo with default value */
			remoteMachineInfo = new RemoteMachineInfo();
			remoteMachineInfo.put(RemoteMachineInfo.KEY_HOSTNAME, serverName);
		}
			
		/* set SERVER_DEFAULT variables */
		if (propServerDefault != null) {
			remoteMachineInfo.overwriteParameter(propServerDefault);
		}
		
		/* set server specified variables */
		if (propServerInfo != null) {
			remoteMachineInfo.overwriteParameter(propServerInfo);
		}
		
		/* put remoteMachineInfo into cache */
		putRemoteMachineInfo(serverName, tagName, remoteMachineInfo);
	}

	/**
	 * @param hostName
	 * @param className
	 * @param classPath
	 */
	protected void putRemoteClassPathInfo(String hostName, String className, 
		RemoteClassPathInfo classPath) throws GrpcException {
		Properties tmpProp = (Properties)mapRemoteMachineInfoCache;
		Enumeration keys = tmpProp.keys();
		while (keys.hasMoreElements()) {
			RemoteMachineInfo remoteMachineInfo =
				(RemoteMachineInfo) tmpProp.get(keys.nextElement());
			if (remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME).equals(hostName)) {
				/* put RemoteClassPathInfo into RemoteMachineInfo */
				remoteMachineInfo.putRemoteClassPath(className, classPath);
			}
		}
	}

	/**
	 * @param className
	 * @param remoteClassInfo
	 */
	protected void putRemoteClassInfo(
		String className, RemoteClassInfo remoteClassInfo) {
		mapRemoteClassInfoCache.put(className, remoteClassInfo);
	}
	
	/**
	 * @param prop
	 */
	protected void putMDSInfo(MDSInfo info) throws GrpcException {
		/* register */
		String handleString = info.getHandleString();
		mapMDSInfo.put(handleString, info);
		listMDSInfo.add(info);
	}

	/**
	 * @param prop
	 */
	protected void putInvokeServerInfo(String type, InvokeServerInfo info) throws GrpcException {
		/* register */
		mapInvokeServerInfo.put(type, info);
	}

	/**
	 * @param localHostInfo
	 */
	protected void putLocalMachineInfo(
		Properties localHostInfo) throws GrpcException {
		if (this.propLocalMachineInfo != null) {
			/* if there is already LocalMachineInfo, then do nothing */
			putInfoLog("NgInformationManager#putLocalMachineInfo : LocalMachineInfo does already exist");
			return;
		}
		
		/* put LocalMachineInfo into InformationManager */
		Properties propLocalMachineInfo = new Properties();
		/* set default value */
		setDefaultClientParameter(propLocalMachineInfo);
		
		/* set specified values */
		if (localHostInfo != null) {
			Enumeration keys = localHostInfo.keys();
			while (keys.hasMoreElements()) {
				String keyString = (String) keys.nextElement();
				
				/* put entries of LocalMachineInformation into Properties */
				if (keyString.equals(KEY_CLIENT_HOSTNAME)) {
					/* get default variable of hostname */
					CoGProperties cog_prop = CoGProperties.getDefault();
					cog_prop.setIPAddress(
						(String) localHostInfo.get(KEY_CLIENT_HOSTNAME));
					/* set default variable of hostname */
					CoGProperties.setDefault(cog_prop);
					propLocalMachineInfo.put(keyString,	cog_prop.getIPAddress());
				} else {
					propLocalMachineInfo.put(keyString, localHostInfo.get(keyString));
				}
			}
		}
		
		/* register */
		this.propLocalMachineInfo = propLocalMachineInfo;
	}
	
	/**
	 * @throws UnknownHostException
	 */
	private void setDefaultClientParameter(
		Properties tmpProperties) throws GrpcException {
		if (tmpProperties.containsKey(KEY_CLIENT_HOSTNAME) != true) {
			tmpProperties.put(KEY_CLIENT_HOSTNAME, Util.getLocalHostAddress());
		}
		if (tmpProperties.containsKey(KEY_CLIENT_SAVE_SESSIONINFO) != true) {
			tmpProperties.put(KEY_CLIENT_SAVE_SESSIONINFO, "256");
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOGLEVEL) != true) {
			tmpProperties.put(KEY_CLIENT_LOGLEVEL, DEFAULT_CLIENT_LOGLEVEL);
		}
		/* get Default value for logLevel */
		String logLevel = (String) tmpProperties.get(KEY_CLIENT_LOGLEVEL);
		if (tmpProperties.containsKey(KEY_CLIENT_LOGLEVEL_GT) != true) {
			tmpProperties.put(KEY_CLIENT_LOGLEVEL_GT, logLevel);
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOGLEVEL_NGPROT) != true) {
			tmpProperties.put(KEY_CLIENT_LOGLEVEL_NGPROT, logLevel);
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOGLEVEL_NGINT) != true) {
			tmpProperties.put(KEY_CLIENT_LOGLEVEL_NGINT, logLevel);
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOGLEVEL_NGGRPC) != true) {
			tmpProperties.put(KEY_CLIENT_LOGLEVEL_NGGRPC, logLevel);
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOG_FILEPATH) != true) {
			/* set empty value */
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOG_SUFFIX) != true) {
			/* set empty value */
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOG_NFILES) != true) {
			tmpProperties.put(KEY_CLIENT_LOG_NFILES, "1");
		}
		/* set default of log_maxFileSize */
		if (tmpProperties.containsKey(KEY_CLIENT_LOG_MAXFILESIZE) != true) {
			if ((tmpProperties.containsKey(KEY_CLIENT_LOG_NFILES) == true) &&
				(Integer.parseInt(
				(String) tmpProperties.get(KEY_CLIENT_LOG_NFILES)) > 1)) {
				tmpProperties.put(KEY_CLIENT_LOG_MAXFILESIZE,
					NgConfig.convertSizeValue("1M"));
			} else {
				/* set empty value (unlimited) */
			}
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LOG_OVERWRITEDIR) != true) {
			tmpProperties.put(KEY_CLIENT_LOG_OVERWRITEDIR, "false");
		}
		if (tmpProperties.containsKey(KEY_CLIENT_INVOKE_SERVER_LOG) != true) {
			/* set empty value */
		}
		if (tmpProperties.containsKey(KEY_CLIENT_TMP_DIR) != true) {
			/* set empty value */
		}
		if (tmpProperties.containsKey(KEY_CLIENT_FORTRAN_COMPATIBLE) != true) {
			/* set empty value */
		}
		if (tmpProperties.containsKey(KEY_CLIENT_HANDLING_SIGNALS) != true) {
			/* set empty value */
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LISTEN_PORT_RAW) != true) {
			tmpProperties.put(KEY_CLIENT_LISTEN_PORT_RAW, "0");
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LISTEN_PORT_AUTHONLY) != true) {
			tmpProperties.put(KEY_CLIENT_LISTEN_PORT_AUTHONLY, "0");
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LISTEN_PORT_GSI) != true) {
			tmpProperties.put(KEY_CLIENT_LISTEN_PORT_GSI, "0");
		}
		if (tmpProperties.containsKey(KEY_CLIENT_LISTEN_PORT_SSL) != true) {
			tmpProperties.put(KEY_CLIENT_LISTEN_PORT_SSL, "0");
		}
	}
	
	/**
	 * @param config
	 */
	protected void registerConfigInformation(NgConfig config)
		throws GrpcException {
		try {
			/* lock InformationManager */
			lockInformationManager();
			
			/* Init RemoteMachineInfo */
			config.initRemoteMachineInformation(this);
			/* read LocalLDIF */
			LocalLDIFFile.parseFile(config.getLocalLDIFFiles(), this);
			/* parse config and put information into InformationManager */
			config.registerConfigInformation(this);
		} finally {
			/* unlock InformationManager */
			unlockInformationManager();
		}
	}
	
	/**
	 * @return
	 */
	protected int getMDSInfoCount() {
		return listMDSInfo.size();
	}
	
	/**
	 * @param index
	 * @return
	 */
	protected MDSInfo getMDSInfoByIndex(int index) {
		if (index >= listMDSInfo.size()) {
			return null;
		} else {
			return (MDSInfo) listMDSInfo.get(index);
		}
	}
	
	/**
	 * @return
	 */
	protected Properties getDefaultRemoteMachineProperties(
		String className) throws GrpcException {
		for (int i = 0; i < this.listServerInfo.size(); i++) {
			Properties propServer = (Properties) this.listServerInfo.get(i);
			String tagName = (String) propServer.get(RemoteMachineInfo.KEY_TAG);
			String hostName = (String) propServer.get(RemoteMachineInfo.KEY_HOSTNAME);
			if (((tagName != null) &&
				(isClassPathInfoRegistered(tagName, className) == true)) ||
				(isClassPathInfoRegistered(hostName, className) == true)) {
				/* found */
				return propServer;
			}
		}

		/* nothing */
		return (Properties) this.listServerInfo.get(0);
	}
	
	/**
	 * @return
	 */
	protected NgGrpcClient getContext() {
		return this.context;
	}
	
	/**
	 * @param message
	 * @throws GrpcException
	 */
	private void putWarningLog(String message) throws GrpcException {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_WARN,
				context,
				message);
		}
	}
	
	/**
	 * @param message
	 * @throws GrpcException
	 */
	private void putInfoLog(String message) throws GrpcException {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				context,
				message);
		}
	}
	
	/**
	 * @param message
	 * @throws GrpcException
	 */
	private void putDebugLog(String message) throws GrpcException {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				context,
				message);
		}
	}
	
	/**
	 * @return
	 */
	protected Properties getServerDefault() {
		return config.getServerDefault();
	}
	
	/**
	 * @param config
	 * @throws GrpcException
	 */
	protected void resetInformationManager(NgConfig config) throws GrpcException {
		/* check SERVER information in new config file */
		List newServers = config.getServerInfo();
		for (int i = 0; i < newServers.size(); i++) {
			Properties tmpProp = (Properties) newServers.get(i);
			String targetTag = (String) tmpProp.get(RemoteMachineInfo.KEY_TAG);
			String targetHost = (String) tmpProp.get(RemoteMachineInfo.KEY_HOSTNAME);
			if (isRemoteMachineInfoRegistered(targetHost, targetTag) == true) {
				/*
				 * if the SERVER information already in InformationManager,
				 * then reset it before re-configure RemoteMachineInfo.
				 */
				RemoteMachineInfo remoteMachineInfo =
					getRemoteMachineInfo(targetHost, targetTag);
				remoteMachineInfo.reset();
			} else {
				/* if the SERVER information does not exist in InformationManager,
				 * then add it into server information list.
				 */
				this.listServerInfo.add(tmpProp);
			}
		}
		
		/* check MDS_SERVER information in new config file */
		List newMDSServers = config.getMDSServerInfo();
		for (int i = 0; i < newMDSServers.size(); i++) {
			MDSInfo tmpMDSInfo = (MDSInfo) newMDSServers.get(i);
			if (getMDSInfoByHandleString(tmpMDSInfo.getHandleString()) != null) {
				/* if the MDS_SERVER information does not exist in InformationManager,
				 * then add it into MDS server information list.
				 */
				this.listMDSInfo.remove(i);
			}
		}
	}
	
	/**
	 * @param config
	 * @throws GrpcException
	 */
	public void readConfigInformation(NgConfig config) throws GrpcException {
		try {
			/* lock the InformationManager before re-read config */
			lockInformationManager();
			
			/* reset InformationManager before re-read config */
			resetInformationManager(config);
			/* Init RemoteMachineInfo */
			config.initRemoteMachineInformation(this);
			/* read LocalLDIF */
			LocalLDIFFile.parseFile(config.getLocalLDIFFiles(), this);
			
			/* parse config and put information into InformationManager */
			config.registerConfigInformation(this);
		} finally {
			/* unlock the InformationManager */
			unlockInformationManager();
		}
	}
	
	/**
	 * Lock InformationManager.
	 * 
	 * @throws GrpcException if it's interrupted.
	 */
	protected synchronized void lockInformationManager() throws GrpcException {
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
	 * Unlock InformationManager.
	 * 
	 * @throws GrpcException  if it's interrupted.
	 */
	protected synchronized void unlockInformationManager() throws GrpcException {
		/* check if it's locked */
		if (isLocked == false) {
			throw new NgException("Nobody lock the InformationManager.");
		}
		/* unlock */
		isLocked = false;
		/* notifyAll */
		notifyAll();
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
		for (int i = 0; i < NgGlobals.debugIndent; i++) {
			sb.append(" ");
		}
		String indentStrChild = sb.toString();
		
		sb = new StringBuffer();
		
		/* RemoteMachineInfo */
		Object[] keys = (Object[]) mapRemoteMachineInfoCache.keySet().toArray();
		for (int i = 0; i < keys.length; i++) {
			sb.append(indentStr + "+ RemoteMachineInfo[" + keys[i] + "]");
			sb.append("\n");
		
			RemoteMachineInfo rmi =
				(RemoteMachineInfo) mapRemoteMachineInfoCache.get(keys[i]);
			sb.append(rmi.toString(indent + NgGlobals.debugIndent));
		}
		
		/* MDSInfo */
		keys = (Object[]) mapMDSInfo.keySet().toArray();
		for (int i = 0; i < keys.length; i++) {
			sb.append(indentStr + "+ MDSInfo[" + keys[i] + "]");
			sb.append("\n");
		
			MDSInfo mdsInfo = (MDSInfo) mapMDSInfo.get(keys[i]);
			sb.append(mdsInfo.toString(indent + NgGlobals.debugIndent));
		}
		sb.append(indentStr + "+ Order of searching MDS server");
		sb.append("\n");
		for (int i = 0; i < getMDSInfoCount(); i++) {
			sb.append(indentStrChild + (i+1) + ". " + getMDSInfoByIndex(i).getHandleString());
			sb.append("\n");
		}
				
		/* LocalMachineInfo */
		sb.append(indentStr + "+ LocalMachineInfo");
		sb.append("\n");
		Enumeration lmKeys = propLocalMachineInfo.keys();
		while (lmKeys.hasMoreElements()) {
			String key = (String) lmKeys.nextElement();
			sb.append(indentStrChild + "- " + key + " : " + propLocalMachineInfo.get(key));
			sb.append("\n");
		}
		
		/* RemoteClassInfo */
		keys = (Object[]) mapRemoteClassInfoCache.keySet().toArray();
		sb.append(indentStr + "+ RemoteClassInfo");
		sb.append("\n");
		for (int i = 0; i < keys.length; i++) {
			RemoteClassInfo rci =
				(RemoteClassInfo) mapRemoteClassInfoCache.get(keys[i]);
			sb.append(rci.toString(indent + NgGlobals.debugIndent));
		}
		
		
		/* InvokeServerInfo */
		keys = (Object[]) mapInvokeServerInfo.keySet().toArray();
		for (int i = 0; i < keys.length; i++) {
			sb.append(indentStr + "+ InvokeServerInfo[" + keys[i] + "]");
			sb.append("\n");
			InvokeServerInfo isi =
				(InvokeServerInfo) mapInvokeServerInfo.get(keys[i]);
			sb.append(isi.toString(indent + NgGlobals.debugIndent));
		}
		
		return sb.toString();
	}
}
