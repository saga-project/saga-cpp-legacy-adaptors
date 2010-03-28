package com.fujitsu.arcon.servlet;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Iterator;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;

import org.unicore.User;
import org.unicore.utility.ConsignForm;

import javax.net.ssl.KeyManager;

/**
 * How a user is identified to Unicore.
 * 
 * The user's Identity is given by his private key plus the certificate issued
 * by a Unicore CA. These are loaded from a keystore (file) in either PKCS12,
 * JKS or JCEKS formats.
 * 
 * Identity instances are the source of SSL sockets to Unicore servers. This
 * means that they also maintain a list of CAs trusted to issue (server)
 * certificates.
 * 
 * @author Sven van den Berghe, fujitsu
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 *  
 */
public class Identity {

	public static class Exception extends java.lang.Exception {
		public Exception(String message) {
			super(message);
		}

		public Exception(String message, Throwable cause) {
			super(message, cause);
		}
	}

	private User user;

	/**
	 * Returns a new Identity derived from the certificate contained in the file
	 * (PKCS12, JKS or JCEKS).
	 * 
	 * This instance will be initialised for SSL.
	 * <p>
	 * This will also search for public certificates to load as trusted signers
	 * (that is the CAs that issue certificates for the Unicore servers that
	 * this client will use). All files with a ".pem" or ".der" extension in the
	 * same directory as the keystore will be loaded as trusted.
	 * <p>
	 * If the keystore contains multiple key entries, then the first one found
	 * is used.
	 * <p>
	 * Any certificate entries found in the keystore file are loaded as trusted
	 * signers.
	 * 
	 * @param keystore_file
	 *            The keystore file
	 * @param in_password
	 *            password to unlock the keystore.
	 *  
	 */
	public Identity(File keystore_file, char[] in_password)
			throws Identity.Exception {

		password = in_password;

		File abs_keystore_file = keystore_file.getAbsoluteFile();
		File cert_directory = abs_keystore_file.getParentFile();

		CLogger.status("Loading trusted CAs (.pem files) from <"
				+ cert_directory + ">");

		TrustManager[] tm;

		KeyStore ks = null;

		try {
			ks = KeyStore.getInstance("JKS");
			ks.load(null, null); // initialise, but empty

			int alias_counter = 0;

			File[] listing = cert_directory.listFiles(); // cert_directory is a
														 // directory so OK
			for (int i = 0; i < listing.length; i++) {
				if (listing[i].getName().endsWith(".pem")
						|| listing[i].getName().endsWith(".der")) {
					CLogger
							.status("Loading trusted certificate signer(s) from: "
									+ listing[i]);
					CertificateFactory cf = CertificateFactory
							.getInstance("X.509");
					Collection loaded = cf
							.generateCertificates(new FileInputStream(
									listing[i]));
					Iterator ii = loaded.iterator();
					while (ii.hasNext()) {
						Certificate cert = (Certificate) ii.next();
						ks
								.setCertificateEntry(
										"Alias_" + alias_counter++, cert);
					}
				}
			}

		} catch (java.lang.Exception ex) {
			throw new Identity.Exception("Cannot load CA certificate.", ex);
		}

		CLogger.status("Loading certificates and keys from <"
				+ abs_keystore_file + ">");

		//170903 KeyManager[] km;
		KeyManagerFactory kmf = null;

		if (abs_keystore_file.exists()) {

			// First as PKCS12
			KeyStore temp_ks;
			try {
				temp_ks = KeyStore.getInstance("PKCS12");
				try {
					temp_ks.load(new FileInputStream(abs_keystore_file),
							password);
				} catch (java.lang.Exception ex) {
					// Try again, could be JKS format
					temp_ks = KeyStore.getInstance("JKS");
					try {
						temp_ks.load(new FileInputStream(abs_keystore_file),
								password);
					} catch (java.lang.Exception exex) {
						// Try again, could be JCEKS format
						temp_ks = KeyStore.getInstance("JCEKS");
						temp_ks.load(new FileInputStream(abs_keystore_file),
								password);
					}
				}
			} catch (java.lang.Exception ex) {
				throw new Identity.Exception(
						"Invalid password or certificate corrupt", ex);
			}

			// Use the first, creating an internal version with just this one
			try {
				Enumeration e = temp_ks.aliases();
				while (e.hasMoreElements()) {
					String alias = (String) e.nextElement();
					if (kmf == null && temp_ks.isKeyEntry(alias)) {
						CLogger.status("Using the keystore alias <" + alias
								+ "> for identity");
						this_ks = KeyStore.getInstance("JKS");
						this_ks.load(null, null);
						this_ks.setKeyEntry("alias", temp_ks.getKey(alias,
								password), password, temp_ks
								.getCertificateChain(alias));
						kmf = KeyManagerFactory.getInstance("SunX509",
								"SunJSSE");
						kmf.init(this_ks, password);
					} else if (temp_ks.isCertificateEntry(alias)) {
						CLogger.status("Adding keystore alias <" + alias
								+ "> to trusted CAs.");
						ks.setCertificateEntry(alias, temp_ks
								.getCertificate(alias));
					}
				}
			} catch (java.lang.Exception ex) {
				throw new Identity.Exception("Error reading certificate.", ex);
			}
		} else {
			throw new Identity.Exception("User certificate file not found.");
		}

		if (kmf == null) {
			throw new Identity.Exception("No key entry found in <"
					+ abs_keystore_file + "> so no Identity.");
		}

		try {
			TrustManagerFactory tmf = TrustManagerFactory.getInstance(
					"SunX509", "SunJSSE");
			tmf.init(ks);
			tm = tmf.getTrustManagers();

			SSLContext context = SSLContext.getInstance("SSLv3");
			context.init(kmf.getKeyManagers(), tm,
					new java.security.SecureRandom());
			ssf = context.getSocketFactory();
		} catch (java.lang.Exception ex) {
			throw new Identity.Exception("Error sorting out trust.", ex);
		}

		CLogger.status("Done loading user certificate.");

		try {
			Certificate[] cchain = (Certificate[]) this_ks
					.getCertificateChain("alias");
			if (cchain == null) {
				user = new User(
						new X509Certificate[] { (X509Certificate) this_ks
								.getCertificate("alias") },
						"need to set an email address");
			} else {
				X509Certificate[] x509certs = new X509Certificate[cchain.length];
				for (int i = 0; i < cchain.length; i++) {
					x509certs[i] = (X509Certificate) cchain[i];
				}
				user = new User(x509certs, "need to set an email address");
			}
		} catch (java.lang.Exception ex) {
			ex.printStackTrace();
			throw new Identity.Exception("Problems generating user.", ex);
		}

	}

