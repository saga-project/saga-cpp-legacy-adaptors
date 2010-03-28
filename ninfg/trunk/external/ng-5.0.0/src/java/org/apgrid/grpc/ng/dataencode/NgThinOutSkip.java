/*
 * $RCSfile: NgThinOutSkip.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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
package org.apgrid.grpc.ng.dataencode;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.List;

import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgParamTypes;
import org.gridforum.gridrpc.GrpcException;
import org.apgrid.grpc.ng.NgGlobals;

class NgThinOutSkip implements NgEncodeDataInterface {
	private static final int ENCODE_HEADER_LEN = 8;
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.dataencode.NgEncodeDataInterface#decodeData(byte[], java.util.List)
	 */
	public byte[] decodeData(byte[] buffer, List encodeInfo)
	 throws GrpcException {
		int[] typeArray = (int[]) encodeInfo.get(0);
		int[][] arrayShapes = (int[][]) encodeInfo.get(1);
		int argSize = NgParamTypes.getSize(typeArray[0]);
		ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
		ByteArrayOutputStream bo =
		 new ByteArrayOutputStream(NgGlobals.argBufferSize);
		
		// decode data
		decodeData(bi, argSize, arrayShapes.length, arrayShapes, bo);
		
		try {
			bo.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		return bo.toByteArray();
	}
	
	/**
	 * @param bi
	 * @param argSize
	 * @param nDims
	 * @param arrayShapes
	 * @param bo
	 */
	private void decodeData(ByteArrayInputStream bi, int argSize,
	 int nDims, int[][] arrayShapes, ByteArrayOutputStream bo) {
		int size  = arrayShapes[nDims - 1][0];
		int start = arrayShapes[nDims - 1][1];
		int end   = arrayShapes[nDims - 1][2];
		int step  = arrayShapes[nDims - 1][3];
		int elemSize = getElemSize(argSize, nDims - 1, arrayShapes);
		
		// check end
		if (end == 0) {
			end = size;
		}
		// check step 
		if (step == 0) {
			step = 1;
		}
		
		int nextTarget = start;
		for (int i = 0; i < size; i++) {
			if (i == nextTarget) {
				if (nDims > 1) {
					decodeData(bi, argSize, nDims - 1, arrayShapes, bo);
				} else {
					byte[] targetData = new byte[argSize];
					bi.read(targetData, 0, argSize);
					bo.write(targetData, 0, argSize);
				}
				nextTarget += step;
				if (nextTarget >= end) {
					nextTarget = -1;
				}
			} else {
				for (int j = 0; j < elemSize; j++) {
					bo.write(0);
				}
			}
		}
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.dataencode.NgEncodeDataInterface#encodeData(byte[], java.util.List)
	 */
	public byte[] encodeData(byte[] buffer, List encodeInfo)
	 throws GrpcException {
		int[] sizeArray     = (int[]) encodeInfo.get(0);
		int[][] arrayShapes = (int[][]) encodeInfo.get(1);
		int argSize = sizeArray[0];
		ByteArrayOutputStream bo =
		 new ByteArrayOutputStream(NgGlobals.argBufferSize);
		
		// encode data
		encodeData(buffer, 0, argSize, arrayShapes.length, arrayShapes, bo);
		
		try {
			byte[] dummyBuffer = new byte[ENCODE_HEADER_LEN];
			bo.write(dummyBuffer);
			bo.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		return bo.toByteArray();
	}
	
	/**
	 * @param buffer
	 * @param currentIndex
	 * @param argSize
	 * @param nDim
	 * @param arrayShape
	 * @param bo
	 */
	private void encodeData(byte[] buffer, int currentIndex,
	 int argSize, int nDims, int[][] arrayShapes, ByteArrayOutputStream bo) {
		int start = arrayShapes[nDims - 1][1];
		int end   = arrayShapes[nDims - 1][2];
		int step  = arrayShapes[nDims - 1][3];
		
		// check end
		if (end == 0) {
			end = arrayShapes[nDims - 1][0];
		}
		// check step
		if (step == 0) {
			step = 1;
		}
		
		if (nDims > 1) {
			int sizeElem = getElemSize(argSize, nDims - 1, arrayShapes);
			// process lower array
			for (int i = start; i < end; i += step) {
				encodeData(buffer, currentIndex + (i * sizeElem),
					argSize, nDims - 1, arrayShapes, bo);
			}
		} else {
			// write data of array 
			for (int i = start; i < end; i += step) {
				bo.write(buffer, currentIndex + (i * argSize), argSize);
			}
		}
	}
	
	/**
	 * @param argSize
	 * @param nDims
	 * @param arrayShapes
	 * @return
	 */
	private int getElemSize(int argSize, int nDims, int[][] arrayShapes) {
		int sizeElem = argSize;
		for (int i = 0; i < nDims; i++) {
			sizeElem *= arrayShapes[nDims - 1][0];
		}
		return sizeElem;
	}
}
