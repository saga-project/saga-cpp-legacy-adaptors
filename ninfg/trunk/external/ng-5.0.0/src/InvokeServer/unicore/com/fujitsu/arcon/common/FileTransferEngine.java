package com.fujitsu.arcon.common;

import java.util.LinkedList;

/**
 * Controls the concurrent transfer of a batch of files (from, for example a
 * Portfolio). The transfer details are handled by the instances of Readers and
 * Writers created by the factories. A FileTransferEngine initiates the dynamic
 * creation of new Writers and Readers as necessary, while limited by the
 * policies of therir respective factories. Part of the function of a
 * FileTransferEngine is to buffer read bytes for the writers, these may be
 * large and there may be a large number of these, so FileTransferEngines are
 * created by a Factory that can, one day, keep a central view of this resource.
 * <p>
 * The direction of transfer and the file stransferred depends on the instances
 * returned by the factories and is not controlled by a FileTransferEngine.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 28, 2003
 */
public class FileTransferEngine {

	public static interface Requestor {
		public void transferDone();

		public boolean done();
	}

	public static FileTransferEngine create(ReaderFactory rf, WriterFactory wf,
			ChunkManager cm, Requestor r, Logger logger, int buffer_size) {
		FileTransferEngine fte = new FileTransferEngine(rf, wf, cm, r, logger,
				buffer_size);
		rf.setFileTransferEngine(fte);
		wf.setFileTransferEngine(fte);
		return fte;
	}

	private boolean waiting_for_last_chunks; // all read chunks allocated,

	// waiting for reads of them to
	// complete

	private ReaderFactory reader_factory;

	private WriterFactory writer_factory;

	private ChunkManager chunk_manager;

	private Requestor requestor;

	private Logger logger;

	private int buffer_size;

	/**
	 *  
	 */
	private FileTransferEngine(ReaderFactory rf, WriterFactory wf,
			ChunkManager cm, Requestor r, Logger logger, int buffer_size) {
		waiting_for_last_chunks = false;
		empty_buffers = new LinkedList(); // no need for synched, as called from
		// synched methods
		full_chunks = new LinkedList(); // ditto

		reader_factory = rf;
		writer_factory = wf;

		chunk_manager = cm;
		requestor = r;

		this.buffer_size = buffer_size;

		this.logger = logger;
	}

	/**
	 * Try to add a Writer to the active set.
	 * 
	 * @param w
	 */
	public synchronized void addWriter() {
		if (reader_factory.canMakeWriter() && writer_factory.makeWriter()) {
			addBuffer();
		}
	}

	/**
	 * Try to add a Reader to the active set.
	 */
	public synchronized void addReader() {
		if (writer_factory.canMakeReader() && reader_factory.makeReader()) {
			addBuffer();
		}
	}

	/**
	 * Start the transfer up (requires that at least one Reader and one Writer
	 * will start up in this call, if not the wholeprocess is shut down).
	 *  
	 */
	public synchronized void doTransfer() {
		// add a Writer (so that there is something to write single buffer
		// reads)
		if (writer_factory.makeWriter()) {
			// Start one reading, the others should grow from this
			if (!reader_factory.makeReader()) {
				logger
						.logError("First Reader failed to start, aborting file transfer");
				error_found = true;
				allWritersTerminated(); // leaves a Writer hanging, and a
				// Thread, self-referential loop
				// to fte, hopefully this will get garbage collected. If not,
				// will
				// have to kick to completion using notifyAll and replace RF, WF
				// and CM
				// by dummies that force witer to complete
			}
		} else {
			logger
					.logError("First Writer failed to start, aborting file transfer");
			error_found = true;
			allWritersTerminated(); // Nothing go started so this is OK
		}
		addBuffer();
		addBuffer();
	}

	private boolean error_found = false;

	private String main_error_message = "All OK";

	private Exception offender;

	/**
	 * Call after transfer is complete (see {@link FileTransferEngine.Requestor}).
	 * The offending error message can be found from {@link #errorMessage}.
	 * 
	 * @return True if transfer was OK, false otherwise
	 */
	public boolean transferOK() {
		return !error_found;
	}

	/**
	 * Message from the error that failed the transfer (if any).
	 * 
	 * @return
	 */
	public String errorMessage() {
		return main_error_message;
	}

