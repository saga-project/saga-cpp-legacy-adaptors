#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclMDSaccess.c,v $ $Revision: 1.42 $ $Date: 2006/06/08 05:59:38 $";
#endif /* NG_OS_IRIX */
/* 
 * $AIST_Release: 4.2.4 $
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

/**
 * MDS accessing module for Ninf-G Client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "ng.h"

#ifdef NGI_NO_MDS2_MODULE
#ifdef NGI_NO_MDS4_MODULE
#define NGCLL_NO_MDS_MODULE
#endif /* NGI_NO_MDS4_MODULE */
#endif /* NGI_NO_MDS2_MODULE */

#ifndef NGCLL_NO_MDS_MODULE

#ifndef NGI_NO_MDS2_MODULE
#include <ldap.h>

#define MDS_SEARCH_BASE "Mds-Vo-name=%s,o=Grid"
#define MDS_HOST_FILTER "(&(Mds-Host-hn=%s)(objectClass=GridRPC))"
#define MDS_EXEC_FILTER "(GridRPC-Funcname=%s)"
#endif /* NGI_NO_MDS2_MODULE */

#ifndef NGI_NO_MDS4_MODULE
#include "ngXML.h"
#include <globus_xml_buffer.h>
#endif /* NGI_NO_MDS4_MODULE */

/**
 * Prototypes
 */

static int ngcllMDSaccessFindMDSserver(ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng, int *error);
static int ngcllMDSaccessRemoteMachineInformationGetWithMDS(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng, int *error);
static int ngcllMDSaccessRemoteClassInformationGet(ngclContext_t *context,
    char *className, int *error);
static int ngcllMDSaccessRemoteClassInformationGetWithMDS(
    ngclContext_t *context, char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng, int *error);

static int ngcllMDSaccessRemoteMachineInformationIsRegistered(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *isRegistered, int *error);
static int ngcllMDSaccessExecutablePathInformationIsRegistered(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className, int *isRegistered, int *error);
static int ngcllMDSaccessRemoteClassInformationIsRegistered(
    ngclContext_t *context, char *className, int *isRegistered, int *error);
static int ngcllMDSaccessRemoteMachineInformationRegister(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char *hostDn, int mpiNcpus, int *error);
static int ngcllMDSaccessExecutablePathInformationRegister(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className, char *execPath, int *error);
static int ngcllMDSaccessRemoteClassInformationRegister(
    ngclContext_t *context, char *className, char *classInfo,
    int isAlreadyValid, int *error);
static int
    ngcllMDSaccessRemoteMachineInformationUnregisterMDSserverInformation(
    ngclContext_t *context, ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error);


static int 
ngcllMDSaccessMDSserverInformationInitializeByMDS2(
    ngclContext_t *context, ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error);
static int 
ngcllMDSaccessMDSserverInformationFinalizeByMDS2(
    ngclContext_t *context, ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error);
static int
ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS2(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng, char **searched_hostDn,
    int *searched_mpiNcpus, int *error);
static int
ngcllMDSaccessRemoteExecutableInformationGetByMDS2(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className, char **searched_execPath, char **searched_classInfo,
    int *error);
static int
ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS2(
    ngclContext_t *context, char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng, char **searched_classInfo,
    int *error);
#ifndef NGI_NO_MDS2_MODULE
static int ngcllMDSaccessLDAPsearch(ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng, int *error,
    LDAP *ld, char *searchBase, int scope, char *filter, int timeout,
    char **dn, int arguments, ...);
static char *ngcllMDSaccessGetString(char *format, char *arg);
static int ngcllMDSaccessGetValueStr(LDAP *ld, LDAPMessage *ent,
    char *attrName, char **attrValue, ngclContext_t *context, int *error);
static int ngcllMDSaccessGetValueInt(LDAP *ld, LDAPMessage *ent,
    char *attrName, int *attrValue, ngclContext_t *context, int *error);

typedef enum ngcllMDSaccessLDAPsearchArgumentType_e {
     NGCLL_MDSACCESS_TYPE_NONE, /* for error */
     NGCLL_MDSACCESS_TYPE_STR,  /* get string type */
     NGCLL_MDSACCESS_TYPE_INT  /* get int type */
} ngcllMDSaccessLDAPsearchArgumentType_t;
   
/* save arguments for ngcllMDSaccessLDAPsearch */
typedef struct ngcllMDSaccessLDAPsearchArgument_s {
    ngcllMDSaccessLDAPsearchArgumentType_t nmalsa_type; /* desired type */
    char *nmalsa_attrName;  /* desired attribute name */
    void *nmalsa_store;    /* search result is stored here */
} ngcllMDSaccessLDAPsearchArgument_t;
#endif /* NGI_NO_MDS2_MODULE */

static int
ngcllMDSaccessMDSserverInformationInitializeByMDS4(
    ngclContext_t *context, ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error);
static int 
ngcllMDSaccessMDSserverInformationFinalizeByMDS4(
    ngclContext_t *context, ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error);
static int
ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS4(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_hostDn, int *searched_mpiNcpus, int *error);
static int
ngcllMDSaccessRemoteExecutableInformationGetByMDS4(
    ngclContext_t *context, ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className, char **searched_execPath, char **searched_classInfo,
    int *error);
static int
ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS4(
    ngclContext_t *context, char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng, char **searched_classInfo,
    int *error);
#ifndef NGI_NO_MDS4_MODULE
static int ngcllMDSaccessISsearch(ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    IndexService_client_handle_t index, char *xpath,
    int timeout, char **arg, int *error);
static char *ngcllMDSaccessXPathHostInfo(char const *hostname);
static char *ngcllMDSaccessXPathExecInfo(char const *className);
static char *ngcllMDSaccessXMLprettyPrint(ngiXMLelement_t *element,
    ngLog_t *log, int *error);

#define NGCLL_MDS4_DUMMY_HOST_DN "MDS4_dummy_host_dn"

#endif /* NGI_NO_MDS4_MODULE */

/**
 * Functions
 */

/**
 * Initialize all MDS Server.
 */
int 
ngcliMDSaccessInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessInitialize";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    int result, listLocked;
    ngLog_t *log;

    listLocked = 0;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Lock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of MDS Server Information.\n", fName);
        goto error;
    }
    listLocked = 1;

    /* Set the flag */
    context->ngc_mdsAccessEnabled = 1;

    /* Initialize all MDS Server */
    mdsInfoMng = NULL; /* NULL to get first element. */
    while ((mdsInfoMng = ngcliMDSserverInformationCacheGetNext(
                               context, mdsInfoMng, error)) != NULL) {

        /* Initialize the MDS Server */
        result = ngcliMDSaccessMDSserverInformationInitialize(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the MDS Access on the MDS Server.\n",
                fName);
            goto error;
        }
    }

    /* Unlock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadUnlock(context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (listLocked != 0) {
        result = ngcliMDSserverInformationListReadUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of MDS Server Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Finalize all MDS Server.
 */
int 
ngcliMDSaccessFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessFinalize";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    int result, returnCode, listLocked;
    ngLog_t *log;

    listLocked = 0;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Lock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of MDS Server Information.\n", fName);
        goto error;
    }
    listLocked = 1;

    returnCode = 1;

    /* Finalize all MDS Server */
    mdsInfoMng = NULL; /* NULL to get first element. */
    while ((mdsInfoMng = ngcliMDSserverInformationCacheGetNext(
        context, mdsInfoMng, error)) != NULL) {

        /* Finalize the MDS Server */
        result = ngcliMDSaccessMDSserverInformationFinalize(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the MDS Access on the MDS Server.\n",
                fName);
            returnCode = 0;
            /* Not return */
        }
    }

    /* Set the flag */
    context->ngc_mdsAccessEnabled = 0;

    /* Unlock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadUnlock(context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return returnCode;

    /* Error occurred */
