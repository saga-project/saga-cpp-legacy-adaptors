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
 * $RCS_file$ $Revision: 1.9 $ $Date: 2007/01/22 06:54:21 $
 */
//package diag;

import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.Dcomplex;
import org.apgrid.grpc.ng.NgCallbackInterface;
import org.apgrid.grpc.ng.Scomplex;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;

/**
 * Diagnostic program for Ninf-G
 * This program checks data types of Ninf-G parameters with shrink.
 */
public class SkipTest {
	private static final String targetModule = "skip_test/";

	private static final int SIZE = 30;
	private static final int SKIP = 3;
	private static boolean verbose = false;
	
	/* instance variables */
	private GrpcClient client;
	
	/* Initialize Client */
	public SkipTest(String configName) throws GrpcException {
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
	/* ======================================================= */
	/* skip char test */
	private boolean skipCharTest() {
		final String targetEntry = "skip_char_test";
		byte[] in = new byte [SIZE]; 
		byte[] out = new byte [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = (byte)i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (out[i] != (in[i] * 2)) {
					return false;
				}
			} else {
				if (out[i] != 0) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip short test */
	private boolean skipShortTest() {
		final String targetEntry = "skip_short_test";
		short[] in = new short [SIZE]; 
		short[] out = new short [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = (byte)i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (out[i] != (in[i] * 2)) {
					return false;
				}
			} else {
				if (out[i] != 0) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip int test */
	private boolean skipIntTest() {
		final String targetEntry = "skip_int_test";
		int[] in = new int [SIZE]; 
		int[] out = new int [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (out[i] != (in[i] * 2)) {
					return false;
				}
			} else {
				if (out[i] != 0) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip long test */
	private boolean skipLongTest() {
		final String targetEntry = "skip_long_test";
		long[] in = new long [SIZE]; 
		long[] out = new long [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (out[i] != (in[i] * 2)) {
					return false;
				}
			} else {
				if (out[i] != 0) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip float test */
	private boolean skipFloatTest() {
		final String targetEntry = "skip_float_test";
		float[] in = new float [SIZE]; 
		float[] out = new float [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (out[i] != (in[i] * 2)) {
					return false;
				}
			} else {
				if (out[i] != 0) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip double test */
	private boolean skipDoubleTest() {
		final String targetEntry = "skip_double_test";
		double[] in = new double [SIZE]; 
		double[] out = new double [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (out[i] != (in[i] * 2)) {
					return false;
				}
			} else {
				if (out[i] != 0) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip scomplex test */
	private boolean skipScomplexTest() {
		final String targetEntry = "skip_scomplex_test";
		Scomplex[] in = new Scomplex [SIZE]; 
		Scomplex[] out = new Scomplex [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = new Scomplex();
			out[i] = new Scomplex();
			
			in[i].r = (float)i;
			in[i].i = (float)(i + 0.1);
			out[i].r = out[i].i = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (!compareFloat(out[i].r, (in[i].r * 2)) ||
					!compareFloat(out[i].i, (float) (in[i].i + 0.1))) {
					return false;
				}
			} else {
				if ((out[i].r != 0) || (out[i].r != 0)) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i].r + ", " + out[i].i);
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip dcomplex test */
	private boolean skipDcomplexTest() {
		final String targetEntry = "skip_dcomplex_test";
		Dcomplex[] in = new Dcomplex [SIZE]; 
		Dcomplex[] out = new Dcomplex [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = new Dcomplex();
			out[i] = new Dcomplex();
			
			in[i].r = (double)i;
			in[i].i = (double)(i + 0.1);
			out[i].r = out[i].i = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (!compareDouble(out[i].r, (in[i].r * 2)) ||
					!compareDouble(out[i].i, (double) (in[i].i + 0.1))) {
					return false;
				}
			} else {
				if ((out[i].r != 0) || (out[i].r != 0)) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i].r + ", " + out[i].i);
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* skip int test */
	private boolean skipInOutTest() {
		final String targetEntry = "skip_inout_test";
		int[] in = new int [SIZE]; 
		int[] inout = new int [SIZE]; 
		int[] out = new int [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			inout[i] = i * 2;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE),	new Integer(SKIP), in, inout, out);
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
		/* INOUT */
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (inout[i] != (in[i] * 4)) {
					return false;
				}
			} else {
				if (inout[i] != in[i] * 2) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (inout[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		/* OUT */
		for (int i = 0; i < SIZE; i++) {
			if ((i % SKIP) == 0) {
				if (out[i] != (in[i] * 8)) {
					return false;
				}
			} else {
				if (out[i] != 0) {
					return false;
				}
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean skipOneTest() {
		final String targetEntry = "skip_int_test";
		int[] in = new int [SIZE]; 
		int[] out = new int [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE), new Integer(1), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if (out[i] != (in[i] * 2)) {
				return false;
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean skip2DTest() {
		final String targetEntry = "skip_2D_test";
		int[][] in = new int [SIZE][SIZE]; 
		int[][] out = new int [SIZE][SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				in[i][j] = i + j;
				out[i][j] = 0;
			}
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE), new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				if (((i % SKIP) == 0) && ((j % SKIP) == 0)) {
					if (out[i][j] != (in[i][j] * 2)) {
						return false;
					}
				} else {
					if (out[i][j] != 0) {
						return false;
					}
				}

				if (verbose) {
					System.out.print (out[i][j] + " ");
				}
			}
			if (verbose) {
				System.out.println ("");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean skip3DTest() {
		final String targetEntry = "skip_3D_test";
		int[][][] in = new int [SIZE][SIZE][SIZE]; 
		int[][][] out = new int [SIZE][SIZE][SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				for (int k = 0; k < SIZE; k++) {
					in[i][j][k] = i + j;
					out[i][j][k] = 0;
				}
			}
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE), new Integer(SKIP), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				for (int k = 0; k < SIZE; k++) {
					if (((i % SKIP) == 0) && ((j % SKIP) == 0) && ((k % SKIP) == 0)) {
						if (out[i][j][k] != (in[i][j][k] * 2)) {
							return false;
						}
					} else {
						if (out[i][j][k] != 0) {
							return false;
						}
					}
					if (verbose) {
						System.out.print (out[i][j][k] + " ");
					}
				}
			}
			if (verbose) {
				System.out.println ("");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean skipOmitSkipTest() {
		final String targetEntry = "skip_omit_skip_test";
		int[] in = new int [SIZE]; 
		int[] out = new int [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if (out[i] != (in[i] * 2)) {
				return false;
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean skipOmitEndTest() {
		final String targetEntry = "skip_omit_end_test";
		int[] in = new int [SIZE]; 
		int[] out = new int [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if (out[i] != (in[i] * 2)) {
				return false;
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean skipOmitStartTest() {
		final String targetEntry = "skip_omit_start_test";
		int[] in = new int [SIZE]; 
		int[] out = new int [SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			in[i] = i;
			out[i] = 0;
		}
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), in, out);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			if (out[i] != (in[i] * 2)) {
				return false;
			}
			
			if (verbose) {
				System.out.print (out[i] + " ");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	private boolean skipCallbackTest() {
		final String targetEntry = "skip_callback_test";
		int[][] in = new int [SIZE][SIZE]; 
		int[][] out = new int [SIZE][SIZE]; 

		/* init data */
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				in[i][j] = i + j;
				out[i][j] = 0;
			}
		}
		NgCallbackInterface skipCallback = new SkipCallback();
		
		GrpcFunctionHandle handle = null;
		try {
			/* create GrpcHandle */
			handle = this.client.getFunctionHandle(targetModule + targetEntry);

			/* call char_test function */
			handle.call(new Integer(SIZE), new Integer(0),
				new Integer(SIZE), new Integer(SKIP), in, out, skipCallback);
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
		if (verbose) {
			System.out.println ("");
		}
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				if (((i % SKIP) == 0) && ((j % SKIP) == 0)) {
					if (out[i][j] != (in[i][j] * 2)) {
						return false;
					}
				} else {
					if (out[i][j] != 0) {
						return false;
					}
				}

				if (verbose) {
					System.out.print (out[i][j] + " ");
				}
			}
			if (verbose) {
				System.out.println ("");
			}
		}
		if (verbose) {
			System.out.println ("");
		}
		
		/* succeed */
		return true;
	}

	/* ======================================================= */
	/* class for callback */
	class SkipCallback implements NgCallbackInterface {
		/* (non-Javadoc)
		 * @see org.apgrid.grpc.ng.NgCallbackInterface#callback(java.util.List)
		 */
		public void callback(List args) {
			int size = ((int [])args.get(0))[0];
			int start = ((int [])args.get(1))[0];
			int end = ((int [])args.get(2))[0];
			int skip = ((int [])args.get(3))[0];
			int[][] in = ((int[][])args.get(4));
			int[][] out = ((int[][])args.get(5));
			
			for (int i = start; i < size; i += skip) {
				for (int j = start; j < size; j += skip) {
					out[i][j] = in[i][j] * 2;
				}
			}
		}
	}

	/* ======================================================= */
	/* compare float */
	private boolean compareFloat(float a, float b) {
		if (a == 0.0 || b == 0.0) {
			return (a == b);
		}
		return (Math.abs((a - b) / a)) < 0.000001;
	}
	
	/* compare double */
	private boolean compareDouble(double a, double b) {
		if (a == 0.0 || b == 0.0) {
			return (a == b);
		}
		return (Math.abs((a - b) / a)) < 0.000001;
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
		
		System.out.println("=====    SkipTest start    =====");
		SkipTest skipTest = null;
		try {
			skipTest = new SkipTest(params[0]);

			/* ===== skip test ===== */
			/* char test */
			System.out.print ("Skip Char Test : ");
			if (skipTest.skipCharTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* short test */
			System.out.print ("Skip Short Test : ");
			if (skipTest.skipShortTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* int test */
			System.out.print ("Skip Int Test : ");
			if (skipTest.skipIntTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* long test */
			System.out.print ("Skip Long Test : ");
			if (skipTest.skipLongTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* float test */
			System.out.print ("Skip Float Test : ");
			if (skipTest.skipFloatTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* double test */
			System.out.print ("Skip Double Test : ");
			if (skipTest.skipDoubleTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* scomplex test */
			System.out.print ("Skip Scomplex Test : ");
			if (skipTest.skipScomplexTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* dcomplex test */
			System.out.print ("Skip Dcomplex Test : ");
			if (skipTest.skipDcomplexTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* inout test */
			System.out.print ("Skip inout Test : ");
			if (skipTest.skipInOutTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* skip one test */
			System.out.print ("Skip one Test : ");
			if (skipTest.skipOneTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* skip 2D test */
			System.out.print ("Skip 2D Test : ");
			if (skipTest.skip2DTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* skip 3D test */
			System.out.print ("Skip 3D Test : ");
			if (skipTest.skip3DTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* omit skip test */
			System.out.print ("Skip Omit Skip Test : ");
			if (skipTest.skipOmitSkipTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* omit end test */
			System.out.print ("Skip Omit End Test : ");
			if (skipTest.skipOmitEndTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
			
			/* omit start test */
			System.out.print ("Skip Omit Start Test : ");
			if (skipTest.skipOmitEndTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}

			/* callback test */
			System.out.print ("Skip Callback Test : ");
			if (skipTest.skipCallbackTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			try {
				if (skipTest != null) {
					/* deactivate client */
					skipTest.deactivateClient();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println("===== SkipTest was finished =====");
	}
}
