package com.fujitsu.arcon.common;

import org.unicore.ajo.Portfolio;

/**
 * A part of a file.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 29, 2003
 */
public class Chunk {

	private String file_name;

	private long start_byte;

	private long chunk_length;

	private byte[] buffer;

	public Chunk() {
	}

	public String getFileName() {
		return file_name;
	}

	public void setFileName(String file_name) {
		this.file_name = file_name;
	}

	public long getStartByte() {
		return start_byte;
	}

	public long getChunkLength() {
		return chunk_length;
	}

	public byte[] getBuffer() {
		return buffer;
	}

	public void setBuffer(byte[] buffer) {
		failed = false;
		this.buffer = buffer;
	}

	public void setChunkLength(long l) {
		chunk_length = l;
	}

	public void setStartByte(long l) {
		start_byte = l;
	}

	private byte mode;

	public byte getMode() {
		return mode;
	}

	public void setMode(byte b) {
		mode = b;
	}

	private Portfolio portfolio;

	public Portfolio getPortfolio() {
		return portfolio;
	}

	public void setPortfolio(Portfolio i) {
		portfolio = i;
	}

	private boolean failed = false;

	public boolean hasFailed() {
		return failed;
	}

	public void setFailed(boolean failed) {
		this.failed = failed;
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
