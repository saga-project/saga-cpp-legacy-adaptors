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
 * $RCSfile: NgGrpcClient.java,v $ $Revision: 1.55 $ $Date: 2006/09/12 08:26:29 $
 */
package org.apgrid.grpc.ng;

import java.io.IOException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Vector;

import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.globus.io.gass.server.GassServer;
import org.globus.util.deactivator.Deactivator;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;
import org.gridforum.gridrpc.GrpcObjectHandle;

/**
 * Ninf-G implementation of GrpcClient interface.<br>
 * 
 * Provides functions defined in standard interfaces
 * and also provides Ninf-G special functions. 
 */
public class NgGrpcClient implements GrpcClient {
	private int ID = 0;

	private List handleList = new Vector();
	private NgConfig config;
	private NgInformationManager informationManager;
	private GassServer gassServerSecure;
	private GassServer gassServerNoSecure;
	private PortManager portManagerNoSecure;
	private PortManager portManagerSSL;
	private PortManager portManagerGSI;
	private CommunicatorTable commTable;
	private NgLog ngLog;
	private int lastJobID = 0;
	private int lastExecutableID = 0;
	private Properties propExecutableID = new Properties();
	private Map mapNJobs = new HashMap();
	private Map mapNConnectBack = new HashMap();
	private Map mapJobRMInfo = new HashMap();
	private NgInvokeServerManager isMng;
	
	private static String envConfigFilename = "ninfg.configFile";
	
	/**
	 * Creates NgGrpcClient object.
	 */
	public NgGrpcClient() {
		ID = NgGlobals.generateContextID();
		NgGlobals.putContext(this);
	}

	/**
	 * Initialize with a name of configuration file.<br>
	 * If configFilename was null,
	 * "user.configFile" SystemProperty is used as configuration file.
	 * 
	 * @param configFilename a name of configuration file.
	 * @throws GrpcException it it failed to initialize.
	 */
	public void activate(String configFilename) throws GrpcException {
		/* check if configFilename were set in environment value */
		if (configFilename != null) {
			config = new NgConfig(configFilename);
		} else if (System.getProperty(envConfigFilename) != null) {
			config = new NgConfig(System.getProperty(envConfigFilename));
		} else {
			throw new NgInitializeGrpcClientException(
				"not specified configfile.");
		}
		
		activate();
	}

	/**
	 * Initialize with Properties object which contains information for server.<br>
	 * If "user.configFile" SystemProperty was set, use it as configuration file.<br>
	 * If it's not set, then use Properties as information of the server.
	 * 
	 * @param prop information of server.
	 * @throws GrpcException if it failed to initialize.
	 */
	public void activate(Properties prop) throws GrpcException {
		/* check if configFilename were set in environment value */
		if (System.getProperty(envConfigFilename) != null) {
			config = new NgConfig(System.getProperty(envConfigFilename));
		} else {
			config = new NgConfig(prop);
		}
		activate();
	}
	
	/**
	 * Initialize NgGrpcClient object.
	 * 
	 * @throws GrpcException
	 */
	private void activate() throws GrpcException {
		/* create InformationManager */
		informationManager = new NgInformationManager(this);
		informationManager.registerConfigInformation(config);
		
		/* prepare Log facility */
		try {
			informationManager.lockInformationManager();
			
			ngLog = new NgLog(informationManager.getLocalMachineInfo());
		} finally {
			informationManager.unlockInformationManager();
		}
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#activate(): created NgLog.");

		/* start GASS server */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#activate(): start GASSServer.");
		startGassServer();

		/* create table of Communicator */
		commTable = new CommunicatorTable(this);

		/* start PortManager */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#activate(): start PortManager.");
		startPortManager();
		
		/* start InvokeServerManager */
		startInvokeServerManager();
	}

