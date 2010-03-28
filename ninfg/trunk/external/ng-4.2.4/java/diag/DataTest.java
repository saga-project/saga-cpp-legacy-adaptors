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
 * $RCS_file$ $Revision: 1.18 $ $Date: 2005/10/25 10:48:23 $
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
public class DataTest {
	private static final String targetModule = "data/";

	private static final int NELEMENTS = 3;
	private static final int SKIP = 3;
	private static final int TIMES = 10;
	private static boolean verbose = false;
	
	/* instance variables */
	private GrpcClient client;
	
	/* Initialize Client */
	public DataTest(String configName) throws GrpcException {
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
	private boolean charTest() {
		final String targetEntry = "char_test";
		byte[] a = new byte [NELEMENTS]; 
		byte[] b = new byte [NELEMENTS]; 

		/* init data */
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = (byte)i;
			b[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Byte((byte)NELEMENTS), a, b);
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
			if (a[i] != b[i]) {
				if (verbose) {
					System.out.println (
						"char_test: " +
						"a[" + i + "] : " + a[i] + " != " +
					 	"b[" + i + "] : " + b[i]);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* short test */
	private boolean shortTest() {
		final String targetEntry = "short_test";
		short[] a = new short [NELEMENTS]; 
		short[] b = new short [NELEMENTS]; 

		/* init data */
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = (short)i;
			b[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Short((short)NELEMENTS), a, b);
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
			if (a[i] != b[i]) {
				if (verbose) {
					System.out.println (
						"short_test: " +
						"a[" + i + "] : " + a[i] + " != " +
						"b[" + i + "] : " + b[i]);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* int test */
	private boolean intTest() {
		final String targetEntry = "int_test";
		int[] a = new int [NELEMENTS]; 
		int[] b = new int [NELEMENTS]; 

		/* init data */
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = i;
			b[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer((int)NELEMENTS), a, b);
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
			if (a[i] != b[i]) {
				if (verbose) {
					System.out.println (
						"int_test: " +
						"a[" + i + "] : " + a[i] + " != " +
						"b[" + i + "] : " + b[i]);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* long test */
	private boolean longTest() {
		final String targetEntry = "long_test";
		long[] a = new long [NELEMENTS]; 
		long[] b = new long [NELEMENTS]; 

		/* init data */
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = i;
			b[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Long(NELEMENTS), a, b);
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
			if (a[i] != b[i]) {
				if (verbose) {
					System.out.println (
						"long_test: " +
						"a[" + i + "] : " + a[i] + " != " +
						"b[" + i + "] : " + b[i]);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* float test */
	private boolean floatTest() {
		final String targetEntry = "float_test";
		float scalarIn, scalarOut[];
		float[] a = new float [NELEMENTS]; 
		float[] b = new float [NELEMENTS]; 

		/* init data */
		scalarIn = 1.0f;
		scalarOut = new float[1];
		scalarOut[0] = 0.0f;
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = i;
			b[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer((int)NELEMENTS),
				new Float(scalarIn), a, scalarOut, b);
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
		if (scalarIn != scalarOut[0]) {
			if (verbose) {
				System.out.println(
					"float_test: scalarIn(" + scalarIn + ") scalarOut(" +
					scalarOut + ")");
			}
			return false;
		}
		for (int i = 0; i < NELEMENTS; i++) {
			if (a[i] != b[i]) {
				if (verbose) {
					System.out.println (
						"float_test: " +
						"a[" + i + "] : " + a[i] + " != " +
						"b[" + i + "] : " + b[i]);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* double test */
	private boolean doubleTest() {
		final String targetEntry = "double_test";
		double scalarIn, scalarOut[];
		double[] a = new double [NELEMENTS]; 
		double[] b = new double [NELEMENTS]; 

		/* init data */
		scalarIn = 1.0;
		scalarOut = new double[1];
		scalarOut[0] = 0.0;
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = i;
			b[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer((int)NELEMENTS),
				new Double(scalarIn), a, scalarOut, b);
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
		if (scalarIn != scalarOut[0]) {
			if (verbose) {
				System.out.println(
					"double_test: scalarIn(" + scalarIn + ") scalarOut(" +
					scalarOut + ")");
			}
			return false;
		}
		for (int i = 0; i < NELEMENTS; i++) {
			if (a[i] != b[i]) {
				if (verbose) {
					System.out.println (
						"double_test: " +
						"a[" + i + "] : " + a[i] + " != " +
						"b[" + i + "] : " + b[i]);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* scomplex test */
	private boolean scomplexTest() {
		final String targetEntry = "scomplex_test";
		Scomplex scalarIn = new Scomplex();
		Scomplex[] scalarOut = new Scomplex[1];
		Scomplex[] a = new Scomplex [NELEMENTS]; 
		Scomplex[] b = new Scomplex [NELEMENTS]; 

		/* init data */
		scalarIn.r = 1.0f;
		scalarIn.i = 2.0f;
		scalarOut[0] = new Scomplex();
		scalarOut[0].r = 0.0f;
		scalarOut[0].i = 0.0f;
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = new Scomplex();
			b[i] = new Scomplex();
			a[i].r = (float)i;
			a[i].i = (float)(i+0.1);
			b[i].r = 0.0f;
			b[i].i = 0.0f;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer((int)NELEMENTS),
				scalarIn, a, scalarOut, b);
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
		if ((scalarOut[0].r != scalarIn.r) || (scalarOut[0].i != scalarIn.i)) {
			if (verbose) {
				System.out.println(
					"scomplex_test: " + 
					"scalarIn(" + scalarIn.r + ", " + scalarIn.i + ") "  +
					"scalarOut(" + scalarOut[0].r + ", " +
					scalarOut[0].i + ") ");
			}
			return false;
		}
		for (int i = 0; i < NELEMENTS; i++) {
			if (a[i].r != b[i].r || a[i].i != b[i].i) {
				if (verbose) {
					System.out.println (
						"scomplex_test: " +
						"a[" + i + "] : " + a[i].r + "," + a[i].i + " != " +
						"b[" + i + "] : " + b[i].r + "," + b[i].i);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* dcomplex test */
	private boolean dcomplexTest() {
		final String targetEntry = "dcomplex_test";
		Dcomplex scalarIn = new Dcomplex();
		Dcomplex[] scalarOut = new Dcomplex[1];
		Dcomplex[] a = new Dcomplex [NELEMENTS]; 
		Dcomplex[] b = new Dcomplex [NELEMENTS]; 

		/* init data */
		scalarIn.r = 1.0;
		scalarIn.i = 2.0;
		scalarOut[0] = new Dcomplex();
		scalarOut[0].r = 0.0;
		scalarOut[0].i = 0.0;
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = new Dcomplex();
			b[i] = new Dcomplex();
			a[i].r = (double)i;
			a[i].i = (double)(i+0.1);
			b[i].r = 0.0f;
			b[i].i = 0.0f;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer((int)NELEMENTS),
				scalarIn, a, scalarOut, b);
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
		if ((scalarOut[0].r != scalarIn.r) || (scalarOut[0].i != scalarIn.i)) {
			if (verbose) {
				System.out.println(
					"dcomplex_test: " + 
					"scalarIn(" + scalarIn.r + ", " + scalarIn.i + ") "  +
					"scalarOut(" + scalarOut[0].r + ", " +
					scalarOut[0].i + ") ");
			}
			return false;
		}
		for (int i = 0; i < NELEMENTS; i++) {
			if (a[i].r != b[i].r || a[i].i != b[i].i) {
				if (verbose) {
					System.out.println (
						"dcomplex_test: " +
						"a[" + i + "] : " + a[i].r + "," + a[i].i + " != " +
						"b[" + i + "] : " + b[i].r + "," + b[i].i);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* work test */
	private boolean workTest() {
		final String targetEntry = "work_test";
		int[] a = new int [NELEMENTS]; 
		int[] b = new int [NELEMENTS]; 

		/* init data */
		for (int i = 0; i < NELEMENTS; i++) {
			a[i] = i;
			b[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer((int)NELEMENTS), a, null, b);
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
			if (a[i] != b[i]) {
				if (verbose) {
					System.out.println (
						"work_test: " +
						"a[" + i + "] : " + a[i] + " != " +
						"b[" + i + "] : " + b[i]);
				}
				/* failed */
				return false;
			}
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean stringTest() {
		final String targetEntry = "string_test";

		String in = new String("This is String Test.");
		String out[] = new String[1];

		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(in, out);
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
		if (!in.equals(out[0])) {
			return false;
		}
		
		return true;
	}
	
	/* ======================================================= */
	private boolean stringArrayTest() {
		final String targetEntry = "string_array_test";

		String in[] = new String[5];
		in[0] = new String("This is a test for array of string.");
		in[1] = new String("Hello, World");
		in[2] = new String("Sun Mon Tue Wed Thu Fri Sat");
		in[3] = new String("ls -- list directory contents");
		in[4] = new String("All tests were successful.");
		String answer[] = new String[3];
		answer[0] = new String("Were all tests successful?");
		answer[1] = new String("Sun Mon Tue Wed Thu Fri Sat");
		answer[2] = new String("Good morning");
		String out[] = new String[3];

		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(in, out);
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
		for (int i = 0; i < 3; i++) {
			if (!out[i].equals(answer[i])) {
				System.out.println (i + ": " + out[i] + " != " + answer[i]);
				return false;
			}
		}
		
		return true;
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
		
		System.out.println("=====    DataTest start    =====");
		DataTest dataTest = null;
		try {
			dataTest = new DataTest(params[0]);
			
			/* char test */
			System.out.print ("Char Test : ");
			if (dataTest.charTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* short test */
			System.out.print ("Short Test : ");
			if (dataTest.shortTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* int test */
			System.out.print ("Int Test : ");
			if (dataTest.intTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* long test */
			System.out.print ("Long Test : ");
			if (dataTest.longTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* float test */
			System.out.print ("Float Test : ");
			if (dataTest.floatTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* double test */
			System.out.print ("Double Test : ");
			if (dataTest.doubleTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
	
			/* scomplex test */
			System.out.print ("Scomplex Test : ");
			if (dataTest.scomplexTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* dcomplex test */
			System.out.print ("Dcomplex Test : ");
			if (dataTest.dcomplexTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* work test */
			System.out.print ("Work Test : ");
			if (dataTest.workTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* string test */
			System.out.print ("String Test : ");
			if (dataTest.stringTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* string array test */
			System.out.print ("String Array Test : ");
			if (dataTest.stringArrayTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			try {
				if (dataTest != null) {
					/* deactivate client */
					dataTest.deactivateClient();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println("===== DataTest was finished =====");
	}
}
