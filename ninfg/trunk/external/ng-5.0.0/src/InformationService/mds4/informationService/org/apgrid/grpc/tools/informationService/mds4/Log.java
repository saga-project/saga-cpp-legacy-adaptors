/*
 * $RCSfile: Log.java,v $ $Revision: 1.3 $ $Date: 2008/02/07 08:17:39 $
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

import java.util.Calendar;
import java.io.*;
import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;
import java.util.regex.*;
import java.lang.IllegalArgumentException;
import java.lang.NullPointerException;

class JobId extends ThreadLocal<String> {

    /**
     * @return String
     */
    protected synchronized String initialValue() {
        return "------";
    }

    /**
     * @return String
     */
    public String toString() {
        String s = get();
        if (s == null) {
            return "------";
        } else {
            return s;
        }
    }
}


class WriterOutputStream extends OutputStream{
    Writer writer;

    public WriterOutputStream(Writer writer){
        this.writer = writer;
    }

    public void close(){
    }
    public void flush() throws IOException {
        writer.flush();
    }
    public void write(byte[] b)  throws IOException{
        writer.write(new String(b));
    }
    public void write(byte[] b, int off, int len)  throws IOException{
        writer.write(new String(b, off, len));
    }
    public void write(int b)  throws IOException{
        writer.write(b);
    }
}

class NullWriter extends Writer {
  public void close() {}
  public void flush() {}
  public void write(char [] cbuf, int off, int len) {}
}


public class Log {

    public static class Flag {
        String string ;
        public Flag(String str) {
            if (str == null) {
                throw new NullPointerException();
            }
            string = str;
        }
        public boolean equals(Object o) {
            return (o instanceof Flag) &&
                   ((o == this) ||
                    ((Flag)o).string.equals(this.string));
        }
        public int hashCode() {
            return this.string.hashCode();
        }
    }
    private static Set<Flag> categorySet = new HashSet<Flag>();
    private static boolean outputAll = true;
    public static final Flag ALWAYS = null;
    public static final Flag IS_COMMAND = new Flag("IS_COMMAND");

    static {
        categorySet.add(ALWAYS);
    }

    static boolean flagsSet = false;
    static void setFlags(String str, Flag[] availableFlagsSet) throws IllegalArgumentException {
        final Pattern p = Pattern.compile("\\G[\\s]*([^\\s]+)[\\s]*");
        boolean allFlag = false;
        Matcher m = p.matcher(str);
        String cat;

        if (flagsSet) {
            /* Do nothing */
            return;
        }
        flagsSet = true;

        if (str != null && !Pattern.matches("\\s*", str)) {
            /* Create log flags set */
            Set<Flag> flags = new HashSet<Flag>();

            flags.add(Log.ALWAYS);
            flags.add(Log.IS_COMMAND);
            for (int i = 0;i < availableFlagsSet.length;++i) {
                flags.add(availableFlagsSet[i]);
            }

            /* Parse */
            do {
                if (!m.find()) {
                    throw new IllegalArgumentException(str);
                }

                cat = m.group(1);
                if (cat.equals("ALL")) {
                    allFlag= true;
                } else {
                    Flag f = new Flag(cat);
                    categorySet.add(f);
                    if (!flags.contains(f)) {
                        Log.log(Log.ALWAYS, "Warning: \"" + cat + "\" is unknown log flag.");
                    }
                }
            } while(m.end() != str.length());
        }

        outputAll = allFlag;
    }

    static PrintWriter stream = new PrintWriter(new NullWriter());
    static void setStream(PrintWriter _stream){
        stream = _stream;
    }

    static PrintStream getPrintStream(){
        return new PrintStream(new WriterOutputStream(stream));
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
    static public void log(String str) {
        log(ALWAYS, str);
    }

    /**
     * logging string
     * @param str
     */
    static public void log(Object obj) {
        log(ALWAYS, obj);
    }

    /**
     * logging string
     * @param cat 
     * @param str
     */
    static public void log(Flag cat, String str) {
        if (outputAll || categorySet.contains(cat)) {
            stream.println(header() + str);
            stream.flush();
        }
    }

    /**
     * logging object name
     * @param cat 
     * @param obj
     */
    static public void log(Flag cat, Object obj) {
        log(cat, obj.toString());
    }

    /**
     * print stack trace
     * @param e
     */
    static public void printStackTrace(Throwable e) {
        /* Print always */
        stream.print(header());
        e.printStackTrace(stream);
        stream.flush();
    }

    /**
     * set string
     * @param str
     */
    static public void set(String str) {
        jobId.set(str);
    }
}
