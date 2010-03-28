/**
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
 * $RCSfile: GrpcInfoService.java,v $ $Revision: 1.2 $ $Date: 2008/02/06 10:37:13 $
 */
package org.apgrid.ninf.ng5.grpcinfo.impl;

import java.rmi.RemoteException;

import javax.xml.soap.SOAPException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apgrid.ninf.ng5.grpcinfo.types.RescanRequestType;
import org.apgrid.ninf.ng5.grpcinfo.types.RescanResponseType;
import org.globus.wsrf.ResourceContext;
import org.globus.wsrf.impl.SingletonResourceHome;

public class GrpcInfoService {
	private static Log logger = LogFactory.getLog(GrpcInfoService.class.getName());

	public GrpcInfoService() {
		logger.trace("GrpcInfoService() called");
	}

	private void logCheck() {
		logger.trace("1 rescan() trace log test");
		logger.debug("2 rescan() debug log test");
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
