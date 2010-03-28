/*
 * $RCSfile: EntryChecker.java,v $ $Revision: 1.4 $ $Date: 2008/02/07 08:17:43 $
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
package org.apgrid.grpc.ng.config;

import org.apgrid.grpc.ng.NgConfigEntry;
import org.apgrid.grpc.ng.NgConfigException;

class EntryChecker {

	private EntryChecker() {
	}

	public static void checkInteger(NgConfigEntry anEnt)
	 throws NgConfigException {
		try {
			Integer.parseInt(anEnt.getValue());
		}  catch (NumberFormatException e) {
			throw new NgConfigException("Invalid value " + anEnt);
		}
	}

	public static void checkNegativeNumber(NgConfigEntry anEnt)
	 throws NgConfigException {
		try {
			Integer num = Integer.parseInt(anEnt.getValue());
			if (num < 0) {
				throw new NgConfigException(anEnt.getKey()
										+ " was specified negative value "
										+ num);
			}
		} catch (NumberFormatException e) {
			throw new NgConfigException("Invalid value " + anEnt);
		}
	}

	public static void checkTypeBoolean(NgConfigEntry anEnt) 
	 throws NgConfigException {
		String value = anEnt.getValue();
		if (value.equalsIgnoreCase("true") ||
			value.equalsIgnoreCase("false")) {
			return;
		}
		throw new NgConfigException(anEnt.getKey() + " is not Boolean");
	}

}

