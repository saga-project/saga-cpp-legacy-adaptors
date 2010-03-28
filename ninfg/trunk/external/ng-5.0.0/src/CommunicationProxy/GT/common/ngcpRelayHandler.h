/* 
 * $RCSfile: ngcpRelayHandler.h,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $
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

#ifndef NGCP_RELAY_HANDLER_H_
#define NGCP_RELAY_HANDLER_H_

#include "ngcpXIO.h"
#include "ngcpOptions.h"
#include "ngrGT.h"
#include "ngcpUtility.h"

#define NGCP_LOGCAT_RELAY_HANDLER "Relay Handler"
#define NGCP_RELAY_HANDLER_BUFFER_SIZE (32*1024)

typedef struct ngcpRequest_s      ngcpRequest_t;
typedef struct ngcpNotify_s       ngcpNotify_t;
typedef struct ngcpNotifyInfo_s   ngcpNotifyInfo_t;
typedef struct ngcpRelayHandler_s ngcpRelayHandler_t;
typedef struct ngcpLineParser_s   ngcpLineParser_t;
typedef void (*ngcpNotifyHandler_t)(ngcpRelayHandler_t *, ngcpNotify_t *, void *);

NGEM_DECLARE_LIST_OF(ngcpNotifyInfo_t);

struct ngcpRequest_s {
    /* Request part */
    char              *ngr_name;
    char              *ngr_params;
    NGEM_LIST_OF(char) ngr_arguments;
    bool               ngr_multiLine;
    bool               ngr_multiLineReply;

    /* Reply part */
    bool               ngr_setReply;
    bool               ngr_completedReply;
    ngemResult_t       ngr_result;
    char              *ngr_replyParams;
    char              *ngr_replyMessage;
    NGEM_LIST_OF(char) ngr_replyArguments;
};

struct ngcpNotify_s {
    ngcpNotifyInfo_t  *ngn_info;
    char              *ngn_params;
    NGEM_LIST_OF(char) ngn_arguments;
};

struct ngcpNotifyInfo_s {
    char                *ngni_name;
    bool                 ngni_multiLine;
    ngcpNotifyHandler_t  ngni_handler;
    void                *ngni_userData;
};

struct ngcpLineParser_s {
    char   *nglp_buffer;
    size_t  nglp_head;
    size_t  nglp_tail;
    size_t  nglp_capacity;
    bool    nglp_eof;
};

struct ngcpRelayHandler_s {
    bool                            ngrh_crypt;
    globus_xio_handle_t             ngrh_handle;
    ngcpLineParser_t               *ngrh_lpReply;
    ngcpLineParser_t               *ngrh_lpNotify;
    NGEM_LIST_OF(ngcpNotifyInfo_t)  ngrh_notifyInfos;
    bool                            ngrh_written;
    ngcpRequest_t                  *ngrh_sentRequest;
    bool                            ngrh_replyReceived;
    ngcpNotify_t                   *ngrh_receivedNotify;
    bool                            ngrh_notifyReceived;
    int32_t                         ngrh_header[NGR_PACK_HEADER_NELEMENTS];
    unsigned char                   ngrh_buffer[NGCP_RELAY_HANDLER_BUFFER_SIZE];
    int32_t                         ngrh_rest;
    int32_t                         ngrh_callbackType;
    ngcpCommonLock_t               *ngrh_lock;
    bool                            ngrh_available;
    ngcpGSISSHinfo_t               *ngrh_gsisshInfo;
};

/* Request */
ngcpRequest_t *ngcpRequestCreate(const char *, bool, bool);
void ngcpRequestDestroy(ngcpRequest_t *);
ngemResult_t ngcpRequestAppendArgument(ngcpRequest_t *, const char *, ...) NG_ATTRIBUTE_PRINTF(2, 3);

/* Relay Handler */
ngcpRelayHandler_t *ngcpRelayHandlerCreate(ngcpOptions_t *, char *, ngcpCommonLock_t *);
ngemResult_t ngcpRelayHandlerSendQueryFeatures(ngcpRelayHandler_t *, char **, char **);
ngemResult_t ngcpRelayHandlerSendExit(ngcpRelayHandler_t *);
void ngcpRelayHandlerDestroy(ngcpRelayHandler_t *);
ngemResult_t ngcpRelayHandlerSendRequest(ngcpRelayHandler_t *, ngcpRequest_t *);
ngemResult_t ngcpRelayHandlerRegisterNotify(
    ngcpRelayHandler_t *, const char *, bool, ngcpNotifyHandler_t, void *);

#endif/*NGCP_RELAY_HANDLER_H_*/
