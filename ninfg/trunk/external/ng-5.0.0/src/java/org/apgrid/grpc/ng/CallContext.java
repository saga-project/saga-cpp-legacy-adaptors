/*
 * $RCSfile: CallContext.java,v $ $Revision: 1.10 $ $Date: 2008/02/07 10:26:15 $
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
import java.io.IOException;
import java.util.List;
import java.util.Vector;
import java.lang.reflect.Array;

import org.apgrid.grpc.ng.info.*;
import org.gridforum.gridrpc.GrpcException;

public class CallContext {
	private int numParams;
	private int[] size;
	private int[] count;   // num of data each arguments (num of elem?)
	private int[] intArgs; // scalar argument values
	private List args;     // list of argument data (in Java data types)
	private byte[][] data;
	private RemoteMethodInfo remoteMethodInfo;

	private static final int ENCODE_HEADER_LEN = 8;
	
	// for callback 
	private List callbackFunc = new Vector();
	private List callbackFuncInfo = new Vector(); // <RemoteMethodInfo?>
	
	/**
	 * @param remoteMethodInfo
	 * @param args
	 */
	public CallContext(RemoteMethodInfo remoteMethodInfo, List args)
	 throws GrpcException {
		this.numParams = remoteMethodInfo.getNumParams();
		this.remoteMethodInfo = remoteMethodInfo;
		this.data      = new byte[numParams][];
		int callbackIndex = 0;
		this.size      = new int[numParams];
		this.count     = new int[numParams];
		this.args      = args;
		this.intArgs   = setupIntArguments(); // Depend numParams

		// transform arguments into byteArray 
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			// get information for argument 
			RemoteMethodArg rma = 
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
				
			// transform java value data into Ninf value data 
			if ( isTypeFilename(rma.getType()) ) {
				transformJavaIntoNinfFilename(i, rma);
			} else if (isModeInput(rma.getMode())) {
				transformJavaIntoNinfScalar(i, rma);
			} else if ( isTypeCallback(rma.getType()) ) {
				transformJavaIntoNinfCallback(i, rma,
					remoteMethodInfo.getShrink());
			} else {
				data[i] = null;
			}
		}
	}

	/* 
	 * transform Java value data Into Ninf value data(Filename) 
	 */
	private void transformJavaIntoNinfFilename(int index, RemoteMethodArg aRma)
	 throws GrpcException {
		// set size 
		this.size[index] = NgParamTypes.getSize(aRma.getType());
		// check if it's null 
		if (this.args.get(index) == null) {
			this.count[index] = 0;
			this.data[index]  = new byte[ENCODE_HEADER_LEN];
			return ;
		}
		// transform args and set byte array 
		// add created buffer into list 
		this.data[index] =
		 transformArg(aRma.getNDims(), this.args.get(index), aRma, true); 
		// set count 
		this.count[index] = _getDataCount(aRma);

		return ;
	}

	//  Scalar 
	private void transformJavaIntoNinfScalar(int index, RemoteMethodArg aRma)
	 throws GrpcException {
		// set size
		this.size[index] = NgParamTypes.getSize(aRma.getType());
		// check if it's null
		if ( ! isTypeString(aRma.getType()) && this.args.get(index) == null) {
			this.count[index] = 0;
			this.data[index] = new byte[ENCODE_HEADER_LEN];
			return ;
		}
		// transform args and set byte array 
		// add created buffer into list
		this.data[index] =
		 transformArg(aRma.getNDims(), this.args.get(index), aRma, true);

		// set count
		if (isTypeString(aRma.getType())) {
			this.count[index] = _getDataCount(aRma);
		} else {
			this.count[index] =
			 (this.data[index].length - ENCODE_HEADER_LEN) / this.size[index];
		}
		return ;
	}

	private void transformJavaIntoNinfCallback(int index, RemoteMethodArg aRma,
	 boolean aShrink)
	 throws GrpcException {
		// check if it's null 
		if (this.args.get(index) == null) {
			throw new NgExecRemoteMethodException("callback argument is null.");
		}
		// Add callback object into list 
		this.callbackFunc.add(this.args.get(index));

		// put RemoteMethodInfo for the callback
		RemoteMethodInfo callbackInfo = aRma.getCallbackInfo();
		callbackInfo.setShrink(aShrink);
		this.callbackFuncInfo.add(callbackInfo);
		return ;
	}
	

	/**
	 * @param remoteMethodInfo
	 * @param argsData
	 * @throws GrpcException
	 */
	public CallContext(RemoteMethodInfo remoteMethodInfo,
	 byte[][] argsData, int[] intArguments)
	 throws GrpcException {
		// convert Argument for callback and put them into List 
		this.numParams = remoteMethodInfo.getNumParams();
		this.size      = new int[numParams];
		this.count     = new int[numParams];
		this.args      = new Vector();
		this.intArgs   = newScalarArgs(intArguments);
		this.data      = new byte[numParams][0];
		this.remoteMethodInfo = remoteMethodInfo;
		
		// Initialize byte array and argument list 
		int dataIndex = 0;
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg rma =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			if ( isModeInput(rma.getMode())) {
				this.data[i] = argsData[dataIndex++];
			}

			// Initialize list for arguments 
			Object target = initArray(rma.getNDims(), rma);
			if (target == null) { target = "dummy"; }

			// put it(?) into argument list 
			args.add(target);
		}
		
		// transform received data 
		transformCBArg();

		// convert String 
		//convertString(remoteMethodInfo, numParams, args);
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg rma =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			if (isTypeString(rma.getType()) && isModeInput(rma.getMode()) ) {
				args.set(i, (String [])args.get(i));
			}
		}
	}

	private int[] newScalarArgs(int[] arguments) {
		int [] ret = new int[arguments.length];
		System.arraycopy(arguments, 0, ret, 0, ret.length);
		return ret;
	}

	private void convertString(RemoteMethodInfo aRmi, int nParams, List anArgs) {
		for (int i = 0; i < nParams; i++) {
			RemoteMethodArg rma = (RemoteMethodArg) aRmi.getArgs().get(i);
			if (isTypeString(rma.getType()) && isModeInput(rma.getMode()) ) {
				anArgs.set(i, (String [])anArgs.get(i));
			}
		}
	}
	
	private int[] setupIntArguments() {
		if (remoteMethodInfo == null) {
			throw new NullPointerException("remoteMethodInfo is null");
		}
		int numParams = remoteMethodInfo.getNumParams();
		int[] intArguments = new int[numParams];
		
		// check int values in arguments 
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg rma =
				(RemoteMethodArg)remoteMethodInfo.getArgs().get(i);
			if ( ! isModeInput( rma.getMode()) ) {
				continue; // nothing will be done 
			}

			if (rma.getNDims() == 0) {
				if ((rma.getType() == NgParamTypes.NG_TYPE_CHAR)) {
					Byte tmpByte = (Byte) args.get(i);
					intArguments[i] = tmpByte.intValue();
				} else if ((rma.getType() == NgParamTypes.NG_TYPE_SHORT)) {
					Short tmpShort = (Short) args.get(i);
					intArguments[i] = tmpShort.intValue();
				} else if ((rma.getType() == NgParamTypes.NG_TYPE_INT)) {
					Integer tmpInt = (Integer) args.get(i);
					intArguments[i] = tmpInt.intValue();
				} else if ((rma.getType() == NgParamTypes.NG_TYPE_LONG)) {
					Long tmpLong = (Long) args.get(i);
					intArguments[i] = tmpLong.intValue();
				}
			}
		} 
		return intArguments;
	}
	
	public int[] getIntArguments() {
		// defensive copy
		int [] ret = new int[intArgs.length];
		System.arraycopy(intArgs, 0, ret, 0, ret.length);
		return ret;
	}

	/**
	 * @param nDims
	 * @param target
	 * @param remoteMethodArg
	 * @return
	 */
	private byte[] transformArg(int nDims, Object target,
	 RemoteMethodArg anRma, boolean firstFlag)
	 throws GrpcException {

		ByteArrayOutputStream bo =
			new ByteArrayOutputStream(NgGlobals.argBufferSize);
		
		// get size, start, end, skip 
		int start = 0;
		int end = 1;
		int skip = 1;
		if (nDims >= 1) {
			RemoteMethodArgSubScript rmass =
				anRma.getRemoteMethodArgSubscript(nDims - 1);

			NgExpression expSize = rmass.getSize();			
			// set end to size 
			end = (int) expSize.calc(intArgs);
		}
		
		if (nDims > 1) { 
			// process of multi dimensional array
			// check if it's string 
			if ( isTypeString(anRma.getType()) ) {
				throw new NgArgTypeException("Invalid String type array.");
			}
			
			// get byte[] from target 
			Object[] targetArray = (Object[]) target;
			nDims--;
			
			for (int i = start; i < end; i += skip) {
				// call this method again 
				byte[] buffer =
					transformArg(nDims, targetArray[i], anRma, false);
				bo.write(buffer, 0, buffer.length);
			}
		} else if ( nDims == 1 ){
			// transform and write data of Ninf Param 
			NgParamTypes.transformArg(anRma.getType(), end, target, bo);
		} else {
			// get byte[] from target &
			// transform and write data of Ninf Param 
			NgParamTypes.transformScalarArg(anRma.getType(),
			 (Object)target, bo);
		}
		
		// return created buffer 
		byte[] returnBuffer;
		try {
			if (firstFlag) {
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

	public int getCount(int index) {
		return count[index];
	}
	
	// return the method argument data
	public byte[] getData(int index) {
		return data[index];
	}
	
	public int getSize(int index) {
		return size[index];
	}
	
	/**
	 * @param args args of NgFunction/ObjectGrpcHandle#callWith(prop, args)
	 * @return
	 */
	protected void transformResult(List args) throws GrpcException {
		// check all of MODE_{IN,}OUT parameters 
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			RemoteMethodArg rma =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);

			if ( isModeOutput(rma.getMode())) {
				if (data[i] == null) { continue; }
				// put argElement into data[i]
				ByteArrayInputStream bi = new ByteArrayInputStream(data[i]);
				Object target = args.get(i);
				if ((bi.available() < 1 ) || (target == null)) {
					continue;
				}
				_transformResult(rma.getNDims(), bi, target, rma);
			}
		}
	}

	/**
	 * @param nDims
	 * @param bi
	 * @param remoteMethodArg
	 */
	private void _transformResult(int nDims, ByteArrayInputStream bi,
	 Object arg, RemoteMethodArg anRma)
	 throws GrpcException {

		// get size, start, end, skip 
		RemoteMethodArgSubScript rmass =
			anRma.getRemoteMethodArgSubscript(nDims - 1);

		int size  = 0;
		int start = 0;
		int end   = 1;
		int skip  = 1;
		int elemSize = 0;

		if (rmass != null) {
			int argSize;
			NgExpression expSize  = rmass.getSize();
			NgExpression expStart = rmass.getStart();
			NgExpression expEnd   = rmass.getEnd();
			NgExpression expSkip  = rmass.getSkip();
			
			// set size, start, end, skip 
			size  = (int) expSize.calc(intArgs);
			start = (int) expStart.calc(intArgs);
			end   = (int) expEnd.calc(intArgs);
			skip  = (int) expSkip.calc(intArgs);
			
			// end was omitted 
			if (end == 0) { end = (int) expSize.calc(intArgs); }
			// skip was omitted
			if (skip == 0) { skip = 1; }

			// get size of 1 elem 
			elemSize = getElemSize(nDims - 1, anRma);
		}
		
		// get Object[] from byte[]
		if (nDims > 1) {
			Object[] array = (Object[])arg;
			nDims--;
			// set elements 
			int nextTarget = start;
			for (int i = 0; i < size; i++) {
				if (i == nextTarget) {
					_transformResult(nDims, bi, array[i], anRma);
					nextTarget = i + skip;
					if (nextTarget > end) {
						nextTarget = -1;
					}
				} else {
					// drop data
					byte[] buffer = new byte[elemSize];
					bi.read(buffer, 0, elemSize);
				}
			}
		} else {
			// set elements 
			NgParamTypes.transformResult(
				anRma.getType(), bi, arg, size, start, end, skip);
		}
	}
	
	/**
	 * @param nDims
	 * @param remoteMethodArg
	 * @throws GrpcException
	 */
	private int getElemSize(int nDims, RemoteMethodArg anRma)
	 throws GrpcException {
		int sizeElem = NgParamTypes.getSize(anRma.getType());
		for (int i = 0; i < nDims; i++) {
			RemoteMethodArgSubScript rmass =
				anRma.getRemoteMethodArgSubscript(nDims);
			NgExpression expSize = rmass.getSize();

			sizeElem *= expSize.calc(intArgs);
		}
		return sizeElem;
	}

	public int getNumParams() {
		return numParams;
	}
	
	public RemoteMethodInfo getRemoteMethodInfo() {
		return remoteMethodInfo;
	}

	/**
	 * @param resultData
	 */
	protected void setResultData(byte[][] resultData) throws GrpcException {
		int resultIndex = 0;
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			// get information for argument 
			RemoteMethodArg rma = 
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
			
			// transform java value data into Ninf value data 
			if (isTypeString(rma.getType()) && isModeOutput(rma.getMode())) {

				// transform string and set byte array 
				data[i] = resultData[resultIndex]; 
				size[i] = data[i].length;      // set size 
				count[i] = _getDataCount(rma); // set count 
				resultIndex++; // increment index of Result data 
			} else if ( ! isTypeFilename(rma.getType()) &&
					      isModeOutput(rma.getMode())) {

				// set size 
				size[i] = NgParamTypes.getSize(rma.getType());
				// transform args and set byte array 
				data[i] = resultData[resultIndex];
					
				// set count
				if (data[i] != null) {
					count[i] = data[i].length / size[i];
				} 
				resultIndex++;// increment index of Result data 
			}
		}
	}

	private int _getDataCount(RemoteMethodArg rma) throws GrpcException {
		if (rma == null) { throw new NullPointerException(); }

		if (rma.getNDims() == 0 ) { return 1; } // one dimension

		// multi-dimension
		RemoteMethodArgSubScript rmass =
        	rma.getRemoteMethodArgSubscript(0);
        NgExpression expSize = rmass.getSize();

        return (int) expSize.calc(this.intArgs);  // throw NgArgTypeException
	}
	
	/**
	 * @param index
	 * @param dataArray
	 */
	public void setData(int index, byte[] dataArray) {
		data[index] = dataArray;
	}
	
	protected List getCallbackFuncList() {
		return callbackFunc;
	}
	
	protected List getCallbackFuncInfoList() {
		return callbackFuncInfo;
	}
	
	public List getArgs() {
		return args;
	}

	private boolean isModeOutput(int mode) {
		return ((mode == NgParamTypes.NG_MODE_OUT) 
				|| (mode == NgParamTypes.NG_MODE_INOUT));
	} 
	private boolean isModeInput(int mode) {
		return ((mode == NgParamTypes.NG_MODE_IN)
				|| (mode == NgParamTypes.NG_MODE_INOUT));
	} 

	private boolean isTypeString(int type) {
		return (type == NgParamTypes.NG_TYPE_STRING);
	} 
	private boolean isTypeFilename(int type) {
		return (type == NgParamTypes.NG_TYPE_FILENAME);
	}
	private boolean isTypeCallback(int type) {
		return (type == NgParamTypes.NG_TYPE_CALLBACK);
	}


	private void transformCBArg() throws GrpcException {
		// check all of MODE_{IN,}OUT parameters
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			RemoteMethodArg rma =
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);

			if ( isModeInput(rma.getMode())) {
				// set size 
				size[i] = NgParamTypes.getSize(rma.getType());
				// put argElement into data[i]
				ByteArrayInputStream bi = new ByteArrayInputStream(data[i]);
				Object target = args.get(i);
				_transformResult(rma.getNDims(), bi, target, rma);
			}
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void transformCBResult() throws GrpcException{
		// transform arguments into byteArray 
		for (int i = 0; i < remoteMethodInfo.getNumParams(); i++) {
			// get information for argument 
			RemoteMethodArg rma = 
				(RemoteMethodArg) remoteMethodInfo.getArgs().get(i);
				
			// transform java value data into Ninf value data 
			if ( isTypeString(rma.getType()) && isModeOutput(rma.getMode())) {

				ByteArrayOutputStream bo =
				 new ByteArrayOutputStream(NgGlobals.argBufferSize);

				String targetString[] = (String[]) args.get(i);
				NgParamTypes.transformArg(rma.getType(),
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
				count[i] = _getDataCount(rma);
			} else if ( isModeOutput(rma.getMode())) { 
				// set size 
				size[i] = NgParamTypes.getSize( rma.getType() );
				// transform args and set byte array 
				byte[] buffer =
					transformArg(rma.getNDims(), args.get(i), rma, true);

				// add created buffer into list 
				data[i] = buffer;
				// set count 
				count[i] = buffer.length / size[i];
			} else {
				data[i] = null;
			}
		}
	}
	
	/**
	 * @param nDims
	 * @param rma
	 * @return
	 */
	private Object initArray(int nDims, RemoteMethodArg rma)
	 throws GrpcException {
		// get sizes 
		int size[] = new int[nDims];
		for (int i = 0; i < nDims; i++) {
			// get Subscript 
			RemoteMethodArgSubScript rmas =
			 rma.getRemoteMethodArgSubscript(nDims - 1);
			// get size of array 
			NgExpression expSize = rmas.getSize();
			size[i] = (int) expSize.calc(intArgs);
		}

		// create array and return it 
		switch (rma.getType()) {
			case NgParamTypes.NG_TYPE_CHAR:
				return Array.newInstance(char.class, size);
			case NgParamTypes.NG_TYPE_SHORT:
				return Array.newInstance(short.class, size);
			case NgParamTypes.NG_TYPE_INT:
				return Array.newInstance(int.class, size);
			case NgParamTypes.NG_TYPE_LONG:
				return Array.newInstance(long.class, size);
			case NgParamTypes.NG_TYPE_FLOAT:
				return Array.newInstance(float.class, size);
			case NgParamTypes.NG_TYPE_DOUBLE:
				return Array.newInstance(double.class, size);
			case NgParamTypes.NG_TYPE_SCOMPLEX:
				return Array.newInstance(Scomplex.class, size);
			case NgParamTypes.NG_TYPE_DCOMPLEX:
				return Array.newInstance(Dcomplex.class, size);
			case NgParamTypes.NG_TYPE_STRING:
				return Array.newInstance(String.class, size);
		}
		// nothing will be returned 
		return null;
	}

	/**
	 * @throws GrpcException
	 */
	public void debugTransformCBResult() throws GrpcException {
		transformCBResult();
	}
}
