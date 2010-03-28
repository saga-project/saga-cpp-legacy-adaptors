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
 * $RCSfile: GrpcExecInfo.java,v $ $Revision: 1.5 $ $Date: 2004/01/22 02:08:37 $
 */
package org.gridforum.gridrpc;

/**
 * Provides information about time to call RemoteFunctions or RemoteMethods.<br>
 * This class provides following informations.<br>
 * 
 * <dl>
 * <li> the time to search information of RemoteFunction/RemoteMethod.
 * <li> the time to invoke a FunctionHandle/ObjectHandle.
 * <li> the time to transfer data of arguments.
 * <li> the time to execute RemoteFunction/RemoteMethod.
 * <li> the time to transfer data of results.
 * </dl>
 */
public interface GrpcExecInfo {
	/**
	 * Gets the time to search information of RemoteFunction/RemoteMethod.
	 *  
	 * @return the time as double in second.
	 */
	public double getLookupTime();

	/**
	 * Gets the time to invoke a FunctionHandle/ObjectHandle.
	 * 
	 * @return the time as double in second.
	 */
	public double getInvokeTime();

	/**
	 * Gets the time to transfer data of arguments.
	 * 
	 * @return the time as double in second.
	 */
	public double getForeTime();

	/**
	 * Gets the time to execute RemoteFunction/RemoteMethod.
	 * 
	 * @return the time as double in second.
	 */
	public double getExecTime();

	/**
	 * Gets the time to transfer data of results.
	 * 
	 * @return the time as double in second.
	 */
	public double getBackTime();
}
