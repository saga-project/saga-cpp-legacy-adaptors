/*
 * $RCSfile: CommunicationManager.java,v $ $Revision: 1.16 $ $Date: 2008/03/13 11:06:37 $
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

import java.util.List;

import org.apgrid.grpc.ng.info.*;
import org.apgrid.grpc.ng.protocol.ProtCancelSessionReply;
import org.apgrid.grpc.ng.protocol.ProtExitExeReply;
import org.apgrid.grpc.ng.protocol.ProtTransferArgumentReply;
import org.apgrid.grpc.ng.protocol.ProtTransferCallbackResultReply;
import org.apgrid.grpc.ng.protocol.ProtConnectionCloseRequest;
import org.apgrid.grpc.ng.protocol.ProtConnectionCloseReply;
import org.apgrid.grpc.ng.protocol.ProtFunctionCompleteNotify;
import org.apgrid.grpc.ng.protocol.ProtIAmAliveNotify;
import org.apgrid.grpc.ng.protocol.ProtInvokeCallbackNotify;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.NgLog.CAT_NG_GRPC;
import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;

/*
 * This class manage communication between Ninf-G Client and Ninf-G Executable. 
 */
class CommunicationManager implements Runnable {

	private class CommunicationManagerCallback implements NgCallbackInterface {
		public void callback(List args) {
			hbTimer.touch();
		}
	}

	private int executableID;
	private int sequenceNum = 1;
	private NgGrpcClient context;
	private NgGrpcHandle handle;
	private Communicator communicator;
	private List encodeType;
	private boolean keep_connection = true;
	private int target; // target protocol type
	private NgProtocolRequest targetProtocol;
	private Protocol receivedProtocol;
	private CondWait complete_func;
	private CondWait receive_reply;
	private long session_timeout = 0;
	private NgHeartbeatTimer hbTimer; // heartbeat 
	private boolean isValid;
	private static final int INVALID_TARGET = -9999;
	private NgLog commLog = null;

	/**
	 * @param context
	 * @param handle
	 * @throws GrpcException
	 */
	public CommunicationManager(NgGrpcClient context, NgGrpcHandle handle)
	 throws GrpcException {
		this.context = context;
		this.handle  = handle;
		this.executableID = handle.getExecutableID();
		this.communicator = getCommunicator();
		this.communicator.setReceiveCallback(new CommunicationManagerCallback());
		this.encodeType = communicator.getEncodeTypes();
		this.hbTimer = null;

		setupCommunicationLog();

		// reset the target protocol type
		this.target = INVALID_TARGET;

		// setup fields of heartbeat info
		setupHeartBeatTimer();

		// Init valid flag 
		this.isValid = true;

		// initialize Communicator
		communicator.initComm(handle.getRemoteMachineInfo());
	}

	// Helper for constructor
	private Communicator getCommunicator() throws GrpcException {
		if ((this.handle == null) || (this.context == null)) {
			throw new IllegalStateException("handle or context field is not set.");
		}
		return this.context.getPortManagerNoSecure()
					.getCommunicator(handle.getExecutableID(),
									 handle.getJobStartTimeout(),
						 			 handle.getJob());
	}

	protected boolean reconnect() throws GrpcException {
		logDebug("reconnect(): Start...");
		communicator = this.context.getPortManagerNoSecure()
			.getCommunicatorNoTimeout(handle.getExecutableID(),
				handle.getJob());
		if (communicator == null) {
			return false;
		}
		setupCommunicationLog();
		communicator.initComm(handle.getRemoteMachineInfo());
		logDebug("reconnect(): Connected.");
		return true;
	}

	/* 
	 * Setup the CommunicationLog, if 'commLog_enable' attribute set
	 *  in client configuration. 
	 * Helper for constructor
	 */
	private void setupCommunicationLog() throws GrpcException {
		if ((this.communicator == null) || (this.handle == null)) {
			throw new IllegalArgumentException();
		}

		// set commLog environment 
		if (this.commLog != null) {
			this.communicator.setCommLog(this.commLog);
		} else {
			RemoteMachineInfo rmi = this.handle.getRemoteMachineInfo();
			String commLogEnable  = rmi.getCommLogInfo().getCommlogEnable();
			if ( (commLogEnable != null) &&
				 (Boolean.parseBoolean(commLogEnable)) ) {
				this.communicator.setCommLog( this.commLog
					= new NgLog(rmi, this.handle.getExecutableID()) );
			}
		}
	}

	class HBHandler implements HeartbeatHandler {
		public void timeout() {
			try {
				// put error message 
				context.getNgLog().logError(CAT_NG_GRPC, context.logHeader() +
					"HeartBeat timeout was detected.");
				communicator.close();
				communicator = null;
			} catch (GrpcException e) {
				context.getNgLog().logError(CAT_NG_GRPC, e.toString());
			} finally {
				stopHeartbeatTimer();
				setInvalid();
			}
		}
	}

