/*
 * $RCSfile: HandleConnection.java,v $ $Revision: 1.11 $ $Date: 2008/03/05 07:36:06 $
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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.InetAddress;
import java.util.List;

import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import static org.apgrid.grpc.ng.protocol.ProtocolHeader.FIRST_CONNECTION;
import org.gridforum.gridrpc.GrpcException;

class HandleConnection implements Runnable {

	private Socket socket;
	private PortManager server; // Negotiation Server
	// use XDR when negotiation 
	private XDRInputStream xdrIn;
	private XDROutputStream xdrOut;

	/**
	 * @param manager
	 * @param socket
	 */
	HandleConnection(PortManager manager, Socket socket) {
		this.server = manager;
		this.socket = socket;
	}

	/* 
	 * Ninf-G Negotiation process
	 */
	public void run(){
		boolean success = false;
		try {
			// get Input/OutputStream 
			this.xdrIn  = new XDRInputStream(socket.getInputStream());
			this.xdrOut = new XDROutputStream(socket.getOutputStream());

			///// negotiation  start
			byte [] buffer = receiveConnectInfo();
			// receive RemoteMachineInfo
			NgConnectInfo remoteConnectInfo =
				NgConnectInfo.createRemoteConnectInfo(buffer);

			// get executable ID from Context
			int nConnect = remoteConnectInfo.getNumberOfConnection();
			int jobID = remoteConnectInfo.getJobID();
			int executableID;
			boolean sendEncodeType;
			if (nConnect == FIRST_CONNECTION) {
				sendEncodeType = true;
				executableID = server.getContext().getExecutableID(jobID);
				remoteConnectInfo.setExecutableID(executableID);
			} else {
				sendEncodeType = false;
				executableID = remoteConnectInfo.getExecutableID();
			}

			// check if information of connection were valid
			// send Result
			sendResult( remoteConnectInfo.check() );

			// tentative (throws Exception)
			checkSimpleAuthorization( remoteConnectInfo );

			// send LocalMachineInfo and receive Result 
			RemoteMachineInfo remoteMachineInfo =
				server.getContext().getRemoteMachineInfoForJob(jobID);

			int auth_no = server.getContext().getAuthNum(jobID);
			sendLocalConnectInfo( NgConnectInfo.createLocalConnectInfo(
				server.getContext().getID(), executableID,
				0, jobID, remoteMachineInfo, auth_no,
				sendEncodeType, nConnect));

			// Receive Result about local connection information sending.
			int result = xdrIn.readInt();
			if (result != 0) {
				throw new NgExecRemoteMethodException(
					"NGCONNECT: failed to negotiate result["+ result +"]");
			}
			//// negotiation end

			// get supported expressions 
			List encodeType = null;
			if (remoteConnectInfo.getParameterLength() != 0) {
				encodeType = remoteConnectInfo.getSupportEncodeType();
			}

			// set TcpNoDelay
			if (Boolean.valueOf(remoteMachineInfo.getTcpNodelay())) {
				socket.setTcpNoDelay(true);
			}
			if (remoteMachineInfo.getCommunicationProxyInfo().getCommunicationProxy() != RemoteMachineInfo.DISABLE_COMMUNICATION_PROXY) {
				InetAddress peerAddress = null;
				peerAddress = socket.getInetAddress();
				if (peerAddress == null) {
					throw new NgIOException(
						"Socket is not connected");
				}
				if (!peerAddress.isLoopbackAddress() ) {
					throw new NgIOException(
						"The handle uses Communication Proxy, but it is connected from remote host.\n");
				}
			}

			// create Communicator
			Communicator comm =
				new Communicator(
						new XDRWire(socket),
						remoteConnectInfo.getVersion(),
						encodeType);

			// register Communicator 
			server.putCommunicator(executableID, comm);
			success = true;
		} catch (GrpcException e) {
			server.getContext().getNgLog().logFatal(NgLog.CAT_NG_PROTOCOL,
				e);
		} catch (IOException e) {
			server.getContext().getNgLog().logFatal(NgLog.CAT_NG_PROTOCOL, 
				new NgExecRemoteMethodException(e) );
		} finally {
			if (!success) {
				if (socket != null) {
					try {
						socket.close();
					} catch (IOException e) {
						// Ignore exception.
					}
					socket = null;
				}
			}
		}
	}


	private void checkSimpleAuthorization(NgConnectInfo remoteConnectInfo )
	 throws NgException {
		int target       = remoteConnectInfo.getAuthNum();
		int jobID	 = remoteConnectInfo.getJobID();
		int registered   = server.getContext().getAuthNum(jobID);

		if (target == registered) {
			server.getContext().getNgLog().logInfo(NgLog.CAT_NG_PROTOCOL,
				"Simple Authorization Number matched.");
			server.getContext().getNgLog().logDebug(NgLog.CAT_NG_PROTOCOL,
				"Simple Authorization Number: Registered [" + registered 
				+ "] Ninf-G Executable's[" + target +"]");
			return;
		} else {
			throw new NgException("Registered simple authorization number[" 
				+ registered + "] doesn't match Ninf-G Job's[" 
				+ jobID + "] it[" + target + "].");
		}
	}


	/**
	 * @return
	 * @throws GrpcException
	 */
	private byte[] receiveConnectInfo() throws GrpcException{
		byte[] receiveInfo = null;
		try {
			// receive header 
			int header_length = 32;
			byte[] header = new byte[header_length];
			int nRead = xdrIn.read(header);
			ByteArrayInputStream bi =
				new ByteArrayInputStream(header, header_length - 4, 4);
			XDRInputStream xi = new XDRInputStream(bi);
			int length = xi.readInt();
			xi.close();

			// receive other parameters 
			byte[] otherParameter = new byte[length];
			nRead = xdrIn.read(otherParameter);
			
			// put header and parameters into buffer 
			ByteArrayOutputStream bo =
				new ByteArrayOutputStream(NgGlobals.smallBufferSize);
			bo.write(header, 0, header_length);
			bo.write(otherParameter, 0, length);

			receiveInfo = bo.toByteArray();
			bo.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		return receiveInfo;
	}

	/**
	 * @param result
	 * @throws GrpcException
	 */
	private void sendResult(int result) throws GrpcException {
		try {
			xdrOut.writeInt(result);
			xdrOut.flush();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @param localConnectInfo
	 * @throws GrpcException
	 */
	private void sendLocalConnectInfo(NgConnectInfo localConnectInfo)
	 throws GrpcException{
		try {
			xdrOut.write(localConnectInfo.toByteArray());
			xdrOut.flush();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	private int recvResult() throws GrpcException {
		return xdrIn.readInt();
	}
	
}

