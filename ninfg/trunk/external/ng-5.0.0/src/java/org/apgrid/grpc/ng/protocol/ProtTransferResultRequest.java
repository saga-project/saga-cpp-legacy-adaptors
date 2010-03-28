/*
 * $RCSfile: ProtTransferResultRequest.java,v $ $Revision: 1.8 $ $Date: 2008/02/07 08:17:43 $
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

import org.apgrid.grpc.ng.CallContext;
import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.Version;

public final class ProtTransferResultRequest
extends NoParamRequestSkeleton
implements NgProtocolRequest {

	private final static String NAME = "TransferResultRequest";
	private final static int KIND = 0;
	private final static int TYPE = 0x32;

	private CallContext callContext;

	/**
	 */
	public ProtTransferResultRequest(int sequenceNum,
	 int contextID, int executableID, int sessionID,
	 Version protVersion,
	 CallContext callContext) {

		super(KIND, TYPE,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			protVersion);

		// set CallContext 
		this.callContext = callContext;
	}

	public int getType() {
		return TYPE;
	}

	public String getName() {
		return NAME;
	}

	/**
	 * @return
	 */
	protected CallContext getCallContext() {
		return this.callContext;
	}
}

