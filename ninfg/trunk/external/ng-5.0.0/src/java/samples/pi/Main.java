/*
 * $RCSfile: Main.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:08 $
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
 * Call the Function
 */
class PI implements Runnable {
	// Target calculate module
	private static final String functionName = "pi/pi_trial";

	// Instance variables 
	private GrpcClient client;
	private GrpcFunctionHandle handle;
	private GrpcExecInfo execInfo;
	private String exechost;

	// Arguments for calculate module(pi/pi_trial)
	private Integer seed; 
	private Long times;
	private long[] count;
	

	public PI(GrpcClient client, int nSampling,
	 String hostName, int numHosts, int number) {
		this.client = client;
		this.exechost = hostName;
		this.execInfo = null;
		this.seed  = new Integer(number);
		this.times = new Long(nSampling / numHosts);
		this.count = new long[1];
	}

	public String getExecHost() {
		return this.exechost;
	}

	public GrpcExecInfo getExecInfo() {
		return this.execInfo;
	}

	public long getCount() {
		return this.count[0];
	}

	public void run() {
		// Set hostname attribute for Function Handle
		Properties prop = new Properties();
		prop.put(NgGrpcHandleAttr.KEY_HOSTNAME,	exechost);

		try {
			// Get Function Handle 
			this.handle = client.getFunctionHandle(functionName, prop);

			// Call function(pi_trial)
			execInfo = handle.call(seed, times, count);

		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			if (handle == null) { return ; }
			try {
				handle.dispose();
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
	}

}


/**
 * Ninf-G Java Client Program Sample
 * Calculates the circle ratio using Monte-Carlo simulation.
 */
public class Main {
	private static final String printInfo = "ninfg.sample.printInfo";

	public static void main(String [] args) {
		String configName = null;
		int nSampling = 1;
		String[] exechosts = null;

		// Set the Command line Arguments
		try {
			checkArguments(args);
			configName = getConfigFile(args[0]);
			nSampling  = getNumberOfSampling(args[1]);
			exechosts  = getNgExecutableHosts(args);
		} catch (IllegalArgumentException e) {
			System.err.println(e.getMessage());
			System.exit(1);
		}

		System.out.println ("===== PI program start =====");

		GrpcClient client = null;
		try {
			// Set GrpcClient(Ninf-G Client)
			client =
				 GrpcClientFactory.getClient("org.apgrid.grpc.ng.NgGrpcClient");

			// Activate GrpcClient 
			client.activate(configName);

			// Set the workers of pi computation
			PI[] workers = new PI[exechosts.length];
			Thread[] piThreads = new Thread[exechosts.length];
			for (int i = 0; i < exechosts.length; i++) {
				workers[i] =
					new PI(client, nSampling, exechosts[i], exechosts.length, i);
				piThreads[i] = new Thread(workers[i]);
				piThreads[i].start(); 
			}

			// Wait for finish, then calc result
			long sum = 0;
			for (int i = 0; i < exechosts.length; i++) {
				piThreads[i].join();
				sum += workers[i].getCount();
			}
			double result = 4.0 * (sum / (double)nSampling);

			printResult(result, workers);

		} catch (GrpcException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (Throwable e) {
			System.err.println(e.getMessage());
			e.printStackTrace();
			System.err.println("===== PI program end =====");
			System.exit(1);
		} finally {
			try {
				if (client != null) {
					client.deactivate();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println("===== PI program end =====");
	}


	private static void checkArguments(String [] args) {
		if ((args.length < 3) || isSetNullArgument(args) ) {
			StringBuilder sb = new StringBuilder("args: ");
			for (int i = 0; i <  args.length; i++) {
				sb.append("[" + args[i] + "] ");
			}
			sb.append("\n");
			sb.append("Usage: java Main configfile nSampling servername ...");
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
		String [] result = new String[args.length - 2];
		for (int i = 0; i < result.length ; i++) {
			result[i] = args[i+2];
		}
		return result;
	}

	private static int getNumberOfSampling(String number) {
		try {
			return Integer.parseInt(number);
		} catch (NumberFormatException e) {
			throw new IllegalArgumentException(
				"must specify numeric variable at 2nd argument");
		}
	}

	private static void printResult(double pi, PI [] workers) {
		System.out.println ("PI = " + pi);
		if ( !  isSetPrintInfoProperty() ) { return; }

		// print information about session
		StringBuilder sb;
		for (int i = 0; i < workers.length; i++) {
			if (workers[i].getExecInfo() == null) {
				System.out.println(
					"couldn't receive information about handle[" + i + "]...");
				continue;
			}
			sb = new StringBuilder();
			sb.append(" ===== information of handle[")
			  .append( i )
			  .append("] =====");
			sb.append("\n");
			sb.append("(on server [")
			  .append( workers[i].getExecHost() )
			  .append("])");
			sb.append("\n");
			sb.append( workers[i].getExecInfo() );
			sb.append("\n");
			System.out.println(sb.toString());
		}
	}

	private static boolean isSetPrintInfoProperty() {
		String prop = System.getProperty(printInfo);
		if (prop == null) { return false; }
		return prop.equals("on");
	}

}
