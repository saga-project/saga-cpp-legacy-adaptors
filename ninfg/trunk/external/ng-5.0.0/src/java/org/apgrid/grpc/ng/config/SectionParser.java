/*
 * $RCSfile: SectionParser.java,v $ $Revision: 1.6 $ $Date: 2008/02/07 08:17:43 $
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

abstract class SectionParser {

	protected List<NgConfigSection> config = null;
	private static final char COMMENT   = '#';
	private static final char TAG_OPEN  = '<';
	private static final char TAG_CLOSE = '>';

	// It is mean values of the number of elements of each section.
	private static final int DEFAULT_CAPACITY = 14;

	/*
	 * @return configurations of some section.
	 */
	public List<NgConfigSection> parse(BufferedReader config_io)
	 throws IOException, NgConfigException {
		if (config_io == null) {
			throw new IOException("Configuration I/O is null.");
		}
		List<NgConfigEntry> entries =
			new ArrayList<NgConfigEntry>(DEFAULT_CAPACITY);

		String line = null;
		boolean tag_is_closed = false;
		while ((line = config_io.readLine()) != null) {
			int i = indexOfNotSpace(line); // skip space
			if ( i < 0 ) { // line is all spaces or null line
				continue;
			}

			if ( isCommentChar(line.charAt(i)) ) {
				continue;
			}

			if ( isTagClosed(line.substring(i)) ) { // </NAME>
				tag_is_closed = true;
				break;
			}

			String entry_str = line.substring(i);
			if ( isNotQuoteComplete(entry_str) ) {
				if (entry_str.charAt(entry_str.length()-1) != '\\' ) {
					throw new NgConfigException("Doesn't close quotation marks");
				}
				StringBuilder sb =
				 new StringBuilder(entry_str.substring(0, entry_str.length()-2));
				sb.append( completeQuotedArgument(config_io) );
				entry_str = sb.toString(); 
			}

			NgConfigEntry ent = parseEntry(entry_str);
			if (! checkEntry(entries, ent) ) {
				continue;
			}
			entries.add(ent);
		}

		checkRequiredAttr(entries);

		if (! tag_is_closed ) {
			throw new NgConfigException("Not yet finished section \"" 
										+ getName() + "\"");
		}

		config = new ArrayList<NgConfigSection>();
		config.add(new NgConfigSection(getName(), entries));
		return config;
	}

	/**
	 * Check the parsed config entry
	 * @param entries Already set config entries.
	 * @param ent parsed config entry.
	 * @return true Argument ent is valid.
	 * @return false Argument ent is invalid, but not error
	 * @throws NgConfigException Argument ent is invalid.
	 */
	abstract boolean checkEntry(List<NgConfigEntry> entries, NgConfigEntry ent)
	 throws NgConfigException;

	// get Section name
	abstract String getName();
	// get Section attributes
	abstract String [] getAttributes();
	// get cannot be omitted attributes in Section
	abstract String [] getRequiredAttributes();

	protected boolean isAttribute(String key) {
		String [] attributes = getAttributes();
		for (int i = 0; i < attributes.length  ; i++) {
			if (key.equals(attributes[i])) {
				return true;
			}
		}
		return false;
	}

	protected int contains(final List<NgConfigEntry> entries, 
	 final NgConfigEntry ent) {
		int ret = 0;
		for (NgConfigEntry e : entries) {
			if (e.getKey().equals(ent.getKey())) {
				return ret;
			}
			ret++;
		}
		return -1;
	}

	private void checkRequiredAttr(List<NgConfigEntry> entries)
	 throws NgConfigException {
		String [] required = getRequiredAttributes();

		// there is not required attribute
		if (required.length == 0) return;

		boolean found;
		for (int i = 0; i < required.length; i++) {
			found = false;
			for (NgConfigEntry ent : entries) {
				if (required[i].equals(ent.getKey())) {
					found = true;
					break;
				}
			}
			if (! found)
				throw new NgConfigException("<" + getName() 
					+ "> attribute \"" + required[i] + "\" cannot be omitted");
		}
	}

