/*
 * $RCSfile: CallbackCompressInformation.java,v $ $Revision: 1.5 $ $Date: 2007/11/27 02:27:42 $
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

import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class CallbackCompressInformation {
	// definitions 
	// tag of this node 
	protected static final String tagName = "callbackCompressInformation";
	// attributes 
	private static final String attrCallbackID = "callbackID";
	
	// callback ID 
	private int ID;
	// compressInformation 
	private CompressInformation[] compressInfo;
	
	// protocol version 
	Version version;

	/**
	 * @param callbackID
	 * @param compressInfo
	 */
	public CallbackCompressInformation(int callbackID,
	 CompressInformation[] compressInfo) {
		// set received variables into instance 
		this.ID = callbackID;
		this.compressInfo = compressInfo;
	}
	
	/**
	public CallbackCompressInformation(String xmlString,
	 int versionMajor, int versionMinor, int versionPatch)
	 throws GrpcException {
		this(XMLUtil.getNode(xmlString), versionMajor, versionMinor, versionPatch);
	}
	 */

	/**
	 * @param node
	 * @throws GrpcException
	 */
	public CallbackCompressInformation(Node node,
	 int versionMajor, int versionMinor, int versionPatch)
	 throws GrpcException {
		// set protocol version 
		this.version =
			new Version(versionMajor, versionMinor, versionPatch);

		// get information about deflate 
		this.ID = getIDValue(node, attrCallbackID);

		// parse all of arguments 
		int nElems = XMLUtil.countChildNode(
			node, SessionInformation.namespaceURI, CompressInformation.tagName);
		this.compressInfo = new CompressInformation[nElems];
		for (int i = 0; i < compressInfo.length; i++) {
			this.compressInfo[i] = 
				 new CompressInformation(
					XMLUtil.getChildNode(node, SessionInformation.namespaceURI,
						CompressInformation.tagName, i), version);
		}
	}
	
	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private int getIDValue(Node node, String target) throws GrpcException {
		String targetValue = XMLUtil.getAttributeValue(node, target);
		return Integer.parseInt(targetValue);
	}
	
	/**
	 * @param callbackID
	 * @return
	 */
	protected CompressInformation getCompressInformation(int callbackID) {
		if (callbackID > this.compressInfo.length) {
			return null;
		}
		return this.compressInfo[callbackID];
	}
	
	/**
	 * @param callbackID
	 * @param compressInfo
	 * @return
	 */
	protected void setCompressInformation(int callbackID,
	 CompressInformation compressInfo) {
		if (callbackID > this.compressInfo.length)  return ; // do nothing 
		this.compressInfo[callbackID] = compressInfo;
	}
	
	/**
	 * @return
	 */
	public String toXMLString() {
		StringBuffer sb = new StringBuffer();
		sb.append("<" + tagName + " " + attrCallbackID 
			+ "=\"" + this.ID + "\">\n");
		
		if (compressInfo != null) {
			/* print all of compressInformation */
			for (int i = 0; i < compressInfo.length; i++) {
				if (compressInfo[i] != null) {
					sb.append(compressInfo[i].toXMLString());
				}
			}
		}

		sb.append("</" + tagName + ">\n");

		return sb.toString();
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		// put all of data into String 
		StringBuffer sb = new StringBuffer();
		
		if (compressInfo != null) {
			for (int i = 0; i < compressInfo.length; i++) {
				if (compressInfo[i] != null) {
					sb.append("----- compressInformation about arg[" 
						+ compressInfo[i].getArgID() 
						+ "] -----\n");
					sb.append(compressInfo[i]);
					sb.append("\n");
				}
			}
		}
		
		// return String 
		return sb.toString();
	}
}
