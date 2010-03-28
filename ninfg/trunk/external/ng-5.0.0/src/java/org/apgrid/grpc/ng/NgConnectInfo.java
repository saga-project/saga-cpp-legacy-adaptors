/*
 * $RCSfile: NgConnectInfo.java,v $ $Revision: 1.8 $ $Date: 2008/02/07 08:17:43 $
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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

import org.apgrid.grpc.ng.dataencode.NgEncodeData;
import org.apgrid.grpc.ng.info.RemoteMachineInfo;
import static org.apgrid.grpc.ng.protocol.ProtocolHeader.INVALID_EXECUTABLE_ID;
import org.gridforum.gridrpc.GrpcException;

/*
 * Parameter of Negotiation Information
 */
class EncodeType {
	private int id; // encode type id
	private List<Integer> options;

	public EncodeType(int id, List<Integer> options) {
		this.id = id;
		this.options = options; // new ArrayList(options)?
	}
	public int getType() { return id; }
	public List<Integer> getOptions() { return options; }
	public String toString() {
		StringBuilder sb = new StringBuilder("Type:" + id);
		sb.append(", Options:" + options);
		return sb.toString();
	}
}

/*
 * Negotiation Information
 */
public class NgConnectInfo {

	private int arch; // Architecture Number
	private int contextID;
	private int executableID;
	private int jobID;
	private int length;
	private List<EncodeType> encodeTypes;
	private Version version;
	private int nConnect;      // Number of Connect of remote
	private int simpleAuthNum; // Simple Authentication Number

	private int subJobID; 

	public static final Version VALID_PROTOCOL_VERSION =
		new Version(5, 0, 0);

	// Negotiation result error
	private static final int BAD_ARCHITECTURE = -1;
	private static final int BAD_PROTOCOL     = -2;
	private static final int BAD_CONTEXT      = -3;
	private static final int BAD_EXECUTABLE   = -4;
	private static final int BAD_ENCODE       = -5;

	private NgConnectInfo() {
		this.arch         = -1;
		this.version      = null;
		this.contextID    = -1;
		this.executableID = INVALID_EXECUTABLE_ID;
		this.jobID        = -1;
		this.length       = -1;
		this.nConnect     = -1;
		this.encodeTypes  = null;
		this.subJobID     = -1;
	}

	private void setArch(int number)     { this.arch         = number; }
	private void setSubJobId(int id)     { this.subJobID     = id; }
	private void setContextId(int id)    { this.contextID    = id; }
	private void setExecutableId(int id) { this.executableID = id; }
	private void setJobId(int id)        { this.jobID        = id; }
	private void setAuthNum(int number)  { this.simpleAuthNum = number; }
	private void setNumOfConnect(int number) { this.nConnect = number; }
	private void setLength(int length)   { this.length       = length; }
	private void setVersion(int major, int minor, int patch) {
		this.version = new Version(major, minor, patch);
	}
	private void setVersionByInt(int aVersion) {
		int vMajor = aVersion >>> 24;
		int vMinor = (aVersion >>> 16) & 0xff;
		int vPatch = (aVersion & 0xffff);
		this.version =
			new Version(vMajor, vMinor, vPatch);
	}
	private void setEncodeType(List<EncodeType> list) {
		this.encodeTypes = list;
	}

	/**
	 * Creates Connection Information by Ninf-G Client side.
	 * 
	 * @param contextID
	 * @param executableID
	 * @param subJobID
	 * @param jobID
	 * @return
	 */
	protected static NgConnectInfo createLocalConnectInfo(int contextID,
	 int executableID, int subJobID, int jobID,
	 RemoteMachineInfo remoteMachineInfo, int authNum,
	 boolean sendEncodeType, int nConnectLocal) {
		// setting for Ninf-G Java 
		List<EncodeType> encodeTypes = new ArrayList<EncodeType>();

		int length = 0;
		if (sendEncodeType) {
			// set encodeType to Argument #1
			setEncodeType(encodeTypes, remoteMachineInfo);
			length = calcParameterLength(encodeTypes);
		}

		// Architecture Number(0) and Version number(5, 0, 0) uses
		// fixed number on Ninf-G Java Client.
		NgConnectInfo ret = new NgConnectInfo();
		ret.setArch( 0 );
		ret.setVersion( 5, 0, 0 );
		ret.setAuthNum( authNum );
		ret.setNumOfConnect( nConnectLocal );
		ret.setContextId( contextID );
		ret.setExecutableId( executableID );
		ret.setSubJobId( subJobID );
		ret.setJobId( jobID );
		ret.setLength( length );
		if (sendEncodeType) {
			ret.setEncodeType( encodeTypes );
		}
			return ret;
	}

