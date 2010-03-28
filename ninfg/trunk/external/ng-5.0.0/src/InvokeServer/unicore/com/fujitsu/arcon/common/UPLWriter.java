package com.fujitsu.arcon.common;

import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Set;
import java.util.zip.ZipEntry;

import org.unicore.upl.ConsignJob;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Aug 12, 2003
 */
public class UPLWriter implements Writer, Runnable {

	protected Set open_writers; // All peer UPLWriters so that we can hand off
								// contiguous chunks to them

	protected Chunk chunk;

	protected boolean something_to_do;

	protected FileTransferEngine fte;

	protected WriterFactory wf;

	protected Logger logger;

	protected WriterChunk me_as_writer_chunk = new WriterChunk();

	protected Connection connection;

	/**
	 *  
	 */
	public UPLWriter(FileTransferEngine fte, WriterFactory wf,
			Connection connection, Set open_writers, Logger logger) {
		this.fte = fte;
		something_to_do = true;
		this.wf = wf;
		this.open_writers = open_writers;
		this.logger = logger;
		this.connection = connection;
	}

	public void setChunk(Chunk c) {
		chunk = c;
	}

	public Chunk getChunk() {
		return chunk;
	}

	/**
	 * That's all. Finish off and tidy up.
	 * 
	 * Pre-condition: Writer is not reading (usually is in run() though)
	 */
	public void terminate(boolean with_error) {
		something_to_do = false;
		on_error = on_error || with_error;
		if (me_as_writer_chunk != null) {
			Chunk c = me_as_writer_chunk.getChunk();
			while (c != null) {
				fte.disposeWrittenChunk(c);
				c = me_as_writer_chunk.getChunk();
			}
		}
	}

	protected boolean on_error = false;

	public void run() {

		execute();

		if (on_error) {
			connection.closeError();
		} else {
			connection.closeOK();
		}

		wf.notifyTerminated(this);
		// This has really done, clear reference to avoid memory leak
		wf = null;
		fte = null;
		chunk = null;
	}

