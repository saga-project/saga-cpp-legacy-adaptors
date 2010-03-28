/*
 * $RCSfile: NgHeartbeatTimer.java,v $ $Revision: 1.6 $ $Date: 2008/03/12 11:27:16 $
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

import java.util.Timer;
import java.util.TimerTask;

class NgHeartbeatTimer {

	private Timer hbTimer;
	private HeartbeatHandler handler;
	private long lastUpdateTime;
	private int hbTimeoutTime;
	private int hbInterval;
	private boolean isScheduled;

	private static class Checker extends TimerTask {
		private NgHeartbeatTimer hbTimer;
		public Checker(NgHeartbeatTimer timer) {
			this.hbTimer = timer;
		}
		public void run() {
			if (this.hbTimer.isTimeout())
				this.hbTimer.timeout();
		}
	}

	public NgHeartbeatTimer(HeartbeatInfo hbInfo, HeartbeatHandler handle) {
		if ((hbInfo == null) || (handle == null))
			throw new NullPointerException();

		this.hbInterval = hbInfo.getHeartbeat();
		if (this.hbInterval < 0)
			throw new IllegalArgumentException("");

		this.hbTimeoutTime =
			this.hbInterval * hbInfo.getHeartbeatTimeoutCount();

		this.hbTimer = new Timer();
		this.handler = handle;
		this.isScheduled = false;
	}

	synchronized public void start() {
		if (hbInterval == 0) return;
		if (isScheduled) return;
		touch();
		this.hbTimer.schedule(new Checker(this), 1000, hbInterval * 1000);
		isScheduled = true;
	}

	synchronized public void stop() {
		if (!isScheduled) {
			return;
		}
		this.hbTimer.cancel();
		isScheduled = false;
	}

	synchronized public void touch() {
		this.lastUpdateTime = currentTime();
	}

	synchronized boolean isTimeout() {
		return (currentTime() - this.lastUpdateTime) > this.hbTimeoutTime;
	}

	private void timeout() {
		this.handler.timeout();
	}

	private long currentTime() {
		return System.currentTimeMillis() / 1000;
	}

}
