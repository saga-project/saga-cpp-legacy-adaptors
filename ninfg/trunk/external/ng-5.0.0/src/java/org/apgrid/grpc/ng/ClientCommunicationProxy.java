/*
 * $RCSfile: ClientCommunicationProxy.java,v $ $Revision: 1.10 $ $Date: 2008/03/28 03:25:55 $
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
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.io.IOException;
import java.io.LineNumberReader;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;



/*
 * Client Communication Proxy
 */
public class ClientCommunicationProxy implements ExtModule{
	private ClientCommunicationProxyManager ccpm;
	private boolean valid;
	private String type;
	private int listen_port;
	private int max_jobs;
	private ProcessCommunicator process = null;
	private boolean exit_request = false;
	private int buffer_size = 8 * 1024;/* 8KB */

	// Number of connected jobs.
	private int nJobs = 0;

	// Number of destroyed jobs.
	private int destroyedJobs = 0;

	// options for Client Communication Proxy
	private List<String> options;

	private NgLog logger = null;

	// prepared Client Communication Proxy's information
	private Map<Integer, List<NgAttribute>> attribute;

	private int requestId = 1;

	// Client Communication Proxy's notifies
	private static final String COMMUNICATION_REPLY = "COMMUNICATION_REPLY";


	/*
	 * Constructor
	 * 
	 * @param command line
	 * @param type of Communication Proxy
	 * @param listening port of Ninf-G Client 
	 * @param a number
	 * @param options of Client Communication Proxy
	 * @param logging object
	 */
	public ClientCommunicationProxy(ClientCommunicationProxyManager ccpm,
			String[] ccpCommand, 
			String type,
			int port,
			int max_jobs,
			int buffer_size,
			List<String> options,
			NgLog log)
		 throws NgException {
			 if ((ccpm == null) || (ccpCommand == null)||(type == null)) {
				 throw new NullPointerException();
			 }
			 if ((port < 0)||(max_jobs < 0)) {
				 throw new IllegalArgumentException();
			 }

			 this.ccpm = ccpm;
			 this.type = type;
			 this.listen_port = port;
			 this.max_jobs = max_jobs;
			 this.buffer_size = buffer_size;
			 if (options == null) {
				 this.options = new ArrayList<String>();
			 } else {
				 this.options = options;
			 }
			 this.logger = log;
			 this.attribute = new HashMap<Integer, List<NgAttribute>>();

			 try {
				 this.process = new ProcessCommunicator(ccpCommand);
				 List<String> features = queryFeatures();
				 logDebugInternal("Correspondable features: " + features);
				 // checked features in queryFeatures();

				 initialize();
			 } catch(IOException e) {
				 throw new NgException(e);
			 } catch(NgException e) {
				 throw e;
			 }
			 NotifyThreadManager notifyThread = new NotifyThreadManager();
			 notifyThread.setExtModule(this);
			 notifyThread.setNgLog(this.logger);
			 notifyThread.startThread();
			 this.valid = true;
	}
 
