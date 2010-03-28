/*
 * $RCSfile: TimeInfo.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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

class TimeInfo {
	private double real;
	private double cpu;

	public TimeInfo(double r, double c) {
		this.real = r;
		this.cpu = c;
	}
	public synchronized void setReal(double r) { real = r; }
	public synchronized void setCpu(double c) { cpu = c; }

	public synchronized double getReal() { return this.real; }
	public synchronized double getCpu()  { return this.cpu; }

	public synchronized void add(TimeInfo time) {
		real += time.getReal();
		cpu  += time.getCpu();
	}

	public String toString() {
		return "real " + this.real + "\n" + "cpu  " + this.cpu;
	}
}

