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

/**
 * Interface for ACL implementations.
 */
public interface AccessControl {
	/**
	 * @return caller information
	 */
	public CallerInfo getCallerInfomation();

	/**
	 * @param resource a directory
	 * @param callerInfo caller information
	 * @return true if the caller can read(lookup/list) the directory
	 */
	public boolean canRead(RNSResource resource, CallerInfo callerInfo);

	/**
	 * @param resource a directory
	 * @param callerInfo caller information
	 * @return true if the caller can write(add/remove/setMetadata) the directory
	 */
	public boolean canWrite(RNSResource resource, CallerInfo callerInfo);

	/**
	 * @param resource a directory
	 * @param callerInfo caller information
	 * @return true if the caller can modify the ACL of the directory
	 */
	public boolean canModify(RNSResource resource, CallerInfo callerInfo);

	/**
	 * @param resource a directory
	 * @param callerInfo caller information
	 * @return true if the caller is Administrator
	 */
	public boolean isAdmin(RNSResource resource, CallerInfo callerInfo);
}