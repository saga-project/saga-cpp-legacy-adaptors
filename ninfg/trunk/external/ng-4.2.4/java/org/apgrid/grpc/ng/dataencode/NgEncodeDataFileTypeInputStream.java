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
 * $RCSfile: NgEncodeDataFileTypeInputStream.java,v $ $Revision: 1.8 $ $Date: 2006/10/11 08:13:49 $
 */
package org.apgrid.grpc.ng.dataencode;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.apgrid.grpc.ng.NgGlobals;
import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.XDROutputStream;

public class NgEncodeDataFileTypeInputStream extends InputStream {
	/* definitions */
	public static int ENCODE_HEADER_LEN = 8;
	
	static int FILENAME_NULL = 0;
	static int FILENAME_NORMAL = 1;
	static int FILENAME_NULL_STRING = 2;
	
	/* instance variable */
	protected int nRead = 0;
	protected int fileLength;
	protected int mode = NgParamTypes.NG_MODE_NONE;
	protected int containsDataKind;
	protected byte[] headerData = null;
	protected int headerLength = 0;
	protected FileInputStream fis = null;
	
	/**
	 * @param arg0
	 * @param length
	 * @param mode
	 * @throws IOException
	 */
	public NgEncodeDataFileTypeInputStream(String arg0, int mode) throws IOException {
		if ((arg0 == null) || arg0.equals("")) {
			if (arg0 == null) {
				containsDataKind = FILENAME_NULL;
			} else if (arg0.equals("")) {
				containsDataKind = FILENAME_NULL_STRING;
			}
			this.fileLength = 0;
		} else {
			if ((mode == NgParamTypes.NG_MODE_IN) ||
				(mode == NgParamTypes.NG_MODE_INOUT)){
				this.fis = new FileInputStream(arg0);
				this.fileLength = (int) new File(arg0).length();
			} else {
				this.fileLength = 0;
			}
			containsDataKind = FILENAME_NORMAL;
		}
		/* set mode */
		this.mode = mode;

		/* create header data */
		createHeaderData();
	}
	
	/* (non-Javadoc)
	 * @see java.io.InputStream#available()
	 */
	public int available() throws IOException {
		int restOfHeader = 0;
		if (nRead <= this.headerLength) {
			restOfHeader = this.headerLength - nRead;
		}
		
		if (this.fis != null) {
			return fis.available() + restOfHeader;
		} else {
			return restOfHeader;
		}
	}
	
	/* (non-Javadoc)
	 * @see java.io.InputStream#close()
	 */
	public void close() throws IOException {
		if (this.fis != null) {
			this.fis.close();
			this.fis = null;
		}
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#finalize()
	 */
	public void finalize() throws IOException {
		close();
	}
	
	/* (non-Javadoc)
	 * @see java.io.InputStream#read()
	 */
	public int read() throws IOException {
		byte[] buffer = new byte[1];
		read(buffer);
		return buffer[0];
	}
	
	/* (non-Javadoc)
	 * @see java.io.InputStream#read(byte[])
	 */
	public int read(byte[] buffer) throws IOException {
		return read(buffer, 0, buffer.length);
	}
	
	/* (non-Javadoc)
	 * @see java.io.InputStream#read(byte[], int, int)
	 */
	public int read(byte[] buffer, int offset, int length) throws IOException {
		if ((offset + length) > buffer.length) {
			throw new IOException();
		}
		
		/* read header data before reading file data */
		int index = offset;
		int readFromHeader = 0;
		if (nRead < this.headerLength) {
			int endRead = (nRead + length) > this.headerLength ?
					this.headerLength : (nRead + length);
			for (int i = nRead; i < endRead; i++) {
				buffer[index++] = this.headerData[nRead++];
				readFromHeader++;
			}
		}

		if (fis != null) {
			return fis.read(buffer, index, length - index) + readFromHeader;
		} else {
			return readFromHeader;
		}
	}

	/**
	 * @throws IOException
	 */
	private void createHeaderData() throws IOException {
		ByteArrayOutputStream bos = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xos = new XDROutputStream(bos);
		
		/* put flag for null */
		xos.writeInt(containsDataKind);
		/* put size of data */
		xos.writeInt(fileLength);
		bos.close();
		
		/* set byte array for header */
		this.headerData = bos.toByteArray();
		
		/* set length of header */
		this.headerLength = ENCODE_HEADER_LEN;
	}
}
