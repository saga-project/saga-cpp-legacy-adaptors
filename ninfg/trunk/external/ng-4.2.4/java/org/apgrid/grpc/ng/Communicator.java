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
 * $RCSfile: Communicator.java,v $ $Revision: 1.29 $ $Date: 2006/01/27 03:31:05 $
 */

package org.apgrid.grpc.ng;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.List;

import org.apgrid.grpc.ng.info.*;
import org.gridforum.gridrpc.GrpcException;

class Communicator {
	Wire wire;
	Peer peer;
	Peer mySelf;
	int protoType;
	List encodeType;
	ByteArrayOutputStream baos = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
	
	boolean boolForceXDR = false;
	
	ByteArrayOutputStream testBo;
	
	/* protocol version */
	int versionMajor;
	int versionMinor;
	int versionPatch;
	
	/* definitions */
	public static final int PROTO_NONE = 0x00;
	public static final int PROTO_XML = 0x01;
	public static final int PROTO_BIN = 0x02;
	
	/**
	 * @param wire
	 */
	public Communicator(Wire wire) {
		this.wire = wire;
		this.peer = wire.getPeer();
		this.mySelf = wire.getMySelf();
		this.protoType = PROTO_NONE;
		this.boolForceXDR = false;
	}
	
	/**
	 * @param wire
	 */
	public Communicator(Wire wire, boolean forceXDR) {
		this.wire = wire;
		this.peer = wire.getPeer();
		this.mySelf = wire.getMySelf();
		this.protoType = PROTO_NONE;
		this.boolForceXDR = forceXDR;
	}
	
	/**
	 * @param remoteMachineInfo
	 * @throws GrpcException
	 */
	protected void initComm(RemoteMachineInfo remoteMachineInfo) throws GrpcException {
		if (remoteMachineInfo.get(
			RemoteMachineInfo.KEY_PROTOCOL).equals(RemoteMachineInfo.VAL_PROTO_XML)) {
			/* specified XML protocol */
			this.protoType = PROTO_XML;
		} else if (remoteMachineInfo.get(
			RemoteMachineInfo.KEY_PROTOCOL).equals(RemoteMachineInfo.VAL_PROTO_BIN)) {
			/* specified binary protocol */
			this.protoType = PROTO_BIN;
		} else {
			throw new 
				NgExecRemoteMethodException("Communicator: unknown type of Protocol.");
		}
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void finalComm() throws GrpcException {
		/* set protoType to NONE */
		this.protoType = PROTO_NONE;
	}
	
	/**
	 * @return
	 */
	protected Protocol getCommand() throws GrpcException {
		if (protoType == PROTO_XML) {
			String str;
			try {
				str = wire.receiveString();
				Protocol returnProtocol = Protocol.readProtocol(str);
				/* set version */
				returnProtocol.setVersion(
					versionMajor, versionMinor, versionPatch);
				return returnProtocol;
			} catch (GrpcException e) {
				throw new NgExecRemoteMethodException(e);
			}
		} else if (protoType == PROTO_BIN) {
			/* read header */
			byte[] buffer = new byte[32];
			if (wire != null) {
				wire.receiveBytes(buffer);
			}
			/* create InputStream */
			NgDataInputStream ngdi = getNgInputStream(buffer);
			/* create Protocol with header data */
			Protocol returnProtocol = Protocol.readProtocol(ngdi);
			returnProtocol.setHeader(buffer);
			/* close InputStream */
			ngdi.close();
			
			/* set version */
			returnProtocol.setVersion(
				versionMajor, versionMinor, versionPatch);
			
			return returnProtocol;
		} else {
			throw new 
				NgExecRemoteMethodException("Communicator: unknown type of Protocol.");
		}
	}
	
	/**
	 * @param prot
	 * @return
	 * @throws GrpcException
	 */
	void parseParam(Protocol received_prot, Protocol target_prot) throws GrpcException {
		/* read & parse parameter */
		received_prot.parseParam(wire, target_prot);
	}
	
	/**
	 * @param buffer
	 * @return
	 */
	private NgDataInputStream
			getNgInputStream(byte[] buffer) throws GrpcException {
		ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
		/* create XDRStream and return it */
		return new XDRInputStream(bi);
	}
	
	/**
	 * @param prot
	 */
	protected void putCommand(Protocol prot) throws GrpcException {
		if (protoType == PROTO_XML) {
			try {
				wire.sendString(prot.toString());
			} catch (GrpcException e) {
				throw new NgExecRemoteMethodException(e);
			}
		} else if (protoType == PROTO_BIN) {
			prot.setupParameter();
			prot.setupHeader();
			prot.sendBINDataToWire(wire);
		}
		if (wire != null) {
			wire.flush();
		}
	}
	
	/**
	 * @return
	 */
	private NgDataOutputStream getNgOutputStream() throws GrpcException {
		return getNgOutputStream(new ByteArrayOutputStream(NgGlobals.smallBufferSize));
	}
	
	/**
	 * @param baos
	 * @return
	 */
	private NgDataOutputStream getNgOutputStream(
		ByteArrayOutputStream baos) throws GrpcException {
		/* create XDRStream and return it */
		return new XDROutputStream(baos);
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
	public int getProtoType() {
		return protoType;
	}
	
	/**
	 * @return
	 */
	protected Wire getWire() {
		return wire;
	}
	
	/**
	 * @param mySelf
	 */
	public void setMySelf(Peer mySelf) {
		this.mySelf = mySelf;
	}
	
	/**
	 * @param peer
	 */
	public void setPeer(Peer peer) {
		this.peer = peer;
	}
	
	/**
	 * @param protoType
	 */
	public void setProtoType(int protoType) {
		this.protoType = protoType;
	}
	
	/**
	 * @param listEncodeInfo
	 */
	public void setEncodeType(List listEncodeInfo) {
		/* put encode information into List */
		this.encodeType = listEncodeInfo;
	}

	/**
	 * @return
	 */
	public List getEncodeTypes() {
		return encodeType;
	}
	
	/**
	 * @param major
	 * @param minor
	 * @param patch
	 */
	protected void setVersion(int major, int minor, int patch) {
		this.versionMajor = major;
		this.versionMinor = minor;
		this.versionPatch = patch;
	}
	
	/**
	 * @return
	 */
	protected int getVersionMajor() {
		return versionMajor;
	}
	
	/**
	 * @return
	 */
	protected int getVersionMinor() {
		return versionMinor;
	}
	
	/**
	 * @return
	 */
	protected int getVersionPatch() {
		return versionPatch;
	}
}
