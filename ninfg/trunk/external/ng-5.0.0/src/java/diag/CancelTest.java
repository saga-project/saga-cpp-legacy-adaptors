/*
 * $RCSfile: CancelTest.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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

import org.apgrid.grpc.ng.NgGrpcFunctionHandle;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;

/**
 * Diagnostic program for Ninf-G
 * This program checks API for session cancel
 */
public class CancelTest implements Runnable {
	private static final String targetExe = "cancel/cancel";
	// specify time to sleep in second 
	private static final long SLEEP_TIME = 5;

	private GrpcFunctionHandle myHandle;

	public CancelTest (GrpcFunctionHandle myHandle){
		this.myHandle = myHandle;
	}
	
	public static void main (String args[]) {
		System.out.println("=====    CancelTest start     =====");
		GrpcClient client = null;
		GrpcFunctionHandle handle =	null;
		try {
			// create GrpcClient 
			client =
				GrpcClientFactory.getClient("org.apgrid.grpc.ng.NgGrpcClient");

			// check arguments 
			if (args.length < 1) {
				System.err.println("configFileName must be specified in argument");
			}

			// activate GrpcClient 
			client.activate(args[0]);
			
			// create GrpcHandle 
			handle = client.getFunctionHandle(targetExe);

			// call function 
			System.out.print ("Cancel Test : ");
			CancelTest cancelTest = new CancelTest(handle);
			Thread cancelThread = new Thread(cancelTest);
			cancelThread.start();
			
			// wait for a little 
			try {
				Thread.sleep(SLEEP_TIME * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			// cancel session 
			try {
				handle.cancel();
			} catch (GrpcException e) {
				e.printStackTrace();
			}
			
			// check if cancel was succeed with grpc_probe 
			NgGrpcFunctionHandle ngFunctionHandle =
				(NgGrpcFunctionHandle) handle;
			if ( ngFunctionHandle.isIdle() ) {
				System.out.println ("OK!");
			} else {
				System.out.println ("FAILED!");
			}
		} catch (GrpcException e) {
			e.printStackTrace();
		} finally {
			// dispose handle 
			if (handle != null) {
				try {
					handle.dispose();
				} catch (GrpcException e1) {
					e1.printStackTrace();
				}
			}
			try {
				if (client != null) {
					// deactivate client 
					client.deactivate();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		System.out.println("===== CancelTest was finished =====");
	}

	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		try {
			// start session 
			myHandle.call();
		} catch (GrpcException e) {
			e.printStackTrace();
		}
	}
}
