/*
 * $RCSfile: GrpcFunctionHandle.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:08 $
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
package org.gridforum.gridrpc;

import java.util.List;
import java.util.Properties;
import java.util.Vector;

/**
 * Provides interfaces to control RemoteFunctions.<br>
 * This class keeps information about a server and a RemoteFunction,
 * can call a RemoteFunction with arguments and receive a result of it.<br>
 * 
 * If an error occurred in this class, then it will be notified by throwing
 * {@link org.gridforum.gridrpc.GrpcException}.
 */
public abstract class GrpcFunctionHandle {
	/**
	 * Calls a RemoteFunction with specified arguments.<br>
	 * 
	 * @param args List of arguments for a RemoteFunction.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteFunction.
	 */
	public abstract GrpcExecInfo callWith(Properties sessionAttr, List args)
	 throws GrpcException;
	
	/**
	 * Cancels a current session.
	 * 
	 * @throws GrpcException if it failed to cancel.
	 */
	public abstract void cancel() throws GrpcException;
	
	/**
	 * Disposes this handle.<br>
	 * This method must be called when you don't need this handle any more.
	 * 
	 * @throws GrpcException if it failed to dispose.
	 */
	public abstract void dispose() throws GrpcException;
	
	/**
	 * Calls a RemoteFunction without any arguments.<br>
	 * 
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteFunction.
	 */
	public GrpcExecInfo call() throws GrpcException {
		/* no argument here */
		return call((Properties)null);
	}

	/**
	 * Calls a RemoteFunction with a specified argument.<br>
	 * 
	 * @param args arguments for a RemoteFunction.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteFunction.
	 */
	public GrpcExecInfo call(Object ... args) throws GrpcException{
		return call((Properties)null, args);
	}
	
	/**
	 * Calls a RemoteFunction with attributes and no arguments.<br>
	 * 
	 * @param sessionAttr attributes for the session
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteFunction.
	 */
	public GrpcExecInfo call(Properties sessionAttr) throws GrpcException {
		/* no argument here */
		Vector arg = new Vector();
		return callWith(sessionAttr, arg);
	}
	/**
	 * Calls a RemoteFunction with attributes and a specified argument.<br>
	 * 
	 * @param sessionAttr attributes for the session
	 * @param args arguments for a RemoteFunction.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteFunction.
	 */
	public GrpcExecInfo call(Properties sessionAttr, Object ... args)
	 throws GrpcException{
		Vector arg = new Vector();
		for (int i = 0; i < args.length; i++) {
			arg.addElement(args[i]);
		}
		return callWith(sessionAttr, arg);
	}

}
