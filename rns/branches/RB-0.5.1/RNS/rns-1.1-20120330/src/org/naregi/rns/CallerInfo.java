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

import java.util.List;

/**
 * Interface for caller informations.
 */
public interface CallerInfo {
	/**
	 * @return true if the caller is Administrator
	 */
	public boolean isAdmin();

	/**
	 * @return the caller name (not null)
	 */
	public String getUserName();

	/**
	 * @return main group the caller belong to (not null)
	 */
	public String getMainGroup();

	/**
	 * @return groups the caller belong to (null is no group)
	 */
	public List<String> getGroupList();
}
