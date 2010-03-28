/*
 * $RCSfile: Base64.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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


/*
 * Base64 decoder for LocalLDIF
 */
public class Base64 {

	public static byte [] encode(byte[] anArray) {
		throw new UnsupportedOperationException();
	}

	public static byte[] decode(byte [] bs64Str) {
		int nBlocks = bs64Str.length / 4;
		if ((nBlocks * 4) != bs64Str.length) {
			throw new IllegalArgumentException("There is surplus");
		}

		int padding = 0; // no padding
		if ( (bs64Str[bs64Str.length -2] == '=' ) &&
			 (bs64Str[bs64Str.length -1] == '=' ) ) {
			padding = 2; // 2byte paddings
		} else if ( bs64Str[bs64Str.length -1] == '=' ) {
			padding = 1; // 1byte padding
		}

		byte [] result = new byte[ (nBlocks * 3) - padding];

		int inIndex = 0, outIndex = 0;
		int endBlock = nBlocks - (padding == 0 ? 0 : 1) ;
		int ch0, ch1, ch2, ch3;
		byte out1, out2, out3;
		for (int i = 0; i <  endBlock; i++ ) {
			ch0 = base64ToCode( bs64Str[inIndex++] );
			ch1 = base64ToCode( bs64Str[inIndex++] );
			ch2 = base64ToCode( bs64Str[inIndex++] );
			ch3 = base64ToCode( bs64Str[inIndex++] );
			int resultChars =
				( (( ch0 & 0x3F) << 18) | (( ch1 & 0x3F) << 12)
				| (( ch2 & 0x3F) <<  6) |  ( ch3 & 0x3F));
			out1 = (byte)((resultChars >> 16) & 0xFF);
			out2 = (byte)((resultChars >>  8) & 0xFF);
			out3 = (byte)(resultChars & 0xFF);
			result[outIndex++] = out1;
			result[outIndex++] = out2;
			result[outIndex++] = out3;
		}

		if (padding != 0) {
			// there is padding
			ch0 = base64ToCode( bs64Str[inIndex++] );
			ch1 = base64ToCode( bs64Str[inIndex++] );
			ch2 = ch3 = 0;
			if (padding == 1) {
				ch2 = base64ToCode( bs64Str[inIndex++] );
			}
			int resultChars = 
				( (( ch0 & 0x3F) << 18) | (( ch1 & 0x3F) << 12)
				| (( ch2 & 0x3F) <<  6) |  ( ch3 & 0x3F));
			out1 = (byte)((resultChars >> 16) & 0xFF);
			out2 = (byte)((resultChars >>  8) & 0xFF);
			result[outIndex++] = out1;
			if (padding == 1) {
				result[outIndex++] = out2;
			}
		}

		return result;
	}

	private static int base64ToCode(byte num) {
		int ret = DECODE_TABLE[num];
		if ( ret < 0) {
			throw new IllegalArgumentException( num + " is not Base64 Char");
		}
		return ret;
	}

	private static final byte [] DECODE_TABLE = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0 .. 9
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 10 .. 19
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 20 .. 29
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 30 .. 39
		-1, -1, -1, 62, -1, -1, -1, 63, 52, 53, // 40 .. 49
		54, 55, 56, 57, 58, 59, 60, 61, -1, -1, // 50 .. 59
		-1, -1, -1, -1, -1,  0,  1,  2,  3,  4, // 60 .. 69
		 5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 70 .. 79
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 80 .. 89
		25, -1, -1, -1, -1, -1, -1, 26, 27, 28, // 90 .. 99
		29, 30, 31, 32, 33, 34, 35, 36, 37, 38, // 100 .. 109
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, // 110 .. 119
		49, 50, 51                              // 120 
	};

}

