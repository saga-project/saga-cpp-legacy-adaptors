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
 * $RCSfile: DestroyCommand.java,v $ $Revision: 1.2 $ $Date: 2005/11/01 07:46:58 $
 */
package org.apgrid.grpc.tools.invokeServer.unicore;
import java.io.*;


class DestroyCommand extends Command{
  String jobId;

  DestroyCommand(UnicoreInvokeServer is, String jobId){
	super(is);
	this.jobId = jobId;
  }

  void handle(){
	UnicoreJob job = is.getJob(jobId);
	
	if (job == null){  // no job in the map 
	  // anyway,  return success
	  Log.log("Warn: No known job for " + jobId + ", ignore");
	  reply("S");
	} else {  // a job is in the map
	  job.destroy(); 
	  reply("S");
	}

  }

  void dump(PrintWriter pw){
	pw.println("DestroyCommand: id = " + jobId);
  }

}
