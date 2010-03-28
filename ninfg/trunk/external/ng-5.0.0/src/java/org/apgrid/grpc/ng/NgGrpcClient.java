/*
 * $RCSfile: NgGrpcClient.java,v $ $Revision: 1.14 $ $Date: 2008/02/23 05:43:29 $
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
import java.util.Iterator;
import java.util.Properties;

import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;
import org.gridforum.gridrpc.GrpcObjectHandle;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.HOSTNAME;
import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;


// ID generator
// this class designed MT-safe
import java.util.concurrent.atomic.AtomicInteger;
import java.util.Random;
class IdGenerator {

	private final AtomicInteger jobId;
	private final AtomicInteger executableId;
	private final Random random;

	public IdGenerator() {
		this.jobId = new AtomicInteger(1);
		this.executableId = new AtomicInteger(1);
		this.random = new Random();
	}

	// @return The number(0 ~ Integer.MAX_VALUE)
	public int nextJobId() {
		int next = -1;
		synchronized (jobId) {
			next = this.jobId.getAndIncrement();
			if (next == Integer.MAX_VALUE)
				this.jobId.set(0);
		}
		if (next < 0) throw new RuntimeException();
		return next;
	}

	// @return The number(0 ~ Integer.MAX_VALUE)
	public int nextExecutableId() {
		int next = -1;
		synchronized (executableId) {
			next = this.executableId.getAndIncrement();
			if (next == Integer.MAX_VALUE)
				this.executableId.set(0);
		}
		if (next < 0) throw new RuntimeException();
		return next;
	}

	// @return The number(1 ~ Integer.MAX_VALUE)
	public int nextAuthNo() {
		int r = this.random.nextInt(Integer.MAX_VALUE -1);
		r++;
		if (r <= 0)  throw new RuntimeException();
		return r;
	}

}

/**
 * Ninf-G implementation of GrpcClient interface.<br>
 * 
 * Provides functions defined in standard interfaces
 * and also provides Ninf-G special functions. 
 */
public final class NgGrpcClient implements GrpcClient {

	private int ID = 0;
	private final IdGenerator idGenerator = new IdGenerator();

	private Handles handleList = new Handles();
	private NgConfig config;
	private NgInformationManager informationManager;
	private PortManager portManagerNoSecure;
	private CommunicatorTable commTable;
	private NgInvokeServerManager isMng;
        private ClientCommunicationProxyManager clientCommProxyMgr;
	private NgLog ngLog;
	private boolean activated = false;


	private Map<String, Integer> executableIds =
		new HashMap<String, Integer>();
	private Map<Integer, Integer> authNumMap = new HashMap<Integer, Integer>();
	private Map<Integer, Integer> mapNConnectBack =
		new HashMap<Integer, Integer>();
	private Map<Integer, Integer> mapNJobs = new HashMap<Integer, Integer>();
	private Map<Integer,RemoteMachineInfo> mapJobRMInfo = 
	new HashMap<Integer,RemoteMachineInfo>();

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
	public synchronized void activate(String configFilename)
	 throws GrpcException {
		// check if configFilename were set in environment value
		String filename = getConfigFileName(configFilename);
		if (filename == null) {
			throw new NgInitializeGrpcClientException(
				"not specified configfile.");
		}
		config = new NgConfig(filename);

		_activate();

		activated = true;
	}

	/**
	 * Initialize with Properties object which contains information for
	 * server.<br>
	 * If "user.configFile" SystemProperty was set, use it as configuration
	 * file.<br>
	 * If it's not set, then use Properties as information of the server.
	 * 
	 * @param prop information of server.
	 * @throws GrpcException if it failed to initialize.
	 */
	public synchronized void activate(Properties prop) throws GrpcException {
		// check if configFilename were set in environment value
		if (System.getProperty(envConfigFilename) != null) {
			config = new NgConfig(System.getProperty(envConfigFilename));
		} else {
			config = new NgConfig(prop);
		}
		_activate();
	}

