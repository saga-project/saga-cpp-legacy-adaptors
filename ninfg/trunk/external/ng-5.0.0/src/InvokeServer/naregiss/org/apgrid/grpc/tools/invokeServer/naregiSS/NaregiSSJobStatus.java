/*
 * $RCSfile: NaregiSSJobStatus.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:05 $
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
package org.apgrid.grpc.tools.invokeServer.naregiSS;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.StringWriter;
import org.apgrid.grpc.tools.invokeServer.*;

import org.w3c.dom.*;
import java.util.*;

public class NaregiSSJobStatus extends TargetJobStatus{
	String naregiStatusString;

	public NaregiSSJobStatus(int stat) {
        jobStatus = stat;
    }

	public NaregiSSJobStatus(Document stat) throws TargetJob.Exception {
		String stateString = Driver.getStateString(stat);
		if (stateString == null)
			throw new TargetJob.Exception("failed to get status from Status XML");
		Integer code = (Integer)statusMap.get(stateString);
		if (code == null)
			throw new TargetJob.Exception("failed to map status " + stateString);
		jobStatus = code.intValue();
		naregiStatusString = stateString;
	}

	//  static final int PENDING = 0;
	//  static final int ACTIVE  = 1;
	//  static final int DONE    = 2;
	//  static final int FAILED  = 3;
	  
	static Map statusMap = new HashMap();
	static {
		statusMap.put("New"          , new Integer(TargetJob.PENDING));
		statusMap.put("Pending"      , new Integer(TargetJob.PENDING));

		statusMap.put("StagingIn"    , new Integer(TargetJob.ACTIVE));
		statusMap.put("ExecutionPending" , new Integer(TargetJob.ACTIVE));
		statusMap.put("Running"      , new Integer(TargetJob.ACTIVE));
		statusMap.put("Suspended"    , new Integer(TargetJob.ACTIVE));
		statusMap.put("ExecutionComplete", new Integer(TargetJob.ACTIVE));
		statusMap.put("StagingOut"   , new Integer(TargetJob.ACTIVE));
		statusMap.put("Complete"     , new Integer(TargetJob.ACTIVE));
		statusMap.put("CleaningUp"   , new Integer(TargetJob.ACTIVE));
		statusMap.put("ShuttingDown" , new Integer(TargetJob.ACTIVE));

		statusMap.put("Terminated"   , new Integer(TargetJob.DONE));
		statusMap.put("Done"         , new Integer(TargetJob.DONE));

		statusMap.put("Exception"    , new Integer(TargetJob.FAILED));
		statusMap.put("Other"        , new Integer(TargetJob.FAILED));
		statusMap.put("NotKnown"     , new Integer(TargetJob.FAILED));
	}
  
}
