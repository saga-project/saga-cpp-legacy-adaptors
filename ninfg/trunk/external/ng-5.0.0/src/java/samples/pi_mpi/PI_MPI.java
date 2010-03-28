/**
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
 * $RCS_file$ $Revision: 1.1 $ $Date: 2008/02/07 08:43:12 $
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
 * invoke Ninf-G Executable(calculate PI on MPI).
 */
public class PI_MPI implements Runnable {
	/* definitions */
	private static final String targetExe = "pi_mpi/pi_trial";
	private static int TIMES;
	
	/* verbose? */
	public static final String printInfo = "ninfg.sample.printInfo";
	
	/* instance variables */
	private GrpcClient context;
	private GrpcFunctionHandle myHandle;
	private GrpcExecInfo myExecInfo;
	private String hostName;
	private Integer intNumber;
	private Long longTimes;
	private long[] count;
	
	/* constructor for this class */
	public PI_MPI(GrpcClient context, String hostName, int numHosts, int number) {
		this.context = context;
		this.hostName = hostName;
		this.intNumber = new Integer(number);
		this.longTimes = new Long(TIMES / numHosts);
		this.count = new long[1];
	}

	public static void main (String args[]) {
		/* check arguments */
		if (args.length < 3) {
			System.err.println (
				"must specify number of times, name of configfile and server");
			System.err.println (
				"usage: java PI configfile TIMES servername ...");
			System.exit(1);
		}

		/* get name of configfile */
		String configName = args[0];
		if (new File(configName).exists() != true) {
			System.err.println (
				"specified config file( " + configName + " ) does not exists.");
			System.exit(1);
		}
		/* get variable of TIMES */
		try {
			TIMES = Integer.parseInt(args[1]);
		} catch (NumberFormatException e) {
			System.err.println ("must specify numeric variable at 2nd argument");
			System.exit(1);
		}
		/* get list of hostname */
		String[] hostName = new String[args.length - 2];
		int numHosts = hostName.length;
		for (int i = 0; i < hostName.length; i++) {
			hostName[i] = args[i+2];
		}

		/* start */
		System.out.println ("===== PI program start =====");			

		/* Init client */
		GrpcClient client = null;
		try {
			/* create GrpcClient */
			client = GrpcClientFactory.getClient(
				"org.apgrid.grpc.ng.NgGrpcClient");
			/* activate GrpcClient */
			client.activate(configName);
			
			/* call function */
			PI_MPI[] piArray = new PI_MPI[hostName.length];
			Thread[] piThread = new Thread[hostName.length];
			for (int i = 0; i < hostName.length; i++) {
				piArray[i] = new PI_MPI(client, hostName[i], hostName.length, i);
				piThread[i] = new Thread(piArray[i]);
				piThread[i].start();
			}
			
			/* wait for finish, then calc result */
			long sum = 0;
			for (int i = 0; i < hostName.length; i++) {
				piThread[i].join();
				sum += piArray[i].count[0];
			}
			double pi = 4.0 * (sum / (double)TIMES);

			/* print result */
			System.out.println ("PI = " + pi);
			String propPrintInfo = System.getProperty(printInfo);
			if ((propPrintInfo != null) && (propPrintInfo.equals("on"))) {
				/* print information about session */
				for (int i = 0; i < hostName.length; i++) {
					if (piArray[i].myExecInfo != null) {
						System.out.println (" ===== information of handle[" + i + "] =====");
						System.out.println ("(on server [" + hostName[i] +"])");
						System.out.println (piArray[i].myExecInfo);
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
		/* end */
		System.out.println ("===== PI program end =====");			
	}

	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		try {
			/* specify hostname */
			Properties prop = new Properties();
			prop.put(NgGrpcHandleAttr.KEY_HOSTNAME,	hostName);
				
			/* create handle */
			this.myHandle = context.getFunctionHandle(targetExe, prop);

			/* call function */
			myExecInfo = myHandle.call(longTimes, count);
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
