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
 * Iterator interface for List and Search operations.
 */
public interface RNSIterator<E> extends Iterator<E> {

	/**
	 * Return the RNSError which was occurred in Iterator#next() or
	 * Iterator#hasNext() methods.
	 *
	 * @return null if no error occurs
	 */
	public RNSError getError();
}
