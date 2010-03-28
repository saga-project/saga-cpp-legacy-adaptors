/*
 * $RCSfile: Driver.java,v $ $Revision: 1.7 $ $Date: 2008/03/28 08:47:44 $
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

import org.apgrid.grpc.tools.invokeServer.Log;
import org.naregi.ss.service.client.JobScheduleService;
import org.naregi.ss.service.client.JobScheduleServiceException;
import org.naregi.ss.service.client.JobScheduleServiceFactory;
import org.w3c.dom.*;


import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.*;
import java.util.*;

public class Driver {
	static final String stateNSURI = "http://www.naregi.org/ws/bpel/state/02";
    static final int defaultWallTimeLimit = 300;

	private static void usage(){
		System.err.println("java NaregiDom true|false true|false");
		System.exit(2);
	}

	public NaregiDom createDom(String client_name,
							   boolean stageExecutable,
							   boolean stageOutErr,
							   String  executable,
							   String[]execArgs,
							   String[]execEnvs,
							   String  outFileName,
							   String  errFileName,
							   String  workingDir,
							   
							   int     count,
                               int     IndividualCPUCount,
							   String  OperatingSystemName,
							   String  CPUArchitectureName,
                               List    CandidateHosts,
							   int     wallTimeLimit,
                               int     CPUTimeLimit,
                               long    MemoryLimit,
                               long    VirtualMemoryLimit,

							   String  backend,
							   int     MPITasksPerHost,
							   String  MPIType,
                               String  chmodScript
							   ) 
    throws ClassNotFoundException, InstantiationException, IllegalAccessException {
		String localhostName = client_name;
		NaregiDom dom = new NaregiDom("Dummy"); // I believe it does have effect

		Element createWorking = null;
		Element chmodExecutable = null;
		Element execute = null;
		Element cleanUp = null;
		Element stageInExecutable = null;
		Element stageInScript = null;
		Element stageOutStdout = null;
		Element stageOutStderr = null;

        int resourceCount = (count + MPITasksPerHost - 1)/MPITasksPerHost;

        Log.log(Log.ALWAYS, "Enter creating DOM");

		// Activity of  Ninf-G Executable 
        String jobExecutable = null;
        String[] jobArgs = new String[1];

        String curDir = workingDir;
        // If curDir starts character which is not "/",
        // curDir is relative path to home directory.
        if (curDir.equals("~")) {
            curDir = "";
        } else if (curDir.startsWith("~/")) {
            curDir = curDir.substring(2);
        }

        if (stageExecutable) {
            String executablePath = "./" + NaregiDom.getFileName(executable);
            List argsList = new LinkedList();
            jobExecutable = "/bin/sh";

            argsList.add(NaregiDom.getFileName(chmodScript));
            argsList.add(executablePath);
            for (int i = 0; i < execArgs.length; i++) {
                String arg = execArgs[i];
                argsList.add(arg);
            }
            jobArgs = (String[])argsList.toArray(jobArgs);
        } else {
            jobExecutable = executable;
            jobArgs = execArgs;
        }

        String outBaseName = null;
        String errBaseName = null;
		if (stageOutErr) {
             outBaseName = NaregiDom.getFileName(outFileName);
             errBaseName = NaregiDom.getFileName(errFileName);
        }
        execute = dom.createJSDLActivity(
                                         "execute",
                                         jobExecutable,
                                         jobArgs,
                                         execEnvs, 
                                         outBaseName,
                                         errBaseName,
                                         curDir,
                                         wallTimeLimit,
                                         CPUTimeLimit,
                                         MemoryLimit,
                                         VirtualMemoryLimit);
        
        if (resourceCount != 1 || OperatingSystemName != null || CPUArchitectureName != null ||
            !CandidateHosts.isEmpty()) {
            dom.addResourceRequirements(execute, resourceCount, 
                                        IndividualCPUCount,
                                        OperatingSystemName, 
                                        CPUArchitectureName,
                                        CandidateHosts);
        }
        if (count > 1 || !backend.equals("NORMAL")) {
            dom.addMPIApplicationSpecific(execute, MPIType, 
                                          count, MPITasksPerHost);
        }
        dom.addActivity(execute);

        // Working Directory
        createWorking= dom.createMkdirActivity(
                                          "createWorkingDirectory",
                                          NaregiDom.createTargetUrl(execute, workingDir, null),
                                          defaultWallTimeLimit);
        dom.addActivity(createWorking);
        cleanUp = dom.createRmdirActivity(
                                         "cleanUp",
                                          NaregiDom.createTargetUrl(execute, workingDir, null),
                                          defaultWallTimeLimit);
        dom.addActivity(cleanUp);

		// staging activities
		if (stageExecutable) {
			stageInExecutable =
				dom.createTransferActivity(
								   "stageInExecutable",
								   defaultWallTimeLimit,
								   NaregiDom.createLocalUrl(localhostName, executable),
								   NaregiDom.createTargetUrl(execute, workingDir, executable));
			dom.addActivity(stageInExecutable);

			stageInScript =
				dom.createTransferActivity(
								   "stageInScript",
								   defaultWallTimeLimit,
								   NaregiDom.createLocalUrl(localhostName, chmodScript),
								   NaregiDom.createTargetUrl(execute, workingDir, chmodScript));
			dom.addActivity(stageInScript);
		}

		if (stageOutErr) {
			stageOutStdout = 
				dom.createTransferActivity(
								   "stageOutStdout",
								   defaultWallTimeLimit,
								   NaregiDom.createTargetUrl(execute, workingDir, outFileName),
								   NaregiDom.createLocalUrl(localhostName, outFileName));
			dom.addActivity(stageOutStdout);
		}

		if (stageOutErr) {
			stageOutStderr = 
				dom.createTransferActivity(
								   "stageOutStderr",
								   defaultWallTimeLimit,
								   NaregiDom.createTargetUrl(execute, workingDir, errFileName),
								   NaregiDom.createLocalUrl(localhostName, errFileName));
			dom.addActivity(stageOutStderr);
		}


		////////////////////////////
		//  Setting up dependencies
		////////////////////////////
		if (stageExecutable) {
            dom.addDependency(createWorking,     stageInExecutable);
            dom.addDependency(createWorking,     stageInScript);
			dom.addDependency(stageInExecutable, execute);
			dom.addDependency(stageInScript,     execute);
		} else {
            dom.addDependency(createWorking,     execute);
		}

		if (stageOutErr) {
			dom.addDependency(execute,           stageOutStdout);
			dom.addDependency(execute,           stageOutStderr);
	        dom.addDependency(stageOutStdout,    cleanUp);
			dom.addDependency(stageOutStderr,    cleanUp);
		} else {
			dom.addDependency(execute,           cleanUp);
		}
		dom.setPrefix();

		return dom;
	}


	private static Document getDocumentFromFile(String file) { 
		Document doc = null;
		DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
		dbf.setNamespaceAware(true);
		DocumentBuilder db;
		try {
			db = dbf.newDocumentBuilder();
			doc = db.parse(new File(file));
		} catch (Exception e) {
			e.printStackTrace();
		}
		return doc;
	}

	public static String getStateString (Document stat){
		Element states = stat.getDocumentElement();

		//System.out.println(states.getLocalName());
		//System.out.println(states.getNamespaceURI());

		NodeList statusList = states.getChildNodes();
		String topLevelWf = "wf1";

		for (int i = 0; i < statusList.getLength(); i++) {
			Node node = statusList.item(i);
			if (!(node instanceof Element))
				continue;
			Element status = (Element)node;
			if ((!stateNSURI.equals(status.getNamespaceURI())) ||
				(!"activityState".equals(status.getLocalName())))
				continue;

			Element nameElem = 
				(Element)status.getElementsByTagNameNS(stateNSURI, "name").item(0);
			//System.out.println("nameElem = " + nameElem);
			String str = NaregiDom.getTextFromElement(nameElem);
			//	System.out.println(str);
			if (str.equals(topLevelWf)){
				Element stateElem = 
					(Element)status.getElementsByTagNameNS(stateNSURI, "state").item(0);
				return NaregiDom.getTextFromElement(stateElem);
			}
		}
		return null;
	}
	
	public static void main0(String [] args) throws Exception{ 
		Document doc = getDocumentFromFile(args[0]);
		System.out.println(getStateString(doc));

	}


	public static void main(String [] args) throws Exception{ 
		if (args.length != 2)
			usage();
		boolean stageExecutable = args[0].equalsIgnoreCase("true");
		boolean stageOutErr     = args[1].equalsIgnoreCase("true");
		String  executableName  = "/bin/ls";
		String[]executeArgs     = new String[] {"-al", "/tmp"};
		String[]executeEnvs     = null;
		String  outFileName     = "/tmp/stdout_file";
		String  errFileName     = "/tmp/stderr_file";
		String  workingDir      = "/tmp/01234";
        String  chmodScript     = "/tmp/chmodScript.sh";

		int     count              = 1;
        int     IndividualCPUCount = -1;
		String  os              = null;
		String  arch            = null;
        List CandidateHosts     = new ArrayList();

		NaregiDom dom = 
			(new Driver()).createDom("localhost",
									 stageExecutable, 
									 stageOutErr,
									 executableName,
									 executeArgs,
									 executeEnvs,
									 outFileName,
									 errFileName,
									 workingDir,
									 count,
                                     IndividualCPUCount,
									 os,
									 arch,
                                     CandidateHosts,
									 1000,
                                     -1,
                                     -1,
                                     -1,
									 "NORMAL",
									 1,
									 null,
                                     chmodScript);
		dom.output(System.out);

        JobScheduleService jss = JobScheduleServiceFactory.create();
        
        String id;
        Document stat;
        String account = "test";
        String pass_phrase = "testPass";
        
        id = jss.submitJob(dom.getDocument(), account, pass_phrase);
        System.out.println("Return Job ID = \n\"" + id +"\"");
        
        for (int i=0;i < 30;i++) {
            stat = jss.queryJob(id, account, pass_phrase);
            String stateString = getStateString(stat);
            System.err.println(stateString);
            if ("Done"      .equals(stateString) ||
                "Exception" .equals(stateString) ||
                "Terminated".equals(stateString))
                break;
        }
        
        // jss.cancelJob(id, account, pass_phrase);
    
        jss.deleteJob(id, account, pass_phrase);

	}
}
