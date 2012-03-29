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
package org.naregi.rns;

import java.util.List;
import java.util.Map;
import java.util.Random;

import org.globus.wsrf.impl.ResourceHomeImpl;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.util.TimeoutSessionMap;

/**
 * This implementation discovers and removes Search session.
 */
public class SearchIteratorResourceHome extends ResourceHomeImpl {

	/* 1 min. */
	private static Map<String, SearchSession> session = new TimeoutSessionMap<String, SearchSession>(
			60 * 1000);

	public static List<String> getSessionResults(String sessionID) {
		SearchSession s = session.get(sessionID);
		if (s == null) {
			return null;
		}
		return s.getResults();
	}

	private static Random rand = new Random();

	public static String setSessionResults(List<String> results) {
		SearchSession s = new SearchSession();
		s.setResults(results);
		String key = s.hashCode() + "_" + (rand.nextInt() >>> 1);
		session.put(key, s);
		return key; /* sessionID */
	}

	public static void removeSession(String sessionID) {
		session.remove(sessionID);
		RNSUtil.gc();
		System.out.println("[RNS] Remove Search Iterator session: " + sessionID);
	}
}
