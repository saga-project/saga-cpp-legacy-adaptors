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
 * $RCSfile: UnicoreInvokeServer.java,v $ $Revision: 1.2 $ $Date: 2005/11/01 07:46:58 $
 */
package org.apgrid.grpc.tools.invokeServer.unicore;

import java.io.*;
import java.util.*;

public class UnicoreInvokeServer {
  private static final String CMD_CREATE     = "JOB_CREATE";
  private static final String CMD_STATUS     = "JOB_STATUS";
  private static final String CMD_DESTROY    = "JOB_DESTROY";
  private static final String CMD_EXIT       = "EXIT";

  private static final String lineSeparator  = new String(new char[]{0x0d, 0x0a});
  private static final String DEFAULT_LOG_FILE = "/tmp/unicore.is.log";

  
  static long pollingInterval = 18000; // 3 min.

  Timer timer = new Timer();
  static LineNumberReader lnr = new LineNumberReader(new InputStreamReader(System.in));

  private void start(){
	Command cmd;
	try {
	  while ((cmd = readCommand()) != null){
		Log.log(cmd);
		cmd.handle();
	  }
	} catch (Command.Exception e){
	  sendReply("F " + "failed to read command " + e.getMessage());
	  abort("failed to read command: " + e.getMessage());
	}
  }

  private Command readCommand() throws Command.Exception{
	String line = readLine();
	StringTokenizer st = new StringTokenizer(line);
	String id = null;
	try {
	  id = st.nextToken(); 
	} catch (NoSuchElementException e){
	  abort("failed to read Command 0 : str = " + line + ". Aborting ");
	}
	if (id.equals(CMD_EXIT))
	  return new ExitCommand(this);
	String next = null;
	try {
	  next = st.nextToken();
	} catch (NoSuchElementException e){
	  abort("failed to read Command 1 : str = " + line + ". Aborting ");
	}

	// other commands require one arg
	if (next == null) 
	  abort("failed to read Command 2 : str = " + line + ". Aborting ");
	if (id.equals(CMD_CREATE))
	  return new CreateCommand(this, next);
	if (id.equals(CMD_STATUS))
	  return new StatusCommand(this, next);
	if (id.equals(CMD_DESTROY))
	  return new DestroyCommand(this, next);
	
	abort("cannot parse command: str = " + line + ". Aborting ");
	return null;
  }


  /** 
   * table to store unicore job
   */
  HashMap<String, UnicoreJob>  map = new HashMap<String, UnicoreJob>();

  synchronized void removeJob(UnicoreJob job){
	job.cancel();  // stop the timer task
	map.remove(job.jobId);
  }

  synchronized void registerJob(UnicoreJob job){
	timer.schedule(job, pollingInterval, pollingInterval);
	map.put(job.jobId, job);
  }

  synchronized UnicoreJob getJob(String jobId){
	return map.get(jobId);
  }
  
  /** 
   *   read one line from stdin and return 
   */ 
  String readLine() {
	try {
	  String str = lnr.readLine();
	  Log.log("--> " + str);
	  return str; 
	} catch (IOException e) {
	  // failed to read a line. nothing can be done.
	  abort("Failed to Read. ABORTING");
	}
	return null;
  }

  static void abort(String str){
	System.err.println(str);
	Log.log(str);
	System.exit(2);
  }



  /** 
   *	output the string to the stdout stream  
   */
  synchronized void sendReply(String str){
	Log.log("REPLY: " + str);
	System.out.print(str);
	System.out.print(lineSeparator);
	System.out.flush();
  }

  /** 
   *    output the string to the stderr stream 
   */
  synchronized void sendNotify(String str){
	Log.log("NOTIFY: " + str);
	System.err.print(str);
	System.err.print(lineSeparator);
	System.err.flush();
  }



  void cancelAll(){
  }

  public static void main(String [] args) throws IOException {
	String logfile = DEFAULT_LOG_FILE;

	if (args.length >= 2 && args[0].equals("-l")) 
	   logfile = args[1];
	try {
	  Log.setStream(new PrintWriter(new FileWriter(logfile, true), true));
	} catch (IOException e){
	  abort("failed to open log");
	}

	// setup line separator that will be used by 'println'
	System.setProperty("line.separator", lineSeparator);

	Log.log("Starting up... UNICORE INVOKE SERVER");
	(new UnicoreInvokeServer()).start();
  }


}
