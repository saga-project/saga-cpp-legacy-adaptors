/*
 * $RCSfile: NgGrpcJob.java,v $ $Revision: 1.24 $ $Date: 2008/03/28 03:25:55 $
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

import java.util.Vector;
import java.util.List;

import org.apgrid.grpc.ng.info.*;
import org.apgrid.grpc.ng.NgInitializeGrpcClientException;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;
import static org.apgrid.grpc.ng.info.RemoteMachineInfo.INVOKE_SERVER_NONE;

/*
 *
 */
class NgGrpcJob {
	// system property for TMP_DIR 
	public static final String TMP_DIR         = "TMP_DIR";
	public static final String TMP_FILE_PREFIX = "ngtmpfile";
	// public static final String TMP_FILE_SUFFIX = null; 

	// definitions for status of JOBs
	protected static final int INVALID_JOB_ID      = -1;

	// instance variables 
	private NgGrpcClient context;
	private RemoteMachineInfo remoteMachineInfo;
	private String hostName;
	private String className;
	private int jobID;
	private String jobType;
	private int jobCount;
	private int exitCount = 0;
	private boolean requiredJobCancel = false;
	private JobStatus ngJobStatus = new JobStatus();

	private String invokeServerType;
	private String jobIDString;
	private NgInvokeServer ngInvokeServer;
	private NgLog ngLog;
	private String fileStdout;
	private String fileStderr;
	private boolean sendDestroy = false;
	private int simpleAuthNo;
	
	// conditions for status
	private CondWait condActive;
	private CondWait condDone;

        // for ClientCommunicationProxy
	private ClientCommunicationProxy ccp;
        private List<NgAttribute> ccpInfo;

	/**
	 * @param context
	 * @param remoteMachineInfo
	 * @param className
	 * @param JobID
	 * @param jobType
	 * @param jobCount
	 */
	public NgGrpcJob(
		NgGrpcClient context, RemoteMachineInfo remoteMachineInfo,
		String hostName, String className, int JobID, String jobType,
		int jobCount) throws NgException {
		CommunicationProxyInfo cpi;
		String ccpType;

		this.context = context;
		this.remoteMachineInfo = remoteMachineInfo;
		this.hostName = hostName;
		this.className = className;
		this.jobID = JobID; // request ID, not Created Job ID
		this.jobType = jobType;
		this.jobCount = jobCount;
		this.condActive = new CondWait();
		this.condDone   = new CondWait();
		this.invokeServerType = remoteMachineInfo.getInvokeServer();
		this.ngInvokeServer = null;
		cpi = remoteMachineInfo.getCommunicationProxyInfo();
		ccpType = cpi.getCommunicationProxy();
		if (ccpType != RemoteMachineInfo.DISABLE_COMMUNICATION_PROXY) {
			ClientCommunicationProxyManager ccpm = context.getClientCommunicationProxyManager();
			this.ccp = ccpm.get(ccpType);
			this.ccpInfo = ccp.getInfo(remoteMachineInfo);
		}
		this.ngLog = context.getNgLog();
		this.simpleAuthNo = -1;
		checkNoOfCPUs();
	}

	/*
	 * Invoke Ninf-G Job using Invoke Server
	 * @throws NgInitializeGrpcClientException if it doesn't specified Invoke 
	 *          Server type.
	 */
	public void create() throws GrpcException {
		if (this.invokeServerType.equals(INVOKE_SERVER_NONE)) {
			throw new NgInitializeGrpcClientException(
				"Invoke server is not specified");
		}

		// using InvokeServer
		this.ngLog.logInfo(CAT_NG_INTERNAL, context.logHeader()
			+ "NgGrpcJob: Invoke Ninf-G Executable by InvokeServer.");

		// create tmpfile for InvokeServer
		setTmpFileForInvokeServer( remoteMachineInfo.getRedirectOuterr(),
			context.getLocalMachineInfo().getTmpDir());

		// get InvokeServer 
		this.ngInvokeServer = this.context.getInvokeServer(this);

		// start JOB by InvokeServer
		this.ngInvokeServer.createJob(this);
	}

