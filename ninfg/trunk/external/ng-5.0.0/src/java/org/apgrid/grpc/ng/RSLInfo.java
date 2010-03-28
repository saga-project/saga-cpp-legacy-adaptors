/*
 * $RCSfile: RSLInfo.java,v $ $Revision: 1.3 $ $Date: 2008/01/23 06:47:49 $
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
import java.util.Vector;
import java.util.Map;
import java.util.HashMap;

import org.gridforum.gridrpc.GrpcException;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.JOB_STARTTIMEOUT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.JOB_STOPTIMEOUT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.JOB_RSL_EXTENSION;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.SUBJECT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.QUEUE;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.PROJECT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.HOSTCOUNT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.MINMEMORY;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.MAXMEMORY;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.MAXTIME;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.MAXWALLTIME;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.MAXCPUTIME;

/*
 * Information for RemoteMachineInfo class
 */
public class RSLInfo {
	private String job_start_timeout = null;
	private String job_stop_timeout = null;
	private List<String> job_rslExtensions = null;
	private String job_queue = null;
	private String job_project = null;
	private String job_hostCount = null;
	private String job_minMemory = null;
	private String job_maxMemory = null;
	private String job_maxTime = null;
	private String job_maxWallTime = null;
	private String job_maxCpuTime = null;


	/**
	 * RSLInfo Constructor
	 */
	public RSLInfo() {
		setDefaultParameter();
	}
	
	/**
	 * @param param
	 */
	public RSLInfo(Map<Object, Object> param) {
		setDefaultParameter();
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void setDefaultParameter() {
		this.job_rslExtensions = new Vector<String>();
		this.job_start_timeout = "0";
		this.job_stop_timeout = "-1";
	}

	public String getJobStartTimeout() {
		return this.job_start_timeout;
	}
	public String getJobStopTimeout() {
		return this.job_stop_timeout;
	}
	public List<String> getJobRslExtensions() {
		return this.job_rslExtensions;
	}
	public String getJobQueue() {
		return this.job_queue;
	}
	public String getJobProject() {
		return this.job_project;
	}
	public String getJobHostCount() {
		return this.job_hostCount;
	}
	public String getJobMinMemory() {
		return this.job_minMemory;
	}
	public String getJobMaxMemory() {
		return this.job_maxMemory;
	}
	public String getJobMaxTime() {
		return this.job_maxTime;
	}
	public String getJobMaxWallTime() {
		return this.job_maxWallTime;
	}
	public String getJobMaxCpuTime() {
		return this.job_maxCpuTime;
	}
	
	/**
	 * @param prop
	 */
	public void update(Map<Object, Object> param) {
		for (Map.Entry<Object, Object> ent : param.entrySet()) {
			String key = (String)ent.getKey();
			if ( key.equals(JOB_RSL_EXTENSION) ) {
				_update((List<String>)ent.getValue());
			} else {
				_update(key, (String)ent.getValue());
			}
		}
	}

	public void put(List<String> param) {
		_update(param);
	}

	public void put(String aKey, String value) {
		_update(aKey, value);
	}

	private final void _update(List<String> param) {
		List<String> ext = this.job_rslExtensions;
		if (ext == null) {
			throw new NullPointerException("job_rslExtensions field is null");
		}

		if (ext.containsAll(param)) {
			return ; // same list
		}

		// copy all of variables 
		for (int i = 0; i < param.size(); i++) {
			if (! ext.contains(param.get(i))) {
				ext.add(param.get(i));
			}
		}

		// set new map of options 
		this.job_rslExtensions = ext;
	}

	private final void _update(String aKey, String value) {
		if ( aKey.equals(JOB_STARTTIMEOUT) ) {
			this.job_start_timeout = value;
			return;
		} else if (aKey.equals(JOB_STOPTIMEOUT) ) {
			this.job_stop_timeout = value;
			return;
		} else if ( aKey.equals(QUEUE) ) {
			this.job_queue = value;
			return;
		} else if ( aKey.equals(PROJECT) ) {
			this.job_project = value;
			return;
		} else if ( aKey.equals(HOSTCOUNT) ) {
			this.job_hostCount = value;
			return;
		} else if ( aKey.equals(MINMEMORY) ) {
			this.job_minMemory = value;
			return;
		} else if ( aKey.equals(MAXMEMORY) ) {
			this.job_maxMemory = value;
			return;
		} else if ( aKey.equals(MAXTIME) ) {
			this.job_maxTime = value;
			return;
		} else if ( aKey.equals(MAXWALLTIME) ) {
			this.job_maxWallTime = value;
			return;
		} else if ( aKey.equals(MAXCPUTIME) ) {
			this.job_maxCpuTime = value;
			return;
		}
		throw new IllegalArgumentException(aKey);
	}

	public static RSLInfo copy(RSLInfo other) {
		RSLInfo result = new RSLInfo();
		result.job_start_timeout = other.job_start_timeout;
		result.job_stop_timeout = other.job_stop_timeout;
		result.job_rslExtensions = new Vector<String>(other.job_rslExtensions);
		result.job_queue = other.job_queue;
		result.job_project = other.job_project;
		result.job_hostCount = other.job_hostCount;
		result.job_minMemory = other.job_minMemory;
		result.job_maxMemory = other.job_maxMemory;
		result.job_maxTime = other.job_maxTime;
		result.job_maxWallTime = other.job_maxWallTime;
		result.job_maxCpuTime = other.job_maxCpuTime;
		return result;
	}

	public void reset() {
		resetAllFields();
		setDefaultParameter();
	}

	private final void resetAllFields() {
		this.job_start_timeout = null;
		this.job_stop_timeout = null;
		this.job_rslExtensions = null;
		this.job_queue = null;
		this.job_project = null;
		this.job_hostCount = null;
		this.job_minMemory = null;
		this.job_maxMemory = null;
		this.job_maxTime = null;
		this.job_maxWallTime = null;
		this.job_maxCpuTime = null;
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("job_start_timeout: " + job_start_timeout + "\n");
		sb.append("job_stop_timeout: " + job_stop_timeout + "\n");
		sb.append("job_rslExtensions: " + job_rslExtensions + "\n");
		sb.append("job_queue: " + job_queue + "\n");
		sb.append("job_project: " + job_project + "\n");
		sb.append("job_hostCount: " + job_hostCount + "\n");
		sb.append("job_minMemory: " + job_minMemory + "\n");
		sb.append("job_maxMemory: " + job_maxMemory + "\n");
		sb.append("job_maxTime: " + job_maxTime + "\n");
		sb.append("job_maxWallTime: " + job_maxWallTime + "\n");
		sb.append("job_maxCpuTime: " + job_maxCpuTime + "\n");
		return sb.toString();
	}

}
