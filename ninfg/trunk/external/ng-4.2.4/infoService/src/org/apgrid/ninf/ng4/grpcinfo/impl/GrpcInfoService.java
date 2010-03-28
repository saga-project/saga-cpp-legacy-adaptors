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
 * $RCSfile: GrpcInfoService.java,v $ $Revision: 1.3 $ $Date: 2006/10/11 08:13:49 $
 */
package org.apgrid.ninf.ng4.grpcinfo.impl;

import java.rmi.RemoteException;

import javax.xml.soap.SOAPException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apgrid.ninf.ng4.grpcinfo.types.RescanRequestType;
import org.apgrid.ninf.ng4.grpcinfo.types.RescanResponseType;
import org.globus.wsrf.ResourceContext;
import org.globus.wsrf.impl.SingletonResourceHome;

public class GrpcInfoService {
	private static Log logger = LogFactory.getLog(GrpcInfoService.class.getName());

	public GrpcInfoService() {
		logger.trace("GrpcInfoService() called");
	}

	private void logCheck() {
		logger.trace("1 recan() trace log test");
		logger.debug("2 recan() debug log test");
		logger.info("3 rescan() info log test");
		logger.warn("4 rescan() warn log test");
		logger.error("5 rescan() error log test");
		logger.fatal("6 rescan() fatal log test");
	}

	public RescanResponseType rescan(RescanRequestType dummy) throws RemoteException {
		logger.trace("GrpcInfoService#rescan() ent");
		// logCheck();
		((GrpcInfoHome)ResourceContext.getResourceContext().getResourceHome()).rescan();
		final RescanResponseType response = new RescanResponseType();
		logger.trace("GrpcInfoService#rescan() fin");
		return response;
	}
}
