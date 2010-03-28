/*
 * $RCSfile: NaregiSSFeature.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:05 $
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
package org.apgrid.grpc.tools.invokeServer.naregiSS;

import org.apgrid.grpc.tools.invokeServer.*;

import java.util.Iterator;
import java.util.ArrayList;

public class NaregiSSFeature extends TargetFeature {
    private static ArrayList features = new ArrayList();
    static {
	features.add("protocol_version 2.0");
	features.add("request JOB_CREATE");
	features.add("request JOB_STATUS");
	features.add("request JOB_DESTROY");
	features.add("request QUERY_REQUESTS");
	features.add("request EXIT");
    }

    public NaregiSSFeature(QueryFeaturesCommand command) throws Command.Exception{
        super(command);
    }

    protected ArrayList getFeatures() {
	return features;
    }
}
