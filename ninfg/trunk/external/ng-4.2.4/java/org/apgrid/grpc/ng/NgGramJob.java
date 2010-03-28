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
 * $RCSfile: NgGramJob.java,v $ $Revision: 1.6 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng;

import java.io.File;
import java.util.List;
import java.util.Properties;
import java.util.StringTokenizer;

import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.globus.gram.GramException;
import org.globus.gram.GramJob;
import org.globus.gram.GramJobListener;
import org.globus.gram.internal.GRAMConstants;
import org.globus.io.gass.server.GassServer;
import org.globus.io.gass.server.JobOutputStream;
import org.gridforum.gridrpc.GrpcException;
import org.ietf.jgss.GSSException;

class NgGramJob  implements GramJobListener {
	/* definitions */
	private static final String STDOUT_URI = "out-rgs";
	private static final String STDERR_URI = "err-rgs";
	
	/* instance variables */
	private NgGrpcClient context;
	private RemoteMachineInfo remoteMachineInfo;
	private String className;
	private int jobID;
	private int jobCount;
	private boolean redirect_outerr = false;
	private GassServer gassServer = null;
	private String jobType;
	private NgLog ngLog;
	
	/* OutputListener for JOB */
	protected NgGrpcJobOutputListener stderrListener;
	protected NgGrpcJobOutputListener stdoutListener;
	/* GRAM JOB */
	protected GramJob job;
	/* NgGrpcJob */
	private NgGrpcJob ngJob;

