/*
 * $RCSfile: NgGrpcExecInfo.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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

import org.gridforum.gridrpc.GrpcExecInfo;

/**
 * Provides information about session.<br>
 * This class saves information of time to execute RemoteFunction/RemoteMethod.<br>
 * This class doesn't only implement standard interface of Grid RPC
 * but also has functions which save extra informations.<br>
 * <br>
 * NOTE: The information of CPU time on Java client is not available.
 */
public class NgGrpcExecInfo implements GrpcExecInfo {
	private double remoteMachineInfoRequestRealTime;
	private double remoteMachineInfoRequestCPUTime;
	private double remoteClassInfoRequestRealTime;
	private double remoteClassInfoRequestCPUTime;
	private double gramInvokeRealTime;
	private double gramInvokeCPUTime;

	/* session information on client side */
	SessionInformation clientSessionInfo;
	/* session information on server side */
	SessionInformation serverSessionInfo;
	
	/* ----- get information from this class ----- */
	/**
	 * Gets the time of searching information about a server and
	 * RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getLookupTime() {
		return remoteMachineInfoRequestRealTime + remoteClassInfoRequestRealTime;
	}
	
	/**
	 * Gets the time of searching information about a server.
	 * 
	 * @return the time in second.
	 */
	public double getLookupServerInfoTime() {
		return remoteMachineInfoRequestRealTime;		
	}
	
	/**
	 * Gets the time of searching information about a RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getLookupClassInfoTime() {
		return remoteClassInfoRequestRealTime;		
	}
	
	/**
	 * Gets the time of invoking a RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getInvokeTime() {
		return gramInvokeRealTime;
	}
	
	/**
	 * Gets the CPU time of searching information about a server.
	 * 
	 * @return the time in second.
	 */
	public double getLookupServerInfoCPUTime() {
		return remoteMachineInfoRequestCPUTime;		
	}
	
	/**
	 * Gets the CPU time of searching information about a RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getLookupClassInfoCPUTime() {
		return remoteClassInfoRequestCPUTime;		
	}
	
	/**
	 * Gets the CPU time of invoking a RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getInvokeCPUTime() {
		return gramInvokeCPUTime;
	}
	
	/**
	 * Sets the time of searching information about a server.
	 * 
	 * @param remoteMachineInfoRequestRealTime the time in second.
	 */
	protected void setLookupServerInfoTime(double remoteMachineInfoRequestRealTime) {
		this.remoteMachineInfoRequestRealTime = remoteMachineInfoRequestRealTime;		
	}
	
	/**
	 * Sets the time of searching information about a RemoteFunction/RemoteMethod.
	 * 
	 * @param remoteClassInfoRequestRealTime the time in second.
	 */
	protected void setLookupClassInfoTime(double remoteClassInfoRequestRealTime) {
		this.remoteClassInfoRequestRealTime = remoteClassInfoRequestRealTime;		
	}
	
	/**
	 * Sets the time of invoking a RemoteFunction/RemoteMethod.
	 * 
	 * @param gramInvokeRealTime the time in second.
	 */
	protected void setInvokeTime(double gramInvokeRealTime) {
		this.gramInvokeRealTime = gramInvokeRealTime;
	}
	
	/**
	 * Sets the CPU time of searching information about a server.
	 * 
	 * @param remoteMachineInfoRequestCPUTime the time in second.
	 */
	protected void setLookupServerInfoCPUTime(double remoteMachineInfoRequestCPUTime) {
		this.remoteMachineInfoRequestCPUTime = remoteMachineInfoRequestCPUTime;		
	}
	
	/**
	 * Sets the CPU time of searching information about a RemoteFunction/RemoteMethod.
	 * 
	 * @param remoteClassInfoRequestCPUTime the time in second.
	 */
	protected void setLookupClassInfoCPUTime(double remoteClassInfoRequestCPUTime) {
		this.remoteClassInfoRequestCPUTime = remoteClassInfoRequestCPUTime;		
	}
	
	/**
	 * Sets the CPU time of invoking a RemoteFunction/RemoteMethod.
	 * 
	 * @param gramInvokeCPUTime the time in second.
	 */
	protected void setInvokeCPUTime(double gramInvokeCPUTime) {
		this.gramInvokeCPUTime = gramInvokeCPUTime;
	}
	