	protected void execute() {
		try {
			outer_loop: while (something_to_do) {

				fte.finishedAWrite(this);

				if (chunk != null) { // Chunk can be null when stopping, call
									 // finishedAWrite again to complete

					if (chunk.hasFailed()) {
						// Reading failed and so transfer is suspect. Need to
						// flag an error
						// to receiving end.
						logger.logComment("Writing error marker to UPL output");
						ZipEntry marker = new ZipEntry("ERROR");
						connection.getDataOutputStream().putNextEntry(marker);
						connection.getDataOutputStream().closeEntry();
						connection.doneWithDataOutputStream();
						continue outer_loop;
					}

					if (chunk.getFileName().equals("NO_OVERWRITE")) {
						logger
								.logComment("Adding no overwrite marker to stream for <"
										+ chunk.getPortfolio()
												.getUPLDirectoryName() + ">");
						ZipEntry marker = new ZipEntry(chunk.getPortfolio()
								.getUPLDirectoryName()
								+ "/");
						marker.setExtra(new byte[1]);
						connection.getDataOutputStream().putNextEntry(marker);
						connection.getDataOutputStream().closeEntry();
						connection.doneWithDataOutputStream();
						continue outer_loop;
					}

					// Try to preserve contiguous writes by handing off this
					// chunk to another
					// Writer if it is writing the previous chunk.
					//
					// open_writers is a list of UPLWriters with the the
					// FileChunk(s) that they are
					// currently writing

					synchronized (open_writers) {

						Iterator iterator = open_writers.iterator();
						while (iterator.hasNext()) {
							WriterChunk wc = (WriterChunk) iterator.next();
							if (wc.name.equals(chunk.getFileName())) {
								if (wc.end_of_known_writes == chunk
										.getStartByte()) {
									// This is active, hand off to the current
									// Writer
									logger.logComment(Thread.currentThread()
											.getName()
											+ " is handing off <"
											+ chunk.getFileName()
											+ "> <"
											+ chunk.getStartByte() + ">");
									wc.appendChunk(chunk);
									chunk = null; // this Writer needs another
												  // chunk
									continue outer_loop;
								}
							}
						}

						// Got here coz we could not hand off so write from this
						// one
						// First see if the chunk is contiguous with the last we
						// wrote, if so append.
						// otherwise close the entry and create a new one
						if (chunk.getFileName().equals(me_as_writer_chunk.name)
								&& me_as_writer_chunk.end_of_known_writes == chunk
										.getStartByte()) {
							logger.logComment(Thread.currentThread().getName()
									+ " has contiguous chunk <"
									+ chunk.getFileName() + "> <"
									+ chunk.getStartByte() + ">");
							me_as_writer_chunk.end_of_known_writes += chunk
									.getChunkLength();
						} else {
							// not contig (or first)

							// close any previous
							if (me_as_writer_chunk.name != null) {
								logger.logComment(Thread.currentThread()
										.getName()
										+ " has new chunk <"
										+ chunk.getFileName()
										+ "> <"
										+ chunk.getStartByte() + ">");
								connection.getDataOutputStream().flush();
								connection.getDataOutputStream().closeEntry();
							}

							me_as_writer_chunk.initChunk(chunk);

							// Note that Portfolio id is needed in UPL and is
							// placed in Chunk by chunk manager
							// we also need to name each new chunk differently
							// (since ZIP stream barfs if the
							// the file names are the same)
							String file_name;
							if (chunk.getPortfolio() != null) {
								file_name = chunk.getPortfolio()
										.getUPLDirectoryName()
										+ "/" + chunk.getFileName();
							} else {
								file_name = chunk.getFileName();
							}
							if (chunk.getStartByte() != 0) { // First chunk name
															 // is not changed -
															 // for backwards
															 // compatibility
								file_name = "CHUNK_" + chunk.getStartByte()
										+ "_" + file_name;
							}

							ZipEntry zip_entry = new ZipEntry(file_name);

							// File mode information for destination
							zip_entry.setExtra(new byte[] { chunk.getMode() });

							connection.getDataOutputStream().putNextEntry(
									zip_entry);

						}

						open_writers.add(me_as_writer_chunk); // lay claim to
															  // this file chunk
															  // and any
															  // following

					}

					Chunk to_write = chunk;
					while (to_write != null) {
						logger.logComment(Thread.currentThread().getName()
								+ " is writing <" + to_write.getFileName()
								+ "> <" + to_write.getStartByte() + "> <"
								+ to_write.getChunkLength() + "> <"
								+ (char) to_write.getBuffer()[0] + ">");

						connection.getDataOutputStream().write(
								to_write.getBuffer(), 0,
								(int) to_write.getChunkLength());

						fte.disposeWrittenChunk(to_write);

						synchronized (open_writers) {
							// Unfortunately, I do mean open_writers and not
							// me_as_writer_chunk. This
							// is to stop other threads handing off at the same
							// time as this one runs
							// out of chunks, hence the removal in this block
							to_write = me_as_writer_chunk.getChunk();
							if (to_write == null)
								open_writers.remove(me_as_writer_chunk); // Finished,
																		 // remove
																		 // any
																		 // claims
						}

					}
					connection.getDataOutputStream().flush(); // just in case we
															  // get held up
				}
			} // something to do loop

			//					connection.getDataOutputStream().flush();
			connection.getDataOutputStream().closeEntry();
			connection.doneWithDataOutputStream();

		} catch (IOException e) {
			terminate(true);
			fte.notifyError(chunk, e);
		} catch (Exception e) {
			terminate(true);
			fte.notifyError(chunk, e);
		} finally {
		}
	}

	class WriterChunk {

		public String name;

		public long end_of_known_writes;

		//		public long start_byte;
		//		public WriterChunk next;
		private LinkedList chunks = new LinkedList(); // list of chunks still to
													  // be written

		public WriterChunk() {
		}

		void initChunk(Chunk chunk) {
			chunks.clear();
			name = chunk.getFileName();
			//			start_byte = chunk.getStartByte();
			end_of_known_writes = chunk.getStartByte() + chunk.getChunkLength();
			//			appendChunk(chunk);
		}

		public synchronized void appendChunk(Chunk chunk) {
			chunks.addLast(chunk);
			end_of_known_writes += chunk.getChunkLength();
		}

		public synchronized Chunk getChunk() {
			if (chunks.isEmpty()) {
				return (Chunk) null;
			} else {
				return (Chunk) chunks.removeFirst();
			}
		}
	}

	public static class Initial extends UPLWriter {

		private ConsignJob cj;

		public Initial(ConsignJob cj, FileTransferEngine fte,
				UPLCJWriterFactory uwf, Set open_writers, Logger logger,
				Connection connection) {
			super(fte, uwf, connection, open_writers, logger);
			this.cj = cj;
		}

		public void run() {

			try {
				connection.writeServerRequest(cj);
				execute();
				if (!on_error)
					((UPLCJWriterFactory) wf).setConsignJobReply(connection
							.readReply());
			} catch (IOException e) {
				terminate(true);
				fte.notifyError(chunk, e);
			}

			if (on_error) {
				connection.closeError();
			} else {
				connection.closeOK();
			}

			wf.notifyTerminated(this);
			// This has really done, clear reference to avoid memory leaks
			wf = null;
			fte = null;
			chunk = null;
		}

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
