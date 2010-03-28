/*
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
 * $RCSfile: NgGrpcHandleAttr.java,v $ $Revision: 1.5 $ $Date: 2005/07/29 15:35:37 $
 */
package org.apgrid.grpc.ng;

import java.util.Properties;

import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;

/**
 * Provides strings which is used as key of attribute variables of server.
 */
public class NgGrpcHandleAttr {
	/** server name */
	public static final String KEY_HOSTNAME = RemoteMachineInfo.KEY_HOSTNAME;

	/** port number */
	public static final String KEY_PORT = RemoteMachineInfo.KEY_PORT;

	/** RemoteFunction/RemoteObject name */
	public static final String KEY_CLASSNAME =
		RemoteClassPathInfo.KEY_CLASS_PATH_CLASSNAME;

	/** timout of job start */
	public static final String KEY_STARTTIMEOUT =
		RemoteMachineInfo.KEY_JOB_STARTTIMEOUT;

	/** timeout of job stop */
	public static final String KEY_STOPTIMEOUT =
		RemoteMachineInfo.KEY_JOB_STOPTIMEOUT;
	
	/** number of MPI CPUs */
	public static final String KEY_MPI_NCPUS =
		RemoteMachineInfo.KEY_MPI_NCPUS;
	
	/** name of queue */
	public static final String KEY_QUEUENAME =
		RemoteMachineInfo.KEY_QUEUE;
	
	/** name of jobmanager */
	public static final String KEY_JOBMANAGER =
		RemoteMachineInfo.KEY_JOBMANAGER;
	
	/** name of subject */
	public static final String KEY_SUBJECT =
		RemoteMachineInfo.KEY_SUBJECT;
	
	/**
	 * @param prop
	 * @return
	 */
	public static Properties convertNumOfCPUs(Properties prop) {
		Properties propNCPUs = new Properties();
		propNCPUs.put("", prop.get(KEY_MPI_NCPUS));
		prop.put(KEY_MPI_NCPUS, propNCPUs);
		
		return prop;
	}
}
