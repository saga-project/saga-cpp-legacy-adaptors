/*
 * $RCSfile: ProtTransferResultReply.java,v $ $Revision: 1.10 $ $Date: 2007/09/26 04:14:08 $
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
import org.apgrid.grpc.ng.NgProtocolRequest;
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
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.dataencode.NgEncodeData.NG_COMPRESS_ZLIB;
import static org.apgrid.grpc.ng.NgGlobals.smallBufferSize;
import static org.apgrid.grpc.ng.NgParamTypes.NG_TYPE_FILENAME;
import static org.apgrid.grpc.ng.NgParamTypes.NG_TYPE_STRING;


public final class ProtTransferResultReply
extends AbstractReceivableProtocol
implements Protocol {

	private final static String NAME = "TransferResultReply";
	private final static int KIND = 1;
	private final static int TYPE = 0x32;

// move from ProtTransferArgumentRequest.java
    private static final int ENABLE_SKIP    = 0x0100;
    private static final int DIVIDE_DATA_CONTINUE = 0x001;
    private static final int DIVIDE_DATA_LENGTH   = -1;

    private static final int XDR_LIMIT            = 4;
    private static final int ENCODE_HEADER_LEN    = 8;
    private static final int SIZE_OF_NUM_PARAMS   = 4;
    private static final int SIZE_OF_NDIM         = 4;
    private static final int SIZE_OF_SKIP_HEADER  = 16;
    private static final int SIZE_OF_PARAM_HEADER = 24;

    // data for arguments
    private CallContext callContext;
    private byte[][] paramHeader;
    private byte[][] argumentData;
    // list for encoders
    private List encoderList;
    // compress
    private boolean boolCompress = false;
    private int compressThreshold = 0;
    private int paddNBytes = 0;
    private long originalDataLength[];
    private long compressedDataLength[];
    private GrpcTimer timer;
    private double compressRealTime[];
    private double compressCPUTime[];

    // receive info and data
    private int[] numArray;
    private int[] typeArray;
    private int[] modeArray;


	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtTransferResultReply(NgDataInputStream ngdi)
	 throws GrpcException {
		super(KIND, TYPE, ngdi);
	}

	public String getName() { return NAME; }

	private void log(Wire logger, String msg, byte[] data) {
		log(logger, msg, data, data.length);
	}

	private void log(Wire logger, String msg, byte[] data, int length) {
		logger.logCommLog(this, msg, data, length);
	}

	private int getNumPadding(int length) {
		int mod = length % XDR_LIMIT;
		if (mod == 0) { return mod; }
		return XDR_LIMIT - mod;
	}

	public void parseParam(Wire wire, NgProtocolRequest prot)
	 throws GrpcException {
		ProtTransferResultRequest resultRequest =
			(ProtTransferResultRequest) prot;
		CallContext callContext = 
			resultRequest.getCallContext();

		this.paramData = new byte[SIZE_OF_NUM_PARAMS];
		wire.receiveBytes(this.paramData);
		log(wire, "Number of Results.", this.paramData);

		XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(this.paramData));

		// count of Result 
		int numParams = xi.readInt();

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

		// read Result 
		int resultIndex = 0;
		for (int i = 0; i < numParams; i++) {
			paramHeader[i] = new byte[SIZE_OF_PARAM_HEADER];
			wire.receiveBytes(paramHeader[i], 0, SIZE_OF_PARAM_HEADER);
			log(wire, "Argument Parameter.", paramHeader[i]);

			xi = new XDRInputStream(new ByteArrayInputStream(paramHeader[i]));

			int resultID = xi.readInt();   // number of Result 
			int type = xi.readInt();       // type 
			int mode = xi.readInt();       // mode 
			int encodeType = xi.readInt(); // encode 
			int nElements = xi.readInt();  // number of Elemnt 
			int length = xi.readInt();     // length of data
			
			// set index information 
			this.numArray[i] = resultID;
			
			// read skip info 
			Vector skipEncodeInfo = new Vector();
			if ((encodeType & ENABLE_SKIP) != 0) {
				byte[] nDimBuffer = new byte[SIZE_OF_NDIM];
				wire.receiveBytes(nDimBuffer, 0, SIZE_OF_NDIM);
				log(wire, "Number of Dimension.", nDimBuffer);

				xi = new XDRInputStream(new ByteArrayInputStream(nDimBuffer));
				int nDims = xi.readInt();
				try {
					xi.close();
				} catch (IOException e) {
					throw new NgIOException(e);
				}

				byte[] skipHeader = new byte[SIZE_OF_SKIP_HEADER * nDims];
				wire.receiveBytes(skipHeader, 0, SIZE_OF_SKIP_HEADER * nDims);
				log(wire, "Skip Info.", skipHeader);

				xi = new XDRInputStream(new ByteArrayInputStream(skipHeader));
				int arrayShapes[][] = new int[nDims][4];
				// read arrayShape 
				for (int j = 0; j < nDims; j++) {
					arrayShapes[j][0] = xi.readInt(); // size 
					arrayShapes[j][1] = xi.readInt(); // start 
					arrayShapes[j][2] = xi.readInt(); // end 
					arrayShapes[j][3] = xi.readInt(); // skip 
				}
				try {
					xi.close();
				} catch (IOException e) {
					throw new NgIOException(e);
				}

				int argType[] = new int[1];
				argType[0] = type;

				skipEncodeInfo.add(argType);
				skipEncodeInfo.add(arrayShapes);

				// reset paramHeader
				byte[] orig_paramHeader = paramHeader[i];
				paramHeader[i] =
					new byte[paramHeader[i].length + nDimBuffer.length
							+ skipHeader.length];
				System.arraycopy(orig_paramHeader, 0, paramHeader[i], 0,
					orig_paramHeader.length);
				System.arraycopy(nDimBuffer, 0,
					paramHeader[i], orig_paramHeader.length,
					nDimBuffer.length);
				System.arraycopy(skipHeader, 0,
					paramHeader[i], orig_paramHeader.length + nDimBuffer.length,
					skipHeader.length);
			}

			// read data 
			byte[] buffer = null;
			if (length == DIVIDE_DATA_LENGTH) {
				// the data is divided 
				int loopCount = 1;
				RemoteMethodArg rma = null;
				if (callContext != null) {
					RemoteMethodInfo rmi = callContext.getRemoteMethodInfo();
					rma = (RemoteMethodArg) rmi.getArgs().get(resultID - 1);
					if ((type == NG_TYPE_FILENAME) && (rma.getNDims() > 0)) {
						loopCount = nElements;
					}
				} // else if callContext == null?

				OutputStream os = null;
				// loop if it's array of filename type 
				for (int j = 0; j < loopCount; j++) {
					// create OutputStream for output data 
					if (type == NG_TYPE_FILENAME) {
						List args = callContext.getArgs();
						try {
							if ((rma != null) &&
								(rma.getNDims() > 0)) {
								String[] fileArray =
									 (String []) args.get(resultID - 1);
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

					// flag for divide data 
					boolean divideContinue = false;
					// length of divided data 
					int divide_length = 0;
					// InputStream 
					ByteArrayInputStream divide_bi = null;
					XDRInputStream divide_xi = null;

					// read divided/compressed data 
					do {
						// read encode type 
						int divideEncodeType = readIntFromWire(wire);
						// length of padding 
						int nPadd = 0;
						
						// Is it compressed? 
						if (divideEncodeType == NgEncodeData.NG_COMPRESS_ZLIB) {
							int compressed_length = readIntFromWire(wire);
							
							// read compressed data 
							byte compresssedData[] = new byte[compressed_length];
							wire.receiveBytes(compresssedData, 0, compressed_length);
							log(wire, "Compressed Data", compresssedData);
							// get length of padding 
							int remainder = compressed_length % XDR_LIMIT;
							if (remainder != 0) {
								nPadd = XDR_LIMIT - remainder;
							}
							
							// initialize time of compress/decompress 
							if (this.compressedDataLength[i] == -1) {
								this.compressedDataLength[i] = 0;
								this.originalDataLength[i] = 0;
								this.compressRealTime[i] = 0;
								this.compressCPUTime[i] = 0;
							}
							
							// decompress 
							this.compressedDataLength[i] += compresssedData.length;
							timer.start();
							compresssedData = NgEncodeData.decode(
								NgEncodeData.NG_COMPRESS_ZLIB, compresssedData, null);
							this.compressRealTime[i] += timer.getElapsedTime();
							this.originalDataLength[i] += compresssedData.length;

							// read encode type 
							divide_bi = new ByteArrayInputStream(compresssedData);
							divide_xi = new XDRInputStream(divide_bi);
							
							// get encode type 
							divideEncodeType = divide_xi.readInt();
							
							// get continue flag 
							divideContinue =
								divide_xi.readInt() == DIVIDE_DATA_CONTINUE ? true : false;
							// get length of divided data 
							divide_length = divide_xi.readInt();
						} else if (divideEncodeType == NgEncodeData.NG_DATA_DIVIDE){
							// get continue flag 
							divideContinue =
								readIntFromWire(wire) == DIVIDE_DATA_CONTINUE ? true : false;
							// get length of divide data 
							divide_length = readIntFromWire(wire);

							// read divided data 
							byte dividedData[] = new byte[divide_length];
							wire.receiveBytes(dividedData, 0, divide_length);
							log(wire, "Divided Data", dividedData);
							// get length of padding 
							int remainder = divide_length % XDR_LIMIT;
							if (remainder != 0) {
								nPadd = XDR_LIMIT - remainder;
							}

							// read encode type 
							divide_bi = new ByteArrayInputStream(dividedData);
							divide_xi = new XDRInputStream(divide_bi);
						}
						
						// check if it's valid 
						if (divideEncodeType != NgEncodeData.NG_DATA_DIVIDE) {
							throw new NgException("Invalid divided data.");
						}
						
						// read divided data 
						byte[] dividedData = new byte[divide_length];
						divide_xi.readBytes(dividedData, 0, divide_length);
						
						// close InputStream 
						try {
							divide_bi.close();
						} catch (IOException e2) {
							throw new NgIOException(e2);
						}

						// read padding 
						if (nPadd != 0) {
							byte[] padding = new byte[nPadd];
							wire.receiveBytes(padding);
							log(wire, "padding.",padding);
							paddNBytes += nPadd;
						}
						
						try {
							// write byte[] to OutputStream 
							os.write(dividedData);
						} catch (IOException e2) {
							throw new NgIOException(e2);
						}
					} while (divideContinue);
					
					// merge all of data 
					try {
						os.close();
					} catch (IOException e1) {
						throw new NgIOException(e1);
					}
				}
				
				if (type == NG_TYPE_FILENAME) {
					continue; // file transfer is completed 
				} else {
					// merged data, process another decode routine 
					ByteArrayOutputStream bos = (ByteArrayOutputStream)os;
					buffer = bos.toByteArray();
				}
				// end of `if (length == DIVIDE_DATA_LENGTH)'
			} else if (type == NG_TYPE_FILENAME) {
				// length == DIVIDE_DATA_LENGTH
				int loopCount = 1;
				RemoteMethodInfo rmi = callContext.getRemoteMethodInfo();
				RemoteMethodArg rma =
					(RemoteMethodArg)rmi.getArgs().get(resultID-1);
				if (rma.getNDims() > 0) {
					loopCount = nElements;
				}

				List args = callContext.getArgs();
				for (int j = 0; j < loopCount; j++) {
					// create OutputStream for output data 
					OutputStream fos = null;
					int headerBufferSize = NgEncodeDataRawFileInputStream.FILE_ENCODE_RAW_HEADER_LEN;
					try {
						if (rma.getNDims() > 0) {
							String[] fileArray =
								(String [])args.get(resultID - 1);
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
					
					// filename type (not divide) 
					byte[] headerBuffer = new byte[headerBufferSize];
					wire.receiveBytes(headerBuffer);
					log(wire, "Filename Data", headerBuffer, headerBuffer.length);

					try {
						fos.write(headerBuffer);
					} catch (IOException e2) {
						throw new NgIOException(e2);
					}
					// get size of the file 
					NgEncodeDataFileTypeOutputStream ngfos =
						(NgEncodeDataFileTypeOutputStream)(fos);
					int fileLength = ngfos.getLength();
					int restOfData = fileLength;
					
					// read data from stream and write it into file 
					try {
						byte fileOutputBuffer[] = null;
						do {
							// prepare buffer 
							int bufferSize = NgGlobals.fileBufferSize;
							if (bufferSize >= restOfData) {
								bufferSize = restOfData;
							}
							fileOutputBuffer = new byte[bufferSize];
							
							// read data from wire 
							wire.receiveBytes(fileOutputBuffer);
							
							// write data into the file 
							fos.write(fileOutputBuffer);
							log(wire, "Filename Data.", fileOutputBuffer,
								restOfData > bufferSize ? bufferSize : restOfData);
							restOfData -= bufferSize;
						} while (restOfData > 0);

						// close FileOutputStream 
						fos.close();

						// read padding 
						int remainder = fileLength % XDR_LIMIT;
						if (remainder != 0) {
							int npad = XDR_LIMIT - remainder;
							byte[] padding = new byte[npad];
							wire.receiveBytes(padding);
							log(wire, "padding", padding);
							paddNBytes += npad;
						}
					} catch (IOException e) {
						throw new NgIOException(e);
					}
				}
				
				continue; // file transfer is completed 
			} else {
				buffer = new byte[length];
				wire.receiveBytes(buffer, 0, length);
				log(wire, "Result Data", buffer);
			}

			// decode data 
			xi = new XDRInputStream(new ByteArrayInputStream(buffer));
			// check if it's RAW? 
			if (xi.readInt() == NgEncodeData.NG_DATA_REPRESENT_RAW) {
				for (int j=0; j < buffer.length - ENCODE_HEADER_LEN; j++) {
					buffer[j] = buffer[j + ENCODE_HEADER_LEN];
				}
				for (int j=buffer.length - ENCODE_HEADER_LEN;
					 j < buffer.length; j++) {
					buffer[j] = 0;
				}
			} else {
				this.compressedDataLength[i] =
					buffer.length - ENCODE_HEADER_LEN;
				timer.start();
				buffer = decodeData(buffer, null);
				this.compressRealTime[i] = timer.getElapsedTime();
				this.compressCPUTime[i] = 0;
				this.originalDataLength[i] =
					buffer.length + ENCODE_HEADER_LEN;
			}

			try {
				xi.close();
			} catch (IOException e1) {
				throw new NgIOException(e1);
			}
			// decoding XDR moved to NgParamTypes 

			// skip 
			if ((type != NG_TYPE_STRING) && ((encodeType & ENABLE_SKIP) != 0)) {
				buffer = decodeDataSkip(buffer, skipEncodeInfo);
			}
			
			// set argumentdata 
			this.argumentData[resultIndex++] = buffer;

			// read padding 
			if ((length != DIVIDE_DATA_LENGTH) && ((length % 4) != 0)) {
				int nPad = 4 - (length % 4);
				byte[] dummyBytes = new byte[nPad];
				wire.receiveBytes(dummyBytes, 0, nPad);
				log(wire, "padding", dummyBytes);
			}
		} // end of `for (int i = 0; i < numParams; i++)' 
	}

	// move from ProtTransferArgumentRequest.java
    protected int readIntFromWire(Wire wire) throws GrpcException {
        byte[] buffer = new byte[XDR_LIMIT];
        wire.receiveBytes(buffer, 0, XDR_LIMIT);
        log(wire, "Read Integer", buffer);

        XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(buffer));

        // get int variable
        int returnValue = xi.readInt();

        try {
            xi.close();
        } catch (IOException e1) {
            throw new NgIOException(e1);
        }

        return returnValue;
   }

	// move from ProtTransferArgumentRequest.java
    protected byte[] decodeData(byte[] originalData, List encodeInfo)
     throws GrpcException {
        byte[] buffer = originalData;
        while (true) {
            XDRInputStream xi =
				new XDRInputStream(new ByteArrayInputStream(buffer));
            int decodeType = xi.readInt();
            int dataLength = xi.readInt();
            byte[] targetBuffer = new byte[buffer.length - ENCODE_HEADER_LEN];
            try {
                xi.read(targetBuffer, 0, buffer.length - ENCODE_HEADER_LEN);
            } catch (IOException e) {
                throw new NgIOException(e);
            }
            if (decodeType == NG_COMPRESS_ZLIB) {
                this.boolCompress = true;
            }
            buffer = NgEncodeData.decode(decodeType, targetBuffer, encodeInfo);
            if (decodeType == NgEncodeData.NG_DATA_REPRESENT_RAW) {
                break;
            }
        }
        return buffer;
    }

	// move from ProtTransferArgumentRequest.java
    protected byte[] decodeDataSkip(byte[] originalData, List encodeInfo)
        throws GrpcException {
        return NgEncodeData.decode(
            NgEncodeData.NG_THIN_OUT_SKIP, originalData, encodeInfo);
    }

	// move from ProtTransferArgumentRequest.java
    private int getMaxResultID() {
        if (numArray == null) { return 0; }
        int maxResultID = 0;
        for (int i = 0; i < numArray.length; i++) {
            if (maxResultID < numArray[i]) {
                maxResultID = numArray[i];
            }
        }
        return maxResultID;
    }

	/**
	 * @return
	 */
	public byte[][] getResultData() {
		return argumentData;
	}

	// move from ProtTransferArgumentRequest
	public int getOriginalDataTotalLength() {
		int ret = 0;
		for (int i = 0; i < this.originalDataLength.length; i++) {
			if (this.originalDataLength[i] != -1) {
				ret += this.originalDataLength[i];
			}
		}
		return ret;
	}

	// move from  ProtTransferArgumentRequest
	public long[] getOriginalDataLength() {
		if (this.originalDataLength.length < getMaxResultID()) {
			// reset originalDataLength array
			long[] newOriginalDataLength = new long[getMaxResultID()];
			for (int i = 0; i < newOriginalDataLength.length; i++) {
				newOriginalDataLength[i] = -1;
			}
			for (int i = 0; i < this.originalDataLength.length; i++) {
				newOriginalDataLength[numArray[i] - 1] =
				this.originalDataLength[i];
			}
			// set new originalDataLength array
			this.originalDataLength = newOriginalDataLength;
		}
		return this.originalDataLength;
	}

	// move from  ProtTransferArgumentRequest
	public long[] getConvertedDataLength() {
		if (this.compressedDataLength.length < getMaxResultID()) {
			// reset compressedDataLength array
			long[] newConvertedDataLength = new long[getMaxResultID()];
			for (int i = 0; i < newConvertedDataLength.length; i++) {
				newConvertedDataLength[i] = -1;
			}
			for (int i = 0; i < this.compressedDataLength.length; i++) {
				newConvertedDataLength[numArray[i] - 1] =
				this.compressedDataLength[i];
			}
			// set new originalDataLength array
			this.compressedDataLength = newConvertedDataLength;
		}
		return this.compressedDataLength;
	}

	// move from  ProtTransferArgumentRequest
    public double[] getConvertRealTime() {
        if (this.compressRealTime.length < getMaxResultID()) {
            // reset compressTime array
            double[] newCompressTime = new double[getMaxResultID()];
            for (int i = 0; i < newCompressTime.length; i++) {
                newCompressTime[i] = -1;
            }
            for (int i = 0; i < this.compressRealTime.length; i++) {
                newCompressTime[numArray[i] - 1] = this.compressRealTime[i];
            }
            // set new originalDataLength array
            this.compressRealTime = newCompressTime;
        }
        return this.compressRealTime;
    }

	// move from  ProtTransferArgumentRequest
    public double[] getConvertCPUTime() {
        if (this.compressRealTime.length < getMaxResultID()) {
            // reset compressTime array
            double[] newCompressTime = new double[getMaxResultID()];
            for (int i = 0; i < newCompressTime.length; i++) {
                newCompressTime[i] = -1;
            }
            for (int i = 0; i < this.compressRealTime.length; i++) {
                newCompressTime[numArray[i] - 1] = this.compressRealTime[i];
            }
            // set new originalDataLength array
            this.compressRealTime = newCompressTime;
        }
        return this.compressRealTime;
    }

	// move from  ProtTransferArgumentRequest
	public int getConvertedDataTotalLength() {
		int compressedLength = 0;
		for (int i = 0; i < this.compressedDataLength.length; i++) {
			if (this.compressedDataLength[i] != -1) {
				compressedLength += this.compressedDataLength[i];
			}
		}
		return compressedLength;
	}

}

