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
 * $RCSfile: NaregiSSJobFactory.java,v $ $Revision: 1.3 $ $Date: 2006/10/11 08:13:50 $
 */
package org.apgrid.grpc.tools.invokeServer.naregiSS;
import  org.apgrid.grpc.tools.invokeServer.*;

public class NaregiSSJobFactory implements TargetJobFactory {
	public TargetJob create(CreateCommand command) throws Command.Exception{
		return new NaregiSSJob(command);
	}
}
