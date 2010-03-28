/*
 * $RCSfile: CommLog.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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

class CommLog {

	/*
	 * Return formatted communication data.
	 *
	 * @param data Communication data.
	 * @param length Communication data length
	 */
	public static String dump(byte [] data, int length) {
		StringBuilder sb = new StringBuilder();
		int count = 0;
		while (count < length) {
			sb.append(makeCommLogLine(data, count, 16));
			count += 16;
		}
		return sb.toString();
	}

	private static String makeCommLogLine(byte[] data, int offset, int length) {
		StringBuilder sb = new StringBuilder("  "); // indent 
		// offset
		for (int i = 3; i > 0; i--) {
			int a = (offset >> (i * 8)) & 0xff;
			if ( a > 0) {
				sb.append(toHexStr((byte)a));
			} else {
				sb.append("  ");
			}
		}
		sb.append(toHexStr((byte)(offset & 0xff)));
		sb.append(" ");

		// data 
		for (int i = 0; i < 16; i++) {
			if ( (i > length) || 
				 ((offset + i) >= data.length) ) {
				sb.append("  ");
			} else {
				sb.append(toHexStr(data[offset + i]));
			}
			if (((i + 1) % 4) == 0) { // if wrote 4byte then put white space
				sb.append(" ");
			}
		}
		sb.append("    ");
 
		// char 
		for (int i = 0; i < 16; i++) {
			if ((i > length) || ((offset + i) >= data.length)) {
				break;
			}
			char ch = (char)data[offset + i];
			if (Character.isLetterOrDigit(ch)) {
				sb.append(ch);
				//sb.append(Character.toString(ch));
			} else {
				sb.append(".");
			}
		}
		sb.append("\n");

		return sb.toString();
	}

	private static String toHexStr(byte ch) {
		char [] cbuf = new char[2];
		int c1 = (int)(ch & 0x0F);        // subordinate 4bit
		int c2 = (int)((ch >> 4) & 0x0F); // high rank 4bit
		cbuf[1] = hexTable[c1];
		cbuf[0] = hexTable[c2];
		return new String(cbuf);
	}

	final static char[] hexTable = {
        '0' , '1' , '2' , '3' , '4' , '5' ,
        '6' , '7' , '8' , '9' , 'a' , 'b' ,
        'c' , 'd' , 'e' , 'f'
	};


}

