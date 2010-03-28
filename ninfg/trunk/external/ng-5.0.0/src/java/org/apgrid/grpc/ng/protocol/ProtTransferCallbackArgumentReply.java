/*
 * $RCSfile: ProtTransferCallbackArgumentReply.java,v $ $Revision: 1.10 $ $Date: 2008/02/07 08:17:43 $
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
import java.io.IOException;
import java.io.OutputStream;
import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgGlobals;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.Protocol;
import org.apgrid.grpc.ng.NgProtocolRequest;
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.Wire;
import org.apgrid.grpc.ng.info.RemoteMethodArg;
import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.apgrid.grpc.ng.dataencode.NgEncodeData;
import org.apgrid.grpc.util.GrpcTimer;
import org.gridforum.gridrpc.GrpcException;

public final class ProtTransferCallbackArgumentReply
extends AbstractReceivableProtocol
implements Protocol {

	private final static String NAME = "TransferCallbackArgumentReply";
	private final static int KIND = 1;
	private final static int TYPE = 0x33;

	private final static int SIZE_OF_CALLBACKINFO = 4;

	private int callbackID;

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
	public ProtTransferCallbackArgumentReply(NgDataInputStream ngdi)
	 throws GrpcException {
		super(KIND, TYPE, ngdi);
	}
	
	public String getName() { return NAME; }

	/**
	 * @return
	 * @throws GrpcException
	 */
	public byte[][] getArgumentData() throws GrpcException {
		return argumentData;
	}

	/*
	 */
	public void parseParam(Wire wire, NgProtocolRequest prot)
	 throws GrpcException {

		byte[] callbackInfo = new byte[SIZE_OF_CALLBACKINFO];
		wire.receiveBytes(callbackInfo);
		log(wire, "Callback Info", callbackInfo);

		XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(callbackInfo));

		this.callbackID = xi.readInt();  // ID of callback 

		try {
			xi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		// parse Parameter for Callback argument reply
		parseParam2(wire, prot);

		// reset paramData 
		byte[] num_of_param = this.paramData;
		this.paramData = new byte[callbackInfo.length + num_of_param.length]; 
		for (int i = 0; i < callbackInfo.length; i++) {
			this.paramData[i] = callbackInfo[i];
		}
		for (int i = 0; i < num_of_param.length; i++) {
			this.paramData[callbackInfo.length + i] = num_of_param[i];
		}
	}

	private boolean specifiedDataSkip(int encodeType) {
		return ((encodeType & ENABLE_SKIP) != 0);
	}

	private boolean isTypeFilename(int type) {
		return type == NgParamTypes.NG_TYPE_FILENAME;
	}

	private boolean isTypeString(int type) {
		return type == NgParamTypes.NG_TYPE_STRING;
	}

	private boolean isDataCompressed(int encodeType) {
		return encodeType == NgEncodeData.NG_COMPRESS_ZLIB;
	}

	private boolean isTypeDivide(int encodeType) {
		return encodeType == NgEncodeData.NG_DATA_DIVIDE;
	}

	private boolean isDataDivided(int length) {
		return length == DIVIDE_DATA_LENGTH;
	}

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


	// int i influence by(?) paramHeader
	private List makeSkipInfo(Wire wire, int type, int i) throws GrpcException {
		byte[] nDimBuffer = new byte[SIZE_OF_NDIM];
		wire.receiveBytes(nDimBuffer, 0, SIZE_OF_NDIM);
		log(wire, "Number of Dimension.", nDimBuffer);

		XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(nDimBuffer));

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
		int [][] arrayShapes = new int[nDims][4];
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

		int [] argType = new int[1];
		argType[0] = type;

		List skipEncodeInfo = new Vector();
		skipEncodeInfo.add(argType);
		skipEncodeInfo.add(arrayShapes);

		// reset paramHeader (need?)
		byte[] orig_paramHeader = paramHeader[i];
		paramHeader[i] =
			new byte[paramHeader[i].length + nDimBuffer.length 
				+ skipHeader.length];
		System.arraycopy(orig_paramHeader, 0, paramHeader[i], 0,
			orig_paramHeader.length);
		System.arraycopy(nDimBuffer, 0, paramHeader[i], orig_paramHeader.length,
			nDimBuffer.length);
		System.arraycopy(skipHeader, 0,
			paramHeader[i], orig_paramHeader.length + nDimBuffer.length,
			skipHeader.length);

		return skipEncodeInfo;
	}

	private byte[] decompressData(Wire wire, int compressed_length, int i)
	 throws GrpcException {
		// read compressed data
		byte [] data = new byte[compressed_length];
		wire.receiveBytes(data, 0, compressed_length);
		log(wire, "Compressed Data", data);

		// initialize time of compress/decompress 
		if (this.compressedDataLength[i] == -1) {
			this.compressedDataLength[i] = 0;
			this.originalDataLength[i] = 0;
			this.compressRealTime[i] = 0;
			this.compressCPUTime[i] = 0;
		}

		// decompress 
		this.compressedDataLength[i] += data.length;
		timer.start();
		byte [] ret =
			NgEncodeData.decode(NgEncodeData.NG_COMPRESS_ZLIB, data, null);
		this.compressRealTime[i] += timer.getElapsedTime();
		this.originalDataLength[i] += ret.length;

		return ret;
	}

	private void parseParam2(Wire wire, NgProtocolRequest prot)
	 throws GrpcException {

		this.paramData = new byte[SIZE_OF_NUM_PARAMS];
		wire.receiveBytes(this.paramData);
		log(wire, "Number of Results.", this.paramData);

		XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(this.paramData));

		// count of Result 
		int numParams = xi.readInt();

		// init begin
		this.argumentData = new byte[numParams][];
		this.numArray     = new int[numParams];
		this.typeArray    = new int[numParams];
		this.modeArray    = new int[numParams];
		this.paramHeader  = new byte[numParams][];
		this.originalDataLength   = new long[numParams];
		this.compressedDataLength = new long[numParams];
		this.compressRealTime = new double[numParams];
		this.compressCPUTime = new double[numParams];
		for (int i = 0; i < originalDataLength.length; i++) {
			this.originalDataLength[i] = -1;
			this.compressedDataLength[i] = -1;
			this.compressRealTime[i] = -1;
			this.compressCPUTime[i] = -1;
		}
		this.timer = new GrpcTimer();
		// init end

		// read Result  (Arguments)
		int resultIndex = 0;
		for (int i = 0; i < numParams; i++) {
			paramHeader[i] = new byte[SIZE_OF_PARAM_HEADER]; // 24
			wire.receiveBytes(paramHeader[i], 0, SIZE_OF_PARAM_HEADER);
			log(wire, "Argument Parameter.", paramHeader[i]);

			xi = new XDRInputStream(new ByteArrayInputStream(paramHeader[i]));
			int resultID   = xi.readInt();// number of Result 
			int type       = xi.readInt();// type 
			int mode       = xi.readInt();// mode 
			int encodeType = xi.readInt();// encode 
			int nElements  = xi.readInt();// number of Elemnt 
			int length     = xi.readInt();// length of data

			// set index information 
			this.numArray[i] = resultID;
			
			// read skip info 
			List skipEncodeInfo = null;
			if ( specifiedDataSkip(encodeType) ) {
				skipEncodeInfo = makeSkipInfo(wire, type, i);
			}

			// read data 
			byte[] buffer = null;
			if ( isDataDivided(length) ) {
				// the data is divided 
				int loopCount = 1;
				OutputStream os = null;
				// loop if it's array of filename type 
				for (int j = 0; j < loopCount; j++) {
					// create OutputStream for output data 
					if ( isTypeFilename(type) ) {
						// doesn't implement filetype argument for callback
						throw new UnsupportedOperationException("");
					} else {
						os = new ByteArrayOutputStream(NgGlobals.argBufferSize);
					}

					// flag for divide data 
					boolean divideContinue = false;
					// length of divided data 
					int divide_length = 0;
					// InputStream 
					//ByteArrayInputStream divide_bi = null;
					XDRInputStream divide_xi = null;

					// read divided/compressed data 
					do {
						// read encode type 
						int divideEncodeType = readIntFromWire(wire);
						// length of padding 
						int nPadd = 0;

						// Is it compressed? 
						if ( isDataCompressed(divideEncodeType) ) {

							int compressed_length = readIntFromWire(wire);
							nPadd = getNumPadding(compressed_length);

							byte[] decompressedData =
								decompressData(wire, compressed_length, i);

							// read encode type 
							divide_xi =
								new XDRInputStream(
									new ByteArrayInputStream(decompressedData));	
							// get encode type 
							divideEncodeType = divide_xi.readInt();

							// get continue flag 
							divideContinue =
								divide_xi.readInt() == DIVIDE_DATA_CONTINUE ? true : false;
							// get length of divided data 
							divide_length = divide_xi.readInt();
						} else if ( isTypeDivide( divideEncodeType) ){
							// get continue flag 
							divideContinue =
								readIntFromWire(wire) == DIVIDE_DATA_CONTINUE ? true : false;
							// get length of divide data 
							divide_length = readIntFromWire(wire);
							// get length of padding 
							nPadd = getNumPadding(divide_length);

							// read divided data 
							byte dividedData[] = new byte[divide_length];
							wire.receiveBytes(dividedData, 0, divide_length);
							log(wire, "Divided Data", dividedData);

							divide_xi =
								new XDRInputStream(
									new ByteArrayInputStream(dividedData));
						}

						// check if it's valid 
						if ( ! isTypeDivide(divideEncodeType) ) {
							throw new NgException("Invalid divided data.");
						}

						// read divided data 
						byte[] dividedData = new byte[divide_length];
						divide_xi.readBytes(dividedData, 0, divide_length);

						// close InputStream 
						try {
							divide_xi.close();
						} catch (IOException e2) {
							throw new NgIOException(e2);
						}

						// read padding 
						if (nPadd != 0) {
							byte [] padding = new byte[nPadd];
							wire.receiveBytes(padding);
							log(wire, "padding", padding);
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

				if ( isTypeFilename(type) ) {
					// doesn't implement filetype argument for callback
					throw new UnsupportedOperationException("");
				} else {
					// merged data, process another decode routine 
					ByteArrayOutputStream bos = (ByteArrayOutputStream)os;
					buffer = bos.toByteArray();
				}
			} else if ( isTypeFilename(type) ) {
				// doesn't implement filetype argument for callback
				throw new UnsupportedOperationException("");
			} else {
				buffer = new byte [length];
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
			if ( ( ! isTypeString(type)) && specifiedDataSkip(encodeType) ) {
				buffer = decodeDataSkip(buffer, skipEncodeInfo);
			}

			// set argumentdata
			this.argumentData[resultIndex++] = buffer;

			// read padding 
			if ( ! isDataDivided(length) ) {
				int nPad = getNumPadding(length);
				if (nPad != 0) {
					byte[] padding = new byte[nPad];
					wire.receiveBytes(padding, 0, nPad);
					log(wire, "padding", padding);
				}
			}
		} // end of `for (int i = 0; i < numParams; i++)' 
	}

    private int readIntFromWire(Wire wire) throws GrpcException {
        byte[] buffer = new byte[XDR_LIMIT];
        wire.receiveBytes(buffer, 0, XDR_LIMIT);
        log(wire, "Read Integer", buffer);

        XDRInputStream xi =
			new XDRInputStream( new ByteArrayInputStream(buffer) );

        int returnValue = xi.readInt(); // get int variable

        try {
            xi.close();
        } catch (IOException e1) {
            throw new NgIOException(e1);
        }
        return returnValue;
   }

    private byte[] decodeData(byte[] originalData, List encodeInfo)
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

            if ( isDataCompressed(decodeType))
				this.boolCompress = true;

            buffer = NgEncodeData.decode(decodeType, targetBuffer, encodeInfo);
            if (decodeType == NgEncodeData.NG_DATA_REPRESENT_RAW) {
                break;
            }
        }
        return buffer;
    }

	private byte[] decodeDataSkip(byte[] originalData, List encodeInfo)
	 throws GrpcException {
		return NgEncodeData.decode(
			NgEncodeData.NG_THIN_OUT_SKIP, originalData, encodeInfo);
	}

}

