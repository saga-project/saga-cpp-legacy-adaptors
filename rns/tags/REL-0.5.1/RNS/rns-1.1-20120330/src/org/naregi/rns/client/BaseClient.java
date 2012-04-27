/*
 * Copyright (C) 2008-2012 Osaka University.
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Copyright 1999-2006 University of Chicago
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.naregi.rns.client;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.PrintWriter;
import java.security.cert.X509Certificate;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import javax.security.auth.Subject;
import javax.xml.namespace.QName;
import javax.xml.rpc.Stub;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.OptionBuilder;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.commons.cli.PosixParser;
import org.globus.axis.gsi.GSIConstants;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.axis.message.addressing.ReferenceParametersType;
import org.globus.axis.util.Util;
import org.globus.gsi.CertUtil;
import org.globus.wsrf.encoding.ObjectDeserializer;
import org.globus.wsrf.impl.SimpleResourceKey;
import org.globus.wsrf.impl.security.authentication.Constants;
import org.globus.wsrf.impl.security.authentication.encryption.EncryptionCredentials;
import org.globus.wsrf.impl.security.authorization.HostAuthorization;
import org.globus.wsrf.impl.security.authorization.HostOrSelfAuthorization;
import org.globus.wsrf.impl.security.authorization.IdentityAuthorization;
import org.globus.wsrf.impl.security.authorization.NoAuthorization;
import org.globus.wsrf.impl.security.authorization.SelfAuthorization;
import org.globus.wsrf.impl.security.descriptor.ClientSecurityDescriptor;
import org.globus.wsrf.security.authorization.client.Authorization;
import org.globus.wsrf.security.descriptor.client.Anonymous;
import org.globus.wsrf.security.descriptor.client.CertKeyFileType;
import org.globus.wsrf.security.descriptor.client.ClientSecurityConfig;
import org.globus.wsrf.security.descriptor.client.CredentialType;
import org.globus.wsrf.security.descriptor.client.GSISecureConversation;
import org.globus.wsrf.security.descriptor.client.ValIntType;
import org.globus.wsrf.security.descriptor.client.ValStringType;
import org.xml.sax.InputSource;

/**
 * Web service client APIs for Globus.
 *
 * Improved org.globus.wsrf.client.BaseClient.
 *
 * This object can be created(initialized) many times.
 * (org.globus.wsrf.client.BaseClient cannot be created many times.)
 *
 * @see <a
 *      href="http://www.globus.org/api/javadoc-4.2.1/globus_java_ws_core/org/globus/wsrf/client/BaseClient.html">org.globus.wsrf.client.BaseClient</a>
 */
public abstract class BaseClient {

	public static final int COMMAND_LINE_ERROR = 1;
	public static final int APPLICATION_ERROR = 2;

	protected EndpointReferenceType endpoint;
	protected boolean debugMode;
	protected String customUsage;
	protected String helpFooter;
	protected String helpHeader;
	protected Options options = new Options();

	protected String mechanism;
	protected Object protection = Constants.SIGNATURE;
	protected Object delegation;
	protected Object authorization;
	protected Object anonymous;
	protected Integer contextLifetime;
	protected String msgActor;
	protected String convActor;
	protected String publicKeyFilename;
	protected String descriptorFile;
	protected String proxyFile;
	protected String certFile;
	protected String keyFile;

	protected String name = getClass().getName();

	protected int timeout = -1;

	protected ClientSecurityDescriptor clientSecDesc;

	static final String AUTHZ_DESC = "Sets authorization, can be 'self', 'host', 'hostSelf', 'none' or a "
			+ "string specifying the expected identity of the remote party";

	static final String MECHANISM_DESC = "Sets authentication mechanism: 'msg' (for GSI Secure Message), or"
			+ " 'conv' (for GSI Secure Conversation)";

	static final String PROTECTION_DESC = "Sets protection level, can be 'sig' (for signature) "
			+ " can be 'enc' (for encryption)";

