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
 * $RCSfile: GrpcClient.java,v $ $Revision: 1.10 $ $Date: 2006/08/22 10:54:33 $
 */
package org.gridforum.gridrpc;

import java.util.Properties;

/**
 * This interface provides functions to initialize, finalize and get/set methods for GridRPC.
 * <br>
 * It keeps information(servers, functions...) for GridRPC,
 * And has functions for process of GridRPC.<br>
 * 
 * Use {@link org.gridforum.gridrpc.GrpcClientFactory#getClient(java.lang.String)}
 * to create object which implements this interface.
 */
public interface GrpcClient {
	/**
	 * Initialize GrpcClient with a name of configuration file.
	 * 
	 * @param configFilename a name of configuration file.
	 * @throws GrpcException it it failed to initialize.
	 */
	void activate(String configFilename) throws GrpcException;
	
	/**
	 * Initialize GrpcClient with Properties object which contains information for server.
	 * 
	 * @param prop information of server.
	 * @throws GrpcException if it failed to initialize.
	 */
	void activate(Properties prop) throws GrpcException;
	
	/**
	 * Create {@link org.gridforum.gridrpc.GrpcFunctionHandle} for the specified function on the server.
	 * 
	 * @param functionName a name of function.
	 * @param prop information of server.
	 * @return {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 */
	GrpcFunctionHandle getFunctionHandle(String functionName, Properties prop) throws GrpcException;
	
	/**
	 * Create {@link org.gridforum.gridrpc.GrpcFunctionHandle} without server information.<br>
	 * Default server(it depends on implementation) will be used.
	 * 
	 * @param functionName a name of function.
	 * @return {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 */
	GrpcFunctionHandle getFunctionHandle(String functionName) throws GrpcException;
	
	/**
	 * Create specified number of {@link org.gridforum.gridrpc.GrpcFunctionHandle} for the specified function on the server.
	 * 
	 * @param functionName a name of function.
	 * @param prop information of server.
	 * @param nHandles a number of FunctionHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 */
	GrpcFunctionHandle[] getFunctionHandles(String functionName, Properties prop, int nHandles) throws GrpcException;
	
	/**
	 * Create {@link org.gridforum.gridrpc.GrpcFunctionHandle} without server information.<br>
	 * Default server(it depends on implementation) will be used.
	 * 
	 * @param functionName a name of function.
	 * @param nHandles a number of FunctionHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcFunctionHandle}.
	 * @throws GrpcException if it failed to create FunctionHandle.
	 */
	GrpcFunctionHandle[] getFunctionHandles(String functionName, int nHandles) throws GrpcException ;
	
	/**
	 * Create {@link org.gridforum.gridrpc.GrpcObjectHandle} for the specified object on the server.
	 * 
	 * @param objectName a name of object.
	 * @param prop information of server.
	 * @return {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 */
	GrpcObjectHandle getObjectHandle(String objectName, Properties prop) throws GrpcException;
	
	/**
	 * Create {@link org.gridforum.gridrpc.GrpcObjectHandle} without server information.<br>
	 * Default server(it depends on implementation) will be used.
	 * 
	 * @param objectName a name of object.
	 * @return {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 */
	GrpcObjectHandle getObjectHandle(String objectName) throws GrpcException;
	
	/**
	 * Create specified number of {@link org.gridforum.gridrpc.GrpcObjectHandle} for the specified function on the server.
	 * 
	 * @param objectName a name of object.
	 * @param prop information of server.
	 * @param nHandles a number of FunctionHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 */
	GrpcObjectHandle[] getObjectHandles(String objectName, Properties prop, int nHandles) throws GrpcException ;
	
	/**
	 * Create {@link org.gridforum.gridrpc.GrpcObjectHandle} without server information.<br>
	 * Default server(it depends on implementation) will be used.
	 * 
	 * @param objectName a name of object.
	 * @param nHandles a number of FunctionHandle.
	 * @return a list of {@link org.gridforum.gridrpc.GrpcObjectHandle}.
	 * @throws GrpcException if it failed to create ObjectHandle.
	 */
	GrpcObjectHandle[] getObjectHandles(String objectName, int nHandles) throws GrpcException ;
	
	/**
	 * Finalize GrpcClient.
	 * 
	 * @throws GrpcException if it failed to finalize.
	 */
	void deactivate() throws GrpcException;
}
