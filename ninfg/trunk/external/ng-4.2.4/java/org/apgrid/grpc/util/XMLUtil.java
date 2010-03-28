/*
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
 * $RCSfile: XMLUtil.java,v $ $Revision: 1.3 $ $Date: 2004/01/27 08:08:09 $
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
import org.apache.xerces.parsers.DOMParser;
import org.apgrid.grpc.ng.NgXMLReadException;

/**
 * Provides utility functions for parsing XML.
 */
public class XMLUtil {
	/**
	 * Gets a CDATA node from the specified node.
	 *  
	 * @param node the specified node.
	 * @return a CDATA node node.
	 * @throws NgXMLReadException if can't find CDATA node.
	 */
	public static Node getCdataNode(Node node)
		throws NgXMLReadException {

		NodeList list = node.getChildNodes();
		int N = list.getLength();
		if (N == 0)
			return node;
		else 
			for (int i = 0; i < N; i++) {
			Node tmp = list.item(i);
			if (tmp.getNodeType() == Node.TEXT_NODE) {
				if (tmp.getNodeValue().trim().length() >= 0)
				return tmp;
			}
			}
		throw new NgXMLReadException
			("can't find cdata node , in " + node);
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
	public static Node getChildNodeGentle(Node node, String name) {
		int count = 0;
		NodeList list = node.getChildNodes();
		for (int i = 0; i < list.getLength(); i++) {
			Node tmp = list.item(i);
			if (tmp.getNodeName().equals(name))
				return tmp;
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
	public static Node getChildNode(Node node, String name)
		throws NgXMLReadException {

		return getChildNode(node, name, 0);
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
	public static Node getChildNode(Node node, String name, int occur)
		throws NgXMLReadException {

		int count = 0;
		NodeList list = node.getChildNodes();
		for (int i = 0; i < list.getLength(); i++) {
			Node tmp = list.item(i);
			if (tmp.getNodeName().equals(name) && occur <= count++)
				return tmp;
		}
		throw new NgXMLReadException
			("Can't find node [" + name + "], in " + node );
	}

	/**
	 * Counts the number of a child node.
	 * 
	 * @param node the specified node.
	 * @param name a name of the child node.
	 * @return the number of a child node.
	 * @throws NgXMLReadException if there are no specified node.
	 */
	public static int countChildNode(Node node, String name)
		throws NgXMLReadException {

		int count = 0;
		NodeList list = node.getChildNodes();
		for (int i = 0; i < list.getLength(); i++) {
			Node tmp = list.item(i);
			if (tmp.getNodeName().equals(name))
				count++;
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
		if (map == null)
			throw new NgXMLReadException
				("node " + node + " has no attribute");
		Node attr = map.getNamedItem(name);
		if (attr == null)
			throw new NgXMLReadException
				("node " + node + " has no [" + name+ "]attribute ");
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
		StringReader reader = new StringReader(str);
		DOMParser parser = new DOMParser();
		try {
			parser.parse(new InputSource(reader));
		} catch (IOException e) {
			throw new NgXMLReadException(e);
		} catch (SAXException e) {
			e.printStackTrace();
			throw new NgXMLReadException(e);
		}
		Document d = parser.getDocument();
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
		if (tmp.length() == 0)
			return;
		for (int i = 0; i < level; i++)
			System.out.print("  ");
		System.out.println("[" + tmp + "]");
	}

	/**
	 * Prints attribute variables of the specified node.
	 * 
	 * @param node the specified node.
	 * @param level a number of indent.
	 */
	private static void printAttribute(Node node, int level) {
		for (int i = 0; i < level; i++)
			System.out.print("  ");
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
		for (int i = 0; i < level; i++)
			System.out.print("  ");
		System.out.println(node.getNodeName());
		NamedNodeMap attrs = node.getAttributes();
		if (attrs != null)
			for (int i = 0; i < attrs.getLength(); i++)
				printNode(attrs.item(i), level + 1);

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
