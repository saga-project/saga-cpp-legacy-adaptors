/*
 * $RCSfile: NgInformationManager.java,v $ $Revision: 1.18 $ $Date: 2008/03/25 05:39:07 $
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

import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

import org.apgrid.grpc.ng.info.InvokeServerInfo;
import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.apgrid.grpc.ng.info.RemoteExecutableInfo;
import org.gridforum.gridrpc.GrpcException;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.HOSTNAME;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.TAG;


class InformationCache {
	private Map<String, RemoteMachineInfo> RMICache =
		new ConcurrentHashMap<String, RemoteMachineInfo>();
	private Map<String, RemoteClassInfo> RCICache =
		new ConcurrentHashMap<String, RemoteClassInfo>();
	private Map<String, InvokeServerInfo>  ISvrInfoCache =
		new ConcurrentHashMap<String, InvokeServerInfo>();

	public InformationCache() {}


	public RemoteMachineInfo getRemoteMachineInfo(String hostname) {
		if (hostname == null)
			throw new IllegalArgumentException();

		return RMICache.get(hostname);
	}

	public RemoteMachineInfo getRemoteMachineInfo(String hostname, String tag) {
		return getRemoteMachineInfo(
			RemoteMachineInfo.makeHandleString(hostname, tag));
	}

	public RemoteClassInfo getRemoteClassInfo(String classname) {
		if (classname == null)
			throw new IllegalArgumentException();

		return RCICache.get(classname);
	}

	public InvokeServerInfo getInvokeServerInfo(String type) {
		if (type == null)
			throw new IllegalArgumentException();

		return ISvrInfoCache.get(type);
	}


	/*
	 * Put RemoteMachineInfo into cache
	 */
	public void put(String key, RemoteMachineInfo aRmi) {
		if (key == null)
			throw new IllegalArgumentException();

		RMICache.put(key, aRmi);
	}
	public void put(String hostname, String tag, RemoteMachineInfo aRmi) {
		put(RemoteMachineInfo.makeHandleString(hostname, tag), aRmi);
	}

	/*
	 * Put RemoteMachineInfo into cache
	 */
	public void put(String key, RemoteClassInfo aRci) {
		if (key == null)
			throw new IllegalArgumentException();
		RCICache.put(key, aRci);
	}

	/*
	 * Put RemoteMachineInfo into cache
	 */
	public void put(String key, InvokeServerInfo anISvrInfo) {
		if (key == null)
			throw new IllegalArgumentException();
		ISvrInfoCache.put(key, anISvrInfo);
	}

	/*
	 *
	 */
	public String toString(int indent) {
		StringBuffer sb = new StringBuffer();
		sb.append("+ RemoteMachineInfo\n");
		for (Map.Entry<String, RemoteMachineInfo> ent : RMICache.entrySet()) {
			sb.append(ent.getKey() + ":\n" + ent.getValue().toString() + "\n");
		}

		sb.append("+ RemoteClassInfo\n");
		for (Map.Entry<String, RemoteClassInfo> ent : RCICache.entrySet()) {
			sb.append(ent.getKey() + ":\n" + ent.getValue().toString() + "\n");
		}

		sb.append("+ InvokeServerInfo\n");
		for (Map.Entry<String, InvokeServerInfo> ent : ISvrInfoCache.entrySet()) {
			sb.append(ent.getKey() + ":\n" + ent.getValue().toString(indent) + "\n");
		}

		return sb.toString();
	}
    }


class NgInformationManager {
	private NgGrpcClient context;
	private InformationCache icache = new InformationCache();
	private NgInformationServiceManager isvcManager=null;
	private List listServerConfig;
	private boolean isLocked;
	private LocalMachineInfo localMachineInfo = null;

	// short class name
	private static final String name = "NgInformationManager";


	/**
	 * NgInformationManager Constructor 
	 * 
	 * @param context
	 */
	public NgInformationManager(NgGrpcClient context) throws GrpcException {
		this.context = context;
		NgConfig config = context.getConfig();
		this.listServerConfig = config.getServerInfo();
		// Create Information Service Manager.
		// As a result, Information Service process is execute.
		this.isvcManager =
			new NgInformationServiceManager(this, config, context.getNgLog());
	}


