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
 * $RCS_file$ $Revision: 1.5 $ $Date: 2005/06/30 10:42:47 $
 */
//package diag;

import java.util.Vector;

import org.apgrid.grpc.ng.Dcomplex;
import org.apgrid.grpc.ng.Scomplex;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;

/**
 * Diagnostic program for Ninf-G
 * This program checks data types of Ninf-G parameters.
 */
public class ZeroElementTest {
	private static final String targetModule = "zero_element/";

	private static final int NELEMENTS = 5;
	private static boolean verbose = true;
	
	/* instance variables */
	private GrpcClient client;
	
	/* Initialize Client */
	public ZeroElementTest(String configName) throws GrpcException {
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

	/* ===== call TEST PROGRAMS ===== */
	/* char test */
	private boolean zeroElementArrayTest() {
		final String targetEntry = "zero_element_array";
		int[] rpcResult = new int[1];
		boolean success = true;
		
		byte[] a_char = new byte [NELEMENTS]; 
		byte[] b_char = new byte [NELEMENTS]; 
		short[] a_short = new short [NELEMENTS]; 
		short[] b_short = new short [NELEMENTS]; 
		int[] a_int = new int [NELEMENTS]; 
		int[] b_int = new int [NELEMENTS]; 
		long[] a_long = new long [NELEMENTS]; 
		long[] b_long = new long [NELEMENTS]; 
		float[] a_float = new float [NELEMENTS]; 
		float[] b_float = new float [NELEMENTS]; 
		double[] a_double = new double [NELEMENTS]; 
		double[] b_double = new double [NELEMENTS]; 
		Scomplex[] a_scomplex = new Scomplex [NELEMENTS]; 
		Scomplex[] b_scomplex = new Scomplex [NELEMENTS]; 
		Dcomplex[] a_dcomplex = new Dcomplex [NELEMENTS]; 
		Dcomplex[] b_dcomplex = new Dcomplex [NELEMENTS]; 

		/* init data */
		for (int i = 0; i < NELEMENTS; i++) {
			a_char[i] = (byte)i;
			b_char[i] = (byte)(NELEMENTS - i);
			a_short[i] = (short)i;
			b_short[i] = (short)(NELEMENTS - i);
			a_int[i] = i;
			b_int[i] = NELEMENTS - i;
			a_long[i] = i;
			b_long[i] = NELEMENTS - i;
			a_float[i] = i;
			b_float[i] = NELEMENTS - i;
			a_double[i] = i;
			b_double[i] = NELEMENTS - i;
			
			a_scomplex[i] = new Scomplex();
			b_scomplex[i] = new Scomplex();
			a_scomplex[i].r = i;
			a_scomplex[i].i = i;
			b_scomplex[i].r = NELEMENTS - i;
			b_scomplex[i].i = NELEMENTS - i;

			a_dcomplex[i] = new Dcomplex();
			b_dcomplex[i] = new Dcomplex();
			a_dcomplex[i].r = i;
			a_dcomplex[i].i = i;
			b_dcomplex[i].r = NELEMENTS - i;
			b_dcomplex[i].i = NELEMENTS - i;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(0), rpcResult,
				a_char, b_char, a_short, b_short, a_int, b_int, a_long, b_long, 
				a_float, b_float, a_double, b_double, a_scomplex, b_scomplex,
				a_dcomplex, b_dcomplex);

			/* check the result */
			if (rpcResult[0] == 0) {
				if (verbose) {
					System.out.println("error was occurred at Remote Executable.");
				}
				success = false;
			}
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
		for (int i = 0; i < NELEMENTS; i++) {
			if ((a_char[i] != i)  || (b_char[i] != NELEMENTS -i) ||
				(a_short[i] != i)  || (b_short[i] != NELEMENTS -i) ||
				(a_int[i] != i)  || (b_int[i] != NELEMENTS -i) ||
				(a_long[i] != i)  || (b_long[i] != NELEMENTS -i) ||
				(a_float[i] != i)  || (b_float[i] != NELEMENTS -i) ||
				(a_double[i] != i)  || (b_double[i] != NELEMENTS -i) ||
				(a_scomplex[i].r != i)  || (b_scomplex[i].r != NELEMENTS -i) ||
				(a_scomplex[i].i != i)  || (b_scomplex[i].i != NELEMENTS -i) ||
				(a_dcomplex[i].r != i)  || (b_dcomplex[i].r != NELEMENTS -i) ||
				(a_dcomplex[i].i != i)  || (b_dcomplex[i].i != NELEMENTS -i)) {
				
				if (verbose) {
					System.out.println (
						"zero_element_array_test: " + "[" + i + "] failed.");
				}
				/* failed */
				success = false;
			}
		}
		
		/* succeed */
		return success;
	}

	/* ======================================================= */
	/* zero length string test */
	private boolean ZeroLengthStringTest() {
		final String targetEntry = "zero_length_string";
		int[] rpcResult = new int[1];
		String[] b_string = new String[1];
		boolean success = true;

		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(rpcResult, "", b_string);

			/* check the result */
			if (rpcResult[0] == 0) {
				if (verbose) {
					System.out.println("error was occurred at Remote Executable.");
				}
				success = false;
			}
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
		if ((b_string[0] == null) || (! b_string[0].equals(""))) {
			if (verbose) {
				System.out.println("OUT String is not equal \"\"");
			}
			success = false;
		}
		
		/* succeed */
		return success;
	}

	/* ======================================================= */
	/* zero element string array test */
	private boolean ZeroElementStringArray() {
		final String targetEntry = "zero_element_string_array";
		String[] a_string = new String[NELEMENTS];
		String[] b_string = new String[NELEMENTS];
		boolean success = true;
		int[] rpcResult = new int[1];
		
		/* init data */
		for (int i = 0; i < NELEMENTS; i++) {
			a_string[i] = new String("" + i);
			b_string[i] = new String("" + (NELEMENTS - i));
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(0), rpcResult, a_string, b_string);

			/* check the result */
			if (rpcResult[0] == 0) {
				if (verbose) {
					System.out.println("error was occurred at Remote Executable.");
				}
				success = false;
			}
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
		for (int i = 0; i < NELEMENTS; i++) {
			if ((! a_string[i].equals("" + i)) || (! b_string[i].equals("" + (NELEMENTS - i)))) {
				if (verbose) {
					System.out.println (
						"zero_element_string_array_test: " + "[" + i + "] failed.");
				}
				/* failed */
				success = false;
			}
		}
		
		/* succeed */
		return success;
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
	public static void main (String[] args) throws InterruptedException {
		/* parse arguments */
		String[] params = parseMyArg(args);
		
		System.out.println("=====    ZeroElementTest start    =====");
		ZeroElementTest zeroElementTest = null;
		try {
			zeroElementTest = new ZeroElementTest(params[0]);

			/* zero_element test */
			System.out.print ("ZeroElement Test : ");
			if (zeroElementTest.zeroElementArrayTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* short test */
			System.out.print ("ZeroLength String Test : ");
			if (zeroElementTest.ZeroLengthStringTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* int test */
			System.out.print ("ZeroElement String Array Test : ");
			if (zeroElementTest.ZeroElementStringArray() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			try {
				if (zeroElementTest != null) {
					/* deactivate client */
					zeroElementTest.deactivateClient();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println("===== ZeroElementTest was finished =====");
	}
}
