/* 
 * $RCSfile: nginReadNRF.h,v $ $Revision: 1.1 $ $Date: 2007/05/11 08:27:59 $
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

#ifndef NGIN_READ_NRF_H_
#define NGIN_READ_NRF_H_

#include "ngemUtility.h"
#include "ngemLog.h"
#include "ngemList.h"

#define NGIN_LOGCAT_NRF "Information Service NRF"

typedef struct nginRemoteExecutableInformation_s {
    char *ngrei_hostname;
    char *ngrei_classname;
    char *ngrei_xmlString;
} nginRemoteExecutableInformation_t;

NGEM_DECLARE_LIST_OF(nginRemoteExecutableInformation_t);

typedef struct nginREIcontainer_s {
    NGEM_LIST_OF(nginRemoteExecutableInformation_t) ngrc_reInfo;
    char                                           *ngrc_filename;
} nginREIcontainer_t;

/* Return a NULL terminate array.*/
nginREIcontainer_t *nginReadNRF(const char *);

nginREIcontainer_t *nginREIcontainerCreate(const char *);
ngemResult_t nginREIcontainerDestroy(nginREIcontainer_t *);

nginRemoteExecutableInformation_t * nginREIcontainerFind(
    nginREIcontainer_t *, const char *, const char *);
ngemResult_t nginREIcontainerErase(
    nginREIcontainer_t *, const char *, const char *);
bool nginREIcontainerIsEmpty(nginREIcontainer_t *);

#endif /* NGIN_READ_NRF_H_ */
