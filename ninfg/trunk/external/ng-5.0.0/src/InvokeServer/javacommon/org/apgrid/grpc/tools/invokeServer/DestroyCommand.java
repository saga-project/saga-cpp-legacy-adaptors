/*
 * $RCSfile: DestroyCommand.java,v $ $Revision: 1.4 $ $Date: 2008/02/07 08:17:42 $
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
package org.apgrid.grpc.tools.invokeServer;
import java.io.*;


class DestroyCommand extends Command{
    String jobId;

    DestroyCommand(InvokeServer is, String jobId){
        super(is);
        this.jobId = jobId;
    }

    void handle(){
        TargetJob job = is.getJob(jobId);
    
        if (job == null){  // no job in the map 
            // anyway,  return success
            Log.log(Log.ALWAYS,
                 "Warn: No known job for " + jobId + ", ignore");
            reply("S");
        } else {  // a job is in the map
            job.destroy(); 
            reply("S");
        }
    }

    public String toString() {
        return "DestroyCommand: id = " + jobId;
    }

    void dump(PrintWriter pw){
        pw.println("DestroyCommand: id = " + jobId);
    }

}
