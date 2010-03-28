/*
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
 * $RCSfile: CondWait.java,v $ $Revision: 1.3 $ $Date: 2004/01/27 08:08:09 $
 */
package org.apgrid.grpc.util;

/**
 * Provides condition variable for Java.
 */
public class CondWait {
	/** condition variable */
	boolean cond = false;
	
	/**
	 * Sets condition variable true, and notifies it to all.
	 */
	public synchronized void set() {
		cond = true;
		notifyAll();
	}
	
	/**
	 * Waits until the condition was set true.
	 */
	public synchronized void waitFor() {
		while (!cond) {
			try {
				wait();
			} catch (InterruptedException e) {
				// nothing
			}
		}
	}
	
	/**
	 * Waits until the condition was set true or the specified time was passed.
	 * 
	 * @param timeout time to timeout.
	 * @return 0 if the condition was set true, -1 otherwise(maybe it's timeout).
	 */
	public synchronized int waitFor(long timeout) {
		try {
			wait(timeout);
		} catch (InterruptedException e) {
			// nothing
		}
		
		/* condition was set */
		if (cond == true) {
			return 0;
		} else {
			return -1;
		}
	}
}
