/*
 * $RCSfile: RemoteMachineInfo.java,v $ $Revision: 1.19 $ $Date: 2008/03/16 03:26:02 $
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
package org.apgrid.grpc.ng.info;

import java.util.Iterator;
import java.util.List;
import java.util.Properties;
import java.util.Vector;
import java.util.Map;
import java.util.HashMap;
import java.util.regex.PatternSyntaxException;

import org.apgrid.grpc.ng.CommLogInfo;
import org.apgrid.grpc.ng.CompressInfo;
import org.apgrid.grpc.ng.DebugInfo;
import org.apgrid.grpc.ng.HeartbeatInfo;
import org.apgrid.grpc.ng.RSLInfo;
import org.apgrid.grpc.ng.TcpConnectInfo;
import org.apgrid.grpc.ng.CommunicationProxyInfo;

import org.apgrid.grpc.ng.NgException;
import org.gridforum.gridrpc.GrpcException;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.*;

/*
 * This class manages Remote Machine Information.
 * Remote Machine Information is almost the same as <SERVER> section. 
 */
public class RemoteMachineInfo implements Cloneable {

	private Map<String, RemoteClassPathInfo> classPaths = 
	 new HashMap<String, RemoteClassPathInfo>();

	// basic information of Remote Machine Information.
	private Map<String, String> info = new HashMap<String, String>();
	// keys for info
	private static final String[] keys = {
		HOSTNAME,
		TAG,
		PORT,
		SUBJECT,
		JOBMANAGER,
		TCP_NODELAY,
		KEEP_CONNECTION,
		BLOCK_SIZE,
		REDIRECT_OUTERR,
		ARG_TRANS,
		WORK_DIR,
		CORE_SIZE,
		CLIENT_HOSTNAME,
	};

	private int mpi_runNoOfCPUs_default = 0;
	private Map<String, Integer> mpi_runNoOfCPUs = new HashMap<String,Integer>();

	private String hostDN; 
	private Map environment;

	private String invoke_server;
	private List<String> invoke_server_option;

	private RSLInfo rslInfo;
	private DebugInfo debugInfo;
	private CommLogInfo commLogInfo;
	private CompressInfo compressInfo;
	private HeartbeatInfo heartbeatInfo;
	private TcpConnectInfo tcpConnectInfo;
        private CommunicationProxyInfo communicationProxyInfo;

	// value 
	public static final String VAL_BACKEND_NORMAL = "normal";
	public static final String VAL_BACKEND_MPI    = "mpi";
	public static final String VAL_BACKEND_BLACS  = "blacs";
	public static final String INVOKE_SERVER_NONE = "none";

	public static final String DISABLE_COMMUNICATION_PROXY = null;

///// Constructors

	/**
	 * RemoteMachineInfo Default Constructor
	 */
	public RemoteMachineInfo() {
		initInfo();
		setDefaultServerParameter();
	}
	
	/**
	 * RemoteMachineInfo Constructor
	 * @param hostName
	 */
	public RemoteMachineInfo(String hostName) {
		this(); // call default constructor
		info.put(HOSTNAME, hostName);
	}
	
