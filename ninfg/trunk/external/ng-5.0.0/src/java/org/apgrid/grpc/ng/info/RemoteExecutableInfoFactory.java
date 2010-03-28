/*
 * $RCSfile: RemoteExecutableInfoFactory.java,v $ $Revision: 1.3 $ $Date: 2007/11/27 02:27:42 $
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

import org.gridforum.gridrpc.GrpcException;
import org.apgrid.grpc.util.XMLUtil;
import org.apgrid.grpc.ng.NgXMLReadException;
import org.w3c.dom.Node;

/*
 * RemoteExecutableInfo factory
 */
public class RemoteExecutableInfoFactory {

	private RemoteExecutableInfoFactory() {
	}

	public static RemoteExecutableInfo newRemoteExecutableInfo(String xmlData)
	 throws GrpcException {

		Node node = XMLUtil.getNode(xmlData);
		if (!XMLUtil.nodeIsElementOf(node,
				RemoteExecutableInfo.namespaceURI, RemoteExecutableInfo.elemREI)) {
			throw new NgXMLReadException(
				"Invalid XML Data. Got Node name is " +
				node.getNodeName() + ", expect " +
				"{" + RemoteExecutableInfo.namespaceURI + "}" +
				RemoteExecutableInfo.elemREI);
		}

		String hostname = getData(node, RemoteExecutableInfo.elemHostname);
		String path     = getData(node, RemoteExecutableInfo.elemPath);
		String ngdir    = getData(node, RemoteExecutableInfo.elemNgdir);

		RemoteClassInfo rci = 
			RemoteClassInfoFactory.newRemoteClassInfo(xmlData);

		String classname = rci.getName();
		RemoteClassPathInfo rcpi =
			new RemoteClassPathInfo(hostname, classname, path);

		RemoteExecutableInfo rei =
			new RemoteExecutableInfo(rcpi, rci);

		return rei;
	}

	private static String getData(Node node, String element)
	 throws GrpcException {
		Node target = XMLUtil.getChildNode(node, RemoteExecutableInfo.namespaceURI, element);
		Node tmp = target.getFirstChild();
		if (tmp.getNodeType() != Node.TEXT_NODE) {
			throw new IllegalArgumentException(element + " has no TEXT node");
		}

		return tmp.getNodeValue();
	}

}


