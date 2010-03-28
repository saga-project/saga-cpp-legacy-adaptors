/*
 * $RCSfile: GrpcTimer.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:08 $
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
package org.apgrid.grpc.util;

/**
 * Provides the functions which measure the specified time.<br>
 * You need to call start() at the start of measuring and need to call
 * getElapsedTime() at the end of measuring.
 */
public class GrpcTimer {
	/* a time of start of measuring */
	private long start_time;
	/* a time of end of measuring */
	private long end_time;
	/* a elapsed time */
	private long elapsed_time;
	
	/**
	 * Creates GrpcTimer object and starts to measure.
	 */
	public GrpcTimer() {
		start();
	}
	
	/**
	 * Gets current time.
	 * 
	 * @return current time(in milli second).
	 */
	private long getCurrentTime() {
		return System.currentTimeMillis();
	}
	
	/**
	 * Stops timer and Gets the elapsed time.
	 * 
	 * @return the elapsed time(in second).
	 */
	public double getElapsedTime() {
		stop();
		return (double)((double) elapsed_time / 1000.0);
	}
	
	/**
	 * Resets a timer and starts to measure.
	 */
	public void start() {
		start_time = end_time = getCurrentTime();
		elapsed_time = 0;
	}
	
	/**
	 * Stops a timer and saves elapsed time.
	 */
	private void stop() {
		end_time = getCurrentTime();
		elapsed_time = end_time - start_time;
	}
}
