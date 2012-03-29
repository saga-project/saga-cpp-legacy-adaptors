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

import java.util.Calendar;
import java.util.List;

import org.globus.wsrf.InvalidResourceKeyException;
import org.globus.wsrf.NoSuchResourceException;
import org.globus.wsrf.PersistenceCallback;
import org.globus.wsrf.RemoveCallback;
import org.globus.wsrf.Resource;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.ResourceIdentifier;
import org.globus.wsrf.ResourceKey;
import org.globus.wsrf.ResourceLifetime;
import org.globus.wsrf.ResourceProperties;
import org.globus.wsrf.ResourceProperty;
import org.globus.wsrf.ResourcePropertySet;
import org.globus.wsrf.TerminationTimeRejectedException;
import org.globus.wsrf.impl.ReflectionResourceProperty;
import org.globus.wsrf.impl.SimpleResourcePropertySet;

/**
 * Search session (Store status of a client temporally for Search operation.)
 */
public class SearchIteratorResource implements Resource, ResourceIdentifier,
		PersistenceCallback, RemoveCallback, ResourceProperties,
		ResourceLifetime {

	private String id;
	private ResourcePropertySet propSet = null;
	private List<String> results;

	private Calendar terminationTime;
	{
		extendTerminationTime();
	}

	private void extendTerminationTime() {
		terminationTime = Calendar.getInstance();
		terminationTime.add(Calendar.SECOND, 60); /* 60sec. */
	}

	public List<String> getSessionResults() {
		extendTerminationTime();
		return results;
	}

	@Override
	public Object getID() {
		return id;
	}

	@Override
	public synchronized void load(ResourceKey resourcekey)
			throws ResourceException, NoSuchResourceException,
			InvalidResourceKeyException {
		String idStr = (String) resourcekey.getValue();
		if (idStr != null) {
			System.out.println("[RNS] Load Search Iterator session: " + idStr);
			id = idStr;
			results = SearchIteratorResourceHome.getSessionResults(id);
			if (results == null) {
				throw new NoSuchResourceException(
						"Search session error: no resource, or timeout, or purged results because of no memory");
			}
		} else {
			throw new NoSuchResourceException("invalid ResourceKey");
		}
	}

	@Override
	public void store() throws ResourceException {
		/* unused */
	}

	@Override
	public void remove() throws ResourceException {
		SearchIteratorResourceHome.removeSession(id);
	}

	@Override
	public Calendar getCurrentTime() {
		return Calendar.getInstance();
	}

	@Override
	public Calendar getTerminationTime() {
		return terminationTime;
	}

	@Override
	public void setTerminationTime(Calendar calendar)
			throws TerminationTimeRejectedException {
		terminationTime = calendar;
	}

	@Override
	public ResourcePropertySet getResourcePropertySet() {
		if (propSet != null) {
			return propSet;
		}

		propSet = new SimpleResourcePropertySet(
				RNSQNames.ITERATOR_RESOURCE_PROPERTIES);
		ResourceProperty rp;
		try {
			/* WS-Iterator Resource Properties */
			rp = new ReflectionResourceProperty(
					RNSQNames.ITERATOR_RP_ELEMENTCOUNT, this);
			propSet.add(rp);
			rp = new ReflectionResourceProperty(
					RNSQNames.ITERATOR_RP_PREFERREDBLOCKSIZE, this);
			propSet.add(rp);
		} catch (Exception e) {
			propSet = null;
			throw new RuntimeException(e.getMessage());
		}
		return propSet;
	}

	/* ResourceProperties Getter/Setter */
	public long getElementCount() throws ResourceException {
		try {
			return results.size();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public final void setElementCount(long value) {
		/* unused */
	}

	public int getPreferredBlockSize() {
		int i = RNSConfig.getIteratorUnit();
		if (i > 0) {
			return i;
		}
		return 5;
	}

	public void setPreferredBlockSize(int size) {
		/* unused */
	}
}
