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
 * $RCSfile: NgGrpcHandle.java,v $ $Revision: 1.98 $ $Date: 2005/10/04 06:04:15 $
 */
package org.apgrid.grpc.ng;

import java.util.List;
import java.util.Properties;

import org.apgrid.grpc.ng.info.*;
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
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;

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
	private int jobID;
	private int executableID;
	private String hostName;
	private RemoteMachineInfo remoteMachineInfo;
	private RemoteClassPathInfo classPathInfo;
	private String className;
	private RemoteClassInfo remoteClassInfo;
	private NgGrpcJob job;
	private NgGrpcExecInfo execInfo;
	private SessionInformation sessionInfo;
	private GrpcTimer timer;
	private int sessionID;
	private Integer status;
	private CommunicationManager commManager;
	private String methodName;
	private int methodID;
	private RemoteMethodInfo remoteMethodInfo;
	private NgLog ngLog;
	private int jobCount;
	private Properties propGrpcHandleAttr;
	private int[] intArguments;
	private boolean isCanceled;
	private boolean isLocked;
	private long jobStartTimeout;
	
	/* for callback */
	private List callbackFunc;
	private List callbackFuncInfo;

	/* protocol version */
	int versionMajor;
	int versionMinor;
	int versionPatch;
	
	/* status of Ninf-G Client */
	/** not available */
	public final int CLIENTSTATE_NONE = 0x00;
	/** idling */
	public final int CLIENTSTATE_IDLE = 0x01;
	/** initializing handle */
	public final int CLIENTSTATE_INIT = 0x02;
	/** invoking session */
	public final int CLIENTSTATE_INVOKE_SESSION = 0x03;
	/** sending arguments */
	public final int CLIENTSTATE_TRANSARG = 0x05;
	/** waiting for function complete */
	public final int CLIENTSTATE_WAIT = 0x07;
	/** received notify of function complete */
	public final int CLIENTSTATE_COMPLETE_CALCULATING = 0x08;
	/** transforming results */
	public final int CLIENTSTATE_TRANSRES = 0x09;
	/** pulling back session */
	public final int CLIENTSTATE_PULLBACK = 0x0b;
	/** suspending session */
	public final int CLIENTSTATE_SUSPEND = 0x0c;
	/** resuming session */
	public final int CLIENTSTATE_RESUME = 0x0e;
	/** disposing handle */
	public final int CLIENTSTATE_DISPOSE = 0x0f;
	/** resetting handle */
	public final int CLIENTSTATE_RESET = 0x10;
	/** canceling session */
	public final int CLIENTSTATE_CANCEL = 0x11;
	/** executing callback */
	public final int CLIENTSTATE_INVOKE_CALLBACK = 0x12;
	
	/**
	 * Creates NgGrpcHandle.
	 */
	protected NgGrpcHandle() {
		/* nothing will be done */
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
		int jobID, int executableID, int jobCount) throws GrpcException {
		NgInformationManager infoMgr = context.getNgInformationManager();
		Properties tmpProp = null;
		try {
			infoMgr.lockInformationManager();
			tmpProp =
				getDefaultServerProperties(infoMgr.getDefaultRemoteMachineProperties(className));
		} finally {
			infoMgr.unlockInformationManager();
		}
		
		initialize(className, tmpProp, context, jobID, executableID, jobCount);
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
		NgGrpcClient context, int jobID, int executableID, int jobCount)
		throws GrpcException {
		
		initialize(className, prop, context, jobID, executableID, jobCount);
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
	private void initialize(String className, Properties prop,
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
		
		/* init status */
		this.status = new Integer(CLIENTSTATE_NONE);
		
		/* get name of server */
		if (prop == null) {
			throw new NgInitializeGrpcHandleException(
				"Invalid ServerInformation.");
		}
		this.hostName = (String) prop.get(NgGrpcHandleAttr.KEY_HOSTNAME);
		
		/* If hostname is not specified, use default server */
		if (this.hostName == null) {
			NgInformationManager infoMgr = context.getNgInformationManager();
			Properties tmpProp = null;
			try {
				infoMgr.lockInformationManager();
				
				tmpProp = infoMgr.getDefaultRemoteMachineProperties(className);
			} finally {
				infoMgr.unlockInformationManager();
			}
			if (tmpProp == null) {
				throw new NgInitializeGrpcHandleException(
					"Server is not specified.");
			}
			this.hostName =
				(String) tmpProp.get(NgGrpcHandleAttr.KEY_HOSTNAME);
		}

		/* get name of class */
		if (prop.get(NgGrpcHandleAttr.KEY_CLASSNAME) != null) {
			this.className = (String) prop.get(NgGrpcHandleAttr.KEY_CLASSNAME);
		} else {
			this.className = className;
		}

		/* check name of host and class */
		if ((hostName == null) || (className == null)) {
			throw new NgInitializeGrpcHandleException(
				"Invalid hostname or classname");
		}

		/* set attribute for GrpcHandle */
		this.propGrpcHandleAttr = prop;
		
		/* get NgLog */
		this.ngLog = context.getNgLog();
		
		/* activate GrpcHandle */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#initialize(): activate handle.");
		activate();						

		/* set status */
		setStatus("", CLIENTSTATE_IDLE);
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
		/* set status */
		setStatus("activate", CLIENTSTATE_INIT);
			
		/* lookup information */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#activate(): search information.");
		/* get RemoteMachineInfo */
		RemoteClassPathInfo rcPath = null;
		timer.start();
		NgInformationManager infoManager = context.getNgInformationManager();
		try {
			infoManager.lockInformationManager();

			rcPath = infoManager.getClassPathInfo(hostName, className);
			if (rcPath == null) {
				throw new NgInitializeGrpcHandleException(
					"can't get RemoteClassPathInfo.");
			}
				
			remoteMachineInfo = infoManager.getRemoteMachineInfoCopy(hostName);
		} finally {
			infoManager.unlockInformationManager();
		}
		/* set time to lookup information */
		execInfo.setLookupServerInfoTime(timer.getElapsedTime());
		/* get RemoteClassInfo */
		timer.start();
		try {
			infoManager.lockInformationManager();
			
			remoteClassInfo = (RemoteClassInfo)infoManager.getRemoteClassInfo(className);
		} finally {
			infoManager.unlockInformationManager();
		}
		/* set time to lookup information */
		execInfo.setLookupClassInfoTime(timer.getElapsedTime());

		/* set attribute of Handle */
		if (this.propGrpcHandleAttr.containsKey(NgGrpcHandleAttr.KEY_MPI_NCPUS) == true) {
			remoteMachineInfo.resetNumCPUs();
			this.propGrpcHandleAttr =
				NgGrpcHandleAttr.convertNumOfCPUs(this.propGrpcHandleAttr);
		}
		remoteMachineInfo.overwriteParameter(this.propGrpcHandleAttr);
		
		/* invoke GrpcJob */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#activate(): invoke Ninf-G Executable.");
		/* set timeout */
		String jobStartTimeout =
			(String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_JOB_STARTTIMEOUT);
		this.jobStartTimeout = Long.parseLong(jobStartTimeout) * 1000;
		/* check the timeout variable */
		if (this.jobStartTimeout < 0) {
			throw new NgInitializeGrpcHandleException("job_startTimeout is invalid.");
		}
		/* set jobType */
		String jobType = null;
		if (remoteClassInfo != null) {
			jobType = remoteClassInfo.getBackend();
		}
		if (rcPath != null) {
			String jobTypeInClassPathInfo =
				(String) rcPath.get(RemoteClassPathInfo.KEY_CLASS_PATH_BACKEND);
			if (jobTypeInClassPathInfo != null &&
				(jobTypeInClassPathInfo.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
				 jobTypeInClassPathInfo.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {
				jobType = jobTypeInClassPathInfo;
			}
		}
		
		/* invoke JOB */
		timer.start();
		job = new NgGrpcJob(context, remoteMachineInfo,
			className, jobID, jobType, jobCount);
		
		/* set time to invoke */
		execInfo.setInvokeTime(timer.getElapsedTime());
		
		/* start CommunicationManager, this will wait for active */
		startCommunicationManager();
		/* get protocol version */
		this.versionMajor = commManager.getVersionMajor();
		this.versionMinor = commManager.getVersionMinor();
		this.versionPatch = commManager.getVersionPatch();
	}
	
	/**
	 * Starts CommunicationManager to manager send/receive Protocol.
	 * 
	 * @throws GrpcException if failed to start CommunicationManager.
	 */
	protected void startCommunicationManager() throws GrpcException {
		/* start CommunicationManager */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#startCommunicationManager () : start CommunicationManager.");
		try {
			commManager = new CommunicationManager(context, this);
		} catch (GrpcException e) {
			job.setRequiredJobCancel();
			job.incrementExitCount();
			throw e;
		}
		new Thread(commManager).start();
	}
	
	/**
	 * Cancels a current session.
	 * 
	 * @throws GrpcException if failed to cancel.
	 */
	protected void cancel() throws GrpcException {
		cancelSession();
		pullbackSession();
	}
	
	/**
	 * Sends request of cancel and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to cancel.
	 */
	private void cancelSession() throws GrpcException {
		try {
			/* lock the handle */
			lockHandle();

			/* set cancel flag */
			this.isCanceled = true;

			int status = getLocalStatus();
			if ((status == CLIENTSTATE_NONE) || (status == CLIENTSTATE_INIT) ||
				(status == CLIENTSTATE_TRANSRES)) {
				/* nothing will be done */
				return;
			} else if (status == CLIENTSTATE_IDLE) {
				/* reset isCanceled flag */
				this.isCanceled = false;
				return;
			}

			/* send and receive Cancel Protocol */
			ngLog.printSessionLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#cancelSession(): cancel Session.");
			Protocol cancelRequest = new ProtCancelSessionRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch);
			Protocol cancelReply = 
				commManager.sendProtocol(cancelRequest);
			/* check result of Reply */
			checkResult(cancelReply);

			/* set status */
			setStatus("cancelSession", CLIENTSTATE_CANCEL);
		} finally {
			/* unlock the handle */
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
		try {
			/* lock the handle */
			lockHandle();

			if (getLocalStatus() != CLIENTSTATE_IDLE) {
				/* invalid status */
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_ERROR,
					this,
					"NgGrpcHandle#dispose(): invalid status.");
				throw new NgFinalizeGrpcHandleException("failed to dispose");
			}

			/* dispose handle */		
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#dispose(): dispose Handle.");
			Protocol exitExeRequest = new ProtExitExeRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch);
			Protocol exitExeReply = 
				commManager.sendProtocol(exitExeRequest);
			/* check result of Reply */
			checkResult(exitExeReply);
		} catch (GrpcException e) {
			/* set requiredJobCancel */
			job.setRequiredJobCancel();
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#dispose(): set Require JobCancel .");
		} finally {
			/* set status */
			setStatus("dispose", CLIENTSTATE_NONE);
			/* increment exit count */
			job.incrementExitCount();			
			/* remove this from handle list */
			context.removeHandle(this);
			/* unlock the handle */
			unlockHandle();
		}

		/* wait for done */
		String jobStopTimeout =
			(String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_JOB_STOPTIMEOUT);
		long stopTimeout = Long.parseLong(jobStopTimeout) * 1000;
		if (stopTimeout < 0) {
			job.waitForDone();
		} else if (stopTimeout == 0) {
			/* don't wait JOB DONE */
		} else {
			if (job.waitForDone(stopTimeout) != 0) {
				/* timeout */
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_WARN,
					this,
					"NgGrpcHandle#dispose(): timeout to dispose.");
				//throw new NgFinalizeGrpcHandleException("failed to dispose");
			}
		}
		
		/* dispose JOB */
		job.dispose();
	}
	
	/**
	 * Checks if this handle doesn't handle any jobs.
	 * 
	 * @return true if it's idle, false otherwise.
	 */
	protected boolean isIdle() {
		if (getLocalStatus() == CLIENTSTATE_IDLE) {
			return true;
		} else {
			return false;
		}
	}
	
	/**
	 * Sends request of execInfo and wait for the reply.
	 * 
	 * @return status code of a RemoteExecutable.<br>
	 *         if Ninf-G Executable is not available, returns -1.
	 * @throws GrpcException if it's failed to get status code.
	 */
	protected int getExeInfo() throws GrpcException {
		int status = getLocalStatus();
		if ((status == CLIENTSTATE_NONE) || (status == CLIENTSTATE_INIT)) {
			/* nothing will be done */
			return -1;
		}

		/* get Information of Ninf-G Executable */
		ngLog.printSessionLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#getInfo(): get Information.");
		Protocol queryExeStatusRequest = new ProtQueryExeStatusRequest(
			commManager.generateSequenceNum(), context.getID(),
			executableID, sessionID,
			versionMajor, versionMinor, versionPatch);
		ProtQueryExeStatusReply queryExeStatusReply = 
			(ProtQueryExeStatusReply)
			commManager.sendProtocol(queryExeStatusRequest);
		/* check result of Reply */
		checkResult(queryExeStatusReply);

		return queryExeStatusReply.getExeInfo();
	}
	
	/**
	 * Gets a status code of this handle.
	 * 
	 * @return a status code of this handle.
	 */
	protected int getLocalStatus() {
		return status.intValue();
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
	 * Sends request of pullbackSession and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to pull back a session.
	 */
	protected void pullbackSession() throws GrpcException {
		try {
			/* lock the handle */
			lockHandle();

			int status = getLocalStatus();
			if (status != CLIENTSTATE_CANCEL) {
				/* invalid status */
				ngLog.printSessionLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_ERROR,
					this,
					"NgGrpcHandle#pullbackSession(): invalid status.");
				throw new NgExecRemoteMethodException("failed to pullback");
			}
			
			/* pullback session from Ninf-G Executable */
			ngLog.printSessionLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#pullbackSession(): Pullback Session.");
			Protocol pullbackSessionRequest = new ProtPullbackSessionRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch);
			Protocol pullbackSessionReply = 
				commManager.sendProtocol(pullbackSessionRequest);
			/* check result of Reply */
			checkResult(pullbackSessionReply);

			/* set status */
			setStatus("pullbackSession", CLIENTSTATE_IDLE);
			/* reset cancel flag */
			this.isCanceled = false;
		} finally {
			/* unlock the handle */
			unlockHandle();
		}
	}
	
	/**
	 * Sends request of resetExecutable and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to reset.
	 */
	protected void resetExecutable() throws GrpcException {
		try {
			/* lock the handle */
			lockHandle();

			int status = getLocalStatus();
			if ((status == CLIENTSTATE_NONE) || (status == CLIENTSTATE_INIT) ||
				(status == CLIENTSTATE_IDLE)) {
				/* nothing will be done */
				return;
			}
			
			/* reset Ninf-G Executable */
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#resetExecutable(): Reset Session.");
			Protocol resetExeRequest = new ProtResetExeRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch);
			Protocol resetExeReply = 
				commManager.sendProtocol(resetExeRequest);
			/* check result of Reply */
			checkResult(resetExeReply);

			/* set status */
			setStatus("resetExecutable", CLIENTSTATE_IDLE);
		} finally {
			/* unlock the handle */
			unlockHandle();
		}
	}
	
	/**
	 * Sends request of resetExecutable and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to resume.
	 */
	protected void resumeSession() throws GrpcException {
		try {
			/* lock the handle */
			lockHandle();

			int status = getLocalStatus();
			if ((status != CLIENTSTATE_WAIT) && (status != CLIENTSTATE_SUSPEND)) {
				/* invalid status */
				ngLog.printSessionLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_ERROR,
					this,
					"NgGrpcHandle#resumeSession(): invalid status.");
				throw new NgExecRemoteMethodException("failed to resume");
			}

			/* resume Ninf-G Executable */
			ngLog.printSessionLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#resumeSession(): Resume Session.");
			Protocol resumeSessionRequest = new ProtResumeSessionRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch);
			Protocol resumeSessionReply = 
				commManager.sendProtocol(resumeSessionRequest);
			/* check result of Reply */
			checkResult(resumeSessionReply);

			/* set status */
			setStatus("resumeSession", CLIENTSTATE_WAIT);
		} finally {
			/* unlock the handle */
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
	protected NgGrpcExecInfo startSession(
			Properties sessionAttr, List args) throws GrpcException {
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
		String methodName, Properties sessionAttr, List args) throws GrpcException {
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcHandle#startSession()");

		/* set methodName */
		this.methodName = methodName;

		/* if there is no class information, do search */
		if (remoteClassInfo == null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#startSession() : get name of Method.");
			NgInformationManager infoMgr = context.getNgInformationManager();

			try {
				infoMgr.lockInformationManager();
				
				if (methodName == null) {
					remoteMethodInfo = infoMgr.getRemoteMethodInfo(className);
				} else {
					remoteMethodInfo = 
						infoMgr.getRemoteMethodInfo(className, methodName);
				}
				/* Maybe you can get RemoteClassInfo from RemoteExecutable */
				remoteClassInfo = infoMgr.getRemoteClassInfo(className);
			} catch (GrpcException e) {
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					this,
					"NgGrpcHandle#startSession() : can't get MethodInfo from InformationManager.");
			} finally {
				infoMgr.unlockInformationManager();
			}
		} else {
			remoteMethodInfo = remoteClassInfo.getRemoteMethodInfo(methodName);
		}
		
		/* if failed to get RemoteClassInfo, get it from RemoteExecutable */
		if (remoteMethodInfo == null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#startSession() : get RemoteClassInfo from Executable.");
			/* get RemoteClassInfo from RemoteExecutable */
			remoteClassInfo = getRemoteClassInfo();
			if (methodName == null) {
				remoteMethodInfo = 
					context.getNgInformationManager().getRemoteMethodInfo(
					className);
			} else {
				remoteMethodInfo = 
					context.getNgInformationManager().getRemoteMethodInfo(
					className, methodName);
			}
		}

		/* Do calculation */
		return doSession(sessionAttr, args);
	}

	/**
	 * Starts a session(Invokes a RemoteFunction/RemoteMethod, Waits for complete it).
	 * 
	 * @param args arguments of RemoteFunction/RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to complete a session.
	 */
	private NgGrpcExecInfo doSession(Properties sessionAttr, List args) throws GrpcException {
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_DEBUG,
			this,
			"NgGrpcHandle#doSession()");
		
		/* set timeout SessionCancel */
		String sessionTimeout = null;
		if ((sessionAttr != null) &&
			(sessionAttr.get(NgGrpcSessionAttr.KEY_SESSION_TIMEOUT) != null)) {
			sessionTimeout =
				(String)sessionAttr.get(NgGrpcSessionAttr.KEY_SESSION_TIMEOUT);
		} else {
			sessionTimeout =
				(String) remoteMachineInfo.getRemoteClassPath(
					className).get(RemoteClassPathInfo.KEY_CLASS_PATH_SESSION_TIMEOUT);
		}
		if (sessionTimeout != null) {
			commManager.setSessionTimeout(Integer.parseInt(sessionTimeout));
		}

		/* reset intArguments */
		this.intArguments = null;
		
		/* generate sessionID */
		int sessionID = generateSessionID();
		
		/* set methodID */
		this.methodID = remoteClassInfo.getRemoteMethodID(methodName);
		
		/* ----- Invoke Session ----- */
		try {
			/* lock the handle */
			lockHandle();

			if ((this.isCanceled == true) || (getLocalStatus() != CLIENTSTATE_IDLE)) {
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					this,
					"NgGrpcHandle#doSession(): unexpected status, can't continue session");
				this.isCanceled = false;
				return null;
			}
			/* reset Cond of Complete Function */
			commManager.resetCompleteFunction();
			/* check timeout of session */
			commManager.checkSessionTimeout();
		
			/* Invoke Session */
			Protocol invokeSessionRequest = new ProtInvokeSessionRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID, methodID,
				versionMajor, versionMinor, versionPatch);
			Protocol invokeSessionReply = 
				commManager.sendProtocol(invokeSessionRequest);
			/* check result of Reply */
			checkResult(invokeSessionReply);

			setStatus("doSession", CLIENTSTATE_INVOKE_SESSION);		
		} finally {
			/* unlock the handle */
			unlockHandle();
		}
		
		/* transform arg */
		ngLog.printSessionLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#doSession(): translate argument data.");
		CallContext callContext = new CallContext(remoteMethodInfo, args);
		/* set information about callback */
		callbackFunc = callContext.getCallbackFuncList();
		callbackFuncInfo = callContext.getCallbackFuncInfoList();
		this.intArguments = callContext.getIntArguments();
		
		/* create SessionInformation */
		this.sessionInfo =
			new SessionInformation(callContext.getNumParams(), callbackFunc.size());
		
		/* send Arguments */
		boolean compressEnable = remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMPRESS).equals(
			RemoteMachineInfo.VAL_COMPRESS_ZLIB);
		int compressThreshold =
			new Integer((String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMPRESS_THRESHOLD)).intValue();
		int blockSize =
			new Integer((String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_BLOCK_SIZE)).intValue();

		/* ----- Transform Arguments ----- */
		try {
			/* lock the handle */
			lockHandle();

			if (getLocalStatus() != CLIENTSTATE_INVOKE_SESSION) {
				ngLog.printSessionLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					this,
					"NgGrpcHandle#doSession(): unexpected status, can't continue session");
				return null;
			}
			
			/* check timeout of session */
			commManager.checkSessionTimeout();

			/* Transform arguments */
			timer.start();
			Protocol transferArgumentRequest = new ProtTransferArgumentRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID, callContext,
				commManager.getEncodeTypes(), compressEnable,
				compressThreshold, blockSize,
				versionMajor, versionMinor, versionPatch);
			Protocol transferArgumentReply = 
				commManager.sendProtocol(transferArgumentRequest);
			this.sessionInfo.setTransferArgumentRealTime(timer.getElapsedTime());
			/* check result of Reply */
			checkResult(transferArgumentReply);

			/* put information about compress */
			if (compressEnable == true) {
				ProtTransferArgumentRequest protTransArg =
					(ProtTransferArgumentRequest)transferArgumentRequest;
				if (protTransArg.getOriginalDataTotalLength() != 0) {
					ngLog.printSessionLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_INFO,
							this,
							"NgGrpcHandle#doSession(): Before compress Data = " + 
							protTransArg.getOriginalDataTotalLength() + " Bytes, " +
							"After compress Data = " +
							protTransArg.getConvertedDataTotalLength() + " Bytes, " +
							"The rate of compression = " +
							(((double)protTransArg.getConvertedDataTotalLength() / (double)protTransArg.getOriginalDataTotalLength()) * 100.0) +
							"%.");
				}
				
				/* set CompressInformation */
				this.sessionInfo.setCompressionInformation(
						protTransArg.getOriginalDataLength(),
						protTransArg.getConvertedDataLength(),
						protTransArg.getConvertRealTime(),
						protTransArg.getConvertCPUTime());			
			}

			/* set status */
			setStatus("doSession", CLIENTSTATE_WAIT);
		} finally {
			/* unlock the handle */
			unlockHandle();
		}
		/* check timeout of session */
		commManager.checkSessionTimeout();

		/* wait for the end of calculation */
		timer.start();
		commManager.waitCompleteFunction();
		this.sessionInfo.setCalculationRealTime(timer.getElapsedTime());
		
		/* ----- Transform Result ----- */
		Protocol transferResultReply = null;
		try {
			/* lock the handle */
			lockHandle();

			if (getLocalStatus() != CLIENTSTATE_WAIT) {
				ngLog.printSessionLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					this,
					"NgGrpcHandle#doSession(): unexpected status, can't continue session");
				return null;
			}

			/* receive Results */
			timer.start();
			Protocol transferResultRequest = new ProtTransferResultRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch, callContext);
			transferResultReply = commManager.sendProtocol(transferResultRequest);
			this.sessionInfo.setTransferResultRealTime(timer.getElapsedTime());
			/* check result of Reply */
			checkResult(transferResultReply);

			/* put information about compress */
			if (compressEnable == true) {
				ProtTransferResultReply protTransRes =
					(ProtTransferResultReply)transferResultReply;
				if (protTransRes.getOriginalDataTotalLength() != 0) {
					ngLog.printSessionLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_INFO,
							this,
							"NgGrpcHandle#doSession(): Before compress Data = " + 
							protTransRes.getOriginalDataTotalLength() + " Bytes, " +
							"After compress Data = " +
							protTransRes.getConvertedDataTotalLength() + " Bytes, " +
							"The rate of compression = " +
							(((double)protTransRes.getConvertedDataTotalLength() / (double)protTransRes.getOriginalDataTotalLength()) * 100.0) +
							"%.");
				}
				
				/* set CompressInformation */
				this.sessionInfo.setDecompressionInformation(
						protTransRes.getOriginalDataLength(),
						protTransRes.getConvertedDataLength(),
						protTransRes.getConvertRealTime(),
						protTransRes.getConvertCPUTime());
			}
			
			/* set status */
			setStatus("doSession", CLIENTSTATE_TRANSRES);
		} finally {
			/* unlock the handle */
			unlockHandle();
		}

		/* set result data into CallContext */
		ngLog.printSessionLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#doSession(): transform result data.");
		ProtTransferResultReply resultReply =
			(ProtTransferResultReply)transferResultReply;
		callContext.setResultData(resultReply.getResultData());

		/* translate Results */
		callContext.transformResult(args);
		
		/* ----- Pullback Session ----- */
		try {
			/* lock the handle */
			lockHandle();

			if ((isCanceled == true) || (getLocalStatus() != CLIENTSTATE_TRANSRES)) {
				ngLog.printSessionLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					this,
					"NgGrpcHandle#doSession(): unexpected status, can't continue session");
				return null;
			}
			
			Protocol pullbackSessionRequest = new ProtPullbackSessionRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch);
			Protocol pullbackSessionReply = 
				commManager.sendProtocol(pullbackSessionRequest);
			/* check result of Reply */
			checkResult(pullbackSessionReply);
		
			/* get and set SessionInfo of server */
			ProtPullbackSessionReply protPullBack =
				(ProtPullbackSessionReply) pullbackSessionReply;
			execInfo.setServerSessionInformation(protPullBack.getServerSessionInfo());
			
			/* put received SessionInformation */
			ngLog.printSessionLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#doSession(): Session Information as below.\n" +
				protPullBack.getServerSessionInformationString());
			
			/* set SessionInfo of client */
			execInfo.setClientSessionInformation(this.sessionInfo);
			
			/* set status */
			setStatus("doSession", CLIENTSTATE_IDLE);
			/* reset cancel flag */
			this.isCanceled = false;
		} finally {
			/* unlock the handle */
			unlockHandle();
		}
		
		/* release callbackInfo */
		callbackFunc = null;
		callbackFuncInfo = null;

		return execInfo;
	}
	
	/**
	 * Sends request of suspendSession and wait for the reply.
	 * 
	 * @throws GrpcException if it failed to suspend.
	 */
	protected void suspendSession() throws GrpcException {
		try {
			/* lock the handle */
			lockHandle();

			int status = getLocalStatus();
			if ((status != CLIENTSTATE_WAIT) && (status != CLIENTSTATE_SUSPEND)) {
				/* invalid status */
				ngLog.printSessionLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_ERROR,
					this,
					"NgGrpcHandle#suspendSession(): invalid status.");
				throw new NgExecRemoteMethodException("failed to suspend");
			}
			/* suspend Ninf-G Executable */
			ngLog.printSessionLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				this,
				"NgGrpcHandle#suspendSession(): suspend Session.");
			Protocol suspendSessionRequest = new ProtSuspendSessionRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID,
				versionMajor, versionMinor, versionPatch);
			Protocol suspendSessionReply = 
						commManager.sendProtocol(suspendSessionRequest);
			/* check result of Reply */
			checkResult(suspendSessionReply);

			/* set status */
			setStatus("suspendSession", CLIENTSTATE_SUSPEND);
		} finally {
			/* unlock the handle */
			unlockHandle();
		}
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
	 * Sends request of invokeCallback and wait for the reply.
	 * 
	 * @param prot received Protocol(Notify of callback).
	 * @throws GrpcException if it failed to execute callback.
	 */
	protected void invokeCallback(Protocol prot) throws GrpcException {
		try {
			/* lock the handle */
			lockHandle();

			if ((isCanceled == true) || (getLocalStatus() != CLIENTSTATE_WAIT)) {
				/* invalid status */
				ngLog.printSessionLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_ERROR,
					this,
					"NgGrpcHandle#invokeCallback(): invalid status.");
				throw new NgExecRemoteMethodException("failed to invoke Callback");
			}
			/* set status */
			setStatus("invokeCallback", CLIENTSTATE_INVOKE_CALLBACK);

			/* get ID of callback */
			ProtInvokeCallbackNotify protInvokeCallback =
				(ProtInvokeCallbackNotify) prot;
			int id = protInvokeCallback.getID();
			int seq = protInvokeCallback.sequence;
			
			/* count callback */
			this.sessionInfo.incrementCallbackNTimesCalled();
		
			/* ----- Transform arguments of Ninf-G callback ----- */ 
			/* Request arguments for callback */
			timer.start();
			ProtTransferCallbackArgumentRequest cbTransferArgumentRequest =
				new ProtTransferCallbackArgumentRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID, id, seq,
				versionMajor, versionMinor, versionPatch);
			/* Receive arguments for callback */
			Protocol cbTransferArgumentReply =
				commManager.sendProtocol(cbTransferArgumentRequest);
			this.sessionInfo.setCallbackTransferArgumentRealTime(timer.getElapsedTime());
			/* check result of Reply */
			checkResult(cbTransferArgumentReply);
			
			/* create CallContext for callback */
			ProtTransferCallbackArgumentReply callbackTransferArgumentReply =
				(ProtTransferCallbackArgumentReply)cbTransferArgumentReply;
			CallContext callContext = new CallContext(
				(RemoteMethodInfo) callbackFuncInfo.get(id),
				callbackTransferArgumentReply.getArgumentData(), this.intArguments);

			/* get information about Compress */
			boolean compressEnable =
				new Boolean((String) remoteMachineInfo.get(
				RemoteMachineInfo.KEY_COMPRESS)).booleanValue();
			/* set CompressInformation */
			/*
			 * not implement on 2.4.0
			if (compressEnable == true) {
				this.sessionInfo.setCallbackInflateInformation(id,
						callbackTransferArgumentReply.getOriginalDataLength(),
						callbackTransferArgumentReply.getConvertedDataLength(),
						callbackTransferArgumentReply.getConvertRealTime(),		
						callbackTransferArgumentReply.getConvertCPUTime());			
			}
			 */

			/* ----- call Ninf-G callback function ----- */ 
			/* get callback Object */
			NgCallbackInterface callback =
				(NgCallbackInterface) callbackFunc.get(id);
			/* call callback function */
			timer.start();
			callback.callback(callContext.getArgs());
			this.sessionInfo.setCallbackCalculationRealTime(timer.getElapsedTime());
		
			/* ----- Transform results of Ninf-G callback ----- */ 
			/* transform Result */
			callContext.transformCBResult();
		
			/* Send result for callback */
			int compressThreshold =
				new Integer((String) remoteMachineInfo.get(
				RemoteMachineInfo.KEY_COMPRESS_THRESHOLD)).intValue();
			int blockSize =
				new Integer((String) remoteMachineInfo.get(
				RemoteMachineInfo.KEY_BLOCK_SIZE)).intValue();
			ProtTransferCallbackResultRequest cbTransferResultRequest =
				new ProtTransferCallbackResultRequest(
				commManager.generateSequenceNum(), context.getID(),
				executableID, sessionID, id, seq, callContext,
				commManager.getEncodeTypes(), compressEnable, compressThreshold,
				blockSize, versionMajor, versionMinor, versionPatch);	
			/* Receive reply for callback */
			timer.start();
			Protocol cbTransferResultReply =
				commManager.sendProtocol(cbTransferResultRequest);
			this.sessionInfo.setCallbackTransferResultRealTime(timer.getElapsedTime());
			/* check result of Reply */
			checkResult(cbTransferResultReply);

			/* set CompressInformation */
			/*
			 * not implement on 2.4.0
			if (compressEnable == true) {
				this.sessionInfo.setCallbackDeflateInformation(id,
						cbTransferResultRequest.getOriginalDataLength(),
						cbTransferResultRequest.getConvertedDataLength(),
						cbTransferResultRequest.getConvertRealTime(),		
						cbTransferResultRequest.getConvertCPUTime());			
			}
			*/
			
			/* set status */
			setStatus("invokeCallback", CLIENTSTATE_WAIT);
		} finally {
			/* unlock the handle */
			unlockHandle();
		}
	}
	
	/**
	 * Sends request of queryFunctionInfo and wait for the reply.
	 * 
	 * @return information of a RemoteFunction/RemoteMethod.
	 * @throws GrpcException if it failed to execute callback.
	 */
	private RemoteClassInfo getRemoteClassInfo() throws GrpcException {
		int status = getLocalStatus();
		if ((status == CLIENTSTATE_NONE) || (status == CLIENTSTATE_INIT)) {
			/* nothing will be done */
			return null;
		}

		/* get Information of Ninf-G Executable */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#getRemoteClassInfo(): get RemoteClassInfo.");
		/* send queryFunctionInfoRequest to Executable */
		Protocol queryFunctionInfoRequest = new ProtQueryFunctionInfoRequest(
			commManager.generateSequenceNum(), context.getID(),
			executableID, sessionID,
			versionMajor, versionMinor, versionPatch);
		/* receive queryFunctionInfoReply from Executable */
		ProtQueryFunctionInfoReply queryFunctionInfoReply = 
			(ProtQueryFunctionInfoReply)
			commManager.sendProtocol(queryFunctionInfoRequest);
		/* check result of Reply */
		checkResult(queryFunctionInfoReply);

		/* put RemoteClassInfo into NgInformationManager */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#getRemoteClassInfo(): put RemoteClassInfo to InformationManager.");
		NgInformationManager infoManager = context.getNgInformationManager();
		try {
			infoManager.lockInformationManager();
			
			infoManager.putRemoteClassInfo(className,
				queryFunctionInfoReply.getRemoteClassInfo());
		} finally {
			infoManager.unlockInformationManager();
		}
		
		/* return information */
		return queryFunctionInfoReply.getRemoteClassInfo();
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
	 * Checks result code in Protocol object.
	 * 
	 * @param protocol target Protocol.
	 * @throws GrpcException if result code is not 0 or received protocol is null.
	 */
	private void checkResult(Protocol protocol) throws GrpcException {
		/* check if protocol were null */
		if (protocol == null) {
			throw new NgException("protocol is null.");
		} else if (protocol.result != 0) {
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
	 * Sets status code of client.<br>
	 * If you set loglevel greater than 4, puts log message.
	 * 
	 * @param methodName a name of method(for log message).
	 * @param status status code to set.
	 */
	private void setStatus(String methodName, int status) {
		/* get String for status */
		String statString = null;
		switch (status) {
			case CLIENTSTATE_NONE:
			statString = "NONE";
			break;
			case CLIENTSTATE_IDLE:
			statString = "IDLE";
			break;
			case CLIENTSTATE_INIT:
			statString = "INIT";
			break;
			case CLIENTSTATE_INVOKE_SESSION:
			statString = "INVOKE_SESSION";
			break;
			case CLIENTSTATE_TRANSARG:
			statString = "TRANSARG";
			break;
			case CLIENTSTATE_WAIT:
			statString = "WAIT";
			break;
			case CLIENTSTATE_COMPLETE_CALCULATING:
			statString = "COMPLETE_CALCULATING";
			break;
			case CLIENTSTATE_TRANSRES:
			statString = "TRANSRES";
			break;
			case CLIENTSTATE_PULLBACK:
			statString = "PULLBACK";
			break;
			case CLIENTSTATE_SUSPEND:
			statString = "SUSPEND";
			break;
			case CLIENTSTATE_RESUME:
			statString = "RESUME";
			break;
			case CLIENTSTATE_DISPOSE:
			statString = "DISPOSE";
			break;
			case CLIENTSTATE_RESET:
			statString = "RESET";
			break;
			case CLIENTSTATE_CANCEL:
			statString = "CANCEL";
			break;
			case CLIENTSTATE_INVOKE_CALLBACK:
			statString = "INVOKE_CALLBACK";
			break;
			default:
			statString = "unknown...";
		}

		/* set status */
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			this,
			"NgGrpcHandle#" + methodName + ": client status -> " + statString + ".");
		this.status = new Integer(status);
	}
	
	/**
	 * Lock the handle.
	 * 
	 * @throws GrpcException if it's interrupted.
	 */
	private synchronized void lockHandle() throws GrpcException {
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
	 * Unlock the handle.
	 * 
	 * @throws GrpcException  if it's interrupted.
	 */
	private synchronized void unlockHandle() throws GrpcException {
		/* check if it's locked */
		if (isLocked == false) {
			throw new NgException("Nobody lock the handle.");
		}
		/* unlock */
		isLocked = false;
		/* notifyAll */
		notifyAll();
	}
	
	/**
	 * @return
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
	 *
	 * @return 
	 */
	private static Properties getDefaultServerProperties(Properties prop) {
		Properties tmpProperties = new Properties();
		tmpProperties.put(RemoteMachineInfo.KEY_HOSTNAME, prop.get(RemoteMachineInfo.KEY_HOSTNAME));
		return tmpProperties;
	}
	
}
