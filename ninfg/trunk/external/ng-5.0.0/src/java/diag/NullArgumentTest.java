/*
 * $RCSfile: NullArgumentTest.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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
 *
 * A client program which uses null as arguments of GrpcObjectHandle.invoke().
 * This program uses char, short, int, long, float, double,
 * scomplex, dcomplex and string types of variables.
 */
public class NullArgumentTest {
	private static final String targetModule = "nullArgument/";

	private static final int NELEMENTS = 5;
	private static boolean verbose = true;
	
	// instance variables 
	private GrpcClient client;
	
	// Initialize Client 
	public NullArgumentTest(String configName) throws GrpcException {
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


	// ===== call TEST PROGRAMS ===== 
	private boolean nullArgumentTest() {
		final String targetEntry = "nullArgument";
		boolean success = true;
		
		GrpcObjectHandle handle = null;
		try {
			// Result of RPC 
			int[] rpcResultArray = new int[3];
			// create GrpcHandle 
			handle = this.client.getObjectHandle(targetModule + targetEntry);

			// String scalar 
			handle.invoke("nullStringInScalar", (String)null, rpcResultArray);
			if (rpcResultArray[0] != 1) {
				throw new NgException("Remote Executable returned NG.");
			}

			// String scalar 
			rpcResultArray[0] = 1;
			handle.invoke("nullStringInScalar", "", rpcResultArray);
			if (rpcResultArray[0] != 0) {
				throw new NgException("Remote Executable returned NG.");
			}

			// String array  (So what?)
			rpcResultArray[0] = 1;
			rpcResultArray[1] = 0;
			rpcResultArray[2] = 0;

			String[] stringArray = new String[3];
			stringArray[0] = null;
			stringArray[1] = "";
			stringArray[2] = "test";
				
			handle.invoke("nullStringInArray",
				new Integer(3), stringArray, rpcResultArray);
			if ((rpcResultArray[0] != 1) || (rpcResultArray[1] != 0) ||
				(rpcResultArray[2] != 0)) {
				throw new NgException("Remote Executable returned NG.");
			}
			
///// Filename scalar begin
			rpcResultArray[0] = 0;
			handle.invoke("nullFilenameIN", (String)null, rpcResultArray);
			if (rpcResultArray[0] != 1) {
				throw new NgException("Remote Executable returned NG.");
			}

			rpcResultArray[0] = 1;
			handle.invoke("nullFilenameIN", "", rpcResultArray);
			if (rpcResultArray[0] != 0) {
				throw new NgException("Remote Executable returned NG.");
			}
			
			rpcResultArray[0] = 0;
			handle.invoke("nullFilenameOUT", (String)null, rpcResultArray);
			if (rpcResultArray[0] != 1) {
				throw new NgException("Remote Executable returned NG.");
			}

			rpcResultArray[0] = 1;
			handle.invoke("nullFilenameOUT", "", rpcResultArray);
			if (rpcResultArray[0] != 0) {
				throw new NgException("Remote Executable returned NG.");
			}
///// Filename scalar end
			
			// call nullArgument/nullArgument Object
			// TEST the validation of null argument
			handle.invoke("nullArgument", new Integer(0),
				(byte [])  null,
				(short []) null,
				(int [])   null,
				(long [])  null,
				(float []) null,
				(double [])null,
				(Scomplex [])null,
				(Dcomplex [])null);

		} catch (GrpcException e) {
			e.printStackTrace();
			return false;
		} finally {
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
		return success;
	}


	// ===== main routines ===== 
	// check argument for this program (accept "-verbose" and name of config) 
	static String[] parseArgs(String arg[]){
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

	// main routine for DataTest 
	public static void main(String[] args) throws InterruptedException {
		String[] params = parseArgs(args);
		
		System.out.println("=====    NullArgumentTest start    =====");
		NullArgumentTest nullElementTest = null;
		try {
			nullElementTest = new NullArgumentTest(params[0]);

			// zero_element test 
			System.out.print ("NullArgument Test : ");
			if ( nullElementTest.nullArgumentTest() ) {
				System.out.println("OK!");
			} else {
				System.out.println("FAILED!");
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			if (nullElementTest != null) {
				nullElementTest.deactivateClient();
			}
		}
		System.out.println("===== NullArgumentTest was finished =====");
	}
}
