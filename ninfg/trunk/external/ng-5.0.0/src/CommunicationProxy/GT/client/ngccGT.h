/*
 * $RCSfile: ngccGT.h,v $ $Revision: 1.2 $ $Date: 2008/02/25 10:17:26 $
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
#ifndef NGCC_GT_H_
#define NGCC_GT_H_

#include "ngccProtocol.h"
#include "ngcpOptions.h"

#define NGCC_LOGCAT_GT        "Client Communication Proxy GT"
#define NGCC_APPLICATION_NAME "Client Communication Proxy GT"

#define NGCC_OPTION_PORT_RANGE   "GT_portRange"
#define NGCC_OPTION_CLIENT_RELAY "GT_clientRelay"

/* Types */
typedef struct ngccRelayHandlerManager_s ngccRelayHandlerManager_t;
typedef struct ngccRelayHandler_s        ngccRelayHandler_t;
typedef struct ngccPrepareCommunicationResult_s ngccPrepareCommunicationResult_t;
typedef void (*ngccRelayHandlerConnectRequestCallback_t)(void *, const char *, ngcpCommunicationSecurity_t, bool, bool);

struct ngccPrepareCommunicationResult_s {
    ngemResult_t  ngpcr_result;
    char         *ngpcr_message;
    char         *ngpcr_contactString;
    bool          ngpcr_canceled;
};

/* Relay Handler */
ngccRelayHandlerManager_t *ngccRelayHandlerManagerCreate(ngccRelayHandlerConnectRequestCallback_t, void *);
void ngccRelayHandlerManagerDestroy(ngccRelayHandlerManager_t *);
ngccRelayHandler_t *ngccRelayHandlerManagerGet(ngccRelayHandlerManager_t *, ngcpOptions_t *);

ngemResult_t ngccRelayHandlerPrepareCommunication(ngccRelayHandler_t *,
    ngccPrepareCommunicationOptions_t *, ngccPrepareCommunicationResult_t *);
ngemResult_t ngccRelayHandlerUnref(ngccRelayHandler_t *);

#endif/*NGCC_GT_H_*/
