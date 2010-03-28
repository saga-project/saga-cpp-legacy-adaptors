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
 * $RCSfile: NgEncodeData.java,v $ $Revision: 1.9 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng.dataencode;

import java.io.ByteArrayInputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import org.apgrid.grpc.ng.XDRInputStream;
import org.gridforum.gridrpc.GrpcException;

public class NgEncodeData {
	/* ID for binary protocol & negotiation */
	public static final int NG_DATA_REPRESENT_RAW = 0x0000;
	public static final int NG_DATA_REPRESENT_NINF = 0x0001;
	public static final int NG_DATA_REPRESENT_XDR = 0x0002;
	
	public static final int NG_THIN_OUT_RAW = 0x0010;
	public static final int NG_THIN_OUT_DEFAULT = 0x0011;
	public static final int NG_THIN_OUT_MULTI = 0x0012;
	public static final int NG_THIN_OUT_DIFF = 0x0013;
	public static final int NG_THIN_OUT_ZERO = 0x0014;
	
	public static final int NG_COMPRESS_RAW = 0x0020;
	public static final int NG_COMPRESS_ZLIB = 0x0021;
	
	public static final int NG_DATA_CONVERT_RAW = 0x0030;
	public static final int NG_DATA_CONVERT_BASE64 = 0x0031;
	public static final int NG_DATA_CONVERT_STRING = 0x0032;
	
	public static final int NG_DATA_DIVIDE = 0x0041;
	
	public static final int NG_THIN_OUT_SKIP = 0x0100;

	/* ID for XML protocol */
	public static final String NG_DATA_REPRESENT_RAW_STR = "represent_raw";
	public static final String NG_DATA_REPRESENT_NINF_STR = "ninf";
	public static final String NG_DATA_REPRESENT_XDR_STR = "xdr";

	public static final String NG_THIN_OUT_RAW_STR = "thinout_raw";
	public static final String NG_THIN_OUT_SKIP_STR = "skip";
	public static final String NG_THIN_OUT_DEFAULT_STR = "default";
	public static final String NG_THIN_OUT_MULTI_STR = "multiple";
	public static final String NG_THIN_OUT_DIFF_STR = "difference";
	public static final String NG_THIN_OUT_ZERO_STR = "zerosuppress";

	public static final String NG_COMPRESS_RAW_STR = "compress_raw";
	public static final String NG_COMPRESS_ZLIB_STR = "zlib";

	public static final String NG_DATA_CONVERT_RAW_STR = "convert_raw";
	public static final String NG_DATA_CONVERT_STRING_STR = "string";
	public static final String NG_DATA_CONVERT_BASE64_STR = "base64";

	public static final String NG_DATA_DIVIDE_STR = "divide";

	/* map for encoder */
	public static Map mapEncoder = new HashMap();
	public static Map mapIDEncoder = new HashMap();
	
	/* supported expressions */
	private static List supportedEncodeType = new Vector();
	
