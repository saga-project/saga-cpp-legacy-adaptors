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
 * $RCSfile: Wire.java,v $ $Revision: 1.9 $ $Date: 2004/12/15 08:05:19 $
 */
package org.apgrid.grpc.ng;

import org.gridforum.gridrpc.GrpcException;

public abstract class Wire {
	/**
	 * @return
	 */
	abstract protected Peer getPeer();

	/**
	 * @return
	 */
	abstract protected Peer getMySelf();

	/**
	 * @throws GrpcException
	 */
	abstract protected void close() throws GrpcException;

	/**
	 * @throws GrpcException
	 * @return
	 */
	abstract public String receiveString() throws GrpcException;

	/**
	 * @param buffer
	 * @param start
	 * @param length
	 * @return
	 * @throws GrpcException
	 */
	abstract public byte[] receiveBytes(byte[] buffer, int start, int length) throws GrpcException;
	
	/**
	 * @param buffer
	 * @return
	 * @throws GrpcException
	 */
	public byte[] receiveBytes(byte[] buffer) throws GrpcException {
		return receiveBytes(buffer, 0, buffer.length);
	}

	/**
	 * @param string
	 * @throws GrpcException
	 */
	abstract protected void sendString(String string) throws GrpcException;

	/**
	 * @param buffer
	 * @throws GrpcException
	 */
	public abstract void sendBytes(byte[] buffer) throws GrpcException;
	
	/**
	 * @param buffer
	 * @param offset
	 * @param length
	 * @throws GrpcException
	 */
	public abstract void sendBytes(byte[] buffer, int offset, int length) throws GrpcException;
	
	/**
	 * @throws GrpcException
	 */
	abstract protected void flush() throws GrpcException;
}
