package com.fujitsu.arcon.common;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 29, 2003
 */
public interface Reader {

	/**
	 * New chunk of a file to read.
	 * 
	 * @param c
	 *            The new chunk (null if there are no more, should call fte for
	 *            end of read)
	 */
	public void setChunk(Chunk c);

	public Chunk getChunk();

	public void terminate(boolean with_error);
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
