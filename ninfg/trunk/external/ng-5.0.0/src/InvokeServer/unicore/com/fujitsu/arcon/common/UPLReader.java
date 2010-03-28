package com.fujitsu.arcon.common;

import java.io.IOException;
import java.util.zip.ZipEntry;

import org.unicore.ajo.Portfolio;

/**
 * Reads Chunks from a UPL connection.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Aug 1, 2003
 */
public class UPLReader implements Reader, Runnable {

	private static int id = 0;

	private Logger logger;

	private Connection connection;

	private ReaderFactory rf;

	private FileTransferEngine fte;

	private Chunk chunk;

	private boolean not_terminated;

	private int my_id;

	public UPLReader(FileTransferEngine fte, Connection connection,
			ReaderFactory rf, Logger logger) {
		this.fte = fte;
		this.connection = connection;
		this.rf = rf;
		this.logger = logger;

		not_terminated = true;

		my_id = id++;

	}

	/**
	 * Called by the ChunkManager to set next Chunk, but for UPL reading this is
	 * determined by the incoming data, so should never be called.
	 */
	public void setChunk(Chunk c) {
		chunk = c;
	}

	public Chunk getChunk() {
		return chunk;
	}

	/**
	 * Called to terminate current connection (can be revoked by
	 * setNewConnection). Clean up any open InputStreams here?
	 */
	public void terminate(boolean with_error) {
		if (with_error) {
			connection.closeError();
		} else {
			connection.closeOK();
		}
		not_terminated = false;
		logger.logComment("UPLRreader-" + my_id + " terminated"
				+ (with_error ? " with error" : ""));
	}

	public void setNewConnection(Connection c) {
		this.connection = c;
		not_terminated = true;
		logger.logComment("UPLRreader-" + my_id + " new connection");
	}

	// The file chunk being read from the input stream
	String current_file_name;

	long offset_in_current_file;

	byte mode;

	// and if this is a directory (no overwrite marker)
	private Portfolio portfolio;

	private boolean data_on_stream;

	/**
	 * 
	 * @throws Exception
	 *             Problems with the ZipEntry (file name, format etc)
	 */
	private void openNewInputFileChunk() throws Exception {

		// Expect the input stream to be positioned at the start of a new Zip
		// entry
		ZipEntry entry;
		try {
			entry = connection.getDataInputStream().getNextEntry();
		} catch (IOException e) {
			throw new Exception("Problems reading new ZIPEntry", e);
		}

		if (entry != null) {
			if (entry.isDirectory()) {
				if (entry.getExtra() != null) {
					current_file_name = entry.getName();
					offset_in_current_file = 0;
					// This is a marker saying that the Portfolio must not
					// over-write files on reveal
					// - parse for Portfolio Id and place special chunk for
					// reader
					portfolio = new Portfolio("Marker");
					logger.logComment("UPLRreader-" + my_id
							+ " No overwrite marker for <" + current_file_name
							+ ">");
				} else {
					// spurious directory in stream, skip it
					openNewInputFileChunk();
				}
			} else if (entry.getName().equals("ERROR")) {
				throw new Exception(
						"There was an error sent by the source of the files.");
			} else {
				// This is a new input file chunk, extract the target name.
				// Differences in
				// buffer sizes mean that we need to rechunk internally, set up
				// for this.
				current_file_name = entry.getName();
				offset_in_current_file = 0;
				portfolio = null;

				// If this is a subsequent chunk (not first), then the name will
				// be
				// made unique (for ZIP stream) by adding the start byte to the
				// front, extract this
				if (current_file_name.startsWith("CHUNK_")) {
					int i = 6; // = "CHUNK_".length());
					int j = current_file_name.indexOf("_", i);
					try {
						offset_in_current_file = Long
								.parseLong(current_file_name.substring(i, j));
					} catch (NumberFormatException e) {
						Exception ex = new Exception(
								"Problems parsing input file chunk name <"
										+ current_file_name + ">");
						ex.initCause(e);
						throw ex;
					}

					current_file_name = current_file_name.substring(j + 1);
				}

				if (entry.getExtra() != null) {
					mode = entry.getExtra()[0];
				} else {
					mode = 6; // no mode, no offset. Use default mode
				}

				logger.logComment("UPLReader-" + my_id + " Extracting file <"
						+ current_file_name + ">, size <" + entry.getSize()
						+ ">, mode <" + mode + ">, offset <"
						+ offset_in_current_file + ">");

				data_on_stream = true;
			}
		} else {
			data_on_stream = false; // This is where we find the inout stream
									// EOF in normal processing
		}
	}

