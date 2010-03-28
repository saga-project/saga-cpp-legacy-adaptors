/*
 * $RCSfile: ProtTransferCallbackResultReply.java,v $ $Revision: 1.9 $ $Date: 2007/09/26 04:14:08 $
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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.Wire;
import org.gridforum.gridrpc.GrpcException;

public final class ProtTransferCallbackResultReply
extends AbstractReceivableProtocol
implements Protocol {

	private final static String NAME = "TransferCallbackResultReply";
	private final static int KIND = 1;
	private final static int TYPE = 0x34;

	private int callbackID;
	private int cut_off;
	
	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtTransferCallbackResultReply(NgDataInputStream ngdi)
	 throws GrpcException {
		super(KIND, TYPE, ngdi);
	}
	
	public String getName() { return NAME; }

	public int getCallbackID() { return callbackID; }
	public int getCutOff() { return cut_off; }

	/*
	 */
	public void parseParam(Wire wire, NgProtocolRequest prot)
	 throws GrpcException {

		this.paramData = new byte[this.header.getParamLength()];
		wire.receiveBytes(this.paramData);
		wire.logCommLog(this, "Callback Info",
			this.paramData, this.paramData.length);

		XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(this.paramData));

		this.callbackID = xi.readInt();
		//this.cut_off    = xi.readInt();

		try {
			xi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

}

