/*
 * $RCSfile: CallbackTest.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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

import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.NgCallbackInterface;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;
import org.gridforum.gridrpc.GrpcObjectHandle;

/**
 * Diagnostic program for Ninf-G
 * This program checks callback type parameter.
 * 
 * A client program which invokes Ninf-G callbacks.
 */
public class CallbackTest {
	private static final String targetModule = "callback/";
	private static boolean verbose = false;
	
	public static int maxCBArg = 32;

	public static double sum = 0.0;
	public static final int NELEMS = 5;
	public static double doubleArray2D[][] = new double[NELEMS][NELEMS]; 
	public static String string = null;
	
	// instance variables 
	private GrpcClient client;
	
	// variables for callback_max test 
	public static int callback_max_working;
	public static int callback_max_cmp_value;
	
	// Initialize Client 
	public CallbackTest(String configName) throws GrpcException {
		// create GrpcClient 
		this.client =
			GrpcClientFactory.getClient("org.apgrid.grpc.ng.NgGrpcClient");
		// activate GrpcClient 
		client.activate(configName);
	}

	// Finalize Client 
	private void deactivateClient() {
		try {
			client.deactivate();
		} catch (GrpcException e) {
			e.printStackTrace();
		}
	}

	private void disposeHandle(GrpcFunctionHandle handle) {
		if (handle == null) { return; }
		try {
			handle.dispose();
		} catch (GrpcException e) {
			e.printStackTrace();
		}
	}

	private void disposeHandle(GrpcObjectHandle handle) {
		if (handle == null) { return; }
		try {
			handle.dispose();
		} catch (GrpcException e) {
			e.printStackTrace();
		}
	}



///// Tests begin

	private boolean callbackSimpleTest() {
		final String targetEntry = "callback_test";

		NgCallbackInterface callback_func =	new CallbackSimple();
		double initial = 100.0;
		int times = 1;
		sum = 0.0;
		GrpcFunctionHandle handle = null;

		try {
			// create GrpcHandle 
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			// call char_test function 
			handle.call(new Double(initial), new Integer(times), callback_func);

		} catch (GrpcException e) {
			e.printStackTrace();
			return false;
		} finally {
			disposeHandle(handle);
		}

		// check result 
		return checkResultForSimpleTest(initial, times);
	}



	private boolean callback2DTest() {
		final String targetEntry = "callback2D_test";

		NgCallbackInterface callback2D_func = new Callback2D();
		double argDoubleArray2D[][] = new double[NELEMS][NELEMS];
		for (int i = 0; i < NELEMS; i++) {
			for (int j = 0; j < NELEMS; j++) {
				argDoubleArray2D[i][j] = i * NELEMS + j;
				doubleArray2D[i][j] = 0.0;
			}
		}

		GrpcFunctionHandle handle = null;
		try {
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			// call char_test function 
			handle.call(new Integer(NELEMS), argDoubleArray2D, callback2D_func);

		} catch (GrpcException e) {
			e.printStackTrace();
			return false;
		} finally {
			disposeHandle(handle);
		}
		
		// check result 
		return checkResultFor2DTest(argDoubleArray2D, doubleArray2D);
	}

	private boolean callbackReturnTest() {
		final String targetEntry = "callback_return_test";

		NgCallbackInterface callbackReturn_func =	new CallbackReturn();
		double argDoubleArrayIn[][] = new double[NELEMS][NELEMS];
		double argDoubleArrayOut[][] = new double[NELEMS][NELEMS];
		for (int i = 0; i < NELEMS; i++) {
			for (int j = 0; j < NELEMS; j++) {
				argDoubleArrayIn[i][j] = i * NELEMS + j;
				argDoubleArrayOut[i][j] = 0.0;
			}
		}

		GrpcFunctionHandle handle = null;
		try {
			// create GrpcHandle 
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			// call char_test function 
			handle.call(new Integer(NELEMS),
				argDoubleArrayIn, argDoubleArrayOut, callbackReturn_func);

		} catch (GrpcException e) {
			e.printStackTrace();
			return false;
		} finally {
			disposeHandle(handle);
		}
		
		// check result 
		return checkResultForReturnTest(argDoubleArrayIn, argDoubleArrayOut);
	}

