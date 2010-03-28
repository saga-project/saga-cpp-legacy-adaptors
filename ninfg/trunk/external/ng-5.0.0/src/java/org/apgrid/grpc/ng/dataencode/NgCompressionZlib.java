/*
 * $RCSfile: NgCompressionZlib.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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
import java.util.zip.DeflaterOutputStream;
import java.util.zip.InflaterInputStream;

import org.apgrid.grpc.ng.NgIOException;
import org.gridforum.gridrpc.GrpcException;
import org.apgrid.grpc.ng.NgGlobals;

class NgCompressionZlib implements NgEncodeDataInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgEncodeDataInterface#decodeData(byte[], java.util.Vector)
	 */
	public byte[] decodeData(byte[] buffer, List encodeInfo) throws GrpcException {
		byte[] receiveBuffer = new byte[NgGlobals.zlibInputBufferSize];
		ByteArrayInputStream bin = new ByteArrayInputStream(buffer);
		InflaterInputStream iin = new InflaterInputStream(bin);
		ByteArrayOutputStream bout = new ByteArrayOutputStream(NgGlobals.argBufferSize);
		
		/* decode zip data */
		try {
			int nread = 0;
			while ((nread = iin.read(receiveBuffer)) > 0) {
				bout.write(receiveBuffer, 0, nread);
			}
			/* close */
			bin.close();
			bout.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		return bout.toByteArray();
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgEncodeDataInterface#encodeData(byte[], java.util.Vector)
	 */
	public byte[] encodeData(byte[] buffer, List encodeInfo) throws GrpcException {
		byte[] receiveBuffer = new byte[NgGlobals.zlibInputBufferSize];
		ByteArrayInputStream bin = new ByteArrayInputStream(buffer);
		ByteArrayOutputStream bout = new ByteArrayOutputStream(NgGlobals.argBufferSize);
		DeflaterOutputStream dos = new DeflaterOutputStream(bout);
		
		/* decode zip data */
		try {
			while (bin.available() > 0) {
				int nread = bin.read(receiveBuffer);
				dos.write(receiveBuffer, 0, nread);
			}
			/* finish to write */
			dos.finish();
			dos.close();

			/* close */
			bin.close();
			bout.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		return bout.toByteArray();
	}
}
