/*
 * $RCSfile: NgLog.java,v $ $Revision: 1.12 $ $Date: 2008/02/07 08:17:43 $
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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.IOException;
import java.util.Date;
import java.util.Properties;

import org.apgrid.grpc.ng.info.*;
import org.apgrid.grpc.util.NgUtil;
import org.gridforum.gridrpc.GrpcException;


class NgLogWriter {
	static class NullOutputStream extends OutputStream {
		public void write(int b) {}
	}
	static class NullWriter extends Writer {
		public void write(char [] cbuf, int off, int len) {}
		public void close() {}
		public void flush() {}
	}
	private OutputStream output = null;
	private Writer writer = null;
	private long written = 0;

	public NgLogWriter() {
		output = new NullOutputStream();
		writer = new NullWriter();
	}

	public NgLogWriter(OutputStream ostr) {
		output = ostr;
		writer = new BufferedWriter(new OutputStreamWriter(ostr));
	}

	public void write(String msg) {
		try {
			writer.write(msg + "\n");
		} catch (IOException e) { }
		countUp(msg.length());
	}

	public void write(char [] cbuf, int off, int len) {
		try {
			writer.write(cbuf, off, len);
		} catch (IOException e) { }
		countUp(len);
	}

	public void write(Exception e) {
		String msg = e.toString();
		try {
			writer.write(msg + "\n");
		} catch (IOException ioe) { }
		countUp(msg.length());
		e.printStackTrace(new PrintWriter(writer));
	}

	public void flush() {
		try {
			writer.flush();
		} catch (IOException e) { }
	}

	public void close() throws IOException {
		if (writer == null) {
			return ;
		}
		writer.close();
		writer = null;
		output = null;
	}

	public void setStream(OutputStream os) {
		output = os;
		writer = new BufferedWriter(new OutputStreamWriter(os));
	}

	public long length() {
		return written;
	}

	private void countUp(int n) {
		written += n;
	}
}


/**
 * 
 */
public class NgLog {

	private int logLevel[] = {0, 0, 0, 0};
	private int maxFileSize = 0;
	private boolean boolOverwriteDir = false;
	private String localhostName = null;
	private NgLogWriter out = null;
	private LogFile logFile; // represent Ninf Log File
	
	// commLog 
	private long lastPrintTime = System.currentTimeMillis();
	private String serverName = null;
	private String clientName = null;
	
	// loglevels
	public enum Level {
		// Do not correct order.
		OFF("Off"),
		FATAL("Fatal"),
		ERROR("Error"),
		WARN("Warning"),
		INFO("Information"),
		DEBUG("Debug");

		private final String value;
		Level(String val) { this.value = val; }

		public String toString() { return value; }
	}

	// log category
	public static final int LOGCATEGORY_GLOBUS_TOOLKIT = 0;
	public static final int LOGCATEGORY_NINFG_PROTOCOL = 1;
	public static final int LOGCATEGORY_NINFG_INTERNAL = 2;
	public static final int LOGCATEGORY_NINFG_GRPC = 3;
	// newly log category
	public static final int CAT_NG_PROTOCOL = 1;
	public static final int CAT_NG_INTERNAL = 2;
	public static final int CAT_NG_GRPC     = 3;

	// commLog
	public static final String COMMLOG_SEND = "Send";
	public static final String COMMLOG_RECV = "Recv";

	/*
	 * NgLog constructor for Test
	 */
	public NgLog() {
		initLogLevel(
			Level.DEBUG.ordinal(),
			Level.DEBUG.ordinal(),
		 	Level.DEBUG.ordinal(),
			Level.DEBUG.ordinal());
		out = new NgLogWriter();
		out.setStream(System.err);
        this.localhostName = NgUtil.getLocalHostName();
	}

	public NgLog(LocalMachineInfo anLocalMachineInfo) throws GrpcException {
		// set Log Level
		initLogLevel(
			getLogLevel(anLocalMachineInfo.getLoglevelGlobusToolkit()),
			getLogLevel(anLocalMachineInfo.getLoglevelNinfgInternal()),
			getLogLevel(anLocalMachineInfo.getLoglevelNinfgGrpc()),
			getLogLevel(anLocalMachineInfo.getLoglevelNinfgProtocol()));

		// set the name of client
		this.localhostName = NgUtil.getLocalHostName();

		out = new NgLogWriter();
		String filename = anLocalMachineInfo.getLogFilePath();
		if (filename == null) {
			out.setStream(System.err);
			return; // put message to Sys.err
		}

		this.logFile = createLogFile(filename,
			anLocalMachineInfo.getLogSuffix(),
			anLocalMachineInfo.getLogNFiles());

		// set maxFileSize 
		initMaxFileSize(anLocalMachineInfo.getLogMaxFileSize());

		// set boolOverwriteDir
		initOverwriteDir(anLocalMachineInfo.getLogOverwriteDirectory());

		// open logfile
		try {
			openLogFile();
		} catch (GrpcException e) {
			throw new NgInitializeGrpcClientException(e);
		}
	}
	
