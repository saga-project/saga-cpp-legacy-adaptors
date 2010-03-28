/**
 * $AIST_Release: 4.2.4 $
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
 * $RCSfile: CommunicationManager.java,v $ $Revision: 1.55 $ $Date: 2006/08/04 11:02:08 $
 */

package org.apgrid.grpc.ng;

import java.util.List;
import java.util.Timer;

import org.apgrid.grpc.ng.info.*;
import org.apgrid.grpc.ng.protocol.ProtCancelSessionReply;
import org.apgrid.grpc.ng.protocol.ProtExitExeReply;
import org.apgrid.grpc.ng.protocol.ProtFunctionCompleteNotify;
import org.apgrid.grpc.ng.protocol.ProtIAmAliveNotify;
import org.apgrid.grpc.ng.protocol.ProtInvokeCallbackNotify;
import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;

class CommunicationManager implements Runnable {
	private int executableID;
	private NgGrpcClient context;
	private NgGrpcHandle handle;
	private Communicator comm;
	private int target;
	private Protocol targetProtocol;	
	private Protocol receivedProtocol;
	private CondWait complete_func;
	private CondWait receive_reply;
	private int sequenceNum = 1;
	private NgLog commLog;
	private Timer hbTimer;
	private String server;
	private String client;
	private long session_timeout = 0;
	
	/* heartbeat */
	private int heartBeatInterval;
	private int heartBeatTimeoutTime;
	private int heartBeatTimeoutTimeOnTransfer;
	private volatile boolean heartBeatIsDataTransferring;
	private long lastTime;
	private boolean isValid;
	
	private static final int INVALID_TARGET = -9999;
	
