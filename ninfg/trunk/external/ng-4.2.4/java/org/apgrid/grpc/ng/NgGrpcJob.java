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
 * $RCSfile: NgGrpcJob.java,v $ $Revision: 1.43 $ $Date: 2006/01/24 06:56:33 $
 */
package org.apgrid.grpc.ng;

import java.util.Properties;
import java.util.Vector;

import org.apgrid.grpc.ng.info.*;
import org.apgrid.grpc.util.*;
import org.globus.io.gass.server.GassServer;
import org.gridforum.gridrpc.GrpcException;

class NgGrpcJob {
	/* system property for TMP_DIR */
	public static final String TMP_DIR = "TMP_DIR";
	public static final String TMP_FILE_PREFIX = "ngtmpfile";
	public static final String TMP_FILE_SUFFIX = null;
	
	/* definitions for status of JOBs */
	protected static final int INVALID_JOB_ID = -1;
	protected static final int NGJOB_STATE_NONE = 0;
	protected static final int NGJOB_STATE_PENDING = 1;
	protected static final int NGJOB_STATE_ACTIVE = 2;
	protected static final int NGJOB_STATE_DONE = 3;
	protected static final int NGJOB_STATE_FAILED = 4;
	
	/* instance variables */
	private NgGrpcClient context;
	private RemoteMachineInfo remoteMachineInfo;
	private String className;
	private int jobID;
	private String jobType;
	private int jobCount;
	private int exitCount = 0;
	private boolean requiredJobCancel = false;
	private int ngJobStatus = NGJOB_STATE_NONE;
	private String invokeServerType;
	private String jobIDString;
	private NgInvokeServer ngInvokeServer;
	private NgLog ngLog;
	private String fileStdout;
	private String fileStderr;
	private boolean sendDestroy = false;
	
	/* GRAM Job for GRPC */
	private NgGramJob ngJob;
	
	/* conditions for status */
	private CondWait condActive;
	private CondWait condDone;
	
	/**
	 * @param context
	 * @param remoteMachineInfo
	 * @param className
	 * @param JobID
	 * @param jobType
	 * @param jobCount
	 * @throws GrpcException
	 */
	public NgGrpcJob(NgGrpcClient context,
		RemoteMachineInfo remoteMachineInfo, String className,
		int JobID, String jobType, int jobCount)
		throws GrpcException {
		this.context = context;
		this.remoteMachineInfo = remoteMachineInfo;
		this.className = className;
		this.jobID = JobID;
		this.jobType = jobType;
		this.jobCount = jobCount;
		this.condActive = new CondWait();
		this.condDone = new CondWait();
		this.invokeServerType =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_INVOKE_SERVER);
		this.ngInvokeServer = null;
		this.ngLog = context.getNgLog();

		if (invokeServerType.equals(RemoteMachineInfo.VAL_INVOKE_SERVER_NONE)) {
			/* using GRAM */
			this.ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgGrpcJob: Invoke Ninf-G Executable by GRAM.");

			/* create NgGramJob */
			this.ngJob = new NgGramJob(context, remoteMachineInfo, this);
			/* start JOB by Pre-WS GRAM */
			ngJob.invokeExecutable();
		} else {
			/* using InvokeServer */
			this.ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgGrpcJob: Invoke Ninf-G Executable by InvokeServer.");

			/* create tmpfile for InvokeServer */
			String redirect_outerr =
				(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_REDIRECT_OUTERR);
			if ((redirect_outerr != null) &&
				(redirect_outerr.equalsIgnoreCase("true"))) {
				String tmp_dir = 
					(String) context.getNgInformationManager().getLocalMachineInfo().get(NgInformationManager.KEY_CLIENT_TMP_DIR);
				if (tmp_dir != null) {
					this.fileStdout = tmp_dir + "/" + TMP_FILE_PREFIX + jobID + ".out";
					this.fileStderr = tmp_dir + "/" + TMP_FILE_PREFIX + jobID + ".err";
				} else {
					this.fileStdout = TMP_FILE_PREFIX + jobID + ".out";
					this.fileStderr = TMP_FILE_PREFIX + jobID + ".err";
				}
			} else {
				this.fileStdout = null;
				this.fileStderr = null;
			}
			/* get InvokeServer */
			this.ngInvokeServer = this.context.getInvokeServer(this);
			/* start JOB by InvokeServer */
			this.ngInvokeServer.createJob(this);
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void waitForActive() throws GrpcException {
		/* wait for changing status */
		condActive.waitFor();
		/* check status of job, throw Exception when it's not active */
		if (this.ngJobStatus != NGJOB_STATE_ACTIVE) {
			throw new NgException("Couldn't activate JOB.");
		}
	}

	/**
	 * @param timeout
	 * @throws GrpcException
	 */
	protected int waitForActive(long timeout) throws GrpcException {
		/* wait for changing status */
		int result = condActive.waitFor(timeout);
		/* check status of job, throw Exception when it's not active */
		if ((result != 0) || (this.ngJobStatus != NGJOB_STATE_ACTIVE)){
			throw new NgException("Couldn't activate JOB.");
		}
		
		return result;
	}

	/**
	 * @throws GrpcException
	 */
	protected void waitForDone() throws GrpcException {
		/* wait for changing status */
		condDone.waitFor();
		/* check status of job, throw Exception when it's not active */
		if (this.ngJobStatus != NGJOB_STATE_DONE) {
			throw new NgException("Couldn't deactivate JOB.");
		}
	}

	/**
	 * @param timeout
	 * @throws GrpcException
	 */
	protected int waitForDone(long timeout) throws GrpcException {
		/* wait for changing status */
		int result = condDone.waitFor(timeout);
		/* check status of job, throw Exception when it's not active */
		if ((result != 0) || (this.ngJobStatus != NGJOB_STATE_DONE)){
			throw new NgException("Couldn't deactivate JOB.");
		}
		
		return result;
	}

	/**
	 * @return
	 * @throws GrpcException
	 */
	protected int getStatus() throws GrpcException {
		return this.ngJobStatus;
	}
	
	/**
	 * @param status
	 */
	protected void setStatus(int status) {
		/* set new status */
		this.ngJobStatus = status;
		this.ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgGrpcJob#setStatus: new status is [" + status + "].");
		
		/* set conditions */
		switch (status) {
		case NGJOB_STATE_DONE:
		case NGJOB_STATE_FAILED:
			if (condDone != null) {
				condDone.set();
			}
		case NGJOB_STATE_ACTIVE:
			if (condActive != null) {
				condActive.set();
			}
			break;
		default:
			this.ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_ERROR,
				context,
				"NgGrpcJob#setStatus: Required to set invalid job status.");
		}
	}
	