/////////////////////////////////////////////////
/////     Private Methods                   /////
/////////////////////////////////////////////////

	private boolean isNotQuoteComplete(String str)
	 throws NgConfigException {
		int i = indexOfDoubleQuote(str);
		if (i < 0) { // There is no '"'
			return false;
		}

		if (indexOfDoubleQuote(str, i+1) < 0 ) {
			return true;
		}

		return false;
	}

	private int indexOfDoubleQuote(String str) {
		return indexOfDoubleQuote(str, 0);
	}
	private int indexOfDoubleQuote(String str, int fromIdx) {
		if (fromIdx < 0) {
			return -1;
		}
		int nextSt = fromIdx, i = 0, length = str.length();
		while (nextSt < length ) {
			i = str.indexOf('"', nextSt);
			if (i < 0) {
				return -1;
			} 
			if ('\\' != str.charAt(i-1)) {
				return i;
			}
			nextSt = i + 1;
		}
		return -1;
	}

	private String completeQuotedArgument(BufferedReader conf_io)
	 throws IOException, NgConfigException {
		StringBuilder sb = new StringBuilder();
		String line = null;
		while ((line = conf_io.readLine()) != null) {
			if (line.charAt(line.length()-1) == '\\') { // last char is '\'
				int i = indexOfNotSpace(line);
				sb.append(line.substring(i, line.length()-2));
				continue;
			}
			if (indexOfDoubleQuote(line) > -1) { // There is '"'
				int i = indexOfNotSpace(line);
				sb.append(line.substring(i));
				break;
			}
			throw new NgConfigException("Doesn't close quotation marks");
		}
		return sb.toString();
	}

	private NgConfigEntry parseEntry(String str) throws NgConfigException {
		if (str == null) {
			throw new RuntimeException("unexpected argument null");
		}
		int attr_end = indexOfNextSpace(str);
		if (attr_end < 0) {
			attr_end = str.length();
		}
		String key = str.substring(0, attr_end); // Set the key

		int arg_start = indexOfNotSpace(str, attr_end); // skip spaces
		if (arg_start < 0) {
			throw new NgConfigException("No argument for the attribute \"" 
										+ key + "\"");
		}
		arg_start += attr_end;
		if (isCommentChar(str.charAt(arg_start))) {
			throw new NgConfigException("No argument for the attribute \"" 
										+ key + "\"");
		}

		if (str.charAt(arg_start) == '"') {
			arg_start++;
		}
		String val = mkValue(str.substring(arg_start));

		int i = indexOfNotSpace(str, arg_start+val.length()+1);
		if ( (i >= 0) && (! isCommentChar(str.charAt(arg_start+val.length()+i)))) {
			throw new NgConfigException("multiple keyword appear in same line \"" + str + "\"");
		}

		return new NgConfigEntry(key, val);
	}

	private String mkValue(String str) {
		int arg_start = 0;
		int arg_end = 0;

		arg_end = indexOfDoubleQuote(str);
		if (arg_end < 0) {
			arg_end = indexOfNextSpace(str);
			if (arg_end < 0) { // There is no space(' \t\n\r')
				arg_end = str.length();
			}
		}
		return str.substring(arg_start, arg_end);
	}

	private int indexOfNextSpace(String str) {
		return indexOfNextSpace(str, 0);
	}

	private int indexOfNextSpace(String str, int fromIndex) {
		char [] cbuf = str.toCharArray();
		for (int i = fromIndex, j = 0; i < cbuf.length; i++, j++) {
            if (( ' ' == cbuf[i]) || ('\t' == cbuf[i]) ||
                ('\n' == cbuf[i]) || ('\r' == cbuf[i])) {
                return j;
            }
		}
		return -1;
	}

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

	private int indexOfTagCloseChar(String str) {
		return str.indexOf(TAG_CLOSE);
	}

	private boolean isCommentChar(char c) {
		return COMMENT == c;
	}

	private boolean isTagClosed(String aTag) {
		if (aTag.charAt(0) != TAG_OPEN || aTag.charAt(1) != '/') {
			return false;
		}
		int i = indexOfTagCloseChar(aTag);
		if (i < 0) {
			return false;
		}
		if (getName().equals(aTag.substring(2, i))) {
			return true;
		}
		return false;
	}

}

