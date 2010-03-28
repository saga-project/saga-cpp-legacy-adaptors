/*
 * $RCSfile: ClientCommunicationProxyManager.java,v $ $Revision: 1.8 $ $Date: 2008/03/28 03:25:55 $
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
import java.util.Collection;

import org.apgrid.grpc.util.NgUtil;
import org.apgrid.grpc.ng.info.ClientCommunicationProxyInfo;

/*
 * ClientCommunicationProxy create/destroy management class
 */
class ClientCommunicationProxyManager {

	// repository of created Client Communication Proxies
	private Map<String, List<ClientCommunicationProxy>> repository;

	// configuration of Client Communication Proxies
	private Map<String, ClientCommunicationProxyInfo> config;

	private NgGrpcClient context;
	private NgLog ngLog;

	/*
	 * Constructor
	 */
	public ClientCommunicationProxyManager(NgGrpcClient context) {
		if (context == null) {
			throw new NullPointerException();
		}
		this.context = context;

		this.repository = new HashMap<String, List<ClientCommunicationProxy>>();

		this.config = buildConfig( context.getConfig().getClientCommProxyInfo() );
		if (this.config == null) {
			throw new NullPointerException("Client Communication Proxy configuration is null");
		}

		this.ngLog = context.getNgLog();
	}
	
	private Map<String, ClientCommunicationProxyInfo>
	buildConfig(List<NgConfigSection> config) {

		if (config == null) {
			return null;
		}
		Map<String, ClientCommunicationProxyInfo> ret = new HashMap<String, ClientCommunicationProxyInfo>();

		for (NgConfigSection section : config) {
			ClientCommunicationProxyInfo info = new ClientCommunicationProxyInfo(section);

			ret.put(info.getType(), info);
		}
		return ret;
	}

	
	/*
	 * Returns ClientCommunicationProxy of type(argument).
	 * 
	 * @param type of Communication Proxy
	 * @return connectable Client Communication Proxy
	 */
	public synchronized ClientCommunicationProxy get(String type)
		 throws NgException {
			 if (type == null) {
				 throw new NullPointerException();
			 }
			 if (type.length() == 0) {
				 throw new IllegalArgumentException();
			 }
			 if (!repository.containsKey(type)) {
				 ClientCommunicationProxy proxy = createProxies(type);
				 if (proxy.useProxy()) {
					 return proxy;
				 }
			 }

			 List<ClientCommunicationProxy> proxies = repository.get(type);
			 for (ClientCommunicationProxy proxy : proxies) {
				 if ( proxy.useProxy() ) {
					 return proxy;
				 }
			 }

			 // no one connectable Client Communication Proxy
			 ClientCommunicationProxy newProxy = createProxy(type);
			 proxies.add(newProxy);
			 if (newProxy.useProxy()) {
				 return newProxy;
			 }

			 return null;
	}

	/*
	 * Set-up the list of ClientCommunicationProxy
	 * & Returns the first ClientCommunicationProxy for the type.
	 * 
	 * @return a newly ClientCommunicationProxy
	 * @throws NgException if failed to create ClientCommunicationProxy
	 */
	private ClientCommunicationProxy createProxies(String type)
		 throws NgException {
			 ClientCommunicationProxy newProxy = createProxy(type);
		
			 List<ClientCommunicationProxy> proxies = new ArrayList<ClientCommunicationProxy>();
			 proxies.add(newProxy);
			 repository.put(type, proxies);

			 return newProxy;
	}

	private ClientCommunicationProxy createProxy(String type)
		 throws NgException {
			 ClientCommunicationProxyInfo info = this.config.get(type);
			 if (info == null) {
				 info = new ClientCommunicationProxyInfo(type);
			 }
			 
			 String path = info.getPath();
			 if (path == null) {
				 path = NgUtil.getDefaultPath("ng_client_communication_proxy", type);
			 }
			 String log_filePath = info.getLogFilePath();

			 if (log_filePath == null) {
				 log_filePath = this.context.getLocalMachineInfo().getClientCommunicationProxyLog();
				 if (log_filePath != null) {
					 log_filePath += "."+info.getType();
				 }
			 }
		
			 String [] command = null;
			 if (log_filePath == null) {
				 command = new String[] { path };
			 } else {
				 command = new String[] { path, "-l", log_filePath};
			 }
			 
			 int listen_port = this.context.getPortManagerNoSecure().getPort();

			 return new ClientCommunicationProxy(
					 this, command, type, listen_port,
					 info.getMaxJobs(), info.getBufferSize(),
					 info.getOption(), this.ngLog);
	}

	/**
	 * Remove ClientCommunicationProxy from repository.
	 */
	protected synchronized void remove(String type, ClientCommunicationProxy proxy) {
		for (Map.Entry<String, List<ClientCommunicationProxy>> ent : repository.entrySet()) {
			if (!ent.getKey().equals(type)) {
				continue;
			}
			List<ClientCommunicationProxy> list = ent.getValue();
			list.remove(proxy);
			break;
		}
	}
	/**
	 * Deactivate all of ClientCommunicationProxy.
	 */
	protected synchronized void deactivate() {
		Collection<List<ClientCommunicationProxy>> c = repository.values();
		for (List<ClientCommunicationProxy> proxies : c ) {
			for (ClientCommunicationProxy proxy : proxies) {
				proxy.deactivate();
			}
		}
	}

	/**
	 * Disposes all of ClientCommunicationProxy
	 */
	public synchronized void dispose() {
		Collection<List<ClientCommunicationProxy>> c = repository.values();
		for (List<ClientCommunicationProxy> proxies : c ) {
			for (ClientCommunicationProxy proxy : proxies) {
				try {
					proxy.requestExit();
				} catch (NgException e) {
					ngLog.logError(NgLog.CAT_NG_INTERNAL, e);
				}
			}
		}
	}
	
}
