/*
 * $RCSfile: ClientCommunicationProxySectionParser.java,v $ $Revision: 1.3 $ $Date: 2008/02/01 06:29:27 $
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

final class ClientCommunicationProxySectionParser extends SectionParser {
	private static final String name = "CLIENT_COMMUNICATION_PROXY";

	public ClientCommunicationProxySectionParser() {
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

		// check the buffer_size
		if (key.equals("buffer_size")) {
			return checkBufferSize(ent);
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

	private boolean checkBufferSize(NgConfigEntry buffer_size)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(buffer_size);
		return true;
	}

	private static final String [] required_attributes = {
		"type"
	};

	private static final String [] attributes = {
		"type",
		"path",
		"buffer_size",
		"log_filePath",
		"max_jobs",
		"option"
	};
}