error:
    /* Unlock */
    if (listLocked != 0) {
        result = ngcliMDSserverInformationListReadUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of MDS Server Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Initialize the MDS Server.
 */
int 
ngcliMDSaccessMDSserverInformationInitialize(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessMDSserverInformationInitialize";
    int result, mdsLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    log = context->ngc_log;
    mdsLocked = 0;

    /* Check the state */
    if (context->ngc_mdsAccessEnabled == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS Access is not enabled.\n",
            fName);
        goto error;
    }

    /* Lock the MDS Server Information */
    result = ngcliMDSserverInformationWriteLock(mdsInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the MDS Server Information.\n",
            fName);
        goto error;
    }
    mdsLocked = 1;

    /* Initialize the MDS2 (LDAP) or MDS4 (Information Services). */
    if (mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2) {
        result = ngcllMDSaccessMDSserverInformationInitializeByMDS2(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to initialize the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS2");
            goto error;
        }

    } else if (mdsInfoMng->ngmsim_info.ngmsi_type ==
        NGCL_MDS_SERVER_TYPE_MDS4) {
        result = ngcllMDSaccessMDSserverInformationInitializeByMDS4(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to initialize the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS4");
            goto error;
        }

    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown MDS type %d for MDS Server \"%s\".\n",
            fName,
            (int)mdsInfoMng->ngmsim_info.ngmsi_type,
            mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* Unlock the MDS Server Information */
    result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
    mdsLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the MDS Server Information */
    if (mdsLocked != 0) {
        result = ngcliMDSserverInformationWriteUnlock(
            mdsInfoMng, log, NULL);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the MDS Server Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Finalize the MDS Server.
 */
int 
ngcliMDSaccessMDSserverInformationFinalize(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessMDSserverInformationFinalize";
    int result, mdsLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    log = context->ngc_log;
    mdsLocked = 0;

    /* Check the state */
    if (context->ngc_mdsAccessEnabled == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS Access is not enabled.\n",
            fName);
        /* Not return */
    }

    /* Lock the MDS Server Information */
    result = ngcliMDSserverInformationWriteLock(mdsInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the MDS Server Information.\n",
            fName);
        goto error;
    }
    mdsLocked = 1;

    /* Clear the MDS Server entry on the all Remote Machine Information */
    result =
        ngcllMDSaccessRemoteMachineInformationUnregisterMDSserverInformation(
        context, mdsInfoMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unregister the MDS Server Information \"%s\""
            " on the Remote Machine Information.\n",
            fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);
        /* Not return */
    }

    /* Finalize connection for MDSserver */
    if (mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2) {
        result = ngcllMDSaccessMDSserverInformationFinalizeByMDS2(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to finalize the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS2");
            goto error;
        }

    } else if (mdsInfoMng->ngmsim_info.ngmsi_type ==
        NGCL_MDS_SERVER_TYPE_MDS4) {
        result = ngcllMDSaccessMDSserverInformationFinalizeByMDS4(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to finalize the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS4");
            goto error;
        }

    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown MDS type %d for MDS Server \"%s\".\n",
            fName,
            (int)mdsInfoMng->ngmsim_info.ngmsi_type,
            mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* Unlock the MDS Server Information */
    result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
    mdsLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the MDS Server Information */
    if (mdsLocked != 0) {
        result = ngcliMDSserverInformationWriteUnlock(
            mdsInfoMng, log, NULL);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the MDS Server Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * do fill rmInfoMng including retrieve from MDS.
 * if rmInfoMng doesn't know MDS Server, then search that, at first.
 */
int
ngcliMDSaccessRemoteMachineInformationGet(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessRemoteMachineInformationGet";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    char *mdsTagName, *mdsHostName;
    int result, listLocked;
    ngLog_t *log;

    listLocked = 0;
    mdsTagName = NULL;
    mdsHostName = NULL;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (rmInfoMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Check if already accessed */
    if ((rmInfoMng->ngrmim_mdsServer != NULL) ||
        (rmInfoMng->ngrmim_hostDN != NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_ALREADY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Remote Machine Information \"%s\""
            " already retrieved from MDS \"%s\".\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName,
            rmInfoMng->ngrmim_mdsServer->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Retrieving the Remote Machine Information \"%s\""
        " by MDS.\n",
        fName, rmInfoMng->ngrmim_info.ngrmi_hostName);

    /* Lock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of MDS Server Information.\n", fName);
        goto error;
    }
    listLocked = 1;

    /* If MDS Server specified, try it first */
    mdsTagName = rmInfoMng->ngrmim_info.ngrmi_mdsTag;
    mdsHostName = rmInfoMng->ngrmim_info.ngrmi_mdsServer;

    if ((mdsTagName != NULL) || (mdsHostName != NULL)) {

        /* Get MDSserverInformation */
        mdsInfoMng = ngcliMDSserverInformationCacheGet(context,
                 mdsTagName, mdsHostName,
                 NGCLI_MDS_SERVER_CACHE_GET_MODE_MATCH, error);
        if (mdsInfoMng == NULL) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: MDS Server"
                " (tag name = \"%s\", host name = \"%s\") not found.\n",
                fName, ((mdsTagName != NULL) ? mdsTagName : "null"),
                ((mdsHostName != NULL) ? mdsHostName : "null"));
            goto error;
        }

        /* Get RemoteMachineInformation from MDS */
        result = ngcllMDSaccessRemoteMachineInformationGetWithMDS(
            context, rmInfoMng, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Remote Machine Information"
                " from MDS Server (tag name = \"%s\", host name = \"%s\").\n",
                fName, ((mdsTagName != NULL) ? mdsTagName : "null"),
                ((mdsHostName != NULL) ? mdsHostName : "null"));
            /* Not return */
        }
    }

    /* Try MDS Server list */
    if (rmInfoMng->ngrmim_mdsServer == NULL) {

        result = ngcllMDSaccessFindMDSserver(context, rmInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: No valid MDS Server was found.\n", fName);
            goto error;
        }
    }

    /* Unlock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadUnlock(context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of MDS Server Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (listLocked != 0) {
        result = ngcliMDSserverInformationListReadUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of MDS Server Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Search MDS Server which has host information for rmInfoMng.
 *  by retrieving rmInfoMng host info from each MDS Server. 
 *  if successfully get, use that MDS Server continuously.
 * this function do fill rmInfoMng also.
 */
static int
ngcllMDSaccessFindMDSserver(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    static const char fName[] = "ngcllMDSaccessFindMDSserver";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    int subError; /* fail will always occur */
    char *mdsTagName, *mdsHostName;
    int result, returnCode;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    mdsTagName = NULL;
    mdsHostName = NULL;

    if ((rmInfoMng->ngrmim_mdsServer != NULL) ||
        (rmInfoMng->ngrmim_hostDN != NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_ALREADY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: The Remote Machine Information \"%s\""
            " already retrieved from the MDS Server \"%s\".\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName,
            rmInfoMng->ngrmim_mdsServer->ngmsim_info.ngmsi_hostName);
        return 0;
    }

    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Finding the MDS Server which has Remote Machine Information"
        " \"%s\" from the MDS Server list.\n",
        fName, rmInfoMng->ngrmim_info.ngrmi_hostName);

    returnCode = 0;
    mdsInfoMng = NULL; /* to retrieve first element */
    while ((mdsInfoMng = ngcliMDSserverInformationCacheGetNext(
                        context, mdsInfoMng, error)) != NULL) {
        result = ngcllMDSaccessRemoteMachineInformationGetWithMDS(
                context, rmInfoMng, mdsInfoMng, &subError);

        /* found */
        if (result != 0) {
            returnCode = 1;
            break;
        }
        if ((subError != NG_ERROR_NO_ERROR) &&
            (subError != NG_ERROR_NOT_EXIST)) {

            NGI_SET_ERROR(error, subError);
            mdsTagName = mdsInfoMng->ngmsim_info.ngmsi_tagName;
            mdsHostName = mdsInfoMng->ngmsim_info.ngmsi_hostName;
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: An error occur in getting information"
                " from the MDS Server"
                " (tag name = \"%s\", host name = \"%s\").\n",
                fName, ((mdsTagName != NULL) ? mdsTagName : "null"),
                ((mdsHostName != NULL) ? mdsHostName : "null"));
            returnCode = 0;
            break;
        }
    }

    if (returnCode == 1) {
        /* Success */
        return 1;
    }

    /* failure. No MDS Server has specified RemoteMachineInformation. */
    if ((error != NULL) && (*error == NG_ERROR_NO_ERROR)) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    }
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: No MDS Server has the Remote Machine Information \"%s\".\n",
        fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
    return 0;
}

/**
 * Get the MDS RemoteMachineInformation from specific MDS Server.
 * failure will occur naturally.
 * if successful, store that result into rmInfoMng.
 */
static int
ngcllMDSaccessRemoteMachineInformationGetWithMDS(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationGetWithMDS";
    char *searched_hostDn;
    int searched_mpiNcpus;
    int result, mdsLocked;
    int isRegistered;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(mdsInfoMng != NULL);
    assert(rmInfoMng->ngrmim_info.ngrmi_hostName != NULL);

    log = context->ngc_log;
    isRegistered = 0;
    searched_hostDn = NULL;
    searched_mpiNcpus = 0;
    mdsLocked = 0;

    /* Lock the MDS Server Information */
    result = ngcliMDSserverInformationWriteLock(mdsInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the MDS Server Information.\n", fName);
        goto error;
    }
    mdsLocked = 1;

    /* Check if the RemoteMachineInformation MDS already registered */
    result = ngcllMDSaccessRemoteMachineInformationIsRegistered(
        context, rmInfoMng, &isRegistered, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't check the Remote Machine Information \"%s\".\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
        goto error;
    }
    if (isRegistered != 0) {
        /* Already registered */

        /* log */
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: The Remote Machine Information \"%s\""
            " was already available.\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName);

        /* Unlock the MDS Server Information */
        result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't unlock the MDS Server Information.\n",
                fName);
            goto error;
        }
     
        /* Success */
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Retrieving the Remote Machine Information \"%s\""
        " from the MDS Server \"%s\".\n",
        fName, rmInfoMng->ngrmim_info.ngrmi_hostName,
        mdsInfoMng->ngmsim_info.ngmsi_hostName);

    /* MDS access */
    if (mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2) {
        result = ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS2(
            context, rmInfoMng, mdsInfoMng,
            &searched_hostDn, &searched_mpiNcpus, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to access the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS2");
            goto error;
        }

    } else if (mdsInfoMng->ngmsim_info.ngmsi_type ==
        NGCL_MDS_SERVER_TYPE_MDS4) {
        result = ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS4(
            context, rmInfoMng, mdsInfoMng,
            &searched_hostDn, &searched_mpiNcpus, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to access the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS4");
            goto error;
        }

    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown MDS type %d for MDS Server \"%s\".\n",
            fName,
            (int)mdsInfoMng->ngmsim_info.ngmsi_type,
            mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* Register to RemoteMachineInformation */
    result = ngcllMDSaccessRemoteMachineInformationRegister(
        context, rmInfoMng, mdsInfoMng, searched_hostDn,
        searched_mpiNcpus, error);
    globus_libc_free(searched_hostDn);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Remote Machine Information \"%s\".\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
        goto error;
    }

    /* Unlock the MDS Server Information */
    result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
    mdsLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't unlock the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the MDS Server Information */
    if (mdsLocked != 0) {
        result = ngcliMDSserverInformationWriteUnlock(
            mdsInfoMng, log, NULL);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't unlock the MDS Server Information.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * This function retrieves RemoteExecutable Information.
 * RemoteExecutable Information includes
 *  ExecutablePath, and RemoteClass (includes RemoteMethod) Information.
 */
int
ngcliMDSaccessRemoteExecutableInformationGet(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessRemoteExecutableInformationGet";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    char *searched_execPath, *searched_classInfo;
    int epIsRegistered, rcIsRegistered;
    int result, mdsLocked;
    ngLog_t *log;

    mdsInfoMng = NULL;
    mdsInfoMng = NULL;
    searched_execPath = NULL;
    searched_classInfo = NULL;
    mdsLocked = 0;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;
    epIsRegistered = 0;
    rcIsRegistered = 0;

    /* Check the arguments */
    if (className == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        goto error;
    }

    if (rmInfoMng == NULL){
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Remote Machine Information was not specified."
            " Try to get only Remote Class Information.\n",
            fName);

        result = ngcllMDSaccessRemoteClassInformationGet(
            context, className, error);

        return result;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Retrieving the Executable Path Information \"%s\" \"%s\""
        " by MDS.\n",
        fName, className, rmInfoMng->ngrmim_info.ngrmi_hostName);

    /* If the hostDN was not determined, then set hostDN at first */
    if ((rmInfoMng->ngrmim_mdsServer == NULL) ||
        (rmInfoMng->ngrmim_hostDN == NULL)) {
        result = ngcliMDSaccessRemoteMachineInformationGet(
            context, rmInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Remote Machine Information.\n",
                fName);
            goto error;
        }
    }

    /* Is the hostDN determined? */
    if ((rmInfoMng->ngrmim_hostDN == NULL) ||
        (rmInfoMng->ngrmim_mdsServer == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: The MDS Server which has the Remote Machine Information"
            " \"%s\" was not found.\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
        goto error;
    }

    mdsInfoMng = rmInfoMng->ngrmim_mdsServer;

    /* Lock the MDS Server Information */
    result = ngcliMDSserverInformationWriteLock(mdsInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the MDS Server Information.\n", fName);
        goto error;
    }
    mdsLocked = 1;

    /* Check if the ExecutablePathInformation already registered */
    result = ngcllMDSaccessExecutablePathInformationIsRegistered(
        context, rmInfoMng, className, &epIsRegistered, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't check the Executable Path Information"
            " \"%s\" \"%s\".\n",
            fName, className, rmInfoMng->ngrmim_info.ngrmi_hostName);
        goto error;
    }

    /* Check if the RemoteClassInformation already registered */
    result = ngcllMDSaccessRemoteClassInformationIsRegistered(
        context, className, &rcIsRegistered, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't check the Remote Class Information \"%s\".\n",
            fName, className);
        goto error;
    }

    if ((epIsRegistered != 0) && (rcIsRegistered != 0)) {
        /* Already registered */

        /* Unlock the MDS Server Information */
        result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't unlock the MDS Server Information.\n",
                fName);
            goto error;
        }
     
        /* Success */
        return 1;
    }

    /* MDS access */
    if (mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2) {
        result = ngcllMDSaccessRemoteExecutableInformationGetByMDS2(
            context, rmInfoMng, className,
            &searched_execPath, &searched_classInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to access the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS2");
            goto error;
        }

    } else if (mdsInfoMng->ngmsim_info.ngmsi_type ==
        NGCL_MDS_SERVER_TYPE_MDS4) {
        result = ngcllMDSaccessRemoteExecutableInformationGetByMDS4(
            context, rmInfoMng, className,
            &searched_execPath, &searched_classInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to access the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS4");
            goto error;
        }

    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown MDS type %d for MDS Server \"%s\".\n",
            fName,
            (int)mdsInfoMng->ngmsim_info.ngmsi_type,
            mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* Register to ExecutablePathInformation */
    result = ngcllMDSaccessExecutablePathInformationRegister(
        context, rmInfoMng, className, searched_execPath, error);
    globus_libc_free(searched_execPath);
    if (result == 0) {
        globus_libc_free(searched_classInfo);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Executable Path Information"
            " \"%s\" \"%s\".\n",
            fName, className, rmInfoMng->ngrmim_info.ngrmi_hostName);
        goto error;
    }

    /* Register to RemoteClassInformation */
    result = ngcllMDSaccessRemoteClassInformationRegister(
        context, className, searched_classInfo, 1, error);
    globus_libc_free(searched_classInfo);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Remote Class Information \"%s\".\n",
            fName, className);
        goto error;
    }

    /* Unlock the MDS Server Information */
    result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
    mdsLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the MDS Server Information */
    if (mdsLocked != 0) {
        result = ngcliMDSserverInformationWriteUnlock(
            mdsInfoMng, log, NULL);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the MDS Server Information.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Search class information unless specifying hostname.
 */
static int
ngcllMDSaccessRemoteClassInformationGet(
    ngclContext_t *context,
    char *className,
    int *error)
{
    static const char fName[] = "ngcllMDSaccessRemoteClassInformationGet";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    int subError; /* failure will always occur */
    int result, returnCode, listLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);

    log = context->ngc_log;
    mdsInfoMng = NULL; 
    listLocked = 0;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Retrieving the Remote Class Information \"%s\""
        " from MDS Server list.\n",
        fName, className);

   /* Lock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of MDS Server Information.\n", fName);
        goto error;
    }
    listLocked = 1;

    returnCode = 0;
    mdsInfoMng = NULL; /* to retrieve first element */
    while ((mdsInfoMng = ngcliMDSserverInformationCacheGetNext(
                         context, mdsInfoMng, error)) != NULL) {
        result = ngcllMDSaccessRemoteClassInformationGetWithMDS(
                context, className, mdsInfoMng, &subError);
        if (result != 0) {
            returnCode = 1;
            break;
        }
        if ((subError != NG_ERROR_NO_ERROR) &&
            (subError != NG_ERROR_NOT_EXIST)) {
            NGI_SET_ERROR(error, subError);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: An error occur in getting information from"
                " the MDS Server \"%s\".\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);
            returnCode = 0;
            break;
        }
    }

    /* Unlock the MDS Server Information List */
    result = ngcliMDSserverInformationListReadUnlock(context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of MDS Server Information.\n",
            fName);
        goto error;
    }

    if (returnCode == 1) {
        /* Success */
        return 1;
    }

    /* failure. No MDS Server has specified RemoteClassInformation */
    if ((error != NULL) && (*error == NG_ERROR_NO_ERROR)) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    }
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: No MDS Server has Remote Class Information \"%s\".\n",
        fName, className);

    /* Failed */
    return 0;

    /* Error occurred */
