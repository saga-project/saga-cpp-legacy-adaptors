/*
 * $RCSfile: NgGrpcHandle.java,v $ $Revision: 1.26 $ $Date: 2008/03/13 09:54:18 $
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
import java.util.Properties;
import java.util.ArrayList;

import org.apgrid.grpc.ng.info.*;
/////////
import org.apgrid.grpc.ng.protocol.ProtCancelSessionRequest;
import org.apgrid.grpc.ng.protocol.ProtExitExeRequest;
import org.apgrid.grpc.ng.protocol.ProtInvokeCallbackNotify;
import org.apgrid.grpc.ng.protocol.ProtInvokeSessionRequest;
import org.apgrid.grpc.ng.protocol.ProtPullbackSessionReply;
import org.apgrid.grpc.ng.protocol.ProtPullbackSessionRequest;
import org.apgrid.grpc.ng.protocol.ProtQueryExeStatusReply;
import org.apgrid.grpc.ng.protocol.ProtQueryExeStatusRequest;
import org.apgrid.grpc.ng.protocol.ProtQueryFunctionInfoReply;
import org.apgrid.grpc.ng.protocol.ProtQueryFunctionInfoRequest;
import org.apgrid.grpc.ng.protocol.ProtResetExeRequest;
import org.apgrid.grpc.ng.protocol.ProtResumeSessionRequest;
import org.apgrid.grpc.ng.protocol.ProtSuspendSessionRequest;
import org.apgrid.grpc.ng.protocol.ProtTransferArgumentRequest;
import org.apgrid.grpc.ng.protocol.ProtTransferCallbackArgumentReply;
import org.apgrid.grpc.ng.protocol.ProtTransferCallbackArgumentRequest;
import org.apgrid.grpc.ng.protocol.ProtTransferCallbackResultRequest;
import org.apgrid.grpc.ng.protocol.ProtTransferResultReply;
import org.apgrid.grpc.ng.protocol.ProtTransferResultRequest;
import org.apgrid.grpc.ng.protocol.ProtConnectionCloseRequest;
/////////
import org.apgrid.grpc.util.GrpcTimer;
import org.apgrid.grpc.util.CondWait;
import org.gridforum.gridrpc.GrpcException;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.HOSTNAME;
import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;


/**
 * Provides interfaces for Grpc*Handle class.<br>
 * This class is used internally, you should use
 * NgGrpcFunctionHandle or NgGrpcObjectHandle.<br>
 * 
 * And provides variables of status code.<br>
 * It's used in {@link NgGrpcFunctionHandle#getLocalStatus()} and 
 * {@link NgGrpcObjectHandle#getLocalStatus()}.
 */
public class NgGrpcHandle implements Cloneable {
	private NgGrpcClient context;
	private int executableID;
	private int sessionID;
	private int methodID;
	private SessionInformation sessionInfo;
	private RemoteMachineInfo remoteMachineInfo;
	private RemoteClassPathInfo classPathInfo;
	private RemoteClassInfo remoteClassInfo;
	private RemoteMethodInfo remoteMethodInfo;
	private NgGrpcJob job;
	private NgGrpcExecInfo execInfo;
	private GrpcTimer timer;
	private ClientStatus status = new ClientStatus();

	private CommunicationManager commManager;
	private boolean keep_connection;

	private String hostName;
	private String methodName;
	private String className;
	private NgLog ngLog;
	private Properties propGrpcHandleAttr;
	private int[] intArguments;
	private boolean isCanceled;
	private boolean isLocked;
	private long jobStartTimeout;
	private Properties propSessionAttr;
	
	private int jobID;
	private int jobCount;

	// for callback 
	private List callbackFunc;
	private List callbackFuncInfo;

	// protocol version 
	private Version version;

	// for cancel
	private CondWait canceled;

	// Communication Proxy Type
	private String commProxyType;

	/**
	 * Creates NgGrpcHandle.
	 */
	protected NgGrpcHandle() {
		// nothing will be done 
	}
	
	/**
	 * Creates NgGrpcHandle without any server information.<br>
	 * Default server information(described at 1st &lt SERVER_INFO &gt section
	 * in a configuration file) will be used as server.  
	 * 
	 * @param className a name of RemoteFunction/RemoteObject.
	 * @param context NgGrpcClient.
	 * @param jobID ID of Job.
	 * @param executableID ID of executable.
	 * @param jobCount a number of Jobs.
	 * @throws GrpcException if failed to create handle.
	 */
	protected NgGrpcHandle(String className, NgGrpcClient context,
	 int jobID,
	 int executableID,
	 int jobCount)
	 throws GrpcException {

		NgInformationManager infoMgr = context.getNgInformationManager();
		Properties tmpProp = null;
		tmpProp =
			getDefaultServerProperties(
				infoMgr.getDefaultRemoteMachineProperties(className));

		initialize(className, tmpProp, context, jobID, executableID, jobCount);
		activate();	
	}

	/**
	 * Creates NgGrpcHandle.<br>
	 * Attribute variables of the server must set in prop.<br> 
	 * 
	 * @param className a name of RemoteFunction/RemoteObject.
	 * @param prop attribute variables of the server.
	 * @param context NgGrpcClient.
	 * @param jobID ID of Job.
	 * @param executableID ID of executable.
	 * @param jobCount a number of Jobs.
	 * @throws GrpcException if failed to create handle.
	 * @see NgGrpcHandleAttr
	 */
	protected NgGrpcHandle(String className, Properties prop,
	 NgGrpcClient context,
	 int jobID, int executableID, int jobCount)
	 throws GrpcException {

		initialize(className, prop, context, jobID, executableID, jobCount);
		activate();
	}

