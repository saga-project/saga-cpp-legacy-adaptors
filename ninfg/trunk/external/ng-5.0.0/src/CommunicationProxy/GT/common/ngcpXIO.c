/*
 * $RCSfile: ngcpXIO.c,v $ $Revision: 1.16 $ $Date: 2008/03/28 03:52:30 $
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

#include <ngemLog.h>
#include <ngemUtility.h>
#include <globus_gss_assist.h>

#include "ngcpXIO.h"

NGI_RCSID_EMBED("$RCSfile: ngcpXIO.c,v $ $Revision: 1.16 $ $Date: 2008/03/28 03:52:30 $")

typedef struct ngcplXIOinfo_s {
    globus_xio_stack_t  ngxi_tcpStack;
    globus_xio_stack_t  ngxi_gsiStack;
    globus_xio_stack_t  ngxi_fileStack;
    globus_xio_driver_t ngxi_tcpDriver;
    globus_xio_driver_t ngxi_gsiDriver;
    globus_xio_driver_t ngxi_fileDriver;
} ngcplXIOinfo_t;

ngcplXIOinfo_t ngcplXIOinfo;
bool ngcplXIOinitialized = false;
bool ngcplXIOactivated   = false;

/* Communication Security Level -> String */
const char *ngcpCommunicationSecurityString[] = {
    "none",
    "identity",
    "integrity",
    "confidentiality",
};

static globus_xio_stack_t ngcplXIOgetStack(ngcpCommunicationSecurity_t);
static ngemResult_t ngcplXIOattrSetSecurity(globus_xio_attr_t, ngcpCommunicationSecurity_t);
static ngemResult_t ngcplXIOhandleCreateFromFD(globus_xio_handle_t *, int);
static ngemResult_t ngcplGetUIDfromName(char *, uid_t *, gid_t *);
static ngemResult_t ngcplGlobusXIOcheckPeerNameHost(globus_xio_handle_t, bool *);
static ngemResult_t ngcplGlobusXIOcheckPeerNameSelf(globus_xio_handle_t, bool *);

/**
 * Initialize Globus XIO
 */
ngemResult_t
ngcpGlobusXIOinitialize(void)
{
    int result;
    globus_result_t gResult;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngcpGlobusXIOinitialize);

    log = ngemLogGetDefault();

    if (ngcplXIOinitialized) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Globus XIO is already initialized.\n");
        return NGEM_FAILED;
    }
    NGEM_ASSERT(ngcplXIOactivated == false);

    ngcplXIOinfo.ngxi_tcpStack   = NULL;
    ngcplXIOinfo.ngxi_gsiStack   = NULL;
    ngcplXIOinfo.ngxi_fileStack  = NULL;
    ngcplXIOinfo.ngxi_tcpDriver  = NULL;
    ngcplXIOinfo.ngxi_gsiDriver  = NULL;
    ngcplXIOinfo.ngxi_fileDriver = NULL;

    /* Activate */
    result = globus_module_activate(GLOBUS_XIO_MODULE);
    if (result != GLOBUS_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't activate Globus XIO.\n");
        goto error;
    }
    ngcplXIOactivated = true;

    /* Initialize Drivers */
    gResult = globus_xio_driver_load("tcp",  &ngcplXIOinfo.ngxi_tcpDriver);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_driver_load(\"tcp\")", gResult);
        goto error;
    }
    gResult = globus_xio_driver_load("gsi",  &ngcplXIOinfo.ngxi_gsiDriver);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_driver_load(\"gsi\")", gResult);
        goto error;
    }
    gResult = globus_xio_driver_load("file", &ngcplXIOinfo.ngxi_fileDriver);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_driver_load(\"file\")", gResult);
        goto error;
    }

    /* Initialize Stacks */
    gResult = globus_xio_stack_init(&ngcplXIOinfo.ngxi_tcpStack, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_stack_init", gResult);
        goto error;
    }
    gResult = globus_xio_stack_push_driver(ngcplXIOinfo.ngxi_tcpStack, ngcplXIOinfo.ngxi_tcpDriver);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_stack_push_driver", gResult);
        goto error;
    }

    gResult = globus_xio_stack_init(&ngcplXIOinfo.ngxi_gsiStack, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_stack_init", gResult);
        goto error;
    }
    gResult = globus_xio_stack_push_driver(ngcplXIOinfo.ngxi_gsiStack, ngcplXIOinfo.ngxi_tcpDriver);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_stack_push_driver", gResult);
        goto error;
    }

    gResult = globus_xio_stack_push_driver(ngcplXIOinfo.ngxi_gsiStack, ngcplXIOinfo.ngxi_gsiDriver);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_stack_push_driver", gResult);
        goto error;
    }

    gResult = globus_xio_stack_init(&ngcplXIOinfo.ngxi_fileStack, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_stack_init", gResult);
        goto error;
    }
    gResult = globus_xio_stack_push_driver(ngcplXIOinfo.ngxi_fileStack, ngcplXIOinfo.ngxi_fileDriver);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_stack_push_driver", gResult);
        goto error;
    }

    ngcplXIOinitialized = true;

    return NGEM_SUCCESS;
error:
    nResult = ngcpGlobusXIOfinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't finalize Globus XIO.\n");
    }

    return NGEM_FAILED;
}

/**
 * Finalize Globus XIO
 */
ngemResult_t
ngcpGlobusXIOfinalize(void)
{
    int result;
    globus_result_t gResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngcpGlobusXIOfinalize);

    log = ngemLogGetDefault();

    if (!ngcplXIOinitialized) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Globus XIO is not initialized.\n");
        return NGEM_FAILED;
    }

    /* Finalize Stacks */