error:
    /* Unlock */
    if (listLocked != 0) {
        result = ngcliMDSserverInformationListReadUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of MDS Server Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteClassInformation from specific MDS Server.
 */
static int
ngcllMDSaccessRemoteClassInformationGetWithMDS(
    ngclContext_t *context,
    char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteClassInformationGetWithMDS";
    char *searched_classInfo;
    int result, mdsLocked;
    int isRegistered;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(mdsInfoMng != NULL);

    log = context->ngc_log;
    searched_classInfo = NULL;
    mdsLocked = 0;
    isRegistered = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Retrieving the Remote Class Information \"%s\""
        " from the MDS Server \"%s\".\n",
        fName, className, mdsInfoMng->ngmsim_info.ngmsi_hostName);

    /* Lock the MDS Server Information */
    result = ngcliMDSserverInformationWriteLock(mdsInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the MDS Server Information.\n", fName);
        goto error;
    }
    mdsLocked = 1;

    /* Check if the RemoteClassInformation already registered */
    result = ngcllMDSaccessRemoteClassInformationIsRegistered(
        context, className, &isRegistered, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't check the Remote Class Information \"%s\".\n",
            fName, className);
        goto error;
    }
    if (isRegistered != 0) {
        /* Already registered */

        /* Unlock the MDS Server Information */
        result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't unlock the MDS Server Information.\n",
                fName);
            goto error;
        }
     
        /* Success */
        return 1;
    }

    /* MDS access */
    if (mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2) {
        result = ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS2(
            context, className, mdsInfoMng,
            &searched_classInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to access the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS2");
            goto error;
        }

    } else if (mdsInfoMng->ngmsim_info.ngmsi_type ==
        NGCL_MDS_SERVER_TYPE_MDS4) {
        result = ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS4(
            context, className, mdsInfoMng,
            &searched_classInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to access the MDS Server \"%s\" (%s).\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS2");
            goto error;
        }

    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown MDS type %d for MDS Server \"%s\".\n",
            fName,
            (int)mdsInfoMng->ngmsim_info.ngmsi_type,
            mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* Register to RemoteClassInformation */
    result = ngcllMDSaccessRemoteClassInformationRegister(
        context, className, searched_classInfo, 0, error);
    globus_libc_free(searched_classInfo);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Remote Class Information \"%s\".\n",
            fName, className);
        goto error;
    }

    /* Unlock the MDS Server Information */
    result = ngcliMDSserverInformationWriteUnlock(mdsInfoMng, log, error);
    mdsLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the MDS Server Information */
    if (mdsLocked != 0) {
        result = ngcliMDSserverInformationWriteUnlock(
            mdsInfoMng, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the MDS Server Information.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Check if the RemoteMachineInformation got from MDS registered.
 */
static int
ngcllMDSaccessRemoteMachineInformationIsRegistered(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *isRegistered,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationIsRegistered";
    int result, rmLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(isRegistered != NULL);

    log = context->ngc_log;
    rmLocked = 0;

    *isRegistered = 0;

    /* Lock the Remote Machine Information */
    result = ngcliRemoteMachineInformationWriteLock(rmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Remote Machine Information.\n", fName);
        goto error;
    }
    rmLocked = 1;

    /* Check if registered */
    if ((rmInfoMng->ngrmim_mdsServer != NULL) &&
        (rmInfoMng->ngrmim_hostDN != NULL)) {
        *isRegistered = 1;

    } else if ((rmInfoMng->ngrmim_mdsServer != NULL) ||
        (rmInfoMng->ngrmim_hostDN != NULL)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Two MDS information (MDS Server and hostDN) for"
            " Remote Machine Information \"%s\" mismatched.\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
        *isRegistered = 0;

    } else {
        *isRegistered = 0;
    }

    /* Unlock the Remote Machine Information */
    result = ngcliRemoteMachineInformationWriteUnlock(rmInfoMng, log, error);
    rmLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Remote Machine Information.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Machine Information */
    if (rmLocked != 0) {
        result = ngcliRemoteMachineInformationWriteUnlock(
            rmInfoMng, log, NULL);
        rmLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Remote Machine Information.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Check if the ExecutablePathInformation registered.
 */
static int
ngcllMDSaccessExecutablePathInformationIsRegistered(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    int *isRegistered,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessExecutablePathInformationIsRegistered";
    ngcliExecutablePathInformationManager_t *epInfoMng;
    int result, listLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(className != NULL);
    assert(isRegistered != NULL);

    log = context->ngc_log;
    epInfoMng = NULL;
    listLocked = 0;

    *isRegistered = 0;

    /* Lock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadLock(
        rmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable Path Information.\n",
            fName);
        goto error;
    }
    listLocked = 1;

    /* Get ExecutablePathInformation */
    epInfoMng = ngcliExecutablePathInformationCacheGet(
        context, rmInfoMng, className, error);
    if ((epInfoMng != NULL) &&
        (epInfoMng->ngepim_info.ngepi_path != NULL)) {
        *isRegistered = 1;
    }

    /* Unlock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadUnlock(
        rmInfoMng, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable Path Information.\n",
             fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable Path Information */
    if (listLocked != 0) {
        result = ngcliExecutablePathInformationListReadUnlock(
            rmInfoMng, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Executable Path Information.\n",
                 fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Check if the RemoteClassInformation registered.
 */
static int
ngcllMDSaccessRemoteClassInformationIsRegistered(
    ngclContext_t *context,
    char *className,
    int *isRegistered,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteClassInformationIsRegistered";
    ngclRemoteClassInformationManager_t *rcInfoMng;
    int result, listLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(isRegistered != NULL);

    log = context->ngc_log;
    rcInfoMng = NULL;
    listLocked = 0;

    *isRegistered = 0;

    /* Lock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Remote Class Information.\n", fName);
        goto error;
    }
    listLocked = 1;

    /* Get RemoteClassInformation */
    rcInfoMng = ngcliRemoteClassInformationCacheGet(
        context, className, error);
    if (rcInfoMng != NULL) {
        *isRegistered = 1;
    }

    /* Unlock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadUnlock(context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Remote Class Information.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Class Information */
    if (listLocked != 0) {
        result = ngcliRemoteClassInformationListReadUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Remote Class Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Register RemoteMachineInformation got from MDS.
 */
static int
ngcllMDSaccessRemoteMachineInformationRegister(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char *hostDn,
    int mpiNcpus,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationRegister";
    int result, rmLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(mdsInfoMng != NULL);
    assert(hostDn != NULL);

    log = context->ngc_log;
    rmLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Registering the Remote Machine Information \"%s\""
        " (mpiNcpus=%d, hostDN=\"%s\") got from MDS.\n",
        fName, rmInfoMng->ngrmim_info.ngrmi_hostName,
        mpiNcpus, hostDn);

    /* Lock the Remote Machine Information */
    result = ngcliRemoteMachineInformationWriteLock(rmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Remote Machine Information.\n", fName);
        goto error;
    }
    rmLocked = 1;

    if ((rmInfoMng->ngrmim_mdsServer != NULL) ||
        (rmInfoMng->ngrmim_hostDN != NULL)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: The MDS setting for the Remote Machine Information"
            " \"%s\" already exists.\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
    }

    rmInfoMng->ngrmim_mdsServer = mdsInfoMng;
    rmInfoMng->ngrmim_hostDN = strdup(hostDn);
    if (rmInfoMng->ngrmim_hostDN == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* If data is undefined, then set MDS data */
    if (rmInfoMng->ngrmim_info.ngrmi_mpiNcpus <= 0) {
        rmInfoMng->ngrmim_info.ngrmi_mpiNcpus = mpiNcpus;
    }

    /* Unlock the Remote Machine Information */
    result = ngcliRemoteMachineInformationWriteUnlock(rmInfoMng, log, error);
    rmLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Remote Machine Information.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Machine Information */
    if (rmLocked != 0) {
        result = ngcliRemoteMachineInformationWriteUnlock(
            rmInfoMng, log, NULL);
        rmLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Remote Machine Information.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Register ExecutablePathInformation got from MDS.
 *  If ExecutablePath exists in context, set execPath to that information.
 *  If ExecutablePath not exist, then new ExecutablePath are registered.
 */
static int
ngcllMDSaccessExecutablePathInformationRegister(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    char *execPath,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessExecutablePathInformationRegister";
    ngcliExecutablePathInformationManager_t *epInfoMng;
    ngcliExecutablePathInformation_t *epInfo;
    int result, listLocked, epLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(className != NULL);
    assert(execPath != NULL);

    log = context->ngc_log;
    epInfoMng = NULL;
    epInfo = NULL;
    listLocked = 0;
    epLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Registering the Executable Path Information"
        " \"%s\" \"%s\" got from MDS.\n",
        fName, className, rmInfoMng->ngrmim_info.ngrmi_hostName);

    /* Lock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadLock(rmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable Path Information.\n", fName);
        goto error;
    }
    listLocked = 1;

    /* Get Executable Path Information */
    epInfoMng = ngcliExecutablePathInformationCacheGet(
        context, rmInfoMng, className, error);
    if (epInfoMng != NULL) {

        /* Lock the Executable Path Information */
        result = ngcliExecutablePathInformationWriteLock(
            epInfoMng, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, error,
                "%s: Can't lock the Executable Path Information.\n", fName);
            goto error;
        }
        epLocked = 1;

        /* Release previous path (but this will not happen) */
        if (epInfoMng->ngepim_info.ngepi_path != NULL) {
            globus_libc_free(epInfoMng->ngepim_info.ngepi_path);
            epInfoMng->ngepim_info.ngepi_path = NULL;
        }

        /* Copy executable path */
        epInfoMng->ngepim_info.ngepi_path = strdup(execPath);
        if (epInfoMng->ngepim_info.ngepi_path == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for string.\n",
                fName);
            goto error;
        }

        /* Unock the Executable Path Information */
        result = ngcliExecutablePathInformationWriteUnlock(
            epInfoMng, log, error);
        epLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't unlock the Executable Path Information.\n", fName);
            goto error;
        }
    }

    /* Unlock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadUnlock(
        rmInfoMng, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable Path Information.\n",
             fName);
        goto error;
    }
    
    if (epInfoMng != NULL) {
        /* Success */
        return 1;
    }

    /* Executable Path Information was not registered */

    epInfo = ngcliExecutablePathInformationAllocate(context, error);
    if (epInfo == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Executable Path Information.\n",
            fName);
        goto error;
    }

    result = ngcliExecutablePathInformationInitialize(context, epInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Executable Path Information.\n",
            fName);
        goto error;
    }

    epInfo->ngepi_hostName = strdup(rmInfoMng->ngrmim_info.ngrmi_hostName);
    if (epInfo->ngepi_hostName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    epInfo->ngepi_className = strdup(className);
    if (epInfo->ngepi_className == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    epInfo->ngepi_path = strdup(execPath);
    if (epInfo->ngepi_path == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    result = ngcliExecutablePathInformationCacheRegister(
        context, rmInfoMng, epInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Executable Path Information.\n",
            fName);
        goto error;
    }

    result = ngclExecutablePathInformationRelease(
        context, epInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the storage for Executable Path Information.\n",
            fName);
        goto error;
    }

    result = ngcliExecutablePathInformationFree(context, epInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't free the storage for Executable Path Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unock the Executable Path Information */
    if (epLocked != 0) {
        result = ngcliExecutablePathInformationWriteUnlock(
            epInfoMng, log, NULL);
        epLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't unlock the Executable Path Information.\n",
                fName);
        }
    }

    /* Unlock the list of Executable Path Information */
    if (listLocked != 0) {
        result = ngcliExecutablePathInformationListReadUnlock(
            rmInfoMng, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Executable Path Information.\n",
                 fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Register RemoteClassInformation got from MDS.
 */
static int
ngcllMDSaccessRemoteClassInformationRegister(
    ngclContext_t *context,
    char *className,
    char *classInfo,
    int isAlreadyValid,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteClassInformationRegister";
    ngclRemoteClassInformationManager_t *rcInfoMng;
    ngRemoteClassInformation_t *rcInfo;
    int result, listLocked, subError;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(classInfo != NULL);

    log = context->ngc_log;
    rcInfoMng = NULL;
    rcInfo = NULL;
    listLocked = 0;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Registering the Remote Class Information"
        " \"%s\" got from MDS.\n",
        fName, className);

    /* Lock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Remote Class Information.\n", fName);
        goto error;
    }
    listLocked = 1;

    /* Get RemoteClassInformation */
    rcInfoMng = ngcliRemoteClassInformationCacheGet(
        context, className, &subError);

    /* Unlock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadUnlock(context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Remote Class Information.\n",
            fName);
        goto error;
    }

    if (rcInfoMng != NULL) {
        if (isAlreadyValid == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: The Remote Class Information \"%s\""
                " already registered.\n",
                fName, className);
        }

        /* Success */
        return 1;
    }

    /* Generate RemoteClassInformation from XML */
    rcInfo = ngcliRemoteClassInformationGenerate(
        context, classInfo, strlen(classInfo), error);
    if (rcInfo == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't generate the Remote Class Information \"%s\".\n",
            fName, className);
        goto error;
    }

    /* Register to context */
    result = ngcliRemoteClassInformationCacheRegister(
        context, rcInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Remote Class Information \"%s\".\n",
            fName, className);
        goto error;
    } 

    /* Release RemoteClassInformation */
    result = ngclRemoteClassInformationRelease(context, rcInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Remote Class Information.\n", fName);
        goto error;
    }

    /* Free RemoteClassInformation */
    result = ngcliRemoteClassInformationFree(context, rcInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't free the Remote Class Information.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Class Information */
    if (listLocked != 0) {
        result = ngcliRemoteClassInformationListReadUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Remote Class Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Unregister the MDS Server registered to Remote Machine Information.
 */
static int
ngcllMDSaccessRemoteMachineInformationUnregisterMDSserverInformation(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationUnregisterMDSserverInformation";
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    int result, subError, listLocked, rmLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    log = context->ngc_log;
    listLocked = 0;
    rmLocked = 0;
    rmInfoMng = NULL;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Cleaning MDS Server Information \"%s\" entry"
        " from all Remote Machine Information.\n",
        fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);

    /* Lock the list of Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Remote Machine Information.\n",
            fName);
        goto error;
    }
    listLocked = 1;

    rmInfoMng = NULL; /* retrieve head item */
    while ((rmInfoMng = ngcliRemoteMachineInformationCacheGetNext(
        context, rmInfoMng, &subError)) != NULL) {

        /* Lock the Remote Machine Information */
        result = ngcliRemoteMachineInformationWriteLock(
            rmInfoMng, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Remote Machine Information.\n", fName);
            goto error;
        }
        rmLocked = 1;
     
        /* Does this rmInfoMng know the mdsInfoMng? */
        if (rmInfoMng->ngrmim_mdsServer == mdsInfoMng) {
            /* log */
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: Remote Machine Information \"%s\" has"
                " the entry to MDS Server Information \"%s\". Clean it.\n",
                fName, rmInfoMng->ngrmim_info.ngrmi_hostName,
                mdsInfoMng->ngmsim_info.ngmsi_hostName);

            /* Unlink */
            rmInfoMng->ngrmim_mdsServer = NULL;

            /* Clear the hostDN */
            if (rmInfoMng->ngrmim_hostDN != NULL) {
                globus_libc_free(rmInfoMng->ngrmim_hostDN);
                rmInfoMng->ngrmim_hostDN = NULL;
            }
        }

        /* Unlock the Remote Machine Information */
        result = ngcliRemoteMachineInformationWriteUnlock(
            rmInfoMng, log, error);
        rmLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Remote Machine Information.\n", fName);
            goto error;
        }
    }

    /* Unlock the list of Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Remote Machine Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Machine Information */
    if ((rmInfoMng != NULL) && (rmLocked != 0)) {
        result = ngcliRemoteMachineInformationWriteUnlock(
            rmInfoMng, log, NULL);
        rmLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Remote Machine Information.\n", fName);
        }
    }

    /* Unlock the list of Remote Machine Information */
    if (listLocked != 0) {
        result = ngcliRemoteMachineInformationListReadUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Remote Machine Information.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

#ifndef NGI_NO_MDS2_MODULE
/**
 * MDS2 depend functions.
 */

/**
 * Initialize the MDS Server for MDS2.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int 
ngcllMDSaccessMDSserverInformationInitializeByMDS2(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationInitializeByMDS2";
    in_port_t portNo;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_mdsAccessEnabled != 0);
    assert(mdsInfoMng != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2);

    portNo = 0;

    /* Check the state */
    if (mdsInfoMng->ngmsim_ldapInitialized != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS access on the MDS Server is already initialized.\n",
            fName);
        goto error;
    }

    /* Set the flag */
    mdsInfoMng->ngmsim_ldapInitialized = 1;

    portNo = mdsInfoMng->ngmsim_info.ngmsi_portNo;
    if (portNo == 0) { 
        portNo = NGCLI_MDS2_DEFAULT_PORT;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: ldap_init(mds=\"%s\", port=%d, timeout=%d).\n",
        fName, mdsInfoMng->ngmsim_info.ngmsi_hostName,
        portNo,
        mdsInfoMng->ngmsim_info.ngmsi_serverTimeout);

    /* Initialize connection for MDSserver */
    mdsInfoMng->ngmsim_ldap = ldap_init(
        mdsInfoMng->ngmsim_info.ngmsi_hostName,
        portNo);
    if (mdsInfoMng->ngmsim_ldap == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Initialize connection to MDS Server \"%s\" fail.\n",
            fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* set TIMEOUT */
    if (mdsInfoMng->ngmsim_info.ngmsi_serverTimeout > 0) {
        result = ldap_set_option(
            mdsInfoMng->ngmsim_ldap, LDAP_OPT_TIMELIMIT,
            &(mdsInfoMng->ngmsim_info.ngmsi_serverTimeout));
        if (result != 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: MDS Server \"%s\" set option fail.\n",
                fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Finalize the MDS Server for MDS2.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int 
ngcllMDSaccessMDSserverInformationFinalizeByMDS2(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationFinalizeByMDS2";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_mdsAccessEnabled != 0);
    assert(mdsInfoMng != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2);

    /* Check the state */
    if (mdsInfoMng->ngmsim_ldapInitialized == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS access on the MDS Server is not initialized.\n",
            fName);
        
        return 1;
    }

    /* Set the flag */
    mdsInfoMng->ngmsim_ldapInitialized = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: ldap_unbind(mds=\"%s\").\n",
        fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);

    /* Finalize connection for MDSserver */
    result = ldap_unbind(mdsInfoMng->ngmsim_ldap);
    if (result != LDAP_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unbind the MDS Server \"%s\".\n",
            fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteMachineInformation from MDS2.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int
ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS2(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_hostDn,
    int *searched_mpiNcpus,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS2";
    char *voName, *hostname, *searchBase, *hostFilter;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(mdsInfoMng != NULL);
    assert(searched_hostDn != NULL);
    assert(searched_mpiNcpus != NULL);
    assert(rmInfoMng->ngrmim_info.ngrmi_hostName != NULL);
    assert(mdsInfoMng->ngmsim_ldap != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_voName != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2);

    *searched_hostDn = NULL;
    *searched_mpiNcpus = 0;

    searchBase = NULL;
    hostFilter = NULL;
    hostname = NULL;
    voName = NULL;

    voName = mdsInfoMng->ngmsim_info.ngmsi_voName;
    hostname = rmInfoMng->ngrmim_info.ngrmi_hostName;

    searchBase = ngcllMDSaccessGetString(MDS_SEARCH_BASE, voName);
    if (searchBase == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    hostFilter = ngcllMDSaccessGetString(MDS_HOST_FILTER, hostname);
    if (hostFilter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* Get Informations from LDAP */
    result = ngcllMDSaccessLDAPsearch(context, mdsInfoMng, error,
        mdsInfoMng->ngmsim_ldap, searchBase, LDAP_SCOPE_SUBTREE,
        hostFilter, mdsInfoMng->ngmsim_info.ngmsi_clientTimeout,
        searched_hostDn, 1,
        NGCLL_MDSACCESS_TYPE_INT, "GridRPC-MpirunNoOfCPUs", searched_mpiNcpus
        );
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Can't get the Remote Machine Information \"%s\""
            " from the MDS Server \"%s\" (%s).\n",
            fName, rmInfoMng->ngrmim_info.ngrmi_hostName,
            mdsInfoMng->ngmsim_info.ngmsi_hostName, "MDS2");
        goto error;
    }
   
    if (searchBase != NULL) {
        globus_libc_free(searchBase);
    }

    if (hostFilter != NULL) {
        globus_libc_free(hostFilter);
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (searchBase != NULL) {
        globus_libc_free(searchBase);
    }

    if (hostFilter != NULL) {
        globus_libc_free(hostFilter);
    }

    /* Failed */
    return 0;
}

/**
 * This function retrieves RemoteExecutable Information from MDS2.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int
ngcllMDSaccessRemoteExecutableInformationGetByMDS2(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    char **searched_execPath,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteExecutableInformationGetByMDS2";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    char *execFilter;
    int result;

    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(className != NULL);
    assert(searched_execPath != NULL);
    assert(searched_classInfo != NULL);
    assert(rmInfoMng->ngrmim_mdsServer != NULL);

    *searched_execPath = NULL;
    *searched_classInfo = NULL;

    mdsInfoMng = rmInfoMng->ngrmim_mdsServer;
    execFilter = NULL;

    assert(mdsInfoMng->ngmsim_ldap != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_voName != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2);

    execFilter = ngcllMDSaccessGetString(MDS_EXEC_FILTER, className);
    if (execFilter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* Get Informations from LDAP */
    result = ngcllMDSaccessLDAPsearch(context, mdsInfoMng, error,
        mdsInfoMng->ngmsim_ldap, rmInfoMng->ngrmim_hostDN, LDAP_SCOPE_SUBTREE,
        execFilter, mdsInfoMng->ngmsim_info.ngmsi_clientTimeout,
        NULL, 2,
        NGCLL_MDSACCESS_TYPE_STR, "GridRPC-Path", searched_execPath,
        NGCLL_MDSACCESS_TYPE_STR, "GridRPC-Stub", searched_classInfo);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't get the Executable Information"
            " \"%s\" \"%s\" from the MDS Server \"%s\".\n",
            fName, className, rmInfoMng->ngrmim_info.ngrmi_hostName,
            mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    globus_libc_free(execFilter);

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (execFilter != NULL) {
        globus_libc_free(execFilter);
    }

    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteClassInformation from MDS2.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int
ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS2(
    ngclContext_t *context,
    char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS2";
    char *voName, *searchBase, *execFilter;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(mdsInfoMng != NULL);
    assert(searched_classInfo != NULL);
    assert(mdsInfoMng->ngmsim_ldap != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_voName != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2);

    *searched_classInfo = NULL;

    voName = NULL;
    searchBase = NULL;
    execFilter = NULL;

    voName = mdsInfoMng->ngmsim_info.ngmsi_voName;

    searchBase = ngcllMDSaccessGetString(MDS_SEARCH_BASE, voName);
    if (searchBase == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    execFilter = ngcllMDSaccessGetString(MDS_EXEC_FILTER, className);
    if (execFilter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* Get Information from MDS */
    result = ngcllMDSaccessLDAPsearch(context, mdsInfoMng, error,
        mdsInfoMng->ngmsim_ldap, searchBase, LDAP_SCOPE_SUBTREE,
        execFilter, mdsInfoMng->ngmsim_info.ngmsi_clientTimeout,
        NULL, 1,
        NGCLL_MDSACCESS_TYPE_STR, "GridRPC-Stub", searched_classInfo);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't get Remote Class Information \"%s\""
            " from the MDS Server \"%s\".\n",
            fName, className, mdsInfoMng->ngmsim_info.ngmsi_hostName);
        goto error;
    }

    if (searchBase != NULL) {
        globus_libc_free(searchBase);
    }

    if (execFilter != NULL) {
        globus_libc_free(execFilter);
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (searchBase != NULL) {
        globus_libc_free(searchBase);
    }

    if (execFilter != NULL) {
        globus_libc_free(execFilter);
    }

    /* Failed */
    return 0;
}

/**
 * Do ldap_search and get specified datum.
 */
static int
ngcllMDSaccessLDAPsearch(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error,
    LDAP *ld, 
    char *searchBase,
    int scope,
    char *filter,
    int timeout,
    char **dn,
    int arguments,
    ...)
{
    static const char fName[] = "ngcllMDSaccessLDAPsearch";
    ngcllMDSaccessLDAPsearchArgumentType_t cur_type;
    ngcllMDSaccessLDAPsearchArgument_t *argTable;
    LDAPMessage *ldresult, *ent;
    int result, firstTime, goNext, i, j;
    struct timeval timeoutValEntity, *timeoutVal;
    char *cur_attrName, *mdsHostName;
    int *cur_storeInt;
    char **cur_storeStr, **cur_freeStr;
    char *cur_dn;
    va_list args;

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);
    assert(ld != NULL);
    assert(searchBase != NULL);
    assert(filter != NULL);
    assert((dn != NULL) || (arguments > 0));

    mdsHostName = mdsInfoMng->ngmsim_info.ngmsi_hostName;

    /* Store function arguments to argTable */
    if (arguments > 0) {
        argTable = (ngcllMDSaccessLDAPsearchArgument_t *)
            globus_libc_malloc(
            sizeof(ngcllMDSaccessLDAPsearchArgument_t) * arguments);
        if (argTable == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for LDAP access.\n",
                fName);
            return 0;
        }

        va_start(args, arguments);
        for (i = 0; i < arguments; i++) {
            argTable[i].nmalsa_type = va_arg(args,
                ngcllMDSaccessLDAPsearchArgumentType_t);

            argTable[i].nmalsa_attrName = va_arg(args, char *);
            assert(argTable[i].nmalsa_attrName != NULL);

            argTable[i].nmalsa_store = va_arg(args, void *);
            assert(argTable[i].nmalsa_store != NULL);
        }
        va_end(args);
    } else {
        argTable = NULL;
    }

    /* Set timeoutVal */
    if (timeout > 0) {
        timeoutValEntity.tv_sec = timeout;
        timeoutValEntity.tv_usec = 0;
        timeoutVal = &timeoutValEntity;
    } else {
        timeoutVal = NULL;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: ldap_search_st(mds=\"%s\", base=\"%s\", filter=\"%s\","
        " timeout=%d).\n",
        fName, mdsHostName, searchBase, filter, timeout);

    /* Retrieve information from MDS */
    result = ldap_search_st(ld, searchBase,
                     scope, filter, NULL, 0, timeoutVal, &ldresult);
    if (result != LDAP_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: ldap_search_st(mds=\"%s\") failed.\n",
            fName, mdsHostName);

        return 0;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: ldap_search_st(mds=\"%s\") was successful.\n",
        fName, mdsHostName);

    /* Analyze information got */
    for (ent = ldap_first_entry(ld, ldresult); ent != NULL;
         ent = ldap_next_entry(ld, ent)) {

        firstTime = 1; /* TRUE */
        goNext = 0; /* FALSE */

        /* if dn is specified to get, then get DN */
        if (dn != NULL) {
            cur_dn = ldap_get_dn(ld, ent);
            if (cur_dn == NULL) {
                if (firstTime) {
                    continue;
                } else {
                    /* NOT REACHED */
                    abort();
                    return 0;
                }
            }
            firstTime = 0; /* FALSE */

            *dn = strdup(cur_dn);
            ldap_memfree(cur_dn);
            if (*dn == NULL) {
                ldap_msgfree(ldresult);
                if (argTable != NULL) {
                    globus_libc_free(argTable);
                }
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for string.\n",
                    fName);
                return 0;
            }
            
        }

        /* Get all required data specified form this function argument. */
        for (i = 0; i < arguments; i++) {

            /* set cur_* */
            cur_type = argTable[i].nmalsa_type;
            cur_attrName = argTable[i].nmalsa_attrName;
            cur_storeInt = NULL;
            cur_storeStr = NULL;
            if (cur_type == NGCLL_MDSACCESS_TYPE_STR) {
                cur_storeStr = (char **)argTable[i].nmalsa_store;
            } else if (cur_type == NGCLL_MDSACCESS_TYPE_INT) {
                cur_storeInt = (int *)argTable[i].nmalsa_store;
            } else {
                /* NOT REACHED */
                abort();
                return 0;
            }

            /* Get value */
            if (cur_type == NGCLL_MDSACCESS_TYPE_STR) {
                result = ngcllMDSaccessGetValueStr(ld, ent,
                    cur_attrName, cur_storeStr, context, error);
                if (result == 0) {
                    ngclLogPrintfContext(context,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
                        NULL,
                        "%s: Can't get string from LDAP result.\n",
                        fName);
                }

            } else if (cur_type == NGCLL_MDSACCESS_TYPE_INT) {
                result = ngcllMDSaccessGetValueInt(ld, ent,
                    cur_attrName, cur_storeInt, context, error);
                if (result == 0) {
                    ngclLogPrintfContext(context,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
                        NULL,
                        "%s: Can't get integer from LDAP result.\n",
                        fName);
                }

            } else {
                /* NOT REACHED */
                abort();
                return 0;
            }

            /* Failure */
            if ((result == 0) ||
                ((cur_type == NGCLL_MDSACCESS_TYPE_STR)
                    && (*cur_storeStr == NULL))){

                /* free() all malloc()ed object in this loop */
                if (*dn != NULL) {
                    globus_libc_free(*dn);
                }
                for (j = 0; j < i; j++) {
                    if (argTable[j].nmalsa_type == NGCLL_MDSACCESS_TYPE_STR) {
                        cur_freeStr = (char **)argTable[j].nmalsa_store;
                        assert(*cur_freeStr);
                        globus_libc_free(*cur_freeStr);
                        *cur_freeStr = NULL;
                    }
                }
                
                if (firstTime) {
                    /* If failure when first time, try next entry */
                    goNext = 1; /* TRUE */
                    break;
                } else {
                    /* If failure when not first time, */
                    /* That means incomplete data was found, thus failure */
                    ldap_msgfree(ldresult);
                    globus_libc_free(argTable);
                    ngclLogPrintfContext(context,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't get required all data from LDAP result.\n",
                        fName);
                    return 0;
                }
            }
            firstTime = 0; /* FALSE */

        }
        if (goNext) {
            /* Go to analyze Next result entry */
            continue;
        }

        /* Getting all Informations desired was found */
        ldap_msgfree(ldresult);
        if (argTable != NULL) {
            globus_libc_free(argTable);
        }

        /* Success */
        return 1;
    }

    /* No valid entry was found */
    ldap_msgfree(ldresult);
    if (argTable != NULL) {
        globus_libc_free(argTable);
    }

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Desired entry was not found in this connection."
        " (mds=\"%s\", base=\"%s\", filter=\"%s\")\n",
        fName, mdsHostName, searchBase, filter);

    /* Failure */
    return 0;
}

/**
 * Get String for MDS LDAP search base and filter.
 *  just malloc() and snprintf().
 */
static char *
ngcllMDSaccessGetString(
    char *format,
    char *arg)
{
    int result, len;
    char *returnBuffer;

    /* Check the arguments */
    assert(format != NULL);

    /* Get length for buffer */
    len = strlen(format);
    if (arg != NULL) {
        len += strlen(arg);
    }
    len += 1; /* for NULL */

    returnBuffer = (char *)globus_libc_malloc(sizeof(char) * len);
    if (returnBuffer == NULL) {
        return NULL;
    }

    if (arg == NULL) {
        result = snprintf(returnBuffer, len, format);
    } else {
        result = snprintf(returnBuffer, len, format, arg);
    }
    if (result <= -1 || result >= len) {
        /* failure */
        globus_libc_free(returnBuffer);
        return NULL;
    }

    /* Success */
    return returnBuffer;
}

/**
 * return 1, if specified value are found and attrValue are set.
 * return 0, if not found.
 */
static int
ngcllMDSaccessGetValueStr(
    LDAP *ld,
    LDAPMessage *ent,
    char *attrName,
    char **attrValue,
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllMDSaccessGetValueStr";
    char **vals;

    /* Check the arguments */
    assert(ld != NULL);
    assert(ent != NULL);
    assert(attrName != NULL);
    assert(attrValue != NULL);
    
    *attrValue = NULL; /* for safety */

    vals = ldap_get_values(ld, ent, attrName);
    
    if ((vals == NULL) || (vals[0] == NULL) || (vals[0][0] == '\0')) {
        if (vals != NULL) {
            ldap_value_free(vals);
        }
        /* Not found */
        /* It may happen normally */
        return 0;
    }

    *attrValue = strdup(vals[0]);
    ldap_value_free(vals);
    if (*attrValue == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
                    fName);

        return 0;
    }

    return 1;
}

/**
 * return 1, if specified value are found and attrValue are set.
 * return 0, if not found.
 */
static int
ngcllMDSaccessGetValueInt(
    LDAP *ld,
    LDAPMessage *ent,
    char *attrName,
    int *attrValue,
    ngclContext_t *context,
    int *error)
{
    int result;
    char *tmpStr;

    /* Check the arguments */
    assert(ld != NULL);
    assert(ent != NULL);
    assert(attrName != NULL);
    assert(attrValue != NULL);

    tmpStr = NULL; /* No meaning, but for safety */
    result = ngcllMDSaccessGetValueStr(
        ld, ent, attrName, &tmpStr, context, error);
    if ((result == 0) || (tmpStr == NULL)) {
        /* Failure */
        return 0;
    }

    *attrValue = (int)strtol(tmpStr, NULL, 10);
    globus_libc_free(tmpStr);

    /* Success */
    return 1;
}

#else /* NGI_NO_MDS2_MODULE */
/**
 * Dummy functions for No MDS2 environment.
 */

/**
 * Initialize the MDS Server for MDS2. (Just error)
 */
static int 
ngcllMDSaccessMDSserverInformationInitializeByMDS2(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationInitializeByMDS2";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS2 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * Finalize the MDS Server for MDS2. (Just error)
 */
static int 
ngcllMDSaccessMDSserverInformationFinalizeByMDS2(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationFinalizeByMDS2";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS2 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteMachineInformation from MDS2. (Just error)
 */
static int
ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS2(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_hostDn,
    int *searched_mpiNcpus,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS2";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS2 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * This function retrieves RemoteExecutable Information from MDS2. (Just error)
 */
static int
ngcllMDSaccessRemoteExecutableInformationGetByMDS2(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    char **searched_execPath,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteExecutableInformationGetByMDS2";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS2 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteClassInformation from MDS2. (Just error)
 */
static int
ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS2(
    ngclContext_t *context,
    char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS2";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS2 module.\n",
        fName);

    /* Failed */
    return 0;
}

#endif /* NGI_NO_MDS2_MODULE */

#ifndef NGI_NO_MDS4_MODULE
/**
 * MDS4 depend functions.
 */

/**
 * Initialize the MDS Server for MDS4.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int
ngcllMDSaccessMDSserverInformationInitializeByMDS4(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationInitializeByMDS4";
    globus_result_t globus_result;
    gss_buffer_desc nameBuffer;
    char *anonymousString;
    OM_uint32 majorCode;
    OM_uint32 minorCode;
    long authMethod;
    gss_OID  nType;
    char *subject;

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_mdsAccessEnabled != 0);
    assert(mdsInfoMng != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS4);

    subject = NULL;

    /* Check the state */
    if (mdsInfoMng->ngmsim_indexInitialized != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS access on the Index Service is already initialized.\n",
            fName);
        goto error;
    }

    /* Set the flag */
    mdsInfoMng->ngmsim_indexInitialized = 1;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: MDS4 Initialize (mds=\"%s\").\n",
        fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);

    /* Initialize SOAP Message Attribution */
    globus_result = globus_soap_message_attr_init(
        &(mdsInfoMng->ngmsim_index_message_attr));
    if (globus_result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Initialize SOAP Message Attribution fail.\n",
            fName);
        goto error;
    }

    /* Set subject SOAP Message Attribution */
    subject = mdsInfoMng->ngmsim_info.ngmsi_subject;
    authMethod = GLOBUS_SOAP_MESSAGE_AUTHZ_HOST;

    if (subject != NULL) {
        authMethod = GLOBUS_SOAP_MESSAGE_AUTHZ_IDENTITY;

        nType = GSS_C_NO_OID;
        nameBuffer.value = subject;
        nameBuffer.length = strlen(subject);
        anonymousString = "<anonymous>";

        if (strchr(subject, '@') && !strstr(subject, "CN=")) {
            nType = GSS_C_NT_HOSTBASED_SERVICE;
        } else if (!strncmp(
            anonymousString, subject, strlen(anonymousString))) {
            nType = GSS_C_NT_ANONYMOUS;
        }

        majorCode = gss_import_name(
            &minorCode, &nameBuffer, nType,
            &(mdsInfoMng->ngmsim_subjectIdentity));
        if (GSS_ERROR(majorCode)) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Set SOAP Message Attribution \"%s\" %s fail.\n",
                fName, "subject",
                "gss_import_name()");
            goto error;
        }

        globus_result = globus_soap_message_attr_set(
            mdsInfoMng->ngmsim_index_message_attr,
            GLOBUS_SOAP_MESSAGE_AUTHZ_TARGET_NAME_KEY,
            NULL, NULL,
            mdsInfoMng->ngmsim_subjectIdentity);
        if (globus_result != GLOBUS_SUCCESS) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Set SOAP Message Attribution \"%s\" %s fail.\n",
                fName, "subject",
                "GLOBUS_SOAP_MESSAGE_AUTHZ_TARGET_NAME_KEY");
            goto error;
        }
    }

    globus_result = globus_soap_message_attr_set(
        mdsInfoMng->ngmsim_index_message_attr,
        GLOBUS_SOAP_MESSAGE_AUTHZ_METHOD_KEY,
        NULL, NULL,
        (void *)authMethod);
    if (globus_result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Set SOAP Message Attribution \"%s\" %s fail.\n",
            fName, "subject",
            "GLOBUS_SOAP_MESSAGE_AUTHZ_METHOD_KEY");
        goto error;
    }

    /* Set timeout SOAP Message Attribution */
    if (mdsInfoMng->ngmsim_info.ngmsi_clientTimeout > 0) {
        GlobusTimeReltimeSet(
            mdsInfoMng->ngmsim_clientTimeout,
            mdsInfoMng->ngmsim_info.ngmsi_clientTimeout,
            0);
        globus_result = globus_soap_message_attr_set(
            mdsInfoMng->ngmsim_index_message_attr,
            GLOBUS_SOAP_MESSAGE_TIMEOUT_KEY,
            NULL,
            NULL,
            (void *)&mdsInfoMng->ngmsim_clientTimeout);
        if (globus_result != GLOBUS_SUCCESS) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Set SOAP Message Attribution \"%s\" fail.\n",
                fName, "timeout");
            goto error;
        }
    }

    /* Initialize Index Service Client Handle */
    globus_result = IndexService_client_init(
        &(mdsInfoMng->ngmsim_index),
        mdsInfoMng->ngmsim_index_message_attr,
        NULL);
    if (globus_result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Initialize Index Service Client Handle fail.\n",
            fName);
        goto error;
    }

    /* set TIMEOUT */
    if (mdsInfoMng->ngmsim_info.ngmsi_serverTimeout > 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: MDS4 Server timeout is not effective.\n",
            fName);
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Finalize the MDS Server for MDS4.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int 
ngcllMDSaccessMDSserverInformationFinalizeByMDS4(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationFinalizeByMDS4";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_mdsAccessEnabled != 0);
    assert(mdsInfoMng != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS4);

    /* Check the state */
    if (mdsInfoMng->ngmsim_indexInitialized == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS access on the MDS is not initialized.\n",
            fName);

        return 1;
    }

    /* Set the flag */
    mdsInfoMng->ngmsim_indexInitialized = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: MDS4 Finalize (mds=\"%s\").\n",
        fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);

    /* Destroy Index Service Client Handle */
    IndexService_client_destroy(mdsInfoMng->ngmsim_index);
    mdsInfoMng->ngmsim_index = NULL;

    /* Destroy SOAP Message Attribution */
    globus_soap_message_attr_destroy(mdsInfoMng->ngmsim_index_message_attr);
    mdsInfoMng->ngmsim_index_message_attr = NULL;

    /* Success */
    return 1;
}

