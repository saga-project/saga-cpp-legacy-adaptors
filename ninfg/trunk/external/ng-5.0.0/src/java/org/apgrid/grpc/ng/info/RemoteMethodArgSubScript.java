/*
 * $RCSfile: RemoteMethodArgSubScript.java,v $ $Revision: 1.5 $ $Date: 2007/11/27 02:27:42 $
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
package org.apgrid.grpc.ng.info;

import org.apgrid.grpc.ng.NgExpression;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class RemoteMethodArgSubScript {

	public static final String elemSubscript = "subscript";
	
	private static final String elemSize  = "size";
	private static final String elemStart = "start";
	private static final String elemEnd   = "end";
	private static final String elemSkip  = "skip";

	private static final String namespaceURI = RemoteClassInfo.namespaceURI;
	
	private NgExpression size;
	private NgExpression start;
	private NgExpression end;
	private NgExpression skip;
	
	/**
	 * @param size
	 * @param start
	 * @param end
	 * @param skip
	 */
	public RemoteMethodArgSubScript(NgExpression size, NgExpression start,
	 NgExpression end, NgExpression skip) {
		this.size  = size;
		this.start = start;
		this.end   = end;
		this.skip  = skip;
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public RemoteMethodArgSubScript(Node node) throws GrpcException {
		// get size 
		Node sizeNode = XMLUtil.getChildNode(node, namespaceURI, elemSize);
		Node sizeExpNode =
			XMLUtil.getChildNode(sizeNode, namespaceURI, NgExpression.elemExpression);
		this.size = new NgExpression(sizeExpNode);

		// get start 
		Node startNode = XMLUtil.getChildNode(node, namespaceURI, elemStart);
		Node startExpNode =
			XMLUtil.getChildNode(startNode, namespaceURI, NgExpression.elemExpression);
		this.start = new NgExpression(startExpNode);

		// get end 
		Node endNode = XMLUtil.getChildNode(node, namespaceURI, elemEnd);
		Node endExpNode =
			XMLUtil.getChildNode(endNode, namespaceURI, NgExpression.elemExpression);
		this.end = new NgExpression(endExpNode);

		// get skip 
		Node skipNode = XMLUtil.getChildNode(node, namespaceURI, elemSkip);
		Node skipExpNode =
			XMLUtil.getChildNode(skipNode, namespaceURI, NgExpression.elemExpression);
		this.skip = new NgExpression(skipExpNode);
	}

///// Getter

	public NgExpression getEnd() {
		return end;
	}
	
	public NgExpression getSize() {
		return size;
	}
	
	public NgExpression getStart() {
		return start;
	}
	
	public NgExpression getSkip() {
		return skip;
	}
	
///// Setter

	public void setEnd(NgExpression end) {
		this.end = end;
	}
	
	public void setSize(NgExpression size) {
		this.size = size;
	}
	
	public void setStart(NgExpression start) {
		this.start = start;
	}
	
	public void setSkip(NgExpression skip) {
		this.skip = skip;
	}

	/**
	 * @return
	 */
	public String toXMLString(String prefix) {
		StringBuffer sb = new StringBuffer();
		
		sb.append("<" + prefix + ":" + elemSubscript + ">");
		sb.append("\n");

		// size 
		sb.append("<" + prefix + ":" + elemSize + ">");
		sb.append(size.toXMLString(prefix));
		sb.append("</" + prefix + ":" + elemSize + ">");
		sb.append("\n");
		
		// start 
		sb.append("<" + prefix + ":" + elemStart + ">");
		sb.append(start.toXMLString(prefix));
		sb.append("</" + prefix + ":" + elemStart + ">");
		sb.append("\n");
		
		// end 
		sb.append("<" + prefix + ":" + elemEnd + ">");
		sb.append(end.toXMLString(prefix));
		sb.append("</" + prefix + ":" + elemEnd + ">");
		sb.append("\n");
		
		// skip 
		sb.append("<" + prefix + ":" + elemSkip + ">");
		sb.append(skip.toXMLString(prefix));
		sb.append("</" + prefix + ":" + elemSkip + ">");
		sb.append("\n");
		
		sb.append("</" + prefix + ":" + elemSubscript + ">");
		sb.append("\n");

		return sb.toString();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder("[");
		// size 
		sb.append(elemSize).append("=").append(size).append(" ");
		// start 
		sb.append(elemStart).append("=").append(start).append("  ");
		// end 
		sb.append(elemEnd).append("=").append(end).append(" ");
		// skip 
		sb.append(elemSkip).append("=").append(skip);
		sb.append("]");
		return sb.toString();
	}

}
