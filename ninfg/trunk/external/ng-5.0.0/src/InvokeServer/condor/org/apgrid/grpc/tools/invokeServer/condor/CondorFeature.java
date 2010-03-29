/*
 * $RCSfile: CondorFeature.java,v $ $Revision: 1.4 $ $Date: 2008/03/04 23:45:07 $
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
package org.apgrid.grpc.tools.invokeServer.condor;

import org.apgrid.grpc.tools.invokeServer.*;

import java.util.Iterator;
import java.util.ArrayList;

public class CondorFeature extends TargetFeature {
    private static ArrayList features = new ArrayList();
    static {
	features.add("protocol_version 2.0");
	features.add("feature STAGING_AUTH_NUMBER");
	features.add("request JOB_CREATE");
	features.add("request JOB_STATUS");
	features.add("request JOB_DESTROY");
	features.add("request QUERY_REQUESTS");
	features.add("request EXIT");
    }

    public CondorFeature(QueryFeaturesCommand command) throws Command.Exception{
        super(command);
    }

    protected ArrayList getFeatures() {
	return features;
    }
}