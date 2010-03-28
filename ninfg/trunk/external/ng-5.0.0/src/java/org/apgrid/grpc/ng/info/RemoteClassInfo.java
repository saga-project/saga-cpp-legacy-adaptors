/*
 * $RCSfile: RemoteClassInfo.java,v $ $Revision: 1.6 $ $Date: 2007/11/27 02:27:42 $
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

import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.ng.NgXMLReadException;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;


/*
 * This class manages Function Information.
 */
public class RemoteClassInfo {

	public static final String namespaceURI   = "http://ninf.apgrid.org/2006/12/RemoteClassInformation";
	public static final String elemClass      = "class";
	public static final String attrName       = "name";
	public static final String attrVersion    = "version";
	public static final String attrNumMethods = "numMethods";
	public static final String elemClassAttr   = "classAttribute";
	public static final String elemClassDesc   = "description";
	public static final String attrBackend    = "backend";
	public static final String attrLanguage   = "language";

	private String name;
	private int numMethods;
	private String version;
	private List<RemoteMethodInfo> listRemoteMethodInfo;

	// attribute 
	private String language;
	private String backend;
	private String classDescription;

	/**
	 * @param name
	 * @param numMethods
	 * @param listRemoteMethodInfo
	 * @param backend
	 */
	public RemoteClassInfo (String name, int numMethods,
	 List<RemoteMethodInfo> listRemoteMethodInfo, String version,
	 String language, String backend,
	 String classDescription) {
		this.name = name;
		this.numMethods = numMethods;
		this.listRemoteMethodInfo = listRemoteMethodInfo;
		this.version = version;
		this.language = language;
		this.backend = backend;
		this.classDescription = classDescription;
	}

	/**
	 * @param node
	 * call at ProtQueryFunctionInfoReply
	 */
	public RemoteClassInfo(Node node) throws GrpcException {

		if (!XMLUtil.nodeIsElementOf(node, namespaceURI, elemClass)) {
			throw new NgXMLReadException(
				"Invalid XML Data. Got Node name is " + node.getNodeName() +
				", expect {" + namespaceURI + "}" + elemClass);
		}

		// name of this class 
		this.name    = XMLUtil.getAttributeValue(node, attrName);
		this.version = XMLUtil.getAttributeValue(node, attrVersion);
		// number of methods 
		this.numMethods = Integer.parseInt(
			XMLUtil.getAttributeValue(node, attrNumMethods));

		// read information about methods 
		listRemoteMethodInfo = new Vector<RemoteMethodInfo>();
		for (int i = 0; i < this.numMethods; i++) {
			listRemoteMethodInfo.add(
				new RemoteMethodInfo(XMLUtil.getChildNode(node,
					namespaceURI, RemoteMethodInfo.elemMethod, i)));
		}

		// get information of attribute 
		Node attributeNode = XMLUtil.getChildNode(node, namespaceURI, elemClassAttr);
		this.language = XMLUtil.getAttributeValue(attributeNode, attrLanguage);
		this.backend = XMLUtil.getAttributeValue(attributeNode, attrBackend);
		// description 
		Node descriptionNode =
			XMLUtil.getChildNode(attributeNode, namespaceURI, elemClassDesc);
		this.classDescription = XMLUtil.getTextData(descriptionNode);
	}

	/**
	 * @param methodName
	 * @return
	 */
	public RemoteMethodInfo getRemoteMethodInfo(String methodName)
	 throws GrpcException {
		if ( isUnavailableRemoteMethodInfo() ) {
			return null; // no data is available 
		}
		
		if (methodName == null) {
			// maybe function handle 
			return listRemoteMethodInfo.get(0);
		}
		
		// search RemoteMethodInfo 
		for (int i = 0; i < listRemoteMethodInfo.size(); i++) {
			RemoteMethodInfo remoteMethodInfo =
				listRemoteMethodInfo.get(i);
			if (remoteMethodInfo.getName().equals(methodName)) {
				return remoteMethodInfo; // found RemoteMethodInfo 
			}
		}

		// not found RemoteMethodInfo 
		throw new NgException("Mismatched Class and Method information.");
	}

	private boolean isUnavailableRemoteMethodInfo() {
		return (listRemoteMethodInfo == null) ||
               (listRemoteMethodInfo.size() < 1);
	}
	
