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
 * $RCSfile: InvokeServer.java,v $ $Revision: 1.10 $ $Date: 2008/03/28 06:36:50 $
 */

package org.apgrid.grpc.tools.invokeServer;

import java.io.*;
import java.util.*;

public class InvokeServer{
    private static final String CMD_CREATE     = "JOB_CREATE";
    private static final String CMD_STATUS     = "JOB_STATUS";
    private static final String CMD_DESTROY    = "JOB_DESTROY";
    private static final String CMD_EXIT       = "EXIT";

    private static final String lineSeparator  = new String(new char[]{0x0d, 0x0a});
    private static final String DEFAULT_LOG_FILE = "/tmp/generic.is.log";

  
    static long pollingInterval = 60000; // 1 min

    private PrintStream outStream;
    private PrintStream errStream;

    Timer timer = new Timer();
    static LineNumberReader lnr = new LineNumberReader(new InputStreamReader(System.in));

    /*
     * Main loop: reads commands from Ninf-G Client and handles them.
     */
    private void start(){
        Command cmd;

        for (;;) {
            try {
                cmd = readCommand();
                if (cmd == null) {
                    /* EOF */
                    break;
                }
                Log.log(Log.IS_COMMAND, cmd);
                cmd.handle();
            } catch (Command.Exception e){
                sendReply("F " + removeCR(e.getMessage()));
            }
        }

        /* Wait that all Jobs have done. */
        synchronized(this) {
            while (!map.isEmpty()) {
                try {
                    this.wait();
                } catch (java.lang.InterruptedException e) {
                }
            }
        }

        /* Kill timer thread immediately.
         * If "timer.cancel()" is not called,
         * timer thread doesn't finish for a long time.
         */
        timer.cancel();

        return;
    }


    static private String removeCR(String msg){
        return msg.replace('\n', '\t');
    }

    private Command readCommand() throws Command.Exception{
        String line = readLine();
        if (line == null) {
            Log.log(Log.ALWAYS,
                 "Unexpected client death! Cancelling all running jobs");
            cancelAll();

            return null;
        }
        StringTokenizer st = new StringTokenizer(line);
        String id = null;
        try {
            id = st.nextToken(); 
        } catch (NoSuchElementException e){
            throw new Command.Exception("No token in line: " + line);
        }
        if (id.equals(CMD_EXIT))
            return new ExitCommand(this);
        String next = null;
        try {
            next = st.nextToken();
        } catch (NoSuchElementException e){
            throw new Command.Exception("Can't get argument of command:" + line);
        }
        //assert next != null: next;

        if (id.equals(CMD_CREATE))
            return new CreateCommand(this, next);
        if (id.equals(CMD_STATUS))
            return new StatusCommand(this, next);
        if (id.equals(CMD_DESTROY))
            return new DestroyCommand(this, next);
    
        throw new Command.Exception("Unkown Command:" + line);
        /* NOTREACHED */
    }


    /** 
     * table to store target job
     */
    HashMap map = new HashMap();

    synchronized void removeJob(TargetJob job){
        job.cancel();  // stop the timer task
        Log.log(Log.ALWAYS, "Removes Job[" + job.jobId + "]");
        map.remove(job.jobId);
        if (map.isEmpty()) {
            this.notify();
        }
    }

    synchronized void registerJob(TargetJob job){
        long pollingTime = job.getStatusPolling() * 1000L;
        if (pollingTime <= 0) {
            pollingTime = pollingInterval;
        }

        timer.schedule(job, pollingTime, pollingTime);
        map.put(job.jobId, job);
    }

    synchronized TargetJob getJob(String jobId){
        TargetJob job = (TargetJob)map.get(jobId);
        return job;
    }
  
    /** 
     *   read one line from stdin and return 
     */ 
    String readLine() {
        try {
            String str = lnr.readLine();
            Log.log(Log.IS_COMMAND, "--> " + str);
            return str; 
        } catch (IOException e) {
            Log.printStackTrace(e);
        }
        return null;
    }

    static void abort(String str){
        System.err.println(str);
        Log.log(Log.ALWAYS, str);
        throw new RuntimeException();
    }

    /** 
     *    output the string to the stdout stream  
     */
    synchronized void sendReply(String str){
        Log.log(Log.IS_COMMAND, "REPLY: " + str);
        outStream.print(str);
        outStream.print(lineSeparator);
        outStream.flush();
      }

    /** 
     *    output the string to the stderr stream 
     */
    synchronized void sendNotify(String str){
        Log.log(Log.IS_COMMAND, "NOTIFY: " + removeCR(str));
        errStream.print(removeCR(str));
        errStream.print(lineSeparator);
        errStream.flush();
    }

    void cancelAll(){
        for (Iterator i = map.values().iterator(); i.hasNext(); ) {
            TargetJob job = (TargetJob)i.next();
            job.destroy();
        }
    }

    static boolean checkFactory(){
        String factoryProperty = "org.apgrid.grpc.tools.invokeServer.FactoryClassName";
        String factoryName  = System.getProperty(factoryProperty);
        if (factoryName == null) {
            Log.log(Log.ALWAYS, "Cannot find factory property " + factoryProperty);
            return false;
        }
        try {
            Class factoryClass = Class.forName(factoryName);
        } catch (ClassNotFoundException e){
            Log.printStackTrace(e);
            return false;
        }
        return true;
    }

    InvokeServer(){
        outStream = System.out;
        errStream = System.err;

        System.setOut(Log.getPrintStream());
        System.setErr(Log.getPrintStream());
    }

    public static void main(String [] args) {
        try {
            String logfile = null;
            int i;

            /*
             * Analyzes options 
             * Ignore unknown option
             */ 
            for (i = 0;i < args.length - 1;++i) {
                if (args[i].equals("-l")) {
                    logfile = args[i+1];
                }
            }

            /* Opens a logfile */
            try {
                if (logfile != null) {
                    Log.setStream(new PrintWriter(new FileWriter(logfile, true), true));
                }
            } catch (IOException e){
                abort("Failed to open log");
            }

            if (!checkFactory()){
                abort("No Factory, aborting ...");
            }
          
            // setup line separator that will be used by 'println'
            System.setProperty("line.separator", lineSeparator);
          
            Log.log(Log.ALWAYS, "Starting up... INVOKE SERVER");
          
            (new InvokeServer()).start();

        } catch (RuntimeException e) {
            Log.printStackTrace(e);
            Log.log(Log.ALWAYS, "Invoke Server is aborted.");
            System.exit(1); 
        } catch (Throwable e){
            Log.printStackTrace(e);
        }
        
        Log.log(Log.ALWAYS, "Exit... INVOKE SERVER");
    }
}
