/*
 * $RCSfile: InformationService.java,v $
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

import java.io.*;
import java.util.*;

public class InformationService {
    private class WaitQuery extends Thread {
	private InformationService info;
	private boolean exit = false;

	public WaitQuery(InformationService info) {
	    this.info = info;
	}

	protected void setExit() {
	    exit = true;
	}

	public void run() {
	    while (!exit) {
		synchronized (info) {
		    try {
			info.wait(1000);
		    } catch (InterruptedException e) {
			// Ignore.
		    }

		    ArrayList<Integer> keys = new ArrayList<Integer>();
		    HashMap<Integer, Query> map = info.getMap();
		    Collection<Query> values = map.values();
		    for (Query query : values) {
			ArrayList<String> reInfo;
			reInfo = query.getRemoteExecutableInformation();

			String[] notify;
			if (query.isCanceled()) {
			    notify = new String[] {
				NOTIFY_REMOTE_EXECUTABLE_INFORMATION,
				"query_id " + query.getQueryID(),
				"result F",
				"error_message Canceled",
				NOTIFY_REMOTE_EXECUTABLE_INFORMATION_END
			    };
			} else if (query.isTimeout()) {
			    notify = new String[] {
				NOTIFY_REMOTE_EXECUTABLE_INFORMATION,
				"query_id " + query.getQueryID(),
				"result F",
				"error_message Timeout",
				NOTIFY_REMOTE_EXECUTABLE_INFORMATION_END
			    };
			} else if (query.isNotFound()) {
			    notify = new String[] {
				NOTIFY_REMOTE_EXECUTABLE_INFORMATION,
				"query_id " + query.getQueryID(),
				"result F",
				"error_message Not found",
				NOTIFY_REMOTE_EXECUTABLE_INFORMATION_END
			    };
			} else if (reInfo == null) {
			    continue;
			} else {
			    notify = new String[reInfo.size() + 4];
			    int i = 0;
			    notify[i++] = NOTIFY_REMOTE_EXECUTABLE_INFORMATION;
			    notify[i++] = "query_id " + query.getQueryID();
			    notify[i++] = "result S";
			    for (String str : reInfo) {
				notify[i++] =
				    "remote_executable_information " + str;
			    }
			    notify[i++] = NOTIFY_REMOTE_EXECUTABLE_INFORMATION_END;
			}
			info.sendNotify(notify);
			keys.add(query.getQueryID());
		    }

		    for (Integer key : keys) {
			info.removeQuery(info.getQuery(key));
		    }
		}
	    }
	}
    }

    protected static final String PROTOCOL_VERSION = "protocol_version 1.0";
    protected static final String CMD_QUERY_FEATURES = "QUERY_FEATURES";
    protected static final String CMD_QUERY_REMOTE_EXECUTABLE_INFORMATION
	= "QUERY_REMOTE_EXECUTABLE_INFORMATION";
    protected static final String CMD_CANCEL_QUERY = "CANCEL_QUERY";
    protected static final String CMD_EXIT = "EXIT";
    protected static final String NOTIFY_REMOTE_EXECUTABLE_INFORMATION
	= "REMOTE_EXECUTABLE_INFORMATION_NOTIFY";
    protected static final String NOTIFY_REMOTE_EXECUTABLE_INFORMATION_END
	= "REMOTE_EXECUTABLE_INFORMATION_NOTIFY_END";

    private static final String lineSeparator = new String(new char[]{0x0d, 0x0a});

    private PrintStream replyStream;
    private PrintStream notifyStream;

    static BufferedReader requestBuffer
	= new BufferedReader(new InputStreamReader(System.in));

    InformationService() {
	replyStream = System.out;
	notifyStream = System.err;

	System.setOut(Log.getPrintStream());
	System.setErr(Log.getPrintStream());
    }

    public static void main(String[] args) {
	try {
	    String logFile = null;

	    /*
	     * Analyzes options.
	     * Ignore unknown options.
	     */
	    for (int i = 0; i < args.length - 1; i++) {
		if (args[i].equals("-l")) {
		    i++;
		    logFile = args[i];
		}
	    }

	    // Opens a logfile.
	    try {
		if (logFile != null) {
		    Log.setStream(new PrintWriter(new FileWriter(logFile, true), true));
		}
	    } catch (IOException e) {
		abort("Failed to open log");
	    }

	    Log.log(Log.ALWAYS, "Starting up... INFORMATION SERVICE.");
	    new InformationService().executeRequest();
	} catch (RuntimeException e) {
	    Log.printStackTrace(e);
	    Log.log(Log.ALWAYS, "Information Service is aborted.");
	    System.exit(1);
	} catch (Throwable e) {
	    Log.printStackTrace(e);
	}

	Log.log(Log.ALWAYS, "Exit... INFORMATION SERVICE");
	System.exit(0);
    }

    /*
     * Main loop: reads commands from Ninf-G Client and handles them.
     */
    private void executeRequest() {
	Command cmd;
	WaitQuery wq = new WaitQuery(this);
	wq.start();

	while (true) {
	    try {
		cmd = readCommand();
		if (cmd == null) {
		    // EOF
		    break;
		}
		Log.log(Log.IS_COMMAND, cmd);
		cmd.handle();
		if (cmd.request[0].equals(CMD_EXIT)) {
		    break;
		}
	    } catch (Command.Exception e) {
		sendReply("F " + removeCR(e.getMessage()));
	    }
	}
	// EXIT command.
	wq.setExit();
	try {
	    wq.join(3 * 1000);
	} catch (InterruptedException e) {
	    // Ignore.
	}
    }

    // Remove CR.
    static private String removeCR(String s) {
	return s.replace('\n', ' ');
    }

    // Reads commands.
    private Command readCommand() throws Command.Exception {
	String line = readLine();
	if (line == null) {
	    Log.log(Log.ALWAYS, "Unexpected client death!");
	    return null;
	}

	String request[] = line.split("\\s");
	if (request[0].equals(CMD_QUERY_FEATURES)) {
	    return new QueryFeaturesCommand(request, this);
	}
	if (request[0].equals(CMD_QUERY_REMOTE_EXECUTABLE_INFORMATION)) {
	    return new QueryRemoteExecutableInformationCommand(request, this);
	}
	if (request[0].equals(CMD_CANCEL_QUERY)) {
	    return new CancelQueryCommand(request, this);
	}
	if (request[0].equals(CMD_EXIT)) {
	    return new ExitCommand(request, this);
	}

	throw new Command.Exception("Unknown request:" + request[0]);
    }

    // Read one line from stdin.
    String readLine() {
	try {
	    String line = requestBuffer.readLine();
	    Log.log(Log.IS_COMMAND, "--> " + line);
	    return line;
	} catch (IOException e) {
	    Log.printStackTrace(e);
	}
	return null;
    }

    // Abort.
    static void abort(String str) {
	System.err.println(str);
	Log.log(Log.ALWAYS, str);
	throw new RuntimeException();
    }

    // Send the reply.
    synchronized void sendReply(String str) {
	Log.log(Log.IS_COMMAND, "REPLY: " + str);
	replyStream.print(str);
	replyStream.print(lineSeparator);
	replyStream.flush();
    }

    // Send the notify.
    synchronized void sendNotify(String[] str) {
	for (String s : str) {
	    Log.log(Log.IS_COMMAND, "NOTIFY: " + s);
	    notifyStream.print(s);
	    notifyStream.print(lineSeparator);
	}
	notifyStream.flush();
    }

    /**
     * Table to store query.
     */
    HashMap<Integer, Query> map = new HashMap<Integer, Query>();

    protected synchronized void registerQuery(Query query) {
	map.put((query.getQueryID()), query);
    }

    private synchronized void removeQuery(Query query) {
	map.remove(new Integer(query.getQueryID()));
    }

    protected synchronized Query getQuery(int queryID) {
	return map.get(new Integer(queryID));
    }

    private HashMap<Integer, Query> getMap() {
	return map;
    }
}
