/* 
 * $RCSfile: ngrcProtocol.h,v $ $Revision: 1.6 $ $Date: 2008/02/25 10:17:27 $
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
#ifndef NGRC_PROTOCOL_H_
#define NGRC_PROTOCOL_H_

#include "ngemProtocol.h"
#include "../common/ngcpVersion.h"

#define NGRC_LOGCAT_PROTOCOL "Remote Communication Proxy Protocol"

typedef struct ngrcProtocol_s          ngrcProtocol_t;
typedef struct ngrcInitializeOptions_s ngrcInitializeOptions_t;

/* Return identify. Failed, return negative value */
typedef ngemResult_t (*ngrcInitializeBeginCallback_t)(
    ngrcProtocol_t *, ngemOptionAnalyzer_t *);
typedef ngemResult_t (*ngrcInitializeEndCallback_t)(
    ngrcProtocol_t *, ngrcInitializeOptions_t *, char **, ngemResult_t);

/**
 * Protocol Callbacks
 */
typedef struct ngrcProtocolActions_s {
    ngrcInitializeBeginCallback_t  ngpa_initializeBegin;
    ngrcInitializeEndCallback_t    ngpa_initializeEnd;
} ngrcProtocolActions_t;

struct ngrcInitializeOptions_s {
    char                        *ngio_clientHostname;
    int                          ngio_bufferSize;
    bool                         ngio_tcpNodelay;
    bool                         ngio_retryInfoInitialized;
    ngiConnectRetryInformation_t ngio_retryInfo;
};

struct ngrcProtocol_s {
    ngemProtocol_t          *ngp_protocol;
    ngrcProtocolActions_t   *ngp_actions;
    ngrcInitializeOptions_t *ngp_options;
    bool                     ngp_initialized;
    void                    *ngp_userData;
};

ngrcProtocol_t *ngrcProtocolCreate(ngrcProtocolActions_t *, void *);
ngemResult_t    ngrcProtocolDestroy(ngrcProtocol_t *);

#endif /* NGRC_PROTOCOL_H_ */