	/**
	 * @param context
	 * @param handle
	 * @throws GrpcException
	 */
	public CommunicationManager(NgGrpcClient context, NgGrpcHandle handle) throws GrpcException {
		this.context = context;
		this.handle = handle;
		this.executableID = handle.getExecutableID();
		this.comm = context.getPortManagerNoSecure().getCommunicator(
			executableID, handle.getJobStartTimeout(), handle.getJob());
		this.hbTimer = null;
		this.server = this.comm.getPeer().getHostName();
		this.client = this.comm.getMySelf().getHostName();
		
		/* set commLog environment */
		RemoteMachineInfo remoteMachineInfo = handle.getRemoteMachineInfo();
		String commLogEnable = 
			(String) remoteMachineInfo.get(
			RemoteMachineInfo.KEY_COMMLOG_ENABLE);
		if (commLogEnable != null) {
			if (new Boolean(commLogEnable).booleanValue() == true) {
				this.commLog = new NgLog(
					handle.getRemoteMachineInfo(), this.executableID);
				this.commLog.setClientName(this.client);
				this.commLog.setServerName(this.server);
			}
		}
		
		/* reset target */
		this.target = INVALID_TARGET;
		
		/* set last time */
		this.lastTime = System.currentTimeMillis() / 1000;
		
		/* set data transferring */
		this.heartBeatIsDataTransferring = false;
		
		/* set heartbeat interval */
		this.heartBeatInterval =
			Integer.parseInt((String)remoteMachineInfo.get(
			RemoteMachineInfo.KEY_HEARTBEAT));
		if (this.heartBeatInterval < 0) {
			throw new NgInitializeGrpcHandleException("heartbeat is invalid.");
		}
		/* set heartbeat timeout time */
		int heartBeatTimeoutCount = 
			Integer.parseInt((String)remoteMachineInfo.get(
			RemoteMachineInfo.KEY_HEARTBEAT_TIMEOUTCOUNT));
		this.heartBeatTimeoutTime = 
			heartBeatInterval * heartBeatTimeoutCount;
		/* set heartbeat timeout time on transfer */
		int heartBeatTimeoutCountOnTransfer = 
			Integer.parseInt((String)remoteMachineInfo.get(
			RemoteMachineInfo.KEY_HEARTBEAT_TIMEOUTCOUNT_ONTRANSFER));
		if (heartBeatTimeoutCountOnTransfer == -1) {
			this.heartBeatTimeoutTimeOnTransfer = 
				heartBeatTimeoutCount;
		} else {
			/* 0 means do not check heartbeat on transfer */
			this.heartBeatTimeoutTimeOnTransfer = 
				heartBeatInterval * heartBeatTimeoutCountOnTransfer;			
		}
		
		
		/* Init valid flag */
		this.isValid = true;
		
		/* initialize Communicator */
		comm.initComm(handle.getRemoteMachineInfo());
		
		/* start HeartbeatTimer */
		startHeartbeatTimer();
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		while (true) {
			/* get protocol data from Executable */
			Protocol prot = null;
			try {
				prot = comm.getCommand();
			} catch (GrpcException e) {
				try {
					context.getNgLog().printLog(
						NgLog.LOGCATEGORY_NINFG_GRPC,
						NgLog.LOGLEVEL_FATAL,
						e);
				} catch (GrpcException e1) {
					/* can't manage */
				}
			}
			
			/* check if prot is null */
			if (prot == null) {
				try {
					context.getNgLog().printLog(
						NgLog.LOGCATEGORY_NINFG_GRPC,
						NgLog.LOGLEVEL_ERROR,
						handle,
						"received null Protocol.");
				} catch (GrpcException e) {
					/* can't manage */
				}
				//putLogMessage("GET PROTOCOL: received null data");
				setInvalid();
				break;
			}
			
			/* set commLog to Protocol */
			prot.setCommLog(commLog);
			
			/* parse Param */
			try {
				comm.parseParam(prot, targetProtocol);
			} catch (GrpcException e) {
				try {
					context.getNgLog().printLog(
						NgLog.LOGCATEGORY_NINFG_GRPC,
						NgLog.LOGLEVEL_FATAL,
						e);
				} catch (GrpcException e1) {
					/* can't manage */
				}
			}
			
			/* exit when received Reply of Exit */
			if (prot instanceof ProtExitExeReply) {
				receivedProtocol = prot;
				receive_reply.set();
				break;
			}
			
			/* send condsignal when received Reply of Cancel */
			if (prot instanceof ProtCancelSessionReply) {
				complete_func.set();
			}
			
			/* check if it's protocol of target */
			if (prot.getType() == target) {
				receivedProtocol = prot;
				receive_reply.set();
				continue;
			}
			
			/* check if it's notification and process it */
			if (prot instanceof ProtIAmAliveNotify) {
				lastTime = System.currentTimeMillis() / 1000;
			} else if (prot instanceof ProtFunctionCompleteNotify) {
				complete_func.set();
			} else if (prot instanceof ProtInvokeCallbackNotify) {
				/* Invoke Ninf-G Callback */
				NgInvokeCallback invokeCallback =
					new NgInvokeCallback(context, handle, prot);
				new Thread(invokeCallback, "invokeCallback").start();
			}
		}
		try {
			/* stop HeartbeatTimer */
			stopHeartbeatTimer();
			/* finalize Communicator */
			comm.finalComm();
			/* close Communicator */
			comm.close();
			/* put Communicator to PortManager */
			/*
			RemoteMachineInfo remoteMachineInfo = handle.getRemoteMachineInfo();
			if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CRYPT).equals("none")) {
				context.getPortManagerNoSecure().putCommunicator(executableID, comm);
			} else if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CRYPT).equals("SSL")) {
				context.getPortManagerSSL().putCommunicator(executableID, comm);
			} else if (remoteMachineInfo.get(RemoteMachineInfo.KEY_CRYPT).equals("GSI")) {
				context.getPortManagerGSI().putCommunicator(executableID, comm);
			}
			*/
		} catch (GrpcException e) {
			try {
				context.getNgLog().printLog(
					NgLog.LOGCATEGORY_NINFG_GRPC,
					NgLog.LOGLEVEL_ERROR,
					e);
			} catch (GrpcException e1) {
				/* can't manage */
			}
		}
	}
	
