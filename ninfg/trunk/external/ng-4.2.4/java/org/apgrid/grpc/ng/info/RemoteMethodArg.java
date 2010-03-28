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
 * $RCSfile: RemoteMethodArg.java,v $ $Revision: 1.6 $ $Date: 2006/08/22 10:54:33 $
 */
package org.apgrid.grpc.ng.info;

import org.apgrid.grpc.ng.NgParamTypes;
import org.apgrid.grpc.ng.NgXMLReadException;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class RemoteMethodArg {
	public static final String elemName = "arg";
	
	private static final String attrIOMode = "ioMode";
	private static final String attrType = "dataType";

	private int type;
	private int mode;
	private int numParams;
	private int nDims;
	private RemoteMethodArgSubScript[] argSubScript;
	private RemoteMethodInfo callbackInfo;
	
	/**
	 * @param node
	 */
	public RemoteMethodArg(Node node) throws GrpcException {
		this.mode = NgParamTypes.getModeVal(
			XMLUtil.getAttributeValue(node, "ioMode"));
		try {
			this.type = NgParamTypes.getTypeVal(
				XMLUtil.getAttributeValue(node, "dataType"));
		} catch (NumberFormatException e) {
			throw new NgXMLReadException(e);
		}
		
		/* number of dimension */
		this.nDims = XMLUtil.countChildNode(
			node, RemoteMethodArgSubScript.elemName);
		if (this.nDims == 0) {
			/* no dimensions */
			this.argSubScript = null;
		} else {
			/* set for dimensions */
			this.argSubScript = new RemoteMethodArgSubScript[this.nDims];

			/* read information of subscript */
			for (int i = 0; i < this.nDims; i++) {
				Node subscriptNode = XMLUtil.getChildNode(node,
					RemoteMethodArgSubScript.elemName, i);
				this.argSubScript[i] =
					new RemoteMethodArgSubScript(subscriptNode);
			}
		}
		
		/* create RemoteMethodInfo for callback */
		if (this.type == NgParamTypes.NG_TYPE_CALLBACK) {
			this.callbackInfo =	new RemoteMethodInfo(
				XMLUtil.getChildNode(node, RemoteMethodInfo.elemName));
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
	
	/**
	 * @return
	 */
	public int getMode() {
		return mode;
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
	public int getType() {
		return type;
	}
	
	/**
	 * @return
	 */
	public int getNDims() {
		return nDims;
	}
	
	/**
	 * @return
	 */
	public RemoteMethodInfo getCallbackInfo() {
		return callbackInfo;
	}
	
	/**
	 * @param argSubScript
	 */
	public void setArgSubScript(RemoteMethodArgSubScript[] argSubScript) {
		this.argSubScript = argSubScript;
		this.nDims = argSubScript.length;
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
	public String toXMLString() throws GrpcException {
		StringBuffer sb = new StringBuffer();
		
		sb.append("<" + elemName + " ");
		sb.append(attrIOMode + "=\"" + NgParamTypes.getModeStr(mode) + "\" ");
		sb.append(attrType + "=\"" + NgParamTypes.getTypeStr(type) + "\">");
		sb.append("\n");
		
		/* subscription */
		if (nDims > 0) {
			for (int i = 0 ; i < argSubScript.length; i++) {
				sb.append(argSubScript[i].toXMLString());
			}
		}

		/* callback */
		if (type == NgParamTypes.NG_TYPE_CALLBACK) {
			sb.append(callbackInfo.toXMLString());
		}
		
		sb.append("</" + elemName + ">");

		return sb.toString();
	}
}