	private static void setEncodeType(List<EncodeType> anEncodeTypes,
	 RemoteMachineInfo rmi) {
		if ( anEncodeTypes == null ) { throw new IllegalArgumentException(); }

		for (Integer encodeTypeId : NgEncodeData.getSupportEncodeType() ) {
			switch ( encodeTypeId ) {
			case NgEncodeData.NG_DATA_REPRESENT_RAW:
				anEncodeTypes.add( new EncodeType(encodeTypeId, null) );
				break;
			case NgEncodeData.NG_COMPRESS_ZLIB:
				CompressInfo compressInfo = rmi.getCompressInfo();
				if ( compressInfo.getCompress()
						.equals(CompressInfo.COMPRESS_ZLIB) ) {
					List<Integer> options = new ArrayList<Integer>();
					options.add( compressInfo.getCompressThreshold() );

					anEncodeTypes.add( 
						new EncodeType(encodeTypeId, options) );
				}
				break;
			case NgEncodeData.NG_DATA_DIVIDE:
				List<Integer> options = new ArrayList<Integer>();
				options.add(Integer.parseInt(rmi.getBlockSize()));
				anEncodeTypes.add( new EncodeType(encodeTypeId, options) );
				break;
			}
		}
	}

	private static int calcParameterLength(final List<EncodeType> params) {
		int length = 0;
		for (EncodeType et : params) {
			if (et.getOptions() == null ) {
				length +=  8; // id len(4) + nOptions len(4)
			} else {
				length += 12; // id len(4) + nOptions len(4) + option len(4)
			}
		}
		return length;
	}