	/**
	 * Initialize Handle.
	 * 
	 * @param className
	 * @param prop
	 * @param context
	 * @param jobID
	 * @param executableID
	 * @param jobCount
	 * @throws GrpcException
	 */
	private final void initialize(String aClassName, Properties attr,
	 NgGrpcClient context, int jobID, int executableID, int jobCount)
	 throws GrpcException {
		this.context = context;
		this.jobID = jobID;
		this.executableID = executableID;
		this.jobCount = jobCount;
		this.execInfo = new NgGrpcExecInfo();
		this.timer = new GrpcTimer();
		this.intArguments = null;
		this.isCanceled = false;
		this.isLocked = false;
		this.ngLog = this.context.getNgLog();


		if (attr == null) {
			throw new NgInitializeGrpcHandleException(
				"Invalid ServerInformation.");
		}

		// get server name
		this.hostName = getServerHostname(attr, aClassName);
		if (this.hostName == null) {
			throw new NgInitializeGrpcHandleException("Invalid hostname");
		}

		// get class name 
		this.className = getRemoteClassname(attr, aClassName);
		if (this.className == null) {
			throw new NgInitializeGrpcHandleException("Invalid classname");
		}

		// set attribute for GrpcHandle
		this.propGrpcHandleAttr = attr;
	}

	protected void createStatus() {
		status = new ClientStatus();
		status.set(ClientStatus.IDLE);
	}

	private final String getServerHostname(Properties prop, String aClassName)
	 throws GrpcException {
		String hostname = prop.getProperty(NgGrpcHandleAttr.KEY_HOSTNAME);
		if (hostname != null) {
			return hostname;
		}
		// If hostname is not specified, use default server
		return getDefaultServerHostname(aClassName);
	}


	private final String getDefaultServerHostname(String aClassName) 
	 throws GrpcException {
		Properties tmpProp = null;
		NgInformationManager infoMgr = context.getNgInformationManager();
		tmpProp = infoMgr.getDefaultRemoteMachineProperties(aClassName);
		if (tmpProp != null) {
			return tmpProp.getProperty(NgGrpcHandleAttr.KEY_HOSTNAME);
		}
		return null;
	}


	private String getRemoteClassname(Properties prop, String classname) {
		String ret = prop.getProperty(NgGrpcHandleAttr.KEY_CLASSNAME);
		if (ret != null) {
			return ret;
		}
		return classname;
	}



	/**
	 * Initialize NgGrpcHandle.<br>
	 * Searches information of the server and RemoteFunction/RemoteObject.<br>
	 * And invokes RemoteFunction/RemoteObject.<br>
	 * Then starts Thread for CommunicationManager and Timer of HeartBeat.
	 * 
	 * @throws GrpcException if it failed to initialize.
	 */
	private void activate() throws GrpcException {
		status.set(ClientStatus.INIT); // set status

		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() 
			+ "NgGrpcHandle#activate(): activate handle.");
			
		// lookup information
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() 
			+ "NgGrpcHandle#activate(): search information.");

		timer.start();
		NgInformationManager infoManager = context.getNgInformationManager();

		// --- Get RemoteMachineInfo ---
		RemoteClassPathInfo rcPath = null;
		rcPath = getRemoteClassPathInfo(infoManager);
		if (rcPath == null) {
			throw new NgInitializeGrpcHandleException(
				"can't get RemoteClassPathInfo for " + hostName + " " 
				+ className );
		}

		// --- Set RemoteMachineInfo(<SERVER>info)
		setRemoteMachineInfo(infoManager);
		keep_connection = Boolean.valueOf(
			remoteMachineInfo.getKeepConnection());


		commProxyType = remoteMachineInfo.getCommunicationProxyInfo()
                    .getCommunicationProxy();

		// set time to lookup Remote Server information
		execInfo.setLookupServerInfoTime(timer.getElapsedTime());

		// --- Set RemoteClassInfo ---
		timer.start();
		setRemoteClassInfo(infoManager);

		// set time to lookup Remote Class information
		execInfo.setLookupClassInfoTime(timer.getElapsedTime());

		// set attribute of Handle
		setMpiNCpus();

		// invoke GrpcJob 
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
			+ "NgGrpcHandle#activate(): invoke Ninf-G Executable.");

		// set timeout of job start
		setJobStartTimeout();

		// invoke JOB
		timer.start();
		String jobType = getJobType(rcPath);
		invokeJob(jobType);

		// set time to invoke job
		execInfo.setInvokeTime(timer.getElapsedTime());

		// start CommunicationManager, this will wait for active
		startCommunicationManager();

		// set protocol version
		this.version = commManager.getVersion();

		status.set(ClientStatus.IDLE);
	}

	private final RemoteClassPathInfo getRemoteClassPathInfo(
	 final NgInformationManager infoManager)
	 throws GrpcException {
		return infoManager.getClassPathInfo(hostName, className);
	}

	private final void setRemoteMachineInfo(
	 final NgInformationManager infoManager)
	 throws GrpcException {
		this.remoteMachineInfo =
			infoManager.getRemoteMachineInfoCopy(this.hostName);
	}

	private final void setRemoteClassInfo(
	 final NgInformationManager infoManager)
	 throws GrpcException {
		if (this.remoteMachineInfo == null)
			throw new IllegalStateException("remoteMachineInfo field is null");

		String server_tag = this.remoteMachineInfo.getTag();
		this.remoteClassInfo =
			infoManager.getClassInfo(
				this.className, this.hostName, server_tag);
	} 

	private final void setMpiNCpus() throws GrpcException {
		if ( this.propGrpcHandleAttr.containsKey(NgGrpcHandleAttr.KEY_MPI_NCPUS) ) {
			this.remoteMachineInfo.resetNumCPUs();
			this.propGrpcHandleAttr =
				NgGrpcHandleAttr.convertNumOfCPUs(this.propGrpcHandleAttr);
		}
		//// update the RemoteMachineInfo
		this.remoteMachineInfo.update(this.propGrpcHandleAttr);
	}

	private final void setJobStartTimeout() throws GrpcException {
		String jobStartTimeout = this.remoteMachineInfo.getRslInfo().getJobStartTimeout();
		this.jobStartTimeout = Long.parseLong(jobStartTimeout) * 1000;
		if (this.jobStartTimeout < 0) {
			throw new NgInitializeGrpcHandleException(
				"job_startTimeout is invalid.");
		}
	}

	private String getJobType(RemoteClassPathInfo rcpath) {
		String jobType = null;
		if (remoteClassInfo != null) {
			jobType = remoteClassInfo.getBackend();
		}
		if (rcpath != null) {
			String type = rcpath.getBackend();
			if (type != null
				 && (type.equals(RemoteMachineInfo.VAL_BACKEND_MPI)
				 || type.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {
				jobType = type;
			}
		}
		return jobType;
	}

	/*
	 * Invoke Ninf-G Executable
	 * @param jobType Job Backend Type
	 */
	private final void invokeJob(String jobType)
	 throws GrpcException {
		this.job = new NgGrpcJob(
			this.context, this.remoteMachineInfo, this.hostName,
			this.className, this.jobID, jobType, this.jobCount);
		int authNo = this.context.getAuthNum(this.jobID);
		this.job.setSimpleAuthNo(authNo);
		this.job.create();
	}

	/**
	 * Starts CommunicationManager to manager send/receive Protocol.
	 * 
	 * @throws GrpcException if failed to start CommunicationManager.
	 */
	protected void startCommunicationManager() throws GrpcException {
		// start CommunicationManager
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() 
			+ "NgGrpcHandle#startCommunicationManager(): Start CommunicationManager.");
		try {
			commManager = new CommunicationManager(context, this);
		} catch (GrpcException e) {
			job.setRequiredJobCancel();
			job.incrementExitCount();
			throw e;
		}
		new Thread(commManager).start();
	}

	private void restartCommunicationManager() throws GrpcException {
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader()
	    		+ "NgGrpcHandle#restartCommunicationManager(): Restart Communication Manager.");
	    	new Thread(commManager).start();
	}

