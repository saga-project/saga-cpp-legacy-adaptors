/*
 * $RCSfile: SessionInformation.java,v $ $Revision: 1.7 $ $Date: 2007/11/27 02:27:42 $
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
import java.util.Vector;

import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

/*
 * Management the execution time of session
 */
public class SessionInformation {

	public static final String namespaceURI = "http://ninf.apgrid.org/2007/11/SessionInformation";
	private static final String tagName = "sessionInformation";
	private static final String tagRealTime = "realTime";
	private static final String tagCPUTime = "CPUtime";
	private static final String tagCallbackInfo = "callbackInformation";

	private static final String attrTransArg = "transferArgument";
	private static final String attrTransFileClientToRemote = "transferFileClientToRemote";
	private static final String attrCalculation = "calculation";
	private static final String attrTransResult = "transferResult";
	private static final String attrTransFileRemoteToClient = "transferFileRemoteToClient";
	private static final String attrCBTransArg = "callbackTransferArgument";
	private static final String attrCBCalculation = "callbackCalculation";
	private static final String attrCBTransResult = "callbackTransferResult";
	
	private static final String attrCallbackNTimesCalled = "numberOfTimesWhichCalled";


	private static final String secString = "s";
	private static final String usecString ="us";
	private static final String separator = " ";
	
	// protocol version 
	private Version protocol_ver;
	
	// information for executing Executable 
	private TimeInfo transferArgumentTime;
	private TimeInfo transferFileClientToRemoteTime;
	private TimeInfo transferFileRemoteToClientTime;
	private TimeInfo transferResultTime;

	private TimeInfo calculationTime;

	// information for callback
	private TimeInfo callbackTransferArgumentTime;
	private TimeInfo callbackTransferResultTime;
	private TimeInfo callbackCalculationTime;

	
	// information for callback
	private int nCallbacks;
	private int callbackNTimeCalled;
	private CallbackCompressInformation[] callbackCompressInformation;
	
	// information for compression 
	private int nArgs;
	private CompressInformation[] compressInfo;


	//-----  Constructors begin -------------------------------
	public SessionInformation(int nArgs, int nCallback) {
		this.nArgs = nArgs;
		this.nCallbacks = nCallback;
		this.callbackNTimeCalled = 0;
		this.protocol_ver = new Version(2,2,0);

		this.transferArgumentTime = new TimeInfo(0, 0);
		this.transferFileClientToRemoteTime = new TimeInfo(0, 0);
		this.transferFileRemoteToClientTime = new TimeInfo(0, 0);
		this.transferResultTime = new TimeInfo(0, 0);
		this.calculationTime = new TimeInfo(0, 0);
		this.callbackTransferArgumentTime = new TimeInfo(0, 0);
		this.callbackTransferResultTime = new TimeInfo(0, 0);
		this.callbackCalculationTime = new TimeInfo(0, 0);

		// init CompressInformation
		this.compressInfo = new CompressInformation[nArgs];
	}
	
	/**
	 * @param transferArgumentRealTime
	 * @param transferArgumentCPUTime
	 * @param transferFileClientToRemoteRealTime
	 * @param transferFileClientToRemoteCPUTime
	 * @param clientRealTime
	 * @param clientCPUTime
	 * @param transferResultRealTime
	 * @param transferResultCPUTime
	 * @param transferFileRemoteToClientReadTime
	 * @param transferFileRemoteToClientCPUTime
	 * @param callbackTransferArgumentRealTime
	 * @param callbackTransferArgumentCPUTime
	 * @param callbackCalculationRealTime
	 * @param callbackCalculationCPUTime
	 * @param callbackTransferResultRealTime
	 * @param callbackTransferResultCPUTime
	 */
	public SessionInformation(
		double transferArgumentRealTime, 
		double transferArgumentCPUTime,
		double clientRealTime,
		double clientCPUTime,
		double transferResultRealTime,
		double transferResultCPUTime,
		double callbackTransferArgumentRealTime,
		double callbackTransferArgumentCPUTime,
		double callbackCalculationRealTime,
		double callbackCalculationCPUTime,
		double callbackTransferResultRealTime, 
		double callbackTransferResultCPUTime)
	{
		this.transferArgumentTime = 
		 new TimeInfo(transferArgumentRealTime, transferArgumentCPUTime);

		this.calculationTime = 
		 new TimeInfo(clientRealTime, clientCPUTime);

		this.transferResultTime = 
		 new TimeInfo(transferResultRealTime, transferResultCPUTime);

		this.callbackTransferArgumentTime =
		 new TimeInfo(callbackTransferArgumentRealTime, callbackTransferArgumentCPUTime);

		this.callbackCalculationTime =
		 new TimeInfo(callbackCalculationRealTime, callbackCalculationCPUTime);

		this.callbackTransferResultTime =
		 new TimeInfo(callbackTransferResultRealTime, callbackTransferResultCPUTime);
		
		// set protocol version
		this.protocol_ver = new Version(2,0,0);
	}