	private void newChunk() {
		chunk.setFileName(current_file_name);
		chunk.setStartByte(offset_in_current_file);
		chunk.setChunkLength(0);
		chunk.setMode(mode);
		chunk.setPortfolio(portfolio);
	}

	public void run() {

		try {

			fte.finishedARead(this); // get first buffer

			// This has exclusive access to a single UPL stream. On every
			// iteration of the loop
			// it will read a buffer's worth. This may or may not match a Chunk
			// written to the
			// UPL stream. Therefore, this needs to keep track of what chunk is
			// being read from the
			// stream and perhaps rename it to a local chunk scheme.

			// offset_in_current_file = where the current internal chunk start
			// in file == start_byte
			// so_far = how much of the current buffer we have read ==
			// chunk_length in the end

			while (not_terminated) {

				try {
					openNewInputFileChunk();
					newChunk();

					while (data_on_stream) {

						// Try to fill up the internal buffer with data from
						// stream
						int read = 0;
						int so_far = (int) chunk.getChunkLength();
						do {
							so_far += read; // here coz read == -1 at end of ZIP
											// entry (which we don't want added
											// to length!)
							read = connection.getDataInputStream().read(
									chunk.getBuffer(), so_far,
									chunk.getBuffer().length - so_far);
						} while (read > 0);

						chunk.setChunkLength(so_far);

						if (read == 0) {

							offset_in_current_file += chunk.getChunkLength(); // advance
																			  // this
																			  // as
																			  // we
																			  // have
																			  // a
																			  // new
																			  // internal
																			  // chunk

							logger.logComment("UPLReader-" + my_id
									+ " Placing a complete chunk of <"
									+ current_file_name + "> from <"
									+ chunk.getStartByte() + "> length <"
									+ chunk.getChunkLength() + "> <"
									+ (char) chunk.getBuffer()[0] + ">");

							// Last read was OK, probably filled internal buffer
							// - get rid of it
							fte.finishedARead(this);
							if (!not_terminated)
								return; // error with concurrent read

							newChunk();
						} else { // read == -1
							// Got to end of input stream, need a new Entry from
							// ZIP stream
							String old_file_name = current_file_name;
							long old_offset = offset_in_current_file;

							openNewInputFileChunk();
							if (data_on_stream) {
								if (current_file_name.equals(old_file_name)
										&& offset_in_current_file == old_offset
												+ chunk.getChunkLength()) {
									// carry on, contiguous file, same chunk,
									// partially filled buffer
									// correct offset since it reflects the new
									// input chunk and not the offset
									// of the current chunk
									offset_in_current_file = old_offset;
									logger.logComment("UPLReader-" + my_id
											+ " new contiguous ZIP entry <"
											+ current_file_name + "> from <"
											+ chunk.getStartByte() + ">");

								} else {
									// non-contig, dispose of buffer and then
									// read new one
									logger
											.logComment("UPLReader-"
													+ my_id
													+ " new non-contiguous ZIP entry, dumping <"
													+ old_file_name
													+ "> from <"
													+ chunk.getStartByte()
													+ "> length <"
													+ chunk.getChunkLength()
													+ ">");
									if (chunk.getChunkLength() != 0)
										fte.finishedARead(this);
									if (!not_terminated)
										return; // error with concurrent read
									// NB safe to not go back because we know
									// that there is data on the wire (just read
									// file header)
									newChunk();
								}
							}
						}
					}

					logger.logComment("UPLReader-" + my_id
							+ " Placing a complete END chunk of <"
							+ current_file_name + "> from <"
							+ chunk.getStartByte() + "> length <"
							+ chunk.getChunkLength() + ">");
					// Got to input stream EOF (openNewInputFileChunk() set no
					// data on stream)
					// finished this stream so clear the buffer. On return go to
					// Factory
					// to see if there is another input stream to read, if so,
					// then the
					// terminate will have been undone.
					fte.finishedARead(this);
					// Must always go back coz ended a stream so may be
					// completely done
					if (not_terminated)
						terminate(false);

				} catch (Exception e) {
					terminate(true);
					fte.notifyError(chunk, e);
					chunk.setFailed(true);
					fte.finishedARead(this);
				} finally {
					rf.notifyTerminated(this); // this may undo the terminate()
											   // with a new Connection
				}
			}

		} finally {
			// This has really done, clear reference to avoid memory leaks
			rf = null;
			fte = null;
			connection = null;
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
