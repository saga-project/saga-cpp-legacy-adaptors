/*
 * $RCSfile: Protocol.java,v $ $Revision: 1.11 $ $Date: 2007/09/26 04:14:07 $
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

/*
 * Interface about Ninf-G Receivable Protocol(Reply & Notify)
 */
public interface Protocol {
	public static final int KEEP_CONNECTION = 0;
	public static final int DISCONNECT      = 1;

	/**
	 * Return the protocol type.
	 */
	public int getType();

	/**
	 * Return the protocol name.
	 */
	public String getName();

	/**
	 * Return the received protocol result.
	 */
	public int getResult();

	/**
	 * Parse parameter from wire.
	 * 
	 * @param wire
	 * @param prot
	 * @throws GrpcException
	 */
	public void parseParam(Wire wire, NgProtocolRequest prot)
	 throws GrpcException;

	/**
	 * Set Protocol version
	 */ 
	public void setVersion(Version version);

}
