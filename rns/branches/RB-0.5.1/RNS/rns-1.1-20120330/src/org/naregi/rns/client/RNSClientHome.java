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
package org.naregi.rns.client;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import javax.xml.namespace.QName;
import javax.xml.rpc.Stub;

import org.apache.axis.AxisFault;
import org.apache.axis.types.URI;
import org.apache.axis.types.URI.MalformedURIException;
import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.ParseException;
import org.globus.axis.message.addressing.AttributedURIType;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.wsrf.security.authorization.client.Authorization;
import org.naregi.rns.RNSConfig;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.stubs.ACLFaultType;
import org.naregi.rns.stubs.AddRequestType;
import org.naregi.rns.stubs.AddResponseType;
import org.naregi.rns.stubs.GetACLRequestType;
import org.naregi.rns.stubs.GetACLResponseType;
import org.naregi.rns.stubs.GetCallerInfoRequestType;
import org.naregi.rns.stubs.GetCallerInfoResponseType;
import org.naregi.rns.stubs.GetServerStatusRequestType;
import org.naregi.rns.stubs.GetServerStatusResponseType;
import org.naregi.rns.stubs.IterateRequestType;
import org.naregi.rns.stubs.IterateResponseType;
import org.naregi.rns.stubs.LookupRequestType;
import org.naregi.rns.stubs.LookupResponseType;
import org.naregi.rns.stubs.NoopRequestType;
import org.naregi.rns.stubs.NoopResponseType;
import org.naregi.rns.stubs.RNSExtensionPortType;
import org.naregi.rns.stubs.RNSPortType;
import org.naregi.rns.stubs.RNSSearchPortType;
import org.naregi.rns.stubs.ReadNotPermittedFaultType;
import org.naregi.rns.stubs.RemoveACLRequestType;
import org.naregi.rns.stubs.RemoveACLResponseType;
import org.naregi.rns.stubs.RemoveRequestType;
import org.naregi.rns.stubs.RemoveResponseType;
import org.naregi.rns.stubs.RenameRequestType;
import org.naregi.rns.stubs.RenameResponseType;
import org.naregi.rns.stubs.SearchFaultType;
import org.naregi.rns.stubs.SearchRequestType;
import org.naregi.rns.stubs.SearchResponseType;
import org.naregi.rns.stubs.SetACLRequestType;
import org.naregi.rns.stubs.SetACLResponseType;
import org.naregi.rns.stubs.SetMetadataRequestType;
import org.naregi.rns.stubs.SetMetadataResponseType;
import org.naregi.rns.stubs.StartProfileRequestType;
import org.naregi.rns.stubs.StartProfileResponseType;
import org.naregi.rns.stubs.StopProfileRequestType;
import org.naregi.rns.stubs.StopProfileResponseType;
import org.naregi.rns.stubs.WSIteratorPortType;
import org.naregi.rns.stubs.WriteNotPermittedFaultType;
import org.naregi.rns.stubs.service.RNSExtensionServiceAddressingLocator;
import org.naregi.rns.stubs.service.RNSSearchServiceAddressingLocator;
import org.naregi.rns.stubs.service.RNSServiceAddressingLocator;
import org.naregi.rns.stubs.service.WSIteratorServiceAddressingLocator;
import org.oasis.wsrf.lifetime.Destroy;
import org.oasis.wsrf.lifetime.DestroyResponse;
import org.oasis.wsrf.lifetime.ResourceNotDestroyedFaultType;
import org.oasis.wsrf.properties.GetMultipleResourcePropertiesResponse;
import org.oasis.wsrf.properties.GetMultipleResourceProperties_Element;
import org.oasis.wsrf.properties.GetResourcePropertyResponse;
import org.oasis.wsrf.properties.InvalidResourcePropertyQNameFaultType;
import org.oasis.wsrf.resource.ResourceUnavailableFaultType;
import org.oasis.wsrf.resource.ResourceUnknownFaultType;

/**
 * Interpret command options and Initialize RNS client environments.
 *
 * See rns-* -help message.
 */
