/*
 * $RCSfile: QueryFeaturesCommand.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:05 $
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
package org.apgrid.grpc.tools.invokeServer;
import java.io.*;
import java.util.Iterator;
import java.util.ArrayList;

public class QueryFeaturesCommand extends Command{
    TargetFeature feature;

    QueryFeaturesCommand(InvokeServer is) throws Command.Exception {
        super(is);
	feature = createTargetFeature();
    }

    void handle(){
        reply("SM");
	ArrayList list = feature.getFeatures();
	for (Iterator i = list.iterator(); i.hasNext();) {
	    String s = (String)i.next();
	    reply(s);
	}
	reply("REPLY_END");
    }

    private TargetFeature createTargetFeature() throws Command.Exception {
        String factoryProperty = "org.apgrid.grpc.tools.invokeServer.FactoryClassName";
        String factoryName  = System.getProperty(factoryProperty);
        if (factoryName == null) 
            throw new Command.Exception("Cannot find factory property " + factoryProperty);
        try {
            Class factoryClass = Class.forName(factoryName);
            TargetJobFactory jobFactory = (TargetJobFactory)factoryClass.newInstance();
            return jobFactory.createFeature(this);
        } catch (ClassNotFoundException e){
            Log.printStackTrace(e);
            throw new Command.Exception("Cannot create factory");
        } catch (IllegalAccessException  e){
            Log.printStackTrace(e);
            throw new Command.Exception("Cannot create factory");
        } catch (InstantiationException  e){
            Log.printStackTrace(e);
            throw new Command.Exception("Cannot create factory");
        }
    }

    public String toString() {
        return "QueryFeaturesCommand";
    }

    void dump(PrintWriter pw){
        pw.println("QueryFeaturesCommand");
    }
}
