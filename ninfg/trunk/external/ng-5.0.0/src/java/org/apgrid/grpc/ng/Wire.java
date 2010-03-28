/*
 * $RCSfile: Wire.java,v $ $Revision: 1.6 $ $Date: 2008/03/28 03:25:55 $
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

import org.gridforum.gridrpc.GrpcException;

public interface Wire {

	/**
	 * Return the Peer Information
	 */
	public Peer getPeer();

	/**
	 * Return the Self Information
	 */
	public Peer getMySelf();

	/**
	 * @throws GrpcException
	 */
	public void close() throws GrpcException;

	/**
	 * @throws GrpcException
	 * @return
	 */
	@Deprecated
	public String receiveString() throws GrpcException;

	/**
	 * @param buffer
	 * @param start
	 * @param length
	 * @return
	 * @throws GrpcException
	 */
	public byte[] receiveBytes(byte[] buffer, int start, int length)
	 throws GrpcException;
	
	/**
	 * @param buffer
	 * @return
	 * @throws GrpcException
	 */
	public byte[] receiveBytes(byte[] buffer) throws GrpcException;

	/**
	 * @param string
	 * @throws GrpcException
	 * nobody call this method 
	 */
	@Deprecated
	public void sendString(String string) throws GrpcException;

	/**
	 * @param buffer
	 * @throws GrpcException
	 */
	public void sendBytes(byte[] buffer) throws GrpcException;
	
	/**
	 * @param buffer
	 * @param offset
	 * @param length
	 * @throws GrpcException
	 */
	public void sendBytes(byte[] buffer, int offset, int length)
	 throws GrpcException;
	
	/**
	 * @throws GrpcException
	 */
	public void flush() throws GrpcException;

	// newly added
	public void setLogger(NgLog logger);
	public void logCommLog(NgProtocolRequest prot, String msg, byte[] data, int length);
	public void logCommLog(Protocol prot, String msg, byte[] data, int length);
	public void setReceiveCallback(NgCallbackInterface callback);
}
