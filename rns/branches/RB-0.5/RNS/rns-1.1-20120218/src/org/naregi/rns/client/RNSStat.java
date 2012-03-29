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

import java.util.Calendar;

import org.apache.axis.types.UnsignedLong;

/**
 * Structure for RNS directory properties.
 */
public class RNSStat {
	private UnsignedLong elementCount;
	private Calendar createTime;
	private Calendar accessTime;
	private Calendar modificationTime;
	private boolean readable;
	private boolean writable;

	@Override
	public String toString() {
		return "elementCount=" + elementCount.toString() + ", createTime="
				+ createTime.getTimeInMillis();
	}

	/**
	 * Get the total number of elements.
	 *
	 * @return the total number of elements
	 */
	public UnsignedLong getElementCount() {
		return elementCount;
	}

	/**
	 * Set a total number of elements.
	 *
	 * @param elementCount a total number of elements
	 */
	public void setElementCount(UnsignedLong elementCount) {
		this.elementCount = elementCount;
	}

	/**
	 * Get the time at which the RNS directory was created.
	 *
	 * @return the time at which the RNS directory was created
	 */
	public Calendar getCreateTime() {
		return createTime;
	}

	/**
	 * Set a time at which the RNS directory is created
	 *
	 * @param createTime a time at which the RNS directory is created
	 */
	public void setCreateTime(Calendar createTime) {
		this.createTime = createTime;
	}

	/**
	 * Get the time at which the RNS directory was last accessed.
	 *
	 * @return the time at which the RNS directory was last accessed
	 */
	public Calendar getAccessTime() {
		return accessTime;
	}

	/**
	 * Set a time at which the RNS directory is last accessed.
	 *
	 * @param accessTime a time at which the RNS directory is last accessed
	 */
	public void setAccessTime(Calendar accessTime) {
		this.accessTime = accessTime;
	}

	/**
	 * Get the time at which the RNS directory was last modified.
	 *
	 * @return the time at which the RNS directory was last modified
	 */
	public Calendar getModificationTime() {
		return modificationTime;
	}

	/**
	 * Set a time at which the RNS directory is last modified.
	 *
	 * @param modificationTime a time at which the RNS directory is last modified
	 */
	public void setModificationTime(Calendar modificationTime) {
		this.modificationTime = modificationTime;
	}

	/**
	 * Check whether the directory is readable.
	 *
	 * @return true if the directory is readable
	 */
	public boolean isReadable() {
		return readable;
	}

	/**
	 * Set a flag of whether the directory is readable.
	 *
	 * @param readable true if the directory is readable
	 */
	public void setReadable(boolean readable) {
		this.readable = readable;
	}

	/**
	 * Check whether the directory is writable.
	 *
	 * @return true if the directory is writable
	 */

	public boolean isWritable() {
		return writable;
	}

	/**
	 * Set a flag of whether the directory is writable.
	 *
	 * @param writable true if the directory is writable
	 */
	public void setWritable(boolean writable) {
		this.writable = writable;
	}
}