#define NGCPL_XIO_STACK_DESTROY(stack) \
    if ((stack) != NULL) { \
        gResult = globus_xio_stack_destroy(stack); \
        if (gResult != GLOBUS_SUCCESS) { \
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName, \
                "globus_xio_stack_destroy", gResult); \
            ret = NGEM_FAILED; \
        } \
        (stack) = NULL; \
    }
    NGCPL_XIO_STACK_DESTROY(ngcplXIOinfo.ngxi_tcpStack);
    NGCPL_XIO_STACK_DESTROY(ngcplXIOinfo.ngxi_gsiStack);
    NGCPL_XIO_STACK_DESTROY(ngcplXIOinfo.ngxi_fileStack);
#undef NGCPL_XIO_STACK_DESTROY

#define NGCPL_XIO_DRIVER_UNLOAD(driver) \
    if ((driver) != NULL) { \
        gResult = globus_xio_driver_unload(driver); \
        if (gResult != GLOBUS_SUCCESS) { \
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName, \
                "globus_xio_driver_unload", gResult); \
            ret = NGEM_FAILED; \
        } \
        (driver) = NULL; \
    }

    /* Initialize Drivers */
    NGCPL_XIO_DRIVER_UNLOAD(ngcplXIOinfo.ngxi_tcpDriver);
    NGCPL_XIO_DRIVER_UNLOAD(ngcplXIOinfo.ngxi_gsiDriver);
    NGCPL_XIO_DRIVER_UNLOAD(ngcplXIOinfo.ngxi_fileDriver);
#undef NGCPL_XIO_DRIVER_UNLOAD

    if (ngcplXIOactivated) {
        /* Activate */
        result = globus_module_deactivate(GLOBUS_XIO_MODULE);
        if (result != GLOBUS_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't deactivate Globus XIO.\n");
            ret = NGEM_FAILED;
        }
        ngcplXIOactivated = false;
    }

    ngcplXIOinitialized = false;

    return ret;
}

/**
 * Print log for Globus error.
 */
void
ngcpLogGlobusError(
    ngLog_t *log,
    const char *cat,
    const char *funcName,
    const char *globusFunc,
    globus_result_t gResult)
{
    globus_object_t *object = NULL;
    globus_object_t *curObj;
    char *msg = NULL;
    char *string = NULL;

    /* Get the error object */
    object = globus_error_peek(gResult);
    if (object != NULL) {
        for (curObj = object; curObj != NULL; curObj = globus_error_get_cause(curObj)) {
            /* Get the error message */
            string = globus_object_printable_to_string(curObj);
            if (string != NULL) {
                msg = string;
            } else {
                msg = "Unkown error";
            }
            ngLogFatal(log, cat, funcName, "%s: %s.\n", globusFunc, msg);

            /* Deallocate error message */
            globus_libc_free(string);
            string = NULL;

            /* Get next object */
        }
    } else {
        ngLogFatal(log, cat, funcName, "%s: Unkown error.\n", globusFunc);
    }

    return;
}

/**
 * Print log for GSS error.
 */
void
ngcpLogGSSerror(
    ngLog_t *log,
    const char *cat,
    const char *func_name,
    const char *gss_func,
    OM_uint32 majorErrorStatus,
    OM_uint32 minorErrorStatus)
{
    OM_uint32 major_status;
    OM_uint32 minor_status;
    gss_buffer_desc buf = GSS_C_EMPTY_BUFFER;
    unsigned int msg_context = 0;
    NGEM_FNAME(ngcpLogGSSerror);

#define NGCPL_GSS_ERROR_PRINT(type, code)                                     \
    msg_context = 0;                                                          \
    do {                                                                      \
        major_status = gss_display_status(&minor_status, code, type,          \
            GSS_C_NULL_OID, &msg_context, &buf);                              \
        if (GSS_ERROR(major_status)) {                                        \
            ngLogError(log, cat, fName,                                       \
                "Can't get error message.\n");                                \
            goto error;                                                       \
        }                                                                     \
        ngLogError(log, cat, func_name,                                       \
                "%s: %.*s\n", gss_func, (int)buf.length, (char *)buf.value);  \
        major_status = gss_release_buffer(&minor_status, &buf);               \
        buf.value = NULL;                                                     \
        buf.length = 0;                                                       \
        if (GSS_ERROR(major_status)) {                                        \
            ngLogError(log, cat, fName,                                       \
                "Can't release error message.\n");                            \
            goto error;                                                       \
        }                                                                     \
    } while(msg_context != 0);

    NGCPL_GSS_ERROR_PRINT(GSS_C_GSS_CODE, majorErrorStatus);
    NGCPL_GSS_ERROR_PRINT(GSS_C_MECH_CODE, minorErrorStatus);
#undef NGCPL_GSS_ERROR_PRINT

    return;
error:
    ngLogError(log, cat, func_name,
        "%s failed: major status = %lu, minor status %lu.\n",
        gss_func, (unsigned long)majorErrorStatus, (unsigned long)minorErrorStatus);

    major_status = gss_release_buffer(&minor_status, &buf);
    if (GSS_ERROR(major_status)) {
        goto error;
    }

    return;
}

