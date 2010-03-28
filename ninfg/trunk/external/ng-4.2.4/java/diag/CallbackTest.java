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
 * $RCS_file$ $Revision: 1.11 $ $Date: 2005/10/25 10:48:23 $
 */
//package diag;

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
 */
public class CallbackTest {
	private static final String targetModule = "callback/";
	
	public static int maxCBArg = 32;

	public static double sum = 0.0;
	private static boolean verbose = false;
	public static final int NELEMS = 5;
	public static double doubleArray2D[][] = new double[NELEMS][NELEMS]; 
	public static String string = null;
	
	/* instance variables */
	private GrpcClient client;
	
	/* variables for callback_max test */
	public static int callback_max_working;
	public static int callback_max_cmp_value;
	
	/* Initialize Client */
	public CallbackTest(String configName) throws GrpcException {
		/* create GrpcClient */
		this.client = GrpcClientFactory.getClient(
			"org.apgrid.grpc.ng.NgGrpcClient");
		/* activate GrpcClient */
		client.activate(configName);
	}
	
	/* Finalize Client */
	private void deactivateClient() throws GrpcException {
		/* deactivate client */
		client.deactivate();
	}

	/* ======================================================= */
	private boolean callbackSimpleTest() {
		final String targetEntry = "callback_test";

		/* Init params */
		NgCallbackInterface callback_func =	new CallbackSimple();
		double initial = 100.0;
		int times = 1;
		sum = 0.0;
		GrpcFunctionHandle handle = null;

		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Double(initial), new Integer(times), callback_func);
		} catch (GrpcException e) {
			e.printStackTrace();
			
			/* failed */
			return false;
		} finally {
			/* dispose handle */
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
		
		/* check result */
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

	/* ======================================================= */
	private boolean callback2DTest() {
		final String targetEntry = "callback2D_test";

		/* Init params */
		NgCallbackInterface callback2D_func =	new Callback2D();
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

			/* call char_test function */
			handle.call(new Integer(NELEMS), argDoubleArray2D, callback2D_func);
		} catch (GrpcException e) {
			e.printStackTrace();

			/* failed */
			return false;
		} finally {
			/* dispose handle */
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
		
		/* check result */
		for (int i = 0; i < NELEMS; i++) {
			for (int j = 0; j < NELEMS; j++) {
				if (argDoubleArray2D[i][j] != doubleArray2D[i][j]) {
					if (verbose) {
						System.err.println("mismatched element was found.");
						System.err.println("arg[" + i + "][" + j + "] : " + argDoubleArray2D[i][j] + ".");
						System.err.println("result[" + i + "][" + j + "] : " + doubleArray2D[i][j] + ".");
					}
					return false;
				}
			}
		}
		
		return true;
	}

	/* ======================================================= */
	private boolean callbackReturnTest() {
		final String targetEntry = "callback_return_test";

		/* Init params */
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
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(NELEMS),
				argDoubleArrayIn, argDoubleArrayOut, callbackReturn_func);
		} catch (GrpcException e) {
			e.printStackTrace();

			/* failed */
			return false;
		} finally {
			/* dispose handle */
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
		
		/* check result */
		for (int i = 0; i < NELEMS; i++) {
			for (int j = 0; j < NELEMS; j++) {
				if (argDoubleArrayIn[i][j] != argDoubleArrayOut[i][j]) {
					if (verbose) {
						System.err.println("mismatched element was found.");
						System.err.println("arg[" + i + "][" + j + "] : " + argDoubleArrayIn[i][j] + ".");
						System.err.println("result[" + i + "][" + j + "] : " + argDoubleArrayOut[i][j] + ".");
					}
					return false;
				}
			}
		}
		
		return true;
	}

	/* ======================================================= */
	private boolean callbackMultiTest() {
		final String targetEntry = "callback_multi_test";

		/* Init params */
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
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(NELEMS), argDoubleArrayIn,
				argDoubleArrayOut, callbackReturn_func, callbackReturn_func);
		} catch (GrpcException e) {
			e.printStackTrace();

			/* failed */
			return false;
		} finally {
			/* dispose handle */
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
		
		/* check result */
		for (int i = 0; i < NELEMS; i++) {
			for (int j = 0; j < NELEMS; j++) {
				if (argDoubleArrayIn[i][j] != argDoubleArrayOut[i][j]) {
					if (verbose) {
						System.err.println("mismatched element was found.");
						System.err.println("arg[" + i + "][" + j + "] : " + argDoubleArrayIn[i][j] + ".");
						System.err.println("result[" + i + "][" + j + "] : " + argDoubleArrayOut[i][j] + ".");
					}
					return false;
				}
			}
		}
		
		return true;
	}

	/* ======================================================= */
	private boolean callbackStringTest() {
		final String targetEntry = "callbackstr";

		/* Init params */
		NgCallbackInterface callbackString_func =	new CallbackString();
		String argString[] = new String[5];
		StringBuffer answer = new StringBuffer();
		for (int i = 0; i < argString.length; i++) {
			argString[i] = new String("caller" + i);
			answer.append(argString[i]);
		}

		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(argString, callbackString_func);
		} catch (GrpcException e) {
			e.printStackTrace();

			/* failed */
			return false;
		} finally {
			/* dispose handle */
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
		
		/* check result */
		if (answer.toString().equals(CallbackTest.string)) {
			return true;
		} else {
			if (verbose) {
				System.err.println("arg and result are mismatched.");
				System.err.println("arg : " + answer + ", result : " + CallbackTest.string + ".");
			}
			return false;
		}
	}

	/* ======================================================= */
	private boolean callbackMaxTest() {
		final String targetEntry = "callback_max_test";

		/* Init params */
		boolean retResult = true;
		NgCallbackInterface callback_max_start = new CallbackMaxStart();
		NgCallbackInterface callback_max_end = new CallbackMaxEnd();
		NgCallbackInterface callback_max_get = new CallbackMaxGet();
		NgCallbackInterface callback_max_put = new CallbackMaxPut();
		
		GrpcObjectHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getObjectHandle(targetModule + targetEntry);

			/* invoke initializer */
			handle.invoke("initialize");
			
			/* Compute (1 + 2 + ... + 32) * 2 */
			int answer = 0;
			for (int i = 1; i <= CallbackTest.maxCBArg; i++) {
				answer += i;
			}
			answer *= 2;
			
			/* invoke calculator */
			int in = 1;
			int out[] = new int[1];
			handle.invoke("max_method2x", new Integer(in), out,
				callback_max_start, callback_max_get,
				callback_max_put, callback_max_end);
			
			/* check result */
			int finalComp = CallbackTest.callback_max_cmp_value;
			if (out[0] != 1) {
				retResult = false;
			}
			if (finalComp != answer) {
				retResult = false;
			}
			
			if (verbose) {
				System.err.println ("");
				System.err.println ("out = " + out[0] +
					" , answer = " + answer + ", comp = " + finalComp);
			}
			
			/* reset */
			out[0] = 0;
			
			/* get result */
			handle.invoke("get_result", out);
			
			/* check result */
			if (out[0] != 1) {
				retResult = false;
			}
			
			if (verbose) {
				System.err.println ("");
				System.err.println ("result = " + out[0] + " , ret = " + retResult);
			}
			
			/* Compute (1 + 2 + ... + 32) * 4 */
			answer = 0;
			for (int i = 1; i <= CallbackTest.maxCBArg; i++) {
				answer += i;
			}
			answer *= 4;
			
			/* invoke calculator */
			in = 1;
			out[0] = 0;
			handle.invoke("max_method4x", new Integer(in), out,
				callback_max_start, callback_max_get,
				callback_max_put, callback_max_end);
			
			/* check result */
			finalComp = CallbackTest.callback_max_cmp_value;
			if (out[0] != 1) {
				retResult = false;
			}
			if (finalComp != answer) {
				retResult = false;
			}
			
			if (verbose) {
				System.err.println ("");
				System.err.println ("out = " + out[0] +
					" , answer = " + answer + ", comp = " + finalComp);
			}
			
			/* reset */
			out[0] = 0;
			
			/* get result */
			handle.invoke("get_result", out);
			
			/* check result */
			if (out[0] != 1) {
				retResult = false;
			}
			
			if (verbose) {
				System.err.println ("");
				System.err.println ("result = " + out[0] + " , ret = " + retResult);
			}
			
			/* invoke finalizer */
			handle.invoke("finalize");

			/* check result */
			if (out[0] != 1) {
				retResult = false;
			}
		} catch (GrpcException e) {
			e.printStackTrace();

			/* failed */
			retResult = false;
		} finally {
			/* dispose handle */
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
		
		return retResult;
	}

	/* ===== main routines ===== */
	/* check argument for this program (accept "-verbose" and name of config) */
	static String[] parseMyArg(String arg[]){
		Vector tmpV = new Vector();
		int index = 0;
		for (int i = 0; i < arg.length; i++){
			if (arg[i].equalsIgnoreCase("-verbose"))
			verbose = true;
			else 
			tmpV.addElement(arg[i]);
		}
		String tmp[] = new String[tmpV.size()];
		for (int i = 0; i < tmpV.size(); i++) {
			tmp[i] = (String)(tmpV.elementAt(i));
		}
		return tmp;
	}

	/* main routine for DataTest */
	public static void main (String[] args) {
		/* parse arguments */
		String[] params = parseMyArg(args);
		CallbackTest cancelTest = null;
		
		System.out.println("=====    CallbackTest start    =====");
		try {
			cancelTest = new CallbackTest(params[0]);

			/* callback Simple test */
			System.out.print ("Callback Test       : ");
			if (cancelTest.callbackSimpleTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* callback 2D test */
			System.out.print ("Callback2D Test     : ");
			if (cancelTest.callback2DTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* callback Return test */
			System.out.print ("CallbackReturn Test : ");
			if (cancelTest.callbackReturnTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* callback Multi test */
			System.out.print ("CallbackMulti Test  : ");
			if (cancelTest.callbackMultiTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* callback String test */
			System.out.print ("CallbackString Test : ");
			if (cancelTest.callbackStringTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* callback Max test */
			System.out.print ("CallbackMax    Test : ");
			if (cancelTest.callbackMaxTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			try {
				if (cancelTest != null) {
					/* deactivate client */
					cancelTest.deactivateClient();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println("===== CallbackTest was finished =====");
	}
}

/* class for callback-1 */
class CallbackSimple implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		CallbackTest.sum += ((double[])args.get(0))[0];
	}
}

/* class for multi dimension array */
class Callback2D implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		double doubleArray2D[][] = (double[][]) args.get(0);
		for (int i = 0; i < CallbackTest.NELEMS; i++) {
			for (int j = 0; j < CallbackTest.NELEMS; j++) {
				CallbackTest.doubleArray2D[i][j] = doubleArray2D[i][j];
			}
		}
	}
}

/* class for return variable */
class CallbackReturn implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
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

/* class for string variable */
class CallbackString implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		String argString[] = (String []) args.get(0);
		CallbackTest.string = new String(argString[0]);
	}
}

/* class for setting start the test */
class CallbackMaxStart implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		CallbackTest.callback_max_working = 1;
		CallbackTest.callback_max_cmp_value = 0;
	}
}

