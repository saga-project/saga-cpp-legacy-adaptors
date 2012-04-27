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
package org.naregi.rns;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import org.apache.commons.logging.Log;

/**
 * Definitions of server configurations.
 */
public class RNSConfig {
	private static final String configFileName = "rns-server.conf";
	private static final String rnsServicePath = "/wsrf/services/rns/ResourceNamespaceService";
	private static final String listLteratorServicePath = "/wsrf/services/rns/ListIteratorService";
	private static final String searchServicePath = "/wsrf/services/rns/SearchService";
	private static final String searchIteratorServicePath = "/wsrf/services/rns/SearchIteratorService";
	private static final String extensionServicePath = "/wsrf/services/rns/ExtensionService";
	private static final String xqueryIteratorServicePath = "/wsrf/services/rns/XQueryIteratorService";
	private static final String rootID = "ROOT";

	private RNSConfig() {
	}

	public static String getRNSServicePath() {
		return rnsServicePath;
	}

	public static String getListIteratorServicePath() {
		return listLteratorServicePath;
	}

	public static String getSearchServicePath() {
		return searchServicePath;
	}

	public static String getSearchIteratorServicePath() {
		return searchIteratorServicePath;
	}

	public static String getExtensionServicePath() {
		return extensionServicePath;
	}

	public static String getXQueryIteratorServicePath() {
		return xqueryIteratorServicePath;
	}

	public static String getRootID() {
		return rootID;
	}

	/* ------------------------------------------------------------- */

	public static final String CONFDIR_ETC = System.getProperty("file.separator")
			+ "etc" + System.getProperty("file.separator");
	public static final String CONFDIR_HOME = System.getProperty("user.home")
			+ System.getProperty("file.separator") + ".";

	/* for configuration file */
	public static final String KEY_PREFIX = "rns.server.";

	public static final String KEY_ITERATOR_UNIT = "iteratorUnit";
	public static final String KEY_ATIME = "commitAccessTime";
	public static final String KEY_STORAGE_DIR = "storageDir";
	public static final String KEY_DBTYPE = "dbType";
	public static final String KEY_ACCESS_CONTROL_TYPE = "accessControlType";
	public static final String KEY_REPLACE_LOCAL_HOSTNAME = "replaceLocalHostName";
	public static final String KEY_REPLACE_LOCAL_PORT = "replaceLocalPort";
	public static final String KEY_DB_CACHE = "dbCache";
	public static final String KEY_MIN_MEMORY = "minFreeMemory";
	public static final String KEY_TMP_DIR = "tmpdir";
	public static final String KEY_LIMIT_METADATA_SIZE = "limitMetadataSize";
	public static final String KEY_TEMP_FILE_FOR_SEARCH_THRESHOLD = "tempFileForSearchThreshold"; /* for debug */
	public static final String KEY_DELAYED_DESTROY = "delayedDestroy"; /* for debug */
	public static final String KEY_DEBUG = "debug"; /* for debug */

	private static Properties prop = null;

	private synchronized static void loadConfig() {
		if (prop != null) {
			return;
		}
		prop = new Properties();

		Log logger = RNSLog.getLog();

		File f = new File(CONFDIR_HOME + configFileName);
		if (!f.canRead()) {
			f = new File(CONFDIR_ETC + configFileName);
		}
		FileInputStream inStream;
		try {
			inStream = new FileInputStream(f);
		} catch (FileNotFoundException e) {
			logger.warn("config file not found (use default values)");
			return;
		}
		try {
			prop.load(inStream);
		} catch (IOException e) {
			logger.error("config file error (use default values): "
					+ e.getMessage());
		} finally {
			try {
				inStream.close();
			} catch (IOException e) {
				logger.debug("config file error (close failed): "
						+ e.getMessage());
			}
		}
	}

	/* with default value */

	private static Integer iteratorUnit = null;

	public synchronized static int getIteratorUnit() {
		if (iteratorUnit != null) {
			return iteratorUnit;
		}
		loadConfig();
		iteratorUnit = Integer.parseInt(prop.getProperty(KEY_PREFIX
				+ KEY_ITERATOR_UNIT, "1000"));
		return iteratorUnit;
	}

	private static Boolean isCommitAccessTime = null;

	public synchronized static boolean isCommitAccessTime() {
		if (isCommitAccessTime != null) {
			return isCommitAccessTime;
		}
		loadConfig();
		isCommitAccessTime = Boolean.parseBoolean(prop.getProperty(KEY_PREFIX
				+ KEY_ATIME, "false"));
		return isCommitAccessTime;
	}

