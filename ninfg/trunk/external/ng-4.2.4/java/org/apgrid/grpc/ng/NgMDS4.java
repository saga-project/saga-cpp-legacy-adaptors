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
 * $RCSfile: NgMDS4.java,v $ $Revision: 1.13 $ $Date: 2008/07/17 06:55:23 $
 */
package org.apgrid.grpc.ng;

import javax.xml.namespace.QName;
import org.apache.axis.message.MessageElement;
import org.apgrid.grpc.ng.info.MDSInfo;
import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.gridforum.gridrpc.GrpcException;

class NgMDS4 extends NgMDS {
	private static final String HOSTNAME = "hostName";
	private static final String MPIRUN_NUMCPU = "mpiCpus";
	private static final String CLASS_NAME = "className";
	private static final String EXE_PATH = "execPath";

	private static final String MDS4_DEFAULT_PROTOCOL = "https";
	private static final int MDS4_DEFAULT_PORT = 8443;
	private static final String MDS4_DEFAULT_PATH =
		"/wsrf/services/org/apgrid/ninf/ng4/grpcinfo/GrpcInfoService";

	/**
	 * @param manager
	 */
	NgMDS4(NgInformationManager manager) {
		this.context = manager.getContext();
		this.manager = manager;
	}

	/**
	 * @param hostName
	 * @return
	 */
	protected RemoteMachineInfo getRemoteMachineInfo(MDSInfo mdsInfo,
		String hostName, RemoteMachineInfo remoteMachineInfo) throws GrpcException {
		putInfoLog(
			"NgMDS4#getRemoteMachineInfo(): get server info from MDS4.");

		/* set condition of search */
		String xPath = "//*[namespace-uri()=" +
			"'http://ninf.apgrid.org/ng4/grpcinfo/types' and local-name()='grpcInfoSet']" +
			"/*[namespace-uri()='http://ninf.apgrid.org/ng4' and local-name()='hostInfo'" +
			" and @" + HOSTNAME + "='" +
			remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME) + "']";

		/* get client */
		String mds4ServiceUrlString = makeMDS4URLString(mdsInfo);
		MDS4QueryClient mdsqc =
			MDS4QueryClient.newMDS4QueryClient(mds4ServiceUrlString,
			(String) mdsInfo.get(MDSInfo.KEY_SUBJECT));

		/* set timeout for Client */
		if (mdsInfo.get(MDSInfo.KEY_CLIENT_TIMEOUT) != null) {
			mdsqc.setTimeout(Integer.parseInt(
				(String) mdsInfo.get(MDSInfo.KEY_CLIENT_TIMEOUT)));
		}
		// TODO: set server Timeout into MDS RequestQuery

		/* search */
		putInfoLog(
			"NgMDS4#getRemoteMachineInfo(): search on " + mds4ServiceUrlString + ".");
		putDebugLog(
			"NgMDS4#getRemoteMachineInfo(): XPath is " + xPath + ".");

		MessageElement[] results = mdsqc.query(xPath);
		if ((results != null) && (results.length > 0)) {
			putInfoLog(
				"NgMDS4#getRemoteMachineInfo(): found the information.");
			if (results.length > 1) {
				putErrorLog(
					"NgMDS4#getRemoteMachineInfo(): found multiple information, " +
					"the other than 1st is ignored");
			}
			return makeRemoteMachineInfo(remoteMachineInfo, results[0]);
		}

