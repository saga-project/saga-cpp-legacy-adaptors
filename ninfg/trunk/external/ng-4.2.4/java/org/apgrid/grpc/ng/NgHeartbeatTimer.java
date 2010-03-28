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
 * $RCSfile: NgHeartbeatTimer.java,v $ $Revision: 1.4 $ $Date: 2004/01/27 06:53:23 $
 */
package org.apgrid.grpc.ng;

import java.util.TimerTask;

class NgHeartbeatTimer extends TimerTask {
	NgGrpcClient context;
	CommunicationManager commMan;
	
	/**
	 * @param context
	 * @param commMan
	 */
	public NgHeartbeatTimer(NgGrpcClient context, CommunicationManager commMan) {
		this.context = context;
		this.commMan = commMan;
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		/* check Heartbeat */
		commMan.checkHeartbeat();
	}
}
