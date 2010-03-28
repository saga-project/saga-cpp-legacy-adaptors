/*
 * $RCSfile: RemoteMethodInfo.java,v $ $Revision: 1.7 $ $Date: 2007/11/27 02:27:42 $
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

import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.NgExpression;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class RemoteMethodInfo {
	
	public static final String elemMethod         = "method";
	public static final String DEFAULT_METHODNAME = "__default__";
	private static final String attrName          = "name";
	private static final String attrID            = "id";
	private static final String elemMethodAttr     = "methodAttribute";
	private static final String attrShrink        = "shrink";
	private static final String elemCalcOrder     = "calculationOrder";
	private static final String elemDescription   = "description";
	private static final String namespaceURI      = RemoteClassInfo.namespaceURI;
	
	private int ID;
	private String name;
	private int numParams;
	private List<RemoteMethodArg> args;
	
	// attribute
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
	public RemoteMethodInfo(int ID, String name, int numParams,
	 List<RemoteMethodArg> args,
	 NgExpression order, boolean shrink, String methodDescription) {
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
		this.name = XMLUtil.getAttributeValue(node, attrName);
		this.ID   = Integer.parseInt(XMLUtil.getAttributeValue(node, attrID));
		// number of arguments 
		this.numParams = XMLUtil.countChildNode(node, namespaceURI, RemoteMethodArg.elemArg);
		
		// read information of arguments 
		args = new Vector<RemoteMethodArg>();
		for (int i = 0; i < this.numParams; i++) {
			args.add(
				new RemoteMethodArg(XMLUtil.getChildNode(
					node, namespaceURI, RemoteMethodArg.elemArg, i)));
		}
		
		// read attribute 
		Node attributeNode = XMLUtil.getChildNode(node, namespaceURI, elemMethodAttr);
		// shrink 
		this.shrink =
			XMLUtil.getAttributeValue(attributeNode, attrShrink)
					.equalsIgnoreCase("yes");

		// order 
		Node calcOrderNode =
			XMLUtil.getChildNode(attributeNode, namespaceURI, elemCalcOrder);
		Node expressionNode =
			XMLUtil.getChildNode(calcOrderNode, namespaceURI, NgExpression.elemExpression);
		this.order = new NgExpression(expressionNode);
		
		// description 
		Node descriptionNode = 
			XMLUtil.getChildNode(attributeNode, namespaceURI, elemDescription);
		this.methodDescription = XMLUtil.getTextData(descriptionNode);
	}

///// Getter

	public List<RemoteMethodArg> getArgs() { return args; }
	
	public int getID() { return ID; }
	
	public String getName() { return name; }
	
	public int getNumParams() { return numParams; }
	
	public NgExpression getOrder() { return order; }
	
	public boolean getShrink() { return shrink; }

	public String getMethodDescription() { return methodDescription; }

///// Setter

	/**
	 * @param boolShrink
	 */
	public void setShrink(boolean boolShrink) {
		this.shrink = boolShrink;
	}


	/**
	 * @return
	 */
	public String toXMLString(String prefix) throws GrpcException {
		StringBuffer sb = new StringBuffer();
		sb.append("<" + prefix + ":" + elemMethod + " ");
		sb.append(attrName + "=\"" + name + "\" ");
		sb.append(attrID   + "=\"" + ID   + "\">");
		sb.append("\n");
		// information of arguments 
		for (int i = 0; i < numParams; i++) {
			RemoteMethodArg arg = args.get(i);
			sb.append(arg.toXMLString(prefix));
			sb.append("\n");
		}
		// put information of attribute 
		// attribute for this method 
		sb.append("<" + prefix + ":" + elemMethodAttr + " ");
		sb.append(attrShrink + "=\"" + shrink + "\">");
		sb.append("\n");
		// order of this calculation 
		sb.append("<" + prefix + ":" + elemCalcOrder + ">");
		sb.append("\n");
		sb.append(order.toXMLString(prefix));
		sb.append("\n");
		sb.append("</" + prefix + ":" + elemCalcOrder + ">");
		sb.append("\n");
		// description 
		sb.append("<" + prefix + ":" + elemDescription + ">");
		sb.append(methodDescription);
		sb.append("</" + prefix + ":" + elemDescription + ">");
		sb.append("\n");
		sb.append("</" + prefix + ":" + elemMethodAttr + ">");
		sb.append("\n");
		sb.append("</" + prefix + ":" + elemMethod + ">");
		sb.append("\n");
		return sb.toString();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder("RemoteMethodInfo\n");
		sb.append("ID ").append(ID).append("\n");
		sb.append("name ").append(name).append("\n");
		sb.append("numParams ").append(numParams).append("\n");
		sb.append("args ").append(args).append("\n");
		sb.append("order ").append(order).append("\n");
		sb.append("shrink ").append(shrink).append("\n");
		sb.append("method Description ").append(methodDescription);
		return sb.toString();
	}

}
