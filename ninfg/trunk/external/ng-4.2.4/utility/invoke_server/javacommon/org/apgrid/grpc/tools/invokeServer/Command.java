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
 * $RCSfile: Command.java,v $ $Revision: 1.1 $ $Date: 2006/11/08 08:35:45 $
 */
package org.apgrid.grpc.tools.invokeServer;
import java.io.*;

public abstract class Command {
    InvokeServer is;

    Command(InvokeServer is) {
        this.is = is;
    }

    void notify(String str){
        is.sendNotify(str);
    }

    void reply(String str){
        is.sendReply(str);
    }

    abstract void handle();

    abstract void dump(PrintWriter pw);


    public static class Exception extends java.lang.Exception{
        public Exception(String str){
            super(str);
        }
    }
}
