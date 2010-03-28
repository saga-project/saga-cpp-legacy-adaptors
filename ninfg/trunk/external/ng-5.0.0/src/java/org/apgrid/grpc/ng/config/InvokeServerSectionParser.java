/*
 * $RCSfile: InvokeServerSectionParser.java,v $ $Revision: 1.5 $ $Date: 2007/09/26 04:14:07 $
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

import java.util.List;

import org.apgrid.grpc.ng.NgConfigSection;
import org.apgrid.grpc.ng.NgConfigEntry;
import org.apgrid.grpc.ng.NgConfigException;

final class InvokeServerSectionParser extends SectionParser {
	private static final String name = "INVOKE_SERVER";

	public InvokeServerSectionParser() {
	}

	boolean checkEntry(List<NgConfigEntry> entries, NgConfigEntry ent)
	 throws NgConfigException {
		if (ent == null) {
			throw new NgConfigException("Argument NgConfigEntry is null");
		}

		String key = ent.getKey();
		if (! isAttribute(key)) {
			throw new NgConfigException("\"" + key 
										+ "\" is not attribute of " 
										+ name + " section.");
		}

		if ( isRedefine(entries, ent) ) {
			throw new NgConfigException("attribute \"" + key + "\" redefined");
		}

		// check the max jobs
		if (key.equals("max_jobs")) {
			return checkMaxJobs(ent);
		}

		// check the status polling
		if (key.equals("status_polling")) {
			return checkStatusPolling(ent);
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
		// "option" is permitted multiple definition in this section
		if (ent.getKey().equals("option")) { return false; } 

		if (contains(entries, ent) < 0) {
			return false;
		}
		return true;
	}

	private boolean checkMaxJobs(NgConfigEntry max_jobs)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(max_jobs);
		return true;
	}

	private boolean checkStatusPolling(NgConfigEntry status_polling)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(status_polling);
		return true;
	}

	private static final String [] required_attributes = {
		"type"
	};

	private static final String [] attributes = {
		"type",
		"path",
		"max_jobs",
		"log_filePath",
		"status_polling",
		"option"
	};
}
