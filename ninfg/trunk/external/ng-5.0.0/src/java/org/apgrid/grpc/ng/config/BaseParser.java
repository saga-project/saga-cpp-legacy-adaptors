/*
 * $RCSfile: BaseParser.java,v $ $Revision: 1.9 $ $Date: 2007/12/12 05:44:21 $
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
import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

import org.apgrid.grpc.ng.NgConfigSection;
import org.apgrid.grpc.ng.NgConfigEntry;
import org.apgrid.grpc.ng.NgConfigException;

class BaseParser {

	private static final char COMMENT   = '#';
	private static final char TAG_OPEN  = '<';
	private static final char TAG_CLOSE = '>';

	private static class Section {
		static final String CLIENT = "CLIENT";
		static final String SERVER = "SERVER";
		static final String INFORMATION_SOURCE = "INFORMATION_SOURCE";
		static final String INVOKE_SERVER = "INVOKE_SERVER";
		static final String INCLUDE = "INCLUDE";
		static final String FUNCTION_INFO = "FUNCTION_INFO";
		static final String SERVER_DEFAULT = "SERVER_DEFAULT";
		static final String CLIENT_COMMUNICATION_PROXY =
			"CLIENT_COMMUNICATION_PROXY";
	}

	private static final String [] prohibitRepetition = {
		"CLIENT",
		"SERVER_DEFAULT"
	 };

	private List<NgConfigSection> config = null;
	private List<File> includedFiles = null;

	public BaseParser() {
		this.includedFiles = new ArrayList<File>();
	}

	public BaseParser(List<File> files) {
		this.includedFiles = files;
	}

	/*
	 * Parse the Ninf-G Client Configuration File
	 *
	 * @param conf_io Configuration File Reader
	 */
	public List<NgConfigSection> parse(BufferedReader conf_io)
	 throws IOException, NgConfigException {

		config = new ArrayList<NgConfigSection>();

		// begin parse 
		String line = null;
		while ((line = conf_io.readLine()) != null) {
			int i = indexOfNotSpace(line);
			if ( i < 0 ) { // line is all spaces or null line
				continue;
			}

			char c = line.charAt(i);
			if ( isCommentChar(c) ) {
				continue;
			}
			if ( isTagOpenChar(c) ) {
				int j = indexOfTagCloseChar(line); // search '>'
				if ( j < 0 ) { // line is all spaces or null line
					throw new NgConfigException("Invalid Tag " + line);
				}

				// Create concrete section parser with tag name
				SectionParser section =
					createSectionParser(line.substring(i+1, j));

				// Check line that follows tag closed
				int k = indexOfNotSpace(line, j+1);
				if (( k >= 0 ) && (! isCommentChar(line.charAt(j+k+1)))) {
					throw new NgConfigException("multiple keyword appear in same line");
				}

				// parse the section
				List<NgConfigSection> result = section.parse(conf_io);

				appendSectionConfig(result);
			} else {
				throw new NgConfigException("Not starting with section " + line);
			}
		}

		return config;
	}

	/**
	 * @return index of not space(\t\n\r ).
	 *         return -1 if argument str is all spaces or null line.
	 */
    private int indexOfNotSpace(String str) {
        return indexOfNotSpace(str, 0);
    }
	private int indexOfNotSpace(String str, int fromIndex) {
		char [] cbuf = str.toCharArray();
		for (int i = fromIndex, j = 0; i < cbuf.length; i++, j++) {
			if (( ' ' == cbuf[i]) || ('\t' == cbuf[i]) ||
				('\n' == cbuf[i]) || ('\r' == cbuf[i])) {
				continue;
			} else {
				return j;
			}
		}
		return -1;
	}

	private boolean isCommentChar(char c) {
		return COMMENT == c;
	}

	private boolean isTagOpenChar(char c) {
		return TAG_OPEN == c;
	}

	/**
	 * @return index of tag close character(>).
	 *         return -1 if argument str is all spaces or null line.
	 */
	private int indexOfTagCloseChar(String str) {
		return str.indexOf(TAG_CLOSE);
	}

	/**
	 * Factory method that it creates concrete SectionParser class.
	 *
	 * @param section_name Concrete section parser name
	 * @param Concrete section parser
	 */
	private SectionParser createSectionParser(String section_name)
	 throws NgConfigException {

		if (section_name.equals(Section.CLIENT)) {
			return new ClientSectionParser();
		} else if (section_name.equals(Section.SERVER)) {
			return new ServerSectionParser();
		} else if (section_name.equals(Section.INVOKE_SERVER)) {
			return new InvokeServerSectionParser();
		} else if (section_name.equals(Section.INCLUDE)) {
			return new IncludeSectionParser(includedFiles);
		} else if (section_name.equals(Section.FUNCTION_INFO)) {
			return new FunctionInfoSectionParser();
		} else if (section_name.equals(Section.SERVER_DEFAULT)) {
			return new ServerDefaultSectionParser();
		} else if (section_name.equals(Section.INFORMATION_SOURCE)) {
			return new InformationSourceSectionParser();
		} else if (section_name.equals(Section.CLIENT_COMMUNICATION_PROXY)) {
			return new ClientCommunicationProxySectionParser();
		} else {
			throw new NgConfigException("No such section \"" 
										+ section_name + "\"");
		}
	}

	/*
	 * Append parsed sections
	 */
	private void appendSectionConfig(List<NgConfigSection> sections)
	 throws NgConfigException {
		if (config == null) {
			throw new RuntimeException("config field is null");
		}

		for (NgConfigSection s : sections) {
			if ( s.isEmpty() ) {
				//System.err.println(s.getName() + " section is empty. ignore");
				continue;
			}
			checkSection(s);
			checkTag(s);
			config.add(s);
		}
	}

	private boolean checkSection(NgConfigSection section)
	 throws NgConfigException {
		if ( isDuplicateSection(section) && isNotPermitRepetition(section) ) {
			throw new NgConfigException(section.getName() 
										+ " section is duplicate.");
		}
		return true;
	}

	private boolean isDuplicateSection(NgConfigSection section) {
		String name = section.getName();
		for (NgConfigSection cs : config) {
			if (name.equals(cs.getName())) {
				return true;
			}
		}
		return false;
	}

	private boolean isNotPermitRepetition(NgConfigSection section) {
		String name = section.getName();
		for (int i = 0; i < prohibitRepetition.length  ; i++) {
			if (name.equals(prohibitRepetition[i])) {
				return true;
			}
		}
		return false;
	}

	/*
	 * Check "tag" attribute in SERVER or INFORMATION_SOURCE section
	 */
	private void checkTag(NgConfigSection section)
	 throws NgConfigException {
		String sect_name = section.getName();
		if (sect_name.equals("SERVER")) {
			checkTagInServer(section);
		} else if (sect_name.equals("INFORMATION_SOURCE")) {
			checkTagInInformationSource(section);
		} else {
			return ; // section has not "tag"
		}
	}

	// Check "tag" attribute in SERVER section
	private void checkTagInServer(NgConfigSection section)
	 throws NgConfigException {
		NgConfigEntry ent = section.get("tag");
		if (ent == null)
			return; // does not specified "tag"
		String tag = ent.getValue();

		for (NgConfigSection sect : config) {
			if (! sect.getName().equals("SERVER") )
				continue;
			ent = sect.get("tag");
			if (ent == null)
				continue;

			if (tag.equals(ent.getValue())) 
				throw new NgConfigException("tag \"" + tag 
					+ "\" doesn't unique");
		}
	}

	// Check "tag" attribute in INFORMATION_SOURCE section
	private void checkTagInInformationSource(NgConfigSection section)
	 throws NgConfigException {
		NgConfigEntry ent = section.get("tag");
		if (ent == null)
			return; // does not specified "tag"
		String tag  = ent.getValue();
		String type = section.get("type").getValue();

		for (NgConfigSection sect : config) {
			if (! sect.getName().equals("INFORMATION_SOURCE") )
				continue;
			NgConfigEntry reg_ent = sect.get("tag");
			if (reg_ent == null)
				continue;

			String reg_type = sect.get("type").getValue();
			if (type.equals(reg_type) &&
				 tag.equals(reg_ent.getValue()) ) {
				throw new NgConfigException("type \"" + type + "\" tag \"" 
					+ tag + "\" doesn't unique");
			}
		}
	}

}

