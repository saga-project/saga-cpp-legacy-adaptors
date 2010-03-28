/* 
 * $RCSfile: ngrcGT.h,v $ $Revision: 1.5 $ $Date: 2008/03/28 03:52:30 $
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
#ifndef NGRC_GT_H_
#define NGRC_GT_H_

#include "ngemType.h"
#include "ngrcProtocol.h"
#include "ngcpXIO.h"
#include "ngcpOptions.h"
#include "ngcpRelayHandler.h"

#define NGRC_LOGCAT_GT "Remote Communication Proxy GT"
#define NGRC_BUFFER_SIZE (32 * 1024)

#define NGRC_OPTION_REMOTE_RELAY "GT_remoteRelay"

/**
 * Socket(Cancelable)
 */
typedef struct ngrcSocket_s {
    int ngs_fd;
    int ngs_pipe[2];
} ngrcSocket_t;

/* Socket */
ngrcSocket_t *ngrcSocketCreateListener(bool);
char *ngrcSocketGetContactString(ngrcSocket_t *);
ngrcSocket_t *ngrcSocketAccept(ngrcSocket_t *, bool *);
ngrcSocket_t *ngrcSocketDup(ngrcSocket_t *);
ngemResult_t ngrcSocketRead(ngrcSocket_t *, void *, size_t, size_t *);
ngemResult_t ngrcSocketWrite(ngrcSocket_t *, void *, size_t, size_t *);
ngemResult_t ngrcSocketCancel(ngrcSocket_t *);
ngemResult_t ngrcSocketDestroy(ngrcSocket_t *);

/**
 * I/O operator for handle of Globus XIO 
 *
 * This is for canceling reading or writing.
 * When ngrclOperatorCancel() is called, 
 * running operation and operation that will run are canceled.
 */
typedef struct ngrcOperator_s {
    globus_xio_handle_t  ngo_handle;
    globus_mutex_t       ngo_mutex;
    globus_cond_t        ngo_cond;
    bool                 ngo_canceled;
} ngrcOperator_t;

typedef struct ngrcRelayHandler_s {
    ngcpCommonLock_t    ngrh_lock;
    ngcpRelayHandler_t *ngrh_relayHandler;
    char               *ngrh_address;
} ngrcRelayHandler_t;

ngemResult_t ngrcOperatorInitialize(ngrcOperator_t *, globus_xio_handle_t);
ngemResult_t ngrcOperatorFinalize(ngrcOperator_t*);

ngemResult_t ngrcOperatorRead(ngrcOperator_t *, void *, size_t, size_t *, bool *);
ngemResult_t ngrcOperatorWrite(ngrcOperator_t *, void *, size_t, size_t *, bool *);
ngemResult_t ngrcOperatorCancel(ngrcOperator_t *);

ngrcRelayHandler_t *ngrcRelayHandlerCreate(ngcpOptions_t *, ngrcInitializeOptions_t *, char *, bool);
void ngrcRelayHandlerDestroy(ngrcRelayHandler_t *);

#endif /* NGRC_GT_H_ */
