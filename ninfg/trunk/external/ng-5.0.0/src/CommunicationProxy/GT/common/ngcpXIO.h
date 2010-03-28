/* 
 * $RCSfile: ngcpXIO.h,v $ $Revision: 1.9 $ $Date: 2008/03/28 03:52:30 $
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

#ifndef NGCP_XIO_H
#define NGCP_XIO_H

#define NGI_NG5_DO_NOT_DEFINE_GLOBUS_REMOVE_LATER 1

#include "globus_common.h"
#include "globus_xio.h"
#include "globus_xio_tcp_driver.h"
#include "globus_xio_gsi.h"
#include "globus_xio_file_driver.h"

#include "ngemType.h"
#include "ngemProtocol.h"
#include "ngUtility.h"
#include "ngcpUtility.h"

#define NGCP_LOGCAT_GT "Communication Proxy GT"

#define NGCP_OPTION_COMMUNICATION_SECURITY "GT_communicationSecurity"
#define NGCP_OPTION_CONTACT_STRING         "GT_contactString"

/**
 * Range of port number
 */
typedef struct ngcpPortRange_s {
    unsigned short ngpr_min;
    unsigned short ngpr_max;
} ngcpPortRange_t;

/**
 * Communication Security Level
 */
typedef enum ngcpCommunicationSecurity_s {
    NGCP_COMMUNICATION_SECURITY_NONE,
    NGCP_COMMUNICATION_SECURITY_IDENTITY,
    NGCP_COMMUNICATION_SECURITY_INTEGRITY,
    NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY
} ngcpCommunicationSecurity_t;

/**
 * Auth mode
 */
typedef enum ngcpCommunicationAuth_s {
    NGCP_COMMUNICATION_AUTH_NONE         = 0,
    NGCP_COMMUNICATION_AUTH_SELF         = 0x1,
    NGCP_COMMUNICATION_AUTH_HOST         = 0x2,
} ngcpCommunicationAuth_t;

/* For port range */
#define NGCP_PORT_RANGE_SET(portRange, min, max) \
    do { \
        (portRange)->ngpr_min = (min); \
        (portRange)->ngpr_max = (max); \
    } while (0) 

ngemResult_t ngcpOptionsAnalyzerSetPortRange(ngemOptionAnalyzer_t *, ngcpPortRange_t *, char *, char *);

ngemResult_t ngcpGlobusXIOinitialize(void);
ngemResult_t ngcpGlobusXIOfinalize(void);
ngemResult_t ngcpGlobusXIOfileOpen(globus_xio_handle_t *, const char *);
ngemResult_t ngcpGlobusXIOconnect(
    globus_xio_handle_t *, const char *, ngcpCommunicationSecurity_t, unsigned int, bool);
ngemResult_t ngcpGlobusXIOcreateListener(globus_xio_server_t *, ngcpPortRange_t, ngcpCommunicationSecurity_t, bool);
ngemResult_t ngcpGlobusXIOcheckPeerNameByGridmap(globus_xio_handle_t, bool *, uid_t *uid, gid_t *);
ngemResult_t ngcpGlobusXIOcheckPeerName(globus_xio_handle_t, unsigned int, bool *);
ngemResult_t ngcpGlobusXIOpeerNameIsSelf(globus_xio_handle_t, bool *);

NGEM_DECLARE_OPTION_ANALYZER_SET_ENUM(
    ngcpOptionAnalyzerSetCommunicationSecurity, ngcpCommunicationSecurity_t);

void ngcpLogGlobusError(ngLog_t *log, const char *, const char *, const char *, globus_result_t);
void ngcpLogGSSerror(ngLog_t *log, const char *, const char *, const char *, OM_uint32, OM_uint32);

typedef struct ngcpXIOstandardIO_s {
    globus_xio_handle_t ngsio_in;
    globus_xio_handle_t ngsio_out;
    globus_xio_handle_t ngsio_err;
} ngcpXIOstandardIO_t;

pid_t ngcpXIOpopenArgv(ngcpXIOstandardIO_t *, char **, uid_t, gid_t, ngemEnvironment_t *);

typedef struct ngcpGSISSHinfo_s {
    ngcpXIOstandardIO_t nggs_sio;
    pid_t               nggs_pid;
    char               *nggs_address;
    ngcpCommonLock_t   *nggs_lock;
    int                 nggs_callbackCount;
} ngcpGSISSHinfo_t;

ngcpGSISSHinfo_t *ngcpGSISSHinvoke(char *, char *, NGEM_LIST_OF(char) *, char *,
    NGEM_LIST_OF(char) *, ngcpCommunicationSecurity_t, ngcpCommonLock_t *);
void ngcpGSISSHinfoDestroy(ngcpGSISSHinfo_t *);

ngemResult_t ngcpGlobusXIOinitDelegation(globus_xio_handle_t);
globus_result_t ngcpGlobusXIOregisterInitDelegation(globus_xio_handle_t,
    globus_xio_gsi_delegation_init_callback_t,  void *);  
globus_result_t ngcpGlobusXIOacceptDelegation(globus_xio_handle_t, gss_cred_id_t *);
globus_result_t ngcpGlobusXIOregisterAcceptDelegation(
    globus_xio_handle_t, globus_xio_gsi_delegation_accept_callback_t,  void *); 

ngemResult_t ngcpGlobusXIOpeerIsSelf(globus_xio_handle_t, bool *);

ngemResult_t ngcpGSSexportCred(gss_cred_id_t, char **, uid_t, gid_t);

bool ngcpCredentialAvailable(void);

extern const char *ngcpCommunicationSecurityString[4];

#endif /* NGCP_XIO_H */
