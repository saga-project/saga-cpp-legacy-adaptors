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
 * $RCSfile: NgInvokeServer.java,v $ $Revision: 1.23 $ $Date: 2006/09/14 08:22:35 $
 */
package org.apgrid.grpc.ng;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.StringReader;
import java.util.Enumeration;
import java.util.List;
import java.util.Properties;

import org.apgrid.grpc.ng.info.InvokeServerInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.globus.io.gass.server.GassServer;
import org.gridforum.gridrpc.GrpcException;

class NgInvokeServer implements Runnable {
	/* instance variables */
	private Properties propRequestedJOBs;
	private Properties propCreatedJOBs;
	private NgInvokeServerManager invokeServerManager;
	private NgGrpcClient context;
	private NgLog ngLog;
	private int ID;
	private int requestCount;
	private int jobCount;
	private int maxJobs;
	private int statusPolling;
	private List createOptions;
	private boolean isLocked;
	private boolean isExited;
	private boolean isValid;
	
	/* communicators to InvokeServer */
	private Process isProcess;
	private OutputStream osRequest;
	private InputStream isReply;
	private InputStream isNotify;
	private BufferedReader brReply;
	private BufferedReader brNotify;
	
	private static final String propNgDir = "NG_DIR";
	private static final String invokeServerSuffix = "ng_invoke_server";
	private static final int BUFFER_SIZE = 1024;
	private static final int POLL_INTERVAL = 200;
	
	/* protocol definitions */
	private static final String INVOKE_SERVER_PROT_DELIMITER = "\r\n";
	
	private static final String REQUEST_JOB_CREATE = "JOB_CREATE";
	private static final String REQUEST_JOB_CREATE_END = "JOB_CREATE_END";
	private static final String REQUEST_JOB_STATUS = "JOB_STATUS";
	private static final String REQUEST_JOB_DESTROY = "JOB_DESTROY";
	private static final String REQUEST_EXIT = "EXIT";
	
	private static final String REPLY_SUCCESS = "S";
	private static final String REPLY_FAILED = "F";

	private static final String STATUS_PENDING = "PENDING";
	private static final String STATUS_ACTIVE = "ACTIVE";
	private static final String STATUS_DONE = "DONE";
	private static final String STATUS_FAILED = "FAILED";
	
	private static final String NOTIFY_CREATE = "CREATE_NOTIFY";
	private static final String NOTIFY_STATUS = "STATUS_NOTIFY";
	
	/* arguments for CREATE_JOB */
	private static final String ARG_HOSTNAME = "hostname";
	private static final String ARG_PORT = "port";
	private static final String ARG_JOBMANAGER = "jobmanager";
	private static final String ARG_SUBJECT = "subject";
	private static final String ARG_EXECUTABLE_PATH = "executable_path";
	private static final String ARG_BACKEND = "backend";
	private static final String ARG_COUNT = "count";
	private static final String ARG_STAGING = "staging";
	private static final String ARG_ARGUMENT = "argument";
	private static final String ARG_WORK_DIRECTORY = "work_directory";
	private static final String ARG_GASS_URL = "gass_url";
	private static final String ARG_REDIRECT_ENABLE = "redirect_enable";
	private static final String ARG_STDOUT_FILE = "stdout_file";
	private static final String ARG_STDERR_FILE = "stderr_file";
	private static final String ARG_ENVIRONMENT = "environment";
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
	private static final String ARG_CLIENT_NAME = "client_name";
	private static final String ARG_RSL_EXTENSION = "rsl_extensions";
	private static final String ARG_TMP_DIR = "tmp_dir";
	
	
	/**
	 * @param isMng
	 * @param context
	 * @param isType
	 * @param ID
	 * @param isInfo
	 * @throws GrpcException
	 */
	NgInvokeServer(NgInvokeServerManager isMng, NgGrpcClient context,
		String isType, int ID, InvokeServerInfo isInfo) throws GrpcException {
		/* create Map for JOBs */
		this.propRequestedJOBs = new Properties();
		this.propCreatedJOBs = new Properties();
		
		/* set InvokeServerManager and NgLog */
		this.context = context;
		this.invokeServerManager = isMng;
		this.ngLog = context.getNgLog();
		this.ID = ID;
		if (isInfo != null) {
			/* get information from InvokeServerInfo */
			this.maxJobs = isInfo.getMaxJobs();
			this.createOptions = isInfo.getOptions();
			this.statusPolling = isInfo.getStatusPolling();
		} else {
			/* set default value */
			this.maxJobs = 0;
			this.createOptions = null;
			this.statusPolling = 0;
		}
		
		/* reset log flag */
		this.isLocked = false;
		/* reset exit flag */
		this.isExited = false;
		/* reset requestCount, jobCount */
		this.requestCount = 0;
		this.jobCount = 0;
		
		/* launch InvokeServer */
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				context,
				"NgInvokeServer : run Invoke Server process.");

