/*
 * Copyright (C) 2008-2011 Osaka University.
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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
package org.naregi.rns.client;

/**
 * Divided pathname into dirname and basename.
 */
public class DirnameBasename {
	private String dirname;
	private String basename;

	public DirnameBasename(String path) throws RNSError {
		if (path == null) {
			dirname = "/";
			basename = null;
			return;
		}
		boolean fullpath = false;
		String[] p = path.split("/");
		StringBuilder parent = new StringBuilder();
		String name;
		if (path.startsWith("/")) {
			parent.append("/");
			fullpath = true;
		} else if (p.length == 1) {
			dirname = null;
			basename = p[0];
			return;
		}
		if (p.length == 0) {
			name = null;
		} else { /* p.length >= 1 */
			int start = 0;
			if (fullpath) {
				start = 1;
			}
			for (int i = start; i < p.length - 1; i++) {
				parent.append(p[i]);
				if (i < p.length - 2) {
					parent.append("/");
				}
			}
			name = p[p.length - 1];
		}
		dirname = parent.toString();
		basename = name;
		return;
	}

	/**
	 * Get the dirname.
	 *
	 * @return dirname
	 */
	public String getDirname() {
		return dirname;
	}

	/**
	 * Set a dirname.
	 *
	 * @param dirname
	 */
	public void setDirname(String dirname) {
		this.dirname = dirname;
	}

	/**
	 * Get the basename.
	 *
	 * @return basename
	 */
	public String getBasename() {
		return basename;
	}

	/**
	 * Set a basename.
	 *
	 * @param basename
	 */
	public void setBasename(String basename) {
		this.basename = basename;
	}
}