	private void setupHeartBeatTimer() throws GrpcException {
		if (this.handle == null) {
			throw new IllegalStateException("handle field is not set.");
		}

		HeartbeatHandler hbHandle = new HBHandler();
		HeartbeatInfo heartbeatInfo =
			 this.handle.getRemoteMachineInfo().getHeartbeatInfo();

		this.hbTimer = new NgHeartbeatTimer(heartbeatInfo, hbHandle);
	}

	/*
	 * @return The current time in seconds.
	 */
	private long currentTime() {
		return System.currentTimeMillis() / 1000;
	}

	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {

		startHeartbeatTimer();

		logDebug("run(): Start Communication Managing Thread");
		while (true) {
			// get protocol data from Executable
			Protocol recv = null;
			try {
				recv = communicator.getCommand();
			} catch (GrpcException e) {
				context.getNgLog().logFatal(CAT_NG_GRPC, e);
			}

			// check if prot is null 
			if (recv == null) {
				context.getNgLog().logError(CAT_NG_GRPC,
					handle.logHeader() + "run(): received null Protocol.");
				setInvalid();
				break;
			}

			// parse Param 
			try {
				// targetProtocol !!
				communicator.parseParam(recv, targetProtocol);
			} catch (GrpcException e) {
				context.getNgLog().logFatal(CAT_NG_GRPC, e);
			}
			logDebug("run(): received " + recv);

			// exit when received Reply of Exit 
			if (recv instanceof ProtExitExeReply) {
				receivedProtocol = recv;
				receive_reply.set();
				break; // !! Exit
			}

			// send condsignal when received Reply of Cancel
			else if (recv instanceof ProtCancelSessionReply) {
				setKeepConnection(true);
				receivedProtocol = recv;
				receive_reply.set();
			} else if (recv instanceof ProtConnectionCloseReply) {
				targetProtocol = null;
				try {
					close();
				} catch (GrpcException e) {
					context.getNgLog().logFatal(CAT_NG_GRPC, e);
				}
				try {
					handle.unlockHandle();
				} catch (GrpcException e) {
					context.getNgLog().logFatal(CAT_NG_GRPC, e);
				}
				return;
			}
			
			// check if it's protocol of target 
			// target !!
			else if (recv.getType() == target) {
				receivedProtocol = recv;
				if (keep_connection) {
					receive_reply.set();	
					continue;
				}
				if (!(recv instanceof ProtTransferCallbackResultReply) &&
						!(recv instanceof ProtTransferArgumentReply)) {
					receive_reply.set();
					continue;
				}
				try {
					close();
				} catch (GrpcException e) {
					context.getNgLog().logFatal(CAT_NG_GRPC, e);
				}
				receive_reply.set();
				return;
			}

			// check if it's notification and process it  (Notify)
			else if (recv instanceof ProtIAmAliveNotify) {
				logDebug("run(): received I Am Alive Notify");
				this.hbTimer.touch();
				if (handle.isCanceled())
					continue;
				if ((receive_reply == null) && (!keep_connection)) {
					try {
						sendRequestConnectionClose();
					} catch (GrpcException e) {
						context.getNgLog().logFatal(CAT_NG_GRPC, e);
					}
				}
			} else if (recv instanceof ProtFunctionCompleteNotify) {
				if (targetProtocol instanceof ProtConnectionCloseRequest) {
					logDebug("run(): received Function Complete Notify ignore.");
					targetProtocol = null;
					continue;
				}
				logDebug("run(): received Function Complete Notify");
				setKeepConnection(true);
				complete_func.set(); // notify
			} else if (recv instanceof ProtInvokeCallbackNotify) {
				// Invoke Ninf-G Callback 
				NgInvokeCallback invokeCallback =
					new NgInvokeCallback(context, handle, recv);
				new Thread(invokeCallback, "invokeCallback").start();
			}
		}

		stopHeartbeatTimer(); // stop HeartbeatTimer 
		logDebug("run(): End of Communication Managing Thread");
	}

	/**
	 * Sends request of connectionClose.
	 */
	protected void sendRequestConnectionClose() throws GrpcException {
		try {
			handle.lockHandle();
			NgProtocolRequest connectionCloseRequest =
				new ProtConnectionCloseRequest(
					generateSequenceNum(),
					context.getID(), executableID,
					handle.getVersion());
			sendProtocolNoWaitReply(connectionCloseRequest);
		} catch (GrpcException e) {
			handle.unlockHandle();
		}
	}

