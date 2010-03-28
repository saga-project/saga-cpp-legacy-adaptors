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
 * $RCSfile: CondorJob.java,v $ $Revision: 1.9 $ $Date: 2007/07/10 07:10:26 $
 */

package org.apgrid.grpc.tools.invokeServer.condor;

import org.apgrid.grpc.tools.invokeServer.*;

import condorAPI.*;
import condorAPI.event.*;

import java.util.Iterator;

public class CondorJob extends TargetJob {
    private static final String IS_CONDOR_LOG="ninfg-invoke-server-condor-log";
    protected String universe = "vanilla";
    protected int num_process = 1;

    public Log.Flag[] getLogFlags() {
        return new Log.Flag[0];
    }

    public CondorJob(CreateCommand command) throws Command.Exception{
        super(command);
        if (universe.equals("vanilla")) {
            num_process = count;
        }
    }

    static Condor condor = new Condor(IS_CONDOR_LOG);
    private JobDescription jd;
    protected void createJob() throws TargetJob.Exception {
        try {
            jd = new JobDescription();
            jd.addAttribute("executable", executable_path);
            jd.addAttribute("universe", universe);

            StringBuffer arg = new StringBuffer();
            for (Iterator itr = args.iterator(); itr.hasNext(); )
                arg.append(" ").append(itr.next());
            jd.addAttribute("arguments", arg.toString());

            StringBuffer env = new StringBuffer();
            for (Iterator itr = args.iterator(); itr.hasNext(); ) 
                env.append(" ").append(itr.next());
            if (env.length() > 0) {
                jd.addAttribute("environment" , env.toString());
            }

            if (redirect_enable) {
                jd.addAttribute("output", stdout_file);
                jd.addAttribute("error", stderr_file);
                if (! staging) {
                  jd.addAttribute("transfer_executable", "False");
                  jd.addAttribute("when_to_transfer_output", "ON_EXIT");
                  jd.addAttribute("should_transfer_files", "YES");
                }
            }

            if (staging) {
                jd.addAttribute("should_transfer_files", "YES");
                jd.addAttribute("when_to_transfer_output", "ON_EXIT");
            }

            // constant attributes
            jd.addAttribute("copy_to_spool", "False");
            jd.addAttribute("on_exit_remove", "true");
            jd.addAttribute("notification", "Never");

            jd.addQueue(num_process);

            jd.setTmpDir(tmp_dir);
        } catch (CondorException e) {
            Log.printStackTrace(e);
            throw new TargetJob.Exception(e.getMessage());
        }
    }

    Cluster cluster;
    protected void submitJob() throws TargetJob.Exception{
        Log.log("SUBMIT JOB\n" + jd);

        try {
            cluster = condor.submit(jd);
        } catch (CondorException e) {
            Log.printStackTrace(e);
            throw new TargetJob.Exception(e.getMessage());
        }
        jobId = Integer.toString(cluster.id);
        Log.set(jobId);
        Log.log("got Job ID = \"" + jobId +"\"");
    }

    protected void cancelJob() throws TargetJob.Exception {
        try {
            Log.log("These jobs are canceled " + cluster.dump());
            condor.rm(cluster);
        } catch (CondorException e) {
            Log.printStackTrace(e);
            throw new TargetJob.Exception(e.getMessage());
        }
    }

    protected TargetJobStatus getTargetJobStatus() throws TargetJob.Exception {
        return new CondorJobStatus(cluster);
    }
}