	/**
	 * @param transferArgumentRealTime
	 * @param transferArgumentCPUTime
	 * @param transferFileClientToRemoteRealTime
	 * @param transferFileClientToRemoteCPUTime
	 * @param clientRealTime
	 * @param clientCPUTime
	 * @param transferResultRealTime
	 * @param transferResultCPUTime
	 * @param transferFileRemoteToClientReadTime
	 * @param transferFileRemoteToClientCPUTime
	 * @param callbackTransferArgumentRealTime
	 * @param callbackTransferArgumentCPUTime
	 * @param callbackCalculationRealTime
	 * @param callbackCalculationCPUTime
	 * @param callbackTransferResultRealTime
	 * @param callbackTransferResultCPUTime
	 */
	public SessionInformation(
		double transferArgumentRealTime, 
		double transferArgumentCPUTime,
		double transferFileClientToRemoteRealTime,
		double transferFileClientToRemoteCPUTime,
		double clientRealTime, 
		double clientCPUTime,
		double transferResultRealTime, 
		double transferResultCPUTime,
		double transferFileRemoteToClientRealTime,
		double transferFileRemoteToClientCPUTime,
		double callbackTransferArgumentRealTime, 
		double callbackTransferArgumentCPUTime,
		double callbackCalculationRealTime, 
		double callbackCalculationCPUTime,
		double callbackTransferResultRealTime, 
		double callbackTransferResultCPUTime)
	{
		this.transferArgumentTime =
		 new TimeInfo(transferArgumentRealTime, transferArgumentCPUTime);

		this.transferFileClientToRemoteTime =
		 new TimeInfo(transferFileClientToRemoteRealTime, transferFileClientToRemoteCPUTime);

		this.calculationTime =
		 new TimeInfo(clientRealTime, clientCPUTime);

		this.transferResultTime =
		 new TimeInfo(transferResultRealTime, transferResultCPUTime);

		this.transferFileRemoteToClientTime = 
		 new TimeInfo(transferFileRemoteToClientRealTime, transferFileRemoteToClientCPUTime);

		this.callbackTransferArgumentTime =
		 new TimeInfo(callbackTransferArgumentRealTime, callbackTransferArgumentCPUTime);

		this.callbackCalculationTime =
		 new TimeInfo(callbackCalculationRealTime, callbackCalculationCPUTime);

		this.callbackTransferResultTime =
		 new TimeInfo(callbackTransferResultRealTime, callbackTransferResultCPUTime);
		
		// set protocol version
		this.protocol_ver = new Version(2,1,0);
	}

