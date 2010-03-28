/*
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
 * $RCSfile: NgInvokeServerManager.java,v $ $Revision: 1.6 $ $Date: 2006/09/14 08:22:35 $
 */
package org.apgrid.grpc.ng;

import java.util.Enumeration;
import java.util.Properties;
import java.util.Vector;

import org.apgrid.grpc.ng.info.InvokeServerInfo;
import org.gridforum.gridrpc.GrpcException;

class NgInvokeServerManager {
	/* Ninf-G Context */
	NgGrpcClient context;
	/* map of InvokeServer */
	Properties propInvokeServer;
	/* list of InvokeServer */
	Vector listInvokeServer;
	/* Log */
	NgLog ngLog;
	/* ID of InvokeServer */
	int lastID;
	
	/**
	 * @throws GrpcException
	 * 
	 */
	NgInvokeServerManager(NgGrpcClient context) throws GrpcException {
		this.context = context;
		this.propInvokeServer = new Properties();
		this.listInvokeServer = new Vector();
		this.ngLog = context.getNgLog();
		this.lastID = 0;
	}
	
	/**
	 * @param ngJob
	 * @return
	 * @throws GrpcException
	 */
	protected synchronized NgInvokeServer getInvokeServer(NgGrpcJob ngJob) throws GrpcException {
		/* get type of InvokeServer */
		String invokeServerType = ngJob.getInvokeServerType();
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServerManager#getInvokeServer : requested invokeServer for " +
				invokeServerType);
		
		/* get max_jobCount for the Invoke Server */
		NgGrpcClient client = ngJob.getClient();
		NgInformationManager infoMng = client.getNgInformationManager();
		InvokeServerInfo isInfo = infoMng.getInvokeServerInfo(invokeServerType);
		int maxJobs = 0;
		if (isInfo != null) {
			maxJobs = isInfo.getMaxJobs();
		}
		
		/* check if it's in map and it's not over limit */
		NgInvokeServer ngInvokeServer =
			(NgInvokeServer) propInvokeServer.get(invokeServerType);
		if ((ngInvokeServer != null) && (ngInvokeServer.isValid()) &&
			((maxJobs == 0) || ngInvokeServer.getRequestCount() < maxJobs)) {
			/* InvokeServer is already in map */
			ngInvokeServer.incrementRequestCount();
			return ngInvokeServer;
		}
		
		/* create new InvokeServer */
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServerManager#getInvokeServer : requested invokeServer is not in map. So create it.");
		NgInvokeServer newInvokeServer = new NgInvokeServer(this,
				ngJob.getClient(), ngJob.getInvokeServerType(), this.lastID, isInfo);
		/* increment lastID */
		this.lastID += 1;
		
		/* start NgInvokeServer Thread */
		new Thread(newInvokeServer, "NgInvokeServer" + invokeServerType).start();
		
		/* register InvokeServer to map */
		propInvokeServer.put(invokeServerType, newInvokeServer);
		/* it unregisters InvokeServer from map ant add to list */
		if (ngInvokeServer != null) {
			listInvokeServer.add(ngInvokeServer);
		}
		
		newInvokeServer.incrementRequestCount();
		return newInvokeServer;
	}
	
	/**
	 * @param ngInvokeServer
	 * @throws GrpcException
	 */
	protected synchronized void unregisterInvokeServer(NgInvokeServer ngInvokeServer) throws GrpcException {
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServerManager#unregisterInvokeServer : unregister specified Invoke Server.");
		/* unregister InvokeServer from Map */
		Enumeration invokeServers = propInvokeServer.keys();
		
		/* check if specified InvokeServer is in Map */
		while (invokeServers.hasMoreElements()) {
			String targetKey = (String) invokeServers.nextElement();
			NgInvokeServer registeredInvokeServer =
				(NgInvokeServer) propInvokeServer.get(targetKey);
			
			if (registeredInvokeServer == ngInvokeServer) {
				/* target InvokeServer is in Map, remove it */
				propInvokeServer.remove(targetKey);
				break;
			}
		}
		
		/* check if specified InvokeServer is in list */
		for (int i = 0; i < listInvokeServer.size(); i++) {
			NgInvokeServer registeredInvokeServer =
				(NgInvokeServer) listInvokeServer.get(i);
			
			if (registeredInvokeServer == ngInvokeServer) {
				/* target InvokeServer is in list, remove it */
				listInvokeServer.remove(i);
				break;
			}
		}
	}
	
	/**
	 * @throws GrpcException
	 * 
	 */
	protected synchronized void dispose() throws GrpcException {
		ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				"NgInvokeServerManager#dispose : stop all of Invoke Server.");

		/* stop all of InvokeServer */
		Enumeration invokeServers = propInvokeServer.keys();
		/* map */
		while (invokeServers.hasMoreElements()) {
			NgInvokeServer ngInvokeServer =
				(NgInvokeServer) propInvokeServer.get(invokeServers.nextElement());
			try {
				/* exit Invoke Server */
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_DEBUG,
						context,
						"NgInvokeServerManager#dispose : exit Invoke Server[" + ngInvokeServer.getID() + "]");
				ngInvokeServer.exit();
			} catch (GrpcException e) {
				/* something wrong was happend... */
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_WARN,
						context,
						"NgInvokeServerManager#dispose : failed to exit Invoke Server[" + ngInvokeServer.getID() + "]");
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_WARN,
						e);
			}
		}
		
		/* list */
		for (int i = 0; i < listInvokeServer.size(); i++) {
			NgInvokeServer ngInvokeServer =	(NgInvokeServer) listInvokeServer.get(i);
			try {
				/* exit Invoke Server */
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_DEBUG,
						context,
						"NgInvokeServerManager#dispose : exit Invoke Server[" + ngInvokeServer.getID() + "]");
				ngInvokeServer.exit();
			} catch (GrpcException e) {
				/* something wrong was happend... */
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_WARN,
						context,
						"NgInvokeServerManager#dispose : failed to exit Invoke Server[" + ngInvokeServer.getID() + "]");
				ngLog.printLog(
						NgLog.LOGCATEGORY_NINFG_INTERNAL,
						NgLog.LOGLEVEL_WARN,
						e);
			}
		}
	}
}
