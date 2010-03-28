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
 * $RCSfile: NgInvokeServerBase.java,v $ $Revision: 1.2 $ $Date: 2005/11/24 09:54:47 $
 */
package org.apgrid.grpc.tools.invokeServer.gt4;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.Enumeration;
import java.util.Properties;

abstract class NgInvokeServerBase implements Runnable {
	/* protocol */
	public static final String REQUEST_JOB_CREATE = "JOB_CREATE";
	public static final String REQUEST_JOB_CREATE_END = "JOB_CREATE_END";
	public static final String REQUEST_JOB_STATUS = "JOB_STATUS";
	public static final String REQUEST_JOB_DESTROY = "JOB_DESTROY";
	public static final String REQUEST_EXIT = "EXIT";
	
	public static final String REPLY_SUCCESS = "S";
	public static final String REPLY_FAILED = "F";

	public static final String NOTIFY_CREATE = "CREATE_NOTIFY";
	public static final String NOTIFY_STATUS = "STATUS_NOTIFY";
	
	private static final String INVOKE_SERVER_PROT_DELIMITER = "\r\n";

	/* status of JOBs */
	public static final String STATUS_PENDING = "PENDING";
	public static final String STATUS_ACTIVE = "ACTIVE";
	public static final String STATUS_DONE = "DONE";
	public static final String STATUS_FAILED = "FAILED";
	
	/* arguments for CREATE_JOB */
	public static final String ARG_HOSTNAME = "hostname";
	public static final String ARG_PORT = "port";
	public static final String ARG_JOBMANAGER = "jobmanager";
	public static final String ARG_SUBJECT = "subject";
	public static final String ARG_EXECUTABLE_PATH = "executable_path";
	public static final String ARG_BACKEND = "backend";
	public static final String ARG_COUNT = "count";
	public static final String ARG_STAGING = "staging";
	public static final String ARG_ARGUMENT = "argument";
	public static final String ARG_WORK_DIRECTORY = "work_directory";
	public static final String ARG_GASS_URL = "gass_url";
	public static final String ARG_REDIRECT_ENABLE = "redirect_enable";
	public static final String ARG_STDOUT_FILE = "stdout_file";
	public static final String ARG_STDERR_FILE = "stderr_file";
	public static final String ARG_ENVIRONMENT = "environment";
	public static final String ARG_STATUS_POLLING = "status_polling";
	public static final String ARG_REFRESH_CREDENTIAL = "refresh_credential";
	public static final String ARG_MAX_TIME = "max_time";
	public static final String ARG_MAX_WALL_TIME = "max_wall_time";
	public static final String ARG_MAX_CPU_TIME = "max_cpu_time";
	public static final String ARG_QUEUE_NAME = "queue_name";
	public static final String ARG_PROJECT = "project";
	public static final String ARG_HOST_COUNT = "host_count";
	public static final String ARG_MIN_MEMORY = "min_memory";
	public static final String ARG_MAX_MEMORY = "max_memory";

	/* map for JOBs */
	Properties propRequestedJobs;
	Properties propCreatedJobs;
	/* stdin */
	BufferedReader br;
	/* logfile */
	PrintStream ps;
	
	/**
	 * 
	 */
	public NgInvokeServerBase(PrintStream ps) {
		/* create map for JOBs */
		this.propRequestedJobs = new Properties();
		this.propCreatedJobs = new Properties();

		/* prepare BufferedReader for stdin */
		this.br = new BufferedReader(new InputStreamReader(System.in));
		/* PrintStream for logfile */
		this.ps = ps;
	}
	
	/**
	 * @param ngJob
	 * @param JobID
	 */
	protected void setJobID(Object ngJob, String JobID) {
		putLogMessage("NgInvokeServer#setJobID: set JobID");
		
		/* search for specified NgJob object */
		Enumeration keys = propRequestedJobs.keys();
		while (keys.hasMoreElements()) {
			String target = (String) keys.nextElement();
			Object tmpNgJob = propRequestedJobs.get(target);
			putLogMessage("NgInvokeServer#setJobID:searching JOB for ID: " + target + ".");
			if (tmpNgJob == ngJob) {
				/* found it! move from mapRequested to mapCreated */
				putLogMessage("NgInvokeServer#setJobID: set Request[" +
					target + "] to jobID [" + JobID + "]");
				propCreatedJobs.put(JobID, ngJob);
				propRequestedJobs.remove(target);
				
				/* send create notify */
				System.err.print(NOTIFY_CREATE + " " +
					target + " " + REPLY_SUCCESS + " " + JobID);
				System.err.print(INVOKE_SERVER_PROT_DELIMITER);

				return;
			}
		}
		
		/* can't find target... */
		putLogMessage("NgInvokeServer#setJobID: can't fine the target...");
	}
	
	/**
	 * @param ngJob
	 * @param status
	 */
	protected void setStatus(Object ngJob, String status) {
		putLogMessage("NgInvokeServer#setStatus: setStatus");
		
		/* search for specified NgJob object */
		Enumeration keys = propCreatedJobs.keys();
		while (keys.hasMoreElements()) {
			String target = (String) keys.nextElement();
			Object tmpNgJob = propCreatedJobs.get(target);
			putLogMessage("NgInvokeServer#setStatus:searching JOB for ID: " + target + ".");
			if (tmpNgJob == ngJob) {
				/* found it! send status notify */
				putLogMessage("NgInvokeServer#setStatus: set JobID[" +
						target + "] to status [" + status + "]");
				System.err.print(NOTIFY_STATUS + " " + target + " " + status);
				System.err.print(INVOKE_SERVER_PROT_DELIMITER);
				return;
			}
		}
		
		/* can't find target... */
		putLogMessage("NgInvokeServer#setStatus: can't fine the target...");
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		/* receive request from stdin and send reply to stdout */
		String line = null;
		try {
			putLogMessage("NgInvokeServer#run: start Thread for receiving request.");
			while ((line = this.br.readLine()) != null) {
				putLogMessage("NgInvokeServer#run: received request is " + line + ".");
				/* received request */
				String reply = null;
				try {
					reply = processRequest(line);
				} catch (Exception e1) {
					putLogMessage("NgInvokeServer#run: something wrong was happened, return Failed.");
					putLogMessage (e1.getMessage());
					
					System.out.print(REPLY_FAILED);
					System.out.print(INVOKE_SERVER_PROT_DELIMITER);
					continue;
				}
				/* send reply */
				putLogMessage("NgInvokeServer#run: send reply is " + reply + ".");
				System.out.print(reply);
				System.out.print(INVOKE_SERVER_PROT_DELIMITER);
				
				/* if it's EXIT request,then exit */
				if (line.startsWith(REQUEST_EXIT)) {
					break;
				}
			}
		} catch (IOException e) {
			/* something wrong was happend */
			putLogMessage("NgInvokeServer#run: something wrong was happened, return Failed.");
			putLogMessage (e.getMessage());
			
			System.out.print(REPLY_FAILED);
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
		} finally {
			try {
				/* close InputStream */
				br.close();
			} catch (IOException e1) {
				putLogMessage("NgInvokeServer#run: something wrong was happened, return Failed.");
				putLogMessage (e1.getMessage());
			}
		}
	}
	
	/**
	 * @param message
	 */
	protected void putLogMessage(String message) {
		if (ps != null) {
			ps.println(message);
		}
	}
	
	/**
	 * @param requestString
	 * @return
	 * @throws Exception
	 */
	abstract String processRequest(String requestString) throws Exception;
}