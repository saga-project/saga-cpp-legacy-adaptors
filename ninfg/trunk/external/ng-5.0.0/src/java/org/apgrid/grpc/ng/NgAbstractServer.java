/*
 * $RCSfile: NgAbstractServer.java,v $ $Revision: 1.5 $ $Date: 2007/09/26 04:14:07 $
 * $AIST_Release: 5.0.0 $
 * $AIST_Copyright:
 *  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
 *  National Institute of Advanced Industrial Science and Technology
 *  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  $
 */
package org.apgrid.grpc.ng;

import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.io.IOException;

public abstract class NgAbstractServer implements Runnable {

	protected boolean shutdownRequested = false;
	protected ServerSocket _server = null;
	protected int timeout = 5 * 60 * 1000; // Socket timeout 

	private Thread serverThread = null;

	// Constructors 
	public NgAbstractServer() throws IOException {
		this(0);
	}

	public NgAbstractServer(int port) throws IOException {
		this._server = new ServerSocket(port);
		start();
	}

	// Start the server thread
	protected void start() {
		if (serverThread == null) {
			serverThread = new Thread(this);
			serverThread.start();
			shutdownRequested = false;
		}
	}

	public void shutdown() {
		shutdownRequested = true;
		ServerSocket wk = _server;
		_server = null;
		try {
			wk.close();
		} catch (IOException e) {
		}
		// reset everything
		serverThread = null;
	}

	// Main Thread on NgAbstractServer
	public void run() {
		Socket socket = null;
		while (!shutdownRequested) {
			try {
				socket = _server.accept();
			} catch (SocketException e) {
				if (_server == null && shutdownRequested ) {
					break; // ServerSocket is closed
				}
				e.printStackTrace();
				break;
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}

			handleConnection(socket);
		}
	}

	public int getPort() {
		return _server.getLocalPort();
	}

	protected abstract void handleConnection(Socket socket);
}

