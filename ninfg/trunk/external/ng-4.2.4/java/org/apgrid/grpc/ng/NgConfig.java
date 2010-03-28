/**
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
 * $RCSfile: NgConfig.java,v $ $Revision: 1.105 $ $Date: 2006/09/12 08:26:29 $
 */

package org.apgrid.grpc.ng;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Properties;
import java.util.Stack;
import java.util.StringTokenizer;
import java.util.Vector;

import org.apgrid.grpc.ng.info.MDSInfo;
import org.apgrid.grpc.ng.info.RemoteClassPathInfo;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.apgrid.grpc.ng.info.InvokeServerInfo;
import org.gridforum.gridrpc.GrpcException;

class NgConfig {
	private List serverInfo = new Vector();
	private List mdsInfo = new Vector();
	private List functionList = new Vector();
	private Properties localHostInfo = null;
	private List localLDIFFiles = new Vector();
	private Properties serverDefault = null;
	private Properties tmpProperties = null;
	private int state = NGCONFIG_STATE_NONE;
	private List listIncludeFiles = new Vector();
	private List invokeServerInfo = new Vector();
	
	private static String envLogLevel = "ninfg.logLevel";
	
	/* char for comment */
	private static final int NGCONFIG_CHAR_COMMENT = '#';
	
	/* tags */
	private static int NGCONFIG_TAG_NONE = 0;
	private static int NGCONFIG_TAG_START_INCLUDE = 1;
	private static int NGCONFIG_TAG_END_INCLUDE = 2;
	private static int NGCONFIG_TAG_START_CLIENT = 3;
	private static int NGCONFIG_TAG_END_CLIENT = 4;
	private static int NGCONFIG_TAG_START_LOCAL_LDIF = 5;
	private static int NGCONFIG_TAG_END_LOCAL_LDIF = 6;
	private static int NGCONFIG_TAG_START_FUNCTION_INFO = 7;
	private static int NGCONFIG_TAG_END_FUNCTION_INFO = 8;
	private static int NGCONFIG_TAG_START_MDS_SERVER = 9;
	private static int NGCONFIG_TAG_END_MDS_SERVER = 10;
	private static int NGCONFIG_TAG_START_SERVER = 11;
	private static int NGCONFIG_TAG_END_SERVER = 12;
	private static int NGCONFIG_TAG_START_SERVER_DEFAULT = 13;
	private static int NGCONFIG_TAG_END_SERVER_DEFAULT = 14;
	private static int NGCONFIG_TAG_START_INVOKE_SERVER = 15;
	private static int NGCONFIG_TAG_END_INVOKE_SERVER = 16;
	
	/* tag string */
	private static String NGCONFIG_TAGSTR_START = "<";
	private static String NGCONFIG_TAGSTR_ENDSTR = "/";
	private static String NGCONFIG_TAGSTR_END = ">";
	private static String NGCONFIG_TAGSTR_INCLUDE = "INCLUDE";
	private static String NGCONFIG_TAGSTR_CLIENT = "CLIENT";
	private static String NGCONFIG_TAGSTR_LOCAL_LDIF = "LOCAL_LDIF";
	private static String NGCONFIG_TAGSTR_FUNCTION_INFO = "FUNCTION_INFO";
	private static String NGCONFIG_TAGSTR_MDS_SERVER = "MDS_SERVER";
	private static String NGCONFIG_TAGSTR_SERVER = "SERVER";
	private static String NGCONFIG_TAGSTR_SERVER_DEFAULT = "SERVER_DEFAULT";
	private static String NGCONFIG_TAGSTR_INVOKE_SERVER = "INVOKE_SERVER";
	
	/* status */
	private static int NGCONFIG_STATE_NONE = 0;
	private static int NGCONFIG_STATE_INCLUDE = 1;
	private static int NGCONFIG_STATE_CLIENT = 2;
	private static int NGCONFIG_STATE_LOCAL_LDIF = 3;
	private static int NGCONFIG_STATE_FUNCTION_INFO = 4;
	private static int NGCONFIG_STATE_MDS_SERVER = 5;
	private static int NGCONFIG_STATE_SERVER = 6;
	private static int NGCONFIG_STATE_SERVER_DEFAULT = 7;
	private static int NGCONFIG_STATE_INVOKE_SERVER = 8;
	
	/**
	 * @param configFile
	 * @throws GrpcException
	 */
	public NgConfig(String configFile) throws GrpcException {
		parseConfigFile(configFile);
	}
	
	/**
	 * @param prop
	 * @throws GrpcException
	 */
	public NgConfig(Properties prop) throws GrpcException {
		serverInfo.add(prop);
	}
	
	/**
	 * @return
	 */
	public List getLocalLDIFFiles() {
		return localLDIFFiles;
	}
	