/**
 * Get the MDS RemoteMachineInformation from MDS4.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int
ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS4(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_hostDn,
    int *searched_mpiNcpus,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS4";
    char *rmHostName, *mdsHostName;
    ngiXMLelement_t *element;
    ngiXMLparser_t *parser;
    char *hostInfo, *xpath;
    char *mpiNcpusStr;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(mdsInfoMng != NULL);
    assert(searched_hostDn != NULL);
    assert(searched_mpiNcpus != NULL);
    assert(rmInfoMng->ngrmim_info.ngrmi_hostName != NULL);
    assert(mdsInfoMng->ngmsim_index != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS4);

    *searched_hostDn = NULL;
    *searched_mpiNcpus = 0;

    log = context->ngc_log;
    parser = NULL;
    element = NULL;
    xpath = NULL;
    hostInfo = NULL;
    mpiNcpusStr = NULL;

    rmHostName = rmInfoMng->ngrmim_info.ngrmi_hostName;
    mdsHostName = mdsInfoMng->ngmsim_info.ngmsi_hostName;

    xpath = ngcllMDSaccessXPathHostInfo(rmHostName);
    if (xpath == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* Get Informations from MDS */
    result = ngcllMDSaccessISsearch(context, mdsInfoMng,
        mdsInfoMng->ngmsim_index, xpath,
        mdsInfoMng->ngmsim_info.ngmsi_clientTimeout,
        &hostInfo, error);
    if ((result == 0) || (hostInfo == NULL)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Can't get the Remote Machine Information \"%s\""
            " from the MDS Server \"%s\" (%s).\n",
            fName, rmHostName, mdsHostName, "MDS4");
        goto error;
    }

    /* Parse and get attrs in hostInfo */

    /* Construct XML parser */
    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for XMLparser.\n",
            fName);
        goto error;
    }

    /* Parse XML */
    result = ngiXMLparserParse(
        parser, hostInfo, strlen(hostInfo), 1, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: XML parse for MDS4 hostInfo \"%s\""
            " from MDS Server \"%s\" fail.\n",
            fName, rmHostName, mdsHostName);
        goto error;
    }

    /* Get root element tree */
    element = ngiXMLparserGetRootElement(parser, log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get root element.\n", fName);
        goto error;
    }

    /* Get hostInfo element tree */
    element = ngiXMLelementGetNext(
        element, NULL, "ng4:hostInfo", log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get ng4:hostInfo element in XML.\n", fName);
        goto error;
    }

    mpiNcpusStr = ngiXMLattributeGetValue(element, "mpiCpus", log, error);
    if (mpiNcpusStr == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get mpiNcpus attr in hostInfo XML.\n", fName);
        goto error;
    }
    *searched_mpiNcpus = atoi(mpiNcpusStr);

    *searched_hostDn = globus_libc_strdup(NGCLL_MDS4_DUMMY_HOST_DN);
    if (*searched_hostDn == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    if (parser != NULL) {
        /* Destruct XML parser */
        result = ngiXMLparserDestruct(parser, log, error);
        parser = NULL;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct XML parser.\n", fName);
            goto error;
        }
    }

    if (xpath != NULL) {
        globus_libc_free(xpath);
        xpath = NULL;
    }

    if (hostInfo != NULL) {
        globus_libc_free(hostInfo);
        hostInfo = NULL;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (xpath != NULL) {
        globus_libc_free(xpath);
        xpath = NULL;
    }

    if (hostInfo != NULL) {
        globus_libc_free(hostInfo);
        hostInfo = NULL;
    }

    if (parser != NULL) {
        /* Destruct XML parser */
        result = ngiXMLparserDestruct(parser, log, NULL);
        parser = NULL;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct XML parser.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * This function retrieves RemoteExecutable Information from MDS4.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int
ngcllMDSaccessRemoteExecutableInformationGetByMDS4(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    char **searched_execPath,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteExecutableInformationGetByMDS4";
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    char *rmHostName, *mdsHostName;
    char *execInfo, *classInfo;
    char *execPath;
    ngiXMLelement_t *element;
    ngiXMLparser_t *parser;
    ngLog_t *log;
    char *xpath;
    int result;

    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(className != NULL);
    assert(searched_execPath != NULL);
    assert(searched_classInfo != NULL);
    assert(rmInfoMng->ngrmim_mdsServer != NULL);

    *searched_execPath = NULL;
    *searched_classInfo = NULL;

    mdsInfoMng = rmInfoMng->ngrmim_mdsServer;
    log = context->ngc_log;
    parser = NULL;
    element = NULL;
    xpath = NULL;
    execInfo = NULL;
    execPath = NULL;
    classInfo = NULL;

    rmHostName = rmInfoMng->ngrmim_info.ngrmi_hostName;
    mdsHostName = mdsInfoMng->ngmsim_info.ngmsi_hostName;

    xpath = ngcllMDSaccessXPathExecInfo(className);
    if (xpath == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* Get Informations from MDS */
    result = ngcllMDSaccessISsearch(context, mdsInfoMng,
        mdsInfoMng->ngmsim_index, xpath,
        mdsInfoMng->ngmsim_info.ngmsi_clientTimeout,
        &execInfo, error);
    if ((result == 0) || (execInfo == NULL)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't get the Executable Information"
            " \"%s\" \"%s\" from the MDS Server \"%s\".\n",
            fName, className, rmHostName, mdsHostName);
        goto error;
    }

    /* Parse and get info in execInfo */

    /* Construct XML parser */
    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for XMLparser.\n",
            fName);
        goto error;
    }

    /* Parse XML */
    result = ngiXMLparserParse(
        parser, execInfo, strlen(execInfo), 1, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: XML parse for MDS4 execInfo fail.\n", fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: XML parse for MDS4 execInfo \"%s\""
            " from MDS Server \"%s\" fail.\n",
            fName, rmHostName, mdsHostName);
        goto error;
    }

    /* Get root element tree */
    element = ngiXMLparserGetRootElement(parser, log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get root element.\n", fName);
        goto error;
    }

    /* Get execInfo element tree */
    element = ngiXMLelementGetNext(
        element, NULL, "ng4:execInfo", log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get execInfo element in XML.\n", fName);
        goto error;
    }

    execPath = ngiXMLattributeGetValue(element, "execPath", log, error);
    if (execPath == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get execPath attribute in XML.\n", fName);
        goto error;
    }

    *searched_execPath = globus_libc_strdup(execPath);
    if (*searched_execPath == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* Get class element tree */
    element = ngiXMLelementGetNext(element, NULL, "class", log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get class element in XML.\n", fName);
        goto error;
    }

    classInfo = ngcllMDSaccessXMLprettyPrint(element, log, error);
    if (classInfo == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Class element in XML can't serialize.\n", fName);
        goto error;
    }

    if (xpath != NULL) {
        globus_libc_free(xpath);
        xpath = NULL;
    }

    if (execInfo != NULL) {
        globus_libc_free(execInfo);
        execInfo = NULL;
    }

    if (classInfo != NULL) {
        *searched_classInfo = globus_libc_strdup(classInfo);
        if (*searched_classInfo == NULL) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for string.\n", fName);
            goto error;
        }

        globus_libc_free(classInfo);
        classInfo = NULL;
    }

    if (parser != NULL) {
        /* Destruct XML parser */
        result = ngiXMLparserDestruct(parser, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct XML parser.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (xpath != NULL) {
        globus_libc_free(xpath);
        xpath = NULL;
    }

    if (execInfo != NULL) {
        globus_libc_free(execInfo);
        execInfo = NULL;
    }

    if (classInfo != NULL) {
        globus_libc_free(classInfo);
        classInfo = NULL;
    }

    if (parser != NULL) {
        /* Destruct XML parser */
        result = ngiXMLparserDestruct(parser, log, NULL);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct XML parser.\n", fName);
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteClassInformation from MDS4.
 * Note:
 * Write Lock the MDS before using this function, and unlock the MDS after use.
 */
static int
ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS4(
    ngclContext_t *context,
    char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS4";
    char *execInfo, *classInfo;
    ngiXMLparser_t *parser;
    ngiXMLelement_t *element;
    char *mdsHostName;
    ngLog_t *log;
    char *xpath;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(mdsInfoMng != NULL);
    assert(searched_classInfo != NULL);
    assert(mdsInfoMng->ngmsim_info.ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS4);

    *searched_classInfo = NULL;

    log = context->ngc_log;
    parser = NULL;
    element = NULL;
    xpath = NULL;
    execInfo = NULL;
    classInfo = NULL;

    mdsHostName = mdsInfoMng->ngmsim_info.ngmsi_hostName;

    xpath = ngcllMDSaccessXPathExecInfo(className);
    if (xpath == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    /* Get Informations from MDS */
    result = ngcllMDSaccessISsearch(context, mdsInfoMng,
        mdsInfoMng->ngmsim_index, xpath,
        mdsInfoMng->ngmsim_info.ngmsi_clientTimeout,
        &execInfo, error);
    if ((result == 0) || (execInfo == NULL)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Can't get the Remote Class Information \"%s\""
            " from the MDS Server \"%s\" (%s).\n",
            fName, className, mdsHostName, "MDS4");
        goto error;
    }

    /* Parse and get info in execInfo */

    /* Construct XML parser */
    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for XMLparser.\n",
            fName);
        goto error;
    }

    /* Parse XML */
    result = ngiXMLparserParse(
        parser, execInfo, strlen(execInfo), 1, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: XML parse for MDS4 execInfo \"%s\""
            " from MDS Server \"%s\" fail.\n",
            fName, className, mdsHostName);
        goto error;
    }

    /* Get root element tree */
    element = ngiXMLparserGetRootElement(parser, log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get root element.\n", fName);
        goto error;
    }

    /* Get execInfo element tree */
    element = ngiXMLelementGetNext(
        element, NULL, "ng4:execInfo", log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get execInfo element in XML.\n", fName);
        goto error;
    }

    /* Get class element tree */
    element = ngiXMLelementGetNext(element, NULL, "class", log, error);
    if (element == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get class element in XML.\n", fName);
        goto error;
    }

    classInfo = ngcllMDSaccessXMLprettyPrint(element, log, error);
    if (classInfo == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Class \"%s\" element in XML can't serialize.\n",
            fName, className);
        goto error;
    }

    if (xpath != NULL) {
        globus_libc_free(xpath);
        xpath = NULL;
    }

    if (execInfo != NULL) {
        globus_libc_free(execInfo);
        execInfo = NULL;
    }

    if (classInfo != NULL) {
        *searched_classInfo = globus_libc_strdup(classInfo);
        if (*searched_classInfo == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for string.\n",
                fName);
            goto error;
        }
 
        globus_libc_free(classInfo);
        classInfo = NULL;
    }

    if (parser != NULL) {
        /* Destruct XML parser */
        result = ngiXMLparserDestruct(parser, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct XML parser.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (xpath != NULL) {
        globus_libc_free(xpath);
        xpath = NULL;
    }

    if (execInfo != NULL) {
        globus_libc_free(execInfo);
        execInfo = NULL;
    }

    if (classInfo != NULL) {
        globus_libc_free(classInfo);
        classInfo = NULL;
    }

    if (parser != NULL) {
        /* Destruct XML parser */
        result = ngiXMLparserDestruct(parser, log, NULL);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct XML parser.\n", fName);
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Do search and get specified datum.
 */
static int
ngcllMDSaccessISsearch(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    IndexService_client_handle_t index,
    char *xpath,
    int timeout,
    char **arg,
    int *error)
{
    static const char fName[] = "ngcllMDSaccessISsearch";
    wsrp_QueryResourcePropertiesResponseType *queryResourcePropertiesResponse;
    wsrp_QueryResourcePropertiesType *queryResourceProperties;
    IndexPortType_QueryResourceProperties_fault_t fault_type;
    char *url_format, *url, *protocol, *path;
    globus_result_t gresult;
    int len, portNo;
    char *mdsHostName;
    xsd_any *fault;

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);
    assert(index != NULL);
    assert(xpath != NULL);
    assert(arg != NULL);

    url_format = NULL;
    protocol = NULL;
    portNo = 0;
    path = NULL;
    url = NULL;
    len = 0;
    queryResourceProperties = NULL;
    queryResourcePropertiesResponse = NULL;
    fault = NULL;
    mdsHostName = mdsInfoMng->ngmsim_info.ngmsi_hostName;

    assert(mdsHostName != NULL);

    url_format = "%s://%s:%d%s";

    protocol = mdsInfoMng->ngmsim_info.ngmsi_protocol;
    if (protocol == NULL) {
        protocol = NGCLI_MDS4_SERVICE_URL_PROTO;
    }

    portNo = mdsInfoMng->ngmsim_info.ngmsi_portNo;
    if (portNo == 0) {
        portNo = NGCLI_MDS4_DEFAULT_PORT;
    }

    path = mdsInfoMng->ngmsim_info.ngmsi_path;
    if (path == NULL) {
        path = NGCLI_MDS4_SERVICE_URL_PATH;
    }

    assert(url_format != NULL);
    assert(protocol != NULL);
    assert(portNo != 0);
    assert(mdsHostName != NULL);
    assert(path != NULL);

    len = strlen(url_format) + strlen(protocol) + 
        NGI_INT_MAX_DECIMAL_DIGITS + strlen(mdsHostName) +
        strlen(path) + 1;

    url = globus_libc_malloc(len);
    if (url == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n",
            fName);
        goto error;
    }

    snprintf(url, len, url_format, protocol, mdsHostName, portNo, path);

    protocol = NULL;
    portNo = 0;
    path = NULL;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: MDS4 access URL=\"%s\", xpath=\"%s\".\n",
        fName, url, xpath);

    gresult = wsrp_QueryResourcePropertiesType_init(&queryResourceProperties);
    if (gresult != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to initialize QueryResourcePropertiesType.\n",
            fName);
        goto error;
    }

    xsd_anyURI_init(&(queryResourceProperties->QueryExpression._Dialect));
    *(queryResourceProperties->QueryExpression._Dialect) =
        "http://www.w3.org/TR/1999/REC-xpath-19991116";
    xsd_any_init(&(queryResourceProperties->QueryExpression.any));
    queryResourceProperties->QueryExpression.any->registry = NULL;
    queryResourceProperties->QueryExpression.any->any_info =
        &xsd_string_contents_info;
    queryResourceProperties->QueryExpression.any->element = NULL;
    xsd_string_init_cstr(
        (xsd_string **)
        (&(queryResourceProperties->QueryExpression.any->value)),
        xpath);

    gresult = IndexPortType_QueryResourceProperties(
        index, url,
        queryResourceProperties,
        &queryResourcePropertiesResponse,
        &fault_type,
        &fault);

    if (queryResourcePropertiesResponse) {
        if (queryResourcePropertiesResponse->any.length > 0) {
            xsd_any *p = &queryResourcePropertiesResponse->any.elements[0];
            if (p->any_info == &globus_xml_buffer_contents_info) {
                globus_xml_buffer *buffer_handle =
                    (globus_xml_buffer *)p->value;
                if (buffer_handle) {
                    *arg = globus_libc_strdup(buffer_handle->buffer);
                }
            }
        }
        wsrp_QueryResourcePropertiesResponseType_destroy(
            queryResourcePropertiesResponse);
    } else {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to QueryResourceProperties to URL \"%s\".\n",
            fName, url);
        goto error;
    }

    globus_libc_free(url);

    return 1;

