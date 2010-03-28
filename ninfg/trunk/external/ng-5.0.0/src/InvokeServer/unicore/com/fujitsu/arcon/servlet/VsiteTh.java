package com.fujitsu.arcon.servlet;

import org.unicore.Vsite;
import org.unicore.sets.*;

/**
 * Client view of a Vsite.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 *  
 */
public class VsiteTh {

	private Reference reference;

	/**
	 * Return the Reference instance that provides access to this Vsite.
	 *  
	 */
	public Reference getReference() {
		return reference;
	}

	private void setReference(Reference reference) {
		this.reference = reference;
	}

	private String name;

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	/**
	 * New Vsite accessible through the reference.
	 *  
	 */
	public VsiteTh(Reference reference, String name) {
		setReference(reference);
		setName(name);
	}

	/**
	 * New Vsite accessible through the reference.
	 *  
	 */
	public VsiteTh(Reference reference, org.unicore.Vsite vsite) {
		setReference(reference);
		setVsite(vsite);
	}

	private ResourceSet resources;

	/**
	 * Return the resources provided by this Vsite.
	 * 
	 * @return The set of resources if obtained, otherwise null.
	 *  
	 */
	public ResourceSet getResources() {
		return resources;
	}

	public void setResources(ResourceSet resources) {
		this.resources = resources;
	}

	public String toString() {
		return name;
	}

	private org.unicore.Vsite from_gateway;

	/**
	 * Return the instance of Viste that desccribes the Vsite (this was obtained
	 * from the Gateway).
	 * 
	 * @return The Vsite instance if known, otherwise null.
	 *  
	 */
	public org.unicore.Vsite getVsite() {
		if (from_gateway == null)
			from_gateway = new Vsite(getReference().getAddress(), getName());
		return from_gateway;
	}

	public void setVsite(org.unicore.Vsite vsite) {
		from_gateway = vsite;
		setName(vsite.getName());
	}

	/**
	 * Vsites are equal if they have the same name and Reference.
	 *  
	 */
	public boolean equals(Object o) {
		if (o instanceof VsiteTh) {
			return name.equals(((VsiteTh) o).name)
					&& reference.equals(((VsiteTh) o).reference);
		} else {
			return false;
		}
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
