/*
 * $RCSfile: NgCommunicationProxy.java,v $ $Revision: 1.8 $ $Date: 2008/02/07 08:17:43 $
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

import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.io.IOException;

/*
 * Representation of Client Communication Proxy
 */
class CommunicationProxy {

	private ProcessCommunicator process = null;
	private String type = null;
	private int listen_port;
	private int timeout;
	private List<String> option; // Communication Proxy option
	private NgLog logger = null;

	// Client Communication Proxy attribute for Remote Communication Proxy
	private List<String> attribute; 

	/*
	 * Constructor
	 */
	private CommunicationProxy() {

	}

	/*
	 * Create method
	 * 
	 * @param log logging class
	 * @param port Ninf-G Client listen port
	 * @param options Client Communication Proxy Options
	 *        (List or other class?)
	 */
	public static CommunicationProxy newInstance(NgLog log, int port,
	 List<String> options){
		// - parse? options get timeout
		//   - set default timeout if timeout doesn't specified 
		// - create command line
		// - new CommunicationProxy
		// - set Timer
		// - call initialize()
		// - check timer
		//   - throw Exception if timeout has come
		// - return CommunicationProxy
		return null;
	}

	/**
	 * Request QUERY_FEATURES
	 * 
	 * @return Strings("name val") of reply. User should parse string.
	 */
	public List<String> queryFeatures() throws NgException {
		try  {
			ExtModuleRequest req = ExtModuleRequest.QUERY_FEATURES;
			logDebugInternal("Request is " + req);
			process.request( req );
			logInfoInternal("Send QUERY_FEATURES request to Communication Proxy");

			ExtModuleReply rep = process.reply();
			logInfoInternal("Receive QUERY_FEATURES reply from Communication Proxy");
			logDebugInternal("Reply is " + rep);
			if ( rep.result().equals("SM") ) {
				throw new NgException(req + " error. returned " + rep);
			}
			return rep.returnValues();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * Request EXIT
	*/
	public void exit() {
		ExtModuleRequest req = ExtModuleRequest.EXIT;
		logDebugInternal("Request is " + req);
		try {
			process.request(req);
			logInfoInternal("Send EXIT request to Communication Proxy");
			ExtModuleReply rep = process.reply();
			logInfoInternal("Receive EXIT reply from Communication Proxy");
			logDebugInternal("Reply is " + rep);
		} catch (IOException e) {
			logErrorInternal(e.getMessage());
		}
		process.exit();
	}


	/*
	 * Request INITIALIZE
	 * 
	 */
	public void initialize() throws NgException {

		ExtModuleRequest req =
			ExtModuleRequest.issuesMultiple("INITIALIZE");
		req.addAttribute("listen_port", String.valueOf(listen_port));
		for (String opt : option) {
			req.addAttribute(opt);
		}
		logDebugInternal("Request is " + req);

		try {
			process.request(req);
			logInfoInternal("Send EXIT request to Communication Proxy");

			ExtModuleReply rep = process.reply();
			logInfoInternal("Receive EXIT reply from Communication Proxy");
			logDebugInternal("Reply is " + rep);

			 this.attribute = rep.returnValues();
		} catch (IOException e) {
			throw new NgException(e.getMessage());
		}
	}


    private void logInfoInternal(String msg) {
        if (logger != null)
            logger.logInfo(NgLog.CAT_NG_INTERNAL, "InformationService#" + msg);
    }

    private void logDebugInternal(String msg) {
        if (logger != null)
            logger.logDebug(NgLog.CAT_NG_INTERNAL, "InformationService#" + msg);
    }

    private void logWarnInternal(String msg) {
        if (logger != null)
            logger.logWarn(NgLog.CAT_NG_INTERNAL, "InformationService#" + msg);
    }

    private void logErrorInternal(String msg) {
        if (logger != null)
            logger.logError(NgLog.CAT_NG_INTERNAL, "InformationService#" + msg); 
    }


}
