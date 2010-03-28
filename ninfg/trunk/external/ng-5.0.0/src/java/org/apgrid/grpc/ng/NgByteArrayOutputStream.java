/*
 * $RCSfile: NgByteArrayOutputStream.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;

/*
 * This is subclass(proper descendent) of ByteArrayOutputStream for Ninf-G.
 * The data is written into a byte array by XDR format.
 * The data can be retrieve to getByteArray() and inherited 
 * ByteArrayOutputStream methods. but getByteArray() does not 
 * creates a newly allocated byte array.
 *
 * @see java.io.ByteArrayOutputStream
 */
public class NgByteArrayOutputStream extends ByteArrayOutputStream {

	private DataOutputStream exOs = null; // EXternal OutputStream

	public NgByteArrayOutputStream(int size) {
		super(size);
		exOs = new DataOutputStream(this);
	}

	/*
	 * Return the byte array. It doesn't creates a newly allocated byte array.
	 *
	 * @return the byte array
	 */
	public byte[] getByteArray() {
		return buf; // buf is inherit variable
	}

	/*
	 * Return the number of valid bytes in the buffer.
	 */
	public int getCount() {
		return count; // count is inherit variable 
	}

	/*
	 * @see java.io.DataOutputStream#writeInt(int v)
	 */
	public void writeInt(int v) throws IOException {
		exOs.writeInt(v);
	}

	/*
	 * @see java.io.DataOutputStream#write(byte[] buf, int off, int len)
	 */
	public void writeBytes(byte[] buf, int off, int len) throws IOException {
		exOs.write(buf, off, len);
	}

}