	/**
	 * @param sendProt
	 * @return
	 */
	protected synchronized Protocol sendProtocol(
		Protocol sendProt) throws GrpcException {
		/* check if it's valid */
		if (isValid != true) {
			throw new NgHeartbeatTimeoutException("timeout or something wrong was happened.");
		}

		/* set expected Protocol */
		this.receivedProtocol = null;
		this.receive_reply = new CondWait();
		this.target = sendProt.getType();
		this.targetProtocol = sendProt;
		
		/* set commLog to Protocol */
		sendProt.setCommLog(commLog);
		
		try {
			this.heartBeatIsDataTransferring = true;
			
			/* send Protocol */
			comm.putCommand(sendProt);
		
			/* waiting for expected Protocol */
			this.receive_reply.waitFor();
			if (isValid != true) {
				throw new NgHeartbeatTimeoutException("timeout or something wrong was happened.");
			}
		} finally {
			this.lastTime = System.currentTimeMillis() / 1000;
			this.heartBeatIsDataTransferring = false;
		}
		
		/* reset */
		this.receive_reply = null;
		this.target = INVALID_TARGET;
		this.targetProtocol = null;
		
		/* return Reply Protocol */
		return receivedProtocol;
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void waitCompleteFunction() throws GrpcException {
		/* check if it's valid */
		if (isValid != true) {
			throw new NgHeartbeatTimeoutException("timeout or something wrong was happened.");
		}

		/* session timeout and wait for complete function notify */
		if (session_timeout != 0) {
			long timeout = session_timeout - (System.currentTimeMillis() / 1000);
			if ((timeout <= 0) || (complete_func.waitFor(timeout) == -1)) {
				setInvalid();
			}
		} else {
			complete_func.waitFor();			
		}
		
		if (isValid != true) {
			throw new NgHeartbeatTimeoutException("timeout or something wrong was happened.");
		}
	}
	
	/**
	 * 
	 */
	protected void resetCompleteFunction() {
		complete_func = new CondWait();
	}

	/**
	 * 
	 */
	protected int generateSequenceNum() {
		return sequenceNum++;
	}
	
	/**
	 * @return
	 */
	protected List getEncodeTypes() {
		if (comm == null) {
			return null;
		} else {
			return comm.getEncodeTypes();
		}
	}
	
	/**
	 * @return
	 */
	protected boolean isValid() {
		return isValid;
	}
	
	/**
	 * 
	 */
	protected void checkHeartbeat() {
		int timeoutTime = this.heartBeatTimeoutTime;
		if (this.heartBeatIsDataTransferring) {
			timeoutTime = this.heartBeatTimeoutTimeOnTransfer;
			if (timeoutTime == 0) {
				 return;
			}
		}
		/* check if it's timed out */
		if (((System.currentTimeMillis() / 1000) - lastTime) > timeoutTime) {
			/* timeout!!! */
			try {
				/* put error message */
				context.getNgLog().printLog(
					NgLog.LOGCATEGORY_NINFG_GRPC,
					NgLog.LOGLEVEL_ERROR,
					context,
					"HeartBeat timeout was detected.");
				/* close Communicator */
				comm.close();
			} catch (GrpcException e) {
				try {
					context.getNgLog().printLog(
						NgLog.LOGCATEGORY_NINFG_GRPC,
						NgLog.LOGLEVEL_ERROR,
						e);
				} catch (GrpcException e1) {
					/* can't manage */
				}
			} finally {
				try {
					/* stop HeartbeatTimer */
					stopHeartbeatTimer();
				} catch (GrpcException e1) {
					/* can't manage */
				}
				setInvalid();
			}
		}
	}

	/**
	 * Starts HeartBeatTimer.
	 * @throws GrpcException
	 */
	private void startHeartbeatTimer() throws GrpcException {
		/* start timer for Heartbeat */
		if (heartBeatInterval != 0) {
			context.getNgLog().printLog(
				NgLog.LOGCATEGORY_NINFG_INTERNAL,
				NgLog.LOGLEVEL_INFO,
				handle,
				"CommunicationManager#startHeartbeatTimer () : start NgHeartbeatTimer.");
			this.hbTimer = new Timer();
			this.hbTimer.schedule(new NgHeartbeatTimer(context, this), 
				1000, heartBeatInterval * 1000);
		}
	}
	
	/**
	 * Stops HeartBeatTimer.
	 */
	private void stopHeartbeatTimer() throws GrpcException {
		/* stop HeartBeat timer */
		context.getNgLog().printLog(
			NgLog.LOGCATEGORY_NINFG_INTERNAL,
			NgLog.LOGLEVEL_INFO,
			handle,
			"CommunicationManager#stopHeartbeatTimer () : stop NgHeartbeatTimer.");
		if (hbTimer != null) {
			this.hbTimer.cancel();
			this.hbTimer = null;
		}
	}
	
	/**
	 * @param timeout
	 */
	protected void setSessionTimeout(long timeout) {
		if (timeout != 0) {
			this.session_timeout =
				(System.currentTimeMillis() / 1000) + timeout;
		}
	}
	
	/**
	 * 
	 */
	protected void checkSessionTimeout() {
		if ((session_timeout != 0) && 
				((System.currentTimeMillis() / 1000) >= session_timeout)) {
			/* timeout!!! */
			setInvalid();
		}
	}
	
	/**
	 * 
	 */
	private void setInvalid() {
		isValid = false;
		if (complete_func != null) {
			complete_func.set();
		}
		if (receive_reply != null) {
			receive_reply.set();
		}
	}
	
	/**
	 * @return
	 */
	protected int getVersionMajor() {
		return comm.getVersionMajor();
	}
	
	/**
	 * @return
	 */
	protected int getVersionMinor() {
		return comm.getVersionMinor();
	}
	
	/**
	 * @return
	 */
	protected int getVersionPatch() {
		return comm.getVersionPatch();
	}
}
