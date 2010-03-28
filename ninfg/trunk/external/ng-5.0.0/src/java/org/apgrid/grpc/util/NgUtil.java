/*
 * $RCSfile: NgUtil.java,v $ $Revision: 1.4 $ $Date: 2008/02/07 08:17:43 $
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
package org.apgrid.grpc.util;

import java.net.InetAddress;
import java.net.UnknownHostException;
import org.apgrid.grpc.ng.NgGrpcHandle;
import org.apgrid.grpc.ng.NgGrpcClient;

import java.io.File;

public class NgUtil {

	private NgUtil() {
	}

	public static String getLocalHostName() {
		try {
			return InetAddress.getLocalHost().getCanonicalHostName();
		} catch (UnknownHostException e) {
			return "127.0.0.1";
		}
	}

	public static String getDefaultPath(String cmd, String type) {
		String ng_dir =
			System.getenv("NG_DIR");
		// set current directory unless set NG_DIR environment variable
		if (ng_dir == null)
			ng_dir = ".";
		StringBuilder sb = new StringBuilder(ng_dir);
		sb.append(File.separator).append("bin").append(File.separator);
		sb.append(cmd).append(".").append(type);
		if ( isWindowsOS() ) {
			sb.append(".bat");
		}
		return sb.toString();
	}

	private static boolean isWindowsOS() {
		String os_name = System.getProperty("os.name");
		return os_name.startsWith("Windows");
	}

}
