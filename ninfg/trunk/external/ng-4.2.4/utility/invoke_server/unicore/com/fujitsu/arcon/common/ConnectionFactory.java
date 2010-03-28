package com.fujitsu.arcon.common;

import java.io.IOException;

/**
 * Source of UPL connections. This is set to connect to a particular Vsite.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:23 $
 * 
 * Copyright Fujitsu Laboratories of Europe 2003
 * 
 * Created Aug 1, 2003
 */
public interface ConnectionFactory {

	public Connection connect() throws IOException;

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
