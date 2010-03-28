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
 * $RCSfile: ProtTransferResultReply.java,v $ $Revision: 1.49 $ $Date: 2005/10/03 02:13:24 $
 */
package org.apgrid.grpc.ng.protocol;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;
import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.CallContext;
import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgExecRemoteMethodException;
import org.apgrid.grpc.ng.NgGlobals;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgLog;
import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.dataencode.NgEncodeData;
import org.apgrid.grpc.ng.dataencode.NgEncodeDataFileTypeOutputStream;
import org.apgrid.grpc.ng.dataencode.NgEncodeDataRawFileInputStream;
import org.apgrid.grpc.ng.dataencode.NgEncodeDataRawFileOutputStream;
import org.apgrid.grpc.ng.info.RemoteMethodArg;
import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.apgrid.grpc.util.GrpcTimer;
import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class ProtTransferResultReply extends ProtTransferArgumentRequest {
	public final static String PROTO_STR = "ProtTransferResultReply";
	public final static int PROTOCOL_KIND = 1;
	public final static int PROTOCOL_TYPE = 0x32;
	public final static String PROTOCOL_TYPE_XML_STR = "transferResultData";

	/**
	 * @param kind
	 * @param type
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param result
	 * @param argumentData
	 */
	public ProtTransferResultReply(int kind, int type, int sequenceNum,
		int contextID, int executableID, int sessionID, int result,
		CallContext callContext, boolean compressEnable, int compressThreshold,
		int versionMajor, int versionMinor, int versionPatch) {
		super(kind,
			type,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			result,
			versionMajor,
			versionMinor,
			versionPatch);
		this.callContext = callContext;
		this.encoderList = NgEncodeData.getSupportEncodeType();
		this.boolCompress = compressEnable;
		this.compressThreshold = compressThreshold;
		
		this.strType = PROTOCOL_TYPE_XML_STR;
	}

	/**
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param result
	 * @param argumentData
	 */
	public ProtTransferResultReply(int sequenceNum,
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
			versionMajor,
			versionMinor,
			versionPatch);
		this.callContext = callContext;
		this.encoderList = NgEncodeData.getSupportEncodeType();
		this.boolCompress = compressEnable;
		this.compressThreshold = compressThreshold;
		
		this.strType = PROTOCOL_TYPE_XML_STR;
	}

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtTransferResultReply(NgDataInputStream ngdi)
		throws GrpcException {
		super(ngdi);
		setTypeAndKind();
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public ProtTransferResultReply(Node node)
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

		for (int i = 0; i < remoteMethodArgs.size(); i++) {
			/* get information for argument */
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(i);
				
			/* check if Mode was OUT,INOUT */
			if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) ||
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_OUT) &&
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
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_OUT) ||
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)) {
				numParams++;
			} 
		}
		try {
			xo.writeInt(numParams);
			this.length += 4;
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		/* alloc area for param */
		int nArgs = remoteMethodArgs.size();
		this.paramHeader = new byte[nArgs][0];
		this.argumentData = new byte[nArgs][0];

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
			/* prepare for paramHeader */
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
	 * @see org.apgrid.grpc.ng.Protocol#parseParam(org.apgrid.grpc.ng.Wire, org.apgrid.ng.Protocol)
	 */
	protected void parseParam(Wire wire, Protocol prot) throws GrpcException {
		ProtTransferResultRequest resultRequest = null;
		CallContext callContext = null;

		if ((this instanceof ProtTransferCallbackArgumentReply) != true) {
			resultRequest = (ProtTransferResultRequest) prot;
			callContext = resultRequest.getCallContext();
		}

		this.paramData = new byte[SIZE_OF_NUM_PARAMS];
		wire.receiveBytes(this.paramData);
		/* print commLog */
		if (! (this instanceof ProtTransferCallbackArgumentReply)) {
			printHeaderCommLog(NgLog.COMMLOG_RECV);
		}
		printBodyCommLog(NgLog.COMMLOG_RECV, this.paramData);
		
		ByteArrayInputStream bi = new ByteArrayInputStream(this.paramData);
		XDRInputStream xi = new XDRInputStream(bi);

		/* count of Result */
		int numParams = xi.readInt();
		try {
			bi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		this.argumentData = new byte[numParams][];
		this.numArray = new int[numParams];
		this.typeArray = new int[numParams];
		this.modeArray = new int[numParams];
		this.paramHeader = new byte[numParams][];
		this.originalDataLength = new long[numParams];
		this.compressedDataLength = new long[numParams];
		this.timer = new GrpcTimer();
		this.compressRealTime = new double[numParams];
		this.compressCPUTime = new double[numParams];
		for (int i = 0; i < originalDataLength.length; i++) {
			this.originalDataLength[i] = -1;
			this.compressedDataLength[i] = -1;
			this.compressRealTime[i] = -1;
			this.compressCPUTime[i] = -1;
		}
		
		/* read Result */
		int resultIndex = 0;
		for (int i = 0; i < numParams; i++) {
			paramHeader[i] = new byte[SIZE_OF_PARAM_HEADER];
			wire.receiveBytes(paramHeader[i], 0, SIZE_OF_PARAM_HEADER);
			printBodyCommLog(NgLog.COMMLOG_RECV, paramHeader[i]);
			
			bi = new ByteArrayInputStream(paramHeader[i]);
			xi = new XDRInputStream(bi);
			
			/* number of Result */
			int resultID = xi.readInt();
			/* type */
			int type = xi.readInt();
			/* mode */
			int mode = xi.readInt();
			/* encode */
			int encodeType = xi.readInt();
			/* number of Elemnt */
			int nElements = xi.readInt();
			/* length of data */
			int length = xi.readInt();
			
			try {
				bi.close();
			} catch (IOException e) {
				throw new NgIOException(e);
			}

			/* set index information */
			this.numArray[i] = resultID;
			
			/* read skip info */
			Vector skipEncodeInfo = new Vector();
			if ((encodeType & ProtTransferArgumentRequest.ENABLE_SKIP) != 0) {
				byte[] nDimBuffer = new byte[SIZE_OF_NDIM];
				wire.receiveBytes(nDimBuffer, 0, SIZE_OF_NDIM);
				printBodyCommLog(NgLog.COMMLOG_RECV, nDimBuffer);
				
				bi = new ByteArrayInputStream(nDimBuffer);
				xi = new XDRInputStream(bi);

				int nDims = xi.readInt();
				try {
					bi.close();
				} catch (IOException e) {
					throw new NgIOException(e);
				}

				byte[] skipHeader = new byte[SIZE_OF_SKIP_HEADER * nDims];
				wire.receiveBytes(skipHeader, 0, SIZE_OF_SKIP_HEADER * nDims);
				printBodyCommLog(NgLog.COMMLOG_RECV, skipHeader);

				bi = new ByteArrayInputStream(skipHeader);
				xi = new XDRInputStream(bi);

				int arrayShapes[][] = new int[nDims][4];
				/* read arrayShape */
				for (int j = 0; j < nDims; j++) {
					/* size */
					arrayShapes[j][0] = xi.readInt();
					/* start */
					arrayShapes[j][1] = xi.readInt();
					/* end */
					arrayShapes[j][2] = xi.readInt();
					/* skip */
					arrayShapes[j][3] = xi.readInt();
				}

				try {
					bi.close();
				} catch (IOException e) {
					throw new NgIOException(e);
				}

				int argType[] = new int[1];
				argType[0] = type;
				
				skipEncodeInfo.add(argType);
				skipEncodeInfo.add(arrayShapes);

				/* reset paramHeader */
				byte[] orig_paramHeader = paramHeader[i];
				paramHeader[i] =
					new byte[paramHeader[i].length + nDimBuffer.length + skipHeader.length];
				System.arraycopy(orig_paramHeader, 0, paramHeader[i], 0, orig_paramHeader.length);
				System.arraycopy(nDimBuffer, 0, paramHeader[i], orig_paramHeader.length, nDimBuffer.length);
				System.arraycopy(skipHeader, 0, paramHeader[i], orig_paramHeader.length + nDimBuffer.length, skipHeader.length);
			}
			
			/* read data */
			byte[] buffer = null;
			if (length == DIVIDE_DATA_LENGTH) {
				/* the data is divided */
				int loopCount = 1;
				RemoteMethodArg remoteMethodArg = null;
				if (callContext != null) {
					RemoteMethodInfo remoteMethodInfo = callContext.getRemoteMethodInfo();
					remoteMethodArg =
						(RemoteMethodArg) remoteMethodInfo.getArgs().get(resultID - 1);
					if ((type == NgParamTypes.NG_TYPE_FILENAME) &&
						(remoteMethodArg.getNDims() > 0)) {
						loopCount = nElements;
					}
				}
				
				OutputStream os = null;
				/* loop if it's array of filename type */
				for (int j = 0; j < loopCount; j++) {
					/* create OutputStream for output data */
					if (type == NgParamTypes.NG_TYPE_FILENAME) {
						List args = callContext.getArgs();
						try {
							if ((remoteMethodArg != null) &&
								(remoteMethodArg.getNDims() > 0)) {
								String[] fileArray = (String []) args.get(resultID - 1);
								if (j == 0) {
									os = new NgEncodeDataRawFileOutputStream(fileArray[j]);
								} else {
									os = new NgEncodeDataFileTypeOutputStream(fileArray[j]);
								}
							} else {
								os = new NgEncodeDataRawFileOutputStream((String) args.get(resultID - 1));
							}
						} catch (FileNotFoundException e1) {
							throw new NgIOException(e1);
						}
					} else {
						os = new ByteArrayOutputStream(NgGlobals.argBufferSize);
					}

					/* flag for divide data */
					boolean divideContinue = false;
					/* length of divided data */
					int divide_length = 0;
					/* InputStream */
					ByteArrayInputStream divide_bi = null;
					XDRInputStream divide_xi = null;
					
					/* read divided/compressed data */
					do {
						/* read encode type */
						int divideEncodeType = readIntFromWire(wire);
						/* length of padding */
						int nPadd = 0;
						
						/* Is it compressed? */
						if (divideEncodeType == NgEncodeData.NG_COMPRESS_ZLIB) {
							int compressed_length = readIntFromWire(wire);
							
							/* read compressed data */
							byte compresssedData[] = new byte[compressed_length];
							wire.receiveBytes(compresssedData, 0, compressed_length);
							printBodyCommLog(NgLog.COMMLOG_RECV, compresssedData);
							/* get length of padding */
							if ((compressed_length % XDR_LIMIT) != 0) {
								nPadd = XDR_LIMIT - (compressed_length % XDR_LIMIT);
							}
							
							/* initialize time of compress/decompress */
							if (this.compressedDataLength[i] == -1) {
								this.compressedDataLength[i] = 0;
								this.originalDataLength[i] = 0;
								this.compressRealTime[i] = 0;
								this.compressCPUTime[i] = 0;
							}
							
							/* decompress */
							this.compressedDataLength[i] += compresssedData.length;
							timer.start();
							compresssedData = NgEncodeData.decode(
								NgEncodeData.NG_COMPRESS_ZLIB, compresssedData, null);
							this.compressRealTime[i] += timer.getElapsedTime();
							this.originalDataLength[i] += compresssedData.length;

							/* read encode type */
							divide_bi = new ByteArrayInputStream(compresssedData);
							divide_xi = new XDRInputStream(divide_bi);
							
							/* get encode type */
							divideEncodeType = divide_xi.readInt();
							
							/* get continue flag */
							divideContinue =
								divide_xi.readInt() == DIVIDE_DATA_CONTINUE ? true : false;
							/* get length of divided data */
							divide_length = divide_xi.readInt();
						} else if (divideEncodeType == NgEncodeData.NG_DATA_DIVIDE){
							/* get continue flag */
							divideContinue =
								readIntFromWire(wire) == DIVIDE_DATA_CONTINUE ? true : false;
							/* get length of divide data */
							divide_length = readIntFromWire(wire);

							/* read divided data */
							byte dividedData[] = new byte[divide_length];
							wire.receiveBytes(dividedData, 0, divide_length);
							printBodyCommLog(NgLog.COMMLOG_RECV, dividedData);
							/* get length of padding */
							if ((divide_length % XDR_LIMIT) != 0) {
								nPadd = XDR_LIMIT - (divide_length % XDR_LIMIT);
							}

							/* read encode type */
							divide_bi = new ByteArrayInputStream(dividedData);
							divide_xi = new XDRInputStream(divide_bi);
						}
						
						/* check if it's valid */
						if (divideEncodeType != NgEncodeData.NG_DATA_DIVIDE) {
							throw new NgException("Invalid divided data.");
						}
						
						/* read divided data */
						byte[] dividedData = new byte[divide_length];
						divide_xi.readBytes(dividedData, 0, divide_length);
						
						/* close InputStream */
						try {
							divide_bi.close();
						} catch (IOException e2) {
							throw new NgIOException(e2);
						}

						/* read padding */
						if (nPadd != 0) {
							byte[] paddingBytes = new byte[nPadd];
							for (int k = 0; k < nPadd; k++) {
								paddingBytes[k] = 0;
							}
							wire.receiveBytes(paddingBytes);
							printBodyCommLog(NgLog.COMMLOG_RECV, paddingBytes);

							paddNBytes += nPadd;
						}
						
						try {
							/* write byte[] to OutputStream */
							os.write(dividedData);
						} catch (IOException e2) {
							throw new NgIOException(e2);
						}
					} while (divideContinue == true);
					
					/* merge all of data */
					try {
						os.close();
					} catch (IOException e1) {
						throw new NgIOException(e1);
					}
				}
				
				if (type == NgParamTypes.NG_TYPE_FILENAME) {
					/* file transfer is completed */
					continue;
				} else {
					/* merged data, process another decode routine */
					ByteArrayOutputStream bos = (ByteArrayOutputStream)os;
					buffer = bos.toByteArray();
				}
			} else if ((versionMinor > 1) && (type == NgParamTypes.NG_TYPE_FILENAME)) {
				int loopCount = 1;
				RemoteMethodInfo remoteMethodInfo = callContext.getRemoteMethodInfo();
				RemoteMethodArg remoteMethodArg =
					(RemoteMethodArg) remoteMethodInfo.getArgs().get(resultID - 1);
				if (remoteMethodArg.getNDims() > 0) {
					loopCount = nElements;
				}
				
				List args = callContext.getArgs();
				for (int j = 0; j < loopCount; j++) {
					/* create OutputStream for output data */
					OutputStream fos = null;
					int headerBufferSize = NgEncodeDataRawFileInputStream.FILE_ENCODE_RAW_HEADER_LEN;
					try {
						if (remoteMethodArg.getNDims() > 0) {
							String[] fileArray = (String []) args.get(resultID - 1);
							if (j == 0) {
								fos = new NgEncodeDataRawFileOutputStream(fileArray[j]);
							} else {
								fos = new NgEncodeDataFileTypeOutputStream(fileArray[j]);
								headerBufferSize = NgEncodeDataRawFileInputStream.ENCODE_HEADER_LEN;
							}
						} else {
							fos = new NgEncodeDataRawFileOutputStream((String) args.get(resultID - 1));
						}
					} catch (FileNotFoundException e1) {
						throw new NgIOException(e1);
					}
					
					/* filename type (not divide) */
					byte[] headerBuffer = new byte[headerBufferSize];
					wire.receiveBytes(headerBuffer);
					printBodyCommLog(NgLog.COMMLOG_RECV,
						headerBuffer,headerBuffer.length);
					try {
						fos.write(headerBuffer);
					} catch (IOException e2) {
						throw new NgIOException(e2);
					}
					/* get size of the file */
					NgEncodeDataFileTypeOutputStream ngfos =
						(NgEncodeDataFileTypeOutputStream)(fos);
					int fileLength = ngfos.getLength();
					int restOfData = fileLength;
					
					/* read data from stream and write it into file */
					try {
						byte fileOutputBuffer[] = null;
						do {
							/* prepare buffer */
							int bufferSize = NgGlobals.fileBufferSize;
							if (bufferSize >= restOfData) {
								bufferSize = restOfData;
							}
							fileOutputBuffer = new byte[bufferSize];
							
							/* read data from wire */
							wire.receiveBytes(fileOutputBuffer);
							
							/* write data into the file */
							fos.write(fileOutputBuffer);
							printBodyCommLog(NgLog.COMMLOG_RECV,
								fileOutputBuffer,
								restOfData > bufferSize ? bufferSize : restOfData);
							restOfData -= bufferSize;
						} while (restOfData > 0);
						
						/* close FileOutputStream */
						fos.close();
						
						/* read padding */
						if ((fileLength % XDR_LIMIT) != 0) {
							int npad = XDR_LIMIT - (fileLength % XDR_LIMIT);
							byte[] paddingBytes = new byte[npad];
							for (int k = 0; k < npad; k++) {
								paddingBytes[k] = 0;
							}
							wire.receiveBytes(paddingBytes);
							printBodyCommLog(NgLog.COMMLOG_RECV, paddingBytes);

							paddNBytes += npad;
						}
					} catch (IOException e) {
						throw new NgIOException(e);
					}
				}
				
				/* file transfer is completed */
				continue;
			} else {
				buffer = new byte [length];
				wire.receiveBytes(buffer, 0, length);
				printBodyCommLog(NgLog.COMMLOG_RECV, buffer);
			}

			/* decode data */
			if (versionMinor > 0) {
				bi = new ByteArrayInputStream(buffer);
				xi = new XDRInputStream(bi);

				/* check if it's RAW? */
				if (xi.readInt() == NgEncodeData.NG_DATA_REPRESENT_RAW) {
					for (int j = 0; j < buffer.length - ENCODE_HEADER_LEN; j++) {
						buffer[j] = buffer[j + ENCODE_HEADER_LEN];
					}
					for (int j = buffer.length - ENCODE_HEADER_LEN; j < buffer.length; j++) {
						buffer[j] = 0;
					}
				} else {
					this.compressedDataLength[i] = buffer.length - ENCODE_HEADER_LEN;
					timer.start();
					buffer = decodeData(buffer, null);
					this.compressRealTime[i] = timer.getElapsedTime();
					this.compressCPUTime[i] = 0;
					this.originalDataLength[i] = buffer.length + ENCODE_HEADER_LEN;
				}
				
				try {
					bi.close();
				} catch (IOException e1) {
					throw new NgIOException(e1);
				}
			}
			/* decoding XDR moved to NgParamTypes */

			/* skip */
			if ((type != NgParamTypes.NG_TYPE_STRING) &&
				((encodeType & ProtTransferArgumentRequest.ENABLE_SKIP) != 0)) {
				buffer = decodeDataSkip(buffer, skipEncodeInfo);
			}
			
			/* set argumentdata */
			this.argumentData[resultIndex++] = buffer;

			/* read padding */
			if ((length != DIVIDE_DATA_LENGTH) && ((length % 4) != 0)) {
				int nPad = 4 - (length % 4);
				byte[] dummyBytes = new byte[nPad];
				wire.receiveBytes(dummyBytes, 0, nPad);
				printBodyCommLog(NgLog.COMMLOG_RECV, dummyBytes);
			}
		}
	}

	/**
	 * @param node
	 * @throws GrpcException
	 */
	protected void parseParam(Node node) throws GrpcException {
		int numParams = XMLUtil.countChildNode(node, tagArgument);
		argumentData = new byte[numParams][];
		numArray = new int[numParams];
		typeArray = new int[numParams];
		modeArray = new int[numParams];
		int resultIndex = 0;
		
		for (int i = 0; i < XMLUtil.countChildNode(node, tagArgument); i++) {
			Node argNode = XMLUtil.getChildNode(node, tagArgument, i);
		
			/* argument number */
			this.numArray[i] = Integer.parseInt(
				XMLUtil.getAttributeValue(argNode, attrArgumentNo));
			/* type & mode */
			this.typeArray[i] = NgParamTypes.getTypeVal(
				XMLUtil.getAttributeValue(argNode, attrDataType));
			this.modeArray[i] = NgParamTypes.getModeVal(
				XMLUtil.getAttributeValue(argNode, attrModeType));

			/* number of Result */
			int resultID = Integer.parseInt(
				XMLUtil.getAttributeValue(argNode, attrArgumentNo));
			/* type */
			int type = NgParamTypes.getTypeVal(
				XMLUtil.getAttributeValue(argNode, attrDataType));
			/* mode */
			int mode = NgParamTypes.getModeVal(
				XMLUtil.getAttributeValue(argNode, attrModeType));
			/* skip and representation */
			String skipEnable =
				XMLUtil.getAttributeValue(argNode, attrSkip);
			String dataRepresentation =
				XMLUtil.getAttributeValue(argNode, attrRepresentation);
			
			/* get skip_info */
			int argType[] = new int[1];
			int arrayShape[][] = null;
			if (argNode != null) {
				argType[0] = type;
				arrayShape = new int[XMLUtil.countChildNode(argNode, tagSkipInfo)][4];
				for (int j = 0; j < XMLUtil.countChildNode(argNode, tagSkipInfo); j++) {
					Node skipInfoNode = XMLUtil.getChildNode(argNode, tagSkipInfo, j);
					/* size */
					arrayShape[j][0] = Integer.parseInt(
						XMLUtil.getAttributeValue(skipInfoNode, attrSkipSize));
					/* start */
					arrayShape[j][1] = Integer.parseInt(
						XMLUtil.getAttributeValue(skipInfoNode, attrSkipStart));
					/* end */
					arrayShape[j][2] = Integer.parseInt(
						XMLUtil.getAttributeValue(skipInfoNode, attrSkipEnd));
					/* skip */
					arrayShape[j][3] = Integer.parseInt(
						XMLUtil.getAttributeValue(skipInfoNode, attrSkipSkip));
				}
			}
			
			/* get DataRepresentation */
			if (! dataRepresentation.equals(REPRESENT_XDR_STR)) {
				throw new NgExecRemoteMethodException("Not supported Encode type : " +
					XMLUtil.getAttributeValue(argNode, attrRepresentation));
			}
			
			/* decode data */
			byte[] buffer = decode(argNode, null);
			
			/* XDR */
			Vector encodeInfo = new Vector();
			encodeInfo.add(new Integer(type));
			buffer = decodeDataXDR(buffer, encodeInfo);

			/* skip */
			if ((arrayShape.length > 0) && (new Boolean(skipEnable).booleanValue())) {
				encodeInfo = new Vector();
				encodeInfo.add(argType);
				encodeInfo.add(arrayShape);
				buffer = decodeDataSkip(buffer, encodeInfo);
			}
		
			/* set argumentdata */
			argumentData[resultIndex++] = buffer;
		}
	}
	
	/**
	 * @return
	 */
	public byte[][] getResultData() {
		return argumentData;
	}
	

	/**
	 * @param callContext
	 * @throws GrpcException
	 */
	public void setupCallContext(CallContext callContext) throws GrpcException {
		this.callContext = callContext;
		
		RemoteMethodInfo remoteMethodInfo =	callContext.getRemoteMethodInfo();
		List remoteMethodArgs = remoteMethodInfo.getArgs();

		int resultIndex = 0;
		for (int i = 0; i < numArray.length; i++) {
			/* get information for argument and check type/mode */
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodArgs.get(numArray[i] - 1);
			if (typeArray[i] != remoteMethodArg.getType()) {
				throw new NgException(
					"ProtTransferResultReply# mismatched type");
			}
			if ((modeArray[i] != NgParamTypes.NG_MODE_OUT) &&
				(modeArray[i] != NgParamTypes.NG_MODE_INOUT)) {
				throw new NgException(
					"ProtTransferResultReply# invalid mode");
			}
			if (typeArray[i] == NgParamTypes.NG_TYPE_FILENAME) {
				/* nothing will be done */
				continue;
			}

			/* set data into CallContext */
			callContext.setData(numArray[i] - 1, argumentData[resultIndex++]);
		}
	}
	
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Protocol#printCommLog(java.lang.String)
	 */
	public void printCommLog(String sendOrReceive) {
		if (this.commLog == null) {
			return;
		}
		
		super.printCommLog(sendOrReceive);
	}
}
