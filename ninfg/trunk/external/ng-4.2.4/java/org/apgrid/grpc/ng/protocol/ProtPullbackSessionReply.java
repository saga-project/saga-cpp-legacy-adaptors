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
 * $RCSfile: ProtPullbackSessionReply.java,v $ $Revision: 1.24 $ $Date: 2005/10/03 02:13:24 $
 */
package org.apgrid.grpc.ng.protocol;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgGlobals;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgLog;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.SessionInformation;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class ProtPullbackSessionReply extends Protocol {
	public final static String PROTO_STR = "ProtPullbackSessionReply";
	public final static int PROTOCOL_KIND = 1;
	public final static int PROTOCOL_TYPE = 0x25;
	public final static String PROTOCOL_TYPE_XML_STR = "pullBackSession";

	private static String tagSessionInfo = "sessionInformation";

	private String serverSessionInfoStr;
	private SessionInformation serverSessionInfo;
	
	/**
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param result
	 */
	public ProtPullbackSessionReply(int sequenceNum,
		int contextID, int executableID, int sessionID, int result,
		SessionInformation sessionInfo,
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
		this.serverSessionInfo = sessionInfo;

		this.strType = PROTOCOL_TYPE_XML_STR;
	}

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtPullbackSessionReply(NgDataInputStream ngdi)
		throws GrpcException {
		super(ngdi);
		setTypeAndKind();
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public ProtPullbackSessionReply(Node node)
		throws GrpcException {
		super(node);
		setTypeAndKind();

		/* parse SessionInfo */
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
		sb.append(serverSessionInfo.toXMLString());
		return sb;
	}
	
	/**
	 * @return
	 */
	public SessionInformation getServerSessionInfo() {
		return serverSessionInfo;
	}
	
	/**
	 * @return
	 */
	public String getServerSessionInformationString() {
		return this.serverSessionInfoStr;
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#setupParameter()
	 */
	protected void setupParameter() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);
		xo.writeBytes(serverSessionInfo.toXMLString().getBytes());

		this.paramData = bo.toByteArray();
		try {
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
		wire.receiveBytes(this.paramData);
		ByteArrayInputStream bi = new ByteArrayInputStream(this.paramData);
		XDRInputStream xi = new XDRInputStream(bi);
		this.serverSessionInfoStr = xi.readString();
		try {
			bi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		this.serverSessionInfo = new SessionInformation(this.serverSessionInfoStr,
			versionMajor, versionMinor, versionPatch);

		/* print commLog */
		printCommLog(NgLog.COMMLOG_RECV);
	}
	
	/**
	 * @param node
	 */
	private void parseParam(Node node) throws GrpcException {
		/* parse body */
		Node childNode = XMLUtil.getChildNode(node, tagSessionInfo);
		
		this.serverSessionInfo = new SessionInformation(childNode,
			versionMajor, versionMinor, versionPatch);
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#printCommLog(java.lang.String)
	 */
	public void printCommLog(String sendOrReceive) {
		if (this.commLog == null) {
			return;
		}
		
		StringBuffer sb = new StringBuffer();
		
		int offset = 0;
		while (offset < paramData.length) {
			int length = 16;
			if (offset + 16 > paramData.length) {
				length = paramData.length - offset;
			}
			sb.append(dumpCommLog(paramData, offset, length));
			
			offset += length;
		}
		
		printCommLog(sendOrReceive, sb.toString());
	}
}
