package com.fujitsu.arcon.common;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Jul 30, 2003
 */
public interface WriterFactory {

	public void setFileTransferEngine(FileTransferEngine fte);

	public boolean makeWriter();

	/**
	 * Can (should) the FTE make a Reader given the current state of the
	 * Writers? One, only, use is to stop multiple Readers until we are sure
	 * that we have an NJS that suuports parallel streaming.
	 * 
	 * @return
	 */
	public boolean canMakeReader();

	/**
	 * A Writer has finished its execution.
	 * 
	 * @param r
	 */
	public void notifyTerminated(Writer w);

	/**
	 * Have all created Writers terminated?
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
