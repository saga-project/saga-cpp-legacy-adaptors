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
 * $RCSfile: NgGrpcObjectHandle.java,v $ $Revision: 1.18 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng;

import java.util.List;
import java.util.Properties;

import org.apgrid.grpc.ng.info.*;
import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcExecInfo;
import org.gridforum.gridrpc.GrpcObjectHandle;

/**
 * Ninf-G implementation of GrpcObjectHandle class.<br>
 * 
 * Provides functions defined in standard interfaces
 * and also provides Ninf-G special functions. 
 */
public class NgGrpcObjectHandle extends GrpcObjectHandle implements Cloneable {
	private NgGrpcHandle handle;

	/**
	 * Creates NgGrpcObjectHandle without any server information.<br>
	 * Default server information(described at 1st &lt SERVER_INFO &gt section
	 * in a configuration file) will be used as server.  
	 * 
	 * @param className a name of RemoteObject.
	 * @param context NgGrpcClient.
	 * @param jobID ID of Job.
	 * @param executableID ID of executable.
	 * @param jobCount a number of Job.
	 * @throws GrpcException if it failed to create a handle.
	 */
	protected NgGrpcObjectHandle(String className, NgGrpcClient context,
		int jobID, int executableID, int jobCount) throws GrpcException {
		handle = new NgGrpcHandle (className,
			context, jobID, executableID, jobCount);
	}
	
	/**
	 * Creates NgGrpcObjectHandle.
	 * 
	 * @param className a name of RemoteObject.
	 * @param prop attribute variables of server.
	 * @param context NgGrpcClient.
	 * @param jobID ID of Job.
	 * @param executableID ID of executable.
	 * @param jobCount a number of Job.
	 * @throws GrpcException if it failed to create a handle.
	 */
	protected NgGrpcObjectHandle(String className, Properties prop, NgGrpcClient context,
		int jobID, int executableID, int jobCount) throws GrpcException {
		handle = new NgGrpcHandle (className,
			prop, context, jobID, executableID, jobCount);
	}
	

	/* (non-Javadoc)
	 * @see org.gridforum.gridrpc.GrpcObjectHandle#invokeWith(java.lang.String, java.util.List)
	 */
	public GrpcExecInfo invokeWith(String methodName,
		Properties sessionAttr, List args) throws GrpcException {
		return (GrpcExecInfo)handle.startSession(methodName, (Properties)null, args);
	}
	
	/* (non-Javadoc)
	 * @see org.gridforum.gridrpc.GrpcObjectHandle#cancel()
	 */
	public void cancel() throws GrpcException {
		handle.cancel();
	}

	/* (non-Javadoc)
	 * @see org.gridforum.gridrpc.GrpcObjectHandle#dispose()
	 */
	public void dispose() throws GrpcException {
		handle.dispose();
	}
	
	/**
	 * Checks if this handle doesn't handle any jobs.
	 * 
	 * @return true if it's idle, false otherwise.
	 */
	public boolean isIdle() {
		return handle.isIdle(); 
	}
	
	/**
	 * Gets a status code of this handle.<br>
	 * A status code is defined in {@link NgGrpcHandle}.
	 * 
	 * @return a status code of this handle.
	 * @see NgGrpcHandle
	 */
	public int getLocalStatus() {
		return handle.getLocalStatus();
	}
	
	/**
	 * Sets ID of a executable.
	 * 
	 * @param executableID ID to set as executableID.
	 */
	protected void setExecutableID(int executableID) {
		handle.setExecutableID(executableID);
	}
	
	/**
	 * Gets ID of a executable associated with this handle.
	 * 
	 * @return ID of the executable.
	 */
	protected int getExecutableID() {
		return handle.getExecutableID();
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#clone()
	 */
	protected Object clone() throws CloneNotSupportedException {
		return super.clone();
	}
	
	/**
	 * Resets handle.
	 * 
	 * @throws GrpcException if it failed to reset.
	 */
	protected void resetHandle() throws GrpcException {
		try {
			handle = (NgGrpcHandle) handle.clone();
		} catch (CloneNotSupportedException e) {
			throw new NgExecRemoteMethodException(e);
		}
	}
	
	/**
	 * Gets RemoteMachineInfo for this handle.
	 * 
	 * @return {@link org.apgrid.grpc.ng.info.RemoteMachineInfo}.
	 */
	protected RemoteMachineInfo getRemoteMachineInfo() {
		return handle.getRemoteMachineInfo();
	}
	
	/**
	 * Starts CommunicationManager to manage sending/receiving Protocols.
	 * 
	 * @throws GrpcException if failed to start CommunicationManager.
	 */
	protected void startCommunicationManager() throws GrpcException {
		handle.startCommunicationManager();
	}
}