	/* ----- get information from SessionInfo of client ----- */
	/**
	 * Gets the time of sending data of arguments.
	 * 
	 * @return the time in second.
	 */
	public double getForeTime() {
		return clientSessionInfo.getCallbackTransferResultRealTime();
	}
	
	/**
	 * Gets the time of executing RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getExecTime() {
		return clientSessionInfo.getCallbackCalculationRealTime();
	}
	
	/**
	 * Gets the time of receiving data of results.
	 * 
	 * @return the time in second.
	 */
	public double getBackTime() {
		return clientSessionInfo.getCallbackTransferResultRealTime();
	}
	
	/**
	 * Gets the CPU time of sending data of arguments.
	 * 
	 * @return the time in second.
	 */
	public double getForeCPUTime() {
		return clientSessionInfo.getCallbackTransferResultCPUTime();
	}
	
	/**
	 * Gets the CPU time of executing RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getExecCPUTime() {
		return clientSessionInfo.getCallbackCalculationCPUTime();
	}
	
	/**
	 * Gets the CPU time of receiving data of results.
	 * 
	 * @return the time in second.
	 */
	public double getBackCPUTime() {
		return clientSessionInfo.getCallbackTransferResultCPUTime();
	}
	
	/**
	 * Sets the time of sending data of arguments.
	 * 
	 * @param transformArgumentRealTime the time in second.
	 */
	protected void setForeTime(double transformArgumentRealTime) {
		clientSessionInfo.setTransferArgumentRealTime(transformArgumentRealTime);
	}
	
	/**
	 * Sets the time of executing RemoteFunction/RemoteMethod.
	 * 
	 * @param calculationRealTime the time in second.
	 */
	protected void setExecTime(double calculationRealTime) {
		clientSessionInfo.setCalculationRealTime(calculationRealTime);
	}
	
	/**
	 * Gets the time of receiving data of results.
	 * 
	 * @param transferResultRealTime the time in second.
	 */
	protected void setBackTime(double transferResultRealTime) {
		clientSessionInfo.setTransferResultRealTime(transferResultRealTime);
	}
	
	/**
	 * Sets the CPU time of sending data of arguments.
	 * 
	 * @param transformArgumentRealTime the time in second.
	 */
	protected void setForeCPUTime(double transformArgumentCPUTime) {
		clientSessionInfo.setTransferArgumentCPUTime(transformArgumentCPUTime);
	}
	
	/**
	 * Sets the CPU time of executing RemoteFunction/RemoteMethod.
	 * 
	 * @param calculationRealTime the time in second.
	 */
	protected void setExecCPUTime(double calculationCPUTime) {
		clientSessionInfo.setCalculationCPUTime(calculationCPUTime);
	}
	
	/**
	 * Gets the CPU time of receiving data of results.
	 * 
	 * @param transferResultRealTime the time in second.
	 */
	protected void setBackCPUTime(double transferResultCPUTime) {
		clientSessionInfo.setTransferResultCPUTime(transferResultCPUTime);
	}
	
	/* ----- get information from SessionInfo of server ----- */
	/**
	 * Gets the time of sending data of arguments.
	 * 
	 * @return the time in second.
	 */
	public double getForeTimeOnServer() {
		return serverSessionInfo.getCallbackTransferResultRealTime();
	}
	
	/**
	 * Gets the time of executing RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getExecTimeOnServer() {
		return serverSessionInfo.getCallbackCalculationRealTime();
	}
	
	/**
	 * Gets the time of receiving data of results.
	 * 
	 * @return the time in second.
	 */
	public double getBackTimeOnServer() {
		return serverSessionInfo.getCallbackTransferResultRealTime();
	}
	
	/**
	 * Gets the CPU time of sending data of arguments.
	 * 
	 * @return the time in second.
	 */
	public double getForeCPUTimeOnServer() {
		return serverSessionInfo.getCallbackTransferResultCPUTime();
	}
	
	/**
	 * Gets the CPU time of executing RemoteFunction/RemoteMethod.
	 * 
	 * @return the time in second.
	 */
	public double getExecCPUTimeOnServer() {
		return serverSessionInfo.getCallbackCalculationCPUTime();
	}
	
	/**
	 * Gets the CPU time of receiving data of results.
	 * 
	 * @return the time in second.
	 */
	public double getBackCPUTimeOnServer() {
		return serverSessionInfo.getCallbackTransferResultCPUTime();
	}
	