	private final String getConfigFileName(String name) {
		String sys_prop = System.getProperty(envConfigFilename);
		if (name != null) {
			return name;
		} else if (sys_prop != null) {
			return sys_prop;
		} else {
			return null;
		}
	}
	
	/**
	 * Initialize NgGrpcClient object.
	 * @throws GrpcException
	 */
	private final void _activate() throws GrpcException {
		// prepare Log facility
		ngLog = new NgLog(config.getLocalMachineInfo2());

		// create InformationManager
		informationManager = new NgInformationManager(this);
		informationManager.registerConfigInformation(config);

		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#activate(): created NgLog.");

		// create table of Communicator 
		commTable = new CommunicatorTable(this);

		// start PortManager 
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#activate(): start PortManager.");

		startPortManager();
		
		// start InvokeServerManager
		startInvokeServerManager();

		// start ClientCommunicationProxyManager
		startClientCommunicationProxyManager();
	}

///// getFunctionHandle[s] begin

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
	public GrpcFunctionHandle getFunctionHandle(String functionName,
	 Properties prop)
	 throws GrpcException {
		return getFunctionHandles(functionName, prop, 1)[0];
	}

	/**
	 * Creates {@link org.gridforum.gridrpc.GrpcFunctionHandle} 
	 * without server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section 
	 * in a configuration file will be used.
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
	 * Creates specified number of
	 *  {@link org.gridforum.gridrpc.GrpcFunctionHandle}
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
		if (nHandles < 0) {
			throw new IllegalArgumentException("nHandles is negative");
		}
		// create handle 
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#getFunctionHandles(): create handle.");

		String jobType = decideJobType(functionName);
		if ( isJobTypeMPI(jobType) ) {
			return createFuncHandlesBackendMPI(functionName, prop, nHandles);
		} else {
			// create Function Handles Backend Normal
			return createFuncHandles(functionName, prop, nHandles);
		} 
	}

	/*
	 * create NgFunctionHandles Backend Normal
	 * @return Array of NgFunctionHandle
	 */
	private NgGrpcFunctionHandle [] createFuncHandles(
	 String functionName,  Properties prop, int nHandles)
	 throws GrpcException {

		//int jobID = generateJobID(); // generate JobID 
		int jobID = idGenerator.nextJobId(); // generate JobID 
		NgGrpcFunctionHandle[] handles = new NgGrpcFunctionHandle[nHandles];

		setNumOfJobs(jobID, nHandles); // put nJobs into map
		setNumOfConnectBack(jobID, 0); // init num of connect back into map

		// generate Executable ID before connect back
		for (int i = 1; i < nHandles; i++) {
			generateAndSetExecutableID(jobID, i);
		}

		// get and put RemoteMachineInfo
		RemoteMachineInfo rmi = 
			informationManager.getRemoteMachineInfo(
				getTargetHost(prop, functionName));
		putRemoteMachineInfo(jobID, rmi);

		// create first function handle
		int exeId = generateAndSetExecutableID(jobID);
		generateAndSetAuthNo(jobID);
		handles[0] = 
			createFunctionHandle(functionName, prop, nHandles,
				jobID, exeId);

		handleList.add(handles[0]); // put handle into handle list

		// create handles 
		for (int i = 1; i < nHandles ; i++) {
			handles[i] = NgGrpcFunctionHandle.copy(handles[0]);
			handles[i].createStatus();
			handles[i].setExecutableID( getExecutableID(jobID, i) );
			handles[i].startCommunicationManager(); // thread run
			handleList.add(handles[i]);
		}
		return handles;
	}


