package com.fujitsu.arcon.common;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Aug 12, 2003
 */
public class UPLCJReaderFactory implements ReaderFactory {

	private FileTransferEngine fte;

	int limit; // maximum number of connections to accept

	Logger logger;

	private int made_reader_count = 0; // All created UPLReaders

	private int active_reader_count = 0;

	public UPLCJReaderFactory(int limit, Logger logger) {
		this.limit = 1;
		//UPLD
		//UPLD
		this.logger = logger;
	}

	public void setFileTransferEngine(FileTransferEngine fte) {
		this.fte = fte;
	}

	// NOTE: these need to be synched as acceptConnection is not called from
	// within the FTE
	//       synch blocks
	/**
	 * The Readers created by this factory are driven by incoming requests. The
	 * Factory will accept these requests, and create a UPLReader for each,
	 * until the limit of active Readers is reached. The returned Raeders are
	 * not active, they need to have their run method called to start processing -
	 * usually happens when the Receptionist passes control to them (need this
	 * to allow a UPL Reply before reading the incoming data).
	 */
	public synchronized UPLReader acceptConnection(Connection connection) {
		if (made_reader_count + active_reader_count < limit && !all_terminated) {
			UPLReader ur = new UPLReader(fte, connection, this, logger);
			made_reader_count++;
			logger.logComment("Creating new UPL Reader, " + active_reader_count
					+ " active, " + made_reader_count + " waiting."); //+counter);
			return ur;
		} else {
			return (UPLReader) null;
		}
	}

	public synchronized boolean makeReader() {
		if (made_reader_count > 0) {
			made_reader_count--;
			active_reader_count++;
			logger.logComment("Activating UPL Reader, " + active_reader_count
					+ " active, " + made_reader_count + " waiting.");
			return true;
		} else {
			return false;
		}
	}

	public synchronized void notifyTerminated(Reader r) {
		active_reader_count--;
		logger.logComment("UPL reader terminated, " + active_reader_count
				+ " left.");
		if (active_reader_count <= 0) {
			logger.logComment("All UPL readers terminated");
			all_terminated = true;
			fte.allReadersTerminated();
			fte = null;
		}
	}

	private boolean all_terminated;

	public boolean allTerminated() {
		return all_terminated;
	}

	public boolean canMakeWriter() {
		// This is at the server side, so always happy for new Writers since
		// we can't mess up buffers here?
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