	/*
	 * set fileStdout, fileStderr fields
	 * This depend on jobId & TMP_FILE_PREFIX field
	 */
	private void setTmpFileForInvokeServer(String redirect_outerr, 
	 String tmp_dir) {
		if ((redirect_outerr != null) && Boolean.valueOf(redirect_outerr)) {
			this.fileStdout = TMP_FILE_PREFIX + jobID + ".out";
			this.fileStderr = TMP_FILE_PREFIX + jobID + ".err";
			if (tmp_dir != null) {
				this.fileStdout = tmp_dir + "/" + this.fileStdout;
				this.fileStderr = tmp_dir + "/" + this.fileStderr;
			}
			return ;
		}
		this.fileStdout = null;
		this.fileStderr = null;
		return;
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void waitForActive() throws GrpcException {
		// wait for changing status 
		condActive.waitFor();
		// check status of job, throw Exception when it's not active 
		if ( ! ngJobStatus.isActive() ) {
			throw new NgException("Couldn't activate JOB.");
		}
	}

	/**
	 * @param timeout
	 * @throws GrpcException
	 */
	protected int waitForActive(long timeout) throws GrpcException {
		// wait for changing status 
		int result = condActive.waitFor(timeout);
		// check status of job, throw Exception when it's not active 
		if ((result != 0) || (! ngJobStatus.isActive()) ){
			throw new NgException("Couldn't activate JOB.");
		}
		
		return result;
	}

	/**
	 * @throws GrpcException
	 */
	public synchronized void waitForDone() throws GrpcException {
		if ( ngJobStatus.isDone() || ngJobStatus.isFailed() )
			return; // died
		// wait for changing status 
		condDone.waitFor();
		// check status of job, throw Exception when it's not active 
		if (! ngJobStatus.isDone() ) {
			throw new NgException("Couldn't deactivate JOB.");
		}
	}

	/**
	 * @param timeout
	 * @throws GrpcException
	 */
	public synchronized int waitForDone(long timeout) throws GrpcException {
		// wait for changing status 
		int result = condDone.waitFor(timeout);
		// check status of job, throw Exception when it's not active 
		if ((result != 0) || (! ngJobStatus.isDone() )){
			throw new NgException("Couldn't deactivate JOB.");
		}

		return result;
	}

	/**
	 * @return
	 * @throws GrpcException
	 */
	protected int getStatus() throws GrpcException {
		return this.ngJobStatus.getStatus();
	}
	
	/**
	 * @param status
	 */
	protected void setStatus(int status) {
		// set new status 
		this.ngJobStatus.set(status);

		this.ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader()
				+ "NgGrpcJob#setStatus: new status is [" + status + "].");

		setConditions(status);
	}

	// comes off setStatus
	private void setConditions(final int status) {
		switch (status) {
			case JobStatus.DONE:
			case JobStatus.FAILED:
				if (condDone != null) {
					condDone.set();
				}
			case JobStatus.ACTIVE:
				if (condActive != null) {
					condActive.set();
				}
				break;
			default:
				this.ngLog.logError(CAT_NG_INTERNAL, context.logHeader()
					+ "NgGrpcJob#setConditions: Required to set invalid job status.");
		}
	}
	

	protected void incrementExitCount() throws GrpcException {
		exitCount++;
		ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader()
			+ "NgGrpcJob#incrementExitCount: exit count is [" + exitCount + "].");

		// unregister JOB from context 
		if (exitCount == jobCount) {
			context.unregisterJob(jobID);
		}

