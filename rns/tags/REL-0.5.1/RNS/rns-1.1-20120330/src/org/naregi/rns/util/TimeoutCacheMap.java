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
package org.naregi.rns.util;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.map.ReferenceMap;

/**
 * A Map implementation whose each entry has a lifetime. The stored entry may be
 * removed by the garbage collector before timeout.
 */
public class TimeoutCacheMap<K, V> implements Map<K, V> {

	private long timeout;

	private Map<K, MyObject> map;

	private class MyObject {
		private V obj;
		private long timestamp;

		MyObject(V obj) {
			this.obj = obj;
			this.timestamp = System.currentTimeMillis();
		}
	}

	private class MyEntry<K2, V2> implements Entry<K2, V2> {
		private final K2 key;
		private V2 value;

		MyEntry(K2 key, V2 value) {
			this.key = key;
			this.value = value;
		}

		public K2 getKey() {
			return key;
		}

		public V2 getValue() {
			return value;
		}

		public V2 setValue(V2 value) {
			this.value = value;
			return this.value;
		}
	}

	private boolean isTimeout(Object key, MyObject o) {
		if (System.currentTimeMillis() - o.timestamp >= timeout) {
			if (key != null) {
				map.remove(key);
			}
			return true; /* timeout */
		} else {
			return false;
		}
	}

	/**
	 * This call is equivalent to that of calling
	 * {@link TimeoutCacheMap#TimeoutCacheMap(long, boolean)
	 * TimeoutCacheMap(timeout, true)}
	 */
	public TimeoutCacheMap(long timeout) {
		this(timeout, true);
	}

	/**
	 * Initialize TimeoutCacheMap that will use the specified timeout.
	 *
	 * @param timeout a lifetime of each entry (millisecond)
	 * @param isSynchronized true if this map will be synchronized.
	 */
	@SuppressWarnings("unchecked")
	public TimeoutCacheMap(long timeout, boolean isSynchronized) {
		this.timeout = timeout;
		map = (Map<K, MyObject>) new ReferenceMap(ReferenceMap.SOFT,
				ReferenceMap.SOFT, 512, 0.75F, false);
		if (isSynchronized) {
			map = Collections.synchronizedMap(map);
		}
	}

	@Override
	public void clear() {
		map.clear();
	}

	@Override
	public boolean containsKey(Object key) {
		if (get(key) == null) {
			return false;
		}
		return map.containsKey(key);
	}

	@Override
	public boolean containsValue(Object value) {
		if (map.values() == null) {
			return false;
		}
		for (MyObject mo : map.values()) {
			if (mo == value || mo.equals(value)) {
				if (isTimeout(null, mo)) {
					/* cannot remove */
					return false;
				} else {
					return true;
				}
			}
		}
		return false;
	}

	@Override
	public Set<java.util.Map.Entry<K, V>> entrySet() {
		Set<Entry<K, V>> newSet = new HashSet<Entry<K, V>>();
		Set<Entry<K, MyObject>> oldSet = map.entrySet();

		for (Entry<K, MyObject> entry : oldSet) {
			MyObject mo = entry.getValue();
			if (isTimeout(entry.getKey(), mo) == false) {
				newSet.add(new MyEntry<K, V>(entry.getKey(), mo.obj));
			}
		}
		return newSet;
	}

	@Override
	public V get(Object key) {
		MyObject o = map.get(key);
		if (o == null) {
			return null;
		}
		if (isTimeout(key, o)) {
			return null;
		}
		return o.obj;
	}

	@Override
	public boolean isEmpty() {
		return map.isEmpty();
	}

	@Override
	public Set<K> keySet() {
		return map.keySet();
	}

	@Override
	public V put(K arg0, V arg1) {
		MyObject o = map.put(arg0, new MyObject(arg1));
		if (o == null) {
			return null;
		}
		return o.obj;
	}

	@Override
	public void putAll(Map<? extends K, ? extends V> arg0) {
		for (Entry<? extends K, ? extends V> entry : arg0.entrySet()) {
			V o = entry.getValue();
			if (o != null) {
				map.put(entry.getKey(), new MyObject(o));
			}
		}
	}

	@Override
	public V remove(Object key) {
		MyObject mo = map.remove(key);
		if (mo == null) {
			return null;
		}
		return mo.obj;
	}

	@Override
	public int size() {
		return map.size();
	}

	@Override
	public Collection<V> values() {
		List<V> list = new ArrayList<V>();
		for (MyObject mo : map.values()) {
			if (isTimeout(null, mo) == false) {
				list.add(mo.obj);
			}
			/* cannot remove */
		}
		return list;
	}

}