	static final String ANON_DESC = "Use anonymous authentication (requires either -m 'conv' or"
			+ " transport (https) security)";

	static final String FILENAME_DESC = "A file with server's certificate used for encryption. "
			+ "Used in the case of GSI Secure Message encryption";

	static final String CONTEXT_DESC = "Lifetime of context created for GSI Secure "
			+ "Conversation (requires -m 'conv')";

	/*
	 * static final String MSG_ACTOR_DESC =
	 * "Sets actor name for GSI Secure Message";
	 *
	 * static final String CONV_ACTOR_DESC =
	 * "Sets actor name for GSI Secure Conversation";
	 */

	static final String DELEG_DESC = "Performs delegation. Can be 'limited' or 'full'. "
			+ "(requires -m 'conv')";

	static final String PROXY_DESC = "Sets the proxy file to use as credential.";

	static final String CERT_KEY_DESC = "Sets the certificate and key  file to use as credential. Key file"
			+ " must not be encrypted. certificateFile keyFile";

	static final String DESCRIPTOR_DESC = "Sets client security descriptor. Overrides all other security "
			+ "settings";

	@SuppressWarnings("static-access")
	public final Option HELP = OptionBuilder.withDescription("Displays help")
			.withLongOpt("help")
			.create("h");

	@SuppressWarnings("static-access")
	public final Option EPR_FILE = OptionBuilder.withArgName("file")
			.hasArg()
			.withDescription("Loads EPR from file")
			.withLongOpt("eprFile")
			.create("e");

	@SuppressWarnings("static-access")
	public final Option SERVICE_URL = OptionBuilder.withArgName("url")
			.hasArg()
			.withDescription("Service URL")
			.withLongOpt("service")
			.create("s");

	@SuppressWarnings("static-access")
	public final Option RESOURCE_KEY = OptionBuilder.withArgName("name value")
			.hasArgs(2)
			.withDescription("Resource Key")
			.withLongOpt("key")
			.create("k");

	@SuppressWarnings("static-access")
	public final Option DEBUG = OptionBuilder.withDescription(
			"Enables debug mode")
			.withLongOpt("debug")
			.create("d");

	@SuppressWarnings("static-access")
	public final Option AUTHZ = OptionBuilder.withArgName("type")
			.hasArg()
			.withDescription(AUTHZ_DESC)
			.withLongOpt("authorization")
			.create("z");

	@SuppressWarnings("static-access")
	public final Option MECHANISM = OptionBuilder.withArgName("type")
			.hasArg()
			.withDescription(MECHANISM_DESC)
			.withLongOpt("securityMech")
			.create("m");

	@SuppressWarnings("static-access")
	public final Option ANON = OptionBuilder.withDescription(ANON_DESC)
			.withLongOpt("anonymous")
			.create("a");

	@SuppressWarnings("static-access")
	public final Option PROTECTION = OptionBuilder.withArgName("type")
			.hasArg()
			.withDescription(PROTECTION_DESC)
			.withLongOpt("protection")
			.create("p");

	@SuppressWarnings("static-access")
	public final Option PUB_KEY_FILE = OptionBuilder.withArgName("file")
			.hasArg()
			.withDescription(FILENAME_DESC)
			.withLongOpt("serverCertificate")
			.hasOptionalArg()
			.create("c");

	@SuppressWarnings("static-access")
	public final Option CONTEXT = OptionBuilder.withArgName("value")
			.hasArg()
			.withDescription(CONTEXT_DESC)
			.withLongOpt("contextLifetime")
			.create("l");

	@SuppressWarnings("static-access")
	public final Option PROXY = OptionBuilder.withArgName("value")
			.hasArg()
			.withDescription(PROXY_DESC)
			.withLongOpt("proxyFileName")
			.create("x");

