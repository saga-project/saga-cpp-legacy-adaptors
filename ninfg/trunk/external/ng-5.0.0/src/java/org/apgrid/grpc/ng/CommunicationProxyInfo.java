/*
 * $RCSfile: CommunicationProxyInfo.java,v $ $Revision: 1.4 $ $Date: 2008/02/15 12:09:41 $
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

import java.util.List;
import java.util.ArrayList;
import java.util.Map;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMUNICATION_PROXY_TYPE;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMUNICATION_PROXY_STAGING;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMUNICATION_PROXY_PATH;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMUNICATION_PROXY_BUFFER_SIZE;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMUNICATION_PROXY_OPTION;
import static org.apgrid.grpc.ng.info.RemoteMachineInfo.DISABLE_COMMUNICATION_PROXY;

/*
 * Information for RemoteMachineInfo class
 */
public class CommunicationProxyInfo {
	private String communicationProxy = null;
	private boolean	communicationProxyStaging = false;
	private String communicationProxyPath = null;
	private int communicationProxyBufferSize = 8 * 1024;
	private List<String> communicationProxyOption = null;

	/**
	 * CommunicationProxyInfo Constructor
	 */
	public CommunicationProxyInfo() {
		setDefaultParameter();
	}

	/**
	 * @param prop
	 */
	public CommunicationProxyInfo(Map<Object, Object> param) {
		setDefaultParameter();
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void setDefaultParameter() {
		this.communicationProxy = DISABLE_COMMUNICATION_PROXY;
		this.communicationProxyStaging	= false;
		this.communicationProxyBufferSize = 8 * 1024;
		this.communicationProxyOption = new ArrayList<String>();
	}

	public String getCommunicationProxy() {
		return communicationProxy;
	}
	public boolean getCommunicationProxyStaging() {
		return communicationProxyStaging;
	}
	public String getCommunicationProxyPath() {
		return communicationProxyPath;
	}
	public int getCommunicationProxyBufferSize() {
		return communicationProxyBufferSize;
	}
	public List<String> getCommunicationProxyOption() {
		return communicationProxyOption;
	}


	/**
	 * @param  param
	 */
	public void update(Map<Object, Object> param) {
		for (Map.Entry<Object, Object> ent : param.entrySet()) {
			_update((String)ent.getKey(), (String)ent.getValue());
		}
	}
	
	public void put(String aKey, String value) {
		_update(aKey, value);
	}
	private final void _update(String aKey, String value) {
		if (aKey.equals(COMMUNICATION_PROXY_TYPE)) {
			this.communicationProxy = value;
			return;
		} else if (aKey.equals(COMMUNICATION_PROXY_STAGING)) {
			this.communicationProxyStaging = Boolean.valueOf(value);
			return;
		} else if (aKey.equals(COMMUNICATION_PROXY_PATH)) {
			this.communicationProxyPath = value;
			return;
		} else if (aKey.equals(COMMUNICATION_PROXY_BUFFER_SIZE)) {
			this.communicationProxyBufferSize = Integer.parseInt(value);
			return;
		} else if (aKey.equals(COMMUNICATION_PROXY_OPTION)) {
			;
		}
		throw new IllegalArgumentException(aKey);
	}
	
	public void putCommunicationProxyOption(List<String> options) {
		this.communicationProxyOption = options;
	}

	public static CommunicationProxyInfo copy(CommunicationProxyInfo other) {
		CommunicationProxyInfo result = new CommunicationProxyInfo();
		result.communicationProxy = other.communicationProxy;		 
		result.communicationProxyStaging = other.communicationProxyStaging;
		result.communicationProxyPath = other.communicationProxyPath;
		result.communicationProxyBufferSize = other.communicationProxyBufferSize;
		result.communicationProxyOption = other.communicationProxyOption;
		return result;
	}

	public void reset() {
		resetAllFields();
		setDefaultParameter();
	}

	private final void resetAllFields() {
		this.communicationProxy = null;
		this.communicationProxyStaging = false;
		this.communicationProxyPath = null;
		this.communicationProxyBufferSize = 8 * 1024;
		this.communicationProxyOption = null;
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("communication_proxy: " + communicationProxy + "\n");
		sb.append("communication_proxy_staging: " + communicationProxyStaging + "\n");
		sb.append("communication_proxy_path: " + communicationProxyPath + "\n");
		sb.append("communication_proxy_buffer_size: " + communicationProxyBufferSize + "\n");
		sb.append("communication_proxy_option: " + communicationProxyOption + "\n");
		return sb.toString();
	}
}