	/**
	 * @param transferArgumentRealTime
	 * @param transferArgumentCPUTime
	 * @param transferFileClientToRemoteRealTime
	 * @param transferFileClientToRemoteCPUTime
	 * @param clientRealTime
	 * @param clientCPUTime
	 * @param transferResultRealTime
	 * @param transferResultCPUTime
	 * @param transferFileRemoteToClientRealTime
	 * @param transferFileRemoteToClientCPUTime
	 * @param callbackTransferArgumentRealTime
	 * @param callbackTransferArgumentCPUTime
	 * @param callbackCalculationRealTime
	 * @param callbackCalculationCPUTime
	 * @param callbackTransferResultRealTime
	 * @param callbackTransferResultCPUTime
	 * @param argCompressInformation
	 * @param callbackCompressInformation
	 */
	public SessionInformation(
			double transferArgumentRealTime, 
			double transferArgumentCPUTime,
			double transferFileClientToRemoteRealTime,
			double transferFileClientToRemoteCPUTime,
			double clientRealTime, double clientCPUTime,
			double transferResultRealTime, 
			double transferResultCPUTime,
			double transferFileRemoteToClientRealTime,
			double transferFileRemoteToClientCPUTime,
			double callbackTransferArgumentRealTime,
			double callbackTransferArgumentCPUTime,
			double callbackCalculationRealTime, 
			double callbackCalculationCPUTime,
			double callbackTransferResultRealTime,
			double callbackTransferResultCPUTime,
			CompressInformation[] argCompressInformation,
			CallbackCompressInformation[] callbackCompressInformation)
		{
			this.transferArgumentTime = 
			 new TimeInfo(transferArgumentRealTime, transferArgumentCPUTime);

			this.transferFileClientToRemoteTime =
			 new TimeInfo(transferFileClientToRemoteRealTime, transferFileClientToRemoteCPUTime);
			this.transferFileRemoteToClientTime =
			 new TimeInfo(transferFileRemoteToClientRealTime, transferFileRemoteToClientCPUTime); 
			this.transferResultTime = 
			 new TimeInfo(transferResultRealTime, transferResultCPUTime);

			this.calculationTime =
			 new TimeInfo(clientRealTime, clientCPUTime);

			this.callbackTransferArgumentTime =
			 new TimeInfo(callbackTransferArgumentRealTime, callbackTransferArgumentCPUTime); 
			this.callbackTransferResultTime =
			 new TimeInfo(callbackTransferResultRealTime, callbackTransferResultCPUTime); 
			this.callbackCalculationTime =
			 new TimeInfo(callbackCalculationRealTime, callbackCalculationCPUTime);

			this.compressInfo = argCompressInformation;
			/* 
			 * not implement on 2.4.0
			this.callbackCompressInformation = callbackCompressInformation;
			*/
			
			// set protocol version
			this.protocol_ver = new Version(2,2,0);
		}

	/**
	 * @param xmlString
	 * @throws GrpcException
	 */
	public SessionInformation(String xmlString, Version version)
	 throws GrpcException {
		this(XMLUtil.getNode(xmlString), version);
	}

	/**
	 * @param node
	 * @throws GrpcException
	 */
	public SessionInformation(Node node, Version version)
	 throws GrpcException {

		// set protocol version 
		this.protocol_ver =  version;

		// get information about realTime 
		Node nodeRealTime = XMLUtil.getChildNode(node, namespaceURI, tagRealTime);
		// get information about CPUtime
		Node nodeCPUTime = XMLUtil.getChildNode(node, namespaceURI, tagCPUTime);


		// get time of transferArgument
		this.transferArgumentTime =
		 new TimeInfo(getTimeValue(nodeRealTime, attrTransArg),
		              getTimeValue(nodeCPUTime, attrTransResult));

		if (version.getMinor() > 0) {
			// get time of transferFileClientToRemote 
			this.transferFileClientToRemoteTime =
			 new TimeInfo(getTimeValue(nodeRealTime, attrTransFileClientToRemote),
			              getTimeValue(nodeCPUTime, attrTransFileClientToRemote));
			// get time of transferFileRemoteToClient
			this.transferFileRemoteToClientTime =
			 new TimeInfo(getTimeValue(nodeRealTime, attrTransFileRemoteToClient),
			              getTimeValue(nodeCPUTime, attrTransFileRemoteToClient));
		} 

		// get time of transferResult
		this.transferResultTime = 
		 new TimeInfo(getTimeValue(nodeRealTime, attrTransResult),
		              getTimeValue(nodeCPUTime, attrTransResult));


		// get time of calculation
		this.calculationTime =
		 new TimeInfo(getTimeValue(nodeRealTime, attrCalculation),
		              getTimeValue(nodeCPUTime, attrCalculation));
		              //getTimeValue(nodeCPUTime, attrTransResult));


		// get time of callBackTransferResult
		this.callbackTransferArgumentTime =
		 new TimeInfo(getTimeValue(nodeRealTime, attrCBTransArg),
		              getTimeValue(nodeCPUTime, attrCBTransArg));

		// get time of callbackCalculation 
		this.callbackCalculationTime =
		 new TimeInfo(getTimeValue(nodeRealTime, attrCBCalculation),
		              getTimeValue(nodeCPUTime, attrCBCalculation));

		// get time of callbackTransferResult
		this.callbackTransferResultTime =
		 new TimeInfo(getTimeValue(nodeRealTime, attrCBTransResult),
		              getTimeValue(nodeCPUTime, attrCBTransResult));

		// get information about compression 
		if (version.getMinor() > 1) {
			// callback N times 
			Node nodeCBInfo = XMLUtil.getChildNode(node, namespaceURI, tagCallbackInfo);
			this.callbackNTimeCalled =
				Integer.parseInt((String)(XMLUtil.getAttributeValue(nodeCBInfo, attrCallbackNTimesCalled)));

			// compress information for results 
			int nCompressionInfo =
				XMLUtil.countChildNode(node, namespaceURI, CompressInformation.tagName);
			// list of CompressInformation 
			List listCompressionInfo = new Vector();
			// size of CompressInformation
			int maxArgID = 0;

			// get CompressInformation from sessionInformation 
			for (int i = 0; i < nCompressionInfo; i++) {
				CompressInformation compressInfo =
				 new CompressInformation(
					XMLUtil.getChildNode(node, namespaceURI, CompressInformation.tagName, i),
					version);
				listCompressionInfo.add(compressInfo);
				
				// update maxArgID
				if (maxArgID <= compressInfo.getArgID()) {
					maxArgID = compressInfo.getArgID();
				}
			}
			
			// set CompressInformation into this.serverCompressInfo 
			this.compressInfo = new CompressInformation[maxArgID];

			for (int i = 0; i < listCompressionInfo.size(); i++) {
				CompressInformation compressInfo =
					(CompressInformation)listCompressionInfo.get(i);
				this.compressInfo[compressInfo.getArgID() - 1] = compressInfo;
			}
			
			/* compress information for callback
			 * not implement on 2.4.0
			 */
		}
	}

