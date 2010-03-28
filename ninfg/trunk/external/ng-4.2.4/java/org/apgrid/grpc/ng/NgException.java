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
 * $RCSfile: NgException.java,v $ $Revision: 1.6 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng;

import org.gridforum.gridrpc.GrpcException;

/**
 * Thrown to indicate that something wrong was happened in Ninf-G Java library.<br>
 * 
 * This is the super class of Exception classes
 * which included in this package.
 */
public class NgException extends GrpcException {
	/**
	 * Creates NgException with an error message and original Exception.
	 * 
	 * @param string an error message.
	 * @param e original Exception.
	 */
	public NgException(String string, Exception e) {
		super(e);
		str = string;
	}

	/**
	 * Creates NgException.
	 */
	public NgException() {
		super();
	}

	/**
	 * Creates NgException with original Exception.
	 * 
	 * @param e original Exception.
	 */
	public NgException(Exception e) {
		super(e);
	}
	
	/**
	 * Creates NgException with an error message.
	 * 
	 * @param string an error message.
	 */
	public NgException(String string) {
		super(string);
	}
}