ngemResult_t
ngcpGlobusXIOfileOpen(
    globus_xio_handle_t *handlePtr,
    const char *filename)
{
    globus_result_t gResult;
    globus_xio_handle_t handle = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngcpGlobusXIOfileOpen);

    NGEM_ASSERT(handlePtr != NULL);

    log = ngemLogGetDefault();

    gResult = globus_xio_handle_create(&handle, ngcplXIOinfo.ngxi_fileStack);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_handle_create", gResult);
        goto error;
    }

    gResult = globus_xio_open(handle, filename, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_open", gResult);
        goto error;
    }

    *handlePtr = handle;

    return NGEM_SUCCESS;
error:

    if (handle != NULL) {
        gResult = globus_xio_close(handle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
        }
    }

    return NGEM_FAILED;
}

ngemResult_t
ngcpGlobusXIOconnect(
    globus_xio_handle_t *handlePtr,
    const char *contactString,
    ngcpCommunicationSecurity_t commSecurity,
    unsigned int commAuth,
    bool tcpNodelay)
{
    globus_result_t gResult;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_SUCCESS;
    globus_xio_handle_t handle = NULL;
    globus_xio_attr_t attr = NULL;
    globus_xio_stack_t stack;
    ngLog_t *log;
    bool ok = false;
    globus_bool_t tmp;
    NGEM_FNAME(ngcpGlobusXIOconnect);

    NGEM_ASSERT(handlePtr != NULL);

    log = ngemLogGetDefault();

    /* Socket */
    stack = ngcplXIOgetStack(commSecurity);

    /* attribute */
    gResult = globus_xio_attr_init(&attr);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_attr_init", gResult);
        ret = NGEM_FAILED;
        goto finalize;
    }

    tmp = (tcpNodelay)?GLOBUS_TRUE:GLOBUS_FALSE;
    gResult = globus_xio_attr_cntl(
        attr, ngcplXIOinfo.ngxi_tcpDriver,
        GLOBUS_XIO_TCP_SET_NODELAY, tmp);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_attr_cntl(GLOBUS_XIO_TCP_SET_NODELAY)", gResult);
        ret = NGEM_FAILED;
        goto finalize;
    }

    nResult = ngcplXIOattrSetSecurity(attr, commSecurity);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't set security attributes.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    /* Connect */
    gResult = globus_xio_handle_create(&handle, stack);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_handle_create", gResult);
        ret = NGEM_FAILED;
        goto finalize;
    }

    gResult = globus_xio_open(handle, contactString, attr);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_handle", gResult);
        ret = NGEM_FAILED;
        goto finalize;
    }

    if (commSecurity != NGCP_COMMUNICATION_SECURITY_NONE) {
        nResult = ngcpGlobusXIOcheckPeerName(handle, commAuth, &ok);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't check peer name.\n");
            ret = NGEM_FAILED;
            goto finalize;
        }
        if (!ok) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Target name of peer credential is not authorized.\n");
            ret = NGEM_FAILED;
            goto finalize;
        }
    }

finalize:
    if (attr != NULL) {
        gResult = globus_xio_attr_destroy(attr);
        attr = NULL;
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_destroy", gResult);
            ret = NGEM_FAILED;
        }
        attr = NULL;
    }

    if (ret != NGEM_SUCCESS) {
        /* Failed */
        if (handle != NULL) {
            gResult = globus_xio_close(handle, NULL);
            if (gResult != GLOBUS_SUCCESS) {
                ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                    "globus_xio_close", gResult);
            }
        }
    } else {
        /* SUCCESS */
        *handlePtr = handle;
    }

    return ret;
}

ngemResult_t
ngcpGlobusXIOcreateListener(
    globus_xio_server_t *serverPtr,
    ngcpPortRange_t range,
    ngcpCommunicationSecurity_t commSecurity,
    bool tcpNodelay)
{
    globus_result_t gResult;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_SUCCESS;
    globus_xio_server_t server = NULL;
    globus_xio_attr_t attr = NULL;
    globus_xio_stack_t stack;
    ngLog_t *log;
    globus_bool_t tmp;
    NGEM_FNAME(ngcpGlobusXIOcreateListener);

    NGEM_ASSERT(serverPtr != NULL);

    log = ngemLogGetDefault();

    /* Socket */
    stack = ngcplXIOgetStack(commSecurity);

    /* attribute */
    gResult = globus_xio_attr_init(&attr);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_attr_init", gResult);
        ret = NGEM_FAILED;
        goto finalize;
    }

    nResult = ngcplXIOattrSetSecurity(attr, commSecurity);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't set security attributes.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    tmp = (tcpNodelay)?GLOBUS_TRUE:GLOBUS_FALSE;
    gResult = globus_xio_attr_cntl(
        attr, ngcplXIOinfo.ngxi_tcpDriver,
        GLOBUS_XIO_TCP_SET_NODELAY, tmp);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_attr_cntl(GLOBUS_XIO_TCP_SET_NODELAY)", gResult);
        ret = NGEM_FAILED;
        goto finalize;
    }

    if ((range.ngpr_min > 0) || (range.ngpr_max > 0)) {
        gResult = globus_xio_attr_cntl(attr, ngcplXIOinfo.ngxi_tcpDriver,
            GLOBUS_XIO_TCP_SET_LISTEN_RANGE, 
            (int)range.ngpr_min, (int)range.ngpr_max);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_cntl(GLOBUS_XIO_TCP_SET_LISTEN_RANGE)", gResult);
            ret = NGEM_FAILED;
            goto finalize;
        }
    }

    /* Listen */
    gResult = globus_xio_server_create(&server, attr, stack);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_server_create", gResult);
        ret = NGEM_FAILED;
        goto finalize;
    }

