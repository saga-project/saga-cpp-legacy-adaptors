/* 
 * $RCSfile: nginProtocol.h,v $ $Revision: 1.4 $ $Date: 2007/11/27 02:27:40 $
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
#ifndef NGIN_PROTOCOL_H_
#define NGIN_PROTOCOL_H_

#include "ngemProtocol.h"

#define NGIN_LOGCAT_PROTOCOL "Information Service Protocol"

#define NGIN_PROTOCOL_VERSION_MAJOR 1
#define NGIN_PROTOCOL_VERSION_MINOR 0

typedef struct nginProtocol_s        nginProtocol_t;
typedef struct nginQueryREIoptions_s nginQueryREIoptions_t;

/* Return identify. Failed, return negative value */
typedef int (*nginQueryBeginCallback_t)(nginProtocol_t *, ngemOptionAnalyzer_t *);
typedef ngemResult_t (*nginQueryEndCallback_t)(nginProtocol_t *, int, nginQueryREIoptions_t *, ngemResult_t);
typedef ngemResult_t (*nginCancelQueryCallback_t)(nginProtocol_t *, int);

/**
 * Protocol Callbacks
 */
typedef struct nginProtocolActions_s {
    nginQueryBeginCallback_t  ngpa_queryBegin;
    nginQueryEndCallback_t    ngpa_queryEnd;
    nginCancelQueryCallback_t ngpa_cancelQuery;
} nginProtocolActions_t;

struct nginQueryREIoptions_s {
    char              *ngqo_hostname;
    char              *ngqo_classname;
    NGEM_LIST_OF(char) ngqo_sources;
};

struct nginProtocol_s {
    ngemProtocol_t        *ngp_protocol;
    nginProtocolActions_t *ngp_actions;
    int                    ngp_currentQueryIdentify;
    nginQueryREIoptions_t *ngp_options;
    void                  *ngp_userData;
};

nginProtocol_t *nginProtocolCreate(nginProtocolActions_t *, void *);
ngemResult_t nginProtocolDestroy(nginProtocol_t *);
ngemResult_t nginProtocolSendInformationNotify(nginProtocol_t *, int, const char *);
ngemResult_t nginProtocolSendInformationNotifyFailed(nginProtocol_t *, int, const char *);

#endif /* NGIN_PROTOCOL_H_ */
