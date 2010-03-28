package com.fujitsu.arcon.common;

import java.util.HashSet;
import java.util.Set;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 30, 2003
 */
public class FileReaderFactory implements ReaderFactory {

	private FileTransferEngine fte;

	private int limit;

	private Logger logger;

	private Set all_readers = new HashSet();

	public FileReaderFactory(int limit, Logger logger) {
		this.limit = 1;
		//UPLD
		//UPLD
		this.logger = logger;
	}

	private static int counter = 0;

	public synchronized boolean makeReader() {
		if (all_readers.size() < limit) {
			FileReader fr = new FileReader(fte, this, logger);
			all_readers.add(fr);
			(new Thread(fr, "ConcurrentFTReader-" + counter++)).start();
			logger.logComment("Creating new file reader " + counter);
			return true;
		} else {
			return false;
		}
	}

	public synchronized void notifyTerminated(Reader r) {
		all_readers.remove(r);
		logger.logComment("File reader terminated");
		if (all_readers.isEmpty()) {
			all_terminated = true;
			fte.allReadersTerminated();
			fte = null;
			logger.logComment("All file readers terminated");
		}
	}

	private boolean all_terminated = false;

	public boolean allTerminated() {
		return all_terminated;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ReaderFactory#setFileTransferEngine(FileTransferEngine)
	 */
	public void setFileTransferEngine(FileTransferEngine fte) {
		this.fte = fte;
	}

	public boolean canMakeWriter() {
		// Always happy to have more Writers
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
