/*
 * $RCSfile: NaregiSSJob.java,v $ $Revision: 1.10 $ $Date: 2008/03/28 08:47:44 $
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
package org.apgrid.grpc.tools.invokeServer.naregiSS;
import java.io.*;
import java.util.*;
import org.apgrid.grpc.tools.invokeServer.*;

import org.naregi.sec.renewal.gridss.ExtendedGridSSClient;
import org.naregi.sec.renewal.client.RenewalServiceException;
import org.naregi.ss.service.client.JobScheduleService;
import org.naregi.ss.service.client.JobScheduleServiceException;
import org.naregi.ss.service.client.JobScheduleServiceFactory;
import org.globus.gsi.GlobusCredentialException;
import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSException;
import org.w3c.dom.*;


public class NaregiSSJob extends TargetJob{
    private static HashMap optionMap = new HashMap();
    static {
        // value: 0: unsupported, 1: supported
        optionMap.put("hostname",           new Integer(0));
        optionMap.put("port",               new Integer(0));
        optionMap.put("jobmanager",         new Integer(0));
        optionMap.put("subject",            new Integer(0));
        optionMap.put("client_name",        new Integer(0));
        optionMap.put("executable_path",    new Integer(1));
        optionMap.put("backend",            new Integer(1));
        optionMap.put("count",              new Integer(1));
        optionMap.put("staging",            new Integer(1));
        optionMap.put("argument",           new Integer(1));
        optionMap.put("work_directory",     new Integer(1));
        optionMap.put("gass_url",           new Integer(0));
        optionMap.put("redirect_enable",    new Integer(1));
        optionMap.put("stdout_file",        new Integer(1));
        optionMap.put("stderr_file",        new Integer(1));
        optionMap.put("environment",        new Integer(1));
        optionMap.put("tmp_dir",            new Integer(0));
        optionMap.put("status_polling",     new Integer(0));
        optionMap.put("refresh_credential", new Integer(0));
        optionMap.put("max_time",           new Integer(0));
        optionMap.put("max_wall_time",      new Integer(1));
        optionMap.put("max_cpu_time",       new Integer(0));
        optionMap.put("queue_name",         new Integer(0));
        optionMap.put("project",            new Integer(0));
        optionMap.put("host_count",         new Integer(0));
        optionMap.put("min_memory",         new Integer(0));
        optionMap.put("max_memory",         new Integer(0));
        optionMap.put("rsl_extensions",     new Integer(0));

	// Extra options
	optionMap.put("workingPrefix",	    new Integer(1));
	optionMap.put("OperatingSystemName",new Integer(1));
	optionMap.put("CPUArchitectureName",new Integer(1));
	optionMap.put("MPIType",	        new Integer(1));
	optionMap.put("MPITasksPerHost",    new Integer(1));
	optionMap.put("WallTimeLimit",      new Integer(1));
	optionMap.put("logFlags",           new Integer(1));
	optionMap.put("MemoryLimit",        new Integer(1));
	optionMap.put("CandidateHost",      new Integer(1));
	optionMap.put("IndividualCPUCount", new Integer(1));
}

    static Log.Flag SS_WF_ID   = new Log.Flag("SS_WF_ID");
    static Log.Flag SS_COMMAND = new Log.Flag("SS_COMMAND");
    static Log.Flag[] flags = {SS_WF_ID, SS_COMMAND};

    static final String CHMOD_SCRIPT = "naregiss_is_execute.sh";
	String account    = null;
	String passphrase = null;
    GSSCredential credential = null;
	String workingPrefix = "~";
	String OperatingSystemName = null;
	String CPUArchitectureName = null;
    List   CandidateHosts = new ArrayList();
    int    IndividualCPUCount = -1;
	int    WallTimeLimit      = 1000;
    int    CPUTimeLimit       = -1;
    long   MemoryLimit        = -1;
    long   VirtualMemoryLimit = -1;
    int i;

	int    MPITasksPerHost     = 1;
	String MPIType             = "GridMPI";
    boolean isCanceled         = false;

	ExtendedGridSSClient ssclient = new ExtendedGridSSClient();

    public Log.Flag[] getLogFlags() {
        return flags;
    }

	/** 
	 *  check the command. if it is not ok, throws an exception
	 *  account        : account name
	 *  passphraseFile : file name stores passphrase for myproxy? 
	 */
	public NaregiSSJob(CreateCommand command) throws Command.Exception{
		super(command);
		// Parse command args specific for the NAREGISS
		
		String tmp;
        String passFile;

		// Use $X509_USER_PROXY or /tmp/x509up_u<UID>
		try { 
			credential = NaregiSSCredential.getCredential();
		} catch (GlobusCredentialException e) {
			throw new Command.Exception(e.getMessage());
		} catch (GSSException e) {
			throw new Command.Exception(e.getMessage());
		}

		if ((tmp = command.list.get("workingPrefix")) == null) {
		    ; //workingPrefix is optional
        } else {
			workingPrefix = tmp;
        }

		if ((tmp = command.list.get("OperatingSystemName")) == null) {
		    ; //OperatingSystemName is optional
        } else {
			OperatingSystemName = tmp;
        }

		if ((tmp = command.list.get("CPUArchitectureName")) == null) {
		    ; //CPUArchitectureName is optional
        } else {
			CPUArchitectureName = tmp;
        }

		if ((tmp = command.list.get("MPIType")) == null) {
		    ; //MPIType is optional
        } else {
			MPIType = tmp;
        }

		if ((tmp = command.list.get("MPITasksPerHost")) == null) {
		    ; //MPITasksPerHost is optional
        } else {
			MPITasksPerHost = Integer.parseInt(tmp);
        }

        boolean wallTimeLimitSet = false;
        if ((tmp = command.list.get("WallTimeLimit")) != null) {
            WallTimeLimit = Integer.parseInt(tmp);
            wallTimeLimitSet = true;
        }
        if ((tmp = command.list.get("max_wall_time")) != null) {
            if (wallTimeLimitSet) {
                Log.log(Log.ALWAYS, 
                    "Warning: job_maxWallTime is ignored, because WallTimeLimit option specified.");
            } else {
                WallTimeLimit = Integer.parseInt(tmp) * 60;
                /* max_wall_time is specified by minutes, 
                 * WallTimeLimit in WFML is specified by seconds. */
            }
        } 
        if (!wallTimeLimitSet) {
			;  // use default value 1000;
        }

		if ((tmp = command.list.get("max_cpu_time")) == null) {
		    ; //max_cpu_time is optional
        } else {
			CPUTimeLimit = Integer.parseInt(tmp);
        }

		if ((tmp = command.list.get("max_memory")) == null) {
		    ; //max_memory is optional
        } else {
            VirtualMemoryLimit = Long.parseLong(tmp);
        }

		if ((tmp = command.list.get("MemoryLimit")) == null) {
		    ; //MemoryLimit is optional
        } else {
            MemoryLimit = Long.parseLong(tmp);
        }

        for (i = 0; (tmp = command.list.get("CandidateHost", i)) != null; i++) {
            CandidateHosts.add(tmp); 
        }

		if ((tmp = command.list.get("IndividualCPUCount")) == null) {
		    ; //IndividualCPUCount is optional
        } else {
            IndividualCPUCount = Integer.parseInt(tmp);
        }

        // working directory
        if (work_directory != null) {
            args.add("--workDirectory=" + work_directory);
        }
	}
	
	private String readPass(String passphraseFile) throws Command.Exception {
		try {
			LineNumberReader lnr = new LineNumberReader(new FileReader(passphraseFile));
			String line = lnr.readLine();
			return line.trim();
		} catch (IOException e){
			throw new Command.Exception("failed to read passphraseFile " + passphraseFile);
		}
	}

	private String createWorkingDir(){
		int tmp = (int)(Math.random()*Integer.MAX_VALUE);
		return workingPrefix+ "/ngtmp" +tmp;
	}

    static String chmodScriptPath = null;
    synchronized private String getChmodScriptPath() {
        String ngdir = null;
        if (chmodScriptPath == null) {
            ngdir = System.getProperty("org.apgrid.grpc.ngDir");
            if (ngdir == null) {
                Log.log(Log.ALWAYS, "NG_DIR is not set.");
            } else {
                chmodScriptPath = ngdir + "/lib/" + CHMOD_SCRIPT;
            }
        }
        return chmodScriptPath;
    }

	NaregiDom dom;

	protected void createJob() throws TargetJob.Exception{
		String workingDir = createWorkingDir();
        String chmodScript = getChmodScriptPath();
        if (chmodScript == null) {
			throw new TargetJob.Exception("Can't get path script for changing mode");
        }
		try {
			dom = 
				(new Driver()).createDom(client_name,
										 staging, 
										 redirect_enable,
										 executable_path,
										 (String[])args.toArray(new String[0]),
										 (String[])envs.toArray(new String[0]),
										 stdout_file,
										 stderr_file,
										 workingDir,
										 count,
                                         IndividualCPUCount,
										 OperatingSystemName,
										 CPUArchitectureName,
                                         CandidateHosts,
										 WallTimeLimit,
                                         CPUTimeLimit,
                                         MemoryLimit,
                                         VirtualMemoryLimit,
										 backend,
										 MPITasksPerHost,
										 MPIType,
                                         chmodScript);
		} catch (ClassNotFoundException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		} catch (InstantiationException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		} catch (IllegalAccessException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		}
	}
	
	private Map jobIdMap = new HashMap();
	static int jobNum = 0;

	synchronized private String createJobId(String EPR){
		String tmpId = ""+ jobNum++;
		jobIdMap.put(tmpId, EPR);
		return tmpId;
	}

	private String getEPR(String jobId) throws TargetJob.Exception {
		String tmpEPR = (String)jobIdMap.get(jobId);
		if (tmpEPR == null) 
			throw new TargetJob.Exception("cannot find EPR for " + jobId);
		return tmpEPR;
	}

	protected void submitJob() throws TargetJob.Exception{
        String EPR;
		Log.log(NaregiSSJob.SS_COMMAND, dom);

		try {		
			EPR = ssclient.submitJob(dom.getDocument(), credential);
			jobId = createJobId(EPR);
		} catch (JobScheduleServiceException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		} catch (RenewalServiceException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		}

		Log.set(jobId);
		Log.log(Log.ALWAYS, "got Job ID = \"" + jobId +"\"");
        Log.log(NaregiSSJob.SS_WF_ID, "EPR = \n" + EPR);
	}
	
	protected void cancelJob() throws TargetJob.Exception{
        /* Comment out because NAREGISS bug.
		try {
            Log.log("cancelJob");
			jss.cancelJob(getEPR(jobId), account, passphrase);
		} catch (JobScheduleServiceException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		}
        */
        /* Instead, stands flags. */
        synchronized (this) {
            this.isCanceled = true;
        }
	}

	protected void destroyJob() throws TargetJob.Exception{
		try {
            Log.log(Log.ALWAYS, "deleteJob");
			ssclient.deleteJob(getEPR(jobId), credential);
		} catch (JobScheduleServiceException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		} catch (RenewalServiceException e) {
			Log.printStackTrace(e);
			throw new TargetJob.Exception(e.getMessage());
		}
	}

	protected TargetJobStatus getTargetJobStatus() throws TargetJob.Exception{
		Document stat;
        NaregiSSJobStatus s;
        boolean isCanceled;

        /* If isCanceled is true, Job is done. 
         * It is code for escaping NAREGI SS bug.
         */
        synchronized(this) {
            isCanceled = this.isCanceled;
        }
        if (isCanceled) {
            return new NaregiSSJobStatus(TargetJob.FAILED);
        } else {
            try {
				stat = ssclient.queryJob(getEPR(jobId), credential);
                /* If Error or job failed, output job status(XML doc) always */
                try {
					s = new NaregiSSJobStatus(stat);
                } catch (TargetJob.Exception e) {
                    Log.log(Log.ALWAYS, new NaregiDom(stat));
                    throw e;
                }
                if (s.getJobStatus() == TargetJob.FAILED) {
                    Log.log(Log.ALWAYS, new NaregiDom(stat));
                } else {
                    Log.log(NaregiSSJob.SS_COMMAND, new NaregiDom(stat));
                }
                return s;
            } catch (JobScheduleServiceException e) {
                Log.printStackTrace(e);
                throw new TargetJob.Exception(e.getMessage());
            }
        }
	}

    protected void checkOptions(String option) {
        if (!optionMap.containsKey(option)) {
            Log.log("Warning: Unknown option \"" + option + "\".");
        } else if ( ((Integer)(optionMap.get(option))).intValue() == 0) {
            Log.log("Warning: Unsupported option \"" + option + "\".");
        }
    }

}