public class RNSClientHome extends BaseClient {
	private static final String SYSPROPKEY_CONFIG = "rns.config";
	private static final String SYSPROPKEY_COMMAND_NAME = "rns.command.name";

	private static final String PREFIX = "rns.client.";

	private static final String PROPKEY_RNSURL = PREFIX + "serviceURL";
	private static final String PROPKEY_TCPMONITORPORT = PREFIX
			+ "TCPMonitorPort";
	private static final String PROPKEY_OPTIONARGS = PREFIX + "optionArgs";
	private static final String PROPKEY_MAXRECURSIVE = PREFIX + "maxRecursive";
	private static final String PROPKEY_CACHETIMEOUT = PREFIX + "cacheTimeout"; /* millisec. */

	private Properties defaultOptions;
	private String defaultConfigFileName = System.getProperty("user.home")
			+ System.getProperty("file.separator") + ".rns-client.conf";
	private String configFileName = defaultConfigFileName;
	private Properties clientConfig;

	private RNSClient rnsClient = null;
	private RNSExtensionClient extClient = null;
	private RNSSearchClient searchClient = null;

	/* --- configurable values ------------ */
	private String rnsURL = "http://localhost:8080"
			+ RNSConfig.getRNSServicePath();
	private int tcpMonitorPort = 0; /* 0 = disable */
	private String optionArgsString = null;
	private int maxRecursive = 10;
	private long cacheTimeout = 10000;

	/**
	 * Get the TCPMonitor port from rns.client.TCPMonitorPort of client config.
	 *
	 * TCPMonitor is org.apache.axis.utils.tcpmon.
	 *
	 * @return the number of TCPMonitor port.
	 */
	public int getTcpMonitorPort() {
		return tcpMonitorPort;
	}

	/**
	 * Get the rns.client.optionArgs String.
	 *
	 * @return option arguments
	 */
	public String getOptionArgsString() {
		return optionArgsString;
	}

	/**
	 * Get the default maximum depth number for recursive operations from
	 * rns.client.maxRecursive of client config.
	 *
	 * @return the depth number
	 */
	public int getMaxRecursive() {
		return maxRecursive;
	}

	/**
	 * Get the client cache lifetime from rns.client.cacheTimeout of client
	 * config.
	 *
	 * @return rns.client.cacheTimeout (millisecond)
	 */
	public long getCacheTimeout() {
		return cacheTimeout;
	}

	/* ------------------------------ */

	public RNSClientHome() {
		super();
		defaultOptions = new Properties();
		String conf = System.getProperty(SYSPROPKEY_CONFIG);
		if (conf != null && !conf.equals("")) {
			configFileName = conf;
		}
		String name = System.getProperty(SYSPROPKEY_COMMAND_NAME);
		if (name != null && !name.equals("")) {
			setName(name);
		}
		helpHeader = "Globus Common Options:";
	}

	/**
	 * Get Authorization type from a command option (-z).
	 *
	 * @return Authorization object
	 */
	public Authorization getAuthorization() {
		return (Authorization) authorization;
	}

	/**
	 * Set a pathname of client configuration file.
	 *
	 * @param fileName a pathname of local disk
	 */
	public void setConfigFileName(String fileName) {
		configFileName = fileName;
	}

	private void loadConfigFile() throws IOException {
		clientConfig = new Properties();
		FileInputStream inStream;
		try {
			inStream = new FileInputStream(configFileName);
		} catch (FileNotFoundException e) {
			if (defaultConfigFileName.equals(configFileName) == false) {
				throw e;
			}
			// use default value
			return;
		}
		try {
			clientConfig.load(inStream);
		} catch (IOException e) {
			throw e;
		} finally {
			try {
				inStream.close();
			} catch (IOException e) {
			}
		}
		rnsURL = clientConfig.getProperty(PROPKEY_RNSURL, rnsURL);
		tcpMonitorPort = Integer.parseInt(clientConfig.getProperty(
				PROPKEY_TCPMONITORPORT, "0"));
		optionArgsString = clientConfig.getProperty(PROPKEY_OPTIONARGS);
		maxRecursive = Integer.parseInt(clientConfig.getProperty(
				PROPKEY_MAXRECURSIVE, "10"));
		cacheTimeout = Long.parseLong(clientConfig.getProperty(
				PROPKEY_CACHETIMEOUT, "10000"));
	}