		Runtime runtime = Runtime.getRuntime();
		String ng_dir = System.getProperty(propNgDir);
		try {
			/* get logfile for InvokeServer */
			String isLogFile = null;
			if (isInfo != null) {
				/* get log filename from INVOKE_SERVER */ 
				isLogFile = isInfo.getLogFilePath();
			}
			if (isLogFile == null) {
				/* get log filename from CLIENT */
				isLogFile = 
					(String) context.getNgInformationManager().getLocalMachineInfo().get(NgInformationManager.KEY_CLIENT_INVOKE_SERVER_LOG);
				if (isLogFile != null) {
					isLogFile = isLogFile + "." + isType;
				}
			}
	
			/* create command line for InvokeServer */
			String[] isCommand;
			int numOfCommand = 1;
			int numOfOptions = 2;
			if (isLogFile != null) {
				/* launch Invoke Server with log file option */
				isCommand = new String[numOfCommand + numOfOptions];
				isCommand[numOfCommand] = "-l";
				if (this.maxJobs > 0) {
					isCommand[numOfCommand + 1] = isLogFile + "-" + this.ID;
				} else {
					isCommand[numOfCommand + 1] = isLogFile;
				}
			} else {
				/* launch Invoke Server without log file */
				isCommand = new String[numOfCommand];
			}
			
			/* create command line for Invoke Server option */
			isCommand[0] = null;
			if (isInfo != null) {
				isCommand[0] = isInfo.getPath();
			}
			if (isCommand[0] == null) {
				isCommand[0] = ng_dir + "/bin/" + invokeServerSuffix + "." + isType;
				
				/* If target is Windows, then append ".bat" */
				String targetOS = System.getProperty("os.name");
				if (targetOS.indexOf("Windows") != -1) {
					isCommand[0] = isCommand[0].concat(".bat");
				}
			}
			
			/* launch InvokeServer */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_DEBUG,
					context,
					"NgInvokeServer : Invoke Server command is " + isCommand[0] + ".");
			this.isProcess =	runtime.exec(isCommand);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		/* set Input/OutputStream */
		this.osRequest = this.isProcess.getOutputStream();
		this.isReply = this.isProcess.getInputStream();
		this.isNotify = this.isProcess.getErrorStream();
		this.brReply = new BufferedReader(new InputStreamReader(isReply));
		this.brNotify = new BufferedReader(new InputStreamReader(isNotify));
		