	/*
	 * create NgFunctionHandles Backend MPI(&BLACS)
	 * @return Array of NgFunctionHandle
	 */
	private NgGrpcFunctionHandle [] createFuncHandlesBackendMPI(
	 String functionName,  Properties prop, int nHandles)
	 throws GrpcException {
 
		//int jobID = generateJobID(); // generate JobID 
		int jobID = idGenerator.nextJobId(); // generate JobID 
		int numJobs = 0;			 // number of Jobs

		NgGrpcFunctionHandle[] handles = new NgGrpcFunctionHandle[nHandles];

		setNumOfJobs(jobID, 1); // put nJobs into map 
		setNumOfConnectBack(jobID, 0); // init num of connect back into map 

		// number of Jobs 
		if (nHandles > 1) {
			numJobs = 1;
		} else if (nHandles == 1) {
			numJobs = NgGrpcJob.INVALID_JOB_ID;
		} else {
			throw new Error("");
		}

		// get and put RemoteMachineInfo
		String targetHost = getTargetHost(prop, functionName);
		putRemoteMachineInfo(jobID, 
			informationManager.getRemoteMachineInfo(targetHost));

		// create first function handle
		generateAndSetAuthNo(jobID);
		int exeId = generateAndSetExecutableID(jobID);
		handles[0] = 
			createFunctionHandle(functionName, prop, numJobs,
				jobID, exeId);

		// put handle into handle list
		handleList.add(handles[0]);

		// create handles for MPI and put them into handle list
		for (int i = 1; i < nHandles; i++) {
			//jobID = generateJobID(); // generate new JobID 
			jobID = idGenerator.nextJobId(); // generate new JobID 
			setNumOfJobs(jobID, 1); // put nJobs into map
			setNumOfConnectBack(jobID, 0); // init num of connect back into map

			exeId = generateAndSetExecutableID(jobID);
			handles[i] =
				createFunctionHandle(functionName, prop, 1, jobID, exeId);
 
			// put Job into map (handle list not?)
			putRemoteMachineInfo(jobID,
				informationManager.getRemoteMachineInfo(targetHost));
		}
		return handles;
	}

	/*
	 * create NgFunctionHandle
 	 */
	private NgGrpcFunctionHandle createFunctionHandle(
	 String functionName, Properties prop, int numJobs, int jobId, int ExecId)
	 throws GrpcException {
		if (prop == null) {
			return new NgGrpcFunctionHandle(functionName, this,
					jobId, ExecId, numJobs);
		} else {
			return new NgGrpcFunctionHandle(functionName, prop, this,
					jobId, ExecId, numJobs);
		}
	}

	/**
	 * Create specified number of
	 *  {@link org.gridforum.gridrpc.GrpcFunctionHandle}
	 * without server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section in
	 *  a configuration file will be used.
	 * 
	 * @param functionName a name of function.
	 * @param nHandles a number of FunctionHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 */
	public GrpcFunctionHandle[] getFunctionHandles(
	 String functionName, int nHandles)
	 throws GrpcException {
		return getFunctionHandles(functionName, null, nHandles);
	}

///// getFunctionHandle[s] end

///// getObjectHandle[s] begin

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
	public GrpcObjectHandle getObjectHandle(String objectName, Properties prop)
	 throws GrpcException {
		return getObjectHandles(objectName, prop, 1)[0];
	}

