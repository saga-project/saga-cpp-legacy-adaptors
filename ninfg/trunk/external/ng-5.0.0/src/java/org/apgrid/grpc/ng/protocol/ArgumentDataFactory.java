/*
 * $RCSfile: ArgumentDataFactory.java,v $ $Revision: 1.4 $ $Date: 2008/02/07 08:17:43 $
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

import java.io.IOException;
import java.io.File;
import java.util.List;
import java.util.ArrayList;
import java.util.Vector;

import org.apgrid.grpc.ng.info.RemoteMethodInfo;
import org.apgrid.grpc.ng.info.RemoteMethodArg;
import org.apgrid.grpc.ng.info.RemoteMethodArgSubScript;
import org.apgrid.grpc.ng.CallContext;
import org.apgrid.grpc.ng.CompressInfo;
import org.apgrid.grpc.ng.NgByteArrayOutputStream;
import org.apgrid.grpc.ng.Version;
import org.apgrid.grpc.ng.NgExpression;
import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgExecRemoteMethodException;
import org.apgrid.grpc.ng.dataencode.NgEncodeData;
import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.NgConnectInfo;
import org.apgrid.grpc.util.GrpcTimer;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.NgParamTypes.NG_TYPE_STRING;
import static org.apgrid.grpc.ng.NgParamTypes.NG_TYPE_FILENAME;
import static org.apgrid.grpc.ng.NgParamTypes.NG_MODE_IN;
import static org.apgrid.grpc.ng.NgParamTypes.NG_MODE_INOUT;
import static org.apgrid.grpc.ng.dataencode.NgEncodeData.NG_COMPRESS_ZLIB;
import static org.apgrid.grpc.ng.NgGlobals.smallBufferSize;

class ArgumentDataFactory {

	private static final int REPRESENT_XDR  = 0x0002;
	private static final int ENABLE_SKIP    = 0x0100;
	private static final int DIVIDE_DATA_LENGTH = -1;
	private static final int ENCODE_HEADER_LEN  = 8;
	private static final int XDR_LIMIT          = 4;

	public static ArgumentData newInstance(
	 int index,
	 RemoteMethodInfo rmi,
	 RemoteMethodArg rma,
	 CallContext callContext,
	 Version version,
	 List encoderList,
	 CompressInfo aCompressInfo,
	 int blockSize)
	 throws GrpcException {

		// prepare this method process.
		boolean is_array_arg_and_shrinking =
			isArrayShrinking(rma, rmi.getShrink());
		boolean boolCompress =
			aCompressInfo.getCompress().equals(CompressInfo.COMPRESS_ZLIB);
		int compressThreshold = aCompressInfo.getCompressThreshold();

		boolean arg_is_compressed = false;
		long originalDataLength   = -1;
		long compressedDataLength = -1;
		double compressRealTime   = -1;
		double compressCPUTime    = -1;

		int nArg = index + 1;     // Argument Number
		int type = rma.getType(); // Argument Type
		int mode = rma.getMode(); // Argument Mode
		// optional data
		Object data_of_java = callContext.getArgs().get(index);
		int nDims = rma.getNDims(); // rma.getNDims();

		// Argument encode type 
		int encodeType = REPRESENT_XDR;
		byte[] argData = callContext.getData(index); 
		if (argData == null)
			throw new NullPointerException("callContext.getData(" + String.valueOf(index) +  ") is null");
		if ( is_array_arg_and_shrinking ) {
			encodeType |= ENABLE_SKIP;
			// string type is not support shrinking.
			if (rma.getType() != NG_TYPE_STRING ) {
				argData = encodeDataSkip(rma, argData, rmi,
					callContext.getIntArguments());
			}
		}

		// Number of Elements
		int nElems =
			getNumOfElems(argData.length, rma, index, callContext, version);

		int argumentDataLength = Integer.MIN_VALUE;
		if (version.compareTo(NgConnectInfo.VALID_PROTOCOL_VERSION) < 0) {
			throw new NgException("Invalid Protocol Version " + version);
		}

		// RAW
		retreatData(argData);
		pushTypeOfRAW(argData);
		pushDataLengthByRAW(argData);
		// length of data
		argumentDataLength = argData.length;

		// check if it's able to divide data
		if ( isDivideArgumentDataSpecified(encoderList, blockSize) ) {
			// set length to DIVIDE_DATA_LENGTH(-1)
			argumentDataLength = DIVIDE_DATA_LENGTH;
		} else if ( isTypeFilename( rma.getType() ) && (blockSize == 0) ) {
			// filetype & doesn't specified data division. 
			// (& client protocol version 210 later)
			// transfer data of file via Ninf-G Protocol
			// what the here ENCODE_HEADER_LEN ? RAW? Filename?
			long inputFileLength = ENCODE_HEADER_LEN;
			long totalFileLength =
				getFileLength(rma, index,
					callContext.getArgs(), callContext.getCount(index));
			inputFileLength += totalFileLength;
			if (inputFileLength > Integer.MAX_VALUE) {
				throw new NgException(
					"can't transfer data over 2GB without division.");
			}
			// set length of file length 
			argumentDataLength = (int)inputFileLength;
		} else if (boolCompress
			&& (argumentDataLength > compressThreshold)) {
			// convert data (RAW -> compress) 
			// save original data length 
			originalDataLength = argumentDataLength;
			GrpcTimer timer = new GrpcTimer();
			timer.start();
			argData = encodeData(NG_COMPRESS_ZLIB, argData, encoderList);
			compressRealTime = timer.getElapsedTime();
			compressCPUTime  = 0;

			argData = insertConvertInfo(NG_COMPRESS_ZLIB, argData);
			arg_is_compressed = true;

			// save compressed data length
			// ENCODE_HEADER_LEN == compress header length (?)
			compressedDataLength = argData.length - ENCODE_HEADER_LEN;

			// set length to compressed data length 
			argumentDataLength = argData.length;
		}

		// create skip header info
		SkipInfoHeader skipInfo = null;
		if ( is_array_arg_and_shrinking ) {
			skipInfo =
				createSkipHeaderInfo(rma, callContext.getIntArguments());
		}

		ArgumentData ret = new ArgumentData(nArg, type, mode, encodeType,
			nElems, argData, argumentDataLength, data_of_java, nDims, skipInfo);
		if ( arg_is_compressed ) {
			ret.setCompressInfo(originalDataLength, compressedDataLength,
				compressRealTime, compressCPUTime);
		}

		return ret;
	}


	private static boolean isDivideArgumentDataSpecified(List encoderList,
	 int blockSize) {
		boolean specified_divide_argument_data = (blockSize != 0);
		boolean data_division_is_possible =
			encoderList.contains(NgEncodeData.NG_DATA_DIVIDE);

		return specified_divide_argument_data && data_division_is_possible;
	}

	private static int getNumOfElems(int length, RemoteMethodArg rma, int index,
	 CallContext callContext, Version version) {
		if ( getSizeOfType(rma) != 0) {
			return (length - ENCODE_HEADER_LEN) / getSizeOfType(rma);
		}
		if ( isTypeFilename(rma.getType()) &&
			(callContext.getCount(index) == 0) &&
			(callContext.getArgs().get(index) == null) ) {
			return 1; // expect protocol version 210 or later (220 =< target)
		} else {
			return callContext.getCount(index);
		}
	}

	private static SkipInfoHeader createSkipHeaderInfo(
	 RemoteMethodArg rma, final int []scalarArgs)
	 throws GrpcException {
		// number of Dimension 
		int nDims = rma.getNDims();
		SkipInfoHeader skipHeader = new SkipInfoHeader(nDims);
		for (int i = 0; i < nDims; i++) {
			RemoteMethodArgSubScript argScript =
				rma.getRemoteMethodArgSubscript(i);

			// get skip info 
			int size  = (int) argScript.getSize().calc(scalarArgs);
			int start = (int) argScript.getStart().calc(scalarArgs);
			int end   = (int) argScript.getEnd().calc(scalarArgs);
			int skip  = (int) argScript.getSkip().calc(scalarArgs);
			if ( end == 0) { end = size; }// check end 
			if (skip == 0) { skip = 1; } // check skip 

			skipHeader.add(size, start, end, skip);
		}
		return skipHeader;
	}

	private static long getFileLength(
	 RemoteMethodArg rma, int index, List args, int loopCount) {
		long retLength = 0;
		//List args = callContext.getArgs(); // get Args by Java

		if (rma.getNDims() > 0) {
			// return length of files
			//int loopCount = callContext.getCount(index);
			String[] fileArray = (String [] )args.get(index);
			return calcLengthOfFiles(fileArray, loopCount, rma.getMode());
		}

		// return length of file
		String inputFile =(String)args.get(index);
		// append length of file
		retLength += getLengthOfFile(inputFile, rma.getMode());
		// append padding length
		int rem = (int)(retLength % XDR_LIMIT);
		if (rem != 0) {
			int npad = XDR_LIMIT - rem;
			retLength += npad;
		}
		return retLength;
	}

	private static long calcLengthOfFiles(String [] fileArray, int nElems,
	 int mode) {
		long retLength = 0;
		for (int i = 0; i < nElems; i++) {
			String inputFile = fileArray[i];
			// append length of file
			retLength += getLengthOfFile(inputFile, mode);
			// append padding length
			int rem = (int)(retLength % XDR_LIMIT);
			if (rem != 0) {
				int npad = XDR_LIMIT - rem;
				retLength += npad;
			}
		}
		return retLength;
	}

	private static long getLengthOfFile(String inputFile, int mode) {
		if (inputFile == null || (inputFile.length() == 0)) {
			// parameter(file data) length is 0. 
			// return filename type header length.
			return ENCODE_HEADER_LEN; 
		} else {
			if ( isModeIN_INOUT(mode) ) {
				// return parameter(file data) length + 
				//  filename type header length
				return (new File(inputFile).length() + ENCODE_HEADER_LEN);
			} else {
				return ENCODE_HEADER_LEN;
			}
		}
	}

	// depend on argData behind surplus 8 byte.
	private static void retreatData(byte[] argData) {
		for (int i = argData.length - 1; i > 7; i--) {
			argData[i] = argData[i-ENCODE_HEADER_LEN];
		}
	}

	private static void pushTypeOfRAW(byte[] argData) {
		argData[0] = 0;
		argData[1] = 0;
		argData[2] = 0;
		argData[3] = NgEncodeData.NG_DATA_REPRESENT_RAW;
	}

	private static void pushDataLengthByRAW(byte [] argData) {
		int argLength = argData.length - ENCODE_HEADER_LEN;
		argData[4] = (byte)((argLength >>> 24) & 0xFF);
		argData[5] = (byte)((argLength >>> 16) & 0xFF);
		argData[6] = (byte)((argLength >>>  8) & 0xFF);
		argData[7] = (byte)((argLength >>>  0) & 0xFF);
	}

	private static boolean unsupportedShrinking(RemoteMethodArg rma) {
		return (rma.getType() == NG_TYPE_STRING);
	}

	private static int getSizeOfType(RemoteMethodArg rma) {
		// inherit GrpcException from getSize()
		if (rma == null) throw new NullPointerException("");
		return NgParamTypes.getSize(rma.getType());
	}

	/*
	 * Check remote method argument is multi-dimension & shrink flag is true.
	 */
	private static boolean isArrayShrinking(RemoteMethodArg rma,
	 boolean shrink){
		if (rma == null) { return false; }
		return ((rma.getNDims() > 0) && shrink);
	}

	private static boolean isTypeFilename(int type) {
		return (type == NG_TYPE_FILENAME);
	}

	private static boolean isModeIN_INOUT(int mode) {
		return ((mode == NG_MODE_IN) || (mode == NG_MODE_INOUT)); 
	}
	
	/**
	 */
	private static byte[] insertConvertInfo(int type, byte[] argumentData)
	 throws GrpcException {
		NgByteArrayOutputStream bos =
			new NgByteArrayOutputStream(smallBufferSize);
		try {
			bos.writeInt(type);
			bos.writeInt(argumentData.length);
			bos.writeBytes(argumentData, 0, argumentData.length);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return bos.toByteArray();
	}

	private static byte[] encodeDataSkip(RemoteMethodArg rma,
	 byte buffer[], RemoteMethodInfo rmi, int [] scalarArgs)
	 throws GrpcException {

		// check validation of ArgSubScript
		hasValidSubScript(rmi, scalarArgs);

		// make information for skip
		int [] sizeArray = new int[1];
		sizeArray[0] = getSizeOfType(rma);
		int [][] arrayShapes = new int[rma.getNDims()][4];
		//int [] scalarArgs    = callContext.getIntArguments();
		for (int i = 0; i < arrayShapes.length; i++) {
			RemoteMethodArgSubScript as = rma.getRemoteMethodArgSubscript(i);
			arrayShapes[i][0] = (int)as.getSize().calc(scalarArgs);
			arrayShapes[i][1] = (int)as.getStart().calc(scalarArgs);
			arrayShapes[i][2] = (int)as.getEnd().calc(scalarArgs);
			arrayShapes[i][3] = (int)as.getSkip().calc(scalarArgs);
		}
		List encodeInfo = new Vector();
		encodeInfo.add(sizeArray);
		encodeInfo.add(arrayShapes);

		return NgEncodeData.encode(NgEncodeData.NG_THIN_OUT_SKIP,
			buffer, encodeInfo);
	}

	private static byte[] encodeData(int type, byte[] buffer, List encoderList)
	 throws GrpcException {
		if (! encoderList.contains(Integer.valueOf(type))) {
			throw new NgExecRemoteMethodException(
				"Specified encoder(" + type + ") is not supported...");
		}
		// encode data 
		// #3 argument is not used
		return NgEncodeData.encode(type, buffer, null);
	}

	private static void hasValidSubScript(RemoteMethodInfo rmi,
	 int [] scalarArgs)
	 throws GrpcException {
		//int[] scalarArgs = callContext.getIntArguments();
		List<RemoteMethodArg> listArguments = rmi.getArgs();

		// look up all of params 
		for (int i = 0; i < listArguments.size(); i++) {
			RemoteMethodArg rma = listArguments.get(i);
			// look up all of dims 
			for (int j = 0; j < rma.getNDims(); j++) {
				RemoteMethodArgSubScript argSubscript =
					rma.getRemoteMethodArgSubscript(j);

				// get variable of skip 
				int size  = (int) argSubscript.getSize().calc(scalarArgs);
				int start = (int) argSubscript.getStart().calc(scalarArgs);
				int end   = (int) argSubscript.getEnd().calc(scalarArgs);
				int skip  = (int) argSubscript.getSkip().calc(scalarArgs);

				// check if start,end > size 
				if ((start != 0) && (start > size)) {
					throw new NgException("Invalid subscript: start(" 
								+ start + ") > size(" 
								+ size  + ").");
				} else if ((end != 0) && (end > size)) {
					throw new NgException("Invalid subscript: end(" 
								+ end  + ") > size(" 
								+ size + ").");
				}
			}
		}
	}

}

