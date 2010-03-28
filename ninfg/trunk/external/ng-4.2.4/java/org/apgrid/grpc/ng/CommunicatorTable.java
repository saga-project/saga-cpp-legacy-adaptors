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
 * $RCSfile: CommunicatorTable.java,v $ $Revision: 1.6 $ $Date: 2005/09/27 10:48:17 $
 */
package org.apgrid.grpc.ng;

import java.util.HashMap;

import org.gridforum.gridrpc.GrpcException;

class CommunicatorTable {
	private NgGrpcClient context;
	private HashMap commTable;

	/**
	 * @param context
	 */
	protected CommunicatorTable(NgGrpcClient context) {
		this.context = context;
		commTable = new HashMap();
	}

	/**
	 * @param executableID
	 * @return
	 * @throws GrpcException
	 */
	protected Communicator getCommunicator(
		int executableID, long timeout, NgGrpcJob job) throws GrpcException {
		Communicator comm = null;
		
		/* get count of loop */
		long sleepTime = 100;
		long numIncrement = (timeout == 0) ? 0 : 1;
		long loopCount = (timeout == 0) ? 1 : timeout / sleepTime;
		
		/* loop specified times to find Communicator */
		for (long i = 0; i < loopCount; i += numIncrement) {
			synchronized (commTable) {
				comm = (Communicator) commTable.get(String.valueOf(executableID));
				if (comm != null) {
					commTable.remove(String.valueOf(executableID));
					return comm;
				}
			}
			try {
				Thread.sleep(sleepTime);
				int jobStatus = job.getStatus();
				if ((jobStatus == NgGrpcJob.NGJOB_STATE_DONE) ||
					(jobStatus == NgGrpcJob.NGJOB_STATE_FAILED)) {
						/* fail to invoke Job */
						throw new NgInitializeGrpcHandleException(
							"PortManager: fail to invoke job.");
				}
			} catch (InterruptedException e) {
				throw new NgInitializeGrpcHandleException(e);
			}
		}
		/* fail to get Communicator */
		throw new NgInitializeGrpcHandleException(
			"PortManager: fail to get Communicator(timeout of invoking job)");
	}
	
	/**
	 * @param executableID
	 * @param comm
	 * @throws GrpcException
	 */
	protected synchronized void putCommunicator(int executableID, Communicator comm) throws GrpcException {
		commTable.put(String.valueOf(executableID), comm);
	}
	
}
