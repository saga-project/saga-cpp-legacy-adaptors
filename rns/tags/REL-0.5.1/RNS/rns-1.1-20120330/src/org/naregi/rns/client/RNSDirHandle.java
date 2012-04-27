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

import java.util.Iterator;

/**
 * Handle to get an entry.
 */
public class RNSDirHandle implements Iterable<RNSDirent> {
	private RNSIterator<RNSDirent> iterator;

	/**
	 * Iterable for RNSDirent entries with getting error method.
	 *
	 * @param iterator RNSIterator
	 */
	public RNSDirHandle(RNSIterator<RNSDirent> iterator){
		this.iterator = iterator;
	}

	@Override
	public Iterator<RNSDirent> iterator() {
		return iterator;
	}

	/**
	 * Get an Error which was occurred in Iterator#next().
	 *
	 * @return null if no error occurred
	 */
	public RNSError getError() {
		return iterator.getError();
	}
}
