/*
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
 * $RCSfile: NgGrpcSessionAttr.java,v $ $Revision: 1.1 $ $Date: 2005/04/07 11:12:58 $
 */
package org.apgrid.grpc.ng;

import org.apgrid.grpc.ng.info.RemoteClassPathInfo;

/**
 * Provides strings which is used as key of attribute variables of sessions.
 */
public class NgGrpcSessionAttr {
	/** server name */
	public static final String KEY_SESSION_TIMEOUT =
		RemoteClassPathInfo.KEY_CLASS_PATH_SESSION_TIMEOUT;
}
