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
 * $RCSfile: NgMDS.java,v $ $Revision: 1.38 $ $Date: 2006/01/11 10:06:23 $
 */
package org.apgrid.grpc.ng;

import org.apgrid.grpc.ng.info.MDSInfo;
import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.gridforum.gridrpc.GrpcException;

class NgMDS {
	NgGrpcClient context;
	NgInformationManager manager;
	private NgMDS2 ngMDS2 = null;
	private NgMDS4 ngMDS4 = null;
	
	/**
	 * 
	 */
	NgMDS() {
	}
	
	/**
	 * @param manager
	 */
	NgMDS(NgInformationManager manager) throws GrpcException {
		this.context = manager.getContext();
		this.manager = manager;
		/* create modules for MDS2, MDS4 */
		this.ngMDS2 = new NgMDS2(manager);
		this.ngMDS4 = new NgMDS4(manager);
	}

	/**
	 * @param hostName
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfo(String hostName,
		RemoteMachineInfo remoteMachineInfo) throws GrpcException {
		putInfoLog(
			"NgMDS#getRemoteMachineInfo(): get server info from MDS.");

		/* variables for list */
		MDSInfo propMDSServerInfo;
		int mdsServerIndex = 0;
		
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_HOSTNAME) != null) {
			/* get specified MDSInfo ( mds_hostname ) */
			propMDSServerInfo =
				(MDSInfo) manager.getMDSInfoByHandleString(
					MDSInfo.makeHandleString(
						(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_HOSTNAME),
						null));
		} else if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_TAG) != null) {
			/* get specified MDSInfo ( mds_tag ) */
			propMDSServerInfo =
				(MDSInfo) manager.getMDSInfoByTag(
					(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_TAG));
		} else {
			/* get first of MDS2ServerInfo */
			propMDSServerInfo = (MDSInfo) manager.getMDSInfoByIndex(mdsServerIndex);
			if (propMDSServerInfo == null) {
				throw new NgInitializeGrpcHandleException(
					"no MDSInfo is available.");
			}
			mdsServerIndex++;
		}
		
		/* search RemoteMachineInfo */
		RemoteMachineInfo targetRMInfo = null;
		while (true) {
			/* lock the MDS */
			propMDSServerInfo.lockMDS();
			
			try {
				String type = (String) propMDSServerInfo.get(MDSInfo.KEY_TYPE);
				if (type.equals(MDSInfo.VAL_TYPE_MDS4)) {
					/* search by MDS4 */
					targetRMInfo =
						ngMDS4.getRemoteMachineInfo(propMDSServerInfo, hostName, remoteMachineInfo);
				} else {
					/* search by MDS2 */
					targetRMInfo =
						ngMDS2.getRemoteMachineInfo(propMDSServerInfo, hostName, remoteMachineInfo);
				}
			} catch (GrpcException e) {
				/* something wrong was happend */
				putWarnLog("NgMDS#getRemoteMachineInfo(): couldn't search RemoveMachineInfo, try to next MDS server.");
				putWarnLog("NgMDS#getRemoteMachineInfo(): " + e);
			} finally {
				/* unlock the MDS */
				propMDSServerInfo.unlockMDS();
			}

			/* check result */
			if (targetRMInfo != null) {
				/* found it! */
				break;
			}
			
			/* get next MDSInfo */
			if (mdsServerIndex >= manager.getMDSInfoCount()) {
				break;
			}
			propMDSServerInfo = (MDSInfo) manager.getMDSInfoByIndex(mdsServerIndex);
			mdsServerIndex++;
		}

		return targetRMInfo;
	}

	/**
	 * @param hostName
	 * @param className
	 * @return
	 */
	protected RemoteClassPathInfo getRemoteClassPathInfo(
		String hostName, String className) throws GrpcException {
		putInfoLog(
		"NgMDS#getRemoteClassPathInfo(): get path info from MDS.");
	
		/* check if it's already in cache */
		if (manager.isClassPathInfoRegistered(hostName, className)) {
			putInfoLog(
				"NgMDS#getRemoteClassPathInfo(): get path info from cache.");			
			return manager.getClassPathInfo(hostName, className);
		}

		/* get remoteMachineInfo for hostname */
		RemoteMachineInfo remoteMachineInfo = null;
		if (hostName != null) {
			remoteMachineInfo = manager.getRemoteMachineInfo(hostName);
		}

		/* get information about MDS server */
		int mdsServerIndex = 0;
		MDSInfo propMDSServerInfo = null;
		if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_HOSTNAME) != null) {
			/* get specified MDSInfo ( mds_hostname ) */
			propMDSServerInfo =
				(MDSInfo) manager.getMDSInfoByHandleString(
					MDSInfo.makeHandleString(
						(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_HOSTNAME),
						null));
		} else if (remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_TAG) != null) {
			/* get specified MDSInfo ( mds_tag ) */
			propMDSServerInfo =
				(MDSInfo) manager.getMDSInfoByTag(
					(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_MDS_TAG));
		} else {
			propMDSServerInfo = (MDSInfo) manager.getMDSInfoByIndex(mdsServerIndex);
			if (propMDSServerInfo == null) {
				throw new NgInitializeGrpcHandleException(
					"no MDSInfo is available.");
			}
			mdsServerIndex++;
		}

		/* search RemoteClassPathInfo */
		RemoteClassPathInfo targetRCPInfo = null;
		while (true) {
			/* get information about MDS server */
			if (propMDSServerInfo == null) {
				throw new NgInitializeGrpcHandleException(
					"failed to find information about MDS");
			}

			/* lock the MDS */
			propMDSServerInfo.lockMDS();

			try {
				String type = (String) propMDSServerInfo.get(MDSInfo.KEY_TYPE);
				if (type.equals(MDSInfo.VAL_TYPE_MDS4)) {
					/* search by MDS4 */
					targetRCPInfo =
						ngMDS4.getRemoteClassPathInfo(propMDSServerInfo, hostName, className);
				} else {
					/* search by MDS2 */
					targetRCPInfo =
						ngMDS2.getRemoteClassPathInfo(propMDSServerInfo, hostName, className);
				}
			} catch (GrpcException e) {
				/* something wrong was happend */
				putWarnLog("NgMDS#getRemoteClassPathInfo(): couldn't search RemoteClassPathInfo, try to next MDS server.");
				putWarnLog("NgMDS#getRemoteClassPathInfo(): " + e);
			} finally {
				/* unlock the MDS */
				propMDSServerInfo.unlockMDS();
			}

			/* check result */
			if (targetRCPInfo != null) {
				/* found it! */
				break;
			}
			
			/* get next MDS2ServerInfo */
			if (mdsServerIndex >= manager.getMDSInfoCount()) {
				break;
			}
			propMDSServerInfo = (MDSInfo) manager.getMDSInfoByIndex(mdsServerIndex);
			mdsServerIndex++;
		}
	
		return targetRCPInfo;
	}

	/**
	 * @param hostName
	 * @param className
	 * @return
	 * @throws GrpcException
	 */
	protected RemoteClassInfo getRemoteClassInfo(String hostName,
		String className) throws GrpcException {
		RemoteClassPathInfo remoteClassPathInfo =
			getRemoteClassPathInfo(hostName, className);
		if (remoteClassPathInfo != null) {
			return manager.getRemoteClassInfo(className);
		}
		/* couldn't find the information */
		return null;
	}
	
	/**
	 * @param message
	 * @throws GrpcException
	 */
	protected void putErrorLog(String message) throws GrpcException {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_ERROR,
				context,
				message);
		} 
	}

	/**
	 * @param message
	 * @throws GrpcException
	 */
	protected void putWarnLog(String message) throws GrpcException {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_WARN,
				context,
				message);
		} 
	}

	/**
	 * @param message
	 * @throws GrpcException
	 */
	protected void putInfoLog(String message) throws GrpcException {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				context,
				message);
		} 
	}

	/**
	 * @param message
	 * @throws GrpcException
	 */
	protected void putDebugLog(String message) throws GrpcException {
		NgLog ngLog = context.getNgLog();
		if (ngLog != null) {
			ngLog.printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_DEBUG,
				context,
				message);
		} 
	}
}
