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
 * $RCSfile: XDROutputStream.java,v $ $Revision: 1.6 $ $Date: 2004/12/15 08:05:19 $
 */
package org.apgrid.grpc.ng;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.gridforum.gridrpc.GrpcException;

public class XDROutputStream
		extends DataOutputStream
		implements NgDataOutputStream {
	private static int XDR_LIMIT = 4;

	/**
	 * @param stream
	 */
	public XDROutputStream(OutputStream stream) {
		super(stream);
	}

	/**
	 * 
	 */
	public void close() {
		/* nothing */
	}

	/**
	 * @param length
	 * @throws GrpcException
	 */
	private void writePad(int length) throws GrpcException {
		for (int i = 0; i < length; i++) {
			try {
				write((byte)0);
			} catch (IOException e) {
				throw new NgIOException(e);
			}
		}
	}
	
	/**
	 * @param string
	 */
	public void writeString(String string) throws GrpcException {
		try {
			int length = string.length();
			this.writeInt(length);
			int rest = length % XDR_LIMIT;
			rest = (rest != 0) ? XDR_LIMIT - rest : rest;
			writeBytes(string.getBytes());
			if (rest != 0) {
				this.writePad(rest);
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
	}

	/**
	 * @param buffer
	 */
	public void writeBytes(byte[] buffer) throws GrpcException {
		try {
			super.write(buffer);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * @param buffer
	 * @param offset
	 * @param length
	 */
	public void writeBytes(byte[] buffer, int offset, int length) throws GrpcException {
		try {
			super.write(buffer, offset, length);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
}
