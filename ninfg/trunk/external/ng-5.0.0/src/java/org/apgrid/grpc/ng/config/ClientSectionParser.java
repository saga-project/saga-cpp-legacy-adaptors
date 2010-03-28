/*
 * $RCSfile: ClientSectionParser.java,v $ $Revision: 1.9 $ $Date: 2008/03/25 05:39:08 $
 * $AIST_Release: 5.0.0 $
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
 */
package org.apgrid.grpc.ng.config;

import java.io.BufferedReader;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

import org.apgrid.grpc.ng.NgConfigSection;
import org.apgrid.grpc.ng.NgConfigEntry;
import org.apgrid.grpc.ng.NgConfigException;

final class ClientSectionParser extends SectionParser {
	private static final String name = "CLIENT";

	public ClientSectionParser() {
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.SectionParser#checkEntry
	 */
	boolean checkEntry(List<NgConfigEntry> entries, NgConfigEntry ent)
	 throws NgConfigException {
		String key = ent.getKey();

		if (! isAttribute(key)) {
			throw new NgConfigException("\"" + key + "\" is not attribute of " 
										+ name + " section.");
		}

		// Does not multiple defines all attributes of CLIENT section.
		if ( isRedefine(entries, ent) ) {
			throw new NgConfigException("attribute \"" + key + "\" redefined");
		}

		// check the not support attribute on Java Client
		if ( notSupportAttribute(ent) ) {
			return false;
		}

		// check the loglevels
		if (key.startsWith("loglevel")) {
			if (key.endsWith("globusToolkit")) {
				// loglevel_globusToolkit is invalid on Ninf-G5
				// this attribute remove at Ninf-G ver5.0.0 release
				return false;
			}
			return checkLogLevelValue(ent);
		}

		// check the log_nFiles
		if (key.equals("log_nFiles")) {
			return checkLogNumFiles(ent);
		}

		if (key.equals("log_maxFileSize")) {
			return checkLogMaxFileSize(ent);
		}

		// check the log_overwriteDirectory
		if (key.equals("log_overwriteDirectory")) {
			return checkLogOverwriteDirectory(ent);
		}

		// check the listen ports
		if (key.startsWith("listen_port")) {
			if ( (key.endsWith("authonly")) ||
				 (key.endsWith("GSI")) ||
				 (key.endsWith("SSL")) ) {
				// these attributes are invalid on Ninf-G5
				// this attributes remove at Ninf-G ver5.0.0 release
				return false;
			}
			return checkListenPort(ent);
		}

		return true;
	}


	String getName() {
		return this.name;
	}

	String [] getAttributes() {
		return this.attributes;
	}

	String [] getRequiredAttributes() {
		return this.required_attributes;
	}

	private boolean isRedefine(final List<NgConfigEntry> entries,
	 final NgConfigEntry ent) {
		if (contains(entries, ent) < 0) {
			return false;
		}
		return true;
	}

	private boolean notSupportAttribute(NgConfigEntry ent) {
		String [] not_support_attr = { 
			"fortran_compatible", "handling_signals"
		};

		for (int i = 0; i < not_support_attr.length; i++) {
			if ( ent.getKey().equals( not_support_attr[i] ) ) {
				return true;
			}
		}
		return false;
	}

	private boolean checkLogLevelValue(NgConfigEntry loglevel)
	 throws NgConfigException {
		String value = loglevel.getValue();

		String initial = "OFEWID";
		String [] level_str = {
			"Off", "Fatal", "Error", "Warning", "Information", "Debug"
		};
		int i = initial.indexOf(value.toUpperCase().charAt(0));
		if (i >= 0) {
			// Log Level is String
			for (int j = 0; j < level_str.length; j++) {
				if (level_str[j].equals(value)) { return true ; }
			}
			throw new NgConfigException("Invalid log level " + loglevel);
		}

		// Log Level is number
		try {
			Integer lv = Integer.parseInt(loglevel.getValue());
			if (lv < 0 || lv > 5) {
				throw new NgConfigException("Invalid log level " + loglevel);
			} 
		} catch (NumberFormatException e) {
			throw new NgConfigException(e);
		}
		return true;
	}

	private boolean checkLogNumFiles(NgConfigEntry log_nFiles)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(log_nFiles);
		return true;
	}

	private boolean checkLogMaxFileSize(NgConfigEntry log_maxFileSize) 
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(log_maxFileSize);
		return true;
	}
	
	private boolean checkLogOverwriteDirectory(NgConfigEntry log_overwriteDirectory)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(log_overwriteDirectory);
		return true;
	}

	private boolean checkListenPort(NgConfigEntry listen_port)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(listen_port);
		return true;
	}

	// nothing required attribute in CLIENT section.
	private static final String [] required_attributes = {};

	private static final String [] attributes = {
		"hostname",
		"save_sessionInfo",
		"loglevel",
		"loglevel_globusToolkit",
		"loglevel_ninfgProtocol",
		"loglevel_ninfgInternal",
		"loglevel_ninfgGrpc",
		"log_filePath",
		"log_suffix",
		"log_nFiles",
		"log_maxFileSize",
		"log_overwriteDirectory",
		"tmp_dir",
		"refresh_credential",
		"invoke_server_log",
		"fortran_compatible",
		"handling_signals",
		"listen_port",
		"listen_port_authonly",
		"listen_port_GSI",
		"listen_port_SSL",
		"information_service_log",
		"client_communication_proxy_log"
	};

}
