package com.fujitsu.arcon.common;

import java.io.IOException;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

import org.unicore.upl.Reply;
import org.unicore.upl.ServerRequest;

/**
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 * Created Aug 1, 2003
 */
public interface Connection {

	/**
	 * Send a UPL serverRequest to the Vsite. Note that the caller does not know
	 * the destination Vsite so this needs to be filled in.
	 * 
	 * @param sr
	 * @return
	 */
	public Reply sendServerRequest(ServerRequest sr) throws IOException;

	public void writeServerRequest(ServerRequest sr) throws IOException;

	public Reply readReply() throws IOException;

	public ZipInputStream getDataInputStream() throws IOException;

	public ZipOutputStream getDataOutputStream() throws IOException;

	public void doneWithDataOutputStream() throws IOException;

	/**
	 * Done with this Connection for this UPL Request/Reply.
	 *  
	 */
	public void closeOK();

	public void closeError();

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