	/**
	 * Creates {@link org.gridforum.gridrpc.GrpcObjectHandle} without
	 *  server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section in 
	 *  a configuration file will be used.
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
	 * Creates specified number of
	 *  {@link org.gridforum.gridrpc.GrpcObjectHandle}
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
	public GrpcObjectHandle[] getObjectHandles(String objectName,
	 Properties prop, int nHandles)
	 throws GrpcException {
		if (nHandles < 0) {
			throw new IllegalArgumentException("nHandles is negative");
		}

		//int jobID = generateJobID(); // generate JobID
		int jobID = idGenerator.nextJobId(); // generate JobID
		int numJobs = 0; // number of Jobs

		// create handle
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#getObjectHandles(): create handle.");

		NgGrpcObjectHandle[] handles = new NgGrpcObjectHandle[nHandles];

		String jobType = decideJobType(objectName);
		
		// create handle for MPI 
		if ( isJobTypeMPI(jobType) ) {

			// put nJobs into map
			setNumOfJobs(jobID, 1);

			// init num of connect back into map
			setNumOfConnectBack(jobID, 0);

			// number of Jobs 
			if (nHandles > 1) {
				numJobs = 1;
			}else if (nHandles == 1) {
				numJobs = NgGrpcJob.INVALID_JOB_ID;
			} else {
				throw new NgException("Invalid number of handles: " + nHandles);
			}
		}
		// It's not for MPI
		else {
			// put nJobs into map
			setNumOfJobs(jobID, nHandles);

			// init num of connect back into map
			setNumOfConnectBack(jobID, 0);

			// generate Executable ID before connect back
			for (int i = 1; i < nHandles; i++) {
				generateAndSetExecutableID(jobID, i);
			}
			// number of Jobs 
			numJobs = nHandles;
		}

		// get and put RemoteMachineInfo
		String targetHost = getTargetHost(prop, objectName);
		putRemoteMachineInfo(jobID,
			informationManager.getRemoteMachineInfo(targetHost));
		
		// create first handle
		generateAndSetAuthNo(jobID);
		int exeId = generateAndSetExecutableID(jobID);
		handles[0] =
			createObjectHandle(objectName, prop, numJobs,
				jobID, exeId);

		// put handles into handle list
		handleList.add(handles[0]);

		// create handles for MPI and put them into handle list 
		if ( isJobTypeMPI(jobType)) {
			for (int i = 1; i < nHandles; i++) {

				//jobID = generateJobID(); // generate new JobID 
				jobID = idGenerator.nextJobId(); // Generate new JobID 
				setNumOfJobs(jobID, 1); // put nJobs into map
				setNumOfConnectBack(jobID, 0); // init num of connect back into map

				exeId = generateAndSetExecutableID(jobID);
				handles[i] =
					createObjectHandle(objectName, prop, 1,
						jobID, exeId);

				// put Job into map
				putRemoteMachineInfo(Integer.valueOf(jobID),
					informationManager.getRemoteMachineInfo(targetHost));
			}
		}
		// It's not for MPI, clone handles and put them into handle list 
		else {
			for (int i = 1; i < nHandles ; i++) {
				handles[i] = NgGrpcObjectHandle.copy(handles[0]);
				handles[i].setExecutableID(getExecutableID(jobID, i));
				handles[i].startCommunicationManager();
				handleList.add(handles[i]);
			}
		}
		return handles;

	}

	/**
	 * Creates specified number of
	 *  {@link org.gridforum.gridrpc.GrpcObjectHandle}
	 * without server information.<br>
	 * The server which described at the 1st &lt SERVER &gt section in
	 *  a configuration file will be used.
	 * 
	 * @param objectName a name of object.
	 * @param nHandles a number of ObjectHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 */
	public GrpcObjectHandle[] getObjectHandles(String objectName, int nHandles)
	 throws GrpcException {
		return getObjectHandles(objectName, null, nHandles);
	}

	/*
	 * create NgObjectHandle
	 */
	private NgGrpcObjectHandle createObjectHandle(
	 String objectName, Properties prop, int numJobs, int jobId, int ExecId)
	 throws GrpcException {
		if (prop == null) {
			return new NgGrpcObjectHandle(objectName, this,
					jobId, ExecId, numJobs);
		} else {
			return new NgGrpcObjectHandle(objectName, prop, this,
					jobId, ExecId, numJobs);
		}
	}

///// getObjectHandle[s] end

	/*
	 * @param prop Ninf-G Handle Attribute
	 * @param name(experimental) using query Default Server Name
	 */
	private String getTargetHost(Properties prop, String name)
	 throws GrpcException {
		if (prop != null) {
			String ret = prop.getProperty(HOSTNAME);
			if (ret != null) {
				return ret;
			}
		}
		// Query default server machine name
		Properties tmpProp =
			informationManager.getDefaultRemoteMachineProperties(name);
		return tmpProp.getProperty(HOSTNAME);
	}

