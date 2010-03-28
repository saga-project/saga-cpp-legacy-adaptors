/*
 * $RCSfile: LogFile.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;
import java.text.DecimalFormat;

import org.apgrid.grpc.ng.info.*;
import org.apgrid.grpc.util.NgUtil;
import org.gridforum.gridrpc.GrpcException;

class LogFile {
	private static final String STR_TIME = "%t";
	private static final String STR_HOSTNAME = "%h";
	private static final String STR_PROCESS = "%p";
	private static final String STR_PERCENT = "%%";

	private static final int DEFAULT_NFILES = 1;
	private String sequenceFormat = null;
	private int nFiles;
	private int processFiles = 0;
	private DecimalFormat df = null;

	private String name;
	private String suffix;

	public LogFile(String anName, String anSuffix, String anNFiles)
	 throws GrpcException {
		this.name = anName;
		this.suffix = anSuffix;

		replaceMetaChar();

		int nf = DEFAULT_NFILES;
		if (anNFiles != null) {
			nf = Integer.parseInt(anNFiles);
			if (nf < 0) {
				throw new NgInitializeGrpcClientException("nFiles is invalid.");
			}
		}
		this.nFiles = nf;

		// set sequence format 
		initSequenceFormat();
		initFormatter();
	}

	private void replaceMetaChar() throws GrpcException {
		// check "%t" and replace to String of time
		replaceTimeString();
		// check "%h" and replace to String of hostname 
		replaceHostString();
		// check "%p" and replace to String of process 
		replaceProcessString();
		// check "%%" and replace to String of percent
		replacePercentString();
	}

	private void replaceTimeString() {
		if (name.indexOf(STR_TIME) < 0) { return; }

		Calendar calendar = Calendar.getInstance();
		DecimalFormat df = new DecimalFormat("00");
		DecimalFormat dfMs = new DecimalFormat("000");

		String dirName = String.valueOf(
			calendar.get(java.util.Calendar.YEAR)
			+ df.format(calendar.get(java.util.Calendar.MONTH) + 1) 
			+ df.format(calendar.get(java.util.Calendar.DATE)) + "-" 
			+ df.format(calendar.get(java.util.Calendar.HOUR_OF_DAY)) 
			+ df.format(calendar.get(java.util.Calendar.MINUTE)) 
			+ df.format(calendar.get(java.util.Calendar.SECOND)) + "-" 
			+ dfMs.format(calendar.get(java.util.Calendar.MILLISECOND)) );
		name = name.replaceAll(STR_TIME, dirName);
	}

	/**
	 * "%h" in log file name replaces the client host name.
	 */
	private void replaceHostString() throws GrpcException {
		if (name.indexOf(STR_HOSTNAME) < 0) { return; }
		name =
			name.replaceAll(STR_HOSTNAME, NgUtil.getLocalHostName());
	}

	private void replaceProcessString() throws GrpcException {
		if (name.indexOf(STR_PROCESS) < 0) { return ; }

		System.err.println("%p is not supported for the name of logfile on Ninf-G Java Client");
		//logFileName = logFileName.replaceAll(STR_PROCESS, getPID());
	} 

	private void replacePercentString() throws GrpcException {
		if (name.indexOf(STR_PERCENT) < 0) { return; }
		name = name.replaceAll(STR_PERCENT, "%");
	}

	private String getLogFileSuffix() {
		if (suffix == null) return "";
		return "." + suffix;
	}

	private String getSequence() {
		if (nFiles == 1) return "";
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
	 * @return Log File name
	 */
	public String getName() {
		return name + makeSuffix();
	}

	private void initSequenceFormat() {
		StringBuffer format = new StringBuffer();
		if (nFiles == 0) {
			format.append("0000000000");
		} else {
			int tmpNFiles = nFiles - 1;
			do {
				format.append("0");
				tmpNFiles /= 10;
			} while (tmpNFiles > 0);
		}
		
		this.sequenceFormat = format.toString();
	}

	private void initFormatter() {
		this.df = new DecimalFormat(sequenceFormat);
	}

	public void increase() {
		if ((nFiles != 0) && (processFiles >= (nFiles - 1))) {
			processFiles = 0;
		} else {
			processFiles++;
		} 
	}
}

