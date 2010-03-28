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
 * $RCSfile: LocalLDIFFile.java,v $ $Revision: 1.26 $ $Date: 2006/01/25 04:55:00 $
 */
package org.apgrid.grpc.ng;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.Properties;
import java.util.StringTokenizer;

import org.apgrid.grpc.ng.info.RemoteClassInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.globus.util.Base64;
import org.gridforum.gridrpc.GrpcException;

class LocalLDIFFile
{
	/* tags in LocalLdif file */
	static final String HOST_INFO = "HOST_INFO";
	static final String FUNCTION_INFO = "FUNCTION_INFO";

	static final String GRIDRPC_HOSTNAME = "GridRPC-Hostname:";
	static final String GRIDRPC_MPICOMMAND = "GridRPC-MpirunCommand:";
	static final String GRIDRPC_MPINOOFCPU = "GridRPC-MpirunNoOfCPUs:";

	static final String GRIDRPC_FUNCNAME = "GridRPC-Funcname:";
	static final String GRIDRPC_MODULE = "GridRPC-Module:";
	static final String GRIDRPC_ENTRY = "GridRPC-Entry:";
	static final String GRIDRPC_PATH = "GridRPC-Path:";
	static final String GRIDRPC_STUB = "GridRPC-Stub::";

	/**
	 * @param files
	 * @param manager
	 * @throws GrpcException
	 */
	protected static void parseFile(List files, NgInformationManager manager)
		throws GrpcException {
		for (int i = 0; i < files.size(); i++) {
			parseFile((String) files.get(i), manager);
		}
	}
	
	/**
	 * @param file
	 * @param manager
	 * @throws GrpcException
	 */
	private static void parseFile(String file, NgInformationManager manager)
		throws GrpcException {
		try {
			FileReader fr = new FileReader(file);
			BufferedReader br = new BufferedReader(fr);
			String recvBuffer = null;

			while ((recvBuffer = br.readLine()) != null) {
				String trimBuffer = recvBuffer.trim();
				if (trimBuffer.equals(HOST_INFO)) {
					/* information for server(gatekeeper) */
					readHostInfo(br, manager);
				} else if (trimBuffer.equals(FUNCTION_INFO)) {
					/* information for function(module/entry) */
					readFuncInfo(br, manager);
				} else {
					/* unrecognized information, ignored */
				}
			}

			/* close file */
			fr.close();
		} catch (IOException e) {
			/* something wrong was happened */
			throw new NgIOException("can't read LocalLDIF file : " + file, e);
		}
	}

