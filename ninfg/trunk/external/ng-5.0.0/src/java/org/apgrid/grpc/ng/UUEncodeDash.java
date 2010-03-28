/*
 * $RCSfile: UUEncodeDash.java,v $ $Revision: 1.3 $ $Date: 2008/02/15 12:09:41 $
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

public class UUEncodeDash {
	
	public static String uuEncodeDash(String input) {
		String t = "";
		for ( int i=0; i< input.length(); i++) {
			String x = input.substring(i,i+1);
			t = t + f(x);
		}
		return t;
	}
	private static String f(String x){
		int i =x.charAt(0);
		if ( 'a' <= i && i <= 'z' ) return x;
		if ( 'A' <= i && i <= 'Z' ) return x;
		if ( '0' <= i && i <= '9' ) return x;
		if ( '.' == i ) return x;
		if ( '-' == i ) return x;
		if ( '_' == i ) return x;
		if ( ' ' == i ) return "+";
		return "%" + Character.toUpperCase(Integer.toHexString(i).charAt(0)) + Character.toUpperCase(Integer.toHexString(i).charAt(1));
	}
}