	/**
	 * Creates Connection Information by Ninf-G Executable side.
	 * 
	 * @param buffer
	 * @return
	 */
	public static NgConnectInfo createRemoteConnectInfo(byte[] buffer)
	 throws GrpcException {
		XDRInputStream xi =
			new XDRInputStream(new ByteArrayInputStream(buffer));
		NgConnectInfo ret = new NgConnectInfo();

		// parse negotiation header info(Don't correct this order)
		ret.setArch(         xi.readInt() );
		ret.setVersionByInt( xi.readInt() );
		ret.setAuthNum(      xi.readInt() );
		ret.setNumOfConnect( xi.readInt() );
		ret.setContextId(    xi.readInt() );
		ret.setJobId(        xi.readInt() );
		ret.setExecutableId( xi.readInt() );
		int paramLength =    xi.readInt();
		ret.setLength(       paramLength  );
		// parse parameter of negotiation info.
		List<EncodeType> encodeTypes = readEncodeType(xi, paramLength);
		ret.setEncodeType(   encodeTypes );

		try {
			xi.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		return ret;
	}

	private static List<EncodeType> readEncodeType(
	 XDRInputStream xdrIn, int paramLen)
	 throws GrpcException {
		List<EncodeType> ret = new ArrayList<EncodeType>();
		if(paramLen == 0) { return ret; }

		int readLen = 0;
		while (readLen < paramLen) {
			Integer encodeTypeId = Integer.valueOf(xdrIn.readInt());
			readLen += 4;

			// read length of parameter 
			int numOfParam = xdrIn.readInt();
			readLen += 4;

			List<Integer> options = new ArrayList<Integer>();
			for (int i = 0; i < numOfParam; i++) {
				options.add( xdrIn.readInt() );
				readLen += 4;
			}
			ret.add( new EncodeType(encodeTypeId, options) );
		}
		return ret;
	}


	/**
	 * @return
	 */
	public byte[] toByteArray() throws GrpcException {
		NgByteArrayOutputStream bos =
			new NgByteArrayOutputStream(NgGlobals.smallBufferSize);

		byte [] ret = null;
		// write connectInfo to byte array
		try {
			// Don't correct this order
			bos.writeInt(arch);
			bos.writeInt(version.getInt());
			bos.writeInt(simpleAuthNum);
			bos.writeInt(nConnect);
			bos.writeInt(contextID);
			bos.writeInt(jobID);
			bos.writeInt(executableID);

			if ((encodeTypes == null) || (encodeTypes.isEmpty())) {
				 // there is no parameter
				if (length != 0) 
					throw new RuntimeException("parameter length is not 0");
				bos.writeInt(length);
			} else {
				// write parameter length to byte array by OutputStream
				int notSendBytes = 0;
				boolean hasZlib = false;
				for (EncodeType et : encodeTypes) {
					if (et.getType() == NgEncodeData.NG_COMPRESS_ZLIB) {
						hasZlib = true;
						notSendBytes = 8; // don't send information about RAW
					}
				}
				bos.writeInt(length - notSendBytes);

				for (EncodeType et : encodeTypes) {
					// write parameter type
					int typeId = et.getType();
					if ( (typeId == NgEncodeData.NG_DATA_REPRESENT_RAW)
						&& hasZlib ) {
						// RAW is only required when ZLIB is not available
						//  on remote 
						continue;
					}
					bos.writeInt( typeId );

					// write options(supported encode type list)
					switch ( typeId ) {
					case NgEncodeData.NG_DATA_REPRESENT_RAW:
						bos.writeInt(0); // no element in type of RAW
						break;
					case NgEncodeData.NG_COMPRESS_ZLIB:
					case NgEncodeData.NG_DATA_DIVIDE:
						// type of NG_COMPRESS_ZLIB & NG_DATA_DIVIDE is same
						// logic 
						bos.writeInt( et.getOptions().size() );
						for (Integer opt : et.getOptions() ) {
							bos.writeInt( opt );
						}
						break;
					}
				}
			}
			// get buffer 
			ret = bos.toByteArray();
			bos.close();
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		return ret;
	}

	/**
	 * Check the Negotiation Information, Architecture Number, Protocol Version,
	 * Context ID, Executable ID & Encode Type.
	 * 
	 * @param connectInfo
	 * @return 0 is OK, -1 to -5 are error.
	 */
	public int check() {
		// compare with information of local 
		// archNum 
		if (! isValidArchNumber(this.arch) ) {
			return BAD_ARCHITECTURE;
		}
		// protocol version
		if (! isValidProtocolVersion(this.version) ) {
			return BAD_PROTOCOL;
		}
		// contextID 
		if ( NgGlobals.getContext(this.contextID) == null) {
			return BAD_CONTEXT;
		}

		// executableID(JobID,subJobID) 
		if (this.executableID == INVALID_EXECUTABLE_ID) {
			return BAD_EXECUTABLE;
		}

		// information about data encoding
		if ( this.encodeTypes.isEmpty() ) {
			// no data encoding was supported, maybe use raw data 
			return 0; // OK
		}

		// encodeType 
		boolean supportedParameter = false;
		for (EncodeType et : this.encodeTypes) {
			if ( NgEncodeData.getSupportEncodeType().contains(
				et.getType()) ) {
				supportedParameter = true;
				break;
			}
		}
		if (! supportedParameter )  {
			return BAD_ENCODE;
		}
		return 0; // OK 
	}

	/*
	 * The architecture number is referred to for `getarchnumber' script.
	 * AFX8,BFLY,CM2,CM5, ...=11, ...
	 * LINUXIA64=23, LINUXAMD64=24, MACOSX=25
	 *
	 * @param anArch architecture number
	 * @return true valid architecture number.
	 * @return false invalid architecture number.
	 */
	private boolean isValidArchNumber(final int anArch) {
		return ((anArch >= 11) && (anArch <= 25));
	}

	private boolean isValidProtocolVersion(final Version target_version) {
		return target_version.compareTo(VALID_PROTOCOL_VERSION) >= 0;
	}

	protected List getSupportEncodeType() {
		List ret = new ArrayList();
		for (EncodeType et : this.encodeTypes) {
			if ( NgEncodeData.getSupportEncodeType().contains(et.getType()) )
				ret.add(et.getType());
		}
		return ret;
	}

	protected int getNumberOfConnection() { return nConnect; }
	protected int getJobID() { return jobID; }
	protected int getExecutableID() { return executableID; }
	protected void setExecutableID(int id) { this.executableID = id; }
	protected int getParameterLength() { return length; }
	public Version getVersion() { return version; }
	public int getAuthNum() { return this.simpleAuthNum; }

	public String toString() {
		StringBuilder sb = new StringBuilder("NgConnectInfo[");
		sb.append(arch).append(",");
		sb.append(version).append(",");
		sb.append(simpleAuthNum).append(",");
		sb.append(nConnect).append(",");
		sb.append(contextID).append(",");
		sb.append(jobID).append(",");
		sb.append(executableID).append(",");
		sb.append(length).append(",");
		if (encodeTypes != null)
			sb.append(encodeTypes);
		sb.append("]");
		return sb.toString();
	}

}
