/*
 * $RCSfile: ProtocolFactory.java,v $ $Revision: 1.5 $ $Date: 2007/09/26 04:14:08 $
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
package org.apgrid.grpc.ng.protocol;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.NgException;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.ProtocolTypes.*;

public final class ProtocolFactory {

	private static final int TYPE_REPLY   = 1;
	private static final int TYPE_NOTIFY  = 2;

	private ProtocolFactory() {
	}

	/*
	 *  Protocol create method
	 */
	public static Protocol create(NgDataInputStream di)
	 throws GrpcException {
		int data = di.readInt();
		int type = data >> 24;
		int id   = data & 0x0FFF;
		if ( isReply(type) ) {
			return createReply(id, di);
		} else if ( isNotify(type) ) {
			return createNotify(id, di);
		}
		throw new NgException("Unknown Protocol Type[" + type + "] Id["+ id +"]");
	}

	private static boolean isReply(int aType) {
		return aType == TYPE_REPLY;
	}

	private static boolean isNotify(int aType) {
		return aType == TYPE_NOTIFY;
	}

	// create Reply method
	private static Protocol createReply(int id, NgDataInputStream di) 
	 throws GrpcException {
		switch (id) {
		case queryFunctionInformation:
			return new ProtQueryFunctionInfoReply(di);
		case queryExecutableInformation:
			return new ProtQueryExeStatusReply(di);
		case resetExecutable:
			return new ProtResetExeReply(di);
		case exitExecutable:
			return new ProtExitExeReply(di);
		case invokeSession:
			return new ProtInvokeSessionReply(di);
		case suspendSession:
			return new ProtSuspendSessionReply(di);
		case resumeSession:
			return new ProtResumeSessionReply(di);
		case cancelSession:
			return new ProtCancelSessionReply(di);
		case pullBackSession:
			return new ProtPullbackSessionReply(di);
		case transferArgumentData:
			return new ProtTransferArgumentReply(di);
		case transferResultData:
			return new ProtTransferResultReply(di);
		case transferCallbackArgumentData:
			return new ProtTransferCallbackArgumentReply(di);
		case transferCallbackResultData:
			return new ProtTransferCallbackResultReply(di);
		case connectionClose:
			return new ProtConnectionCloseReply(di);
		}
		throw new NgException("Unknown Protocol ID " + id);
	}

	// create Notify method
	private static Protocol createNotify(int id, NgDataInputStream di) 
	 throws GrpcException {
		switch (id) {
		case iAmAlive:
			return new ProtIAmAliveNotify(di);
		case calculationEnd:
			return new ProtFunctionCompleteNotify(di);
		case invokeCallback:
			return new ProtInvokeCallbackNotify(di);
		}
		throw new NgException("Unknown Protocol ID " + id);
	}

}

