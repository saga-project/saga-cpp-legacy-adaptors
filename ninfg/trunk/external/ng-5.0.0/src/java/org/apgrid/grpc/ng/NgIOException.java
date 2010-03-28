/*
 * $RCSfile: NgIOException.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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
package org.apgrid.grpc.ng;

/**
 * Thrown to indicate that something wrong was happened when
 * processing Input/Output.
 */
public class NgIOException extends NgException {
	/** buffer of the data */
	private byte[] buffer;
	
	/**
	 * Creates NgIOException with an error message and original Exception.
	 * 
	 * @param string an error message.
	 * @param e original Exception.
	 */
	public NgIOException(String string, Exception e) {
		super(string, e);
	}

	/**
	 * Creates NgIOException with original Exception.
	 * @param e original Exception.
	 */
	public NgIOException(Exception e) {
		super(e);
	}

	/**
	 * Creates NgIOException with an error message.
	 * 
	 * @param string an error message.
	 */
	public NgIOException(String string) {
		super(string);
	}

	/**
	 * Creates NgIOException with the data
	 * which was the transmission and reception when error occurred.
	 * 
	 * @param buffer the data which was the transmission
	 * and reception when error occurred.
	 */
	public NgIOException (byte[] buffer) {
		this.buffer = buffer;
	}
	
	/**
	 * Gets the data
	 * which was the transmission and reception when error occurred.
	 * 
	 * @return the data
	 * which was the transmission and reception when error occurred.
	 */
	public byte[] getBuffer() {
		return buffer;
	}
}
