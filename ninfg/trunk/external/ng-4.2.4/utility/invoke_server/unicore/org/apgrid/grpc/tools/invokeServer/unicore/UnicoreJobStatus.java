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
 * $RCSfile: UnicoreJobStatus.java,v $ $Revision: 1.2 $ $Date: 2005/11/01 07:46:58 $
 */
package org.apgrid.grpc.tools.invokeServer.unicore;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.StringWriter;

import org.unicore.outcome.ActionGroup_Outcome;
import org.unicore.outcome.ConditionalAction_Outcome;
import org.unicore.outcome.Outcome;
import org.unicore.sets.OutcomeEnumeration;

import com.fujitsu.arcon.servlet.OutcomeTh;


/**
 * This class provides a Manage UNICORE and Condor job related information.
 * 
 * @author Yasuyoshi ITOU (Fujitsu Limited)
 * @version $Revision: 1.2 $ $Date: 2005/11/01 07:46:58 $ 
 * 
 */
public class UnicoreJobStatus {

  int jobStatus;
  
  int exitCode;
  
  int exitSignal;
  
  float byteSend;
  
  float byteRecvd;
  
  boolean exitBySignal;
  
  float remoteWallClockTime;
  
  String unicoreJobStatus;
  
  String unicoreJobId;
  
  String errMessage;

  OutcomeTh outcome;
  
  /**
   * Construct a default JobStatusInfo.
   */
  public UnicoreJobStatus() {
  }


  
  /**
   * @param outcome
   */
  public void setUnicoreJobStatus(OutcomeTh outcome) {
	try {
	  unicoreJobStatus = outcome.getOutcome().getStatus().toString();
	} catch (NullPointerException e) {
	  Log.log("** Can't get status because outcome is null!");
	}
  }
  
  /**
   *  return job status code defined in the UnicoreJob
   */
  public int getJobStatus() {
	
	if (unicoreJobStatus == null) {
	  return UnicoreJob.PENDING;
	} else if (unicoreJobStatus.equals("CONSIGNED") ||
			   unicoreJobStatus.equals("PENDING")||
			   unicoreJobStatus.equals("READY")||
			   unicoreJobStatus.equals("QUEUED")||
			   unicoreJobStatus.equals("SUSPENDED")) {
	  return UnicoreJob.PENDING;
	} else if (unicoreJobStatus.equals("RUNNING")||
			   unicoreJobStatus.equals("EXECUTING")) {
	  return UnicoreJob.ACTIVE;
	} else if (unicoreJobStatus.equals("SUCCESSFUL")||
			   unicoreJobStatus.equals("KILLED")) {
	  return UnicoreJob.DONE;
	} else if (unicoreJobStatus.equals("NOT_SUCCESSFUL") ||
			   unicoreJobStatus.equals("FAILED_IN_INCARNATION")||
			   unicoreJobStatus.equals("FAILED_IN_EXECUTION")||
			   unicoreJobStatus.equals("FAILED_IN_CONSIGN")||
			   unicoreJobStatus.equals("NEVER_TAKEN")){
	  return UnicoreJob.FAILED;
	} else {
	  return UnicoreJob.PENDING;
	}
  }
  
  public void createUnicoreLog(Outcome outcome) {
	
	String st = outcome.getStatus().toString();
	
	Log.log("AbstractAction <" + outcome.getId().getName() + "> Log:\n" +
			outcome.getLog());
	
	if (outcome instanceof ActionGroup_Outcome) {
	  OutcomeEnumeration oe = ((ActionGroup_Outcome) outcome)
		.getOutcomes();
	  while (oe.hasMoreElements()) {
		createUnicoreLog(oe.nextElement());
	  }
	} else if (outcome instanceof ConditionalAction_Outcome) {
	  createUnicoreLog(((ConditionalAction_Outcome) outcome)
					   .getTrueBranchOutcome());
	  createUnicoreLog(((ConditionalAction_Outcome) outcome)
					   .getFalseBranchOutcome());
	}
  }
}
