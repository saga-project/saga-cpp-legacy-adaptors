package com.fujitsu.arcon.servlet;

/**
 * The details required to be able to establish a Connection to a Gateway.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 * 
 */

public abstract class Reference {

	public static class Exception extends java.lang.Exception {
		public Exception(String message) {
			super(message);
		}
	}

	/**
	 * Make connections using SSL. Thus requires an Identity.
	 *  
	 */
	public static class SSL extends Reference.Socket {
		/**
		 * A Gateway reachable via SSL at the given address as the given user.
		 *  
		 */
		public SSL(String address, Identity user) throws Reference.Exception {
			setAddress(address);
			setIdentity(user);
		}

		private Identity identity;

		public Identity getIdentity() {
			return identity;
		}

		private void setIdentity(Identity identity) {
			this.identity = identity;
		}

		public boolean equals(Object o) {
			if (o == null)
				return false;
			if (o instanceof Reference.SSL) {
				Reference.SSL r = (Reference.SSL) o;
				return address.equals(r.address)
						&& identity.getUser().equals(r.getIdentity().getUser());
			} else {
				return false;
			}
		}

		public int hashCode() {
			return address.hashCode();
		}

	}

	/**
	 * Make connections using plain sockets.
	 *  
	 */
	public static class Socket extends Reference {

		public Socket() { // SSL needs this to stop exception being thrown
		}

		/**
		 * A Gateway reachable via SSL at the given address as the given user.
		 *  
		 */
		public Socket(String address) throws Reference.Exception {
			if (setAddress(address)) {
				throw new Reference.Exception(
						"The supplied address requires SSL");
			}
		}

		public boolean equals(Object o) {
			if (o == null)
				return false;
			if (o instanceof Reference.Socket) {
				Reference.Socket r = (Reference.Socket) o;
				return address.equals(r.address);
			} else {
				return false;
			}
		}

		public int hashCode() {
			return address.hashCode();
		}
	}

	/**
	 * Set the address of the Gateway.
	 * 
	 * Assumed format = [ssl://]arcon.fle.fujitsu.com:4433
	 * 
	 * @return true If the supplied address requires SSL
	 *  
	 */
	protected boolean setAddress(String address) throws Reference.Exception {

		this.address = address;
		// Parse components of this address

		try {
			// Parse the address into a machine name and a port
			// Machine name is between the ":"s, port follows
			// second ":". Assume only 2 :": in string
			int port_start = address.lastIndexOf(':');

			boolean is_ssl = address.toLowerCase().startsWith("ssl://");

			int name_start;
			if (is_ssl) {
				name_start = "ssl://".length();
			} else {
				name_start = 0;
			}

			machine_name = address.substring(name_start, port_start);

			port = (new Integer(address.substring(port_start + 1))).intValue();

			return is_ssl;
		} catch (java.lang.Exception ex) {
			throw new Reference.Exception("Invalid reference address <"
					+ address + "> " + ex.getMessage());
		}
	}

	protected String address;

	public String getAddress() {
		return address;
	}

	protected int port;

	public int getPortNumber() {
		return port;
	}

	protected String machine_name;

	public String getMachineName() {
		return machine_name;
	}

	public String toString() {
		return address;
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
