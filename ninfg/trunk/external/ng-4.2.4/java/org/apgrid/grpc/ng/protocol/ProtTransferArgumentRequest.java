/*
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
 * $RCSfile: ProtTransferArgumentRequest.java,v $ $Revision: 1.76 $ $Date: 2005/10/03 02:13:24 $
 */
package org.apgrid.grpc.ng.protocol;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Stack;
import java.util.Vector;

import org.apgrid.grpc.ng.CallContext;
import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgExecRemoteMethodException;
import org.apgrid.grpc.ng.NgExpression;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgLog;
import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.dataencode.NgEncodeData;
import org.apgrid.grpc.ng.dataencode.NgEncodeDataFileTypeInputStream;
import org.apgrid.grpc.ng.dataencode.NgEncodeDataRawFileInputStream;
import org.apgrid.grpc.ng.info.RemoteMethodArg;
import org.apgrid.grpc.ng.info.RemoteMethodArgSubScript;
import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.apgrid.grpc.util.GrpcTimer;
import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;
import org.apgrid.grpc.ng.NgGlobals;


public class ProtTransferArgumentRequest extends Protocol {
	public final static String PROTO_STR = "ProtTransferArgumentRequest";
	public final static int PROTOCOL_KIND = 0;
	public final static int PROTOCOL_TYPE = 0x31;
	public final static String PROTOCOL_TYPE_XML_STR = "transferArgumentData";

	protected static String tagArgument = "argument";
	protected static String tagSkipInfo = "skip_info";
	protected static String tagArgumentData = "argument_data";
	
	protected static String attrArgumentNo = "argumentNo";
	protected static String attrDataType = "dataType";
	protected static String attrModeType = "modeType";
	protected static String attrSkip = "skipEnable";
	protected static String attrRepresentation = "representType";
	protected static String attrEncodeType = "encodeType";
	protected static String attrNElements = "nElements";
	protected static String attrSkipSize = "size";
	protected static String attrSkipStart = "start";
	protected static String attrSkipEnd = "end";
	protected static String attrSkipSkip = "skip";
	
	protected static final int ENABLE_SKIP = 0x0100;
	protected static final int REPRESENT_NINF = 0x0001;
	protected static final int REPRESENT_XDR = 0x0002;
	protected static final String REPRESENT_NINF_STR = "Ninf";
	protected static final String REPRESENT_XDR_STR = "XDR";
	protected static final String COMPRESS_ZLIB_STR = "zlib";
	protected static final String CONVERT_BASE64_STR = "base64";

	protected static final int XDR_LIMIT = 4;
	protected static final int ENCODE_HEADER_LEN = 8;
	protected static final int SIZE_OF_NUM_PARAMS = 4;
	protected static final int SIZE_OF_NDIM = 4;
	protected static final int SIZE_OF_SKIP_HEADER = 16;
	protected static final int SIZE_OF_PARAM_HEADER = 24;
	
	protected static final int DIVIDE_DATA_END = 0x0000;
	protected static final int DIVIDE_DATA_CONTINUE = 0x001;
	protected static final int DIVIDE_HEADER_LENGTH = 8;
	protected static final int DIVIDE_DATA_LENGTH = -1;

	/* data for arguments */
	CallContext callContext;
	byte[][] paramHeader;
	byte[][] argumentData;
	/* list for encoders */
	List encoderList;
	/* compress */
	boolean boolCompress = false;
	int compressThreshold = 0;
	int paddNBytes = 0;
	long originalDataLength[];
	long compressedDataLength[];
	GrpcTimer timer;
	double compressRealTime[];
	double compressCPUTime[];
	
	/* receive info and data */
	int[] numArray;
	int[] typeArray;
	int[] modeArray;
	
	/* divide data */
	int blockSize = 0;
	int[] paramLength;
	boolean printCommLogWithWire = false;
	
	/**
	 * @param kind
	 * @param type
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param callContext
	 * @param encoderList
	 * @param boolCompress
	 * @param compressThreshold
	 */
	public ProtTransferArgumentRequest(
		int kind, int type, int sequenceNum,
		int contextID, int executableID, int sessionID,
		CallContext callContext, List encoderList,
		boolean boolCompress, int compressThreshold, int blockSize,
		int versionMajor, int versionMinor, int versionPatch) {
		super(kind,
			type,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			0,		/* result */
			versionMajor,
			versionMinor,
			versionPatch);

		/* set CallContext */
		this.callContext = callContext;
		/* set Encode list */
		this.encoderList = encoderList;
		/* set boolCompress */
		this.boolCompress = boolCompress;
		/* set compressThreshold */
		this.compressThreshold = compressThreshold;
		/* set divideSize */
		this.blockSize = blockSize;

		this.strType = PROTOCOL_TYPE_XML_STR;		
	}
	
	/**
	 * @param kind
	 * @param type
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param result
	 * @param versionMajor
	 * @param versionMinor
	 * @param versionPatch
	 */
	public ProtTransferArgumentRequest(int kind, int type,
		int sequenceNum, int contextID, int executableID,
		int sessionID, int result,
		int versionMajor, int versionMinor, int versionPatch) {
		super(kind,
			type,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			result,		/* result */
			versionMajor,
			versionMinor,
			versionPatch);

		this.strType = PROTOCOL_TYPE_XML_STR;		
	}

	/**
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param callContext
	 * @param encoderList
	 * @param boolCompress
	 * @param compressThreshold
	 * @param versionMajor
	 * @param versionMinor
	 * @param versionPatch
	 */
	public ProtTransferArgumentRequest(int sequenceNum,
		int contextID, int executableID, int sessionID,
		CallContext callContext, List encoderList,
		boolean boolCompress, int compressThreshold, int blockSize,
		int versionMajor, int versionMinor, int versionPatch) {
		super(PROTOCOL_KIND,		/* Request */
			PROTOCOL_TYPE,	/* TransferArgument */
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			0,		/* result */
			versionMajor,
			versionMinor,
			versionPatch);
		/* set CallContext */
		this.callContext = callContext;
		/* set Encode list */
		this.encoderList = encoderList;
		/* set boolCompress */
		this.boolCompress = boolCompress;
		/* set compressThreshold */
		this.compressThreshold = compressThreshold;
		/* set divideSize */
		this.blockSize = blockSize;

		this.strType = PROTOCOL_TYPE_XML_STR;		
	}

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtTransferArgumentRequest(NgDataInputStream ngdi)
		throws GrpcException {
		super(ngdi);
		setTypeAndKind();
		
		/* get list of encoder */
		this.encoderList = NgEncodeData.getSupportEncodeType();
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public ProtTransferArgumentRequest(Node node)
		throws GrpcException {
		super(node);
		setTypeAndKind();
		
		/* get list of encoder */
		this.encoderList = NgEncodeData.getSupportEncodeType();
		
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
	protected StringBuffer appendXMLParameter(StringBuffer sb)
		throws GrpcException {
		RemoteMethodInfo remoteMethodInfo =
			callContext.getRemoteMethodInfo();
		List remoteMethodArgs = remoteMethodInfo.getArgs();

		for (int i = 0; i < callContext.getNumParams(); i++) {
			/* get information for argument */
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(i);
				
			/* check if Mode was IN,INOUT */
			if ((remoteMethodArg.getType() != NgParamTypes.NG_TYPE_FILENAME) &&
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_IN) &&
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_INOUT)) {
					continue;
			}
			
			/* put argument */
			sb = putArgumentData(sb, i, remoteMethodInfo, remoteMethodArg);
		}
		return sb;
	}
	
