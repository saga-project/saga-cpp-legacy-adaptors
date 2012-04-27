/*
 * Copyright (C) 2008-2012 Osaka University.
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URLDecoder;

import org.globus.ftp.DataChannelAuthentication;
import org.globus.ftp.FTPClient;
import org.globus.ftp.GridFTPClient;
import org.globus.gsi.gssapi.auth.Authorization;
import org.globus.io.streams.FTPInputStream;
import org.globus.io.streams.FTPOutputStream;
import org.globus.io.streams.GridFTPInputStream;
import org.globus.io.streams.GridFTPOutputStream;
import org.globus.util.GlobusURL;

/**
 * Utilities for FTP and GridFTP client.
 */
public class GridFTPUtil {
	private GridFTPUtil() {
	}

	public static FTPClient getFTPClient(Authorization auth, String url)
			throws Exception {
		GlobusURL gurl = new GlobusURLFixed(url);
		return getFTPClient(auth, gurl);
	}

	public static FTPClient getFTPClient(Authorization auth, GlobusURL gurl)
			throws Exception {
		boolean dcau = true;
		String proto = gurl.getProtocol();
		if (proto.equalsIgnoreCase("gsiftp")
				|| proto.equalsIgnoreCase("gridftp")) {
			GridFTPClient gridFtp = new GridFTPClient(gurl.getHost(),
					gurl.getPort());
			gridFtp.setAuthorization(auth);
			// gridFtp.authenticate(cred);
			gridFtp.authenticate(null); // important
			if (gridFtp.isFeatureSupported("DCAU")) {
				if (!dcau)
					gridFtp.setDataChannelAuthentication(DataChannelAuthentication.NONE);
			} else {
				gridFtp.setLocalNoDataChannelAuthentication();
			}
			return gridFtp;
		} else if (proto.equalsIgnoreCase("ftp")) {
			FTPClient ftp = new FTPClient(gurl.getHost(), gurl.getPort());
			ftp.authorize(gurl.getUser(), gurl.getPwd());
			return ftp;
		}
		throw new Exception("Destination protocol: " + proto
				+ " not supported!");
	}

	public static InputStream getInputStream(Authorization auth, String url)
			throws Exception {
		GlobusURL gurl = new GlobusURLFixed(url);
		return getInputStream(auth, gurl);
	}

	@SuppressWarnings("deprecation")
	public static InputStream getInputStream(Authorization auth, GlobusURL gurl)
			throws Exception {
		String path = URLDecoder.decode(gurl.getPath());
		String proto = gurl.getProtocol();
		if (proto.equalsIgnoreCase("gsiftp")
				|| proto.equalsIgnoreCase("gridftp")) {
			return new GridFTPInputStream(null, auth, gurl.getHost(),
					gurl.getPort(), path, false);
		} else if (proto.equalsIgnoreCase("ftp")) {
			return new FTPInputStream(gurl.getHost(), gurl.getPort(),
					gurl.getUser(), gurl.getPwd(), path);
		} else {
			throw new Exception("Destination protocol: " + proto
					+ " not supported!");
		}
	}

	public static OutputStream getOutputStream(Authorization auth, String url,
			boolean overwrite, boolean appendMode) throws Exception {
		GlobusURL gurl = new GlobusURLFixed(url);
		return getOutputStream(auth, gurl, overwrite, appendMode);
	}

	@SuppressWarnings("deprecation")
	public static OutputStream getOutputStream(Authorization auth,
			GlobusURL gurl, boolean overwrite, boolean appendMode)
			throws Exception {
		String path = URLDecoder.decode(gurl.getPath());
		if (!overwrite) {
			FTPClient ftp = getFTPClient(auth, gurl);
			if (ftp.exists(path)) {
				throw new Exception(gurl.getURL() + " exists.");
			}
			ftp.close();
		}
		String proto = gurl.getProtocol();
		if (proto.equalsIgnoreCase("gsiftp")
				|| proto.equalsIgnoreCase("gridftp")) {
			return new GridFTPOutputStream(null, auth, gurl.getHost(),
					gurl.getPort(), path, appendMode, false);
		} else if (proto.equalsIgnoreCase("ftp")) {
			return new FTPOutputStream(gurl.getHost(), gurl.getPort(),
					gurl.getUser(), gurl.getPwd(), path, false);
		} else {
			throw new Exception("Destination protocol: " + proto
					+ " not supported!");
		}
	}

	public static void streamCopy(InputStream in, OutputStream out,
			int bufferSize) throws IOException {
		byte buffer[] = new byte[bufferSize];
		int rsize;
		while ((rsize = in.read(buffer)) != -1) {
			out.write(buffer, 0, rsize);
		}
		out.flush();
	}
}
