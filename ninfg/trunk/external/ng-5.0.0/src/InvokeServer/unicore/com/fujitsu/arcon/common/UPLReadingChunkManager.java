package com.fujitsu.arcon.common;

/**
 * //UPLD //UPLD //UPLD //UPLD //UPLD
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Aug 1, 2003
 */
public class UPLReadingChunkManager implements ChunkManager {

	ReaderFactory reader_factory;

	public UPLReadingChunkManager(ReaderFactory reader_factory) {
		this.reader_factory = reader_factory;
	}

	/**
	 * Returns immediately.
	 */
	public void getNextChunk(Reader r) {
		r.setChunk(new Chunk());
	}

	public boolean noMoreChunks() {
		return reader_factory.allTerminated();
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
