/*
 * $RCSfile: CondorJobStatus.java,v $ $Revision: 1.4 $ $Date: 2008/03/17 07:06:35 $
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
            int code = _getStatus(job);
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

    private int _getStatus(Job job) {
 	if (job.isCompleted()) {
		return TargetJob.DONE;
	}
	if (job.isRemoved()) {
		return TargetJob.DONE;
	}

	if (job.isRunning()) {
		return TargetJob.ACTIVE;
	}

	if (job.isHeld()) {
		return TargetJob.PENDING;
	}
	if (job.isIdle()) {
		return TargetJob.PENDING;
	}

	return TargetJob.FAILED;
    }
}
