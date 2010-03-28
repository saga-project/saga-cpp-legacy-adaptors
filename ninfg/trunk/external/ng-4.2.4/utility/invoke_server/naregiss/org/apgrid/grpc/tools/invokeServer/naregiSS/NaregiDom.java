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
 * $RCSfile: NaregiDom.java,v $ $Revision: 1.12 $ $Date: 2008/04/03 09:43:24 $
 */
package org.apgrid.grpc.tools.invokeServer.naregiSS;

import org.w3c.dom.bootstrap.*;
import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.*;
import java.util.*;


public class NaregiDom {
	private Document doc;

	// to cope with a fake gridVM bug
	static private boolean useArgForRedirect = false;  


	static final String NAREGI_JSDL_NS_URI = "http://www.naregi.org/ws/2005/08/jsdl-naregi-draft-02";
	static final String NAREGI_WFML_NS_URI = "http://www.naregi.org/wfml/02";
	static final String JSDL_NS_URI        = "http://schemas.ggf.org/jsdl/2005/06/jsdl";
	static final String JSDL_POSIX_NS_URI  = "http://schemas.ggf.org/jsdl/2005/06/jsdl-posix";
    static final String SWEEP_NS_URI       = "http://schemas.ogf.org/jsdl/2007/04/sweep";
    static final String SWEEP_FUNCS_NS_URI = "http://schemas.ogf.org/jsdl/2007/04/sweep/functions";


	// just to use toString()
	public NaregiDom(Document doc){
		this.doc = doc;
	}

	public NaregiDom(String providerName)
    throws ClassNotFoundException, InstantiationException, IllegalAccessException {

		String tmp = System.getProperty(DOMImplementationRegistry.PROPERTY);
		if (tmp == null)
			System.setProperty(DOMImplementationRegistry.PROPERTY,
                               "com.sun.org.apache.xerces.internal.dom.DOMImplementationSourceImpl");
		
		// get an instance of the DOMImplementation registry
		DOMImplementationRegistry registry =
			DOMImplementationRegistry.newInstance();
		// get a DOM implementation the Level 3 XML module
		DOMImplementation domImpl =
			registry.getDOMImplementation("XML 3.0");
		
		DocumentType docType = 
			domImpl.createDocumentType("definitions", "public ID", "system ID");
		doc = domImpl.createDocument(NAREGI_WFML_NS_URI,
									 "definitions", docType);
		doc.getDocumentElement().setAttribute("name", "mainstage");
		
		Element activityModel = doc.createElementNS(NAREGI_WFML_NS_URI, "activityModel");
		doc.getDocumentElement().appendChild(activityModel);

		addCompositionModel(providerName);

	}

    public Element createMkdirActivity( String activityName,
                                        String directory,
                                        int walltimelimit) {
        return createDirActivity(activityName, directory, walltimelimit, "mkdir");
    }

    public Element createRmdirActivity( String activityName,
                                        String directory,
                                        int walltimelimit) {
        return createDirActivity(activityName, directory, walltimelimit, "rmdir");
    }

    private Element createDirActivity( String activityName,
                                       String directory,
                                       int walltimelimit,
                                       String actionName) {
		Element activity = doc.createElementNS(NAREGI_WFML_NS_URI, "activity");
		Element target = doc.createElementNS(NAREGI_WFML_NS_URI, "target");
		Element action  = doc.createElementNS(NAREGI_WFML_NS_URI, actionName);

        activity.appendChild(action);
        action.appendChild(target);

		activity.setAttribute("name", activityName);
		action.setAttribute("WallTimeLimit", "" + walltimelimit);
		target.setAttribute("name", directory);

        return activity;
    }
					  