	private void close() throws GrpcException {
		logDebug("close(): Clean up");
		if (communicator != null) {
			communicator.close();  // close Communicator 
			communicator = null;
		}
	}


	// dispose the Communication Manager
	public void dispose() throws GrpcException {
		logDebug("dispose(): Clean up");
		close();
		if (hbTimer != null) {
			stopHeartbeatTimer();//
			hbTimer = null;
		}
	}

	/**
	 * @param sendProt
	 * @return
	 */
	public synchronized Protocol sendProtocol(NgProtocolRequest req)
	 throws GrpcException {
		// check if it's valid 
		if (! isValid ) {
			throw new NgHeartbeatTimeoutException(
				"timeout or something wrong was happened.");
		}

		// set expected Protocol 
		this.receivedProtocol = null;
		this.receive_reply    = new CondWait();
		this.target           = req.getType();
		this.targetProtocol   = req;

		// send Protocol
		communicator.put(req);
		logDebug("sendProtocol(): Sent " + req);

		// waiting for expected Protocol received
		this.receive_reply.waitFor();
		if (! isValid ) {
			throw new NgHeartbeatTimeoutException(
				"timeout or something wrong was happened.");
		}

		// reset 
		this.receive_reply  = null;
		this.target         = INVALID_TARGET;
		this.targetProtocol = null;

		// return Reply Protocol
		// receivedProtocol set the run()
		return receivedProtocol;
	}

	/**
	 * @param sendProtNoWaitReply
	 * @return
	 */
	public synchronized void sendProtocolNoWaitReply(NgProtocolRequest req)
	 throws GrpcException {
		// check if it's valid 
		if (! isValid ) {
			throw new NgHeartbeatTimeoutException(
				"timeout or something wrong was happened.");
		}

		// set expected Protocol 
		this.receivedProtocol = null;
		this.target           = req.getType();
		this.targetProtocol   = req;

		// send Protocol
		communicator.put(req);
		logDebug("sendProtocol(): Sent " + req);
	}

	/**
	 * @throws GrpcException
	 */
	protected void waitCompleteFunction() throws GrpcException {
		// check if it's valid 
		if (! isValid ) {
			throw new NgHeartbeatTimeoutException(
				"timeout or something wrong was happened.");
		}

		// session timeout and wait for complete function notify 
		if (session_timeout != 0) {
			long timeout = session_timeout - currentTime();
			if ((timeout <= 0) ||
				(complete_func.waitFor(timeout * 1000) == -1)) {
				setInvalid();
			}
		} else {
			complete_func.waitFor();			
		}

		if (! isValid ) {
			throw new NgHeartbeatTimeoutException(
				"timeout or something wrong was happened.");
		}
	}

	protected boolean checkCompleteFunction() throws GrpcException {
		return complete_func.waitFor(1000) == 0;
	}

	protected void setKeepConnection(boolean keep_connection) {
		this.keep_connection = keep_connection;
	}

	protected void resetCompleteFunction() {
		complete_func = new CondWait();
	}

	protected int generateSequenceNum() {
		return sequenceNum++;
	}

	private void startHeartbeatTimer() {
		logInfo("startHeartbeatTimer(): Start NgHeartbeatTimer.");
		this.hbTimer.start();
	}
	
	private void stopHeartbeatTimer() {
		logInfo("stopHeartbeatTimer(): Stop NgHeartbeatTimer.");
		this.hbTimer.stop();
	}

	protected boolean isConnected() {
		return communicator != null;
	}
	
	/**
	 * @param timeout
	 */
	protected void setSessionTimeout(long timeout) {
		if (timeout < 0) throw new IllegalArgumentException();
		if (timeout != 0) {
			this.session_timeout = currentTime() + timeout;
		}
	}

	protected void checkSessionTimeout() {
		if (session_timeout == 0)
			return ;
		if (currentTime() >= session_timeout) {
			// timeout
			setInvalid();
		}
	}

	private void setInvalid() {
		synchronized (this) {
			isValid = false;
		}
		signalToCondition();
	}

	private void signalToCondition() {
		if (complete_func != null) {
			complete_func.set();
		}
		if (receive_reply != null) {
			receive_reply.set();
		}
	}

	private void logInfo(String msg) {
		context.getNgLog().logInfo(CAT_NG_INTERNAL,
			handle.logHeader() + "CommunicationManager#" + msg);
	}

	private void logDebug(String msg) {
		context.getNgLog().logDebug(CAT_NG_INTERNAL,
			handle.logHeader() + "CommunicationManager#" + msg);
	}

	protected boolean isValid() { return isValid; }

	public Version getVersion() { return communicator.getVersion(); }

	protected List getEncodeTypes() {
		return encodeType;
	}

}

