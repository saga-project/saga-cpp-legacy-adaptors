/*
 * $RCSfile: ngemLog.c,v $ $Revision: 1.9 $ $Date: 2008/02/25 05:21:46 $
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

#include "ngemLog.h"
#include "ngemUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngemLog.c,v $ $Revision: 1.9 $ $Date: 2008/02/25 05:21:46 $")

static ngLog_t *ngemlLog = NULL;

ngemResult_t
ngemLogInitialize(
    char *appname,
    char *filename)
{
    int result;
    ngLogConstructArgument_t logCarg;
    bool infoInitialized = false;
    ngemResult_t ret = NGEM_FAILED;
    NGEM_FNAME_TAG(ngemLogInitialize);

    ngLogSetDefaultNull();

    if (!NGEM_STRING_IS_NONZERO(filename)) {
        return NGEM_SUCCESS;
    }
    if (!NGEM_STRING_IS_NONZERO(appname)) {
        goto finalize;
    }

    result = ngLogConstructArgumentInitialize(&logCarg, NULL, NULL);
    if (result == 0) {
        goto finalize;
    }
    infoInitialized = true;

    logCarg.nglca_output        = NG_LOG_FILE;
    logCarg.nglca_filePath      = ngiStrdup(filename, NULL, NULL);
    logCarg.nglca_suffix        = NULL;
    logCarg.nglca_nFiles        = 1;
    logCarg.nglca_maxFileSize   = 0;
    logCarg.nglca_overWriteDir  = 1;/* True */
    logCarg.nglca_appending     = 1;/* True */
    logCarg.nglca_categories    = NULL;

    if (logCarg.nglca_filePath == NULL) {
        goto finalize;
    }

    ngemlLog = ngLogConstruct(appname, &logCarg, NULL, NULL);
    if (ngemlLog == NULL) {
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:
    if (infoInitialized) {
        result = ngLogConstructArgumentFinalize(&logCarg, NULL, NULL);
        if (result == 0) {
            ret = NGEM_FAILED;
        }
        infoInitialized = false;
    }

    return ret;
}

ngLog_t *
ngemLogGetDefault(void)
{
    NGEM_FNAME_TAG(ngemLogGetDefault);

    return ngemlLog;
}

ngemResult_t
ngemLogFinalize(void)
{
    int result;
    NGEM_FNAME_TAG(ngemLogFinalize);

    if (ngemlLog != NULL) {
        result = ngLogDestruct(ngemlLog, NULL, NULL);
        if (result == 0) {
            return NGEM_FAILED;
        }
    }

    return NGEM_SUCCESS;
}