	/**
	 * 
	 */
	protected void incrementExitCount() throws GrpcException {
		this.exitCount++;
		this.ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgGrpcJob#incrementExitCount: exit count is [" + this.exitCount + "].");
		
		/* unregister JOB from context */
		if (this.exitCount == jobCount) {
			this.context.unregisterJob(this.jobID);
		}
		
		/* request cancel JOB if it's needed */
		if ((this.exitCount == jobCount) && (this.requiredJobCancel == true)) {
			if ((this.ngInvokeServer != null) && (this.sendDestroy == false)) {
				/* set send flag */
				this.sendDestroy = true;
				/* destroy JOB for canceling JOB */
				this.ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_DEBUG,
						context,
						"NgGrpcJob#incrementExitCount: send destroyJob to Invoke Server");
				ngInvokeServer.destroyJob(jobIDString);
			} else if (this.requiredJobCancel == true) {
				/* canceling JOB */
				this.ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_INFO,
						context,
						"NgGrpcJob#incrementExitCount: cancel GRAM JOB.");
				ngJob.cancel();
			}
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void dispose() throws GrpcException {
		if ((this.exitCount == jobCount) && (this.requiredJobCancel == false)) {
			if ((this.ngInvokeServer != null) && (this.sendDestroy == false)) {
				/* set send flag */
				this.sendDestroy = true;
				/* destroy JOB */
				this.ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_DEBUG,
						context,
						"NgGrpcJob#dispose: send destroyJob to Invoke Server");
				ngInvokeServer.destroyJob(jobIDString);
			}
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void setRequiredJobCancel() throws GrpcException {
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			context,
			"NgGrpcJob#setRequiredJobCancel: set request of cancelling JOB.");
		this.requiredJobCancel = true;
	}
	
	/**
	 * @return
	 */
	protected String getInvokeServerType() {
		return this.invokeServerType;
	}
	
	/**
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfo() {
		return this.remoteMachineInfo;
	}
	
	/**
	 * @return
	 */
	protected String getClassName() {
		return this.className;
	}
	
	/**
	 * @return
	 */
	protected String getJobType() {
		return this.jobType;
	}
	
	/**
	 * @return
	 */
	protected int getJobCount() {
		return this.jobCount;
	}

	/**
	 * @return
	 */
	protected int getRequestID() {
		return this.jobID;
	}
	
	/**
	 * @param jobIDString
	 */
	protected void setJobIDString(String jobIDString) {
		/* this must be set by NgInvokeServer */
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				context,
				"NgGrpcJob#setJobIDString: JOB ID is [" + jobIDString + "]");
		this.jobIDString = jobIDString;
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	protected NgGrpcClient getClient() throws GrpcException {
		return this.context;
	}
	
