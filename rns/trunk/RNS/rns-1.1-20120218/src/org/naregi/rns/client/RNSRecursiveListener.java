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

import org.globus.axis.message.addressing.EndpointReferenceType;

/**
 * Callback interface for recursive operations.
 */
public interface RNSRecursiveListener {

	/**
	 * This method is called at every entry in recursive operations.
	 *
	 * See {@link RNSClient#removeRecursive(String, int, RNSRecursiveListener)}
	 *
	 * @param epr EPR of the entry
	 * @param dirname a pathname of the parent directory
	 * @param basename a name of the entry
	 * @throws RNSError if an error occurs
	 */
	public void action(EndpointReferenceType epr, String dirname,
			String basename) throws RNSError;
}
