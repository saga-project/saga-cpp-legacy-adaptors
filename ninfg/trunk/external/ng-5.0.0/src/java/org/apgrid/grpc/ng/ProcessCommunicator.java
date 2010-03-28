/*
 * $RCSfile: ProcessCommunicator.java,v $ $Revision: 1.12 $ $Date: 2008/03/06 06:56:44 $
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
package org.apgrid.grpc.ng;

import java.io.BufferedWriter;
import java.io.File;
import java.io.OutputStreamWriter;
import java.io.LineNumberReader;
import java.io.InputStreamReader;
import java.io.IOException;

/*
 * This class communicate with external process.
 * The communication is done through ExtModuleRequest,Reply,Notify classes.
 */
public class ProcessCommunicator {
	private String  path = null;
	private String	args = null;
	private Process proc = null;
	private LineNumberReader reply_stream = null;
	private LineNumberReader notify_stream = null;
	private BufferedWriter request_stream = null;

	/**
	 * @param array containing the command to call and its arguments.
	 * @throws IOException If an I/O error occurs
	 */ 
	public ProcessCommunicator(String [] cmdarray)
	 throws IOException, NgException {
		if (cmdarray== null) {
			throw new NullPointerException();
		}

		File file = new File(cmdarray[0]);
		if (!file.exists()) {
			throw new NgException(cmdarray[0] + ": File is not found.");
		}

		// Exec command
		Runtime runtime = Runtime.getRuntime();
		this.proc = runtime.exec(cmdarray);

		// set instance variables
		this.path = cmdarray[0];
		StringBuilder sb = new StringBuilder();
		for (int i = 1; i < cmdarray.length; i++) {
			sb.append(" ");
			sb.append(cmdarray[i]);
		}
		this.args = sb.toString();

		// set streams
		request_stream = 
			new BufferedWriter(new OutputStreamWriter(proc.getOutputStream()));
		reply_stream = 
			new LineNumberReader(new InputStreamReader(proc.getInputStream()));
		notify_stream =
			new LineNumberReader(new InputStreamReader(proc.getErrorStream()));
	}

	/**
	 * Send the request message to process, and returns the reply
	 *
	 * @param request a external module request
	 * @return a external module reply
	 * @throws IOException 
	 */
	public ExtModuleReply send(ExtModuleRequest request) throws IOException {
		request(request);
		return reply();
	}

	/*
	 * Send request message to process.
	 */
	public void request(ExtModuleRequest req) throws IOException {
		synchronized (request_stream) {
			req.submit(request_stream);
		}
	}

	/*
	 * Get reply message from process
	 */
	public ExtModuleReply reply() throws IOException {
		synchronized (reply_stream) {
			return ExtModuleReply.issues(reply_stream);
		}
	}

	/**
	 * Returns the reader for notify received
	 */
	public LineNumberReader getNotifyReader() {
		return this.notify_stream;
	}

	private void close() {
		try {
			if (request_stream != null) {
				request_stream.close();
                request_stream = null;
			}
			if (reply_stream != null) {
				reply_stream.close();
				reply_stream = null;
			}
			if (notify_stream != null) {
				notify_stream.close();
				notify_stream = null;
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Kills the sub-process.
	 */
	public void destroy() {
		this.close();
		proc.destroy();
		proc = null;
	}

	/**
	 * wait for exit
	 */
	public void exit() {
		try {
			this.close();
			proc.waitFor();
			proc = null;
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	/**
	 * @return true process is alive.
	 * @return false process is done.
	 */
	public boolean isAlive() {
		if (proc == null) return false;
		try {
			proc.exitValue();
		} catch (IllegalThreadStateException e) {
			return true;
		}
		return false;
	}

	/*
	 * Returns the exit value for the subprocess.
	 * 
	 * @return exit value.
	 *         if proc member is null or process is running 
	 *         then return the invalid value(Integer.MAX_VALUE)
	 */
	public int exitValue() {
		if (proc == null) return Integer.MAX_VALUE;
		int ret;
		try {
			ret = proc.exitValue();
		} catch (IllegalThreadStateException e) {
			ret = Integer.MAX_VALUE;
		}
		return ret;
	}

	public String toString() {
		return "Command: " + this.path + " " + this.args;
	}
}

