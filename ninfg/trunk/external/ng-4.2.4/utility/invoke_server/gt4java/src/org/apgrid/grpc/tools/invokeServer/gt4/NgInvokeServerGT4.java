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
 * $RCSfile: NgInvokeServerGT4.java,v $ $Revision: 1.1 $ $Date: 2005/11/01 08:02:37 $
 */
package org.apgrid.grpc.tools.invokeServer.gt4;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.Properties;
import java.util.Vector;

class NgInvokeServerGT4 extends NgInvokeServerBase {
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