//// Initialize methods end ////


	/**
	 * Cancels a current session.
	 * 
	 * @throws GrpcException if failed to cancel.
	 */
	protected void cancel() throws GrpcException {
		this.canceled = new CondWait();
		this.isCanceled = true;
		this.canceled.waitFor();
	}
	
	/**
	 * Sends request of cancel and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to cancel.
	 */
	private void cancelSession() throws GrpcException {
		lockHandle();
		try {
			// set cancel flag
			this.isCanceled = true;

			if ((status.get() == ClientStatus.NONE) ||
				(status.get() == ClientStatus.INIT) ||
				(status.get() == ClientStatus.TRANSRES)) {
				// nothing will be done 
				return;
			} else if (status.get() == ClientStatus.IDLE) {
				this.isCanceled = false; // reset isCanceled flag 
				return;
			}

			// send and receive Cancel Protocol
			ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
				+ "NgGrpcHandle#cancelSession(): cancel Session.");
			NgProtocolRequest cancelRequest =
				new ProtCancelSessionRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version);
			Protocol cancelReply = 
				commManager.sendProtocol(cancelRequest);

			// check result of Reply 
			checkResult(cancelReply);

			status.set(ClientStatus.CANCEL);
		} finally {
			unlockHandle();
		}
	}

	/**
	 * Sends request of dispose and wait for the reply.<br>
	 * And stops CommunicationManager and Timer of HeartBeat.
	 * 
	 * @throws GrpcException if it failed to dispose.
	 */
	protected void dispose() throws GrpcException {
		lockHandle();
		try {
			if (status.get() != ClientStatus.IDLE) {
				// invalid status 
				ngLog.logError(CAT_NG_INTERNAL, logHeader() + 
					"NgGrpcHandle#dispose(): invalid status. " + status);
				throw new NgFinalizeGrpcHandleException("failed to dispose");
			}

			// dispose handle 
			ngLog.logInfo(CAT_NG_INTERNAL, logHeader() + 
				"NgGrpcHandle#dispose(): dispose Handle.");
			NgProtocolRequest exitExeRequest =
				new ProtExitExeRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version);
			Protocol exitExeReply = 
				commManager.sendProtocol(exitExeRequest);
			checkResult(exitExeReply); // check result of Reply 
		} catch (GrpcException e) {
			job.setRequiredJobCancel(); // set requiredJobCancel

			ngLog.logInfo(CAT_NG_INTERNAL, logHeader() 
				+ "NgGrpcHandle#dispose(): set Require JobCancel .");
		} finally {
			status.set(ClientStatus.NONE); // set status
			job.incrementExitCount();      // increment exit count
			context.removeHandle(this);	   // remove this from handle list
			unlockHandle();
		}

		// wait for done 
		String jobStopTimeout = remoteMachineInfo.getRslInfo().getJobStopTimeout();
		long stopTimeout = Long.parseLong(jobStopTimeout) * 1000;
		if (stopTimeout < 0) {
			ngLog.logDebug(CAT_NG_INTERNAL, logHeader() 
				+ "NgGrpcHandle#dispose(): wait for job["
				+ job.getRequestID() + "] done.");
			job.waitForDone();
		} else if (stopTimeout == 0) {
			ngLog.logDebug(CAT_NG_INTERNAL, logHeader() 
				+ "NgGrpcHandle#dispose(): don't wait job done.");
		} else {
			ngLog.logDebug(CAT_NG_INTERNAL, logHeader() 
				+ "NgGrpcHandle#dispose(): wait for job["
				+ job.getRequestID() + "] done(" + stopTimeout + "sec).");
			if (job.waitForDone(stopTimeout) != 0) {
				ngLog.logWarn(CAT_NG_INTERNAL, logHeader() +
					"NgGrpcHandle#dispose(): timeout to dispose.");
				//throw new NgFinalizeGrpcHandleException("failed to dispose");
			}
		}

		job.dispose();
		if (commManager != null) {
			commManager.dispose();
		}
	}
	
	/**
	 * Checks if this handle doesn't handle any jobs.
	 * 
	 * @return true if it's idle, false otherwise.
	 */
	protected boolean isIdle() {
		if (status.get() == ClientStatus.IDLE) {
			return true;
		}
		return false;
	}
	
	/**
	 * Checks if this handle is canceled?
	 * @return true if canceled.
	 */
	protected boolean isCanceled() {
		return isCanceled;
	}
	/**
	 * Sends request of execInfo and wait for the reply.
	 * 
	 * @return status code of a RemoteExecutable.<br>
	 *         if Ninf-G Executable is not available, returns -1.
	 * @throws GrpcException if it's failed to get status code.
	 */
	protected int getExeInfo() throws GrpcException {
		if ((status.get() == ClientStatus.NONE) ||
			(status.get() == ClientStatus.INIT)) {
			// nothing will be done
			return -1;
		}

		// get Information of Ninf-G Executable
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() + 
			"NgGrpcHandle#getInfo(): get Information.");
		NgProtocolRequest queryExeStatusRequest =
			new ProtQueryExeStatusRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				version);
		ProtQueryExeStatusReply queryExeStatusReply = 
			(ProtQueryExeStatusReply)
			commManager.sendProtocol(queryExeStatusRequest);
		// check result of Reply
		checkResult(queryExeStatusReply);

		return queryExeStatusReply.getExeInfo();
	}


	/**
	 * Sends request of pullbackSession and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to pull back a session.
	 */
	protected void pullbackSession() throws GrpcException {
		lockHandle();
		try {
			if (status.get() != ClientStatus.CANCEL) {
				// invalid status
				ngLog.logError(CAT_NG_INTERNAL, logHeaderWithSession()
					+ "NgGrpcHandle#pullbackSession(): invalid status.");
				throw new NgExecRemoteMethodException("failed to pullback");
			}
			
			// pullback session from Ninf-G Executable
			ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
				+ "NgGrpcHandle#pullbackSession(): Pullback Session.");
			NgProtocolRequest pullbackSessionRequest =
				new ProtPullbackSessionRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version);
			Protocol pullbackSessionReply = 
				commManager.sendProtocol(pullbackSessionRequest);
			// check result of Reply
			checkResult(pullbackSessionReply);

			status.set(ClientStatus.IDLE);
			this.isCanceled = false; // reset cancel flag 
		} finally {
			unlockHandle();
		}
	}
	
	/**
	 * Sends request of resetExecutable and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to reset.
	 */
	protected void resetExecutable() throws GrpcException {
		lockHandle();
		try {
			if ((status.get() == ClientStatus.NONE) ||
				(status.get() == ClientStatus.INIT) ||
				(status.get() == ClientStatus.IDLE)) {
				// nothing will be done 
				return;
			}
			
			// reset Ninf-G Executable
			ngLog.logInfo(CAT_NG_INTERNAL, logHeader() + 
				"NgGrpcHandle#resetExecutable(): Reset Session.");

			NgProtocolRequest resetExeRequest =
				new ProtResetExeRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version);
			Protocol resetExeReply = commManager.sendProtocol(resetExeRequest);

			checkResult(resetExeReply); // check result of Reply

			status.set(ClientStatus.IDLE);
		} finally {
			unlockHandle();
		}
	}
	
	/**
	 * Sends request of resetExecutable and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to resume.
	 */
	protected void resumeSession() throws GrpcException {
		lockHandle();
		try {
			if ((status.get() != ClientStatus.WAIT) &&
				(status.get() != ClientStatus.SUSPEND)) {
				// invalid status
				ngLog.logError(CAT_NG_INTERNAL, logHeaderWithSession()
					+ "NgGrpcHandle#resumeSession(): invalid status.");
				throw new NgExecRemoteMethodException("failed to resume");
			}

			// resume Ninf-G Executable
			ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
				+ "NgGrpcHandle#resumeSession(): Resume Session.");
			NgProtocolRequest resumeSessionRequest =
				new ProtResumeSessionRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version);
			Protocol resumeSessionReply = 
				commManager.sendProtocol(resumeSessionRequest);
			
			checkResult(resumeSessionReply); // check result of Reply 

			status.set(ClientStatus.WAIT);
		} finally {
			unlockHandle();
		}
	}

	/**
	 * Starts a session with arguments.
	 * 
	 * @param args arguments for a RemoteFunction/RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to complete a session.
	 */
	protected NgGrpcExecInfo startSession(Properties sessionAttr, List args)
	 throws GrpcException {
		return startSession((String)null, sessionAttr, args);
	}

	/**
	 * Starts a session with a name of method and arguments.<br>
	 * If method name is not supplied, then call default method.
	 * 
	 * @param methodName a name of a method.
	 * @param args arguments of RemoteFunction/RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to complete a session.
	 */
	protected NgGrpcExecInfo startSession(
	 String aMethodName, Properties sessionAttr, List args)
	 throws GrpcException {
		ngLog.logDebug(CAT_NG_INTERNAL, logHeader() 
			+ "NgGrpcHandle#startSession()");

		// set function/method name and session attribute.
		this.methodName = aMethodName;
		this.propSessionAttr = sessionAttr;

		initRemoteClassInfo();

		// Do calculation 
		return doSession(sessionAttr, args);
	}

	private void initRemoteClassInfo() throws GrpcException {
		ngLog.logDebug(CAT_NG_INTERNAL, logHeader() 
			+ "NgGrpcHandle#initRemoteClassInfo(): Initialize RemoteClassInfo");

		if (this.remoteClassInfo != null) {
			this.remoteMethodInfo =
				this.remoteClassInfo.getRemoteMethodInfo(this.methodName);
		} else {
			// If there is no class information, do search 
			setRemoteClassInfoByInformationManager();
		}

		if (this.remoteMethodInfo == null) {
			// if failed to get RemoteClassInfo, get it from RemoteExecutable 
			setRemoteClassInfoByExecutable(); 
		}
	}

	private void setRemoteClassInfoByInformationManager()
	 throws GrpcException {
		ngLog.logDebug(CAT_NG_INTERNAL, logHeader() + 
			"NgGrpcHandle#setRemoteClassInfoByInformationManager(): Set RemoteClassInfo(& RemoteMethodInfo) by InformationManager");

		NgInformationManager infoMgr = context.getNgInformationManager();

		infoMgr.lockInformationManager();
		try {
			this.remoteMethodInfo = 
				infoMgr.getMethodInfo(this.className, this.methodName);
			if (this.remoteMethodInfo == null)
				ngLog.logInfo(CAT_NG_INTERNAL, logHeader() 
					+ "NgGrpcHandle#setRemoteClassInfoByInformationManager(): can't get MethodInfo from InformationManager.");
				// Maybe you can get RemoteClassInfo from RemoteExecutable 

			this.remoteClassInfo = infoMgr.getClassInfo(this.className);
		} finally {
			infoMgr.unlockInformationManager();
		}
	}

	private void setRemoteClassInfoByExecutable() throws GrpcException {
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() +
			"NgGrpcHandle#setRemoteClassInfoByExecutable(): set RemoteClassInfo by Executable.");
		// get RemoteClassInfo from RemoteExecutable 
		this.remoteClassInfo = getRemoteClassInfoFromExecutable();

		// following process depend getRemoteClassInfoFromExecutable() 
		this.remoteMethodInfo = 
			context.getNgInformationManager().getMethodInfo(
				this.className, this.methodName);
	}


	/**
	 * Starts a session(Invokes a RemoteFunction/RemoteMethod, Waits
	 *  for complete it).
	 * 
	 * @param args arguments of RemoteFunction/RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to complete a session.
	 */
	private NgGrpcExecInfo doSession(Properties sessionAttr, List args)
	 throws GrpcException {
		// assert RemoteClassInfo != null && remoteMethodInfo != null 

		ngLog.logDebug(CAT_NG_INTERNAL, logHeader() 
			+ "NgGrpcHandle#doSession()");

		 // set timeout SessionCancel 
		_setSessionTimeout(sessionAttr);

		// reset intArguments(scalar arguments)
		this.intArguments = null;

		// generate sessionID 
		int sessionID = generateSessionID();

		// set methodID 
		this.methodID = remoteClassInfo.getRemoteMethodID(methodName);

		// ----- Invoke Session ----- 
		try {
			_invokeSession();
		} catch (GrpcException e) {
			ngLog.logInfo(CAT_NG_INTERNAL, logHeader() 
				+ e.getMessage());
			throw e;
		}

		// transform arg 
		ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession() 
			+ "NgGrpcHandle#doSession(): translate argument data.");

		CallContext callContext = new CallContext(remoteMethodInfo, args);

		// set information about callback 
		callbackFunc      = callContext.getCallbackFuncList();
		callbackFuncInfo  = callContext.getCallbackFuncInfoList();
		this.intArguments = callContext.getIntArguments();

		// create SessionInformation 
		this.sessionInfo =
			new SessionInformation(
				callContext.getNumParams(), callbackFunc.size());

		// send Arguments 
		CompressInfo compressInfo = remoteMachineInfo.getCompressInfo();

		// ----- Transform Arguments ----- 
		commManager.setKeepConnection(keep_connection);
		try {
			_transferArgumentData(callContext, compressInfo);
		} catch (GrpcException e) {
			ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
				+ e.getMessage());
			throw e;
		}

		// check timeout of session 
		commManager.checkSessionTimeout();

		timer.start();

		while (true) {
			int status = this.job.getStatus();
			if ((status == JobStatus.DONE) || (status == JobStatus.FAILED)) {
				if (commManager != null) {
					commManager.dispose();
				}
				throw new GrpcException("Unexpect job done");
			}

			if (commManager.checkCompleteFunction()) {
				break;
			}
			if (keep_connection && isCanceled) {
				break;
			}
			if (!commManager.isConnected() && commManager.reconnect()) {
				restartCommunicationManager();
				if (isCanceled) {
					break;
				}
			}
		}

		this.sessionInfo.setCalculationRealTime(timer.getElapsedTime());

		if (!commManager.checkCompleteFunction() && isCanceled) {
			cancelSession();
			pullbackSession();
			isCanceled = false;
			canceled.set();
		} else {
			boolean localCanceled = isCanceled;
			isCanceled = false;
			
			// ----- Transform Result ----- 
			Protocol transferResultReply = null;
			try {
				transferResultReply = _transferResult(callContext, compressInfo);
			} catch (GrpcException e) {
				ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession() 
					+ e.getMessage());
				if (localCanceled) {
					canceled.set();
				}
				throw e;
			}

			// Set result data into CallContext
			ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
				+ "NgGrpcHandle#doSession(): transform result data.");
			ProtTransferResultReply resultReply =
				(ProtTransferResultReply)transferResultReply;
			callContext.setResultData(resultReply.getResultData());

			// Translate & Set Results into the argument args
			callContext.transformResult(args);

			// ----- Pullback Session ----- 
			try {
				_pullBackSession();
			} catch (GrpcException e) {
				ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession() 
					+ e.getMessage());
				throw e;
			} finally {
				if (localCanceled) {
					canceled.set();
				}
			}
		}

		// release callbackInfo 
		callbackFunc     = null;
		callbackFuncInfo = null;

		return execInfo;
	}


	private void _setSessionTimeout(Properties attr) {
		String sessionTimeout = null;
		if ((attr != null) &&
			(attr.get(NgGrpcSessionAttr.KEY_SESSION_TIMEOUT) != null)) {
			sessionTimeout =
				(String)attr.get(NgGrpcSessionAttr.KEY_SESSION_TIMEOUT);
		} else {
			sessionTimeout =
				remoteMachineInfo.getRemoteClassPath(className)
								 .getSessionTimeout();
		}
		if (sessionTimeout != null) {
			commManager.setSessionTimeout(Integer.parseInt(sessionTimeout));
		}
	}

	private long getSessionTimeout() {
		long timeout = 0;
		String sessionTimeout = null;
		if ((propSessionAttr != null) &&
			(propSessionAttr.get(NgGrpcSessionAttr.KEY_SESSION_TIMEOUT) != null)) {
			sessionTimeout =
				(String)propSessionAttr.get(NgGrpcSessionAttr.KEY_SESSION_TIMEOUT);
		} else {
			sessionTimeout =
				remoteMachineInfo.getRemoteClassPath(className)
								 .getSessionTimeout();
		}
		if (sessionTimeout != null) {
			timeout = Integer.parseInt(sessionTimeout);
		}

		return timeout;
	}


	// Invoke Session Request to Ninf-G Executable
	private void _invokeSession() throws GrpcException {
		lockHandle(); // lock the handle 
		try {
			if ((this.isCanceled) || (status.get() != ClientStatus.IDLE)) {
				this.isCanceled = false;
				throw new NgException("unexpected status");
			}

			// reset Cond of Complete Function 
			commManager.resetCompleteFunction();

			commManager.checkSessionTimeout(); // check timeout of session 

			// Invoke Session 
			NgProtocolRequest invokeSessionRequest =
				new ProtInvokeSessionRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID, 
					version,
					methodID);

			Protocol invokeSessionReply = 
				commManager.sendProtocol(invokeSessionRequest);

			checkResult(invokeSessionReply); // check result of Reply 

			status.set(ClientStatus.INVOKE_SESSION);
		} finally {
			unlockHandle(); // unlock the handle 
		}
	}


	/*
	 * Transfer Argument Data Request to Ninf-G Executable
	 */
	private void _transferArgumentData(
	 CallContext aCallContext, CompressInfo aCompressInfo)
	 throws GrpcException {
		boolean compressEnable =
			aCompressInfo.getCompress().equals(CompressInfo.COMPRESS_ZLIB);
		//int compressThreshold =
		//	aCompressInfo.getCompressThreshold();
		int blockSize =
			Integer.parseInt(remoteMachineInfo.getBlockSize());

		lockHandle();
		try {
			if (status.get() != ClientStatus.INVOKE_SESSION) {
				throw new NgException("unexpected status");
				//return null;
			}

			// check timeout of session
			commManager.checkSessionTimeout();

			timer.start();
			// Transform arguments 
			NgProtocolRequest transferArgumentRequest =
				new ProtTransferArgumentRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version,
					aCallContext,
					commManager.getEncodeTypes(),
					aCompressInfo,
					blockSize,
					Boolean.valueOf(remoteMachineInfo.getKeepConnection()));

			Protocol transferArgumentReply = 
				commManager.sendProtocol(transferArgumentRequest);

			this.sessionInfo.setTransferArgumentRealTime(timer.getElapsedTime());
			// check result of Reply
			checkResult(transferArgumentReply);

			// put information about compress 
			if ( compressEnable ) {
				ProtTransferArgumentRequest protTransArg =
					(ProtTransferArgumentRequest)transferArgumentRequest;

				if (protTransArg.getOriginalDataTotalLength() != 0) {
					ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
						+ "NgGrpcHandle#doSession(): Before compress Data = "
						+ protTransArg.getOriginalDataTotalLength()
						+ " Bytes, After compress Data = " 
						+ protTransArg.getConvertedDataTotalLength() 
						+ " Bytes, The rate of compression = " 
						+ (((double)protTransArg.getConvertedDataTotalLength() / (double)protTransArg.getOriginalDataTotalLength()) * 100.0) + "%.");
				}
				
				// set CompressInformation 
				this.sessionInfo.setCompressionInformation(
					protTransArg.getOriginalDataLength(),
					protTransArg.getConvertedDataLength(),
					protTransArg.getConvertRealTime(),
					protTransArg.getConvertCPUTime());
			}

			status.set(ClientStatus.WAIT);
		} finally {
			unlockHandle();
		} 
	}

	/*
	 * Transfer Result Request to Ninf-G Executable
	 */
	private Protocol _transferResult(
	 CallContext aCallContext, CompressInfo aCompressInfo)
	 throws GrpcException {
		
		boolean compressEnable =
			aCompressInfo.getCompress().equals(CompressInfo.COMPRESS_ZLIB);
		Protocol transferResultReply = null;

		lockHandle();
		try {
			if (status.get() != ClientStatus.WAIT) {
				ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
					+ "NgGrpcHandle#doSession(): unexpected status, can't continue session");
				throw new NgException("unexpected status");
			}

			// receive Results 
			timer.start();
			NgProtocolRequest transferResultRequest =
				new ProtTransferResultRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version,
					aCallContext);
			transferResultReply =
				commManager.sendProtocol(transferResultRequest);

			this.sessionInfo.setTransferResultRealTime(timer.getElapsedTime());

			checkResult(transferResultReply); // check result of Reply 

			// put information about compress 
			if ( compressEnable ) {
				ProtTransferResultReply protTransRes =
					(ProtTransferResultReply)transferResultReply;
				if (protTransRes.getOriginalDataTotalLength() != 0) {
					ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
						+ "NgGrpcHandle#doSession(): Before compress Data = " 
						+ protTransRes.getOriginalDataTotalLength() 
						+ " Bytes, After compress Data = " 
						+ protTransRes.getConvertedDataTotalLength() 
						+ " Bytes, The rate of compression = " 
						+ (((double)protTransRes.getConvertedDataTotalLength() / (double)protTransRes.getOriginalDataTotalLength()) * 100.0) + "%.");
				}
				
				// set CompressInformation 
				this.sessionInfo.setDecompressionInformation(
					protTransRes.getOriginalDataLength(),
					protTransRes.getConvertedDataLength(),
					protTransRes.getConvertRealTime(),
					protTransRes.getConvertCPUTime());
			}
			
			status.set(ClientStatus.TRANSRES);
		} finally {
			unlockHandle();
		}
		return transferResultReply;
	}


	/* 
	 * Pull Back Session
	 */
	private void _pullBackSession() throws GrpcException {
		lockHandle();
		try {
			if (isCanceled || (status.get() != ClientStatus.TRANSRES)) {
				throw new NgException("unexpected status");
			}

			NgProtocolRequest pullbackSessionRequest =
				new ProtPullbackSessionRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version);
			Protocol pullbackSessionReply = 
				commManager.sendProtocol(pullbackSessionRequest);

			checkResult(pullbackSessionReply);	// check result of Reply 

			// get and set SessionInfo of server 
			ProtPullbackSessionReply protPullBack =
				(ProtPullbackSessionReply) pullbackSessionReply;

			execInfo.setServerSessionInformation(protPullBack.getServerSessionInfo());

			// put received SessionInformation 
			ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
				+ "NgGrpcHandle#doSession(): Session Information as below.\n" +
				protPullBack.getServerSessionInformationString());
			
			// set SessionInfo of client 
			execInfo.setClientSessionInformation(this.sessionInfo);
			
			status.set(ClientStatus.IDLE);
			this.isCanceled = false; // reset cancel flag
		} finally {
			unlockHandle();
		}
	}


	/**
	 * Sends request of suspendSession and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to suspend.
	 */
	protected void suspendSession() throws GrpcException {
		lockHandle();
		try {
			if ((status.get() != ClientStatus.WAIT) &&
				(status.get() != ClientStatus.SUSPEND)) {
				// invalid status 
				ngLog.logError(CAT_NG_INTERNAL, logHeaderWithSession()
					+ "NgGrpcHandle#suspendSession(): invalid status.");
				throw new NgExecRemoteMethodException("failed to suspend");
			}
			// suspend Ninf-G Executable 
			ngLog.logInfo(CAT_NG_INTERNAL, logHeaderWithSession()
				+ "NgGrpcHandle#suspendSession(): suspend Session.");
			NgProtocolRequest suspendSessionRequest =
				new ProtSuspendSessionRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version);
			Protocol suspendSessionReply = 
				commManager.sendProtocol(suspendSessionRequest);
			// check result of Reply 
			checkResult(suspendSessionReply);

			status.set(ClientStatus.SUSPEND);
		} finally {
			unlockHandle();
		}
	}
	
	/**
	 * Sends request of connectionClose and wait for the reply.
	 */
	protected void _connectionClose() throws GrpcException {
		lockHandle();
		try {
			NgProtocolRequest connectionCloseRequest =
				new ProtConnectionCloseRequest(
					commManager.generateSequenceNum(),
					context.getID(), executableID, version);
			Protocol connectionCloseReply =
				commManager.sendProtocol(connectionCloseRequest);
			checkResult(connectionCloseReply);
		} finally {
			unlockHandle();
		}
	}

	/**
	 * Sends request of invokeCallback and wait for the reply.
	 * 
	 * @param prot received Protocol(Notify of callback).
	 * @throws GrpcException if it failed to execute callback.
	 */
	protected void invokeCallback(Protocol prot) throws GrpcException {
		lockHandle(); // lock the handle 
		try {
			if ( isCanceled  || (status.get() != ClientStatus.WAIT)) {
				// invalid status
				ngLog.logError(CAT_NG_INTERNAL, logHeaderWithSession() 
					+ "NgGrpcHandle#invokeCallback(): invalid status.");
				throw new NgExecRemoteMethodException("failed to invoke Callback");
			}
			// set status 
			status.set(ClientStatus.INVOKE_CALLBACK);

			// get ID of callback
			ProtInvokeCallbackNotify protInvokeCallback =
				(ProtInvokeCallbackNotify) prot;
			int id = protInvokeCallback.getID();
			int seq = protInvokeCallback.getSequenceNo();

			// count callback 
			this.sessionInfo.incrementCallbackNTimesCalled();
		
			// ----- Transform arguments of Ninf-G callback ----- 
			// Request arguments for callback 
			timer.start();
			NgProtocolRequest cbTransferArgumentRequest =
				new ProtTransferCallbackArgumentRequest(
					commManager.generateSequenceNum(), context.getID(),
					executableID, sessionID,
					version,
					id);
			// Receive arguments for callback 
			Protocol cbTransferArgumentReply =
				commManager.sendProtocol(cbTransferArgumentRequest);
			this.sessionInfo.setCallbackTransferArgumentRealTime(timer.getElapsedTime());
			// check result of Reply 
			checkResult(cbTransferArgumentReply);
			
			// create CallContext for callback 
			ProtTransferCallbackArgumentReply callbackTransferArgumentReply =
				(ProtTransferCallbackArgumentReply)cbTransferArgumentReply;

			CallContext callContext = new CallContext(
				(RemoteMethodInfo) callbackFuncInfo.get(id),
				callbackTransferArgumentReply.getArgumentData(), this.intArguments);

			// get information about Compress
			//boolean compressEnable =
			//	remoteMachineInfo.getCompressInfo().getCompress().equals(CompressInfo.COMPRESS_ZLIB);

			// set CompressInformation 
			/** not implement on 2.4.0
			if (compressEnable == true) {
				this.sessionInfo.setCallbackInflateInformation(id,
						callbackTransferArgumentReply.getOriginalDataLength(),
						callbackTransferArgumentReply.getConvertedDataLength(),
						callbackTransferArgumentReply.getConvertRealTime(),		
						callbackTransferArgumentReply.getConvertCPUTime());			
			}
			 */

			// ----- call Ninf-G callback function ----- 
			// get callback Object 
			NgCallbackInterface callback =
				(NgCallbackInterface) callbackFunc.get(id);
			// call callback function 
			timer.start();
			callback.callback(callContext.getArgs());
			this.sessionInfo.setCallbackCalculationRealTime(timer.getElapsedTime());
		
			// ----- Transform results of Ninf-G callback ----- 
			// transform Result 
			callContext.transformCBResult();
		
			// Send result for callback 
			//int compressThreshold =
			//	remoteMachineInfo.getCompressInfo().getCompressThreshold();
			int blockSize =
				Integer.parseInt(remoteMachineInfo.getBlockSize());
			ProtTransferCallbackResultRequest cbTransferResultRequest =
			 new ProtTransferCallbackResultRequest(
				commManager.generateSequenceNum(),
				context.getID(), executableID, sessionID, version,
				id,
				callContext,
				commManager.getEncodeTypes(), remoteMachineInfo.getCompressInfo(),
				blockSize,
				Boolean.valueOf(remoteMachineInfo.getKeepConnection()));	

			// Receive reply for callback
			timer.start();
			Protocol cbTransferResultReply =
				commManager.sendProtocol(cbTransferResultRequest);

			this.sessionInfo.setCallbackTransferResultRealTime(timer.getElapsedTime());
			// check result of Reply 
			checkResult(cbTransferResultReply);

			// set CompressInformation
			/** not implement on 2.4.0
			if (compressEnable == true) {
				this.sessionInfo.setCallbackDeflateInformation(id,
						cbTransferResultRequest.getOriginalDataLength(),
						cbTransferResultRequest.getConvertedDataLength(),
						cbTransferResultRequest.getConvertRealTime(),		
						cbTransferResultRequest.getConvertCPUTime());			
			}
			*/
			
			status.set(ClientStatus.WAIT);
		} finally {
			unlockHandle();
		}
	}
	
	/**
	 * Sends request of queryFunctionInfo (to Ninf-G Executable?) 
	 * and wait for the reply.
	 * 
	 * @return information of a RemoteFunction/RemoteMethod.
	 * @throws GrpcException if it failed to execute callback.
	 */
	private RemoteClassInfo getRemoteClassInfoFromExecutable()
	 throws GrpcException {
		if ((status.get() == ClientStatus.NONE) ||
			(status.get() == ClientStatus.INIT)) {
			return null; // nothing will be done
		}

		// get Information of Ninf-G Executable 
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() +
			"NgGrpcHandle#getRemoteClassInfoFromExecutable(): get RemoteClassInfo from Remote Executable.");

		// send queryFunctionInfoRequest to Executable 
		NgProtocolRequest queryFunctionInfoRequest =
			new ProtQueryFunctionInfoRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				version);

		// receive queryFunctionInfoReply from Executable 
		ProtQueryFunctionInfoReply queryFunctionInfoReply = 
			(ProtQueryFunctionInfoReply)
			commManager.sendProtocol(queryFunctionInfoRequest);

		// check result of Reply 
		checkResult(queryFunctionInfoReply);

		// put RemoteClassInfo into NgInformationManager 
		ngLog.logInfo(CAT_NG_INTERNAL, logHeader() +
			"NgGrpcHandle#getRemoteClassInfoFromExecutable(): put RemoteClassInfo to InformationManager.");
		NgInformationManager infoManager = context.getNgInformationManager();

		// put RemoteClassInfo into NgInformationManager
		infoManager.putClassInfo(className,
			queryFunctionInfoReply.getRemoteClassInfo());

		// return information 
		return queryFunctionInfoReply.getRemoteClassInfo();
	}
	
	/**
	 * Checks result code in Protocol object.
	 * 
	 * @param protocol target Protocol.
	 * @throws GrpcException if result code is not 0 or received protocol is null.
	 */
	private void checkResult(Protocol protocol) throws GrpcException {
		// check if protocol were null
		if (protocol == null) {
			throw new NgException("protocol is null.");
		} else if (protocol.getResult() != 0) {
			throw new NgException("protocol has invalid result code.");
		}
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#clone()
	 */
	protected Object clone() throws CloneNotSupportedException {
		return super.clone();
	}


	/**
	 * Lock the handle.
	 * 
	 * @throws GrpcException if it's interrupted.
	 */
	protected synchronized void lockHandle() throws GrpcException {
		// wait for Unlocked 
		while ( isLocked ) {
			try {
				wait();
			} catch (InterruptedException e) {
				throw new NgException(e);
			}
		}
		isLocked = true; // lock 
	}
	
	/**
	 * Unlock the handle.
	 * 
	 * @throws GrpcException  if it's interrupted.
	 */
	protected synchronized void unlockHandle() throws GrpcException {
		// check if it's locked
		if (! isLocked ) {
			throw new NgException("Nobody lock the handle.");
		}
		isLocked = false; // unlock
		notifyAll();
	}

	/**
	 * Gets a status code of this handle.
	 * @return a status code of this handle.
	 */
	public int getLocalStatus() {
		return status.get();
	}
	
	/**
	 * Generates ID of session.
	 * 
	 * @return ID of session.
	 */
	private int generateSessionID() {
		sessionID += 1;
		return sessionID;
	}
	

	/**
	 * Gets RemoteMachineInfo.
	 * 
	 * @return RemoteMachineInfo of this handle.
	 */
	protected RemoteMachineInfo getRemoteMachineInfo() {
		return remoteMachineInfo;
	}

	/**
	 * Gets ID of associated with this handle.
	 * 
	 * @return ID of the executable.
	 */
	protected int getExecutableID() {
		return executableID;
	}
	
	/**
	 * Sets ID of associated with this handle.
	 * 
	 * @param executableID ID to set as executableID.
	 */
	protected void setExecutableID(int executableID) {
		this.executableID = executableID;
	}
	
	/**
	 * @return the time of job start timeout in milliseconds.
	 */
	protected long getJobStartTimeout() {
		return this.jobStartTimeout;
	}
	
	/**
	 * @return
	 */
	protected NgGrpcJob getJob() {
		return this.job;
	}
	
	/**
	 * @return
	 */
	protected NgGrpcClient getContext() {
		return this.context;
	}
	
	/**
	 * @return
	 */
	protected int getID() {
		return this.executableID;
	}
	
	/**
	 * @return
	 */
	protected int getSessionID() {
		return this.sessionID;
	}

	/**
	 * @param prop
	 * @return 
	 */
	private static Properties getDefaultServerProperties(Properties prop) {
		Properties tmpProperties = new Properties();
		tmpProperties.put(HOSTNAME, prop.get(HOSTNAME));
		return tmpProperties;
	}

	/**
	 * @return
	 */
	protected Version getVersion() {
		return this.version;
	}

	public String logHeader() {
		return "Executable [" + String.valueOf(executableID) + "]: ";
	}

	private String logHeaderWithSession() {
		return logHeader() + "Session [" + String.valueOf(sessionID) + "]: ";
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("Context: ").append(context).append("\n");
		sb.append("JobID: ").append(jobID).append("\n");
		sb.append("ExecutableID: ").append(executableID).append("\n");
		sb.append("Session ID: ").append(sessionID).append("\n");
		sb.append("Method ID: ").append(methodID).append("\n");
		sb.append("JobCount: ").append(jobCount).append("\n");
		sb.append("SessionInfo: ").append(sessionInfo).append("\n");
		sb.append("RemoteMachineInfo: ").append(remoteMachineInfo.hashCode())
			.append("\n");
		sb.append("RemoteClassPathInfo: ").append(classPathInfo).append("\n");
		sb.append("RemoteClassInfo: ").append(remoteClassInfo).append("\n");
		sb.append("RemoteMethodInfo: " + remoteMethodInfo + "\n");
		sb.append("NgGrpcJob: ").append(job).append("\n");
		sb.append("NgGrpcExecInfo: ").append(execInfo).append("\n");
		sb.append("ClientStatus: ").append(status).append("\n");
		sb.append("CommunicationManager: ").append(commManager).append("\n");
		sb.append("RemoteServerName: ").append(hostName).append("\n");
		sb.append("MethodName: ").append(methodName).append("\n");
		sb.append("ClassName: ").append(className).append("\n");
		sb.append("HandleAttr: ").append(propGrpcHandleAttr).append("\n");
		sb.append("IntArguments: ")
			.append( java.util.Arrays.toString(intArguments) )
			.append("\n");
		sb.append("CallbackFunc: ").append(callbackFunc).append("\n");
		sb.append("CallbackFuncInfo: ").append(callbackFuncInfo).append("\n");
		sb.append("Protocol Version: ").append(version);

		return sb.toString();
	}
}