	/**
	 * @param configFile
	 * @throws GrpcException
	 */
	public void parseConfigFile(String configFile) throws GrpcException {
		try {
			/* open file and Reader */
			FileReader fr = new FileReader(configFile);
			listIncludeFiles.add(configFile);
			BufferedReader br = new BufferedReader(fr);
			Stack stackReader = new Stack();
			
			/* get line and parse it */
			String line;
			while (((line = br.readLine()) != null) || ! stackReader.empty()) {
				/* if reached to the end of file, then pop Reader from Stack */
				if (line == null) {
					br.close();
					br = (BufferedReader) stackReader.pop();
					state = NGCONFIG_TAG_START_INCLUDE;
					continue;
				}

				/* check String */
				int firstElementIndex = -1;
				int secondElementIndex = -1;
				boolean inElemString = false;
				for (int i = 0; i < line.length(); i++) {
					if (Character.isWhitespace(line.charAt(i)) == false) {
						if (inElemString == false) {
							if (firstElementIndex == -1) {
								if (line.charAt(i) == NGCONFIG_CHAR_COMMENT) {
									/* it's comment */
									break;
								}
								firstElementIndex = i;
								inElemString = true;
							} else if (secondElementIndex == -1) {
								secondElementIndex = i;
								break;
							} else {
								throw new NgInitializeGrpcClientException(
									"Something wrong was happend when parse config " +
									line);
							}
						}
					} else if (inElemString == true) {
						inElemString = false;
					}
				}
				
				/* parse config attributes */
				int nTokens = 0;
				String key = null;
				String val = null;
				if (firstElementIndex == -1) {
					/* no data is in the line */
					continue;
				} else if (secondElementIndex == -1) {
					/* 1 data is in the line */
					key = line.trim();
					nTokens = 1;
				} else {
					/* more than 2 data is in the line */
					key = line.substring(
						firstElementIndex, secondElementIndex - 1).trim();
					val = line.substring(secondElementIndex);
					if (val.startsWith("\"")) {
						/* check if the key is valid for quoted variable */
						if (! (key.equals(RemoteMachineInfo.KEY_JOB_RSL_EXTENSION) ||
							key.equals(RemoteMachineInfo.KEY_SUBJECT) ||
							key.equals(RemoteMachineInfo.KEY_INVOKE_SERVER_OPTION) ||
							key.equals(MDSInfo.KEY_SUBJECT) ||
							key.equals(InvokeServerInfo.KEY_OPTION)) ) {
							throw new NgInitializeGrpcClientException(
								"Invalid quoted parameter: " + val);
						}
						
						/* check if the multiple line with invalid attribute */
						if ((val.charAt(val.length() - 1) == '\\') &&
							(! key.equals(RemoteMachineInfo.KEY_JOB_RSL_EXTENSION))) {
							throw new NgInitializeGrpcClientException(
								"Multiple line is not supported for the key: " + key);
						}
						
						/* get quoted String */
						StringBuffer sb = new StringBuffer();
						while (val.charAt(val.length() - 1) == '\\') {
							sb.append(val.substring(0, val.length() - 1));
							sb.append("\n");
							val = br.readLine();
						}
						sb.append(val);
						val = sb.toString();
						
						/* check validation and replace special characters */
						val = getQuotedVariable(val);
						
						/* count the number of elements */
						nTokens = 2;
					} else {
						/* normal String */
						if (val.indexOf(NGCONFIG_CHAR_COMMENT) != -1) {
							val = val.substring(0,
								val.indexOf(NGCONFIG_CHAR_COMMENT) - 1);
						}
						/* count the number of elements */
						StringTokenizer st = new StringTokenizer(val);
						nTokens = st.countTokens() + 1;
						
						if (nTokens == 2) {
							val = val.trim();
						}
					}
				}
				
				/* special or unrecognize line */
				if (nTokens > 2) {
					if (state == NGCONFIG_STATE_CLIENT) {
						/* multiple server are defined */
						StringTokenizer st = new StringTokenizer(val);
						processMultipleClientDefinitions(key, st);
					} else if (state == NGCONFIG_STATE_SERVER) {
						/* multiple server are defined */
						StringTokenizer st = new StringTokenizer(val);
						processMultipleServerDefinitions(key, st);
					} else {
						throw new NgInitializeGrpcClientException(
							"Invalid config parameter: " + val);
					}
				}
				/* tokens == 1 (maybe tag) */
				else if (nTokens == 1) {
					int tag = getTag(key);
					if (tag == NGCONFIG_TAG_NONE) {
						throw new NgInitializeGrpcClientException(
							"Unrecognized tag.");
					}
					if (state == NGCONFIG_STATE_NONE) {
						if (tag == NGCONFIG_TAG_START_INCLUDE) {
							state = NGCONFIG_STATE_INCLUDE;
						} else if (tag == NGCONFIG_TAG_START_CLIENT) {
							state = NGCONFIG_STATE_CLIENT;
						} else if (tag == NGCONFIG_TAG_START_LOCAL_LDIF) {
							state = NGCONFIG_STATE_LOCAL_LDIF;
						} else if (tag == NGCONFIG_TAG_START_FUNCTION_INFO) {
							state = NGCONFIG_STATE_FUNCTION_INFO;
						} else if (tag == NGCONFIG_TAG_START_MDS_SERVER) {
							state = NGCONFIG_STATE_MDS_SERVER;
						} else if (tag == NGCONFIG_TAG_START_SERVER) {
							state = NGCONFIG_STATE_SERVER;
						} else if (tag == NGCONFIG_TAG_START_SERVER_DEFAULT) {
							state = NGCONFIG_STATE_SERVER_DEFAULT;
						} else if (tag == NGCONFIG_TAG_START_INVOKE_SERVER) {
							state = NGCONFIG_STATE_INVOKE_SERVER;
						} else {
							throw new NgInitializeGrpcClientException(
								"Invalid config format.");
						}
						openSection();
						continue;
					} else if (state == NGCONFIG_STATE_INCLUDE) {
						if (tag == NGCONFIG_TAG_END_INCLUDE) {
							closeSection();
							continue;
						}
					} else if (state == NGCONFIG_STATE_CLIENT) {
						if (tag == NGCONFIG_TAG_END_CLIENT) {
							closeSection();
							continue;
						}
					} else if (state == NGCONFIG_STATE_LOCAL_LDIF) {
						if (tag == NGCONFIG_TAG_END_LOCAL_LDIF) {
							closeSection();
							continue;
						}
					} else if (state == NGCONFIG_STATE_FUNCTION_INFO) {
						if (tag == NGCONFIG_TAG_END_FUNCTION_INFO) {
							closeSection();
							continue;
						}
					} else if  (state == NGCONFIG_STATE_MDS_SERVER) {
						if (tag == NGCONFIG_TAG_END_MDS_SERVER) {
							closeSection();
							continue;
						}
					} else if  (state == NGCONFIG_STATE_SERVER) {
						if (tag == NGCONFIG_TAG_END_SERVER) {
							closeSection();
							continue;
						}
					} else if  (state == NGCONFIG_STATE_SERVER_DEFAULT) {
						if (tag == NGCONFIG_TAG_END_SERVER_DEFAULT) {
							closeSection();
							continue;
						}
					} else if  (state == NGCONFIG_STATE_INVOKE_SERVER) {
						if (tag == NGCONFIG_TAG_END_INVOKE_SERVER) {
							closeSection();
							continue;
						}
					}
					throw new NgInitializeGrpcClientException(
						"Invalid config format.");
				}
				/* tokens == 2 (maybe elements) */
				else if (nTokens == 2) {
					/* get INCLUDE file and open it */
					if (state == NGCONFIG_STATE_INCLUDE) {
						String includeFilename = getIncludeFile(key, val);
						
						/* check if it's already included */
						for (int i = 0; i < listIncludeFiles.size(); i++) {
							if (listIncludeFiles.get(i).equals(includeFilename)) {
								throw new NgInitializeGrpcClientException(
									"file " + includeFilename + " is already read.");
							}
						}
						listIncludeFiles.add(includeFilename);

						stackReader.push(br);
						FileReader newfr = new FileReader(includeFilename);
						br = new BufferedReader(newfr);
						state = NGCONFIG_STATE_NONE;
						continue;
					} else {
						/* other section */
						processTag(key, val);
					}
				}
			}
			/* close file */
			br.close();
		} catch (FileNotFoundException e) {
			throw new NgInitializeGrpcClientException(e);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}
	
	/**
	 * @param target
	 * @return
	 */
	private int getTag(String target) {
		if (target.equals(makeStartTagString(NGCONFIG_TAGSTR_INCLUDE))) {
			return NGCONFIG_TAG_START_INCLUDE;
		} else if (target.equals(makeStartTagString(NGCONFIG_TAGSTR_CLIENT))) {
				return NGCONFIG_TAG_START_CLIENT;
		} else if (target.equals(
					makeStartTagString(NGCONFIG_TAGSTR_LOCAL_LDIF))) {
			return NGCONFIG_TAG_START_LOCAL_LDIF;
		} else if (target.equals(
					makeStartTagString(NGCONFIG_TAGSTR_FUNCTION_INFO))) {
			return NGCONFIG_TAG_START_FUNCTION_INFO;
		} else if (target.equals(
					makeStartTagString(NGCONFIG_TAGSTR_MDS_SERVER))) {
			return NGCONFIG_TAG_START_MDS_SERVER;
		} else if (target.equals(
					makeStartTagString(NGCONFIG_TAGSTR_SERVER))) {
			return NGCONFIG_TAG_START_SERVER;
		} else if (target.equals(
					makeStartTagString(NGCONFIG_TAGSTR_SERVER_DEFAULT))) {
			return NGCONFIG_TAG_START_SERVER_DEFAULT;
		} else if (target.equals(
				makeStartTagString(NGCONFIG_TAGSTR_INVOKE_SERVER))) {
			return NGCONFIG_TAG_START_INVOKE_SERVER;
		} else if (target.equals(
					makeEndTagString(NGCONFIG_TAGSTR_INCLUDE))) {
			return NGCONFIG_TAG_END_INCLUDE;
		} else if (target.equals(
					makeEndTagString(NGCONFIG_TAGSTR_CLIENT))) {
			return NGCONFIG_TAG_END_CLIENT;
		} else if (target.equals(
					makeEndTagString(NGCONFIG_TAGSTR_LOCAL_LDIF))) {
			return NGCONFIG_TAG_END_LOCAL_LDIF;
		} else if (target.equals(
					makeEndTagString(NGCONFIG_TAGSTR_FUNCTION_INFO))) {
			return NGCONFIG_TAG_END_FUNCTION_INFO;
		} else if (target.equals(
					makeEndTagString(NGCONFIG_TAGSTR_MDS_SERVER))) {
			return NGCONFIG_TAG_END_MDS_SERVER;
		} else if (target.equals(
					makeEndTagString(NGCONFIG_TAGSTR_SERVER))) {
			return NGCONFIG_TAG_END_SERVER;
		} else if (target.equals(
					makeEndTagString(NGCONFIG_TAGSTR_SERVER_DEFAULT))) {
			return NGCONFIG_TAG_END_SERVER_DEFAULT;
		} else if (target.equals(
				makeEndTagString(NGCONFIG_TAGSTR_INVOKE_SERVER))) {
			return NGCONFIG_TAG_END_INVOKE_SERVER;
		}
		return NGCONFIG_TAG_NONE;
	}
	
	/**
	 * @param target
	 * @return
	 */
	private String makeStartTagString(String target) {
		return NGCONFIG_TAGSTR_START + target +	NGCONFIG_TAGSTR_END;
	}
	
	/**
	 * @param target
	 * @return
	 */
	private String makeEndTagString(String target) {
		return NGCONFIG_TAGSTR_START + 
				NGCONFIG_TAGSTR_ENDSTR + target + NGCONFIG_TAGSTR_END;
	}
	
	/**
	 * 
	 */
	private void closeSection()
		throws UnknownHostException, GrpcException {
		if (state == NGCONFIG_STATE_INCLUDE) {
			/* nothing */
		} else if (state == NGCONFIG_STATE_CLIENT) {
			if (localHostInfo != null) {
				throw new NgInitializeGrpcClientException(
					"NGCONFIG: multiple CLIENT section");
			}
			/* check if loglevel was specified */
			if (tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL)) {
				Object logLevel = tmpProperties.get(SECTION_CLIENT_LOGLEVEL);
				/* check if logleval_globusToolkit was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_GT)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_GT, logLevel);
				}
				/* check if logleval_ninfgProtocol was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_NGPROT)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_NGPROT, logLevel);
				}
				/* check if logleval_ninfgInternal was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_NGINT)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_NGINT, logLevel);
				}
				/* check if logleval_ninfgGrpc was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_NGGRPC)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_NGGRPC, logLevel);
				}
			}
			/* check if NG_LOG_LEVEL was set */
			else if (System.getProperty(envLogLevel) != null) {
				Object logLevel = System.getProperty(envLogLevel);
				/* check if logleval_globusToolkit was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_GT)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_GT, logLevel);
				}
				/* check if logleval_ninfgProtocol was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_NGPROT)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_NGPROT, logLevel);
				}
				/* check if logleval_ninfgInternal was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_NGINT)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_NGINT, logLevel);
				}
				/* check if logleval_ninfgGrpc was not specified */
				if (!tmpProperties.containsKey(SECTION_CLIENT_LOGLEVEL_NGGRPC)) {
					tmpProperties.put(SECTION_CLIENT_LOGLEVEL_NGGRPC, logLevel);
				}
			}
			/* put read data into localHostInfo */
			localHostInfo = tmpProperties;
		} else if (state == NGCONFIG_STATE_LOCAL_LDIF) {
			/* nothing */
		} else if (state == NGCONFIG_STATE_FUNCTION_INFO) {
			for (int i = 0; i < functionList.size(); i++) {
				Properties prop = (Properties) functionList.get(i);
				if (makeFunctionInfoKey(tmpProperties).equals(
					makeFunctionInfoKey(prop))) {
					throw new NgInitializeGrpcClientException(
						"NGCONFIG: multiple FUNCTION_INFO section");
				}
			}
		
			/* check can't omitted element */
			if (tmpProperties.containsKey(SECTION_FUNCINFO_HOSTNAME) != true) {
				throw new NgException(SECTION_FUNCINFO_HOSTNAME +
					" is missing in FUNCINFO section.");
			}
			if (tmpProperties.containsKey(SECTION_FUNCINFO_FUNCNAME) != true) {
				throw new NgException(SECTION_FUNCINFO_FUNCNAME +
					" is missing in FUNCINFO section.");
			}
			if (tmpProperties.containsKey(SECTION_FUNCINFO_PATH) != true) {
				throw new NgException(SECTION_FUNCINFO_PATH +
					" is missing in FUNCINFO section.");
			}

			functionList.add(tmpProperties);
		} else if (state == NGCONFIG_STATE_MDS_SERVER) {
			for (int i = 0; i < mdsInfo.size(); i++) {
				MDSInfo info = (MDSInfo) mdsInfo.get(i);
				String mdsHostName = (String) tmpProperties.get(SECTION_MDS_HOSTNAME);
				String mdsTag = (String) tmpProperties.get(SECTION_MDS_TAG);
				if ((mdsTag != null) && mdsTag.equals(info.get(SECTION_MDS_TAG))) {
					throw new NgInitializeGrpcClientException(
						"NGCONFIG: multiple MDS_SERVER section");
				} else if (((mdsTag == null) && (info.get(SECTION_MDS_TAG) == null)) &&
					(mdsHostName.equals(info.get(SECTION_MDS_HOSTNAME)))) {
					throw new NgInitializeGrpcClientException(
						"NGCONFIG: multiple MDS_SERVER section");
				}
			}

			/* check can't omitted element */
			if (tmpProperties.containsKey(SECTION_MDS_HOSTNAME) != true) {
				throw new NgException(SECTION_MDS_HOSTNAME +
					" is missing in MDS_SERVER section.");
			}
			mdsInfo.add(new MDSInfo(tmpProperties));
		} else if (state == NGCONFIG_STATE_SERVER) {
			/* check can't omitted element */
			if (!tmpProperties.containsKey(SECTION_SERVER_HOSTNAME)) {
				throw new NgException(SECTION_SERVER_HOSTNAME +
					" is missing in SERVER section.");
			}
			
			/* check mds_hostname and mds_tag */
			if (tmpProperties.containsKey(SECTION_SERVER_MDS_HOSTNAME) &&
				tmpProperties.containsKey(SECTION_SERVER_MDS_TAG)) {
				throw new NgException(SECTION_SERVER_MDS_HOSTNAME + " and " +
					SECTION_SERVER_MDS_TAG +
					" is not able to set in same SERVER section.");
			}

			/* can't define multiple hostname in <SERVER> with tag */
			List hostnames = (List) tmpProperties.get(SECTION_SERVER_HOSTNAME);
			if ((tmpProperties.containsKey(SECTION_SERVER_TAG)) &&
				hostnames.size() > 1) {
				throw new NgInitializeGrpcClientException(
					"NGCONFIG: multiple SERVER section with tag : " +
					tmpProperties.get(SECTION_SERVER_TAG));						
			}

			/* check if the tag already registered */
			String currentHost = (String)((List) tmpProperties.get(SECTION_SERVER_HOSTNAME)).get(0);
			String currentTag = (String) tmpProperties.get(SECTION_SERVER_TAG);
			for (int i = 0; i < serverInfo.size(); i++) {
				Properties prop = (Properties) serverInfo.get(i);
				if ((currentTag != null) &&
					(currentTag.equals(prop.get(SECTION_SERVER_TAG)))) {
					/* tag is matched, it's already registered */
					throw new NgInitializeGrpcClientException(
						"NGCONFIG: multiple SERVER section with tag : " +
						tmpProperties.get(SECTION_SERVER_TAG));
				}
			}

			/* Check multiple definitions */
			int nHosts = hostnames.size();
			for (int i = 0; i < nHosts; i++) {
				for (int j = i+1; j < nHosts; j++) {
					if (hostnames.get(i).equals(hostnames.get(j))) {
						throw new NgInitializeGrpcClientException(
							"NGCONFIG: multiple SERVER section with hostname: " +
							hostnames.get(i));
					}
				}
			}
			for (int i = 0; i < hostnames.size(); i++) {
				for (int j = 0; j < serverInfo.size(); j++) {
					String hostname = (String) hostnames.get(i);
					Properties prop = (Properties) serverInfo.get(j);
					if (hostname.equals(prop.get(SECTION_SERVER_HOSTNAME))) {
						/* hostname is matched */
						String targetTag = prop.getProperty(SECTION_SERVER_TAG);
						if (((currentTag == null) && (targetTag == null)) ||
							((currentTag != null) && (targetTag != null) &&
							currentTag.equals(targetTag))) {
							/* tag is also matched */
							throw new NgInitializeGrpcClientException(
								"NGCONFIG: multiple SERVER section with hostname: " +
								hostname);					
						}
					}
				}
			}
			/* separate multiple hostname element and put into serverInfo */
			for (int i = 0; i < hostnames.size(); i++) {
				Properties prop = (Properties) tmpProperties.clone();
				registerHostnameParam(prop,
					SECTION_SERVER_HOSTNAME, (String) hostnames.get(i));
				
				/* create new List for new Properties */
				if (prop.containsKey(SECTION_SERVER_ENVIRONMENT)) {
					List listEnv = (List) prop.get(SECTION_SERVER_ENVIRONMENT);
					List listEnvNew = new Vector();
					for (int j = 0; j < listEnv.size(); j++) {
						listEnvNew.add(listEnv.get(j));
					}
					prop.put(SECTION_SERVER_ENVIRONMENT, listEnvNew);
				}
				if (prop.containsKey(SECTION_SERVER_INVOKE_SERVER_OPTION)) {
					List listISOpt = (List) prop.get(SECTION_SERVER_INVOKE_SERVER_OPTION);
					List listISOptNew = new Vector();
					for (int j = 0; j < listISOpt.size(); j++) {
						listISOptNew.add(listISOpt.get(j));
					}
					prop.put(SECTION_SERVER_INVOKE_SERVER_OPTION, listISOptNew);
				}
				
				/* append new ServerInfo */
				serverInfo.add(prop);
			}
		} else if (state == NGCONFIG_STATE_SERVER_DEFAULT) {
			if (serverDefault != null) {
				throw new NgInitializeGrpcClientException(
					"NGCONFIG: multiple SERVER_DEFAULT section");
			}
			/* check mds_hostname and mds_tag */
			if (tmpProperties.containsKey(SECTION_SERVER_MDS_HOSTNAME) &&
				tmpProperties.containsKey(SECTION_SERVER_MDS_TAG)) {
				throw new NgException(SECTION_SERVER_MDS_HOSTNAME + " and " +
					SECTION_SERVER_MDS_TAG +
					" is not able to set in same SERVER section.");
			}

			serverDefault = tmpProperties;
		} else if (state == NGCONFIG_STATE_INVOKE_SERVER) {
			for (int i = 0; i < invokeServerInfo.size(); i++) {
				Properties prop = (Properties) invokeServerInfo.get(i);
				if (tmpProperties.get(InvokeServerInfo.KEY_TYPE).equals(
					prop.get(InvokeServerInfo.KEY_TYPE))) {
					throw new NgInitializeGrpcClientException(
						"NGCONFIG: multiple INVOKE_SERVER section");
				}
			}
		
			/* check can't omitted element */
			if (tmpProperties.containsKey(SECTION_INVOKE_SERVER_TYPE) != true) {
				throw new NgException(SECTION_INVOKE_SERVER_TYPE +
					" is missing in INVOKE_SERVER section.");
			}

			invokeServerInfo.add(tmpProperties);
		}
		
		/* reset temporary area */
		state = NGCONFIG_STATE_NONE;
		tmpProperties = null;
	}
	
	/**
	 * 
	 */
	private void openSection() {
		if (state == NGCONFIG_STATE_LOCAL_LDIF) {
			/* nothing */
		} else {
			tmpProperties = new Properties();
		}
	}
	
	/**
	 * @param st
	 * @throws GrpcException
	 */
	private void processTag(String key, String value) throws GrpcException {
		if (state == NGCONFIG_STATE_CLIENT) {
			processClient(key, value);
		} else if (state == NGCONFIG_STATE_LOCAL_LDIF) {
			processLocalLdif(key, value);
		} else if (state == NGCONFIG_STATE_FUNCTION_INFO) {
			processFunctionInfo(key, value);
		} else if (state == NGCONFIG_STATE_MDS_SERVER) {
			processMDSServer(key, value);
		} else if (state == NGCONFIG_STATE_SERVER) {
			processServer(key, value);
		} else if (state == NGCONFIG_STATE_SERVER_DEFAULT) {
			processServerDefault(key, value);
		} else if (state == NGCONFIG_STATE_INVOKE_SERVER) {
			processInvokeServerInfo(key, value);
		}
	}
	
	/* parameters for INCLUDE section */
	private static final String SECTION_INCLUDE_FILENAME = "filename";

	/**
	 * @param key
	 * @param value
	 * @return
	 * @throws GrpcException
	 */
	private String getIncludeFile(String key, String value) throws GrpcException {
		if (checkIncludeParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse INCLUDE section.");
		} else if (key.equals(SECTION_INCLUDE_FILENAME)) {
			return value;
		} else {
			throw new NgInitializeGrpcClientException(
				"Failed to parse INCLUDE section.");			
		}
	}
	
	/* parameters for CLIENT section */
	private static final String SECTION_CLIENT_HOSTNAME = 
		NgInformationManager.KEY_CLIENT_HOSTNAME;
	private static final String SECTION_CLIENT_SAVE_SESSIONINFO =
		NgInformationManager.KEY_CLIENT_SAVE_SESSIONINFO;
	private static final String SECTION_CLIENT_LOGLEVEL =
		NgInformationManager.KEY_CLIENT_LOGLEVEL;
	private static final String SECTION_CLIENT_LOGLEVEL_GT =
		NgInformationManager.KEY_CLIENT_LOGLEVEL_GT;
	private static final String SECTION_CLIENT_LOGLEVEL_NGPROT =
		NgInformationManager.KEY_CLIENT_LOGLEVEL_NGPROT;
	private static final String SECTION_CLIENT_LOGLEVEL_NGINT =
		NgInformationManager.KEY_CLIENT_LOGLEVEL_NGINT;
	private static final String SECTION_CLIENT_LOGLEVEL_NGGRPC =
		NgInformationManager.KEY_CLIENT_LOGLEVEL_NGGRPC;
	private static final String SECTION_CLIENT_LOG_FILEPATH =
		NgInformationManager.KEY_CLIENT_LOG_FILEPATH;
	private static final String SECTION_CLIENT_LOG_SUFFIX =
		NgInformationManager.KEY_CLIENT_LOG_SUFFIX;
	private static final String SECTION_CLIENT_LOG_NFILES =
		NgInformationManager.KEY_CLIENT_LOG_NFILES;
	private static final String SECTION_CLIENT_LOG_MAXFILESIZE =
		NgInformationManager.KEY_CLIENT_LOG_MAXFILESIZE;
	private static final String SECTION_CLIENT_LOG_OVERWRITEDIR =
		NgInformationManager.KEY_CLIENT_LOG_OVERWRITEDIR;
	private static final String SECTION_CLIENT_REFRESH_CREDENTIAL =
		NgInformationManager.KEY_CLIENT_REFRESH_CREDENTIAL;
	private static final String SECTION_CLIENT_INVOKE_SERVER_LOG =
		NgInformationManager.KEY_CLIENT_INVOKE_SERVER_LOG;
	private static final String SECTION_CLIENT_TMP_DIR =
		NgInformationManager.KEY_CLIENT_TMP_DIR;
	private static final String SECTION_CLIENT_FORTRAN_COMPATIBLE =
		NgInformationManager.KEY_CLIENT_FORTRAN_COMPATIBLE;
	private static final String SECTION_CLIENT_HANDLING_SIGNALS =
		NgInformationManager.KEY_CLIENT_HANDLING_SIGNALS;
	private static final String SECTION_CLIENT_LISTEN_PORT_RAW =
		NgInformationManager.KEY_CLIENT_LISTEN_PORT_RAW;
	private static final String SECTION_CLIENT_LISTEN_PORT_AUTHONLY =
		NgInformationManager.KEY_CLIENT_LISTEN_PORT_AUTHONLY;
	private static final String SECTION_CLIENT_LISTEN_PORT_GSI =
		NgInformationManager.KEY_CLIENT_LISTEN_PORT_GSI;
	private static final String SECTION_CLIENT_LISTEN_PORT_SSL =
		NgInformationManager.KEY_CLIENT_LISTEN_PORT_SSL;
		
	/**
	 * @param key
	 * @return
	 */
	private boolean checkIncludeParameter(String key) {
		if (key.equals(SECTION_INCLUDE_FILENAME)){
			return true;
		}
		return false;
	}
	
	/**
	 * @param key
	 * @param value
	 * @throws GrpcException
	 */
	private void processClient(String key, String value) throws GrpcException {
		if (checkClientParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse CLIENT section(the key (" +
				key + ") is not valid.)");
		} else if (tmpProperties.containsKey(key)) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse CLIENT section(the key (" +
				key + ") already exists.");	
		} else if (key.equals(SECTION_CLIENT_LOG_MAXFILESIZE)) {
			registerClient(key, convertSizeValue(value));
		} else {
			registerClient(key, value);
		}
	}
	
	/**
	 * @param key
	 * @return
	 */
	private boolean checkClientParameter(String key) {
		if (key.equals(SECTION_CLIENT_HOSTNAME)||
			key.equals(SECTION_CLIENT_SAVE_SESSIONINFO) ||
			key.equals(SECTION_CLIENT_LOGLEVEL) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_GT) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_NGPROT) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_NGINT) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_NGGRPC) ||
			key.equals(SECTION_CLIENT_LOG_FILEPATH) ||
			key.equals(SECTION_CLIENT_LOG_SUFFIX) ||
			key.equals(SECTION_CLIENT_LOG_NFILES) ||
			key.equals(SECTION_CLIENT_LOG_MAXFILESIZE) ||
			key.equals(SECTION_CLIENT_LOG_OVERWRITEDIR) ||
			key.equals(SECTION_CLIENT_REFRESH_CREDENTIAL) ||
			key.equals(SECTION_CLIENT_INVOKE_SERVER_LOG) ||
			key.equals(SECTION_CLIENT_TMP_DIR) ||
			key.equals(SECTION_CLIENT_FORTRAN_COMPATIBLE) ||
			key.equals(SECTION_CLIENT_HANDLING_SIGNALS) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_RAW) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_AUTHONLY) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_GSI) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_SSL)) {
				return true;
		}
		return false;
	}
	
	/**
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerClient(
		String key, String val) throws GrpcException {
		/* Check String variables */
		if (key.equals(SECTION_CLIENT_HOSTNAME)||
			key.equals(SECTION_CLIENT_LOG_FILEPATH) ||
			key.equals(SECTION_CLIENT_LOG_SUFFIX) ||
			key.equals(SECTION_CLIENT_INVOKE_SERVER_LOG) ||
			key.equals(SECTION_CLIENT_TMP_DIR)) {
			registerStringParam(tmpProperties, key, val);
				
		}
		/* Check Numeric variables */
		else if (key.equals(SECTION_CLIENT_LOG_NFILES) ||
			key.equals(SECTION_CLIENT_LOG_MAXFILESIZE) ||
			key.equals(SECTION_CLIENT_REFRESH_CREDENTIAL) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_RAW) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_AUTHONLY) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_GSI) ||
			key.equals(SECTION_CLIENT_LISTEN_PORT_SSL)) {
			registerNumericParam(tmpProperties, key, val);
		}
		/* Check LogLevel variables */
		else if (key.equals(SECTION_CLIENT_LOGLEVEL) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_GT) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_NGPROT) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_NGINT) ||
			key.equals(SECTION_CLIENT_LOGLEVEL_NGGRPC)) {
			registerLogLevelParam(tmpProperties, key, val);
		}
		/* Check Boolean variables */
		else if (key.equals(SECTION_CLIENT_LOG_OVERWRITEDIR)) {
			registerBooleanParam(tmpProperties, key, val);
		}
		/* unsupported attribute */
		else if (key.equals(SECTION_CLIENT_SAVE_SESSIONINFO) ||
			key.equals(SECTION_CLIENT_FORTRAN_COMPATIBLE) ||
			key.equals(SECTION_CLIENT_HANDLING_SIGNALS)) {
			putNotSupportedWarning(key);
		}
		/* unrecognized key */
		else {
			throw new NgException(key + " is not keyword for CLIENT.");
		}
	}
	
	/* parameters for LOCAL_LDIF section */
	private static final String SECTION_LOCALLDIF_FILENAME = "filename";

	/**
	 * @param key
	 * @param value
	 * @throws GrpcException
	 */
	private void processLocalLdif(String key, String value) throws GrpcException {
		if (checkLocalLdifParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse LOCAL_LDIF section.(the key (" +
				key + ") is not valid.");
		} else if (key.equals(SECTION_LOCALLDIF_FILENAME)) {
			if (localLDIFFiles.contains(value) == true) {
				throw new NgInitializeGrpcClientException(
					"Failed to parse LOCAL_LDIF section(the value (" +
					 value + ") already exists.");
			}
			localLDIFFiles.add(value);
		}
	}
	
	/**
	 * @param key
	 * @return
	 */
	private boolean checkLocalLdifParameter(String key) {
		if (key.equals(SECTION_LOCALLDIF_FILENAME)){
			return true;
		}
		return false;
	}
	
	/* parameters for FUNCTION_INFO section */
	private static final String SECTION_FUNCINFO_HOSTNAME = "hostname";
	private static final String SECTION_FUNCINFO_FUNCNAME = "funcname";
	private static final String SECTION_FUNCINFO_STAGING = "staging";
	private static final String SECTION_FUNCINFO_PATH = "path";
	private static final String SECTION_FUNCINFO_BACKEND = "backend";
	private static final String SECTION_FUNCINFO_SESSION_TIMEOUT = "session_timeout";

	/**
	 * @param key
	 * @param value
	 * @throws GrpcException
	 */
	private void processFunctionInfo(String key, String value) throws GrpcException {
		if (checkFunctionInfoParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse FUNCTION_INFO section.(the key (" +
				key + ") is not valid.");
		} else if (tmpProperties.containsKey(key)) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse FUNCTION_INFO section(the key (" +
				key + ")) already exists.");	
		} else {
			registerFunctionInfo(key, value);
		}
	}
	
	/**
	 * @param key
	 * @return
	 */
	private boolean checkFunctionInfoParameter(String key) {
		if (key.equals(SECTION_FUNCINFO_HOSTNAME)||
			key.equals(SECTION_FUNCINFO_FUNCNAME) ||
			key.equals(SECTION_FUNCINFO_STAGING) ||
			key.equals(SECTION_FUNCINFO_PATH) ||
			key.equals(SECTION_FUNCINFO_BACKEND) ||
			key.equals(SECTION_FUNCINFO_SESSION_TIMEOUT)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerFunctionInfo(
		String key, String val) throws GrpcException {
		/* check String Parameter */
		if (key.equals(SECTION_FUNCINFO_HOSTNAME)||
			key.equals(SECTION_FUNCINFO_FUNCNAME) ||
			key.equals(SECTION_FUNCINFO_PATH) ||
			key.equals(SECTION_FUNCINFO_BACKEND)) {
			registerStringParam(tmpProperties, key, val);
		}
		/* check Boolean Parameter */
		else if (key.equals(SECTION_FUNCINFO_STAGING)) {
			registerBooleanParam(tmpProperties, key, val);
		}
		/* check Integer Parameter */
		else if (key.equals(SECTION_FUNCINFO_SESSION_TIMEOUT)) {
			registerNumericParam(tmpProperties, key, convertTimeValue(val));
		}
		/* unrecognized key */
		else {
			throw new NgException(key + " is not keyword for FUNCTION_INFO.");
		}
	}
	
	/**
	 * @param prop
	 * @return
	 */
	private String makeFunctionInfoKey(Properties prop) {
		return prop.get(SECTION_FUNCINFO_HOSTNAME) + ":" + 
			prop.get(SECTION_FUNCINFO_FUNCNAME);
	}
	
	/* parameters for MDS_SERVER section */
	private static final String SECTION_MDS_TAG = MDSInfo.KEY_TAG;
	private static final String SECTION_MDS_HOSTNAME = MDSInfo.KEY_HOSTNAME;
	private static final String SECTION_MDS_PORT = MDSInfo.KEY_PORT;
	private static final String SECTION_MDS_PROTOCOL = MDSInfo.KEY_PROTOCOL;
	private static final String SECTION_MDS_PATH = MDSInfo.KEY_PATH;
	private static final String SECTION_MDS_SUBJECT = MDSInfo.KEY_SUBJECT;
	private static final String SECTION_MDS_TYPE = MDSInfo.KEY_TYPE;
	private static final String SECTION_MDS_VONAME = MDSInfo.KEY_VONAME;
	private static final String SECTION_MDS_CLIENT_TIMEOUT =
		MDSInfo.KEY_CLIENT_TIMEOUT;
	private static final String SECTION_MDS_SERVER_TIMEOUT =
		MDSInfo.KEY_SERVER_TIMEOUT;

	/**
	 * @param key
	 * @param value
	 * @throws GrpcException
	 */
	private void processMDSServer(String key, String value) throws GrpcException {
		if (checkMDSServerParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse MDS_SERVER section.(the key (" +
				key + ") is not valid.");
		} else if (tmpProperties.containsKey(key)) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse MDS_SERVER section(the key (" +
				key + ")) already exists.");	
		} else if (key.equals(SECTION_MDS_CLIENT_TIMEOUT)) {
			registerMDSServer(key, convertTimeValue(value));
		/* unsupported key */
		} else if (key.equals(SECTION_MDS_SERVER_TIMEOUT)) {
			putNotSupportedWarning(key);
		} else {
			registerMDSServer(key, value);
		}
	}
	
	/**
	 * @param key
	 * @return
	 */
	private boolean checkMDSServerParameter(String key) {
		if (key.equals(SECTION_MDS_HOSTNAME)||
			key.equals(SECTION_MDS_TAG) ||
			key.equals(SECTION_MDS_PORT) ||
			key.equals(SECTION_MDS_PROTOCOL) ||
			key.equals(SECTION_MDS_PATH) ||
			key.equals(SECTION_MDS_SUBJECT) ||
			key.equals(SECTION_MDS_TYPE) ||
			key.equals(SECTION_MDS_VONAME) ||
			key.equals(SECTION_MDS_CLIENT_TIMEOUT) ||
			key.equals(SECTION_MDS_SERVER_TIMEOUT)) {
				return true;
		}
		return false;
	}
	
	/**
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerMDSServer(
		String key, String val) throws GrpcException {
		/* Check String Parameter */
		if (key.equals(SECTION_MDS_HOSTNAME)||
			key.equals(SECTION_MDS_TAG)||
			key.equals(SECTION_MDS_PROTOCOL)||
			key.equals(SECTION_MDS_PATH)||
			key.equals(SECTION_MDS_SUBJECT)||
			key.equals(SECTION_MDS_TYPE)||
			key.equals(SECTION_MDS_VONAME)) {
			registerStringParam(tmpProperties, key, val);
		}
		/* Check Numeric Parameter */
		else if (key.equals(SECTION_MDS_PORT) ||
			key.equals(SECTION_MDS_CLIENT_TIMEOUT) ||
			key.equals(SECTION_MDS_SERVER_TIMEOUT)) {
			registerNumericParam(tmpProperties, key, val);
		}
		/* unrecognized key */
		else {
			throw new NgException(key + " is not keyword for MDS_SERVER.");
		}
	}

	/* parameters for SERVER section */
	private static final String SECTION_SERVER_HOSTNAME =
		RemoteMachineInfo.KEY_HOSTNAME;
	private static final String SECTION_SERVER_TAG =
		RemoteMachineInfo.KEY_TAG;
	private static final String SECTION_SERVER_PORT = 
		RemoteMachineInfo.KEY_PORT;
	private static final String SECTION_SERVER_MDS_HOSTNAME =
		RemoteMachineInfo.KEY_MDS_HOSTNAME;
	private static final String SECTION_SERVER_MDS_TAG =
		RemoteMachineInfo.KEY_MDS_TAG;
	private static final String SECTION_SERVER_MPI_RUNCOMMAND =
		RemoteMachineInfo.KEY_MPI_RUNCOMMAND;
	private static final String SECTION_SERVER_MPI_NCPUS =
		RemoteMachineInfo.KEY_MPI_NCPUS;
	private static final String SECTION_SERVER_GASS_SCHEME =
		RemoteMachineInfo.KEY_GASS_SCHEME;
	private static final String SECTION_SERVER_CRYPT = 
		RemoteMachineInfo.KEY_CRYPT;
	private static final String SECTION_SERVER_PROTOCOL =
		RemoteMachineInfo.KEY_PROTOCOL;
	private static final String SECTION_SERVER_FORCEXDR =
		RemoteMachineInfo.KEY_FORCEXDR;
	private static final String SECTION_SERVER_JOBMANAGER =
		RemoteMachineInfo.KEY_JOBMANAGER;
	private static final String SECTION_SERVER_SUBJECT =
		RemoteMachineInfo.KEY_SUBJECT;
	private static final String SECTION_SERVER_QUEUE = 
		RemoteMachineInfo.KEY_QUEUE;
	private static final String SECTION_SERVER_PROJECT = 
		RemoteMachineInfo.KEY_PROJECT;
	private static final String SECTION_SERVER_HOSTCOUNT = 
		RemoteMachineInfo.KEY_HOSTCOUNT;
	private static final String SECTION_SERVER_MINMEMORY = 
		RemoteMachineInfo.KEY_MINMEMORY;
	private static final String SECTION_SERVER_MAXMEMORY = 
		RemoteMachineInfo.KEY_MAXMEMORY;
	private static final String SECTION_SERVER_MAXTIME = 
		RemoteMachineInfo.KEY_MAXTIME;
	private static final String SECTION_SERVER_MAXWALLTIME = 
		RemoteMachineInfo.KEY_MAXWALLTIME;
	private static final String SECTION_SERVER_MAXCPUTIME = 
		RemoteMachineInfo.KEY_MAXCPUTIME;
	private static final String SECTION_SERVER_JOB_STARTTIMEOUT =
		RemoteMachineInfo.KEY_JOB_STARTTIMEOUT;
	private static final String SECTION_SERVER_JOB_STOPTIMEOUT =
		RemoteMachineInfo.KEY_JOB_STOPTIMEOUT;
	private static final String SECTION_SERVER_HEARTBEAT =
		RemoteMachineInfo.KEY_HEARTBEAT;
	private static final String SECTION_SERVER_HEARTBEAT_TIMEOUTCOUNT =
		RemoteMachineInfo.KEY_HEARTBEAT_TIMEOUTCOUNT;
	private static final String SECTION_SERVER_HEARTBEAT_TIMEOUTCOUNT_ONTRANSFER =
		RemoteMachineInfo.KEY_HEARTBEAT_TIMEOUTCOUNT_ONTRANSFER;
	private static final String SECTION_SERVER_REDIRECT_OUTERR =
		RemoteMachineInfo.KEY_REDIRECT_OUTERR;
	private static final String SECTION_SERVER_ARG_TRANS =
		RemoteMachineInfo.KEY_ARG_TRANS;
	private static final String SECTION_SERVER_COMPRESS =
		RemoteMachineInfo.KEY_COMPRESS;
	private static final String SECTION_SERVER_COMPRESS_THRESHOLD =
		RemoteMachineInfo.KEY_COMPRESS_THRESHOLD;
	private static final String SECTION_SERVER_COMMLOG_ENABLE =
		RemoteMachineInfo.KEY_COMMLOG_ENABLE;
	private static final String SECTION_SERVER_COMMLOG_FILEPATH =
		RemoteMachineInfo.KEY_COMMLOG_FILEPATH;
	private static final String SECTION_SERVER_COMMLOG_SUFFIX =
		RemoteMachineInfo.KEY_COMMLOG_SUFFIX;
	private static final String SECTION_SERVER_COMMLOG_NFILES =
		RemoteMachineInfo.KEY_COMMLOG_NFILES;
	private static final String SECTION_SERVER_COMMLOG_MAXFILESIZE =
		RemoteMachineInfo.KEY_COMMLOG_MAXFILESIZE;
	private static final String SECTION_SERVER_COMMLOG_OVERWRITEDIR =
		RemoteMachineInfo.KEY_COMMLOG_OVERWRITEDIR;
	private static final String SECTION_SERVER_WORK_DIR =
		RemoteMachineInfo.KEY_WORK_DIR;
	private static final String SECTION_SERVER_CORE_SIZE =
		RemoteMachineInfo.KEY_CORE_SIZE;
	private static final String SECTION_SERVER_DEBUG =
		RemoteMachineInfo.KEY_DEBUG;
	private static final String SECTION_SERVER_DEBUG_DISPLAY =
		RemoteMachineInfo.KEY_DEBUG_DISPLAY;
	private static final String SECTION_SERVER_DEBUG_TERM =
		RemoteMachineInfo.KEY_DEBUG_TERM;
	private static final String SECTION_SERVER_DEBUG_DEBUGGER =
		RemoteMachineInfo.KEY_DEBUG_DEBUGGER;
	private static final String SECTION_SERVER_DEBUG_BUSYLOOP =
		RemoteMachineInfo.KEY_DEBUG_BUSYLOOP;
	private static final String SECTION_SERVER_ENVIRONMENT = 
		RemoteMachineInfo.KEY_ENVIRONMENT;
	private static final String SECTION_SERVER_TCP_NODELAY = 
		RemoteMachineInfo.KEY_TCP_NODELAY;
	private static final String SECTION_SERVER_BLOCK_SIZE =
		RemoteMachineInfo.KEY_BLOCK_SIZE;
	private static final String SECTION_SERVER_TCP_CONNECT_RETRY_COUNT =
		RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_COUNT;
	private static final String SECTION_SERVER_TCP_CONNECT_RETRY_BASEINTERVAL =
		RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_BASEINTERVAL;
	private static final String SECTION_SERVER_TCP_CONNECT_RETRY_INCREASERATIO =
		RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_INCREASERATIO;
	private static final String SECTION_SERVER_TCP_CONNECT_RETRY_RANDOM =
		RemoteMachineInfo.KEY_TCP_CONNECT_RETRY_RANDOM;
	private static final String SECTION_SERVER_INVOKE_SERVER =
		RemoteMachineInfo.KEY_INVOKE_SERVER;
	private static final String SECTION_SERVER_INVOKE_SERVER_OPTION =
		RemoteMachineInfo.KEY_INVOKE_SERVER_OPTION;
	private static final String SECTION_SERVER_CLIENT_HOSTNAME =
		RemoteMachineInfo.KEY_CLIENT_HOSTNAME;
	private static final String SECTION_SERVER_JOB_RSL_EXTENSION =
		RemoteMachineInfo.KEY_JOB_RSL_EXTENSION;
	
	/**
	 * @param key
	 * @param value
	 * @throws GrpcException
	 */
	private void processServer(String key, String value) throws GrpcException {
		if (checkServerParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse SERVER section. key: " + key + ", val: " + value);
		} else if (key.equals(SECTION_SERVER_HOSTNAME)) {
			List tmpList = (List) tmpProperties.get(SECTION_SERVER_HOSTNAME);
			if (tmpList == null) {
				tmpList = new Vector();
			}
			if (tmpList.contains(value)) {
				throw new NgInitializeGrpcClientException(
					"SERVER: already defined hostname: " + value);
			}
			tmpList.add(value);
			tmpProperties.put(SECTION_SERVER_HOSTNAME, tmpList);
		} else if (key.equals(SECTION_SERVER_ENVIRONMENT)) {
			List tmpList = null;
			if (tmpProperties.containsKey(key) == true) {
				tmpList = (List) tmpProperties.get(key);
			}
			if (tmpList == null) {
				tmpList = new Vector();
			}
			if (tmpList.contains(value)) {
				throw new NgInitializeGrpcClientException(
					"SERVER: already defined" + key);
			}
			tmpList.add(value);
			tmpProperties.put(key, tmpList);
		} else if (key.equals(SECTION_SERVER_MPI_NCPUS)) {
			Properties propNCPUs =
				(Properties) tmpProperties.get(SECTION_SERVER_MPI_NCPUS);
			if (propNCPUs == null) {
				propNCPUs = new Properties();
			}
			/* parse noOfCPUs */
			String funcName = null;
			String nCPUs = null;
			if (value.indexOf('=') == -1) {
				funcName = "";
				nCPUs = value;
			} else {
				StringTokenizer stNCPUs = new StringTokenizer(value, "=");
				if (stNCPUs.countTokens() < 1 || stNCPUs.countTokens() > 2) {
					throw new NgInitializeGrpcClientException(
						"SERVER: invalid variable for " + SECTION_SERVER_MPI_NCPUS);
				}
				if (stNCPUs.countTokens() == 2) {
					/* get name of function */
					funcName = stNCPUs.nextToken();
				} else {
					/* default variable */
					funcName = "";
				}
				/* get num of CPUs */
				nCPUs = stNCPUs.nextToken();
			}
			
			/* check if it's numeric */
			try {
				Integer.parseInt(nCPUs);
			} catch (NumberFormatException e) {
				throw new NgInitializeGrpcClientException(
					"SERVER: invalid variable for " +
					SECTION_SERVER_MPI_NCPUS);
			}

			/* check if it's already defined */
			if (propNCPUs.containsKey(funcName)) {
				throw new NgInitializeGrpcClientException(
					"SERVER: already defined number of CPUs for " +
					funcName);
			}
			propNCPUs.put(funcName, nCPUs);
			tmpProperties.put(SECTION_SERVER_MPI_NCPUS, propNCPUs);
		} else if (key.equals(SECTION_SERVER_INVOKE_SERVER_OPTION) ||
					key.equals(SECTION_SERVER_JOB_RSL_EXTENSION)) {
			List tmpList = null;
			if (tmpProperties.containsKey(key) == true) {
				tmpList = (List) tmpProperties.get(key);
			}
			if (tmpList == null) {
				tmpList = new Vector();
			}
			if (tmpList.contains(value)) {
				throw new NgInitializeGrpcClientException(
					"SERVER: already defined" + key);
			}
			tmpList.add(value);
			tmpProperties.put(key, tmpList);
		} else if (tmpProperties.containsKey(key)) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse SERVER section(the key (" +
				 key + ")) already exists.");	
		} else if ((key.equals(SECTION_SERVER_JOB_STARTTIMEOUT)) ||
			(key.equals(SECTION_SERVER_JOB_STOPTIMEOUT)) ||
			(key.equals(SECTION_SERVER_HEARTBEAT))) {
			registerServer(key, convertTimeValue(value));
		} else if ((key.equals(SECTION_SERVER_COMPRESS_THRESHOLD)) ||
			(key.equals(SECTION_SERVER_COMMLOG_MAXFILESIZE)) ||
			(key.equals(SECTION_SERVER_CORE_SIZE)) ||
			(key.equals(SECTION_SERVER_BLOCK_SIZE))) {
			registerServer(key, convertSizeValue(value));
		} else {
			registerServer(key, value);		
		}
	}
	
	/**
	 * @param key
	 * @param st
	 * @throws GrpcException
	 */
	private void processMultipleClientDefinitions(String key, StringTokenizer st)
		throws GrpcException{
		/* check if it's handling_signals in CLIENT */
		if (!key.equals(SECTION_CLIENT_HANDLING_SIGNALS)) {
			StringBuffer val = new StringBuffer();
			while (st.hasMoreTokens()) {
				val.append(st.nextToken() + " ");
			}
			throw new NgInitializeGrpcClientException(
					"Invalid config parameter: " + val.toString());
		}
		/* just ignore values */
		putNotSupportedWarning(key);
	}
		
	/**
	 * @param key
	 * @param st
	 * @throws GrpcException
	 */
	private void processMultipleServerDefinitions(String key, StringTokenizer st)
		throws GrpcException{
		/* check if it's hostname in SERVER */
		if (!key.equals(SECTION_SERVER_HOSTNAME)) {
			throw new NgInitializeGrpcClientException(
				"NGCONFIG: invalid multiple parameters");
		} else {
			List tmpList;
			if (tmpProperties.containsKey(SECTION_SERVER_HOSTNAME)) {
				tmpList = (List) tmpProperties.get(SECTION_SERVER_HOSTNAME);
			} else {
				tmpList = new Vector();
			}
			/* add hostname values into List */
			while (st.hasMoreTokens()) {
				tmpList.add(st.nextToken());
			}
			/* set List into Properties for SERVER section */ 
			tmpProperties.put(SECTION_SERVER_HOSTNAME, tmpList);
		}
	}
	
	/**
	 * @param key
	 * @return
	 */
	private boolean checkServerParameter(String key) {
		if (key.equals(SECTION_SERVER_HOSTNAME)||
			key.equals(SECTION_SERVER_TAG) ||
			key.equals(SECTION_SERVER_PORT) ||
			key.equals(SECTION_SERVER_MDS_HOSTNAME) ||
			key.equals(SECTION_SERVER_MDS_TAG) ||
			key.equals(SECTION_SERVER_MPI_RUNCOMMAND) ||
			key.equals(SECTION_SERVER_MPI_NCPUS) ||
			key.equals(SECTION_SERVER_GASS_SCHEME) ||
			key.equals(SECTION_SERVER_CRYPT) ||
			key.equals(SECTION_SERVER_PROTOCOL) ||
			key.equals(SECTION_SERVER_FORCEXDR) ||
			key.equals(SECTION_SERVER_JOBMANAGER) ||
			key.equals(SECTION_SERVER_SUBJECT) ||
			key.equals(SECTION_SERVER_QUEUE) ||
			key.equals(SECTION_SERVER_PROJECT) ||
			key.equals(SECTION_SERVER_HOSTCOUNT) ||
			key.equals(SECTION_SERVER_MINMEMORY) ||
			key.equals(SECTION_SERVER_MAXMEMORY) ||
			key.equals(SECTION_SERVER_MAXTIME) ||
			key.equals(SECTION_SERVER_MAXWALLTIME) ||
			key.equals(SECTION_SERVER_MAXCPUTIME) ||
			key.equals(SECTION_SERVER_JOB_STARTTIMEOUT) ||
			key.equals(SECTION_SERVER_JOB_STOPTIMEOUT) ||
			key.equals(SECTION_SERVER_HEARTBEAT) ||
			key.equals(SECTION_SERVER_HEARTBEAT_TIMEOUTCOUNT) ||
			key.equals(SECTION_SERVER_HEARTBEAT_TIMEOUTCOUNT_ONTRANSFER) ||
			key.equals(SECTION_SERVER_REDIRECT_OUTERR) ||
			key.equals(SECTION_SERVER_ARG_TRANS) ||
			key.equals(SECTION_SERVER_COMPRESS) ||
			key.equals(SECTION_SERVER_COMPRESS_THRESHOLD) ||
			key.equals(SECTION_SERVER_COMMLOG_ENABLE) ||
			key.equals(SECTION_SERVER_COMMLOG_FILEPATH) ||
			key.equals(SECTION_SERVER_COMMLOG_SUFFIX) ||
			key.equals(SECTION_SERVER_COMMLOG_NFILES) ||
			key.equals(SECTION_SERVER_COMMLOG_MAXFILESIZE) ||
			key.equals(SECTION_SERVER_COMMLOG_OVERWRITEDIR) ||
			key.equals(SECTION_SERVER_WORK_DIR) ||
			key.equals(SECTION_SERVER_CORE_SIZE) ||
			key.equals(SECTION_SERVER_DEBUG) ||
			key.equals(SECTION_SERVER_DEBUG_DISPLAY) ||
			key.equals(SECTION_SERVER_DEBUG_TERM) ||
			key.equals(SECTION_SERVER_DEBUG_DEBUGGER) ||
			key.equals(SECTION_SERVER_DEBUG_BUSYLOOP) ||
			key.equals(SECTION_SERVER_ENVIRONMENT) ||
			key.equals(SECTION_SERVER_TCP_NODELAY) ||
			key.equals(SECTION_SERVER_BLOCK_SIZE) ||
			key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_COUNT) ||
			key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_BASEINTERVAL) ||
			key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_INCREASERATIO) ||
			key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_RANDOM) ||
			key.equals(SECTION_SERVER_INVOKE_SERVER) ||
			key.equals(SECTION_SERVER_INVOKE_SERVER_OPTION) ||
			key.equals(SECTION_SERVER_CLIENT_HOSTNAME) ||
			key.equals(SECTION_SERVER_JOB_RSL_EXTENSION)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerServer(
		String key, String val) throws GrpcException {
		/* Check obsoleted parameter */
		if (key.equals(SECTION_SERVER_MPI_RUNCOMMAND)) {
			/* put error message */
			System.err.println("syntax warning: Obsolete syntax \"" +
				SECTION_SERVER_MPI_RUNCOMMAND + "\"." +
				"Ignoring this setting continue.");
		}
		/* Check String Parameters */
		else if (key.equals(SECTION_SERVER_MDS_HOSTNAME) ||
			key.equals(SECTION_SERVER_MDS_TAG) ||
			key.equals(SECTION_SERVER_JOBMANAGER) ||
			key.equals(SECTION_SERVER_SUBJECT) ||
			key.equals(SECTION_SERVER_QUEUE) ||
			key.equals(SECTION_SERVER_PROJECT) ||
			key.equals(SECTION_SERVER_COMMLOG_FILEPATH) ||
			key.equals(SECTION_SERVER_COMMLOG_SUFFIX) ||
			key.equals(SECTION_SERVER_WORK_DIR) ||
			key.equals(SECTION_SERVER_DEBUG_DISPLAY) ||
			key.equals(SECTION_SERVER_DEBUG_TERM) ||
			key.equals(SECTION_SERVER_DEBUG_DEBUGGER) ||
			key.equals(SECTION_SERVER_INVOKE_SERVER) ||
			key.equals(SECTION_SERVER_CLIENT_HOSTNAME)) {
			registerStringParam(tmpProperties, key, val);
		}
		/* Check Hostname Parameters */
		else if (key.equals(SECTION_SERVER_HOSTNAME)||
			key.equals(SECTION_SERVER_TAG)) {
			registerTagnameParam(tmpProperties, key, val);
		}
		/* check Numeric Parameters */
		else if (key.equals(SECTION_SERVER_PORT) ||
			key.equals(SECTION_SERVER_HOSTCOUNT) ||
			key.equals(SECTION_SERVER_MINMEMORY) ||
			key.equals(SECTION_SERVER_MAXMEMORY) ||
			key.equals(SECTION_SERVER_MAXTIME) ||
			key.equals(SECTION_SERVER_MAXWALLTIME) ||
			key.equals(SECTION_SERVER_MAXCPUTIME) ||
			key.equals(SECTION_SERVER_JOB_STARTTIMEOUT) ||
			key.equals(SECTION_SERVER_JOB_STOPTIMEOUT) ||
			key.equals(SECTION_SERVER_HEARTBEAT) ||
			key.equals(SECTION_SERVER_HEARTBEAT_TIMEOUTCOUNT) ||
			key.equals(SECTION_SERVER_HEARTBEAT_TIMEOUTCOUNT_ONTRANSFER) ||
			key.equals(SECTION_SERVER_COMPRESS_THRESHOLD) ||
			key.equals(SECTION_SERVER_COMMLOG_NFILES) ||
			key.equals(SECTION_SERVER_COMMLOG_MAXFILESIZE) ||
			key.equals(SECTION_SERVER_CORE_SIZE) ||
			key.equals(SECTION_SERVER_BLOCK_SIZE) ||
			key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_COUNT) ||
			key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_BASEINTERVAL)) {
			registerNumericParam(tmpProperties, key, val);
		}
		/* check GASS scheme Parameters */
		else if (key.equals(SECTION_SERVER_GASS_SCHEME)) {
			registerGassSchemeParam(tmpProperties, key, val);
		}
		/* check crypt Parameters */
		else if (key.equals(SECTION_SERVER_CRYPT)) {
			registerCryptParam(tmpProperties, key, val);
		}
		/* check protocol Parameters */
		else if (key.equals(SECTION_SERVER_PROTOCOL)) {
			registerProtocolParam(tmpProperties, key, val);
		}
		/* check Boolean Parameters */
		else if (key.equals(SECTION_SERVER_FORCEXDR) ||
			key.equals(SECTION_SERVER_REDIRECT_OUTERR) ||
			key.equals(SECTION_SERVER_COMMLOG_ENABLE) ||
			key.equals(SECTION_SERVER_COMMLOG_OVERWRITEDIR) ||
			key.equals(SECTION_SERVER_DEBUG) ||
			key.equals(SECTION_SERVER_DEBUG_BUSYLOOP) ||
			key.equals(SECTION_SERVER_TCP_NODELAY) ||
			key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_RANDOM)) {
			registerBooleanParam(tmpProperties, key, val);
		}
		/* check TransArg Parameters */
		else if (key.equals(SECTION_SERVER_ARG_TRANS)) {
			putNotSupportedWarning(key);
		}
		/* check Compress Parameters */
		else if (key.equals(SECTION_SERVER_COMPRESS)) {
			registerCompressParam(tmpProperties, key, val);
		}
		/* check Object Parameters */
		else if (key.equals(SECTION_SERVER_MPI_NCPUS) ||
				key.equals(SECTION_SERVER_ENVIRONMENT) ||
				key.equals(SECTION_SERVER_INVOKE_SERVER_OPTION) ||
				key.equals(SECTION_SERVER_JOB_RSL_EXTENSION)) {
			registerObjectParam(tmpProperties, key, val);
		}
		/* check Float Parameters */
		else if (key.equals(SECTION_SERVER_TCP_CONNECT_RETRY_INCREASERATIO)) {
			registerFloatParam(tmpProperties, key, val);
		}
		/* unrecognized key */
		else {
			throw new NgException(key + " is not keyword for SERVER.");
		}
	}
	
	/**
	 * @param key
	 * @param value
	 * @throws GrpcException
	 */
	private void processServerDefault(String key, String value) throws GrpcException {
		if (checkServerParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
					"Failed to parse SERVER_DEFAULT section. key: " + key + ", val: " + value);
		} else if (key.equals(SECTION_SERVER_ENVIRONMENT)) {
			List tmpList = null;
			if (tmpProperties.containsKey(key) == true) {
				tmpList = (List) tmpProperties.get(key);
			}
			if (tmpList == null) {
				tmpList = new Vector();
			}
			if (tmpList.contains(value)) {
				throw new NgInitializeGrpcClientException(
					"SERVER_DEFAULT: already defined " + key);
			}
			tmpList.add(value);
			tmpProperties.put(key, tmpList);
		} else if (key.equals(SECTION_SERVER_MPI_NCPUS)) {
			Properties tmpProp =
				(Properties) tmpProperties.get(SECTION_SERVER_MPI_NCPUS);
			if (tmpProp == null) {
				tmpProp = new Properties();
			}
			if (tmpProp.contains(value)) {
				throw new NgInitializeGrpcClientException(
					"SERVER_DEFAULT: already defined number of CPUs");
			}
			/* parse noOfCPUs */
			StringTokenizer stNCPUs = new StringTokenizer(value, "=");
			if (stNCPUs.countTokens() != 1) {
				throw new NgInitializeGrpcClientException(
					"SERVER_DEFAULT: invalid variable for " + SECTION_SERVER_MPI_NCPUS);
			}
			String funcName = "";
			/* get num of CPUs */
			String nCPUs = stNCPUs.nextToken();
			
			/* check if it's numeric */
			try {
				Integer.parseInt(nCPUs);
			} catch (NumberFormatException e) {
				throw new NgInitializeGrpcClientException(
					"SERVER: invalid variable for " + SECTION_SERVER_MPI_NCPUS);
			}

			tmpProp.put(funcName, nCPUs);
			tmpProperties.put(SECTION_SERVER_MPI_NCPUS, tmpProp);
		} else if (key.equals(SECTION_SERVER_INVOKE_SERVER_OPTION)) {
			List tmpList = null;
			if (tmpProperties.containsKey(key) == true) {
				tmpList = (List) tmpProperties.get(key);
			}
			if (tmpList == null) {
				tmpList = new Vector();
			}
			if (tmpList.contains(value)) {
				throw new NgInitializeGrpcClientException(
					"SERVER: already defined" + key);
			}
			tmpList.add(value);
			tmpProperties.put(key, tmpList);
		} else if (tmpProperties.containsKey(key)) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse SERVER section(the key (" +
				key + ") already exists.");	
		} else if (key.equals(SECTION_SERVER_HOSTNAME)) {
			throw new NgInitializeGrpcClientException(
				"SERVER_DEFAULT: hostname exists!");			
		} else if (key.equals(SECTION_SERVER_TAG)) {
			throw new NgInitializeGrpcClientException(
				"SERVER_DEFAULT: tag exists!");			
		} else if ((key.equals(SECTION_SERVER_JOB_STARTTIMEOUT)) ||
			(key.equals(SECTION_SERVER_JOB_STOPTIMEOUT)) ||
			(key.equals(SECTION_SERVER_HEARTBEAT))) {
			registerServer(key, convertTimeValue(value));		
		} else if ((key.equals(SECTION_SERVER_COMPRESS_THRESHOLD)) ||
			(key.equals(SECTION_SERVER_COMMLOG_MAXFILESIZE)) ||
			(key.equals(SECTION_SERVER_CORE_SIZE)) ||
			(key.equals(SECTION_SERVER_BLOCK_SIZE))) {
			registerServer(key, convertSizeValue(value));		
		} else {
			registerServer(key, value);		
		}
	}
	
	/* parameters for INVOKE_SERVER section */
	private static final String SECTION_INVOKE_SERVER_TYPE = InvokeServerInfo.KEY_TYPE;
	private static final String SECTION_INVOKE_SERVER_PATH = InvokeServerInfo.KEY_PATH;
	private static final String SECTION_INVOKE_SERVER_LOG_FILEPATH =
		InvokeServerInfo.KEY_LOG_FILEPATH;
	private static final String SECTION_INVOKE_SERVER_MAXJOBS =
		InvokeServerInfo.KEY_MAXJOBS;
	private static final String SECTION_INVOKE_SERVER_OPTION =
		InvokeServerInfo.KEY_OPTION;
	private static final String SECTION_INVOKE_SERVER_STATUS_POLLING =
		InvokeServerInfo.KEY_STATUS_POLLING;

	/**
	 * @param key
	 * @param value
	 * @throws GrpcException
	 */
	private void processInvokeServerInfo(String key, String value) throws GrpcException {
		if (checkInvokeServerInfoParameter(key) == false) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse INVOKE_SERVER section.(the key (" +
				key + ") is not valid.");
		} else if (key.equals(SECTION_INVOKE_SERVER_OPTION)) {
			List tmpList = null;
			if (tmpProperties.containsKey(key) == true) {
				tmpList = (List) tmpProperties.get(key);
			}
			if (tmpList == null) {
				tmpList = new Vector();
			}
			if (tmpList.contains(value)) {
				throw new NgInitializeGrpcClientException(
					"INVOKE_SERVER: already defined" + key);
			}
			tmpList.add(value);
			tmpProperties.put(key, tmpList);
		} else if (tmpProperties.containsKey(key)) {
			throw new NgInitializeGrpcClientException(
				"Failed to parse INVOKE_SERVER section(the key (" +
				key + ")) already exists.");	
		} else {
			registerInvokeServerInfo(key, value);
		}
	}
	
	/**
	 * @param key
	 * @return
	 */
	private boolean checkInvokeServerInfoParameter(String key) {
		if (key.equals(SECTION_INVOKE_SERVER_TYPE)||
			key.equals(SECTION_INVOKE_SERVER_PATH) ||
			key.equals(SECTION_INVOKE_SERVER_LOG_FILEPATH) ||
			key.equals(SECTION_INVOKE_SERVER_MAXJOBS) ||
			key.equals(SECTION_INVOKE_SERVER_OPTION) ||
			key.equals(SECTION_INVOKE_SERVER_STATUS_POLLING)) {
			return true;
		}
		return false;
	}
	
	/**
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerInvokeServerInfo(String key, String val) throws GrpcException {
		/* check String Parameter */
		if (key.equals(SECTION_INVOKE_SERVER_TYPE)||
			key.equals(SECTION_INVOKE_SERVER_PATH) ||
			key.equals(SECTION_INVOKE_SERVER_LOG_FILEPATH)) {
			registerStringParam(tmpProperties, key, val);
		}
		/* check Integer Parameter */
		else if (key.equals(SECTION_INVOKE_SERVER_MAXJOBS)) {
			registerNumericParam(tmpProperties, key, val);
		}
		/* check Time Parameter */
		else if (key.equals(SECTION_INVOKE_SERVER_STATUS_POLLING)) {
			registerNumericParam(tmpProperties, key, convertTimeValue(val));
		}
		/* check Object Parameters */
		else if (key.equals(SECTION_INVOKE_SERVER_OPTION)) {
			registerObjectParam(tmpProperties, key, val);
		}
		/* unrecognized key */
		else {
			throw new NgException(key + " is not keyword for INVOKE_SERVER.");
		}
	}
	
	/**
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private String getQuotedVariable(String target) throws GrpcException {
		/* check validation */
		checkQuotedVariable(target);
		
		/* check special characters */
		target = replaceSpecialCharacter(target);
		
		/* separate String by double quote String */
		return target.substring(target.indexOf('"') + 1, target.lastIndexOf('"'));
	}
	
	/**
	 * @param target
	 * @throws GrpcException
	 */
	private static void checkQuotedVariable(String target) throws GrpcException {
		/* check if the quote is valid */
		if (! target.startsWith("\"")) {
			throw new NgInitializeGrpcClientException(
				"NGCONFIG: invalid quoted parameters: " + target);
		}
		
		/* check if multiple quote char is in it */
		boolean findQuote = false;
		int nowIndex = 0;
		int lastIndex = 0;
		while ((nowIndex = target.indexOf("\"", nowIndex + 1)) != -1) {
			if (target.charAt(nowIndex - 1) != '\\') {
				if (findQuote == true) {
					throw new NgInitializeGrpcClientException(
						"NGCONFIG: invalid quoted parameters: " + target);
				}
				findQuote = true;
				lastIndex = nowIndex;
			}
		}
		
		/* check if the last quote is valid */
		if (lastIndex != target.lastIndexOf("\"")) {
			throw new NgInitializeGrpcClientException(
				"NGCONFIG: invalid quoted parameters: " + target);
		}
		
		/* check if there are any String after quote */
		String afterString = target.substring(lastIndex + 1).trim();
		if ((afterString.length() > 0) && (! afterString.startsWith("#"))) {
			throw new NgInitializeGrpcClientException(
				"NGCONFIG: invalid string after quote: " + target);
		}
	}
	
	/**
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private static String replaceSpecialCharacter(String target) throws GrpcException {
		/* check backslash */
		int bsIndex = target.indexOf('\\');
		while (bsIndex != -1) {
			char bsChar = target.charAt(bsIndex + 1);
			if ((bsChar != '"') && (bsChar != '\\')) {
				throw new NgInitializeGrpcClientException(
					"NGCONFIG: invalid backslash parameters: " + target);
			}
			
			/* move to next index */
			bsIndex = target.indexOf('\\', bsIndex + 2);
		}
		/* replace \" -> " */
		if (target.indexOf("\\\"") != -1) {
			target = replaceString(target, "\\\"", "\"");
		}
		
		/* check backslash */
		bsIndex = target.indexOf('\\');
		while (bsIndex != -1) {
			char bsChar = target.charAt(bsIndex + 1);
			if (bsChar != '\\') {
				throw new NgInitializeGrpcClientException(
					"NGCONFIG: invalid backslash parameters: " + target);
			}
			
			/* move to next index */
			bsIndex = target.indexOf('\\', bsIndex + 2);
		}
		/* replace \\ -> \ */
		if (target.indexOf("\\\\") != -1) {
			target = replaceString(target, "\\\\", "\\");
		}
		
		return target;
	}
	
	/**
	 * @param target
	 * @param before
	 * @param after
	 * @return
	 * @throws GrpcException
	 */
	private static String replaceString(String target, String before, String after) throws GrpcException {
		/* check backslash */
		int beforeLength = before.length();
		int afterLength = after.length();
		int targetIndex = target.indexOf(before);
		
		/* replaced String */
		StringBuffer replacedString = new StringBuffer();
		int nowIndex = 0;
		
		while (true) {
			/* separate String */
			replacedString.append(target.substring(nowIndex, targetIndex));
			replacedString.append(after);
			
			/* move nowIndex */
			nowIndex = targetIndex + beforeLength;
			
			/* move to next index */
			targetIndex = target.indexOf(before, nowIndex);
			if (targetIndex == -1) {
				replacedString.append(target.substring(nowIndex, target.length()));
				break;
			}
		}
		
		return replacedString.toString();
	}
	
	/**
	 * @param str
	 * @return
	 * @throws GrpcException
	 */
	protected static String convertSizeValue(String str) throws GrpcException {
		/* check "Kilo", "Mega", "Giga" */
		Vector unitList = new Vector();
		unitList.add("Kilo");
		unitList.add("Mega");
		unitList.add("Giga");
		
		/* loop and check if it include String of unit */
		for (int i = 0; i < unitList.size(); i++) {
			String target = (String)unitList.get(i);
			int index = str.indexOf(target.charAt(0));
			if (index != -1) {
				String target_unit = str.substring(index);
				if (target_unit.length() > target.length()) {
					throw new NgInitializeGrpcClientException("Invalid Unit String.");
				} else if (target_unit.regionMatches(0,
								target, 0, target_unit.length()) == false) {
					throw new NgInitializeGrpcClientException("Invalid Unit String.");									
				}
				int target_num = Integer.parseInt(str.substring(0, index));
				for (int j = 0; j <= i; j++) {
					target_num *= 1024;
				}
				return new String(Integer.toString(target_num));
			}
		}
		return str;
	}
	
	/**
	 * @param str
	 * @return
	 * @throws GrpcException
	 */
	private String convertTimeValue(String str) throws GrpcException {
		/* check "second", "minute", "hour" */
		Vector unitList = new Vector();
		unitList.add("second");
		unitList.add("minute");
		unitList.add("hour");
		
		/* loop and check if it include String of unit */
		for (int i = 0; i < unitList.size(); i++) {
			String target = (String)unitList.get(i);
			int index = str.indexOf(target.charAt(0));
			if (index != -1) {
				String target_unit = str.substring(index);
				if (target_unit.length() > target.length()) {
					throw new NgInitializeGrpcClientException("Invalid Unit String.");
				} else if (target_unit.regionMatches(0,
								target, 0, target_unit.length()) == false) {
					throw new NgInitializeGrpcClientException("Invalid Unit String.");									
				}
				int target_num = Integer.parseInt(str.substring(0, index));
				for (int j = 0; j < i; j++) {
					target_num *= 60;
				}
				return new String(Integer.toString(target_num));
			}
		}
		
		return str;
	}
	
	/**
	 * @param informationManager
	 * @throws GrpcException
	 */
	protected void initRemoteMachineInformation(NgInformationManager informationManager)
		throws GrpcException {
		/* put RemoteMachineInformation into InformationManager */
		for (int i = 0; i < serverInfo.size(); i++) {
			Properties propRemoteMachineInfo = (Properties) serverInfo.get(i);
			
			/* put RemoteMachineInfo into InformationManager */
			informationManager.putRemoteMachineInfo(
				propRemoteMachineInfo.getProperty(SECTION_SERVER_HOSTNAME),
				propRemoteMachineInfo.getProperty(SECTION_SERVER_TAG),
				propRemoteMachineInfo, null);
		}
	}
	
	/**
	 * @param informationManager
	 * @throws GrpcException
	 */
	protected void registerConfigInformation(NgInformationManager informationManager)
		throws GrpcException {
		/* check all of MDS_SERVER info specified by <SERVER> mds_tag do exit */
		for (int i = 0; i < serverInfo.size(); i++) {
			/* get <SERVER> mds_tag */
			Properties propRemoteMachineInfo = (Properties) serverInfo.get(i);
			String mds_tag = propRemoteMachineInfo.getProperty(SECTION_SERVER_MDS_TAG);
			
			/* check if it's valid name */
			if (mds_tag == null) {
				continue;
			}
			
			/* is the specified tag contained? */
			boolean boolTagFound = false;
			for (int j = 0; j < mdsInfo.size(); j++) {
				MDSInfo info = (MDSInfo) mdsInfo.get(j);
				String tag = (String) info.get(MDSInfo.KEY_TAG);
				if (mds_tag.equals(tag)) {
					/* found specified mds_tag */
					boolTagFound = true;
					break;
				}
			}
			
			if (boolTagFound == false) {
				/* the mds_tag is not in list of MDSInfo */
				throw new NgException(
					"NgConfig#registerConfigInformation: specified mds_tag[" + mds_tag + "] does not exist.");
			}
		}
		
		/* check <SERVER> or <SERVER_DEFAULT> mds_hostname, mds_tag */
		if (serverDefault != null) {
			String mds_hostname_default =
				(String) serverDefault.get(SECTION_SERVER_MDS_HOSTNAME);
			String mds_tag_default =
				(String) serverDefault.get(SECTION_SERVER_MDS_TAG);
			
			/* check if mds_tag in SERVER_DEFAULT is in config file */
			boolean boolTagFound = false;
			if (mds_tag_default != null) {
				for (int i = 0; i < mdsInfo.size(); i++) {
					MDSInfo info = (MDSInfo) mdsInfo.get(i);
					String tag = (String) info.get(MDSInfo.KEY_TAG);
					if (mds_tag_default.equals(tag)) {
						/* found specified mds_tag */
						boolTagFound = true;
						break;
					}
				}
			}
			
			if ((mds_tag_default != null) && (boolTagFound == false)) {
				/* the mds_tag is not in list of MDSInfo */
				throw new NgException(
					"NgConfig#registerConfigInformation: specified mds_tag[" + mds_tag_default + "] does not exist.");
			}
			
			if (mds_hostname_default != null) {
				/* mds_hostname is in SERVER_DEFAULT */
				for (int i = 0; i < serverInfo.size(); i++) {
					Properties propRemoteMachineInfo = (Properties) serverInfo.get(i);
					if (propRemoteMachineInfo.get(SECTION_SERVER_MDS_TAG) != null) {
						throw new NgException(
							"NgConfig#registerConfigInformation: mds_hostname is in SERVER_DEFAULT, mds_tag is in SERVER.");
					}
				}
			} else if (mds_tag_default != null) {
				/* mds_tag is in SERVER_DEFAULT */
				for (int i = 0; i < serverInfo.size(); i++) {
					Properties propRemoteMachineInfo = (Properties) serverInfo.get(i);
					if (propRemoteMachineInfo.get(SECTION_SERVER_MDS_HOSTNAME) != null) {
						throw new NgException(
							"NgConfig#registerConfigInformation: mds_tag is in SERVER_DEFAULT, mds_hostname is in SERVER.");
					}
				}
			}
		}
		
		/* put RemoteMachineInformation into InformationManager */
		for (int i = 0; i < serverInfo.size(); i++) {
			Properties propRemoteMachineInfo = (Properties) serverInfo.get(i);
			
			/* put RemoteMachineInfo into InformationManager */
			informationManager.putRemoteMachineInfo(
				propRemoteMachineInfo.getProperty(SECTION_SERVER_HOSTNAME),
				propRemoteMachineInfo.getProperty(SECTION_SERVER_TAG),
				propRemoteMachineInfo, serverDefault);
		}
		
		/* put MDSInformation into InformationManager */
		for (int i = 0; i < mdsInfo.size(); i++) {
			MDSInfo info = (MDSInfo) mdsInfo.get(i);
			informationManager.putMDSInfo(info);
		}
		
		/* put LocalMachineInformation into InformationManager */
		informationManager.putLocalMachineInfo(localHostInfo);
		
		/* put FunctionInfo into InformationManager */
		for (int i = 0; i < functionList.size(); i++) {
			Properties propFunctionInfo = (Properties)functionList.get(i);
			String remoteHostName =
				(String)propFunctionInfo.get(SECTION_FUNCINFO_HOSTNAME);
			String remoteClassName =
				(String)propFunctionInfo.get(SECTION_FUNCINFO_FUNCNAME);
			RemoteMachineInfo rmi = null;
			/* create RemoteClassPathInfo */
			RemoteClassPathInfo rcpi =
				new RemoteClassPathInfo(remoteHostName,	remoteClassName,
				(String) propFunctionInfo.get(SECTION_FUNCINFO_STAGING),
				(String) propFunctionInfo.get(SECTION_FUNCINFO_PATH),
				(String) propFunctionInfo.get(SECTION_FUNCINFO_BACKEND),
				(String) propFunctionInfo.get(SECTION_FUNCINFO_SESSION_TIMEOUT));
			/* check if RemoteMachineInfo is registered */
			if (informationManager.isRemoteMachineInfoRegistered(
				remoteHostName) == true) {
				rmi = informationManager.getRemoteMachineInfo(
					remoteHostName);
			} else {
				/* create RemoteMachineInfo */
				rmi = new RemoteMachineInfo(remoteHostName);
			}
			
			/* register ClassPathInfo into InformationManager */
			informationManager.putRemoteClassPathInfo(remoteHostName,
				remoteClassName, rcpi);
			/* register RemoteMachineInfo into manager */
			informationManager.putRemoteMachineInfo(remoteHostName, null, rmi);
		}

		/* create MDSInfo only in SERVER section */
		for (int i = 0; i < serverInfo.size(); i++) {
			Properties propServerInfo = (Properties) serverInfo.get(i);
			String mdsServerName =
				(String) propServerInfo.get(SECTION_SERVER_MDS_HOSTNAME);

			/* get mds_hostname from server_default */
			if ((mdsServerName == null) && (serverDefault != null)) {
				mdsServerName =
					(String) serverDefault.get(SECTION_SERVER_MDS_HOSTNAME);
			}

			/* check if mds_hostname is in InformationManager */
			if (mdsServerName != null) {
				if (informationManager.getMDSInfoByHandleString(
					MDSInfo.makeHandleString(mdsServerName, null)) == null) {
					/* MDS server only in SERVER section */
					Properties propMDSServer = new Properties();
					propMDSServer.put(SECTION_MDS_HOSTNAME, mdsServerName);
					informationManager.putMDSInfo(new MDSInfo(propMDSServer));
				}
			}
		}
		
		/* put InvokeServerInfo into InformationManager */
		for (int i = 0; i < invokeServerInfo.size(); i++) {
			Properties tmpProp = (Properties) invokeServerInfo.get(i);
			InvokeServerInfo info = new InvokeServerInfo(tmpProp);
			informationManager.putInvokeServerInfo(info.getType(), info);
		}
	}
	
	/**
	 * @return
	 */
	protected List getServerInfo() {
		return serverInfo;
	}
	
	/**
	 * @return
	 */
	protected List getMDSServerInfo() {
		return mdsInfo;
	}
	
	/* routines for checking variables */
	/**
	 * @param prop
	 * @param key
	 * @param val
	 */
	private void registerObjectParam(
		Properties prop, String key, Object val) {
		prop.put(key, val);
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerStringParam(
		Properties prop, String key, String val) {
		prop.put(key, val);
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerHostnameParam(
		Properties prop, String key, String val) throws GrpcException {
		prop.put(key, val);
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerTagnameParam(
		Properties prop, String key, String val) throws GrpcException {
		/* Just ensure the Resource Manager Contact splitable. */
		if ((val.indexOf(':') != -1) || (val.indexOf('/') != -1)) {
			throw new NgException("Invalid param for " + key + ":[" + val + "].");
		}
		prop.put(key, val);
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerNumericParam(Properties prop,
		String key, String val) throws GrpcException {
		try {
			/* check if it's number variable */
			Integer.parseInt(val);
		} catch (NumberFormatException e){
			throw new NgException("Invalid param for " + key + ".");
		}
		prop.put(key, val);
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerFloatParam(Properties prop,
		String key, String val) throws GrpcException {
		try {
			/* check if it's number variable */
			Double.parseDouble(val);
		} catch (NumberFormatException e){
			throw new NgException("Invalid param for " + key + ".");
		}
		prop.put(key, val);
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerLogLevelParam(Properties prop,
		String key, String val) throws GrpcException {
		if (NgLog.validLogLevelParam(val) != true) {
			throw new NgException("Invalid param for " + key + ".");
		}
		prop.put(key, val);
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerBooleanParam(Properties prop,
		String key, String val) throws GrpcException {
		if (val.equals("true") || val.equals("false")) {
			prop.put(key, val);
		} else {
			throw new NgException("Invalid param for " + key + ".");
		}
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerGassSchemeParam(Properties prop,
		String key, String val) throws GrpcException {
		if (val.equals("https") || val.equals("http")) {
			prop.put(key, val);
		} else {
			throw new NgException("Invalid param for " + key + ".");
		}
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerCryptParam(Properties prop,
		String key, String val) throws GrpcException {
		if (val.equals("false") || val.equals("SSL") || val.equals("GSI")) {
			if (val.equals("false") == true) {
				val = "none";
			}
			prop.put(key, val);
		} else {
			throw new NgException("Invalid param for " + key + ".");
		}
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerProtocolParam(Properties prop,
		String key, String val) throws GrpcException {
		if (val.equals("XML") || val.equals("binary")) {
			if (val.equals("XML")) {
				throw new NgException("XML protocol is not supported...");
			}
			prop.put(key, val);
		} else {
			throw new NgException("Invalid param for " + key + ".");
		}
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerTransArgParam(Properties prop,
		String key, String val) throws GrpcException {
		if (val.equals("wait") || val.equals("nowait") ||
			val.equals("copy")) {
			prop.put(key, val);
		} else {
			throw new NgException("Invalid param for " + key + ".");
		}
	}
	
	/**
	 * @param prop
	 * @param key
	 * @param val
	 * @throws GrpcException
	 */
	private void registerCompressParam(Properties prop,
		String key, String val) throws GrpcException {
		if (val.equals(RemoteMachineInfo.VAL_COMPRESS_RAW) ||
			val.equals(RemoteMachineInfo.VAL_COMPRESS_ZLIB)) {
			prop.put(key, val);
		} else {
			throw new NgException("Invalid param for " + key + ".");
		}
	}
	
	/**
	 * @return
	 */
	protected Properties getServerDefault() {
		return serverDefault;
	}
	
	/**
	 * @param attrName
	 */
	private void putNotSupportedWarning(String attrName) {
		System.err.println("The attribute " + attrName + " is not supported. ignore this setting.");
	}

	/* debug */
	public String toString() {
		StringBuffer sb = new StringBuffer();
		if (serverInfo != null) {
			sb.append("serverInfo is " + serverInfo.toString() + "\n");
		}
		if (mdsInfo != null) {
			sb.append("mdsInfo is " + mdsInfo.toString() + "\n");
		}
		if (localHostInfo != null) {
			sb.append("localHostInfo is " + localHostInfo.toString() + "\n");
		}
		if (localLDIFFiles != null) {
			sb.append("localLDIFFiles is " + localLDIFFiles.toString() + "\n");
		}
		if (functionList != null) {
			sb.append("functionList is " + functionList.toString() + "\n");
		}
		if (serverDefault != null) {
			sb.append("serverDefault is " + serverDefault.toString() + "\n");
		}
		
		return sb.toString();
	}
}