	/**
	 * @param hostName
	 * @return
	 */
	public RemoteMachineInfo getRemoteMachineInfo(String hostname)
	 throws GrpcException {

		putInfoLog("#getRemoteMachineInfo(): get information about " 
			+ hostname + ".");

		return searchRemoteMachineInfo(hostname);
	}

	/**
	 * @param hostname server name
	 * @param tag server tag
	 */
	private RemoteMachineInfo getRemoteMachineInfo(
	 String hostname, String tag) {

		putInfoLog("#getRemoteMachineInfo(): get information about " 
			+ hostname + ", " + tag);

		return icache.getRemoteMachineInfo(hostname, tag);
	}

	/**
	 * @param hostname
	 * @return
	 */
	public RemoteMachineInfo getRemoteMachineInfoCopy(String hostname)
	 throws GrpcException {
		putInfoLog("#getRemoteMachineInfoCopy(): Get information about " 
			+ hostname + ".");

		RemoteMachineInfo ret;
		synchronized (this) {
			ret = searchRemoteMachineInfo(hostname).getCopy();
		}
		return ret;
	}

	/**
	 * @param hostName (or tag)
	 * @return
	 */
	private RemoteMachineInfo searchRemoteMachineInfo(String hostName) {
		// search RemoteMachineInfo by tag 
		Properties propRMInfo = null;

		for (int i = (listServerConfig.size() - 1); i >= 0; i--) {
			propRMInfo = (Properties)listServerConfig.get(i);
			if ((propRMInfo.get(TAG) != null) &&
				(propRMInfo.get(TAG).equals(hostName))) {
				// found the target!
				return icache.getRemoteMachineInfo(
						propRMInfo.getProperty(HOSTNAME),
						propRMInfo.getProperty(TAG));
			}
			propRMInfo = null;
		}
		
		// search RemoteMachineInfo by hostname 
		for (int i = 0; i < listServerConfig.size(); i++) {
			propRMInfo = (Properties) listServerConfig.get(i);
			if (propRMInfo.get(HOSTNAME).equals(hostName)) {
				// found the target! 
				return icache.getRemoteMachineInfo(
							propRMInfo.getProperty(HOSTNAME),
							propRMInfo.getProperty(TAG));
			}
			propRMInfo = null;
		}

		return null; // no information was found!
	}
	
	/**
	 * @param hostName
	 * @return
	 */
	private boolean isRemoteMachineInfoRegistered(String hostName)
	 throws GrpcException {
		// search RemoteMachineInfo by tag 
		Properties propRMInfo = null;
		for (int i = (listServerConfig.size() - 1); i >= 0; i--) {
			propRMInfo = (Properties) listServerConfig.get(i);
			if ((propRMInfo.get(TAG) != null) &&
				(propRMInfo.get(TAG).equals(hostName))) {
				if (icache.getRemoteMachineInfo(
						propRMInfo.getProperty(HOSTNAME),
						propRMInfo.getProperty(TAG)) != null) {
					// found the target! 
					return true;
				}
			}
		}
		
		// search RemoteMachineInfo by hostname 
		for (int i = (listServerConfig.size() - 1); i >= 0; i--) {
			propRMInfo = (Properties) listServerConfig.get(i);
			if (propRMInfo.get(HOSTNAME).equals(hostName)) {
				if (icache.getRemoteMachineInfo(
						propRMInfo.getProperty(HOSTNAME),
						propRMInfo.getProperty(TAG)) != null) {
					// found the target! 
					return true;
				}
			}
		}

		// can't find the target 
		return false;
	}

