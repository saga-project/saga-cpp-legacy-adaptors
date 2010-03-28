/*
 * $RCSfile: ProtTransferArgumentReply.java,v $ $Revision: 1.9 $ $Date: 2007/09/26 04:14:08 $
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
package org.apgrid.grpc.ng.protocol;

import org.apgrid.grpc.ng.NgDataInputStream;
import org.apgrid.grpc.ng.Protocol;
import org.gridforum.gridrpc.GrpcException;

public final class ProtTransferArgumentReply
extends AbstractReceivableProtocol
implements Protocol {

	private final static String NAME = "TransferArgumentReply";
	private final static int KIND = 1;
	private final static int TYPE = 0x31;

	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public ProtTransferArgumentReply(NgDataInputStream ngdi)
	 throws GrpcException {
		super(KIND, TYPE, ngdi);
	}
	
	public String getName() { return NAME; }
	
}