		// request cancel JOB if it's needed 
		if ((exitCount == jobCount) && (requiredJobCancel)) {
			if ( ! isSendDestroyToInvokeServer() ) {
				sendDestroy = true; // set send flag 
				// destroy JOB for canceling JOB 
				ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader()
					+ "NgGrpcJob#incrementExitCount: send destroyJob to Invoke Server");
				ngInvokeServer.destroyJob(jobIDString);
			} else if (requiredJobCancel) {
				throw new NgException("Unsupported GRAM job using NgGramJob on Ninf-G5");
				/*
                this.ngLog.logInfo(CAT_NG_INTERNAL,
					context.logHeader() +
					"NgGrpcJob#incrementExitCount: cancel GRAM JOB.");
                ngJob.cancel();  // Gram job cancel
				*/
			}
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	public void dispose() throws GrpcException {
		if ((exitCount == jobCount) && (! requiredJobCancel)) {
			if ( ! isSendDestroyToInvokeServer() ) {
				sendDestroy = true; // set send flag 
				// destroy JOB 
				ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader()
					+ "NgGrpcJob#dispose: send destroyJob to Invoke Server");
				ngInvokeServer.destroyJob(jobIDString);
			}
			if (ccp != null) {
				ccp.releaseProxy();
			}
		}
	}

	private void checkNoOfCPUs() throws NgException {
		if (jobType == null) {
			return;
		}
		if (!jobType.equals(RemoteMachineInfo.VAL_BACKEND_MPI) &&
		    !jobType.equals(RemoteMachineInfo.VAL_BACKEND_BLACS)) {
			return;
		}

		int nCPUs = remoteMachineInfo.getNumCPUs(className);
		if (nCPUs <= 0) {
			throw new NgException(
				"No of CPUs " + nCPUs + " for MPI function \""
				+ className + "\" hostname \"" + hostName
				+ "\" is invalid.");
		}
	}

	private boolean isSendDestroyToInvokeServer() {
		if (ngInvokeServer == null) {
			return false;
		}
		return sendDestroy;
	}

	/**
	 * @throws GrpcException
	 */
	protected void setRequiredJobCancel() throws GrpcException {
		ngLog.logInfo(CAT_NG_INTERNAL, context.logHeader()
			+ "NgGrpcJob#setRequiredJobCancel: set request of cancelling JOB.");
		requiredJobCancel = true;
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
	 * Set created job id.
	 * @param jobID created job id by NgInvokeServer
	 */
	protected void setJobIDString(String jobID) {
		// this must be set by NgInvokeServer
		ngLog.logInfo(CAT_NG_INTERNAL, context.logHeader()
			+ "NgGrpcJob#setJobIDString: JOB ID is [" + jobID + "]");

		this.jobIDString = jobID;
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	protected NgGrpcClient getClient() throws GrpcException {
		return this.context;
	}

	public void setSimpleAuthNo(int number) {
		this.simpleAuthNo = number;
	}

	/**
	 * @return String[] Arguments list for InvokeServer
	 * @throws GrpcException
	 */
	protected String[] getArgumentList() throws GrpcException {
		// list for argument 
		Vector<String> listArg = new Vector<String>();

		// Set Debug Info (check if debug was activated) 
		DebugInfo debugInfo = remoteMachineInfo.getDebugInfo();
		if ( Boolean.valueOf(debugInfo.getDebug()) ) {
			if (debugInfo.getDebugDisplay() != null) {
				listArg.add("--debugDisplay=" + debugInfo.getDebugDisplay());
			}
			if (debugInfo.getDebugTerm() != null) {
				listArg.add("--debugTerminal=" + debugInfo.getDebugTerm());
			}
			if (debugInfo.getDebugDebugger() != null) {
				listArg.add("--debugger=" + debugInfo.getDebugDebugger());
			}
			listArg.add("--debugEnable=1");
		}
		if ((debugInfo.getDebugBusyloop() != null) &&
			Boolean.valueOf(debugInfo.getDebugBusyloop()))  {
			listArg.add("--debugBusyLoop=1");
		}

		// decide port number to send Executable 
		listArg.add( getArgumentClient() );

		listArg.add("--contextID=" + context.getID()); // set contextID 
		listArg.add("--jobID=" + this.jobID);          // set jobID 
		
		// heartbeat 
		listArg.add("--heartbeat=" 
			+ remoteMachineInfo.getHeartbeatInfo().getHeartbeat());
		
		// size of core 
		if (remoteMachineInfo.getCoreSize() != null) {
			listArg.add("--coreDumpSize=" + remoteMachineInfo.getCoreSize());
		}

		// set tcpNoDelay 
		if ((remoteMachineInfo.getTcpNodelay() != null) &&
			Boolean.valueOf(remoteMachineInfo.getTcpNodelay())) {
			listArg.add("--tcpNodelay=1");
		} else {
			listArg.add("--tcpNodelay=0");
		}

		// Set Tcp Connect Info
		String arg = getArgumentTcpConnectInfo();
		if ( arg != null ) {
			listArg.add( arg );
		}

		// Set ClientCommunicationProxy Info to InvokeServer option
                if ( ccpInfo != null) {

                    listArg.add("--communicationProxyType="+
                                remoteMachineInfo.getCommunicationProxyInfo().getCommunicationProxy());

                    if ( remoteMachineInfo.getCommunicationProxyInfo()
                         .getCommunicationProxyPath() != null ) {
                        listArg.add("--communicationProxyPath="+
                                    remoteMachineInfo.getCommunicationProxyInfo()
                                    .getCommunicationProxyPath());
                    }

                    for(NgAttribute attr : ccpInfo) {
                        if ((attr.getName().equals("request_id"))||
                            (attr.getName().equals("result")    )  ) {
                            continue;
                        }
                        listArg.add("--communicationProxyOption="+
                                    UUEncodeDash.uuEncodeDash 
                                    (attr.getName() + " " + attr.getValue()));
                    }
			listArg.add("--communicationProxyOption="+
				UUEncodeDash.uuEncodeDash 
				("buffer_size " + remoteMachineInfo.getCommunicationProxyInfo().getCommunicationProxyBufferSize()));

                    if (
                        this
                        .remoteMachineInfo
                        .getCommunicationProxyInfo()
                        .getCommunicationProxyOption()
                        != null) {
                        for (String e :
                                 this
                                 .remoteMachineInfo
                                 .getCommunicationProxyInfo()
                                 .getCommunicationProxyOption()
                                ) {
                            listArg.add("--communicationProxyOption="+
                                        UUEncodeDash.uuEncodeDash(e));
                        }
                    }
                }

		// create return value 
		String[] arguments = new String[listArg.size()];
		for (int i = 0; i < listArg.size(); i++) {
			ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader()
				+ "NgGrpcJob#getArgumentList: argument [" + i + "]" 
					+ " is [" + listArg.get(i) + "]");

			arguments[i] = listArg.get(i);
		}

		return arguments;
	}

///// Helper methods for getArgumentList()  begin

	private String getArgumentClient() {
		StringBuilder sb = new StringBuilder("--connectbackAddress=ng_tcp://");

		if (remoteMachineInfo.getClientHostname() != null) {
			sb.append(remoteMachineInfo.getClientHostname() + ":" );
		} else {
			LocalMachineInfo lmi = context.getLocalMachineInfo();
			sb.append(lmi.getHostname() + ":" );
		}

		int clientPort = context.getPortManagerNoSecure().getPort();
		sb.append(clientPort);
		sb.append("/");

		return sb.toString();
	}

	private String getArgumentTcpConnectInfo() {
		TcpConnectInfo tcpConnectInfo = remoteMachineInfo.getTcpConnectInfo();
		if (Integer.parseInt(tcpConnectInfo.getTcpConnectRetryCount()) <= 0) {
			return null;
		}
		String random = "random";
		if (! Boolean.valueOf(tcpConnectInfo.getTcpConnectRetryRandom())) {
			random =  "fixed";
		}
		StringBuilder sb = new StringBuilder("--connectRetry=");
		sb.append( tcpConnectInfo.getTcpConnectRetryCount() + "," );
		sb.append( tcpConnectInfo.getTcpConnectRetryBaseinterval() + "," );
		sb.append( tcpConnectInfo.getTcpConnectRetryIncreaseratio() + "," );
		sb.append( random );
		return sb.toString();
	}

///// Helper methods for getArgumentList() end

	protected int getSimpleAuthNumber() {
		return simpleAuthNo;
	}

	/**
	 * @return file path of Ninf-G Job stdout
	 */
	protected String getStdout() {
		return this.fileStdout;
	}
	
	/**
	 * @return file path of Ninf-G Job stderr
	 */
	protected String getStderr() {
		return this.fileStderr;
	}
}
