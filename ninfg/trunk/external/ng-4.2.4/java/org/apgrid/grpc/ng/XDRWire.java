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
 * $RCSfile: XDRWire.java,v $ $Revision: 1.9 $ $Date: 2005/08/01 10:39:11 $
 */
package org.apgrid.grpc.ng;

import java.io.IOException;
import java.net.Socket;

import org.gridforum.gridrpc.GrpcException;

public class XDRWire extends Wire {
	private Socket socket;
	private XDRInputStream is;
	private XDROutputStream os;
	
	/**
	 * @param socket
	 * @throws GrpcException
	 */
	public XDRWire(Socket socket) throws GrpcException {
		this.socket = socket;
		try {
			/* disable timeout of socket */
			this.socket.setSoTimeout(0);
			
			/* get {In,Out}PutStream from Socket */
			this.is = new XDRInputStream(socket.getInputStream());
			this.os = new XDROutputStream(socket.getOutputStream());
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#getPeer()
	 */
	protected Peer getPeer() {
		return new Peer(socket.getInetAddress().getHostName(), socket.getPort());
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#getMySelf()
	 */
	protected Peer getMySelf() {
		return new Peer(socket.getLocalAddress().getHostName(), socket.getLocalPort());
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#close()
	 */
	protected void close() throws GrpcException {
		is.close();
		os.close();
		try {
			socket.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#receiveString()
	 */
	public String receiveString() throws GrpcException {
		return is.readString();
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#receiveBytes(byte[], int, int)
	 */
	public byte[] receiveBytes(byte[] buffer, int start, int length) throws GrpcException {
		is.readBytes(buffer, start, length);
		return buffer;
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#sendString(java.lang.String)
	 */
	protected void sendString(String string) throws GrpcException {
		os.writeString(string);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#sendBytes(byte[])
	 */
	public void sendBytes(byte[] buffer) throws GrpcException {
		os.writeBytes(buffer);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#sendBytes(byte[], int, int)
	 */
	public void sendBytes(byte[] buffer, int offset, int length) throws GrpcException {
		os.writeBytes(buffer, offset, length);
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.Wire#flush()
	 */
	protected void flush() throws GrpcException {
		try {
			os.flush();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
}