finalize:
    if (attr != NULL) {
        gResult = globus_xio_attr_destroy(attr);
        attr = NULL;
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_destroy", gResult);
            ret = NGEM_FAILED;
        }
        attr = NULL;
    }

    if (ret != NGEM_SUCCESS) {
        /* Failed */
        if (server != NULL) {
            gResult = globus_xio_server_close(server);
            if (gResult != GLOBUS_SUCCESS) {
                ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                    "globus_xio_server_close", gResult);
            }
        }
    } else {
        /* SUCCESS */
        *serverPtr = server;
    }
    return ret;
}

/**
 * Check Peer name
 */
ngemResult_t
ngcpGlobusXIOcheckPeerNameByGridmap(
    globus_xio_handle_t handle,
    bool *ok,
    uid_t *uid,
    gid_t *gid)
{
    gss_buffer_desc buf = GSS_C_EMPTY_BUFFER;
    gss_name_t peer_name = GSS_C_NO_NAME;
    gss_OID oid = GSS_C_NO_OID;
    char *user_name = NULL;
    char *display_name = NULL;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    int result;
    globus_result_t gResult;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_FAILED;
    ngLog_t *log;
    NGEM_FNAME(ngcpGlobusXIOcheckPeerNameByGridmap);

    NGEM_ASSERT(handle != NULL);
    NGEM_ASSERT(uid != NULL);
    NGEM_ASSERT(gid != NULL);
    NGEM_ASSERT(ok != NULL);

    log = ngemLogGetDefault();

    *uid = -1;

    gResult = globus_xio_handle_cntl(handle, ngcplXIOinfo.ngxi_gsiDriver,
       GLOBUS_XIO_GSI_GET_PEER_NAME,  &peer_name);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_cntl(GLOBUS_XIO_GSI_GET_PEER_NAME)", gResult);
        goto finalize;
    }
    
    major_status = gss_display_name(&minor_status, peer_name, &buf, &oid);
    if (GSS_ERROR(major_status)) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_display_name", major_status, minor_status);
        goto finalize;
    }

    display_name = ngiMalloc(buf.length+1, log, NULL);
    if (display_name == NULL) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't allocate storage for the string.\n");
        goto finalize;
    }

    strncpy(display_name, (char *)buf.value, buf.length);
    display_name[buf.length] = '\0';
    
    result = globus_gss_assist_gridmap(display_name, &user_name);
    if (result != 0) {
        ngLogInfo(log, NGCP_LOGCAT_GT, fName,
            "Can't get user name from subject."
            " There is not subject \"%s\" in the gridmap file or "
            "the process reading gridmap file is failed\n",
            display_name);
    } else {
        nResult = ngcplGetUIDfromName(user_name, uid, gid);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't get uid from name.\n");
            goto finalize;
        }

        if (oid != GSS_C_NO_OID) {
            ngLogInfo(log, NGCP_LOGCAT_GT, fName, "OID %s\n", (char *)oid->elements);
        }
        ngLogInfo(log, NGCP_LOGCAT_GT, fName, "Subject   %s\n", display_name);
        ngLogInfo(log, NGCP_LOGCAT_GT, fName, "User name %s\n", user_name);

        *ok = true;
    }
    ret = NGEM_SUCCESS;
finalize:

    globus_free(user_name);
    ngiFree(display_name, log, NULL);

    if (buf.value != NULL) {
        major_status = gss_release_buffer(&minor_status, &buf);
        if (GSS_ERROR(major_status)) {
            ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
                "gss_release_buffer", major_status, minor_status);
        }
        buf.value = NULL;
    }
    return ret;
}
  
ngemResult_t
ngcpGlobusXIOpeerNameIsSelf(
    globus_xio_handle_t handle,
    bool *isSelf)
{
    return ngcplGlobusXIOcheckPeerNameSelf(handle, isSelf);
}

ngemResult_t
ngcpGlobusXIOcheckPeerName(
    globus_xio_handle_t handle,
    unsigned int auth,
    bool *ok) 
{
    bool auth_ok = false;
    uid_t uid;
    gid_t gid;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngcpGlobusXIOcheckPeerName);

    log = ngemLogGetDefault();

    NGEM_ASSERT(handle != NULL);
    NGEM_ASSERT(auth != 0);
    NGEM_ASSERT(ok != NULL);

    if ((auth & NGCP_COMMUNICATION_AUTH_SELF) != 0) {
        if (getuid() == 0) {
            nResult = ngcpGlobusXIOcheckPeerNameByGridmap(
                handle, &auth_ok, &uid, &gid);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCP_LOGCAT_GT, fName,
                    "Can't check peer name by gridmap.\n");
                return NGEM_FAILED;
            }
        } else {
            nResult = ngcplGlobusXIOcheckPeerNameSelf(
                handle, &auth_ok);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCP_LOGCAT_GT, fName,
                    "Can't check peer name by self.\n");
                return NGEM_FAILED;
            }
        }
    }

    if ((!auth_ok) && (auth & NGCP_COMMUNICATION_AUTH_HOST) != 0) {
        nResult = ngcplGlobusXIOcheckPeerNameHost(
            handle, &auth_ok);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't check peer name by self.\n");
            return NGEM_FAILED;
        }
    }
    *ok = auth_ok;

    return NGEM_SUCCESS;
}

