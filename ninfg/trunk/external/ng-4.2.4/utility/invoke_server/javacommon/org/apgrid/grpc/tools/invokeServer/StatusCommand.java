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
 * $RCSfile: StatusCommand.java,v $ $Revision: 1.1 $ $Date: 2006/11/08 08:35:45 $
 */

package org.apgrid.grpc.tools.invokeServer;
import java.io.*;

class StatusCommand extends Command{
    String jobId;

    StatusCommand(InvokeServer is, String jobId){
    super(is);
        this.jobId = jobId;
    }

    void handle(){
        TargetJob job = is.getJob(jobId);
	
        if (job == null){  // no job in the map 
            reply("F No known Job for " + jobId);
        } else {
            reply("S " + job.getStatusString());
        }
    }

    public String toString() {
        return "StatusCommand: id = " + jobId;
    }

    void dump(PrintWriter pw){
        pw.println("StatusCommand: id = " + jobId);
    }

}
