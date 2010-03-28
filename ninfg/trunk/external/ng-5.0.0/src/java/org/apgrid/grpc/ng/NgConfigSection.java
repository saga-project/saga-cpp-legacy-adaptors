/*
 * $RCSfile: NgConfigSection.java,v $ $Revision: 1.4 $ $Date: 2008/01/07 05:51:45 $
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
package org.apgrid.grpc.ng;

import java.util.List;
import java.util.Iterator;

/**
 * Representation of a section in Ninf-G Client Configuration.
 */
public class NgConfigSection implements Iterable<NgConfigEntry> {

	/* The section name */
	private String name = null;

	private List<NgConfigEntry> entries;

	public NgConfigSection(String name, List<NgConfigEntry> entries) {
		this.entries = entries;
		this.name = name;
	}

	public String getName() {
		return name;
	}

	protected void update(NgConfigSection newCs) {
		for (NgConfigEntry newCe : newCs.entries) {
			String newKey = newCe.getKey();
			String newValue = newCe.getValue();
			if (newKey.equals("type") || newKey.equals("tag")) {
				continue;
			}

			NgConfigEntry same = null;
			for (NgConfigEntry ce : entries) {
				String key = ce.getKey();
				String value = ce.getValue();
				if (newKey.equals(key) && newValue.equals(value)) {
					same = ce;
					break;
				}
			}
			if (same == null) {
				entries.add(newCe);
			}
		}
	}

	public NgConfigEntry get(String key) {
		for (NgConfigEntry e : entries) {
			if (key.equals(e.getKey())) {
				return e;
			}
		}
		return null;
	}

	public NgConfigEntry get(int index) {
		return entries.get(index);
	}

	public Iterator<NgConfigEntry> iterator() {
		return entries.iterator();
	}

	public int size() {
		return entries.size();
	}

	public boolean isEmpty() {
		return entries.isEmpty();
	}

	public String toString() {
		return name + "=" + entries + "\n";
	}
}

