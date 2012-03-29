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

import java.io.Serializable;
import java.util.Calendar;
import java.util.Comparator;

import org.naregi.rns.util.RNSUtil;

/**
 * Comparator for RNSDirent. Sort by name, ctime, atime or mtime.
 */
public class RNSDirentComparator implements Comparator<RNSDirent>, Serializable {

	private static final long serialVersionUID = 1L;

	/**
	 * Sorting modes.
	 */
	public enum MODE {
		NAME, CTIME, ATIME, MTIME
	};

	private MODE mode;
	private boolean reverse;

	/**
	 * Initialize RNSDirentComparator.
	 *
	 * @param mode NAME, CTIME, ATIME or MTIME
	 * @param reverse sort reversely if this is true
	 */
	public RNSDirentComparator(MODE mode, boolean reverse) {
		this.mode = mode;
		this.reverse = reverse;
	}

	public int compare(RNSDirent o1, RNSDirent o2) {
		int rev;
		if (reverse) {
			rev = -1;
		} else {
			rev = 1;
		}
		return compare2(o1, o2) * rev;
	}

	private int compare2(RNSDirent o1, RNSDirent o2) {
		if (o1 == null || o2 == null) {
			return 1;
		}
		if (RNSUtil.isDirectory(o1.getMeta())) {
			if (!RNSUtil.isDirectory(o2.getMeta())) {
				return -1;
			}
		} else if (RNSUtil.isDirectory(o2.getMeta())) {
			if (!RNSUtil.isDirectory(o1.getMeta())) {
				return 1;
			}
		}
		String name1 = o1.getName();
		String name2 = o2.getName();
		if (mode.equals(MODE.NAME)) {
			return compareName(name1, name2);
		}
		RNSStat st1 = o1.getStat();
		RNSStat st2 = o2.getStat();
		if (st1 == null || st2 == null) {
			if (st1 != null) {
				return 1;
			} else if (st2 != null) {
				return -1;
			}
			return compareName(name1, name2);
		}
		Calendar cal1, cal2;
		switch (mode) {
		case CTIME:
			cal1 = st1.getCreateTime();
			cal2 = st2.getCreateTime();
			break;
		case ATIME:
			cal1 = st1.getAccessTime();
			cal2 = st2.getAccessTime();
			break;
		case MTIME:
			cal1 = st1.getModificationTime();
			cal2 = st2.getModificationTime();
			break;
		default:
			return compareName(name1, name2);
		}
		if (cal1 == null || cal2 == null) {
			if (cal1 != null) {
				return 1;
			} else if (cal2 != null) {
				return -1;
			}
			return compareName(name1, name2);
		}
		int i = (int) (cal1.getTimeInMillis() - cal2.getTimeInMillis());
		if (i == 0) {
			return 1;
		}
		return i;
	}

	private int compareName(String name1, String name2) {
		if (name1 == null) {
			if (name2 != null) {
				return -1;
			} else {
				return 1;
			}
		}
		return name1.compareTo(name2);
	}
}
