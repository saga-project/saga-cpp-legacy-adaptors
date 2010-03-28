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
 * $RCSfile: UnicoreUtils.java,v $ $Revision: 1.3 $ $Date: 2005/11/01 07:49:33 $
 */
package org.apgrid.grpc.tools.invokeServer.unicore;

import org.unicore.*;
import org.unicore.ajo.*;
import org.unicore.ajo.ActionGroup.*;
import org.unicore.sets.*;
import com.fujitsu.arcon.servlet.*;
import com.sun.net.ssl.internal.ssl.*;

import java.io.*;
import java.util.*;
import java.security.Security;

/**
 * provides unicore related routines
 */ 
class UnicoreUtils {


  static public Vsite getVsiteFromGw(String gateway, String vsite_name,
									 Identity identity)  
  throws org.apgrid.grpc.tools.invokeServer.unicore.UnicoreJob.Exception{
	Exception e = null;
	for (int i = 10; i >= 0; i--){
	  e = null;
	  try {
		Gateway gw = new Gateway(new Reference.SSL(gateway, identity));
		VsiteManager.addGateway(gw);
		JobManager.listVsites(gw);
		Iterator it = VsiteManager.getVsites();
		
		while (it.hasNext()) {
		  VsiteTh vsiteTh = (VsiteTh)it.next();
		  if (vsiteTh.toString().equals(vsite_name)) 
			return vsiteTh.getVsite();
		}
		Log.log("vsite " + vsite_name  + " is not registered .. might retry");
		
	  } catch (Gateway.Exception e1) {   e = e1;
	  } catch (Reference.Exception e1) { e = e1;
	  } catch (Connection.Exception e1) { e = e1;
	  } catch (java.lang.Exception e1) {  e = e1;
	  }
	  if (e != null) 
		Log.log(e.toString() + " <Retry: " + i + " more times >");
	  try {
		Thread.sleep((long) (10000 * Math.random()));
	  } catch (InterruptedException e2) {}
	}
	// never come here
	throw new org.apgrid.grpc.tools.invokeServer.unicore.UnicoreJob.Exception(e);
  }
  

  static public Identity createIdentity(String keystore, String passwd, String userAlias)
  throws com.fujitsu.arcon.servlet.Identity.Exception {
	Security.addProvider(new Provider());
	
	File keystorefile = new File(keystore);
	
	if (keystorefile.exists() == false) {
	  Log.log("## Not Found!! Keystore File");
	}
	
	Identity identity = new Identity(keystorefile, passwd.toCharArray(),
									 userAlias);
	
	return identity;
  }


  /** 
   * create portfolio to transfer in
   */
  static public PortfolioTh createPortfolioTh(String [] files){
	MakePortfolio mkpf = new MakePortfolio();
	mkpf.setFiles(files);
	
	File[] f = new File[files.length];
	for (int i = 0; i < files.length; i++) 
	  f[i] = new File(files[i]);

	PortfolioTh pf = new PortfolioTh(mkpf.getPortfolio(), f);
	return pf;
  }
  


  /**
   * create export task
   * @param exports
   *            file name you want to export from UNICORE
   * @return ActionGroup action group
   * @exception InvalidDependencyException
   *                invalid workflow dependency
   */
  static public ActionGroup createExportTask( String[] exports)
  throws InvalidDependencyException {
		 
	// IncarnateFiles->MakePortfolio->PutPortfolio->DeletePortfolio->ExportTask
	
	ActionGroup actg_expt = new ActionGroup("EXPORT");
	
	IncarnateFiles inf_expt = new IncarnateFiles("EXPORT-Task-1");
	
	inf_expt.addFile("export_file", exports[0].getBytes());
	// now export[] -> exports[0]
	
	MakePortfolio mkpf_expt = new MakePortfolio("EXPORT-Task-2");
	
	for (int i = 0; i < exports.length; i++) {
	  mkpf_expt.addFile(exports[i]);
	}
	
	// ppf_expt.setOverwrite(true);
	DeletePortfolio delpf_expt = new DeletePortfolio("EXPORT-Task-3");
	delpf_expt.setPortfolio(mkpf_expt.getPortfolio().getId());
	
	// copy PutPortfolio file to Outcome
	ResourceSet rscset = new ResourceSet();
	CopyPortfolioToOutcome cppftout_expt = new CopyPortfolioToOutcome(
																	  "EXPORT-Task-4", rscset.cloneSet(), mkpf_expt.getPortfolio()
																	  .getId());
	
	// add tasks to action groups
	actg_expt.add(inf_expt);
	actg_expt.add(mkpf_expt);
	actg_expt.add(delpf_expt);
	actg_expt.add(cppftout_expt);
	
	// set dependency among action groups
	actg_expt.addDependency(inf_expt, mkpf_expt);
	actg_expt.addDependency(mkpf_expt, delpf_expt);
	actg_expt.addDependency(delpf_expt, cppftout_expt);
	
	return actg_expt;
  }
  
  
  /**
   * create import task
   * 
   * @param pf
   *            portfoli contains import file's information
   * @return ActionGroup action group
   * @exception InvalidDependencyException
   *                invalid workflow dependency
   */
  static public ActionGroup createImportTask(PortfolioTh pfth)
  throws InvalidDependencyException {

	Portfolio pf = pfth.getPortfolio();
	ActionGroup actg_impt = new ActionGroup("IMPORT");
	
	CopyFile cpf_impt = new CopyFile("IMPORT-TASK-1");
	
	// copy portfolio files to upper directory
	cpf_impt.setFrom("." + pf.getUPLDirectoryName() + "/*");
	cpf_impt.setTo("");
	cpf_impt.setOverwrite(true);
	
	DeclarePortfolio decpf_impt = new DeclarePortfolio("IMPORT-TASK-2");
	decpf_impt.setPortfolio(pf);
	
	// add tasks to action groups
	actg_impt.add(cpf_impt);
	actg_impt.add(decpf_impt);
	
	// set dependency among action groups
	actg_impt.addDependency(cpf_impt, decpf_impt);
	
	return actg_impt;
  }
  
