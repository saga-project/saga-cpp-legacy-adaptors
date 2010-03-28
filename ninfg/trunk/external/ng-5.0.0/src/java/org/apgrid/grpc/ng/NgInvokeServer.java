/*
 * $RCSfile: NgInvokeServer.java,v $ $Revision: 1.20 $ $Date: 2008/03/28 03:25:55 $
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

import java.io.BufferedReader;
import java.io.LineNumberReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.StringReader;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.io.FileNotFoundException;
import java.io.IOException;

import org.apgrid.grpc.util.NgUtil;
import org.apgrid.grpc.ng.info.InvokeServerInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.gridforum.gridrpc.GrpcException;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.HOSTNAME;

class NgInvokeServer implements ExtModule {

	// requested & create NgGrpcJob map
	private Map<Integer, NgGrpcJob> requestedJobs;
	private Map<String, NgGrpcJob>  createdJobs;
	private Map<String, NgGrpcJob>  destroyedJobs;

	private NgInvokeServerManager invokeServerManager;
	private NgGrpcClient context;
	private NgLog ngLog;
	private int ID;
	private int requestCount;
	private int jobCount; // number of requested job
	private int destroyedJobCount;
	private int maxJobs;
	private int statusPolling;
	private List createOptions;
	private boolean isLocked;
	private boolean isExited;
	private boolean isValid;
	private String type;
	private boolean stagingAuthNumber = false;
	private boolean stagingCommunicationProxy = false;

	private static final String NG_DIR = "NG_DIR"; // environment val key
	private static final String DefaultCommandName = "ng_invoke_server";
	private static final String CommunicationProxyCommandName = "ng_remote_communication_proxy";

	// protocol definitions 
	// REQUESTS
	private static final String JOB_CREATE  = "JOB_CREATE";
	private static final String JOB_STATUS  = "JOB_STATUS";
	private static final String JOB_DESTROY = "JOB_DESTROY";
	private static final String EXIT        = "EXIT";

	private static final String REPLY_SUCCESS = "S";
	private static final String REPLY_FAILED  = "F";

	private static final String STATUS_PENDING = "PENDING";
	private static final String STATUS_ACTIVE  = "ACTIVE";
	private static final String STATUS_DONE    = "DONE";
	private static final String STATUS_FAILED  = "FAILED";

	private static final String CREATE_NOTIFY = "CREATE_NOTIFY";
	private static final String STATUS_NOTIFY = "STATUS_NOTIFY";

	// arguments for  JOB_CREATE request
	private static final String ARG_HOSTNAME = "hostname";
	private static final String ARG_PORT = "port";
	private static final String ARG_JOBMANAGER = "jobmanager";
	private static final String ARG_SUBJECT = "subject";
	private static final String ARG_CLIENT_NAME = "client_name";
	private static final String ARG_EXECUTABLE_PATH = "executable_path";
	private static final String ARG_BACKEND = "backend";
	private static final String ARG_COUNT = "count";
	private static final String ARG_STAGING = "staging";
	private static final String ARG_ARGUMENT = "argument";
	private static final String ARG_WORK_DIRECTORY = "work_directory";
	private static final String ARG_REDIRECT_ENABLE = "redirect_enable";
	private static final String ARG_STDOUT_FILE = "stdout_file";
	private static final String ARG_STDERR_FILE = "stderr_file";
	private static final String ARG_ENVIRONMENT = "environment";
	private static final String ARG_TMP_DIR = "tmp_dir";
	private static final String ARG_STATUS_POLLING = "status_polling";
	private static final String ARG_REFRESH_CREDENTIAL = "refresh_credential";
	private static final String ARG_MAX_TIME = "max_time";
	private static final String ARG_MAX_WALL_TIME = "max_wall_time";
	private static final String ARG_MAX_CPU_TIME = "max_cpu_time";
	private static final String ARG_QUEUE_NAME = "queue_name";
	private static final String ARG_PROJECT = "project";
	private static final String ARG_HOST_COUNT = "host_count";
	private static final String ARG_MIN_MEMORY = "min_memory";
	private static final String ARG_MAX_MEMORY = "max_memory";
	private static final String ARG_RSL_EXTENSION = "rsl_extensions";
	private static final String ARG_AUTH_NUMBER = "auth_number";
	private static final String ARG_COMMUNICATION_PROXY_STAGING = "communication_proxy_staging";
	private static final String ARG_COMMUNICATION_PROXY_PATH = "communication_proxy_path";

	// communicators to InvokeServer 
	private ProcessCommunicator process = null;

	/**
	 * @param isMng
	 * @param context
	 * @param isType
	 * @param ID
	 * @param isInfo
	 * @throws GrpcException
	 */
	NgInvokeServer(NgInvokeServerManager isMng, NgGrpcClient context,
	 String isType, int ID, InvokeServerInfo isInfo)
	 throws GrpcException {
		// create Map for JOBs 
		this.requestedJobs = new HashMap<Integer, NgGrpcJob>();
		this.createdJobs   = new HashMap<String, NgGrpcJob>();
		this.destroyedJobs = new HashMap<String, NgGrpcJob>();
		
		// set InvokeServerManager and NgLog 
		this.context = context;
		this.invokeServerManager = isMng;
		this.ngLog = context.getNgLog();
		this.ID = ID;
		this.type = isType;

		if (isInfo != null) {
			// get information from InvokeServerInfo
			this.maxJobs = isInfo.getMaxJobs();
			this.createOptions = isInfo.getOptions();
			this.statusPolling = isInfo.getStatusPolling();
		} else {
			// set default value 
			this.maxJobs = 0;
			this.createOptions = null;
			this.statusPolling = 0;
		}

		this.isLocked = false; // reset lock flag 
		this.isExited = false; // reset exit flag

		// reset requestCount, jobCount 
		this.requestCount = 0;
		this.jobCount = 0;

		logInfoInternal("new : run Invoke Server process.");

		// launch InvokeServer 
		try {
			// get logfile for InvokeServer
			String isLogFile = getLogFilePath(isInfo);

			// create command line for InvokeServer 
			String[] isCommand = createCommandLine(isInfo, isLogFile);

			// launch InvokeServer 
			logDebugInternal("new() : Invoke Server command is " 
				+ isCommand[0]);
			this.process = new ProcessCommunicator(isCommand);
			queryFeatures();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		// turn on validate flag
		this.isValid = true;
	} 

	private String getLogFilePath(InvokeServerInfo isInfo) {
		String ret = null;
		if (isInfo != null) {
			ret = isInfo.getLogFilePath();
			if (ret != null) { return ret; }
		}
		// Did not set log_filePath in <INVOKE_SERVER>
		// get log_filePath from <CLIENT>
		ret = context.getLocalMachineInfo().getInvokeServerLog();
		if (ret != null) {
			return ret + "." + this.type;
		}
		return null;
	}

	/*
	 * Create command line for Invoke Server
	 * @return command[0] = command name
	 * @return command[1] = "-l" if specified log_filePath 
	 * @return command[2] = logfile if specified log_filePath 
	 */
	private String[] createCommandLine(InvokeServerInfo isInfo, String logfile){
		String [] command;
		if (logfile != null) {
			// launch Invoke Server with log file option 
			command = new String[3];
			command[1] = "-l";
			if (this.maxJobs > 0) {
				command[2] = logfile + "-" + this.ID;
			} else {
				command[2] = logfile;
			}
		} else {
			// launch Invoke Server without log file
			command = new String[1];
		}

		// create command line from Invoke Server option 
		command[0] = null;
		if (isInfo != null) {
			command[0] = isInfo.getPath();
		}
		if (command[0] == null) {
			command[0] = NgUtil.getDefaultPath(DefaultCommandName, this.type);
		}
		return command;
	}

	/**
	 * Request QUERY_FEATURES.
	 * @throws NgException
	 */
	private void queryFeatures() throws NgException {
		try {
			ExtModuleRequest req = ExtModuleRequest.QUERY_FEATURES;
			logDebugInternal("Request is " + req);
			process.request(req);
			logInfoInternal("Send QUERY_FEATURES request to Invoke Server");

			ExtModuleReply rep = process.reply();
			logInfoInternal("Receive QUERY_FEATURES reply from Invoke Server");
			logDebugInternal("Reply is " + rep);
			if (!rep.result().equals("SM")) {
				throw new NgException(req + "error. returned " + rep);
			}

			List<String> reply = rep.returnValues();
			for (String line: reply) {
				String[] words = line.split(" ");
				if (words.length != 2) {
					throw new NgException("Invalid reply: " + line);
				}
				String name = words[0];
				String value = words[1];
				if (name.equals("protocol_version")) {
					double version = Double.parseDouble(value);
					if (version < 2.0) {
						throw new NgException("Invalid version: " + value + ", Require 2.0 or later");
					}
				} else if (name.equals("feature")) {
					if (value.equals("STAGING_AUTH_NUMBER")) {
						this.stagingAuthNumber = true;
					} else if (value.equals("STAGING_COMMUNICATION_PROXY")) {
						this.stagingCommunicationProxy = true;
					} else {
						logWarnInternal("Unknown feature: " + value);
					}
				} else if (name.equals("request")) {
					// Do nothing.
				} else {
					logWarnInternal("Unknown attribute: " + line);
				}
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * Send JOB_CREATE request to Invoke Server process
	 * @throws GrpcException
	 */
	protected void createJob(NgGrpcJob ngJob) throws GrpcException {
		// check if it's valid 
		if (! this.isValid ) {
			throw new NgException(
				"NgInvokeServer#createJob: Invalid InvokeServer.");
		}

		_putRequestedJob(ngJob); // put JOB into map 
		this.jobCount += 1;      // increment jobCount 

		// Build string of JOB_CREATE
		ExtModuleRequest job_create = buildJobCreateRequest(ngJob);

		lockInvokeServer();
		try {
			logInfoInternal("createJob : send JOB_CREATE to Invoke Server.");

			String reply = send(job_create);

			// Receive reply string 
			logInfoInternal("createJob : receive JOB_CREATE reply from Invoke Server.");
			logDebugInternal("createJob : receive JOB_CREATE reply is " 
				+ reply);
			
			// check if the command was failed 
			if ( reply.startsWith(REPLY_FAILED) ) {
				_removeRequestedJob(ngJob.getRequestID());
				throw new NgException("NgInvokeServer#createJob: " + reply);
			}
		} finally {
			unlockInvokeServer();
		}
	}

	private ExtModuleRequest buildJobCreateRequest(final NgGrpcJob ngJob)
	 throws GrpcException {
		ExtModuleRequest req =
			ExtModuleRequest.issuesMultiple(JOB_CREATE,
				String.valueOf(ngJob.getRequestID()));

		//// options in InvokeServerInfo 
		if (this.createOptions != null) {
			for (int i = 0; i < this.createOptions.size(); i++) {
				req.addAttribute(createOptions.get(i).toString());
			}
		}

		RemoteMachineInfo rmi = ngJob.getRemoteMachineInfo();
		//// invoke_server_options 
		List<String> listIsOpt = rmi.getInvokeServerOption();
		for (int i = 0; i < listIsOpt.size(); i++) {
			req.addAttribute(listIsOpt.get(i));
		}
		// create arguments 
		//// client_hostname 
		req.addAttribute( strOfClientHostname(rmi) );
		//// hostname 
		req.addAttribute(ARG_HOSTNAME, rmi.getHostname());
		//// port 
		req.addAttribute( strOfPort(rmi) );
		//// jobmanager 
		if (rmi.getJobmanager() != null) {
			req.addAttribute(ARG_JOBMANAGER, rmi.getJobmanager());
		}
		//// subject 
		if (rmi.getSubject() != null) {
			req.addAttribute(ARG_SUBJECT, rmi.getSubject());
		}

		RemoteClassPathInfo rcpi =
			rmi.getRemoteClassPath(ngJob.getClassName());
		//// executable_path 
		req.addAttribute(ARG_EXECUTABLE_PATH, rcpi.getClasspath());
		//// backend 
		req.addAttribute( strOfBackend(ngJob) );
		//// count 
		req.addAttribute( strOfJobCount(ngJob) );
		//// staging 
		if (rcpi.getStaging() == null) {
			req.addAttribute(ARG_STAGING, "false");
		} else {
			req.addAttribute(ARG_STAGING, rcpi.getStaging());
		}
		//// argument 
		int authNo = ngJob.getSimpleAuthNumber();
		if (stagingAuthNumber) {
			req.addAttribute(ARG_AUTH_NUMBER + " " + authNo);
		} else {
			req.addAttribute(ARG_ARGUMENT, "--authNumber=" + authNo);
		}
		String[] arrayArguments = ngJob.getArgumentList();
		for (int i = 0; i < arrayArguments.length; i++) {
			req.addAttribute(ARG_ARGUMENT, arrayArguments[i]);
		}
		//// work_directory 
		if (rmi.getWorkDir() != null) {
			req.addAttribute(ARG_WORK_DIRECTORY, rmi.getWorkDir());
		}
		//// redirect_enable 
		String redirectEnable = rmi.getRedirectOuterr();
		req.addAttribute(ARG_REDIRECT_ENABLE, redirectEnable);
		//// stdout_file 
		if (redirectEnable.equalsIgnoreCase("true")) {
			//// file for stdout 
			req.addAttribute(ARG_STDOUT_FILE, ngJob.getStdout());
			//// file for stderr 
			req.addAttribute(ARG_STDERR_FILE, ngJob.getStderr());
		}
		//// environment ("environment KEY=VALUE\n\r")
		if (rmi.getEnvironment() != null) {
			Map environment = rmi.getEnvironment();
			for (Object o : environment.entrySet()) {
				Map.Entry ent = (Map.Entry)o;
				req.addAttribute(ARG_ENVIRONMENT,
					ent.getKey() + "=" + ent.getValue());
			}
		}
		//// tmp_dir
		String tmpDir = context.getLocalMachineInfo().getTmpDir();
		if (tmpDir != null) {
			req.addAttribute(ARG_TMP_DIR, tmpDir);
		}
		//// status_polling 
		req.addAttribute(ARG_STATUS_POLLING, String.valueOf(statusPolling));
		//// refresh credential 
		LocalMachineInfo localMachineInfo = context.getLocalMachineInfo();
		if (localMachineInfo.getRefreshCredential() != null) {
			req.addAttribute(ARG_REFRESH_CREDENTIAL,
				localMachineInfo.getRefreshCredential());
		} else {
			req.addAttribute(ARG_REFRESH_CREDENTIAL, "0");
		}
		//// append RSLInfo(for GT)
		addAttributesOfRSLInfo(req, rmi);
		//// work directory (duplicate?)
		String stagingFlag = rcpi.getStaging();
		if ((stagingFlag == null) || (stagingFlag.equals("true") != true)) {
			if (rmi.getWorkDir() != null) {
				req.addAttribute(ARG_WORK_DIRECTORY, rmi.getWorkDir());
			} else {
				String _classPath = rcpi.getClasspath();
				String _parentDir = new File(_classPath).getParent();
				req.addAttribute(ARG_WORK_DIRECTORY,
					_parentDir.replace('\\', '/') );
			}
		}

		// Remote Communication Proxy
		CommunicationProxyInfo cpInfo = rmi.getCommunicationProxyInfo();
		if (cpInfo.getCommunicationProxy() != null) {
			if (cpInfo.getCommunicationProxyStaging()) {
				if (stagingCommunicationProxy == false) {
					final String errmsg = "Invoke server is not supporting staging of Remote Communication Proxy.\n";
					logErrorInternal(errmsg);
					throw new NgException(errmsg);
				}
				req.addAttribute(ARG_COMMUNICATION_PROXY_STAGING, "true");
				String cpPath = cpInfo.getCommunicationProxyPath();
				if (cpPath == null) {
					cpPath = NgUtil.getDefaultPath(CommunicationProxyCommandName, cpInfo.getCommunicationProxy());
				}
				req.addAttribute(ARG_COMMUNICATION_PROXY_PATH, cpPath);
			}
		}

		// JOB_CREATE build end
		logDebugInternal("buildJobCreateRequest: request string is \n"
			+ req.toString() );

		return req;
	}

	private String strOfClientHostname(final RemoteMachineInfo rmi) {
		if (rmi.getClientHostname() != null) {
			return ARG_CLIENT_NAME + " " + rmi.getClientHostname();
		} 
		return ARG_CLIENT_NAME + " " + context.getLocalMachineInfo().getHostname();
	}

	private String strOfPort(final RemoteMachineInfo rmi) {
		if (rmi.getPort() == null) {
			return ARG_PORT + " 0";
		}
		return ARG_PORT + " " + rmi.getPort();
	}

	private String strOfBackend(final NgGrpcJob job) {
		String type = job.getJobType();
		if ( isJobTypeMPI(type) ) {
			return ARG_BACKEND + " " + type.toUpperCase();
		}
		return ARG_BACKEND + " " 
				+ RemoteMachineInfo.VAL_BACKEND_NORMAL.toUpperCase();
	}

	private String strOfJobCount(final NgGrpcJob job) {
		if ( isJobTypeMPI(job.getJobType()) ) {
			RemoteMachineInfo rmi = job.getRemoteMachineInfo();
			return ARG_COUNT + " " + rmi.getNumCPUs(job.getClassName());
		}
		return ARG_COUNT + " " + job.getJobCount();
	}

	private boolean isJobTypeMPI(String type) {
		if (type == null) return false;
		return (type.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			 	type.equals(RemoteMachineInfo.VAL_BACKEND_BLACS));
	}

	private void addAttributesOfRSLInfo(ExtModuleRequest req,
	 final RemoteMachineInfo rmi)
	 throws NgIOException {
		RSLInfo rslInfo = rmi.getRslInfo();
		if ( rslInfo == null ) { return ; }

		//// max_time 
		if (rslInfo.getJobMaxTime() != null) {
			req.addAttribute(ARG_MAX_TIME, rslInfo.getJobMaxTime());
		}
		//// max_wall_time 
		if (rslInfo.getJobMaxWallTime() != null) {
			req.addAttribute(ARG_MAX_WALL_TIME, rslInfo.getJobMaxWallTime());
		}
		//// max_cpu_time 
		if (rslInfo.getJobMaxCpuTime() != null) {
			req.addAttribute(ARG_MAX_CPU_TIME, rslInfo.getJobMaxCpuTime());
		}
		//// queue_name 
		if (rslInfo.getJobQueue() != null) {
			req.addAttribute(ARG_QUEUE_NAME, rslInfo.getJobQueue());
		}
		//// project 
		if (rslInfo.getJobProject() != null) {
			req.addAttribute(ARG_PROJECT, rslInfo.getJobProject());
		}
		//// host_count 
		if (rslInfo.getJobHostCount() != null) {
			req.addAttribute(ARG_HOST_COUNT, rslInfo.getJobHostCount());
		}
		//// min_memory 
		if (rslInfo.getJobMinMemory() != null) {
			req.addAttribute(ARG_MIN_MEMORY, rslInfo.getJobMinMemory());
		}
		//// max_memory 
		if (rslInfo.getJobMaxMemory() != null) {
			req.addAttribute(ARG_MIN_MEMORY, rslInfo.getJobMaxMemory());
		}
		//// RSL extension 
		if (rslInfo.getJobRslExtensions() != null) {
			List rslExtensions = rslInfo.getJobRslExtensions();
			for (int i = 0; i < rslExtensions.size(); i++) {
				StringReader sr =
					new StringReader((String)rslExtensions.get(i));
				BufferedReader br = new BufferedReader(sr);
				String line = null;
				try {
					while ((line = br.readLine()) != null) {
						req.addAttribute(ARG_RSL_EXTENSION, line);
					}
				} catch (IOException e) {
					// invalid GASSServer 
					throw new NgIOException(
						"NgInvokeServer#createJob: invalid rsl extensions.");
				}
			}
		}
		return ;
	}
	
	/**
	 * Returns the status of specified job.
	 * 
	 * @param jobID
	 * @return status
	 * @throws GrpcException
	 */
	public int getStatus(String jobID) throws GrpcException {
		// check if it's valid 
		if (! this.isValid ) {
			throw new NgException("NgInvokeServer#getStatus: Invalid InvokeServer.");
		}

		// create JOB_STATUS request
		ExtModuleRequest jobStatusRequest =
			ExtModuleRequest.issuesSingle(JOB_STATUS, jobID);

		logDebugInternal("getStatus : request string is " 
			+ jobStatusRequest);

		lockInvokeServer();
		try {
			logInfoInternal("getStatus : send JOB_STATUS to Invoke Server.");

			String reply = send(jobStatusRequest);

			// Receive reply string 
			logInfoInternal("getStatus : receive JOB_STATUS reply from Invoke Server.");
			logDebugInternal("getStatus : receive JOB_STATUS reply is " + reply + ".");
			// check if the command was failed 
			if (reply.startsWith(REPLY_FAILED)) {
				throw new NgException("getStatus: " + reply);
			} else {
				// get status string, return it
				return getJobStatus(reply);
			}
		} finally {
			unlockInvokeServer();
		}
	}

	private int getJobStatus(String reply) throws NgException {
		String status = reply.substring(2).trim();
		if (status.equals(STATUS_PENDING)) {
			return JobStatus.PENDING;
		} else if (status.equals(STATUS_ACTIVE)) {
			return JobStatus.ACTIVE;
		} else if (status.equals(STATUS_DONE)) {
			return JobStatus.DONE;
		} else if (status.equals(STATUS_FAILED)) {
			return JobStatus.FAILED;
		} else {
			throw new NgException("NgInvokeServer#getJobStatus: Invalid status [" + status + "].");
		}
	}
	

	private void logInfoInternal(String msg) {
		ngLog.logInfo(NgLog.CAT_NG_INTERNAL,
			context.logHeader() + "NgInvokeServer#" + msg);
	}

	private void logDebugInternal(String msg) {
		ngLog.logDebug(NgLog.CAT_NG_INTERNAL,
			context.logHeader() + "NgInvokeServer#" + msg);
	}

	private void logWarnInternal(String msg) {
		ngLog.logWarn(NgLog.CAT_NG_INTERNAL,
			context.logHeader() + "NgInvokeServer#" + msg);
	}

	private void logErrorInternal(String msg) {
		ngLog.logError(NgLog.CAT_NG_INTERNAL,
			context.logHeader() + "NgInvokeServer#" + msg); 
	}


	/**
	 * Send JOB_DESTROY request to Invoke Server process,
	 * & terminate Invoke Server process if managing jobs exited.
	 *
	 * @param jobID
	 * @throws GrpcException
	 * call at NgGrpcJob
	 */
	public void destroyJob(String jobID) throws GrpcException {
		// create JOB_DESTROY request
		ExtModuleRequest jobDestroyRequest =
			ExtModuleRequest.issuesSingle(JOB_DESTROY, jobID);

		logDebugInternal("destroyJob : request string is " + jobDestroyRequest);

		lockInvokeServer();
		try { 
			logInfoInternal("destroyJob : send JOB_DESTROY to Invoke Server.");

			String reply = send(jobDestroyRequest);

			logInfoInternal("destroyJob : receive JOB_DESTROY reply from Invoke Server."); 
			logDebugInternal("destroyJob : receive JOB_DESTROY reply is " 
				+ reply + ".");

			// remove JOB from map 
			if (createdJobs.containsKey(jobID)) {
				NgGrpcJob _job = createdJobs.remove(jobID);
				destroyedJobs.put(jobID, _job);
				logDebugInternal("destroyJob: move Job[" + jobID + "] to destroyed jobs map");
			}

			// check if the command was failed
			if (reply.startsWith(REPLY_FAILED)) {
				throw new NgException("NgInvokeServer#destroyJob: " + reply);
			}
		} finally {
			destroyedJobCount++;

			// Below must move to somewhere...  (What's the below?)
			// remove NgGrpcJob from Properties
			// createdJobs.remove(jobID);
			unlockInvokeServer();

			// check if it over limit of JOBs 
			if (((!isValid()) && (this.destroyedJobCount >= this.jobCount)) ||
			    ((this.maxJobs != 0) && (this.jobCount >= this.maxJobs)) &&
				requestedJobs.isEmpty() && createdJobs.isEmpty() ) {
				logDebugInternal("destroyJob : It's time to exit Invoke Server.");
				exit();
			}
		}
		logDebugInternal("destroyJob : end ");
	}


	/**
	 * Send EXIT request to Invoke Server process
	 * @throws GrpcException
	 */
	public void exit() throws GrpcException {
		// unregister InvokeServer from Manager 
		invokeServerManager.unregister(this);

		// create request string 
		ExtModuleRequest exitRequest =
			ExtModuleRequest.issuesSingle(EXIT);

		logDebugInternal("exit() : request string is " + exitRequest);

		lockInvokeServer();
		try {
			logInfoInternal("exit : send EXIT to Invoke Server.");

			String reply = send(exitRequest);

			logInfoInternal("exit : receive EXIT reply from Invoke Server.");
			logDebugInternal("exit() : receive EXIT reply is " + reply + ".");

			this.isExited = true;
			// check if the command was failed 
			if ( reply.startsWith(REPLY_FAILED) )
				throw new NgException("NgInvokeServer#exit: " + reply);

			this.process.exit();
		} finally {
			this.isValid = false; // turn off validate flag 
			setDoneToAllJobs();   // set DONE to all JOBs 
			unlockInvokeServer();
		}
	}

	/*
	 * Send the external module request
	 * 
	 * @param request a external module request
	 * @throws GrpcException
	 */
	private String send(ExtModuleRequest request) throws GrpcException {
		ExtModuleReply reply = null;
		try {
			reply = this.process.send(request);
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		if ( isInvalidReply( reply.result() ) ) {
			throw new NgException(
				"NgInvokeServer#receiveReply: Invalid reply.");
		}
		return reply.toString();
	}

	/*
	 * @return true invalid reply
	 * @return false valid reply
	 */
	private boolean isInvalidReply(String rep) {
		if (rep == null)
			return true;
		if ( (! rep.startsWith(REPLY_SUCCESS)) &&
			 (! rep.startsWith(REPLY_FAILED)) )
			return true;
		return false;
	}
	
	/**
	 * Lock InvokeServer.
	 * 
	 * @throws GrpcException if it's interrupted.
	 */
	private synchronized void lockInvokeServer() throws GrpcException {
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
	 * Unlock InvokeServer.
	 * 
	 * @throws GrpcException  if it's interrupted.
	 */
	private synchronized void unlockInvokeServer() throws GrpcException {
		// check if it's locked 
		if (! isLocked ) {
			throw new NgException("Nobody lock the InvokeServer.");
		}
		isLocked = false;
		notifyAll();
	}

	/*
	 * Sets the created job id.
	 *
	 * @param param list of CREATE_NOTIFY's parameter
	 */
	public void requestedJob(List<String> param) {
		if (param == null)
			throw new NullPointerException();
		if (param.size() != 4)
			throw new IllegalArgumentException(param.toString());

		String requestId = param.get(1);
		String result = param.get(2);
		String value = param.get(3);

		if ( ! result.equals(REPLY_SUCCESS) ) {
			logErrorInternal("CREATE_NOTIFY fail: " + param.toString());
			// XXX
			return ;
		}

		// Get target NgJob 
		NgGrpcJob ngJob = requestedJobs.get(Integer.valueOf(requestId));

		// Set ID string into NgJob 
		ngJob.setJobIDString(value);
		logDebugInternal("ngJobCreate : set Job ID [" 
			+ value+ "] to request Job [" + requestId+ "]");

		// put ngJob into map
		_removeRequestedJob(Integer.parseInt(requestId));
		createdJobs.put(value, ngJob);
		logDebugInternal("ngJobCreate: put Job[" + value + "] into map");
	}


	/*
	 * Updates the notified job status.
	 *
	 * @param param list of STATUS_NOTIFY's parameter
	 */
	public void statusUpdate(List<String> param) {
		if (param == null)
			throw new NullPointerException();
		if (param.size() != 3) {
			logErrorInternal("STATUS_NOTIFY's parameter is not enough");
		}

		logInfoInternal("Got STATUS_NOTIFY: " + param.toString());

		// get target NgJob
		String job_id     = param.get(1);
		String job_status = param.get(2);
		String msg = null; // param.get(3);

		NgGrpcJob ngJob = createdJobs.get(job_id);
		// Is ngJob available?
		if ( ngJob == null ) {
			// check JOB_DESTROY requested Job
			ngJob = destroyedJobs.get(job_id);
			if (ngJob == null) {
				logWarnInternal("run : The Job is already not available.");
				return ;
			}
			destroyedJobs.remove(job_id);
		}

		logDebugInternal("analyzeNgJobStatus : set status of JobID ["
			+ job_id + "] to [" + job_status + "]");
		// Set status to NgJob 
		if (job_status.equals(STATUS_PENDING)) {
			// nothing 
		} else if (job_status.equals(STATUS_ACTIVE)) {
			ngJob.setStatus(JobStatus.ACTIVE);
		} else if (job_status.equals(STATUS_DONE)) {
			ngJob.setStatus(JobStatus.DONE);
		} else if (job_status.equals(STATUS_FAILED)) {
			ngJob.setStatus(JobStatus.FAILED);
		}

		// put stdout/stderr
		if ((job_status.equals(STATUS_DONE)) ||
			(job_status.equals(STATUS_FAILED))) {

			putMessage(ngJob.getStdout(), System.out);
			putMessage(ngJob.getStderr(), System.err);
			// delete files for stdout/stderr 
			if (ngJob.getStdout() != null)
				new File(ngJob.getStdout()).delete();
			if (ngJob.getStderr() != null)
				new File(ngJob.getStderr()).delete();
		}
	}

	/*
	 * Writes the file(srcFile) to the specified PrintStream
	 * 
	 * @param file
	 * @param ps 
	 */
	private void putMessage(String srcFile, PrintStream ps) {
		if (srcFile == null) {
			return; // redirect is not required 
		}

		// put output message to specified PrintStream 
		BufferedReader br = null;
		try {
			br = new BufferedReader(
				new InputStreamReader(new FileInputStream(srcFile)));
			String line = null;
			while ((line = br.readLine()) != null)  {
				ps.println(line);
			}
		} catch (FileNotFoundException e) {
			logWarnInternal("putMessage : stdout/stderr file does not exist.");
		} catch (IOException e) {
			logErrorInternal(e.getMessage());
		} finally {
			if (br != null)  {
				try {
					br.close();
				} catch (IOException e) {
					logErrorInternal(e.getMessage());
				}
			}
		}
	}
	
	private void setDoneToAllJobs() {
		// set DONE to requested JOBs 
		for (Iterator<NgGrpcJob> itr = requestedJobs.values().iterator();
			 itr.hasNext();  ) {
			NgGrpcJob targetJob = itr.next();
			targetJob.setStatus(JobStatus.DONE);
		}

		// set DONE to created JOBs
		for (Iterator<NgGrpcJob> itr = createdJobs.values().iterator();
			 itr.hasNext();  ) {
			NgGrpcJob targetJob = itr.next();
			targetJob.setStatus(JobStatus.DONE);
		}
	}
	

	public int getID() {
		return this.ID;
	}
	
	public int getRequestCount() {
		return this.requestCount;
	}

	protected void deactivate() {
		this.isValid = false;
	}

	public boolean isValid() {
		return this.isValid;
	}

	public String getType() {
		return this.type;
	}

	public void incrementRequestCount() {
		this.requestCount += 1;
	}

	private void _putRequestedJob(NgGrpcJob job) {
		this.requestedJobs.put(Integer.valueOf(job.getRequestID()), job);
	}

	private void _removeRequestedJob(int requestedId) {
		this.requestedJobs.remove(Integer.valueOf(requestedId));
	}

	/*
	 * Returns the notify of invoke server
	 */
	public ExtModuleNotify createNotify() throws IOException {
		if ((!this.isValid) || this.isExited) {
			return null;
		}

		LineNumberReader reader = process.getNotifyReader();
		String line = reader.readLine();
		if (line == null) {
			logInfoInternal("reached EOF");
			return null;
		}

		if ( line.startsWith(CREATE_NOTIFY) ) {
			return new CreateNotify(this, line);
		} else if ( line.startsWith(STATUS_NOTIFY) ) {
			return new StatusNotify(this, line);
		} else {
			logErrorInternal("unknown notify received.");
			throw new IOException("unknown notify received.");
		}
	}

	private static class CreateNotify implements ExtModuleNotify {
		private NgInvokeServer invokeServer;
		private List<String> parameter;

		public CreateNotify(NgInvokeServer invokeServer, String line)
		throws IOException {
			this.invokeServer = invokeServer;
			this.parameter = ExtModuleNotifyParser.parse(line);
		}
		
		public void handle() {
			// error handling does not implemented yet
			this.invokeServer.requestedJob(parameter);
		}
	}

	private static class StatusNotify implements ExtModuleNotify {
		private NgInvokeServer invokeServer;
		private List<String> parameter;

		public StatusNotify(NgInvokeServer invokeServer, String line)
		throws IOException {
			this.invokeServer = invokeServer;
			this.parameter = ExtModuleNotifyParser.parse(line);
		}
		public void handle() {
			this.invokeServer.statusUpdate(parameter); 
		}
	}

}