	private static String strageDir = null;

	public synchronized static String getStorageDir() {
		if (strageDir != null) {
			return strageDir;
		}
		loadConfig();
		strageDir = prop.getProperty(KEY_PREFIX + KEY_STORAGE_DIR,
				"/tmp/RNS_DATA");
		return strageDir;
	}

	private static String dataBaseType = null;

	public synchronized static String getDataBaseType() {
		if (dataBaseType != null) {
			return dataBaseType;
		}
		loadConfig();
		dataBaseType = prop.getProperty(KEY_PREFIX + KEY_DBTYPE,
				RNSDB.TYPE_DERBY);
		return dataBaseType;
	}

	private static String accessControlType = null;

	public synchronized static String getAccessControlType() {
		if (accessControlType != null) {
			return accessControlType;
		}
		loadConfig();
		accessControlType = prop.getProperty(KEY_PREFIX
				+ KEY_ACCESS_CONTROL_TYPE, AccessControlSwitch.TYPE_NONE);
		return accessControlType;
	}

	private static String replaceLocalHostName = null;

	public synchronized static String getReplaceLocalHostName() {
		if (replaceLocalHostName != null) {
			return replaceLocalHostName;
		}
		loadConfig();
		replaceLocalHostName = prop.getProperty(KEY_PREFIX
				+ KEY_REPLACE_LOCAL_HOSTNAME);
		return replaceLocalHostName;
	}

	private static String rplaceLocalPort = null;

	public synchronized static String getReplaceLocalPort() {
		if (rplaceLocalPort != null) {
			return rplaceLocalPort;
		}
		loadConfig();
		rplaceLocalPort = prop.getProperty(KEY_PREFIX + KEY_REPLACE_LOCAL_PORT);
		return rplaceLocalPort;
	}

	private static Boolean isDataBaseCache = null;

	public synchronized static boolean isDataBaseCache() {
		if (isDataBaseCache != null) {
			return isDataBaseCache;
		}
		loadConfig();
		isDataBaseCache = Boolean.parseBoolean(prop.getProperty(KEY_PREFIX
				+ KEY_DB_CACHE, "true"));
		return isDataBaseCache;
	}

	private static Integer minFreeMemory = null;

	public synchronized static int minFreeMemory() {
		if (minFreeMemory != null) {
			return minFreeMemory;
		}
		loadConfig();
		int tmp = Integer.parseInt(prop.getProperty(
				KEY_PREFIX + KEY_MIN_MEMORY, "10"));
		if (tmp < 1) {
			tmp = 1;
		}
		minFreeMemory = tmp * 1024 * 1024;
		return minFreeMemory;
	}

	private static String tmpdir;

	public synchronized static String tmpdir() {
		if (tmpdir != null) {
			return tmpdir;
		}
		loadConfig();
		tmpdir = prop.getProperty(KEY_PREFIX + KEY_TMP_DIR, "/tmp/RNS_TMP");
		return tmpdir;
	}

	private static Integer limitMetadataSize = null;

	public synchronized static int limitMetadataSize() {
		if (limitMetadataSize != null) {
			return limitMetadataSize;
		}
		loadConfig();
		limitMetadataSize = Integer.parseInt(prop.getProperty(KEY_PREFIX
				+ KEY_LIMIT_METADATA_SIZE, "1048576")); /* 1MB */
		return limitMetadataSize;
	}

	private static Integer tempFileForSearchThreshold = null; /* for debug */

	public synchronized static int tempFileForSearchThreshold() {
		if (tempFileForSearchThreshold != null) {
			return tempFileForSearchThreshold;
		}
		loadConfig();
		tempFileForSearchThreshold = Integer.parseInt(prop.getProperty(
				KEY_PREFIX + KEY_TEMP_FILE_FOR_SEARCH_THRESHOLD, "1"));
		return tempFileForSearchThreshold;
	}

	private static Boolean delayedDestroy = null; /* for debug */

	public synchronized static boolean delayedDestroy() {
		if (delayedDestroy != null) {
			return delayedDestroy;
		}
		loadConfig();
		delayedDestroy = Boolean.parseBoolean(prop.getProperty(
				KEY_PREFIX + KEY_DELAYED_DESTROY, "false"));
		return delayedDestroy;
	}

	private static Boolean debug = null; /* for debug */

	public synchronized static boolean debug() {
		if (debug != null) {
			return debug;
		}
		loadConfig();
		debug = Boolean.parseBoolean(prop.getProperty(
				KEY_PREFIX + KEY_DEBUG, "false"));
		return debug;
	}
}
