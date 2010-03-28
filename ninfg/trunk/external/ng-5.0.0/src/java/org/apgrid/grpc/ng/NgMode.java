/*
 * $RCSfile: NgMode.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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

public abstract class NgMode {

	public static final NgMode NONE  = new NgMode(0, "none")  {};
	public static final NgMode IN    = new NgMode(1, "in")    {};
	public static final NgMode OUT   = new NgMode(2, "out")   {};
	public static final NgMode INOUT = new NgMode(3, "inout") {};
	public static final NgMode WORK  = new NgMode(4, "work")  {};

	private static final NgMode [] modes = {NONE, IN, OUT, INOUT, WORK}; 
	private int value;
	private String name;

	NgMode(int value, String name) {
		this.value = value;
		this.name =name;
	}

	public String toString() { return name; }
	public int value() { return value; }

	public static NgMode get(String name) {
		for (int i = 0; i < modes.length; i++) {
			if (name.equals(modes[i].toString())) {
				return modes[i];
			}
		}
		throw new IllegalArgumentException();
	}

	public static NgMode get(int index) {
		if (index < 0 || index >= modes.length) {
			throw new IllegalArgumentException();
		}
		return modes[index];
	}
}

