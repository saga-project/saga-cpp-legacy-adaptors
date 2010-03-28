/*
 * $RCSfile: NotifyThreadManager.java,v $ $Revision: 1.3 $ $Date: 2008/03/28 04:11:49 $
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

import java.io.IOException;

/*
 * Thread manager for external module notify.
 */
class NotifyThreadManager implements Runnable {

	private ExtModule extModule = null;
	private NgLog log = null;

	/*
	 * Constructor
	 */
	public NotifyThreadManager() { }

	
	/**
	 * Sets the ExtModule object.
	 *
	 * @param a object of ExtModule.
	 */
	public void setExtModule(ExtModule extModule) {
		this.extModule = extModule;
	}

	/**
	 * Sets the NgLog object.
	 *
	 * @param log a NgLog
	 */
	public void setNgLog(NgLog log) {
		this.log = log;
	}

	/**
	 * Starts notify thread.
	 *
	 * @throws NullPointerException if valid ExtModule is not set.
	 */
	public void startThread() {
		if (extModule == null)
			throw new NullPointerException("Does not set ExtModule");

		(new Thread(this)).start();
	}

	public void run() {
		log.logInfo(NgLog.CAT_NG_INTERNAL, "Notify thread start");
		for (;;) {
			ExtModuleNotify extNotify = null;
			try {
				extNotify = extModule.createNotify();
			} catch (IOException e) {
				log.logError(NgLog.CAT_NG_INTERNAL, e);
				break;
			}

			if (extNotify == null) {
				log.logInfo(NgLog.CAT_NG_INTERNAL, "Can not get notify.");
				break;
			}
			log.logDebug(NgLog.CAT_NG_INTERNAL, "Call notify handle.");
			extNotify.handle();
		}
		log.logInfo(NgLog.CAT_NG_INTERNAL, "Notify thread end");
	}

}

