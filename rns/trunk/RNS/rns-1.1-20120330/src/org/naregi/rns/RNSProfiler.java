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
package org.naregi.rns;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.axis.types.UnsignedInt;
import org.apache.axis.types.UnsignedLong;
import org.naregi.rns.stubs.ProfileType;

/**
 * Measure the processing time of server operations.
 */
public class RNSProfiler {

	private static final String LS = System.getProperty("line.separator");

	private static boolean enable = false;

	public static void enable() {
		enable = true;
		reset();
	}

	public static void disable() {
		enable = false;
	}

	public static boolean isEnable() {
		return enable;
	}

	private long total;
	private int count;

	private RNSProfiler() {
		count = 0;
		total = 0;
	}

	/**
	 * Types of observation point.
	 */
	public static enum TYPE {
		Total_Add, Total_Bulk_Add, Total_Lookup, Total_Bulk_Lookup, Total_List,
		Total_ListIterator, Total_Remove, Total_Bulk_Remove, Total_Destroy,
		Total_Rename, Total_Bulk_Rename, Total_SetMetadata,
		Total_Bulk_SetMetadata, Total_Search, Total_Bulk_Search,
		Total_SearchIterator, Add_SetACL_DB, Add_InsertDB, Add_ModifyMtimeDB,
		Add_CommitDB, Search_GetRNSEntryFromDB, Search_Serialize,
		Search_RunXQuery_First, Search_RunXQuery_Reuse, Search_Deserialize,
		IntentionalGC
	};

	private static Map<TYPE, RNSProfiler> typeMap = new HashMap<TYPE, RNSProfiler>();

	static {
		for (TYPE type : TYPE.values()) {
			typeMap.put(type, new RNSProfiler());
		}
	}

	public static long start() {
		if (enable == false) {
			return 0;
		}
		return System.nanoTime();
	}

	public static void stop(TYPE type, long startTime) {
		if (enable == false) {
			return;
		}
		long tmp = System.nanoTime() - startTime;
		if (tmp >= 0) {
			RNSProfiler pf = typeMap.get(type);
			synchronized (pf) {
				pf.total += tmp;
				pf.count++;
			}
		} else {
			System.out.println("negative nanoTime : " + tmp);
		}
	}

	public static void reset() {
		if (enable == false) {
			return;
		}
		for (RNSProfiler pf : typeMap.values()) {
			synchronized (pf) {
				pf.count = 0;
				pf.total = 0;
			}
		}
	}

	private static String format(TYPE type, long total, int count) {
		return String.format("%7d", (total / 1000000))
				+ "ms ("
				+ String.format("%8.3f", (double) total / (double) count
						/ 1000000) + "ms * " + String.format("%7d", count)
				+ ") : " + type.toString();
	}

	public static String resultString() {
		StringBuilder sb = new StringBuilder();
		for (TYPE type : TYPE.values()) {
			RNSProfiler pf = typeMap.get(type);
			synchronized (pf) {
				sb.append(format(type, pf.total, pf.count) + LS);
			}
		}
		return sb.toString();
	}

	public static void printResult() {
		System.out.println(resultString());
	}

	public static List<ProfileType> getResultList() {
		List<ProfileType> al = new ArrayList<ProfileType>();
		for (TYPE type : TYPE.values()) {
			ProfileType pt = new ProfileType();
			pt.setName(type.toString());
			RNSProfiler pf = typeMap.get(type);
			synchronized (pf) {
				pt.setTotal(new UnsignedLong(pf.total));
				pt.setCount(new UnsignedInt(pf.count));
			}
			al.add(pt);
		}
		return al;
	}
}
