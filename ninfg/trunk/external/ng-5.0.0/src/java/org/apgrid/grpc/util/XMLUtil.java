/*
 * $RCSfile: XMLUtil.java,v $ $Revision: 1.5 $ $Date: 2007/11/27 02:27:42 $
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
package org.apgrid.grpc.util;

import java.io.IOException;
import java.io.StringReader;

import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.apgrid.grpc.ng.NgXMLReadException;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

/**
 * Provides utility functions for parsing XML.
 */
public class XMLUtil {
	/**
	 * Node is element whose namespace is "namespace" and localname is "localname"?
	 * 
	 * @param node XML node.
	 * @param namespace Namespace
	 * @param localname Local name
	 * @return Node is element whose namespace is "namespace" and localname is "localname"?
	 */
	public static boolean nodeIsElementOf(Node node, String namespace, String localname) {

		if (node.getNodeType() != Node.ELEMENT_NODE) {
			return false;
		}
		String ns = node.getNamespaceURI();

		if (((ns != null) && (namespace != null) && (ns.equals(namespace))) ||
		    ((ns == null) && (namespace == null))) {
		} else {
			return false;
		}

		String ln = node.getLocalName();
		if (!ln.equals(localname)) {
			return false;
		}
		return true;
	}

	/**
	 * Gets a CDATA node from the specified node.
	 *  
	 * @param node the specified node.
	 * @return a CDATA node node.
	 * @throws NgXMLReadException if can't find CDATA node.
	 */
	public static Node getCdataNode(Node node) throws NgXMLReadException {
		NodeList list = node.getChildNodes();
		int n = list.getLength();
		if (n == 0) {
			if (node.getNodeType() == Node.TEXT_NODE) {
				return node;
			}
		} else  {
			for (int i = 0; i < n; i++) {
				Node tmp = list.item(i);
				if (tmp.getNodeType() == Node.TEXT_NODE) {
					if (tmp.getNodeValue().trim().length() >= 0) {
						return tmp;
					}
				}
			}
		}
		throw new NgXMLReadException("Can't find cdata node , in " + node);
	}

	/**
	 * Gets a TEXT node value from the specified Element node.
	 *
	 * @param element a ELEMENT Node has TEXT node 
	 */
	/* TODO: Child? */
	public static String getTextData(Node element) {
		if (element.getNodeType() != Node.ELEMENT_NODE) {
			throw new IllegalArgumentException(element 
				+ " is not ELEMENT node");
		}
		Node description = element.getNextSibling();
		if (description.getNodeType() != Node.TEXT_NODE) {
			throw new IllegalArgumentException(description
				+ " is not TEXT node");
		}
		return description.getNodeValue();
	}

	/**
	 * Gets specified child node from the specified node.<br>
	 * If there are several nodes which have same name,
	 * the node which was appeared at 1st will be returned.<br>
	 * This methods will not throw any Exception.
	 * 
	 * @param node the specified node.
	 * @param name a name of the child node.
	 * @return the child node when it is found, null otherwise.
	 */
	public static Node getChildNodeGentle(Node node, String namespace, String localname) {
		int count = 0;
		NodeList list = node.getChildNodes();
		for (int i = 0; i < list.getLength(); i++) {
			Node tmp = list.item(i);
			if (nodeIsElementOf(tmp, namespace, localname)) {
				return tmp;
			}
		}
		return null;
	}

	/**
	 * Gets specified child node from the specified node.<br>
	 * If there are several nodes which have same name,
	 * the node which was appeared at 1st will be returned.
	 * 
	 * @param node the specified node.
	 * @param name a name of the child node.
	 * @return the child node.
	 * @throws NgXMLReadException if there are no specified node.
	 */
	public static Node getChildNode(Node node, String namespace, String localname)
		throws NgXMLReadException {

		return getChildNode(node, namespace, localname, 0);
	}

	/**
	 * Gets specified child node from the specified node.<br>
	 * If there are several nodes which have same name,
	 * the node which was appeared in specified index will be returned.<br>
	 * 
	 * @param node the specified node.
	 * @param name a name of the child node.
	 * @param occur specified index.
	 * @return the child node.
	 * @throws NgXMLReadException if there are no specified node.
	 */
	public static Node getChildNode(Node node, String namespace, String localname, int occur)
		throws NgXMLReadException {

		int count = 0;
		NodeList list = node.getChildNodes();
		for (int i = 0; i < list.getLength(); i++) {
			Node tmp = list.item(i);
			if (nodeIsElementOf(tmp, namespace, localname)) {
				if (count < occur) {
					count++;
				} else {
					return tmp;
				}
			}
		}
		throw new NgXMLReadException
			("Can't find node [{" + namespace + "}" + localname + "], in " + node );
	}


