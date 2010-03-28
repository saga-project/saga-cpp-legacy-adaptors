package com.fujitsu.arcon.servlet;

import java.util.*;

/**
 * Representation of a Gateway. This contains a list of the services provided by
 * the Gateway, both Vsites (executing full Unicore jobs) and other services
 * (ports). The set of Ports (as returned from the Gateway) includes the set of
 * Vsites.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 *  
 */
public class Gateway {

	public static class Exception extends java.lang.Exception {
		public Exception(String message) {
			super(message);
		}
	}

	/**
	 * A Gateway reachable at the given address.
	 * 
	 * @param reference
	 *            The address and contact details for the Gateway.
	 *  
	 */
	public Gateway(Reference reference) throws Gateway.Exception {
		vsites = new HashSet();
		ports = new HashSet();
		this.reference = reference;
	}

	private Reference reference;

	public Reference getReference() {
		return reference;
	}

	private Set vsites;

	public void addVsite(VsiteTh vsite) {
		vsites.add(vsite);
	}

	public void removeVsite(VsiteTh vsite) {
		vsites.remove(vsite);
	}

	private Set ports;

	public void addPort(VsiteTh port) {
		ports.add(port);
	}

	public void removePort(VsiteTh port) {
		ports.remove(port);
	}

	public Collection getVsites() {
		return vsites;
	}

	public Collection getPorts() {
		return ports;
	}

	/**
	 * Remove all entries from the Vsite list.
	 *  
	 */
	public void clearVsites() {
		vsites.clear();
	}

	/**
	 * Remove all entries from the Port list.
	 *  
	 */
	public void clearPorts() {
		ports.clear();
	}

	public String toString() {
		return "Gateway through: " + reference.toString();
	}

	/**
	 * Gateways are equal if they have the same address (Reference).
	 *  
	 */
	public boolean equals(Object o) {
		if (o instanceof Gateway) {
			return reference.equals(((Gateway) o).getReference());
		} else {
			return false;
		}
	}

	public int hashCode() {
		return reference.hashCode();
	}

}
//
//                   Copyright (c) Fujitsu Ltd 2000 - 2004
//
//                Use and distribution is subject a License.
// A copy was supplied with the distribution (see documentation or the jar
// file).
//
// This product includes software developed by Fujitsu Limited
// (http://www.fujitsu.com).