	/**
	 * Returns a guess of the user's name extracted from the public certificate.
	 *  
	 */
	public String getName() {
		String name = user.getName();
		int i = name.indexOf("CN=");
		if (i > 0) {
			int j = name.indexOf(',', i);
			if (j > 0) {
				return name.substring(i + 3, j);
			} else {
				return name.substring(i + 3);
			}
		} else {
			return name;
		}
	}

	/**
	 * Returns just the name (suitable for UI lists etc).
	 *  
	 */
	public String toString() {
		return getName();
	}

	/**
	 * Get the user's Identity in Unicore form. This instance is passed to NJSs
	 * as the endorser of all AJOs sent to Vsites using this Identity. This any
	 * fields set in this instance will be passed to Unicore servers.
	 *  
	 */
	public User getUser() {
		return user;
	}

	/**
	 * @deprecated Use {@link #getUser}as it is more appropriately named.
	 *  
	 */
	public User getUlogin() {
		return user;
	}

	/**
	 * A Signature to be used for signing AJOs etc.
	 *  
	 */
	public ConsignForm.SignatureFactory getSignature() {

		return new ConsignForm.SignatureFactory() {

			public Signature getInstance() {
				try {
					Signature signature;
					signature = Signature.getInstance("MD5withRSA");
					try {
						signature.initSign((java.security.PrivateKey) this_ks
								.getKey("alias", password));
					} catch (java.security.InvalidKeyException ikex) {
						// Perhaps it is DSA
						signature = Signature.getInstance("SHA1withDSA");
						signature.initSign((java.security.PrivateKey) this_ks
								.getKey("alias", password));
					}
					return signature;
				} catch (java.lang.Exception ex) {
					CLogger
							.status("Problems generating cryptographic objects.\n"
									+ ex);
					return null;
				}
			}
		};
	}

