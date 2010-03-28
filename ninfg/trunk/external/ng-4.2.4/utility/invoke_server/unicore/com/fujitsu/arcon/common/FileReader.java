package com.fujitsu.arcon.common;

import java.io.FileInputStream;
import java.io.IOException;

/**
 * 
 * Read local Files (e.g.in a Unicore Client)
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 29, 2003
 */
public class FileReader implements Reader, Runnable {

	private Chunk chunk;

	private boolean something_to_do;

	private FileTransferEngine fte;

	private FileReaderFactory eru;

	private Logger logger;

	/**
	 *  
	 */
	public FileReader(FileTransferEngine fte, FileReaderFactory eru,
			Logger logger) {
		this.fte = fte;
		something_to_do = true;
		this.eru = eru;
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
	 * Pre-condition: Reader is not reading (usually is in run() though)
	 */
	public void terminate(boolean with_error) {
		something_to_do = false;
	}

	public void run() {

		String file_name = "none";
		long position_in_file = -1;
		FileInputStream file_input = null;

		fte.finishedARead(this);

		try {
			while (something_to_do) {

				if (chunk.getFileName().equals("NO_OVERWRITE")) {
					if (file_input != null)
						try {
							file_input.close();
							file_input = null;
						} catch (IOException ex) {
						}
					logger.logComment("Writing no-overwrite marker to buffer.");
					fte.finishedARead(this);
				}

				if (!(chunk.getFileName().equals(file_name) && chunk
						.getStartByte() == position_in_file)) {
					// This is not contiguous to the last chunk, close it
					// and open the new one
					if (file_input != null)
						try {
							file_input.close();
						} catch (IOException ex) {
						}
					file_name = chunk.getFileName();
					file_input = new FileInputStream(file_name);
					position_in_file = chunk.getStartByte();

					// Move to start point in file
					long skip_total = 0;
					while (skip_total < position_in_file) {
						skip_total += file_input.skip(position_in_file
								- skip_total);
					}
				}

				logger.logComment("Reading a chunk from file <"
						+ chunk.getFileName() + "> from <"
						+ chunk.getStartByte() + ">");

				long read_total = 0;
				while (read_total < chunk.getChunkLength()) {
					int actually_read = file_input.read(chunk.getBuffer(),
							(int) read_total,
							(int) (chunk.getChunkLength() - read_total));
					if (actually_read == -1)
						throw new IOException(
								"Got to end of file having only read <"
										+ read_total + "> bytes.");
					read_total += actually_read;
				}

				position_in_file += chunk.getChunkLength();

				logger.logComment("Finished reading a chunk from file <"
						+ chunk.getFileName() + "> from <"
						+ chunk.getStartByte() + "> now at <"
						+ position_in_file + ">");

				fte.finishedARead(this);

			}
		} catch (Exception e) {
			terminate(true);
			fte.notifyError(chunk, e);
			chunk.setFailed(true);
			fte.finishedARead(this);
		}

		// All normal and handled exits go this way, unhandled force collapse
		// in a heap (no finally as the try/catch below swallows any errors)
		eru.notifyTerminated(this);
		try {
			if (file_input != null)
				file_input.close();
		} catch (IOException e) {
			// Don't care
		}
		// This has really done, clear reference to avoid memory leaks
		eru = null;
		fte = null;
		file_input = null;
		chunk = null;

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