error:
    return 0;
}

/**
 * Get String for hostInfo MDS search XPath.
 *  just malloc() and snprintf().
 */
static char *
ngcllMDSaccessXPathHostInfo(
    char const *hostname)
{
    int result, len;
    char *returnBuffer;

    char const format[] =
        "//*[namespace-uri()='http://ninf.apgrid.org/ng4/grpcinfo/types'"
        " and local-name()='grpcInfoSet']"
        "/*[namespace-uri()='http://ninf.apgrid.org/ng4'"
        " and local-name()='hostInfo'"
        " and @hostName='%s']";

    /* Check the arguments */
    assert(hostname != NULL);

    /* Get length for buffer */
    len = (sizeof format) + strlen(hostname) + 1;

    returnBuffer = globus_libc_malloc(len);
    if (returnBuffer) {
        result = snprintf(returnBuffer, len, format, hostname);
        if ((result <= -1) || (result >= len)) {
            /* failure */
            globus_libc_free(returnBuffer);
            returnBuffer = NULL;
        }
    }

    /* Success */
    return returnBuffer;
}

/**
 * Get String for classInfo MDS search XPath.
 *  just malloc() and snprintf().
 */
static char *
ngcllMDSaccessXPathExecInfo(
    char const *className)
{
    int result, len;
    char *returnBuffer;

    char const format[] =
        "//*[namespace-uri()='http://ninf.apgrid.org/ng4/grpcinfo/types'"
        " and local-name()='grpcInfoSet']"
        "/*[namespace-uri()='http://ninf.apgrid.org/ng4'"
        " and local-name()='execInfo'"
        " and @className='%s']";

    /* Check the arguments */
    assert(className != NULL);

    /* Get length for buffer */
    len = (sizeof format) + strlen(className) + 1;

    returnBuffer = globus_libc_malloc(len);
    if (returnBuffer) {
        result = snprintf(returnBuffer, len, format, className);
        if ((result <= -1) || (result >= len)) {
            /* failure */
            globus_libc_free(returnBuffer);
            returnBuffer = NULL;
        }
    }

    /* Success */
    return returnBuffer;
}