	/*
	 * @return Job type String (example "MPI")
	 *        if not specified Remote Class Information, return value is null
	 */
	private String decideJobType(String functionName)
	 throws GrpcException {
		RemoteClassInfo rmtClassInfo =
			informationManager.getClassInfo(functionName);
		if ( rmtClassInfo != null ) {
			return rmtClassInfo.getBackend();
		}
		return null;
	}

	/*
	 * @return true Argument type equals MPI
	 *         false Argument type is not equals MPI (Normal Job)
	 */
	private boolean isJobTypeMPI(String type) {
		if (type == null) { return false; }
		if (type.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			type.equals(RemoteMachineInfo.VAL_BACKEND_BLACS) ) {
			return true;
		}
		return false;
	}

	/* (non-Javadoc)
	 * @see org.gridforum.gridrpc.GrpcClient#deactivate()
	 */
	public void deactivate() throws GrpcException {
		if (! activated) {
			if (ngLog != null)
				ngLog.logDebug(CAT_NG_INTERNAL, logHeader()
					+ "NgGrpcClient is not activate.");
			return;
		}
		if (ngLog == null) {
			throw new NullPointerException("ngLog is null");
		}
		ngLog.logDebug(CAT_NG_INTERNAL, logHeader() 
			+ "NgGrpcClient#deactivate()");

		stopPortManager();
		stopInvokeServerManager();
		stopInformationManager();
                clientCommProxyMgr.dispose();
	}


	/**
	 * Generates ID of the RemoteExecutable.
	 * 
	 * @param jobID ID of the Job.
	 * @return ID of the RemoteExecutable.
	 */
	private int generateAndSetExecutableID(int jobID) {
		return generateAndSetExecutableID(jobID, 0);
	}
	
	/**
	 * Generates ID of the RemoteExecutable.
	 * 
	 * @param jobID ID of the Job.
	 * @param subJobID a rank of this in list of Jobs.
	 * @return ID of the RemoteExecutable.
	 */
	private int generateAndSetExecutableID(int jobID, int subJobID){
		// generate executableID
		int executableID = idGenerator.nextExecutableId();

		// register into map<String, Integer> 
		executableIds.put(makeExecutableIDKey(jobID, subJobID),
						executableID);

		return executableID;
	}

	private int generateAndSetAuthNo(int jobID) {
		int authno = idGenerator.nextAuthNo();
		authNumMap.put(jobID, authno);
		return authno;
	}

	public int getAuthNum(int jobID) {
		Integer ret = this.authNumMap.get(jobID);
		if (ret == null)
			throw new IllegalStateException("get(" + jobID + ") is null");
		return ret;
	}

	/**
	 * Gets ID of the RemoteExecutable specified by jobID and subJobID.
	 * 
	 * @param jobID ID of the Job.
	 * @return ID of the RemoteExecutable.
	 */
	public int getExecutableID(int jobID) throws GrpcException {
		// get SubJobId
		int subJobID = generateSubJobID(jobID);
		return getExecutableID(jobID, subJobID);
	}

	/*
	 * Gets ID of the RemoteExecutable specified by jobID and subJobID.
	 * @param jobID ID of the Job.
	 * @param subJobID a rank of this in list of Jobs.
	 */
	private int getExecutableID(int jobID, int subJobID) {
		// get executableID by jobID and subJobID
		String key = makeExecutableIDKey(jobID, subJobID);
		if (executableIds.containsKey(key)) {
			return executableIds.get(key);
		} else {
			return -1; // Invalid handle
		}
	}

	/**
	 * Make ID string of the RemoteExecutable specified by jobID
	 *  and subJobID.
	 * 
	 * @param jobID ID of the Job.
	 * @param subJobID a rank of this in list of Jobs.
	 * @return ID string of the RemoteExecutable.
	 */
	private String makeExecutableIDKey(int jobID, int subJobID) {
		return jobID +"/" + subJobID;
	}
	
	/**
	 * Gets NgConfig object which is used in this object.
	 * 
	 * @return {@link org.apgrid.grpc.ng.NgConfig}
	 */
	protected NgConfig getConfig() {
		return config;
	}
	