	private boolean callbackMultiTest() {
		final String targetEntry = "callback_multi_test";

		NgCallbackInterface callbackReturn_func =	new CallbackReturn();
		double argDoubleArrayIn[][] = new double[NELEMS][NELEMS];
		double argDoubleArrayOut[][] = new double[NELEMS][NELEMS];
		for (int i = 0; i < NELEMS; i++) {
			for (int j = 0; j < NELEMS; j++) {
				argDoubleArrayIn[i][j] = i * NELEMS + j;
				argDoubleArrayOut[i][j] = 0.0;
			}
		}

		GrpcFunctionHandle handle = null;
		try {
			// create GrpcHandle 
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			// call char_test function 
			handle.call(new Integer(NELEMS), argDoubleArrayIn,
				argDoubleArrayOut, callbackReturn_func, callbackReturn_func);

		} catch (GrpcException e) {
			e.printStackTrace();
			return false;
		} finally {
			disposeHandle(handle);
		}
		
		// check result 
		return checkResultForMultiTest(argDoubleArrayIn, argDoubleArrayOut);
	}

	private boolean callbackStringTest() {
		final String targetEntry = "callbackstr";

		NgCallbackInterface callbackString_func = new CallbackString();
		String argString[] = new String[5];
		StringBuffer answer = new StringBuffer();
		for (int i = 0; i < argString.length; i++) {
			argString[i] = new String("caller" + i);
			answer.append(argString[i]);
		}

		GrpcFunctionHandle handle = null;
		try {
			// create GrpcHandle 
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			// call char_test function 
			handle.call(argString, callbackString_func);
		} catch (GrpcException e) {
			e.printStackTrace();
			return false;
		} finally {
			disposeHandle(handle);
		}
		
		// check result 
		return checkResultForStringTest(answer.toString());
	}

	private boolean callbackMaxTest() {
		final String targetEntry = "callback_max_test";

		boolean retResult = true;
		NgCallbackInterface callback_max_start = new CallbackMaxStart();
		NgCallbackInterface callback_max_end = new CallbackMaxEnd();
		NgCallbackInterface callback_max_get = new CallbackMaxGet();
		NgCallbackInterface callback_max_put = new CallbackMaxPut();
		
		GrpcObjectHandle handle = null;
		try {
			// create GrpcHandle 
			handle = this.client.getObjectHandle(targetModule + targetEntry);

			// invoke initializer 
			handle.invoke("initialize");


			//--- Compute (1 + 2 + ... + 32) * 2 
			int answer = 0;
			for (int i = 1; i <= CallbackTest.maxCBArg; i++) {
				answer += i;
			}
			answer *= 2;
			
			// invoke calculator 
			int in = 1;
			int out[] = new int[1];
			handle.invoke("max_method2x", new Integer(in), out,
				callback_max_start, callback_max_get,
				callback_max_put, callback_max_end);
			
			// check result 
			int finalComp = CallbackTest.callback_max_cmp_value;
			if (out[0] != 1) {
				retResult = false;
			}
			if (finalComp != answer) {
				retResult = false;
			}
			printAnswerForMaxTest(out[0], answer, finalComp);
			
			out[0] = 0; // reset 
			handle.invoke("get_result", out); // get result 
			// check result 
			if (out[0] != 1) {
				retResult = false;
			}
			printResultForMaxTest(out[0], retResult);


			//--- Compute (1 + 2 + ... + 32) * 4 
			answer = 0;
			for (int i = 1; i <= CallbackTest.maxCBArg; i++) {
				answer += i;
			}
			answer *= 4;
			
			// invoke calculator 
			in = 1;
			out[0] = 0;
			handle.invoke("max_method4x", new Integer(in), out,
				callback_max_start, callback_max_get,
				callback_max_put, callback_max_end);
			
			// check result
			finalComp = CallbackTest.callback_max_cmp_value;
			if (out[0] != 1) {
				retResult = false;
			}
			if (finalComp != answer) {
				retResult = false;
			}
			printAnswerForMaxTest(out[0], answer, finalComp);
			
			out[0] = 0; // reset 
			handle.invoke("get_result", out); // get result 
			
			// check result 
			if (out[0] != 1) {
				retResult = false;
			}
			printResultForMaxTest(out[0], retResult);


			// invoke finalizer 
			handle.invoke("finalize");

			// check result 
			if (out[0] != 1) {
				retResult = false;
			}
		} catch (GrpcException e) {
			e.printStackTrace();
			retResult = false;
		} finally {
			disposeHandle(handle);
		}
		
		return retResult;
	}

///// Check Result methods
	private boolean checkResultFor2DTest(
	 double[][] arrayIn, double[][] arrayOut) {
		for (int i = 0; i < NELEMS; i++) {
			for (int j = 0; j < NELEMS; j++) {
				if (arrayIn[i][j] != arrayOut[i][j]) {
					printResultFor2DTest(i, j, arrayIn[i][j], arrayOut[i][j]);
					return false;
				}
			}
		}
		return true;
	}

