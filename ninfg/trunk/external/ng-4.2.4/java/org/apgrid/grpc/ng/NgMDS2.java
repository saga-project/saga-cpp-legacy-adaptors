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
 * $RCSfile: NgMDS2.java,v $ $Revision: 1.7 $ $Date: 2006/01/30 05:11:53 $
 */
package org.apgrid.grpc.ng;

import java.util.Enumeration;
import java.util.Hashtable;

import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.DirContext;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;
import javax.naming.ldap.InitialLdapContext;

import org.apgrid.grpc.ng.info.MDSInfo;
import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.gridforum.gridrpc.GrpcException;

class NgMDS2 extends NgMDS {
	private static final String MPIRUN_COMMAND = "GridRPC-MpirunCommand";
	private static final String MPIRUN_NUMCPU = "GridRPC-MpirunNoOfCPUs";
	private static final String EXE_PATH = "GridRPC-Path";
	private static final String EXE_CLASSINFO = "GridRPC-Stub";
	
	private static final int MDS2_DEFAULT_PORT = 2135;
	
	/**
	 * @param manager
	 */
	NgMDS2(NgInformationManager manager) {
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
			"NgMDS2#getRemoteMachineInfo(): get server info from MDS2.");

		/* set condition of search */
		String filter =
			"(&(Mds-Host-hn=" +
			remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME) +
			")(objectClass=GridRPC))";
		String[] targetAttrs = new String[]{MPIRUN_COMMAND, MPIRUN_NUMCPU};
		SearchControls constraints = new SearchControls();
		constraints.setReturningAttributes(targetAttrs);
		constraints.setSearchScope(SearchControls.SUBTREE_SCOPE);
		/* set timeout for Client */
		constraints.setTimeLimit(
			Integer.parseInt((String) mdsInfo.get(
			MDSInfo.KEY_CLIENT_TIMEOUT)) * 1000);
		// TODO: set server Timeout into MDS RequestQuery

		String mdsHostName = (String) mdsInfo.get(MDSInfo.KEY_HOSTNAME);
		
		/* get DirContext */
		int port = Integer.parseInt((String) mdsInfo.get(MDSInfo.KEY_PORT));
		if (port == 0) {
			port = MDS2_DEFAULT_PORT;
		}
		DirContext ctx = getDirContext(mdsHostName, port);

		try {
			/* search */
			String baseDNstr = "Mds-Vo-name=" + 
				(String) mdsInfo.get(MDSInfo.KEY_VONAME) + ",o=grid";
			putInfoLog(
				"NgMDS2#getRemoteMachineInfo(): search on " + mdsHostName + ".");
			putDebugLog(
				"NgMDS2#getRemoteMachineInfo(): filter is " + filter + ".");
			putDebugLog(
				"NgMDS2#getRemoteMachineInfo(): baseDN is " + baseDNstr + ".");

			NamingEnumeration results = ctx.search(baseDNstr, filter, constraints);
			if (results.hasMore()) {
				putInfoLog("NgMDS2#getRemoteMachineInfo(): found the information.");
				/* set MDS server for this machine */
				remoteMachineInfo.put(
					RemoteMachineInfo.KEY_MDS_HOSTNAME, mdsHostName);
				return makeRemoteMachineInfo(
					remoteMachineInfo, (SearchResult)results.next());
			}
		} catch (NamingException e) {
			throw new NgInitializeGrpcHandleException(e);
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
			"NgMDS2#getRemoteClassPathInfo(): get path info from MDS2.");
		
		/* check if it's already in cache */
		if (manager.isClassPathInfoRegistered(hostName, className)) {
			putInfoLog(
				"NgMDS2#getRemoteClassPathInfo(): get path info from cache.");			
			return manager.getClassPathInfo(hostName, className);
		}

		/* get remoteMachineInfo for hostname */
		RemoteMachineInfo remoteMachineInfo = null;
		if (hostName != null) {
			remoteMachineInfo = manager.getRemoteMachineInfo(hostName);
		}
		/* get DN for host */
		if (remoteMachineInfo.getHostDN() == null) {
			remoteMachineInfo =
				getRemoteMachineInfo(mdsInfo, hostName, remoteMachineInfo);
			if (remoteMachineInfo == null) {
				putErrorLog(
					"NgMDS2#getRemoteClassPathInfo(): can't get RemoteMachineInfo.");
				return null;
			}
			manager.putRemoteMachineInfo(
				(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
				(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_TAG),
				remoteMachineInfo);
		}

		/* set condition of search */
		String filter = "(&(GridRPC-Funcname=" + className + "))";
		String[] targetAttrs = new String[]{EXE_PATH, EXE_CLASSINFO};
		SearchControls constraints = new SearchControls();
		constraints.setReturningAttributes(targetAttrs);
		constraints.setSearchScope(SearchControls.SUBTREE_SCOPE);
		
		/* set timeout for Client */
		constraints.setTimeLimit(
			Integer.parseInt((String) mdsInfo.get(
			MDSInfo.KEY_CLIENT_TIMEOUT)) * 1000);
		// TODO: set server Timeout into MDS RequestQuery

		/* search */
		int port = Integer.parseInt((String) mdsInfo.get(MDSInfo.KEY_PORT));
		if (port == 0) {
			port = MDS2_DEFAULT_PORT;
		}
		String hostDN = remoteMachineInfo.getHostDN();
		SearchResult sr = null;
		while (true) {
			/* get information about MDS server */
			if (mdsInfo == null) {
				throw new NgInitializeGrpcHandleException(
					"failed to find information about MDS");
			}

			/* get DirContext */
			String mdsHostName = (String) mdsInfo.get(MDSInfo.KEY_HOSTNAME);
			DirContext ctx = getDirContext(mdsHostName,	port);

			try {
				/* search */
				String baseDNstr = hostDN + ",Mds-Vo-name=" + 
					(String) mdsInfo.get(MDSInfo.KEY_VONAME) + ",o=grid";
				putInfoLog(
					"NgMDS2#getRemoteClassPathInfo(): search on " + mdsHostName + ".");
				putDebugLog(
					"NgMDS2#getRemoteClassPathInfo(): filter is " + filter + ".");
				putDebugLog(
					"NgMDS2#getRemoteClassPathInfo(): baseDN is " +
					baseDNstr + ".");

				NamingEnumeration results;
				results = ctx.search(baseDNstr,	filter, constraints);
				if (results.hasMore()) {
					putInfoLog(
						"NgMDS2#getRemoteClassPathInfo(): found the information.");
					/* get SearchResult */
					sr = (SearchResult) results.next();
					break;
				}
			} catch (NamingException e) {
				throw new NgInitializeGrpcHandleException(e);
			}
		}
		
		/* check SearchResult */
		if (sr == null) {
			/* no information was found */
			return null;
		}
		
		/* put Path information into cache */
		putInfoLog(
			"NgMDS2#getRemoteClassPathInfo(): put received info into manager.");
		RemoteClassPathInfo remoteClassPathInfo = new RemoteClassPathInfo(
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
			className, getRemoteClassPath(sr));
		manager.putRemoteClassPathInfo(
			(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
			className, remoteClassPathInfo);

		/* put Class information into cache */
		manager.putRemoteClassInfo(className, makeRemoteClassInfo(sr));
		
		return remoteClassPathInfo;
	}

	/**
	 * @return
	 */
	private DirContext getDirContext(String mdsHostName, int port)
		throws GrpcException {
		Hashtable env = new Hashtable();
		int ldapVersion = 3;
		
		/* LDAP version */
		env.put("java.naming.ldap.version", String.valueOf(ldapVersion));
		/* CTX Factory */
		env.put(Context.INITIAL_CONTEXT_FACTORY,
					"com.sun.jndi.ldap.LdapCtxFactory");
		/* server */
		env.put(Context.PROVIDER_URL, "ldap://" + mdsHostName + ":" + port);
		
		/* Authentication */
		env.put(Context.SECURITY_AUTHENTICATION, "none");
		
		try {
			return new InitialLdapContext(env, null);
		} catch (NamingException e) {
			throw new NgInitializeGrpcHandleException(e);
		}
	}
	
	/**
	 * @param remoteMachineInfo
	 * @param sr
	 * @return
	 * @throws GrpcException
	 */
	private RemoteMachineInfo makeRemoteMachineInfo(
		RemoteMachineInfo remoteMachineInfo, SearchResult sr)
		throws GrpcException {
		/* check if remoteMachineInfo does not exist */
		if (remoteMachineInfo == null) {
			/* create new RemoteMachineInfo */
			remoteMachineInfo = new RemoteMachineInfo();
		}

		try {
			/* set baseDN */
			remoteMachineInfo.setHostDN(sr.getName());
				
			/* get Attributes */
			Attributes attributes = sr.getAttributes();
			NamingEnumeration attributeValues = attributes.getAll();
			while (attributeValues.hasMore()) {
				Attribute attr = (Attribute)attributeValues.next();
				String attribute = attr.getID();
				if (attribute.equals(MPIRUN_COMMAND)) {
					/* put error message */
					System.err.println("syntax error: Obsolete syntax \"" +
						MPIRUN_COMMAND + "\"." +
						"Ignoring this setting continue.");
					Enumeration val = attr.getAll();
					remoteMachineInfo.put(
						RemoteMachineInfo.KEY_MPI_RUNCOMMAND,
						(String) val.nextElement());
				} else if (attribute.equals(MPIRUN_NUMCPU)) {
					Enumeration val = attr.getAll();
					String NumCPUs = (String) val.nextElement();
					remoteMachineInfo.put(
						RemoteMachineInfo.KEY_MPI_NCPUS,
						NumCPUs);						
				}
			}
		} catch (NamingException e) {
			throw new NgInitializeGrpcHandleException(e);
		}
		return remoteMachineInfo;	
	}

	/**
	 * @param sr
	 * @return
	 * @throws GrpcException
	 */
	private RemoteClassInfo makeRemoteClassInfo(SearchResult sr)
		throws GrpcException {
		String remoteClassInfo = null;
		try {
			Attributes attrs = sr.getAttributes();
			for (NamingEnumeration ne = attrs.getAll(); ne.hasMoreElements(); ) {
				Attribute attr = (Attribute) ne.next();
				String attribute = attr.getID();
				if (attribute.equals(EXE_CLASSINFO)) {
					Enumeration val = attr.getAll();
					remoteClassInfo = (String)val.nextElement();
				}
			}
		} catch (NamingException e) {
			throw new NgInitializeGrpcHandleException(e);
		}
		
		/* make RemoteClassInfo from XML string */
		return RemoteClassInfo.readClassInfo(remoteClassInfo);
	}

	/**
	 * @param sr
	 * @return
	 * @throws GrpcException
	 */
	private String getRemoteClassPath(SearchResult sr) throws GrpcException {
		try {
			Attributes attrs = sr.getAttributes();
			for (NamingEnumeration ne = attrs.getAll(); ne.hasMoreElements(); ) {
				Attribute attr = (Attribute) ne.next();
				String attribute = attr.getID();
				if (attribute.equals(EXE_PATH)) {
					Enumeration val = attr.getAll();
					return (String) val.nextElement();
				}
			}
		} catch (NamingException e) {
			throw new NgInitializeGrpcHandleException(e);
		}
		return null;
	}
}