static ngemResult_t
ngcplGlobusXIOcheckPeerNameHost(
    globus_xio_handle_t handle,
    bool *isHost)
{
    gss_name_t peer_name = GSS_C_NO_NAME;
    gss_name_t hostSubject = GSS_C_NO_NAME;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    ngLog_t *log;
    globus_xio_contact_t contactInfo;
    bool contactInfoGotten = false;
    int equal;
    ngemResult_t ret = NGEM_SUCCESS;
    char *contactString = NULL;
    globus_result_t gResult;
    NGEM_FNAME(ngcplGlobusXIOcheckPeerNameHost);

    log = ngemLogGetDefault();

    gResult = globus_xio_handle_cntl(handle, ngcplXIOinfo.ngxi_gsiDriver,
       GLOBUS_XIO_GSI_GET_PEER_NAME,  &peer_name);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_cntl(GLOBUS_XIO_GSI_GET_PEER_NAME)", gResult);
        goto finalize;
    }

    gResult = globus_xio_handle_cntl(handle, ngcplXIOinfo.ngxi_tcpDriver,
        GLOBUS_XIO_TCP_GET_REMOTE_CONTACT, &contactString);
    if(gResult != GLOBUS_SUCCESS)
    {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_handle_cntl(GLOBUS_XIO_TCP_GET_REMOTE_CONTACT)",
            gResult);
        goto finalize;
    }

    gResult = globus_xio_contact_parse(&contactInfo, contactString);
    if(gResult != GLOBUS_SUCCESS)
    {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_contact_parse", gResult);
        goto finalize;
    }
    contactInfoGotten = true;

    gResult = globus_gss_assist_authorization_host_name(
        contactInfo.host, &hostSubject);
    if(gResult != GLOBUS_SUCCESS)
    {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_contact_parse", gResult);
        goto finalize;
    }
    major_status = gss_compare_name( 
        &minor_status, peer_name, hostSubject, &equal);
    if (GSS_ERROR(major_status)) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_compare_name", major_status, minor_status);
        goto finalize;
    }

    if (equal != 0) {
        ngLogInfo(log, NGCP_LOGCAT_GT, fName,
            "Authorization: OK\n");
    } else {
        ngLogInfo(log, NGCP_LOGCAT_GT, fName,
            "Authorization: Failed\n");
    }

    *isHost = equal;
    ret = NGEM_SUCCESS;
finalize:

    if (contactInfoGotten) {
        globus_xio_contact_destroy(&contactInfo);
    }

    if (hostSubject != GSS_C_NO_NAME) {
        major_status = gss_release_name(&minor_status, &hostSubject);
        if (GSS_ERROR(major_status)) {
            ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
                "gss_release_name", major_status, minor_status);
        }
        hostSubject = GSS_C_NO_NAME;
    }
    if (contactString != NULL) {
        globus_free(contactString);
        contactString = NULL;
    }
    return ret; 
}


static ngemResult_t
ngcplGlobusXIOcheckPeerNameSelf(
    globus_xio_handle_t handle,
    bool *isSelf)
{
    gss_name_t peer_name = GSS_C_NO_NAME;
    gss_name_t local_name = GSS_C_NO_NAME;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    globus_result_t gResult;
    ngLog_t *log;
    int equal;
    NGEM_FNAME(ngcplGlobusXIOcheckPeerNameSelf);

    NGEM_ASSERT(handle != NULL);
    NGEM_ASSERT(isSelf != NULL);

    log = ngemLogGetDefault();

    *isSelf = false;

    gResult = globus_xio_handle_cntl(handle, ngcplXIOinfo.ngxi_gsiDriver,
       GLOBUS_XIO_GSI_GET_PEER_NAME,  &peer_name);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_cntl(GLOBUS_XIO_GSI_GET_PEER_NAME)", gResult);
        return NGEM_FAILED;
    }

    gResult = globus_xio_handle_cntl(handle, ngcplXIOinfo.ngxi_gsiDriver,
       GLOBUS_XIO_GSI_GET_LOCAL_NAME, &local_name);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_cntl(GLOBUS_XIO_GSI_GET_LOCAL_NAME)", gResult);
        return NGEM_FAILED;
    }

    major_status = gss_compare_name ( 
        &minor_status, local_name, peer_name, &equal);
    if (GSS_ERROR(major_status)) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_compare_name", major_status, minor_status);
        return NGEM_FAILED;
    }
    if (equal != 0) {
        *isSelf = true;
        ngLogInfo(log, NGCP_LOGCAT_GT, fName,
            "Authorization: OK.\n");
    } else {
        ngLogInfo(log, NGCP_LOGCAT_GT, fName,
            "Authorization: Failed.\n");
    }

    return NGEM_SUCCESS;
}

/**
 * Get User ID from name.
 */
static ngemResult_t
ngcplGetUIDfromName(
    char *name,
    uid_t *uid,
    gid_t *gid)
{
    char *buf= NULL;
    int result;
    ngemResult_t ret = NGEM_FAILED;
    size_t buflen= 1024;
    struct passwd pwbuf;
    struct passwd *pwbufp = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngcplGetUIDfromName);

    log = ngemLogGetDefault();

    NGEM_ASSERT(uid != NULL);
    NGEM_ASSERT(gid != NULL);

    *uid = 0;
    *gid = 0;

    do {
        ngLogDebug(log, NGCP_LOGCAT_GT, fName,
            "allocate %lu\n", (unsigned long)buflen);
        
        buf = ngiMalloc(buflen, log, NULL);
        if (buf == NULL) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't allocate storage for passwd entry.\n");
            goto finalize;
        }

        errno = 0;
        result = getpwnam_r(name, &pwbuf, buf, buflen, &pwbufp);
        if (result != 0) {
            if (result != ERANGE) {
                ngLogError(log, NGCP_LOGCAT_GT, fName,
                    "getpwnam_r: %s\n", strerror(result));
                goto finalize;
            }
        }
        if (pwbufp != NULL) {
            *uid = pwbufp->pw_uid;
            *gid = pwbufp->pw_gid;
        }
        ngiFree(buf, log, NULL);
        buf = NULL;
        buflen *= 2;
    } while (result == ERANGE);

    ret = NGEM_SUCCESS;
