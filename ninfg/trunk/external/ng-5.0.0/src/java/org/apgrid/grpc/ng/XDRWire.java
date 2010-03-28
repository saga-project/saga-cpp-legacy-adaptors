/*
 * $RCSfile: XDRWire.java,v $ $Revision: 1.7 $ $Date: 2008/01/22 07:03:03 $
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

import java.io.BufferedInputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.List;

import org.gridforum.gridrpc.GrpcException;

public class XDRWire implements Wire {

	private class XDRdummyCallback implements NgCallbackInterface {
		public void callback(List args) {
			// Do nothing.
		}
	}

	private Peer peer;
	private Peer myself;
	private Socket socket;
	private XDRInputStream is;
	private XDROutputStream os;
	private NgCallbackInterface receiveCallback;
	private NgLog commLog;

	/**
	 * @param socket
	 * @throws GrpcException
	 */
	public XDRWire(Socket socket) throws GrpcException {
		this.socket = socket;
		this.receiveCallback = new XDRdummyCallback();
		try {
			// disable timeout of socket 
			this.socket.setSoTimeout(0);
			
			// get {In,Out}PutStream from Socket 
			this.is =
				new XDRInputStream(
					new BufferedInputStream(socket.getInputStream()));
			this.os = new XDROutputStream(socket.getOutputStream());
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#getPeer()
	 */
	public Peer getPeer() {
		if (peer == null) {
			peer = new Peer(socket.getInetAddress().getHostName(),
					socket.getPort());
		}
		return peer;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#getMySelf()
	 */
	public Peer getMySelf() {
		if (myself == null) {
			myself = new Peer(socket.getLocalAddress().getHostName(),
					socket.getLocalPort());
		}
		return myself;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#close()
	 */
	public void close() throws GrpcException {
		try {
			is.close();
			os.close();
			socket.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#receiveString()
	 */
	@Deprecated
	public String receiveString() throws GrpcException {
		return is.readString();
	}

	public byte[] receiveBytes(byte[] buffer) throws GrpcException {
		return receiveBytes(buffer, 0, buffer.length);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#receiveBytes(byte[], int, int)
	 */
	public byte[] receiveBytes(byte[] buffer, int start, int length)
	 throws GrpcException {
		int nRead = 0;
		nRead = is.readBytes(buffer, start, length);
		if (nRead < 0)
			return null; // reached end of stream
		receiveCallback.callback(null);
		return buffer;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#sendString(java.lang.String)
	 */
	@Deprecated
	public void sendString(String string) throws GrpcException {
		os.writeString(string);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#sendBytes(byte[])
	 */
	public void sendBytes(byte[] data) throws GrpcException {
		os.writeBytes(data);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#sendBytes(byte[], int, int)
	 */
	public void sendBytes(byte[] data, int offset, int length)
	 throws GrpcException {
		os.writeBytes(data, offset, length);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#flush()
	 */
	public void flush() throws GrpcException {
		try {
			os.flush();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	public void setLogger(NgLog logger) {
		this.commLog = logger;
	}

	public void logCommLog(NgProtocolRequest prot, String msg, byte[] data,
	 int length) {
		if (this.commLog == null)
			return;
		this.commLog.logCommLog(prot.getName() + " Send " + msg , data, length);
	}

	public void logCommLog(Protocol prot, String msg, byte[] data, int length) {
		if (this.commLog == null)
			return;
		this.commLog.logCommLog(prot.getName() + " Recv " + msg, data, length);
	}

	public void setReceiveCallback(NgCallbackInterface callback) {
		this.receiveCallback = callback;
	}
}

