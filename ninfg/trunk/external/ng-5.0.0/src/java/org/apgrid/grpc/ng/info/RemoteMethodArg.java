/*
 * $RCSfile: RemoteMethodArg.java,v $ $Revision: 1.6 $ $Date: 2007/11/27 02:27:42 $
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

import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.NgXMLReadException;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class RemoteMethodArg {
	public static final String elemArg = "arg";
	
	private static final String attrIOMode = "ioMode";
	private static final String attrType = "dataType";

	private static final String namespaceURI = RemoteClassInfo.namespaceURI;


	private int type; // type of argument
	private int mode; // mode of argument(IN, INOUT, OUT,...)
	private int numParams; // number of argument (size of array)
	private int nDims; // Number of Dimensions
	private RemoteMethodArgSubScript[] argSubScript;
	private RemoteMethodInfo callbackInfo;

	/**
	 * @param node
	 */
	public RemoteMethodArg(Node node) throws GrpcException {
		this.mode = NgParamTypes.getModeVal(
			XMLUtil.getAttributeValue(node, attrIOMode));
		try {
			this.type = NgParamTypes.getTypeVal(
				XMLUtil.getAttributeValue(node, attrType));
		} catch (NumberFormatException e) {
			throw new NgXMLReadException(e);
		}
		
		// number of dimension 
		this.nDims = XMLUtil.countChildNode(
			node, namespaceURI, RemoteMethodArgSubScript.elemSubscript);
		if (this.nDims == 0) {
			this.argSubScript = null; // no dimensions 
		} else {
			// set for dimensions 
			this.argSubScript = new RemoteMethodArgSubScript[this.nDims];

			// read information of subscript 
			for (int i = 0; i < this.nDims; i++) {
				Node subscriptNode = XMLUtil.getChildNode(node,
					namespaceURI, RemoteMethodArgSubScript.elemSubscript, i);
				this.argSubScript[i] =
					new RemoteMethodArgSubScript(subscriptNode);
			}
		}
		
		// create RemoteMethodInfo for callback 
		if (this.type == NgParamTypes.NG_TYPE_CALLBACK) {
			this.callbackInfo =
				new RemoteMethodInfo(XMLUtil.getChildNode(node,
						namespaceURI, RemoteMethodInfo.elemMethod));
		}
	}

	/**
	 * @return
	 */
	public RemoteMethodArgSubScript getRemoteMethodArgSubscript(int index) {
		if (argSubScript == null) {
			return null;
		}
		return argSubScript[index];
	}
	
	public int getMode() {
		return mode;
	}
	
	public int getNumParams() {
		return numParams;
	}

	public int getType() {
		return type;
	}
	
	public int getNDims() {
		return nDims;
	}
	
	public RemoteMethodInfo getCallbackInfo() {
		return callbackInfo;
	}

	/**
	 * @param mode
	 */
	public void setMode(int mode) {
		this.mode = mode;
	}
	
	/**
	 * @param numParams
	 */
	public void setNumparams(int numParams) {
		this.numParams = numParams;
	}
	
	/**
	 * @param type
	 */
	public void setType(int type) {
		this.type = type;
	}

	/**
	 * @return
	 */
	public String toXMLString(String prefix) throws GrpcException {
		StringBuffer sb = new StringBuffer();
		sb.append("<" + prefix + ":" + elemArg + " ");
		sb.append(attrIOMode + "=\"" + NgParamTypes.getModeStr(mode) + "\" ");
		sb.append(attrType   + "=\"" + NgParamTypes.getTypeStr(type) + "\">");
		sb.append("\n");

		// subscription 
		if (nDims > 0) {
			for (int i = 0 ; i < argSubScript.length; i++) {
				sb.append(argSubScript[i].toXMLString(prefix));
			}
		}

		// callback 
		if (type == NgParamTypes.NG_TYPE_CALLBACK) {
			sb.append(callbackInfo.toXMLString(prefix));
		}

		sb.append("</" + prefix + ":" + elemArg + ">");
		return sb.toString();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		try {
			sb.append(NgParamTypes.getModeStr(mode).toUpperCase()).append(" ");
			sb.append(NgParamTypes.getTypeStr(type));
		} catch (GrpcException e) {
		}

		// subscription
		if (nDims > 0) {
			for (int i = 0 ; i < argSubScript.length; i++) {
				sb.append(argSubScript[i]).append(" ");
			}
		}

		// callback 
		if (type == NgParamTypes.NG_TYPE_CALLBACK) {
			sb.append(" ").append(callbackInfo).append(" ");
		}

		return sb.toString();
	}
}

