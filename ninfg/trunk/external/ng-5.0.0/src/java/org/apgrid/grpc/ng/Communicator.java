/*
 * $RCSfile: Communicator.java,v $ $Revision: 1.10 $ $Date: 2008/03/12 11:27:16 $
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
import java.io.IOException;
import java.util.List;

import org.apgrid.grpc.ng.NgIOException;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.apgrid.grpc.ng.protocol.ProtocolFactory;
import org.gridforum.gridrpc.GrpcException;

class Communicator {
	private Wire wire;
	private Peer peer;
	private Peer mySelf;
	private List encodeType;
	private Version version; // protocol version 

	/**
	 * Constructor 
	 * 
	 * @param wire
	 * @param version 
	 * @param encodeTypes
	 */
	public Communicator(Wire wire, Version version, List encodeTypes) {
		this.wire = wire;
		this.peer = wire.getPeer();
		this.mySelf = wire.getMySelf();
		this.version = version;
		this.encodeType = encodeTypes;
	}

	/**
	 * @param remoteMachineInfo
	 * @throws GrpcException
	 */
	protected void initComm(RemoteMachineInfo remoteMachineInfo)
	 throws GrpcException {
		// Do nothing.
	}

	/**
	 * finalize this communicator
	 */
	protected void finalComm() {
		// Do nothing.
	}

	/**
	 * @return
	 */
	protected Protocol getCommand() throws GrpcException {
		if (wire == null)
			throw new IllegalStateException();

		// read header 
		byte[] buffer = new byte[32];
		byte[] ret = null;
		ret = wire.receiveBytes(buffer, 0, buffer.length);
		if (ret == null) {
			return null;
		}
		// create InputStream 
		NgDataInputStream ngdi = getNgInputStream(buffer);
		// create Protocol with header data
		Protocol returnProtocol = ProtocolFactory.create(ngdi);

		// close InputStream 
		try {
			ngdi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		// set version 
		returnProtocol.setVersion(version);

		return returnProtocol;
	}
	
	/**
	 * @param prot
	 * @return
	 * @throws GrpcException
	 */
	void parseParam(Protocol received_prot, NgProtocolRequest target_prot)
	 throws GrpcException {
		received_prot.parseParam(wire, target_prot);
	}

	/**
	 * @param buffer
	 * @return
	 */
	private NgDataInputStream getNgInputStream(byte[] buffer) {
		ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
		// create XDRStream and return it
		return new XDRInputStream(bi);
	}
	
	/*
	 * Put Request to Ninf-G Executable
	 */
	public void put(NgProtocolRequest req) throws GrpcException {
		if (wire == null)
			throw new IllegalStateException("wire field is null");

		req.send(wire);
		wire.flush();
	}

	/**
	 * @throws GrpcException
	 */
	protected void close() throws GrpcException {
		wire.close();
		wire = null;
		peer = null;
		mySelf = null;
	}
	
	/**
	 * @return
	 */
	public Peer getMySelf() {
		return mySelf;
	}
	
	/**
	 * @return
	 */
	public Peer getPeer() {
		return peer;
	}

	/**
	 * @return
	 */
	public List getEncodeTypes() {
		return encodeType;
	}

	public Version getVersion() {
		return this.version;
	}

	public void setCommLog(NgLog commLog) {
		commLog.setClientName( mySelf.getHostName() );
		commLog.setServerName( peer.getHostName() );
		wire.setLogger(commLog);
	}

	protected void setReceiveCallback(NgCallbackInterface callback) {
		wire.setReceiveCallback(callback);
	}
}

