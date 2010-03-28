/*
 * $RCSfile: NgType.java,v $ $Revision: 1.6 $ $Date: 2007/09/26 04:14:07 $
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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import org.gridforum.gridrpc.GrpcException;


public abstract class NgType<TYPE> {
	private final int type;
	private final String name;
	private final int size;
	// Parameter Input Buffer Size
	private static final int BUFF_SIZE = 5120;
	private static final int BUFF_SIZE_SMALL = 32;

	/**
	 * @param type id of Data Type
	 * @param name name of Data Type
	 * @param size size of Ninf-G Data
	 */
	protected NgType(int type, String name, int size) {
		this.type = type;
		this.name = name;
		this.size = size;
	}
	public String toString() { return name; }
	public int getType() { return type; }
	public int getSize() { return size; }


	private static void copy(byte [] src, byte [] dest, int length) {
		if ((src == null) || (dest == null)) {
			throw new NullPointerException();
		}
		System.arraycopy(src, 0, dest, 0, length);
	}


	private static byte [] encodeDataXDRString(byte [] in)
	 throws GrpcException {

		if (in == null) { return new byte[0]; }

		ByteArrayInputStream bi = new ByteArrayInputStream(in);
		DataInputStream di = new DataInputStream(bi);

		ByteArrayOutputStream bo =
		 new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);

		while (bi.available() > 0) {
			// read length of String
			try {
				// read length of String
				int strLen = di.readInt();

				// read string
				byte[] strBuffer = new byte[strLen];
				di.read(strBuffer); // ByteArrayInput..(in)#read(strBuffer, 0, strBuffer.length);
				// encode
				ByteArrayOutputStream bo_ninf =
				new ByteArrayOutputStream(NgGlobals.smallBufferSize);
				XDROutputStream xo_ninf = new XDROutputStream(bo_ninf);

				// convert String into XDR format
				if (strBuffer.length != 0) {
					xo_ninf.writeString(new String(strBuffer));
				}
				xo_ninf.close();
				// get byte[]
				byte[] encodedString = bo_ninf.toByteArray();

				// write it to ByteArray
				xo.writeInt(encodedString.length);
				bo.write(encodedString);
			} catch (IOException e) {
				throw new NgIOException(e);
			}
		} // while
		try {
			bo.close();
			bi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return bo.toByteArray(); 
	}

	private static byte[] decodeDataXDRString(byte[] anData)
	 throws GrpcException {
		// decode XDR
		ByteArrayInputStream bi = new ByteArrayInputStream(anData);
		XDRInputStream xi = new XDRInputStream(bi);

		ByteArrayOutputStream bo =
		 new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		DataOutputStream dos = new DataOutputStream(bo);
		try {
			while (xi.available() > 0) {
				// read length
				int bufLen = xi.readInt();
				if (bufLen == 0) { break; }
				// read string
				byte[] strBuf = new byte[bufLen];
				xi.read(strBuf);

				ByteArrayInputStream bi_ninf =
				 new ByteArrayInputStream(strBuf);
				XDRInputStream xi_ninf =
				 new XDRInputStream(bi_ninf);
				ByteArrayOutputStream bo_ninf =
				 new ByteArrayOutputStream(NgGlobals.smallBufferSize);

				try {
					bo_ninf.write(xi_ninf.readString().getBytes());
				} catch (IOException e) {
					throw new NgIOException(e);
				}
				xi_ninf.close(); // ?
				try {
					bo_ninf.close();
				} catch (IOException e1) {
					throw new NgIOException(e1);
				}

				byte[] decodedBuf = bo_ninf.toByteArray();

				// write string data
				dos.writeInt(decodedBuf.length);
				dos.write(decodedBuf);
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		try {
			// close stream
			bi.close();
			bo.close();
		} catch (IOException e1) {
			throw new NgIOException(e1);
		}

		return bo.toByteArray();
	}

	private int readNgData(byte [] src, byte[] dest,
	 int current, int endOfArray)
	 throws GrpcException {

		DataInputStream di =
		 new DataInputStream(new ByteArrayInputStream(src));

        int nRead = endOfArray - current; 
        if (nRead > BUFF_SIZE) { // dest.length?
            nRead = BUFF_SIZE;
		}

        try {
            return di.read(dest, 0, nRead);
        } catch (IOException e) {
            throw new NgIOException(e);
        }
	}


	/**
	 * value is copied onto out.
	 * @param value value of Java data type.
	 * @param i     starting position to copy.
	 * @param out   output buffer.
	 * @return length of copied value. (getSize() value)
	 */
	protected abstract int setByteArray(TYPE value, int i, byte [] out);
	protected int _setByteArrayInt(int val, int i, byte[] out) {
		out[i]   = (byte)((val >>> 24) & 0xFF);
		out[i+1] = (byte)((val >>> 16) & 0xFF);
		out[i+2] = (byte)((val >>>  8) & 0xFF);
		out[i+3] = (byte)((val >>>  0) & 0xFF);
		return 4;
	}
	protected int _setByteArrayLong(long val, int i, byte[] out) {
		out[i]   = (byte)((val >>> 56) & 0xFF);
		out[i+1] = (byte)((val >>> 48) & 0xFF);
		out[i+2] = (byte)((val >>> 40) & 0xFF);
		out[i+3] = (byte)((val >>> 32) & 0xFF);
		out[i+4] = (byte)((val >>> 24) & 0xFF);
		out[i+5] = (byte)((val >>> 16) & 0xFF);
		out[i+6] = (byte)((val >>>  8) & 0xFF);
		out[i+7] = (byte)((val >>>  0) & 0xFF); 
		return 8;
	}

	// get Java data from src 
	protected abstract TYPE getValue(byte [] src, int startPos);
	protected int _getValueInt(byte[] src, int startPos) {
		int ch1 = (int)src[startPos];
		int ch2 = (int)src[startPos+1];
		int ch3 = (int)src[startPos+2];
		int ch4 = (int)src[startPos+3];
		ch1 &= 0xFF;
		ch2 &= 0xFF;
		ch3 &= 0xFF;
		ch4 &= 0xFF;
		return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0));
	}
	protected long _getValueLong(byte[] src, int startPos) {
		long ch1 = _getValueInt(src, startPos);
		return (ch1 << 32L) + (_getValueInt(src, startPos + 4) & 0xFFFFFFFFL); 
	}


	/**
	 * convert Java Data Type to Ninf-G Type (Scalar).
	 * @param anTarget converted data.
	 * @param out      output buffer.
	 */
	protected void convertScalar(TYPE anTarget, byte [] out)
	 throws  GrpcException { 
		if ((out == null) || (anTarget == null)) {
			throw new NullPointerException();
		}
		int argSize = getSize();
		if (argSize > out.length) {
			throw new NgException("The size of out buffer is not enough.");
		}
		byte[] buffer = new byte[ argSize ];
		setByteArray(anTarget, 0, buffer);
		copy(buffer, out, argSize);
	}


	/**
	 * convert Java Data Type to Ninf-G Type (Array).
	 * @param anTarget converted data.
	 * @param out      output buffer.
	 */
	protected void convertArray(TYPE [] target, int length, byte [] out)
	 throws GrpcException { 
		if ((out == null) || (target == null)) {
			throw new NullPointerException();
		}
		byte [] buffer = new byte[BUFF_SIZE];
		int arrayIndex = 0;
		for (int i = 0; i < length; i++) {
			arrayIndex += setByteArray(target[i], arrayIndex, buffer);
			if (arrayIndex >= BUFF_SIZE) {
				copy(buffer, out, arrayIndex);
				arrayIndex = 0;
			}
		}
		copy(buffer, out, arrayIndex);
	}


	/**
	 * @param aSize - size of src 
	 */
	protected void convertResult(byte [] src, TYPE [] dest, int aSize,
	 int startPos, int endPos, int skip)
	 throws GrpcException {


		int nextTarget = 0;
		boolean isThinOut = false; 
		if ((endPos < aSize) || (startPos != 0) || (skip != 1)) {
			isThinOut = true;
			nextTarget = startPos;
		}

		// variables for convert
		byte[] ngDataBuffer = new byte[BUFF_SIZE]; 
		int argSize = getSize();
		int endOfArray = aSize * argSize; // endOfArray is end position of src
		int nRead = 0;
		int current = 0;

		if (isThinOut) {
			while (current < aSize) {
				nRead = readNgData(src, ngDataBuffer,
									current * argSize,
									endOfArray);

				for (int i = 0; i < nRead; i += argSize, current++) { 
					if (current == nextTarget) {
						dest[current] = getValue(ngDataBuffer, i);
						nextTarget = current + skip;
						if (nextTarget > endPos) {
							nextTarget = -1;
						}
					}
				}
			}
		} else {
			while (current < aSize) {
				nRead = readNgData(src, ngDataBuffer,
									current * argSize,
									endOfArray);

				for (int i = 0; i < nRead; i += argSize, current++) { 
					dest[current] = getValue(ngDataBuffer, i);
				}
			}
		}
	}


