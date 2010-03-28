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
 * $RCSfile: GrpcClientFactory.java,v $ $Revision: 1.6 $ $Date: 2004/01/22 02:08:37 $
 */
package org.gridforum.gridrpc;

/**
 * Create object which implements {@link org.gridforum.gridrpc.GrpcClient} interface.
 */
public class GrpcClientFactory {
	/**
	 * Create object which implements {@link org.gridforum.gridrpc.GrpcClient} interface.
	 * 
	 * @param className the name of class to create.
	 * @return GrpcClient.
	 * @throws GrpcException if it failed to create GrpcClient.
	 */
	static public GrpcClient getClient(String className) throws GrpcException {
		try {
			Class clientClass = Class.forName(className);
			Object client = clientClass.newInstance();
			if (client instanceof GrpcClient) {
				return (GrpcClient) client;
			}
			throw new GrpcException("The specified class " + className +
										" was not subclass of GrpcClient ");
		} catch (Exception e){
			throw new GrpcException(e);
		}
	}
}
