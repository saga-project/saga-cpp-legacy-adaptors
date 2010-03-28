/**
 * $AIST_Release: 4.2.4 $
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
 * $RCSfile: NgInvokeCallback.java,v $ $Revision: 1.3 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng;

import org.gridforum.gridrpc.GrpcException;

class NgInvokeCallback implements Runnable {
	NgGrpcClient context;
	NgGrpcHandle handle;
	Protocol protInvokeCallback;
	
	/**
	 * @param handle
	 * @param protInvokeCallback
	 */
	NgInvokeCallback(NgGrpcClient context,
		NgGrpcHandle handle, Protocol protInvokeCallback) {
		this.context = context;
		this.handle = handle;
		this.protInvokeCallback = protInvokeCallback;
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		try {
			handle.invokeCallback(protInvokeCallback);
		} catch (GrpcException e) {
			try {
				context.getNgLog().printLog(
					NgLog.LOGCATEGORY_NINFG_GRPC,
					NgLog.LOGLEVEL_FATAL,
					e);
			} catch (GrpcException e1) {
				/* can't manage */
			}
		}
	}
}
