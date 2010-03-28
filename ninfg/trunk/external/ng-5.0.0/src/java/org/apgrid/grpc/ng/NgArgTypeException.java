/*
 * $RCSfile: NgArgTypeException.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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
 * Thrown to indicate that the type of specified array is incorrect.<br>
 * 
 * Probably, the type of the actually passed arrangement differs
 * from the type defined as IDL file. 
 */
public class NgArgTypeException extends NgException {
	/** argument number */
	private int argNum;
	/** the type of actual argument */
	private String actualType;
	/** required type */
	private String requiredType;
	
	/**
	 * Creates NgArgTypeException with an Exception object.
	 * 
	 * @param e an original Exception.
	 */
	public NgArgTypeException(Exception e) {
		super(e);
	}

	/**
	 * Creates NgArgTypeException with an error message.
	 * 
	 * @param string a string which indicates error.
	 */
	public NgArgTypeException(String string) {
		super(string);
	}

	/**
	 * Creates NgArgTypeException with information of error.
	 * 
	 * @param argNum argument number
	 * @param actualType the type of actual argument
	 * @param requiredType required type
	 */
	public NgArgTypeException(int argNum, String actualType, String requiredType) {
		this.argNum = argNum;
		this.actualType = actualType;
		this.requiredType = requiredType;
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		return "Arg no." + argNum + "mismatch." +
				actualType + "supplied, while " + requiredType + "is required.";
	}
}
