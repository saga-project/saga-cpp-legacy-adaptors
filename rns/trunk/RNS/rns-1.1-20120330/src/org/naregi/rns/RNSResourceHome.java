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

import org.globus.wsrf.NoSuchResourceException;
import org.globus.wsrf.ResourceKey;
import org.globus.wsrf.impl.ResourceHomeImpl;
import org.globus.wsrf.impl.SimpleResourceKey;

/**
 * This implementation discovers and removes RNS resources(directories).
 */
public class RNSResourceHome extends ResourceHomeImpl {

	public ResourceKey createNewDir() throws Exception {
		RNSResource r = (RNSResource) createNewInstance();
		r.initializeNewDir();
		ResourceKey key = new SimpleResourceKey(keyTypeName, r.getID());
		add(key, r);
		return key;
	}

	public ResourceKey getRootDirKey() throws Exception {
		ResourceKey key = new SimpleResourceKey(keyTypeName,
				RNSConfig.getRootID());
		RNSResource r = null;
		try {
			r = (RNSResource) find(key);
			if (r != null) {
				return key;
			}
		} catch (NoSuchResourceException e) {
			/* create new ROOT resource */
		} catch (Exception e) {
			throw e;
		}
		r = (RNSResource) createNewInstance();
		r.initializeNewRootDir();
		add(key, r);
		return key;
	}

	public ResourceKey idToResourceKey(String id) {
		return new SimpleResourceKey(keyTypeName, id);
	}
}