	private boolean checkResultForSimpleTest(double initial, int times) {
		double lsum = 0.0;
		for (int i = 0; i < times; i++) {
			lsum += initial * (i + 1);
		}
		if (sum != lsum) {
			if (verbose) {
				System.err.println("sum and lsum is mismatched.");
				System.err.println("sum : " + sum + ", lsum : " + lsum + ".");
			}
			return false;
		}
		return true;
	}

	private boolean checkResultForStringTest(String answer) {
		if (answer.equals(CallbackTest.string)) {
			return true;
		} else {
			if (verbose) {
				System.err.println("arg and result are mismatched.");
				System.err.println("arg : " + answer + ", result : " + CallbackTest.string + ".");
			}
			return false;
		}
	}

	private boolean checkResultForReturnTest(
	 double[][] arrayIn, double[][] arrayOut) {
		return checkResultFor2DTest(arrayIn, arrayOut);
	}

	private boolean checkResultForMultiTest(
	 double[][] arrayIn, double[][] arrayOut) {
		return checkResultFor2DTest(arrayIn, arrayOut);
	}


	private void printResultFor2DTest(int i, int j,
	 double arrayIn, double arrayOut) {
		printResultForMultiTest(i, j, arrayIn, arrayOut);
	}

	private void printResultForReturnTest(int i, int j,
	 double arrayIn, double arrayOut) {
		printResultForMultiTest(i, j, arrayIn, arrayOut);
	}

	private void printResultForMultiTest(int i, int j,
	 double arrayIn, double arrayOut) {
		if ( ! verbose ) { return ; }
		StringBuilder sb = new StringBuilder("mismatched element was found.");
		sb.append("arg[").append(i).append("][").append(j).append("] : ");
		sb.append(arrayIn).append(".");

		sb.append("result[").append(i).append("][").append(j).append("] : ");
		sb.append(arrayOut).append(".");

		System.err.println(sb.toString());
	}

	private void printAnswerForMaxTest(int out, int answer, int finalComp) {
		if ( ! verbose) { return ; }
		StringBuilder sb = new StringBuilder("\n");
		sb.append("out = " + out);
		sb.append(" , answer = " + answer);
		sb.append(", comp = " + finalComp);
		System.err.println(sb.toString());
	}

	private void printResultForMaxTest(int out, boolean retResult) {
		if ( ! verbose ) { return ;}
		StringBuilder sb = new StringBuilder("\n");
		sb.append("result = " + out);
		sb.append(" , ret = " + retResult);
		System.err.println(sb.toString());
	}

///// Tests end


	// ===== main routines ===== 
	// check argument for this program (accept "-verbose" and name of config) 
	static String[] parseArgs(String arg[]){
		Vector tmpV = new Vector();
		int index = 0;
		for (int i = 0; i < arg.length; i++){
			if (arg[i].equalsIgnoreCase("-verbose")) {
				verbose = true;
			} else {
				tmpV.addElement(arg[i]);
			}
		}
		String tmp[] = new String[tmpV.size()];
		for (int i = 0; i < tmpV.size(); i++) {
			tmp[i] = (String)(tmpV.elementAt(i));
		}
		return tmp;
	}

	static void printResult(boolean result) {
		if ( result ) {
			System.out.println("OK!");
		} else {
			System.out.println("FAILED!");
		}
	}

