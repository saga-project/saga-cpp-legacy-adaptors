/*
 * $RCSfile: ProtTransferCallbackResultRequest.java,v $ $Revision: 1.12 $ $Date: 2008/02/07 08:17:43 $
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

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.List;
import java.util.ArrayList;
import java.util.Vector;

import org.apgrid.grpc.ng.CompressInfo;
import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.CallContext;
import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgExecRemoteMethodException;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.NgByteArrayOutputStream;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.Version;
import org.apgrid.grpc.ng.info.RemoteMethodArg;
import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.apgrid.grpc.ng.info.RemoteMethodArgSubScript;
import org.apgrid.grpc.ng.dataencode.NgEncodeDataFileTypeInputStream;
import org.apgrid.grpc.ng.dataencode.NgEncodeDataRawFileInputStream;
import org.apgrid.grpc.ng.dataencode.NgEncodeData;
import org.apgrid.grpc.util.GrpcTimer;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.dataencode.NgEncodeData.NG_COMPRESS_ZLIB;
import static org.apgrid.grpc.ng.NgGlobals.smallBufferSize;
import static org.apgrid.grpc.ng.NgGlobals.fileBufferSize;
import static org.apgrid.grpc.ng.NgGlobals.argBufferSize;
import static org.apgrid.grpc.ng.NgParamTypes.NG_TYPE_FILENAME;
import static org.apgrid.grpc.ng.NgParamTypes.NG_MODE_IN;
import static org.apgrid.grpc.ng.NgParamTypes.NG_MODE_OUT;
import static org.apgrid.grpc.ng.NgParamTypes.NG_MODE_INOUT;
import static org.apgrid.grpc.ng.Protocol.KEEP_CONNECTION;
import static org.apgrid.grpc.ng.Protocol.DISCONNECT;

public final class ProtTransferCallbackResultRequest
implements NgProtocolRequest {

	private final static String NAME = "TransferCallbackResultRequest";
	private final static int KIND = 0;
	private final static int TYPE = 0x34;

	private int callbackID;

	private static final int DIVIDE_DATA_CONTINUE = 0x001;
	private static final int DIVIDE_DATA_LENGTH   = -1;
	private static final int DIVIDE_DATA_END      = 0x0000;
	private static final int XDR_LIMIT            = 4;
	private static final int ENCODE_HEADER_LEN    = 8;

    private CallContext callContext; // data for arguments
    private List encoderList; // list for encoders
    // compress
	private CompressInfo aCompressInfo;
    private boolean boolCompress = false;
    private int compressThreshold = 0;
    // divide data
    private int blockSize = 0;
    private int disconnect = KEEP_CONNECTION;
    private byte[] paramData;
    private int length;
    private Version version;
	private ProtocolHeader header;
	private GrpcTimer timer;

	// data for Callback Result
	private List<ArgumentData> arguments;

	/**
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 */
	public ProtTransferCallbackResultRequest(int sequenceNum,
	 int contextID, int executableID, int sessionID,
	 Version protVersion,
	 int callbackID,
	 CallContext callContext,
	 List encoderList,
	 CompressInfo aCompressInfo,
	 int blockSize,
	 boolean keepConnection)
	 throws GrpcException {

        this.callContext = callContext;   // set CallContext
        this.encoderList = encoderList;   // set Encode list
        this.boolCompress =
			aCompressInfo.getCompress().equals(CompressInfo.COMPRESS_ZLIB);
        this.compressThreshold =
			aCompressInfo.getCompressThreshold(); // set compressThreshold
		this.aCompressInfo = aCompressInfo;		
        this.blockSize = blockSize;       // set divideSize
		this.disconnect = keepConnection ? KEEP_CONNECTION : DISCONNECT;
		this.version = protVersion;
		// set variables for callback 
		this.callbackID = callbackID;
		this.timer = new GrpcTimer();

		setupParameter();

		this.header = new ProtocolHeader(KIND, TYPE,
			sequenceNum,
			contextID,
			executableID,
			sessionID,
			0,
			this.length);

	}

	public int getType() {
		return TYPE;
	}

	public String getName() {
		return NAME;
	}

	private void setupParameter() throws GrpcException {
		// Create transfer callback results
		this.arguments = createTransferData();

		try {
			NgByteArrayOutputStream bos =
				new NgByteArrayOutputStream(smallBufferSize);

			// put callbackID and Sequence 
			bos.writeInt(callbackID);
			bos.writeInt(disconnect);
			bos.writeInt(arguments.size()); // write NumParams
			this.paramData = bos.toByteArray();
			bos.close();
			this.length += 12;
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		if (isDivideArgumentDataSpecified()) {
			this.length = DIVIDE_DATA_LENGTH;
			return ;
		} else {
			// sum parameter length
			for (ArgumentData arg : arguments) {
				int l = arg.length();
				if (l == DIVIDE_DATA_LENGTH) {
					this.length = DIVIDE_DATA_LENGTH;
					break;
				}
				this.length += l;
			}
		}
	}

	private List<ArgumentData> createTransferData() throws GrpcException {
        List<ArgumentData> ret = new ArrayList<ArgumentData>();

        RemoteMethodInfo rmi = callContext.getRemoteMethodInfo();
        List<RemoteMethodArg> rma_list = rmi.getArgs();
        int nArgs  = rmi.getNumParams();
        for (int i = 0; i < nArgs; i++) {
            // get information for argument
            RemoteMethodArg rma = rma_list.get(i);

            if ( isArgumentForTransmission(rma) ) {
                // create argument data
                ArgumentData elem =
					ArgumentDataFactory.newInstance(i, rmi, rma, callContext,
						version, encoderList, aCompressInfo, blockSize);
                ret.add(elem);
            }
        }
        return ret;
	}

	private boolean isTypeFilename(int type) {
		return (type == NG_TYPE_FILENAME);
	}

	private boolean isModeIN_INOUT(int mode) {
		return ((mode == NG_MODE_IN) || (mode == NG_MODE_INOUT)); 
	}

	/*
	 * @return true RemoteMethodArg should transfer data to Executable
	 * @return false RemoteMethodArg shouldn't transfer data. (e.g. mode is OUT)
	 */
	private boolean isArgumentForTransmission(RemoteMethodArg rma) {
        int mode = rma.getMode();
        return (mode == NG_MODE_OUT) || (mode == NG_MODE_INOUT);
	}

	private boolean isDivideArgumentDataSpecified() {
		boolean specified_divide_argument_data = (this.blockSize != 0);
		boolean data_division_is_possible =
			this.encoderList.contains(NgEncodeData.NG_DATA_DIVIDE);
		return specified_divide_argument_data && data_division_is_possible;
	}

	private void log(Wire logger, String msg, byte[] data) {
		log(logger, msg, data, data.length);
	}
	private void log(Wire logger, String msg, byte[] data, int length) {
		logger.logCommLog(this, msg, data, length);
	}

	public void send(Wire peer) throws GrpcException {
		// 32byte header
		byte[] header_data = this.header.toByteArray();
		peer.sendBytes(header_data);
		log(peer, "Protocol Header.", header_data);

		peer.sendBytes(this.paramData);
		log(peer, "Number of Arguments.", this.paramData);

		for (ArgumentData argument :  arguments) {

			peer.sendBytes(argument.getParamHeader());
			//logSendArgumentHeaderAt(i);

			int nArg = argument.getNumber() - 1;
			if ( argument.isDataDivisionSpecified() 
				|| argument.isTypeFilename() )  {
				// set loop count 
				int loopCount = 1;
				if (argument.isTypeFilename() && argument.isArray()) {
					loopCount = callContext.getCount(nArg);
				}

				for (int j = 0; j < loopCount; j++) { // What loop?
					// set InputStream block begin
					InputStream is = null;
					if ( argument.isTypeFilename() ) {
						String inputFile = null;
						if (argument.isArray()) {
							String[] fileArray =
								(String[])argument.getJavaData();
							inputFile = fileArray[j];
						} else {
							inputFile = (String)argument.getJavaData();
						}

						// create InputStream for the file 
						try {
							if (j == 0) {
								if (argument.isArray()) {
									is = new NgEncodeDataRawFileInputStream(
										inputFile, argument.length() - NgEncodeDataRawFileInputStream.FILE_ENCODE_RAW_HEADER_LEN, argument.getMode());
								} else {
									is = new NgEncodeDataRawFileInputStream(inputFile, argument.getMode());
								}
							} else {
								is = new NgEncodeDataFileTypeInputStream(
									inputFile, argument.getMode());
							}
						} catch (FileNotFoundException e) {
							throw new NgException(e);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
					} else {
						is = new ByteArrayInputStream(argument.getData());
					}
					// set InputStream block end

					if ((! argument.isDataDivisionSpecified()) &&
						argument.isTypeFilename() ) {

						// filename type (not divide) 
						byte [] fileInputBuffer =
							new byte[fileBufferSize];
						try {
							long totalWriteBytes = 0;
							while (is.available() > 0) {
								int nread = is.read(fileInputBuffer);
								peer.sendBytes(fileInputBuffer, 0, nread);
								log(peer, "Filename Data.", fileInputBuffer, nread);
								totalWriteBytes += nread;
							}

							// padding
							int remainder = (int)(totalWriteBytes % XDR_LIMIT);
							if (remainder != 0) {
								byte [] padding = padding_byte(remainder);
								peer.sendBytes(padding);
								log(peer, "padding", padding);
							}
						} catch (IOException e) {
							throw new NgIOException(e);
						}
					} else {
						// divide data 
						int nAvailable = 0;
						try {
							nAvailable = is.available();
						} catch (IOException e1) {
							throw new NgIOException(e1);
						}
						byte[] sendData = divideData(is);

						boolean boolArgCompress = false;
						if ( ( boolCompress ) &&
							(nAvailable > compressThreshold)) {
							boolArgCompress = true;
						}

						if ( boolArgCompress &&
							(! argument.isCompressSpecified()) ) {
							argument.setCompressInfo(0L, 0L, 0L, 0L);
						}

						while (sendData != null) {
							if ( boolArgCompress ) {
								// compress 
                                argument.addOriginalDataLength(sendData.length);
								timer.start();
								sendData = NgEncodeData.encode(
									NG_COMPRESS_ZLIB, sendData, null);
								argument.addCompressRealTime(
									timer.getElapsedTime());
								argument.addCompressDataLength(sendData.length);

								// header 
								NgByteArrayOutputStream bos =
									new NgByteArrayOutputStream(fileBufferSize);
								try {
									bos.writeInt(NG_COMPRESS_ZLIB);
									bos.writeInt(sendData.length);
									bos.writeBytes(sendData, 0, sendData.length);
									bos.close();
								} catch (IOException e) {
									throw new NgIOException(e);
								}

								// set compressed data 
								sendData = bos.toByteArray();
							}

							// write divided data 
							peer.sendBytes(sendData);
							log(peer, "Divided Data.", sendData);

							// padding 
							int remainder = sendData.length % XDR_LIMIT;
							if (remainder != 0) {
								byte[] padding = padding_byte(remainder);
								peer.sendBytes(padding);
								log(peer, "padding", padding);
							}
							// read next data 
							sendData = divideData(is);
						}
						// write the end of divide data 
						byte [] end_data = makeDivideEndData();
						peer.sendBytes(makeDivideEndData());
						log(peer, "Divide End Data", end_data);
					}
					// close the InputStream 
					try {
						is.close();
					} catch (IOException e) {
						throw new NgIOException(e);
					}
				}
			} else {
				// Data Division is not specified
				// "argument_blockSize" is 0 in <SERVER> section
				byte [] data = argument.getData();
				peer.sendBytes(data);
				log(peer, "Argument Data", data);
				// padding 
				int remainder = argument.length() % XDR_LIMIT;
				if (remainder != 0) {
					byte[] padding = padding_byte(remainder);
					peer.sendBytes(padding);
					log(peer, "padding", padding);
				}
			}
		}
	}

	private byte[] padding_byte(int remainder) {
		int npad = XDR_LIMIT - remainder;
		return new byte[npad];
	}

    /**
	 * Divide argument data and add to division header (kind is CONTINUE(1)) 
	 * 
     * @param is
     * @return byte[] Divided data
	 * @return null There is no data. 
     */
    private byte[] divideData(InputStream is) throws GrpcException {
    	byte[] inputBuffer = new byte[this.blockSize];
		NgByteArrayOutputStream bos =
			new NgByteArrayOutputStream(argBufferSize);

    	try {
			bos.writeInt(NgEncodeData.NG_DATA_DIVIDE); // write type of encode 
			if (is.available() == 0) {
				return null;
			} else {
				// write if the data does continue 
				bos.writeInt(DIVIDE_DATA_CONTINUE);
			}
			int nread = is.read(inputBuffer);
			bos.writeInt(nread);		// write length of the data 
			bos.writeBytes(inputBuffer, 0, nread); // write data 
			bos.close();
			return bos.toByteArray();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
    }

	/**
	 * Return a byte array that represent end of data division. (kind is 0)
	 * 
	 * @return byte [] A byte array that end of data division. 
	 */
	private byte[] makeDivideEndData() throws GrpcException {
		NgByteArrayOutputStream bos =
			new NgByteArrayOutputStream(smallBufferSize);
    	try {
			bos.writeInt(NgEncodeData.NG_DATA_DIVIDE); // write type of encode 
			bos.writeInt(DIVIDE_DATA_END); // write if the data does continue 
			bos.writeInt(0); // write length of the data 
			bos.close();
			return bos.toByteArray();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
    }

	public String toString() {
		return this.header.toString();
	}
}

