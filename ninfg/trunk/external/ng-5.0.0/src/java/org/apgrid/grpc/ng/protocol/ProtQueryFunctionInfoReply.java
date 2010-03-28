/*
 * $RCSfile: ProtQueryFunctionInfoReply.java,v $ $Revision: 1.10 $ $Date: 2008/02/07 08:17:43 $
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
import java.io.IOException;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;

public final class ProtQueryFunctionInfoReply
extends AbstractReceivableProtocol
implements Protocol {

	private final static String NAME = "QueryFunctionInfoReply";
	private final static int KIND = 1;
	private final static int TYPE = 0x01;

	RemoteClassInfo remoteClassInfo;

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtQueryFunctionInfoReply(NgDataInputStream ngdi)
	 throws GrpcException {
		super(KIND, TYPE, ngdi);
	}

	public String getName() { return NAME; }

	/**
	 * @return
	 */
	public RemoteClassInfo getRemoteClassInfo() {
		return remoteClassInfo;
	}

	public void parseParam(Wire wire, NgProtocolRequest prot)
	 throws GrpcException {
		this.paramData = new byte[this.header.getParamLength()];
		wire.receiveBytes(this.paramData);
		wire.logCommLog(this, "RemoteClassInfo Data",
			this.paramData, this.paramData.length);

		XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(this.paramData));
		// create information about RemoteClass and save it 
		String rcInfoStr     = xi.readString();
		this.remoteClassInfo = new RemoteClassInfo(XMLUtil.getNode(rcInfoStr));

		try {
			xi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

}

