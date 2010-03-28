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
 * $RCSfile: ADD.java,v $ $Revision: 1.7 $ $Date: 2005/10/26 10:11:26 $
 */
//package sample;

import java.io.File;
import java.util.Properties;

import org.apgrid.grpc.ng.NgGrpcHandleAttr;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcExecInfo;
import org.gridforum.gridrpc.GrpcFunctionHandle;

/**
 * This is a sample program for Ninf-G v2.
 * invoke Ninf-G Executable(add dimensions and return them).
 */
public class ADD implements Runnable {
	/* difinitions */
	private static final String targetExe = "add/dadd";
	private static final int DEPS = 3;
	private static int TIMES;
	
	/* verbose? */
	public static final String printInfo = "ninfg.sample.printInfo";
	
	/* instance variable */
	private GrpcClient context;
	private GrpcFunctionHandle myHandle;
	private GrpcExecInfo myExecInfo;
	private Properties handleAttr;
	private double[][] param1;
	private double[][] param2;
	private double receiveData[][];
	
	/* constructor */
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

	public static void main (String args[]) {
		/* check arguments */
		if (args.length < 2) {
			System.err.println ("must specify name of configfile and hostname");
		}

		/* get name of configfile */
		String configName = args[0];
		if (new File(configName).exists() != true) {
			System.err.println (
			"specified config file( " +configName + " ) does not exists.");
		}
		/* get list of hostname */
		String[] hostName = new String[args.length -1];
		for (int i = 0; i < hostName.length; i++) {
			hostName[i] = args[i+1];
		}
		/* set TIMES */
		TIMES = hostName.length;

		/* setup params */
		double param1[][] = new double[DEPS][DEPS];
		double param2[][] = new double[DEPS][DEPS];
		double wk1, wk2;
			
		/* Initialize Parameter */
		wk1 = 1.0;
		wk2 = 500.0;
		for (int i = 0; i < DEPS; i++) {
			for (int j = 0; j < DEPS; j++) {
				param1[i][j] = wk1++;
				param2[i][j] = wk2++;
			}
		}
		
		/* start */
		System.out.println ("===== ADD program start =====");			

		/* print param1, param2 array */
		printArray(param1);
		printArray(param2);
			
		/* Init client */
		GrpcClient client = null;
		try {
			/* create GrpcClient */
			client = GrpcClientFactory.getClient(
				"org.apgrid.grpc.ng.NgGrpcClient");
			/* activate GrpcClient */
			client.activate(configName);
			
			/* create handle */
			ADD[] addArray = new ADD[hostName.length];
			Thread[] addThread = new Thread[hostName.length];

			/* call function */
			for (int i = 0; i < hostName.length; i++) {
				addArray[i] = new ADD(client, hostName[i], param1, param2);
				addThread[i] = new Thread(addArray[i]);
				addThread[i].start();
			}
				
			/* sync */
			for (int i = 0; i < hostName.length; i++) {
				addThread[i].join();
				System.out.println ("--> result on " + hostName[i] + " <--");
				printArray(addArray[i].receiveData);
			}

			String propPrintInfo = System.getProperty(printInfo);
			if ((propPrintInfo != null) && (propPrintInfo.equals("on"))) {
				/* print information about session */
				for (int i = 0; i < hostName.length; i++) {
					if (addArray[i].myExecInfo != null) {
						System.out.println (" ===== information of handle[" + i + "] =====");
						System.out.println ("(on server [" + hostName[i] +"])");
						System.out.println (addArray[i].myExecInfo);
						System.out.println ("");
					} else {
						System.out.println (
							"couldn't receive information about handle[" + i + "]...");
					}
				}
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} finally {
			try {
				if (client != null) {
					/* deactivate client */
					client.deactivate();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		/* the end */
		System.out.println ("===== ADD program end =====");			
	}
	
	/* print Array elements */
	private static void printArray(double[][] targetArray) {
		for (int i = 0; i < targetArray.length; i++) {
			for (int j = 0; j < targetArray[i].length; j++) {
				System.out.print (targetArray[i][j] + " ");
			}
			System.out.println();
		}
		System.out.println();
	}

	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		/* call function */
		try {
			/* create GrpcHandle */
			myHandle = context.getFunctionHandle(targetExe, handleAttr);

			/* call */
			myExecInfo = myHandle.call(new Integer(DEPS), param1, param2, receiveData);
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			/* dispose handle */
			if (myHandle != null) {
				try {
					myHandle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
		}
	}
}
