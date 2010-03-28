/*
 * $RCSfile: UnicoreJob.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:05 $
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
package org.apgrid.grpc.tools.invokeServer.unicore;
import java.io.*;
import java.util.*;

import org.unicore.*;
import org.unicore.ajo.*;
import org.unicore.ajo.ActionGroup.*;
import org.unicore.sets.*;
import com.fujitsu.arcon.servlet.*;
import com.sun.net.ssl.internal.ssl.*;



class UnicoreJob extends TimerTask{
  static final int PENDING = 0;
  static final int ACTIVE  = 1;
  static final int DONE    = 2;
  static final int FAILED  = 3;

  static final String STATUS_STRINGS[] = {
	"PENDING", "ACTIVE", "DONE", "FAILED"
  };

  String usite = null;
  String vsite = null;
  String keystore = null;
  String passphrase = null;
  String user_alias = null;

  String executable_path = null;
  int    count = -1;
  boolean staging = false;
  ArrayList<String> args = new ArrayList<String>();
  ArrayList<String> envs = new ArrayList<String>();
  String gass_url = null;
  boolean redirect_enable = false;
  String  stdout_file = null;
  String  stderr_file = null;
  int     status_polling = 0;
  int     refresh_credential = 0;


  Identity identity;
  PortfolioTh importPortfolioTh;
  int status = PENDING; // initial state

  String getStatusString(){
	return STATUS_STRINGS[status];
  }

  boolean destroyRequested = false;
  AbstractJob ajo;
  AJOIdentifier ajoId;
  String jobId;
  CreateCommand command;
  VsiteTh vsiteTh;

  /** 
   *  check the command. if it is not ok, throws an exception
   */
  UnicoreJob(CreateCommand command) throws Command.Exception{
	this.command = command;

	if ((usite = command.list.get("usite")) == null)
	  throw new Command.Exception("usite is not in the command");

	if ((vsite = command.list.get("vsite")) == null)
	  throw new Command.Exception("vsite is not in the command");

	if ((keystore = command.list.get("keystore")) == null)
	  throw new Command.Exception("keystore is not in the command");

	if ((passphrase = command.list.get("passphrase")) == null)
	  throw new Command.Exception("passphrase is not in the command");

	if ((user_alias = command.list.get("user_alias")) == null)
	  throw new Command.Exception("user_alias is not in the command");

	if ((executable_path = command.list.get("executable_path")) == null)
	  throw new Command.Exception("executable_path is not in the command");

	String tmp;
	if ((tmp = command.list.get("count")) == null)
	  throw new Command.Exception("count is not in the command");
	count = Integer.parseInt(tmp);

	if ((tmp = command.list.get("staging")) == null)
	  throw new Command.Exception("staging is not in the command");
	staging = Boolean.parseBoolean(tmp);

	for (int i = 0; true; i++){
	  if ((tmp = command.list.get("argument", i)) == null)
		break;
	  args.add(tmp);
	}

	for (int i = 0; true; i++){
	  if ((tmp = command.list.get("environment", i)) == null)
		break;
	  envs.add(tmp);
	}

	if ((gass_url = command.list.get("gass_url")) == null)
	  throw new Command.Exception("gass_url is not in the command");

	if ((tmp = command.list.get("redirect_enable")) == null)
	  throw new Command.Exception("redirect_enable is not in the command");
	redirect_enable = Boolean.parseBoolean(tmp);

	if (redirect_enable){
	  if ((stdout_file = command.list.get("stdout_file")) == null)
		throw new Command.Exception("stdout_file is not in the command");
	  if ((stderr_file = command.list.get("stderr_file")) == null)
		throw new Command.Exception("stderr_file is not in the command");
	}

	if ((tmp = command.list.get("status_polling")) == null)
	  throw new Command.Exception("status_polling is not in the command");
	status_polling = Integer.parseInt(tmp);
  }

  /**
   * actually invokes a job
   */ 

  boolean start(){
	Log.log("Creating job ..");
	try {
	  createAjo();
	} catch (UnicoreJob.Exception e) {
	  Log.printStackTrace(e);
	  sendCreateNotifyFailed(e.getMessage());	  
	  return false;
	}
	Log.log("               .. done");
	Log.log("Starting job ..");
	try {
	  	  consignAjo();
	} catch (UnicoreJob.Exception e) {
	  Log.printStackTrace(e);
	  sendCreateNotifyFailed(e.getMessage());	  
	  return false;
	}
	Log.log("               .. done");	
	sendCreateNotifySuccess();
	return true;
  }

  private void consignAjo() throws Exception {
	try {
	  vsiteTh = 
		new VsiteTh(new Reference.SSL(ajo.getVsite().getAddress(), identity), 
					ajo.getVsite());

	  PortfolioTh[] importPortfolioThs = null;
	  if (importPortfolioTh != null)
		importPortfolioThs = new PortfolioTh[]{importPortfolioTh};		
	  
	  UnicoreUtils.consignAsyncWithRetry(ajo, vsiteTh,
										 importPortfolioThs,
										 5, 1);
	} catch (Reference.Exception e) {
	  throw new Exception(e);
	} catch (Connection.Exception e) {
	  throw new Exception(e);
	} catch (JobManager.Exception e) {
	  throw new Exception(e);
	}
  }



  private String createScriptString(){
	StringBuffer sb = new StringBuffer();
	sb.append("chmod 755 ./*; ");

	String command = executable_path;
	if (staging)
	  command = "./" + command.substring(command.lastIndexOf("/") + 1, command.length());
	
	sb.append(command);

	for (String arg : args){
	  sb.append(" ");
	  sb.append(arg);
	}
	
	return sb.toString();
  }

  private String readPass(String passphraseFile) throws Exception {
	try {
	  LineNumberReader lnr = new LineNumberReader(new FileReader(passphrase));
	  String line = lnr.readLine();
	  return line.trim();
	} catch (IOException e){
	  throw new Exception("failed to read passphraseFile " + passphraseFile);
	}
  }


  private void createAjo() throws Exception{
	try {
	  identity = 
		UnicoreUtils.createIdentity(keystore, readPass(passphrase), user_alias);
	  Vsite vsiteObj = UnicoreUtils.getVsiteFromGw(usite, vsite, identity);
	  
	  ajo = new AbstractJob("Root AJO");
	  
	  String script = createScriptString();
	  Log.log("Script String = " + script);
	  
	  // ScriptTask
	  ActionGroup execTask = 
		UnicoreUtils.createExecScriptTask(script, stdout_file, envs);
	  
	  ajo.add(execTask);
	  
	  // if staging is required, setup import task for the executable
	  
	  if (staging){
		importPortfolioTh = UnicoreUtils.createPortfolioTh(new String[]{executable_path});
		ActionGroup importTask = 
		  UnicoreUtils.createImportTask(importPortfolioTh);
		ajo.add(importTask);
		ajo.addDependency(importTask, execTask);
	  }
	  // no need for export !

	  ajo.setVsite(vsiteObj);

	  jobId = "" + ajo.getAJOId().getValue();
	  ajoId = new AJOIdentifier("Root AJO", ajo.getAJOId().getValue());

	} catch (Identity.Exception e){
	  throw new Exception(e);
	} catch (InvalidDependencyException e){
	  throw new Exception(e);
	}
  }
						 


  /** 
	if the status is already DONE or FAILED,
	simply remove itsself from the map.

	if not, set the destroyRequested flag to true, 
	and 	send cancel command to the server.

   */

  synchronized void destroy(){
	if (status == DONE || status == FAILED){
	  command.is.removeJob(this);
	}
	destroyRequested = true;
	
  }
  
  private void sendCreateNotifySuccess(){
	command.notify("CREATE_NOTIFY " + command.requestId + " S " + jobId );
  }

  private void sendCreateNotifyFailed(String msg){
	command.notify("CREATE_NOTIFY " + command.requestId + " F " + msg );
  }

  private void sendStatusNotify(){
	command.notify("STATUS_NOTIFY " + jobId  + " " + getStatusString());
  }


  synchronized void statusUpdated(int status){
	if (this.status == status)  // no update
	  return;

	this.status = status;
	sendStatusNotify();
	if (status == DONE || status == FAILED)
	  command.is.removeJob(this);
  }

  int errorCount = 0;
  int maxErrors  = 10;


  /** This will periodically called by Timer process **/
  public void run(){
	Log.set(""+command.requestId);
	Log.log("-- getting outcome ");
	UnicoreJobStatus uStatus = null;
	try {
	  uStatus = 
		UnicoreUtils.getUnicoreJobStatus(ajoId, vsiteTh, stderr_file);
	} catch (Connection.Exception e){
	  Log.printStackTrace(e);
	  if (errorCount++ >= maxErrors){
		Log.log("giveup..");
		this.cancel();
	  }
	} catch (TsJobManager.Exception e){
	  Log.printStackTrace(e);
	  if (errorCount++ >= maxErrors){
		Log.log("giveup..");
		this.cancel();
	  }
	}

	statusUpdated(uStatus.getJobStatus());
	Log.log("-- getting outcome .. done ");
	
	if (this.status == DONE || this.status == FAILED)
	  this.cancel();
	  
	Log.set("---");
  }

  public String toString(){

	String tmp = "Unicore Job\n" +
	  "\tusite = \t" + usite + "\n" +
	  "\tvsite = \t" + vsite + "\n" +
	  "\tkeystore = \t" + keystore + "\n" +
	  "\tpassphrase = \t" + passphrase + "\n" +
	  "\tuser_alias = \t" + user_alias + "\n" +
	  "\texecutable_path = \t" + executable_path + "\n" +
	  "\tcount = \t" + count + "\n";
	  
	for (String s : args)
	  tmp += "\t\targ = \t" + s + "\n";
	for (String s : envs)
	  tmp += "\t\tenv = \t" + s + "\n";

	tmp += 
	  "\tgass_url = \t" + gass_url + "\n" +
	  "\tredirect_enable = \t" + redirect_enable + "\n" +
	  "\tstdout_file = \t" + stdout_file + "\n" +
	  "\tstderr_file = \t" + stderr_file + "\n" +
	  "\tstatus_polling = \t" + status_polling + "\n" +
	  "\trefresh_credential = \t" + refresh_credential + "\n";
	return tmp;

  }


  public static class Exception extends java.lang.Exception {
	java.lang.Exception org;
	public Exception(java.lang.Exception org){
	  super("UnicoreJob.Exception:org msg = " + org.getMessage());
	  this.org = org;
	}
	public Exception(String msg){
	  super(msg);
	}

	public void printStackTrace(PrintStream o){
	  if (o == null)
		super.printStackTrace(o);
	  else
		org.printStackTrace(o);
	}
  }

}

