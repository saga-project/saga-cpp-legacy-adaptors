/**
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
 * $RCSfile: NgExecRemoteMethodException.java,v $ $Revision: 1.7 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng;

/**
 * Thrown to indicate that an error occurred
 * when executing RemoteFunction/RemoteMethod.<br>
 */
public class NgExecRemoteMethodException extends NgException {
	/**
	 * Creates NgExecRemoteMethodException with an error message.
	 * 
	 * @param string an error message.
	 */
	public NgExecRemoteMethodException(String string) {
		super(string);
	}

	/**
	 * Creates NgExecRemoteMethodException with original Exception.
	 * 
	 * @param e original Exception.
	 */
	public NgExecRemoteMethodException(Exception e) {
		super(e);
	}
}