	/**
	 * @param br
	 * @param manager
	 * @throws GrpcException
	 */
	static void readHostInfo(BufferedReader br, NgInformationManager manager)
		throws GrpcException {
		Properties propRemoteMachineInfo = new Properties();
		String recvBuffer = null;
		boolean defined_cpus = false;
	
		try {
			while (true) {
				recvBuffer = br.readLine();
				/* check if the end of a entry */
				if (checkTheEndOfEntry(recvBuffer) == true)
					break;
		
				StringTokenizer st = new StringTokenizer(recvBuffer, " \t");
				String tag = st.nextToken();
				if (tag.equals(GRIDRPC_HOSTNAME)) {
					if (propRemoteMachineInfo.get(
						RemoteMachineInfo.KEY_HOSTNAME) != null) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_HOSTNAME + "is already defined.");
					}
					propRemoteMachineInfo.put(
						RemoteMachineInfo.KEY_HOSTNAME,
						st.nextToken());
				} else if (tag.equals(GRIDRPC_MPICOMMAND)) {
					/* put error message */
					NgLog log = manager.getContext().getNgLog();
					String logMessage = 
						"LocalLDIFFile#readHostInfo(): " +
						"Obsolete syntax \"" + GRIDRPC_MPICOMMAND + "\". " +
						"Ignoring this setting continue.";
					if (log != null) {
						log.printLog(
								NgLog.LOGCATEGORY_NINFG_INTERNAL,
								NgLog.LOGLEVEL_WARN, 
								manager.getContext(),
								logMessage);
					} else {
						/* NgLog is created after reading config files, */
						/* so it's maybe null at this time */
						System.err.println (logMessage);
					}
				} else if (tag.equals(GRIDRPC_MPINOOFCPU)) {
					if (defined_cpus == true) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_MPINOOFCPU + "is already defined.");
					}
					try {
						Properties propNumOfCPUs = new Properties();
						propNumOfCPUs.put("", st.nextToken());
						propRemoteMachineInfo.put(
							RemoteMachineInfo.KEY_MPI_NCPUS,
							propNumOfCPUs);
					} catch (NumberFormatException e) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_MPINOOFCPU + " doesn't have valid value.", e);
					}
					defined_cpus = true;
				} else {
					/* unrecognized parameter */
					throw new NgInitializeGrpcClientException(
						"Detected unrecognized parameter");
				}
			}
		} catch (IOException e) {
			/* something wrong was happened */
			throw new NgIOException(e);
		}
	
		/* check if data was valid */
		if (propRemoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME) == null) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_HOSTNAME + " is not defined.");
		}
		if (defined_cpus == false) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_MPINOOFCPU + " is not defined.");
		}
		
		/* check if the specified RemoteMachineInfo already registered */
		RemoteMachineInfo remoteMachineInfo = null;
		if (manager.isRemoteMachineInfoRegistered(
			(String) propRemoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
			null)) {
			remoteMachineInfo = manager.getRemoteMachineInfo(
				(String) propRemoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
				null);
			remoteMachineInfo.reset();
		} else {
			remoteMachineInfo = new RemoteMachineInfo();
			remoteMachineInfo.put(RemoteMachineInfo.KEY_HOSTNAME,
				(String) propRemoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME));
		}
		
		/* write Local LDIF Information */
		remoteMachineInfo.overwriteParameter(propRemoteMachineInfo);
		
		/* overwrite ServerDefault */
		if (manager.getServerDefault() != null) {
			remoteMachineInfo.overwriteParameter(manager.getServerDefault());
		}
		
		/* put RemoteMachineInfo into manager */
		manager.putRemoteMachineInfo(
			(String) propRemoteMachineInfo.get(RemoteMachineInfo.KEY_HOSTNAME),
			null, remoteMachineInfo);
	}

	/**
	 * read information of function
	 * @param br
	 * @param manager
	 */
	static void readFuncInfo(BufferedReader br, NgInformationManager manager)
		throws GrpcException {
		String hostName = null;
		boolean defined_funcName = false;
		String module = null;
		String entry = null;
		String path = null;
		RemoteClassInfo classInfo = null;
	
		String recvBuffer = null;
		boolean base64reading = false;
		StringBuffer functionInfoBuffer = null;
	
		try {
			while ((recvBuffer = br.readLine()) != null) {
				/* check if the end of a entry */
				if (checkTheEndOfEntry(recvBuffer) == true)
					break;
		
				StringTokenizer st = new StringTokenizer(recvBuffer, " \t");
				String tag = st.nextToken();
				if (tag.equals(GRIDRPC_HOSTNAME)) {
					if (hostName != null) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_HOSTNAME + " is already defined.");
					}
					hostName = st.nextToken();
					base64reading = false;
				} else if (tag.equals(GRIDRPC_FUNCNAME)) {
					if (defined_funcName == true) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_FUNCNAME + " is already defined.");
					}
					defined_funcName = true;
					base64reading = false;
				} else if (tag.equals(GRIDRPC_MODULE)) {
					if (module != null) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_MODULE + " is already defined.");
					}
					module = st.nextToken();
					base64reading = false;
				} else if (tag.equals(GRIDRPC_ENTRY)) {
					if (entry != null) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_ENTRY +	" is already defined.");
					}
					entry = st.nextToken();
					base64reading = false;
				} else if (tag.equals(GRIDRPC_PATH)) {
					if (path != null) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_PATH + " is already defined.");
					}
					path = st.nextToken();
					base64reading = false;
				} else if (tag.equals(GRIDRPC_STUB)) {
					if (classInfo != null) {
						throw new NgInitializeGrpcClientException(
							GRIDRPC_STUB + " is already defined.");
					}
					functionInfoBuffer = new StringBuffer(st.nextToken());
					base64reading = true;
				} else if (base64reading == true) {
					functionInfoBuffer.append(recvBuffer.trim());
				} else {
					/* unrecognized parameter */
					throw new NgInitializeGrpcClientException(
						"Detected unrecognized parameter");
				}
			}
		} catch (IOException e) {
			/* something wrong was happened */
			throw new NgIOException(e);
		} catch (NoSuchElementException e) {
			/* the end of data?, nothing will be done */
		} finally {
			if (base64reading == true) {
				byte[] xmlData = Base64.decode(
							functionInfoBuffer.toString().getBytes());
				String xmlDataString = new String(xmlData);
				classInfo = 
					RemoteClassInfo.readClassInfo(xmlDataString);
			}
		}
	
		/* check if data was valid */
		if (hostName == null) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_HOSTNAME + " is not defined.");
		}
		if (defined_funcName != true) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_FUNCNAME + " is not defined.");
		}
		if (module == null) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_MODULE + " is not defined.");
		}
		if (entry == null) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_ENTRY + " is not defined.");
		}
		if (path == null) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_PATH + " is not defined.");
		}
		if (classInfo == null) {
			throw new NgInitializeGrpcClientException(
				GRIDRPC_STUB + " is not defined or invalid.");
		}

		/* set class name */
		String className = module + "/" + entry;
		/* set path into functionInfo, and put FunctionInfo into cache */
		RemoteMachineInfo remoteMachineInfo =
			manager.getRemoteMachineInfo(hostName, null);
		if (remoteMachineInfo == null) {
			remoteMachineInfo = new RemoteMachineInfo();
			remoteMachineInfo.put(
				RemoteMachineInfo.KEY_HOSTNAME, hostName);
		}

		/* set class information */
		RemoteClassPathInfo remoteClassPathInfo =
			remoteMachineInfo.getRemoteClassPath(className);
		if (remoteClassPathInfo == null) {
			remoteClassPathInfo = new RemoteClassPathInfo();
			
		}
		remoteClassPathInfo.put(
			RemoteClassPathInfo.KEY_CLASS_PATH_HOSTNAME, hostName);
		remoteClassPathInfo.put(
			RemoteClassPathInfo.KEY_CLASS_PATH_CLASSNAME, className);
		remoteClassPathInfo.put(
			RemoteClassPathInfo.KEY_CLASS_PATH_CLASSPATH, path);

		/* register RemoteMachineInfo into InformationManager */
		manager.putRemoteMachineInfo(hostName, null, remoteMachineInfo);

		/* register ClassPathInfo into InformationManager */
		manager.putRemoteClassPathInfo(hostName, className, remoteClassPathInfo);

			/* register ClassInfo into InformationManager */
		manager.putRemoteClassInfo(className, classInfo);
	}

	/**
	 * Check the end of Entry
	 * @param targetBuffer
	 * @return
	 */
	static boolean checkTheEndOfEntry(String targetBuffer) {
		if (targetBuffer.trim().length() == 0) {
			return true;
		}
		return false;
	}
}