	/* ----- get/set SessionInformation ----- */
	/**
	 * Gets information which was measured on a client.
	 * 
	 * @return {@link SessionInformation}.
	 */
	protected SessionInformation getClientSessionInformation() {
		return clientSessionInfo;
	}
	
	/**
	 * Gets information which was measured on a server.
	 * 
	 * @return {@link SessionInformation}.
	 */
	protected SessionInformation getServerSessionInformation() {
		return serverSessionInfo;
	}
	
	/**
	 * Sets information which was measured on a client.
	 * 
	 * @param clientSessionInformation SessionInformation to set.
	 */
	protected void setClientSessionInformation(
		SessionInformation clientSessionInformation) {
		this.clientSessionInfo = clientSessionInformation;
	}

	/**
	 * Sets information which was measured on a server.
	 * 
	 * @param serverSessionInformation SessionInformation to set.
	 */
	protected void setServerSessionInformation(
		SessionInformation serverSessionInformation) {
		this.serverSessionInfo = serverSessionInformation;
	}
	
	/* ----- interface for getting SessionInformation ----- */
	/* time to transfer file from client to remote(Real) */
	/**
	 * Gets the Real time of sending file of argument mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileClientToRemoteRealTimeOnClient() {
		return getTransferFileClientToRemoteRealTime(clientSessionInfo);
	}
	
	/**
	 * Gets the Real time of sending file of argument mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileClientToRemoteRealTimeOnServer() {
		return getTransferFileClientToRemoteRealTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getTransferFileClientToRemoteRealTime(SessionInformation sessionInfo) {
		return sessionInfo.getTransferFileClientToRemoteRealTime();
	}
	
	/* time to transfer file from client to remote(CPU) */
	/**
	 * Gets the CPU time of sending file of argument mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileClientToRemoteCPUTimeOnClient() {
		return getTransferFileClientToRemoteCPUTime(clientSessionInfo);
	}
	
	/**
	 * Gets the CPU time of sending file of argument mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileClientToRemoteCPUTimeOnServer() {
		return getTransferFileClientToRemoteCPUTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getTransferFileClientToRemoteCPUTime(SessionInformation sessionInfo) {
		return sessionInfo.getTransferFileClientToRemoteCPUTime();
	}
	
	/* time to transfer file from remote to client(Real) */
	/**
	 * Gets the Real time of receiving file of argument mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileRemoteToClientRealTimeOnClient() {
		return getTransferFileRemoteToClientRealTime(clientSessionInfo);
	}
	
	/**
	 * Gets the Real time of receiving file of argument mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileRemoteToClientRealTimeOnServer() {
		return getTransferFileRemoteToClientRealTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getTransferFileRemoteToClientRealTime(SessionInformation sessionInfo) {
		return sessionInfo.getTransferFileRemoteToClientRealTime();
	}
	
	/* time to transfer file from client to remote(CPU) */
	/**
	 * Gets the CPU time of receiving file of argument mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileRemoteToClientCPUTimeOnClient() {
		return getTransferFileRemoteToClientCPUTime(clientSessionInfo);
	}
	
	/**
	 * Gets the CPU time of receiving file of argument mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getTransferFileRemoteToClientCPUTimeOnServer() {
		return getTransferFileRemoteToClientCPUTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getTransferFileRemoteToClientCPUTime(SessionInformation sessionInfo) {
		return sessionInfo.getTransferFileRemoteToClientCPUTime();
	}
	
	/* ----- interface for getting SessionInformation(callback) ----- */
	/* time to transfer callback arguments(Real) */
	/**
	 * Gets the Real time of receiving callback arguments mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferArgumentRealTimeOnClient() {
		return getCallbackTransferArgumentRealTime(clientSessionInfo);
	}
	
	/**
	 * Gets the Real time of receiving callback arguments mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferArgumentRealTimeOnServer() {
		return getCallbackTransferArgumentRealTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCallbackTransferArgumentRealTime(SessionInformation sessionInfo) {
		return sessionInfo.getCallbackTransferArgumentRealTime();
	}
	
	/* time to transfer callback arguments(CPU) */
	/**
	 * Gets the CPU time of receiving callback arguments mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferArgumentCPUTimeOnClient() {
		return getCallbackTransferArgumentCPUTime(clientSessionInfo);
	}
	
	/**
	 * Gets the CPU time of receiving callback arguments mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferArgumentCPUTimeOnServer() {
		return getCallbackTransferArgumentCPUTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCallbackTransferArgumentCPUTime(SessionInformation sessionInfo) {
		return sessionInfo.getCallbackTransferArgumentCPUTime();
	}
	
	/* time to calculate callback(Real) */
	/**
	 * Gets the Real time of calculating callback on client.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackCalculationRealTimeOnClient() {
		return getCallbackCalculationRealTime(clientSessionInfo);
	}
	
	/**
	 * Gets the Real time of calculating callback on server.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackCalculationRealTimeOnServer() {
		return getCallbackCalculationRealTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCallbackCalculationRealTime(SessionInformation sessionInfo) {
		return sessionInfo.getCallbackCalculationRealTime();
	}
	
	/* time to calculate callback(CPU) */
	/**
	 * Gets the CPU time of calculating callback on client.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackCalculationCPUTimeOnClient() {
		return getCallbackCalculationCPUTime(clientSessionInfo);
	}
	
	/**
	 * Gets the CPU time of calculating callback on server.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackCalculationCPUTimeOnServer() {
		return getCallbackCalculationCPUTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCallbackCalculationCPUTime(SessionInformation sessionInfo) {
		return sessionInfo.getCallbackCalculationCPUTime();
	}
	
	/* time to transfer callback results(Real) */
	/**
	 * Gets the Real time of sending callback results mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferResultRealTimeOnClient() {
		return getCallbackTransferResultRealTime(clientSessionInfo);
	}
	
	/**
	 * Gets the Real time of sending callback results mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferResultRealTimeOnServer() {
		return getCallbackTransferResultRealTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCallbackTransferResultRealTime(SessionInformation sessionInfo) {
		return sessionInfo.getCallbackTransferResultRealTime();
	}
	
	/* time to transfer callback results(CPU) */
	/**
	 * Gets the CPU time of sending callback results mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferResultCPUTimeOnClient() {
		return getCallbackTransferResultCPUTime(clientSessionInfo);
	}
	
	/**
	 * Gets the CPU time of sending callback results mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getCallbackTransferResultCPUTimeOnServer() {
		return getCallbackTransferResultCPUTime(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCallbackTransferResultCPUTime(SessionInformation sessionInfo) {
		return sessionInfo.getCallbackTransferResultCPUTime();
	}
	
	/* count of calling callback */
	/**
	 * Gets the count of calling callback mesured on client.
	 * 
	 * @return the time in second.
	 */
	public int getCallbackNTimesCalledOnClient() {
		return getCallbackNTimesCalled(clientSessionInfo);
	}
	
