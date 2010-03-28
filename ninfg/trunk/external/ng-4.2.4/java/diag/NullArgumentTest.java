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
 * $RCS_file$ $Revision: 1.4 $ $Date: 2005/06/30 10:42:47 $
 */
//package diag;

import java.util.Vector;

import org.apgrid.grpc.ng.Dcomplex;
import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.Scomplex;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcObjectHandle;

/**
 * Diagnostic program for Ninf-G
 * This program checks data types of Ninf-G parameters.
 */
public class NullArgumentTest {
	private static final String targetModule = "nullArgument/";

	private static final int NELEMENTS = 5;
	private static boolean verbose = true;
	
	/* instance variables */
	private GrpcClient client;
	
	/* Initialize Client */
	public NullArgumentTest(String configName) throws GrpcException {
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
	private boolean nullArgumentTest() {
		final String targetEntry = "nullArgument";
		boolean success = true;
		
		GrpcObjectHandle handle = null;
		try {
			/* Result of RPC */
			int[] rpcResultArray = new int[3];
			/* create GrpcHandle */
			handle = this.client.getObjectHandle(targetModule + targetEntry);

			/* String scalar */
			handle.invoke("nullStringInScalar", (String)null, rpcResultArray);
			if (rpcResultArray[0] != 1) {
				throw new NgException("Remote Executable returned NG.");
			}

			/* String scalar */
			rpcResultArray[0] = 1;
			handle.invoke("nullStringInScalar", "", rpcResultArray);
			if (rpcResultArray[0] != 0) {
				throw new NgException("Remote Executable returned NG.");
			}
			
			/* String array */
			rpcResultArray[0] = 1;
			rpcResultArray[1] = 0;
			rpcResultArray[2] = 0;
			String[] stringArray = new String[3];
			stringArray[0] = null;
			stringArray[1] = "";
			stringArray[2] = "test";
				
			handle.invoke("nullStringInArray", new Integer(3), stringArray, rpcResultArray);
			if ((rpcResultArray[0] != 1) || (rpcResultArray[1] != 0) ||
				(rpcResultArray[2] != 0)) {
				throw new NgException("Remote Executable returned NG.");
			}
			
			/* Filename scalar */
			rpcResultArray[0] = 0;
			handle.invoke("nullFilenameIN", (String)null, rpcResultArray);
			if (rpcResultArray[0] != 1) {
				throw new NgException("Remote Executable returned NG.");
			}

			/* Filename scalar */
			rpcResultArray[0] = 1;
			handle.invoke("nullFilenameIN", "", rpcResultArray);
			if (rpcResultArray[0] != 0) {
				throw new NgException("Remote Executable returned NG.");
			}
			
			/* Filename scalar */
			rpcResultArray[0] = 0;
			handle.invoke("nullFilenameOUT", (String)null, rpcResultArray);
			if (rpcResultArray[0] != 1) {
				throw new NgException("Remote Executable returned NG.");
			}

			/* Filename scalar */
			rpcResultArray[0] = 1;
			handle.invoke("nullFilenameOUT", "", rpcResultArray);
			if (rpcResultArray[0] != 0) {
				throw new NgException("Remote Executable returned NG.");
			}
			
			/* call char_test Object */
			handle.invoke("nullArgument", new Integer(0),
				(byte [])null, (short [])null, (int [])null, (long [])null,
				(float [])null, (double [])null,
				(Scomplex [])null, (Dcomplex [])null);
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
		
		System.out.println("=====    NullArgumentTest start    =====");
		NullArgumentTest nullElementTest = null;
		try {
			nullElementTest = new NullArgumentTest(params[0]);

			/* zero_element test */
			System.out.print ("NullArgument Test : ");
			if (nullElementTest.nullArgumentTest() != true) {
				System.out.println("FAILED!");
			} else {
				System.out.println("OK!");
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			try {
				if (nullElementTest != null) {
					/* deactivate client */
					nullElementTest.deactivateClient();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println("===== NullArgumentTest was finished =====");
	}
}