	///// main routine for DataTest 
	public static void main (String[] args) {
		// parse arguments 
		String[] params = parseArgs(args);
		CallbackTest cancelTest = null;
		
		System.out.println("=====    CallbackTest start    =====");
		try {
			cancelTest = new CallbackTest(params[0]);

			///// Callback Simple test 
			System.out.print("Callback Test       : ");
			printResult( cancelTest.callbackSimpleTest() );

			///// Callback 2D test 
			System.out.print("Callback2D Test     : ");
			printResult( cancelTest.callback2DTest() ); 

			///// Callback Return test 
			System.out.print("CallbackReturn Test : ");
			printResult( cancelTest.callbackReturnTest() );

			///// Callback Multi test 
			System.out.print("CallbackMulti Test  : ");
			printResult( cancelTest.callbackMultiTest() );

			///// Callback String test 
			System.out.print("CallbackString Test : ");
			printResult( cancelTest.callbackStringTest() );

			///// Callback Max test 
			System.out.print("CallbackMax    Test : ");
			printResult( cancelTest.callbackMaxTest() );

		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			if (cancelTest != null) {
				cancelTest.deactivateClient();
			}
		}
		System.out.println("===== CallbackTest was finished =====");
	}
}


///// Definition of CallBacks begin

// class for callback-1 
class CallbackSimple implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		CallbackTest.sum += ((double[])args.get(0))[0];
	}
}

// class for multi dimension array 
class Callback2D implements NgCallbackInterface {
	public void callback(List args) {
		double doubleArray2D[][] = (double[][]) args.get(0);
		for (int i = 0; i < CallbackTest.NELEMS; i++) {
			for (int j = 0; j < CallbackTest.NELEMS; j++) {
				CallbackTest.doubleArray2D[i][j] = doubleArray2D[i][j];
			}
		}
	}
}

// class for return variable 
class CallbackReturn implements NgCallbackInterface {
	public void callback(List args) {
		double doubleArrayIn[][] = (double[][]) args.get(0);
		double doubleArrayOut[][] = (double[][]) args.get(1);
		for (int i = 0; i < doubleArrayIn.length; i++) {
			for (int j = 0; j < doubleArrayIn.length; j++) {
				doubleArrayOut[i][j] = doubleArrayIn[i][j];
			}
		}
	}
}

// class for string variable 
class CallbackString implements NgCallbackInterface {
	public void callback(List args) {
		String argString[] = (String []) args.get(0);
		CallbackTest.string = new String(argString[0]);
	}
}

// class for setting start the test 
class CallbackMaxStart implements NgCallbackInterface {
	public void callback(List args) {
		CallbackTest.callback_max_working = 1;
		CallbackTest.callback_max_cmp_value = 0;
	}
}

// class for setting end the test 
class CallbackMaxEnd implements NgCallbackInterface {
	public void callback(List args) {
		CallbackTest.callback_max_working = 0;
	}
}

// class for callback with max arguments 
class CallbackMaxGet implements NgCallbackInterface {
	public void callback(List args) {
		int cbArg[][] = new int[CallbackTest.maxCBArg][];
		
		// check if the initializer was called 
		if (CallbackTest.callback_max_working == 0) {
			return;
		}
		
		for (int i = 0; i < CallbackTest.maxCBArg; i++) {
			cbArg[i] = (int[])args.get(i);
			
			// check if it's null 
			if (cbArg[i] == null) {
				System.err.println ("Invalid callback argument.");
				return;
			}
			
			// set variable 
			cbArg[i][0] = i + 1;
		}
	}
}

// class for callback with max arguments 
class CallbackMaxPut implements NgCallbackInterface {
	public void callback(List args) {
		int cbArg[][] = new int[CallbackTest.maxCBArg][];

		// check if the initializer was called 
		if (CallbackTest.callback_max_working == 0) {
			return;
		}

		for (int i = 0; i < CallbackTest.maxCBArg; i++) {
			cbArg[i] = (int[])args.get(i);

			// check if it's null 
			if (cbArg[i] == null) {
				System.err.println ("Invalid callback argument.");
				return;
			}
			
			// append variable 
			CallbackTest.callback_max_cmp_value += cbArg[i][0];
		}
	}
}
///// Definition of CallBacks end

