/*
 * $RCSfile: NgXMLReadException.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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

import org.w3c.dom.Node;

/**
 * Thrown to indicate that something wrong was happened when
 * reading XML documents.
 */
public class NgXMLReadException extends NgException {
	/** buffer of the XML node */
	private Node node;
	
	/**
	 * Creates NgXMLReadException with the XML node.<br>
	 * The XML node was parsed when error occurred.
	 * 
	 * @param node the XML node which was parsed when error occurred.
	 */
	public NgXMLReadException(Node node) {
		this.node = node;
	}
	
	/**
	 * Creates NgXMLReadException with an error message.
	 * 
	 * @param string an error message.
	 */
	public NgXMLReadException(String string) {
		super(string);
	}
	
	/**
	 * Creates NgXMLReadException with original Exception.
	 * 
	 * @param e original Exception.
	 */
	public NgXMLReadException(Exception e) {
		super(e);
	}
	
	/**
	 * Gets the Node which was parsed when error occurred.
	 * 
	 * @return the data which was parsed when error occurred.
	 */
	public Node getNode() {
		return node;
	}
}