	/**
	 * @param remoteMachineInfo
	 * @throws GrpcException
	 */
	public NgLog(RemoteMachineInfo remoteMachineInfo, int executableID)
	 throws GrpcException {
		// NgLog for Communication Log 
		// set default logLevel for Communication Log
		initLogLevel(
			Level.DEBUG.ordinal(),
			Level.DEBUG.ordinal(),
		 	Level.DEBUG.ordinal(),
			Level.DEBUG.ordinal());

		// set PrintStream 
		out = new NgLogWriter();

		CommLogInfo commLogInfo = remoteMachineInfo.getCommLogInfo();
		String filename = commLogInfo.getCommlogFilepath();
		if (filename == null) {
			out.setStream(System.err);
			return; //put message to Sys.err 
		}

		this.logFile = createLogFile(filename + "-execID-" + executableID,
			commLogInfo.getCommlogSuffix(),
			commLogInfo.getCommlogNfiles());

		// set maxFileSize 
		initMaxFileSize(commLogInfo.getCommlogMaxfilesize());

		// set boolOverwriteDir 
		initOverwriteDir(commLogInfo.getCommlogOverwritedir());

		// open logfile
		try {
			openLogFile();
		} catch (GrpcException e) {
			throw new NgInitializeGrpcHandleException(e);
		}
	}

	private int getLogLevel(String level)  throws GrpcException {
		if (level == null) {
			throw new GrpcException("getLogLevel(): argument level is null");
		}
		try {
			return Integer.parseInt(level);
		} catch (NumberFormatException e) {
			return getLogLevelFromString(level);
		}
	}

	private LogFile createLogFile(String filename, String suffix,
	 String nFiles) throws GrpcException{
		return new LogFile(filename, suffix, nFiles);
	}

	private void initLogLevel(int lv_gt, int lv_ng_int, int lv_ng_grpc,
	 int lv_ng_proto) {
		logLevel[LOGCATEGORY_GLOBUS_TOOLKIT] = lv_gt;
		logLevel[LOGCATEGORY_NINFG_INTERNAL] = lv_ng_int;
		logLevel[LOGCATEGORY_NINFG_GRPC]     = lv_ng_grpc;
		logLevel[LOGCATEGORY_NINFG_PROTOCOL] = lv_ng_proto;
	}

	private void initMaxFileSize(String maxSize) {
		if (maxSize == null) {
			maxFileSize = 0; // unlimited
			return;
		}
		maxFileSize = Integer.parseInt(maxSize);
	}

	private void initOverwriteDir(String str) {
		if (str == null) { return; }
		boolOverwriteDir = Boolean.valueOf(str).booleanValue();
	}
	
	private void makeLogDirectory(File logdir) 
	 throws NgInitializeGrpcClientException {
		if (logdir == null) return;

		if ( (! boolOverwriteDir) && logdir.exists() )
			throw new NgInitializeGrpcClientException(
				"Can't overwrite log directory.");

		// create dierctory to save log file
		if ( ! logdir.exists() ) {
			if ( (! logdir.mkdirs() ) && ( logdir.exists()) )  {
				throw new NgInitializeGrpcClientException(
					"Can't create log directory. :" + logdir.toString());
			}
		}
		boolOverwriteDir = true;
	}

	/**
	 * @throws GrpcException
	 */
	private void openLogFile() throws GrpcException {
		if (logFile == null) { return; } // put to System.err

		String targetLogFile = logFile.getName();

		try {
			// create dierctory to save log file 
			File parentDir = new File(targetLogFile).getParentFile();
			if (parentDir != null)
				makeLogDirectory(parentDir);
			// open log file 
			out.setStream(new FileOutputStream(targetLogFile));
		} catch (FileNotFoundException e) {
			throw new NgInitializeGrpcClientException(e);
		} catch (NgInitializeGrpcClientException e) {
			throw e;
		}

		// increment count of process files 
		logFile.increase();
	}
	
	private void closeLogFile() throws GrpcException {
		try {
			out.close();
		} catch (IOException e) {
			throw new GrpcException(e);
		}
	}
	
	private void changeLogFile() {
		try {
			closeLogFile();
			openLogFile();
		} catch (GrpcException e) {
			// can't log message
		}
	}