	/**
	 * @param context
	 * @param remoteMachineInfo
	 * @param className
	 * @param JobID
	 * @param jobType
	 * @param jobCount
	 * @throws GrpcException
	 */
	public NgGramJob(NgGrpcClient context, RemoteMachineInfo remoteMachineInfo,
			NgGrpcJob ngJob) throws GrpcException {
			this.context = context;
			this.remoteMachineInfo = remoteMachineInfo;
			this.className = ngJob.getClassName();
			this.jobID = ngJob.getRequestID();
			this.jobCount = ngJob.getJobCount();
			this.ngJob = ngJob;
			this.ngLog = context.getNgLog();
			
			/* check if remoteMachineInfo contains classPathInfo */
			if (remoteMachineInfo.getRemoteClassPath(className) == null) {
				throw new NgInitializeGrpcHandleException(
					"RemoteClassPathInfo does not exist."); 
			}
			
			/* check if redirection output streams was activated */
			if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_REDIRECT_OUTERR).equals("true")) {
				this.redirect_outerr = true;
			}
			
			/* get GassServer */
			if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_GASS_SCHEME).equals("http")) {
				this.gassServer = context.getGassServerNoSecure();
			} else if (remoteMachineInfo.get(
				RemoteMachineInfo.KEY_GASS_SCHEME).equals("https")) {
				this.gassServer = context.getGassServerSecure();
			} else {
				throw new NgInitializeGrpcHandleException("Unsupported GASS scheme");
			}
			
			/* set jobType */
			this.jobType = ngJob.getJobType();
		}
		
	/**
	 * @throws GrpcException
	 */
	protected void invokeExecutable() throws GrpcException {
		/* set OutputListener */
		setOutputListener();
		/* make RSL */
		String rsl = makeRSL();

		/* make GramJob */
		job = new GramJob(rsl);
		job.addListener((GramJobListener) this);
		
		String requestString = makeRequestString();
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_DEBUG,
			context,
			"NgGramJob#invokeExecutable : server= " + requestString);
		ngLog.printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_DEBUG, 
			context,
			"NgGramJob#invokeExecutable : RSL= " + rsl);
		try {
			ngLog.printLog(
					NgLog.LOGCATEGORY_NINFG_INTERNAL,
					NgLog.LOGLEVEL_INFO,
					context,
					"NgGramJob#invokeExecutable : call GramJob#request.");
			job.request(requestString);
		} catch (GramException e) {
			this.ngJob.setStatus(NgGrpcJob.NGJOB_STATE_FAILED);
			throw new NgInitializeGrpcHandleException(e);
		} catch (GSSException e) {
			this.ngJob.setStatus(NgGrpcJob.NGJOB_STATE_FAILED);
			throw new NgInitializeGrpcHandleException(e);
		}
	}
	
	/**
	 * @return
	 */
	private String makeRequestString() throws GrpcException {
		/* make contact string */
		String hostName =
			(String)remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME);
		StringBuffer sb = new StringBuffer(hostName);

		/* append port number */
		String port =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_PORT);
		if (port != null) {
			sb.append(":" + port);
		}
		
		/* append jobmanager */
		String jobmanager =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_JOBMANAGER);
		if (jobmanager != null) {
			sb.append("/" + jobmanager);
		}
		
		/* append subject */
		String subject =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_SUBJECT);
		if (subject != null) {
			if ((port == null) && (jobmanager == null)) {
				sb.append(":");
			}
			sb.append(":" + subject);
		}
		
		/* return contact string */
		return sb.toString();
	}

	/**
	 * @throws GrpcException
	 */
	private void setOutputListener() throws GrpcException {
		/* check if redirection output streams was activated */
		if (redirect_outerr == false) {
			/* do nothing */
			return;
		}
		
		/* create OutputListener */
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG, 
				context,
				"NgGramJob#setOutputListener : set OutputListener for the JOB.");
		stderrListener = new NgGrpcJobOutputListener();
		stdoutListener = new NgGrpcJobOutputListener();
		
		/* set output stream to GASS server */
		gassServer.registerJobOutputStream(
			STDERR_URI,	new JobOutputStream(stderrListener));
		gassServer.registerJobOutputStream(
			STDOUT_URI,	new JobOutputStream(stdoutListener));
	}
	
	/**
	 * @return
	 */
	private String makeRSL() throws GrpcException {
		RemoteClassPathInfo rcPath = remoteMachineInfo.getRemoteClassPath(className);
		String staging =
			(String) rcPath.get(RemoteClassPathInfo.KEY_CLASS_PATH_STAGING);
		StringBuffer sb = new StringBuffer();

		/* make RSL */
		sb.append("&");
		sb.append("(rsl_substitution=(NG_GASS_URL " + gassServer.getURL() + "))\n");
		
		/* jobtype */
		if ((jobType != null) &&
			(jobType.equals(RemoteMachineInfo.VAL_BACKEND_MPI) ||
			 jobType.equals(RemoteMachineInfo.VAL_BACKEND_BLACS))) {
			sb.append("(jobType=mpi)");
			jobCount = NgGrpcJob.INVALID_JOB_ID;
		}
		/* count */
		if (jobCount == NgGrpcJob.INVALID_JOB_ID) {
			sb.append("(count=" +
				remoteMachineInfo.getNumCPUs(className) + ")");
		} else {
			sb.append("(count=" + jobCount + ")");
		}
		sb.append("\n");
		
		/* executable */
		sb.append("(executable=");
		if ((staging != null) && (staging.equals("true"))) {
			sb.append("$(NG_GASS_URL) # /");
		}
		sb.append(rcPath.get(RemoteClassPathInfo.KEY_CLASS_PATH_CLASSPATH));
		sb.append(")\n");

		/* set arguments */
		makeRSLArguments(sb);
		
		/* set environment */
		makeEnvironment(sb);
		
		/* set GASS server */
		if (redirect_outerr == true) {
			sb.append("(stderr=$(NG_GASS_URL) # /dev/std" +
				STDERR_URI + ")");
			sb.append("(stdout=$(NG_GASS_URL) # /dev/std" +
				STDOUT_URI + ")");
			sb.append("\n");
		}
		
		/* add jobmanager related strings */
		String queue =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_QUEUE);
		if (queue != null) {
			sb.append("(queue=" + queue + ")");
		}
		String project =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_PROJECT);
		if (project != null) {
			sb.append("(project=" + project + ")");
		}
		String hostCount =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTCOUNT);
		if (hostCount != null) {
			sb.append("(hostCount=" + hostCount + ")");
		}
		String minMemory =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MINMEMORY);
		if (minMemory != null) {
			sb.append("(minMemory=" + minMemory + ")");
		}
		String maxMemory =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXMEMORY);
		if (maxMemory != null) {
			sb.append("(maxMemory=" + maxMemory + ")");
		}
		String maxTime =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXTIME);
		if (maxTime != null) {
			sb.append("(maxTime=" + maxTime + ")");
		}
		String maxWallTime =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXWALLTIME);
		if (maxWallTime != null) {
			sb.append("(maxWallTime=" + maxWallTime + ")");
		}
		String maxCpuTime =
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MAXCPUTIME);
		if (maxCpuTime != null) {
			sb.append("(maxCpuTime=" + maxCpuTime + ")");
		}
		
		/* work directory */
		RemoteClassPathInfo remoteClassPathInfo = 
			remoteMachineInfo.getRemoteClassPath(className);
		String stagingFlag = (String) remoteClassPathInfo.get(
				RemoteClassPathInfo.KEY_CLASS_PATH_STAGING);
		if ((stagingFlag == null) || (stagingFlag.equals("true") != true)) {
			sb.append("(directory=");
			if (remoteMachineInfo.get(RemoteMachineInfo.KEY_WORK_DIR) != null) {
				sb.append(remoteMachineInfo.get(RemoteMachineInfo.KEY_WORK_DIR));
			} else {
				String pathToClass = (String) remoteClassPathInfo.get(
					RemoteClassPathInfo.KEY_CLASS_PATH_CLASSPATH);
				String parentDirOfClass = new File(pathToClass).getParent();
				sb.append(parentDirOfClass.replace('\\', '/'));
			}
			sb.append(")");
		}
		
		/* RSL extensions */
		List rslExtensions =
			(List) remoteMachineInfo.get(RemoteMachineInfo.KEY_JOB_RSL_EXTENSION);
		if (rslExtensions != null) {
			for (int i = 0; i < rslExtensions.size(); i++) {
				sb.append(rslExtensions.get(i));
			}
		}
		
		return sb.toString();
	}

	/**
	 * @param sb
	 * @throws GrpcException
	 */
	private void makeEnvironment(StringBuffer sb) throws GrpcException {
		/* environment */
		List listEnvironment =
			(List) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_ENVIRONMENT);
		if ((listEnvironment == null) || (listEnvironment.size() < 1)) {
			/* don't append environment part */
			return;
		}

		/* Add Environment variable */
		sb.append("(environment = ");
		for (int i = 0; i < listEnvironment.size(); i++) {
			String strEnvironment = (String) listEnvironment.get(i);
			StringTokenizer st = new StringTokenizer(strEnvironment, "=");
			if (st.countTokens() < 1) {
				throw new NgInitializeGrpcHandleException(
					"found invalid environment variable.");
			}
			
			if (st.countTokens() == 1) {
				/* append name of variable and variable */
				sb.append("(" + st.nextToken() + " \"\")");
			} else if (st.countTokens() == 2) {
				/* append name of variable and variable */
				sb.append("(" + st.nextToken() + " \"" + st.nextToken() + "\")");
			}
		}
		sb.append(")");
		sb.append("\n");
	}

	/**
	 * @param sb
	 */
	private void makeRSLArguments(StringBuffer sb) throws GrpcException {
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

		/* set arguments for Ninf-G Executable */
		sb.append("(arguments= ");

		String[] listArguments = ngJob.getArgumentList();
		for (int i = 0; i < listArguments.length; i++) {
			sb.append(" \"" + listArguments[i] + "\"");
		}
		
		sb.append(")");
		sb.append("\n");
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void cancel() throws GrpcException {
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG, 
				context,
				"NgGramJob#cancel : canceling GRAM JOB.");
		try {
			job.cancel();
		} catch (GramException e) {
			throw new NgException(e);
		} catch (GSSException e) {
			throw new NgException(e);
		}
	}
	
	/* (non-Javadoc)
	 * @see org.globus.gram.GramJobListener#statusChanged(org.globus.gram.GramJob)
	 */
	public synchronized void statusChanged(GramJob job) {
		/* it's not status of this job... */
		if (job.equals(this.job) != true) {
			/* do nothing */
			return;
		}
		
		/* get status of job */
		int jobStatus = job.getStatus();
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG, 
				context,
				"NgGramJob#statusChanged : JOB status was changed to " + jobStatus + ".");

		/* change status to active  or failed */
		if (((jobStatus & GRAMConstants.STATUS_ACTIVE) != 0) ||
			((jobStatus & GRAMConstants.STATUS_FAILED) != 0)) {
			/* set condition */
			ngJob.setStatus(NgGrpcJob.NGJOB_STATE_ACTIVE);
		}

		/* change status to done or failed */
		if (((jobStatus & GRAMConstants.STATUS_DONE) != 0) ||
			((jobStatus & GRAMConstants.STATUS_FAILED) != 0)) {
			/* print output of Executable */
			if ((stdoutListener != null) && (stdoutListener.hasData())) {
				String stdoutOutput = stdoutListener.getOutput();
				System.out.println (stdoutOutput);
			}
			if ((stderrListener != null) && (stderrListener.hasData())) {
				String stderrOutput = stderrListener.getOutput();
				System.err.println (stderrOutput);
			}
			
			/* unregist listener from GramJob */
			job.removeListener(this);
			
			if (redirect_outerr == true) {
				/* unregist listener from GASS server */
				gassServer.unregisterJobOutputStream(STDERR_URI);
				gassServer.unregisterJobOutputStream(STDOUT_URI);
			}
			
			/* set condition */
			ngJob.setStatus(NgGrpcJob.NGJOB_STATE_DONE);
		}
	}
}
