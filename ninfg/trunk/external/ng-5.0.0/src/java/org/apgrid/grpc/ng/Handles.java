/*
 * $RCSfile: Handles.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
 * $AIST_Release: 5.0.0 $
 * $AIST_Copyright:
 *  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
 *  National Institute of Advanced Industrial Science and Technology
 *  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  $
 */
package org.apgrid.grpc.ng;

import java.util.List;
import java.util.ArrayList;

class Handles {

	private List<NgGrpcFunctionHandle> functionHandles;
	private List<NgGrpcObjectHandle> objectHandles;

	public Handles() {
		functionHandles = new ArrayList<NgGrpcFunctionHandle>();
		objectHandles = new ArrayList<NgGrpcObjectHandle>();
	}

	public synchronized void add(NgGrpcFunctionHandle handle) {
		functionHandles.add(handle);
	}

	public synchronized void add(NgGrpcObjectHandle handle) {
		objectHandles.add(handle);
	}

	public int size() {
		return functionHandles.size() + objectHandles.size();
	}

	public NgGrpcFunctionHandle getFunctionHandle(int execId) {
		for (NgGrpcFunctionHandle handle : functionHandles) {
			if (handle.getExecutableID() == execId) {
				return handle;
			}
		}
		return null;
	}

	public NgGrpcObjectHandle getObjectHandle(int execId) {
		for (NgGrpcObjectHandle handle : objectHandles) {
			if (handle.getExecutableID() == execId) {
				return handle;
			}
		}
		return null;
	}

	public synchronized void remove(Object handle) {
		int targetHandleID = ((NgGrpcHandle)handle).getExecutableID();

		boolean is_handle_removed = false;
		is_handle_removed = removeFunctionHandle(targetHandleID);
		if (! is_handle_removed ) {
			// handle is not FunctionHandle.
			// because remove Handle from ObjectHandle list.
			removeObjectHandle(targetHandleID);
		}
	}

	private boolean removeFunctionHandle(int execId) {
		NgGrpcFunctionHandle handle = getFunctionHandle(execId);
		if (handle == null) { return false; }
		functionHandles.remove(handle);
		return true;
	}

	private boolean removeObjectHandle(int execId) {
		NgGrpcObjectHandle handle = getObjectHandle(execId);
		if (handle == null) { return false; }
		objectHandles.remove(handle);
		return true;
	}

}
