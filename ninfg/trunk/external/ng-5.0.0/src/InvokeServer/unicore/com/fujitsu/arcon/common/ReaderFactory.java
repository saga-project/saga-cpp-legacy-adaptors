package com.fujitsu.arcon.common;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 30, 2003
 */
public interface ReaderFactory {

	public void setFileTransferEngine(FileTransferEngine fte);

	/**
	 * Create a Reader.
	 * <p>
	 * If there was an error creating the Reader, then the Factory must take the
	 * appropriate action to clean up. If this Factory has already created
	 * Readers, then if is OK to allow them to continue. However, if the first
	 * Reader can't be created, then the Factory must ensure that the
	 * FileTransferEngine get shut down properly.
	 * <p>
	 * A legitimate implementation is to return false i.e. no reader is created,
	 * this could be because resource use limits have been reached.
	 * 
	 * @return True if Reader was created successfully, false if no Reader was
	 *         created.
	 */
	public boolean makeReader();

	/**
	 * Can (should) the FTE make a writer given the current state of the
	 * Readers? One, only, use is to stop multipleWriters until we are sure that
	 * we have an NJS that suuports parallel streaming.
	 * 
	 * @return
	 */
	public boolean canMakeWriter();

	/**
	 * A Reader has finished its execution.
	 * 
	 * @param r
	 */
	public void notifyTerminated(Reader r);

	/**
	 * Have all created Readers terminated? This should return false until at
	 * least one Reader has executed and run to completion (i.e. return true
	 * before any Readers have been started).
	 * 
	 * @return
	 */
	public boolean allTerminated();

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