///////////////////////////////////////////////////////////////////////
////     Class Members
///////////////////////////////////////////////////////////////////////

	/**
	 *  Represent Ninf-G Integer Character
	 */
	public static final NgType CHAR = new NgType<Character>(0x01, "char", 4) {
		protected int setByteArray(Character value, int i, byte[] out) {
			out[i] = out[i+1] = out[i+2] = 0;
			out[i+3] = (byte)(value.charValue()); // char size is 2byte on Java
			return 4;
		}
		protected Character getValue(byte[] src, int startPos) {
			// '3' is difference between Ninf-G Data Size and Java Data Size
			// ? 3  Is it 2?
			return (char)src[startPos + 3];
		}
	};
	/**
	 *  Represent Ninf-G Integer Short Integer
	 */
	public static final NgType SHORT = new NgType<Short>(0x02, "short", 4) {
		protected int setByteArray(Short value, int i, byte[] out) {
			out[i] = out[i+1] = 0;
			out[i+2] = (byte)((value >>>  8) & 0xFF);
			out[i+3] = (byte)((value >>>  0) & 0xFF);
			return 4;
		}
		protected Short getValue(byte[] src, int startPos) {
			int DIFF = 2;
			int ch1 = (int)src[startPos + DIFF];
			int ch2 = (int)src[startPos + DIFF + 1];
			ch1 &= 0xFF;
			ch2 &= 0xFF;
			return (short)((ch1 << 8) + (ch2 << 0));
		} 
	};
	/**
	 * Represent Ninf-G Integer
	 */ 
	public static final NgType INT = new NgType<Integer>(0x03, "int", 4) {
		protected int setByteArray(Integer value, int i, byte[] out) {
			return _setByteArrayInt(value, i, out);
		}
		protected Integer getValue(byte[] src, int startPos) {
			return _getValueInt(src, startPos);
		}
	};
	/**
	 * Represent Ninf-G Long Integer
	 */
	public static final NgType LONG = new NgType<Long>(0x04, "long", 4) {
		protected int setByteArray(Long value, int i, byte[] out) {
			return _setByteArrayLong(value, i, out);
		}
		public Long getValue(byte[] src, int startPos ) {
			return (long)_getValueInt(src, startPos);
		}
	};
	/**
	 * Represent Ninf-G Float
	 */
	public static final NgType FLOAT = new NgType<Float>(0x11, "float", 4) {
		protected int setByteArray(Float value, int i, byte[] out) {
			return _setByteArrayInt(Float.floatToIntBits(value), i, out);
		}
		protected Float getValue(byte[] src, int startPos) {
			return Float.intBitsToFloat(_getValueInt(src, startPos));
		}
	}; 
	/**
	 * Represent Ninf-G Double
	 */
	public static final NgType DOUBLE = new NgType<Double>(0x12, "double", 4) {
		protected int setByteArray(Double value, int i, byte[] out) {
			return _setByteArrayLong(Double.doubleToLongBits(value), i, out);
		}
		protected Double getValue(byte[] src, int startPos) {
			return Double.longBitsToDouble(_getValueLong(src, startPos));
		}
	};
	/**
	 * Represent Ninf-G Scomplex
	 */ 
	public static final NgType SCOMPLEX =
	 new NgType<Scomplex>(0x13, "scomplex", 8) {
		protected int setByteArray(Scomplex value, int i, byte[] out) {
			int wk_r = Float.floatToIntBits(value.r);
			int wk_i = Float.floatToIntBits(value.i);
			int index = i;
			index += _setByteArrayInt(wk_r, index, out);
			_setByteArrayInt(wk_i, index, out);
			return 8;
		}
		protected Scomplex getValue(byte[] src, int startPos) {
			Scomplex sc = new Scomplex();
			sc.r = Float.intBitsToFloat(_getValueInt(src, startPos));
			sc.i = Float.intBitsToFloat(_getValueInt(src, startPos + 4));
			return sc;
		}
	};

	/**
	 * Represent Ninf-G Scomplex
	 */ 
	public static final NgType DCOMPLEX =
	 new NgType<Dcomplex>(0x14, "dcomplex", 16) {
		protected int setByteArray(Dcomplex value, int i, byte[] out) {
			long wk_r = Double.doubleToLongBits(value.r);
			long wk_i = Double.doubleToLongBits(value.i);
			int index = i;
			index += _setByteArrayLong(wk_r, index, out);
			_setByteArrayLong(wk_i, index, out);
			return 16;
		}
		protected Dcomplex getValue(byte[] src, int startPos) {
			Dcomplex dc = new Dcomplex();
			dc.r = Double.longBitsToDouble(_getValueLong(src, startPos));
			dc.i = Double.longBitsToDouble(_getValueLong(src, startPos+4));
			return dc;
		}
	};

	/**
	 * Represent Ninf-G String
	 */ 
	public static final NgType STRING = new NgType<String>(0x21, "string", 0) {
		protected void convertArray(String [] target, int length, byte [] out) 
	 	 throws GrpcException {
			if (length >= target.length) {
				throw new ArrayIndexOutOfBoundsException();
			}
			int count = 0; // The number of valid bytes in the out
			// write all of target elements into out
			for (int i = 0; i < length; i++) {
				count += convertScalar(target[i], out, count);
			}
		}

		/**
		 * copy anTarget into out (out[0]..out[anTarget.length()-1])
		 */
		protected void convertScalar(String anTarget, byte [] out)
		 throws  GrpcException {
			convertScalar(anTarget, out, 0);
		} 

		private int convertScalar(String anTarget, byte[] out, int startPos)
		 throws GrpcException {
			byte [] local_bo = new byte[BUFF_SIZE_SMALL];
			int index = 0;

			// following if-block return the byte [] that
			//  converted [size(4byte)][string(anTarget.length byte)]
			if (anTarget != null) {
				if (anTarget.length() == 0) {
					// indicates null String ""
					index = _setByteArrayInt(1, 0, local_bo);
					local_bo[index] = (byte)0;
				} else {
					index = _setByteArrayInt(anTarget.length(), 0, local_bo);
					byte [] data = anTarget.getBytes();
					// does check byte order ?
					System.arraycopy(data, 0, local_bo, index, data.length);
				}
			} else {
				// indicates null(fill integer 0)
				index = _setByteArrayInt(0, 0, local_bo);
			}
			byte [] xdr = encodeDataXDRString(local_bo);
			//copy(xdr, out, xdr.length);
			System.arraycopy(xdr, 0, out, startPos, xdr.length);
			return xdr.length;
		}

		protected void convertResult(byte [] src, String [] dest, int aSize,
	 	 int startPos, int endPos, int skip)
	 	 throws GrpcException {

			// decode XDR  
			//int length_xdr = di.available();
			//byte buffer_xdr[] = new byte[length_xdr];
			//di.readFully(buffer_xdr);

			byte decodedBuf[] = decodeDataXDRString(src);
			if (decodedBuf.length < 1) {
				return; // there is no data
			}

			DataInputStream di_ninf =
			 new DataInputStream(new ByteArrayInputStream(decodedBuf));
			try {
				for (int i = 0; i < aSize; i++) {
					int length = di_ninf.readInt();
					byte buffer[] = new byte[length];
					di_ninf.readFully(buffer);

					dest[i] = new String(buffer);
				}
				di_ninf.close();
			} catch (IOException e) {
				throw new NgIOException(e);
			} 
		}
		
		protected int setByteArray(String value, int i, byte [] out) {
			return 0;
		}

		protected String getValue(byte[] src, int startPos) {
			return "";
		}


	};

	/**
	 * Represent Ninf-G Filename 
	 *  convertArray() & convertScalar() delegate to STRING:NgType
	 */ 
	public static final NgType FILENAME =
	 new NgType<String>(0x22, "filename", 0) { 
		protected void convertArray(String [] targets, int length, byte [] out)
	 	 throws GrpcException {
			STRING.convertArray(targets, length, out);
		}
		protected void convertScalar(String anTarget, byte [] out)
		 throws  GrpcException {
			STRING.convertScalar(anTarget, out);
		}

		protected void convertResult(byte [] src, String [] dest, int aSize,
	 	 int startPos, int endPos, int skip)
	 	 throws GrpcException {
			return ;
		}
		
		protected int setByteArray(String value, int i, byte [] out) {
			return 0;
		}
		protected String getValue(byte[] src, int startPos) {
			return "";
		}
	}; 

	/**
	 * Represent Ninf-G Callback
	 */ 
	public static final NgType CALLBACK =
	 new NgType<String>(0x31, "callback", 0) {
		protected void convertArray(String [] targets, int length, byte [] out)
		 throws GrpcException {
			return ; // do nothing
		}
		protected void convertScalar(String anTarget, byte [] out)
		 throws  GrpcException {
			return ; // do nothing
		}
		protected void convertResult(byte [] src, String [] dest, int aSize,
	 	 int startPos, int endPos, int skip)
	 	 throws GrpcException {
			return ; // do nothing
		}
		
		protected int setByteArray(String value, int i, byte [] out) {
			return 0;
		}
		protected String getValue(byte[] src, int startPos) {
			return "";
		}
	};

}