	/**
	 * Counts the number of a child node.
	 * 
	 * @param node the specified node.
	 * @param name a name of the child node.
	 * @return the number of a child node.
	 * @throws NgXMLReadException if there are no specified node.
	 */
	public static int countChildNode(Node node, String namespace, String localname)
		throws NgXMLReadException {

		int count = 0;
		NodeList list = node.getChildNodes();
		for (int i = 0; i < list.getLength(); i++) {
			Node tmp = list.item(i);
			if (nodeIsElementOf(tmp, namespace, localname)) {
				count++;
			}
		}
		return count;
	}

	/**
	 * Gets attribute variables of the specified node.
	 * 
	 * @param node the specified node.
	 * @param name a name of the child node.
	 * @return attribute variables of the specified node.
	 * @throws NgXMLReadException if there are no attribute variable.
	 */
	public static String getAttributeValue(Node node, String name)
		throws NgXMLReadException {

		NamedNodeMap map = node.getAttributes();
		if (map == null) {
			throw new NgXMLReadException("node " + node + " has no attribute");
		}
		Node attr = map.getNamedItemNS(null, name);
		if (attr == null) {
			throw new NgXMLReadException(
				"node " + node + " has no [" + name+ "] attribute ");
		}
		return attr.getNodeValue();
	}

	/**
	 * Gets XML node from specified string.
	 * 
	 * @param str XML string.
	 * @return XML node.
	 * @throws NgXMLReadException if there are any error when it's parsing XML string.
	 */
	public static Node getNode(String str) throws NgXMLReadException {
		DocumentBuilder dbuilder;
		Document d;
		try {
			DocumentBuilderFactory dbfactory =
				DocumentBuilderFactory.newInstance();
			dbfactory.setNamespaceAware(true);
			dbuilder = dbfactory.newDocumentBuilder();	
			d = dbuilder.parse(new InputSource(new StringReader(str)));
		} catch (ParserConfigurationException e) {
			throw new NgXMLReadException(e);
		} catch (IOException e) {
			throw new NgXMLReadException(e);
		} catch (SAXException e) {
			e.printStackTrace();
			throw new NgXMLReadException(e);
		}
		Node node = d.getDocumentElement();
		return node;
	}
	
	/**
	 * Gets variable of the specified node.
	 * 
	 * @param node the specified node.
	 * @return variable of the specified node.
	 */
	public static String getNodeValue(Node node) {
		return node.getNodeValue();
	}

	/**
	 * Prints the specified document.
	 * 
	 * @param d the specified document.
	 */
	private static void printDocument(Document d) {
		printNode(d.getDocumentElement(), 0);
	}

	/**
	 * Prints variables of TEXT nodes.
	 * 
	 * @param node a parent node.
	 * @param level a number of indent.
	 */
	private static void printTextNode(Node node, int level) {
		String tmp = node.getNodeValue();
		tmp = tmp.trim();
		if (tmp.length() == 0) {
			return;
		}
		for (int i = 0; i < level; i++) {
			System.out.print("  ");
		}
		System.out.println("[" + tmp + "]");
	}

	/**
	 * Prints attribute variables of the specified node.
	 * 
	 * @param node the specified node.
	 * @param level a number of indent.
	 */
	private static void printAttribute(Node node, int level) {
		for (int i = 0; i < level; i++) {
			System.out.print("  ");
		}
		System.out.println("<" + node.getNodeName() +
			" = " + node.getNodeValue() + ">");
	}

	/**
	 * Prints elements of the specified node.
	 * 
	 * @param node the specified node.
	 * @param level a number of indent.
	 */
	private static void printElement(Node node, int level) {
		for (int i = 0; i < level; i++) {
			System.out.print("  ");
		}
		System.out.print("{" + node.getNamespaceURI() + "}");
		System.out.println(node.getNodeName());
		NamedNodeMap attrs = node.getAttributes();
		if (attrs != null) {
			for (int i = 0; i < attrs.getLength(); i++) {
				printNode(attrs.item(i), level + 1);
			}
		}

		NodeList children = node.getChildNodes();
		int n = children.getLength();
		for (int i = 0; i < n; i++) {
			Node child = children.item(i);
			printNode(child, level+1);
		}
	}

	/**
	 * Prints information of the specified node.
	 * 
	 * @param node the specified node.
	 * @param level a number of indent.
	 */
	public static void printNode(Node node, int level) {
		if (node == null) return;
		switch (node.getNodeType()) {
			case Node.ATTRIBUTE_NODE:
				printAttribute(node, level);
				break;
			case Node.TEXT_NODE:
				printTextNode(node, level);
				break;
			default:
				printElement(node, level);
		}
	}
}
