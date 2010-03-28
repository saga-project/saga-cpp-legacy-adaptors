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
 * $RCSfile: CompressInformation.java,v $ $Revision: 1.5 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng;

import org.apgrid.grpc.util.XMLUtil;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class CompressInformation {
	/* definitions */
	/* tag of this node */
	protected static final String tagName = "compressionInformation";
	/* attribute for argument ID */
	private static final String attrArgumentNo = "argumentNo";
	
	/* attributes for compression */
	private static final String attrBeforeCompressionLength = "beforeCompressionLength";
	private static final String attrAfterCompressionLength = "afterCompressionLength";
	private static final String attrBeforeDecompressionLength = "beforeDecompressionLength";
	private static final String attrAfterDecompressionLength = "afterDecompressionLength";
	private static final String attrCompressionRealTime = "compressionRealTime";
	private static final String attrCompressionCPUTime = "compressionCPUtime";
	private static final String attrDeCompressionRealTime = "decompressionRealTime";
	private static final String attrDeCompressionCPUTime = "decompressionCPUtime";

	/* strings for time */
	private static final String separator = " ";
	private static final String secString = "s";
	private static final String usecString = "us";
	
	/* protocol version */
	private int versionMajor;
	private int versionMinor;
	private int versionPatch;
	
	/* information for executing Executable */
	private int argID;
	private long beforeCompressionLength;
	private long afterCompressionLength;
	private long beforeDecompressionLength;
	private long afterDecompressionLength;

	private double compressionRealTime;
	private double compressionCPUTime;
	private double deCompressionRealTime;
	private double deCompressionCPUTime;

	/**
	 * @param argID
	 */
	public CompressInformation (int argID) {
			/* set received variables into instance */
			this.argID = argID;
			
			this.versionMajor = 2;
			this.versionMinor = 2;
			this.versionPatch = 0;
		}
		
	/**
	 * @param argID
	 * @param originalNBytes
	 * @param compressionNBytes
	 * @param compressionRealTime
	 * @param compressionCPUTime
	 * @param deCompressionRealTime
	 * @param deCompressionCPUTime
	 */
	CompressInformation (int argID, long originalNBytes, long compressionNBytes,
		double compressionRealTime, double compressionCPUTime,
		double deCompressionRealTime, double deCompressionCPUTime) {
		/* set received variables into instance */
		this.argID = argID;
		this.beforeCompressionLength = originalNBytes;
		this.afterCompressionLength = compressionNBytes;
		this.compressionRealTime = compressionRealTime;
		this.compressionCPUTime = compressionCPUTime;
		this.deCompressionRealTime = deCompressionRealTime;
		this.deCompressionCPUTime = deCompressionCPUTime;
		
		this.versionMajor = 2;
		this.versionMinor = 2;
		this.versionPatch = 0;
	}
	
	/**
	 * @param xmlString
	 * @throws GrpcException
	 */
	public CompressInformation(String xmlString,
		int versionMajor, int versionMinor, int versionPatch) throws GrpcException {
		this(XMLUtil.getNode(xmlString), versionMajor, versionMinor, versionPatch);
	}

	/**
	 * @param node
	 * @throws GrpcException
	 */
	public CompressInformation(Node node,
		int versionMajor, int versionMinor, int versionPatch) throws GrpcException {
		/* set protocol version */
		this.versionMajor = versionMajor;
		this.versionMinor = versionMinor;
		this.versionPatch = versionPatch;

		/* get information about deflate */
		this.argID = getIDValue(node, attrArgumentNo);
		
		/* get information about compression */
		this.beforeCompressionLength =
			getLengthValue(node, attrBeforeCompressionLength);
		this.afterCompressionLength =
			getLengthValue(node, attrAfterCompressionLength);
		this.compressionRealTime = getTimeValue(node, attrCompressionRealTime);
		this.compressionCPUTime = getTimeValue(node, attrCompressionCPUTime);
		this.beforeDecompressionLength =
			getLengthValue(node, attrBeforeDecompressionLength);
		this.afterDecompressionLength =
			getLengthValue(node, attrAfterDecompressionLength);
		this.deCompressionRealTime = getTimeValue(node, attrDeCompressionRealTime);
		this.deCompressionCPUTime = getTimeValue(node, attrDeCompressionCPUTime);
	}
	
	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private int getIDValue(Node node, String target) throws GrpcException {
		String targetValue = XMLUtil.getAttributeValue(node, target);
		return Integer.parseInt(targetValue);
	}
	
	/**
	 * @param node
	 * @param target
	 * @return
	 * @throws GrpcException
	 */
	private long getLengthValue(Node node, String target) throws GrpcException {
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
	private double getTimeValue(Node node, String target) throws GrpcException {
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
		String usecStr = timeStr.substring(
			separatorIndex + 1, timeStr.length() - usecString.length());
		
		return Double.parseDouble(secStr + (Double.parseDouble(usecStr) / 1000000));
	}

	/**
	 * @return
	 */
	protected int getArgID() {
		return this.argID;
	}
	
	/**
	 * @return
	 */
	protected long getBeforeCompressionLength() {
		return this.beforeCompressionLength;
	}
	
	/**
	 * @return
	 */
	protected long afterCompressionLength() {
		return this.afterCompressionLength;
	}
	
	/**
	 * @return
	 */
	protected long getBeforeDecompressionLength() {
		return this.beforeDecompressionLength;
	}
	
	/**
	 * @return
	 */
	protected long afterDecompressionLength() {
		return this.afterDecompressionLength;
	}
	
	/**
	 * @return
	 */
	protected double getCompressionRealTime() {
		return this.compressionRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getCompressionCPUTime() {
		return this.compressionCPUTime;
	}
	
	/**
	 * @return
	 */
	protected double getDeCompressionRealTime() {
		return this.deCompressionRealTime;
	}
	
	/**
	 * @return
	 */
	protected double getDeCompressionCPUTime() {
		return this.deCompressionCPUTime;
	}
	
	/**
	 * @param originalNbytes
	 */
	protected void setBeforeCompressionLength(long originalNbytes) {
		this.beforeCompressionLength += originalNbytes;
	}
	
	/**
	 * @param compressionNbytes
	 */
	protected void setAfterCompressionLength(long compressionNbytes) {
		this.afterCompressionLength += compressionNbytes;
	}
	
	/**
	 * @param originalNbytes
	 */
	protected void setBeforeDecompressionLength(long originalNbytes) {
		this.beforeDecompressionLength += originalNbytes;
	}
	
	/**
	 * @param compressionNbytes
	 */
	protected void setAfterDecompressionLength(long compressionNbytes) {
		this.afterDecompressionLength += compressionNbytes;
	}
	
	/**
	 * @param compressionRealTime
	 */
	protected void setCompressionRealTime(double compressionRealTime) {
		this.compressionRealTime += compressionRealTime;
	}
	
	/**
	 * @param compressionCPUTime
	 */
	protected void setCompressionCPUTime(double compressionCPUTime) {
		this.compressionCPUTime += compressionCPUTime;
	}
	
	/**
	 * @param deCompressionRealTime
	 */
	protected void setDeCompressionRealTime(double deCompressionRealTime) {
		this.deCompressionRealTime += deCompressionRealTime;
	}
	
	/**
	 * @param deCompressionCPUTime
	 */
	protected void setDeCompressionCPUTime(double deCompressionCPUTime) {
		this.deCompressionCPUTime += deCompressionCPUTime;
	}
	
	/**
	 * @return
	 */
	public String toXMLString() {
		StringBuffer sb = new StringBuffer();
		sb.append("<" + tagName + " ");
		sb.append(attrArgumentNo + "=\"" + this.argID + "\"" + " ");

		/* compress information */
		sb.append(attrBeforeCompressionLength + "=\"" +
			this.beforeCompressionLength + "\"" + " ");
		sb.append(attrAfterCompressionLength + "=\"" + 
			this.afterCompressionLength + "\"" + " ");
		sb.append(attrCompressionRealTime + "=\"" +
			makeTimeValStr(this.compressionRealTime) + "\"" + " ");
		sb.append(attrCompressionCPUTime + "=\"" +
			makeTimeValStr(this.compressionCPUTime) + "\"" + " ");
		sb.append(attrBeforeDecompressionLength + "=\"" +
			this.beforeDecompressionLength + "\"" + " ");
		sb.append(attrAfterDecompressionLength + "=\"" + 
			this.afterDecompressionLength + "\"" + " ");
		sb.append(attrDeCompressionRealTime + "=\"" +
			makeTimeValStr(this.deCompressionRealTime) + "\"" + " ");
		sb.append(attrDeCompressionCPUTime + "=\"" +
			makeTimeValStr(this.deCompressionCPUTime) + "\"" + " ");

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

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		/* put all of data into String */
		StringBuffer sb = new StringBuffer();
		
		/* print Compression Information */
		if (beforeCompressionLength != -1) {
			sb.append("Before compression length   : " + beforeCompressionLength + "\n");
			sb.append("After compression length    : " + afterCompressionLength + "\n");
			sb.append("Compression Time(Real)      : " + compressionRealTime + "\n");
			sb.append("Compression Time(CPU)       : " + compressionCPUTime + "\n");
			sb.append("Before decompression length : " + beforeDecompressionLength + "\n");
			sb.append("After decompression length  : " + afterDecompressionLength + "\n");
			sb.append("Decompression Time(Real)    : " + deCompressionRealTime + "\n");
			sb.append("Decompression Time(CPU)     : " + deCompressionCPUTime + "\n");
		}
		/* return String */
		return sb.toString();
	}
}
