/*
 * $RCSfile: CancelQueryCommand.java,v $
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

public class CancelQueryCommand extends Command {
    int queryID;

    public CancelQueryCommand(String[] request, InformationService info) throws Command.Exception {
	super(request, info);
	if (request.length != 2) {
	    throw new Command.Exception("Invalid request: " + request);
	}
	try {
	    queryID = Integer.parseInt(request[1]);
	} catch (NumberFormatException e) {
	    throw new Command.Exception(e.getMessage());
	}
    }

    public void handle() {
	info.getQuery(queryID).setCanceled();
	reply("S");
    }

    public String toString() {
	return "CancelQueryCommand";
    }

    public void dump(PrintWriter pw) {
	pw.println(toString());
    }
}
