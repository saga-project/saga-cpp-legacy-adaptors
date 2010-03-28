/*
 * $RCSfile: NgInvokeServerGT4.java,v $ $Revision: 1.10 $ $Date: 2008/03/15 07:09:02 $
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
package org.apgrid.grpc.tools.invokeServer.gt4;

import java.lang.Integer;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Properties;
import java.util.Vector;

class NgInvokeServerGT4 extends NgInvokeServerBase {
	private static HashMap<String, Integer> optionMap = new HashMap<String, Integer>();
	static {
            // Integer: 0: unsupported, 1: supported
	    optionMap.put("hostname",		new Integer(1));
	    optionMap.put("port",		new Integer(1));
	    optionMap.put("jobmanager",		new Integer(1));
	    optionMap.put("subject",		new Integer(0));
	    optionMap.put("client_name",	new Integer(0));
	    optionMap.put("executable_path",	new Integer(1));
	    optionMap.put("backend",		new Integer(1));
	    optionMap.put("count",		new Integer(1));
	    optionMap.put("staging",		new Integer(0));
	    optionMap.put("argument",		new Integer(1));
	    optionMap.put("work_directory",	new Integer(1));
	    optionMap.put("gass_url",		new Integer(0));
	    optionMap.put("redirect_enable",	new Integer(1));
	    optionMap.put("stdout_file",	new Integer(1));
	    optionMap.put("stderr_file",	new Integer(1));
	    optionMap.put("environment",	new Integer(1));
	    optionMap.put("tmp_dir",		new Integer(0));
	    optionMap.put("status_polling",	new Integer(0));
	    optionMap.put("refresh_credential",	new Integer(0));
	    optionMap.put("max_time",		new Integer(0));
	    optionMap.put("max_wall_time",	new Integer(0));
	    optionMap.put("max_cpu_time",	new Integer(0));
	    optionMap.put("queue_name",		new Integer(0));
	    optionMap.put("project",		new Integer(0));
	    optionMap.put("host_count",		new Integer(0));
	    optionMap.put("min_memory",		new Integer(0));
	    optionMap.put("max_memory",		new Integer(0));
	    optionMap.put("rsl_extensions",	new Integer(0));
	}

	/**
	 * 
	 */
	public NgInvokeServerGT4(PrintStream ps) {
		super(ps);
	}
	
	/**
	 * @param requestString
	 * @return
	 * @throws Exception
	 */
	protected String processRequest(String requestString) throws Exception {
		/* get Request ID from String of REQUEST */
		String[] requestStringArray = requestString.split(" ");
		String idStr = null;
		if (requestStringArray.length > 1) {
			idStr = requestStringArray[1];
		}
		putLogMessage("requestString is " + requestString + ".");
		
		if (requestString.startsWith(REQUEST_JOB_CREATE)) {
			/* JOB_CREATE */
			putLogMessage("Process [" + REQUEST_JOB_CREATE + "]");
			/* get arguments for creating JOB */
			String line = null;
			Properties propCreateArg = new Properties();
			while ((line = this.br.readLine()) != null) {
				putLogMessage("CREATE_JOB: \"" + line + "\"");
				if (line.equals(REQUEST_JOB_CREATE_END)) {
					/* the end of JOB_CREATE */
					break;
				} else if (line.startsWith(ARG_ARGUMENT) ||	line.startsWith(ARG_ENVIRONMENT)) {
					/* separate into two parts */
					int keyLength = line.indexOf(' ');
					String key = line.substring(0, keyLength);
					String val = line.substring(keyLength + 1);
					/* argument */
					Vector listTemp = (Vector) propCreateArg.get(key);
					if (listTemp == null) {
						listTemp = new Vector();
					}
					/* append argument into list */
					listTemp.add(val);
					/* put list into map */
					propCreateArg.put(key, listTemp);
				} else {
					/* separate into two parts */
					int keyLength = line.indexOf(' ');
					String key = line.substring(0, keyLength);
					String val = line.substring(keyLength + 1);
					checkOptions(key);

					/* put argument into Properties */
					propCreateArg.put(key, val);
				}
			}
				
			/* create GramJob for GT4 and register it */
			NgGramJobGT4 gt4Job = new NgGramJobGT4(
					Integer.parseInt(idStr), propCreateArg, this);
			putLogMessage("put requested job into properties");
			propRequestedJobs.put(idStr, gt4Job);
			try {
				/* invoke GT4 job */
				gt4Job.invokeExecutable();
			} catch (Exception e) {
				return REPLY_FAILED + " failed to invoke the JOB.";
			}
		} else if (requestString.startsWith(REQUEST_JOB_STATUS)) {
			/* JOB_STATUS */
			putLogMessage("Process [" + REQUEST_JOB_STATUS + "]");
			NgGramJobGT4 gt4Job = (NgGramJobGT4) propCreatedJobs.get(idStr);
			//StateEnumeration jobStatus = gt4Job.getState();
			/* add convert GT STATE -> NG STATE */
		} else if (requestString.startsWith(REQUEST_JOB_DESTROY)) {
			/* JOB DESTROY */
			putLogMessage("Process [" + REQUEST_JOB_DESTROY + "]");
			NgGramJobGT4 gt4Job = (NgGramJobGT4) propCreatedJobs.get(idStr);
			gt4Job.incrementExitCount();
			gt4Job.dispose();
		} else if (requestString.startsWith(REQUEST_QUERY_FEATURES)) {
			System.out.print("SM");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
			System.out.print("protocol_version 2.0");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
			System.out.print("request JOB_CREATE");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
			System.out.print("request JOB_STATUS");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
			System.out.print("request JOB_DESTROY");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
			System.out.print("request QUERY_FEATURES");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
			System.out.print("request EXIT");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
			System.out.print("REPLY_END");
			System.out.print(INVOKE_SERVER_PROT_DELIMITER);
		} else if (requestString.startsWith(REQUEST_EXIT)) {
			putLogMessage("Process [" + REQUEST_EXIT + "]");
			/* nothing will be done */
		} else {
			/* unknown request */
			putLogMessage("Unknown request was received.");
			return REPLY_FAILED + " Unknown Request.";
		}
		
		/* return success */
		return REPLY_SUCCESS;
	}
	
	/**
	 * Check options
	 *
	 * @param option
	 */
	private void checkOptions(String option) {
	    if (!optionMap.containsKey(option)) {
		putLogMessage("Warning: Unknown option \"" + option + "\".");
	    } else if (optionMap.get(option).intValue() == 0) {
		putLogMessage(
		    "Warning: Unsupported option \"" + option + "\".");
	    }
	}

	/**
	 * run Invoke Server Process
	 * 
	 * @param args
	 */
	public static void main (String[] args) {
		/* parse arguments */
		PrintStream ps = null;
		for (int i = 0; i < args.length; i++) {
			// System.err.println ("args[i]" + args[i]);
			if (args[i].equals("-l")) {
				/* specified logfile */
				if ((i + 1) > args.length) {
					// System.err.println("Invalid argument.");
					System.exit(1);
				}
				i += 1;
				try {
					/* open logfile */
					ps = new PrintStream(new FileOutputStream(args[i]));
				} catch (FileNotFoundException e) {
					e.printStackTrace();
					System.exit(1);
				}
			} else {
				// System.err.println("Invalid argument.");
				System.exit(1);
			}
		}
		
		/* create NgInvokeServerGT4 */
		NgInvokeServerGT4 isGT4 = new NgInvokeServerGT4(ps);
		
		/* start Invoke Server by Thread */
		new Thread(isGT4).start();
	}
}
