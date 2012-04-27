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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * RNS server logger.
 */
public class RNSLog {
	private static Log logger = null;

	private RNSLog() {
	}

	public static Log getLog() {
		if (logger != null) {
			return logger;
		}
		logger = LogFactory.getLog("org.naregi.rns.RNS");
		logger.info("start RNS");
		return logger;
	}
}