	/* register encoders into map */
	static {
		/* set supported encode types */
		supportedEncodeType.add(new Integer(NG_DATA_REPRESENT_RAW)); /* RAW */
		//supportedEncodeType.add(new Integer(NG_DATA_REPRESENT_XDR)); /* XDR */
		//.add(new Integer(NG_THIN_OUT_SKIP)); /* skip */
		supportedEncodeType.add(new Integer(NG_COMPRESS_ZLIB)); /* zlib */
		//supportedEncodeType.add(new Integer(NG_DATA_CONVERT_BASE64)); /* base64 */
		supportedEncodeType.add(new Integer(NG_DATA_DIVIDE)); /* Divide */

		/* representation */
		mapEncoder.put(NG_DATA_REPRESENT_RAW_STR, new NgEncodeDataRaw());
		mapEncoder.put(NG_DATA_REPRESENT_NINF_STR, null);
		mapEncoder.put(NG_DATA_REPRESENT_XDR_STR, new NgDataRepresentationXDR());		

		/* thin out */
		mapEncoder.put(NG_THIN_OUT_RAW_STR, new NgEncodeDataRaw());
		mapEncoder.put(NG_THIN_OUT_SKIP_STR, new NgThinOutSkip());
		mapEncoder.put(NG_THIN_OUT_DEFAULT_STR, null);
		mapEncoder.put(NG_THIN_OUT_MULTI_STR, null);
		mapEncoder.put(NG_THIN_OUT_DIFF_STR, null);
		mapEncoder.put(NG_THIN_OUT_ZERO_STR, null);
		
		/* compress */
		mapEncoder.put(NG_COMPRESS_RAW_STR, new NgEncodeDataRaw());
		mapEncoder.put(NG_COMPRESS_ZLIB_STR, new NgCompressionZlib());
		
		/* convert */
		mapEncoder.put(NG_DATA_CONVERT_RAW_STR, new NgEncodeDataRaw());
		mapEncoder.put(NG_DATA_CONVERT_STRING_STR, null);
		mapEncoder.put(NG_DATA_CONVERT_BASE64_STR, new NgDataConvertBase64());
		
		/* supported representation */
		mapIDEncoder.put(new Integer(NG_DATA_REPRESENT_RAW),
			NG_DATA_REPRESENT_RAW_STR);	/* raw */
		mapIDEncoder.put(new Integer(NG_DATA_REPRESENT_XDR),
			NG_DATA_REPRESENT_XDR_STR); /* XDR */

		/* supported thinOut */
		mapIDEncoder.put(new Integer(NG_THIN_OUT_RAW),
			NG_THIN_OUT_RAW_STR); /* raw */
		mapIDEncoder.put(new Integer(NG_THIN_OUT_SKIP),
			NG_THIN_OUT_SKIP_STR); /* skip */
		
		/* supported compression */
		mapIDEncoder.put(new Integer(NG_COMPRESS_RAW),
			NG_COMPRESS_RAW_STR); /* raw */
		mapIDEncoder.put(new Integer(NG_COMPRESS_ZLIB),
			NG_COMPRESS_ZLIB_STR); /* zlib */
		
		/* supported convert */
		mapIDEncoder.put(new Integer(NG_DATA_CONVERT_RAW),
			NG_DATA_CONVERT_RAW_STR); /* raw */
		mapIDEncoder.put(new Integer(NG_DATA_CONVERT_BASE64),
			NG_DATA_CONVERT_BASE64_STR); /* base64 */
		
		/* supported divide */
		mapIDEncoder.put(new Integer(NG_DATA_DIVIDE),
			NG_DATA_DIVIDE_STR); /* divide */
	}
	
	/**
	 * @return
	 */
	public static List getSupportEncodeType() {
		return supportedEncodeType;
	}

	/**
	 * @param buffer
	 * @param encodeInfo
	 * @return
	 * @throws GrpcException
	 */
	public static byte[] decode(byte[] buffer, List encodeInfo) throws GrpcException {
		/* decode by specified decoder */
		while (true) {
			ByteArrayInputStream bi = new ByteArrayInputStream(buffer);
			XDRInputStream xi = new XDRInputStream(bi);

			int type = xi.readInt();
			NgEncodeDataInterface decoder = (NgEncodeDataInterface) mapEncoder.get(
				mapIDEncoder.get(new Integer(type)));
			buffer = decoder.decodeData(buffer, encodeInfo);
			if (type == NG_DATA_CONVERT_RAW) {
				break;
			}
		}

		return buffer;
	}
	
	/**
	 * @param type
	 * @param buffer
	 * @param encodeInfo
	 * @return
	 * @throws GrpcException
	 */
	public static byte[] decode(int type, byte[] buffer,
		List encodeInfo) throws GrpcException {
		/* decode by specified decoder */
		NgEncodeDataInterface decoder = (NgEncodeDataInterface) mapEncoder.get(
			mapIDEncoder.get(new Integer(type)));

		return decoder.decodeData(buffer, encodeInfo);
	}
	
	/**
	 * @param type
	 * @param buffer
	 * @param encodeInfo
	 * @return
	 * @throws GrpcException
	 */
	public static byte[] decode(String type, byte[] buffer,
		List encodeInfo) throws GrpcException {
		if (mapEncoder.containsKey(type)) {
			NgEncodeDataInterface encoder =
				(NgEncodeDataInterface) mapEncoder.get(type);
			return encoder.decodeData(buffer, encodeInfo);
		}
		return buffer;
	}
	
	/**
	 * @param type
	 * @param buffer
	 * @param encodeInfo
	 * @return
	 * @throws GrpcException
	 */
	public static byte[] encode(int type, byte[] buffer,
		List encodeInfo) throws GrpcException {
		return encode(
			(String)mapIDEncoder.get(new Integer(type)),
			buffer, encodeInfo);
	}
	
	/**
	 * @param type
	 * @param buffer
	 * @param encodeInfo
	 * @return
	 * @throws GrpcException
	 */
	public static byte[] encode(String type, byte[] buffer,
		List encodeInfo) throws GrpcException {
		if (mapEncoder.containsKey(type)) {
			NgEncodeDataInterface encoder =
				(NgEncodeDataInterface) mapEncoder.get(type);
			return encoder.encodeData(buffer, encodeInfo);
		}
		return buffer;
	}
}