finalize:
    ngiFree(buf, log, NULL);
    buf = NULL;

    return NGEM_SUCCESS;
}

NGEM_DEFINE_OPTION_ANALYZER_SET_ENUM(
    ngcpOptionAnalyzerSetCommunicationSecurity,
    ngcpCommunicationSecurity_t,
    ngcpCommunicationSecurityString,
    NGI_NELEMENTS(ngcpCommunicationSecurityString))

ngemResult_t
ngcpOptionsAnalyzerSetPortRange(
    ngemOptionAnalyzer_t *oa,
    ngcpPortRange_t *portRange,
    char *name,
    char *value)
{
    long min;
    long max;
    char *endp;
    ngLog_t *log;
    NGEM_FNAME(ngcpOptionsAnalyzerSetPortRange);

    errno = 0;

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCP_LOGCAT_GT, fName, "Called\n");

    min = strtol(value, &endp, 10);
    if (errno != 0) {
        ngemOptionSyntaxError(oa, fName,
            "strtol: %s.\n", strerror(errno));
        return NGEM_SUCCESS;
    }
    if ((min < 0) && (min > USHRT_MAX)) {
        ngemOptionSyntaxError(oa, fName,
            "%ld: too small or large.\n", min);
        return NGEM_SUCCESS;
    }

    switch (*endp) {
    case '\0':
        max = min;
        break;
    case '-':
        max = strtol(endp+1, &endp, 10);
        if (errno != 0) {
            ngemOptionSyntaxError(oa, fName,
                "strtol: %s.\n", strerror(errno));
            return NGEM_SUCCESS;
        }
        if ((max < min) && (max > USHRT_MAX)) {
            ngemOptionSyntaxError(oa, fName,
                "%ld: too small or large.\n", max);
            return NGEM_SUCCESS;
        }
        if (*endp == '\0') {
            break;
        }
        /* THROUGH */
    default:
        ngemOptionSyntaxError(oa, fName,
            "\"%s\" includes invalid character as number.\n", value);
        return NGEM_SUCCESS;
    }

    portRange->ngpr_min = min;
    portRange->ngpr_max = max;

    return NGEM_SUCCESS;
}


static globus_xio_stack_t
ngcplXIOgetStack(
    ngcpCommunicationSecurity_t commSecurity)
{
    NGEM_FNAME_TAG(ngcplXIOgetStack);

    if (commSecurity == NGCP_COMMUNICATION_SECURITY_NONE) {
        return ngcplXIOinfo.ngxi_tcpStack;
    } else {
        return ngcplXIOinfo.ngxi_gsiStack;
    }
}

static ngemResult_t
ngcplXIOattrSetSecurity(
    globus_xio_attr_t attr,
    ngcpCommunicationSecurity_t commSecurity)
{
    globus_result_t gResult;
    globus_xio_gsi_protection_level_t protect =
        GLOBUS_XIO_GSI_PROTECTION_LEVEL_PRIVACY;
    ngLog_t *log;
    NGEM_FNAME(ngcplXIOattrSetSecurity);

    log = ngemLogGetDefault();

    if (commSecurity != NGCP_COMMUNICATION_SECURITY_NONE) {
        /* Credential */
        gResult = globus_xio_attr_cntl(attr, ngcplXIOinfo.ngxi_gsiDriver,
            GLOBUS_XIO_GSI_SET_CREDENTIAL, GSS_C_NO_CREDENTIAL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_cntl(GLOBUS_XIO_GSI_SET_CREDENTIAL)",
                gResult);
            goto error;
        }

        gResult = globus_xio_attr_cntl(attr, ngcplXIOinfo.ngxi_gsiDriver,
            GLOBUS_XIO_GSI_SET_AUTHORIZATION_MODE,
            GLOBUS_XIO_GSI_NO_AUTHORIZATION);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_cntl(GLOBUS_XIO_GSI_SET_AUTHORIZATION_MODE)",
                gResult);
            goto error;
        }
        /* Protect Level */
        switch (commSecurity) {
        case NGCP_COMMUNICATION_SECURITY_IDENTITY:
            protect = GLOBUS_XIO_GSI_PROTECTION_LEVEL_NONE;
            break;
        case NGCP_COMMUNICATION_SECURITY_INTEGRITY:
            protect = GLOBUS_XIO_GSI_PROTECTION_LEVEL_INTEGRITY;
            break;
        case NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY:
            protect = GLOBUS_XIO_GSI_PROTECTION_LEVEL_PRIVACY;
            break;
        case NGCP_COMMUNICATION_SECURITY_NONE:
        default:
            NGEM_ASSERT_NOTREACHED();
        }

        gResult = globus_xio_attr_cntl(attr, ngcplXIOinfo.ngxi_gsiDriver,
            GLOBUS_XIO_GSI_SET_PROTECTION_LEVEL, protect);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_cntl(GLOBUS_XIO_GSI_SET_PROTECTION_LEVEL)",
                gResult);
            goto error;
        }

        /* Not delegate */
        gResult = globus_xio_attr_cntl(attr, ngcplXIOinfo.ngxi_gsiDriver,
            GLOBUS_XIO_GSI_SET_DELEGATION_MODE,
            GLOBUS_XIO_GSI_DELEGATION_MODE_NONE);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_cntl(GLOBUS_XIO_GSI_SET_DELEGATION_MODE)",
                gResult);
            goto error;
        }
    }

    return NGEM_SUCCESS;