	/**
	 * @param methodName
	 * @return
	 * @throws GrpcException
	 */
	public int getRemoteMethodID(String methodName)
	 throws GrpcException {
		if ( isUnavailableRemoteMethodInfo() ) {
			return -1; // no data is available 
		}
		
		if (methodName == null) {
			return 0; // maybe function handle 
		}
		
		// search RemoteMethodInfo 
		for (int i = 0; i < listRemoteMethodInfo.size(); i++) {
			RemoteMethodInfo remoteMethodInfo =
				(RemoteMethodInfo) listRemoteMethodInfo.get(i);
			if (remoteMethodInfo.getName().equals(methodName)) {
				return i; // found RemoteMethodInfo 
			}
		}
		
		// not found RemoteMethodInfo 
		throw new NgException("Mismatched Class and Method information.");
	}
	

///// Getter

	public String getName() {
		return name;
	}
	
	public int getNumMethods() {
		return numMethods;
	}

	public List<RemoteMethodInfo> getRemoteMethodInfoList() {
		return listRemoteMethodInfo;
	}
	
	public String getVersion() {
		return version;
	}
	
	public String getLanguage() {
		return language;
	}
	
	public String getBackend() {
		return backend;
	}
	
	public String getClassDescription() {
		return classDescription;
	}
	
	/**
	 * Create method
	 * @param xmlDataString
	 * @return
	 */
	public static RemoteClassInfo readClassInfo(String xmlDataString)
	 throws GrpcException {
		return new RemoteClassInfo(XMLUtil.getNode(xmlDataString));
	}
	
	/**
	 * @return
	 */
	public String toXMLString() throws GrpcException {
		StringBuffer sb = new StringBuffer();
		String prefix = "rci";

		// header for RemoteClass 
		sb.append("<" + prefix + ":" + elemClass + " ");
		sb.append("xmlns:" + prefix + "=\"" + namespaceURI+ "\" ");
		sb.append(attrName          + "=\"" + name       + "\" ");
		sb.append(attrVersion       + "=\"" + version    + "\" ");
		sb.append(attrNumMethods    + "=\"" + numMethods + "\">");
		sb.append("\n");
		
		// methods 
		for (int i = 0; i < numMethods; i++) {
			RemoteMethodInfo rmi = listRemoteMethodInfo.get(i);
			sb.append(rmi.toXMLString(prefix));
		}
		
		// attributes 
		// attribute variables of class 
		sb.append("<" + prefix + ":" + elemClassAttr + " ");
		sb.append(attrBackend + "=\"" + backend + "\" ");
		sb.append(attrLanguage + "=\"" + language + "\">");
		sb.append("\n");

		// description 
		sb.append("<" + prefix + ":" + elemClassDesc + ">");
		if (classDescription != null) {
			sb.append(classDescription);
		}
		sb.append("</" + prefix + ":" + elemClassDesc + ">");
		sb.append("\n");

		sb.append("</" + prefix + ":" + elemClassAttr + ">");
		sb.append("\n");

		sb.append("</" + prefix + ":" + elemClass + ">");
		return sb.toString();
	}
	
	/**
	 * @param indent
	 * @return
	 */
	public String toString(int indent) {
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < indent; i++) {
			sb.append(" ");
		}
		// the name of this class 
		sb.append("- " + name);
		sb.append("\n");
		
		return sb.toString();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder("RemoteClassInfo");
		sb.append("\n").append(attrName).append("=\"").append(name)
			.append("\"\n");
		sb.append(attrVersion).append("=\"").append(version).append("\"\n");
		sb.append(attrNumMethods).append("=\"").append(numMethods)
			.append("\"\n");

		// methods
		for (int i = 0; i < numMethods; i++) {
			RemoteMethodInfo rmi = listRemoteMethodInfo.get(i);
			sb.append(rmi);
		}

		// attributes
		sb.append("\n").append(elemClassAttr).append("\n");
		sb.append(attrBackend).append("=\"").append(backend).append("\"\n");
		sb.append(attrLanguage).append("=\"").append(language).append("\"\n");

		// description
		sb.append(elemClassDesc).append("\n");
		if (classDescription != null) {
			sb.append(classDescription);
		}

		return sb.toString();
	}

}