	/**
	 * @param ngJob
	 * @return
	 * @throws GrpcException
	 */
	protected String[] getArgumentList() throws GrpcException {
		/* list for argument */
		Vector listArg = new Vector();
		/* get RemoteMachineInfo from NggrpcJob */
		
		/* set initiator */
		Properties localHostInfo = null;
		try {
			context.getNgInformationManager().lockInformationManager();
			
			localHostInfo = context.getNgInformationManager().getLocalMachineInfo();
		} finally {
			context.getNgInformationManager().unlockInformationManager();
		}

		/* decide port number to send Executable */
		int clientPort = 0;
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CRYPT).equals("none")) {
			clientPort = context.getPortManagerNoSecure().getPort();
		} else if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CRYPT).equals("SSL")) {
			clientPort = context.getPortManagerSSL().getPort();
		} else if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CRYPT).equals("GSI")) {
			clientPort = context.getPortManagerGSI().getPort();
		}

		/* set hostname and port */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CLIENT_HOSTNAME) != null) {
			listArg.add("--client=" +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_CLIENT_HOSTNAME) +
					":" + clientPort);
		} else {
			listArg.add("--client=" +
					localHostInfo.get(NgInformationManager.KEY_CLIENT_HOSTNAME) +
					":" + clientPort);
		}

		/* set GASS server */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_GASS_SCHEME) != null) {
			GassServer gassServer;
			if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_GASS_SCHEME).equals("http")) {
				gassServer = context.getGassServerNoSecure();
			} else if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_GASS_SCHEME).equals("https")) {
				gassServer = context.getGassServerSecure();
			} else {
				/* invalid GASSServer */
				throw new NgException(
					"NgInvokeServer#createJob: invalid GASS scheme was specified.");
			}
			listArg.add("--gassServer=" + gassServer.getURL());
		}
		
		/* set crypt mode */
		listArg.add("--crypt=" +
			remoteMachineInfo.get(RemoteMachineInfo.KEY_CRYPT));
		/* set protocol mode */
		listArg.add("--protocol=" +
			remoteMachineInfo.get(RemoteMachineInfo.KEY_PROTOCOL));
			
		/* set contextID */
		listArg.add("--contextID=" + context.getID());
		/* set jobID */
		listArg.add("--jobID=" + this.jobID);
		
		/* heartbeat */
		listArg.add("--heartbeat=" +
				remoteMachineInfo.get(RemoteMachineInfo.KEY_HEARTBEAT));
		
		/* size of core */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CORE_SIZE) != null) {
			listArg.add("--coreDumpSize=" + 
				remoteMachineInfo.get(RemoteMachineInfo.KEY_CORE_SIZE));
		}

		/* set debug (check if debug was activated) */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_DEBUG).equals("true")) {
			if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_DEBUG_DISPLAY) != null) {
				listArg.add("--debugDisplay=" + remoteMachineInfo.get(
					RemoteMachineInfo.KEY_DEBUG_DISPLAY));
			}
			if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_DEBUG_TERM) != null) {
				listArg.add("--debugTerminal=" + remoteMachineInfo.get(
					RemoteMachineInfo.KEY_DEBUG_TERM));
			}
			if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_DEBUG_DEBUGGER) != null) {
				listArg.add("--debugger=" + remoteMachineInfo.get(
					RemoteMachineInfo.KEY_DEBUG_DEBUGGER));
			}
			listArg.add("--debugEnable=1");
		}
		
		/* busy loop */
		if ((remoteMachineInfo.get(RemoteMachineInfo.KEY_DEBUG_BUSYLOOP) != null) &&
			(remoteMachineInfo.get(RemoteMachineInfo.KEY_DEBUG_BUSYLOOP).equals("true"))) {
			listArg.add("--debugBusyLoop=1");
		}
		
		/* set tcpNoDelay */
		if (Boolean.valueOf((String)remoteMachineInfo.get(RemoteMachineInfo.KEY_TCP_NODELAY)).booleanValue()) {
			listArg.add("--tcpNodelay");
		}
		
		/* set tcp connect retry */
		if (Integer.valueOf((String)remoteMachineInfo.get(RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_COUNT)).intValue() > 0) {
			String random =
				remoteMachineInfo.get(RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_RANDOM).equals("true") ? "random" : "fixed";
			listArg.add("--connectRetry=" +
				remoteMachineInfo.get(RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_COUNT) + "," +
				remoteMachineInfo.get(RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_BASEINTERVAL) + "," +
				remoteMachineInfo.get(RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_INCREASERATIO) + "," +
				random);
		}
		
		/* create return value */
		String[] arrayArgument = new String[listArg.size()];
		for (int i = 0; i < listArg.size(); i++) {
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_DEBUG,
					context,
					"NgGrpcJob#getArgumentList: argument [" + i + "]" +
					" is [" + listArg.get(i) + "]");
			arrayArgument[i] = (String) listArg.get(i);
		}
		
		return arrayArgument;
	}
	
	/**
	 * @return
	 */
	protected String getStdout() {
		return this.fileStdout;
	}
	
	/**
	 * @return
	 */
	protected String getStderr() {
		return this.fileStderr;
	}
}
