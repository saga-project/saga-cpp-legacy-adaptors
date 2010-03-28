/*
 * $RCSfile: AbstractReceivableProtocol.java,v $ $Revision: 1.4 $ $Date: 2008/02/07 08:17:43 $
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

import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.Version;
import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgProtocolRequest;
import org.gridforum.gridrpc.GrpcException;

/*
 * Skeleton-implementation for Reply/Notify protocol that It has no parameter.
 */
abstract class AbstractReceivableProtocol
implements Protocol {

	protected ProtocolHeader header;
	protected Version version;
	protected byte[] paramData;

	protected AbstractReceivableProtocol(int kind, int type,
	 NgDataInputStream ngdi)
	 throws GrpcException {

		int sequence     = ngdi.readInt();
		int contextID    = ngdi.readInt();
		int executableID = ngdi.readInt();
		int sessionID    = ngdi.readInt();
		int dummy        = ngdi.readInt();
		int result       = ngdi.readInt();
		int length       = ngdi.readInt();
		if (length >= 0) {
			this.paramData = new byte[length];
		}
		ngdi.readBytes(this.paramData, 0, length);

		this.header = new ProtocolHeader(
			kind, type,
			sequence,
			contextID,
			executableID,
			sessionID,
			result,
			length);
	}

	public int getType() {
		return this.header.getId();
	}

	public int getKind() {
		return this.header.getType();
	}

	public int getResult() {
		return this.header.getResult();
	}

	public void parseParam(Wire wire, NgProtocolRequest prot)
	 throws GrpcException {
		// has no parameter
		byte[] data = new byte[0];
		wire.logCommLog(this, "This protocol has no parameter.", data, 0);
	}

	public void setVersion(Version version) {
		this.version = version;
	}

	public String toString() {
		return getName() + this.header.toString();
	}

}

