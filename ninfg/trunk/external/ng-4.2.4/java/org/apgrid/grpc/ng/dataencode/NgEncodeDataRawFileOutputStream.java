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
 * $RCSfile: NgEncodeDataRawFileOutputStream.java,v $ $Revision: 1.7 $ $Date: 2006/10/11 08:13:49 $
 */
package org.apgrid.grpc.ng.dataencode;

import java.io.ByteArrayInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import org.apgrid.grpc.ng.XDRInputStream;
import org.gridforum.gridrpc.GrpcException;

public class NgEncodeDataRawFileOutputStream extends NgEncodeDataFileTypeOutputStream {
	/* definitions */
	public int FILE_ENCODE_RAW_HEADER_LEN =
		NgEncodeDataRawFileInputStream.FILE_ENCODE_RAW_HEADER_LEN;

	/* instance variables */
	private int encodeType;
	private int decodedDataLength;
	
	/**
	 * @param arg0
	 * @throws FileNotFoundException
	 */
	public NgEncodeDataRawFileOutputStream(String arg0) throws FileNotFoundException {
		super(arg0);
		
		/* set header information */
		this.headerData = new byte[FILE_ENCODE_RAW_HEADER_LEN];
		this.headerLength = FILE_ENCODE_RAW_HEADER_LEN;
	}
	
	/**
	 * @param headerBuffer
	 * @throws IOException
	 */
	protected void parseHeaderData(byte[] headerBuffer) throws IOException {
		ByteArrayInputStream bis = new ByteArrayInputStream(headerBuffer);
		XDRInputStream xis = new XDRInputStream(bis);
		
		try {
			/* get encode type raw */
			this.encodeType = xis.readInt();
			/* get length of encoded data */
			this.decodedDataLength = xis.readInt();
			
			/* get information of file type */
			byte[] headerFileTypeBuffer = new byte[ENCODE_HEADER_LEN];
			/* drop first 8 bytes */
			xis.read(headerFileTypeBuffer);
			/* read header data for filename type */
			xis.read(headerFileTypeBuffer);
			super.parseHeaderData(headerFileTypeBuffer);
			
			/* close stream */
			bis.close();
		} catch (GrpcException e) {
			throw new IOException();
		}
		
		/* set parsed flag */
		this.isHeaderParsed = true;
	}
	
	/**
	 * @return
	 */
	public int getDecodedDataLength() {
		return this.decodedDataLength;
	}
}