static int
ngcllMDSaccessXMLprettyPrint_iter(
    ngiXMLelement_t *element,
    char *buf[],
    size_t *buf_size,
    int depth,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllMDSaccessXMLprettyPrint_iter";
    static char const NEWLINE[] = "\n";
    char const TAB[] = "\t";
    int name_len = strlen(element->ngxe_name);
    int adjust_needs = 0;
    char *cdata;
    ngiXMLelement_t *child;

#define NGCLL_EXPAND(len) \
    do { \
        size_t size = strlen(*buf) + (len) + 1; \
        if (size > *buf_size) { \
            if (!(*buf = globus_libc_realloc(*buf, size))) { \
                return -1; \
            } \
            *buf_size = size; \
        } \
    } while (0)

    child = ngiXMLelementGetNext(element, NULL, NULL, log, error);
    if ((cdata = ngiXMLelementGetCdata(element, log, error))) {
        /* this element has CDATA */
        if (child != NULL) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: XML element has both cdata and child.\n",
                fName);
        }
        adjust_needs = 1;
        NGCLL_EXPAND(sizeof NEWLINE);
        strcat(*buf, NEWLINE);
        {
            /* do indentation */
            int i;
            for (i = 0; i < depth; ++i) {
                NGCLL_EXPAND(sizeof TAB);
                strcat(*buf, TAB);
            }
        }
        NGCLL_EXPAND(1 + name_len + 1 + strlen(cdata) + 1 + 1 + name_len + 1);
        strcat(*buf, "<");
        strcat(*buf, element->ngxe_name);
        strcat(*buf, ">");
        strcat(*buf, cdata);
        strcat(*buf, "</");
        strcat(*buf, element->ngxe_name);
        strcat(*buf, ">");
    } else {
        if (child && depth) {
            adjust_needs = 1;
            NGCLL_EXPAND(sizeof NEWLINE);
            strcat(*buf, NEWLINE);
            {
                /* do indentation */
                int i;
                for (i = 0; i < depth; ++i) {
                    NGCLL_EXPAND(sizeof TAB);
                    strcat(*buf, TAB);
                }
            }
        }
        /* head of start/empty-element tag */
        NGCLL_EXPAND(1 + name_len);
        strcat(*buf, "<");
        strcat(*buf, element->ngxe_name);
        {
            /* attributes */
            ngiXMLattribute_t *attr = NULL;
            while ((attr = ngiXMLattributeGetNext(
                    element,
                    attr,
                    NULL,
                    log,
                    error))) {
                int attr_name_len = strlen(attr->ngxa_name);
                int attr_value_len = strlen(attr->ngxa_value);
                NGCLL_EXPAND(1 + attr_name_len + 1 + 1 + attr_value_len + 1);
                strcat(*buf, " ");
                strcat(*buf, attr->ngxa_name);
                strcat(*buf, "=\"");
                strcat(*buf, attr->ngxa_value);
                strcat(*buf, "\"");
            }
        }
        if (child) {
            int adj;
            /* tail of start tag */
            NGCLL_EXPAND(1);
            strcat(*buf, ">");
            /* recursive call */
            do {
                adj = ngcllMDSaccessXMLprettyPrint_iter(
                    child,
                    buf,
                    buf_size,
                    depth + 1,
                    log,
                    error);
                if (!*buf) {
                    return -1;
                }
            } while ((child = ngiXMLelementGetNext(element, child, NULL, log, error)));
            if (adj) {
                /* adjust-indent */
                int i;
                NGCLL_EXPAND(sizeof NEWLINE);
                strcat(*buf, NEWLINE);
                for (i = 0; i < depth; ++i) {
                    NGCLL_EXPAND(sizeof TAB);
                    strcat(*buf, TAB);
                }
            }
            /* end tag */
            NGCLL_EXPAND(1 + 1 + name_len + 1);
            strcat(*buf, "</");
            strcat(*buf, element->ngxe_name);
            strcat(*buf, ">");
        } else {
            /* tail of empty-element tag */
            NGCLL_EXPAND(3);
            strcat(*buf, " />");
        }
    }
