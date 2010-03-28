package com.fujitsu.arcon.common;

import java.util.HashSet;
import java.util.Set;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Nov 7, 2003
 */
public class UPLROWriterFactory implements WriterFactory {

	private FileTransferEngine fte;

	int limit; // maximum number of connections to accept

	Logger logger;

	private Set all_writers = new HashSet();

	private Set open_writers = new HashSet();

	public UPLROWriterFactory(int limit, Logger logger) {
		this.limit = 1;
		//UPLD
		//UPLD
		this.logger = logger;
	}

	public void setFileTransferEngine(FileTransferEngine fte) {
		this.fte = fte;
	}

	private int inactive = 0;

	public synchronized UPLWriter acceptConnection(Connection connection) {
		if (all_writers.size() < limit && !all_terminated) {
			UPLWriter wr = new UPLWriter(fte, this, connection, open_writers,
					logger);
			logger.logComment("Creating new UPL Writer, " + all_writers.size()
					+ " active.");
			all_writers.add(wr);
			inactive++;
			return wr;
		} else {
			return (UPLWriter) null;
		}
	}

	public synchronized boolean makeWriter() {
		if (inactive > 0) {
			inactive--;
			return true;
		} else {
			return false;
		}
	}

	public synchronized void notifyTerminated(Writer r) {
		synchronized (all_writers) { // erk - two lock => possible deadlock!
			all_writers.remove(r);
		}
		logger.logComment("UPL writer terminated, " + all_writers.size()
				+ " left.");
		if (all_writers.isEmpty()) {
			logger.logComment("All UPL writers terminated");
			all_terminated = true;
			fte.allWritersTerminated();
			fte = null;
		}
	}

	private boolean all_terminated = false;

	public boolean allTerminated() {
		return all_terminated;
	}

	public boolean canMakeReader() {
		return true;
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