		return null;
	}

	/**
	 * @param hostName
	 * @param className
	 * @return
	 */
	protected RemoteClassPathInfo getRemoteClassPathInfo(MDSInfo mdsInfo,
		String hostName, String className) throws GrpcException {
		putInfoLog(
			"NgMDS4#getRemoteClassPathInfo(): get path info from MDS.");
		
		/* check if it's already in cache */
		if (manager.isClassPathInfoRegistered(hostName, className)) {
			putInfoLog(
				"NgMDS4#getRemoteClassPathInfo(): get path info from cache.");			
			return manager.getClassPathInfo(hostName, className);
		}

		/* get remoteMachineInfo for hostname */
		RemoteMachineInfo remoteMachineInfo = null;
		if (hostName != null) {
			remoteMachineInfo = manager.getRemoteMachineInfo(hostName);
		}

		/* set condition of search */
		String xPath = "//*[namespace-uri()=" +
			"'http://ninf.apgrid.org/ng4/grpcinfo/types' and local-name()='grpcInfoSet']" +
			"/*[namespace-uri()='http://ninf.apgrid.org/ng4' and local-name()='execInfo'" +
			" and @" + CLASS_NAME + "='" + className + "'" +
			" and @" + HOSTNAME + "='" +
			remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME) + "']";

		/* search */
		MessageElement[] results = null;

		/* get information about MDS service */
		if (mdsInfo == null) {
			throw new NgInitializeGrpcHandleException(
				"failed to find information about MDS");
		}

		/* get client */
		String mds4ServiceUrlString = makeMDS4URLString(mdsInfo);
		MDS4QueryClient mdsqc =
			MDS4QueryClient.newMDS4QueryClient(mds4ServiceUrlString,
			(String) mdsInfo.get(MDSInfo.KEY_SUBJECT));

		/* set timeout for Client */
		if (mdsInfo.get(MDSInfo.KEY_CLIENT_TIMEOUT) != null) {
			mdsqc.setTimeout(Integer.parseInt(
				(String) mdsInfo.get(MDSInfo.KEY_CLIENT_TIMEOUT)));
		}
		// TODO: set server Timeout into MDS RequestQuery

		/* search */
		putInfoLog(
			"NgMDS4#getRemoteClassPathInfo(): search on " + mds4ServiceUrlString + ".");
		putDebugLog(
			"NgMDS4#getRemoteClassPathInfo(): XPath is " + xPath + ".");

		results = mdsqc.query(xPath);
		if ((results != null) && (results.length > 0)) {
			putInfoLog(
				"NgMDS4#getRemoteClassPathInfo(): found the information.");
		}

		/* check result */
		if ((results == null) || (results.length < 1)) {
			/* no information was found */
			return null;
		}

		if (results.length > 1) {
			putErrorLog(
				"NgMDS4#getRemoteClassPathInfo(): found multiple information, " +
				"the other than 1st is ignored");
		}

		/* put Path information into cache */
		putInfoLog(
			"NgMDS4#getRemoteClassPathInfo(): put received info into manager.");
		RemoteClassPathInfo remoteClassPathInfo = new RemoteClassPathInfo(
			hostName, className, getRemoteClassPath(results[0]));
		manager.putRemoteClassPathInfo(
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
			className, remoteClassPathInfo);

		/* put Class information into cache */
		manager.putRemoteClassInfo(className, makeRemoteClassInfo(results[0]));
		
		return remoteClassPathInfo;
	}

	/**
	 * @param remoteMachineInfo
	 * @param result
	 * @return
	 * @throws GrpcException
	 */
	private RemoteMachineInfo makeRemoteMachineInfo(
		RemoteMachineInfo remoteMachineInfo, MessageElement result)
		throws GrpcException {
		//System.out.println(remoteMachineInfo.toString(0));
		//System.out.println(result);
		/* check if remoteMachineInfo does not exist */
		if (remoteMachineInfo == null) {
			/* create new RemoteMachineInfo */
			remoteMachineInfo = new RemoteMachineInfo();
		}

		/* get Attributes */
		String numCPUs = result.getAttributeValue(MPIRUN_NUMCPU);
		if (numCPUs != null) {
			remoteMachineInfo.put(
				RemoteMachineInfo.KEY_MPI_NCPUS,
				numCPUs);						
		}
		return remoteMachineInfo;	
	}

	/**
	 * @param result
	 * @return
	 * @throws GrpcException
	 */
	private RemoteClassInfo makeRemoteClassInfo(MessageElement result)
		throws GrpcException {
		RemoteClassInfo remoteClassInfo = null;
		String remoteClassInfoXml = result.getChildElement(new QName("", "class")).toString();
		if (remoteClassInfoXml != null) {
			/* make RemoteClassInfo from XML string */
			remoteClassInfo = RemoteClassInfo.readClassInfo(remoteClassInfoXml);
		}
		return remoteClassInfo;
	}

	/**
	 * @param result
	 * @return
	 * @throws GrpcException
	 */
	private String getRemoteClassPath(MessageElement result) throws GrpcException {
		//System.out.println(result);
		String path = result.getAttributeValue(EXE_PATH);
		//System.out.println("path == " + path);
		return path;
	}
	
	/**
	 * @param mdsInfo
	 * @return
	 */
	private String makeMDS4URLString(MDSInfo mdsInfo) throws GrpcException {
		/* check validation of MDSInfo */
		String hostname = (String) mdsInfo.get(MDSInfo.KEY_HOSTNAME);
		if (hostname == null) {
			throw new NgException("NgMDS4#makeMDS4URLString: invalid MDSInfo.");
		}
		
		/* create URL String */
		StringBuffer sb = new StringBuffer();
		/* protocol */
		String protocol = (String) mdsInfo.get(MDSInfo.KEY_PROTOCOL);
		if (protocol == null) {
			sb.append(MDS4_DEFAULT_PROTOCOL);
		} else {
			sb.append(protocol);
		}
		sb.append("://");
		
		/* hostname */
		sb.append(hostname);
		sb.append(":");
		
		/* port */
		int port = Integer.parseInt((String) mdsInfo.get(MDSInfo.KEY_PORT));
		if (port == 0) {
			port = MDS4_DEFAULT_PORT;
		}
		sb.append(port);
		
		/* path */
		String path = (String) mdsInfo.get(MDSInfo.KEY_PATH);
		if (path == null) {
			sb.append(MDS4_DEFAULT_PATH);
		} else {
			sb.append(path);
		}
		
		return sb.toString();
	}
}
