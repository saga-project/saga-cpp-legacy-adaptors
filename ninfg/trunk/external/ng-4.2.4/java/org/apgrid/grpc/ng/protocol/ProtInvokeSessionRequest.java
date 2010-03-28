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
 * $RCSfile: ProtInvokeSessionRequest.java,v $ $Revision: 1.22 $ $Date: 2005/10/03 02:13:24 $
 */
package org.apgrid.grpc.ng.protocol;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgGlobals;
import org.apgrid.grpc.ng.NgLog;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class ProtInvokeSessionRequest extends Protocol {
	public final static String PROTO_STR = "ProtInvokeSessionRequest";
	public final static int PROTOCOL_KIND = 0;
	public final static int PROTOCOL_TYPE = 0x21;
	public final static String PROTOCOL_TYPE_XML_STR = "invokeSession";
	
	private static String tagSession = "session";
	private static String attrFunctionID = "functionName";

	private final int DEFAULT_METHOD_ID = 0; 
	private int methodID;
	
	/**
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 */
	public ProtInvokeSessionRequest(int sequenceNum, int contextID,
		int executableID, int sessionID, int methodID,
		int versionMajor, int versionMinor, int versionPatch) {
		super(PROTOCOL_KIND,		/* Request */
			PROTOCOL_TYPE,	/* InvokeSession */
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			0,		/* result */
			versionMajor,
			versionMinor,
			versionPatch);

		/* set methodID */
		this.methodID = methodID;

		this.strType = PROTOCOL_TYPE_XML_STR;		
	}

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtInvokeSessionRequest(NgDataInputStream ngdi)
		throws GrpcException {
		super(ngdi);
		setTypeAndKind();
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public ProtInvokeSessionRequest(Node node)
		throws GrpcException {
		super(node);
		setTypeAndKind();
		
		/* parse parameter */
		parseParam(node);
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
		sb.append("<" + tagSession + " " +
			attrFunctionID + "=\"" + methodID + "\"" + "/>");
		return sb;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#setupParameter()
	 */
	protected void setupParameter() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);
		try {
			/* set method ID */
			xo.writeInt(methodID);
			this.paramData = bo.toByteArray();
			bo.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#parseParam(org.apgrid.grpc.ng.Wire, org.apgrid.ng.Protocol)
	 */
	protected void parseParam(Wire wire, Protocol prot) throws GrpcException {
		this.paramData = new byte[this.length];
		wire.receiveBytes(this.paramData, 0, this.length);
		ByteArrayInputStream bi = new ByteArrayInputStream(this.paramData);
		XDRInputStream xi = new XDRInputStream(bi);

		this.methodID = xi.readInt();

		try {
			bi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		/* print commLog */
		printCommLog(NgLog.COMMLOG_RECV);
	}
	
	/**
	 * @param node
	 */
	private void parseParam(Node node) throws GrpcException {
		/* parse body */
		Node childNode = XMLUtil.getChildNode(node, tagSession);
		String strMethodID =
			XMLUtil.getAttributeValue(childNode, attrFunctionID);
		
		this.methodID = Integer.parseInt(strMethodID);
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#printCommLog(java.lang.String)
	 */
	public void printCommLog(String sendOrReceive) {
		if (this.commLog == null) {
			return;
		}
		
		printCommLog(sendOrReceive, dumpCommLog(paramData, 0, 4));
	}
}