	private boolean existOption(String shortOpt, String longOpt, String[] args) {
		if (args == null) {
			return false;
		}
		for (String s : args) {
			if (shortOpt != null && ("-" + shortOpt).equals(s)) {
				return true;
			} else if (longOpt != null && ("--" + longOpt).equals(s)) {
				return true;
			}
		}
		return false;
	}

	@SuppressWarnings("unchecked")
	private List<String> initialize(String[] args, int min, int max)
			throws Exception {
		loadConfigFile();
		String[] optArgs = null;
		if (optionArgsString != null) {
			optArgs = optionArgsString.split(" ");
		}
		ArrayList<String> argslist = new ArrayList<String>();
		if (!existOption(EPR_FILE.getOpt(), EPR_FILE.getLongOpt(), optArgs)
				&& !existOption(EPR_FILE.getOpt(), EPR_FILE.getLongOpt(), args)) {
			if (!existOption(SERVICE_URL.getOpt(), SERVICE_URL.getLongOpt(),
					optArgs)
					&& !existOption(SERVICE_URL.getOpt(),
							SERVICE_URL.getLongOpt(), args)) {
				argslist.add("-" + SERVICE_URL.getOpt());
				argslist.add(rnsURL);
			}
			if (!existOption(RESOURCE_KEY.getOpt(), RESOURCE_KEY.getLongOpt(),
					optArgs)
					&& !existOption(RESOURCE_KEY.getOpt(),
							RESOURCE_KEY.getLongOpt(), args)) {
				argslist.add("-" + RESOURCE_KEY.getOpt());
				argslist.add(RNSQNames.RESOURCE_ID.toString());
				argslist.add(RNSConfig.getRootID());
			}
		}
		if (optArgs != null) {
			for (int i = 0; i < optArgs.length; i++) {
				String s = optArgs[i];
				String shortOpt = null;
				String longOpt = null;
				if (s.startsWith("--")) {
					longOpt = s.substring(2);
				} else if (s.startsWith("-")) {
					shortOpt = s.substring(1);
				} else {
					argslist.add(s);
					continue;
				}
				if (existOption(shortOpt, longOpt, args)) {
					// skip
					if (RESOURCE_KEY.getOpt().equals(shortOpt)
							|| RESOURCE_KEY.getLongOpt().equals(longOpt)
							|| CERT_KEY.getOpt().equals(shortOpt)
							|| CERT_KEY.getLongOpt().equals(longOpt)) {
						i += 2;
					} else {
						i += 1;
					}
				} else {
					argslist.add(s);
				}
			}
		}
		for (String s : args) {
			argslist.add(s);
		}

		String[] newArgs = argslist.toArray(new String[0]);
		List<String> al;
		try {
			CommandLine cl = parse(newArgs, defaultOptions);
			al = cl.getArgList();
		} catch (Exception e) {
			throw e;
		}

		if (min >= 0 && (al.size() < min || al.size() > max)) {
			throw new ParseException("invalid number of arguments: "
					+ al.size() + ": " + al.toString());
		}

		rnsClient = new RNSClientImpl(this);
		rnsClient.setDefaultMaxRecursiveDepth(maxRecursive);
		extClient = new RNSExtensionClientImpl(this);
		searchClient = new RNSSearchClientImpl(this);

		return al;
	}

	private List<String> argsList = null;

	/**
	 * Parse command line options and arguments.
	 *
	 * @param args command arguments array
	 * @param min a minimum number of command arguments (-1 means no limitation)
	 * @param max a maximum number of command arguments
	 * @return command arguments without options
	 * @throws Exception
	 */
	public List<String> parseArgs(String[] args, int min, int max)
			throws Exception {
		if (argsList == null) {
			argsList = initialize(args, min, max);
		}
		return argsList;
	}

