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
 * $RCSfile: NgGrpcJobOutputListener.java,v $ $Revision: 1.5 $ $Date: 2006/08/22 10:54:32 $
 */
package org.apgrid.grpc.ng;

import org.globus.io.gass.server.JobOutputListener;

class NgGrpcJobOutputListener implements JobOutputListener {
	private StringBuffer outputBuf = null;

	/* (non-Javadoc)
	 * @see org.globus.io.gass.server.JobOutputListener#outputChanged(java.lang.String)
	 */
	public synchronized void outputChanged(String output) {
		if (outputBuf == null) {
			outputBuf = new StringBuffer();
		}
		outputBuf.append(output);
	}

	/* (non-Javadoc)
	 * @see org.globus.io.gass.server.JobOutputListener#outputClosed()
	 */
	public void outputClosed() {
		/* nothing */
	}

	/**
	 * @return
	 */
	protected String getOutput() {
		if (outputBuf == null) {
			return null;
		} else {
			return outputBuf.toString();
		}
	}

	/**
	 * @return
	 */
	protected boolean hasData() {
		return outputBuf != null;
	}
}
