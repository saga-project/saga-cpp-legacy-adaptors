/*
 * $RCSfile: NgInvokeServerManager.java,v $ $Revision: 1.7 $ $Date: 2008/02/23 03:02:07 $
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

import java.util.Map;
import java.util.HashMap;
import java.util.Vector;
import java.util.List;

import org.apgrid.grpc.ng.info.InvokeServerInfo;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;

/*
 * This class manage the NgInvokeServer.
 */
class NgInvokeServerManager {
	int lastID;           // ID of InvokeServer
	NgGrpcClient context; // Ninf-G Context 

	Map<String, NgInvokeServer> mapInvokeServer;
	List<NgInvokeServer> listInvokeServer;
	NgLog ngLog;

	/**
	 * @throws GrpcException
	 */
	NgInvokeServerManager(NgGrpcClient context) throws GrpcException {
		this.context = context;
		this.mapInvokeServer  = new HashMap<String, NgInvokeServer>();
		this.listInvokeServer = new Vector<NgInvokeServer>();
		this.ngLog = context.getNgLog();
		this.lastID = 0;
	}
	
	/**
	 * @param ngJob
	 * @return
	 * @throws GrpcException
	 */
	protected synchronized NgInvokeServer getInvokeServer(NgGrpcJob ngJob)
	 throws GrpcException {
		// get type of InvokeServer 
		String type = ngJob.getInvokeServerType();

		ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader() 
			+ getClass().getName()
			+ "#getInvokeServer : requested invokeServer for " 
			+ type);
		
		// get max_jobCount for the Invoke Server 
		NgGrpcClient client = ngJob.getClient();

		NgInformationManager infoMng = client.getNgInformationManager();

		InvokeServerInfo isInfo = infoMng.getInvokeServerInfo(type);

		int maxJobs = 0;
		if (isInfo != null) {
			maxJobs = isInfo.getMaxJobs();
		}

		// check if it's in map and it's not over limit 
		NgInvokeServer ngInvokeServer = mapInvokeServer.get(type);
		if ( isRegistered(ngInvokeServer) &&
			 withinBoundsOfMaxJobs(maxJobs, ngInvokeServer)) {
			// InvokeServer(ngInvokeServer) is already in map 
			ngInvokeServer.incrementRequestCount();
			return ngInvokeServer;
		}

		// create new InvokeServer 
		ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader() 
			+ getClass().getName()
			+ "#getInvokeServer : requested invokeServer is not in map. So create it.");

		NgInvokeServer newInvokeServer =
			new NgInvokeServer(this, ngJob.getClient(),
				ngJob.getInvokeServerType(), this.lastID, isInfo);

		this.lastID += 1; // increment lastID 
		
		// start NgInvokeServer Thread 
		_startInvokeServer(newInvokeServer);

		// register InvokeServer to map 
		mapInvokeServer.put(type, newInvokeServer);

		// it unregistered InvokeServer from map and add to list 
		if (ngInvokeServer != null) {
			listInvokeServer.add(ngInvokeServer);
		}

		newInvokeServer.incrementRequestCount();
		return newInvokeServer;
	}

	private boolean isRegistered(NgInvokeServer server) {
		if (server == null) return false;
		return server.isValid();
	}

	private boolean withinBoundsOfMaxJobs(int maxJobs, NgInvokeServer server) {
		return (maxJobs == 0) || (server.getRequestCount() < maxJobs);
	}

	private void _startInvokeServer(NgInvokeServer server) {
		//new Thread(server, "NgInvokeServer" + server.getType()).start();
		NotifyThreadManager notifyThread = new NotifyThreadManager();
		notifyThread.setExtModule(server);
		notifyThread.setNgLog(ngLog);
		notifyThread.startThread();
	}
	
	/**
	 * @param ngInvokeServer
	 * @throws GrpcException
	 */
	protected synchronized void unregister(NgInvokeServer target)
	 throws GrpcException {

		ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader()
			+ getClass().getName()
			+ "#unregister: unregister specified Invoke Server.");

		_removeFromMap(target);  // unregister InvokeServer from Map 
		_removeFromList(target); // unregister InvokeServer from List
	}

	private void _removeFromMap(NgInvokeServer target) {
		String targetKey = null;
		for (Map.Entry<String, NgInvokeServer> entry :
			 mapInvokeServer.entrySet()) {
			if (entry.getValue() == target) {
				targetKey = entry.getKey();
				break;
			}
		}
		if (targetKey != null) {
			mapInvokeServer.remove(targetKey);
		}
		/*
		if (mapInvokeServer.containsValue(target)) {
			String type = target.getType();
			mapInvokeServer.remove(type);
		}
		*/
	}

	private void _removeFromList(NgInvokeServer entry) {
		listInvokeServer.remove(entry);
	}

	/**
	 *
	 */
	protected synchronized void deactivate() {
		for (Map.Entry<String, NgInvokeServer> entry : mapInvokeServer.entrySet()) {
			NgInvokeServer is = entry.getValue();
			is.deactivate();
			mapInvokeServer.remove(is);
			listInvokeServer.remove(is);
		}
	}

	/**
	 * @throws GrpcException
	 */
	public synchronized void dispose() throws GrpcException {
		ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader() 
			+ "NgInvokeServerManager#dispose : stop all of Invoke Server.");

		// stop all of InvokeServer 
		String [] keys =
			(String [])mapInvokeServer.keySet().toArray(new String[0]);
		for (int i = 0; i < keys.length ; i++) {
			NgInvokeServer target = mapInvokeServer.get(keys[i]);
			try {
				ngLog.logDebug(CAT_NG_INTERNAL, context.logHeader() 
					+ getClass().getName() 
					+ "#dispose : exit Invoke Server[" 
					+ target.getID() + "]");

				target.exit();
			} catch (GrpcException e) {
				ngLog.logWarn(CAT_NG_INTERNAL, context.logHeader()
					+ getClass().getName() 
					+ "#dispose : failed to exit Invoke Server[" 
					+ target.getID() + "]");
				ngLog.logWarn(CAT_NG_INTERNAL, e.toString());
			}
		}
				
		while (listInvokeServer.size() > 0) {
			NgInvokeServer target = listInvokeServer.get(0);
			try {
				// exit Invoke Server
				ngLog.logDebug(CAT_NG_INTERNAL,
					context.logHeader()
					+ getClass().getName()
					+ "#dispose : exit Invoke Server[" 
					+ target.getID() + "]");
				// expect decrement listInvokeServer.size()
				target.exit(); 
			} catch (GrpcException e) {
				// something wrong was happend... 
				ngLog.logWarn(CAT_NG_INTERNAL,
					context.logHeader()
					+ getClass().getName() 
					+ "#dispose : failed to exit Invoke Server[" 
					+ target.getID() + "]");
				ngLog.logWarn(CAT_NG_INTERNAL, e.toString());
			}
		}
	}

	public String toString() {
		StringBuilder sb = new StringBuilder(getClass().getName() + "\n");
		sb.append("Map: " + mapInvokeServer + "\n");
		sb.append("List: " + listInvokeServer);
		return sb.toString();
	}

}