	public Element createJSDLActivity( String activityName,
									   String executable,
									   String [] arguments,
									   String [] envs,
									   String stdout,
									   String stderr,
									   String workingDirectory,
									   int    wallTimeLimit,
									   int    CPUTimeLimit,
                                       long   MemoryLimit,
                                       long   VirtualMemoryLimit){
		Element activity = doc.createElementNS(NAREGI_WFML_NS_URI, "activity");
		activity.setAttribute("name", activityName);
		Element jsdl     = doc.createElementNS(NAREGI_WFML_NS_URI, "jsdl");
		Element jobDefinition = 
			doc.createElementNS(JSDL_NS_URI, 
								"JobDefinition");
		Element jobDescription = doc.createElementNS(JSDL_NS_URI, "JobDescription");
		Element application = doc.createElementNS(JSDL_NS_URI, "Application");
		Element posixApplication = 
			doc.createElementNS(JSDL_POSIX_NS_URI,
								"POSIXApplication");

		posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "Executable", executable));
		if (arguments != null)
			for (int i = 0; i < arguments.length; i++) {
				String arg = arguments[i];
				posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "Argument", arg));
			}
		if (stdout != null) {
			if (useArgForRedirect)
				posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "Argument", 
															   ">" + stdout));
			else
				posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "Output", stdout));
		}
		if (stderr != null) {
			if (useArgForRedirect)
				posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "Argument", 
															   "2>"+ stderr));
			else
				posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "Error", stderr));
		}

		if (workingDirectory != null)
			posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "WorkingDirectory", 
														   workingDirectory));
		if (envs != null)
			for (int i = 0; i < envs.length; i++) {
				String env = envs[i];
				posixApplication.appendChild(createEnvironmentElement(env));
			}

		posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "WallTimeLimit",
													   "" + wallTimeLimit));
        if (MemoryLimit >= 0) {
            posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "MemoryLimit",
                                                           "" + MemoryLimit));
        }

        if (CPUTimeLimit >= 0) {
            posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "CPUTimeLimit",
                                                           "" + CPUTimeLimit));
        }

        if (VirtualMemoryLimit >= 0) {
            posixApplication.appendChild(createTextElementNS(JSDL_POSIX_NS_URI, "VirtualMemoryLimit",
                                                           "" + VirtualMemoryLimit));
        }

		application   .appendChild(posixApplication);
		jobDescription.appendChild(application);
		jobDefinition .appendChild(jobDescription);
		jsdl          .appendChild(jobDefinition);
		activity      .appendChild(jsdl);
  	   return activity;
	}

	private Element createResource(int     count,
                                   int     IndividualCPUCount,
								   String  OperatingSystemName,
								   String  CPUArchitectureName,
                                   List    CandidateHosts){
        /* Warning: Order is important. */

		Element resource = doc.createElementNS(JSDL_NS_URI, "Resources");
        if (!CandidateHosts.isEmpty()) {
            Element CandidateHostsElement = doc.createElementNS(JSDL_NS_URI, "CandidateHosts");
            
            Iterator it = CandidateHosts.iterator();
            while (it.hasNext()) {
                String host = (String)it.next();
                Element hostElement = createTextElementNS(JSDL_NS_URI, "HostName", host);
                CandidateHostsElement.appendChild(hostElement);
            }
            resource.appendChild(CandidateHostsElement);
        }

		if (OperatingSystemName != null){
			Element OperatingSystem      = doc.createElementNS(JSDL_NS_URI, "OperatingSystem");
			Element OperatingSystemType  = doc.createElementNS(JSDL_NS_URI, "OperatingSystemType");
			Element OperatingSystemNameE = createTextElementNS(JSDL_NS_URI, "OperatingSystemName", 
															 OperatingSystemName);
			OperatingSystemType.appendChild(OperatingSystemNameE);
			OperatingSystem    .appendChild(OperatingSystemType);
			resource           .appendChild(OperatingSystem);
		}

		if (CPUArchitectureName != null){
			Element CPUArchitecture      = doc.createElementNS(JSDL_NS_URI, "CPUArchitecture");
			Element CPUArchitectureNameE = createTextElementNS(JSDL_NS_URI, "CPUArchitectureName", 
															 CPUArchitectureName);
			CPUArchitecture.appendChild(CPUArchitectureNameE);
			resource       .appendChild(CPUArchitecture);
		}

        if (IndividualCPUCount > 0) {
			Element IndividualCPUCountElement = doc.createElementNS(JSDL_NS_URI, "IndividualCPUCount");
			IndividualCPUCountElement.appendChild(createTextElementNS(JSDL_NS_URI, "LowerBoundedRange", "" + IndividualCPUCount));
			resource.appendChild(IndividualCPUCountElement);
        }

		if (count != 1){
			Element totalResourceCount = doc.createElementNS(JSDL_NS_URI, "TotalResourceCount");
			totalResourceCount.appendChild(createTextElementNS(JSDL_NS_URI, "LowerBoundedRange", "" + count));
			resource          .appendChild(totalResourceCount);
		}
		return resource;
	}

	public void addResourceRequirements(Element activity, 
										int     count,
                                        int     IndividualCPUCount,
										String  OperatingSystemName,
										String  CPUArchitectureName,
                                        List    CandidateHosts){
		Element jsdl          = (Element)activity.getElementsByTagName("jsdl").item(0);
		Element jobDefinition = (Element)activity.getElementsByTagName("JobDefinition").item(0);		
		Element jobDescription= (Element)activity.getElementsByTagName("JobDescription").item(0);		

		jobDescription.appendChild(createResource(count, 
                                                  IndividualCPUCount,
												  OperatingSystemName, 
												  CPUArchitectureName,
                                                  CandidateHosts));

	}

	private Element findMPIInsertionPoint(Element activity){
		Element jsdl          = 
			(Element)activity.getElementsByTagName("jsdl").item(0);
		Element jobDefinition = 
			(Element)activity.getElementsByTagName("JobDefinition").item(0);		
		Element jobDescription= 
			(Element)activity.getElementsByTagName("JobDescription").item(0);		
		Element Application   = 
			(Element)activity.getElementsByTagName("Application").item(0);		
		return Application;
	}

	//  <Application>
	//    <naregi-jsdl:MPIApplicationSpecific 
	//     xmlns:naregi-jsdl="http://www.naregi.org/ws/2005/08/jsdl-naregi-draft-02">
	//		 <naregi-jsdl:MPIType>GridMPI</naregi-jsdl:MPIType>
	//		 <naregi-jsdl:TotalTasks>2</naregi-jsdl:TotalTasks>
	//		 <naregi-jsdl:TasksPerHost>1</naregi-jsdl:TasksPerHost>
	//    </naregi-jsdl:MPIApplicationSpecific>
	//  </Application>

	
	private Element createMPI(String  MPIType,
							  int     count,
							  int     TasksPerHost){
		Element mpiApplicationSpecific = 
			doc.createElementNS( NAREGI_JSDL_NS_URI, "MPIApplicationSpecific");
		if (MPIType != null){
			Element mpiType = createTextElementNS(NAREGI_JSDL_NS_URI, "MPIType", MPIType);
			mpiApplicationSpecific.appendChild(mpiType);
		}
		Element totalTasks = createTextElementNS(NAREGI_JSDL_NS_URI, "TotalTasks", ""+count);
		mpiApplicationSpecific.appendChild(totalTasks);
		Element tasksPerHost = createTextElementNS(NAREGI_JSDL_NS_URI, "TasksPerHost", ""+TasksPerHost);
		mpiApplicationSpecific.appendChild(tasksPerHost);
		return mpiApplicationSpecific;
	}

	public void addMPIApplicationSpecific(Element activity,
										  String  MPIType,
										  int     count,
										  int     TasksPerHost){
		Element Application = findMPIInsertionPoint(activity);
		Application.appendChild(createMPI(MPIType, count, TasksPerHost));
	}

	private Element findBulkJobInsertionPoint(Element activity){
		Element jsdl          = 
			(Element)activity.getElementsByTagName("jsdl").item(0);
		Element jobDefinition = 
			(Element)activity.getElementsByTagName("JobDefinition").item(0);		
		return jobDefinition;
	}

    //<JobDefinition>
    //  <Sweep xmlns="http://schemas.ogf.org/jsdl/2007/04/sweep">
    //    <Assignment>
    //      <Loop xmlns="http://schemas.ogf.org/jsdl/2007/04/sweep/functions" end="1" start="0"/>
    //      <Parameter/>
    //    </Assignment>
    //  </Sweep>
    //</JobDefinition>
    //
	private Element createBulkJob(int count){
		Element bulkJobSpec = doc.createElementNS(SWEEP_NS_URI, "Sweep");
		Element assignment  = doc.createElementNS(SWEEP_NS_URI, "Assignment");
		Element loop = doc.createElementNS(SWEEP_FUNCS_NS_URI, "Loop");
		loop.setAttribute("start", "0");
		loop.setAttribute("end", ""+(count-1));
		Element param = doc.createElementNS(SWEEP_NS_URI, "Parameter");

        bulkJobSpec.appendChild(assignment);
        assignment.appendChild(loop);
        assignment.appendChild(param);

		return bulkJobSpec;
	}

    public void addBulkJobSpecific(Element activity,
                                   int count) {
        Element jobDefinition = findBulkJobInsertionPoint(activity);
        jobDefinition.appendChild(createBulkJob(count));
    }

	public Element createTransferActivity( String activityName,
										   int wallTimeLimit,
										   String source,
										   String target){
		Element activity = doc.createElementNS(NAREGI_WFML_NS_URI, "activity");
		activity.setAttribute("name", activityName);
		Element transfer = doc.createElementNS(NAREGI_WFML_NS_URI, "transfer");
		transfer.setAttribute("WallTimeLimit", ""+wallTimeLimit);
		Element sourceElem = doc.createElementNS(NAREGI_WFML_NS_URI, "source");
		sourceElem.setAttribute("name", source);
		Element targetElem = doc.createElementNS(NAREGI_WFML_NS_URI, "target");
		targetElem.setAttribute("name", target);

		transfer.appendChild(sourceElem);
		transfer.appendChild(targetElem);
		activity.appendChild(transfer);
		return activity;
	}

	public void addActivity(Element element){
		NodeList list = 
			doc.getDocumentElement().getElementsByTagName("activityModel");
		list.item(0).appendChild(element);
	}

	public void addDependency(Element source, Element target){
		String LABEL = "WFTEN";

		Element elem = getDependencyInsertPoint();
		Element controlLink = doc.createElementNS(NAREGI_WFML_NS_URI, "controlLink");
		controlLink.setAttribute("label", LABEL);
		controlLink.setAttribute("source", source.getAttribute("name"));
        if (target != null) {
            controlLink.setAttribute("target", target.getAttribute("name"));
        }
		elem.appendChild(controlLink);

	}

	private Element getDependencyInsertPoint(){
		Element docElem = doc.getDocumentElement();
		Element compositionModel = (Element)docElem.getElementsByTagName("compositionModel").item(0);
		Element exportModel      = (Element)docElem.getElementsByTagName("exportModel").item(0);
		Element exportedActivity = (Element)docElem.getElementsByTagName("exportedActivity").item(0);
		Element controlModel     = (Element)docElem.getElementsByTagName("controlModel").item(0);
		return controlModel;
	}

	public void addCompositionModel(String serviceName){
		String WF = "wf1";

		Element compositionModel = doc.createElementNS(NAREGI_WFML_NS_URI, "compositionModel");
		Element importModel = doc.createElementNS(NAREGI_WFML_NS_URI, "importModel");
		Element exportModel = doc.createElementNS(NAREGI_WFML_NS_URI, "exportModel");		
		Element exportedActivity = doc.createElementNS(NAREGI_WFML_NS_URI, "exportedActivity");
		Element exportedActivityInfo = doc.createElementNS(NAREGI_WFML_NS_URI, "exportedActivityInfo");
		exportedActivityInfo.setAttribute("name", WF);
		exportedActivityInfo.setAttribute("serviceName", serviceName);

		Element controlModel = doc.createElementNS(NAREGI_WFML_NS_URI, "controlModel");
		controlModel.setAttribute("controlIn", WF);

		exportedActivity.appendChild(exportedActivityInfo);
		exportedActivity.appendChild(controlModel);
		exportModel.appendChild(exportedActivity);
		compositionModel.appendChild(importModel);
		compositionModel.appendChild(exportModel);
		doc.getDocumentElement().appendChild(compositionModel);
	}

	public void output(OutputStream os)
		throws TransformerException, UnsupportedEncodingException {
	
		try {
			TransformerFactory transFactory = TransformerFactory.newInstance();
			Transformer tran;
			String retVal = null;
			tran = transFactory.newTransformer();
			Properties props = new Properties();
			props.put(OutputKeys.METHOD, "xml");
			props.put(OutputKeys.INDENT, "yes");
			tran.setOutputProperties(props);
	
			tran.transform(new DOMSource(doc), new StreamResult(os));
		} finally {
			try {
				os.close();
			} catch (IOException e) {
			}
		}
	}

	public String toString(){
		try {
			ByteArrayOutputStream os = new ByteArrayOutputStream(10000);
			output(os);
			return os.toString();
		} catch (TransformerException e){
			return "NaregiDom: failed to print";
		} catch (UnsupportedEncodingException e){
			return "NaregiDom: failed to print";
		}
	}

	private Element createEnvironmentElement(String env){
		int index;
		Element elem = doc.createElementNS(JSDL_POSIX_NS_URI, "Environment");

		index = env.indexOf('=');
		if (index == -1) {
			elem.setAttribute("name", env);
		} else {
			elem.setAttribute("name", env.substring(0, index));
			elem.appendChild(doc.createTextNode(env.substring(index + 1)));
		}

		return elem;
	}

	private Element createTextElementNS(String NS_URI, String tag, String content){
		Element elem = doc.createElementNS(NS_URI, tag);
		elem.appendChild(doc.createTextNode(content));
		return elem;
	}

	public static String getTextFromElement(Element elem){
		StringBuffer sb = new StringBuffer();
		NodeList list = elem.getChildNodes();
		for (int i = 0; i < list.getLength(); i++)
			if (list.item(i) instanceof Text){
				Text text = (Text)(list.item(i));
				sb.append(text.getWholeText());
			}
		return sb.toString();
		
	}

	private void setPrefixOf(String ns, String prefix) {
		NodeList nl = null;
		int i;

		nl = doc.getElementsByTagNameNS(ns, "*");
		for (i = 0;i < nl.getLength();++i) {
			nl.item(i).setPrefix(prefix);
		}
	}

	public void setPrefix() {
		setPrefixOf(NAREGI_JSDL_NS_URI, "nj");
		setPrefixOf(NAREGI_WFML_NS_URI, "wfml");
		setPrefixOf(JSDL_NS_URI,        "jsdl");
		setPrefixOf(JSDL_POSIX_NS_URI,  "jp");
	}

	public Document getDocument(){
		return doc;
	}

	static public String PROTOCOL="default";

	static public String createRemoteUrl(Element activity, String path){
		String name = activity.getAttribute("name");
		try {
			name = java.net.URLEncoder.encode(name, "UTF-8");
		} catch (java.io.UnsupportedEncodingException e){
			e.printStackTrace();
		}
		if (path.charAt(0) == '/')
			path = path.substring(1);
		return PROTOCOL + ":///FixMe(" + name  + ")eMxiF/" + path;
	}

	static public String createRemotePath(String working, String path){
        if (path == null) {
            return working;
        }
		int last = path.lastIndexOf('/');
		path = path.substring(last+1);
		return working + "/" + path;
	}

	static public String createTargetUrl(Element activity, String working, String path){
		return createRemoteUrl(activity, createRemotePath(working, path));
	}

	static public String createLocalUrl(String localhostName, String path){
		if (path.charAt(0) == '/')
			path = path.substring(1);
		return PROTOCOL + "://"+ localhostName +"/" + path;
	}

	static public String getFileName(String path){
        if (path == null) return null;
		int last = path.lastIndexOf('/');
		return path.substring(last+1);
	}


	public static void usage(){
		System.err.println("java NaregiDom true|false true|false");
		System.exit(2);
	}


	public static void main(String [] args) throws Exception{ 
		String localhostName = "localhost";
		if (args.length != 2)
			usage();
		NaregiDom dom = new NaregiDom("pbg1010_njs");

		// gridVM may support creation of the working Directory
		boolean createDirByMyself = true; 

	   	String workingDir = "/tmp/01234";

		String executable = "/bin/ls";
		String [] execArgs    = new String[] {"-al", "/tmp"};

		boolean stageExecutable = args[0].equalsIgnoreCase("true");
		boolean stageOutErr     = args[1].equalsIgnoreCase("true");

		String  outFileName     = "/tmp/stdout_file";
		String  errFileName     = "/tmp/stderr_file";
			
		Element createWorking = null;
		Element execute = null;
		Element cleanUp = null;
		Element stageInExecutable = null;
		Element stageOutStdout = null;
		Element stageOutStderr = null;


		if (createDirByMyself) {
			createWorking= dom.createJSDLActivity(
												  "createWorkingDirectory",
												  "/bin/mkdir",
												  new String[] {"--mode", "744", "-p", workingDir},
												  null, null, null, null,
												  300, -1, -1, -1);
			dom.addActivity(createWorking);
		}
			

		{
			String execExecutable = null;
			if (stageExecutable) 
				execExecutable = createRemotePath(workingDir, executable);
			else
				execExecutable = executable;
			
			execute = dom.createJSDLActivity(
											 "execute",
											 execExecutable,
											 execArgs,
											 null, 
											 NaregiDom.getFileName(outFileName), 
											 NaregiDom.getFileName(errFileName), 
											 workingDir,
                                             1000, -1, -1, -1);
			dom.addActivity(execute);
		}

		{
			cleanUp = dom.createJSDLActivity(
											 "cleanUp",
											 "/bin/rm",
											 new String[]{"-rf", workingDir},
											 null,  // env 
											 null,  // stdout 
											 null,  // stderr
											 null, // workingDir
											 100, -1, -1, -1); // walltimelimit
			dom.addActivity(cleanUp);
		}			


		// staging activities
		if (stageExecutable){		
			stageInExecutable =
				dom.createTransferActivity(
								   "stageInExecutable",
								   300,
								   NaregiDom.createLocalUrl(localhostName, executable),
								   NaregiDom.createTargetUrl(execute, workingDir, executable));
			dom.addActivity(stageInExecutable);
		}

		if (stageOutErr) {
			stageOutStdout = 
				dom.createTransferActivity(
								   "stageOutStdout",
								   300,
								   NaregiDom.createTargetUrl(execute, workingDir, outFileName),
								   NaregiDom.createLocalUrl(localhostName, outFileName));
			dom.addActivity(stageOutStdout);
		}

		if (stageOutErr) {
			stageOutStderr = 
				dom.createTransferActivity(
								   "stageOutStderr",
								   300,
								   NaregiDom.createTargetUrl(execute, workingDir, errFileName),
								   NaregiDom.createLocalUrl(localhostName, errFileName));
			dom.addActivity(stageOutStderr);
		}
		
		if (stageExecutable) {
			if (createDirByMyself) 
				dom.addDependency(createWorking,     stageInExecutable);
			dom.addDependency(stageInExecutable, execute);
		} else {
			if (createDirByMyself) 
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

		dom.output(System.out);

	}
}
