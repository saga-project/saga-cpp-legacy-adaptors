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
 * $RCSfile: NgConnectInfo.java,v $ $Revision: 1.31 $ $Date: 2006/02/08 08:33:37 $
 */

package org.apgrid.grpc.ng;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.Vector;

import org.apgrid.grpc.ng.dataencode.NgEncodeData;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import org.gridforum.gridrpc.GrpcException;

public class NgConnectInfo {
	private int archNum;
	private int versionMajor;
	private int versionMinor;
	private int versionPatchLevel;
	private int contextID;
	private int executableID;
	private int subJobID;
	private int jobID;
	private int length;
	private List encodeType;
	private List encodeInfo;
	
	/**
	 * @param archNum
	 * @param versionMajor
	 * @param versionMinor
	 * @param versionPatchLevel
	 * @param contextID
	 * @param executableID
	 * @param subJobID
	 * @param jobID
	 * @param targetList
	 */
	private NgConnectInfo (int archNum, int versionMajor, int versionMinor,
		int versionPatchLevel, int contextID, int executableID,
		int subJobID, int jobID, int length, List encodeType,
		List encodeInfo) {
		this.archNum = archNum;
		this.versionMajor = versionMajor;
		this.versionMinor = versionMinor;
		this.versionPatchLevel = versionPatchLevel;
		this.contextID = contextID;
		this.executableID = executableID;
		this.subJobID = subJobID;
		this.jobID = jobID;
		this.length = length;
		if (encodeType != null) {
			this.encodeType = encodeType;
		} else {
			this.encodeType = null;
		}
		this.encodeInfo = encodeInfo;
	}
	
	/**
	 * @param contextID
	 * @param executableID
	 * @param subJobID
	 * @param jobID
	 * @return
	 */
	protected static NgConnectInfo getLocalConnectInfo(int contextID,
		int executableID, int subJobID, int jobID,
		RemoteMachineInfo remoteMachineInfo) {
		/* setting for Ninf-G Java */
		int archNum = 0;
		int versionMajor = 2;
		int versionMinor = 2;
		int versionPatchLevel = 0;
		List encodeType = new Vector();
		List encodeInfo = new Vector();
		int length = 0;
		
		List supportedEncodeType = NgEncodeData.getSupportEncodeType();
		
		/* set encodeType */
		for (int i = 0; i < supportedEncodeType.size(); i++) {
			if (versionMinor > 0) {
				Integer encodeID = (Integer)supportedEncodeType.get(i);

				/* add supported encode types into list */
				/* raw */
				if (encodeID.intValue() == NgEncodeData.NG_DATA_REPRESENT_RAW) {
					encodeType.add(supportedEncodeType.get(i));
					length += 8;
				}

				/* zlib */
				else if (encodeID.intValue() == NgEncodeData.NG_COMPRESS_ZLIB) {
					String compressEnable =
						(String) remoteMachineInfo.get(RemoteMachineInfo.KEY_COMPRESS);
					if (compressEnable.equals(RemoteMachineInfo.VAL_COMPRESS_ZLIB)) {
						encodeType.add(supportedEncodeType.get(i));
						encodeInfo.add(remoteMachineInfo.get(
							RemoteMachineInfo.KEY_COMPRESS_THRESHOLD));
				
						length += 12;
					}
				}

				/* divide */
				else if (encodeID.intValue() == NgEncodeData.NG_DATA_DIVIDE) {
					encodeType.add(supportedEncodeType.get(i));
					encodeInfo.add(remoteMachineInfo.get(
						RemoteMachineInfo.KEY_BLOCK_SIZE));
				
					length += 12;
				}
			} else {
				encodeType.add(supportedEncodeType.get(i));
				length += 4;
			}
		}
		
		return new NgConnectInfo(archNum, versionMajor, versionMinor,
			versionPatchLevel, contextID, executableID, subJobID,
			jobID, length, encodeType, encodeInfo);
	}
	
	/**
	 * @param buffer
	 * @return
	 */
	protected static NgConnectInfo getRemoteConnectInfo(byte[] buffer)
		throws GrpcException{
		ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
		XDRInputStream xi = new XDRInputStream(bi);
		
		/* parse connectInfo */
		int archNum = xi.readInt();
		int version = version = xi.readInt();
		int versionMajor = version >>> 24;
		int versionMinor = (version >>> 16) & 0xff;
		int versionPatchLevel = (version & 0xffff);
		int contextID = xi.readInt();
		int jobID = xi.readInt();
		int dummy = xi.readInt(); /* read subJobID */
		dummy = xi.readInt(); /* read padding byte */
		dummy = xi.readInt(); /* read padding byte */
		int parameterLength = xi.readInt();

		/* parse list elemnts */
		List encodeType = new Vector();
		List encodeInfo = new Vector();
		int readLength = 0;
		if(parameterLength > 0) {
			while (readLength < parameterLength) {
				/* added encode type into List */
				encodeType.add(new Integer(xi.readInt()));
				readLength += 4;

				if (versionMinor > 0) {
					/* read length of parameter */
					int numOfParam = xi.readInt();
					readLength += 4;
					byte[] tmpBuffer = new byte[numOfParam * 4];
					xi.readBytes(tmpBuffer, 0, numOfParam * 4);
					encodeInfo.add(tmpBuffer);
					readLength += (numOfParam * 4);
				}
			}
		}
		
		/* close ByteArrayInputStream */
		xi.close();
		
		/* create NgConnectInfo and return it */
		return new NgConnectInfo(archNum, versionMajor, versionMinor,
			versionPatchLevel, contextID, 0, 0, jobID, parameterLength,
			encodeType, encodeInfo);
	}
	
