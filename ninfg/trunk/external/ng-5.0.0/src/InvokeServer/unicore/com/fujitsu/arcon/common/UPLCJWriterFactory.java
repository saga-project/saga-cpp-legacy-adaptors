package com.fujitsu.arcon.common;

import java.io.IOException;
import java.util.HashSet;
import java.util.Set;

import org.unicore.AJOIdentifier;
import org.unicore.upl.ConsignJob;
//UPLD
import org.unicore.upl.Reply;

//UPLD
/**
 * //UPLD //UPLD
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Aug 12, 2003
 */
public class UPLCJWriterFactory implements WriterFactory {

	private FileTransferEngine fte;

	private int limit;

	private Set open_writers = new HashSet();

	private Set all_writers = new HashSet();

	private Logger logger;

	private ConnectionFactory cf;

	AJOIdentifier target;

	ConsignJob cj;

	public UPLCJWriterFactory(ConsignJob cj, int limit, Logger logger,
			ConnectionFactory cf) {
		this.cj = cj;
		this.target = cj.getTarget();
		this.limit = 1;
		//UPLD
		//UPLD
		this.logger = logger;
		this.cf = cf;
	}

	//UPLD

	private static int counter = 0; // counts total created, not existing

	// hold reply from ConsignJob which comes back eventually
	private Reply cj_reply;

	public Reply getConsignJobReply() {
		return cj_reply;
	}

	public void setConsignJobReply(Reply r) {
		cj_reply = r;
	}

	public boolean makeWriter() {

		if (allTerminated())
			return false;

		try {
			if (all_writers.isEmpty()) {
				logger.logComment("Creating initial UPL writer (ConsignJob)");
				Connection connection = cf.connect();
				UPLWriter.Initial uw = new UPLWriter.Initial(cj, fte, this,
						open_writers, logger, connection);
				all_writers.add(uw);
				(new Thread(uw, "ConcurrentUPLWriter-" + counter++)).start();
				//UPLD
				//UPLD
				try {
					Thread.sleep(2000);
				} catch (Exception ex) {
				}
				return true; // don't know yet
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
			//UPLD
			else {
				return false;
			}

		} catch (IOException ioex) {
			logger.logError("IO Error creating a UPLWriter: "
					+ ioex.getMessage());
			return false;
		}
	}

	public void notifyTerminated(Writer r) {

		logger.logComment("UPL Writer terminated");

		all_writers.remove(r);

		if (all_writers.isEmpty()) {
			all_terminated = true;
			fte.allWritersTerminated();
			fte = null;
			logger.logComment("All UPL Writers terminated");
		}

	}

	private boolean all_terminated = false;

	public boolean allTerminated() {
		return all_terminated;
	}

	public void setFileTransferEngine(FileTransferEngine fte) {
		this.fte = fte;
	}

	public boolean canMakeReader() {
		// Only if parallel
		return all_writers.size() != 1;
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
