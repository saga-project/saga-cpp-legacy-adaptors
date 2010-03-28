/*
 * $RCSfile: QueryFeaturesCommand.java,v $
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
package org.apgrid.grpc.tools.informationService.mds4;

import java.io.*;

public class QueryFeaturesCommand extends Command {
    public QueryFeaturesCommand(String[] request, InformationService info) {
	super(request, info);
    }

    public void handle() {
	reply("SM");
	reply(info.PROTOCOL_VERSION);
	reply("request " + info.CMD_QUERY_FEATURES);
	reply("request " + info.CMD_QUERY_REMOTE_EXECUTABLE_INFORMATION);
	reply("request " + info.CMD_CANCEL_QUERY);
	reply("request " + info.CMD_EXIT);
	reply("REPLY_END");
    }

    public String toString() {
	return "QueryFeaturesCommand";
    }

    public void dump(PrintWriter pw) {
	pw.println(toString());
    }
}
