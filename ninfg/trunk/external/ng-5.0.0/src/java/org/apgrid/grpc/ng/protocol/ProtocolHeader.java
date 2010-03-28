/*
 * $RCSfile: ProtocolHeader.java,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:08 $
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
package org.apgrid.grpc.ng.protocol;

import java.io.IOException;
import org.apgrid.grpc.ng.NgByteArrayOutputStream;

public class ProtocolHeader {

	public static final int LENGTH = 32; // Protocol Header length
	public static final int INVALID_EXECUTABLE_ID = -1;
	public static final int INVALID_SESSION_ID = -1;
	public static final int FIRST_CONNECTION = 0;

    private final int type; // request, reply, notify
    private final int id;
    private final int sequence;
    private final int contextID;
    private final int executableID;
    private final int sessionID;
    private final int result;
    private final int length;

	public ProtocolHeader(int type, int id, int sequenceNum, int contextID,
	 int executableID, int sessionID, int result, int paramLength) {
		this.type         = type;
		this.id           = id;
		this.sequence     = sequenceNum;
		this.contextID    = contextID;
		this.executableID = executableID;
		this.sessionID    = sessionID;
		this.result       = result;
		this.length       = paramLength;
	}

	public int getType() {
		return this.type;
	}

	public int getId() {
		return this.id;
	}

	public int getResult() {
		return this.result;
	}

	public int getSequence() {
		return this.sequence;
	}

	public int getParamLength() {
		return this.length;
	}

	public byte[] toByteArray() {
		return pack();
	}

	private byte[] pack() {
		NgByteArrayOutputStream bos = 
			new NgByteArrayOutputStream(LENGTH);
		
		byte[] ret = null;
		try {
			int kind = (type << 24) + id;
            bos.writeInt(kind);
            bos.writeInt(sequence);
            bos.writeInt(contextID);
            bos.writeInt(executableID);
            bos.writeInt(sessionID);
            bos.writeInt(0);  // padding
            bos.writeInt(result);
			bos.writeInt(length);
			ret = bos.toByteArray();
			bos.close();
        } catch (IOException e) {
            return null;
        }
		return ret;
	}

	public String toString() {
		StringBuilder sb = new StringBuilder("[");
    	sb.append(type).append(", ");
    	sb.append("0x").append(Integer.toHexString(id)).append(", ");
    	sb.append(sequence).append(", ");
    	sb.append(contextID).append(", ");
    	sb.append(executableID).append(", ");
    	sb.append(sessionID).append(", ");
    	sb.append(result).append(", ");
    	sb.append(length);
		sb.append("]");
		return sb.toString();
	}

}

