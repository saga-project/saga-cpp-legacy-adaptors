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
 * $RCSfile: ProtTransferCallbackResultReply.java,v $ $Revision: 1.23 $ $Date: 2005/10/03 02:13:24 $
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

public class ProtTransferCallbackResultReply extends Protocol {
	public final static String PROTO_STR = "ProtTransferCallbackResultReply";
	public final static int PROTOCOL_KIND = 1;
	public final static int PROTOCOL_TYPE = 0x34;
	public final static String PROTOCOL_TYPE_XML_STR = "transferCallbackResultData";

	private static String tagCallback =
		ProtTransferCallbackArgumentRequest.tagCallback;
	private static String attrCallbackID =
		ProtTransferCallbackArgumentRequest.attrCallbackID;
	private static String attrSequenceID =
		ProtTransferCallbackArgumentRequest.attrCallbackSeq;

	private int callbackID;
	private int callbackSeq;
	
	/**
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param result
	 */
	public ProtTransferCallbackResultReply(int sequenceNum,
		int contextID, int executableID, int sessionID, int result,
		int callbackID, int callbackSeq,
		int versionMajor, int versionMinor, int versionPatch) {
		super(PROTOCOL_KIND,
			PROTOCOL_TYPE,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			result,
			versionMajor,
			versionMinor,
			versionPatch);
		this.strType = PROTOCOL_TYPE_XML_STR;
		
		this.callbackID = callbackID;
		this.callbackSeq = callbackSeq;
	}

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtTransferCallbackResultReply(NgDataInputStream ngdi)
			throws GrpcException {
		super(ngdi);
		setTypeAndKind();
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public ProtTransferCallbackResultReply(Node node)
		throws GrpcException {
		super(node);
		setTypeAndKind();
		
		parseParam(node);
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#getKind()
	 */
	protected int getType() {
		return PROTOCOL_TYPE;
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
	 * @see org.apgrid.grpc.ng.Protocol#appendXMLParameter(java.lang.StringBuffer)
	 */
	protected StringBuffer appendXMLParameter(StringBuffer sb) {
		sb.append("<" + tagCallback + " " +
			attrCallbackID + "=\"" + callbackID + "\" " +
			attrSequenceID + "=\"" + callbackSeq + "\"" + "/>");
		return sb;
	}
	
	/**
	 * @return
	 */
	public int getCallbackID() {
		return callbackID;
	}
	
	/**
	 * @return
	 */
	public int getCallbackSeq() {
		return callbackSeq;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#setupParameter()
	 */
	protected void setupParameter() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);
		try {
			xo.writeInt(this.callbackID);
			xo.writeInt(this.callbackSeq);
			this.paramData = bo.toByteArray();
			bo.close();

			this.length = 8;
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * @param node
	 */
	private void parseParam(Node node) throws GrpcException {
		/* parse body */
		Node childNode = XMLUtil.getChildNode(node, tagCallback);
		String strCallbackID =
			XMLUtil.getAttributeValue(childNode, attrCallbackID);
		String strSequenceID =
			XMLUtil.getAttributeValue(childNode, attrSequenceID);
		
		this.callbackID = Integer.parseInt(strCallbackID);
		this.callbackSeq = Integer.parseInt(strSequenceID);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#parseParam(org.apgrid.grpc.ng.Wire, org.apgrid.ng.Protocol)
	 */
	protected void parseParam(Wire wire, Protocol prot) throws GrpcException {
		this.paramData = new byte[this.length];
		wire.receiveBytes(this.paramData);
		ByteArrayInputStream bi = new ByteArrayInputStream(this.paramData);
		XDRInputStream xi = new XDRInputStream(bi);

		this.callbackID = xi.readInt();
		this.callbackSeq = xi.readInt();

		try {
			bi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		/* print commLog */
		printCommLog(NgLog.COMMLOG_RECV);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#getCommLog(java.lang.String, java.lang.String, java.lang.String)
	 */
	public void printCommLog(String sendOrReceive) {
		if (this.commLog == null) {
			return;
		}
		
		StringBuffer sb = new StringBuffer();
		int offset = 0;
		
		/* result information */
		while (offset < this.length) {
			sb.append(dumpCommLog(paramData, offset, 16));
			offset += 16;			
		}
		
		printCommLog(sendOrReceive, sb.toString());
	}
}
