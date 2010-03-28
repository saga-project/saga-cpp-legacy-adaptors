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
 * $RCSfile: NaregiSSCredential.java,v $ $Revision: 1.4 $ $Date: 2008/03/28 06:36:51 $
 */
package org.apgrid.grpc.tools.invokeServer.naregiSS;

import org.apgrid.grpc.tools.invokeServer.*;
import java.lang.System;
import org.globus.gsi.GlobusCredential;
import org.globus.gsi.GlobusCredentialException;
import org.globus.gsi.gssapi.GlobusGSSCredentialImpl;
import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSException;

class NaregiSSCredential {
    private static GSSCredential credential    = null;

    static synchronized GSSCredential getCredential()
        throws GlobusCredentialException, GSSException {
        String           proxyName;
        GlobusCredential gcred;
        GSSCredential    cred;

        if (NaregiSSCredential.credential == null) {
            proxyName = System.getProperty("org.apgrid.grpc.X509UserProxy");
            if ((proxyName != null) && (proxyName.length() > 0)) {
                Log.log(Log.ALWAYS, "X509_USER_PROXY is \"" + proxyName + "\"");
                gcred = new GlobusCredential(proxyName);
            } else {
                gcred = GlobusCredential.getDefaultCredential();
            }
            cred =  new GlobusGSSCredentialImpl(gcred,
                        GSSCredential.INITIATE_AND_ACCEPT);

            /* Successful */
            NaregiSSCredential.credential = cred;
        }
        return NaregiSSCredential.credential;
    }
}