error:
    return NGEM_FAILED;
}


/**
 * Invoke new Process.
 * @param gtStdio Pointer for returning pipes of new process's stdin, stdout, stderr.
 * @param argv    Null terminated array which contains arguments used by new process.
 * @return New process's process id.
 */
pid_t
ngcpXIOpopenArgv(
    ngcpXIOstandardIO_t *gtStdio,
    char **argv,
    uid_t uid,
    gid_t gid,
    ngemEnvironment_t *env)
{
    ngLog_t *log = NULL;
    pid_t pid = -1;
    pid_t wpid= -1;
    ngemStandardIO_t stdio;
    globus_xio_handle_t handles[3] = {NULL, NULL, NULL};/* in, out, err */
    int fds[3];
    int i;
    globus_result_t gResult;
    ngemResult_t nResult;
    int result;
    NGEM_FNAME(ngcpXIOpopenArgv);

    log = ngemLogGetDefault();

    pid = ngemPopenArgvEx(&stdio, argv, uid, gid, env);
    if (pid < 0) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't invoke the process.\n");
        goto error;
    }

    i = 0;
    fds[i++] = stdio.ngsio_in;
    fds[i++] = stdio.ngsio_out;
    fds[i++] = stdio.ngsio_error;
    NGEM_ASSERT(i == 3);

    for (i = 0;i < 3;++i) {
        nResult = ngcplXIOhandleCreateFromFD(&handles[i], fds[i]);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't create Globus XIO handle from file descriptor.\n");
            goto error;
        }
        fds[i] = -1;
    }

    i = 0;
    gtStdio->ngsio_in  = handles[i++];
    gtStdio->ngsio_out = handles[i++];
    gtStdio->ngsio_err = handles[i++];
    NGEM_ASSERT(i == 3);

    return pid;
error:
    for (i = 0;i < 3;++i) {
        if (handles[i] != NULL) {
            gResult = globus_xio_close(handles[i], NULL);
            if (gResult != GLOBUS_SUCCESS) {
                ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                    "globus_xio_close", gResult);
            }
            handles[i] = NULL;
        }
    }
    if (pid >= 0) {
        for (i = 0;i < 3;++i) {
            if (fds[i] >= 0) {
                result = close(fds[i]);
                if (result < 0) {
                    ngLogError(log, NGCP_LOGCAT_GT, fName,
                        "Can't close the pipe: %s.\n", strerror(errno));
                }
                fds[i] = -1;
            }
        }

        do {
            wpid = waitpid(pid, NULL, 0);
        } while ((wpid < 0) && (errno == EINTR));
        if (wpid < 0) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "wait: %s.\n", strerror(errno));
        }
        pid = -1;
    }

    return (pid_t)-1;
}

static ngemResult_t
ngcplXIOhandleCreateFromFD(
    globus_xio_handle_t *pHandle,
    int fd)
{
    globus_result_t gResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    globus_xio_handle_t handle = NULL;
    globus_xio_attr_t attr = NULL;
    NGEM_FNAME(ngcplXIOhandleCreateFromFD);

    NGEM_ASSERT(pHandle != NULL);
    NGEM_ASSERT(fd >= 0);

    log = ngemLogGetDefault();

    /* attribute */
    gResult = globus_xio_attr_init(&attr);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_attr_init", gResult);
        goto finalize;
    }

    gResult = globus_xio_attr_cntl(attr,
        ngcplXIOinfo.ngxi_fileDriver, GLOBUS_XIO_FILE_SET_HANDLE, fd);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_attr_cntl", gResult);
        goto finalize;
    }

    gResult = globus_xio_handle_create(&handle, ngcplXIOinfo.ngxi_fileStack);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_handle_create", gResult);
        goto finalize;
    }

    gResult = globus_xio_open(handle, NULL, attr);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "globus_xio_open", gResult);
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:

    if (attr != NULL) {
        gResult = globus_xio_attr_destroy(attr);
        attr = NULL;
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_attr_destroy", gResult);
            ret = NGEM_FAILED;
        }
        attr = NULL;
    }

    if ((ret != NGEM_SUCCESS) && (handle != NULL)) {
        gResult = globus_xio_close(handle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
        }
        handle = NULL;
    }
    if (ret == NGEM_SUCCESS) {
        NGEM_ASSERT(handle != NULL);
        *pHandle = handle;
    }

    return ret;
}

