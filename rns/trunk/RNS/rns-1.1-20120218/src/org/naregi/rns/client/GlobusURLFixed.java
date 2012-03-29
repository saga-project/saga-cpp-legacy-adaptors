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

import java.net.MalformedURLException;

import org.globus.util.GlobusURL;
import org.globus.util.Util;

/**
 * Extend and modify org.globus.util.GlobusURL
 *
 * @see <a
 *      href="http://www.cogkit.org/release/4_1_4/api/jglobus/org/globus/util/GlobusURL.html">org.globus.util.GlobusURL</a>
 */
public class GlobusURLFixed extends GlobusURL {

	public GlobusURLFixed(String url) throws MalformedURLException {
		super(url);
		protocol = null;
		host = null;
		urlPath = null;
		user = null;
		pwd = null;
		this.url = null;
		port = -1;
		url = url.trim();
		int p1 = url.indexOf("://");
		if (p1 == -1)
			throw new MalformedURLException("Missing '[protocol]://'");
		protocol = url.substring(0, p1).toLowerCase();
		p1 += 3;
		String base = null;
		int p2 = url.indexOf('/', p1);
		if (p2 == -1) {
			base = url.substring(p1);
			urlPath = null;
		} else {
			base = url.substring(p1, p2);
			if (p2 + 1 != url.length())
				urlPath = url.substring(p2);
			else
				urlPath = null;
		}
		p1 = base.indexOf('@');
		if (p1 == -1) {
			parseHostPort(base);
		} else {
			parseUserPwd(base.substring(0, p1));
			parseHostPort(base.substring(p1 + 1));
		}
		if (port == -1)
			port = getPort(protocol);
		if (protocol.equals("ftp") && user == null && pwd == null) {
			user = "anonymous";
			pwd = "anon@anon.com";
		}
		this.url = url;
	}

	private void parseHostPort(String str) throws MalformedURLException {
		int start = 0;
		if (str.length() > 0 && str.charAt(0) == '[') {
			start = str.indexOf(']');
			if (start == -1)
				throw new MalformedURLException("Missing ']' in IPv6 address: "
						+ str);
		}
		int p1 = str.indexOf(':', start);
		if (p1 == -1) {
			host = str;
		} else {
			host = str.substring(0, p1);
			String pp = str.substring(p1 + 1);
			try {
				port = Integer.parseInt(pp);
			} catch (NumberFormatException e) {
				throw new MalformedURLException("Invalid port number: " + pp);
			}
		}
	}

	private void parseUserPwd(String str) {
		int p1 = str.indexOf(':');
		if (p1 == -1) {
			user = Util.decode(str);
		} else {
			user = Util.decode(str.substring(0, p1));
			pwd = Util.decode(str.substring(p1 + 1));
		}
	}

	/**
	 * Set a user name of an url.
	 *
	 * @param user user name
	 */
	public void setUser(String user) {
		this.user = user;
	}

	/**
	 * Set a password of an url.
	 *
	 * @param password password
	 */
	public void setPwd(String password) {
		this.pwd = password;
	}
}
