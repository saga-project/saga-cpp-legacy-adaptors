/*
 * $RCSfile: ngcpOptions.c,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $
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

#include "ngcpOptions.h"

NGI_RCSID_EMBED("$RCSfile: ngcpOptions.c,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $")

static const char *ngcplRelayInvokeMethodStrings[] = {
    "manual",
    "gsissh"
};

/**
 * PREPARE_COMMUNICATION options: Create
 */
ngcpOptions_t *
ngcpOptionsCreate(void)
{
    ngLog_t *log = NULL;
    ngcpOptions_t *opts = NULL;
    NGEM_FNAME(ngcpOptionsCreate);

    log = ngemLogGetDefault();

    opts = NGI_ALLOCATE(ngcpOptions_t, log, NULL);
    if (opts == NULL) {
        ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
            "Can't allocate storage for options"
            " for Communication Proxy GT.\n");
        return NULL;
    }
    /* Default */
    opts->ngo_contactString         = NULL;
    opts->ngo_communicationSecurity = NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY;
    opts->ngo_relayHost             = NULL;
    opts->ngo_relayInvokeMethod     = NGCP_RELAY_INVOKE_METHOD_MANUAL;
    opts->ngo_relayCrypt            = true;
    opts->ngo_relayGSISSHcommand    = NULL;

    NGEM_LIST_INITIALIZE(char, &opts->ngo_relayOptions);
    NGEM_LIST_INITIALIZE(char, &opts->ngo_relayGSISSHoptions);

    return opts;;
}

/**
 * PREPARE_COMMUNICATION options: Copy
 */
ngcpOptions_t *
ngcpOptionsCreateCopy(
    ngcpOptions_t *src)
{
    char *val = NULL;
    char *copy = NULL;
    NGEM_LIST_ITERATOR_OF(char) it;
    ngLog_t *log = NULL;
    ngcpOptions_t *opts = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngcpOptionsCreateCopy);

    log = ngemLogGetDefault();

    opts = NGI_ALLOCATE(ngcpOptions_t, log, NULL);
    if (opts == NULL) {
        ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
            "Can't allocate storage for PREPARE_COMMUNICATION's options"
            " for Communication Proxy GT.\n");
        goto error;
    }
    /* Default */
    *opts = *src;
    opts->ngo_relayHost = NULL;
    opts->ngo_relayGSISSHcommand = NULL;
    NGEM_LIST_INITIALIZE(char, &opts->ngo_relayOptions);
    NGEM_LIST_INITIALIZE(char, &opts->ngo_relayGSISSHoptions);

    if (src->ngo_contactString != NULL) {
        opts->ngo_contactString = ngiStrdup(src->ngo_contactString, log, NULL);
        if (opts->ngo_contactString == NULL) {
            ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
    }

    if (src->ngo_relayHost != NULL) {
        opts->ngo_relayHost = ngiStrdup(src->ngo_relayHost, log, NULL);
        if (opts->ngo_relayHost == NULL) {
            ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
    }
    if (src->ngo_relayGSISSHcommand != NULL) {
        opts->ngo_relayGSISSHcommand =
            ngiStrdup(src->ngo_relayGSISSHcommand, log, NULL);
        if (opts->ngo_relayGSISSHcommand == NULL) {
            ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
    }


    copy = NULL;
    NGEM_LIST_FOREACH(char, &src->ngo_relayOptions, it, val) {
        copy = ngiStrdup(val, log, NULL);
        if (copy == NULL) {
            ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
        nResult = NGEM_LIST_INSERT_TAIL(char, &opts->ngo_relayOptions, copy);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
                "Can't insert to list.\n");
            goto error;
        }
        copy = NULL;
    }

    copy = NULL;
    NGEM_LIST_FOREACH(char, &src->ngo_relayGSISSHoptions, it, val) {
        copy = ngiStrdup(val, log, NULL);
        if (copy == NULL) {
            ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
        nResult = NGEM_LIST_INSERT_TAIL(char, &opts->ngo_relayGSISSHoptions, copy);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
                "Can't insert to list.\n");
            goto error;
        }
        copy = NULL;
    }

    return opts;
error:
    ngcpOptionsDestroy(opts);
    opts = NULL;
    ngiFree(copy, log, NULL);
    copy = NULL;

    return NULL;
}

