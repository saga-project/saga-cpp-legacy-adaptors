/*
 * $RCSfile: InformationSourceSectionParser.java,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:07 $
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

final class InformationSourceSectionParser extends SectionParser {
	private static final String name = "INFORMATION_SOURCE";

	public InformationSourceSectionParser() {
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.SectionParser#checkEntry
	 */
	boolean checkEntry(List<NgConfigEntry> entries, NgConfigEntry ent)
	 throws NgConfigException {
		String key   = ent.getKey();

		if (! isAttribute(key)) {
			throw new NgConfigException("\"" + key + "\" is not attribute of " 
										+ name + " section.");
		}

		// Check multiple defines
		if ( isRedefine(entries, ent) ) {
			throw new NgConfigException("attribute \"" + key + "\" redefined");
		}

		// Check timeout
		if (key.equals("timeout")) {
			return checkTimeout(ent);
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
		return this.required_attribute;
	}

	private boolean isRedefine(final List<NgConfigEntry> entries,
	 final NgConfigEntry ent) {
		String key = ent.getKey();
		if (key.equals("source") || key.equals("option")) {
			return false;
		}
		if (contains(entries, ent) < 0) {
			return false; // does not contains
		}
		return true;
	}

	private boolean checkTimeout(NgConfigEntry ent) throws NgConfigException {
		EntryChecker.checkNegativeNumber(ent);
		return true;
	}

	private static final String [] required_attribute = {
		"type",
		"source",
		"tag"
	};

	private static final String [] attributes = {
		"type",
		"source",
		"tag",
		"path",
		"log_filePath",
		"timeout",
		"option"
	};

}
