/*
 * $RCSfile: ArgumentData.java,v $ $Revision: 1.3 $ $Date: 2008/02/07 08:17:43 $
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

import org.apgrid.grpc.ng.NgByteArrayOutputStream;
import org.apgrid.grpc.ng.NgIOException;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.protocol.ProtTransferArgumentRequest.DIVIDE_DATA_LENGTH;
import static org.apgrid.grpc.ng.NgParamTypes.NG_TYPE_FILENAME;

class ArgumentData {

	////// Ninf-G Protocol Data
	private int number; // argument number
	private int type;   // argument type (ninf)
	private int mode;   // mode (in, out, inout)
	private int encodeType; // convert method (none, skip, ...)
	private int nElem;   // number of element (if argument is scalar then 1)
	private int length; // length
	private SkipInfoHeader skipHeader = null; // skip info 
	private byte[] argument = null; // data of Ninf
	private byte[] header = null;

	///// Optional 
	// Compressed Data
	private boolean arg_is_compressed = false;
	private long originalLength   = -1; // Original Argument Data Length
	private long compressedLength = -1; // Compressed Argument Data Length
	private double compressRealTime = -1; // Compressed Time about Real
	private double compressCPUTime  = -1; // Compressed Time about CPU
	// Java Data
	private Object argument_java = null; // data of Java
	private int nDims;


	// Constructor
	public ArgumentData(
	 int num, int type, int mode, int encode, int nElem, byte[] data,
	 int length, Object java_data, int nDims, SkipInfoHeader skip_header)
	 throws GrpcException {
		if (data == null) 
			throw new NullPointerException();
		if (length == Integer.MIN_VALUE)
			throw new IllegalArgumentException();
		boolean specifiedSkip = specifiedSkipEncode(encode);
		if ( (  specifiedSkip && (skip_header == null) ) 
		  || (! specifiedSkip && (skip_header != null) ) )
			throw new IllegalArgumentException();

		this.number = num;
		this.type  = type;
		this.mode = mode;
		this.encodeType = encode;
		this.nElem = nElem;
		this.argument = data;
		// if length == -1 then argument is specified data division.
		this.length = length;
		this.argument_java = java_data;
		this.nDims = nDims;
		this.skipHeader = skip_header;
		this.header = packHeader();
	}

	private boolean specifiedSkipEncode(int encode) {
		// '0x0100' is type of Skip Encode
		return ((encode & 0x0100) == 0x0100);
	}

	private byte[] packHeader() throws NgIOException {
		NgByteArrayOutputStream bos =
			new NgByteArrayOutputStream(24 + getSkipHeaderLength());

		// don't correct this order
		try {
			bos.writeInt(this.number);
			bos.writeInt(this.type);
			bos.writeInt(this.mode);
			bos.writeInt(this.encodeType);
			bos.writeInt(this.nElem);
			bos.writeInt(this.length);
			if (skipHeader != null) {
				byte[] _skipInfo = skipHeader.toByteArray();
				bos.writeBytes(_skipInfo, 0, _skipInfo.length);
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return bos.toByteArray();
	}

	// friend method with ArgumentDataFactory, ProtTransferArgumentRequest
	protected void setCompressInfo(long original, long compressed, double real,
	 double cpu){
		this.arg_is_compressed = true;
		this.originalLength = original;
		this.compressedLength = compressed;
		this.compressRealTime = real;
		this.compressCPUTime = cpu;
	}

	protected void addOriginalDataLength(long length) {
		this.originalLength += length;
	}

	protected void addCompressDataLength(long length) {
		this.compressedLength += length;
	}

	protected void addCompressRealTime(double time) {
		this.compressRealTime += time;
	}

	protected long getOriginalDataLength() {
		return this.originalLength;
	}

	protected long getCompressedDataLength() {
		return this.compressedLength;
	}

	protected double getCompressRealTime() {
		return this.compressRealTime;
	}

	protected boolean isCompressSpecified() {
		return this.arg_is_compressed;
	}

	// return Number of Argument
	public int getNumber() {
		return this.number;
	}

	public int length() {
		return this.length;
	}

	public byte[] getData() {
		return this.argument;
	}
	public Object getJavaData() {
		return this.argument_java;
	}

	public byte[] getParamHeader() {
		if (this.header == null)
			throw new IllegalStateException();
		return this.header;
	}

	public int getHeaderLength() {
		if (this.header == null) 
			throw new IllegalStateException();
		return this.header.length;
	}

	private int getSkipHeaderLength() {
		if (skipHeader == null)
			return 0;

		// nDims size (4byte)
		int ret = 4;
		// nDims x SkipInfo size (4x4byte)
		ret += skipHeader.getNumOfDims() * 16;
		return ret;
	}

	public int getMode() {
		return this.mode;
	}

	public boolean isDataDivisionSpecified() {
		return (this.length == DIVIDE_DATA_LENGTH);
	}

	public boolean isArray() {
		return (this.nDims > 0);
	}

	public boolean isTypeFilename() {
		return (this.type == NG_TYPE_FILENAME);
	}

	public String toString() {
		StringBuilder sb = new StringBuilder("[");
		sb.append(number).append(", ");
		sb.append(type).append(", ");
		sb.append(mode).append(", ");
		sb.append(encodeType).append(", ");
		sb.append(nElem).append(", ");
		sb.append(length).append(", ");
		if (skipHeader != null)
			sb.append(skipHeader);
		sb.append("]");
		return sb.toString();
	}

}