	//-----  Constructors end -------------------------------


	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private double getTimeValue(Node node, String target)
	 throws GrpcException {
		String targetValue = XMLUtil.getAttributeValue(node, target);
		return makeTimeValue(targetValue);
	}
	
	/**
	 * [memo] timeStr format expect "#s #us" (# is number)
	 * @param timeStr
	 * @return
	 */
	private double makeTimeValue(String timeStr) {
		int separatorIndex = timeStr.indexOf(separator);
		String secStr =
			timeStr.substring(0, separatorIndex - secString.length());
		String usecStr = timeStr.substring(
			separatorIndex + 1, timeStr.length() - usecString.length());

		return Double.parseDouble(secStr + (Double.parseDouble(usecStr) / 1000000));
	}

	private String makeTimeValStr(double time) {
		int second = (int) time;
		int usecond = (int) ((time - second) * 1000);
		return second + "s " + usecond + "us";
	}
	private String makeRealTimeStr(TimeInfo time) {
		return makeTimeValStr(time.getReal());
	}
	private String makeCpuTimeStr(TimeInfo time) {
		return makeTimeValStr(time.getCpu());
	}

	// ----- set methods ------------------------------------------
	protected void setTransferArgumentRealTime(double time) {
		transferArgumentTime.setReal(time);
	}
	protected void setTransferArgumentCPUTime(double time) {
		transferArgumentTime.setCpu(time);
	}
	
	protected void setTransferFileClientToRemoteRealTime(double time) {
		this.transferFileClientToRemoteTime.setReal(time);
	}
	protected void setTransferFileClientToRemoteCPUTime(double time) {
		transferFileClientToRemoteTime.setCpu(time);
	}
	
	protected void setTransferResultRealTime(double time) {
		transferResultTime.setReal(time);
	}
	protected void setTransferResultCPUTime(double time) {
		transferResultTime.setCpu(time);
	}

	protected void setTransferFileRemoteToClientRealTime(double time) {
		transferFileRemoteToClientTime.setReal(time);
	}
	protected void setTransferFileRemoteToClientCPUTime(double time) {
		transferFileRemoteToClientTime.setCpu(time);
	}
	