#undef NGCLL_EXPAND

    return adjust_needs;
}

static char *
ngcllMDSaccessXMLprettyPrint(
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error)
{
    static char const NEWLINE[] = "\n";
    /* Check the arguments */
    assert(element != NULL);
    {
        size_t buf_size = 1;
        char *buf = globus_libc_malloc(buf_size);
        if (buf == NULL) {
            return NULL;
        }
        buf[0] = '\0';
        (void)ngcllMDSaccessXMLprettyPrint_iter(element, &buf, &buf_size, 0, log, error);
        if (!buf) {
            return NULL;
        }
        {
            size_t size = strlen(buf) + sizeof NEWLINE + 1;
            if (size > buf_size) {
                if (!(buf = globus_libc_realloc(buf, size))) {
                    return NULL;
                }
            }
        }
        strcat(buf, NEWLINE);
        return buf;
    }
}
#else /* NGI_NO_MDS4_MODULE */
/**
 * Dummy functions for No MDS4 environment.
 */

/**
 * Initialize the MDS Server for MDS4. (Just error)
 */
static int
ngcllMDSaccessMDSserverInformationInitializeByMDS4(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationInitializeByMDS4";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS4 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * Finalize the MDS Server for MDS4.
 */
static int 
ngcllMDSaccessMDSserverInformationFinalizeByMDS4(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessMDSserverInformationFinalizeByMDS4";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS4 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteMachineInformation from MDS4.
 */
static int
ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS4(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_hostDn,
    int *searched_mpiNcpus,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteMachineInformationGetWithMDSbyMDS4";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS4 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * This function retrieves RemoteExecutable Information from MDS4.
 */
static int
ngcllMDSaccessRemoteExecutableInformationGetByMDS4(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    char **searched_execPath,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteExecutableInformationGetByMDS4";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS4 module.\n",
        fName);

    /* Failed */
    return 0;
}

/**
 * Get the MDS RemoteClassInformation from MDS4.
 */
static int
ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS4(
    ngclContext_t *context,
    char *className,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    char **searched_classInfo,
    int *error)
{
    static const char fName[] =
        "ngcllMDSaccessRemoteClassInformationGetWithMDSbyMDS4";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Ninf-G was configured with No MDS4 module.\n",
        fName);

    /* Failed */
    return 0;
}

#endif /* NGI_NO_MDS4_MODULE */

#else /* NGCLL_NO_MDS_MODULE */
/**
 * If the Ninf-G was configured for NO MDS module.
 * Then, MDSaccess module does nothing and return.
 */

/**
 * Initialize all MDS Server. (Do nothing)
 */
int 
ngcliMDSaccessInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessInitialize";
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Ninf-G was configured with No Globus Toolkit MDS module.\n",
        fName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Do nothing and return.\n",
        fName);

    return 1;
}