	/**
	 * Gets the count of calling callback mesured on server.
	 * 
	 * @return the time in second.
	 */
	public int getCallbackNTimesCalledOnServer() {
		return getCallbackNTimesCalled(serverSessionInfo);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private int getCallbackNTimesCalled(SessionInformation sessionInfo) {
		return sessionInfo.getCallbackNTimesCalled();
	}
	
	/* ----- interface for getting CompressionInformation ----- */
	/* length of original data */
	/**
	 * Gets the length of original data mesured on client.
	 * 
	 * @param argID ID of the argument.
	 * @return the length in byte.
	 */
	public long getOriginalNbytesOnClient(int argID) {
		return getOriginalNbytes(clientSessionInfo, argID);
	}
	
	/**
	 * Gets the length of original data mesured on server.
	 * 
	 * @param argID ID of the argument.
	 * @return the length in byte.
	 */
	public long getOriginalNbytesOnServer(int argID) {
		return getOriginalNbytes(serverSessionInfo, argID);
	}
	
	/**
	 * @param sessionInfo
	 * @param argID
	 * @return
	 */
	private long getOriginalNbytes(SessionInformation sessionInfo, int argID) {
		return sessionInfo.getOriginalNbytes(argID);
	}
	
	/* length of compressed data */
	/**
	 * Gets the length of compressed data mesured on client.
	 * 
	 * @param argID ID of the argument.
	 * @return the length in byte.
	 */
	public long getCompressionNbytesOnClient(int argID) {
		return getCompressionNbytes(clientSessionInfo, argID);
	}
	
	/**
	 * Gets the length of compressed data mesured on server.
	 * 
	 * @param argID ID of the argument.
	 * @return the length in byte.
	 */
	public long getCompressionNbytesOnServer(int argID) {
		return getCompressionNbytes(serverSessionInfo, argID);
	}
	
	/**
	 * @param sessionInfo
	 * @param argID
	 * @return
	 */
	private long getCompressionNbytes(SessionInformation sessionInfo, int argID) {
		return sessionInfo.getCompressionNbytes(argID);
	}
	
	/* time to compress arguments(Real) */
	/**
	 * Gets the Real time of compressing arguments mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getCompressionRealTimeOnClient(int argID) {
		return getCompressionRealTime(clientSessionInfo, argID);
	}
	
	/**
	 * Gets the Real time of compressing arguments mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getCompressionRealTimeOnServer(int argID) {
		return getCompressionRealTime(serverSessionInfo, argID);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCompressionRealTime(SessionInformation sessionInfo, int argID) {
		return sessionInfo.getCompressionRealTime(argID);
	}
	
	/* time to compress arguments(CPU) */
	/**
	 * Gets the CPU time of compressing arguments mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getCompressionCPUTimeOnClient(int argID) {
		return getCompressionCPUTime(clientSessionInfo, argID);
	}
	
	/**
	 * Gets the CPU time of compressing arguments mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getCompressionCPUTimeOnServer(int argID) {
		return getCompressionCPUTime(serverSessionInfo, argID);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getCompressionCPUTime(SessionInformation sessionInfo, int argID) {
		return sessionInfo.getCompressionCPUTime(argID);
	}
	
	/* time to decompress arguments(Real) */
	/**
	 * Gets the Real time of decompressing arguments mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getDecompressionRealTimeOnClient(int argID) {
		return getDecompressionRealTime(clientSessionInfo, argID);
	}
	
	/**
	 * Gets the Real time of compressing arguments mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getDecompressionRealTimeOnServer(int argID) {
		return getDecompressionRealTime(serverSessionInfo, argID);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getDecompressionRealTime(SessionInformation sessionInfo, int argID) {
		return sessionInfo.getDeCompressionRealTime(argID);
	}
	
	/* time to compress arguments(CPU) */
	/**
	 * Gets the CPU time of compressing arguments mesured on client.
	 * 
	 * @return the time in second.
	 */
	public double getDecompressionCPUTimeOnClient(int argID) {
		return getDecompressionCPUTime(clientSessionInfo, argID);
	}
	
	/**
	 * Gets the CPU time of compressing arguments mesured on server.
	 * 
	 * @return the time in second.
	 */
	public double getDecompressionCPUTimeOnServer(int argID) {
		return getDecompressionCPUTime(serverSessionInfo, argID);
	}
	
	/**
	 * @param sessionInfo
	 * @return
	 */
	private double getDecompressionCPUTime(SessionInformation sessionInfo, int argID) {
		return sessionInfo.getDeCompressionCPUTime(argID);
	}
	
	/* --------------------------------------------------------------- */
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		StringBuffer sb = new StringBuffer();
		
		/* information RealTime which I have */
		sb.append("getServerInfo(REAL)   : "
			+ remoteMachineInfoRequestRealTime + "\n");
		sb.append("getFunctionInfo(REAL) : " 
			+ remoteClassInfoRequestRealTime + "\n");
		sb.append("gramInvoke(REAL)      : " 
			+ gramInvokeRealTime + "\n");

		/* information CPUTime which I have */
		sb.append("getServerInfo(CPU)    : " 
			+ remoteMachineInfoRequestCPUTime + "\n");
		sb.append("getFunctionInfo(CPU)  : " 
			+ remoteClassInfoRequestCPUTime + "\n");
		sb.append("gramInvoke(CPU)       : " 
			+ gramInvokeCPUTime + "\n");

		/* information in clientSessionInfo */
		if (clientSessionInfo != null) {
			sb.append(clientSessionInfo.toString());
		}
		
		/* information in serverSessionInfo */
		if (serverSessionInfo != null) {
			sb.append(" ===== information measured on server ===== " + "\n");
			sb.append(serverSessionInfo.toString());
		}

		/* return information String */
		return sb.toString();
	}
}
