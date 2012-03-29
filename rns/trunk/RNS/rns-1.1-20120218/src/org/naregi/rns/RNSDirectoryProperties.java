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

/**
 * Structure for RNS Resource(directory) Properties.
 */
public class RNSDirectoryProperties  {
	private Calendar createTime;
	private Calendar accessTime;
	private Calendar modificationTime;

	public Calendar getCreateTime() {
	    return createTime;
	}

	public void setCreateTime(Calendar createTime) {
	    this.createTime = createTime;
	}

	public Calendar getAccessTime() {
	    return accessTime;
	}

	public void setAccessTime(Calendar accessTime) {
	    this.accessTime = accessTime;
	}

	public Calendar getModificationTime() {
	    return modificationTime;
	}

	public void setModificationTime(Calendar modificationTime) {
	    this.modificationTime = modificationTime;
	}
}
