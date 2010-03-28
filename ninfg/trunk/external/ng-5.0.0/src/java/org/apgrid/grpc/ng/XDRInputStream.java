/*
 * $RCSfile: XDRInputStream.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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
package org.apgrid.grpc.ng;

import java.io.DataInputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.gridforum.gridrpc.GrpcException;

public class XDRInputStream
extends FilterInputStream 
implements NgDataInputStream {
	private static int XDR_LIMIT = 4;
	private DataInputStream din = null;

	/**
	 * @param stream
	 */
	public XDRInputStream(InputStream stream) {
		super(stream);
		this.din = new DataInputStream(stream);
	}

	/**
	 * 
	 */
	public void close() throws IOException {
		super.close();
	}

	/**
	 * @return
	 */
	private byte readByte() throws GrpcException {
		int nRead;
		try {
			nRead = in.read();
			if (nRead < 0) {
				throw new NgIOException("XDRInputStream: fail to read.");				
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return (byte) nRead;
	}

	/**
	 * @param buffer
	 */
	private int readBytes(byte[] buffer) throws GrpcException {
		return readBytes(buffer, 0, buffer.length);
	}

	/**
	 * @param buffer
	 * @param offset
	 * @param length
	 * @return
	 * @throws GrpcException
	 */
	public int readBytes(byte[] buffer, int offset, int length) throws GrpcException {
		int nRead = 0;
		while (nRead < length) {
			try {
				int n = in.read(buffer, offset + nRead, length - nRead);
				if (n < 0) {
					return (nRead > 0) ? nRead : -1;
				}
				nRead += n;
			} catch (IOException e) {
				throw new NgIOException(e);
			}
		}
		return nRead;
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	public char readChar() throws GrpcException {
		try {
			return (char)din.readInt();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	public double readDouble() throws GrpcException {
		try {
			return din.readDouble();
		} catch (IOException e) {
			throw new NgIOException(e);
		}				
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	public double readFloat() throws GrpcException {
		try {
			return din.readFloat();
		} catch (IOException e) {
			throw new NgIOException(e);
		}				
	}
	
	/**
	 * @return
	 */
	public int readInt() throws GrpcException {
		try {
			return din.readInt();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * @return
	 * @throws GrpcException
	 */
	public long readLong() throws GrpcException {
		try {
			return din.readLong();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * @return
	 * @throws GrpcException
	 */
	public short readShort() throws GrpcException {
		try {
			return (short)din.readInt();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @return
	 */
	public String readString() throws GrpcException {
		int length = this.readInt();
		byte buffer[] = new byte[length];
		int rest = length % XDR_LIMIT;
		rest = (rest != 0) ? XDR_LIMIT - rest : rest;
		readBytes(buffer);
		for (int i = 0; i < rest; i++) {
			byte dummy = this.readByte();
		}
		return new String(buffer);
	}

}
