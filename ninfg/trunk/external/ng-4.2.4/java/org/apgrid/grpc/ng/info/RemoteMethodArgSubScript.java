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
 * $RCSfile: RemoteMethodArgSubScript.java,v $ $Revision: 1.5 $ $Date: 2006/08/22 10:54:33 $
 */
package org.apgrid.grpc.ng.info;

import org.apgrid.grpc.ng.NgExpression;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class RemoteMethodArgSubScript {
	public static final String elemName = "subscript";
	
	private static final String tagSize = "size";
	private static final String tagStart = "start";
	private static final String tagEnd = "end";
	private static final String tagSkip = "skip";
	
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
	public RemoteMethodArgSubScript(NgExpression size,
		NgExpression start,NgExpression end, NgExpression skip) {
		this.size = size;
		this.start = start;
		this.end = end;
		this.skip = skip;
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public RemoteMethodArgSubScript(Node node) throws GrpcException {
		/* get size */
		Node sizeNode = XMLUtil.getChildNode(node, "size");
		Node sizeExpNode =
			XMLUtil.getChildNode(sizeNode, NgExpression.elemName);
		this.size = new NgExpression(sizeExpNode);

		/* get start */
		Node startNode = XMLUtil.getChildNode(node, "start");
		Node startExpNode =
			XMLUtil.getChildNode(startNode, NgExpression.elemName);
		this.start = new NgExpression(startExpNode);

		/* get end */
		Node endNode = XMLUtil.getChildNode(node, "end");
		Node endExpNode =
			XMLUtil.getChildNode(endNode, NgExpression.elemName);
		this.end = new NgExpression(endExpNode);

		/* get skip */
		Node skipNode = XMLUtil.getChildNode(node, "skip");
		Node skipExpNode =
			XMLUtil.getChildNode(skipNode, NgExpression.elemName);
		this.skip = new NgExpression(skipExpNode);
	}

	/**
	 * @return
	 */
	public NgExpression getEnd() {
		return end;
	}
	
	/**
	 * @return
	 */
	public NgExpression getSize() {
		return size;
	}
	
	/**
	 * @return
	 */
	public NgExpression getStart() {
		return start;
	}
	
	/**
	 * @return
	 */
	public NgExpression getSkip() {
		return skip;
	}
	
	/**
	 * @param end
	 */
	public void setEnd(NgExpression end) {
		this.end = end;
	}
	
	/**
	 * @param size
	 */
	public void setSize(NgExpression size) {
		this.size = size;
	}
	
	/**
	 * @param start
	 */
	public void setStart(NgExpression start) {
		this.start = start;
	}
	
	/**
	 * @param skip
	 */
	public void setSkip(NgExpression skip) {
		this.skip = skip;
	}

	/**
	 * @return
	 */
	public String toXMLString() {
		StringBuffer sb = new StringBuffer();
		
		sb.append("<" + elemName + ">");
		sb.append("\n");

		/* size */
		sb.append("<" + tagSize + ">");
		sb.append(size.toXMLString());
		sb.append("</" + tagSize + ">");
		sb.append("\n");
		
		/* start */
		sb.append("<" + tagStart + ">");
		sb.append(start.toXMLString());
		sb.append("</" + tagStart + ">");
		sb.append("\n");
		
		/* end */
		sb.append("<" + tagEnd + ">");
		sb.append(end.toXMLString());
		sb.append("</" + tagEnd + ">");
		sb.append("\n");
		
		/* skip */
		sb.append("<" + tagSkip + ">");
		sb.append(skip.toXMLString());
		sb.append("</" + tagSkip + ">");
		sb.append("\n");
		
		sb.append("</" + elemName + ">");
		sb.append("\n");

		return sb.toString();
	}
}
