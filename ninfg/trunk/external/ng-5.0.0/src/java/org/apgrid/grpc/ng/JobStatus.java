/*
 * $RCSfile: JobStatus.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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
package org.apgrid.grpc.ng;

/*
 * This class stands for Ninf-G Client Job Status.
 */
class JobStatus {
    public static final int NONE    = 0;
    public static final int PENDING = 1;
    public static final int ACTIVE  = 2;
    public static final int DONE    = 3;
    public static final int FAILED  = 4;

	final static String[] names = {
		"NONE" , "PENDING" , "ACTIVE" , "DONE" , "FAILED"
	};
    
    private int status; 
    
    public JobStatus() {
        this.status = NONE;
    }
    
    public synchronized void set(int aStatus) {
        this.status = aStatus; 
    }
    
    public synchronized int getStatus() {
        return this.status;
    }
    
    public boolean isActive() {
        return this.status == ACTIVE;
    }
    
    public boolean isDone() {
        return this.status == DONE;
    }

	public boolean isFailed() {
		return this.status == FAILED;
	}
    
	public String toString() {
		return names[this.status];
	}
}   