	protected void setCalculationRealTime(double time) {
		calculationTime.setReal(time);
	}
	protected void setCalculationCPUTime(double time) {
		calculationTime.setCpu(time);
	}

	
	protected void setCallbackTransferArgumentRealTime(double time) {
		callbackTransferArgumentTime.setReal(callbackTransferArgumentTime.getReal() + time);
	}
	protected void setCallbackTransferArgumentCPUTime(double time) {
		callbackTransferArgumentTime.setCpu(callbackTransferArgumentTime.getCpu() + time);
	}
	
	protected void setCallbackCalculationRealTime(double time) {
		callbackCalculationTime.setReal(callbackCalculationTime.getReal() + time);
	}
	protected void setCallbackCalculationCPUTime(double time) {
		callbackCalculationTime.setReal(callbackCalculationTime.getCpu() + time);
	}
	
	protected void setCallbackTransferResultRealTime(double time) {
		callbackTransferResultTime.setReal(callbackTransferResultTime.getReal() + time);
	}
	protected void setCallbackTransferResultCPUTime(double time) {
		callbackTransferResultTime.setCpu(callbackTransferResultTime.getCpu() + time);
	}

	
	// ----- set methods end ---------------------------------------------

	// ----- get methods begin -------------------------------------------
	protected double getTransferArgumentRealTime() {
		return transferArgumentTime.getReal();
	}
	
	protected double getTransferArgumentCPUTime() {
		return transferArgumentTime.getCpu();
	}
	
	protected double getTransferFileClientToRemoteRealTime() {
		return transferFileClientToRemoteTime.getReal();
	}
	
	protected double getTransferFileClientToRemoteCPUTime() {
		return transferFileClientToRemoteTime.getCpu();
	}
	
	protected double getCalculationRealTime() {
		return calculationTime.getReal();
	}
	
	protected double getCalculationCPUTime() {
		return calculationTime.getCpu();
	}
	
	protected double getTransferResultRealTime() {
		return transferResultTime.getReal();
	}
	
	protected double getTransferResultCPUTime() {
		return transferResultTime.getCpu();
	}
	
	protected double getTransferFileRemoteToClientRealTime() {
		return transferFileRemoteToClientTime.getReal();
	}
	
	protected double getTransferFileRemoteToClientCPUTime() {
		return transferFileRemoteToClientTime.getCpu();
	}
	
	protected double getCallbackTransferArgumentRealTime() {
		return callbackTransferArgumentTime.getReal();
	}
	
	protected double getCallbackTransferArgumentCPUTime() {
		return callbackTransferArgumentTime.getCpu();
	}
	
	protected double getCallbackCalculationRealTime() {
		return callbackCalculationTime.getReal();
	}
	
	protected double getCallbackCalculationCPUTime() {
		return callbackCalculationTime.getCpu();
	}
	
	protected double getCallbackTransferResultRealTime() {
		return callbackTransferResultTime.getReal();
	}
	
	protected double getCallbackTransferResultCPUTime() {
		return callbackTransferResultTime.getCpu();
	}

	// ---- get methdos end ----------------------------------------
	