	/**
	 * @param hostName
	 * @param tag
	 * @return
	 */
	protected boolean isRemoteMachineInfoRegistered(String hostname,
	 String tag) {
		if (getRemoteMachineInfo(hostname, tag) != null) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * @param hostname
	 * @param classname
	 * @return
	 */
	public synchronized RemoteClassPathInfo getClassPathInfo(String hostname,
	 String classname)
	 throws GrpcException {
		putInfoLog("#getClassPathInfo(hostname, classname): get information about "
			+ classname + " on " + hostname);
		RemoteMachineInfo rmi = getRemoteMachineInfo(hostname);
		if(rmi == null) {
			throw new NgInitializeGrpcHandleException(
				"couldn't get RemoteMachineInfo \"" + hostname + "\".");
		}

		RemoteClassPathInfo rcpi = rmi.getRemoteClassPath(classname);
		if (rcpi == null) {
			// Request for Information Service
			RemoteExecutableInfo rei =
				isvcManager.getRemoteExecutableInfo(classname, hostname, null);

			rcpi = rei.getClassPathInfo();
			if (rcpi == null) {
				throw new NgInitializeGrpcHandleException(
					"couldn't get RemoteClassPathInfo.");
			}

			updateCache(rei);
		}

		return rcpi;
	}


	/**
	 * Get RemoteClassInfo by cache
	 * 
	 * @param className
	 */
	public RemoteClassInfo getClassInfo(String classname) {
		putInfoLog("#getRemoteClassInfo(): get information about " + classname);
		RemoteClassInfo rci = icache.getRemoteClassInfo(classname);

		if (rci == null)
			putInfoLog("#getClassInfo(): can't find class info for "
				+ classname);

		return rci;
	}

	/** 
	 * Get RemoteClassInfo by cache or Information Service
	 *
	 * @param classname 
	 * @param hostname server name
	 * @param tag server tag
	 */
	public RemoteClassInfo getClassInfo(
	 String classname, String hostname, String tag) {
		if (classname == null)
			throw new IllegalArgumentException();

		putInfoLog(
			"#getRemoteClassInfo(classname, hostname): get information about "
			+ classname + ".");

		// get Info by Cache
		RemoteClassInfo rci = icache.getRemoteClassInfo(classname);
		if (rci != null)
			return rci;

		// Cache has no RemoteClassInfo
		putInfoLog(
			"#getRemoteClassInfo(classname, hostname): There is no class info "
			+ classname + " in Cache,  " );
		// get Info by Information Service
		try {
			RemoteExecutableInfo rei =
				isvcManager.getRemoteExecutableInfo(
					classname, hostname, null);

			updateCache(rei);

			rci = rei.getClassInfo();
		} catch (GrpcException e) {
			putInfoLog("#getClassInfo(classname, hostname): can't find remote executable info for "
				+ classname);
			rci = null;
		}
		return rci;
	}

	private void updateCache(RemoteExecutableInfo rei) {
		putDebugLog("#updateCache(RemoteExecutableInfo): RemoteExecutableInfo is \n" + rei);

		RemoteClassPathInfo rcpi = rei.getClassPathInfo();
		RemoteClassInfo rci = rei.getClassInfo();

		// update cache (individual info)
		icache.put(rci.getName(), rci);
		updateCache(rcpi.getHostname(), rcpi);
	}

	private void updateCache(String hostname, RemoteClassPathInfo rcpi) {
		RemoteMachineInfo rmi = icache.getRemoteMachineInfo(hostname);
		if (rmi == null) {
			putInfoLog("#updateCache(hostname, RemoteClassPathInfo): cache has no RemoteMachineInfo about " + hostname + ", therefore create it." );
			rmi = new RemoteMachineInfo(hostname);
		}
		// register a new RemoteClassPathInfo to RemoteMachineInfo
		rmi.putRemoteClassPath(rcpi.getClassname(), rcpi);

		// update cache
		icache.put(hostname, rmi);
	}

	/*
	 * @param className
	 * @param methodName
	 */
	protected RemoteMethodInfo getMethodInfo(
	 String className, String methodName)
	 throws GrpcException {
		RemoteClassInfo rci = getClassInfo(className);
		if (rci == null)
			return null;

		String _methodname = methodName;
		if (_methodname == null)
			_methodname = RemoteMethodInfo.DEFAULT_METHODNAME;

		List<RemoteMethodInfo> listMethods = rci.getRemoteMethodInfoList();
		for (RemoteMethodInfo rmi : listMethods) {
			if (rmi.getName().equals(_methodname)) {
				return rmi;
			}
		}

		return null;
	}


	/**
	 * @param type
	 * @return
	 */
	protected InvokeServerInfo getInvokeServerInfo(String type) {
		return icache.getInvokeServerInfo(type);
	}

	/**
	 * @return LocalMachineInformation
	 */
	public LocalMachineInfo getLocalMachineInfo() {
		return localMachineInfo;
	}


	/**
	 * @param className
	 * @param remoteClassInfo
	 */
	public void putClassInfo(String classname, RemoteClassInfo rci) {
		icache.put(classname, rci);
	}

	/**
	 */
	public synchronized Properties getDefaultRemoteMachineProperties(String className) 
	 throws GrpcException {
		for (int i = 0; i < this.listServerConfig.size(); i++) {
			Properties propServer = (Properties) this.listServerConfig.get(i);
			String tagName  = propServer.getProperty(TAG);
			String hostName =  propServer.getProperty(HOSTNAME);
			if (((tagName != null) &&
				( isClassPathInfoRegistered(tagName, className) )) ||
				( isClassPathInfoRegistered(hostName, className) )) {
				return propServer; // found 
			}
		}
		// nothing
		return (Properties)this.listServerConfig.get(0);
	}

	private boolean isClassPathInfoRegistered(String hostname, String classname)
	 throws GrpcException {
		RemoteMachineInfo rmi = getRemoteMachineInfo(hostname);
		if(rmi == null)
			return false;

		RemoteClassPathInfo rcpi = rmi.getRemoteClassPath(classname);
		if ( (rcpi != null) && rcpi.hasClasspath() )
			return true;

		return false;
	}

	/**
	 * @return
	 */
	protected NgGrpcClient getContext() {
		return this.context;
	}

	/**
	 * @param config
	 */
	public void registerConfigInformation(NgConfig config)
	 throws GrpcException {
		synchronized (this) {
			// Init RemoteMachineInfo 
			InitRemoteMachineInformation(config);

			// parse config and put information into InformationManager 
			RegisterConfigInformation(config);
			isvcManager.initializeInformationSource();
		}
	}

	private void InitRemoteMachineInformation(NgConfig config)
	 throws GrpcException {
		registerServerInfo(config.getServerInfo(), null);
	}

	private void RegisterConfigInformation(NgConfig config)
	 throws GrpcException {
		registerServerInfo(config.getServerInfo(),
			config.getServerDefault());
		registerLocalMachineInfo(config.getLocalMachineInfo2());
		registerFunctionInfo(config.getFunctionInfo());
		registerInvokeServerInfo(config.getInvokeServerInfo());
	}

	/// put RemoteMachineInformation 
	private void registerServerInfo(List<Properties> serverInfo,
	 Properties serverDefault)
	 throws GrpcException {
		for (Properties prop : serverInfo) {
			putRemoteMachineInfo(
				prop.getProperty("hostname"),
				prop.getProperty("tag"),
				prop, serverDefault);
		}
	}

	private void putRemoteMachineInfo(String servername, String tag,
	 Properties aServerInfo, Properties aServerDefault)
	 throws GrpcException {
		RemoteMachineInfo rmi;
		if ( isRemoteMachineInfoRegistered(servername, tag) ) {
			rmi = getRemoteMachineInfo(servername, tag);
		} else {
			// create RemoteMachineInfo with default value
			rmi = new RemoteMachineInfo();
			rmi.setHostname(servername);
		}

		// set SERVER_DEFAULT variables 
		if (aServerDefault != null)
			rmi.update(aServerDefault);

		// set server specified variables
		if (aServerInfo != null)
			rmi.update(aServerInfo);

		// put remoteMachineInfo into cache
		icache.put(servername, tag, rmi);
	}

	private void registerLocalMachineInfo(LocalMachineInfo localHostInfo) {
		this.localMachineInfo = localHostInfo;
	}

	private void registerFunctionInfo(List<Properties> funcInfo)
	 throws GrpcException {
		for (Properties prop : funcInfo) {
			String remoteHostname = prop.getProperty("hostname");
			String remoteClassname = prop.getProperty("funcname");
			RemoteMachineInfo rmi = null;
			RemoteClassPathInfo rcpi =
				new RemoteClassPathInfo(remoteHostname, remoteClassname,
					prop.getProperty("staging"),
					prop.getProperty("path"),
					prop.getProperty("backend"),
					prop.getProperty("session_timeout"));

			if (isRemoteMachineInfoRegistered(remoteHostname)) {
				rmi = getRemoteMachineInfo(remoteHostname);
			} else {
				rmi = new RemoteMachineInfo(remoteHostname);
			}

			putRemoteClassPathInfo(remoteHostname, remoteClassname, rcpi);
			icache.put(remoteHostname, rmi);
		}
	}

	private void registerInvokeServerInfo(List<Properties> invokeServerInfo)
	 throws GrpcException {
		for (Properties prop : invokeServerInfo) {
			InvokeServerInfo info = new InvokeServerInfo(prop);
			icache.put(info.getType(), info);
		}
	}


	/*
	 * @param message log message
	 */
	private void putInfoLog(String message) {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.logInfo(NgLog.CAT_NG_INTERNAL, context.logHeader() 
				+ name + message);
		}
	}
	
