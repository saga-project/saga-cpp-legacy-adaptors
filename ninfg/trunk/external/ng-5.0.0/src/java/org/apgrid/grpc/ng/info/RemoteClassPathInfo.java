/*
 * $RCSfile: RemoteClassPathInfo.java,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:08 $
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

/*
 * This class manages Remote Path Information.
 * Remote Path Information is almost the same as <FUNCTION_INFO> section.
 */
public class RemoteClassPathInfo implements Cloneable {
	private String hostname  = null;
	private String classname = null;
	private String classpath = null;
	private String staging   = null;
	private String backend   = null;
	private String ncpus     = null;
	private String session_timeout = null;

///// Constructor

	public RemoteClassPathInfo() {
		this.hostname  = null;
		this.classname = null;
		this.classpath = null;
		this.staging   = null;
		this.backend   = null;
		this.ncpus     = null;
		this.session_timeout = null;
	}
	
	/**
	 * @param hostName
	 * @param className
	 * @param path
	 */
	public RemoteClassPathInfo(String hostName, String className, String path) {
		this.hostname  = hostName;
		this.classname = className;
		this.staging   = "false";
		this.backend   = null; // invalidate backend
		this.ncpus     = null; // invalidate mpi_nCPUs
		this.classpath = path;
		this.session_timeout = "0";
	}
	
	/**
	 * @param hostName
	 * @param className
	 * @param staging
	 * @param path
	 * @param backend
	 */
	public RemoteClassPathInfo(String hostName, String className,
	 String staging, String path, String backend, String session_timeout) {
		this.hostname  = hostName;
		this.classname = className;

		if (staging != null) {
			this.staging = staging;
		} else {
			this.staging = "false";
		}

		this.classpath = path;

		if (backend != null) {
			this.backend = backend;
		} else {
			this.backend = null; // backend is not set by default 
		}

		if (session_timeout != null) {
			this.session_timeout = session_timeout;
		} else {
			this.session_timeout = "0";
		}

		this.ncpus = null;
		
	}

///// Getter

	public String getHostname() {
		return this.hostname;
	}
	public String getClassname() {
		return this.classname;
	}
	public String getClasspath() {
		return this.classpath;
	}
	public String getStaging() {
		return this.staging;
	}
	public String getBackend() {
		return this.backend;
	}
	public String getMpiNcpus() {
		return this.ncpus;
	}
	public String getSessionTimeout() {
		return this.session_timeout;
	}

///// Setter

	public void setHostname(String hostname) {
		this.hostname = hostname;
	} 
	public void setClassname(String classname) {
		this.classname = classname;
	} 
	public void setClasspath(String path) {
		this.classpath = path;
	} 
	public void setMpiNcpus(String ncpus) {
		this.ncpus = ncpus;
	} 

	public void invalidateMpiNcpus() {
		this.ncpus = null;
	}

///// Check

	public boolean hasClasspath() {
		return classpath != null;
	}

	public boolean hasMpiNcpus() {
		return ncpus != null;
	}

	/**
	 * @param other
	 */
	public void update(RemoteClassPathInfo other) {
		String tmp = null;

		tmp = other.getHostname();
		if (tmp != null) { this.hostname = tmp; }

		tmp = other.getClassname();
		if (tmp != null) { this.classname = tmp; }

		if (other.hasClasspath()) {
			this.classpath = other.getClasspath();
		}

		tmp = other.getStaging();
		if (tmp != null) { this.staging = tmp; }

		tmp = other.getBackend();
		if (tmp != null) { this.backend = tmp; }

		if (other.hasMpiNcpus()) {
			this.ncpus = other.getMpiNcpus();
		}

		tmp = other.getSessionTimeout();
		if (tmp != null) { this.session_timeout = tmp; }

	}
	
	/**
	 * Clone method
	 * @return RemoteClassPathInfo 
	 */
	public RemoteClassPathInfo getCopy() {
		try {
			return (RemoteClassPathInfo)this.clone();
		} catch (CloneNotSupportedException e) {
			throw new Error("Assertion failure");
		}
	}

	/**
	 * @param anIndent
	 * @return
	 */
	public String toString(int anIndent) {
		char [] cb = new char[anIndent];
		for (int i = 0; i < cb.length; i++) {
			cb[i] = ' ';
		}
		String indent = new String(cb);

		StringBuffer sb = new StringBuffer();
		// print key & variables 
		sb.append(indent + "- hostName :"  + hostname  + "\n");
		sb.append(indent + "- className :" + classname + "\n");
		sb.append(indent + "- path :"      + classpath + "\n");
		sb.append(indent + "- staging :"   + staging   + "\n");
		sb.append(indent + "- backend :"   + backend   + "\n");
		sb.append(indent + "- mpi_nCPUs :" + ncpus     + "\n");
		sb.append(indent + "- session_timeout :" + session_timeout + "\n");
		return sb.toString();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("- hostName :").append(hostname).append("\n");
		sb.append("- className :").append(classname).append("\n");
		sb.append("- path :").append(classpath).append("\n");
		sb.append("- staging :").append(staging).append("\n");
		sb.append("- backend :").append(backend).append("\n");
		sb.append("- mpi_nCPUs :").append(ncpus).append("\n");
		sb.append("- session_timeout :").append(session_timeout).append("\n");
		return sb.toString();
	}

}

