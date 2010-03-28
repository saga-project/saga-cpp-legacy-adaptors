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
 * $RCSfile: Log.java,v $ $Revision: 1.2 $ $Date: 2005/11/01 07:46:58 $
 */
package org.apgrid.grpc.tools.invokeServer.unicore;

import java.util.Calendar;
import java.io.*;

class JobId extends ThreadLocal {

    /**
     * @return Object
     */
    protected synchronized Object initialValue() {
        return "------";
    }

    /**
     * @return String
     */
    public String toString() {
        Object o = get();
        if (o == null) {
            return "------";
        } else {
            return o.toString();
        }
    }
}

public class Log {

  static PrintWriter stream = null;
  static void setStream(PrintWriter _stream){
	stream = _stream;
  }

    static JobId jobId = new JobId();

    /**
     * date and time header
     * @return String
     */
    private static String header() {
        
        Calendar cal = Calendar.getInstance();
        int mon = cal.get(Calendar.MONTH);
        int day = cal.get(Calendar.DATE);
        int hour = cal.get(Calendar.HOUR_OF_DAY);
        int min = cal.get(Calendar.MINUTE);
        int sec = cal.get(Calendar.SECOND);
        String daytime =
            mon + 1 + "/" + day + " " + hour + ":" + min + ":" + sec;
        String idStr = " [" + jobId.toString() + "]: ";
        return daytime + idStr;
    }

    /**
     * logging string
     * @param str
     */
    static void log(String str) {
        stream.println(header() + str);
    }

    /**
     * logging object name
     * @param obj
     */
    static void log(Object obj) {
        log(obj.toString());
    }

    /**
     * print stack tarce
     * @param e
     */
    static void printStackTrace(Throwable e) {
        stream.print(header());
        e.printStackTrace(stream);
    }

    /**
     * set object
     * @param obj
     */
    static void set(Object obj) {
        jobId.set(obj);
    }
}







