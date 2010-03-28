/*
 * $RCSfile: SkipInfoHeader.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:08 $
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
package org.apgrid.grpc.ng.protocol;

import java.io.IOException;
import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.NgByteArrayOutputStream;

class SkipInfoHeader {
	static class SkipInfo {
		private int size, start, end,skip;
		public SkipInfo(int size, int start, int end, int skip) {
			this.size = size;
			this.start = start;
			this.end = end;
			this.skip = skip;
		}

		public int getSize() { return this.size; }
		public int getStart() { return this.start; }
		public int getEnd() { return this.end; }
		public int getSkip() { return this.skip; }
		// public void puts(OutputStream os);

		public String toString() {
			return "[" + size + ", " + start + ", " + end + ", " + skip + "]";
		}
	}

	private int nDims;
	private int count;
	private SkipInfo [] infos; 

	// Constructor
	public SkipInfoHeader(int nDims) {
		if (nDims < 0)
			throw new IllegalArgumentException();
		this.nDims = nDims;
		this.infos = new SkipInfo[this.nDims];
		this.count = 0;
	}

	// Add SkipInfo into SkipInfoHeader
	public void add(int size, int start, int end, int skip) {
		if (count >= infos.length)
			throw new IndexOutOfBoundsException();
		this.infos[count] = new SkipInfo(size, start, end, skip);
		count++;
	}

	public int getNumOfDims() {
		return this.nDims;
	}

	public SkipInfo [] getSkipInfo() {
		return this.infos;
	}

	// returned byte array is XDR format
	public byte[] toByteArray() throws NgIOException{
		NgByteArrayOutputStream bos =
			new NgByteArrayOutputStream( 4 + nDims * 16);
		try {
			bos.writeInt(nDims);
			for (int i = 0; i < infos.length; i++) {
				bos.writeInt(infos[i].getSize());
				bos.writeInt(infos[i].getStart());
				bos.writeInt(infos[i].getEnd());
				bos.writeInt(infos[i].getSkip());
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return bos.toByteArray();
	}

	// public void puts(OutputStream os);

	public String toString() {
		StringBuilder sb = new StringBuilder("[");
		sb.append(nDims).append("[");
		for (int i = 0; i < infos.length; i++) {
			sb.append(infos[i]);
		}
		sb.append("]");
		return sb.toString();
	}

}
