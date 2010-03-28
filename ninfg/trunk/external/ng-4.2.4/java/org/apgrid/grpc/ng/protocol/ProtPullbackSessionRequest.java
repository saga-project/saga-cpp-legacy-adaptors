/**
 * $AIST_Release: 4.2.4 $
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
 * $RCSfile: ProtPullbackSessionRequest.java,v $ $Revision: 1.13 $ $Date: 2005/06/13 05:48:06 $
 */
package org.apgrid.grpc.ng.protocol;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgLog;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.Wire;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class ProtPullbackSessionRequest extends Protocol {
	public final static String PROTO_STR = "ProtPullBackSessionRequest";
	public final static int PROTOCOL_KIND = 0;
	public final static int PROTOCOL_TYPE = 0x25;
	public final static String PROTOCOL_TYPE_XML_STR = "pullBackSession";

	/**
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 */
	public ProtPullbackSessionRequest(int sequenceNum,
		int contextID, int executableID, int sessionID,
		int versionMajor, int versionMinor, int versionPatch) {
		super(PROTOCOL_KIND,		/* Request */
			PROTOCOL_TYPE,	/* PullBackSession */
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			0,		/* result */
			versionMajor,
			versionMinor,
			versionPatch);
		this.strType = PROTOCOL_TYPE_XML_STR;		
	}

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtPullbackSessionRequest(NgDataInputStream ngdi)
			throws GrpcException {
		super(ngdi);
		setTypeAndKind();
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public ProtPullbackSessionRequest(Node node)
		throws GrpcException {
		super(node);
		setTypeAndKind();
	}
	
	/**
	 * 
	 */
	private void setTypeAndKind() {
		/* set type & kind for CancelSessionReply */
		this.kind = PROTOCOL_KIND;
		this.type = PROTOCOL_TYPE;		
		this.strType = PROTOCOL_TYPE_XML_STR;		
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#getKind()
	 */
	protected int getType() {
		return PROTOCOL_TYPE;
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#appendXMLParameter(java.lang.StringBuffer)
	 */
	protected StringBuffer appendXMLParameter(StringBuffer sb) {
		return sb;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#parseParam(org.apgrid.grpc.ng.Wire, org.apgrid.ng.Protocol)
	 */
	protected void parseParam(Wire wire, Protocol prot) throws GrpcException {
		/* print commLog */
		printCommLog(NgLog.COMMLOG_RECV);
	}
}
