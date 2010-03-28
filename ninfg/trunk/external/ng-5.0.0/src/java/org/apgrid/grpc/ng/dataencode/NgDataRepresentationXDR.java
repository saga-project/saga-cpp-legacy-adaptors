/*
 * $RCSfile: NgDataRepresentationXDR.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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
import org.apgrid.grpc.ng.XDRInputStream;
import org.apgrid.grpc.ng.XDROutputStream;
import org.apgrid.grpc.ng.info.RemoteMethodArg;
import org.gridforum.gridrpc.GrpcException;
import org.apgrid.grpc.ng.NgGlobals;

class NgDataRepresentationXDR implements NgEncodeDataInterface {
	private static int XDR_LIMIT = 4;

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgEncodeDataInterface#decodeData(byte[], java.util.Vector)
	 */
	public byte[] decodeData(byte[] buffer, List encodeInfo)
		throws GrpcException {

		/* get type of result */
		byte[] decodedByteArray = buffer;
		Integer intType = (Integer) encodeInfo.get(0);
		int argType = intType.intValue();
		
		if (argType == NgParamTypes.NG_TYPE_CHAR) {
			decodedByteArray = new byte[buffer.length / 4];

			/* convert All elements of array int XDR */
			int inputbufferIndex = 0;
			for (int i = 0; i < decodedByteArray.length; i++) {
				decodedByteArray[i] = buffer[inputbufferIndex + 3];
				inputbufferIndex += XDR_LIMIT;
			}
		} else if (argType == NgParamTypes.NG_TYPE_SHORT) {
			decodedByteArray = new byte[buffer.length / 2];

			/* convert All elements of array int XDR */
			int inputbufferIndex = 0;
			for (int i = 0; i < decodedByteArray.length; i+=2) {
				decodedByteArray[i] = buffer[inputbufferIndex + 2];
				decodedByteArray[i+1] = buffer[inputbufferIndex + 3];
				inputbufferIndex += XDR_LIMIT;
			}
		} else if (argType == NgParamTypes.NG_TYPE_STRING) {
			ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
			XDRInputStream xi = new XDRInputStream(bi);
			ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);

			try {
				bo.write(xi.readString().getBytes());
				xi.close();
				bo.close();
			} catch (IOException e1) {
				throw new NgIOException(e1);
			}
			
			decodedByteArray = bo.toByteArray();
		}
		
		return decodedByteArray;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgEncodeDataInterface#encodeData(byte[], java.util.Vector)
	 */
	public byte[] encodeData(byte[] buffer, List encodeInfo)
		throws GrpcException {
		/* get RemoteMethodArg from encodeInfo */
		RemoteMethodArg remoteMethodArg =
			(RemoteMethodArg) encodeInfo.get(0);
		byte[] encodedByteArray;
		
		if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_CHAR) {
			encodedByteArray = new byte[buffer.length * 4];

			/* convert All elements of array int XDR */
			for (int i = 0; i < buffer.length; i++) {
				int index = i * 4;
				encodedByteArray[index++] = 0;
				encodedByteArray[index++] = 0;
				encodedByteArray[index++] = 0;
				encodedByteArray[index++] = buffer[i];
			}
		} else if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_SHORT) {
			encodedByteArray = new byte[buffer.length * 2];

			/* convert All elements of array int XDR */
			for (int i = 0; i < buffer.length; i += 2) {
				int index = i * 2;
				encodedByteArray[index++] = 0;
				encodedByteArray[index++] = 0;
				encodedByteArray[index++] = buffer[i];
				encodedByteArray[index++] = buffer[i+1];
			}
		} else if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_STRING) ||
			(remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME)) {
			ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
			XDROutputStream xo = new XDROutputStream(bo);

			/* convert String into XDR format */
			if (buffer.length != 0) {
				xo.writeString(new String(buffer));
			}
			xo.close();
			/* get byte[] */
			encodedByteArray = bo.toByteArray();
		} else {
			/* no need to convert */
			encodedByteArray = buffer;
		}
		
		return encodedByteArray;
	}
}