/* class for setting end the test */
class CallbackMaxEnd implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		CallbackTest.callback_max_working = 0;
	}
}

/* class for callback with max arguments */
class CallbackMaxGet implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		int cbArg[][] = new int[CallbackTest.maxCBArg][];
		
		/* check if the initializer was called */
		if (CallbackTest.callback_max_working == 0) {
			return;
		}
		
		for (int i = 0; i < CallbackTest.maxCBArg; i++) {
			cbArg[i] = (int[])args.get(i);
			
			/* check if it's null */
			if (cbArg[i] == null) {
				System.err.println ("Invalid callback argument.");
				return;
			}
			
			/* set variable */
			cbArg[i][0] = i + 1;
		}
	}
}

/* class for callback with max arguments */
class CallbackMaxPut implements NgCallbackInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
	 */
	public void callback(List args) {
		int cbArg[][] = new int[CallbackTest.maxCBArg][];
		
		/* check if the initializer was called */
		if (CallbackTest.callback_max_working == 0) {
			return;
		}
		
		for (int i = 0; i < CallbackTest.maxCBArg; i++) {
			cbArg[i] = (int[])args.get(i);
			
			/* check if it's null */
			if (cbArg[i] == null) {
				System.err.println ("Invalid callback argument.");
				return;
			}
			
			/* append variable */
			CallbackTest.callback_max_cmp_value += cbArg[i][0];
		}
	}
}
