/*
 * $RCSfile: Command.java,v $
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

public abstract class Command {
    String[] request;
    InformationService info;

    public Command(String[] request, InformationService info) {
	this.request = request;
	this.info = info;
    }

    protected void reply(String str) {
	info.sendReply(str);
    }

    private void notify(String[] str) {
	info.sendNotify(str);
    }

    public abstract void handle() throws Command.Exception;

    public InformationService getInformationService() {
	return info;
    }

    public abstract void dump(PrintWriter pw);

    public static class Exception extends java.lang.Exception {
	public Exception(String str) {
	    super(str);
	}

	public Exception(java.lang.Exception e) {
	    super(e);
	}
    }
}
