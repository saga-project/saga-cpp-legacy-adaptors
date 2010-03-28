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
 * $RCSfile: RemoteClassInfo.java,v $ $Revision: 1.11 $ $Date: 2006/08/22 10:54:33 $
 */
package org.apgrid.grpc.ng.info;

import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.NgException;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class RemoteClassInfo {
	public static final String elemName = "class";
	
	private static final String attrName = "name";
	private static final String attrVersion = "version";
	private static final String attrNumMethods = "numMethods";
	
	private static final String tagClassAttr = "classAttribute";
	private static final String tagClassDesc = "description";
	
	private static final String attrBackend = "backend";
	private static final String attrLanguage = "language";
	
	private String name;
	private int numMethods;
	private String version;
	private List listRemoteMethodInfo;
	
	/* attribute */
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
								List listRemoteMethodInfo, String version,
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
	 */
	public RemoteClassInfo(Node node) throws GrpcException {
		/* name of this class */
		this.name = XMLUtil.getAttributeValue(node, "name");
		/* version */
		this.version = XMLUtil.getAttributeValue(node, "version");
		/* number of methods */
		this.numMethods = Integer.parseInt(
			XMLUtil.getAttributeValue(node, "numMethods"));

		/* read information about methods */
		listRemoteMethodInfo = new Vector();
		for (int i = 0; i < this.numMethods; i++) {
			listRemoteMethodInfo.add(
				new RemoteMethodInfo(XMLUtil.getChildNode(node,
				RemoteMethodInfo.elemName, i)));
		}

		/* get information of attribute */
		Node attributeNode = XMLUtil.getChildNode(node, "classAttribute");
		/* language */
		this.language = XMLUtil.getAttributeValue(attributeNode, "language");
		/* backend */
		this.backend = XMLUtil.getAttributeValue(attributeNode, "backend");
		/* description */
		Node descriptionNode =
			XMLUtil.getChildNode(attributeNode, "description");
		Node descriptionCdata =
			XMLUtil.getCdataNode(descriptionNode);
		if ((this.classDescription =
			XMLUtil.getNodeValue(descriptionCdata)) == null) {
			this.classDescription = null;
		}
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
	public int getNumMethods() {
		return numMethods;
	}
	
	/**
	 * @param methodName
	 * @return
	 */
	public RemoteMethodInfo getRemoteMethodInfo(
		String methodName) throws GrpcException {
		if ((listRemoteMethodInfo == null) || 
			(listRemoteMethodInfo.size() < 1)) {
			/* no data is available */
			return null;
		}
		
		if (methodName == null) {
			/* maybe function handle */
			return (RemoteMethodInfo) listRemoteMethodInfo.get(0);
		}
		
		/* search RemoteMethodInfo */
		for (int i = 0; i < listRemoteMethodInfo.size(); i++) {
			RemoteMethodInfo remoteMethodInfo =
				(RemoteMethodInfo) listRemoteMethodInfo.get(i);
			if (remoteMethodInfo.getName().equals(methodName) == true) {
				/* found RemoteMethodInfo */
				return remoteMethodInfo;
			}
		}
		
		/* not found RemoteMethodInfo */
		throw new NgException("Mismatched Class and Method information.");
	}
	
	/**
	 * @param methodName
	 * @return
	 * @throws GrpcException
	 */
	public int getRemoteMethodID(String methodName)
		throws GrpcException {
			if ((listRemoteMethodInfo == null) || 
				(listRemoteMethodInfo.size() < 1)) {
				/* no data is available */
				return -1;
			}
		
			if (methodName == null) {
				/* maybe function handle */
				return 0;
			}
		
			/* search RemoteMethodInfo */
			for (int i = 0; i < listRemoteMethodInfo.size(); i++) {
				RemoteMethodInfo remoteMethodInfo =
					(RemoteMethodInfo) listRemoteMethodInfo.get(i);
				if (remoteMethodInfo.getName().equals(methodName) == true) {
					/* found RemoteMethodInfo */
					return i;
				}
			}
		
			/* not found RemoteMethodInfo */
			throw new NgException(
				"Mismatched Class and Method information.");		
	}
	
	/**
	 * @return
	 */
	public List getRemoteMethodInfoList() {
		return listRemoteMethodInfo;
	}
	
	/**
	 * @return
	 */
	public String getVersion() {
		return version;
	}
	
	public String getLanguage() {
		return language;
	}
	
	/**
	 * @return
	 */
	public String getBackend() {
		return backend;
	}
	
	/**
	 * @return
	 */
	public String getClassDescription() {
		return classDescription;
	}
	
	/**
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

		/* header for RemoteClass */
		sb.append("<" + elemName + " ");
		sb.append(attrName + "=\"" + name + "\" ");
		sb.append(attrVersion + "=\"" + version + "\" ");
		sb.append(attrNumMethods + "=\"" + numMethods + "\">");
		sb.append("\n");
		
		/* methods */
		for (int i = 0; i < numMethods; i++) {
			RemoteMethodInfo rmi = (RemoteMethodInfo) listRemoteMethodInfo.get(i);
			sb.append(rmi.toXMLString());
		}
		
		/* attributes */
		sb.append(putAttributes());

		sb.append("</" + elemName + ">");

		return sb.toString();
	}
	
	/**
	 * @return
	 */
	private String putAttributes() {
		StringBuffer sb = new StringBuffer();
		
		/* attribute variables of class */
		sb.append("<" + tagClassAttr + " ");
		sb.append(attrBackend + "=\"" + backend + "\" ");
		sb.append(attrLanguage + "=\"" + language + "\">");
		sb.append("\n");

		/* description */
		sb.append("<" + tagClassDesc + ">");
		if (classDescription != null) {
			sb.append(classDescription);
		}
		sb.append("</" + tagClassDesc + ">");
		sb.append("\n");

		sb.append("</" + tagClassAttr + ">");
		sb.append("\n");

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
		/* the name of this class */
		sb.append("- " + name);
		sb.append("\n");
		
		return sb.toString();
	}
}
