/*
 * $RCSfile: ADD.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:08 $
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

import java.io.File;
import java.util.Properties;

import org.apgrid.grpc.ng.NgGrpcHandleAttr;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcExecInfo;
import org.gridforum.gridrpc.GrpcFunctionHandle;

/**
 * Ninf-G Java Client Program Sample
 * invoke Ninf-G Executable(add dimensions and return them).
 */
public class ADD implements Runnable {
	// difinitions 
	private static final String targetExe = "add/dadd";
	private static final int DEPS = 3;

	public static final String printInfo = "ninfg.sample.printInfo";
	
	// instance variable 
	private GrpcClient context;
	private GrpcFunctionHandle handle;
	private GrpcExecInfo execInfo;
	private Properties handleAttr;
	private double[][] param1;
	private double[][] param2;
	private double receiveData[][];

	public ADD(GrpcClient context, String hostname,
	 double[][] param1, double[][] param2) {
		this.context = context;
		this.handleAttr = new Properties();
		handleAttr.put(NgGrpcHandleAttr.KEY_HOSTNAME, hostname);

		this.param1 = param1;
		this.param2 = param2;
		this.receiveData = new double[DEPS][DEPS];
		for (int i = 0; i < this.receiveData.length; i++) {
			for (int j = 0; j < this.receiveData[i].length; j++) {
				this.receiveData[i][j] = 0.0;
			}
		}
	}

	public void run() {
		// call function 
		try {
			// create GrpcHandle 
			handle = context.getFunctionHandle(targetExe, handleAttr);

			// call 
			execInfo =
				handle.call(new Integer(DEPS), param1, param2, receiveData);
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
	}


	public static void main(String [] args) {
		String configName   = null;
		String [] hostnames = null;

		try {
			checkArguments(args);
			configName = getConfigFile(args[0]);
			hostnames  = getNgExecutableHosts(args);
		} catch (IllegalArgumentException e) {
			System.err.println(e.getMessage());
			System.err.println("Usage: java ADD config hostname...");
			System.exit(1);
		}

		// Setup params 
		double param1[][] = createParam(DEPS, 1.0);
		double param2[][] = createParam(DEPS, 500.0);

		// start 
		System.out.println("===== ADD program start =====");			

		// print param1, param2 array
		printArray(param1);
		printArray(param2);

		// Init client 
		GrpcClient client = null;
		try {
			// create GrpcClient 
			client =
				GrpcClientFactory.getClient("org.apgrid.grpc.ng.NgGrpcClient");
			// activate GrpcClient 
			client.activate(configName);
			
			// create handle 
			ADD[] addArray     = new ADD[hostnames.length];
			Thread[] addThread = new Thread[hostnames.length];
			for (int i = 0; i < hostnames.length; i++) {
				addArray[i]  = new ADD(client, hostnames[i], param1, param2);
				addThread[i] = new Thread(addArray[i]);
				addThread[i].start();
			}

			for (int i = 0; i < hostnames.length; i++) {
				addThread[i].join();
				System.out.println("--> result on " + hostnames[i] + " <--");
				printArray(addArray[i].receiveData);
			}

			printExecInfo(hostnames, addArray);

		} catch (GrpcException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} finally {
			try {
				if (client != null) {
					client.deactivate();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println ("===== ADD program end =====");			
	}


	private static void checkArguments(String [] args) {
		if ((args.length < 2) || isSetNullArgument(args) ) {
			StringBuilder sb = new StringBuilder("args: ");
			for (int i = 0; i < args.length; i++) {
				sb.append("[" + args[i] + "] ");
			}

			sb.append("\n");
			sb.append("must specify name of configfile and hostname");
			throw new IllegalArgumentException(sb.toString());
		}
	}

	private static boolean isSetNullArgument(String [] args) {
		for (int i = 0; i < args.length; i++) {
			if (args[i].length() == 0) {
				return true;
			}
		}
		return false;
	}

	private static String getConfigFile(String name) {
		if ( new File(name).exists() ) {
			return name;
		}
		throw new IllegalArgumentException(
			"specified config file( " + name + " ) does not exists.");
	}

	private static String [] getNgExecutableHosts(String [] args) {
		String[] result = new String[args.length - 1];
		for (int i = 0; i < result.length; i++) {
			result[i] = args[i + 1];
		}
		return result;
	}

	private static double[][] createParam(final int deps, final double init) {
		double[][] ret = new double[deps][deps];
		double value   = init;
		for (int i = 0; i < deps; i++) {
			for (int j = 0; j < deps; j++) {
				ret[i][j] = value++;
			}
		}
		return ret;
	}
	
	// print Array elements 
	private static void printArray(double[][] targetArray) {
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < targetArray.length; i++) {
			for (int j = 0; j < targetArray[i].length; j++) {
				sb.append( targetArray[i][j] + " ");
			}
			sb.append("\n");
		}
		System.out.println(sb.toString());
	}

	private static void printExecInfo(String [] hostnames, ADD[] addArray) {
		if ( ! isSetPrintInfoProperty() ) { return ; }
		// print information about session 
		for (int i = 0; i < hostnames.length; i++) {
			if (addArray[i].execInfo != null) {
				StringBuilder sb = new StringBuilder();
				sb.append(" ===== information of handle["
					+ i + "] =====\n");
				sb.append("(on server [" + hostnames[i] +"])\n");
				sb.append( addArray[i].execInfo );
				System.out.println( sb.toString() );
			} else {
				System.out.println(
					"couldn't receive information about handle[" + i + "]...");
			}
		}
	}

	private static boolean isSetPrintInfoProperty() {
		String prop = System.getProperty(printInfo);
		if (prop == null) { return false; }
		return prop.equals("on");
	}

}