	private Map<String, String> argsMap = null;

	/**
	 * Parse command line options and arguments with supporting plus(+) options.
	 *
	 * <pre>
	 * --- Examples of command options format ---
	 * short flag: +h
	 * short key/value: +k value
	 * long flag: ++help
	 * long key/value : ++key value
	 * argPrefix: non-option arguments
	 * minArg: -1 means no limitation
	 * maxArg: -1 means no limitation
	 * </pre>
	 *
	 * <pre>
	 * --- Examples of getting value from parsed arguments ---
	 * short flag: containsKey("+h")
	 * short key/value: get("+k")
	 * long flag: containsKey("++help")
	 * long key/value: get("++key")
	 * argPrefix: "arg" -> get("arg0"), get("arg1"), ...
	 * </pre>
	 *
	 * @param args
	 * @param shortFlags
	 * @param longFlags
	 * @param shortKeys
	 * @param longKeys
	 * @param argPrefix
	 * @param minArg minimum number of arguments
	 * @param maxArg maximum number of arguments
	 * @return parsed arguments
	 * @throws Exception
	 */
	public Map<String, String> parseArgsWithPlusOption(String[] args,
			String[] shortFlags, String[] longFlags, String[] shortKeys,
			String[] longKeys, String argPrefix, int minArg, int maxArg)
			throws Exception {
		if (argsMap != null) {
			return argsMap;
		}

		List<String> l = initialize(args, -1, -1);
		Map<String, String> map = new HashMap<String, String>();
		String tmpOpt = null;
		int normalArgNum = 0;
		for (String s : l) {
			if (s == null) {
			} else if (s.startsWith("+")) {
				boolean found = false;
				boolean flag = false;
				if (s.startsWith("++")) {
					if (longFlags != null) {
						for (String o : longFlags) {
							if (("++" + o).equals(s)) {
								found = true;
								flag = true;
								break;
							}
						}
					}
					if (longKeys != null) {
						for (String o : longKeys) {
							if (("++" + o).equals(s)) {
								found = true;
								flag = false;
								break;
							}
						}
					}
				} else { /* "+" */
					if (shortFlags != null) {
						for (String o : shortFlags) {
							if (("+" + o).equals(s)) {
								found = true;
								flag = true;
								break;
							}
						}
					}
					if (shortKeys != null) {
						for (String o : shortKeys) {
							if (("+" + o).equals(s)) {
								found = true;
								flag = false;
								break;
							}
						}
					}
				}
				if (found) {
					if (flag == false) {
						tmpOpt = s; /* save key */
					}
					map.put(s, null);
					/* checking map.containsKey(s) */
				} else {
					throw new ParseException("unknown option: " + s);
				}
			} else {
				if (tmpOpt != null) {
					/* key value */
					map.put(tmpOpt, s);
					tmpOpt = null;
				} else {
					/* normal args */
					if (maxArg >= 0 && normalArgNum >= maxArg) {
						throw new ParseException("too many args");
					}
					map.put(argPrefix + normalArgNum, s); /* arg0, arg1, ...*/
					normalArgNum++;
				}
			}
		}
		if (minArg >= 0 && normalArgNum < minArg) {
			throw new ParseException("too few args");
		}
		argsList = l;
		argsMap = map;
		return map;
	}

	private static final String LS = System.getProperty("line.separator");

	/**
	 * Set the description of this command.
	 *
	 * @param description
	 */
	public void setDescription(String description) {
		if (description != null) {
			helpHeader = LS + description + LS + helpHeader;
		}
	}

	/**
	 * Add help message at footer.
	 *
	 * @param optName option name
	 * @param desc description
	 */
	public void addHelp(String optName, String desc) {
		if (helpFooter == null) {
			helpFooter = name + " Options:";
		}
		helpFooter += LS + " " + optName + " ";
		for (int i = 0; i < 31 - optName.length(); i++) {
			helpFooter += " ";
		}
		helpFooter += desc;
	}

