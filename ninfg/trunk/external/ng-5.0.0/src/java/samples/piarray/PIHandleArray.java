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
 * $RCS_file$ $Revision: 1.4 $ $Date: 2008/02/15 11:59:26 $
 */

import java.util.Properties;

import org.apgrid.grpc.ng.NgGrpcHandleAttr;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;

public class PIHandleArray implements Runnable {
	/* definitions */
	private static final String targetExe = "pi/pi_trial";
	private static final int nJobs = 10;
	
	/* instance variables */
	private GrpcFunctionHandle myHandle;
	private Integer seed;
	private Long times;
	private long[] count;
	
	/* constructor for this class */
	public PIHandleArray(GrpcFunctionHandle myHandle, int number, int nSampling) {
		this.myHandle = myHandle;
		this.seed = new Integer(number);
		this.times = new Long(nSampling / nJobs);
		this.count = new long[1];
	}

	public static void main (String args[]) {
		try {
			if (args.length != 3) {
				System.err.println("Usage: java PIHandleArray configFile nSampling serverName");
				System.exit(1);
			}
			String configName = args[0];
			int nSampling = Integer.parseInt(args[1]);
			String hostname = args[2];
			/* create GrpcClient */
			GrpcClient client = GrpcClientFactory.getClient(
				"org.apgrid.grpc.ng.NgGrpcClient");
			/* activate GrpcClient */
			client.activate(configName);
			
			/* create GrpcHandle */
			GrpcFunctionHandle[] handle = null;
			/* specify hostname */
			Properties prop = new Properties();
			prop.put(NgGrpcHandleAttr.KEY_HOSTNAME, hostname);
			
			handle = client.getFunctionHandles(targetExe, prop, nJobs);
			
			/* call function */
			PIHandleArray[] piArray = new PIHandleArray[nJobs];
			Thread[] piThread = new Thread[nJobs];
			for (int i = 0; i < nJobs; i++) {
				piArray[i] = new PIHandleArray(handle[i], i, nSampling);
				piThread[i] = new Thread(piArray[i]);
				piThread[i].start();
			}
			
			/* wait for finish, then calc result */
			long sum = 0;
			for (int i = 0; i < nJobs; i++) {
				piThread[i].join();
				sum += piArray[i].count[0];
			}
			double pi = 4.0 * (sum / (double)nSampling);

			/* print result */
			System.out.println ("PI = " + pi);

			/* deactivate client */
			client.deactivate();
		} catch (NumberFormatException e) {
			System.err.println("Invalid argument: " + args[1]);
			System.exit(1);
		} catch (GrpcException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		try {
			myHandle.call(seed, times, count);
			
			myHandle.dispose();
		} catch (GrpcException e) {
			e.printStackTrace();
		}
	}
}
