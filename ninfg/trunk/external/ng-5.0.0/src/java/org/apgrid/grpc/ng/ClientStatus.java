/*
 * $RCSfile: ClientStatus.java,v $ $Revision: 1.5 $ $Date: 2008/02/07 08:17:43 $
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
 * This class stands for Ninf-G Client Status.
 */
class ClientStatus {

	// status of Ninf-G Client
	public static final int NONE     = 0x00; // not available
	public static final int IDLE     = 0x01; // idling
	public static final int INIT     = 0x02; // initializing handle 
	public static final int INVOKE_SESSION = 0x03; // invoking session
	public static final int TRANSARG = 0x05; // sending arguments
	public static final int WAIT     = 0x07; // waiting for function complete
	public static final int COMPLETE_CALCULATING = 0x08; // received notify of function complete
	public static final int TRANSRES = 0x09; // transforming results
	public static final int PULLBACK = 0x0b; // pulling back session
	public static final int SUSPEND  = 0x0c; // suspending session
	public static final int RESUME   = 0x0e; // resuming session
	public static final int DISPOSE  = 0x0f; // disposing handle
	public static final int RESET    = 0x10; // resetting handle
	public static final int CANCEL   = 0x11; // canceling session
	public static final int INVOKE_CALLBACK = 0x12; // executing callback


	final static String[] names = {
		"NONE" , 
		"IDLE" , 
		"INIT" , 
		"INVOKE_SESSION" , 
		"" , 
		"TRANSARG" , 
		"" , 
		"WAIT" , 
		"COMPLETE_CALCULATING" , 
		"TRANSRES" , 
		"" , 
		"PULLBACK" , 
		"SUSPEND" , 
		"" , 
		"RESUME" , 
		"DISPOSE" , 
		"RESET" , 
		"CANCEL" , 
		"INVOKE_CALLBACK"
	};

	private int status;

	public ClientStatus() {
		this.status = NONE;
	}

	public synchronized int get() {
		return status;
	}

	public synchronized void set(int status) {
		this.status = status;
	}

	public String toString() {
		return names[status];
	}

}