	/**
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param compressionRealTime
	 * @param compressionCPUTime
	 * @throws GrpcException
	 */
	protected void setCompressionInformation(
	 long[] originalNBytes,
	 long[] compressionNBytes,
	 double[] compressionRealTime,
	 double[] compressionCPUTime)
	 throws GrpcException {
		// check if the input arrays are valid 
		if (! isValidInputArray(originalNBytes, compressionNBytes,
		  compressionRealTime, compressionCPUTime) ) {
			return; // Invalid data, do nothing
		}
		
		// check if the CompressInformation is valid 
		if (this.compressInfo == null) {
			throw new NgException(
				"Invalid CompressInformation in SessionInformation.");
		}
		
		// set information about deflation
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] == -1)
				continue;
			this.compressInfo[i] =
				new CompressInformation(i + 1);
			setDeflateInformation(this.compressInfo[i],
				originalNBytes[i], compressionNBytes[i],
				compressionRealTime[i], compressionCPUTime[i]);
		}
	}
	
	/**
	 * @param callbackID
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param compressionRealTime
	 * @param compressionCPUTime
	 */
	protected void setCallbackCompressionInformation(int callbackID,
	 long[] originalNBytes, 
	 long[] compressionNBytes,
	 double[] compressionRealTime, 
	 double[] compressionCPUTime) {
		// check if the input arrays are valid
		if (! isValidInputArray(originalNBytes, compressionNBytes,
				compressionRealTime, compressionCPUTime) ) {
			return; // Invalid data, do nothing
		}
		
		// count the count of valid data and initialize
		if (callbackCompressInformation == null) {
			callbackCompressInformation =
				new CallbackCompressInformation[nCallbacks];
		}
		
		// set information about deflation
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] == -1) { continue; }

			// get target CompressInformation 
			CompressInformation compressInfo = 
				callbackCompressInformation[callbackID - 1].getCompressInformation(i);

			// set CompressInformation
			if (compressInfo == null) {
				compressInfo = new CompressInformation(i + 1);
			}
			setDeflateInformation(compressInfo,
				originalNBytes[i], compressionNBytes[i],
				compressionRealTime[i], compressionCPUTime[i]);

			// set target CompressInformation
			callbackCompressInformation[callbackID - 1].setCompressInformation(callbackID, compressInfo);
		}
	}
	
	/**
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param deCompressionRealTime
	 * @param deCompressionCPUTime
	 * @throws GrpcException
	 */
	protected void setDecompressionInformation(
	 long[] originalNBytes, 
	 long[] compressionNBytes,
	 double[] deCompressionRealTime, 
	 double[] deCompressionCPUTime)
	 throws GrpcException {
		// check if the input arrays are valid
		if (! isValidInputArray(originalNBytes, compressionNBytes, 
				deCompressionRealTime, deCompressionCPUTime) ) {
			// Invalid data, do nothing
			return;
		}
		
		// check if the CompressInformation is valid
		if (compressInfo == null) {
			throw new NgException("Invalid CompressInformation in SessionInformation.");
		}

		// set information about inflation
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] == -1) { continue; }
			compressInfo[i] = new CompressInformation(i + 1);
			setInflateInformation(compressInfo[i],
				originalNBytes[i], compressionNBytes[i],
				deCompressionRealTime[i], deCompressionCPUTime[i]);
		}
	}
	
	/**
	 * @param callbackID
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param deCompressionRealTime
	 */
	protected void setCallbackDecompressionInformation(int callbackID,
	 long[] originalNBytes,
	 long[] compressionNBytes,
	 double[] deCompressionRealTime,
	 double[] deCompressionCPUTime) {
		// check if the input arrays are valid */ 
		if (! isValidInputArray(originalNBytes, compressionNBytes,
				deCompressionRealTime, deCompressionCPUTime) ) {
			return; // Invalid data, do nothing
		}
				
		// count the count of valid data and initialize
		if (callbackCompressInformation == null) {
			callbackCompressInformation =
				new CallbackCompressInformation[nCallbacks];
		}
		
		// set information about inflation
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] == -1) { continue; }

			// get target CompressInformation 
			CompressInformation compressInfo = 
				callbackCompressInformation[callbackID - 1].getCompressInformation(i);

			// set CompressInformation
			if (compressInfo == null) {
				compressInfo = new CompressInformation(i + 1);
			}
			setInflateInformation(compressInfo,
				originalNBytes[i], compressionNBytes[i],
				deCompressionRealTime[i], deCompressionCPUTime[i]);

			// set target CompressInformation 
			callbackCompressInformation[callbackID - 1].setCompressInformation(
				callbackID, compressInfo);
		}
	}
	
	/**
	 * @param beforeLength
	 * @param afterLength
	 * @param convertRealTime
	 * @return
	 */
	private boolean isValidInputArray(long[] beforeLength, long[] afterLength,
	 double[] convertRealTime, double[] convertCPUTime) {
		if ((beforeLength.length != afterLength.length) 
			|| (beforeLength.length != convertRealTime.length)
			|| (beforeLength.length != convertCPUTime.length)) {
			return false; // unexpected data 
		}
		return true;
	}
	
	/**
	 * @param compressInfo
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param realTime
	 * @param CPUTime
	 */
	private void setDeflateInformation(CompressInformation compressInfo,
	 long originalNBytes, long compressionNBytes,
	 double realTime, double CPUTime) {
		compressInfo.addBeforeCompressionLength(originalNBytes);
		compressInfo.addAfterCompressionLength(compressionNBytes);
		compressInfo.setCompressionRealTime(realTime);
		compressInfo.setCompressionCPUTime(CPUTime);
	}
	
	/**
	 * @param compressInfo
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param deCompressionRealTime
	 * @param deCompressionCPUTime
	 */
	private void setInflateInformation(CompressInformation compressInfo,
	 long originalNBytes, long compressionNBytes,
	 double realTime, double CPUTime) {
		compressInfo.addBeforeDecompressionLength(originalNBytes);
		compressInfo.addAfterDecompressionLength(compressionNBytes);
		compressInfo.setDeCompressionRealTime(realTime);
		compressInfo.setDeCompressionCPUTime(CPUTime);
	}
	
	/**
	 * @param callbackNTimesCalled
	 */
	protected void setCallbackNTimesCalled(int count) {
		this.callbackNTimeCalled = count;
	}
	
	protected void incrementCallbackNTimesCalled() {
		this.callbackNTimeCalled += 1;
	}
	
	protected int getCallbackNTimesCalled() {
		return this.callbackNTimeCalled;
	}


	public String toXMLString() {
		StringBuffer sb = new StringBuffer();
		sb.append("<" + tagName + ">\n");

		// Real Time 
		sb.append("<" + tagRealTime + " " + attrTransArg + "=\""
			+ makeRealTimeStr(transferArgumentTime) + "\" ");
		if (protocol_ver.getMinor() > 0) {
			sb.append(attrTransFileClientToRemote + "=\""
				+ makeRealTimeStr(transferFileClientToRemoteTime) + "\" ");
		}
		sb.append(attrCalculation + "=\""
			+ makeRealTimeStr(calculationTime) + "\" "
			+ attrTransResult + "=\""
			+ makeRealTimeStr(transferResultTime) + "\" ");

		if (protocol_ver.getMinor() > 0) {
			sb.append(attrTransFileRemoteToClient + "=\""
				+ makeRealTimeStr(transferFileRemoteToClientTime) + "\" ");
		}
		sb.append(attrCBTransArg + "=\""
			+ makeRealTimeStr(callbackTransferArgumentTime) + "\" "
			+ attrCBCalculation + "=\""
			+ makeRealTimeStr(callbackCalculationTime) + "\" " 
			+ attrCBTransResult + "=\""
			+ makeRealTimeStr(callbackTransferResultTime) + "\""
			+ "/>\n");

		// CPU Time
		sb.append("<" + tagCPUTime + " "
			+ attrTransArg + "=\"" 
			+ makeCpuTimeStr(transferArgumentTime) + "\" ");

		if (protocol_ver.getMinor() > 0) {
			sb.append(attrTransFileClientToRemote + "=\"" 
				+ makeCpuTimeStr(transferFileClientToRemoteTime) + "\" ");
		}

		sb.append(attrCalculation + "=\"" 
			+ makeCpuTimeStr(calculationTime) + "\" " 
			+ attrTransResult + "=\"" 
			+ makeCpuTimeStr(transferResultTime) + "\" ");
			
		if (protocol_ver.getMinor() > 0) {
			sb.append(attrTransFileRemoteToClient + "=\"" 
				+ makeCpuTimeStr(transferFileRemoteToClientTime) + "\" ");
		}

		sb.append(attrCBTransArg + "=\"" 
			+ makeCpuTimeStr(callbackTransferArgumentTime) + "\" " 
			+ attrCBCalculation + "=\"" 
			+ makeCpuTimeStr(callbackCalculationTime) + "\" " 
			+ attrCBTransResult + "=\"" 
			+ makeCpuTimeStr(callbackTransferResultTime) + "\"" 
			+ "/>\n");

		if (protocol_ver.getMinor() > 1) {
			// put count of callback
			sb.append("<" + tagCallbackInfo + " " 
					+ attrCallbackNTimesCalled + "=\"" 
					+ callbackNTimeCalled 
					+ "\"/>\n");
			
			// put CompressInformation
			if (compressInfo != null) {
				for (int i = 0; i < compressInfo.length; i++) {
					if (compressInfo[i] != null) {
						sb.append(compressInfo[i].toXMLString());
					}
				}
			}
			
			// put CompressInformation for Callback
			if (callbackCompressInformation != null) {
				for (int i = 0; i < callbackCompressInformation.length; i++) {
					if (callbackCompressInformation[i] != null) {
						sb.append(callbackCompressInformation[i].toXMLString());
					}
				}
			}
		}
		sb.append("</" + tagName + ">\n");

		return sb.toString();
	}


	/* ----- interface for getting CompressInformation ----- */
	/**
	 * Gets data length of before compression 
	 *
	 * @param argID
	 * @return
	 */
	public long getOriginalNbytes(int argID) {
		return compressInfo[argID - 1].getBeforeCompressionLength();
	}
	
	/**
	 * Gets data length of after compression
	 *
	 * @param argID
	 * @return
	 */
	public long getCompressionNbytes(int argID) {
		return compressInfo[argID - 1].afterCompressionLength();
	}
	
	/**
	 * Gets time to compress data(Real)
	 *
	 * @param argID
	 * @return
	 */
	public double getCompressionRealTime(int argID) {
		return compressInfo[argID - 1].getCompressionRealTime();
	}
	
	/**
	 * Gets time to compress data(CPU)
	 *
	 * @param argID
	 * @return
	 */
	public double getCompressionCPUTime(int argID) {
		return compressInfo[argID - 1].getCompressionCPUTime();
	}
	
	/**
	 * Gets time to decompress data(Real)
	 *
	 * @param argID
	 * @return
	 */
	public double getDeCompressionRealTime(int argID) {
		return compressInfo[argID - 1].getDeCompressionRealTime();
	}
	
	/**
	 * Gets time to decompress data(CPU)
	 *
	 * @param argID
	 * @return
	 */
	public double getDeCompressionCPUTime(int argID) {
		return compressInfo[argID - 1].getDeCompressionCPUTime();
	}
	
	// ---------------------------------------------------------------
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		// put all of data into String
		StringBuffer sb = new StringBuffer();

		
		// Real Time
		sb.append("TransferArgument(REAL) : " 
			+ transferArgumentTime.getReal() + "\n");

		if (protocol_ver.getMinor() > 0) {
			sb.append("TransferFile1(REAL)    : " 
				+ transferFileClientToRemoteTime.getReal() + "\n");
		}

		sb.append("Calculation(REAL)      : " 
			+ calculationTime.getReal() + "\n");
		sb.append("TransferResult(REAL)   : " 
			+ transferResultTime.getReal() + "\n");

		if (protocol_ver.getMinor() > 0) {
			sb.append("TransferFile2(REAL)    : " 
				+ transferFileRemoteToClientTime.getReal() + "\n");
		}
		

		// CPU Time
		sb.append("TransferArgument(CPU)  : " 
			+ transferResultTime.getCpu() + "\n");
		if (protocol_ver.getMinor() > 0) {
			sb.append("TransferFile1(CPU)    : " 
				+ transferFileClientToRemoteTime.getCpu() + "\n");
		}
		sb.append("Calculation(CPU)       : " 
			+ calculationTime.getCpu() + "\n");
		sb.append("TransferResult(CPU)    : " 
			+ transferResultTime.getCpu() + "\n");
		if (protocol_ver.getMinor() > 0) {
			sb.append("TransferFile2(CPU)    : " 
				+ transferFileRemoteToClientTime.getCpu() + "\n");
		}

		
		if ( protocol_ver.getMinor() > 1) {
			// callback count
			sb.append("callback count         : " 
				+ callbackNTimeCalled + "\n");
			// compress Information
			if (compressInfo != null) {
				for (int i = 0; i < compressInfo.length; i++) {
					sb.append("----- compressInformation for arg[" + i + "] -----\n");
					if (compressInfo[i] != null) {
						sb.append(compressInfo[i]);
					}
				}
			}
			
			if (callbackCompressInformation != null) {
				for (int i = 0; i < callbackCompressInformation.length; i++) {
					sb.append("----- compressInformation for callback[" + i + "] -----\n");
					if (callbackCompressInformation[i] != null) {
						sb.append(callbackCompressInformation[i]);
					}
				}
			}
		}
		
		// return String
		return sb.toString();
	}
}
