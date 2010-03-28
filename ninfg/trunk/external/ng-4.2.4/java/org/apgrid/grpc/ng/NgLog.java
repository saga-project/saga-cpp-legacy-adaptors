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
 * $RCSfile: NgLog.java,v $ $Revision: 1.37 $ $Date: 2006/09/11 11:15:09 $
 */

package org.apgrid.grpc.ng;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;

import org.apgrid.grpc.ng.info.*;
import org.globus.util.Util;
import org.gridforum.gridrpc.GrpcException;

public class NgLog {
	private static final String STR_TIME = "%t";
	private static final String STR_HOSTNAME = "%h";
	private static final String STR_PROCESS = "%p";
	private static final String STR_PERCENT = "%%";
	
	private int logLevel[] = {0, 0, 0, 0};
	private PrintStream out = System.err;
	private String logFileName = null;
	private String logFileSuffix = null;
	private int nFiles = 1;
	private int maxFileSize = 0;
	private boolean boolOverwriteDir = false;
	private int processFiles = 0;
	private String localhostName = null;
	private String sequenceFormat = null;
	private long nWrite = 0;
	
	/* commLog */
	private long lastPrintTime = System.currentTimeMillis();
	private String serverName = null;
	private String clientName = null;
	
	/* loglevels */
	public static final int LOGLEVEL_OFF = 0;
	public static final int LOGLEVEL_FATAL = 1;
	public static final int LOGLEVEL_ERROR = 2;
	public static final int LOGLEVEL_WARN = 3;
	public static final int LOGLEVEL_INFO = 4;
	public static final int LOGLEVEL_DEBUG = 5;
	
	private static final String[] logLevelString =
		{ "Off", "Fatal", "Error", "Warning", "Information", "Debug" };
	
	/* Strings of LogLevel for configuration file */
	private static final String LOGLEVEL_OFF_STR = "Off";
	private static final String LOGLEVEL_FATAL_STR = "Fatal";
	private static final String LOGLEVEL_ERROR_STR = "Error";
	private static final String LOGLEVEL_WARN_STR = "Warning";
	private static final String LOGLEVEL_INFO_STR = "Information";
	private static final String LOGLEVEL_DEBUG_STR = "Debug";
	
	/* log category */
	public static final int LOGCATEGORY_GLOBUS_TOOLKIT = 0;
	public static final int LOGCATEGORY_NINFG_PROTOCOL = 1;
	public static final int LOGCATEGORY_NINFG_INTERNAL = 2;
	public static final int LOGCATEGORY_NINFG_GRPC = 3;
	
	/* commLog */
	public static final String COMMLOG_SEND = "Send";
	public static final String COMMLOG_RECV = "Recv";
	
	/**
	 * 
	 */
	public NgLog() {
		/* nothing */
	}
	