	/**
	 * @param param
	 */
	public RemoteMachineInfo(Properties param) {
		this(); // call default constructor
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void initInfo() {
		for (int i = 0; i < keys.length; i++) {
			info.put(keys[i], null);
		}
	}

	private final void setDefaultServerParameter() {
		info.put(TCP_NODELAY, "false");
		info.put(KEEP_CONNECTION, "true");
		info.put(REDIRECT_OUTERR, "true");
		info.put(ARG_TRANS, "wait");
		info.put(BLOCK_SIZE, "16000");

		this.invoke_server = INVOKE_SERVER_NONE;
		this.invoke_server_option = new Vector<String>();

		this.environment = new HashMap();
		this.rslInfo = new RSLInfo();
		this.debugInfo = new DebugInfo();
		this.commLogInfo = new CommLogInfo();
		this.compressInfo = new CompressInfo();
		this.heartbeatInfo = new HeartbeatInfo();
		this.tcpConnectInfo = new TcpConnectInfo();
                this.communicationProxyInfo = new CommunicationProxyInfo();
	}

///// Getter

	private String get(final String key) {
		return this.info.get(key);
	}

	public String getHostname() {
		return get(HOSTNAME);
	} 
	public String getTag() {
		return get(TAG);
	}
	public String getPort() {
		return get(PORT);
	} 
	public String getSubject() {
		return get(SUBJECT);
	}
	public String getJobmanager() {
		return get(JOBMANAGER);
	} 
	public String getClientHostname() {
		return get(CLIENT_HOSTNAME);
	} 
	public String getBlockSize() {
		return get(BLOCK_SIZE);
	} 
	public String getRedirectOuterr() {
	    	return get(REDIRECT_OUTERR);
	} 
	public String getWorkDir() {
		return get(WORK_DIR);
	} 
	public String getCoreSize() {
		return get(CORE_SIZE);
	} 
	public String getTcpNodelay() {
    		return get(TCP_NODELAY);
	} 

	public String getKeepConnection() {
		return get(KEEP_CONNECTION);
	}

	public String getHostDN() {
		return this.hostDN;
	} 
	public Map getEnvironment() {
		return this.environment;
	} 

	public RSLInfo getRslInfo() {
		return this.rslInfo;
	} 
	public DebugInfo getDebugInfo() {
		return this.debugInfo;
	} 
	public CommLogInfo getCommLogInfo() {
		return this.commLogInfo;
	} 
	public HeartbeatInfo getHeartbeatInfo() {
		return this.heartbeatInfo;
	} 
	public TcpConnectInfo getTcpConnectInfo() {
		return this.tcpConnectInfo;
	}
	public CompressInfo getCompressInfo() {
		return this.compressInfo;
	}
        public CommunicationProxyInfo getCommunicationProxyInfo() {
                return this.communicationProxyInfo;
        }

	public String getInvokeServer() {
    	return this.invoke_server;
	}
	public List<String> getInvokeServerOption() {
		return this.invoke_server_option;
	}

///// Setter

	public void setHostDN(String hostDN) {
		this.hostDN = hostDN;
	}
	public void setHostname(String name) {
		info.put(HOSTNAME, name);
	}


	/**
	 * @param className
	 * @return
	 */
	public RemoteClassPathInfo getRemoteClassPath(String className) {
		return classPaths.get(className);
	}
	
	/**
	 * @param className
	 * @param classPathInfo
	 */
	public void putRemoteClassPath(String aClassname,
	 RemoteClassPathInfo aRcpi) {

		RemoteClassPathInfo rcpi = classPaths.get(aClassname);
		if (rcpi != null) {
			rcpi.update(aRcpi);
			aRcpi = rcpi;
		}
		setNumCPUs(aRcpi);
		classPaths.put(aClassname, aRcpi);
	}

	/**
	 * Updates remote machine information
	 * 
	 * @param properties of remote machine information
	 */
	public void update(Properties prop) {
		for (Map.Entry<Object, Object> ent : prop.entrySet()) {
			String key = (String) ent.getKey();
			if ( key.equals(ENVIRONMENT) ) {
				updateEnv((Map)ent.getValue());
			}
			else if ( key.equals(MPI_NCPUS) ) {
				updateNcpus((List<String>)ent.getValue());
			}
			else if ( key.equals(INVOKE_SERVER_OPTION) ) {
				updateInvokeServerOption((List<String>)ent.getValue());
			}
			else if ( key.equals(COMMUNICATION_PROXY_OPTION) ) {
				updateCommunicationProxyOption((List<String>)ent.getValue());
			}
 			else if ( key.equals(HOSTNAME) ) {
				//  nothing will be done
			}
			else if ( key.equals(JOB_RSL_EXTENSION) ) {
				rslInfo.put((List<String>)ent.getValue());
			}
			else {
				updateValue(key, (String)ent.getValue());
			}
		}
	}

	private final void updateEnv(Map param) {
		Map wkEnv = this.environment;
		if (wkEnv == null) {
			throw new NullPointerException("environment field is null");
		}
		if (wkEnv.equals(param) ) {
			return; // same map
		}
		// update
		wkEnv.putAll(param);

		this.environment = wkEnv;
	}

	private final void updateNcpus(List<String> list) {
		for (String value : list) {
			int index = value.indexOf('=');
			if (index < 0) {
				mpi_runNoOfCPUs_default = strToNcpus(value);
			} else {
				try {
					String[] str = value.split("=");
					if (str.length != 2) {
						throw new IllegalArgumentException("attribute " + MPI_NCPUS + "'s value \"" + value + "\" is syntax error");
					}
					int ncpus = strToNcpus(str[1]);
					mpi_runNoOfCPUs.put(str[0], new Integer(ncpus));
				} catch (PatternSyntaxException e) {
					throw new IllegalArgumentException("attribute \"" + MPI_NCPUS + ": " + e.getMessage());
				}
			}
		}
	}

	private int strToNcpus(String value) {
		int ncpus = 0;
		try {
			ncpus = Integer.parseInt(value);
		} catch(NumberFormatException e) {
			throw new IllegalArgumentException(e.getMessage());
		}

		if (ncpus <= 0) {
			throw new IllegalArgumentException("attribute " + MPI_NCPUS + "'s value \"" + value + "\"smaller equal zero.");
		}

		// Success
		return ncpus;
	}

	private final void updateInvokeServerOption(List<String> param) {
		List<String> options = this.invoke_server_option;
		if (options == null) {
			throw new NullPointerException("invoke_server_option field is null");
		}

		if (options.containsAll(param)) { return ; } // same list

		// copy all of variables 
		for (int i = 0; i < param.size(); i++) {
			if (! options.contains(param.get(i))) {
				options.add(param.get(i));
			}
		}
		// set new map of options 
		this.invoke_server_option = options;
	}

        private final void updateCommunicationProxyOption(List<String> param) {
                List<String> options = this.getCommunicationProxyInfo()
                    .getCommunicationProxyOption();
                if (options == null) {
                    throw new NullPointerException("communication_proxy_option field is null");
                }

                if (options.containsAll(param)) { return ; } // same list

		// copy all of variables 
		for (int i = 0; i < param.size(); i++) {
                        if (! options.contains(param.get(i))) {
				options.add(param.get(i));
			}
		}
		// set new map of options 
                this.getCommunicationProxyInfo().putCommunicationProxyOption(options);
    }


	/*
	 * @param key of remote machine information
	 * @param value of remote machine information
	 */
	private final void updateValue(String aKey, String value) {
		if ( aKey.startsWith("job_") ) {
			rslInfo.put(aKey, value);
			return;
		} else if ( aKey.startsWith("commLog_") ) {
			commLogInfo.put(aKey, value);
			return;
		} else if ( aKey.startsWith("debug") ) {
			debugInfo.put(aKey, value);
			return;
		} else if ( aKey.startsWith("tcp_connect") ) {
			tcpConnectInfo.put(aKey, value);
			return;
                } else if ( aKey.startsWith("communication_proxy") ) {
                        this.communicationProxyInfo.put(aKey, value);
			return;
		} else if ( aKey.startsWith(HEARTBEAT) ) {
			heartbeatInfo.put(aKey, value);
			return;
		} else if ( aKey.startsWith(COMPRESS) ) {
			compressInfo.put(aKey, value);
			return ;
		} else if ( aKey.equals(INVOKE_SERVER) ) {
			this.invoke_server = value;
			return;
		} else if ( info.containsKey(aKey) ) {
			info.put(aKey, value);
			return;
		}

		throw new IllegalArgumentException(aKey);
	}


	/**
	 * @param className
	 * @return
	 */
	public int getNumCPUs(String className) {
		RemoteClassPathInfo rcpi = classPaths.get(className);

		if ( (rcpi != null) && rcpi.hasMpiNcpus() ) {
			return Integer.parseInt(rcpi.getMpiNcpus());
		}

		return mpi_runNoOfCPUs_default;
	}

	/**
	 * @param prop
	 */
	private void setNumCPUs(RemoteClassPathInfo rcpi) {
		Integer nCPUs = mpi_runNoOfCPUs.get(rcpi.getClassname());
		if (nCPUs == null) {
			rcpi.setMpiNcpus(String.valueOf(mpi_runNoOfCPUs_default));
		} else {
			rcpi.setMpiNcpus(nCPUs.toString());
		}
	}

	public void resetNumCPUs() throws GrpcException {
		Iterator keys =  classPaths.keySet().iterator();
		// register all elements of numCPUs 
		while (keys.hasNext()) {
			String targetClass = (String)keys.next(); // get name of Class 

			// get RemoteClassPathInfo 
			RemoteClassPathInfo rcpInfo = getRemoteClassPath(targetClass);
			if (rcpInfo == null) {
				throw new NgException("Failed to find RemoteClassPathInfo.");
			}

			// remove num of CPUs 
			rcpInfo.invalidateMpiNcpus();
		}
	}

	/**
	 * @return a clone of this instance.
	 */
	public RemoteMachineInfo getCopy() {
		try {
			RemoteMachineInfo result = (RemoteMachineInfo) this.clone();
			result.classPaths = copyRemoteClassPath();
			result.info = new HashMap<String, String>(this.info);
			result.environment = new HashMap(this.environment);
			result.invoke_server_option =
				new Vector<String>(this.invoke_server_option);
			result.rslInfo = RSLInfo.copy(this.rslInfo);
			result.debugInfo = DebugInfo.copy(this.debugInfo);
			result.commLogInfo = CommLogInfo.copy(this.commLogInfo);
			result.compressInfo = CompressInfo.copy(this.compressInfo);
			result.heartbeatInfo = HeartbeatInfo.copy(this.heartbeatInfo);
			result.tcpConnectInfo = TcpConnectInfo.copy(this.tcpConnectInfo);
			return result;
		} catch (CloneNotSupportedException e) {
			throw new Error("Assertion failure");
		}
	}
	
	private final Map<String, RemoteClassPathInfo> copyRemoteClassPath() {
		Map<String, RemoteClassPathInfo> ret = 
		 new HashMap<String, RemoteClassPathInfo>();

		Iterator<String> keys = classPaths.keySet().iterator();
		while ( keys.hasNext() ) {
			String key = keys.next();
			RemoteClassPathInfo rcpi = classPaths.get(key).getCopy();
			ret.put(key, rcpi);
		}

		return ret;
	}

	public void reset() {
		String hostname = get(HOSTNAME);
		String tag      = get(TAG);

		resetFields();
		setDefaultServerParameter();

		info.put(HOSTNAME, hostname);
		if (tag != null) {
			info.put(TAG, tag);
		}
	}

	private final void resetFields() {
		initInfo();
		this.hostDN = null;
		this.environment = null;
		this.invoke_server = null;
		this.invoke_server_option = null;

		this.rslInfo = null;
		this.debugInfo = null;
		this.commLogInfo = null;
		this.compressInfo = null;
		this.heartbeatInfo = null;
		this.tcpConnectInfo = null;
                this.communicationProxyInfo = null;
	}
	
	/**
	 * @return
	 */
	public String makeHandleString() {
		return makeHandleString(get(HOSTNAME), get(TAG));
	}
	
	/**
	 * @param hostname
	 * @param tag
	 * @return
	 */
	public static String makeHandleString(String aHostname, String aTag) {
		if (aHostname == null)
			throw new NullPointerException("aHostname is null");

		if (aTag == null)
			return aHostname;

		return aHostname + "/" + aTag;
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("hostDN : " + hostDN + "\n");
		for (Map.Entry<String, String> ent : info.entrySet()) {
			sb.append(ent.getKey() + ": " + ent.getValue() + "\n");
		}
		sb.append("environment: " + environment + "\n");

		sb.append("invoke_server: " + invoke_server + "\n");
		sb.append("invoke_server_option: " + invoke_server_option + "\n");

		sb.append(rslInfo.toString());
		sb.append(heartbeatInfo.toString());
		sb.append(commLogInfo.toString());
		sb.append(compressInfo.toString());
		sb.append(debugInfo.toString());
		sb.append(tcpConnectInfo.toString());
		sb.append("+ RemoteClassPathInfo\n");
		for (Map.Entry<String, RemoteClassPathInfo> ent : classPaths.entrySet()) {
		    sb.append(ent.getKey() + "\n" + ent.getValue().toString());
		}

                sb.append(communicationProxyInfo.toString());
		return sb.toString();
	}
}
