/*
 * $RCSfile: NgGramJobGT4.java,v $ $Revision: 1.3 $ $Date: 2008/03/05 03:16:47 $
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
package org.apgrid.grpc.tools.invokeServer.gt4;

import java.util.Enumeration;
import java.util.Properties;
import java.util.Vector;

import org.apache.axis.components.uuid.UUIDGen;
import org.apache.axis.components.uuid.UUIDGenFactory;

import org.globus.cog.abstraction.impl.common.AbstractionFactory;
import org.globus.cog.abstraction.impl.common.StatusEvent;
import org.globus.cog.abstraction.impl.common.task.ExecutionServiceImpl;
import org.globus.cog.abstraction.impl.common.task.IllegalSpecException;
import org.globus.cog.abstraction.impl.common.task.InvalidSecurityContextException;
import org.globus.cog.abstraction.impl.common.task.InvalidServiceContactException;
import org.globus.cog.abstraction.impl.common.task.JobSpecificationImpl;
import org.globus.cog.abstraction.impl.common.task.ServiceContactImpl;
import org.globus.cog.abstraction.impl.common.task.TaskImpl;
import org.globus.cog.abstraction.impl.common.task.TaskSubmissionException;
import org.globus.cog.abstraction.interfaces.ExecutionService;
import org.globus.cog.abstraction.interfaces.JobSpecification;
import org.globus.cog.abstraction.interfaces.SecurityContext;
import org.globus.cog.abstraction.interfaces.ServiceContact;
import org.globus.cog.abstraction.interfaces.Status;
import org.globus.cog.abstraction.interfaces.StatusListener;
import org.globus.cog.abstraction.interfaces.Task;
import org.globus.cog.abstraction.interfaces.TaskHandler;

import java.util.*;


class NgGramJobGT4 implements StatusListener {
	/* definitions */
	private static final String KEY_JOB_ATTRIBUTE = "attribute";
	private static final String PROVIDER_GT4 = "gt4.0.0";
	private static final String DEFAULT_JOBMANAGER = "jobmanager";

       static Map<String, String>  map = new HashMap<String, String> ();
        static {
	    map.put("jobmanager-sge"   , "SGE");
	    map.put("jobmanager-condor", "Condor");
	    map.put("jobmanager-pbs"   , "PBS");	    
	    map.put("jobmanager-fork"  , "Fork");	    
	    map.put("jobmanager"       , "Fork");
	}

	
	/* get UUIDGen class */
	private static final UUIDGen uuidGen;
	static {
		//uuidGen = UUIDGenFactory.getUUIDGen(null);
		uuidGen = UUIDGenFactory.getUUIDGen();
	}
	
	/* received information */
	private int jobID;
	private String uuid;
	private Properties propArgCreate;
	private NgInvokeServerGT4 isGT4;
	private boolean redirect_outerr = false;
	private String jobType;
	private int jobCount;
	
	/* For CoG */
	private TaskHandler taskHandler = null;
	private Task task = null;
	private Status jobStatus = null;
	private String arguments = null;
	private String environment = null;

	/*  */
	private int exitCount = 0;
	private boolean requiredJobCancel = false;
	
	/**
	 * @param JobID
	 * @param propCreateArgs
	 * @param isGT4
	 */
	public NgGramJobGT4(int JobID, Properties propCreateArgs, NgInvokeServerGT4 isGT4){
		this.jobID = JobID;
		this.propArgCreate = propCreateArgs;
		this.isGT4 = isGT4;
		
		this.redirect_outerr = Boolean.getBoolean(
			(String) propCreateArgs.get(NgInvokeServerBase.ARG_REDIRECT_ENABLE));
		this.jobType =
			(String) propCreateArgs.get(NgInvokeServerBase.ARG_BACKEND);
		this.jobCount =	Integer.parseInt(
			(String) propCreateArgs.get(NgInvokeServerBase.ARG_COUNT));
	}
	
	/**
	 * @throws Exception
	 */
	protected void invokeExecutable() throws Exception {
		isGT4.putLogMessage("NgGramJobGT4#invokeExecutable: invoke the JOB.");
		
		/* set Job ID */
		this.uuid = uuidGen.nextUUID();
		this.isGT4.setJobID(this, this.uuid);
		isGT4.putLogMessage("NgGramJobGT4#invokeExecutable: set Request[" +
			this.jobID + "] to jobID [" + this.uuid + "]");

		try {
			/* Create Task */
			isGT4.putLogMessage("NgGramJobGT4#invokeExecutable: create Task.");
			prepareTask();
			
			/* Submit Task */
			isGT4.putLogMessage("NgGramJobGT4#invokeExecutable: submit Task.");
			submitTask();
		} catch (Exception e) {
			isGT4.putLogMessage("NgGramJobGT4#invokeExecutable: " + e.getMessage());
			throw e;
		}
		
	}
	
	/**
	 * @throws Exception
	 */
	public void prepareTask() throws Exception {
		/* create Task */
		this.task = new TaskImpl("Ninf-G4 JOB", Task.JOB_SUBMISSION);
		isGT4.putLogMessage("NgGramJobGT4#prepareTask: Task Identity: " + this.task.getIdentity().toString());

		/* create JobSpecification */
		JobSpecification spec = new JobSpecificationImpl();
		/* path for the executable */
		spec.setExecutable(
			(String) propArgCreate.get(NgInvokeServerBase.ARG_EXECUTABLE_PATH));
		
		/* 
		 * TODO:
		 * how can I set count & jobType?
		 * how can I set hostCount, project, queue, maxMemory, minMemory?
		 * how can I manage staging?
		 */
		
		/* arguments */
		 if (propArgCreate.get(NgInvokeServerBase.ARG_ARGUMENT) != null) {
		     setArguments(spec);
		 }

		 /* environment */
		if (propArgCreate.get(NgInvokeServerBase.ARG_ENVIRONMENT) != null) {
		    setEnvironment(spec);
		}

		/* work_directory */
		if (propArgCreate.get(NgInvokeServerBase.ARG_WORK_DIRECTORY) != null) {
			spec.setDirectory(
				(String) propArgCreate.get(NgInvokeServerBase.ARG_WORK_DIRECTORY));
		}

		/* redirect_enable */
		if (this.redirect_outerr == true) {
			/* set redirect enabled */
		    spec.setRedirected(true);
		    /* stdout */
		    if (propArgCreate.get(NgInvokeServerBase.ARG_STDOUT_FILE) != null) {
			    spec.setStdOutput(
			    	(String) propArgCreate.get(NgInvokeServerBase.ARG_STDOUT_FILE));
		    }
		    /* stderr */
		    if (propArgCreate.get(NgInvokeServerBase.ARG_STDERR_FILE) != null) {
			    spec.setStdError(
			    	(String) propArgCreate.get(NgInvokeServerBase.ARG_STDERR_FILE));
		    }
		}

		/* another attributes */
		if (propArgCreate.get(KEY_JOB_ATTRIBUTE) != null) {
		    setAttributes(spec);
		}
		
		/* set job specification to Task */
		this.task.setSpecification(spec);

		/* create ExecutionService */
		ExecutionService service = new ExecutionServiceImpl();
		service.setProvider(PROVIDER_GT4);

		SecurityContext securityContext =
			AbstractionFactory.newSecurityContext(PROVIDER_GT4);
		securityContext.setCredentials(null);
		service.setSecurityContext(securityContext);

		ServiceContact sc = new ServiceContactImpl(makeRequestString());
		service.setServiceContact(sc);
		
		String jmName = map.get(DEFAULT_JOBMANAGER);
		String propJm = (String)propArgCreate.get(NgInvokeServerBase.ARG_JOBMANAGER);
 		if (propJm != null) {
		    String tmpString = (String) map.get(propJm);
		    if (tmpString != null)
			jmName = tmpString;
		    else
			jmName = propJm;   
		        // when the specified jobmanager is not on the map, just use the specified value
 		}
		service.setJobManager(jmName);
		isGT4.putLogMessage("service.setJobManager("  + jmName + ")");

		this.task.addService(service);
		this.task.addStatusListener(this);
	}
	
	/**
	 * @throws Exception
	 */
	private void submitTask() throws Exception {
		this.taskHandler = AbstractionFactory.newExecutionTaskHandler(PROVIDER_GT4);
		try {
			this.taskHandler.submit(this.task);
		} catch (InvalidSecurityContextException ise) {
			isGT4.putLogMessage("Security Exception: " + ise.getMessage());
		    throw ise;
		} catch (TaskSubmissionException tse) {
		    isGT4.putLogMessage("Submission Exception: " + tse.getMessage());
		    throw tse;
		} catch (IllegalSpecException ispe) {
			isGT4.putLogMessage("Specification Exception: " + ispe.getMessage());
		    throw ispe;
		} catch (InvalidServiceContactException isce) {
		    isGT4.putLogMessage("Service Contact Exception");
		    throw isce;
		}
	}
	
	/**
	 * @param spec
	 */
	private void setArguments(JobSpecification spec) {
		/* get list of Arguments */
		Vector listArg =
			(Vector) propArgCreate.get(NgInvokeServerBase.ARG_ARGUMENT);
		
		for (int i = 0; i < listArg.size(); i++) {
			/* set argument into the JobSpecification */
            spec.addArgument((String) listArg.get(i));
		}
	}
	
	/**
	 * @param spec
	 */
	private void setEnvironment(JobSpecification spec) {
		/* get list of Environment */
		Vector listEnv =
			(Vector) propArgCreate.get(NgInvokeServerBase.ARG_ENVIRONMENT);
		
		for (int i = 0; i < listEnv.size(); i++) {
			/* get target entry */
			String line = (String) listEnv.get(i);
			
			/* separate into two parts */
			int keyLength = line.indexOf('=');
			String key;
			String val;
			if (keyLength == -1) {
				key = line;
				val = null;
			} else {
				key = line.substring(0, keyLength);
				val = line.substring(keyLength + 1);
			}
			
			/* set it into the JobSpecification */
            spec.addEnvironmentVariable(key, val);
		}
	}
	
	/**
	 * @param spec
	 */
	private void setAttributes(JobSpecification spec) {
		/* get Properties for Attributes */
		Properties propEnv = (Properties) propArgCreate.get(KEY_JOB_ATTRIBUTE);
		Enumeration keys = propEnv.keys();
		
		while (keys.hasMoreElements() == true) {
			/* get key */
			String name = (String) keys.nextElement();
			/* get value */
			String value = (String) propEnv.get(name);
			
			/* set it into the JobSpecification */
            spec.addEnvironmentVariable(name, value);
		}
	}
	
	/**
	 * @return
	 */
	private String makeRequestString() {
		/* make contact string */
		String hostName = (String) propArgCreate.get(NgInvokeServerBase.ARG_HOSTNAME);
		StringBuffer sb = new StringBuffer(hostName);

		/* append port number */
		String port = (String) propArgCreate.get(NgInvokeServerBase.ARG_PORT);
		if ((port != null) && (! port.equals("0"))) {
			sb.append(":" + port);
		}
		
		/* append jobmanager */
		//		String jobmanager = (String) propArgCreate.get(NgInvokeServerBase.ARG_JOBMANAGER);
		//		if (jobmanager != null) {
		//			sb.append("/" + jobmanager);
		//		}
		
		/* return contact string */
		return sb.toString();
	}

	/* (non-Javadoc)
	 * @see org.globus.cog.abstraction.interfaces.StatusListener#statusChanged(org.globus.cog.abstraction.impl.common.StatusEvent)
	 */
	public void statusChanged(StatusEvent event) {
		this.jobStatus = event.getStatus();
		isGT4.putLogMessage("Status changed to " + this.jobStatus.getStatusString());
		
		/* print log message */
		/* failed */
		if (this.jobStatus.getStatusCode() == Status.FAILED) {
			if (event.getStatus().getMessage() != null) {
				isGT4.putLogMessage("Job failed: " + event.getStatus().getMessage());
			} else if (event.getStatus().getException() != null) {
				isGT4.putLogMessage("Job failed: " +
					event.getStatus().getException().getMessage());
			} else {
				isGT4.putLogMessage("Job failed");
			}
		}
		/* done */
		if (this.jobStatus.getStatusCode() == Status.COMPLETED) {
			isGT4.putLogMessage("Job completed");
			if (this.task.getStdOutput() != null) {
				System.out.println(this.task.getStdOutput());
			}
			if (this.task.getStdError() != null) {
				System.err.println(this.task.getStdError());
			}
		}
		
		/* redirect stdout/stderr */
		if ((this.jobStatus.getStatusCode() == Status.COMPLETED) || 
			(this.jobStatus.getStatusCode() == Status.FAILED)) {
			/* remove status listener */
			this.task.removeStatusListener(this);
			/* redirect stdout/stderr */
		}
		
		/* set condition */
		if (this.jobStatus.getStatusCode() == Status.SUBMITTED) {
			isGT4.setStatus(this, NgInvokeServerBase.STATUS_PENDING);
		} else if (this.jobStatus.getStatusCode() == Status.ACTIVE) {
			isGT4.setStatus(this, NgInvokeServerBase.STATUS_ACTIVE);
		} else if (this.jobStatus.getStatusCode() == Status.COMPLETED) {
			isGT4.setStatus(this, NgInvokeServerBase.STATUS_DONE);
		} else if (this.jobStatus.getStatusCode() == Status.FAILED) {
			isGT4.setStatus(this, NgInvokeServerBase.STATUS_FAILED);
		}
	}

	/**
	 * @return
	 */
	protected int getState() {
		return this.jobStatus.getStatusCode();
	}
	
	/**
	 * @throws Exception
	 * 
	 */
	protected void incrementExitCount() throws Exception {
		this.exitCount++;
		if ((this.exitCount == jobCount) && (this.requiredJobCancel == true)) {
			/* GRAM Job cancel */
			this.taskHandler.cancel(this.task);
		}
	}
	
	/**
	 * 
	 */
	protected void setRequiredJobCancel() {
		this.requiredJobCancel = true;
	}

	/**
	 * 
	 */
	void dispose() throws Exception {
		if (this.jobStatus.getStatusCode() != Status.COMPLETED) {
			this.taskHandler.remove(this.task);
		}
	}

}
