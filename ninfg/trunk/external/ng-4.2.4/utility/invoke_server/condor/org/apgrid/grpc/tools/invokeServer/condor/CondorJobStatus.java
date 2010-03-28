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
 * $RCSfile: CondorJobStatus.java,v $ $Revision: 1.4 $ $Date: 2007/05/14 05:10:45 $
 */

package org.apgrid.grpc.tools.invokeServer.condor;

import org.apgrid.grpc.tools.invokeServer.*;

import condorAPI.*;
import condorAPI.event.*; 

import java.util.Iterator;

public class CondorJobStatus extends TargetJobStatus {
    public CondorJobStatus(Cluster cluster) throws TargetJob.Exception {

        int stat = internalJobStatus(TargetJob.DONE);
        int job_status = TargetJob.DONE;

        for (Iterator itr = cluster.iterator(); itr.hasNext(); ) {
            Job job = (Job)itr.next();
            int code = _getISJobStatus(job.getStatus());
            if (code < 0)
                throw new TargetJob.Exception("failed to map status " + job.getStatus());

            int s = internalJobStatus(code);
            if ( isSeriousStatusDetected(s, stat) ) {
                stat = s;
                job_status = code;
            }
        }
        // set TargetJobStatus's status
        jobStatus = job_status;
    }


    private boolean isSeriousStatusDetected(int newStat, int oldStat) {
        return (newStat < oldStat);
    }

    /*
     * return a status using CondorJobStatus.
     */
    private int internalJobStatus(int status) {
        switch (status) {
        case TargetJob.FAILED:
            return 0;
        case TargetJob.PENDING:
            return 1;
        case TargetJob.ACTIVE:
            return 2;
        case TargetJob.DONE:
            return 3;
        }
        return 0;
    }

    /*
     * @param cjstatus - A condor job status
     * @return A invoke server job status translated from condor job status
     */
    private int _getISJobStatus(int cjstatus) {
        if (cjstatus < Status.UNEXPANDED || 
            cjstatus > Status.SUBMISSION_ERR ) { return -1; }
        return statusMap[cjstatus];
    }

    // Condor Job status map
    static int [] statusMap =
        new int [] {TargetJob.PENDING, // Status.UNEXPANDED
                    TargetJob.PENDING, // Status.IDLE
                    TargetJob.ACTIVE,  // Status.RUNNING
                    TargetJob.DONE,    // Status.REMOVED
                    TargetJob.DONE,    // Status.COMPLETED
                    TargetJob.PENDING, // Status.HELD
                    TargetJob.FAILED   // Status.SUBMISSION_ERR
        };

}
