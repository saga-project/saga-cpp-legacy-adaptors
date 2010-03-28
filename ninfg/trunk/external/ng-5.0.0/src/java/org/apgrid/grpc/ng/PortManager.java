/*
 * $RCSfile: PortManager.java,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:07 $
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
package org.apgrid.grpc.ng;

import java.io.IOException;
import java.net.Socket;

import org.gridforum.gridrpc.GrpcException;

class PortManager extends NgAbstractServer {
	private NgGrpcClient context;
	private CommunicatorTable commTable;
	
	/**
	 * @param port
	 * @throws IOException
	 */
	public PortManager(NgGrpcClient context, int port) throws IOException {
		super(port);
		this.context = context;
		init();
	}

	/**
	 * 
	 */
	private void init() {
		commTable = context.getCommTable();
	}
	
	/* (non-Javadoc)
	 * @see org.globus.net.BaseServer#handleConnection(java.net.Socket)
	 */
	protected void handleConnection(Socket socket) {
		HandleConnection handle = new HandleConnection(this, socket);
		new Thread(handle).start();
	}
	
	/**
	 * @param executableID
	 * @return
	 * @throws GrpcException
	 */
	protected Communicator getCommunicator(int executableID, long timeout,
	 NgGrpcJob job) throws GrpcException {
		return commTable.getCommunicator(executableID, timeout, job);
	}

	/**
	 * @param executableID
	 * @param NgGrpcJob
	 */
	protected Communicator getCommunicatorNoTimeout(
		int executableID, NgGrpcJob job) {
		return commTable.getCommunicatorNoTimeout(executableID, job);
	}
	
	/**
	 * @param executableID
	 * @param comm
	 * @throws GrpcException
	 */
	protected void putCommunicator(int executableID, Communicator comm)
		throws GrpcException {
		commTable.putCommunicator(executableID, comm);
	}
	
	/**
	 * @return
	 */
	protected NgGrpcClient getContext() {
		return context;
	}
}
