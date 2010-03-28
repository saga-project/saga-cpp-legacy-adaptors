/*
 * $RCSfile: Query.java,v $
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

import java.util.ArrayList;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.IOException;
import org.apache.axis.message.MessageElement;

public class Query {
    private class QueryThread extends Thread {
	ArrayList<String> remoteExecutableInformation;
	boolean notFound = false;
	boolean timeout = false;

	public void run() {
	    Log.log(Log.IS_COMMAND, "Starting query...");
	    try {
		getRemoteClassPathInfo();
	    } catch (Query.TimeoutException e) {
		timeout = true;
	    } catch (Query.NotFoundException e) {
		notFound = true;
	    } catch (Query.Exception e) {
		e.printStackTrace();
	    }
	    InformationService info = getCommand().getInformationService();
	    synchronized (info) {
		info.notify();
	    }
	    Log.log(Log.IS_COMMAND, "                 ...done");
	}

	synchronized protected ArrayList<String> getRemoteExecutableInformation() {
	    return remoteExecutableInformation;
	}

	synchronized private void setRemoteExecutableInformation(ArrayList<String> info) {
	    remoteExecutableInformation = info;
	}

	protected boolean isTimeout() {
	    return timeout;
	}

	protected boolean isNotFound() {
	    return notFound;
	}

	private void getRemoteClassPathInfo()
	    throws Query.TimeoutException, Query.NotFoundException,
		Query.Exception {
	    String xPath =
		"//*[namespace-uri()=" +
		"'http://ninf.apgrid.org/ng5/grpcinfo/types'" +
		" and local-name()='grpcInfoSet']" +
		"/*[namespace-uri()='http://ninf.apgrid.org/ng5'" +
		" and local-name()='execInfo']" +
		"//*[namespace-uri()=" +
		"'http://ninf.apgrid.org/2006/12/RemoteExecutableInformation'" +
		" and local-name()='RemoteExecutableInformation'" +
		" and child::*[namespace-uri()=" +
		"'http://ninf.apgrid.org/2006/12/RemoteExecutableInformation'" +
		" and local-name()='hostName']=" +
		"'" + hostName + "'" +
		" and child::*[namespace-uri()=" +
		"'http://ninf.apgrid.org/2006/12/RemoteClassInformation'" +
		" and local-name()='class'][attribute::name=" +
		"'" + className + "']]";
	    Log.log(Log.IS_COMMAND, xPath);
	    if (timeoutSecond > 0) {
		mdsQuery.setTimeout(timeoutSecond);
	    }
	    MessageElement[] results = mdsQuery.query(xPath);
	    if ((results == null) || (results.length <= 0)) {
		throw new Query.NotFoundException(
		    "Class '" + className + "' is not found.");
	    }
	    Log.log(Log.IS_COMMAND, "Found the information");

	    StringBuffer sb = new StringBuffer();
	    for (int i = 0; i < results.length; i++) {
		sb.append(results[i].toString());
	    }
	    BufferedReader br =
		new BufferedReader(new StringReader(sb.toString()));
	    ArrayList<String> array = new ArrayList<String>();
	    String str;
	    try {
		while ((str = br.readLine()) != null) {
		    array.add(str);
		}
	    } catch (IOException e) {
		throw new Query.NotFoundException(e);
	    }
	    setRemoteExecutableInformation(array);
	}
    }

    private String hostName;
    private String className;
    private int timeoutSecond = 0;
	// The unit of a timeoutSecond is Second.
	// 0: It waits for infinitely.
    private String source = "https://localhost:8443";
    private String path =
	"/wsrf/services/org/apgrid/ninf/ng5/grpcinfo/GrpcInfoService";
    private String subject = "host";
    private String url;

    private static int createId = 0;
    private int queryID;
    private String executableInformation = null;
    private boolean canceled = false;
    private QueryRemoteExecutableInformationCommand command;
    private QueryThread qt;
    private MDS4QueryClient mdsQuery;

    public Query(QueryRemoteExecutableInformationCommand command)
	throws Command.Exception, Query.Exception {
	this.command = command;

	queryID = createID();
	Log.set("" + queryID);

	if ((hostName = command.list.get("hostname")) == null) {
	    throw new Command.Exception("hostname is not specified.");
	}
	if ((className = command.list.get("classname")) == null) {
	    throw new Command.Exception("classname is not specified.");
	}

	String tmp;
	if ((tmp = command.list.get("timeout")) != null) {
	    try {
		timeoutSecond = Integer.parseInt(tmp);
	    } catch (NumberFormatException e) {
		throw new Command.Exception(e.getMessage());
	    }
	}

	if ((tmp = command.list.get("source")) != null) {
	    if (tmp.lastIndexOf('/') >= (tmp.length() - 1)) {
		source = tmp.substring(0, tmp.length() - 1);
	    } else {
		source = tmp;
	    }
	}
	if ((tmp = command.list.get("mds4_subject")) != null) {
	    subject = tmp;
	}

	// Make URL of MDS4.
	url = source + path;
	Log.log(Log.IS_COMMAND, "URL: " + url);
	Log.log(Log.IS_COMMAND, "Subject: " + subject);

	// Get query client.
	mdsQuery = new MDS4QueryClient(url, subject);
    }

    protected void startThread() {
	qt = new QueryThread();
	qt.start();
    }

    private int createID() {
	InformationService info = command.getInformationService();
	do {
	    if (createId >= Integer.MAX_VALUE) {
		createId = 0;
	    } else {
		createId++;
	    }
	} while (info.getQuery(createId) != null);
	return createId;
    }

    public static class Exception extends java.lang.Exception {
	public Exception(String str) {
	    super(str);
	}

	public Exception(java.lang.Exception e) {
	    super(e);
	}
    }

    public static class TimeoutException extends java.lang.Exception {
	public TimeoutException(String str) {
	    super(str);
	}

	public TimeoutException(java.lang.Exception e) {
	    super(e);
	}
    }

    public static class NotFoundException extends java.lang.Exception {
	public NotFoundException(String str) {
	    super(str);
	}

	public NotFoundException(java.lang.Exception e) {
	    super(e);
	}
    }

    /**
     * Defines getters.
     */
    public Command getCommand() {
	return command;
    }

    public String getSource() {
	return source;
    }

    public String getHostName() {
	return hostName;
    }

    public String getClassName() {
	return className;
    }

    public int getTimeout() {
	return timeoutSecond;
    }

    public int getQueryID() {
	return queryID;
    }

    public boolean isCanceled() {
	return canceled;
    }

    public void setCanceled() {
	this.canceled = true;
    }

    protected ArrayList<String> getRemoteExecutableInformation() {
	return qt.getRemoteExecutableInformation();
    }

    protected boolean isTimeout() {
	return qt.isTimeout();
    }

    protected boolean isNotFound() {
	return qt.isNotFound();
    }
}
