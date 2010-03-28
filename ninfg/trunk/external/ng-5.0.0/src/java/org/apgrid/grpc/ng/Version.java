/*
 * $RCSfile: Version.java,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:07 $
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

public class Version {
	private final int major;
	private final int minor;
	private final int patch;

	public Version(int major, int minor, int patch) {
		if ( (major < 0) || (minor < 0) || (patch < 0) )
			throw new IllegalArgumentException();

		this.major = major;
		this.minor = minor;
		this.patch = patch;
	}

	public int getMajor() {
		return major;
	}
	public int getMinor() {
		return minor;
	}
	public int getPatch() {
		return patch;
	}

	public int getInt() {
		return (major << 24) + (minor << 16) + patch;
	}

	public int compareTo(Version another) {
		if (another == null)
			throw new NullPointerException();

		int a_major = another.getMajor();
		int a_minor = another.getMinor();
		int a_patch = another.getPatch();

		if (this.major > a_major) {
			return 1;
		} else if (this.major < a_major) {
			return -1;
		}
		if (this.minor > a_minor) {
			return 1;
		} else if (this.minor < a_minor) {
			return -1;
		}
		if (this.patch > a_patch) {
			return 1;
		} else if (this.patch < a_patch) {
			return -1;
		}
		return 0;
	}

	public String toString() {
		return major + "." + minor + "." + patch;
	}

}

