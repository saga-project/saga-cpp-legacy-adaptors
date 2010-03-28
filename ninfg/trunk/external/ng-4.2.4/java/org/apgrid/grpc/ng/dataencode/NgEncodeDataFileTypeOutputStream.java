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
 * $RCSfile: NgEncodeDataFileTypeOutputStream.java,v $ $Revision: 1.7 $ $Date: 2006/10/11 08:13:49 $
 */
package org.apgrid.grpc.ng.dataencode;

import java.io.ByteArrayInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.apgrid.grpc.ng.XDRInputStream;
import org.gridforum.gridrpc.GrpcException;

public class NgEncodeDataFileTypeOutputStream extends OutputStream {
	/* definitions */
	public int ENCODE_HEADER_LEN = NgEncodeDataRawFileInputStream.ENCODE_HEADER_LEN;
	protected static int FILENAME_NULL =
		NgEncodeDataRawFileInputStream.FILENAME_NULL;
	protected static int FILENAME_NORMAL =
		NgEncodeDataRawFileInputStream.FILENAME_NORMAL;
	protected static int FILENAME_NULL_STRING =
		NgEncodeDataRawFileInputStream.FILENAME_NULL_STRING;

	/* instance variables */
	protected int nWrite;
	protected int length;
	protected int containsDataKind;
	protected boolean isHeaderParsed = false;
	protected FileOutputStream fos;
	protected byte[] headerData;
	protected int headerLength = 0;
	
	/**
	 * @param arg0
	 * @throws FileNotFoundException
	 */
	public NgEncodeDataFileTypeOutputStream(String arg0) throws FileNotFoundException {
		if (arg0 == null) {
			containsDataKind = FILENAME_NULL;
		} else if (arg0.equals("")) {
			containsDataKind = FILENAME_NULL_STRING;
		} else {
			this.fos = new FileOutputStream(arg0);
			containsDataKind = FILENAME_NORMAL;
		}
		
		/* set header information */
		this.headerData = new byte[ENCODE_HEADER_LEN];
		this.headerLength = ENCODE_HEADER_LEN;
	}
	
	/* (non-Javadoc)
	 * @see java.io.OutputStream#write(int)
	 */
	public void write(int arg0) throws IOException {
		byte buffer[] = new byte[1];
		buffer[0] = (byte) arg0;
		write(buffer);
	}
	
	/* (non-Javadoc)
	 * @see java.io.OutputStream#write(byte[])
	 */
	public void write(byte[] buffer) throws IOException {
		write(buffer, 0, buffer.length);
	}
	
	/* (non-Javadoc)
	 * @see java.io.OutputStream#write(byte[], int, int)
	 */
	public void write(byte[] buffer, int offset, int length) throws IOException {
		if (offset + length > buffer.length) {
			throw new IOException();
		}
		
		/* if it already read header data */
		int nReadHeaderFromBuffer = 0;
		if (this.isHeaderParsed != true) {
			/* read header data */
			int index = offset;
			while (nWrite < this.headerLength) {
				headerData[nWrite++] = buffer[index++];
				nReadHeaderFromBuffer++;
				if (index >= buffer.length) {
					break;
				}
			}
			
			/* parse header data */
			if (nWrite >= this.headerLength){
				parseHeaderData(this.headerData);
			}
		}
		
		/* read data from the file */
		if (this.fos != null) {
			this.fos.write(buffer,
					offset + nReadHeaderFromBuffer, length - nReadHeaderFromBuffer);
		}
	}

	/**
	 * @param headerBuffer
	 * @throws IOException
	 */
	protected void parseHeaderData(byte[] headerBuffer) throws IOException {
		ByteArrayInputStream bis = new ByteArrayInputStream(headerBuffer);
		XDRInputStream xis = new XDRInputStream(bis);
		
		try {
			/* get containsData */
			this.containsDataKind = xis.readInt();
			/* get length of data */
			this.length = xis.readInt();
			
			/* close stream */
			bis.close();
		} catch (GrpcException e) {
			throw new IOException();
		}
		
		/* set parsed flag */
		this.isHeaderParsed = true;
	}
	
	/* (non-Javadoc)
	 * @see java.io.OutputStream#close()
	 */
	public void close() throws IOException {
		if (this.fos != null) {
			this.fos.close();
			this.fos = null;
		}
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#finalize()
	 */
	public void finalize() throws IOException {
		close();
	}
	
	/**
	 * @return
	 */
	public int getLength() {
		return this.length;
	}
}