	/**
	 * Creates {@link org.gridforum.gridrpc.GrpcFunctionHandle}
	 * for the specified function on the server.<br>
	 * Use prop argument to set attribute variables of the server.<br>
	 * Use strings defined in {@link NgGrpcHandleAttr} as key strings.
	 * 
	 * @param functionName a name of function.
	 * @param prop information of server.
	 * @return {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 * @see NgGrpcHandleAttr
	 */
	public GrpcFunctionHandle getFunctionHandle(
		String functionName, Properties prop) throws GrpcException {
		return getFunctionHandles(functionName, prop, 1)[0];
	}

	/**
	 * Creates {@link org.gridforum.gridrpc.GrpcFunctionHandle} without server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section in a configuration file
	 * will be used.
	 * 
	 * @param functionName a name of function.
	 * @return {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 */
	public GrpcFunctionHandle getFunctionHandle(String functionName)
		throws GrpcException {
		return getFunctionHandle(functionName, null);
	}

	/**
	 * Creates specified number of {@link org.gridforum.gridrpc.GrpcFunctionHandle}
	 * for the specified function on the server.<br>
	 * Use prop argument to set attribute variables of the server.<br>
	 * Use strings defined in {@link NgGrpcHandleAttr} as key strings.
	 * 
	 * @param functionName a name of function.
	 * @param prop information of server.
	 * @param nHandles a number of FunctionHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 * @see NgGrpcHandleAttr
	 */
	public GrpcFunctionHandle[] getFunctionHandles(
		String functionName, Properties prop, int nHandles)
		throws GrpcException {
		/* generate JobID */
		int jobID = generateJobID();
		/* number of Jobs */
		int numJobs = 0;

		/* create handle */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#getFunctionHandles(): create handle.");
		NgGrpcFunctionHandle[] handles = new NgGrpcFunctionHandle[nHandles];
		String jobType = null;
		try {
			informationManager.lockInformationManager();
			
			RemoteClassInfo remoteClassInfo =
				informationManager.getRemoteClassInfo(functionName);
			jobType = (remoteClassInfo != null) ? remoteClassInfo.getBackend() : null;
		} finally {
			informationManager.unlockInformationManager();
		}
		
		/* create handle for MPI */
		if ((jobType != null) &&
			(jobType.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			jobType.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {

			/* put nJobs into map */
			mapNJobs.put(new Integer(jobID), new Integer(1));
			/* init num of connect back into map */
			mapNConnectBack.put(new Integer(jobID), new Integer(0));

			/* number of Jobs */
			if (nHandles > 1) {
				numJobs = 1;
			}else if (nHandles == 1) {
				numJobs = NgGrpcJob.INVALID_JOB_ID;
			} else {
				throw new NgException("Invalid number of handles : " + nHandles);
			}
		}
		/* It's not for MPI */
		else {
			/* put nJobs into map */
			mapNJobs.put(new Integer(jobID), new Integer(nHandles));
			/* init num of connect back into map */
			mapNConnectBack.put(new Integer(jobID), new Integer(0));
			/* generate Executable ID before connect back */
			for (int i = 1; i < nHandles; i++) {
				generateExecutableID(jobID, i);
			}
			/* number of Jobs */
			numJobs = nHandles;
		}

		/* get and put RemoteMachineInfo */
		String targetHost = null;
		try {
			informationManager.lockInformationManager();
			
			if ((prop == null) || prop.get(RemoteMachineInfo.KEY_HOSTNAME) == null) {
				Properties tmpProp =
					informationManager.getDefaultRemoteMachineProperties(functionName);
				targetHost = (String) tmpProp.get(RemoteMachineInfo.KEY_HOSTNAME);
			} else {
				targetHost = (String) prop.get(RemoteMachineInfo.KEY_HOSTNAME);
			}
			
			mapJobRMInfo.put(new Integer(jobID),
					informationManager.getRemoteMachineInfo(targetHost));
		} finally {
			informationManager.unlockInformationManager();
		}
		
		/* create handle */
		if (prop == null) {
			handles[0] = new NgGrpcFunctionHandle(functionName, this,
					jobID, generateExecutableID(jobID, 0), numJobs);
		} else {
			handles[0] = new NgGrpcFunctionHandle(functionName, prop, this,
					jobID, generateExecutableID(jobID, 0), numJobs);
		}

		/* put handles into handle list */
		handleList.add(handles[0]);
		/* create handles for MPI and put them into handle list */
		if ((jobType != null) &&
			(jobType.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			jobType.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {
			for (int i = 1; i < nHandles; i++) {
				/* generate new JobID */
				jobID = generateJobID();
				/* put nJobs into map */
				mapNJobs.put(new Integer(jobID), new Integer(1));
				/* init num of connect back into map */
				mapNConnectBack.put(new Integer(jobID), new Integer(0));

				if (prop == null) {
					handles[i] = new NgGrpcFunctionHandle(functionName, this,
							jobID, generateExecutableID(jobID, 0), 1);
				} else {
					handles[i] = new NgGrpcFunctionHandle(functionName, prop, this,
							jobID, generateExecutableID(jobID, 0), 1);
				}
				/* put Job into map */
				try {
					informationManager.lockInformationManager();
					
					mapJobRMInfo.put(new Integer(jobID),
							informationManager.getRemoteMachineInfo(targetHost));
				} finally {
					informationManager.unlockInformationManager();
				}
			}
		}
		/* It's not for MPI, clone handles and put them into handle list */
		else {
			for (int i = 1; i < nHandles ; i++) {
				try {
					handles[i] = (NgGrpcFunctionHandle) handles[0].clone();
				} catch (CloneNotSupportedException e) {
					throw new NgInitializeGrpcHandleException(e);
				}
				handles[i].resetHandle();
				handles[i].setExecutableID(getExecutableID(jobID, i));
				handles[i].startCommunicationManager();
				handleList.add(handles[i]);
			}
		}

		return handles;
	}

	/**
	 * Create specified number of {@link org.gridforum.gridrpc.GrpcFunctionHandle}
	 * without server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section in a configuration file
	 * will be used.
	 * 
	 * @param functionName a name of function.
	 * @param nHandles a number of FunctionHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 */
	public GrpcFunctionHandle[] getFunctionHandles(
		String functionName, int nHandles) throws GrpcException {
		return getFunctionHandles(functionName, null, nHandles);
	}

	/**
	 * Creates {@link org.gridforum.gridrpc.GrpcObjectHandle}
	 * for the specified object on the server.<br>
	 * Use prop argument to set attribute variables of the server.<br>
	 * Use strings defined in {@link NgGrpcHandleAttr} as key strings.
	 * 
	 * @param objectName a name of object.
	 * @param prop information of server.
	 * @return {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 * @see NgGrpcHandleAttr
	 */
	public GrpcObjectHandle getObjectHandle(String objectName,
		Properties prop) throws GrpcException {
		return getObjectHandles(objectName, prop, 1)[0];
	}

	/**
	 * Creates {@link org.gridforum.gridrpc.GrpcObjectHandle} without server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section in a configuration file
	 * will be used.
	 * 
	 * @param objectName a name of object.
	 * @return {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 */
	public GrpcObjectHandle getObjectHandle(String objectName)
		throws GrpcException {
		return getObjectHandle(objectName, null);
	}

	/**
	 * Creates specified number of {@link org.gridforum.gridrpc.GrpcObjectHandle}
	 * for the specified function on the server.
	 * Use prop argument to set attribute variables of the server.<br>
	 * Use strings defined in {@link NgGrpcHandleAttr} as key strings.
	 * 
	 * @param objectName a name of object.
	 * @param prop information of server.
	 * @param nHandles a number of ObjectHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 * @see NgGrpcHandleAttr
	 */
	public GrpcObjectHandle[] getObjectHandles(
		String objectName, Properties prop, int nHandles)
		throws GrpcException {
		/* generate JobID */
		int jobID = generateJobID();
		/* number of Jobs */
		int numJobs = 0;

		/* create handle */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#getObjectHandles(): create handle.");
		NgGrpcObjectHandle[] handles = new NgGrpcObjectHandle[nHandles];
		String jobType = null;
		try {
			informationManager.lockInformationManager();
			
			RemoteClassInfo remoteClassInfo =
				informationManager.getRemoteClassInfo(objectName);
			jobType = (remoteClassInfo != null) ? remoteClassInfo.getBackend() : null;
		} finally {
			informationManager.unlockInformationManager();
		}
		
		/* create handle for MPI */
		if ((jobType != null) &&
			(jobType.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			jobType.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {

			/* put nJobs into map */
			mapNJobs.put(new Integer(jobID), new Integer(1));
			/* init num of connect back into map */
			mapNConnectBack.put(new Integer(jobID), new Integer(0));

			/* number of Jobs */
			if (nHandles > 1) {
				numJobs = 1;
			}else if (nHandles == 1) {
				numJobs = NgGrpcJob.INVALID_JOB_ID;
			} else {
				throw new NgException("Invalid number of handles : " + nHandles);
			}
		}
		/* It's not for MPI */
		else {
			/* put nJobs into map */
			mapNJobs.put(new Integer(jobID), new Integer(nHandles));
			/* init num of connect back into map */
			mapNConnectBack.put(new Integer(jobID), new Integer(0));
			/* generate Executable ID before connect back */
			for (int i = 1; i < nHandles; i++) {
				generateExecutableID(jobID, i);
			}
			/* number of Jobs */
			numJobs = nHandles;
		}

		/* get and put RemoteMachineInfo */
		String targetHost = null;
		try {
			informationManager.lockInformationManager();
			
			if ((prop == null) || prop.get(RemoteMachineInfo.KEY_HOSTNAME) == null) {
				Properties tmpProp =
					informationManager.getDefaultRemoteMachineProperties(objectName);
				targetHost = (String) tmpProp.get(RemoteMachineInfo.KEY_HOSTNAME);
			} else {
				targetHost = (String) prop.get(RemoteMachineInfo.KEY_HOSTNAME);
			}
			
			mapJobRMInfo.put(new Integer(jobID),
					informationManager.getRemoteMachineInfo(targetHost));
		} finally {
			informationManager.unlockInformationManager();
		}
		
		/* create handle */
		if (prop == null) {
			handles[0] = new NgGrpcObjectHandle(objectName, this,
					jobID, generateExecutableID(jobID, 0), numJobs);
		} else {
			handles[0] = new NgGrpcObjectHandle(objectName, prop, this,
					jobID, generateExecutableID(jobID, 0), numJobs);
		}

		/* put handles into handle list */
		handleList.add(handles[0]);
		/* create handles for MPI and put them into handle list */
		if ((jobType != null) &&
			(jobType.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			jobType.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {
			for (int i = 1; i < nHandles; i++) {
				/* generate new JobID */
				jobID = generateJobID();
				/* put nJobs into map */
				mapNJobs.put(new Integer(jobID), new Integer(1));
				/* init num of connect back into map */
				mapNConnectBack.put(new Integer(jobID), new Integer(0));

				if (prop == null) {
					handles[i] = new NgGrpcObjectHandle(objectName, this,
							jobID, generateExecutableID(jobID, 0), 1);
				} else {
					handles[i] = new NgGrpcObjectHandle(objectName, prop, this,
							jobID, generateExecutableID(jobID, 0), 1);
				}
				/* put Job into map */
				try {
					informationManager.lockInformationManager();
					
					mapJobRMInfo.put(new Integer(jobID),
							informationManager.getRemoteMachineInfo(targetHost));
				} finally {
					informationManager.unlockInformationManager();
				}
			}
		}
		/* It's not for MPI, clone handles and put them into handle list */
		else {
			for (int i = 1; i < nHandles ; i++) {
				try {
					handles[i] = (NgGrpcObjectHandle) handles[0].clone();
				} catch (CloneNotSupportedException e) {
					throw new NgInitializeGrpcHandleException(e);
				}
				handles[i].resetHandle();
				handles[i].setExecutableID(getExecutableID(jobID, i));
				handles[i].startCommunicationManager();
				handleList.add(handles[i]);
			}
		}

		return handles;
	}

	/**
	 * Creates specified number of {@link org.gridforum.gridrpc.GrpcObjectHandle}
	 * without server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section in a configuration file
	 * will be used.
	 * 
	 * @param objectName a name of object.
	 * @param nHandles a number of ObjectHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 */
	public GrpcObjectHandle[] getObjectHandles(
		String objectName, int nHandles) throws GrpcException {
		return getObjectHandles(objectName, null, nHandles);
	}

	/* (non-Javadoc)
	 * @see org.gridforum.gridrpc.GrpcClient#deactivate()
	 */
	public void deactivate() throws GrpcException {
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcClient#deactivate()");

		/* deactivate */
		stopGassServer();
		stopPortManager();
		Deactivator.deactivateAll();
		stopInvokeServerManager();
	}
	
	/**
	 * Generates ID of the Job.
	 * 
	 * @return ID of the Job.
	 */
	private synchronized int generateJobID() {
		/* generate JobID */
		int jobID = lastJobID;
		lastJobID += 1;
		return jobID;
	}
	
	/**
	 * Generates ID of the RemoteExecutable.
	 * 
	 * @param jobID ID of the Job.
	 * @return ID of the RemoteExecutable.
	 */
	private synchronized int generateExecutableID(int jobID) {
		return generateExecutableID(jobID, 0);
	}
	
	/**
	 * Generates ID of the RemoteExecutable.
	 * 
	 * @param jobID ID of the Job.
	 * @param subJobID a rank of this in list of Jobs.
	 * @return ID of the RemoteExecutable.
	 */
	private synchronized int generateExecutableID(int jobID, int subJobID){
		/* generate executableID from jobID and subJobID */
		int executableID = lastExecutableID;
		lastExecutableID += 1;

		/* register into map */
		propExecutableID.put(generateExecutableIDKey(jobID, subJobID),
							new Integer(executableID));

		return executableID;		
	}
	
	/**
	 * Gets ID of the RemoteExecutable specified by jobID and subJobID.
	 * 
	 * @param jobID ID of the Job.
	 * @param subJobID a rank of this in list of Jobs.
	 * @return ID of the RemoteExecutable.
	 */
	protected int getExecutableID(int jobID, int subJobID) {
		/* get executableID by jobID and subJobID */
		Integer executableID = (Integer)propExecutableID.get(
			generateExecutableIDKey(jobID, subJobID));

		if (executableID == null) {
			/* Invalid handle */
			return -1;
		} else {
			return executableID.intValue();
		}
	}
	
	/**
	 * Generates ID string of the RemoteExecutable specified by jobID and subJobID.
	 * 
	 * @param jobID ID of the Job.
	 * @param subJobID a rank of this in list of Jobs.
	 * @return ID string of the RemoteExecutable.
	 */
	private String generateExecutableIDKey(int jobID, int subJobID) {
		return jobID +"/" + subJobID;
	}
	
	/**
	 * Gets NgConfig object which is used in this object.
	 * 
	 * @return {@link org.apgrid.grpc.ng.NgConfig}
	 * @throws GrpcException if failed to get NgConfig.
	 */
	protected NgConfig getConfig() throws GrpcException {
		return config;
	}
	
	/**
	 * Gets NgLog object which has information of putting log messages.
	 * 
	 * @return {@link org.apgrid.grpc.ng.NgLog}
	 * @throws GrpcException if failed to get NgLog.
	 */
	protected NgLog getNgLog() throws GrpcException {
		return ngLog;
	}
	
	/**
	 * Gets GassServer object which can receive non-encrypted data.
	 * 
	 * @return non-secure mode of GassServer.
	 * @throws GrpcException if failed to get a GassServer.
	 */
	protected GassServer getGassServerNoSecure() throws GrpcException {
		return gassServerNoSecure;
	}
	
	/**
	 * Gets GassServer object which can receive SSL-encrypted data.
	 * 
	 * @return SSL mode of GassServer.
	 * @throws GrpcException if failed to get a GassServer.
	 */
	protected GassServer getGassServerSecure() throws GrpcException {
		return gassServerSecure;
	}
	
	/**
	 * Gets NgInformationManager object.
	 * 
	 * @return {@link NgInformationManager}.
	 * @throws GrpcException if failed to get a NgInformationManager.
	 */
	protected NgInformationManager getNgInformationManager()
		throws GrpcException {
		return informationManager;
	}
	
	/**
	 * Gets PortManager object which can receive non-encrypted data.
	 * 
	 * @return {@link PortManager}.
	 * @throws GrpcException if failed to get a PortManager.
	 */
	protected PortManager getPortManagerNoSecure() throws GrpcException {
		return portManagerNoSecure;
	}
	
	/**
	 * Gets PortManager object which can receive SSL-encrypted data.
	 * 
	 * @return {@link PortManager}.
	 * @throws GrpcException if failed to get a PortManager.
	 */
	protected PortManager getPortManagerSSL() throws GrpcException {
		return portManagerSSL;
	}
	
	/**
	 * Gets PortManager object which can receive GSI-encrypted data.
	 * 
	 * @return {@link PortManager}.
	 * @throws GrpcException if failed to get a PortManager.
	 */
	protected PortManager getPortManagerGSI() throws GrpcException {
		return portManagerGSI;
	}
	
	/**
	 * Starts GassServers. This methods starts 2 type of GassServers.
	 * Non-secure version of GassServer and secure version of GassServer.
	 * 
	 * @throws GrpcException if failed to start GassServers.
	 */
	private void startGassServer() throws GrpcException {
		/* start GASSServer */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#startGassServer(): start GASSServer.");
		try {
			/* secure version(https) */
			gassServerSecure = new GassServer(true, 0);
			/* http */
			gassServerNoSecure = new GassServer(false, 0);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		/* set GASSServer options */
		gassServerSecure.setOptions( GassServer.STDOUT_ENABLE |
			GassServer.STDERR_ENABLE | GassServer.READ_ENABLE |
			GassServer.WRITE_ENABLE);
		gassServerNoSecure.setOptions( GassServer.STDOUT_ENABLE |
			GassServer.STDERR_ENABLE | GassServer.READ_ENABLE |
			GassServer.WRITE_ENABLE);
	}
	
	/**
	 * Stops GassServers which were started in {@link startGassServer()}.
	 * 
	 * @throws GrpcException if failed to stop GassServers.
	 */
	private void stopGassServer() throws GrpcException {
		/* stop GASSServer */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL, 
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcClient#stopGassServer()");
		gassServerSecure.shutdown();
		gassServerNoSecure.shutdown();
	}
	
	/**
	 * Gets ID of this NgGrpcClient.
	 * 
	 * @return ID number of this.
	 */
	public int getID() {
		return ID;
	}
	
	/**
	 * Gets GrpcFunctionHandle or GrpcObjectHandle object
	 * which associated with executableID.
	 * 
	 * @param executableID ID of Grpc*Handle.
	 * @return Grpc*Handle if there is a object associated
	 * with executableID in this object.
	 * If there isn't the object, this returns null.
	 */
	public Object getHandle(int executableID) {
		/* get Grpc*Handle by executableID */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL, 
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcClient#getHandle()");
		
		Object handle;
		if ((handle = getFunctionHandle(executableID)) != null) {
			/* found FunctionHandle */
			return handle;
		}
		if ((handle = getObjectHandle(executableID)) != null) {
			/* found ObjectHandle */
			return handle;
		}
		
		/* not found handle */
		return null;
	}
	
	/**
	 * Gets GrpcFunctionHandle object which associated by executableID.
	 * 
	 * @param executableID ID of GrpcFunctionHandle.
	 * @return GrpcFunctionHandle if there is a object associated
	 * with executableID in this object.
	 * If there isn't the object, this returns null.
	 */
	public GrpcFunctionHandle getFunctionHandle(int executableID) {
		/* get GrpcFunctionHandle by executableID */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL, 
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcClient#getFunctionHandle()");
		
		/* search target in list */
		for (int i = 0; i < handleList.size(); i++) {
			NgGrpcFunctionHandle handle = null;
			try {
				handle = (NgGrpcFunctionHandle) handleList.get(i);
			} catch (ClassCastException e) {
				/* nothing will be done */
				continue;
			}
			if (handle.getExecutableID() == executableID) {
				/* found */
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					this,
					"NgGrpcClient#getFunctionHandle(): found target.");
				return handle;
			}
		}
		
		/* not found */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#getFunctionHandle(): not found target.");
		return null;
	}
	
	/**
	 * Gets GrpcObjectHandle object which associated by executableID.
	 * 
	 * @param executableID ID of GrpcObjectHandle.
	 * @return GrpcObjectHandle if there is a object associated
	 * with executableID in this object.
	 * If there isn't the object, this returns null.
	 */
	public GrpcObjectHandle getObjectHandle(int executableID) {
		/* get GrpcFunctionHandle by executableID */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL, 
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcClient#getObjectHandle()");
		
		/* search target in list */
		for (int i = 0; i < handleList.size(); i++) {
			NgGrpcObjectHandle handle = null;
			try {
				handle = (NgGrpcObjectHandle) handleList.get(i);
			} catch (ClassCastException e) {
				/* nothing will be done */
				continue;
			}
			if (handle.getExecutableID() == executableID) {
				/* found */
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					this,
					"NgGrpcClient#getObjectHandle(): found target.");
				return handle;
			}
		}
		
		/* not found */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcClient#getObjectHandle(): not found target.");
		return null;
	}
	
	/**
	 * Removes specified Grpc*Handle object from a list.
	 * 
	 * @param handle Grpc*Handle object to remove from a list.
	 */
	protected synchronized void removeHandle(Object handle) {
		NgGrpcHandle targetHandle = (NgGrpcHandle) handle;
		int targetHandleID = targetHandle.getExecutableID();
		int elemHandleID = -1;
		
		for (int i = 0; i < handleList.size(); i++) {
			Object elemHandle = handleList.get(i);
			if (elemHandle instanceof NgGrpcFunctionHandle) {
				/* function handle */
				NgGrpcFunctionHandle funcHandle = (NgGrpcFunctionHandle) elemHandle;
				elemHandleID = funcHandle.getExecutableID();
			} else if (elemHandle instanceof NgGrpcObjectHandle) {
				/* object handle */
				NgGrpcObjectHandle objHandle = (NgGrpcObjectHandle) elemHandle;
				elemHandleID = objHandle.getExecutableID();
			} else {
				/* error */
				continue;
			}
			
			/* check if it's the target */
			if (elemHandleID == targetHandleID) {
				/* target was found! remove it */
				handleList.remove(i);
				break;
			}
		}
	}
	
	/**
	 * Gets CommunicatorTable.
	 * 
	 * @return {@link CommunicatorTable}
	 */
	protected CommunicatorTable getCommTable() {
		return commTable;
	}

	/**
	 * Starts PortManagers.<br>
	 * This method starts 3 kind of PortManagers.<br>
	 * Non-encrypt, SSL-enabled and GSI-enabled PortManager.
	 * 
	 * @throws GrpcException if failed to start PortManagers.
	 */
	private void startPortManager() throws GrpcException {
		/* start PortManager */		
		try {
			informationManager.lockInformationManager();
			
			Properties localHostInfo =
				(Properties) informationManager.getLocalMachineInfo();
			
			/* PortManager without SSL */
			int port = Integer.parseInt((String) localHostInfo.get(
				NgInformationManager.KEY_CLIENT_LISTEN_PORT_RAW));
			portManagerNoSecure = new PortManager(this, false, port);

			/* PortManager with authonly is not implemented. */

			/* PortManager with GSI */
			port = Integer.parseInt((String) localHostInfo.get(
					NgInformationManager.KEY_CLIENT_LISTEN_PORT_GSI));	
			portManagerGSI = new PortManager(this, PortManager.CRYPT_GSI, port);

			/* PortManager with SSL */
			port = Integer.parseInt((String) localHostInfo.get(
					NgInformationManager.KEY_CLIENT_LISTEN_PORT_SSL));	
			portManagerSSL = new PortManager(this, PortManager.CRYPT_SSL, port);

		} catch (NumberFormatException e) {
			throw new NgInitializeGrpcClientException(e);
		} catch (IOException e) {
			throw new NgIOException(e);
		} finally {
			informationManager.unlockInformationManager();
		}
	}
	
	/**
	 * Stops PortManager which started in {@link startPortManager()}.
	 * 
	 * @throws GrpcException if it failed to stop PortManagers.
	 */
	private void stopPortManager() throws GrpcException {
		/* stop GASSServer */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL, 
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcClient#stopPortManager()");
		portManagerNoSecure.shutdown();
		portManagerSSL.shutdown();
		portManagerGSI.shutdown();
	}
	
	/**
	 * Generates ID of subJob.<br>
	 * Ninf-G provides the interface to create several Grpc*Handles at one time.<br>
	 * The jobs which are created by the method described above have same "JobID",
	 * so it's necessary another ID to identify these jobs.<br>
	 * "subJobID" is used as identifier of this.
	 * 
	 * @param jobID the ID associate with subJobID.
	 * @return Generated subJobID.
	 */
	protected synchronized int generateSubJobID(int jobID) throws GrpcException {
		Integer intSubJobID = (Integer)(mapNConnectBack.get(new Integer(jobID)));
		if (intSubJobID == null) {
			throw new NgInitializeGrpcHandleException ("specified Ninf-G Executable does not exist.");
		}
		int subJobID = intSubJobID.intValue();
		mapNConnectBack.put(new Integer(jobID), new Integer(subJobID + 1));

		Integer intNJobs = (Integer)(mapNJobs.get(new Integer(jobID)));
		int nJobs = intNJobs.intValue();
		if (subJobID >= nJobs) {
			throw new NgInitializeGrpcHandleException ("invalid subjob ID request.");
		}
		return subJobID;
	}
	
	/**
	 * @param jobID
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfoForJob(int jobID) {
		return (RemoteMachineInfo) mapJobRMInfo.get(new Integer(jobID));
	}
	
	/**
	 * read specified configfile.
	 * 
	 * @param configFile
	 * @throws GrpcException
	 */
	public void readConfig(String configFile) throws GrpcException {
		this.config = new NgConfig(configFile);
		informationManager.readConfigInformation(this.config);
	}
	
	/**
	 * create InvokeServerManager
	 * @throws GrpcException
	 */
	private void startInvokeServerManager() throws GrpcException {
		this.isMng = new NgInvokeServerManager(this);
	}

	/**
	 * destroy InvokeServerManager
	 * @throws GrpcException
	 */
	private void stopInvokeServerManager() throws GrpcException {
		this.isMng.dispose();
	}
	
	/**
	 * @param ngJob
	 * @return
	 * @throws GrpcException
	 */
	protected NgInvokeServer getInvokeServer(NgGrpcJob ngJob) throws GrpcException {
		return this.isMng.getInvokeServer(ngJob);
	}
	
	/**
	 * @param jobID
	 */
	protected void unregisterJob(int jobID) {
		mapNJobs.remove(new Integer(jobID));
		mapNConnectBack.remove(new Integer(jobID));
		mapJobRMInfo.remove(new Integer(jobID));
		
		/* remove ExecutableID from Properties */
		Enumeration keys = propExecutableID.keys();
		String targetKey = jobID + "/";
		while (keys.hasMoreElements()) {
			String elemKey = (String) keys.nextElement();
			if (elemKey.startsWith(targetKey)) {
				/* remove it */
				propExecutableID.remove(elemKey);
			}
		}
	}

	/**
	 * Put information about this instance.
	 * 
	 * @param indent a number of white spaces before information.
	 * @return String which indicates this NgGrpcClient.
	 */
	public String toString(int indent) {
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < indent; i++) {
			sb.append(" ");
		}
		String indentStr = sb.toString();
		sb = new StringBuffer();
		
		/* ID */
		sb.append(indentStr + "+ NgGrpcClient[" + ID + "]");
		sb.append("\n");
		
		/* Information Manager */
		sb.append(informationManager.toString(indent + NgGlobals.debugIndent));
		
		return sb.toString();
	}
}
	
