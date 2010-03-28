package com.fujitsu.arcon.common;

/**
 * Manage the allocation of parts of files to Readers.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 29, 2003
 */
public interface ChunkManager {

	/**
	 * Compute the next chunk that the Reader should read. The Reader instance
	 * is used to maintain some contiguity in the file reads.
	 * <p>
	 * This may place a null chunk (if all have been allocated)
	 * 
	 * @param r
	 */
	public abstract void getNextChunk(Reader r);

	/**
	 * All chunks have been allocated to Readers, no more to do.
	 * 
	 * @return
	 */
	public abstract boolean noMoreChunks();
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
