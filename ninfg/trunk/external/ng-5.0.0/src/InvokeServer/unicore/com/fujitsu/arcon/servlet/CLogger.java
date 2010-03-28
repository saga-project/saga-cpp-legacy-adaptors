package com.fujitsu.arcon.servlet;

import java.io.*;

/**
 * Message logging. Any number of different destinations can be set. To ignore
 * logging messages do not add a destination.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 *  
 */
public class CLogger {

	public static interface LineP {
		public void print(String s);
	}

	private final static LoggerStatus status = new LoggerStatus();

	/**
	 * Add a new destination for status level messages.
	 *  
	 */
	public static void addStatus(PrintStream ps) {
		status.add(new PSWrapper(ps));
	}

	/**
	 * Add a new destination for status level messages.
	 *  
	 */
	public static void addStatus(CLogger.LineP l) {
		status.add(l);
	}

	/**
	 * Log a status-level message.
	 *  
	 */
	public static void status(String s) {
		status.println(s);
	}

}

class LoggerStatus {

	CLogger.LineP[] destinations = new CLogger.LineP[0];

	public void add(CLogger.LineP l) {
		CLogger.LineP[] temp = new CLogger.LineP[destinations.length + 1];
		for (int i = 0; i < destinations.length; i++) {
			temp[i] = destinations[i];
		}
		temp[temp.length - 1] = l;
		destinations = temp;
	}

	public void println(String s) {
		for (int i = 0; i < destinations.length; i++) {
			try {
				destinations[i].print(s);
			} catch (Exception ex) {
			}
		}
	}
}

class PSWrapper implements CLogger.LineP {
	private PrintStream ps;

	public PSWrapper(PrintStream ps) {
		this.ps = ps;
	}

	public void print(String s) {
		ps.println(s);
	}
}
//
//                   Copyright (c) Fujitsu Ltd 2000 - 2004
//
//                Use and distribution is subject a License.
// A copy was supplied with the distribution (see documentation or the jar
// file).
//
// This product includes software developed by Fujitsu Limited
// (http://www.fujitsu.com).
