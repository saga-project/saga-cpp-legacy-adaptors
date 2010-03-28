/*
 * $RCSfile: IncludeSectionParser.java,v $ $Revision: 1.6 $ $Date: 2008/03/11 06:30:12 $
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
import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.apgrid.grpc.ng.NgConfigSection;
import org.apgrid.grpc.ng.NgConfigEntry;
import org.apgrid.grpc.ng.NgConfigException;

final class IncludeSectionParser extends SectionParser {
	private static final String name = "INCLUDE";
	
	private static final String [] required_attributes = {};
	private static final String [] attributes = { "filename" };

	public IncludeSectionParser(List<File> files) {
	}

	/* (non-Javadoc) 
	 * @see org.apgrid.grpc.ng.SectionParser#checkEntry
	 */
	boolean checkEntry(List<NgConfigEntry> entries, NgConfigEntry ent)
	 throws NgConfigException {
		if (ent == null) {
			throw new RuntimeException("Argument NgConfigEntry is null");
		}

		if (! isAttribute(ent.getKey()) ) {
			throw new NgConfigException("\"" + ent.getKey() + "\" is not attribute of " + name + " section." );
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
}
