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
 * $RCSfile: NgDataConvertBase64.java,v $ $Revision: 1.7 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng.dataencode;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.util.List;

import org.apgrid.grpc.ng.NgIOException;
import org.globus.util.Base64;
import org.gridforum.gridrpc.GrpcException;

class NgDataConvertBase64 implements NgEncodeDataInterface {
	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgEncodeDataInterface#decodeData(byte[], java.util.Vector)
	 */
	public byte[] decodeData(byte[] buffer, List encodeInfo) throws GrpcException {
		StringReader sr = new StringReader(new String(buffer));
		BufferedReader br = new BufferedReader(sr);
			
		/* read from base64 file */
		StringBuffer sb = new StringBuffer();
		String readLine;
		try {
			while ((readLine = br.readLine()) != null) {
				sb.append(readLine);
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
		/* close stream */
		sr.close();
		
		return Base64.decode(sb.toString().getBytes());
	}

	/* (non-Javadoc)
	 * @see org.apgrid.grpc.ng.NgEncodeDataInterface#encodeData(byte[], java.util.Vector)
	 */
	public byte[] encodeData(byte[] buffer, List encodeInfo) throws GrpcException {
		return Base64.encode(buffer);
	}
}