	/**
	 * Get RNSClient instance.
	 *
	 * @return RNSClient instance
	 */
	public RNSClient getRNSClient() {
		return rnsClient;
	}

	/**
	 * Get RNSExtensionClient instance.
	 *
	 * @return RNSExtensionClient instance
	 */
	public RNSExtensionClient getRNSExtensionClient() {
		return extClient;
	}

	/**
	 * Get RNSSearchClient instance.
	 *
	 * @return RNSSearchClient instance
	 */
	public RNSSearchClient getRNSSearchClient() {
		return searchClient;
	}

	/**
	 * Print help of usage messages.
	 *
	 * @param ps PrintStream for output
	 */
	public void printUsage(PrintStream ps) {
		ps.print(getUsage());
	}

	/**
	 * Print error message.
	 *
	 * @param e Exception to print
	 * @param ps PrintStream for output
	 */
	public void printError(Throwable e, PrintStream ps) {
		if (isDebugMode() == false) {
			if (e instanceof RNSError) {
				ps.println(e.getMessage());
				Throwable cause = e.getCause();
				if (cause != null) {
					if (cause instanceof AxisFault) {
						AxisFault af = (AxisFault) cause;
						ps.println(af.getFaultString());
					} else {
						cause.printStackTrace(ps);
					}
				} else {

				}
			} else if (e instanceof ParseException
					|| e instanceof org.globus.common.ChainedIOException) {
				ps.println(e.toString());
			} else {
				e.printStackTrace(ps);
			}
		} else {
			if (e instanceof RNSError) {
				Throwable cause = e.getCause();
				if (cause != null) {
					cause.printStackTrace(ps);
				}
			}
			e.printStackTrace(ps);
		}
	}

	/* for RNSClient, RNSListIterator */
	/**
	 * Print debug message.
	 *
	 * @param method method name
	 * @param path pathname
	 * @param msg message
	 */
	void debug(String method, String path, String msg) {
		if (isDebugMode()) {
			String out = "RNSClient[" + method + "]";
			if (path != null) {
				out += ": path=" + path;
			}
			if (msg != null) {
				out += ": " + msg;
			}
			System.err.println(out);
		}
	}

	/**
	 * Get a number of TCPMonitor port.
	 *
	 * @return port number
	 */
	public int getTCPMonitorPort() {
		return tcpMonitorPort;
	}

	/**
	 * Set a number of TCPMonitor port.
	 *
	 * @param port port number
	 */
	public void setTCPMonitorPort(int port) {
		tcpMonitorPort = port;
	}

	private void replacePortNumberForTCPMonitor(EndpointReferenceType epr,
			String path) throws MalformedURIException {
		if (tcpMonitorPort > 0) {
			AttributedURIType address = epr.getAddress();
			URI uri = address.getValue();
			uri.setPort(tcpMonitorPort);
			address.setValue(uri);
		}
	}

	/* synchronized PortTypes---------------------------- */

	private final static Object portTypeLock = new Object(); /* protect AXIS library */

	private static class SynchronizedRNSPortType implements RNSPortType {
		private RNSPortType port;

		SynchronizedRNSPortType(RNSPortType port) {
			this.port = port;
		}

