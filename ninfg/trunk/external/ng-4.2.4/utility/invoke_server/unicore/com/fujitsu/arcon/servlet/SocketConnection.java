package com.fujitsu.arcon.servlet;

import java.io.IOException;
import java.net.Socket;
import java.net.SocketException;

/**
 * Connect to a Gateway using plain sockets (not useful as no Gateways listen on
 * plain sockets).
 * 
 * @author Sven van den Berghe, fujitsu
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 *  
 */

class SocketConnection extends Connection {

	private Reference.Socket reference;

	public SocketConnection(Reference reference) throws Connection.Exception {
		this.reference = (Reference.Socket) reference;
		connect();
	}

	public void _connect() throws Connection.Exception {

		try {
			// Connect to the remote NJS
			socket = new Socket(reference.getMachineName(), reference
					.getPortNumber());
		} catch (SocketException sex) {
			throw new Connection.Exception(
					"Plain socket creation attempt failed: " + sex.getMessage());
		} catch (IOException iex) {
			try {
				socket.close();
			} catch (java.lang.Exception ex) {
			}
			;
			throw new Connection.Exception(
					"Plain socket creation initial IO failed: "
							+ iex.getMessage());
		} catch (java.lang.Exception ex) {
			throw new Connection.Exception(
					"Plain socket creation, Problems getting contact details for NJS? "
							+ ex.getMessage());
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