ngemResult_t
ngcpGlobusXIOinitDelegation(
    globus_xio_handle_t handle)
{
    globus_result_t gResult;
    OM_uint32 minor_status;
    OM_uint32 major_status;
    gss_cred_id_t cred = GSS_C_NO_CREDENTIAL;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    NGEM_FNAME(ngcpGlobusXIOinitDelegation);

    log = ngemLogGetDefault();

    major_status = gss_acquire_cred (&minor_status,
        GSS_C_NO_NAME, 0, GSS_C_NO_OID_SET, GSS_C_BOTH,
        &cred, NULL, NULL); 
    if (GSS_ERROR(major_status)) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_release_cred", major_status, minor_status);
        goto finalize;
    }

    gResult = globus_xio_handle_cntl(handle,
        ngcplXIOinfo.ngxi_gsiDriver, GLOBUS_XIO_GSI_INIT_DELEGATION,
        GSS_C_NO_CREDENTIAL, GSS_C_NO_OID_SET, GSS_C_NO_BUFFER_SET, 0);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_release_cred", major_status, minor_status);
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:
    if (cred != GSS_C_NO_CREDENTIAL) {
        major_status = gss_release_cred(&minor_status, &cred);
        if (GSS_ERROR(major_status)) {
            ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
                "gss_release_cred", major_status, minor_status);
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

globus_result_t
ngcpGlobusXIOregisterInitDelegation(
    globus_xio_handle_t handle,
    globus_xio_gsi_delegation_init_callback_t callback,
    void *userArg)
{
    return globus_xio_handle_cntl(handle,
        ngcplXIOinfo.ngxi_gsiDriver, GLOBUS_XIO_GSI_REGISTER_INIT_DELEGATION,
        GSS_C_NO_CREDENTIAL, GSS_C_NO_OID_SET, GSS_C_NO_BUFFER_SET, 0, callback, userArg);
}

globus_result_t
ngcpGlobusXIOacceptDelegation(
    globus_xio_handle_t handle,
    gss_cred_id_t *cred)
{
    return globus_xio_handle_cntl(handle,
        ngcplXIOinfo.ngxi_gsiDriver, GLOBUS_XIO_GSI_ACCEPT_DELEGATION,
        cred, GSS_C_NO_OID_SET, GSS_C_NO_BUFFER_SET, 0, NULL);
}

globus_result_t
ngcpGlobusXIOregisterAcceptDelegation(
    globus_xio_handle_t handle,
    globus_xio_gsi_delegation_accept_callback_t callback,
    void *userArg) 
{
    return globus_xio_handle_cntl(handle,
        ngcplXIOinfo.ngxi_gsiDriver, GLOBUS_XIO_GSI_REGISTER_ACCEPT_DELEGATION,
        GSS_C_NO_OID_SET, GSS_C_NO_BUFFER_SET, 0, callback, userArg);
}

ngemResult_t
ngcpGSSexportCred(
    gss_cred_id_t cred,
    char **proxyfile,
    uid_t uid,
    gid_t gid)
{
    char *filename;
    OM_uint32 option = 0;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    gss_buffer_desc buf = GSS_C_EMPTY_BUFFER;
    FILE *fp = NULL;
    size_t nw;
    ngemResult_t ret = NGEM_FAILED;
    int result;
    char *name = "X509_USER_PROXY=";
    char *fn = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngcpGSSexportCred);

    NGEM_ASSERT(proxyfile != NULL);

    log = ngemLogGetDefault();

    filename = *proxyfile;

    if (filename == NULL) {
        /* 0 = memory
         * 1 = file */
        option = 1;
    }

    major_status = gss_export_cred(&minor_status, cred, GSS_C_NO_OID, option, &buf);
    if (GSS_ERROR(major_status)) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_export_cred", major_status, minor_status);
        goto finalize;
    }

    if (filename == NULL) {
        ngLogInfo(log, NGCP_LOGCAT_GT, fName,
            "gss_export_cred returns \"%.*s\".\n", (int)buf.length, (char *)buf.value);
        if (strlen(name) >= buf.length) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't get proxy file name.\n");
            goto finalize;
        }
        if (!ngemStringCompare(name, -1, buf.value, strlen(name))) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't get proxy file name.\n");
            goto finalize;
        }

        fn = ngiStrndup((char *)buf.value + strlen(name), buf.length - strlen(name), log, NULL);
        if (fn == NULL) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't duplicate the string.\n");
            goto finalize;
        }
        if (getuid() == 0) {
            result = chown(fn, uid, gid);
            if (result < 0) {
                ngLogError(log, NGCP_LOGCAT_GT, fName,
                    "chown:%s\n", strerror(errno));
                goto finalize;
            }
        }
        *proxyfile = fn;
        fn = NULL;
    } else {
        fp = fopen(filename, "w");
        if (fp == NULL) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "fopen: %s\n", strerror(errno));
            goto finalize;
        }
        nw = fwrite(buf.value, 1, buf.length, fp);
        if (buf.length != nw) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "fwrite: %s\n", strerror(errno));
            goto finalize;
        }
    }

    ret = NGEM_SUCCESS;
finalize:
    ngiFree(fn, log, NULL);
    if (fp != NULL) {
        result = fclose(fp);
        if (result != 0) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "fclose: %s\n", strerror(errno));
        }
    }
    return ret;
}

bool
ngcpCredentialAvailable(void)
{
    OM_uint32 major_status;
    OM_uint32 minor_status;
    gss_cred_id_t cred = GSS_C_NO_CREDENTIAL;
    ngLog_t *log;
    NGEM_FNAME(ngcpCredentialAvailable);

    log = ngemLogGetDefault();
    
    major_status = gss_acquire_cred (&minor_status,
        GSS_C_NO_NAME, 0, GSS_C_NO_OID_SET, GSS_C_BOTH,
        &cred, NULL, NULL); 
    if (GSS_ERROR(major_status)) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_acquire_cred", major_status, minor_status);
        return false;
    }
    major_status = gss_release_cred(&minor_status, &cred);
    if (GSS_ERROR(major_status)) {
        ngcpLogGSSerror(log, NGCP_LOGCAT_GT, fName,
            "gss_release_cred", major_status, minor_status);
    }

    return true;
}