		/* turn on validate flag */
		this.isValid = true;
	}
	
	/**
	 * @throws GrpcException
	 * 
	 */
	protected void createJob(NgGrpcJob ngJob) throws GrpcException {
		/* check if it's valid */
		if (this.isValid == false) {
			throw new NgException("NgInvokeServer#createJob: Invalid InvokeServer.");
		}
		
		/* put JOB into map */
		propRequestedJOBs.put(new Integer(ngJob.getRequestID()), ngJob);
		/* increment jobCount */
		this.jobCount += 1;
		
		/* create request string */
		StringBuffer sb = new StringBuffer();
		sb.append(REQUEST_JOB_CREATE + " " + ngJob.getRequestID());
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* options in InvokeServerInfo */
		if (this.createOptions != null) {
			for (int i = 0; i < this.createOptions.size(); i++) {
				sb.append(createOptions.get(i));
				sb.append(INVOKE_SERVER_PROT_DELIMITER);
			}
		}
		
		/* invoke_server_options */
		RemoteMachineInfo remoteMachineInfo = ngJob.getRemoteMachineInfo();
		List listIsOpt =
			(List) remoteMachineInfo.get(RemoteMachineInfo.KEY_INVOKE_SERVER_OPTION);
		if (listIsOpt != null) {
			for (int i = 0; i < listIsOpt.size(); i++) {
				sb.append(listIsOpt.get(i));
				sb.append(INVOKE_SERVER_PROT_DELIMITER);
			}
		}
		
		/* create arguments */
		/* client_hostname */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CLIENT_HOSTNAME) != null) {
			sb.append(ARG_CLIENT_NAME + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_CLIENT_HOSTNAME));
		} else {
			sb.append(ARG_CLIENT_NAME + " " +
					context.getNgInformationManager().getLocalMachineInfo().get(NgInformationManager.KEY_CLIENT_HOSTNAME));
		}
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* hostname */
		sb.append(ARG_HOSTNAME + " " +
			remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME));
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* port */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_PORT) == null) {
			sb.append(ARG_PORT + " 0");
		} else {
			sb.append(ARG_PORT + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_PORT));
		}
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* jobmanager */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_JOBMANAGER) != null) {
			sb.append(ARG_JOBMANAGER + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_JOBMANAGER));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* subject */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_SUBJECT) != null) {
			sb.append(ARG_SUBJECT + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_SUBJECT));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* executable_path */
		RemoteClassPathInfo remoteClassPathInfo =
			remoteMachineInfo.getRemoteClassPath(ngJob.getClassName());
		sb.append(ARG_EXECUTABLE_PATH + " " +
			remoteClassPathInfo.get(RemoteClassPathInfo.KEY_CLASS_PATH_CLASSPATH));
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* backend */
		String jobType = ngJob.getJobType();
		int jobCount = ngJob.getJobCount();
		if ((jobType != null) &&
			(jobType.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			 jobType.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {
			sb.append(ARG_BACKEND + " " + ngJob.getJobType().toUpperCase());
			jobCount = NgGrpcJob.INVALID_JOB_ID;
		} else {
			sb.append(ARG_BACKEND + " " + RemoteMachineInfo.VAL_BACKEND_NORMAL.toUpperCase());
		}
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
			
		/* count */
		if (jobCount == NgGrpcJob.INVALID_JOB_ID) {
			sb.append(ARG_COUNT + " " +
				remoteMachineInfo.getNumCPUs(ngJob.getClassName()));
		} else {
			sb.append(ARG_COUNT + " " + jobCount);
		}
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
			
		/* staging */
		if (remoteClassPathInfo.get(RemoteClassPathInfo.KEY_CLASS_PATH_STAGING) == null) {
			sb.append(ARG_STAGING + " false");
		} else {
			sb.append(ARG_STAGING + " " +
					remoteClassPathInfo.get(RemoteClassPathInfo.KEY_CLASS_PATH_STAGING));
		}
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* argument */
		String[] arrayArguments = ngJob.getArgumentList();
		for (int i = 0; i < arrayArguments.length; i++) {
			sb.append(ARG_ARGUMENT + " " + arrayArguments[i]);
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* work_directory */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_WORK_DIR) != null) {
			sb.append(ARG_WORK_DIRECTORY + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_WORK_DIR));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* gass_url */
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
			sb.append(ARG_GASS_URL + " " + gassServer.getURL());
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* redirect_enable */
		String redirectEnable =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_REDIRECT_OUTERR);
		sb.append(ARG_REDIRECT_ENABLE + " " + redirectEnable);
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* stdout_file */
		if (redirectEnable.equalsIgnoreCase("true")) {
			/* file for stdout */
			sb.append(ARG_STDOUT_FILE + " " + ngJob.getStdout());
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
			
			/* file for stderr */
			sb.append(ARG_STDERR_FILE + " " + ngJob.getStderr());
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* environment */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_ENVIRONMENT) != null) {
			List listEnvironment =
				(List) remoteMachineInfo.get(RemoteMachineInfo.KEY_ENVIRONMENT);
			for (int i = 0; i < listEnvironment.size(); i++) {
				sb.append(ARG_ENVIRONMENT + " " + listEnvironment.get(i));
				sb.append(INVOKE_SERVER_PROT_DELIMITER);
			}
		}
		
		String tmpDir = (String)context.getNgInformationManager().getLocalMachineInfo().get(NgInformationManager.KEY_CLIENT_TMP_DIR);
		if (tmpDir != null) {
			sb.append(ARG_TMP_DIR + " " + tmpDir);
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* status_polling */
		sb.append(ARG_STATUS_POLLING + " " + statusPolling);
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		
		/* refresh credential */
		Properties propLocalMachineInfo =
			context.getNgInformationManager().getLocalMachineInfo();
		if (propLocalMachineInfo.get(NgInformationManager.KEY_CLIENT_REFRESH_CREDENTIAL) != null) {
			sb.append(ARG_REFRESH_CREDENTIAL + " " +
					Integer.parseInt((String) propLocalMachineInfo.get(
					NgInformationManager.KEY_CLIENT_REFRESH_CREDENTIAL)));
		} else {
			sb.append(ARG_REFRESH_CREDENTIAL + " 0");
		}
		sb.append(INVOKE_SERVER_PROT_DELIMITER);

		/* max_time */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXTIME) != null) {
			sb.append(ARG_MAX_TIME + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXTIME));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* max_wall_time */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXWALLTIME) != null) {
			sb.append(ARG_MAX_WALL_TIME + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXWALLTIME));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* max_cpu_time */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXCPUTIME) != null) {
			sb.append(ARG_MAX_CPU_TIME + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXCPUTIME));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* queue_name */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_QUEUE) != null) {
			sb.append(ARG_QUEUE_NAME + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_QUEUE));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* project */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_PROJECT) != null) {
			sb.append(ARG_PROJECT + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_PROJECT));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* host_count */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTCOUNT) != null) {
			sb.append(ARG_HOST_COUNT + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTCOUNT));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* min_memory */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MINMEMORY) != null) {
			sb.append(ARG_MIN_MEMORY + " " +
					remoteMachineInfo.get(RemoteMachineInfo.KEY_MINMEMORY));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}

		/* max_memory */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXMEMORY) != null) {
			sb.append(ARG_MIN_MEMORY + " " +
				remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXMEMORY));
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* work directory */
		String stagingFlag = (String) remoteClassPathInfo.get(
				RemoteClassPathInfo.KEY_CLASS_PATH_STAGING);
		if ((stagingFlag == null) || (stagingFlag.equals("true") != true)) {
			sb.append(ARG_WORK_DIRECTORY+ " ");
			if (remoteMachineInfo.get(RemoteMachineInfo.KEY_WORK_DIR) != null) {
				sb.append(remoteMachineInfo.get(RemoteMachineInfo.KEY_WORK_DIR));
			} else {
				String pathToClass = (String) remoteClassPathInfo.get(
					RemoteClassPathInfo.KEY_CLASS_PATH_CLASSPATH);
				String parentDirOfClass = new File(pathToClass).getParent();
				sb.append(parentDirOfClass.replace('\\', '/'));
			}
			sb.append(INVOKE_SERVER_PROT_DELIMITER);
		}
		
		/* RSL extension */
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_JOB_RSL_EXTENSION) != null) {
			List rslExtensions =
				(List)remoteMachineInfo.get(RemoteMachineInfo.KEY_JOB_RSL_EXTENSION);
			for (int i = 0; i < rslExtensions.size(); i++) {
				StringReader sr = new StringReader((String) rslExtensions.get(i));
				BufferedReader br = new BufferedReader(sr);
				String line = null;
				try {
					while ((line = br.readLine()) != null) {
						sb.append(ARG_RSL_EXTENSION + " " + line);
						sb.append(INVOKE_SERVER_PROT_DELIMITER);
					}
				} catch (IOException e) {
					/* invalid GASSServer */
					throw new NgIOException(
						"NgInvokeServer#createJob: invalid rsl extensions.");
				}
			}
		}
		
		/* the end of JOB_CREATE */
		sb.append(REQUEST_JOB_CREATE_END);
		sb.append(INVOKE_SERVER_PROT_DELIMITER);
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServer#createJob : request string is " + sb.toString() + ".");

		try {
			/* lock Invoke Server */
			lockInvokeServer();
			
			/* Send request string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#createJob : send JOB_CREATE to Invoke Server.");
			sendRequestString(sb.toString());
			
			/* Receive reply string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#createJob : receive JOB_CREATE reply from Invoke Server.");
			String reply = receiveReplyString();
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_DEBUG,
					context,
					"NgInvokeServer#createJob : receive JOB_CREATE reply is " + reply + ".");
			
			/* check if the command was failed */
			if (reply.startsWith(REPLY_FAILED)) {
				propRequestedJOBs.remove(new Integer(ngJob.getRequestID()));
				throw new NgException("NgInvokeServer#createJob: " + reply);
			}
		} finally {
			/* unlock Invoke Server */
			unlockInvokeServer();
		}
	}
	
	/**
	 * @param jobID
	 * @return
	 * @throws GrpcException
	 */
	protected int getStatus(String jobID) throws GrpcException {
		/* check if it's valid */
		if (this.isValid == false) {
			throw new NgException("NgInvokeServer#getStatus: Invalid InvokeServer.");
		}
		
		/* create request string */
		StringBuffer sb = new StringBuffer();
		sb.append(REQUEST_JOB_STATUS + " " + jobID);
		sb.append(INVOKE_SERVER_PROT_DELIMITER);		
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServer#getStatus : request string is " + sb.toString() + ".");
		
		try {
			/* lock Invoke Server */
			lockInvokeServer();
			
			/* Send request string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#getStatus : send JOB_STATUS to Invoke Server.");
			sendRequestString(sb.toString());
			
			/* Receive reply string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#getStatus : receive JOB_STATUS reply from Invoke Server.");
			String reply = receiveReplyString();
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_DEBUG,
					context,
					"NgInvokeServer#createJob : receive JOB_STATUS reply is " + reply + ".");
			
			/* check if the command was failed */
			if (reply.startsWith(REPLY_FAILED)) {
				throw new NgException("NgInvokeServer#getStatus: " + reply);
			} else {
				/* get status string, return it */
				String status = reply.substring(2).trim();
				if (status.equals(STATUS_PENDING)) {
					return NgGrpcJob.NGJOB_STATE_PENDING;
				} else if (status.equals(STATUS_ACTIVE)) {
					return NgGrpcJob.NGJOB_STATE_ACTIVE;
				} else if (status.equals(STATUS_DONE)) {
					return NgGrpcJob.NGJOB_STATE_DONE;
				} else if (status.equals(STATUS_FAILED)) {
					return NgGrpcJob.NGJOB_STATE_FAILED;
				} else {
					throw new NgException("NgInvokeServer#getStatus: Invalid status [" + status + "].");
				}
			}
		} finally {
			/* unlock Invoke Server */
			unlockInvokeServer();
		}
	}
	
	/**
	 * @param jobID
	 * @throws GrpcException
	 */
	protected void destroyJob(String jobID) throws GrpcException {
		/* check if it's valid */
		if (this.isValid == false) {
			throw new NgException("NgInvokeServer#destroyJob: Invalid InvokeServer.");
		}
		
		/* create request string */
		StringBuffer sb = new StringBuffer();
		sb.append(REQUEST_JOB_DESTROY + " " + jobID);
		sb.append(INVOKE_SERVER_PROT_DELIMITER);		
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServer#destroyJob : request string is " + sb.toString() + ".");
		
		try {
			/* lock Invoke Server */
			lockInvokeServer();
			
			/* Send request string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#destroyJob : send JOB_DESTROY to Invoke Server.");
			sendRequestString(sb.toString());
			
			/* Receive reply string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#destroyJob : receive JOB_DESTROY reply from Invoke Server.");
			String reply = receiveReplyString();
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_DEBUG,
					context,
					"NgInvokeServer#destroyJob : receive JOB_DESTROY reply is " + reply + ".");
			
			/* remove JOB from map */
			if (propCreatedJOBs.containsKey(jobID)) {
				propCreatedJOBs.remove(jobID);
			}
			
			/* check if the command was failed */
			if (reply.startsWith(REPLY_FAILED)) {
				throw new NgException("NgInvokeServer#destroyJob: " + reply);
			}
			
if (false) {
			/* put stdout/stderr */
			NgGrpcJob ngJob = (NgGrpcJob) propCreatedJOBs.get(jobID);
			putMessage(ngJob.getStdout(), System.out);
			putMessage(ngJob.getStderr(), System.err);
			/* delete files for stdout/stderr */
			new File(ngJob.getStdout()).delete();
			new File(ngJob.getStderr()).delete();
}
		} finally {
			/* Below must move to somewhere... */
			/* remove NgGrpcJob from Properties */
			/* propCreatedJOBs.remove(jobID); */
			/* unlock Invoke Server */
			unlockInvokeServer();
			
			/* check if it over limit of JOBs */
			Enumeration keys1 = propRequestedJOBs.keys();
			Enumeration keys2 = propCreatedJOBs.keys();
			if ((this.maxJobs != 0) && (this.jobCount >= this.maxJobs) &&
				(! keys1.hasMoreElements()) && (! keys2.hasMoreElements())) {
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_DEBUG,
						context,
						"NgInvokeServer#destroyJob : It's time to exit Invoke Server.");
				exit();
			}
		}
	}
	
	/**
	 * @throws GrpcException
	 * 
	 */
	protected void exit() throws GrpcException {
		/* check if it's valid */
		if (this.isValid == false) {
			throw new NgException("NgInvokeServer#exit: Invalid InvokeServer.");
		}
		
		/* unregister InvokeServer from Manager */
		invokeServerManager.unregisterInvokeServer(this);
		
		/* create request string */
		StringBuffer sb = new StringBuffer();
		sb.append(REQUEST_EXIT);
		sb.append(INVOKE_SERVER_PROT_DELIMITER);		
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServer#exit : request string is " + sb.toString() + ".");
		
		try {
			/* lock Invoke Server */
			lockInvokeServer();
			
			/* Send request string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#exit : send EXIT to Invoke Server.");
			sendRequestString(sb.toString());
			
			/* Receive reply string */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgInvokeServer#exit : receive EXIT reply from Invoke Server.");
			String reply = receiveReplyString();
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_DEBUG,
					context,
					"NgInvokeServer#exit : receive EXIT reply is " + reply + ".");
			
			/* set exit flag */
			this.isExited = true;
			
			/* check if the command was failed */
			if (reply.startsWith(REPLY_FAILED)) {
				throw new NgException("NgInvokeServer#exit: " + reply);
			}
			
			/* close Input/OutputStream */
			try {
				this.osRequest.close();
				this.brReply.close();
				this.brNotify.close();
			} catch (IOException e) {
				throw new NgIOException(e);
			}
		} finally {
			/* turn off validate flag */
			this.isValid = false;
			/* set DONE to all JOBs */
			setDoneToAllJobs();
			/* unlock Invoke Server */
			unlockInvokeServer();
		}
	}
	
	/**
	 * @param requestString
	 * @throws GrpcException
	 */
	private void sendRequestString(String requestString) throws GrpcException {
		try {
			/* send Request String to InvokeServer */
			osRequest.write(requestString.getBytes());
			osRequest.flush();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	private String receiveReplyString() throws GrpcException {
		/* receive Reply String from InvokeServer */
		String replyString = null;
		try {
			replyString = this.brReply.readLine();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		/* check reply string */
		if ((replyString == null) ||
			((replyString.startsWith(REPLY_SUCCESS) == false) &&
			(replyString.startsWith(REPLY_FAILED) == false))) {
			/* Invalid reply string */
			throw new NgException("NgInvokeServer#receiveReplyString: Invalid reply.");
		}
		
		/* return received String */
		return replyString;
	}
	
	/**
	 * Lock InvokeServer.
	 * 
	 * @throws GrpcException if it's interrupted.
	 */
	private synchronized void lockInvokeServer() throws GrpcException {
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
	 * Unlock InvokeServer.
	 * 
	 * @throws GrpcException  if it's interrupted.
	 */
	private synchronized void unlockInvokeServer() throws GrpcException {
		/* check if it's locked */
		if (isLocked == false) {
			throw new NgException("Nobody lock the InvokeServer.");
		}
		/* unlock */
		isLocked = false;
		/* notifyAll */
		notifyAll();
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		/* receive notify messages */
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				context,
				"NgInvokeServer#run : start thread for receiving NOTIFY.");
		
		try {
			String receivedLine;
			
			while (true) {
				/* check if the Invoke Server is alive */
				try {
					int exitState = this.isProcess.exitValue();
					
					/* Invoke Server is dead, so exit loop */
					break;
				} catch (IllegalThreadStateException e) {
					/* Invoke Server is alive */
				}
				
				/* check if it's able to read */
				if (! brNotify.ready()) {
					try {
						/* it's not able to read, so wait for a while */
						Thread.sleep(POLL_INTERVAL);
						continue;
					} catch (InterruptedException e1) {
						throw new NgException(
							"NgInvokeServer#run : something wrong was happened.");
					}
				}
				
				/* read NOTIFY */
				receivedLine = brNotify.readLine();
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_INFO,
						context,
						"NgInvokeServer#run : received NOTIFY.");
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_DEBUG,
						context,
						"NgInvokeServer#run : received message is " + receivedLine + ".");

				/* CREATE_NOTIFY */
				if (receivedLine.startsWith(NOTIFY_CREATE)) {
					String[] createNotify = receivedLine.split(" ");
					if (createNotify.length != 4) {
						/* invalid notify message */
						ngLog.printLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_ERROR,
							this.context,
							"NgInvokeServer#run(): received invalid notify message : " +
							receivedLine + ".");
						break;
					} else if (createNotify[2].startsWith(REPLY_FAILED)) {
						/* received error notify */
						ngLog.printLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_ERROR,
							this.context,
							"NgInvokeServer#run(): received error notify message : " +
							receivedLine + ".");
						break;
					}
					
					/* get target NgJob */
					int jobID = Integer.parseInt(createNotify[1]);
					NgGrpcJob ngJob =
						(NgGrpcJob) propRequestedJOBs.get(new Integer(jobID));
					
					/* set ID string into NgJob */
					ngJob.setJobIDString(createNotify[3]);
					ngLog.printLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_DEBUG,
							context,
							"NgInvokeServer#run : set ID [" + createNotify[3] +
							"] to JobID [" + createNotify[1] + "].");
					
					/* put ngJob into prop */
					propRequestedJOBs.remove(new Integer(createNotify[1]));
					propCreatedJOBs.put(createNotify[3], ngJob);
				} else if (receivedLine.startsWith(NOTIFY_STATUS)) {
					String[] statusNotify = receivedLine.split(" ");
					if (statusNotify.length != 3) {
						/* invalid notify message */
						break;
					}
					
					/* get target NgJob */
					NgGrpcJob ngJob = (NgGrpcJob) propCreatedJOBs.get(statusNotify[1]);
					ngLog.printLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_DEBUG,
							context,
							"NgInvokeServer#run : set status of JobID [" +
							statusNotify[1] + "] to [" + statusNotify[2] + "].");

					/* Is ngJob available? wait DONE? */
					if (ngJob == null) {
						ngLog.printLog(
								NgLog.LOGCATEGORY_NINFG_INTERNAL,
								NgLog.LOGLEVEL_WARN,
								context,
								"NgInvokeServer#run : The Job is already not available.");
						continue;
					}
					
					/* set status to NgJob */
					if (statusNotify[2].equals(STATUS_PENDING)) {
						/* nothing */
					} else if (statusNotify[2].equals(STATUS_ACTIVE)) {
						/* set JOB ACTIVE */
						ngJob.setStatus(NgGrpcJob.NGJOB_STATE_ACTIVE);
					} else if (statusNotify[2].equals(STATUS_DONE)) {
						/* set JOB DONE */
						ngJob.setStatus(NgGrpcJob.NGJOB_STATE_DONE);
					} else if (statusNotify[2].equals(STATUS_FAILED)) {
						/* set JOB FAILED */
						ngJob.setStatus(NgGrpcJob.NGJOB_STATE_FAILED);
					}

					/* put stdout/stderr */
					if ((statusNotify[2].equals(STATUS_DONE)) || (statusNotify[2].equals(STATUS_FAILED))) {
						putMessage(ngJob.getStdout(), System.out);
						putMessage(ngJob.getStderr(), System.err);
						/* delete files for stdout/stderr */
						if (ngJob.getStdout() != null) {
							new File(ngJob.getStdout()).delete();
						}
						if (ngJob.getStderr() != null) {
							new File(ngJob.getStderr()).delete();
						}
					}
				} else {
					/* Invalid notify was found!!! */
					ngLog.printLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_WARN,
							context,
							"NgInvokeServer#run : received unrecognized notify.");
					ngLog.printLog(
							NgLog.LOGCATEGORY_NINFG_INTERNAL,
							NgLog.LOGLEVEL_WARN,
							context,
							receivedLine);
				}
			}
		} catch (IOException e) {
			if (this.isExited == true) {
				/* expected Exception, nothing will be done */
			} else if (ngLog != null) {
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_ERROR,
					new NgIOException(e));
			}
		} catch (GrpcException e) {
			if (ngLog != null) {
				ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_ERROR,
					e);
			}
		} finally {
			/* turn off validate flag */
			this.isValid = false;
			/* set DONE to rest of JOBs */
			setDoneToAllJobs();
		}
		
		/* the end */
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				context,
				"NgInvokeServer#run : stop thread for receiving NOTIFY.");
		
	}

	/**
	 * @param targetFile
	 * @param ps
	 * @throws GrpcException
	 */
	private void putMessage(String targetFile, PrintStream ps) throws GrpcException {
		if (targetFile == null) {
			/* redirect is not required */
			return;
		}
		if (new File(targetFile).exists() != true) {
			/* target file does not exist */
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_WARN,
					context,
					"NgInvokeServer#putMessage : stdout/stderr file does not exist.");
			return;
		}
		
		/* put output message to specified PrintStream */
		try {
			BufferedReader br =
				new BufferedReader(new InputStreamReader(new FileInputStream(targetFile)));
			while (br.ready()) {
				String receivedLine = br.readLine();
				ps.println(receivedLine);
			}
			br.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * 
	 */
	private void setDoneToAllJobs() {
		/* set DONE to requested JOBs */
		Enumeration keys = propRequestedJOBs.keys();
		while (keys.hasMoreElements()) {
			Integer key = (Integer) keys.nextElement();
			NgGrpcJob targetJob = (NgGrpcJob) propRequestedJOBs.get(key);
			targetJob.setStatus(NgGrpcJob.NGJOB_STATE_DONE);
		}
		
		/* set DONE to created JOBs */
		keys = propCreatedJOBs.keys();
		while (keys.hasMoreElements()) {
			String key = (String) keys.nextElement();
			NgGrpcJob targetJob = (NgGrpcJob) propCreatedJOBs.get(key);
			targetJob.setStatus(NgGrpcJob.NGJOB_STATE_DONE);
		}
	}
	
	/**
	 * @return
	 */
	protected int getID() {
		return this.ID;
	}
	
	/**
	 * @return
	 */
	protected int getRequestCount() {
		return this.requestCount;
	}
	
	/**
	 * 
	 */
	protected void incrementRequestCount() {
		this.requestCount += 1;
	}

	/**
	 * 
	 */
	protected boolean isValid() {
		return this.isValid;
	}
}
