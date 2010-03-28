/*
 * $RCSfile: NgParamTypes.java,v $ $Revision: 1.6 $ $Date: 2007/09/26 04:14:07 $
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

public class NgParamTypes {

	// type 
	public static final int NG_TYPE_UNDEF = 0x00;
	public static final int NG_TYPE_CHAR = 0x01;
	public static final int NG_TYPE_SHORT = 0x02;
	public static final int NG_TYPE_INT = 0x03;
	public static final int NG_TYPE_LONG = 0x04;
	public static final int NG_TYPE_FLOAT = 0x11;
	public static final int NG_TYPE_DOUBLE = 0x12;
	public static final int NG_TYPE_SCOMPLEX = 0x13;
	public static final int NG_TYPE_DCOMPLEX = 0x14;
	public static final int NG_TYPE_STRING = 0x21;
	public static final int NG_TYPE_FILENAME = 0x22;
	public static final int NG_TYPE_CALLBACK = 0x31;

	private static final int bufferSize = NgGlobals.paramInputBufferSize;
	
	// type array 
	private static final int[] typeArray = {
		NG_TYPE_UNDEF,
		NG_TYPE_CHAR,
		NG_TYPE_SHORT,
		NG_TYPE_INT,
		NG_TYPE_LONG,
		NG_TYPE_FLOAT,
		NG_TYPE_DOUBLE,
		NG_TYPE_SCOMPLEX,
		NG_TYPE_DCOMPLEX,
		NG_TYPE_STRING,
		NG_TYPE_FILENAME,
		NG_TYPE_CALLBACK
	};
	
	// name array
	private static final String[] typeNameArray = {
		"dummy",
		"char",
		"short",
		"int",
		"long",
		"float",
		"double",
		"scomplex",
		"dcomplex",
		"string",
		"filename",
		"callback"
	};
	
	// size array 
	private static final int[] sizeArray = {
		0, // undef 
		4, // char 
		4, // short
		4, // int 
		4, // long
		4, // float
		8, // double
		8, // scomplex
		16, // dcomplex
		0, // string
		0, // filename
		0 // callback
	};

	// mode
	public static final int NG_MODE_NONE = 0;
	public static final int NG_MODE_IN = 1;
	public static final int NG_MODE_OUT = 2;
	public static final int NG_MODE_INOUT = 3;
	public static final int NG_MODE_WORK = 4;


	// expression 
	public static final int NG_EXP_VALUE_NONE   = 0;
	public static final int NG_EXP_VALUE_CONST  = 1;
	public static final int NG_EXP_VALUE_IN_ARG = 2;
	public static final int NG_EXP_VALUE_OP     = 3;
	public static final int NG_EXP_VALUE_END_OF_OP = 4;


	/**
	 * @param type
	 * @return
	 */
	public static int getSize(int type) {
		for (int i = 0; i < typeArray.length; i++) {
			if (type == typeArray[i])  {
				return sizeArray[i];
			}
		}
		// couldn't find
		throw new IllegalArgumentException("NgParamType: Unknown Type.");
	}
	
	/**
	 * @param type
	 * @param array
	 * @param bo
	 * @throws GrpcException
	 */
	protected static void transformArg(int type, int length, 
	 Object array, ByteArrayOutputStream bo)
	 throws GrpcException {

		int arrayIndex = 0;
		
		switch (type) {
			case NG_TYPE_CHAR:
				byte[] byteArray = (byte[])array;
				byte[] buffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(buffer, arrayIndex, byteArray[i]);
					if (arrayIndex >= bufferSize) {
						// write all of byte[] elements into bo 
						try {
							bo.write(buffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of byte[] elements into bo 
				bo.write(buffer, 0, arrayIndex);
				break;
			case NG_TYPE_SHORT:
				short[] shortArray = (short[])array;
				byte[] shortBuffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(shortBuffer, arrayIndex, shortArray[i]);
					if (arrayIndex >= bufferSize) {
						// write all of short[] elements into bo 
						try {
							bo.write(shortBuffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of short[] elements into bo 
				bo.write(shortBuffer, 0, arrayIndex);
				break;
			case NG_TYPE_INT:
				int[] intArray = (int[])array;
				byte[] intBuffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(intBuffer, arrayIndex, intArray[i]);
					if (arrayIndex >= bufferSize) {
						// write all of int[] elements into bo 
						try {
							bo.write(intBuffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of int[] elements into bo 
				bo.write(intBuffer, 0, arrayIndex);
				break;
			case NG_TYPE_LONG:
				long[] longArray = (long[])array;
				byte[] longBuffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(longBuffer, arrayIndex, (int)longArray[i]);
					if (arrayIndex >= bufferSize) {
						// write all of long[] elements into bo 
						try {
							bo.write(longBuffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of long[] elements into bo 
				bo.write(longBuffer, 0, arrayIndex);
				break;
			case NG_TYPE_FLOAT:
				float[] floatArray = (float[])array;
				byte[] floatBuffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(floatBuffer, arrayIndex, Float.floatToIntBits(floatArray[i]));
					if (arrayIndex >= bufferSize) {
						// write all of float[] elements into bo 
						try {
							bo.write(floatBuffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of float[] elements into bo 
				bo.write(floatBuffer, 0, arrayIndex);
				break;
			case NG_TYPE_DOUBLE:
				double[] doubleArray = (double[])array;
				byte[] doubleBuffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(doubleBuffer, arrayIndex, Double.doubleToLongBits(doubleArray[i]));
					if (arrayIndex >= bufferSize) {
						// write all of double[] elements into bo 
						try {
							bo.write(doubleBuffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of double[] elements into bo 
				bo.write(doubleBuffer, 0, arrayIndex);
				break;
			case NG_TYPE_SCOMPLEX:
				Scomplex[] scomplexArray = (Scomplex [])array;
				byte[] scomplexBuffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(scomplexBuffer, arrayIndex, Float.floatToIntBits(scomplexArray[i].r));
					arrayIndex += setByteArray(scomplexBuffer, arrayIndex, Float.floatToIntBits(scomplexArray[i].i));
					if (arrayIndex >= bufferSize) {
						// write all of scomplex[] elements into bo 
						try {
							bo.write(scomplexBuffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of scomplex[] elements into bo 
				bo.write(scomplexBuffer, 0, arrayIndex);
				break;
			case NG_TYPE_DCOMPLEX:
				Dcomplex[] dcomplexArray = (Dcomplex [])array;
				byte[] dcomplexBuffer = new byte[bufferSize];
				for (int i = 0; i < length; i++) {
					arrayIndex += setByteArray(dcomplexBuffer, arrayIndex, Double.doubleToLongBits(dcomplexArray[i].r));
					arrayIndex += setByteArray(dcomplexBuffer, arrayIndex, Double.doubleToLongBits(dcomplexArray[i].i));
					if (arrayIndex >= bufferSize) {
						// write all of dcomplex[] elements into bo 
						try {
							bo.write(dcomplexBuffer);
						} catch (IOException e) {
							throw new NgIOException(e);
						}
						arrayIndex = 0;
					}
				}
				// write all of dcomplex[] elements into bo
				bo.write(dcomplexBuffer, 0, arrayIndex);
				break;
			case NG_TYPE_STRING:
			case NG_TYPE_FILENAME:
				String[] stringArray = (String[])array;
				// write all of String[] elements into bo 
				for (int i = 0; i < length; i++) {
					transformScalarArg(type, stringArray[i], bo);
				}
				break;
			case NG_TYPE_CALLBACK:
				break;

			default:
				// didn't match any type... 
				throw new NgArgTypeException("NgParamType: unknown type");
		}
	}

	private static int setByteArray(byte[] buffer, int i, byte value) {
		buffer[i  ] = 0;
		buffer[i+1] = 0;
		buffer[i+2] = 0;
		buffer[i+3] = value;
		return 4;
	}

	private static int setByteArray(byte[] buffer, int index, short value) {
		buffer[index]   = 0;
		buffer[index+1] = 0;
		buffer[index+2] = (byte)((value >>>  8) & 0xFF);
		buffer[index+3] = (byte)((value >>>  0) & 0xFF);
		return 4;
	}

	private static int setByteArray(byte[] buffer, int index, int value) {
		buffer[index]   = (byte)((value >>> 24) & 0xFF);
		buffer[index+1] = (byte)((value >>> 16) & 0xFF);
		buffer[index+2] = (byte)((value >>>  8) & 0xFF);
		buffer[index+3] = (byte)((value >>>  0) & 0xFF);
		return 4;
	}

	private static int setByteArray(byte[] buffer, int index, long value) {
		buffer[index]   = (byte)((value >>> 56) & 0xFF);
		buffer[index+1] = (byte)((value >>> 48) & 0xFF);
		buffer[index+2] = (byte)((value >>> 40) & 0xFF);
		buffer[index+3] = (byte)((value >>> 32) & 0xFF);
		buffer[index+4] = (byte)((value >>> 24) & 0xFF);
		buffer[index+5] = (byte)((value >>> 16) & 0xFF);
		buffer[index+6] = (byte)((value >>>  8) & 0xFF);
		buffer[index+7] = (byte)((value >>>  0) & 0xFF);
		return 8;
	}
	
	/**
	 * @param type
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	protected static void transformScalarArg(int type, Object target,
	 ByteArrayOutputStream bo)
	 throws GrpcException {

		// Write argument target into argument bo
		switch (type) {
		case NG_TYPE_CHAR:
			transformScalar( ((Byte)target).byteValue(), bo);
			break;
		case NG_TYPE_SHORT:
			transformScalar( ((Short)target).shortValue(), bo);
			break;
		case NG_TYPE_INT:
			transformScalar(((Integer)target).intValue(), bo);
			break;
		case NG_TYPE_LONG:
			transformScalar(((Long)target).longValue(), bo);
			break;
		case NG_TYPE_FLOAT:
			transformScalar(((Float)target).floatValue(), bo);
			break;
		case NG_TYPE_DOUBLE:
			transformScalar(((Double)target).doubleValue(), bo);
			break;
		case NG_TYPE_SCOMPLEX:
			transformScalar(((Scomplex)target), bo);
			break;
		case NG_TYPE_DCOMPLEX:
			transformScalar(((Dcomplex)target), bo);
			break;
		case NG_TYPE_STRING:
		case NG_TYPE_FILENAME:
			transformScalar(((String)target), bo);
			break;
		case NG_TYPE_CALLBACK:
			break;
		default:
			// didn't match any type... 
			throw new NgArgTypeException("NgParamType: unknown type");
		}
	}
	
	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(byte target, ByteArrayOutputStream bo)
	 throws GrpcException {
		byte[] buffer = new byte[getSize(NG_TYPE_CHAR)];
		
		// set variable to buffer 
		buffer[0] = 0;
		buffer[1] = 0;
		buffer[2] = 0;
		buffer[3] = target;
		// setByteArray(target, bo);
		bo.write(buffer, 0, buffer.length);
	}
	
	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(short target, ByteArrayOutputStream bo)
	 throws GrpcException {
		int type = NG_TYPE_SHORT;
		int argSize = getSize(type);
		byte[] buffer = new byte[argSize];
		int shiftSize = 8 * (getSize(type) - 1);
		
		// transform short value into NgParam
		buffer[0] = 0;
		buffer[1] = 0;
		shiftSize -= 16;
		for (int j = 2; j < getSize(type); j++) {
			if (shiftSize < 0) {
				throw new NgArgSizeException("NgParamType: wrong size");
			}
			buffer[j] = (byte)((target >> shiftSize) & 0x00ff);
			shiftSize -= 8;
		}
		bo.write(buffer, 0, argSize);
	}

	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(int target, ByteArrayOutputStream bo)
	 throws GrpcException {
		int type = NG_TYPE_INT;
		int argSize = getSize(type);
		byte[] buffer = new byte[argSize];
		int shiftSize = 8 * (getSize(type) - 1);
		
		// transform int value into NgParam 
		for (int j = 0; j < getSize(type); j++) {
			if (shiftSize < 0) {
				throw new NgArgSizeException("NgParamType: wrong size");
			}
			buffer[j] = (byte)((target >> shiftSize) & 0x00ff);
			shiftSize -= 8;
		}
		bo.write(buffer, 0, argSize);
	}
	
	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(long target, ByteArrayOutputStream bo)
	 throws GrpcException {
		int type = NG_TYPE_LONG;
		int argSize = getSize(type);
		byte[] buffer = new byte[argSize];
		int shiftSize = 8 * (getSize(type) - 1);
		int intTarget = (int)target;
		
		// transform int value into NgParam 
		for (int j = 0; j < getSize(type); j++) {
			if (shiftSize < 0) {
				throw new NgArgSizeException( "NgParamType: wrong size");
			}
			buffer[j] = (byte)((intTarget >> shiftSize) & 0x00ff);
			shiftSize -= 8;
		}
		bo.write(buffer, 0, argSize);
	}
	
	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(float target, ByteArrayOutputStream bo)
	 throws GrpcException {
		// put float data to DataOutputStream 
		DataOutputStream dos = new DataOutputStream(bo);
		try {
			dos.writeFloat(target);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(double target, ByteArrayOutputStream bo)
	 throws GrpcException {
		// put float data to DataOutputStream/
		DataOutputStream dos = new DataOutputStream(bo);
		try {
			dos.writeDouble(target);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(Scomplex target,
	 ByteArrayOutputStream bo) throws GrpcException {
		// put float data to DataOutputStream 
		DataOutputStream dos = new DataOutputStream(bo);
		try {
			dos.writeFloat(target.r);
			dos.writeFloat(target.i);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
		
	/**
	 * @param target
	 * @param bo
	 * @throws GrpcException
	 */
	private static void transformScalar(Dcomplex target,
	 ByteArrayOutputStream bo) throws GrpcException {
		// put float data to DataOutputStream 
		DataOutputStream dos = new DataOutputStream(bo);
		try {
			dos.writeDouble(target.r);
			dos.writeDouble(target.i);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
		
	/**
	 * @param string
	 * @param bo
	 */
	private static void transformScalar(String string, ByteArrayOutputStream bo)
	 throws GrpcException {
		ByteArrayOutputStream local_bo =
		 new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		DataOutputStream dos = new DataOutputStream(local_bo);
		try {
			if (string != null) {
				if (string.length() == 0) {
					// indicates null String "" 
					dos.writeInt(1);
					byte nullByte = 0;
					dos.write(nullByte);
				} else {
					dos.writeInt(string.length());
					dos.write(string.getBytes());
				}
			} else {
				// indicates null 
				dos.writeInt(0);
			}

			// convert to XDR 
			bo.write(encodeDataXDRString(local_bo.toByteArray()));
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * @param type
	 * @param bi
	 * @return
	 */
	protected static void transformResult(int type, ByteArrayInputStream bi,
	 Object array, int size, int start, int end, int skip)
	 throws GrpcException {

		// array is Output argument

		DataInputStream di = new DataInputStream(bi);
		boolean flagSkip = false;
		int nextTarget = 0;
		if ((end < size) || (start != 0) || (skip != 1)) {
			flagSkip = true;
			nextTarget = start;
		}
		
		// variables for convert 
		byte[] inputBuffer = null;
		int arrayIndex = 0;
		int currentIndex = 0;
		int endOfArray = 0;
		int argSize = 0;
		int nRead = 0;

		try {
			switch (type) {
			case NG_TYPE_CHAR:
				// get byte array from Object
				byte[] byteArray = (byte[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 4;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {

							if (currentIndex == nextTarget) {
								byteArray[currentIndex] =
								 inputBuffer[arrayIndex + 3];
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}

						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0; 
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {

							byteArray[currentIndex] =
							 inputBuffer[arrayIndex + 3];
						}
					}
				}
				break; 
			case NG_TYPE_SHORT:
				// get short array from Object 
				short[] shortArray = (short[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 4;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {

							if (currentIndex == nextTarget) {
								shortArray[currentIndex] =
								 getShortValue(inputBuffer, arrayIndex + 2);
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}
						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {

							shortArray[currentIndex] =
							 getShortValue(inputBuffer, arrayIndex + 2);

						}
					}
				}
				break;

			case NG_TYPE_INT:
				// get int array from Object
				int[] intArray = (int[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 4;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {

							if (currentIndex == nextTarget) {
								intArray[currentIndex] =
								 getIntValue(inputBuffer, arrayIndex);
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}

						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							intArray[currentIndex] =
							 getIntValue(inputBuffer, arrayIndex);
						}
					}
				}
				break;

			case NG_TYPE_LONG:
				// get long array from Object 
				long[] longArray = (long[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 4;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							if (currentIndex == nextTarget) {
								longArray[currentIndex] =
								 (long)getIntValue(inputBuffer, arrayIndex);
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}
						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							longArray[currentIndex] =
							 (long)getIntValue(inputBuffer, arrayIndex);
						}
					}
				}
				break;

			case NG_TYPE_FLOAT:
				// get float array from Object 
				float[] floatArray = (float[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 4;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
						 currentIndex * argSize, endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							if (currentIndex == nextTarget) {
								floatArray[currentIndex] =
								 Float.intBitsToFloat(
								  getIntValue(inputBuffer, arrayIndex));
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}
						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							floatArray[currentIndex] =
							 Float.intBitsToFloat(
							  getIntValue(inputBuffer, arrayIndex));
						}
					}
				}
				break;

			case NG_TYPE_DOUBLE:
				// get double array from Object 
				double[] doubleArray = (double[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 8;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
						 currentIndex * argSize, endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							if (currentIndex == nextTarget) {
								doubleArray[currentIndex] =
								 Double.longBitsToDouble(
								  getLongValue(inputBuffer, arrayIndex));
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}
						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							doubleArray[currentIndex] =
							 Double.longBitsToDouble(
							  getLongValue(inputBuffer, arrayIndex));
						}
					}
				}
				break;

			case NG_TYPE_SCOMPLEX:
				// get Scomplex array from Object 
				Scomplex[] scomplexArray = (Scomplex[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 8;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							if (currentIndex == nextTarget) {
								scomplexArray[currentIndex].r =
								 Float.intBitsToFloat(
								  getIntValue(inputBuffer, arrayIndex));
								scomplexArray[currentIndex].i =
								 Float.intBitsToFloat(
								  getIntValue(inputBuffer, arrayIndex + 4));
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}
						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							scomplexArray[currentIndex].r =
							 Float.intBitsToFloat(
							  getIntValue(inputBuffer, arrayIndex));
							scomplexArray[currentIndex].i =
							 Float.intBitsToFloat(
							  getIntValue(inputBuffer, arrayIndex + 4));
						}
					}
				}
				break;

			case NG_TYPE_DCOMPLEX:
				// get Dcomplex array from Object 
				Dcomplex[] dcomplexArray = (Dcomplex[])array;

				inputBuffer = new byte[bufferSize];
				argSize = 16;
				endOfArray = size * argSize;

				if (flagSkip) {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							if (currentIndex == nextTarget) {
								dcomplexArray[currentIndex].r =
								 Double.longBitsToDouble(
								  getLongValue(inputBuffer, arrayIndex));
								dcomplexArray[currentIndex].i =
								 Double.longBitsToDouble(
								  getLongValue(inputBuffer, arrayIndex + 8));
								nextTarget = currentIndex + skip;
								if (nextTarget > end) {
									nextTarget = -1;
								}
							}
						}
					}
				} else {
					while (currentIndex < size) {
						nRead = getNextData(di, inputBuffer,
											currentIndex * argSize,
											endOfArray);

						for (arrayIndex = 0;
							 arrayIndex < nRead;
							 arrayIndex += argSize, currentIndex++) {
							dcomplexArray[currentIndex].r =
							 Double.longBitsToDouble(
							  getLongValue(inputBuffer, arrayIndex));
							dcomplexArray[currentIndex].i =
							 Double.longBitsToDouble(
							  getLongValue(inputBuffer, arrayIndex + 8));
						}
					}
				}
				break;

			case NG_TYPE_STRING:
				// decode XDR 
				int length_xdr = di.available();
				byte buffer_xdr[] = new byte[length_xdr];
				di.readFully(buffer_xdr);
				byte decodedBuf[] = decodeDataXDRString(buffer_xdr);
				if (decodedBuf.length < 1) {
					// there is no data 
					break;
				}
				ByteArrayInputStream bi_ninf =
				 new ByteArrayInputStream(decodedBuf);
				DataInputStream di_ninf = new DataInputStream(bi_ninf);
				// get string from Object(Argument) 
				String[] stringArray = (String[])array;
				for (int i = 0; i < size; i++) { // size == Argument
					int length = di_ninf.readInt();
					byte buffer[] = new byte[length];
					di_ninf.readFully(buffer);

					stringArray[i] = new String(buffer);
				}
				di_ninf.close();
				break;

			case NG_TYPE_FILENAME:
				break;

			case NG_TYPE_CALLBACK:
				break;

			default:
				// didn't match any type... 
				throw new NgArgTypeException("NgParamType: unknown type");
			} // end of switch()
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	private static int getNextData(DataInputStream di, byte[] buffer,
	 int currentArrayIndex, int endOfArray) throws GrpcException {
		int numberToRead = 0;

		if ((endOfArray - currentArrayIndex) > bufferSize) {
			numberToRead = bufferSize;
		} else {
			numberToRead = endOfArray - currentArrayIndex;
		}

		try {
			return di.read(buffer, 0, numberToRead);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	private static short getShortValue(byte[] buffer, int index) {
		int ch1 = (int)buffer[index];
		int ch2 = (int)buffer[index+1];
		ch1 &= 0xff;
		ch2 &= 0xff;
		return (short)((ch1 << 8) + (ch2 << 0));
	}

	private static int getIntValue(byte[] buffer, int index) {
		int ch1 = (int)buffer[index];
		int ch2 = (int)buffer[index+1];
		int ch3 = (int)buffer[index+2];
		int ch4 = (int)buffer[index+3];
		ch1 &= 0xff;
		ch2 &= 0xff;
		ch3 &= 0xff;
		ch4 &= 0xff;
		return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0));
	}
	
	private static long getLongValue(byte[] buffer, int index) {
		long tmp = getIntValue(buffer, index);
		return (tmp << 32L) + (getIntValue(buffer, index + 4) & 0xFFFFFFFFL);
	}
	
	/**
	 * @param mode
	 * @return value of mode
	 * @throws GrpcException
	 */
	public static int getModeVal(String mode) throws GrpcException {
		try {
			return NgMode.get(mode).value();
		} catch (IllegalArgumentException e) {
			throw new NgArgTypeException("Invalid mode : " + mode);
		}
	}
	
	/**
	 * @param mode
	 * @return name of mode
	 * @throws GrpcException
	 */
	public static String getModeStr(int mode) throws GrpcException {
		try {
			return NgMode.get(mode).toString();
		} catch (IllegalArgumentException e) {
			throw new NgArgTypeException("Invalid mode val : " + mode);
		}
	}

	/**
	 * @param aType
	 * @return
	 * @throws GrpcException
	 */
	public static int getTypeVal(String aType) throws GrpcException {
		for (int i = 0; i < typeNameArray.length; i++) {
			if (typeNameArray[i].equals(aType)) {
				return typeArray[i];
			}
		}
		throw new NgArgTypeException("invalid type : " + aType);
	}

	/**
	 * @param type
	 * @return
	 * @throws GrpcException
	 */
	public static String getTypeStr(int type) throws GrpcException {
		for (int i = 0; i < typeArray.length; i++) {
			if (typeArray[i] == type) {
				return typeNameArray[i];
			}
		}
		throw new NgArgTypeException("invalid type val : " + type);
	}

	/**
	 * @param buffer
	 * @return
	 * @throws GrpcException
	 */
	private static byte[] encodeDataXDRString(byte buffer[])
	 throws GrpcException {
		ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
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
				di.read(strBuffer);
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
		}
		// close stream 
		try {
			bo.close();
			bi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return bo.toByteArray();
	}

	/**
	 * @param originalData
	 * @return
	 * @throws GrpcException
	 */
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
				if (bufLen == 0) {
					break;
				}
				// read string 
				byte[] strBuf = new byte[bufLen];
				xi.read(strBuf);

				ByteArrayInputStream bi_ninf = new ByteArrayInputStream(strBuf);
				XDRInputStream xi_ninf = new XDRInputStream(bi_ninf);
				ByteArrayOutputStream bo_ninf =
				 new ByteArrayOutputStream(NgGlobals.smallBufferSize);

				try {
					bo_ninf.write(xi_ninf.readString().getBytes());
				} catch (IOException e) {
					throw new NgIOException(e);
				}
				xi_ninf.close();
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
			
		// close stream 
		try {
			bi.close();
			bo.close();
		} catch (IOException e1) {
			throw new NgIOException(e1);
		}

		return bo.toByteArray();
	}
}
