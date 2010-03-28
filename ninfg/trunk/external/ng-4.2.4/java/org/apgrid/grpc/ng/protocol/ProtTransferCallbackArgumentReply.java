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
 * $RCSfile: ProtTransferCallbackArgumentReply.java,v $ $Revision: 1.29 $ $Date: 2005/07/12 10:33:28 $
 */
package org.apgrid.grpc.ng.protocol;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.List;

import org.apgrid.grpc.ng.CallContext;
import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgGlobals;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgLog;
import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.info.RemoteMethodArg;
import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class ProtTransferCallbackArgumentReply extends ProtTransferResultReply {
	public final static String PROTO_STR = "ProtTransferCallbackArgumentReply";
	public final static int PROTOCOL_KIND = 1;
	public final static int PROTOCOL_TYPE = 0x33;
	public final static String PROTOCOL_TYPE_XML_STR = "transferCallbackArgumentData";

	protected final static int SIZE_OF_CALLBACKINFO = 8;

	private static String tagCallback =
		ProtTransferCallbackArgumentRequest.tagCallback;
	private static String attrCallbackID =
		ProtTransferCallbackArgumentRequest.attrCallbackID;
	private static String attrCallbackSeq =
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
	public ProtTransferCallbackArgumentReply(int sequenceNum,
		int contextID, int executableID, int sessionID, int result,
		CallContext callContext, boolean compressEnable, int compressThreshold,
		int versionMajor, int versionMinor, int versionPatch) {
		super(PROTOCOL_KIND,
			PROTOCOL_TYPE,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			result,
			callContext,
			compressEnable,
			compressThreshold,
			versionMajor,
			versionMinor,
			versionPatch);
		setTypeAndKind();
	}

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtTransferCallbackArgumentReply(NgDataInputStream ngdi)
			throws GrpcException {
		super(ngdi);
		setTypeAndKind();
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public ProtTransferCallbackArgumentReply(Node node)
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
	protected StringBuffer appendXMLParameter(StringBuffer sb) throws GrpcException {
		RemoteMethodInfo remoteMethodInfo =	callContext.getRemoteMethodInfo();
		List remoteMethodArgs = remoteMethodInfo.getArgs();

		sb.append("<" + tagCallback + " " +
			attrCallbackID + "=\"" + this.callbackID + "\" " +
			attrCallbackSeq + "=\"" + this.callbackSeq + "\"/>");

		for (int i = 0; i < remoteMethodArgs.size(); i++) {
			/* get information for argument */
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(i);
				
			/* check if Mode was OUT,INOUT */
			if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) ||
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_IN) &&
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_INOUT)) {
				continue;
			}
			
			sb = putArgumentData(sb,
				i, remoteMethodInfo, remoteMethodArg);
		}

		return sb;
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#setupParameter()
	 */
	protected void setupParameter() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);

		RemoteMethodInfo remoteMethodInfo =	callContext.getRemoteMethodInfo();
		List remoteMethodArgs = remoteMethodInfo.getArgs();
		int numParams = 0;

		/* write num of params */
		for (int i = 0; i < callContext.getNumParams(); i++) {
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(i);
			if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) ||
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_IN) ||
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)) {
				numParams++;
			} 
		}
		try {
			/* put callbackID and Sequence */
			xo.writeInt(callbackID);
			xo.writeInt(callbackSeq);

			xo.writeInt(numParams);
			bo.close();

			this.length += 12;

			/* set param */
			this.paramData = bo.toByteArray();
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		/* alloc area for head of param */
		int nArgs = remoteMethodArgs.size();
		this.paramHeader = new byte[nArgs][0];
		this.argumentData = new byte[numParams][0];

		for (int i = 0; i < nArgs; i++) {
			/* get information for argument */
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(i);
			
			/* check if Mode was OUT,INOUT */
			if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) ||
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_OUT) &&
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_INOUT)) {
				continue;
			}
			
			bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
			xo = new XDROutputStream(bo);

			/* put argument data */
			putArgumentData(xo, i, remoteMethodInfo, remoteMethodArg);

			this.paramHeader[i] = bo.toByteArray();
			this.length += this.paramHeader[i].length;
			this.length += this.argumentData[i].length;
		}
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#parseParam(org.apgrid.grpc.ng.Wire)
	 */
	protected void parseParam(Wire wire, Protocol prot) throws GrpcException {
		byte[] callbackInfo = new byte[SIZE_OF_CALLBACKINFO];
		wire.receiveBytes(callbackInfo);
		printHeaderCommLog(NgLog.COMMLOG_RECV);
		printBodyCommLog(NgLog.COMMLOG_RECV, callbackInfo);
		
		ByteArrayInputStream bi = new ByteArrayInputStream(callbackInfo);
		XDRInputStream xi = new XDRInputStream(bi);

		/* ID of callback */
		this.callbackID = xi.readInt();
		/* seq of callback */
		this.callbackSeq = xi.readInt();

		try {
			bi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		/* parseParam */
		super.parseParam(wire, prot);

		/* reset paramData */
		byte[] num_of_param = this.paramData;
		this.paramData = new byte[SIZE_OF_NUM_PARAMS + SIZE_OF_CALLBACKINFO];
		for (int i = 0; i < SIZE_OF_CALLBACKINFO; i++) {
			this.paramData[i] = callbackInfo[i];
		}
		for (int i = SIZE_OF_CALLBACKINFO; i < SIZE_OF_NUM_PARAMS + SIZE_OF_CALLBACKINFO; i++) {
			this.paramData[i] = num_of_param[i - SIZE_OF_CALLBACKINFO];
		}
		
		if (this.length != DIVIDE_DATA_LENGTH) {
			/* print commLog */
			printCommLog(NgLog.COMMLOG_RECV);
		}
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	protected void parseParam(Node node) throws GrpcException {
		Node argNode = XMLUtil.getChildNode(node, tagCallback);
		
		/* get information of Callback */
		String strCallbackID =
			XMLUtil.getAttributeValue(argNode, attrCallbackID);
		String strSequenceID =
			XMLUtil.getAttributeValue(argNode, attrCallbackSeq);
		this.callbackID = Integer.parseInt(strCallbackID);
		this.callbackSeq = Integer.parseInt(strSequenceID);
		
		/* get information of argument */
		super.parseParam(node);
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	public byte[][] getArgumentData() throws GrpcException {
		return argumentData;
	}
	
	/**
	 * @param callContext
	 * @throws GrpcException
	 */
	public void setupCallContext(CallContext callContext) throws GrpcException {
		this.callContext = callContext;
		
		RemoteMethodInfo remoteMethodInfo = callContext.getRemoteMethodInfo();
		List remoteMethodArgs = remoteMethodInfo.getArgs();

		for (int i = 0; i < numArray.length; i++) {
			/* get information for argument and check type/mode */
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(numArray[i] - 1);
			if (typeArray[i] != remoteMethodArg.getType()) {
				throw new NgException(
					"ProtTransferCallbackArgumentReply# mismatched type");
			}
			if ((modeArray[i] != NgParamTypes.NG_MODE_IN) &&
				(modeArray[i] != NgParamTypes.NG_MODE_INOUT)) {
				throw new NgException(
					"ProtTransferCallbackArgumentReply# invalid mode");
			}

			/* set data into CallContext */
			callContext.setData(numArray[i] - 1, argumentData[i]);
		}
	}
}