/**
 * Finalize all MDS Server. (Do nothing)
 */
int 
ngcliMDSaccessFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessFinalize";
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Ninf-G was configured with No Globus Toolkit MDS module.\n",
        fName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Do nothing and return.\n",
        fName);

    return 1;
}

/**
 * Initialize the MDS Server. (Do nothing)
 */
int 
ngcliMDSaccessMDSserverInformationInitialize(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessMDSserverInformationInitialize";

    /* Check the arguments */
    assert(context != NULL);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Ninf-G was configured with No Globus Toolkit MDS module.\n",
        fName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Do nothing and return.\n",
        fName);

    return 1;
}

/**
 * Finalize the MDS Server. (Do nothing)
 */
int 
ngcliMDSaccessMDSserverInformationFinalize(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessMDSserverInformationFinalize";

    /* Check the arguments */
    assert(context != NULL);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Ninf-G was configured with No Globus Toolkit MDS module.\n",
        fName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Do nothing and return.\n",
        fName);

    return 1;
}

/**
 * Get RemoteMachineInformation. (Just error)
 */
int
ngcliMDSaccessRemoteMachineInformationGet(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessRemoteMachineInformationGet";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: No Remote Machine Information for \"%s\".\n",
        fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Ninf-G was configured with No Globus Toolkit MDS module.\n",
        fName);

    return 0;
}

/**
 * Get RemoteExecutableInformation. (Just error)
 */
int
ngcliMDSaccessRemoteExecutableInformationGet(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    int *error)
{
    static const char fName[] = "ngcliMDSaccessRemoteExecutableInformationGet";
    char *hostName;

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);

    hostName = NULL;

    if (rmInfoMng != NULL) {
        hostName = rmInfoMng->ngrmim_info.ngrmi_hostName;
    }

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    if (hostName != NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: No Remote Executable Information for \"%s\" \"%s\".\n",
            fName, className, hostName);
    } else {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: No Remote Class Information for \"%s\".\n",
            fName, className);
    }
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Ninf-G was configured with No Globus Toolkit MDS module.\n",
        fName);

    return 0;
}

#endif /* NGCLL_NO_MDS_MODULE */

