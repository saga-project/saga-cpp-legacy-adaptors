/*
 * $RCSfile: TargetJob.java,v $ $Revision: 1.7 $ $Date: 2008/03/04 23:45:07 $
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
import java.util.*;

public abstract class TargetJob extends TimerTask {

    public static final int PENDING = 0;
    public static final int ACTIVE  = 1;
    public static final int DONE    = 2;
    public static final int FAILED  = 3;

    static final String STATUS_STRINGS[] = {
        "PENDING", "ACTIVE", "DONE", "FAILED"
    };


    protected String  executable_path = null;
    protected int     count = -1;
    protected boolean staging = false;
    protected boolean redirect_enable = false;
    protected String  gass_url = null;
    protected String  backend ;
    protected String  stdout_file = null;
    protected String  stderr_file = null;
    protected int     status_polling = 0;
    int getStatusPolling() {
        /* For InvokeServer Class */
        return status_polling;
    }
    protected int     refresh_credential = 0;
    protected String  client_name = null;
    protected String  tmp_dir = "/tmp";
    protected String  work_directory = null;
    protected String  auth_number = null;

    protected ArrayList args = new ArrayList();
    protected ArrayList envs = new ArrayList();

    int status = PENDING; // initial state

    String getStatusString(){
        return STATUS_STRINGS[status];
    }

    String getStatusString(int status){
        return STATUS_STRINGS[status];
    }

    boolean destroyRequested = false;
    protected String jobId;
    CreateCommand command;

    public abstract Log.Flag[] getLogFlags();

    public TargetJob(CreateCommand command) throws Command.Exception{
        this.command = command;
        Log.log(Log.ALWAYS, "TargetJob create");

        String tmp;

        if ((tmp = command.list.get("logFlags")) == null) {
            /* log_categories is optional. */
            Log.setFlags(null, getLogFlags());
        } else try {
            Log.setFlags(tmp, getLogFlags());
        } catch (IllegalArgumentException e) {
            Log.printStackTrace(e);
            throw new Command.Exception(e.getMessage());
        }

        if ((executable_path = command.list.get("executable_path")) == null)
            throw new Command.Exception("executable_path is not in the command");

        if ((tmp = command.list.get("count")) == null)
            throw new Command.Exception("count is not in the command");
        count = Integer.parseInt(tmp);

        if ((tmp = command.list.get("staging")) == null)
            throw new Command.Exception("staging is not in the command");
        staging = Boolean.valueOf(tmp).booleanValue();

        if ((tmp = command.list.get("backend")) == null)
            throw new Command.Exception("backend is not in the command");
        backend = tmp;

        for (int i = 0; true; i++){
            if ((tmp = command.list.get("argument", i)) == null)
                break;
            args.add(tmp);
        }

        if ((tmp = command.list.get("work_directory")) == null) {
            ; /* work_directory is optional. */
        } else {
            work_directory = tmp;
        }

        for (int i = 0; true; i++){
            if ((tmp = command.list.get("environment", i)) == null)
                break;
            envs.add(tmp);
        }

        /* tmp_dir is not always necessary. */
        if ((tmp = command.list.get("tmp_dir")) != null) {
            tmp_dir = tmp;
        }

        if ((client_name = command.list.get("client_name")) == null)
            throw new Command.Exception("client_name is not in the command");

        /* gass_url is not always necessary. */
        gass_url = command.list.get("gass_url");

        if ((tmp = command.list.get("redirect_enable")) == null)
            throw new Command.Exception("redirect_enable is not in the command");
        redirect_enable = Boolean.valueOf(tmp).booleanValue();

        if (redirect_enable){
            if ((stdout_file = command.list.get("stdout_file")) == null)
                throw new Command.Exception("stdout_file is not in the command");
            if ((stderr_file = command.list.get("stderr_file")) == null)
                throw new Command.Exception("stderr_file is not in the command");
        }

        if ((tmp = command.list.get("status_polling")) == null)
            throw new Command.Exception("status_polling is not in the command");
        status_polling = Integer.parseInt(tmp);

	auth_number = command.list.get("auth_number");

    }

    /**
     * actually invokes a job
     */ 

    boolean start(){
        try {
            Log.log(Log.ALWAYS, "Creating job ..");
            createJob();
            Log.log(Log.ALWAYS, "               .. done");

            Log.log(Log.ALWAYS, "Starting job ..");
            submitJob();
            Log.log(Log.ALWAYS, "               .. done");    
        } catch (TargetJob.Exception e) {
            Log.printStackTrace(e);
            sendCreateNotifyFailed(e.getMessage());      
            return false;
        }
        sendCreateNotifySuccess();
        return true;
    }

    void sendCreateNotifySuccess(){
        command.notify("CREATE_NOTIFY " + command.requestId + " S " + jobId );
    }

    void sendCreateNotifyFailed(String msg){
        command.notify("CREATE_NOTIFY " + command.requestId + " F " + msg );
    }

    void sendStatusNotify(){
        command.notify("STATUS_NOTIFY " + jobId  + " " + getStatusString());
    }
    
    synchronized void statusUpdated(int status){
        if (this.status == status) {
            // no update
            return;
        }

        Log.log(Log.ALWAYS,
             "Status changed(" + getStatusString() + "->" + getStatusString(status) + ")");
        // update
        this.status = status;
        sendStatusNotify();
        if ((status == DONE) || (status == FAILED)) {
            try {
                this.destroyJob();
            } catch (TargetJob.Exception e) {
                Log.log(Log.ALWAYS, e);
                Log.printStackTrace(e);
            }
            if (this.destroyRequested) {
                command.is.removeJob(this);
            }
        }
    }

    public String toString(){

        String tmp = "Job\n" +
            "\texecutable_path = \t" + executable_path + "\n" +
            "\tcount = \t" + count + "\n";
      
        for (Iterator i = args.iterator(); i.hasNext(); ) {
            String s = (String)i.next();
            tmp += "\t\targ = \t" + s + "\n";
        }
        for (Iterator i = envs.iterator(); i.hasNext(); ) {
            String s = (String)i.next();
            tmp += "\t\tenv = \t" + s + "\n";
        }

        tmp += 
            "\tgass_url = \t" + gass_url + "\n" +
            "\tredirect_enable = \t" + redirect_enable + "\n" +
            "\tstdout_file = \t" + stdout_file + "\n" +
            "\tstderr_file = \t" + stderr_file + "\n" +
            "\tstatus_polling = \t" + status_polling + "\n" +
            "\trefresh_credential = \t" + refresh_credential + "\n";
        return tmp;

    }

    /** This will periodically called by Timer process **/
    public void run() {
        Log.set("" + command.requestId);

        try {
            Log.log(Log.ALWAYS, "-- get status ");
            TargetJobStatus nStatus = getTargetJobStatus();
            statusUpdated(nStatus.getJobStatus());
            Log.log(Log.ALWAYS, "-- get status ..done");
        } catch (TargetJob.Exception e){
            /* IGNORE */
        }

        if ((status == DONE) || (status == FAILED)) {
            /* Stop timer */
            this.cancel();
            Log.log(Log.ALWAYS, "Stop timer event.");
        }
      
        Log.set("---");
    }

    /** 
      If the status is already DONE or FAILED,
      simply remove itsself from the map.

      If not, set the destroyRequested flag to true,
      and     send cancel command to the server.
     */

    synchronized void destroy() {
        if ((this.status == DONE) || (this.status == FAILED)) {
            command.is.removeJob(this);
        } else {
            try {
                this.cancelJob();
            } catch(TargetJob.Exception e) {
                Log.log(Log.ALWAYS, e);
                Log.printStackTrace(e);

                command.is.removeJob(this);
            }

        }
        destroyRequested = true;
    }

    /********* abstract methods ********/

    abstract protected void createJob() throws TargetJob.Exception;

    abstract protected void submitJob() throws TargetJob.Exception;

    abstract protected void cancelJob() throws TargetJob.Exception;

             protected void destroyJob() throws TargetJob.Exception {/* Empty */};

    abstract protected TargetJobStatus getTargetJobStatus() throws TargetJob.Exception;
	     protected void checkOptions(String option) { /* Empty */ }


    /********* EXCEPTION ********/

    public static class Exception extends java.lang.Exception {
        java.lang.Exception org;
        public Exception(java.lang.Exception org){
            super("TargetJob.Exception:org msg = " + org.getMessage());
            this.org = org;
        }
        public Exception(String msg){
            super(msg);
        }

        public void printStackTrace(PrintStream o){
            if (o == null)
                super.printStackTrace(o);
            else
                org.printStackTrace(o);
        }
    }

}
