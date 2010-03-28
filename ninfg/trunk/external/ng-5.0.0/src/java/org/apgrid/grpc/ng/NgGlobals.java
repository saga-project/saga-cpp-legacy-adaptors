/*
 * $RCSfile: NgGlobals.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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
import java.util.Vector;

public class NgGlobals {
	// buffer sizes 
	public static final int smallBufferSize = 32;
	public static final int argBufferSize = 5120;
	public static final int fileBufferSize = 5120;
	public static final int paramInputBufferSize = 5120;
	public static final int zlibInputBufferSize = 20 * 1024;
	public static final int debugIndent = 2;
	
	// information to manage NgGrpcClient
	private static int lastContextID = 1;
	private static List listContext = new Vector();

	/**
	 * @return The number (0 ~ Integer.MAX_VALUE)
	 */
	protected synchronized static int generateContextID() {
		int next = lastContextID;
		lastContextID++;
		if (next == Integer.MAX_VALUE) {
			lastContextID = 0;
		}
		return next;
	}
	
	/**
	 * @param contextID
	 * @return
	 */
	protected static NgGrpcClient getContext(int contextID) {
		for (int i = 0; i < listContext.size(); i++) {
			NgGrpcClient context = (NgGrpcClient) listContext.get(i);
			if (context.getID() == contextID) {
				return (NgGrpcClient) listContext.get(i);
			}
		}
		return null;
	}
	
	/**
	 * @param context
	 */
	protected static void putContext(NgGrpcClient context) {
		listContext.add(context);
	}
}
