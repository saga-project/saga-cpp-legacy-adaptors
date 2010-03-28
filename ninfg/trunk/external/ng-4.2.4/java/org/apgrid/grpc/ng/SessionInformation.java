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
 * $RCSfile: SessionInformation.java,v $ $Revision: 1.15 $ $Date: 2005/07/06 09:20:31 $
 */
package org.apgrid.grpc.ng;

import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class SessionInformation {
	/* difinitions */
	private static final String tagName = "sessionInformation";
	private static final String tagRealTime = "realTime";
	private static final String tagCPUTime = "CPUtime";
	private static final String tagCallbackInfo = "callbackInformation";

	private static final String attrTransArg = "transferArgument";
	private static final
		String attrTransFileClientToRemote = "transferFileClientToRemote";
	private static final String attrCalculation = "calculation";
	private static final String attrTransResult = "transferResult";
	private static final
		String attrTransFileRemoteToClient = "transferFileRemoteToClient";
	private static final String attrCBTransArg = "callbackTransferArgument";
	private static final String attrCBCalculation = "callbackCalculation";
	private static final String attrCBTransResult = "callbackTransferResult";
	
	private static final String attrCallbackNTimesCalled = "numberOfTimesWhichCalled";

	private static final String separator = " ";
	private static final String secString = "s";
	private static final String usecString = "us";
	
	/* protocol version */
	private int versionMajor;
	private int versionMinor;
	private int versionPatch;
	
	/* information for executing Executable */
	private double transferArgumentRealTime;
	private double transferArgumentCPUTime;
	private double transferFileClientToRemoteRealTime;
	private double transferFileClientToRemoteCPUTime;
	private double calculationRealTime;
	private double calculationCPUTime;
	private double transferResultRealTime;
	private double transferResultCPUTime;
	private double transferFileRemoteToClientRealTime;
	private double transferFileRemoteToClientCPUTime;

	/* information for callback */
	private double callbackTransferArgumentRealTime;
	private double callbackTransferArgumentCPUTime;
	private double callbackCalculationRealTime;
	private double callbackCalculationCPUTime;
	private double callbackTransferResultRealTime;
	private double callbackTransferResultCPUTime;
	
	/* information for callback */
	private int nCallbacks;
	private int callbackNTimeCalled;
	private CallbackCompressInformation[] callbackCompressInformation;
	
	/* information for compression */
	private int nArgs;
	private CompressInformation[] compressInfo;

	/**
	 * 
	 */
	public SessionInformation(int nArgs, int nCallback) {
		this.nArgs = nArgs;
		this.nCallbacks = nCallback;

		this.callbackNTimeCalled = 0;
		
		this.versionMajor = 2;
		this.versionMinor = 2;
		this.versionPatch = 0;
		
		/* init CompressInformation */
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
		double transferArgumentRealTime, double transferArgumentCPUTime,
		double clientRealTime, double clientCPUTime,
		double transferResultRealTime, double transferResultCPUTime,
		double callbackTransferArgumentRealTime, double callbackTransferArgumentCPUTime,
		double callbackCalculationRealTime, double callbackCalculationCPUTime,
		double callbackTransferResultRealTime, double callbackTransferResultCPUTime)
	{
		this.transferArgumentRealTime = transferArgumentRealTime;
		this.transferArgumentCPUTime = transferArgumentCPUTime;
		this.calculationRealTime = clientRealTime;
		this.calculationCPUTime = clientCPUTime;
		this.transferResultRealTime = transferResultRealTime;
		this.transferResultCPUTime = transferResultCPUTime;
		this.callbackTransferArgumentRealTime = callbackTransferArgumentRealTime;
		this.callbackTransferArgumentCPUTime = callbackTransferArgumentCPUTime;
		this.callbackCalculationRealTime = callbackCalculationRealTime;
		this.callbackCalculationCPUTime = callbackCalculationCPUTime;
		this.callbackTransferResultRealTime = callbackTransferResultRealTime;
		this.callbackTransferResultCPUTime = callbackTransferResultCPUTime;
		
		/* set protocol version */
		this.versionMajor = 2;
		this.versionMinor = 0;
		this.versionPatch = 0;
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
		double transferArgumentRealTime, double transferArgumentCPUTime,
		double transferFileClientToRemoteRealTime,
		double transferFileClientToRemoteCPUTime,
		double clientRealTime, double clientCPUTime,
		double transferResultRealTime, double transferResultCPUTime,
		double transferFileRemoteToClientRealTime,
		double transferFileRemoteToClientCPUTime,
		double callbackTransferArgumentRealTime, double callbackTransferArgumentCPUTime,
		double callbackCalculationRealTime, double callbackCalculationCPUTime,
		double callbackTransferResultRealTime, double callbackTransferResultCPUTime)
	{
		this.transferArgumentRealTime = transferArgumentRealTime;
		this.transferArgumentCPUTime = transferArgumentCPUTime;
		this.transferFileClientToRemoteRealTime = transferFileClientToRemoteRealTime;
		this.transferFileClientToRemoteCPUTime = transferFileClientToRemoteCPUTime;
		this.calculationRealTime = clientRealTime;
		this.calculationCPUTime = clientCPUTime;
		this.transferResultRealTime = transferResultRealTime;
		this.transferResultCPUTime = transferResultCPUTime;
		this.transferFileRemoteToClientRealTime = transferFileRemoteToClientRealTime;
		this.transferFileRemoteToClientCPUTime = transferFileRemoteToClientCPUTime;
		this.callbackTransferArgumentRealTime = callbackTransferArgumentRealTime;
		this.callbackTransferArgumentCPUTime = callbackTransferArgumentCPUTime;
		this.callbackCalculationRealTime = callbackCalculationRealTime;
		this.callbackCalculationCPUTime = callbackCalculationCPUTime;
		this.callbackTransferResultRealTime = callbackTransferResultRealTime;
		this.callbackTransferResultCPUTime = callbackTransferResultCPUTime;
		
		/* set protocol version */
		this.versionMajor = 2;
		this.versionMinor = 1;
		this.versionPatch = 0;
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
			double transferArgumentRealTime, double transferArgumentCPUTime,
			double transferFileClientToRemoteRealTime,
			double transferFileClientToRemoteCPUTime,
			double clientRealTime, double clientCPUTime,
			double transferResultRealTime, double transferResultCPUTime,
			double transferFileRemoteToClientRealTime,
			double transferFileRemoteToClientCPUTime,
			double callbackTransferArgumentRealTime, double callbackTransferArgumentCPUTime,
			double callbackCalculationRealTime, double callbackCalculationCPUTime,
			double callbackTransferResultRealTime, double callbackTransferResultCPUTime,
			CompressInformation[] argCompressInformation,
			CallbackCompressInformation[] callbackCompressInformation)
		{
			this.transferArgumentRealTime = transferArgumentRealTime;
			this.transferArgumentCPUTime = transferArgumentCPUTime;
			this.transferFileClientToRemoteRealTime = transferFileClientToRemoteRealTime;
			this.transferFileClientToRemoteCPUTime = transferFileClientToRemoteCPUTime;
			this.calculationRealTime = clientRealTime;
			this.calculationCPUTime = clientCPUTime;
			this.transferResultRealTime = transferResultRealTime;
			this.transferResultCPUTime = transferResultCPUTime;
			this.transferFileRemoteToClientRealTime = transferFileRemoteToClientRealTime;
			this.transferFileRemoteToClientCPUTime = transferFileRemoteToClientCPUTime;
			this.callbackTransferArgumentRealTime = callbackTransferArgumentRealTime;
			this.callbackTransferArgumentCPUTime = callbackTransferArgumentCPUTime;
			this.callbackCalculationRealTime = callbackCalculationRealTime;
			this.callbackCalculationCPUTime = callbackCalculationCPUTime;
			this.callbackTransferResultRealTime = callbackTransferResultRealTime;
			this.callbackTransferResultCPUTime = callbackTransferResultCPUTime;
			this.compressInfo = argCompressInformation;
			/* 
			 * not implement on 2.4.0
			this.callbackCompressInformation = callbackCompressInformation;
			*/
			
			/* set protocol version */
			this.versionMajor = 2;
			this.versionMinor = 2;
			this.versionPatch = 0;
		}

	/**
	 * @param xmlString
	 * @throws GrpcException
	 */
	public SessionInformation(String xmlString,
		int versionMajor, int versionMinor, int versionPatch) throws GrpcException {
		this(XMLUtil.getNode(xmlString),
		versionMajor, versionMinor, versionPatch);
	}

	/**
	 * @param node
	 * @throws GrpcException
	 */
	public SessionInformation(Node node,
		int versionMajor, int versionMinor, int versionPatch) throws GrpcException {
		/* set protocol version */
		this.versionMajor = versionMajor;
		this.versionMinor = versionMinor;
		this.versionPatch = versionPatch;

		/* get information about realTime */
		Node nodeRealTime = XMLUtil.getChildNode(node, tagRealTime);

		/* get time of transferArgument */
		this.transferArgumentRealTime =
			getTimeValue(nodeRealTime, attrTransArg);
		/* get time of transferFileClientToRemote */
		if (versionMinor > 0) {
			this.transferFileClientToRemoteRealTime =
				getTimeValue(nodeRealTime, attrTransFileClientToRemote);
		}
		/* get time of calculation */
		this.calculationRealTime =
			getTimeValue(nodeRealTime, attrCalculation);
		/* get time of transferResult */
		this.transferResultRealTime =
			getTimeValue(nodeRealTime, attrTransResult);
		/* get time of transferFileRemoteToClient */
		if (versionMinor > 0) {
			this.transferFileRemoteToClientRealTime =
				getTimeValue(nodeRealTime, attrTransFileRemoteToClient);
		}
		/* get time of callBackTransferResult */
		this.callbackTransferArgumentRealTime =
			getTimeValue(nodeRealTime, attrCBTransArg);
		/* get time of callbackCalculation */
		this.callbackCalculationRealTime =
			getTimeValue(nodeRealTime, attrCBCalculation);
		/* get time of callbackTransferResult */
		this.callbackTransferResultRealTime =
			getTimeValue(nodeRealTime, attrCBTransResult);

		/* get information about CPUtime */
		Node nodeCPUTime = XMLUtil.getChildNode(node, tagCPUTime);

		/* get time of transferArgument */
		this.transferArgumentCPUTime =
			getTimeValue(nodeCPUTime, attrTransResult);
		/* get time of transferFileClientToRemote */
		if (versionMinor > 0) {
			this.transferFileClientToRemoteCPUTime =
				getTimeValue(nodeCPUTime, attrTransFileClientToRemote);
		}
		/* get time of calculation */
		this.calculationCPUTime =
			getTimeValue(nodeCPUTime, attrTransResult);
		/* get time of transferResult */
		this.transferResultCPUTime =
			getTimeValue(nodeCPUTime, attrTransResult);
		/* get time of FileRemoteToClient */
		if (versionMinor > 0) {
			this.transferFileRemoteToClientCPUTime =
				getTimeValue(nodeCPUTime, attrTransFileRemoteToClient);
		}
		/* get time of callBackTransferResult */
		this.callbackTransferArgumentCPUTime =
			getTimeValue(nodeCPUTime, attrCBTransArg);
		/* get time of callbackCalculation */
		this.callbackCalculationCPUTime =
			getTimeValue(nodeCPUTime, attrCBCalculation);
		/* get time of callbackTransferResult */
		this.callbackTransferResultCPUTime =
			getTimeValue(nodeCPUTime, attrCBTransResult);
		
		/* get information about compression */
		if (versionMinor > 1) {
			/* callback N times */
			Node nodeCBInfo = XMLUtil.getChildNode(node, tagCallbackInfo);
			this.callbackNTimeCalled =
				Integer.parseInt((String)(XMLUtil.getAttributeValue(nodeCBInfo, attrCallbackNTimesCalled)));

			/* compress information for results */
			int nCompressionInfo =
				XMLUtil.countChildNode(node, CompressInformation.tagName);
			/* list of CompressInformation */
			List listCompressionInfo = new Vector();
			/* size of CompressInformation */
			int maxArgID = 0;
			
			/* get CompressInformation from sessionInformation */
			for (int i = 0; i < nCompressionInfo; i++) {
				CompressInformation compressInfo = new CompressInformation(
					XMLUtil.getChildNode(node, CompressInformation.tagName, i),
					versionMajor, versionMinor, versionPatch);
				listCompressionInfo.add(compressInfo);
				
				/* update maxArgID */
				if (maxArgID <= compressInfo.getArgID()) {
					maxArgID = compressInfo.getArgID();
				}
			}
			
			/* set CompressInformation into this.serverCompressInfo */
			this.compressInfo = new CompressInformation[maxArgID];
			for (int i = 0; i < listCompressionInfo.size(); i++) {
				CompressInformation compressInfo =
					(CompressInformation)listCompressionInfo.get(i);
				this.compressInfo[compressInfo.getArgID() - 1] = compressInfo;
			}
			
			/* compress information for callback */
			/* 
			 * not implement on 2.4.0
			int nCallbackCompressInfo =
				XMLUtil.countChildNode(node, CallbackCompressInformation.tagName);
			this.callbackCompressInformation =
				new CallbackCompressInformation[nCallbackCompressInfo];
			for (int i = 0; i < nCallbackCompressInfo; i++) {
				this.callbackCompressInformation[i] =
					new CallbackCompressInformation(XMLUtil.getChildNode(node, CallbackCompressInformation.tagName, i),
						versionMajor, versionMinor, versionPatch);
			}
			*/
		}
	}
	
	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private double getTimeValue(Node node, String target) throws GrpcException {
		String targetValue = XMLUtil.getAttributeValue(node, target);
		return makeTimeValue(targetValue);
	}
	
	/**
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

	/* ----- set methods ----- */
	/**
	 * @return
	 */
	protected void setTransferArgumentRealTime(double transferArgumentRealTime) {
		this.transferArgumentRealTime = transferArgumentRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setTransferArgumentCPUTime(double transferArgumentCPUTime) {
		this.transferArgumentCPUTime = transferArgumentCPUTime;
	}
	
	/**
	 * @return
	 */
	protected void setTransferFileClientToRemoteRealTime(double transferFileClientToRemoteRealTime) {
		this.transferFileClientToRemoteRealTime = transferFileClientToRemoteRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setTransferFileClientToRemoteCPUTime(double transferFileClientToRemoteCPUTime) {
		this.transferFileClientToRemoteCPUTime = transferFileClientToRemoteCPUTime;
	}
	
	/**
	 * @return
	 */
	protected void setCalculationRealTime(double calculationRealTime) {
		this.calculationRealTime = calculationRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setCalculationCPUTime(double calculationCPUTime) {
		this.calculationCPUTime = calculationCPUTime;
	}
	
	/**
	 * @return
	 */
	protected void setTransferResultRealTime(double transferResultRealTime) {
		this.transferResultRealTime = transferResultRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setTransferResultCPUTime(double transferResultCPUTime) {
		this.transferResultCPUTime = transferResultCPUTime;
	}
	
	/**
	 * @return
	 */
	protected void setTransferFileRemoteToClientRealTime(double transferFileRemoteToClientRealTime) {
		this.transferFileRemoteToClientRealTime = transferFileRemoteToClientRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setTransferFileRemoteToClientCPUTime(double transferFileRemoteToClientCPUTime) {
		this.transferFileRemoteToClientCPUTime = transferFileRemoteToClientCPUTime;
	}
	
	/**
	 * @return
	 */
	protected void setCallbackTransferArgumentRealTime(
		double callbackTransferArgumentRealTime) {
		this.callbackTransferArgumentRealTime += callbackTransferArgumentRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setCallbackTransferArgumentCPUTime(
		double callbackTransferArgumentCPUTime) {
		this.callbackTransferArgumentCPUTime += callbackTransferArgumentCPUTime;
	}
	
	/**
	 * @return
	 */
	protected void setCallbackCalculationRealTime(double callbackCalculationRealTime) {
		this.callbackCalculationRealTime += callbackCalculationRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setCallbackCalculationCPUTime(double callbackCalculationCPUTime) {
		this.callbackCalculationCPUTime += callbackCalculationCPUTime;
	}
	
	/**
	 * @return
	 */
	protected void setCallbackTransferResultRealTime(
		double callbackTransferResultRealTime) {
		this.callbackTransferResultRealTime += callbackTransferResultRealTime;
	}
	
	/**
	 * @return
	 */
	protected void setCallbackTransferResultCPUTime(
		double callbackTransferResultCPUTime) {
		this.callbackTransferResultCPUTime += callbackTransferResultCPUTime;
	}
	
	/* ----- get methods ----- */
	/**
	 * @return
	 */
	protected double getTransferArgumentRealTime() {
		return transferArgumentRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getTransferArgumentCPUTime() {
		return transferArgumentCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getTransferFileClientToRemoteRealTime() {
		return transferFileClientToRemoteRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getTransferFileClientToRemoteCPUTime() {
		return transferFileClientToRemoteCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getCalculationRealTime() {
		return calculationRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getCalculationCPUTime() {
		return calculationCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getTransferResultRealTime() {
		return transferResultRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getTransferResultCPUTime() {
		return transferResultCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getTransferFileRemoteToClientRealTime() {
		return transferFileRemoteToClientRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getTransferFileRemoteToClientCPUTime() {
		return transferFileRemoteToClientCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getCallbackTransferArgumentRealTime() {
		return callbackTransferArgumentRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getCallbackTransferArgumentCPUTime() {
		return callbackTransferArgumentCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getCallbackCalculationRealTime() {
		return callbackCalculationRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getCallbackCalculationCPUTime() {
		return callbackCalculationCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getCallbackTransferResultRealTime() {
		return callbackTransferResultRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getCallbackTransferResultCPUTime() {
		return callbackTransferResultCPUTime;
	}
	
	/**
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param compressionRealTime
	 * @param compressionCPUTime
	 * @throws GrpcException
	 */
	protected void setCompressionInformation(
		long[] originalNBytes, long[] compressionNBytes,
		double[] compressionRealTime, double[] compressionCPUTime)
		throws GrpcException {
		/* check if the input arrays are valid */ 
		if (isValidInputArray(originalNBytes, compressionNBytes,
				compressionRealTime, compressionCPUTime) != true) {
			/* Invalid data, do nothing */
			return;
		}
		
		/* check if the CompressInformation is valid */
		if (this.compressInfo == null) {
			throw new NgException("Invalid CompressInformation in SessionInformation.");
		}
		
		/* set information about deflation */
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] != -1) {
				this.compressInfo[i] = new CompressInformation(i + 1);
				setDeflateInformation(this.compressInfo[i],
					originalNBytes[i], compressionNBytes[i],
					compressionRealTime[i], compressionCPUTime[i]);
			}
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
		long[] originalNBytes, long[] compressionNBytes,
		double[] compressionRealTime, double[] compressionCPUTime) {
		/* check if the input arrays are valid */ 
		if (isValidInputArray(originalNBytes, compressionNBytes,
				compressionRealTime, compressionCPUTime) != true) {
			/* Invalid data, do nothing */
			return;
		}
		
		/* count the count of valid data and initialize */
		if (this.callbackCompressInformation == null) {
			this.callbackCompressInformation =
				new CallbackCompressInformation[nCallbacks];
		}
		
		/* set information about deflation */
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] != -1) {
				/* get target CompressInformation */
				CompressInformation compressInfo = 
					this.callbackCompressInformation[callbackID - 1].getCompressInformation(i);

				/* set CompressInformation */
				if (compressInfo == null) {
					compressInfo = new CompressInformation(i + 1);
				}
				setDeflateInformation(compressInfo,
					originalNBytes[i], compressionNBytes[i],
					compressionRealTime[i], compressionCPUTime[i]);

				/* set target CompressInformation */
				this.callbackCompressInformation[callbackID - 1].setCompressInformation(
					callbackID, compressInfo);
			}
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
		long[] originalNBytes, long[] compressionNBytes,
		double[] deCompressionRealTime, double[] deCompressionCPUTime)
		throws GrpcException {
		/* check if the input arrays are valid */ 
		if (isValidInputArray(originalNBytes, compressionNBytes, 
				deCompressionRealTime, deCompressionCPUTime) != true) {
			/* Invalid data, do nothing */
			return;
		}
		
		/* check if the CompressInformation is valid */
		if (this.compressInfo == null) {
			throw new NgException("Invalid CompressInformation in SessionInformation.");
		}

		/* set information about inflation */
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] != -1) {
				this.compressInfo[i] = new CompressInformation(i + 1);
				setInflateInformation(this.compressInfo[i],
					originalNBytes[i], compressionNBytes[i],
					deCompressionRealTime[i], deCompressionCPUTime[i]);
			}
		}
	}
	
	/**
	 * @param callbackID
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param deCompressionRealTime
	 */
	protected void setCallbackDecompressionInformation(int callbackID,
		long[] originalNBytes, long[] compressionNBytes,
		double[] deCompressionRealTime, double[] deCompressionCPUTime) {
		/* check if the input arrays are valid */ 
		if (isValidInputArray(originalNBytes, compressionNBytes,
				deCompressionRealTime, deCompressionCPUTime) != true) {
			/* Invalid data, do nothing */
			return;
		}
				
		/* count the count of valid data and initialize */
		if (this.callbackCompressInformation == null) {
			this.callbackCompressInformation =
				new CallbackCompressInformation[nCallbacks];
		}
		
		/* set information about inflation */
		for (int i = 0; i < originalNBytes.length; i++) {
			if (originalNBytes[i] != -1) {
				/* get target CompressInformation */
				CompressInformation compressInfo = 
					this.callbackCompressInformation[callbackID - 1].getCompressInformation(i);

				/* set CompressInformation */
				if (compressInfo == null) {
					compressInfo = new CompressInformation(i + 1);
				}
				setInflateInformation(compressInfo,
					originalNBytes[i], compressionNBytes[i],
					deCompressionRealTime[i], deCompressionCPUTime[i]);

				/* set target CompressInformation */
				this.callbackCompressInformation[callbackID - 1].setCompressInformation(
					callbackID, compressInfo);
			}
		}
	}
	
	/**
	 * @param beforeLength
	 * @param afterLength
	 * @param convertRealTime
	 * @return
	 */
	private boolean isValidInputArray(
		long[] beforeLength, long[] afterLength,
		double[] convertRealTime, double[] convertCPUTime) {
		if ((beforeLength.length != afterLength.length) ||
			(beforeLength.length != convertRealTime.length) ||
			(beforeLength.length != convertCPUTime.length)) {
			/* unexpected data */
			return false;
		}
		 return true;
	}
	
	/**
	 * @param compressInfo
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param compressionRealTime
	 * @param compressionCPUTime
	 */
	private void setDeflateInformation(CompressInformation compressInfo,
		long originalNBytes, long compressionNBytes,
		double compressionRealTime, double compressionCPUTime) {
		compressInfo.setBeforeCompressionLength(originalNBytes);
		compressInfo.setAfterCompressionLength(compressionNBytes);
		compressInfo.setCompressionRealTime(compressionRealTime);
		compressInfo.setCompressionCPUTime(compressionCPUTime);
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
			double deCompressionRealTime, double deCompressionCPUTime) {
		compressInfo.setBeforeDecompressionLength(originalNBytes);
		compressInfo.setAfterDecompressionLength(compressionNBytes);
		compressInfo.setDeCompressionRealTime(deCompressionRealTime);
		compressInfo.setDeCompressionCPUTime(deCompressionCPUTime);
	}
	
	/**
	 * @param callbackNTimesCalled
	 */
	protected void setCallbackNTimesCalled(int callbackNTimesCalled) {
		this.callbackNTimeCalled = callbackNTimesCalled;
	}
	
	/**
	 */
	protected void incrementCallbackNTimesCalled() {
		this.callbackNTimeCalled += 1;
	}
	
	/**
	 * @return
	 */
	protected int getCallbackNTimesCalled() {
		return this.callbackNTimeCalled;
	}
	
	/**
	 * @return
	 */
	public String toXMLString() {
		StringBuffer sb = new StringBuffer();
		sb.append("<" + tagName + ">\n");

		/* Real Time */
		sb.append("<" + tagRealTime + " " +
			attrTransArg + "=\"" +
			makeTimeValStr(transferArgumentRealTime) + "\"" + " ");
		if (versionMinor > 0) {
			sb.append(attrTransFileClientToRemote + "=\"" +
				makeTimeValStr(transferFileClientToRemoteRealTime) +
				"\"" + " ");
		}
		sb.append(attrCalculation + "=\"" +
			makeTimeValStr(calculationRealTime) + "\"" + " " +
			attrTransResult + "=\"" +
			makeTimeValStr(transferResultRealTime) + "\"" + " ");
			
		if (versionMinor > 0) {
			sb.append(attrTransFileRemoteToClient + "=\"" +
				makeTimeValStr(transferFileRemoteToClientRealTime) +
				"\"" + " ");
		}
		sb.append(attrCBTransArg + "=\"" +
			makeTimeValStr(callbackTransferArgumentRealTime) + "\"" + " " +
			attrCBCalculation + "=\"" +
			makeTimeValStr(callbackCalculationRealTime) + "\"" + " " +
			attrCBTransResult + "=\"" +
			makeTimeValStr(callbackTransferResultRealTime) + "\"" +
			"/>\n");

		/* CPU Time */
		sb.append("<" + tagCPUTime + " " +
			attrTransArg + "=\"" +
			makeTimeValStr(transferArgumentCPUTime) + "\"" + " ");
		if (versionMinor > 0) {
			sb.append(attrTransFileClientToRemote + "=\"" +
				makeTimeValStr(transferFileClientToRemoteCPUTime) +
				"\"" + " ");
		}
		sb.append(attrCalculation + "=\"" +
			makeTimeValStr(calculationCPUTime) + "\"" + " " +
			attrTransResult + "=\"" +
			makeTimeValStr(transferResultCPUTime) + "\"" + " ");
			
		if (versionMinor > 0) {
			sb.append(attrTransFileRemoteToClient + "=\"" +
				makeTimeValStr(transferFileRemoteToClientCPUTime) +
				"\"" + " ");
		}
		sb.append(attrCBTransArg + "=\"" +
			makeTimeValStr(callbackTransferArgumentCPUTime) + "\"" + " " +
			attrCBCalculation + "=\"" +
			makeTimeValStr(callbackCalculationCPUTime) + "\"" + " " +
			attrCBTransResult + "=\"" +
			makeTimeValStr(callbackTransferResultCPUTime) + "\"" +
			"/>\n");

		if (versionMinor > 1) {
			/* put count of callback */
			sb.append("<" + tagCallbackInfo + " " +
					attrCallbackNTimesCalled + "=\"" + callbackNTimeCalled + "\"" + "/>\n");
			
			/* put CompressInformation */
			if (compressInfo != null) {
				for (int i = 0; i < compressInfo.length; i++) {
					if (compressInfo[i] != null) {
						sb.append(compressInfo[i].toXMLString());
					}
				}
			}
			
			/* put CompressInformation for Callback */
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

	/**
	 * @param time
	 * @return
	 */
	private String makeTimeValStr(double time) {
		int second = (int) time;
		int usecond = (int) ((time - second) * 1000);
		return second + secString + " " + usecond + usecString;
	}

	/* ----- interface for getting CompressInformation ----- */
	/**
	 * Gets data length of before compression 
	 *
	 * @param argID
	 * @return
	 */
	protected long getOriginalNbytes(int argID) {
		return compressInfo[argID - 1].getBeforeCompressionLength();
	}
	
	/**
	 * Gets data length of after compression
	 *
	 * @param argID
	 * @return
	 */
	protected long getCompressionNbytes(int argID) {
		return compressInfo[argID - 1].afterCompressionLength();
	}
	
	/**
	 * Gets time to compress data(Real)
	 *
	 * @param argID
	 * @return
	 */
	protected double getCompressionRealTime(int argID) {
		return compressInfo[argID - 1].getCompressionRealTime();
	}
	
	/**
	 * Gets time to compress data(CPU)
	 *
	 * @param argID
	 * @return
	 */
	protected double getCompressionCPUTime(int argID) {
		return compressInfo[argID - 1].getCompressionCPUTime();
	}
	
	/**
	 * Gets time to decompress data(Real)
	 *
	 * @param argID
	 * @return
	 */
	protected double getDeCompressionRealTime(int argID) {
		return compressInfo[argID - 1].getDeCompressionRealTime();
	}
	
	/**
	 * Gets time to decompress data(CPU)
	 *
	 * @param argID
	 * @return
	 */
	protected double getDeCompressionCPUTime(int argID) {
		return compressInfo[argID - 1].getDeCompressionCPUTime();
	}
	
	/* --------------------------------------------------------------- */
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		/* put all of data into String */
		StringBuffer sb = new StringBuffer();
		
		/* Real Time */
		sb.append("TransferArgument(REAL) : " + transferArgumentRealTime + "\n");
		if (versionMinor > 0) {
			sb.append("TransferFile1(REAL)    : " + transferFileClientToRemoteRealTime + "\n");
		}
		sb.append("Calculation(REAL)      : " + calculationRealTime + "\n");
		sb.append("TransferResult(REAL)   : " + transferResultRealTime + "\n");
		if (versionMinor > 0) {
			sb.append("TransferFile2(REAL)    : " + transferFileRemoteToClientRealTime + "\n");
		}
		
		/* CPU Time */
		sb.append("TransferArgument(CPU)  : " + transferResultCPUTime + "\n");
		if (versionMinor > 0) {
			sb.append("TransferFile1(CPU)    : " + transferFileClientToRemoteCPUTime + "\n");
		}
		sb.append("Calculation(CPU)       : " + calculationCPUTime + "\n");
		sb.append("TransferResult(CPU)    : " + transferResultCPUTime + "\n");
		if (versionMinor > 0) {
			sb.append("TransferFile2(CPU)    : " + transferFileRemoteToClientCPUTime + "\n");
		}
		
		if (versionMinor > 1) {
			/* callback count */
			sb.append("callback count         : " + callbackNTimeCalled + "\n");

			/* compress Information */
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
		
		/* return String */
		return sb.toString();
	}
}