	/**
	 * @param sb
	 * @param index
	 * @param remoteMethodInfo
	 * @param remoteMethodArg
	 * @return
	 * @throws GrpcException
	 */
	protected StringBuffer putArgumentData(StringBuffer sb, int index,
		RemoteMethodInfo remoteMethodInfo, RemoteMethodArg remoteMethodArg)
		throws GrpcException {
		/* start tag */
		sb.append("<" + tagArgument + " ");

		/* argument number */
		sb.append(attrArgumentNo + "=\"" + (index+1) + "\" ");
		/* type & mode */
		sb.append(attrDataType + "=\"" +
			NgParamTypes.getTypeStr(remoteMethodArg.getType()) + "\" ");
		sb.append(attrModeType + "=\"" +
			NgParamTypes.getModeStr(remoteMethodArg.getMode()) + "\" ");
		/* skip & representation */
		sb.append(attrSkip + "=\"" +
			remoteMethodInfo.getShrink() + "\" ");
		sb.append(attrRepresentation + "=\"" + REPRESENT_XDR_STR + "\" ");
				
		/* skip */
		byte[] argumentData = callContext.getData(index);
		if ((remoteMethodArg.getNDims() > 0) && (remoteMethodInfo.getShrink() == true)) {
			argumentData = encodeDataSkip(remoteMethodArg, argumentData);
		}

		/* count */
		if (NgParamTypes.getSize(remoteMethodArg.getType()) != 0) {
			sb.append(attrNElements + "=\"" +
				argumentData.length / NgParamTypes.getSize(remoteMethodArg.getType())
				+ "\">");
		} else {
			sb.append(attrNElements + "=\"" + callContext.getCount(index) + "\">");
		}

		/* add shrink info */
		if (remoteMethodInfo.getShrink() == true) {
			int nDims = remoteMethodArg.getNDims();
			for (int j = 0; j < nDims; j++) {
				RemoteMethodArgSubScript argSubscript =
					remoteMethodArg.getRemoteMethodArgSubscript(j);
				NgExpression expSize = argSubscript.getSize();
				NgExpression expStart = argSubscript.getStart();
				NgExpression expEnd = argSubscript.getEnd();
				NgExpression expSkip = argSubscript.getSkip();

				int[] intArguments = callContext.getIntArguments();
				long size = expSize.calc(intArguments);
				long start = expStart.calc(intArguments);
				long end = expEnd.calc(intArguments);
				if (end == 0) {
					end = expSize.calc(intArguments);
				}
				long skip = expSkip.calc(intArguments);
				if (skip == 0) {
					skip = 1;
				}

				sb.append("<" + tagSkipInfo + " ");
				sb.append(attrSkipSize + "=\"" + size + "\" ");
				sb.append(attrSkipStart + "=\"" + start + "\" ");
				sb.append(attrSkipEnd + "=\"" + end + "\" ");
				sb.append(attrSkipSkip + "=\"" + skip + "\"");
				sb.append("/>");
			}
		}
			
		/* convert data (XDR -> (compress) -> base64) */
		argumentData = encodeDataXDR(remoteMethodArg, argumentData);
		/* compress */
		if (boolCompress && (argumentData.length > compressThreshold)) {
			sb.append("<" + tagArgumentData + " " +
				attrEncodeType + "=\"" + COMPRESS_ZLIB_STR + "\">");
			argumentData = encodeData(
				NgEncodeData.NG_COMPRESS_ZLIB, argumentData, null);
		}
		/* base64 */
		sb.append("<" + tagArgumentData + " " +
			attrEncodeType + "=\"" + CONVERT_BASE64_STR + "\">");
		argumentData = encodeData(
			NgEncodeData.NG_DATA_CONVERT_BASE64, argumentData, null);
		/* append encoded data */
		sb.append(new String(argumentData));
		sb.append("</" + tagArgumentData + ">");
				
		if (boolCompress && (argumentData.length > compressThreshold)) {
			sb.append("</" + tagArgumentData + ">");
		}
			
		/* end tag */
		sb.append("</" + tagArgument + ">");

		return sb;
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#setupParameter()
	 */
	protected void setupParameter() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);
		RemoteMethodInfo remoteMethodInfo = callContext.getRemoteMethodInfo();
		try {
			int numParams = 0;
			List remoteMethodArgs = remoteMethodInfo.getArgs();
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
			xo.writeInt(numParams);
			this.paramData = bo.toByteArray();
			bo.close();
			this.length += 4;

			/* alloc area for head of param */
			int nArgs = callContext.getNumParams();
			this.paramHeader = new byte[nArgs][0];
			this.argumentData = new byte[nArgs][0];
			this.paramLength = new int[nArgs];
			this.originalDataLength = new long[nArgs];
			this.compressedDataLength = new long[nArgs];
			this.timer = new GrpcTimer();
			this.compressRealTime = new double[nArgs];
			this.compressCPUTime = new double[nArgs];
			for (int i = 0; i < nArgs; i++) {
				this.originalDataLength[i] = -1;
				this.compressedDataLength[i] = -1;
				this.compressRealTime[i] = -1;
				this.compressCPUTime[i] = -1;
			}

			for (int i = 0; i < nArgs; i++) {
				/* get information for argument */
				RemoteMethodArg remoteMethodArg =
					(RemoteMethodArg) remoteMethodArgs.get(i);
				
				/* check if Mode was IN,INOUT */
				if ((remoteMethodArg.getType() != NgParamTypes.NG_TYPE_FILENAME) &&
					(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_IN) &&
					(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_INOUT)) {
					continue;
				}					
				
				/* prepare for paramHeader */
				bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
				xo = new XDROutputStream(bo);

				/* put argument data */
				putArgumentData(xo, i, remoteMethodInfo, remoteMethodArg);

				this.paramHeader[i] = bo.toByteArray();
				if (this.length != DIVIDE_DATA_LENGTH) {
					this.length += this.paramHeader[i].length;
				}
				bo.close();
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @param xo
	 * @param index
	 * @param remoteMethodInfo
	 * @param remoteMethodArg
	 * @throws GrpcException
	 */
	protected void putArgumentData(XDROutputStream xo, int index,
		RemoteMethodInfo remoteMethodInfo, 
		RemoteMethodArg remoteMethodArg) throws GrpcException {
		/* argument number */
		try {
			xo.writeInt(index+1);
			/* type & mode */
			xo.writeInt(remoteMethodArg.getType());
			xo.writeInt(remoteMethodArg.getMode());
			/* encode type */
			int encodeType = REPRESENT_XDR;
			byte[] argumentData = callContext.getData(index);
			/* skip */
			if ((remoteMethodInfo.getShrink()) && (remoteMethodArg.getNDims() > 0)) {
				encodeType |= ENABLE_SKIP;
				if (remoteMethodArg.getType() != NgParamTypes.NG_TYPE_STRING ) {
					argumentData = encodeDataSkip(remoteMethodArg, argumentData);
				}
			}
			xo.writeInt(encodeType);
				
			/* count */
			int count;
			if (NgParamTypes.getSize(remoteMethodArg.getType()) != 0) {
				count = (argumentData.length - ENCODE_HEADER_LEN) / NgParamTypes.getSize(remoteMethodArg.getType());
			} else {
				if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) &&
					(callContext.getCount(index) == 0) &&
					(callContext.getArgs().get(index) == null) &&
					(versionMinor > 1)) {
					count = 1;
				} else {
					count = callContext.getCount(index);
				}
			}
			xo.writeInt(count);
				
			/* convert data (RAW -> compress) */
			/* protocol 2.0 or 2.1 ? */
			if (versionMinor > 0) {
				/* RAW */
				for (int i = argumentData.length - 1; i > 7; i--) {
					argumentData[i] = argumentData[i-ENCODE_HEADER_LEN];
				}
				argumentData[0] = 0;
				argumentData[1] = 0;
				argumentData[2] = 0;
				argumentData[3] = NgEncodeData.NG_DATA_REPRESENT_RAW;
				int argLength = argumentData.length - ENCODE_HEADER_LEN;
				argumentData[4] = (byte)((argLength >>> 24) & 0xFF);
				argumentData[5] = (byte)((argLength >>> 16) & 0xFF);
				argumentData[6] = (byte)((argLength >>>  8) & 0xFF);
				argumentData[7] = (byte)((argLength >>>  0) & 0xFF);
				
				/* length of data */
				int argumentDataLength = argumentData.length;
				
				/* check if it's able to divide data */
				if ((this.blockSize != 0) &&
					encoderList.contains(new Integer(NgEncodeData.NG_DATA_DIVIDE))) {
					/* set length to DIVIDE_DATA_LENGTH */
					argumentDataLength = DIVIDE_DATA_LENGTH;
					this.length = DIVIDE_DATA_LENGTH;
					
					/* don't need to print commLog at the end of this section */
					printCommLogWithWire = true;
				} else if ((this.versionMinor > 1) &&
					(remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) &&
					(this.blockSize == 0)) {
					
					/* transfer data of file via Ninf-G Protocol */
					long inputFileLength = ENCODE_HEADER_LEN;
					List args = callContext.getArgs();
					int loopCount = 1;
					if (remoteMethodArg.getNDims() > 0) {
							loopCount = callContext.getCount(index);
					}
					
					/* get length of file(s) */
					for (int i = 0; i < loopCount; i++) {
						String inputFile = null;
						if (remoteMethodArg.getNDims() > 0) {
							String[] fileArray = (String []) args.get(index);
							inputFile = fileArray[i];
						} else {
							inputFile = (String) args.get(index);
						}
						
						/* append length of file */
						if (inputFile == null || inputFile.equals("")) {
							inputFileLength += ENCODE_HEADER_LEN;
						} else {
							if ((remoteMethodArg.getMode() == NgParamTypes.NG_MODE_IN) ||
								(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)) {
								inputFileLength +=
									(int) new File(inputFile).length() + ENCODE_HEADER_LEN;
							} else {
								inputFileLength += ENCODE_HEADER_LEN;
							}
						}
						/* append length of padding */
						if ((inputFileLength % XDR_LIMIT) != 0) {
							int npad = (int) (XDR_LIMIT - (inputFileLength % XDR_LIMIT));
							inputFileLength += npad;
						}
					}
				
					/* set length of file length */
					if (inputFileLength > Integer.MAX_VALUE) {
						throw new NgException("can't transfer data over 2GB without division.");
					}
					argumentDataLength = (int)inputFileLength;
					
					/* don't need to print commLog at the end of this section */
					printCommLogWithWire = true;
				} else if (boolCompress && (argumentDataLength > compressThreshold)) {
					/* save original data length */
					this.originalDataLength[index] = argumentDataLength;
					
					timer.start();
					argumentData = encodeData(
						NgEncodeData.NG_COMPRESS_ZLIB, argumentData, null);
					this.compressRealTime[index] = (long) timer.getElapsedTime();
					this.compressCPUTime[index] = 0;
					argumentData = insertConvertInfo(
						NgEncodeData.NG_COMPRESS_ZLIB, argumentData);
					
					/* save compressed data length */
					this.compressedDataLength[index] =
						argumentData.length - ENCODE_HEADER_LEN;
					
					/* set length to compressed data length */
					argumentDataLength = argumentData.length;
				}
				
				/* write and set length */
				xo.writeInt(argumentDataLength);
				this.paramLength[index] = argumentDataLength;
				if (this.length != DIVIDE_DATA_LENGTH) {
					this.length += argumentDataLength;
				}
			} else {
				/* length */
				xo.writeInt(argumentData.length - ENCODE_HEADER_LEN);
				this.length += argumentData.length - ENCODE_HEADER_LEN;
				this.paramLength[index] = argumentData.length - ENCODE_HEADER_LEN;
			}
			
			/* skip info */
			if ((remoteMethodInfo.getShrink()) && (remoteMethodArg.getNDims() > 0)) {
				/* number of Dimension */
				xo.writeInt(remoteMethodArg.getNDims());
				
				int scalarArgs[] = callContext.getIntArguments();
				for (int j = 0; j < remoteMethodArg.getNDims(); j++) {
					RemoteMethodArgSubScript argScript =
						remoteMethodArg.getRemoteMethodArgSubscript(j);
					/* get skip info */
					int size = (int) argScript.getSize().calc(scalarArgs);
					int start = (int) argScript.getStart().calc(scalarArgs);
					int end = (int) argScript.getEnd().calc(scalarArgs);
					int skip = (int) argScript.getSkip().calc(scalarArgs);
					/* check end */
					if (end == 0) {
						end = size;
					}
					/* check skip */
					if (skip == 0) {
						skip = 1;
					}

					/* write skip info */
					xo.writeInt(size);
					xo.writeInt(start);
					xo.writeInt(end);
					xo.writeInt(skip);
				}
			}

			/* set data */
			this.argumentData[index] = argumentData;
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @param type
	 * @param argumentData
	 * @return
	 * @throws GrpcException
	 */
	protected byte[] insertConvertInfo(int type, byte[] argumentData) throws GrpcException {
		ByteArrayOutputStream tmpByteOut = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream tmpXDROut = new XDROutputStream(tmpByteOut);
		try {
			tmpXDROut.writeInt(type);
			tmpXDROut.writeInt(argumentData.length);
			tmpXDROut.writeBytes(argumentData);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return tmpByteOut.toByteArray();
	}

	/**
	 * @param remoteMethodArg
	 * @param buffer
	 * @return
	 * @throws GrpcException
	 */
	protected byte[] encodeDataSkip(RemoteMethodArg remoteMethodArg,
		byte buffer[]) throws GrpcException {
		
		/* check Validate of ArgSubScript */
		hasValidSubScript(callContext.getRemoteMethodInfo());

		/* make information for skip */
		int sizeArray[] = new int[1];
		sizeArray[0] = NgParamTypes.getSize(remoteMethodArg.getType());
		int arrayShapes[][] = new int[remoteMethodArg.getNDims()][4];
		int scalarArgs[] = callContext.getIntArguments();
		for (int i = 0; i < arrayShapes.length; i++) {
			RemoteMethodArgSubScript argScript =
				remoteMethodArg.getRemoteMethodArgSubscript(i);
			/* size */
			arrayShapes[i][0] = (int) argScript.getSize().calc(scalarArgs);
			/* start */
			arrayShapes[i][1] = (int) argScript.getStart().calc(scalarArgs);
			/* end */
			arrayShapes[i][2] = (int) argScript.getEnd().calc(scalarArgs);
			/* step */
			arrayShapes[i][3] = (int) argScript.getSkip().calc(scalarArgs);
		}
		Vector encodeInfo = new Vector();
		encodeInfo.add(sizeArray);
		encodeInfo.add(arrayShapes);
		
		return NgEncodeData.encode(
			NgEncodeData.NG_THIN_OUT_SKIP, buffer, encodeInfo);
	}
	
	/**
	 * @param remoteMethodArg
	 * @param buffer
	 * @return
	 * @throws GrpcException
	 */
	protected byte[] encodeDataXDR(RemoteMethodArg remoteMethodArg,
		byte buffer[]) throws GrpcException {
		
		/* encode data */
		int type = remoteMethodArg.getType();
		Vector encodeInfo = new Vector();
		encodeInfo.add(remoteMethodArg);
		if (((type == NgParamTypes.NG_TYPE_STRING) ||
			(type == NgParamTypes.NG_TYPE_FILENAME))) {
			ByteArrayInputStream bi =
				new ByteArrayInputStream(buffer);
			DataInputStream di = new DataInputStream(bi);
			ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
			XDROutputStream xo = new XDROutputStream(bo);

			while (bi.available() > 0) {
				/* read length of String */
				try {
					/* read length of String */
					int strLen = di.readInt();
					/* read string */
					byte[] strBuffer = new byte[strLen];
					di.read(strBuffer);
					/* encode */
					byte[] encodedString = NgEncodeData.encode(
						NgEncodeData.NG_DATA_REPRESENT_XDR, strBuffer, encodeInfo);
						
					/* write it to ByteArray */
					xo.writeInt(encodedString.length);
					bo.write(encodedString);
				} catch (IOException e) {
					/* failed */
					throw new NgIOException(e);
				}
			}
			/* close stream */
			try {
				bo.close();
				bi.close();
			} catch (IOException e) {
				/* failed */
				throw new NgIOException(e);
			}
			return bo.toByteArray();
		} else {
			return NgEncodeData.encode(
				NgEncodeData.NG_DATA_REPRESENT_XDR,	buffer, encodeInfo);
		}
	}
	
	/**
	 * @param type
	 * @param buffer
	 * @param encodeInfo
	 * @return
	 */
	protected byte[] encodeData(int type,
		byte[] buffer, List encodeInfo) throws GrpcException {
		if (!encoderList.contains(new Integer(type))) {
			throw new NgExecRemoteMethodException(
				"Specified encoder(" + type + ") is not supported...");
		}
		
		/* encode data */
		return NgEncodeData.encode(type, buffer, encodeInfo);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#parseParam(org.apgrid.grpc.ng.Wire, org.apgrid.ng.Protocol)
	 */
	protected void parseParam(Wire wire, Protocol prot) throws GrpcException {
		try {
			this.paramData = new byte[SIZE_OF_NUM_PARAMS];
			wire.receiveBytes(this.paramData);
			ByteArrayInputStream bi = new ByteArrayInputStream(this.paramData);
			XDRInputStream xi = new XDRInputStream(bi);
			int numParams = xi.readInt();
			bi.close();

			/* alloc area for saving info and data */
			argumentData = new byte[numParams][];
			numArray = new int[numParams];
			typeArray = new int[numParams];
			modeArray = new int[numParams];
			
			for (int i = 0; i < numParams; i++) {
				this.paramHeader[i] = new byte[SIZE_OF_PARAM_HEADER];
				wire.receiveBytes(this.paramHeader[i], 0, SIZE_OF_PARAM_HEADER);
				bi = new ByteArrayInputStream(this.paramHeader[i]);
				xi = new XDRInputStream(bi);

				/* argument number */
				this.numArray[i] = xi.readInt();
				/* type & mode */
				this.typeArray[i] = xi.readInt();
				this.modeArray[i] = xi.readInt();
				/* encode type */
				int skipAndRepresent = xi.readInt();
				/* count */
				int count = xi.readInt();
				/* length */
				int length = xi.readInt();
				bi.close();
				
				/* get encoded data */
				byte[] originalData = new byte[length];
				wire.receiveBytes(originalData, 0, length);
				
				/* decode and set data */
				if (versionMinor > 0) {
					this.argumentData[i] = decodeData(originalData, null);
				} else {
					this.argumentData[i] = originalData;
				}

				/* read padding */
				if ((length % XDR_LIMIT) != 0) {
					int nPad = XDR_LIMIT - (length % XDR_LIMIT);
					byte[] dummyBytes = new byte[nPad];
					wire.receiveBytes(dummyBytes, 0, nPad);
				}
			}
			
			/* print commLog */
			printCommLog(NgLog.COMMLOG_RECV);
		} catch (IOException e) {
			throw new NgIOException(e);
		} catch (GrpcException e) {
			throw new NgExecRemoteMethodException(e);
		}
	}

	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	protected void parseParam(Node node) throws GrpcException {
		try {
			int numParams = XMLUtil.countChildNode(node, tagArgument);
			/* alloc area for saving info and data */
			argumentData = new byte[numParams][];
			numArray = new int[numParams];
			typeArray = new int[numParams];
			modeArray = new int[numParams];
			
			for (int i = 0; i < numParams; i++) {
				Node argNode = XMLUtil.getChildNode(node, tagArgument, i);

				/* argument number */
				this.numArray[i] = Integer.parseInt(
					XMLUtil.getAttributeValue(argNode, attrArgumentNo));
				/* type & mode */
				this.typeArray[i] = NgParamTypes.getTypeVal(
					XMLUtil.getAttributeValue(argNode, attrDataType));
				this.modeArray[i] = NgParamTypes.getModeVal(
					XMLUtil.getAttributeValue(argNode, attrModeType));
				/* skip */
				String skipEnable =
					XMLUtil.getAttributeValue(argNode, attrSkip);
				/* XDR */
				String dataRepresentation =
					XMLUtil.getAttributeValue(argNode, attrRepresentation);
				/* count */
				int count = Integer.parseInt(
					XMLUtil.getAttributeValue(argNode, attrNElements));
				
				/* get skip info */
				int nDims = XMLUtil.countChildNode(argNode, tagSkipInfo);
				int arrayShape[][] = new int[nDims][4];
				for (int j = 0; j < nDims; j++) {
					Node nodeSkipInfo =
						XMLUtil.getChildNode(argNode, tagSkipInfo, j);
					/* size */
					arrayShape[j][0] = Integer.parseInt(
						XMLUtil.getAttributeValue(nodeSkipInfo, attrSkipSize));
					/* start */
					arrayShape[j][1] = Integer.parseInt(
						XMLUtil.getAttributeValue(nodeSkipInfo, attrSkipStart));
					/* end */
					arrayShape[j][2] = Integer.parseInt(
						XMLUtil.getAttributeValue(nodeSkipInfo, attrSkipEnd));
					/* skip */
					arrayShape[j][3] = Integer.parseInt(
						XMLUtil.getAttributeValue(nodeSkipInfo, attrSkipSkip));
				}

				/* get encoded param data */
				byte[] argumentData = decode(argNode, null);

				/* decode XDR */
				List encodeInfo = new Vector();
				encodeInfo.add(new Integer(typeArray[i]));
				argumentData = decodeDataXDR(argumentData, encodeInfo);
				
				/* decode skip */
				if ((new Boolean(skipEnable).booleanValue() == true) && (nDims > 0)) {
					encodeInfo = new Vector();
					int typeArray[] = new int[1];
					typeArray[0] = this.typeArray[i];
					encodeInfo.add(typeArray);
					encodeInfo.add(arrayShape);
					argumentData = decodeDataSkip(argumentData, encodeInfo);
				}
				
				/* decode and set data */
				this.argumentData[i] = argumentData;
			}
		} catch (GrpcException e) {
			throw new NgExecRemoteMethodException(e);
		}
	}

	/**
	 * @param node
	 * @param encodeInfo
	 * @return
	 * @throws GrpcException
	 */
	protected byte[] decode(Node node, List encodeInfo) throws GrpcException {
		/* get encode type */
		Stack encodeTypes = new Stack();
		Node argDataNode = node;
		while (true) {
			try {
				argDataNode = XMLUtil.getChildNode(argDataNode, tagArgumentData);
			} catch (GrpcException e) {
				/* maybe there is no more "argument_data" tag */
				break;
			}
			encodeTypes.push(XMLUtil.getAttributeValue(argDataNode, attrEncodeType));
			if (argDataNode == null) {
				break;
			}
		}
		
		/* there is no encoded data... */
		if (encodeTypes.isEmpty()) {
			throw new NgExecRemoteMethodException(
				"There is no argument/result data in protocol.");
		}
			
		/* get encoded param data */
		Node cdata = XMLUtil.getCdataNode(argDataNode);
		String dataString = XMLUtil.getNodeValue(cdata);
		int length = dataString.length();
		byte[] buffer = dataString.getBytes();
		
		/* decode data */
		while (encodeTypes.isEmpty() != true) {
			String encodeType = (String) encodeTypes.pop();
			buffer = NgEncodeData.decode(encodeType, buffer, null);
			if (encodeType.equals(NgEncodeData.NG_COMPRESS_ZLIB_STR)) {
				this.boolCompress = true;
			}
		}
		
		/* return decoded buffer */
		return buffer;
	}

	/**
	 * @param originalData
	 * @param encodeInfo
	 * @return
	 */
	protected byte[] decodeData(byte[] originalData, List encodeInfo) throws GrpcException {
		byte[] buffer = originalData;
		while (true) {
			ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
			XDRInputStream xi = new XDRInputStream(bi);
			int decodeType = xi.readInt();
			int dataLength = xi.readInt();
			byte[] targetBuffer = new byte[buffer.length - ENCODE_HEADER_LEN];
			try {
				xi.read(targetBuffer, 0, buffer.length - ENCODE_HEADER_LEN);
			} catch (IOException e) {
				throw new NgIOException(e);
			}
			
			if (decodeType == NgEncodeData.NG_COMPRESS_ZLIB) {
				this.boolCompress = true;
			}
			
			buffer = NgEncodeData.decode(decodeType, targetBuffer, encodeInfo);
			if (decodeType == NgEncodeData.NG_DATA_REPRESENT_RAW) {
				break;
			}
		}
		return buffer;
	}
	
	/**
	 * @param originalData
	 * @param encodeInfo
	 * @return
	 * @throws GrpcException
	 */
	protected byte[] decodeDataSkip(byte[] originalData, List encodeInfo)
		throws GrpcException {
		return NgEncodeData.decode(
			NgEncodeData.NG_THIN_OUT_SKIP, originalData, encodeInfo);
	}
	
	/**
	 * @param originalData
	 * @return
	 * @throws GrpcException
	 */
	protected byte[] decodeDataXDR(byte[] originalData, List encodeInfo)
		throws GrpcException {
		/* decode XDR */
		Integer typeInt = (Integer) encodeInfo.get(0);
		int type = typeInt.intValue();
		if (type == NgParamTypes.NG_TYPE_STRING) {
			ByteArrayInputStream bi = new ByteArrayInputStream(originalData);
			XDRInputStream xi = new XDRInputStream(bi);
			ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
			DataOutputStream dos = new DataOutputStream(bo);
			try {
				while (xi.available() > 0) {
					/* read length */
					int bufLen = xi.readInt();
					/* read string */
					byte[] strBuf = new byte[bufLen];
					xi.read(strBuf);
					byte[] decodedBuf = NgEncodeData.decode(
						NgEncodeData.NG_DATA_REPRESENT_XDR, strBuf, encodeInfo);
					/* write string data */
					dos.writeInt(decodedBuf.length);
					dos.write(decodedBuf);
				}
			} catch (IOException e) {
				throw new NgIOException(e);
			}
			
			/* close stream */
			try {
				bi.close();
				bo.close();
			} catch (IOException e1) {
				throw new NgIOException(e1);
			}
			return bo.toByteArray();
		} else {
			return NgEncodeData.decode(
				NgEncodeData.NG_DATA_REPRESENT_XDR, originalData, encodeInfo);
		}
	}

	/**
	 * @param callContext
	 * @throws GrpcException
	 */
	public void setupCallContext(CallContext callContext) throws GrpcException {
		this.callContext = callContext;
		
		RemoteMethodInfo remoteMethodInfo =	callContext.getRemoteMethodInfo();
		List remoteMethodArgs = remoteMethodInfo.getArgs();

		for (int i = 0; i < numArray.length; i++) {
			/* get information for argument and check type/mode */
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(numArray[i] - 1);
			if (typeArray[i] != remoteMethodArg.getType()) {
				throw new NgException(
					"ProtTransferArgumentRequest# mismatched type");
			}
			if ((modeArray[i] != NgParamTypes.NG_MODE_IN) &&
				(modeArray[i] != NgParamTypes.NG_MODE_INOUT)) {
				throw new NgException(
					"ProtTransferArgumentRequest# invalid mode");
			}

			/* set data into CallContext */
			callContext.setData(numArray[i] - 1, argumentData[i]);
		}
	}
	
	/**
	 * @param remoteMethodInfo
	 * @return
	 * @throws GrpcException
	 */
	private void hasValidSubScript(RemoteMethodInfo remoteMethodInfo)
		throws GrpcException {
		int[] scalarArgs = callContext.getIntArguments();
		List listArguments = remoteMethodInfo.getArgs();
		
		/* look up all of params */
		for (int i = 0; i < listArguments.size(); i++) {
			RemoteMethodArg remoteMethodArg = (RemoteMethodArg) listArguments.get(i);
			/* look up all of dims */
			for (int j = 0; j < remoteMethodArg.getNDims(); j++) {
				RemoteMethodArgSubScript argSubscript =
					remoteMethodArg.getRemoteMethodArgSubscript(j);
				
				/* get variable of skip */
				int size = (int) argSubscript.getSize().calc(scalarArgs);
				int start = (int) argSubscript.getStart().calc(scalarArgs);
				int end = (int) argSubscript.getEnd().calc(scalarArgs);
				int skip = (int) argSubscript.getSkip().calc(scalarArgs);
				
				/* check if start,end > size */
				if ((start != 0) && (start > size)) {
					throw new NgException("Invalid subscript: start(" +
						start + ") > size(" + size + ").");
				} else if ((end != 0) && (end > size)) {
					throw new NgException("Invalid subscript: end(" +
						end + ") > size(" + size + ").");
				}
			}
		}
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
		
		/* argument information */
		while (offset < paramData.length) {
			sb.append(dumpCommLog(paramData, offset, 16));
			offset += 16;			
		}

		if (paramHeader == null) {
			printCommLog(sendOrReceive, sb.toString());
		}
			
		for (int i = 0; i < paramHeader.length; i++) {
			offset = 0;
			while (offset < paramHeader[i].length) {
				sb.append(dumpCommLog(paramHeader[i], offset, 16));
				offset += 16;			
			} 
			offset = 0;
			while (offset < argumentData[i].length) {
				sb.append(dumpCommLog(argumentData[i], offset, 16));
				offset += 16;			
			}
		}
		
		printCommLog(sendOrReceive, sb.toString());
	}

	/**
	 * @param param
	 */
	protected void setParameter (byte[] param) {
		this.paramData = param;
		this.length = param.length - paddNBytes;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#sendBINDataToWire(org.apgrid.grpc.ng.Wire)
	 */
	protected void sendBINDataToWire(Wire wire) throws GrpcException {
		RemoteMethodInfo remoteMethodInfo = callContext.getRemoteMethodInfo();
		List remoteMethodArgs = remoteMethodInfo.getArgs();

		/* send header */
		wire.sendBytes(this.headerData);
		/* send argument header */
		wire.sendBytes(this.paramData);
		printHeaderCommLog(NgLog.COMMLOG_SEND);
		printBodyCommLog(NgLog.COMMLOG_SEND, this.paramData);

		for (int i = 0; i < argumentData.length; i++) {
			if (this.paramHeader[i].length == 0) {
				continue;
			}

			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(i);

			/* send param header */
			wire.sendBytes(this.paramHeader[i]);
			printBodyCommLog(NgLog.COMMLOG_SEND, this.paramHeader[i]);
			
			if ((versionMinor > 1) && ((this.paramLength[i] == DIVIDE_DATA_LENGTH) ||
				remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME)) {
				/* set loop count */
				int loopCount = 1;
				if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) &&
					(remoteMethodArg.getNDims() > 0)) {
					loopCount = callContext.getCount(i);
				}
				
				List args = callContext.getArgs();
				for (int j = 0; j < loopCount; j++) {
					/* set InputStream */
					InputStream is = null;
					String inputFile = null;
					if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) {
						if (remoteMethodArg.getNDims() > 0) {
							String[] fileArray = (String []) args.get(i);
							inputFile = fileArray[j];
						} else {
							inputFile = (String) args.get(i);
						}
						
						/* create InputStream for the file */
						try {
							if (j == 0) {
								if (remoteMethodArg.getNDims() > 0) {
									is = new NgEncodeDataRawFileInputStream(
											inputFile,
											this.paramLength[i] - NgEncodeDataRawFileInputStream.FILE_ENCODE_RAW_HEADER_LEN,
											remoteMethodArg.getMode());
								} else {
									is = new NgEncodeDataRawFileInputStream(
											inputFile, remoteMethodArg.getMode());
								}
							} else {
								is = new NgEncodeDataFileTypeInputStream(
									inputFile, remoteMethodArg.getMode());
							}
						} catch (FileNotFoundException e) {
							throw new NgException(e);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
					} else {
						is = new ByteArrayInputStream(argumentData[i]);
					}
					
					if ((this.paramLength[i] != DIVIDE_DATA_LENGTH) &&
						(remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME)) {
						/* filename type (not divide) */
						byte fileInputBuffer[] = new byte[NgGlobals.fileBufferSize];
						try {
							long totalWriteBytes = 0;
							while (is.available() > 0) {
								int nread = is.read(fileInputBuffer);
								wire.sendBytes(fileInputBuffer, 0, nread);
								printBodyCommLog(NgLog.COMMLOG_SEND,
									fileInputBuffer, nread);
								totalWriteBytes += nread;
							}
							
							/* padding */
							if ((totalWriteBytes % XDR_LIMIT) != 0) {
								int npad =
									(int) (XDR_LIMIT - (totalWriteBytes % XDR_LIMIT));
								byte[] paddingBytes = new byte[npad];
								for (int k = 0; k < npad; k++) {
									paddingBytes[k] = 0;
								}
								wire.sendBytes(paddingBytes);
								printBodyCommLog(NgLog.COMMLOG_SEND, paddingBytes);

								paddNBytes += npad;
							}
						} catch (IOException e) {
							throw new NgIOException(e);
						}
					} else {
						/* divide data */
						int nAvailable = 0;
						try {
							nAvailable = is.available();
						} catch (IOException e1) {
							throw new NgIOException(e1);
						}
						byte[] sendData = divideData(is);
						
						boolean boolArgCompress = false;
							if ((boolCompress == true) &&
								(nAvailable > compressThreshold)) {
								boolArgCompress = true;
							}
						
						if ((boolArgCompress == true) && 
							(this.originalDataLength[i] == -1)) {
							this.originalDataLength[i] = 0;
							this.compressRealTime[i] = 0;
							this.compressCPUTime[i] = 0;
							this.compressedDataLength[i] = 0;
						}
						
						while (sendData != null) {
							if (boolArgCompress == true) {
								/* compress */
								this.originalDataLength[i] += sendData.length;
								timer.start();
								sendData = NgEncodeData.encode(
									NgEncodeData.NG_COMPRESS_ZLIB, sendData, null);
								this.compressRealTime[i] += timer.getElapsedTime();
								this.compressedDataLength[i] += sendData.length;
								
								/* header */
								ByteArrayOutputStream bos =
									new ByteArrayOutputStream(NgGlobals.fileBufferSize);
								XDROutputStream xos = new XDROutputStream(bos);
								try {
									bos.close();
									xos.writeInt(NgEncodeData.NG_COMPRESS_ZLIB);
									xos.writeInt(sendData.length);
									xos.write(sendData, 0, sendData.length);							
								} catch (IOException e) {
									throw new NgIOException(e);
								}
								
								/* set compressed data */
								sendData = bos.toByteArray();
							}
							
							/* write divided data */
							wire.sendBytes(sendData);
							printBodyCommLog(NgLog.COMMLOG_SEND, sendData);
							
							/* padding */
							if ((sendData.length % XDR_LIMIT) != 0) {
								int npad = XDR_LIMIT - (sendData.length % XDR_LIMIT);
								byte[] paddingBytes = new byte[npad];
								for (int k = 0; k < npad; k++) {
									paddingBytes[k] = 0;
								}
								wire.sendBytes(paddingBytes);
								printBodyCommLog(NgLog.COMMLOG_SEND, paddingBytes);

								paddNBytes += npad;
							}
							
							/* read next data */
							sendData = divideData(is);
						}
						
						/* write the end of divide data */
						wire.sendBytes(makeDivideEndData());
					}
					
					/* close the InputStream */
					try {
						is.close();
					} catch (IOException e) {
						throw new NgIOException(e);
					}
				}
			} else {
				if (versionMinor > 0) {
					wire.sendBytes(this.argumentData[i]);
					printBodyCommLog(NgLog.COMMLOG_SEND, this.argumentData[i]);
				} else {
					wire.sendBytes(this.argumentData[i], 0,
						this.argumentData[i].length - ENCODE_HEADER_LEN);
					printBodyCommLog(NgLog.COMMLOG_SEND, this.argumentData[i],
						this.argumentData[i].length - ENCODE_HEADER_LEN);
				}

				/* padding */
				if ((argumentData[i].length % XDR_LIMIT) != 0) {
					int npad = XDR_LIMIT - (argumentData[i].length % XDR_LIMIT);
					byte[] paddingBytes = new byte[npad];
					for (int j = 0; j < npad; j++) {
						paddingBytes[j] = 0;
					}
					wire.sendBytes(paddingBytes);
					printBodyCommLog(NgLog.COMMLOG_SEND, paddingBytes);

					paddNBytes += npad;
				}
			}
		}
	}

	protected void setupHeader() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);

		int typeAndKind = (kind << 24) + type;
		try {
			xo.writeInt(typeAndKind);
			xo.writeInt(sequence);
			xo.writeInt(contextID);
			xo.writeInt(executableID);
			xo.writeInt(sessionID);
			xo.writeInt(0);  /* padding */
			xo.writeInt(result);
			xo.writeInt(length);
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		/* set Data to headerData */
		this.headerData = bo.toByteArray();
		xo.close();
	}
    
    /**
     * @return
     */
    public int getOriginalDataTotalLength() {
    	int originalLength = 0;
    	for (int i = 0; i < this.originalDataLength.length; i++) {
    		if (this.originalDataLength[i] != -1) {
        		originalLength += this.originalDataLength[i];
    		}
    	}
    	return originalLength;
    }
    
    /**
     * @return
     */
    public int getConvertedDataTotalLength() {
    	int compressedLength = 0;
    	for (int i = 0; i < this.compressedDataLength.length; i++) {
    		if (this.compressedDataLength[i] != -1) {
        		compressedLength += this.compressedDataLength[i];
    		}
    	}
    	return compressedLength;
    }
    
    /**
     * @return
     */
    public double getConvertTotalRealTime() {
    	double compressionTime = 0;
    	for (int i = 0; i < this.compressRealTime.length; i++) {
    		if (this.compressRealTime[i] != -1) {
        		compressionTime += this.compressRealTime[i];
    		}
    	}
    	return compressionTime;
   }
    
    /**
     * @return
     */
    public double getConvertTotalCPUTime() {
    	double compressionTime = 0;
    	for (int i = 0; i < this.compressCPUTime.length; i++) {
    		if (this.compressCPUTime[i] != -1) {
        		compressionTime += this.compressCPUTime[i];
    		}
    	}
    	return compressionTime;
   }
    
    /**
     * @return
     */
    public long[] getOriginalDataLength() {
    	if (this.originalDataLength.length < getMaxResultID()) {
    		/* reset originalDataLength array */
    		long[] newOriginalDataLength = new long[getMaxResultID()];
    		for (int i = 0; i < newOriginalDataLength.length; i++) {
    			newOriginalDataLength[i] = -1;
    		}
    		for (int i = 0; i < this.originalDataLength.length; i++) {
    			newOriginalDataLength[numArray[i] - 1] = this.originalDataLength[i];
    		}
    		/* set new originalDataLength array */
    		this.originalDataLength = newOriginalDataLength;
    	}
    	return this.originalDataLength;
    }
    
    /**
     * @return
     */
    public long[] getConvertedDataLength() {
    	if (this.compressedDataLength.length < getMaxResultID()) {
    		/* reset compressedDataLength array */
    		long[] newConvertedDataLength = new long[getMaxResultID()];
    		for (int i = 0; i < newConvertedDataLength.length; i++) {
    			newConvertedDataLength[i] = -1;
    		}
    		for (int i = 0; i < this.compressedDataLength.length; i++) {
    			newConvertedDataLength[numArray[i] - 1] = this.compressedDataLength[i];
    		}
    		/* set new originalDataLength array */
    		this.compressedDataLength = newConvertedDataLength;
    	}
    	return this.compressedDataLength;
    }
    
    /**
     * @return
     */
    public double[] getConvertRealTime() {
    	if (this.compressRealTime.length < getMaxResultID()) {
    		/* reset compressTime array */
    		double[] newCompressTime = new double[getMaxResultID()];
    		for (int i = 0; i < newCompressTime.length; i++) {
    			newCompressTime[i] = -1;
    		}
    		for (int i = 0; i < this.compressRealTime.length; i++) {
    			newCompressTime[numArray[i] - 1] = this.compressRealTime[i];
    		}
    		/* set new originalDataLength array */
    		this.compressRealTime = newCompressTime;
    	}
    	return this.compressRealTime;
    }
    
    /**
     * @return
     */
    public double[] getConvertCPUTime() {
    	if (this.compressRealTime.length < getMaxResultID()) {
    		/* reset compressTime array */
    		double[] newCompressTime = new double[getMaxResultID()];
    		for (int i = 0; i < newCompressTime.length; i++) {
    			newCompressTime[i] = -1;
    		}
    		for (int i = 0; i < this.compressRealTime.length; i++) {
    			newCompressTime[numArray[i] - 1] = this.compressRealTime[i];
    		}
    		/* set new originalDataLength array */
    		this.compressRealTime = newCompressTime;
    	}
    	return this.compressRealTime;
    }
    
    /**
     * @return
     */
    protected int getMaxResultID() {
    	int maxResultID = 0;

    	if (numArray != null) {
        	for (int i = 0; i < numArray.length; i++) {
        		if (maxResultID < numArray[i]) {
        			maxResultID = numArray[i];
        		}
        	}
    	}
    	
    	return maxResultID;
    }
    
    /**
     * @param is
     * @return
     */
    private byte[] divideData(InputStream is) throws GrpcException {
    	byte[] inputBuffer = new byte[this.blockSize];
    	ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.argBufferSize);
    	XDROutputStream xo = new XDROutputStream(bo);
    	
    	try {
			/* write type of encode */
			xo.writeInt(NgEncodeData.NG_DATA_DIVIDE);
			
			/* write if the data does continue */
			if (is.available() == 0) {
				return null;
			} else {
				xo.writeInt(DIVIDE_DATA_CONTINUE);
			}
			int nread = is.read(inputBuffer);
			
			/* write length of the data */
			xo.writeInt(nread);
			
			/* write data */
			xo.writeBytes(inputBuffer, 0, nread);
			
			bo.close();
			return bo.toByteArray();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
    }
    
    /**
      * @return
     */
    private byte[] makeDivideEndData() throws GrpcException {
    	ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
    	XDROutputStream xo = new XDROutputStream(bo);
    	
    	try {
			/* write type of encode */
			xo.writeInt(NgEncodeData.NG_DATA_DIVIDE);
			
			/* write if the data does continue */
			xo.writeInt(DIVIDE_DATA_END);
			
			/* write length of the data */
			xo.writeInt(0);
			
			bo.close();
			return bo.toByteArray();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
    }

    /**
     * @param wire
     * @return
     * @throws GrpcException
     */
    protected int readIntFromWire(Wire wire) throws GrpcException {
		byte[] buffer = new byte[XDR_LIMIT];
		wire.receiveBytes(buffer, 0, XDR_LIMIT);
		printBodyCommLog(NgLog.COMMLOG_RECV, buffer);
		ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
		XDRInputStream xi = new XDRInputStream(bi);

		/* get int variable */
		int returnValue = xi.readInt();

		try {
			bi.close();
		} catch (IOException e1) {
			throw new NgIOException(e1);
		}
		
		return returnValue;
   }
}
