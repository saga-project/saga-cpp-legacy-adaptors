/*
 * $RCSfile: CondorJob.java,v $ $Revision: 1.12 $ $Date: 2008/03/21 06:10:44 $
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
package org.apgrid.grpc.tools.invokeServer.condor;

import org.apgrid.grpc.tools.invokeServer.*;

import condorAPI.*;
import condorAPI.event.*;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.StringTokenizer;

public class CondorJob extends TargetJob {
    private static Map<String, Boolean> optionMap = new HashMap<String, Boolean>();
    static {
		// value: false: unsupported, true: supported
        optionMap.put("hostname",           true);
        optionMap.put("port",               false);
        optionMap.put("jobmanager",         false);
        optionMap.put("subject",            false);
        optionMap.put("client_name",        false);
        optionMap.put("executable_path",    true);
        optionMap.put("backend",            false);
        optionMap.put("count",              true);
        optionMap.put("staging",            true);
        optionMap.put("argument",           true);
        optionMap.put("work_directory",     false);
        optionMap.put("gass_url",           false);
        optionMap.put("redirect_enable",    true);
        optionMap.put("stdout_file",        true);
        optionMap.put("stderr_file",        true);
        optionMap.put("environment",        true);
        optionMap.put("tmp_dir",            false);
        optionMap.put("status_polling",     false);
        optionMap.put("refresh_credential", false);
        optionMap.put("max_time",           false);
        optionMap.put("max_wall_time",      false);
        optionMap.put("max_cpu_time",       false);
        optionMap.put("queue_name",         false);
        optionMap.put("project",            false);
        optionMap.put("host_count",         false);
        optionMap.put("min_memory",         false);
        optionMap.put("max_memory",         false);
        optionMap.put("rsl_extensions",     false);
        optionMap.put("logFlags",           true);
        optionMap.put("auth_number",        true);
        optionMap.put("condor_option",      true);
    }

    private static final String IS_CONDOR_LOG="ninfg-invoke-server-condor-log";
    protected String universe = null;
    protected int num_process = 1;
    private String authFile = null;
	private Map<String, String> options = new HashMap<String, String>();

    public Log.Flag[] getLogFlags() {
        return new Log.Flag[0];
    }

    public CondorJob(CreateCommand command) throws Command.Exception{
        super(command);

		int i = 0;
		String condor_option = null;
		while ((condor_option = command.list.get("condor_option", i)) != null) {
			StringTokenizer st = new StringTokenizer(condor_option, " ");
			String key = st.nextToken();
			String value = condor_option.substring(key.length());
			if (key.equals("universe")) {
				if (universe != null) {
					throw new Command.Exception("\"universe\" is specified more then once.");
				}
				universe = value;
			} else {
				options.put(key, value);
			}
			i++;
		}
		if (universe == null) {
			universe = "vanilla";
		}

        //if (universe.equals("vanilla")) {
            num_process = count;
        //}
    }

    static Condor condor = new Condor(IS_CONDOR_LOG);
    private JobDescription jd;
    protected void createJob() throws TargetJob.Exception {
        try {
            jd = new JobDescription();
            jd.addAttribute("executable", executable_path);
            jd.addAttribute("universe", universe);

	    if (auth_number != null) {
		try {
		    File file = File.createTempFile("invoke_server_auth", null);
		    authFile = file.getAbsolutePath();
		    String fileName = file.getName();
		    FileOutputStream fos = new FileOutputStream(file);
		    fos.write(auth_number.getBytes());
		    fos.close();
		    jd.addAttribute("transfer_input_files", authFile);
		    args.add("--authNumberFile=" + fileName);
		} catch (IOException e) {
		    throw new TargetJob.Exception(e.getMessage());
		}
	    }

            StringBuffer arg = new StringBuffer();
            for (Iterator itr = args.iterator(); itr.hasNext(); )
                arg.append(" ").append(itr.next());
            jd.addAttribute("arguments", arg.toString());

            StringBuffer env = new StringBuffer();
            for (Iterator itr = envs.iterator(); itr.hasNext(); ) 
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

			for (Map.Entry<String, String> e: options.entrySet()) {
				jd.addAttribute(e.getKey(), e.getValue());
			}

            jd.addQueue(num_process);

            condor.setTmpDir(tmp_dir);
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

    protected void destroyJob() {
	deleteFile();
    }

    protected TargetJobStatus getTargetJobStatus() throws TargetJob.Exception {
        return new CondorJobStatus(cluster);
    }

    protected void checkOptions(String option) {
        if (!optionMap.containsKey(option)) {
            Log.log("Warning: Unknown option \"" + option + "\".");
        } else if (optionMap.get(option) == false) {
            Log.log("Warning: Unsupported option \"" + option + "\".");
        }
    }

    private void deleteFile() {
	if (authFile != null) {
	    Log.log("Simple authentication file \"" + authFile + "\" is deleted");
	    new File(authFile).delete();
	    authFile = null;
	}
    }
}