/**
 * PREPARE_COMMUNICATION options: Destroy
 */
ngemResult_t
ngcpOptionsDestroy(
    ngcpOptions_t *opts)
{
    ngLog_t *log = NULL;
    int result;
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    NGEM_FNAME(ngcpOptionsDestroy);

    log = ngemLogGetDefault();

    if (opts == NULL) {
        return NGEM_SUCCESS;
    }

    NGEM_LIST_FOREACH(char, &opts->ngo_relayOptions, it, val) {
        ngiFree(val, log, NULL);
    }
    NGEM_LIST_FINALIZE(char, &opts->ngo_relayOptions);

    NGEM_LIST_FOREACH(char, &opts->ngo_relayGSISSHoptions, it, val) {
        ngiFree(val, log, NULL);
    }
    NGEM_LIST_FINALIZE(char, &opts->ngo_relayGSISSHoptions);

    ngiFree(opts->ngo_contactString, log, NULL);
    ngiFree(opts->ngo_relayGSISSHcommand, log, NULL);
    ngiFree(opts->ngo_relayHost, log, NULL);

    opts->ngo_relayInvokeMethod     = NGCP_RELAY_INVOKE_METHOD_MANUAL;
    opts->ngo_relayCrypt            = true;
    opts->ngo_relayHost             = NULL;
    opts->ngo_communicationSecurity = NGCP_COMMUNICATION_SECURITY_NONE;
    opts->ngo_relayGSISSHcommand    = NULL;

    NGEM_LIST_SET_INVALID_VALUE(&opts->ngo_relayOptions);
    NGEM_LIST_SET_INVALID_VALUE(&opts->ngo_relayGSISSHoptions);

    result = NGI_DEALLOCATE(ngcpOptions_t, opts, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_OPTIONS, fName,
            "Can't deallocate storage for PREPARE_COMMUNICATION's options"
            " for Communication Proxy GT.\n");
        return NGEM_FAILED;
    }
    return NGEM_SUCCESS;
}

/**
 * PREPARE_COMMUNICATION options: Equals for relay invoking.
 */
