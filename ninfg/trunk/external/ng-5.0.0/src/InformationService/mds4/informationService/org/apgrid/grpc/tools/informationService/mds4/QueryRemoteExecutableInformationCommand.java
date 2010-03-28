/*
 * $RCSfile: QueryRemoteExecutableInformationCommand.java,v $
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
import java.util.*;

public class QueryRemoteExecutableInformationCommand extends Command {
    class Attribute {
	private String key;
	private String value;

	Attribute(String key, String value) {
	    this.key = key;
	    this.value = value.trim();
	}

	public String getKey() {
	    return key;
	}

	public String getValue() {
	    return value;
	}
    }

    class AttributeList {
	ArrayList<Attribute> list = new ArrayList<Attribute>();

	void put(String key, String value) {
	    list.add(new Attribute(key, value));
	}

	public String get(String key) {
	    for (Attribute attr : list) {
		if (attr.getKey().equals(key)) {
		    return attr.getValue();
		}
	    }
	    return null;
	}
    }

    private static final String CMD_QUERY_REMOTE_EXECUTABLE_INFORMATION_END =
	"QUERY_REMOTE_EXECUTABLE_INFORMATION_END";
    protected AttributeList list = new AttributeList();

    public QueryRemoteExecutableInformationCommand(
	String[] request, InformationService info) throws Command.Exception {
	super(request, info);
	boolean terminated = false;
	String line;
	while ((line = info.readLine()) != null) {
	    if (line.equals(CMD_QUERY_REMOTE_EXECUTABLE_INFORMATION_END)) {
		terminated = true;
		break;
	    } else {
		String attrs[] = line.split("\\s");
		if (attrs.length < 2) {
		    throw new Command.Exception("Invalid attribute: " + line);
		}
		StringBuilder sb = new StringBuilder();
		for (int i = 1; i < attrs.length; i++) {
			sb.append(attrs[i]);
			sb.append(" ");
		}
		list.put(attrs[0], sb.toString());
	    }
	}

	if (!terminated) {
	    throw new Command.Exception("The command was not terminated!");
	}
    }

    public void handle() throws Command.Exception {
	Query query;
	try {
	    query = new Query(this);
	} catch (Query.Exception e) {
	    throw new Command.Exception(e);
	}
	query.startThread();
	info.registerQuery(query);
	reply("S " + query.getQueryID());
    }

    public String toString() {
	return "QueryRemoteExecutableInformationCommand";
    }

    public void dump(PrintWriter pw) {
	pw.println(toString());
    }
}