	@SuppressWarnings("static-access")
	public final Option CERT_KEY = OptionBuilder.withArgName("cert key")
			.hasArgs(2)
			.withDescription(CERT_KEY_DESC)
			.withLongOpt("certKeyFiles")
			.create("v");

	/*
	 * public static final Option MSG_ACTOR =
	 * OptionBuilder.withArgName("actor").hasArg()
	 * .withDescription(MSG_ACTOR_DESC)
	 * .withLongOpt("gsiSecMsgActor").create("x");
	 *
	 * public static final Option CONV_ACTOR =
	 * OptionBuilder.withArgName("actor").hasArg()
	 * .withDescription(CONV_ACTOR_DESC)
	 * .withLongOpt("gsiSecConvActor").create("y");
	 */

	@SuppressWarnings("static-access")
	public final Option DELEG = OptionBuilder.withArgName("mode")
			.hasArg()
			.withDescription(DELEG_DESC)
			.withLongOpt("delegation")
			.create("g");

	@SuppressWarnings("static-access")
	public final Option DESCRIPTOR = OptionBuilder.withDescription(
			DESCRIPTOR_DESC)
			.hasArg()
			.withArgName("file")
			.withLongOpt("descriptor")
			.create("f");

	@SuppressWarnings("static-access")
	public final Option TIMEOUT = OptionBuilder.withDescription(
			"Client timeout (in seconds)")
			.hasArg()
			.withArgName("timeout")
			.withLongOpt("timeout")
			.create("t");

	static {
		Util.registerTransport();
	}

	protected BaseClient() {
		addOptions();
	}

	protected void addEPROptions() {
		options.addOption(EPR_FILE);
		options.addOption(SERVICE_URL);
		options.addOption(RESOURCE_KEY);
	}

	protected void addOptions() {
		options.addOption(HELP);
		options.addOption(DEBUG);

		addEPROptions();

		// security options
		options.addOption(DESCRIPTOR);
		options.addOption(MECHANISM);
		options.addOption(ANON);
		options.addOption(PROTECTION);
		options.addOption(AUTHZ);
		options.addOption(CONTEXT);

		/*
		 * options.addOption(MSG_ACTOR); options.addOption(CONV_ACTOR);
		 */

		options.addOption(DELEG);
		options.addOption(PUB_KEY_FILE);
		options.addOption(TIMEOUT);

		options.addOption(PROXY);
		options.addOption(CERT_KEY);
	}

	public void setCustomUsage(String customUsage) {
		this.customUsage = customUsage;
	}

	public void setHelpFooter(String msg) {
		this.helpFooter = msg;
	}

	public void setHelpHeader(String msg) {
		this.helpHeader = msg;
	}

