/*
 * $RCSfile: ProtInvokeSessionRequest.java,v $ $Revision: 1.8 $ $Date: 2007/09/26 04:14:08 $
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

import java.io.IOException;

import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.Version;
import org.apgrid.grpc.ng.NgByteArrayOutputStream;
import org.apgrid.grpc.ng.NgException;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.NgProtocolRequest.NO_RESULT;

public final class ProtInvokeSessionRequest
implements NgProtocolRequest {

	private final static String NAME = "InvokeSessionRequest";
	private final static int KIND = 0;
	private final static int TYPE = 0x21;
	private final static int PARAM_LENGTH = 4;
	
	private ProtocolHeader header;
	private Version version;
	private int methodID;
	private byte[] paramData;

	/**
	 * @param methodID this protocol's parameter
	 */
	public ProtInvokeSessionRequest(int sequenceNum, int contextID,
	 int executableID, int sessionID,
	 Version protVersion,
	 int methodID) {

		// setup Parameter
		this.methodID = methodID;
		setupParam();

		// setup Header
		if (this.paramData == null)
			throw new IllegalStateException();

		this.header = new ProtocolHeader(KIND, TYPE,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			NO_RESULT,
			this.paramData.length);

		this.version = protVersion;
	}

	private void setupParam() {
		NgByteArrayOutputStream bos =
			new NgByteArrayOutputStream(PARAM_LENGTH);
		try {
			bos.writeInt(methodID); /// set method ID
			this.paramData = bos.toByteArray();
			bos.close();
		} catch (IOException e) {
			this.paramData = null;
		}
	}

	public int getType() {
		return TYPE;
	}

	public String getName() {
		return NAME;
	}

	public void send(Wire wire) throws GrpcException {
		if (this.paramData == null)
			throw new NgException("Parameter is null");

		wire.sendBytes(this.header.toByteArray());
		wire.sendBytes(this.paramData);
	}

}