	/**
	 * @param localMachineInfo
	 * @throws GrpcException
	 */
	public NgLog(Properties localMachineInfo) throws GrpcException {
		/* set logLevel */
		try {
			logLevel[LOGCATEGORY_GLOBUS_TOOLKIT] =
				Integer.parseInt((String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_GT));
		} catch (NumberFormatException e) {
			logLevel[LOGCATEGORY_GLOBUS_TOOLKIT] = 
				getLogLevelFromString((String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_GT));
		}

		try {
			logLevel[LOGCATEGORY_NINFG_PROTOCOL] =
				Integer.parseInt((String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_NGPROT));
		} catch (NumberFormatException e) {
			logLevel[LOGCATEGORY_NINFG_PROTOCOL] =
				getLogLevelFromString((String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_NGPROT));
		}

		try {
			logLevel[LOGCATEGORY_NINFG_INTERNAL] =
				Integer.parseInt((String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_NGINT));
		} catch (NumberFormatException e) {
			logLevel[LOGCATEGORY_NINFG_INTERNAL] =
				getLogLevelFromString((String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_NGINT));
		}
		
		try {
			logLevel[LOGCATEGORY_NINFG_GRPC] = Integer.parseInt(
				(String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_NGGRPC));
		} catch (NumberFormatException e) {
			logLevel[LOGCATEGORY_NINFG_GRPC] =
				getLogLevelFromString((String)localMachineInfo.get(
				NgInformationManager.KEY_CLIENT_LOGLEVEL_NGGRPC));
		}
		
		/* set PrintStream */
		logFileName = (String) localMachineInfo.get(
			NgInformationManager.KEY_CLIENT_LOG_FILEPATH);

		/* there's no specified file */
		if (logFileName == null) {
			/* put message to Sys.err */
			return;
		}

		/* check "%t" and replace to String of time */
		int index = logFileName.indexOf(STR_TIME);
		if (index != -1) {
			replaceTimeString();
		}
		/* check "%h" and replace to String of hostname */
		index = logFileName.indexOf(STR_HOSTNAME);
		if (index != -1) {
			replaceHostString();
		}
		/* check "%p" and replace to String of process */
		index = logFileName.indexOf(STR_PROCESS);
		if (index != -1) {
			replaceProcessString();
		}
		/* check "%%" and replace to String of percent */
		index = logFileName.indexOf(STR_PERCENT);
		if (index != -1) {
			replacePercentString();
		}

		/* check if fileSuffix was supplied */
		logFileSuffix = (String) localMachineInfo.get(
			NgInformationManager.KEY_CLIENT_LOG_SUFFIX);
		
		/* set nFiles */
		String nFilesString = (String) localMachineInfo.get(
			NgInformationManager.KEY_CLIENT_LOG_NFILES);
		if (nFilesString != null) {
			nFiles = Integer.parseInt(nFilesString);
		}
		if (nFiles < 0) {
			throw new NgInitializeGrpcClientException("nFiles is invalid.");
		}
		/* set sequence format */
		setSequenceFormat();
		/* set maxFileSize */
		String maxFileSizeString = (String) localMachineInfo.get(
			NgInformationManager.KEY_CLIENT_LOG_MAXFILESIZE);
		if (maxFileSizeString != null) {
			maxFileSize = Integer.parseInt(maxFileSizeString);
		} else {
			/* unlimited */
			maxFileSize = 0;
		}
		/* set boolOverwriteDir */
		String overwriteDirString = (String) localMachineInfo.get(
			NgInformationManager.KEY_CLIENT_LOG_OVERWRITEDIR);
		if (overwriteDirString != null) {
			boolOverwriteDir = Boolean.valueOf(overwriteDirString).booleanValue();
		}
		
		/* set the name of client */
		this.localhostName = Util.getLocalHostAddress();
		
		/* open logfile */
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
	public NgLog(RemoteMachineInfo remoteMachineInfo,
		int executableID) throws GrpcException {
		/* NgLog for Communication Log */
		/* set default logLevel for Communication Log */
		logLevel[LOGCATEGORY_GLOBUS_TOOLKIT] = LOGLEVEL_DEBUG;
		logLevel[LOGCATEGORY_NINFG_INTERNAL] = LOGLEVEL_DEBUG;
		logLevel[LOGCATEGORY_NINFG_GRPC] = LOGLEVEL_DEBUG;
		logLevel[LOGCATEGORY_NINFG_PROTOCOL] = LOGLEVEL_DEBUG;

		/* set PrintStream */
		logFileName = (String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMMLOG_FILEPATH) +
			"-execID-" + executableID;

		/* there's no specified file, then put message to Sys.err */
		if (logFileName == null) {
			return;
		}

		/* check "%t" and replace to String of time */
		int index = logFileName.indexOf(STR_TIME);
		if (index != -1) {
			replaceTimeString();
		}
		/* check "%h" and replace to String of hostname */
		index = logFileName.indexOf(STR_HOSTNAME);
		if (index != -1) {
			replaceHostString();
		}
		/* check "%p" and replace to String of process */
		index = logFileName.indexOf(STR_PROCESS);
		if (index != -1) {
			replaceProcessString();
		}
		/* check "%%" and replace to String of percent */
		index = logFileName.indexOf(STR_PERCENT);
		if (index != -1) {
			replacePercentString();
		}
		
		/* check if fileSuffix was supplied */
		logFileSuffix = (String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMMLOG_SUFFIX);
		
		/* set nFiles */
		String nFilesString = (String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMMLOG_NFILES);
		if (nFilesString != null) {
			nFiles = Integer.parseInt(nFilesString);
		}
		if (nFiles < 0) {
			throw new NgInitializeGrpcClientException("nFiles is invalid.");
		}
		/* set sequence format */
		setSequenceFormat();
		/* set maxFileSize */
		String maxFileSizeString = (String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMMLOG_MAXFILESIZE);
		if (maxFileSizeString != null) {
			maxFileSize = Integer.parseInt(maxFileSizeString);
		} else {
			/* unlimited */
			maxFileSize = 0;
		}
		/* set boolOverwriteDir */
		String overwriteDirString = (String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMMLOG_OVERWRITEDIR);
		if (overwriteDirString != null) {
			boolOverwriteDir = Boolean.valueOf(overwriteDirString).booleanValue();
		}
		
		/* open logfile */
		try {
			openLogFile();
		} catch (GrpcException e) {
			throw new NgInitializeGrpcHandleException(e);
		}
	}
	
	/**
	 * @return 
	 */ 
	private String getLogFileSuffix() {
		if (logFileSuffix == null) return "";
		return "." + logFileSuffix;
	}
	private String getSequence() {
		if (nFiles == 1) return "";
		DecimalFormat df = new DecimalFormat(this.sequenceFormat);
		return df.format(processFiles).toString();
	}

	/**
	 * @return suffix using log_nFiles and log_suffix
	 */
	private String makeSuffix() {
		String seq = getSequence();
		String suf = getLogFileSuffix();
		if (seq.equals("")) return suf;
		if (suf.equals("")) {
			return "." + seq;
		} else {
			return "-" + seq + suf;
		}
	}
	/**
	 * @return
	 */
	private String makeLogFileName() {
		return logFileName + makeSuffix();
	}
	
	/**
	 * @return
	 */
	private void setSequenceFormat() {
		StringBuffer sequenceFormat = new StringBuffer();
		if (nFiles == 0) {
			sequenceFormat.append("0000000000");
		} else {
			int tmpNFiles = nFiles - 1;
			do {
				sequenceFormat.append("0");
				tmpNFiles /= 10;
			} while (tmpNFiles > 0);
		}
		
		this.sequenceFormat = sequenceFormat.toString();
	}
	
	/**
	 * 
	 */
	private void replaceTimeString() {
		Calendar calendar = Calendar.getInstance();
		DecimalFormat df = new DecimalFormat("00");
		DecimalFormat dfMs = new DecimalFormat("000");

		String dirName = String.valueOf(
			calendar.get(java.util.Calendar.YEAR) +
			df.format(calendar.get(java.util.Calendar.MONTH) + 1) +
			df.format(calendar.get(java.util.Calendar.DATE)) +	"-" +
			df.format(calendar.get(java.util.Calendar.HOUR_OF_DAY)) +
			df.format(calendar.get(java.util.Calendar.MINUTE)) +
			df.format(calendar.get(java.util.Calendar.SECOND)) + "-" +
			dfMs.format(calendar.get(java.util.Calendar.MILLISECOND))
			);
		logFileName = logFileName.replaceAll(STR_TIME, dirName);
	}
	
	/**
	 * 
	 */
	private void replaceHostString() throws GrpcException {
		logFileName =
			logFileName.replaceAll(STR_HOSTNAME, Util.getLocalHostAddress());
	}
	
	/**
	 * 
	 */
	private void replaceProcessString() throws GrpcException {
		System.err.println (
				"%p is not supported for the name of logfile on Ninf-G Java Client");
		//logFileName = logFileName.replaceAll(STR_PROCESS, getPID());
	}
	
	/**
	 * 
	 */
	private void replacePercentString() throws GrpcException {
		logFileName = logFileName.replaceAll(STR_PERCENT, "%");
	}
	
	private void makeLogDirectory(File logdir) 
	   throws NgInitializeGrpcClientException {
		if (logdir == null) return;

		if ( (! boolOverwriteDir) && logdir.exists() )
			throw new NgInitializeGrpcClientException("Can't overwrite log directory.");

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
		if (logFileName == null) { return; } /* put to System.err */

		String targetLogFile = makeLogFileName();

		try {
			// create dierctory to save log file 
			File parentDir = new File(targetLogFile).getParentFile();
			if (parentDir != null)
				makeLogDirectory(parentDir);
			// open log file 
			out = new PrintStream(new FileOutputStream(targetLogFile));
		} catch (FileNotFoundException e) {
			throw new NgInitializeGrpcClientException(e);
		} catch (NgInitializeGrpcClientException e) {
			throw e;
		}
		
		/* reset nWrite */
		nWrite = 0;
		
		/* increment count of process files */
		if ((nFiles != 0) && (processFiles >= (nFiles - 1))) {
			processFiles = 0;
		} else {
			processFiles++;
		} 
	}
	
	/**
	 * @throws GrpcException
	 */
	private void closeLogFile() throws GrpcException {
		out.close();
	}
	
	/**
	 * @return
	 */
	private boolean isFileOverLimit() {
		if ((maxFileSize != 0) && (nWrite > maxFileSize)) {
			return true;
		} else {
			return false;
		}
	}
	
	/**
	 * @param e
	 */
	private synchronized void printLog(GrpcException e) {
		if (isFileOverLimit()) {
			/* change Logfile */
			try {
				closeLogFile();
				openLogFile();
			} catch (GrpcException e1) {
				/* can't manage */
			}
		}
		
		/* put log message */
		String logMessage = e.toString();
		out.println (logMessage);
		nWrite += logMessage.length();
		
		/* put stack trace */
		PrintStream osOrig = e.os; 
		e.os = out;
		e.printStackTrace();
		e.os = osOrig;
	}
	
	/**
	 * @param msg
	 */
	private synchronized void printLog(String category, String msg) {
		if (isFileOverLimit()) {
			/* change Logfile */
			try {
				closeLogFile();
				openLogFile();
			} catch (GrpcException e1) {
				/* can't manage */
			}
		}
		
		/* put log message */
		String logMessage = new Date() + " : Client : " +
			localhostName + ": " + category + ": " + msg;
		out.println (logMessage);
		nWrite += logMessage.length();
	}
	
	/**********   printLog Interfaces   **********/
	/**
	 * @param category
	 * @param level
	 * @param e
	 */
	protected void printLog(int category, int level, GrpcException e) {
		if (logLevel[category] >= level) {
			printLog(e);
		}
	}
	
	/**
	 * @param category
	 * @param level
	 * @param msg
	 */
	private void printLog(int category, int level, String msg) {
		if (logLevel[category] >= level) {
			printLog(logLevelString[level], msg);
		}
	}
	
	/**
	 * @param category
	 * @param level
	 * @param client
	 * @param msg
	 */
	public void printLog(int category, int level, NgGrpcClient client, String msg) {
		if (logLevel[category] >= LOGLEVEL_DEBUG) {
			printLog(category, level,
				getIDString(client) + getThreadIDString() + msg);
		} else {
			printLog(category, level, getIDString(client) + msg);
		}
	}
	
	/**
	 * @param category
	 * @param level
	 * @param handle
	 * @param msg
	 */
	public void printLog(int category, int level, NgGrpcHandle handle, String msg) {
		if (logLevel[category] >= LOGLEVEL_DEBUG) {
			printLog(category, level,
				getIDString(handle) + getThreadIDString() + msg);
		} else {
			printLog(category, level, getIDString(handle) + msg);
		}
	}
	
	/**
	 * @param category
	 * @param level
	 * @param handle
	 * @param msg
	 */
	public void printSessionLog(int category, int level, NgGrpcHandle handle, String msg) {
		if (logLevel[category] >= LOGLEVEL_DEBUG) {
			printLog(category, level,
				getSessionIDString(handle) + getThreadIDString() + msg);
		} else {
			printLog(category, level, getSessionIDString(handle) + msg);
		}
	}
	
	/**
	 * @param strLoglevel
	 */
	private int getLogLevelFromString(String strLoglevel) throws GrpcException {
		int strLogLevelLength = strLoglevel.length();
		String target;

		/* off */
		if (LOGLEVEL_OFF_STR.length() >= strLogLevelLength) {
			target =
				LOGLEVEL_OFF_STR.substring(0, strLogLevelLength);
			if (strLoglevel.compareTo(target) == 0) {
				return LOGLEVEL_OFF;
			}
		}
		/* fatal */
		if (LOGLEVEL_FATAL_STR.length() >= strLogLevelLength) {
			target =
				LOGLEVEL_FATAL_STR.substring(0, strLogLevelLength);
			if (strLoglevel.compareTo(target) == 0) {
				return LOGLEVEL_FATAL;
			}
		}
		/* error */
		if (LOGLEVEL_ERROR_STR.length() >= strLogLevelLength) {
			target =
				LOGLEVEL_ERROR_STR.substring(0, strLoglevel.length());
			if (strLoglevel.compareTo(target) == 0) {
				return LOGLEVEL_ERROR;
			}
		}
		/* warning */
		if (LOGLEVEL_WARN_STR.length() >= strLogLevelLength) {
			target =
				LOGLEVEL_WARN_STR.substring(0, strLoglevel.length());
			if (strLoglevel.compareTo(target) == 0) {
				return LOGLEVEL_WARN;
			}
		}
		/* info */
		if (LOGLEVEL_INFO_STR.length() >= strLogLevelLength) {
			target = LOGLEVEL_INFO_STR.substring(0,strLogLevelLength);
			if (strLoglevel.compareTo(target) == 0) {
				return LOGLEVEL_INFO;
			}
		}
		/* debug */
		if (LOGLEVEL_DEBUG_STR.length() >= strLogLevelLength) {
			target =
				LOGLEVEL_DEBUG_STR.substring(0, strLoglevel.length());
			if (strLoglevel.compareTo(target) == 0) {
				return LOGLEVEL_DEBUG;
			}
		}

		throw new NgInitializeGrpcClientException(
			"NgLog#getLogLevelFromString: unrecognize parameter for loglevel.");
	}

	/**
	 * @param val
	 * @return
	 */
	public static boolean validLogLevelParam(String val) {
		try {
			int logLevel = Integer.parseInt(val);
			if ((logLevel > -1) && (logLevel < 6)) {
				return true;
			}
		} catch (NumberFormatException e) {
			if (val.equals(LOGLEVEL_OFF_STR) ||
				val.equals(LOGLEVEL_FATAL_STR) ||
				val.equals(LOGLEVEL_ERROR_STR) ||
				val.equals(LOGLEVEL_INFO_STR) ||
				val.equals(LOGLEVEL_WARN_STR) ||
				val.equals(LOGLEVEL_DEBUG_STR)) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * @param client
	 * @return
	 */
	private String getIDString(NgGrpcClient client) {
		return "Context: " + client.getID() + ": ";
	}
	
	/**
	 * @param handle
	 * @return
	 */
	private String getIDString(NgGrpcHandle handle) {
		return getIDString(handle.getContext()) +
			"Executable: " + handle.getID() + ": ";
	}
	
	/**
	 * @param handle
	 * @return
	 */
	private String getSessionIDString(NgGrpcHandle handle) {
		return getIDString(handle) + 
			"Session: " + handle.getSessionID() + ": ";
	}
	
	/**
	 * @return
	 */
	private String getThreadIDString() {
		return "Thread: " + Thread.currentThread().getName() + ": ";
	}
	
	/**
	 * @param msg
	 */
	protected synchronized void printCommLog(
		String sendOrReceive, int size,	String msg) {
		/* update last time of communicate */
		long origLastSendReceiveTime = this.lastPrintTime;
		this.lastPrintTime = System.currentTimeMillis();

		/* rotate log file */
		if (isFileOverLimit()) {
			/* change Logfile */
			try {
				closeLogFile();
				openLogFile();
			} catch (GrpcException e1) {
				/* can't manage */
			}
		}
		
		/* make commLog string */
		StringBuffer sb = new StringBuffer();
		/* append Executable / Client information */
		sb.append("Executable: " + serverName + ", Client: " + clientName);
		/* append past time */
		sb.append("  " + getPastTime(this.lastPrintTime - origLastSendReceiveTime) + "  ");
		/* append bytes of send or receive */
		sb.append(sendOrReceive + ": " + size + "bytes");
		sb.append("\n");
		
		/* append commLog information */
		sb.append(msg);
		
		/* put log message */
		String logMessage = new Date() + " " + sb.toString();
		out.println (logMessage);
		nWrite += logMessage.length();
	}
	
	/**
	 * @return
	 */
	private String getPastTime(long pastTimeMS) {
		return "(  " + pastTimeMS / 1000 + "s   " + pastTimeMS % 1000 + "ms   0us)";
	}
	
	/**
	 * @param serverName
	 */
	protected void setServerName(String serverName) {
		this.serverName = serverName;
	}
	
	/**
	 * @param clientName
	 */
	protected void setClientName(String clientName) {
		this.clientName = clientName;
	}
}
