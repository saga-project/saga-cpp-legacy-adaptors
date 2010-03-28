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
 * $RCSfile: NgArgSizeException.java,v $ $Revision: 1.6 $ $Date: 2004/01/23 14:55:52 $
 */
package org.apgrid.grpc.ng;

/**
 * Thrown to indicate that the size of specified array is incorrect.<br>
 * 
 * Probably, the size of the actually passed arrangement differs
 * from the size defined as IDL file. 
 */
public class NgArgSizeException extends NgException {
	/** size of the array. */
	private int size;
	/** the number of the array tried in order to access.  */
	private int accessing;
	/** the index number of depth of the array. */
	private int dimension;
	
	/**
	 * Creates NgArgSizeException with an error message.
	 * 
	 * @param string a string which indicates error.
	 */
	public NgArgSizeException(String string) {
		super(string);
	}

	/**
	 * Creates NgArgSizeException with information of error.
	 * 
	 * @param size size of the array.
	 * @param accessing the number of the array tried in order to access.
	 * @param dimension the index number of depth of the array.
	 */
	public NgArgSizeException(int size, int accessing, int dimension) {
		this.size = size;
		this.accessing = accessing;
		this.dimension = dimension;
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		return "dim." + dimension + "access error." +
				"Accessing" + accessing + "th elem, while the size is " + size + ".";
	}
}
