package com.fujitsu.arcon.servlet;

import java.io.IOException;
import java.net.UnknownHostException;

import javax.net.ssl.SSLSocket;

/**
 * Connect to a Gateway using SSL.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 *  
 */
class SSLConnection extends Connection {

	private Reference.SSL reference;

	protected SSLConnection(Reference reference) throws Connection.Exception {
		if (reference instanceof Reference.SSL) {
			this.reference = (Reference.SSL) reference;
			connect();
		} else {
			throw new Connection.Exception("Need an SSL reference");
		}
	}

	protected void _connect() throws Connection.Exception {

		Identity identity = reference.getIdentity();

		try {
			// Connect to the remote NJS
			socket = identity.createSocket(reference.getMachineName(),
					reference.getPortNumber());

			if (Connection.doNotEncrypt()) {
				javax.net.ssl.SSLSocket ts = (javax.net.ssl.SSLSocket) socket;

				String[] enabled = new String[0];
				String[] supported = ts.getSupportedCipherSuites();
				for (int i = 0; i < supported.length; i++) {
					if (supported[i].startsWith("SSL_RSA_WITH_NULL_")) {
						String[] temp = new String[enabled.length + 1];
						System.arraycopy(enabled, 0, temp, 1, enabled.length);
						enabled = temp;
						enabled[0] = supported[i];
						System.out.println("Non-encrypting suite: "
								+ supported[i]);
					}
				}
				ts.setEnabledCipherSuites(enabled);
			}

			//            	try {
			//					SSLSession session = ((SSLSocket)socket).getSession(); // KEEP
			// THIS - validates session!
			((SSLSocket) socket).getSession();
			//					X509Certificate[] chain = session.getPeerCertificateChain();
			//					byte[] javaxx509 = chain[0].getEncoded();
			//					
			//					java.security.cert.X509Certificate cert;
			//					java.security.cert.CertificateFactory cf;
			//					
			//					cf = java.security.cert.CertificateFactory.getInstance("X.509");
			//					cert = (java.security.cert.X509Certificate)
			// cf.generateCertificate(new ByteArrayInputStream(javaxx509));
			//					System.out.println("---- Server type: "+cert.getType());
			//				}
			//				catch (CertificateEncodingException e) {
			//					
			//				}
			//				catch (CertificateException e) {
			//					
			//				}

		} catch (UnknownHostException uhex) {
			throw new Connection.Exception(
					"SSL socket creation, unknown Gateway host: "
							+ uhex.getMessage());
		} catch (IOException iex) {
			try {
				socket.close();
			} catch (java.lang.Exception ex) {
			}
			;
			throw new Connection.Exception(
					"SSL socket creation initial IO failed: "
							+ iex.getMessage());
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