	/**
	 * Gets NgLog object which has information of putting log messages.
	 * 
	 * @return {@link org.apgrid.grpc.ng.NgLog}
	 */
	protected NgLog getNgLog() {
		return this.ngLog;
	}
	
	/**
	 * Gets NgInformationManager object.
	 * 
	 * @return {@link NgInformationManager}.
	 */
	protected NgInformationManager getNgInformationManager() {
		return this.informationManager;
	}

	public LocalMachineInfo getLocalMachineInfo() {
		return informationManager.getLocalMachineInfo();
	}
	
	/**
	 * Gets PortManager object which can receive non-encrypted data.
	 * 
	 * @return {@link PortManager}.
	 */
	protected PortManager getPortManagerNoSecure() {
		return this.portManagerNoSecure;
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
		// get Grpc*Handle by executableID 
		ngLog.logDebug(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#getHandle()");

		Object handle;
		if ((handle = getFunctionHandle(executableID)) != null) {
			return handle;// found FunctionHandle 
		}
		if ((handle = getObjectHandle(executableID)) != null) {
			return handle;// found ObjectHandle
		}

		// not found handle
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
		ngLog.logDebug(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#getFunctionHandle()");
		
		// search target in list
		NgGrpcFunctionHandle handle =
			handleList.getFunctionHandle(executableID);
		if (handle != null) { return handle; }
		
		// not found
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#getFunctionHandle(): not found target.");
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
		ngLog.logDebug(CAT_NG_INTERNAL,  logHeader()
			+ "NgGrpcClient#getObjectHandle()");
		
		// search target in handle list 
		NgGrpcObjectHandle handle =
			handleList.getObjectHandle(executableID);
		if (handle != null) { return handle; }

		// not found 
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#getObjectHandle(): not found target.");
		return null;
	}

	/**
	 * Removes specified Grpc*Handle object from a list.
	 * 
	 * @param handle Grpc*Handle object to remove from a list.
	 */
	protected synchronized void removeHandle(Object handle) {
		handleList.remove(handle);
	}

	/**
	 * Gets CommunicatorTable.
	 * 
	 * @return {@link CommunicatorTable}
	 */
	protected CommunicatorTable getCommTable() {
		return this.commTable;
	}

	/**
	 * Starts PortManagers.<br>
	 * 
	 * @throws GrpcException if failed to start PortManagers.
	 */
	private void startPortManager() throws GrpcException {
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() + "startPortManager");
		try {
			LocalMachineInfo localHostInfo =
				informationManager.getLocalMachineInfo();

			// all-round(but not secure) PortManager
			int port = Integer.parseInt(localHostInfo.getListenPort());
			portManagerNoSecure = new PortManager(this, port);
		} catch (NumberFormatException e) {
			throw new NgInitializeGrpcClientException(e);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * Stops PortManager which started in {@link startPortManager()}.
	 * 
	 * @throws GrpcException if it failed to stop PortManagers.
	 */
	private void stopPortManager() throws GrpcException {
		ngLog.logDebug(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcClient#stopPortManager()");
		portManagerNoSecure.shutdown();
	}
	
	/**
	 * Generates ID of subJob.<br>
	 * Ninf-G provides the interface to create several Grpc*Handles at 
	 * one time.<br>
	 * The jobs which are created by the method described above have 
	 * same "JobID", so it's necessary another ID to identify these jobs.<br>
	 * "subJobID" is used as identifier of this.
	 * 
	 * @param jobID the ID associate with subJobID.
	 * @return Generated subJobID.
	 */
	private synchronized int generateSubJobID(int jobID)
	 throws GrpcException {
		if (! hasConnectBack(jobID) ) {
			throw new NgInitializeGrpcHandleException(
						"specified Ninf-G Executable does not exist.");
		}
		// increment
		int subJobID = getNumOfConnectBack(jobID);
		setNumOfConnectBack(jobID, subJobID + 1);

		int nJobs = mapNJobs.get(jobID); // use auto-boxing
		if (subJobID >= nJobs) {
			throw new NgInitializeGrpcHandleException(
				"invalid subjob ID request.");
		}
		return subJobID;
	}

	/**
	 * @param jobID
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfoForJob(int jobID) {
		return (RemoteMachineInfo) mapJobRMInfo.get(Integer.valueOf(jobID));
	}
	
	/**
	 * read specified configfile.
	 * 
	 * @param configFile
	 * @throws GrpcException
	 */
	public void readConfig(String configFile) throws GrpcException {
		stopInformationManager();
		this.isMng.deactivate();
		this.clientCommProxyMgr.deactivate();
		this.config = new NgConfig(configFile);
		informationManager = new NgInformationManager(this);
		informationManager.registerConfigInformation(this.config);
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
         * create ClientCommunicationProxyManager
         */
        private void startClientCommunicationProxyManager() {
            this.clientCommProxyMgr 
                = new ClientCommunicationProxyManager(this);
        }

	private void stopInformationManager() {
		this.informationManager.dispose();
	}
	
	/**
	 * @param ngJob
	 * @return
	 * @throws GrpcException
	 */
	protected NgInvokeServer getInvokeServer(NgGrpcJob ngJob)
	 throws GrpcException {
		return this.isMng.getInvokeServer(ngJob);
	}

	/**
	 * @param jobID
	 */
	public synchronized void unregisterJob(int jobID) {
		mapNJobs.remove(Integer.valueOf(jobID)); // Integer?
		mapNConnectBack.remove(Integer.valueOf(jobID));
		mapJobRMInfo.remove(Integer.valueOf(jobID));

		// remove ExecutableID from Map
		removeExecID( jobID + "/" );
	}

	private void removeExecID(String targetKey) {
		Iterator<String> itr = executableIds.keySet().iterator();
		while (itr.hasNext()) {
			String key = itr.next();
			if ( key.startsWith(targetKey) ) {
				itr.remove();
			}
		}
	}


	/**
	 * Return a string representation of the object.
	 */
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("NgGrpcClient[" + ID + "]\n");
		sb.append(informationManager);
		return sb.toString();
	}

	// tentative
	public String logHeader() {
		return "Context [" + String.valueOf(ID) + "]: ";
	}

///// Private methods for mapJobRMInfo

	private void putRemoteMachineInfo(Integer jobID, RemoteMachineInfo rmi) {
		mapJobRMInfo.put(jobID, rmi);
	}

///// Private methods for mapNJobs

	private synchronized void setNumOfJobs(int jobId, int num) {
		mapNJobs.put(jobId, num);
	}

///// Private methods for mapNConnectBack

	private boolean hasConnectBack(int jobId) {
		return mapNConnectBack.containsKey(jobId);
	}

	private int getNumOfConnectBack(int jobId) {
		 return mapNConnectBack.get(jobId);
	}

	/*
	 * @param subjobId maybe 
	 */
	private synchronized void setNumOfConnectBack(int jobId, int subjobId) {
		mapNConnectBack.put(jobId, subjobId);
	}


	/**
	 * Put information about this instance.
	 * 
	 * @param indent a number of white spaces before information.
	 * @return String which indicates this NgGrpcClient.
	 */
	public String toString(int indent) {
		String indentStr = mkIndentString(indent);
		StringBuffer sb = new StringBuffer();
		sb.append(indentStr + "+ NgGrpcClient[" + ID + "]\n");
		sb.append(informationManager.toString(indent + NgGlobals.debugIndent));
		return sb.toString();
	}

	private String mkIndentString(int indent) {
		if (indent <= 0) { return ""; }
		char [] c = new char[indent];
		for (int i = 0; i < c.length ; i++) {
			c[i] = ' ';
		}
		return new String(c);
	}
  
  public  
  ClientCommunicationProxyManager getClientCommunicationProxyManager() {
    return clientCommProxyMgr;
  }
}