	/**
	 * @return
	 */
	protected byte[] getByteArray() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);
		
		/* write connectInfo */
		try {
			xo.writeInt(archNum);
			xo.writeInt((versionMajor << 24) +
				(versionMinor << 16 + versionPatchLevel));
			xo.writeInt(contextID);
			xo.writeInt(executableID);
			xo.writeInt(0);  /* padding */
			xo.writeInt(0);  /* padding */
			xo.writeInt(0);  /* padding */

			if (encodeType == null) {
				/* no Lists here (length MUST be 0) */
				xo.writeInt(length);
			} else {
				/* Does the remote have Zlib? */
				int notSendBytes = 0;
				boolean hasZlib = false;
				if (encodeType.contains(new Integer(NgEncodeData.NG_COMPRESS_ZLIB))) {
					hasZlib = true;
					/* don't send information about RAW */
					notSendBytes = 8;
				}
				
				/* write length of parameter */
				xo.writeInt(length - notSendBytes);
				
				/* write supported encodeType list */
				int infoIndex = 0;
				for (int i = 0; i < encodeType.size(); i++) {
					Integer targetInt = (Integer)(encodeType.get(i));
					int type = targetInt.intValue();
					if ((type == NgEncodeData.NG_DATA_REPRESENT_RAW) &&
						(hasZlib == true)) {
						/* RAW is only required when ZLIB is not available on remote */
						continue;
					}
					/* write type of encode */
					xo.writeInt(type);

					if (versionMinor > 0) {
						if (type == NgEncodeData.NG_DATA_REPRESENT_RAW) {
							/* write number of elements */
							xo.writeInt(0);
						}
						else if (type == NgEncodeData.NG_COMPRESS_ZLIB) {
							/* write number of elements */
							xo.writeInt(1);
							/* write compress threshold */
							String compressThreshold =
								(String) encodeInfo.get(infoIndex++);
							xo.writeInt(Integer.parseInt(compressThreshold));
						}
						else if (type == NgEncodeData.NG_DATA_DIVIDE) {
							/* write number of elements */
							xo.writeInt(1);
							/* write divide size */
							String blockSize =	(String) encodeInfo.get(infoIndex++);
							xo.writeInt(Integer.parseInt(blockSize));
						}
					}
				}
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		
		/* get buffer */
		byte[] connectInfoBuffer = bo.toByteArray();
		/* close ByteArrayOutputStream */
		xo.close();

		return connectInfoBuffer;
	}
	
	/**
	 * @param connectInfo
	 * @return
	 * @throws GrpcException
	 */
	protected static int checkConnectInfo(NgConnectInfo connectInfo)
		throws GrpcException {
		/* compare with information of local */
		/* archNum */
		if (!((connectInfo.archNum > 10) && (connectInfo.archNum < 26))) {
			return -1;
		}
		/* version */
		if (connectInfo.versionMajor != 2) {
			return -2;			
		}
		/* contextID */
		NgGrpcClient context = NgGlobals.getContext(connectInfo.contextID);
		if (context == null) {
			return -3;
		}
		
		/* executableID(JobID,subJobID) */
		if (connectInfo.executableID == -1) {
			return -4;			
		}
		
		/* information about data encoding */
		if (connectInfo.encodeType == null) {
			/* no data encoding was supported, maybe use raw data */
			return 0;
		}
		
		/* encodeType */
		if (connectInfo.versionMinor > 0) {
			boolean supportedParameter = false;
			for (int i = 0; i < connectInfo.encodeType.size(); i++) {
				if (NgEncodeData.getSupportEncodeType().contains(
					connectInfo.encodeType.get(i))) {
					supportedParameter = true;
					break;
				}
			}
			if (supportedParameter == false)  {
				return -5;
			}
		}
		
		/* OK */
		return 0;
	}
	
	/**
	 * @return
	 */
	protected int getJobID() {
		return jobID;
	}
	
	/**
	 * @return
	 */
	protected int getExecutableID() {
		return executableID;
	}
	
	/**
	 * @param executableID
	 */
	protected void setExecutableID(int executableID) {
		this.executableID = executableID; 
	}
	
	/**
	 * @return
	 */
	protected int getParameterLength() {
		return length;
	}

	/**
	 * @param remoteConnectInfo
	 * @return
	 */
	protected static List getSupportEncodeType(
		NgConnectInfo remoteConnectInfo) {
		List tmpList = new Vector();
		tmpList.add(new Integer(NgEncodeData.NG_DATA_REPRESENT_RAW));
		for (int i = 0; i < remoteConnectInfo.encodeType.size(); i++) {
			if (NgEncodeData.getSupportEncodeType().contains(
				remoteConnectInfo.encodeType.get(i))) {
				tmpList.add(remoteConnectInfo.encodeType.get(i));
			}
		}
		return tmpList;
	}
	
	/**
	 * @return
	 */
	protected int getVersionMajor() {
		return versionMajor;
	}
	
	/**
	 * @return
	 */
	protected int getVersionMinor() {
		return versionMinor;
	}
	
	/**
	 * @return
	 */
	protected int getVersionPatch() {
		return versionPatchLevel;
	}
}