		@Override
		public AddResponseType add(AddRequestType arg0) throws RemoteException,
				WriteNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.add(arg0);
			}
		}

		@Override
		public DestroyResponse destroy(Destroy arg0) throws RemoteException,
				ResourceUnknownFaultType, ResourceNotDestroyedFaultType,
				ResourceUnavailableFaultType {
			synchronized (portTypeLock) {
				return port.destroy(arg0);
			}
		}

		@Override
		public GetMultipleResourcePropertiesResponse getMultipleResourceProperties(
				GetMultipleResourceProperties_Element arg0)
				throws RemoteException, InvalidResourcePropertyQNameFaultType,
				ResourceUnknownFaultType, ResourceUnavailableFaultType {
			synchronized (portTypeLock) {
				return port.getMultipleResourceProperties(arg0);
			}
		}

		@Override
		public GetResourcePropertyResponse getResourceProperty(QName arg0)
				throws RemoteException, InvalidResourcePropertyQNameFaultType,
				ResourceUnknownFaultType, ResourceUnavailableFaultType {
			synchronized (portTypeLock) {
				return port.getResourceProperty(arg0);
			}
		}

		@Override
		public LookupResponseType lookup(LookupRequestType arg0)
				throws RemoteException, ReadNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.lookup(arg0);
			}
		}

		@Override
		public RemoveResponseType remove(RemoveRequestType arg0)
				throws RemoteException, WriteNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.remove(arg0);
			}
		}

		@Override
		public RenameResponseType rename(RenameRequestType arg0)
				throws RemoteException, WriteNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.rename(arg0);
			}
		}

		@Override
		public SetMetadataResponseType setMetadata(SetMetadataRequestType arg0)
				throws RemoteException, WriteNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.setMetadata(arg0);
			}
		}
	}

	private static class SynchronizedWSIteratorPortType implements
			WSIteratorPortType {
		private WSIteratorPortType port;

		public SynchronizedWSIteratorPortType(WSIteratorPortType port) {
			this.port = port;
		}

		@Override
		public GetMultipleResourcePropertiesResponse getMultipleResourceProperties(
				GetMultipleResourceProperties_Element arg0)
				throws RemoteException, InvalidResourcePropertyQNameFaultType,
				ResourceUnknownFaultType, ResourceUnavailableFaultType {
			synchronized (portTypeLock) {
				return port.getMultipleResourceProperties(arg0);
			}
		}

		@Override
		public GetResourcePropertyResponse getResourceProperty(QName arg0)
				throws RemoteException, InvalidResourcePropertyQNameFaultType,
				ResourceUnknownFaultType, ResourceUnavailableFaultType {
			synchronized (portTypeLock) {
				return port.getResourceProperty(arg0);
			}
		}

		@Override
		public IterateResponseType iterate(IterateRequestType arg0)
				throws RemoteException {
			synchronized (portTypeLock) {
				return port.iterate(arg0);
			}
		}
	}

	private static class SynchronizedRNSExtensionPortType implements
			RNSExtensionPortType {
		private RNSExtensionPortType port;

		SynchronizedRNSExtensionPortType(RNSExtensionPortType port) {
			this.port = port;
		}

		@Override
		public GetACLResponseType getACL(GetACLRequestType arg0)
				throws RemoteException, ACLFaultType {
			synchronized (portTypeLock) {
				return port.getACL(arg0);
			}
		}

		@Override
		public GetCallerInfoResponseType getCallerInfo(
				GetCallerInfoRequestType arg0) throws RemoteException {
			synchronized (portTypeLock) {
				return port.getCallerInfo(arg0);
			}
		}

		@Override
		public GetMultipleResourcePropertiesResponse getMultipleResourceProperties(
				GetMultipleResourceProperties_Element arg0)
				throws RemoteException, InvalidResourcePropertyQNameFaultType,
				ResourceUnknownFaultType, ResourceUnavailableFaultType {
			synchronized (portTypeLock) {
				return port.getMultipleResourceProperties(arg0);
			}
		}

		@Override
		public GetResourcePropertyResponse getResourceProperty(QName arg0)
				throws RemoteException, InvalidResourcePropertyQNameFaultType,
				ResourceUnknownFaultType, ResourceUnavailableFaultType {
			synchronized (portTypeLock) {
				return port.getResourceProperty(arg0);
			}
		}

		@Override
		public GetServerStatusResponseType getServerStatus(
				GetServerStatusRequestType arg0) throws RemoteException,
				ReadNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.getServerStatus(arg0);
			}
		}

		@Override
		public NoopResponseType noop(NoopRequestType arg0)
				throws RemoteException {
			synchronized (portTypeLock) {
				return port.noop(arg0);
			}
		}

		@Override
		public RemoveACLResponseType removeACL(RemoveACLRequestType arg0)
				throws RemoteException, ACLFaultType {
			synchronized (portTypeLock) {
				return port.removeACL(arg0);
			}
		}

		@Override
		public SetACLResponseType setACL(SetACLRequestType arg0)
				throws RemoteException, ACLFaultType {
			synchronized (portTypeLock) {
				return port.setACL(arg0);
			}
		}

		@Override
		public StartProfileResponseType startProfile(
				StartProfileRequestType arg0) throws RemoteException,
				ReadNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.startProfile(arg0);
			}
		}

		@Override
		public StopProfileResponseType stopProfile(StopProfileRequestType arg0)
				throws RemoteException, ReadNotPermittedFaultType {
			synchronized (portTypeLock) {
				return port.stopProfile(arg0);
			}
		}
	}

	private static class SynchronizedRNSSearchPortType implements
			RNSSearchPortType {
		private RNSSearchPortType port;

		SynchronizedRNSSearchPortType(RNSSearchPortType port) {
			this.port = port;
		}

		@Override
		public SearchResponseType search(SearchRequestType arg0)
				throws RemoteException, ReadNotPermittedFaultType,
				SearchFaultType {
			synchronized (portTypeLock) {
				return port.search(arg0);
			}
		}
	}

	/* portType provider methods ---------------------------------------- */

	private RNSServiceAddressingLocator rnsLocator = new RNSServiceAddressingLocator();

	/**
	 * Get RNSPortType from EPR.
	 *
	 * @param epr EPR
	 * @param path pathname for error message
	 * @return RNSPortType
	 * @throws Exception
	 */
	public RNSPortType getRNSPortType(EndpointReferenceType epr, String path)
			throws Exception {
		replacePortNumberForTCPMonitor(epr, path);
		RNSPortType port = rnsLocator.getRNSPortTypePort(epr);
		setOptions((Stub) port);
		if (getCacheTimeout() > 0) {
			port = new RNSPortTypeCache(port, this);
		}
		return new SynchronizedRNSPortType(port);
	}

	private WSIteratorServiceAddressingLocator iteratorLocator = new WSIteratorServiceAddressingLocator();

	/**
	 * Get WSIteratorPortType from EPR.
	 *
	 * @param epr EPR
	 * @param path pathanem for error message
	 * @return WSIteratorPortType
	 * @throws Exception
	 */
	public WSIteratorPortType getWSIteratorPortType(EndpointReferenceType epr,
			String path) throws Exception {
		replacePortNumberForTCPMonitor(epr, path);
		WSIteratorPortType port = iteratorLocator.getWSIteratorPortTypePort(epr);
		setOptions((Stub) port);
		return new SynchronizedWSIteratorPortType(port);
	}

	private RNSExtensionServiceAddressingLocator extLocator = new RNSExtensionServiceAddressingLocator();

	/**
	 * Get RNSExtensionPortType from EPR.
	 *
	 * @param epr EPR
	 * @param path pathname for error message
	 * @return RNSExtensionPortType
	 * @throws Exception
	 */
	public RNSExtensionPortType getRNSExtensionPortType(
			EndpointReferenceType epr, String path) throws Exception {
		replacePortNumberForTCPMonitor(epr, path);
		RNSExtensionPortType port = extLocator.getRNSExtensionPortTypePort(epr);
		setOptions((Stub) port);
		return new SynchronizedRNSExtensionPortType(port);
	}

	private RNSSearchServiceAddressingLocator searchLocator = new RNSSearchServiceAddressingLocator();

	/**
	 * Get RNSSearchPortType from EPR.
	 *
	 * @param epr EPR
	 * @param path pathname for error message
	 * @return RNSSearchPortType
	 * @throws Exception
	 */
	public RNSSearchPortType getRNSSearchPortType(EndpointReferenceType epr,
			String path) throws Exception {
		replacePortNumberForTCPMonitor(epr, path);
		RNSSearchPortType port = searchLocator.getRNSSearchPortTypePort(epr);
		setOptions((Stub) port);
		return new SynchronizedRNSSearchPortType(port);
	}
}