bool
ngcpOptionsEqualForRelayInvoking(
    ngcpOptions_t *rhs,
    ngcpOptions_t *lhs)
{
    ngLog_t *log = NULL;
    NGEM_LIST_ITERATOR_OF(char) it1;
    NGEM_LIST_ITERATOR_OF(char) it2;
    NGEM_LIST_ITERATOR_OF(char) endit1;
    NGEM_LIST_ITERATOR_OF(char) endit2;

    log = ngemLogGetDefault();

    NGEM_ASSERT(lhs != NULL);
    NGEM_ASSERT(rhs != NULL);

    NGEM_ASSERT(lhs->ngo_relayHost != NULL);
    NGEM_ASSERT(rhs->ngo_relayHost != NULL);

    /* ngo_communicationSecurity is ignored */
    if (strcmp(lhs->ngo_relayHost, rhs->ngo_relayHost) != 0) {
        return false;
    }

    if (lhs->ngo_relayInvokeMethod != rhs->ngo_relayInvokeMethod) {
        return false;
    }

    if (lhs->ngo_relayCrypt != rhs->ngo_relayCrypt) {
        return false;
    }


    if (lhs->ngo_relayInvokeMethod == NGCP_RELAY_INVOKE_METHOD_GSI_SSH) {
        if (ngemStringCompare(lhs->ngo_relayGSISSHcommand, -1, rhs->ngo_relayGSISSHcommand, -1) != 0) {
            return false;
        }

        it1    = NGEM_LIST_BEGIN(char, &lhs->ngo_relayOptions);
        it2    = NGEM_LIST_BEGIN(char, &rhs->ngo_relayOptions);
        endit1 = NGEM_LIST_END(char, &lhs->ngo_relayOptions);
        endit2 = NGEM_LIST_END(char, &lhs->ngo_relayOptions);
        while ((it1 != endit1) && (it2 != endit2)) {
            if (strcmp(NGEM_LIST_GET(char, it1), NGEM_LIST_GET(char, it2)) != 0) {
                return false;
            }
            it1 = NGEM_LIST_NEXT(char, it1);
            it2 = NGEM_LIST_NEXT(char, it2);
        }
        if ((it1 != endit1) || (it2 != endit2)) {
            return false;
        }

        it1    = NGEM_LIST_BEGIN(char, &lhs->ngo_relayGSISSHoptions);
        it2    = NGEM_LIST_BEGIN(char, &rhs->ngo_relayGSISSHoptions);
        endit1 = NGEM_LIST_END(char, &lhs->ngo_relayGSISSHoptions);
        endit2 = NGEM_LIST_END(char, &lhs->ngo_relayGSISSHoptions);
        while ((it1 != endit1) && (it2 != endit2)) {
            if (strcmp(NGEM_LIST_GET(char, it1), NGEM_LIST_GET(char, it2)) != 0) {
                return false;
            }
            it1 = NGEM_LIST_NEXT(char, it1);
            it2 = NGEM_LIST_NEXT(char, it2);
        }
        if ((it1 != endit1) || (it2 != endit2)) {
            return false;
        }
    }
    return NGEM_SUCCESS;
}

/**
 * PREPARE_COMMUNICATION options: Debug Print
 */
void
ngcpOptionsDebugPrint(
    ngcpOptions_t *opts,
    char          *func_name)
{
    ngLog_t *log;
    char *val;
    int i;
    NGEM_LIST_ITERATOR_OF(char) it;

    log = ngemLogGetDefault();

#define NGCPL_STRING(str) (((str) != NULL)?(str):("NULL"))

    ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
        "Contact string = \"%s\"\n", NGCPL_STRING(opts->ngo_contactString));
    ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
        "Communication Security = \"%s\"\n", ngcpCommunicationSecurityString[opts->ngo_communicationSecurity]);
    ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
        "Relay host = \"%s\"\n", NGCPL_STRING(opts->ngo_relayHost));
    ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
        "Relay invoke method = \"%s\"\n", ngcplRelayInvokeMethodStrings[opts->ngo_relayInvokeMethod]);
    ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
        "Relay crypt = \"%s\"\n", (opts->ngo_relayCrypt)?("true"):("false"));
    ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
        "Relay GSISSH command = \"%s\"\n", NGCPL_STRING(opts->ngo_relayGSISSHcommand));
#undef NGCPL_STRING

    i = 0;
    NGEM_LIST_FOREACH(char, &opts->ngo_relayOptions, it, val) {
        NGEM_ASSERT(val != NULL);
        ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
            "Relay option[%d] = \"%s\"\n", i, val);
        i++;
    }

    i = 0;
    NGEM_LIST_FOREACH(char, &opts->ngo_relayGSISSHoptions, it, val) {
        NGEM_ASSERT(val != NULL);
        ngLogDebug(log, NGCP_LOGCAT_OPTIONS, func_name,
        "Relay GSISSH option[%d] = \"%s\"\n", i, val);
        i++;
    }

    return;
}

NGEM_DEFINE_OPTION_ANALYZER_SET_ENUM(
    ngcpOptionAnalyzerSetRelayInvokeMethod,
    ngcpRelayInvokeMethod_t,
    ngcplRelayInvokeMethodStrings,
    NGI_NELEMENTS(ngcplRelayInvokeMethodStrings))
