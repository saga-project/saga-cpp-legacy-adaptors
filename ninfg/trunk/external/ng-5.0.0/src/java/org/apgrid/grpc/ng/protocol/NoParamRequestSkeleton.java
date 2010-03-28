/*
 * $RCSfile: NoParamRequestSkeleton.java,v $ $Revision: 1.1 $ $Date: 2008/02/07 08:17:43 $
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

import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.Version;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.NgProtocolRequest.NO_RESULT;
import static org.apgrid.grpc.ng.NgProtocolRequest.NO_PARAMETER;

/*
 * Skeleton-implementation for Request that It has no parameter.
 */
abstract class NoParamRequestSkeleton
implements NgProtocolRequest {

	private ProtocolHeader header;
	private Version version;

	protected NoParamRequestSkeleton(int type, int id,
	 int sequenceNum, int contextID, int executableID, int sessionID,
	 Version protVersion) {
		this.header = new ProtocolHeader(
			type, id,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			NO_RESULT,
			NO_PARAMETER);

		this.version = protVersion;
	}

	public void send(Wire wire) throws GrpcException {
		byte [] prot_header = header.toByteArray();
		wire.sendBytes(prot_header);
		wire.logCommLog(this, "Protocol Header",
			prot_header, prot_header.length);
	}

	public String toString() {
		return this.header.toString();
	}

}

