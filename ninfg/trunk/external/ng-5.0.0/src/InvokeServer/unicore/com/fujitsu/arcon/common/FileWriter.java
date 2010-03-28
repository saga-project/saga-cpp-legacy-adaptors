package com.fujitsu.arcon.common;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 30, 2003
 */
public class FileWriter implements Writer, Runnable {

	// This holds all open FileOutputStreams for all existing Writers
	private Set open_files;

	private Chunk chunk;

	private boolean something_to_do;

	private FileTransferEngine fte;

	private FileWriterFactory eru;

	private String base_dir;

	private Logger logger;

	/**
	 *  
	 */
	public FileWriter(FileTransferEngine fte, FileWriterFactory eru,
			Set open_files, String base_dir, Logger logger) {
		this.fte = fte;
		something_to_do = true;
		this.eru = eru;
		this.open_files = open_files;
		this.base_dir = base_dir;
		this.logger = logger;
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
	}

	public void run() {

		try {
			outer_loop: while (something_to_do) {

				fte.finishedAWrite(this);

				if (chunk != null) { // Chunk can be null when stopping, call
									 // finishedAWrite again to complete

					if (chunk.hasFailed()) {
						logger
								.logComment("Ignoring a read chunk that is marked as failed.");
						continue outer_loop;
					}

					if (chunk.getPortfolio() != null) {
						File marker = new File(chunk.getFileName()
								+ "UCNOOVERWRITE");
						marker.createNewFile();
						logger.logComment("Writing no-overwrite marker to <"
								+ marker + ">");
						continue outer_loop;
					}

					// Try to preserve contiguous write if possible. Look for an
					// open
					// file chunk of the right root name and position (also
					// free).
					// Otherwise, open a new one.
					//
					// NB No free checks because there should be only one chunk
					// that can use a
					// file chunk at its current position.

					FileChunk target = null;

					synchronized (open_files) {
						Iterator iterator = open_files.iterator();
						while (iterator.hasNext()) {
							FileChunk fc = (FileChunk) iterator.next();
							if (fc.name.equals(chunk.getFileName())) {
								// scan list for the first file chunk that is
								// positioned at
								// the start of the input chunk (list is
								// ordered).
								//
								// Also test for the previous contiguous chunk
								// being writen now,
								// if so, then pass this chunk to the writer so
								// that we do not
								// create too many partial files
								while (fc != null) {
									if (fc.position == chunk.getStartByte()) {
										// This file is not active, use this
										// Writer to write
										fc.appendChunk(chunk);
										target = fc;
										fc = null;
									} else if (fc.end_of_current_write == chunk
											.getStartByte()) {
										// This file is active, hand off to the
										// current Writer
										fc.appendChunk(chunk);
										chunk = null; // this Writer needs
													  // another chunk
										continue outer_loop;
									} else {
										fc = fc.next; // this is null at list
													  // end
									}
								}
								break;
							}
						}

						if (target == null) {
							// File not open. Create one.
							target = new FileChunk(chunk);

							//						if (chunk.getStartByte() == 0) {
							//							target.actual_file = new
							// File(base_dir,target.name);
							//						}
							//						else {
							target.actual_file = new File(base_dir, target.name);
							target.actual_file = new File(target.actual_file
									.getParent(), "CHUNK_"
									+ chunk.getStartByte() + "_"
									+ target.actual_file.getName());
							//						}

							if (!target.actual_file.getParentFile().exists()) {
								if (!target.actual_file.getParentFile()
										.mkdirs()) {
									throw new FileNotFoundException(
											"Failed to create directory <"
													+ target.actual_file
															.getParentFile()
													+ ">");
								}
							}

							target.stream = new FileOutputStream(
									target.actual_file);

							target.position = chunk.getStartByte();

							iterator = open_files.iterator();
							FileChunk existing = null;
							while (iterator.hasNext()) {
								FileChunk fc = (FileChunk) iterator.next();
								if (fc.name.equals(target.name)) {
									existing = fc;
									break;
								}
							}

							if (existing == null) {
								open_files.add(target);
							} else {
								if (existing.start_byte > target.start_byte) {
									// insert at head
									target.next = existing;
									open_files.remove(existing);
									open_files.add(target);
								} else {
									// insert in list (existing is maintained as
									// the last in the list less (or =) target)
									while (existing.next != null
											&& existing.next.start_byte <= target.start_byte) {
										existing = existing.next;
									}
									if (existing.start_byte == target.start_byte)
										throw new Exception(
												"A received chunk has same start byte as an existing chunk <"
														+ existing.start_byte
														+ ">");
									FileChunk temp = existing.next;
									existing.next = target;
									target.next = temp;

								}
							}
						}
					}

					Chunk to_write = target.getChunk(); // no sync since we must
														// have at least 1 (see
														// below)
					while (to_write != null) {
						logger.logComment(Thread.currentThread().getName()
								+ " is writing <" + to_write.getFileName()
								+ "> <" + to_write.getStartByte() + "> <"
								+ chunk.getChunkLength() + ">");
						target.stream.write(to_write.getBuffer(), 0,
								(int) to_write.getChunkLength());
						target.stream.flush();

						target.position += to_write.getChunkLength();

						// Get the next chunk (if any have been handed off while
						// this was writing.
						// It is synchronized here to prevent another thread
						// handing off just as this
						// one thinks that it is complete (if this happens the
						// handed off chunks get
						// orphaned and never written).
						synchronized (open_files) {
							to_write = target.getChunk();
						}
					}
				}

			}
		} catch (FileNotFoundException e) {
			terminate(true);
			fte.notifyError(chunk, e);
		} catch (IOException e) {
			terminate(true);
			fte.notifyError(chunk, e);
		} catch (Exception e) {
			terminate(true);
			fte.notifyError(chunk, e);
		} finally {
			eru.notifyTerminated(this);
			// This has really done, clear reference to avoid memory leaks
			eru = null;
			fte = null;
			chunk = null;
		}

	}

	public static class FileChunk {

		public String name;

		public File actual_file;

		public FileOutputStream stream;

		public long position;

		public long end_of_current_write;

		public long start_byte;

		public FileChunk next;

		private List chunks = new LinkedList();

		public FileChunk(Chunk chunk) {
			name = chunk.getFileName();
			start_byte = chunk.getStartByte();
			end_of_current_write = start_byte;
			appendChunk(chunk);
		}

		public synchronized void appendChunk(Chunk chunk) {
			chunks.add(chunk);
			end_of_current_write += chunk.getChunkLength();
		}

		public synchronized Chunk getChunk() {
			if (chunks.isEmpty()) {
				return (Chunk) null;
			} else {
				return (Chunk) chunks.remove(0);
			}
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
