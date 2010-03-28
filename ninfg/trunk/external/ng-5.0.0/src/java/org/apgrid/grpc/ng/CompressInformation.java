/*
 * $RCSfile: CompressInformation.java,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:07 $
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

import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class CompressInformation {
	// tag of this node 
	protected static final String tagName = "compressionInformation";
	// attribute for argument ID 
	private static final String attrArgumentNo = "argumentNo";
	
	// attributes for compression 
	private static final String attrBeforeCompressionLength = "beforeCompressionLength";
	private static final String attrAfterCompressionLength = "afterCompressionLength";
	private static final String attrBeforeDecompressionLength = "beforeDecompressionLength";
	private static final String attrAfterDecompressionLength = "afterDecompressionLength";
	private static final String attrCompressionRealTime = "compressionRealTime";
	private static final String attrCompressionCPUTime = "compressionCPUtime";
	private static final String attrDeCompressionRealTime = "decompressionRealTime";
	private static final String attrDeCompressionCPUTime = "decompressionCPUtime";

	// strings for time
	private static final String separator = " ";
	private static final String secString = "s";
	private static final String usecString = "us";
	
	// protocol version 
	private Version protocol_ver;
	
	// information for executing Executable
	private int argID;
	private long beforeCompressionLength;
	private long afterCompressionLength;
	private long beforeDecompressionLength;
	private long afterDecompressionLength;

	private TimeInfo compressionTime;
	private TimeInfo deCompressionTime;

	/**
	 * @param argID
	 */
	public CompressInformation (int argID) {
		// set received variables into instance
		this.argID = argID;
		this.protocol_ver = new Version(2, 2, 0);
		this.compressionTime = new TimeInfo(0, 0);
		this.deCompressionTime = new TimeInfo(0, 0);
	}

	/**
	 * @param node
	 * @throws GrpcException
	 */
	public CompressInformation(Node node, Version version)
	 throws GrpcException {
		// set protocol version 
		this.protocol_ver = version;

		// get information about deflate 
		this.argID = getIDValue(node, attrArgumentNo);

		// get information about compression
		this.beforeCompressionLength =
			getLengthValue(node, attrBeforeCompressionLength);
		this.afterCompressionLength =
			getLengthValue(node, attrAfterCompressionLength);

		this.compressionTime =
		 new TimeInfo(getTimeValue(node, attrCompressionRealTime),
		              getTimeValue(node, attrCompressionCPUTime));

		this.beforeDecompressionLength =
			getLengthValue(node, attrBeforeDecompressionLength);
		this.afterDecompressionLength =
			getLengthValue(node, attrAfterDecompressionLength);

		this.deCompressionTime =
		 new TimeInfo(getTimeValue(node, attrDeCompressionRealTime),
		              getTimeValue(node, attrDeCompressionCPUTime));
	}
	
	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private int getIDValue(Node node, String target)
	 throws GrpcException {
		return Integer.parseInt(XMLUtil.getAttributeValue(node, target));
	}
	
	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private long getLengthValue(Node node, String target)
	 throws GrpcException {
		String targetValue = XMLUtil.getAttributeValue(node, target);
		if (targetValue.equals("")) {
			return 0;
		} else {
			return Long.parseLong(targetValue);
		}
	}
	
	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private double getTimeValue(Node node, String target)
	 throws GrpcException {
		String targetValue = XMLUtil.getAttributeValue(node, target);
		if (targetValue.equals("")) {
			return 0;
		} else {
			return makeTimeValue(targetValue);
		}
	}
	
	/**
	 * @param timeStr
	 * @return
	 */
	private double makeTimeValue(String timeStr) {
		int separatorIndex = timeStr.indexOf(separator);
		String secStr =
			timeStr.substring(0, separatorIndex - secString.length());
		String usecStr = 
			timeStr.substring(separatorIndex + 1, timeStr.length() - usecString.length());
		
		return Double.parseDouble(secStr + (Double.parseDouble(usecStr) / 1000000));
	}

	public int getArgID() {
		return this.argID;
	}
	
	public long getBeforeCompressionLength() {
		return beforeCompressionLength;
	}
	
	public long afterCompressionLength() {
		return afterCompressionLength;
	}
	
	public long getBeforeDecompressionLength() {
		return beforeDecompressionLength;
	}
	
	public long afterDecompressionLength() {
		return afterDecompressionLength;
	} 

	public double getCompressionRealTime() {
		return compressionTime.getReal();
	}
	public double getCompressionCPUTime() {
		return compressionTime.getCpu();
	}
	
	public double getDeCompressionRealTime() {
		return deCompressionTime.getReal();
	}
	public double getDeCompressionCPUTime() {
		return deCompressionTime.getCpu();
	}
	

	public void addBeforeCompressionLength(long originalNbytes) {
		beforeCompressionLength += originalNbytes;
	}
	
	public void addAfterCompressionLength(long compressionNbytes) {
		afterCompressionLength += compressionNbytes;
	}
	
	public void addBeforeDecompressionLength(long originalNbytes) {
		this.beforeDecompressionLength += originalNbytes;
	}
	
	public void addAfterDecompressionLength(long compressionNbytes) {
		this.afterDecompressionLength += compressionNbytes;
	}

	public void setCompressionRealTime(double time) {
		compressionTime.setReal(
			compressionTime.getReal() + time);
	} 
	public void setCompressionCPUTime(double time) {
		compressionTime.setCpu(compressionTime.getCpu() + time);
	}
	
	public void setDeCompressionRealTime(double time) {
		deCompressionTime.setReal(deCompressionTime.getReal() + time);
	} 
	public void setDeCompressionCPUTime(double time) {
		deCompressionTime.setCpu(deCompressionTime.getCpu() + time);
	}

	public String toXMLString() {
		StringBuffer sb = new StringBuffer();
		sb.append("<" + tagName + " ");
		sb.append(attrArgumentNo + "=\"" + this.argID + "\" ");

		// compress information 
		sb.append(attrBeforeCompressionLength + "=\"" 
			+ this.beforeCompressionLength + "\" ");
		sb.append(attrAfterCompressionLength + "=\"" 
			+ this.afterCompressionLength + "\" ");
		sb.append(attrCompressionRealTime + "=\"" 
			+ makeTimeValStr(compressionTime.getReal()) + "\" ");

		sb.append(attrCompressionCPUTime + "=\"" 
			+ makeTimeValStr(this.compressionTime.getCpu()) + "\" ");
		sb.append(attrBeforeDecompressionLength + "=\"" 
			+ this.beforeDecompressionLength + "\" ");
		sb.append(attrAfterDecompressionLength + "=\"" 
			+ this.afterDecompressionLength + "\" ");
		sb.append(attrDeCompressionRealTime + "=\"" 
			+ makeTimeValStr(this.deCompressionTime.getReal()) + "\" ");
		sb.append(attrDeCompressionCPUTime + "=\"" 
			+ makeTimeValStr(this.deCompressionTime.getCpu()) + "\" ");

		sb.append("/>\n");

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


	public String toString() {
		if (beforeCompressionLength == -1) { return ""; }

		// put all of data into String 
		StringBuffer sb = new StringBuffer();

		// print Compression Information 
		sb.append("Before compression length   : ")
			.append(beforeCompressionLength).append("\n");
		sb.append("After compression length    : ")
		 	.append(afterCompressionLength).append("\n");
		sb.append("Compression Time(Real)      : ")
		 	.append(compressionTime.getReal()).append("\n");
		sb.append("Compression Time(CPU)       : ")
		 	.append(compressionTime.getCpu()).append("\n");

		sb.append("Before decompression length : ")
		 	.append(beforeDecompressionLength).append("\n");
		sb.append("After decompression length  : ")
		 	.append(afterDecompressionLength).append("\n");
		sb.append("Decompression Time(Real)    : ")
		 	.append(deCompressionTime.getReal()).append("\n");
		sb.append("Decompression Time(CPU)     : ")
		 	.append(deCompressionTime.getCpu()).append("\n");

		// return String 
		return sb.toString();
	}

	/**
	CompressInformation (int argID,
	 long originalNBytes, long compressionNBytes,
	 double compressionRealTime, double compressionCPUTime,
	 double deCompressionRealTime, double deCompressionCPUTime) {
		// set received variables into instance
		this.argID = argID;
		this.beforeCompressionLength = originalNBytes;
		this.afterCompressionLength = compressionNBytes;
		this.compressionTime =
			new TimeInfo(compressionRealTime, compressionCPUTime);
		this.deCompressionTime =
			new TimeInfo(deCompressionRealTime, deCompressionCPUTime);
		this.protocol_ver = new Version(2, 2, 0);
	}
	 */

	/**
	 * @param xmlString
	 * @throws GrpcException
	public CompressInformation(String xmlString,
	 int verMajor, int verMinor, int verPatch)
	 throws GrpcException {
		this(XMLUtil.getNode(xmlString), 
			new Version(verMajor, verMinor, verPatch));
	}
	 */


}

