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
 * $RCSfile: RemoteMethodInfo.java,v $ $Revision: 1.7 $ $Date: 2005/04/01 09:27:12 $
 */
package org.apgrid.grpc.ng.info;

import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.NgExpression;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class RemoteMethodInfo {
	public static final String elemName = "method";
	
	private static final String attrName = "name";
	private static final String attrID = "id";
	private static final String tagMethodAttr = "methodAttribute";
	private static final String attrShrink = "shrink";
	private static final String tagCalcOrder = "calculationOrder";
	private static final String tagDescription = "description";
	
	public static final String DEFAULT_METHODNAME = "__default__";
	
	private int ID;
	private String name;
	private int numParams;
	private List args;
	
	/* attribute */
	private NgExpression order;
	private boolean shrink;
	private String methodDescription;
	
	/**
	 * @param ID
	 * @param name
	 * @param numParams
	 * @param args
	 * @param order
	 * @param shrink
	 * @param methodDescription
	 */
	public RemoteMethodInfo(int ID, String name, int numParams, List args,
								NgExpression order, boolean shrink,
								String methodDescription) {
		this.ID = ID;
		this.name = name;
		this.numParams = numParams;
		this.args = args;
		this.order = order;
		this.shrink = shrink;
		this.methodDescription = methodDescription;
	}
	
	/**
	 * @param node
	 * @throws GrpcException
	 */
	public RemoteMethodInfo(Node node) throws GrpcException {
		/* name */
		this.name = XMLUtil.getAttributeValue(node, attrName);
		/* ID */
		this.ID = Integer.parseInt(XMLUtil.getAttributeValue(node, attrID));
		/* number of arguments */
		this.numParams = XMLUtil.countChildNode(node, "arg");
		
		/* read information of arguments */
		args = new Vector();
		for (int i = 0; i < this.numParams; i++) {
			args.add(new RemoteMethodArg(
				XMLUtil.getChildNode(node, "arg", i)));
		}
		
		/* read attribute */
		Node attributeNode = XMLUtil.getChildNode(node, tagMethodAttr);
		/* shrink */
		this.shrink =
			XMLUtil.getAttributeValue(attributeNode, attrShrink).equalsIgnoreCase("yes");

		/* order */
		Node calcOrderNode = XMLUtil.getChildNode(
			attributeNode, tagCalcOrder);
		Node expressionNode =
			XMLUtil.getChildNode(calcOrderNode, NgExpression.elemName);
		this.order = new NgExpression(expressionNode);
		
		/* description */
		Node descriptionNode = 
			XMLUtil.getChildNode(attributeNode, tagDescription);
		Node descriptionCdata = XMLUtil.getCdataNode(descriptionNode);
		if ((this.methodDescription =
			descriptionCdata.getNodeValue()) == null) {
			this.methodDescription = null;
		}
	}
	
	/**
	 * @return
	 */
	public List getArgs() {
		return args;
	}
	
	/**
	 * @return
	 */
	public int getID() {
		return ID;
	}
	
	/**
	 * @return
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * @return
	 */
	public int getNumParams() {
		return numParams;
	}
	
	/**
	 * @return
	 */
	public NgExpression getOrder() {
		return order;
	}
	
	/**
	 * @return
	 */
	public boolean getShrink() {
		return shrink;
	}
	
	/**
	 * @param boolShrink
	 */
	public void setShrink(boolean boolShrink) {
		this.shrink = boolShrink;
	}
	
	/**
	 * @return
	 */
	public String getMethodDescription() {
		return methodDescription;
	}
	
	/**
	 * @return
	 */
	public String toXMLString() throws GrpcException {
		StringBuffer sb = new StringBuffer();
		
		sb.append("<" + elemName + " ");
		sb.append(attrName + "=\"" + name + "\" ");
		sb.append(attrID + "=\"" + ID + "\">");
		sb.append("\n");
		
		/* information of arguments */
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg arg = (RemoteMethodArg) args.get(i);
			sb.append(arg.toXMLString());
			sb.append("\n");
		}

		/* put information of attribute */
		sb.append(printAttributes());
		
		sb.append("</" + elemName + ">");
		sb.append("\n");

		return sb.toString();
	}
	
	/**
	 * @return
	 */
	private String printAttributes() {
		StringBuffer sb = new StringBuffer();

		/* attribute for this method */
		sb.append("<" + tagMethodAttr + " ");
		sb.append(attrShrink + "=\"" + shrink + "\">");
		sb.append("\n");

		/* order of this calculation */
		sb.append("<" + tagCalcOrder + ">");
		sb.append("\n");
		sb.append(order.toXMLString());
		sb.append("\n");
		sb.append("</" + tagCalcOrder + ">");
		sb.append("\n");

		/* description */
		sb.append("<" + tagDescription + ">");
		sb.append(methodDescription);
		sb.append("</" + tagDescription + ">");
		sb.append("\n");

		sb.append("</" + tagMethodAttr + ">");
		sb.append("\n");
		
		return sb.toString();
	}
}
