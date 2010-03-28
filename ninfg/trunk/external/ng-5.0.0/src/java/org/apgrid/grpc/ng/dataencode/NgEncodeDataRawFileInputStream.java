/*
 * $RCSfile: NgEncodeDataRawFileInputStream.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.apgrid.grpc.ng.NgGlobals;
import org.apgrid.grpc.ng.XDROutputStream;

public class NgEncodeDataRawFileInputStream extends NgEncodeDataFileTypeInputStream {
	// definitions 
	public static int FILE_ENCODE_RAW_HEADER_LEN = 16;

	// instance variable 
	private int fileTypeEncodedLength;
	
	/**
	 * @param arg0
	 * @param mode
	 * @throws IOException
	 */
	public NgEncodeDataRawFileInputStream(String arg0, int mode)
	 throws IOException {
		super(arg0, mode);
		
		// set variable 
		this.fileTypeEncodedLength = this.fileLength + ENCODE_HEADER_LEN;
		// create header data 
		createHeaderData();
	}
	
	/**
	 * @param arg0
	 * @param length
	 * @param mode
	 * @throws IOException
	 */
	public NgEncodeDataRawFileInputStream(String arg0, int length, int mode)
	 throws IOException {
		super(arg0, mode);
		
		// set variable 
		this.fileTypeEncodedLength = length + ENCODE_HEADER_LEN;
		// create header data 
		createHeaderData();
	}
	
	/**
	 * @throws IOException
	 */
	private void createHeaderData() throws IOException {
		// save header for file type 
		byte[] headerForFileType = this.headerData;
		
		// create header for RawFile
		ByteArrayOutputStream bos =
		 new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xos = new XDROutputStream(bos);
		
		// put encode type RAW 
		xos.writeInt(NgEncodeData.NG_DATA_REPRESENT_RAW);
		// put size of RAW encoded data 
		xos.writeInt(this.fileTypeEncodedLength);
		// put header for file type
		xos.write(headerForFileType);
		bos.close();
		
		// set byte array for header 
		this.headerData = bos.toByteArray();
		
		// set length of header
		this.headerLength = FILE_ENCODE_RAW_HEADER_LEN;
	}
}
