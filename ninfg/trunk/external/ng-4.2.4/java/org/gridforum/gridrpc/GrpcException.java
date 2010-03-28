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
 * $RCSfile: GrpcException.java,v $ $Revision: 1.8 $ $Date: 2006/08/22 10:54:33 $
 */
package org.gridforum.gridrpc;

import java.io.PrintStream;

/**
 * Keeps information about an occurred error in Grid RPC libraries.<br>
 * If error occurred in Grid RPC libraries, an object of this class must be thrown.<br>
 * This class keeps string which describe about an error, and an Exception object
 * which was catched in Grid RPC libraries.
 */
public class GrpcException extends Exception{
	/** a string which describe about an error. */
	public String str;
	/** an Exception object which was catched in Grid RPC libraries. */
	public Exception innerException;
	/**
	 * a PrintStream object to put a message about error.<br>
	 * You can change where to put information strings by setting this variable.
	 */
	public PrintStream os = System.err;
	
	/**
	 * Creates GrpcException
	 */
	public GrpcException() {
		super(); 
	}
	
	/**
	 * Creates GrpcException with string
	 * 
	 * @param str a string which describe about an error.
	 */
	public GrpcException(String str) {
		super();
		this.str = str;
	}
	
	/**
	 * Creates GrpcException with Exception
	 * 
	 * @param e an Exception object which was catched in Grid RPC libraries.
	 */
	public GrpcException(Exception e) {
		innerException = e;
	}
	
	/** 
	 * Prints information of stackTrace to PrintStream specified by
	 * {@link org.gridforum.gridrpc.GrpcException#os}.
	 * 
	 * @see java.lang.Throwable#printStackTrace()
	 */
	public void printStackTrace() {
		if (innerException != null) {
			innerException.printStackTrace();
		}
		if (str != null) {
			os.println(str);
		}
		super.printStackTrace(os);
	}
	
	/**
	 * Prints an information string which described about an error
	 * to PrintStream specified by {@link org.gridforum.gridrpc.GrpcException#os}.
	 *  
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		if (innerException != null) {
			return "GrpcException: originalException is " + innerException.toString();
		}
		if (str != null) {
			return "GrpcException: " + str;
		}
		
		return super.toString();
	}
}