	/*
	 * Request QUERY_FEATURES
	 * 
	 * @return Strings("name val") of reply. User should parse string.
	 */
	private List<String> queryFeatures() throws NgException {
		ExtModuleRequest req = ExtModuleRequest.QUERY_FEATURES;
		logDebugInternal("Request is " + req);
		try {
			ExtModuleReply rep = process.send(req);
			logInfoInternal("Send QUERY_FEATURES request to Communication Proxy");
			logInfoInternal("Receive QUERY_FEATURES reply from Communication Proxy");
			logDebugInternal("Reply is " + rep);

			if (rep.returnValues().contains("request INITIALIZE")&&
				rep.returnValues().contains("request PREPARE_COMMUNICATION")&&
				rep.returnValues().contains("request QUERY_FEATURES")&&
				rep.returnValues().contains("request EXIT")) {
			} else {
				throw new NgException("unsupported " + "this Client Communication Proxy\n"+rep);
			}
			if (!rep.result().equals("SM")) {
				throw new NgException(req + " error. returned " + rep);
			}
			return rep.returnValues();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	
	/*
	 * Request INITIALIZE
	 */
	private void initialize() throws NgException {
		ExtModuleRequest req = ExtModuleRequest.issuesMultiple("INITIALIZE");
		req.addAttribute("listen_port", String.valueOf(this.listen_port));
		req.addAttribute("buffer_size", String.valueOf(this.buffer_size));
		for (String e : this.options) {
			req.addAttribute(e);
		}
		logDebugInternal("Request is " + req);

		ExtModuleReply rep = null;
		try {
			rep = process.send(req);
			logInfoInternal("Send INITIALIZE request to Communication Proxy");
			logInfoInternal("Receive INITIALIZE reply from Communication Proxy");
			logDebugInternal("Reply is " + rep);

			if (rep.toString().charAt(0) != 'S'){
				throw new IOException();
			}
		} catch (IOException e) {
			logErrorInternal("ClientCommunicationProxy Error:" + e.getMessage());
			throw new NgException(e.getMessage());
		}
	}

	/*
	 * Request PREPARE_COMMUNICATION
	 * 
	 * @param the options from <SERVER> communication_proxy_option
	 * @return returns the request id
	 * @throws NgException
	 */
 	private int prepare(RemoteMachineInfo rmInfo) throws NgException {
  		ExtModuleRequest req = ExtModuleRequest.issuesMultiple("PREPARE_COMMUNICATION");
 		TcpConnectInfo tcInfo = rmInfo.getTcpConnectInfo();

		int requestId = nextRequestId();
		req.addAttribute("request_id ", String.valueOf(requestId));
		// Sets options specified in <SERVER> communication_proxy_option
		if (rmInfo.getCommunicationProxyInfo().getCommunicationProxyOption() != null) {
			for (String e : rmInfo.getCommunicationProxyInfo().getCommunicationProxyOption()) {
				req.addAttribute(e);
			}
		}

 		req.addAttribute("tcp_nodelay", rmInfo.getTcpNodelay());
 		req.addAttribute("tcp_connect_retryCount", tcInfo.getTcpConnectRetryCount());
 		req.addAttribute("tcp_connect_retryBaseInterval", tcInfo.getTcpConnectRetryBaseinterval());
 		req.addAttribute("tcp_connect_retryIncreaseRatio", tcInfo.getTcpConnectRetryIncreaseratio());
 		req.addAttribute("tcp_connect_retryRandom", tcInfo.getTcpConnectRetryRandom());

		logDebugInternal("Request is " + req);
		try {
			ExtModuleReply rep = process.send(req);	  
			logInfoInternal("Send PREPARE_COMMUNICATION request to Communication Proxy");
			logInfoInternal("Receive PREPARE_COMMUNICATION reply from Communication Proxy");
			logDebugInternal("Reply is " + rep);

			if( rep.toString().charAt(0) != 'S' ){
				throw new IOException();
			}
		} catch (IOException e) {
			throw new NgException(e.getMessage());
		}
		return requestId;
	}

	/*
	 * Request EXIT
	 */
	protected void requestExit() throws NgException {
		exit_request = true;
		ExtModuleRequest req = ExtModuleRequest.EXIT;
		logDebugInternal("Request is " + req);
		try {
			ExtModuleReply rep = process.send(req);
			logInfoInternal("Send EXIT request to Communication Proxy");
			logInfoInternal("Receive EXIT reply from Communication Proxy");
			logDebugInternal("Reply is " + rep);
			if (!rep.result().equals("S")) {
				throw new NgException(req + " error. returned " + rep);
			}
		} catch (IOException e) {
			throw new NgException(e.getMessage());
		}
		process.exit();
	}
 
	/**
	 * Returns the Client Communication Proxy information.
	 * 
	 * @param Remote Machine Information
	 * @return information of prepared Client Communication Proxy
	 */
	public synchronized List<NgAttribute> getInfo(RemoteMachineInfo rmInfo)
		throws NgException  {
		int request_id = 0;
		request_id = prepare(rmInfo);
		while ( !this.attribute.containsKey(request_id) ) {
			try {
			this.wait();
			} catch (InterruptedException e) {
			// Ignore exception.
			}
		}
		List<NgAttribute> result = this.attribute.get(request_id);
		this.attribute.remove(request_id);
	    return result;
	}

	/**
	 * Deactivate this instance.
	 */
	protected synchronized void deactivate() {
		this.valid = false;
	}

	/*
	 * @return true if connectable
	 * @return false if can not connect
	 */
	private synchronized boolean canConnect() {
		if (this.valid == false)
			return false;

		if (this.max_jobs == 0)
			return true;
		return (this.nJobs < this.max_jobs);
	}

	/**
	 * Use the Client Communication Proxy.
	 */
	protected synchronized boolean useProxy() {
		if (canConnect() == false) {
			return false;
		}
		this.nJobs++;
		return true;
	}

	/**
	 * Release the Client Communication Proxy.
	 */
	protected void releaseProxy() throws NgException {
		synchronized (this) {
			this.destroyedJobs++;
			if ((this.valid == false) && (this.destroyedJobs >= this.nJobs)) {
				; // Do nothing.
			} else if ((max_jobs <= 0) || (this.destroyedJobs < this.max_jobs)) {
				return;
			}
			requestExit();
		}
		ccpm.remove(this.type, this);
	}
	
	public void setProxyInfo(List<NgAttribute> parameter) {
		if (parameter == null)
			throw new NullPointerException();

		logDebugInternal("Client Communication Proxy information: " + parameter);

		String result = find("result", parameter);
		String request_id = find("request_id", parameter);

		if ((!result.equals("S"))||(request_id == null)) {
			logErrorInternal("COMMUNICATION_REPLY NOTIFY fail: " + parameter.toString());

			throw new RuntimeException("invalid notify received");
		}
		int req_id = Integer.parseInt(request_id);

		// put request_id, parameter into map
		synchronized (this) {
			this.attribute.put(Integer.valueOf(req_id), parameter);
			this.notifyAll();
		}
	}

	/*
	 * Returns the notify of Client Communication Proxy
	 */
	public ExtModuleNotify createNotify() throws IOException {
		if (exit_request) {
			return null;
		}

		LineNumberReader reader = this.process.getNotifyReader();
		String line = reader.readLine();
		if (line == null) {
			logInfoInternal("CommunicationProxy: reached EOF");
			return null;
		}

		if ( line.startsWith(COMMUNICATION_REPLY) ) {
			return new CommunicationReply(this,  reader);
		} else {
			logErrorInternal("unknown notify received.");
			throw new IOException("unknown notify received.");
		}
	}

	/*
	 * Returns the next request Id for PREPARE_COMMUNICATION.
	 */
	private synchronized int nextRequestId() {
		int next = this.requestId;
		if (this.requestId >= Integer.MAX_VALUE) {
			this.requestId = 1;
		} else {
			this.requestId++;
		}
		return next;
	}

	/*
	 * find method copy form InformationService.java at 2007-12-17-14:10
	 */
	private String find(String key, List<NgAttribute> values) {
		for (NgAttribute attr : values) {
			if (attr.getName().equals(key))
				return attr.getValue();
		}
		return null;
	}

	/*
	 * logging methods
	 */
	private void logInfoInternal(String msg) {
		if (logger == null) return;
		logger.logInfo(NgLog.CAT_NG_INTERNAL, "ClientCommunicationProxy#" + msg);
	}
	private void logDebugInternal(String msg) {
		if (logger == null) return;
		logger.logDebug(NgLog.CAT_NG_INTERNAL, "ClientCommunicationProxy#" + msg);
	}
	private void logWarnInternal(String msg) {
		if (logger == null) return;
		logger.logWarn(NgLog.CAT_NG_INTERNAL, "ClientCommunicationProxy#" + msg);
	}
	private void logErrorInternal(String msg) {
		if (logger == null) return ;
		logger.logError(NgLog.CAT_NG_INTERNAL, "ClientCommunicationProxy#" + msg); 
	}

	/* Inner class
	 * Representation of COMMUNICATION_REPLY notify.
	 * 
	 */
	private static class CommunicationReply implements ExtModuleNotify {
		public static final String NAME = "COMMUNICATION_REPLY";
		private ClientCommunicationProxy ccp;
		private List<NgAttribute> parameter;
		
		public CommunicationReply(ClientCommunicationProxy ccp, 
								  LineNumberReader reader)
			 throws IOException {
				 this.ccp = ccp;
				 this.parameter = ExtModuleNotifyParser.parse(reader, NAME);
		}
		public void handle() {
			this.ccp.setProxyInfo(parameter);
		} 
	}

}