	private boolean isFileOverLimit() {
		if ((maxFileSize != 0) && (out.length() > maxFileSize)) {
			return true;
		} else {
			return false;
		}
	}
	
	private String getHeader() {
		return new Date() + ": Client: " + localhostName + ": "; 
	}

	/**
	 * put log message
	 */
	private void log(String level, String msg) {
		log(getHeader() + level + ": " + msg);
	}

	private void log(GrpcException e) {
		log(e.toString());
	}

	private synchronized void log(String msg) {
		if (isFileOverLimit()) {
			changeLogFile();
		}
		out.write(msg);
		out.flush();
	}

	private void _printLog(int cat, NgLog.Level lv, String msg) {
		if (isLoggable(cat , lv)) {
			log(lv.toString(), msg);
		}
	}

	private boolean isLoggable(int cat, int lv) {
		return logLevel[cat] >= lv;
	}
	private boolean isLoggable(int cat, Level lv) {
		return isLoggable(cat, lv.ordinal());
	}


///// printLog Interfaces begin

	/**
	 * @param category
	 * @param level
	 * @param e
	 */
	public void printLog(int cat, NgLog.Level level, GrpcException e) {
		if (isLoggable(cat,level)) {
			log(e);
		}
	}

	/**
	 * @param category
	 * @param level
	 * @param client
	 * @param msg
	 */
	public void printLog(int cat, NgLog.Level lv, NgGrpcClient client,
	 String msg) {
		if ( isLoggable(cat, Level.DEBUG) ) {
			_printLog(cat, lv, getIDString(client) + getThreadIDString() + msg);
		} else {
			_printLog(cat, lv, getIDString(client) + msg);
		}
	}

	/* 
	 * Puts all communication data 
	 */
	public void logCommLog(String type, byte[] data) {
		logCommLog(type, data, data.length);
	}

	public void logCommLog(String type, byte[] data, int length) {
		if (isFileOverLimit())
			changeLogFile();

		StringBuilder sb = new StringBuilder( getHeader() );
		// append info about Remote Executable
		sb.append("Executable: ").append(serverName).append(" ");
		sb.append(type).append(": ").append(data.length).append("bytes");
		sb.append("\n");
		sb.append( CommLog.dump(data, length) );
		out.write(sb.toString());
		out.flush();
	}

	public void logDebug(int cat, String msg) {
		_printLog(cat, Level.DEBUG, msg);
	}

	public void logInfo(int cat, String msg) {
		_printLog(cat, Level.INFO, msg);
	}

	public void logWarn(int cat, String msg) {
		_printLog(cat, Level.WARN, msg);
	}

	public void logError(int cat, String msg) {
		_printLog(cat, Level.ERROR, msg); 
	}

	public void logError(int cat, Throwable e) {
		_printLog(cat, Level.ERROR, getStackTrace(e));
	}

	public void logFatal(int cat, String msg) {
		_printLog(cat, Level.FATAL, msg);
	}

	public void logFatal(int cat, Throwable e) {
		_printLog(cat, Level.FATAL, getStackTrace(e));
	}

///// printLog Interfaces end

	private String getStackTrace(Throwable e) {
		StringBuilder sb = new StringBuilder(e.toString() + "\n");
		StackTraceElement[] elements = e.getStackTrace();
		for (int i = 0; i < elements.length; i++)
			sb.append("\tat ").append(elements[i]).append("\n");
		return sb.toString();
	}

	/**
	 * @param strLoglevel
	 */
	private int getLogLevelFromString(String anLevel)
	 throws GrpcException {
		for (Level lv : Level.values() ) {
			if (anLevel.equals(lv.toString())) {
				return lv.ordinal();
			}
		}
		throw new NgInitializeGrpcClientException(
			"NgLog#getLogLevelFromString: unrecognize parameter for loglevel [" 			+ anLevel + "]."); 
	}

	/**
	 * @param client
	 * @return
	 */
	private String getIDString(NgGrpcClient client) {
		return "Context: " + client.getID() + ": ";
	}
	
	/**
	 * @return
	 */
	private String getThreadIDString() {
		return "Thread: " + Thread.currentThread().getName() + ": ";
	}

	/**
	 * @return
	 */
	private String getPastTime(long time) {
		return "(" + time/1000 + "s   " + time%1000 + "ms   0us)";
	}
	
	/**
	 * @param serverName
	 */
	protected void setServerName(String name) {
		this.serverName = name;
	}
	
	/**
	 * @param clientName
	 */
	protected void setClientName(String name) {
		this.localhostName = name;
	}

	/*
	 * Close the logging stream
	 */
	public void close() throws NgIOException {
		try {
			out.close();
		} catch (IOException e) {
			throw new NgIOException(e.getMessage());
		}
	}

}