	private SSLSocketFactory ssf;

	char[] password;

	KeyStore this_ks;

	/**
	 * Open an (SSL) socket to the target using this private key.
	 * 
	 * @param target_name
	 *            The name (or IP address) of the machine hosting the Unicore
	 *            server with which to open a connection.
	 * 
	 * @param port_number
	 *            The port that teh Unicore server is listening on
	 *  
	 */
	protected Socket createSocket(String target_name, int port_number)
			throws UnknownHostException, IOException {
		return ssf.createSocket(target_name, port_number);
	}

	/***************************************************************************
	 * 
	 * for AJO 4.1.0
	 * 
	 * 
	 * 
	 * 
	 * 
	 *  
	 **************************************************************************/

	//    private KeyStore ks;
	/**
	 * @param cert_file
	 * @param in_password
	 * @param keyAlias
	 * @throws Identity.Exception
	 */
	public Identity(File cert_file, char[] in_password, String keyAlias)
			throws Identity.Exception {

		password = in_password;

		File abs_cert_file = cert_file.getAbsoluteFile();
		File cert_directory = abs_cert_file.getParentFile();

		CLogger.status("Loading trusted CAs (.pem files) from <"
				+ cert_directory + ">");

		TrustManager[] tm;

		// fixed by H.NAKADA 2004
		// read trusted cert from the same keystore
		if (abs_cert_file.exists()) {

			KeyStore temp_ks;
			try {
				temp_ks = KeyStore.getInstance("PKCS12");
				try {
					temp_ks.load(new FileInputStream(abs_cert_file), password);
				} catch (java.lang.Exception ex) {
					// Try again, could be JKS format
					//  ex.printStackTrace();
					temp_ks = KeyStore.getInstance("JKS");
					temp_ks.load(new FileInputStream(abs_cert_file), password);
				}
			} catch (java.lang.Exception ex) {
				throw new Identity.Exception(
						"Invalid password or certificate corrupt");
			}
			// get the certificate entry and use it as trusted cert
			try {
				KeyStore ks = KeyStore.getInstance("JKS");
				ks.load(null, null); // initialise, but empty
				int alias_counter = 0;

				Enumeration e = temp_ks.aliases();
				while (e.hasMoreElements()) {
					String alias = (String) e.nextElement();
					if (temp_ks.isCertificateEntry(alias)) {
						Certificate cert = temp_ks.getCertificate(alias);
						ks
								.setCertificateEntry(
										"Alias_" + alias_counter++, cert);
					}
				}
				if (alias_counter == 0) {
					throw new Identity.Exception(
							"Error, no trusted certificate inside the keystore");
				}
				TrustManagerFactory tmf = TrustManagerFactory.getInstance(
						"SunX509", "SunJSSE");
				tmf.init(ks);
				tm = tmf.getTrustManagers();
			} catch (java.lang.Exception ex) {
				throw new Identity.Exception("Error reading certificate: " + ex);
			}
		} else {
			throw new Identity.Exception("Error: no keystore");
		}

		//        try {
		//            KeyStore ks = KeyStore.getInstance("JKS");
		//            ks.load(null,null); // initialise, but empty

		//            int alias_counter = 0;

		//            File[] listing = cert_directory.listFiles(); // cert_directory is a
		// directory so OK
		//            for(int i = 0; i < listing.length; i++) {
		//                if(listing[i].getName().endsWith(".pem") ||
		// listing[i].getName().endsWith(".der")) {
		//                    CLogger.status("Loading trusted certificate signer(s) from:
		// "+listing[i]);
		//                    CertificateFactory cf = CertificateFactory.getInstance("X.509");
		//                    Collection loaded = cf.generateCertificates(new
		// FileInputStream(listing[i]));
		//                    Iterator ii = loaded.iterator();
		//                    while(ii.hasNext()) {
		//                        Certificate cert = (Certificate)ii.next();
		//                        ks.setCertificateEntry("Alias_"+alias_counter++,cert);
		//                    }
		//                }
		//            }

		//            TrustManagerFactory tmf =
		// TrustManagerFactory.getInstance("SunX509","SunJSSE");
		//            tmf.init(ks);
		//            tm = tmf.getTrustManagers();
		//        }
		//        catch(java.lang.Exception ex) {
		//            throw new Identity.Exception("Cannot load CA certificate: "+ex);
		//        }

		CLogger.status("Done loading trusted CAs");
		CLogger.status("Loading user certificate from <" + abs_cert_file + ">");

		KeyManager[] km;

		if (abs_cert_file.exists()) {
			// First as PKCS12
			KeyStore temp_ks;
			try {
				temp_ks = KeyStore.getInstance("PKCS12");
				try {
					temp_ks.load(new FileInputStream(abs_cert_file), password);
				} catch (java.lang.Exception ex) {
					// Try again, could be JKS format
					//	ex.printStackTrace();
					temp_ks = KeyStore.getInstance("JKS");
					temp_ks.load(new FileInputStream(abs_cert_file), password);
				}
			} catch (java.lang.Exception ex) {
				throw new Identity.Exception(
						"Invalid password or certificate corrupt");
			}

			// Use the first, creating an internal version with just this one
			// fixed to check the alias if the keyAlias is specified
			try {
				Enumeration e = temp_ks.aliases();
				while (e.hasMoreElements()) {
					String alias = (String) e.nextElement();
					//  the below two lines are inserted by nakada Nov. 19 2004

					if (keyAlias != null && !keyAlias.equals(alias)) {
						continue;
					}

					if (temp_ks.isKeyEntry(alias)) {

						//change ks -> this_ks
						//ks = KeyStore.getInstance("JKS");
						//ks.load(null, null);
						//ks.setKeyEntry("alias", temp_ks.getKey(alias,
						// password), password,
						// temp_ks.getCertificateChain(alias));
						//KeyManagerFactory kmf =
						// KeyManagerFactory.getInstance("SunX509", "SunJSSE");
						//kmf.init(ks, password);

						this_ks = KeyStore.getInstance("JKS");
						this_ks.load(null, null);
						this_ks.setKeyEntry("alias", temp_ks.getKey(alias,
								password), password, temp_ks
								.getCertificateChain(alias));
						KeyManagerFactory kmf = KeyManagerFactory.getInstance(
								"SunX509", "SunJSSE");
						kmf.init(this_ks, password);

						SSLContext context = SSLContext.getInstance("SSLv3");
						context.init(kmf.getKeyManagers(), tm,
								new SecureRandom());
						ssf = context.getSocketFactory();
						break; // Got one, use it
					}
				}
			} catch (java.lang.Exception ex) {
				throw new Identity.Exception("Error reading certificate: " + ex);
			}
		} else {
			throw new Identity.Exception("User certificate file not found.");
		}
		CLogger.status("Done loading user certificate.");

		try {

			//for AJO 4.1.0
			//changed constructor's first args type(org.unicore.User class)
			//X509Certificate -> X509Certificate[]
			X509Certificate[] cert = new X509Certificate[1];

			//chnage ks->this_ks
			//cert[0] = (X509Certificate)ks.getCertificate("alias");
			cert[0] = (X509Certificate) this_ks.getCertificate("alias");

			user = new User(cert, "need to set an email address");
			//for AJO 4.1.0

			//for AJO 4.0
			//user = new User((X509Certificate) ks.getCertificate("alias"),
			// "need to set an email address");
		} catch (java.lang.Exception ex) {
			throw new Identity.Exception("Problems generating user\n" + ex);
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
