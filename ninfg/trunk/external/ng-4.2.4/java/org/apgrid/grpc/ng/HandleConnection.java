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
 * $RCSfile: HandleConnection.java,v $ $Revision: 1.30 $ $Date: 2005/07/12 10:32:37 $
 */

package org.apgrid.grpc.ng;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.List;

import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.gridforum.gridrpc.GrpcException;

class HandleConnection implements Runnable {
	private Socket socket;
	private PortManager manager;
	/* use XDR when negotiation */
	private XDRInputStream xdrIn;
	private XDROutputStream xdrOut;
		
	/**
	 * @param manager
	 * @param socket
	 */
	HandleConnection(PortManager manager, Socket socket) {
		this.manager = manager;
		this.socket = socket;
	}

	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run(){
		try {
			/* get Input/OutputStream */
			this.xdrIn = new XDRInputStream(socket.getInputStream());
			this.xdrOut = new XDROutputStream(socket.getOutputStream());
			
			/* negotiation */
			byte[] buffer = receiveConnectInfo();
			/* receive RemoteMachineInfo and send Result */
			NgConnectInfo remoteConnectInfo =
				NgConnectInfo.getRemoteConnectInfo(buffer);

			/* get subJobID */
			int subJobID = manager.getContext().generateSubJobID(
				remoteConnectInfo.getJobID());
			/* get executable ID */
			int executableID = manager.getContext().getExecutableID(
				remoteConnectInfo.getJobID(), subJobID);
			remoteConnectInfo.setExecutableID(executableID);

			/* check if information of connection were valid */
			sendResult(NgConnectInfo.checkConnectInfo(remoteConnectInfo));

			/* send LocalMachineInfo and receive Result */
			RemoteMachineInfo remoteMachineInfo =
				manager.getContext().getRemoteMachineInfoForJob(remoteConnectInfo.getJobID());
			
			sendConnectInfo(NgConnectInfo.getLocalConnectInfo(
				manager.getContext().getID(), executableID,
				0, remoteConnectInfo.getJobID(), remoteMachineInfo));
			int result = recvResult();
			if (result != 0) {
				throw new NgExecRemoteMethodException(
					"NGCONNECT: failed to negotiate");
			}

			/* get supported expressions */
			List encodeType = null;
			if (remoteConnectInfo.getParameterLength() != 0) {
				encodeType =
					NgConnectInfo.getSupportEncodeType(remoteConnectInfo);
			}

			/* set TcpNoDelay */
			if (Boolean.valueOf((String)remoteMachineInfo.get(RemoteMachineInfo.KEY_TCP_NODELAY)).booleanValue()) {
				socket.setTcpNoDelay(true);
			}
				
			/* make Wire and Communicator */
			Wire wire = makeWire(encodeType);
			Communicator comm = makeCommunicator(wire, executableID);
			comm.setEncodeType(encodeType);
			comm.setVersion(remoteConnectInfo.getVersionMajor(),
				remoteConnectInfo.getVersionMinor(),
				remoteConnectInfo.getVersionPatch());
			
			/* register Communicator */
			manager.putCommunicator(executableID, comm);
		} catch (GrpcException e) {
			try {
				manager.getContext().getNgLog().printLog(
					NgLog.LOGCATEGORY_NINFG_PROTOCOL,
					NgLog.LOGLEVEL_FATAL,
					e);
			} catch (GrpcException e1) {
				/* can't manage */
			}
		} catch (IOException e) {
			try {
				manager.getContext().getNgLog().printLog(
					NgLog.LOGCATEGORY_NINFG_PROTOCOL, 
					NgLog.LOGLEVEL_FATAL,
					new NgExecRemoteMethodException(e));
			} catch (GrpcException e1) {
				/* can't manage */
			}
		}
	}
	
	/**
	 * @return
	 * @throws GrpcException
	 */
	private byte[] receiveConnectInfo() throws GrpcException{
		byte[] receiveInfo = null;
		try {
			/* receive header */
			int header_length = 32;
			byte[] header = new byte[header_length];
			int nRead = xdrIn.read(header);
			ByteArrayInputStream bi =
				new ByteArrayInputStream(header,
				header_length - 4, header_length - 1);
			XDRInputStream xi = new XDRInputStream(bi);
			int length = xi.readInt();
			xi.close();
			
			/* receive other parameters */
			byte[] otherParameter = new byte[length];
			nRead = xdrIn.read(otherParameter);
			
			/* put header and parameters into buffer */
			ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
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
	private void sendConnectInfo(NgConnectInfo localConnectInfo) throws GrpcException{
		try {
			xdrOut.write(localConnectInfo.getByteArray());
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
	
	/**
	 * @param encodeType
	 * @return
	 * @throws GrpcException
	 */
	private Wire makeWire(List encodeType) throws GrpcException {
		/* now we support XDRWire only */
		/* maybe check forceXDR here */
		return new XDRWire(socket);
	}

	/**
	 * @param wire
	 * @param executableID
	 * @return
	 */
	private Communicator makeCommunicator(Wire wire, int executableID)
		throws GrpcException {
		return new Communicator(wire);
	}
}