	public void addOption(Option option) {
		this.options.addOption(option);
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getUsage() {
		String usage = getName() + " [-h] [-s url [-k name value] | -e file] ";

		usage = (this.customUsage == null) ? usage : usage + this.customUsage;

		String header = (this.helpHeader == null) ? "Options:"
				: this.helpHeader;
		HelpFormatter formatter = new HelpFormatter();

		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		PrintWriter pw = new PrintWriter(baos);
		formatter.printHelp(pw, 74, usage, header, options, 1, 3, null);

		if (this.helpFooter != null) {
			pw.println(this.helpFooter);
		}
		pw.flush();
		return baos.toString();
	}

	protected CommandLine parse(String[] args) throws Exception {
		return parse(args, null);
	}

	// Parses command line parameters to get endpoint. Could be
	// service URL, EPR or resource key
	protected void parseEndpoint(CommandLine line) throws Exception {

		if (line.hasOption("e")) {
			if (line.hasOption("k")) {
				throw new ParseException("-e and -k arguments are exclusive");
			}
			if (line.hasOption("s")) {
				throw new ParseException("-e and -s arguments are exclusive");
			}

			FileInputStream in = null;
			try {
				in = new FileInputStream(line.getOptionValue("e"));
				this.endpoint = (EndpointReferenceType) ObjectDeserializer.deserialize(
						new InputSource(in), EndpointReferenceType.class);
			} finally {
				if (in != null) {
					try {
						in.close();
					} catch (Exception e) {
					}
				}
			}
		} else if (line.hasOption("s")) {
			this.endpoint = new EndpointReferenceType();
			this.endpoint.setAddress(new Address(line.getOptionValue("s")));
		} else {
			throw new ParseException("-s or -e argument is required");
		}

		if (line.hasOption("k")) {
			String[] values = line.getOptionValues("k");
			if (values.length != 2) {
				throw new ParseException("-k requires two arguments");
			}
			QName keyName = QName.valueOf(values[0]);
			ReferenceParametersType props = new ReferenceParametersType();
			SimpleResourceKey key = new SimpleResourceKey(keyName, values[1]);
			props.add(key.toSOAPElement());
			this.endpoint.setParameters(props);
		}
	}

	protected CommandLine parse(String[] args, Properties defaultOptions)
			throws Exception {
		validateOptions();

		CommandLineParser parser = new PosixParser();
		CommandLine line = parser.parse(options, args, defaultOptions);

		if (defaultOptions == null) {
			defaultOptions = new Properties();
			defaultOptions.put(TIMEOUT.getOpt(),
					String.valueOf(getDefaultTimeout()));
		} else if (defaultOptions.get(TIMEOUT.getOpt()) == null) {
			defaultOptions.put(TIMEOUT.getOpt(),
					String.valueOf(getDefaultTimeout()));
		}

		@SuppressWarnings("rawtypes")
		Iterator iter = defaultOptions.entrySet().iterator();
		while (iter.hasNext()) {
			@SuppressWarnings("rawtypes")
			Map.Entry entry = (Map.Entry) iter.next();
			Option opt = options.getOption((String) entry.getKey());
			if (opt != null) {
				String desc = opt.getDescription();
				desc += " (Default '" + entry.getValue() + "')";
				opt.setDescription(desc);
			}
		}

		if (line.hasOption("h")) {
			throw new ParseException("Help option");
		}

		// parse settings to get endpoint.
		parseEndpoint(line);

		this.debugMode = line.hasOption("d");

		// Security mechanism
		if (line.hasOption("m")) {
			String value = line.getOptionValue("m");
			if (value != null) {
				if (value.equals("msg")) {
					this.mechanism = Constants.GSI_SEC_MSG;
				} else if (value.equals("conv")) {
					this.mechanism = Constants.GSI_SEC_CONV;
				} else {
					throw new ParseException("Unsupported security mechanism: "
							+ value);
				}
			}
		}

		// Protection
		if (line.hasOption("p")) {
			String value = line.getOptionValue("p");
			if (value != null) {
				if (value.equals("sig")) {
					this.protection = Constants.SIGNATURE;
				} else if (value.equals("enc")) {
					this.protection = Constants.ENCRYPTION;
				} else {
					throw new ParseException("Unsupported protection mode: "
							+ value);
				}
			}
		}

		// Delegation
		if (line.hasOption("g")) {
			String value = line.getOptionValue("g");
			if (value != null) {
				if (value.equals("limited")) {
					this.delegation = GSIConstants.GSI_MODE_LIMITED_DELEG;
				} else if (value.equals("full")) {
					this.delegation = GSIConstants.GSI_MODE_FULL_DELEG;
				} else {
					throw new ParseException("Unsupported delegation mode: "
							+ value);
				}
			}
		}

		// Authz
		if (line.hasOption("z")) {
			String value = line.getOptionValue("z");
			if (value != null) {
				if (value.equals("self")) {
					this.authorization = SelfAuthorization.getInstance();
				} else if (value.equals("host")) {
					this.authorization = HostAuthorization.getInstance();
				} else if (value.equals("none")) {
					this.authorization = NoAuthorization.getInstance();
				} else if (value.equals("hostSelf")) {
					this.authorization = new HostOrSelfAuthorization();
				} else if (authorization == null) {
					this.authorization = new IdentityAuthorization(value);
				}
			}
		}

		// Anonymous
		if (line.hasOption("a")) {
			this.anonymous = Boolean.TRUE;
		}

		// context lifetime
		if (line.hasOption("l")) {
			String value = line.getOptionValue("l");
			if (value != null)
				this.contextLifetime = new Integer(value);
		}

		/*
		 * // msg actor if (line.hasOption("x")) { String value =
		 * line.getOptionValue("x"); this.msgActor = value; }
		 *
		 * // conv actor if (line.hasOption("y")) { String value =
		 * line.getOptionValue("y"); this.convActor = value; }
		 */

		// Server's public key
		if (line.hasOption("c")) {
			String value = line.getOptionValue("c");
			this.publicKeyFilename = value;
		}

		if (line.hasOption("f")) {
			String value = line.getOptionValue("f");
			this.descriptorFile = value;
		}

		// timeout
		if (line.hasOption(TIMEOUT.getOpt())) {
			String value = line.getOptionValue(TIMEOUT.getOpt());
			this.timeout = Integer.parseInt(value) * 1000;
		}

		// cert and key
		if (line.hasOption(CERT_KEY.getOpt())) {
			String[] values = line.getOptionValues(CERT_KEY.getOpt());
			if ((values == null) || (values.length != 2)) {
				throw new ParseException("Option requires certificate and "
						+ "key file name");
			}
			this.certFile = values[0];
			this.keyFile = values[1];
		} else if (line.hasOption(PROXY.getOpt())) {
			this.proxyFile = line.getOptionValue(PROXY.getOpt());
		}

		return line;
	}

	protected ClientSecurityDescriptor getSecurityDescriptor() throws Exception {
		if (this.clientSecDesc == null) {
			this.clientSecDesc = constructClientSecDesc();
		}
		return this.clientSecDesc;
	}

	protected ClientSecurityDescriptor constructClientSecDesc()
			throws Exception {

		if (this.descriptorFile != null) {
			return new ClientSecurityDescriptor(this.descriptorFile);
		}

		ClientSecurityConfig desc = new ClientSecurityConfig();
		CredentialType credentialType = null;
		if (this.proxyFile != null) {
			credentialType = new CredentialType();
			credentialType.setProxyFile(new ValStringType(this.proxyFile));
		} else if (this.certFile != null) {
			CertKeyFileType certKey = new CertKeyFileType();
			certKey.setCertFile(new ValStringType(this.certFile));
			certKey.setKeyFile(new ValStringType(this.keyFile));

			credentialType = new CredentialType();
			credentialType.setCertKeyFiles(certKey);
		}
		desc.setCredential(credentialType);

		ClientSecurityDescriptor descImpl = new ClientSecurityDescriptor(desc);

		GSISecureConversation conv = null;
		if (this.protection != null) {
			// this means if both transport security and message security
			// are enabled both will get the same protection
			if (this.endpoint.getAddress().getScheme().equals("https")) {
				descImpl.setGSISecureTransport((Integer) this.protection);
			}
			if (this.mechanism != null) {
				if (this.mechanism.equals(Constants.GSI_SEC_CONV)) {
					descImpl.setGSISecureConv((Integer) this.protection);
					conv = desc.getGSISecureConversation();
					conv.setDelegation(new ValStringType(
							(String) this.delegation));
					if (Boolean.TRUE.equals(this.anonymous)) {
						conv.setAnonymous(new Anonymous());
					}
					if (this.contextLifetime != null) {
						ValIntType valInt = new ValIntType(
								this.contextLifetime.intValue());
						conv.setContextLifetime(valInt);
					}
				} else if (this.mechanism.equals(Constants.GSI_SEC_MSG)) {
					descImpl.setGSISecureMsg((Integer) this.protection);
					descImpl.setPeerSubject(loadPeerCredentials());
				}
			}
		}

		descImpl.setAuthz((Authorization) this.authorization);
		// descImpl.setSecurityDescriptor(desc);
		return descImpl;
		// actors are missing.

	}

	public void setOptions(Stub stub) throws Exception {

		if (this.descriptorFile != null) {
			stub._setProperty(Constants.CLIENT_DESCRIPTOR_FILE,
					this.descriptorFile);
			return;
		}

		if (this.protection != null) {
			String serviceAddress = getAddress(stub);

			// this means if both transport security and message security
			// are enabled both will get the same protection
			if (serviceAddress.startsWith("https")) {
				stub._setProperty(GSIConstants.GSI_TRANSPORT, this.protection);
			}
			if (this.mechanism != null) {
				stub._setProperty(this.mechanism, this.protection);
			}
		}
		/*
		 * if (this.convActor != null) { stub._setProperty("gssActor",
		 * this.convActor); }
		 */

		if (this.delegation != null) {
			stub._setProperty(GSIConstants.GSI_MODE, this.delegation);
		}

		if (this.authorization != null) {
			stub._setProperty(Constants.AUTHORIZATION, this.authorization);
		}

		if (this.anonymous != null) {
			stub._setProperty(Constants.GSI_ANONYMOUS, this.anonymous);
		}

		/*
		 * if (this.msgActor != null) { stub._setProperty("x509Actor",
		 * this.msgActor); }
		 */

		if ((Constants.GSI_SEC_MSG.equals(this.mechanism))
				&& (Constants.ENCRYPTION.equals(this.protection))) {
			Subject subject = loadPeerCredentials();
			if (subject != null) {
				stub._setProperty(Constants.PEER_SUBJECT, subject);
			} else {
				throw new Exception("Secure message encryption requires valid "
						+ "peer credentials");
			}
		}

		if (this.contextLifetime != null) {
			stub._setProperty(Constants.CONTEXT_LIFETIME, this.contextLifetime);
		}

		if (this.timeout != -1) {
			((org.apache.axis.client.Stub) stub).setTimeout(this.timeout);
		}
	}

	public EndpointReferenceType getEPR() {
		return this.endpoint;
	}

	protected void setEPR(EndpointReferenceType epr) {
		this.endpoint = epr;
	}

	public boolean isDebugMode() {
		return this.debugMode;
	}

	private Subject loadPeerCredentials() throws Exception {
		if (publicKeyFilename == null) {
			return null;
		}
		Subject subject = new Subject();
		X509Certificate serverCert = CertUtil.loadCertificate(publicKeyFilename);
		EncryptionCredentials encryptionCreds = new EncryptionCredentials(
				new X509Certificate[] { serverCert });
		subject.getPublicCredentials().add(encryptionCreds);
		return subject;
	}

	protected void validateOptions() throws Exception {
		@SuppressWarnings("rawtypes")
		Iterator iter = this.options.getOptions().iterator();
		while (iter.hasNext()) {
			Option op = (Option) iter.next();

			// checks short names
			Option opShort = this.options.getOption(op.getOpt());
			if (op != opShort) {
				throw new Exception("Option short name conflict ('"
						+ op.getOpt() + "'). Expected: " + op + ", Got: "
						+ opShort);
			}

			// check long names (if any)
			if (op.getLongOpt() != null) {
				Option opLong = this.options.getOption("--" + op.getLongOpt());
				if (op != opLong) {
					throw new Exception("Option long name conflict ('"
							+ op.getLongOpt() + "'). Expected: " + op
							+ ", Got: " + opLong);
				}
			}
		}
	}

	private int getDefaultTimeout() {
		return (org.apache.axis.Constants.DEFAULT_MESSAGE_TIMEOUT / 1000);
	}

	private String getAddress(Stub stub) {
		return (String) stub._getProperty(org.apache.axis.client.Stub.ENDPOINT_ADDRESS_PROPERTY);
	}

}