	/**
	 * Return the Exception causing the main error.
	 * 
	 * @return
	 */
	public Exception getCause() {
		return offender;
	}

	/**
	 * A Reader, Writer or Factory encountered some unrecoverable error, shut
	 * down the transfer. There is no particular reason to be drastic about it.
	 * Let current writes continue and try to clear read buffers. Obviously, a
	 * Writer error may lead to multiple reports so suppress some.
	 *  
	 */
	public void notifyError(Chunk chunk, Exception e) {

		if (!error_found) {
			main_error_message = "Error while transferring chunk of <"
					+ (chunk != null ? chunk.getFileName() : "")
					+ "> starting at <"
					+ (chunk != null ? chunk.getStartByte() : 0) + "> length <"
					+ (chunk != null ? chunk.getChunkLength() : 0) + "> ";
			logger.logError(main_error_message, e);

			main_error_message += e.getMessage();
			offender = e;
			error_found = true;
		} else {
			logger.logError("Additional error for <"
					+ (chunk != null ? chunk.getFileName() : "") + "> "
					+ e.getMessage());
		}

		// Stop issuing new reads, eventually the buffers will get
		// cleared and this should shut down. Allow current reads to complete?
		//
		// Problem: If the Writes are failing:
		// 1) then we could run out of Writers before clearing buffers
		//      OK, all Readers will terminate anyway, the FTE just needs to clean up
		//
		// 2) also terminate all Writers before Readers => do not detect end
		//      OK, see allWritersTerminated below
	}

	/**
	 * A Reader has finished reading a chunk (buffer's worth) of the files. This
	 * buffer will be stored by the Engine (so that it can be passed onto a
	 * Writer). The ChunkManager will be called to find the next section of a
	 * file for the Reader to get. The Reader is passed the new chunk and a
	 * buffer before return (unless there is nothing left to do, in this case
	 * the Reader is terminated).
	 * 
	 * @param r
	 */
	public void finishedARead(Reader r) {

		boolean terminate_r = false;

		synchronized (this) {

			storeFullChunk(r.getChunk());
			r.setChunk(null);

			if (waiting_for_last_chunks || error_found) {
				// All chunks have been parcelled out. This Reader can go away
				// for now.
				terminate_r = true;
			} else {
				// Grab a buffer first (so that the chunks get processed in
				// order - the
				// notifies on buffer wait are random, so chunk order could
				// change if allocated first)
				if (noEmptyBuffersAvailable()) {
					// Let's see if we can speed up writing by adding another
					// Writer
					addWriter();
				}

				byte[] buffer = getEmptyBuffer();
				if (buffer == null) {
					terminate_r = true;
				} else {

					// Set a chunk to read, the situation may have changed since
					// test (wait in getEmptyBuffer)
					// so there may not be a chunk
					chunk_manager.getNextChunk(r);

					if (r.getChunk() != null) {
						r.getChunk().setBuffer(buffer);
					} else {
						storeEmptyBuffer(buffer);
						terminate_r = true;
					}

					if (chunk_manager.noMoreChunks()) {
						waiting_for_last_chunks = true; // This was the last
						// chunk, change state
					}
				}
			}
		}

		// Outside of synched part in case this takes a while
		if (terminate_r)
			r.terminate(error_found);
	}

	/**
	 * Called by the ReaderFactory when all Readers terminated. Change to
	 * cleaning up Writers.
	 *  
	 */
	public synchronized void allReadersTerminated() {
		// There will be no more full buffers. This means that Writers
		// should no longer wait for them. Also need to wake any that
		// are waiting now, so ...
		notifyAll();
	}

	/**
	 * Usually implies that everything is done. Except where we are completing
	 * on error and all the Writers fail before the Readers are done - Readers
	 * will complete uselessly but harmelessly,
	 *  
	 */
	public synchronized void allWritersTerminated() {

		notifyAll(); // for readers waiting for free buffers that won't be
		// coming

		writer_factory = null;
		reader_factory = null;
		chunk_manager = null;
		// Created a lot of large buffers but have lost handles on them
		// (passed out with chunks which have disappeared). Problem?

		requestor.transferDone();

	}

