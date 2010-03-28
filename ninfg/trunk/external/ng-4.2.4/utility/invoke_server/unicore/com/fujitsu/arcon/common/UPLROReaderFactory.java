package com.fujitsu.arcon.common;

import java.io.IOException;

import org.unicore.AJOIdentifier;
import org.unicore.upl.Reply;
import org.unicore.upl.RetrieveOutcome;
import org.unicore.upl.RetrieveOutcomeReply;

//UPLD

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Nov 10, 2003
 */
public class UPLROReaderFactory implements ReaderFactory {

	private FileTransferEngine fte;

	private int limit;

	private Logger logger;

	private ConnectionFactory cf;

	AJOIdentifier target;

	RetrieveOutcome ro;

	private int made_reader_count = 0; // All created UPLReaders

	Connection initial_connection;

	public UPLROReaderFactory(RetrieveOutcome ro, int limit, Logger logger,
			ConnectionFactory cf) throws IOException {

		this.ro = ro;
		this.target = ro.getTarget();
		this.limit = 1;
		//UPLD
		//UPLD
		this.logger = logger;
		this.cf = cf;

		// Send the RO and get the reply to see if there is any data
		initial_connection = cf.connect();

		ro_reply = initial_connection.sendServerRequest(ro);

		if (ro_reply instanceof RetrieveOutcomeReply) {
			RetrieveOutcomeReply ror = (RetrieveOutcomeReply) ro_reply;
			if (ror.hasStreamed()) {
				ok_to_try = true;
			} else {
				ok_to_try = false;
			}
		} else {
			// is a Reply, thus error, so no streamed data
			ok_to_try = false;
		}
	}

	private boolean ok_to_try = true;

	public boolean hasDataToStream() {
		return ok_to_try;
	}

	// hold reply from ConsignJob which comes back eventually
	private Reply ro_reply;

	public Reply getRetrieveOutcomeReply() {
		return ro_reply;
	}

	public void setRetrieveOutcomeReply(Reply r) {
		ro_reply = r;
	}

	public synchronized boolean makeReader() {

		if (allTerminated())
			return false;

		//UPLD
		if (made_reader_count == 0 && ok_to_try) {
			logger
					.logComment("Creating initial UPL RO reader (using RetrieveOutcome)");
			made_reader_count = 1;
			UPLReader ur = new UPLReader(fte, initial_connection, this, logger);
			(new Thread(ur, "UPL RO Reader-" + made_reader_count)).start();
			return true;
		}
		//UPLD
		//			
		//				
		//			
		//			
		//			
		//				
		//				
		//				
		//				
		//					
		//				
		//					
		//					
		//				
		//			
		//			
		//				
		//					
		//				
		//					
		//					
		//				
		//			
		//UPLD
		else {
			ok_to_try = false;
			return false;
		}
		//UPLD
		//UPLD
		//UPLD
		//UPLD
		//UPLD
	}

	public synchronized void notifyTerminated(Reader r) {
		made_reader_count--;
		logger.logComment("UPL RO reader terminated, " + made_reader_count
				+ " left.");
		if (made_reader_count <= 0) {
			logger.logComment("All UPL RO readers terminated");
			all_terminated = true;
			fte.allReadersTerminated();
			fte = null;
		}
	}

	private boolean all_terminated;

	public boolean allTerminated() {
		return all_terminated;
	}

	public void setFileTransferEngine(FileTransferEngine fte) {
		this.fte = fte;
	}

	public boolean canMakeWriter() {
		// Only in parallel
		return made_reader_count != 1;
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
