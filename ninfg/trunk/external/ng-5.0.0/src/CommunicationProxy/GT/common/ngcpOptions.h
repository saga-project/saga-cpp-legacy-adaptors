/*
 * $RCSfile: ngcpOptions.h,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $
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
#ifndef NGCP_OPTIONS_H
#define NGCP_OPTIONS_H

#include "ngcpXIO.h"

#define NGCP_LOGCAT_OPTIONS "Communication Proxy options"

#define NGCP_OPTION_CONTACT_STRING              "GT_contactString"
#define NGCP_OPTION_COMMUNICATION_SECURITY      "GT_communicationSecurity"
#define NGCP_OPTION_CLIENT_RELAY_HOST           "GT_clientRelayHost"
#define NGCP_OPTION_CLIENT_RELAY_INVOKE_METHOD  "GT_clientRelayInvokeMethod"
#define NGCP_OPTION_CLIENT_RELAY_OPTION         "GT_clientRelayOption"
#define NGCP_OPTION_CLIENT_RELAY_CRYPT          "GT_clientRelayCrypt"
#define NGCP_OPTION_CLIENT_RELAY_GSISSH_COMMAND "GT_clientRelayGSISSHcommand"
#define NGCP_OPTION_CLIENT_RELAY_GSISSH_OPTION  "GT_clientRelayGSISSHoption"
#define NGCP_OPTION_REMOTE_RELAY_HOST           "GT_remoteRelayHost"
#define NGCP_OPTION_REMOTE_RELAY_INVOKE_METHOD  "GT_remoteRelayInvokeMethod"
#define NGCP_OPTION_REMOTE_RELAY_OPTION         "GT_remoteRelayOption"
#define NGCP_OPTION_REMOTE_RELAY_CRYPT          "GT_remoteRelayCrypt"
#define NGCP_OPTION_REMOTE_RELAY_GSISSH_COMMAND "GT_remoteRelayGSISSHcommand"
#define NGCP_OPTION_REMOTE_RELAY_GSISSH_OPTION  "GT_remoteRelayGSISSHoption"

typedef enum ngcpRelayInvokeMethod_e {
    NGCP_RELAY_INVOKE_METHOD_MANUAL,
    NGCP_RELAY_INVOKE_METHOD_GSI_SSH
} ngcpRelayInvokeMethod_t;

typedef struct ngcpOptions_s {
    char                        *ngo_contactString;
    ngcpCommunicationSecurity_t  ngo_communicationSecurity;
    char                        *ngo_relayHost;
    bool                         ngo_relayCrypt;
    NGEM_LIST_OF(char)           ngo_relayOptions;
    char                        *ngo_relayGSISSHcommand;
    NGEM_LIST_OF(char)           ngo_relayGSISSHoptions;
    ngcpRelayInvokeMethod_t      ngo_relayInvokeMethod;
} ngcpOptions_t;

ngcpOptions_t *ngcpOptionsCreate(void);
ngcpOptions_t *ngcpOptionsCreateCopy(ngcpOptions_t *);
ngemResult_t ngcpOptionsDestroy(ngcpOptions_t *);
bool ngcpOptionsEqualForRelayInvoking(ngcpOptions_t *, ngcpOptions_t *);
void ngcpOptionsDebugPrint(ngcpOptions_t *, char *);

NGEM_DECLARE_OPTION_ANALYZER_SET_ENUM(
    ngcpOptionAnalyzerSetRelayInvokeMethod, ngcpRelayInvokeMethod_t);

#endif/*NGCP_OPTIONS_H*/