	/**
	 * A Writer has written a chunk. Need to be allocated another one from the
	 * Engine's buffer.
	 * 
	 * @param w
	 */
	public synchronized void finishedAWrite(Writer w) {
		boolean terminate_w = false;

		if (w.getChunk() != null) {
			storeEmptyBuffer(w.getChunk().getBuffer());
			w.getChunk().setBuffer(null);
			w.setChunk(null);
		}

		if (noFullChunksAvailable()) {
			// There was no buffer available to be written
			if (reader_factory.allTerminated()) {
				terminate_w = true;
			} else {
				// There are some reads still going on, wait until one returns,
				// but
				// first take this as an indication that there may not be enough
				// Readers
				// and see if we can create one (if there is still data to read)
				if (!(waiting_for_last_chunks || error_found)) {
					addReader();
				}
				w.setChunk(getFullChunk());
			}

		} else {

			// This call waits for one to become available, note that we are
			// synched
			// and so the available chunks test is still valid here (no other
			// thread could have got
			// the available one).
			w.setChunk(getFullChunk()); // NOTE: Writer must expect a null chunk
			// (=> immediately finish write again)
		}

		if (terminate_w)
			w.terminate(error_found);

	}

	public synchronized void disposeWrittenChunk(Chunk c) {
		if (c != null) {
			storeEmptyBuffer(c.getBuffer());
			c.setBuffer(null);
		}
	}

	// Manage the buffers, accepting and doling out to threads that need.
	// Threads
	// wanting bufers are blocked in the get methods until one becomes
	// available.
	// In order to avoid deadlock a single monitor is shared for empty and full
	// (and, more importantly, for the whole FTE instance's methods). This
	// implies
	// that notifyAll is used (rather than notify) so that, say, a notify of the
	// availibility of a full buffer is not swallowed by an empty buffer waiter.
	// notifyAll also frees up waiting writers when the transfer is complete.
	//
	// The full buffers should be a FIFO so that we are more likely to be
	// contiguous.

	private LinkedList empty_buffers = new LinkedList();

	private LinkedList full_chunks = new LinkedList();

	private int buffer_count = 0;

	private void addBuffer() {
		storeEmptyBuffer(new byte[buffer_size]);
		//UPLD
	}

	private void storeFullChunk(Chunk chunk) {
		// This is not synched, relies on being called from within a synched
		// method
		if (chunk != null) {
			full_chunks.addLast(chunk);
			// Let one of the waiting Writers (if any) know about this. Note
			// that
			// this notify can never be lost as the caller of this is synched on
			// the
			// FTEngine instance and so a Writer cannot be going to wait
			// concurrently.
			// notifyAll to guarantee that any waiting Writers are informed (not
			// just a Reader).
			notifyAll();
			//UPLD
		} else {
			// Null is allowed, this is from a new Reader or perhaps one at end
			// (was waiting on a new empty but the transfer completed)
		}
	}

	private void storeEmptyBuffer(byte[] buffer) {
		// This is not synched, relies on being called from within a synched
		// method
		if (buffer != null) {
			empty_buffers.addLast(buffer);
			notifyAll();
			//UPLD
		} else {
			// Null is allowed, this is from a new Writer or perhaps one at end
			// (was waiting on a new empty but the transfer completed)
		}
	}

	private synchronized byte[] getEmptyBuffer() {
		while (noEmptyBuffersAvailable()) {
			if (writer_factory == null || writer_factory.allTerminated()) {
				return (byte[]) null; // no more will come, move to termination
			} else {
				try {
					wait();
				} catch (Exception ex) {
				}
			}
		}
		//UPLD

		return (byte[]) empty_buffers.removeFirst();
	}

	private synchronized Chunk getFullChunk() {

		while (noFullChunksAvailable()) {
			if (reader_factory == null || reader_factory.allTerminated()) {
				// Everything read so there will be no more full buffers, move
				// this thread on to completion
				return (Chunk) null;
			}
			try {
				wait();
			} catch (Exception ex) {
			} // wait for a return
		}

		// This is synched (through outer calls) so there must be one now
		Chunk chunk = (Chunk) full_chunks.removeFirst();
		//UPLD
		return chunk;

	}

	private boolean noFullChunksAvailable() {
		return full_chunks.size() == 0;
	}

	private boolean noEmptyBuffersAvailable() {
		return empty_buffers.size() == 0;
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