  /**
   * Create ScriptTask for file transfer
   * @param script
   *            script you want to execute
   * @param export
   *            file name you want to be ridirected to
   * @return ActionGroup this action group contains script task
   * @exception InvalidDependencyException
   *                invalid workflow dependency
   */
  static public ActionGroup createExecScriptTask(String script,
										String export_out, ArrayList<String> envs) 
  throws InvalidDependencyException {

	ActionGroup actg_execst = new ActionGroup("SCRIPT");
	
	IncarnateFiles inf_execst = new IncarnateFiles("SCRIPT-Task-1");
	inf_execst.addFile("script_file", script.getBytes());
	
	MakePortfolio mkpf_execst = new MakePortfolio("SCRIPT-Task-2");
	mkpf_execst.addFile("script_file");
	
	ExecuteScriptTask execst = new ExecuteScriptTask("SCRIPT-Task-3");
	execst.setScriptType(ScriptType.SH);
	execst.setExecutable(mkpf_execst.getPortfolio());
	
	if (export_out != null) {
	  execst.setStdoutFileName(export_out);
	  execst.setRedirectStdout(true);
	}
	
	OptionSet ops = makeEnv(envs);
	if (ops != null) {
	  execst.setEnvironmentVariables(ops);
	}
	
	DeletePortfolio delpf_execst = new DeletePortfolio("SCRIPT-Task-5");
	delpf_execst.setPortfolio(mkpf_execst.getPortfolio().getId());
	
	IncarnateFiles inf_stdin = null;
	MakePortfolio mkpf_stdin = null;

	// add tasks to action groups
	actg_execst.add(inf_execst);
	actg_execst.add(mkpf_execst);
	actg_execst.add(execst);
	actg_execst.add(delpf_execst);
	
	// set dependency among action groups
	actg_execst.addDependency(inf_execst, mkpf_execst);
	actg_execst.addDependency(mkpf_execst, execst);
	actg_execst.addDependency(execst, delpf_execst);
	
	return actg_execst;
  }


  static UnicoreJobStatus getUnicoreJobStatus(AJOIdentifier ajoId, 
											  VsiteTh vsiteTh,
											  String errFile)
  throws Connection.Exception, TsJobManager.Exception {

	UnicoreJobStatus uStatus = new UnicoreJobStatus();
	Log.log("try to getOutcome");

	TsJobManager jobmgr = new TsJobManager();
	//	jobmgr.setOutcomeRootDirectory(new File("."));
	
	if (errFile != null) {
	  jobmgr.setErrFileName(errFile);
	}
	
	OutcomeTh outcome = jobmgr.getOutcome(ajoId, vsiteTh);
	
	
	uStatus.setUnicoreJobStatus(outcome);
	uStatus.exitCode = jobmgr.getExitCode();
	uStatus.exitSignal = jobmgr.getExitSignal();
	uStatus.createUnicoreLog(outcome.getOutcome());
	
	Log.log("                      ... got it.");
	return uStatus;
  }

  static void consignAsyncWithRetry(AbstractJob ajo, VsiteTh vsite_th, 
							 PortfolioTh[] importPortfolioThs,
							 int counter, int interval)
	   throws Connection.Exception, JobManager.Exception {
		 Connection.Exception ce = null;
		 JobManager.Exception je = null;
		 
		 while (counter-- > 0) {
		   try {
			 if (importPortfolioThs == null) {
			   JobManager.consignAsynchronous(ajo, vsite_th, false);
			 } else {
			   JobManager.consignAsynchronous(ajo,
											  importPortfolioThs, 
											  vsite_th, false);
			 }
			 return;
			 
		   } catch (Connection.Exception e) {
			 ce = e;
		   } catch (JobManager.Exception e) {
			 je = e;
		   }
		   Log.log("failed to consign, retry.. ");
		   try {
			 Thread.sleep(interval * 1000);
		   } catch (InterruptedException e) {
		   }
		 }
		 Log.log("give up");
		 if (ce != null)  throw ce;
		 if (je != null)   throw je;
  }

  /**
   * @param env
   * @return
   */
  static public OptionSet makeEnv(ArrayList<String> envs) {
	OptionSet ops = new OptionSet();
	Option op = null;
	if (envs != null && envs.size() >= 1) {
	  String[] environment = null;
	  
	  for (String env : envs) {
		environment = env.split("=");
		op = new Option(environment[0]);
		op.setValue(environment[1]);
		ops.add(op);
	  }
	  return ops;
	} else {
	  return null;
	}
  }
}