	/*
	 * @param message log message
	 */
	private void putDebugLog(String message) {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.logInfo(NgLog.CAT_NG_INTERNAL, context.logHeader()
				+ name + message);
		}
	}

	/**
	 * Lock InformationManager.
	 * 
	 * @throws GrpcException if it's interrupted.
	 */
	protected synchronized void lockInformationManager()
	 throws GrpcException {
		// wait for Unlocked 
		while (isLocked) {
			try {
				wait();
			} catch (InterruptedException e) {
				throw new NgException(e);
			}
		}
		isLocked = true; // lock
	}
	
	/**
	 * Unlock InformationManager.
	 * 
	 * @throws GrpcException  if it's interrupted.
	 */
	protected synchronized void unlockInformationManager()
	 throws GrpcException {
		// check if it's locked 
		if (! isLocked ) {
			throw new NgException("Nobody lock the InformationManager.");
		}
		isLocked = false; // unlock
		notifyAll();
	}

	/**
	 * @param hostName
	 * @param className
	 * @param classPath
	 */
	private void putRemoteClassPathInfo(String hostname, String classname, 
	 RemoteClassPathInfo rcpi)
	 throws GrpcException {
		RemoteMachineInfo rmi = icache.getRemoteMachineInfo(hostname);
		// put RemoteClassPathInfo into RemoteMachineInfo 
		rmi.putRemoteClassPath(classname, rcpi);
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

		// LocalMachineInfo
		sb.append(indentStr + "+ LocalMachineInfo\n");
		for (Map.Entry<String, String> e : localMachineInfo.entrySet()) {
			sb.append(indentStrChild + "- " + e.getKey() + ": " 
				+ e.getValue() + "\n");
		}

		// Information Source
		sb.append(isvcManager.toString());

		// Information Cache
		sb.append(icache.toString(indent));
		
		return sb.toString();
	}

	/**
	 * @param className
	 * @return
	 *
	protected RemoteMethodInfo getMethodInfo(String className)
	 throws GrpcException {
		RemoteClassInfo rci = getClassInfo(className);
		if (rci == null) 
			throw new NgException("couldn't get RemoteClassInfo.");

		List<RemoteMethodInfo> listMethods = rci.getRemoteMethodInfoList();
		if (listMethods.isEmpty())
			return null;

		RemoteMethodInfo rmi = listMethods.get(0);
		if (! rmi.getName().equals(RemoteMethodInfo.DEFAULT_METHODNAME)) {
			throw new NgException(name 
				+ "#getRemoteMethodInfo(classname): RemoteLibrary doesn't have method "
				+ RemoteMethodInfo.DEFAULT_METHODNAME);
		}
		return rmi;
	}
	 */
	
	/**
	 * @param hostName
	 * @param remoteMachineInfo
	 * 
	private void putRemoteMachineInfo(String hostname, String tag,
	 RemoteMachineInfo anRMI) {
		icache.put(hostname, tag, anRMI);
	}
	 */

	/*
	 *  Dispose the resources
	 */
	public void dispose() {
		this.isvcManager.dispose();
		this.isvcManager = null;
	}

}
