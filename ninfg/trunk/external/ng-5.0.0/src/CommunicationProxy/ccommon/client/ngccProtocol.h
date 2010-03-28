/* 
 * $RCSfile: ngccProtocol.h,v $ $Revision: 1.9 $ $Date: 2008/02/25 10:17:27 $
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
#ifndef NGCC_PROTOCOL_H_
#define NGCC_PROTOCOL_H_

#include "ngemProtocol.h"
#include "../common/ngcpVersion.h"

#define NGCC_LOGCAT_PROTOCOL "Client Communication Proxy Protocol"

typedef struct ngccProtocol_s                    ngccProtocol_t;
typedef struct ngccInitializeOptions_s           ngccInitializeOptions_t;
typedef struct ngccPrepareCommunicationOptions_s ngccPrepareCommunicationOptions_t;

/**
 * Type of handler called on receiving INITIALIZE request
 */
typedef ngemResult_t (*ngccInitializeBeginCallback_t)(
    ngccProtocol_t *, ngemOptionAnalyzer_t *);

/**
 * Type of handler called after receiving INITIALIZE request
 */
typedef ngemResult_t (*ngccInitializeEndCallback_t)(
    ngccProtocol_t *, ngccInitializeOptions_t *, ngemReply_t *, ngemResult_t);

/**
 * Type of handler called on receiving PREPARE_COMMUNICATION request
 */
typedef ngemResult_t (*ngccPrepareCommunicationBeginCallback_t)(
    ngccProtocol_t *, ngccPrepareCommunicationOptions_t *, ngemOptionAnalyzer_t *);

/**
 * Type of handler called on receiving PREPARE_COMMUNICATION request
 */
typedef ngemResult_t (*ngccPrepareCommunicationEndCallback_t)(
    ngccProtocol_t *, ngccPrepareCommunicationOptions_t*, ngemReply_t *, ngemResult_t);

/**
 * Protocol Information
 */
typedef struct ngccProtocolInformation_s {
    ngccInitializeBeginCallback_t           ngpi_initializeBegin;
    ngccInitializeEndCallback_t             ngpi_initializeEnd;
    ngccPrepareCommunicationBeginCallback_t ngpi_prepareCommunicationBegin;
    ngccPrepareCommunicationEndCallback_t   ngpi_prepareCommunicationEnd;
    bool                                    ngpi_enableRelay;
} ngccProtocolInformation_t;

/**
 * INITIALIZE request's options
 */
struct ngccInitializeOptions_s {
    unsigned short ngio_clientPort;
    int            ngio_bufferSize;
};

/**
 * PREPARE_COMMUNICATION request's options and information 
 * required for sending COMMUNICATION_REPLY notify
 */
struct ngccPrepareCommunicationOptions_s {
    int           ngpo_requestID;
    ngemNotify_t *ngpo_notify;
    ngemResult_t  ngpo_result;
    char         *ngpo_message;
    void         *ngpo_userData;
    bool          ngpo_tcpNodelay;
    bool          ngpo_retryInfoInitialized;
    ngiConnectRetryInformation_t ngpo_retryInfo;
};

NGEM_DECLARE_LIST_OF(ngccPrepareCommunicationOptions_t);

/**
 * Client Communication Proxy Protocol Talker
 */
struct ngccProtocol_s {
    ngemProtocol_t                                 *ngp_protocol;
    ngccProtocolInformation_t                      *ngp_info;
    ngccInitializeOptions_t                        *ngp_initOpts;
    bool                                            ngp_initialized;
    ngccPrepareCommunicationOptions_t              *ngp_prepareCommOpts;
    NGEM_LIST_OF(ngccPrepareCommunicationOptions_t) ngp_prepareCommOptsList;
    void                                           *ngp_userData;
};

ngccProtocol_t *ngccProtocolCreate(ngccProtocolInformation_t*, void *userData);
ngemResult_t    ngccProtocolDestroy(ngccProtocol_t *);
ngemResult_t    ngccProtocolSendCommunicationReplyNotify(ngccProtocol_t *, ngccPrepareCommunicationOptions_t *);
ngemResult_t    ngccPrepareCommunicationOptionsSetError(ngccPrepareCommunicationOptions_t *, const char *);
ngemNotify_t *ngccPrepareCommunicationOptionGetNotify(ngccPrepareCommunicationOptions_t *);

#define NGCC_PREPARE_COMMUNICATION_OPTION_USERDATA(pCommOpts)  ((pCommOpts)->ngpo_userData)
#define NGCC_PREPARE_COMMUNICATION_OPTION_NOTIFY(pCommOpts)    ((pCommOpts)->ngpo_notify)

#endif /* NGCC_PROTOCOL_H_ */
