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
 * $RCSfile: CallContext.java,v $ $Revision: 1.49 $ $Date: 2005/07/12 10:32:37 $
 */
package org.apgrid.grpc.ng;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.info.*;
import org.gridforum.gridrpc.GrpcException;

public class CallContext {
	private int numParams;
	private int[] size;
	private int[] count;
	private int[] intArgs;
	private List args;
	private byte[][] data;
	private RemoteMethodInfo remoteMethodInfo;

	private static final int ENCODE_HEADER_LEN = 8;
	
	/* for callback */
	private List callbackFunc = new Vector();
	private List callbackFuncInfo = new Vector();
	
	/**
	 * @param remoteMethodInfo
	 * @param args
	 */
	public CallContext(RemoteMethodInfo remoteMethodInfo,
		List args) throws GrpcException {
		this.numParams = remoteMethodInfo.getNumParams();
		this.size = new int[numParams];
		this.count = new int[numParams];
		this.remoteMethodInfo = remoteMethodInfo;
		this.args = args;
		this.intArgs = setupIntArguments();
		this.data = new byte[numParams][];
		int callbackIndex = 0;
		
		/* transform arguments into byteArray */
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			/* get information for argument */
			RemoteMethodArg remoteMethodArg = 
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
				
			/* transform java value data into Ninf value data */
			if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_FILENAME) {
				/* set size */
				size[i] = NgParamTypes.getSize(remoteMethodArg.getType());
				/* check if it's null */
				if (args.get(i) == null) {
					count[i] = 0;
					data[i] = new byte[ENCODE_HEADER_LEN];
					continue;
				}
				/* transform args and set byte array */
				byte[] buffer =	transformArg(remoteMethodArg.getNDims(),
					args.get(i),
					remoteMethodArg, true);
					
				/* add created buffer into list */
				data[i] = buffer;
				
				/* set count */
				if (remoteMethodArg.getNDims() > 0) {
					RemoteMethodArgSubScript remoteMethodArgSubScript =
						remoteMethodArg.getRemoteMethodArgSubscript(0);

					NgExpression expSize = remoteMethodArgSubScript.getSize();	
								
					/* set end to size */
					count[i] = (int) expSize.calc(intArgs);
				} else {
					count[i] = 1;
				}
			} else if (remoteMethodArg.getMode() == NgParamTypes.NG_MODE_IN ||
				remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT) {
				/* set size */
				size[i] = NgParamTypes.getSize(remoteMethodArg.getType());
				/* check if it's null */
				if (remoteMethodArg.getType() != NgParamTypes.NG_TYPE_STRING &&
					args.get(i) == null) {
					count[i] = 0;
					data[i] = new byte[ENCODE_HEADER_LEN];
					continue;
				}
				/* transform args and set byte array */
				byte[] buffer =	transformArg(remoteMethodArg.getNDims(),
					args.get(i),
					remoteMethodArg, true);
					
				/* add created buffer into list */
				data[i] = buffer;
				
				/* set count */
				if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_STRING) {
					if (remoteMethodArg.getNDims() > 0) {
						RemoteMethodArgSubScript remoteMethodArgSubScript =
							remoteMethodArg.getRemoteMethodArgSubscript(0);

						NgExpression expSize = remoteMethodArgSubScript.getSize();	
								
						/* set end to size */
						count[i] = (int) expSize.calc(intArgs);
					} else {
						count[i] = 1;
					}
				} else {
					count[i] = (buffer.length - ENCODE_HEADER_LEN) / size[i];
				}
			} else if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_CALLBACK) {
				/* check if it's null */
				if (args.get(i) == null) {
					throw new NgExecRemoteMethodException(
						"callback argument is null.");
				}
				/* Add callback object into list */
				callbackFunc.add(args.get(i));
				
				/* put RemoteMethodInfo for the callback */
				RemoteMethodInfo callbackInfo = remoteMethodArg.getCallbackInfo();
				callbackInfo.setShrink(remoteMethodInfo.getShrink());
				callbackFuncInfo.add(callbackInfo);
			} else {
				/* set null */
				data[i] = null;
			}
		}
	}
	
	/**
	 * @param remoteMethodInfo
	 * @param argsData
	 * @throws GrpcException
	 */
	public CallContext(RemoteMethodInfo remoteMethodInfo,
		byte[][] argsData, int[] intArguments) throws GrpcException {
		/* convert Argument for callback and put them into List */
		this.numParams = remoteMethodInfo.getNumParams();
		this.size = new int[numParams];
		this.count = new int[numParams];
		this.remoteMethodInfo = remoteMethodInfo;
		this.args = new Vector();
		this.intArgs = intArguments;
		this.data = new byte[numParams][0];
		
		/* Initialize byte array and argument list */
		int dataIndex = 0;
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg rma =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			if ((rma.getMode() == NgParamTypes.NG_MODE_INOUT) ||
				(rma.getMode() == NgParamTypes.NG_MODE_IN)) {
				this.data[i] = argsData[dataIndex++];
			}

			/* Initialize list for arguments */
			Object target = initArray(rma.getNDims(), rma);
			if (target == null) {
				target = "dummy";
			}

			/* put it into argument list */
			args.add(target);
		}
		
		/* transform received data */
		transformCBArg();
		
		/* convert String */
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg rma =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			if ((rma.getType() == NgParamTypes.NG_TYPE_STRING) &&
				((rma.getMode() == NgParamTypes.NG_MODE_INOUT) ||
				(rma.getMode() == NgParamTypes.NG_MODE_IN))) {
				args.set(i, (String []) args.get(i));
			}
		}
	}
	
	/**
	 * @return
	 */
	private int[] setupIntArguments() {
		int numParams = remoteMethodInfo.getNumParams();
		int[] intArguments = new int[numParams];
		
		/* check int values in arguments */
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			if ((remoteMethodArg.getMode() != NgParamTypes.NG_MODE_IN) &&
				(remoteMethodArg.getMode() != NgParamTypes.NG_MODE_INOUT)) {
				/* nothing will be done */
				continue;
			}

			if (remoteMethodArg.getNDims() == 0) {
				if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_CHAR)) {
					Byte tmpByte = (Byte) args.get(i);
					intArguments[i] = tmpByte.intValue();
				} else if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_SHORT)) {
					Short tmpShort = (Short) args.get(i);
					intArguments[i] = tmpShort.intValue();
				} else if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_INT)) {
					Integer tmpInt = (Integer) args.get(i);
					intArguments[i] = tmpInt.intValue();
				} else if ((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_LONG)) {
					Long tmpLong = (Long) args.get(i);
					intArguments[i] = tmpLong.intValue();
				}
			}
		}
		
		return intArguments;
	}
	
	/**
	 * @return
	 */
	public int[] getIntArguments() {
		return intArgs;
	}

	/**
	 * @param nDims
	 * @param target
	 * @param remoteMethodArg
	 * @return
	 */
	private byte[] transformArg(int nDims, Object target,
		RemoteMethodArg remoteMethodArg, boolean firstFlag) throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.argBufferSize);
		
		/* get size, start, end, skip */
		int start = 0;
		int end = 1;
		int skip = 1;
		if (nDims >= 1) {
			RemoteMethodArgSubScript remoteMethodArgSubScript =
				remoteMethodArg.getRemoteMethodArgSubscript(nDims - 1);

			NgExpression expSize = remoteMethodArgSubScript.getSize();			
			/* set end to size */
			end = (int) expSize.calc(intArgs);
		}
		
		if (nDims > 1) {
			/* check if it's string */
			if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_STRING) {
				throw new NgArgTypeException("Invalid String type array.");
			}
			
			/* get byte[] from target */
			Object[] targetArray = (Object[]) target;
		
			/* decrement nDims */
			nDims--;
			
			for (int i = start; i < end; i += skip) {
				/* call this method again */
				byte[] buffer = transformArg(nDims,
					targetArray[i], remoteMethodArg, false);
				bo.write(buffer, 0, buffer.length);
			}
		} else if ( nDims == 1 ){
			/* transform and write data of Ninf Param */
			NgParamTypes.transformArg(remoteMethodArg.getType(), end,
				target, bo);
		} else {
			/* get byte[] from target */
			Object targetValue = (Object) target;
		
			/* transform and write data of Ninf Param */
			NgParamTypes.transformScalarArg(remoteMethodArg.getType(),
				targetValue, bo);
		}
		
		/* return created buffer */
		byte[] returnBuffer;
		try {
			if (firstFlag == true) {
				byte[] dummyBuffer = new byte[ENCODE_HEADER_LEN];
				bo.write(dummyBuffer);
			}
			returnBuffer = bo.toByteArray();
			bo.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		return returnBuffer;
	}

	/**
	 * @return
	 */
	public int getCount(int index) {
		return count[index];
	}
	
	/**
	 * @return
	 */
	public byte[] getData(int index) {
		return data[index];
	}
	
	/**
	 * @return
	 */
	public int getSize(int index) {
		return size[index];
	}
	
	/**
	 * @param args
	 * @return
	 */
	protected void transformResult(List args) throws GrpcException {
		/* check all of MODE_{IN,}OUT parameters */
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			if ((remoteMethodArg.getMode() == NgParamTypes.NG_MODE_OUT) ||
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)) {

				/* put argElement into bi */
				if (data[i] != null) {
					ByteArrayInputStream bi = new ByteArrayInputStream(data[i]);
					Object target = args.get(i);
					if ((bi.available() < 1 ) || (target == null)) {
						continue;
					}
					transformResult(remoteMethodArg.getNDims(),
						bi,	target,	remoteMethodArg);
				}
			}
		}
	}

	/**
	 * @param nDims
	 * @param bi
	 * @param remoteMethodArg
	 * @return
	 */
	private void transformResult(int nDims, ByteArrayInputStream bi,
		Object arg,	RemoteMethodArg remoteMethodArg)
		throws GrpcException {
		/* get size, start, end, skip */
		RemoteMethodArgSubScript remoteMethodArgSubScript =
			remoteMethodArg.getRemoteMethodArgSubscript(nDims - 1);
		int size = 0;
		int start = 0;
		int end = 1;
		int skip = 1;
		int elemSize = 0;

		if (remoteMethodArgSubScript != null) {
			int argSize;
			NgExpression expSize = remoteMethodArgSubScript.getSize();
			NgExpression expStart = remoteMethodArgSubScript.getStart();
			NgExpression expEnd = remoteMethodArgSubScript.getEnd();
			NgExpression expSkip = remoteMethodArgSubScript.getSkip();
			
			/* set size, start, end, skip */
			size = (int) expSize.calc(intArgs);
			start = (int) expStart.calc(intArgs);
			end = (int) expEnd.calc(intArgs);
			skip = (int) expSkip.calc(intArgs);
			
			/* end was omitted */
			if (end == 0) {
				end = (int) expSize.calc(intArgs);
			}
			/* skip was omitted */
			if (skip == 0) {
				skip = 1;
			}
			
			/* get size of 1 elem */
			elemSize = getElemSize(nDims - 1, remoteMethodArg);
		}
		
		/* get Object[] from byte[] */
		if (nDims > 1) {
			/* create array */
			Object[] array = (Object[])arg;
			/* decrement nDims */
			nDims--;
			/* set elements */
			int nextTarget = start;
			for (int i = 0; i < size; i++) {
				if (i == nextTarget) {
					transformResult(nDims,
							bi, array[i], remoteMethodArg);
					nextTarget = i + skip;
					if (nextTarget > end) {
						nextTarget = -1;
					}
				} else {
					/* drop data */
					byte[] buffer = new byte[elemSize];
					bi.read(buffer, 0, elemSize);
				}
			}
		} else {
			/* set elements */
			NgParamTypes.transformResult(
				remoteMethodArg.getType(), bi, arg, size, start, end, skip);
		}
	}
	
	/**
	 * @param nDims
	 * @param remoteMethodArg
	 * @return
	 * @throws GrpcException
	 */
	private int getElemSize(int nDims, RemoteMethodArg remoteMethodArg)
		throws GrpcException {
		int sizeElem = NgParamTypes.getSize(remoteMethodArg.getType());
		for (int i = 0; i < nDims; i++) {
			RemoteMethodArgSubScript argSubscript =
				remoteMethodArg.getRemoteMethodArgSubscript(nDims);
			NgExpression expSize = argSubscript.getSize();

			sizeElem *= expSize.calc(intArgs);
		}
		return sizeElem;
	}
	/**
	 * @return
	 */
	public int getNumParams() {
		return numParams;
	}
	
	/**
	 * @return
	 */
	public RemoteMethodInfo getRemoteMethodInfo() {
		return remoteMethodInfo;
	}

	/**
	 * @param resultData
	 */
	protected void setResultData(byte[][] resultData) throws GrpcException {
		int resultIndex = 0;
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			/* get information for argument */
			RemoteMethodArg remoteMethodArg = 
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			
			/* transform java value data into Ninf value data */
			if (remoteMethodArg.getType() == NgParamTypes.NG_TYPE_STRING &&
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_OUT ||
				remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)) {
				/* transform string and set byte array */
				data[i] = resultData[resultIndex];
				/* set size */
				size[i] = data[i].length;
				/* set count */
				if (remoteMethodArg.getNDims() > 0) {
					RemoteMethodArgSubScript remoteMethodArgSubScript =
						remoteMethodArg.getRemoteMethodArgSubscript(0);

					NgExpression expSize = remoteMethodArgSubScript.getSize();	
								
					/* set end to size */
					count[i] = (int) expSize.calc(intArgs);
				} else {
					count[i] = 1;
				}
				/* increment index of Result data */
				resultIndex++;
			} else if (remoteMethodArg.getType() != NgParamTypes.NG_TYPE_FILENAME &&
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_OUT ||
				remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)) {
				/* set size */
				size[i] = NgParamTypes.getSize(remoteMethodArg.getType());
				/* transform args and set byte array */
				data[i] = resultData[resultIndex];
					
				/* set count */
				if (data[i] != null) {
					count[i] = data[i].length / size[i];
				}
				/* increment index of Result data */
				resultIndex++;
			}
		}
	}
	
	/**
	 * @param index
	 * @param dataArray
	 */
	public void setData(int index, byte[] dataArray) {
		data[index] = dataArray;
	}
	
	/**
	 * @return
	 */
	protected List getCallbackFuncList() {
		return callbackFunc;
	}
	
	/**
	 * @return
	 */
	protected List getCallbackFuncInfoList() {
		return callbackFuncInfo;
	}
	
	/**
	 * @return
	 */
	public List getArgs() {
		return args;
	}

	/**
	 * @throws GrpcException
	 */
	private void transformCBArg() throws GrpcException {
		/* check all of MODE_{IN,}OUT parameters */
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			RemoteMethodArg remoteMethodArg =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			if ((remoteMethodArg.getMode() == NgParamTypes.NG_MODE_IN) ||
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)) {
				/* set size */
				size[i] = NgParamTypes.getSize(remoteMethodArg.getType());
				/* put argElement into bi */
				ByteArrayInputStream bi = new ByteArrayInputStream(data[i]);
				Object target = args.get(i);
				transformResult(remoteMethodArg.getNDims(),
					bi,	target,	remoteMethodArg);
			}
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void transformCBResult() throws GrpcException{
		/* transform arguments into byteArray */
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			/* get information for argument */
			RemoteMethodArg remoteMethodArg = 
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
				
			/* transform java value data into Ninf value data */
			if (((remoteMethodArg.getType() == NgParamTypes.NG_TYPE_STRING) &&
				((remoteMethodArg.getMode() == NgParamTypes.NG_MODE_OUT) ||
				(remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT)))) {
				ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.argBufferSize);
				String targetString[] = (String[]) args.get(i);
				NgParamTypes.transformArg(remoteMethodArg.getType(),
					targetString.length, args.get(i), bo);
				try {
					byte[] dummyBuffer = new byte[ENCODE_HEADER_LEN];
					bo.write(dummyBuffer);
					bo.close();
				} catch (IOException e) {
					throw new NgIOException(e);
				}
				data[i] = bo.toByteArray();
				size[i] = data[i].length;
				if (remoteMethodArg.getNDims() > 0) {
					RemoteMethodArgSubScript remoteMethodArgSubScript =
						remoteMethodArg.getRemoteMethodArgSubscript(0);

					NgExpression expSize = remoteMethodArgSubScript.getSize();	
								
					/* set end to size */
					count[i] = (int) expSize.calc(intArgs);
				} else {
					count[i] = 1;
				}
			} else if (remoteMethodArg.getMode() == NgParamTypes.NG_MODE_OUT ||
				remoteMethodArg.getMode() == NgParamTypes.NG_MODE_INOUT) {
				/* set size */
				size[i] = NgParamTypes.getSize(remoteMethodArg.getType());
				/* transform args and set byte array */
				byte[] buffer =	transformArg(remoteMethodArg.getNDims(),
					args.get(i),
					remoteMethodArg, true);
					
				/* add created buffer into list */
				data[i] = buffer;
				/* set count */
				count[i] = buffer.length / size[i];
			} else {
				/* set null */
				data[i] = null;
			}
		}
	}
	
	/**
	 * @param nDims
	 * @param rma
	 * @return
	 */
	private Object initArray(int nDims, RemoteMethodArg rma) throws GrpcException {
		/* get sizes */
		int size[] = new int[nDims];
		for (int i = 0; i < nDims; i++) {
			/* get Subscript */
			RemoteMethodArgSubScript rmas = rma.getRemoteMethodArgSubscript(nDims - 1);
			/* get size of array */
			NgExpression expSize = rmas.getSize();
			size[i] = (int) expSize.calc(intArgs);
		}
		
		/* create array and return it */
		switch (rma.getType()) {
			/* char */
			case NgParamTypes.NG_TYPE_CHAR:
			return createCharArray(size);
			/* short */
			case NgParamTypes.NG_TYPE_SHORT:
			return createShortArray(size);
			/* int */
			case NgParamTypes.NG_TYPE_INT:
			return createIntArray(size);
			/* long */
			case NgParamTypes.NG_TYPE_LONG:
			return createLongArray(size);
			/* float */
			case NgParamTypes.NG_TYPE_FLOAT:
			return createFloatArray(size);
			/* double */
			case NgParamTypes.NG_TYPE_DOUBLE:
			return createDoubleArray(size);
			/* scomplex */
			case NgParamTypes.NG_TYPE_SCOMPLEX:
			return createSComplexArray(size);
			/* dcomplex */
			case NgParamTypes.NG_TYPE_DCOMPLEX:
			return createDComplexArray(size);
			/* string */
			case NgParamTypes.NG_TYPE_STRING:
			return createStringArray(size);
		}
		
		/* nothing will be returned */
		return null;
	}
	
	/**
	 * @param size
	 * @return
	 */
	private Object createCharArray(int[] size) {
		switch (size.length) {
			case 1:
			return new byte[size[0]];
			case 2:
			return new byte[size[0]][size[1]];
			case 3:
			return new byte[size[0]][size[1]][size[2]];
			case 4:
			return new byte[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new byte[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new byte[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new byte[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new byte[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new byte[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new byte[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}

	/**
	 * @param size
	 * @return
	 */
	private Object createShortArray(int[] size) {
		switch (size.length) {
			case 1:
			return new short[size[0]];
			case 2:
			return new short[size[0]][size[1]];
			case 3:
			return new short[size[0]][size[1]][size[2]];
			case 4:
			return new short[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new short[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new short[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new short[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new short[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new short[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new short[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}

	/**
	 * @param size
	 * @return
	 */
	private Object createIntArray(int[] size) {
		switch (size.length) {
			case 1:
			return new int[size[0]];
			case 2:
			return new int[size[0]][size[1]];
			case 3:
			return new int[size[0]][size[1]][size[2]];
			case 4:
			return new int[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new int[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new int[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new int[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new int[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new int[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new int[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}

	/**
	 * @param size
	 * @return
	 */
	private Object createLongArray(int[] size) {
		switch (size.length) {
			case 1:
			return new long[size[0]];
			case 2:
			return new long[size[0]][size[1]];
			case 3:
			return new long[size[0]][size[1]][size[2]];
			case 4:
			return new long[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new long[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new long[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new long[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new long[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new long[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new long[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}

	/**
	 * @param size
	 * @return
	 */
	private Object createFloatArray(int[] size) {
		switch (size.length) {
			case 1:
			return new float[size[0]];
			case 2:
			return new float[size[0]][size[1]];
			case 3:
			return new float[size[0]][size[1]][size[2]];
			case 4:
			return new float[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new float[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new float[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new float[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new float[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new float[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new float[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}

	/**
	 * @param size
	 */
	private Object createDoubleArray(int[] size) {
		switch (size.length) {
			case 1:
			return new double[size[0]];
			case 2:
			return new double[size[0]][size[1]];
			case 3:
			return new double[size[0]][size[1]][size[2]];
			case 4:
			return new double[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new double[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new double[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new double[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new double[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new double[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new double[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}

	/**
	 * @param size
	 * @return
	 */
	private Object createSComplexArray(int[] size) {
		switch (size.length) {
			case 1:
			return new Scomplex[size[0]];
			case 2:
			return new Scomplex[size[0]][size[1]];
			case 3:
			return new Scomplex[size[0]][size[1]][size[2]];
			case 4:
			return new Scomplex[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new Scomplex[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new Scomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new Scomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new Scomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new Scomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new Scomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}

	/**
	 * @param size
	 * @return
	 */
	private Object createDComplexArray(int[] size) {
		switch (size.length) {
			case 1:
			return new Dcomplex[size[0]];
			case 2:
			return new Dcomplex[size[0]][size[1]];
			case 3:
			return new Dcomplex[size[0]][size[1]][size[2]];
			case 4:
			return new Dcomplex[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new Dcomplex[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new Dcomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new Dcomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new Dcomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new Dcomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new Dcomplex[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}
	
	/**
	 * @param size
	 */
	private Object createStringArray(int[] size) {
		switch (size.length) {
			case 0:
			return new String[1];
			case 1:
			return new String[size[0]];
			case 2:
			return new String[size[0]][size[1]];
			case 3:
			return new String[size[0]][size[1]][size[2]];
			case 4:
			return new String[size[0]][size[1]][size[2]][size[3]];
			case 5:
			return new String[size[0]][size[1]][size[2]][size[3]][size[4]];
			case 6:
			return new String[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]];
			case 7:
			return new String[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]];
			case 8:
			return new String[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]];
			case 9:
			return new String[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]];
			case 10:
			return new String[size[0]][size[1]][size[2]][size[3]][size[4]][size[5]][size[6]][size[7]][size[8]][size[9]];
			default:
			return null;
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	public void debugTransformCBResult() throws GrpcException {
		transformCBResult();
	}
}
