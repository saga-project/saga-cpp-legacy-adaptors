package com.fujitsu.arcon.common;

import java.io.File;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Stack;

import org.unicore.ajo.Portfolio;

/**
 * Doles out chunks of files to Readers. This is used when files are being read
 * from a local disk (e.g. in a client) and written to a Unicore server.
 * <p>
 * Files are part of Portfolios, these are passed along too.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 29, 2003
 */
public class FileChunkManager implements ChunkManager {

	private int buffer_size;

	private Stack unallocated;

	private Logger logger;

	private Map allocations;

	public FileChunkManager(Logger logger, int buffer_size) {
		unallocated = new Stack();
		allocations = new HashMap();
		this.buffer_size = buffer_size;
		this.logger = logger;
	}

	public void addPortfolio(Portfolio pf, File[] files) {
		populateUnallocated(pf, files);
	}

	public void addAllocation(Allocation alloc) {
		unallocated.push(alloc);
	}

	private void populateUnallocated(Portfolio pf, File[] files) {
		for (int i = 0; i < files.length; i++) {
			if (files[i].isDirectory()) {
				populateUnallocated(pf, files[i].listFiles());
			} else {
				Allocation new_alloc = new Allocation(files[i], pf);
				unallocated.push(new_alloc);
			}
		}
	}

	/**
	 * Compute the next chunk that the Reader should read. The Reader instance
	 * is used to maintain some contiguity in the file reads.
	 * 
	 * Assumption that this is synchronized externally.
	 * 
	 * Pre-condition: noMoreChunks() returns false
	 * 
	 * @param r
	 */
	public void getNextChunk(Reader r) {

		if (noMoreChunks()) {
			r.setChunk(null);
			return;
		}

		// Firstly allocate a file to a particular reader. If this is not
		// possible
		// allocate half of the largest remaining part of a file to the Reader.
		//
		// Within an allocation dole out buffer size chunks.
		//
		// Start at allocation beginning. Assume that doled out chunks are
		// always
		// successfully read.
		//

		if (allocations.containsKey(r)) {
			// Already have an allocation for this Reader, give it the next
			// chunk
			allocateChunk(r, (Allocation) allocations.get(r));
			logger.logComment("Allocating consecutive chunk to <"
					+ r.getChunk().getFileName() + "> start <"
					+ r.getChunk().getStartByte() + ">");
		} else {
			// Need to find a new allocation for this Reader
			Allocation new_alloc;
			if (unallocated.empty()) {
				// Need to reallocate something
				// .... first find largest remaining
				Iterator iterator = allocations.values().iterator();
				Allocation largest = (Allocation) iterator.next();
				while (iterator.hasNext()) {
					Allocation alloc = (Allocation) iterator.next();
					if (largest.left < alloc.left)
						largest = alloc;
				}

				if (largest.left <= buffer_size) {
					// This also handles left == 1
					new_alloc = largest;

					// delete previous mapping, assume here that
					// Allocations are unique
					iterator = allocations.entrySet().iterator();
					Object o = null;
					while (iterator.hasNext()) {
						Map.Entry me = (Map.Entry) iterator.next();
						if (me.getValue() == largest)
							o = me.getKey();
					}
					allocations.remove(o);

					logger.logComment("+++++ Allocating remnant "
							+ largest.file_name);

				} else {
					new_alloc = new Allocation(largest.file, largest.pf);

					// split it into 2
					new_alloc.left = largest.left / 2;
					largest.left = largest.left - new_alloc.left;
					new_alloc.start = largest.start + largest.left;

					logger.logComment("+++++ Splitting allocation "
							+ largest.file_name + "   " + new_alloc.start);
				}
			} else {
				//				// Allocate a whole new file
				//				File f = (File) unallocated.pop();
				//				new_alloc = new Allocation();
				//				new_alloc.file_name = f.getPath();
				//				new_alloc.pf = (Portfolio) pfs.pop();
				//				new_alloc.start = 0;
				//				new_alloc.left = f.length();
				//				
				//				new_alloc.mode = 4; // mode is read - goes without saying
				// that we can
				//				if(f.canWrite()) new_alloc.mode += 2;
				//				// Java does not allow execute?

				new_alloc = (Allocation) unallocated.pop();

				logger.logComment("+++++ Allocating new file "
						+ new_alloc.file_name);
			}

			allocations.put(r, new_alloc);
			allocateChunk(r, new_alloc);
		}

	}

	private void allocateChunk(Reader r, Allocation alloc) {

		Chunk chunk = new Chunk();
		chunk.setFileName(alloc.file_name);
		chunk.setPortfolio(alloc.pf);
		chunk.setStartByte(alloc.start);
		chunk.setMode(alloc.mode);

		if (alloc.left > buffer_size) {
			// More than a buffer's worth to go
			chunk.setChunkLength(buffer_size);
			alloc.start += buffer_size;
			alloc.left -= buffer_size;
		} else {
			chunk.setChunkLength(alloc.left);
			// this chunk finishes the file, remove allocation
			allocations.remove(r);
		}

		r.setChunk(chunk);
	}

	public boolean noMoreChunks() {
		return allocations.isEmpty() && unallocated.empty();
	}

	public static class Allocation {

		public Allocation(File file, Portfolio pf) {
			this.file = file;
			file_name = file.getPath();
			this.pf = pf;
			start = 0;
			left = file.length();

			mode = 4; // mode is read - of course we can
			if (file.canWrite())
				mode += 2;
			mode += 1; // Java does not allow execute, assume is?
		}

		public File file;

		public String file_name; // name of file to read

		public Portfolio pf;

		public long start; // where next chunk should start from

		public long left; // number of bytes left to read

		public byte mode; // file mode (rwx) - user only
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
