/*
 * $RCSfile: ProtocolTypes.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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

// Enumerate Protocol Ids
public abstract class ProtocolTypes {

	// Request & Reply
	public static final int queryFunctionInformation   = 0x01;
	public static final int queryExecutableInformation = 0x02;

	public static final int resetExecutable = 0x11;
	public static final int exitExecutable  = 0x12;

	public static final int invokeSession   = 0x21; 
	public static final int suspendSession  = 0x22; 
	public static final int resumeSession   = 0x23; 
	public static final int cancelSession   = 0x24; 
	public static final int pullBackSession = 0x25; 

	public static final int transferArgumentData = 0x31;
	public static final int transferResultData   = 0x32;
	public static final int transferCallbackArgumentData = 0x33;
	public static final int transferCallbackResultData   = 0x34;

	public static final int connectionClose = 0x41;

	// Notify
	public static final int iAmAlive       = 0x01;
	public static final int calculationEnd = 0x02;
	public static final int invokeCallback = 0x03;

}

