/*
 * $RCSfile: nginReadNRF.c,v $ $Revision: 1.9 $ $Date: 2008/02/07 11:30:54 $
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

#include "xmlparse.h"
#include "ngemLog.h"
#include "nginReadNRF.h"

NGI_RCSID_EMBED("$RCSfile: nginReadNRF.c,v $ $Revision: 1.9 $ $Date: 2008/02/07 11:30:54 $")

#define NGINL_BUFFER_SIZE        8192

#define NGINL_NS_NRF "http://ninf.apgrid.org/2006/12/NinfGRemoteInformation"
#define NGINL_NS_RMI "http://ninf.apgrid.org/2006/12/RemoteMachineInformation"
#define NGINL_NS_REI "http://ninf.apgrid.org/2006/12/RemoteExecutableInformation"
#define NGINL_NS_RCI "http://ninf.apgrid.org/2006/12/RemoteClassInformation"

#define NGIL_TAG_REMOTE_EXECUTABLE_INFORMATION "RemoteExecutableInformation"

/**
 * Status for reading Ninf-G Remote Information File
 */
typedef enum nginlNRFstatus_e {
    NGINL_NRF_FIRST,
    NGINL_NRF_IN_NRF,
    NGINL_NRF_IN_EXECUTABLE,
    NGINL_NRF_IN_HOSTNAME,
    NGINL_NRF_END
} nginlNRFstatus_t;

/**
 * Information for XML namespace
 */
typedef struct nginlNamespace_s {
    char *ngn_prefix;
    char *ngn_uri;
    int   ngn_count;
} nginlNamespace_t;
NGEM_DECLARE_LIST_OF(nginlNamespace_t);

/**
 * Information for reading Ninf-G Remote Information File
 */
typedef struct nginlReadingInfo_s {
    XML_Parser                         ngri_parser;
    int                                ngri_depth;
    nginlNRFstatus_t                   ngri_status;
    NGEM_LIST_OF(nginlNamespace_t)     ngri_namespace;
    nginREIcontainer_t                *ngri_container;
    nginRemoteExecutableInformation_t *ngri_current;
    ngemStringBuffer_t                 ngri_xml;
    ngemStringBuffer_t                 ngri_key;
    bool                               ngri_error;
    int                                ngri_nsDeclCount;
} nginlReadingInfo_t;

/* File local functions */
static nginRemoteExecutableInformation_t *
nginlRemoteExecutableInformationCreate(void);

static ngemResult_t
nginlRemoteExecutableInformationDestroy(
    nginRemoteExecutableInformation_t *);

static ngemResult_t
nginlReadingInfoInitialize(
    nginlReadingInfo_t *, XML_Parser, const char *);

static ngemResult_t nginlReadingInfoFinalize(nginlReadingInfo_t *);

static const char *
nginlReadingInfoGetPrefix(nginlReadingInfo_t *, const char *);

static const char *
nginlReadingInfoGetPrefixN(nginlReadingInfo_t *, const char *, size_t);

static ngemResult_t
nginlReadingInfoPopNamespace(nginlReadingInfo_t *, const char *);

static ngemResult_t
nginlReadingInfoPushNamespace(nginlReadingInfo_t *, const char *, const char *);

static ngemResult_t
nginlPrintElementStart(nginlReadingInfo_t *, const XML_Char *, const XML_Char **);

static ngemResult_t
nginlPrintElementEnd(nginlReadingInfo_t *, const XML_Char *);

static ngemResult_t nginlREIcontainerAppend(
    nginREIcontainer_t *, nginRemoteExecutableInformation_t *);

/* Callback Functions */
static void nginlElementStart(void *, const XML_Char *, const XML_Char **);
static void nginlElementEnd(void *, const XML_Char *);
static void nginlNSstart(void *, const XML_Char *, const XML_Char *);
static void nginlNSend(void *, const XML_Char *);
static void nginlCharDataHandler(void *, const XML_Char *, int);
static void nginlDefaultHandler(void *, const XML_Char *, int);

/**
 * Read Ninf-G Remote information File
 */
nginREIcontainer_t *
nginReadNRF(
    const char *filename)
{
    XML_Parser xmlp;
    bool xmlpCreated = false;
    FILE *fp = NULL;
    size_t nread;
    void *buf = NULL;
    int result;
    nginREIcontainer_t *ret = NULL;
    nginlReadingInfo_t rInfo;
    bool rInfoInitialized = false;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(nginReadNRF);

    /* Initialize */
    xmlp = XML_ParserCreateNS(NULL, ':');
    if (!xmlp) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't create XML parser.\n");
        goto finalize;
    }
    xmlpCreated = true;

    XML_SetElementHandler(xmlp, nginlElementStart, nginlElementEnd);
    XML_SetDefaultHandler(xmlp, nginlDefaultHandler);
    XML_SetNamespaceDeclHandler(xmlp, nginlNSstart, nginlNSend);
    XML_SetCharacterDataHandler(xmlp, nginlCharDataHandler);

    XML_SetUserData(xmlp, &rInfo);

    nResult = nginlReadingInfoInitialize(&rInfo, xmlp, filename);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't initialize information for reading "
            "Ninf-G Remote Information File.\n");
        goto finalize;
    }
    rInfoInitialized = true;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "fopen(%s):%s\n", filename, strerror(errno));
        goto finalize;
    }

    /* Main process */
    while (feof(fp) == 0) {
        buf = XML_GetBuffer(xmlp, NGINL_BUFFER_SIZE);
        if (buf == NULL) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                  "Can't get buffer from XML parser.\n%s\n",
                  XML_ErrorString(XML_GetErrorCode(xmlp)));
            goto finalize;
        }

        nread = fread(buf, 1, NGINL_BUFFER_SIZE, fp);
        if (ferror(fp)) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "fread:%s\n", strerror(errno));
            goto finalize;
        }

        result = XML_ParseBuffer(xmlp, nread, feof(fp));
        if (result == 0) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Parse error at line %d:\n%s\n",
                XML_GetCurrentLineNumber(xmlp),
                XML_ErrorString(XML_GetErrorCode(xmlp)));
            goto finalize;
        }
        if (rInfo.ngri_error) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Error occurred while parsing file.\n");
            goto finalize;
        }
    }

    ret = rInfo.ngri_container;
    rInfo.ngri_container = NULL;

    /* Finalize */
finalize:
    if (fp != NULL) {
        result = fclose(fp);
        if (result != 0) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "fclose:%s\n", strerror(errno));
        }
        fp = NULL;
    }

    if (rInfoInitialized) {
        nResult = nginlReadingInfoFinalize(&rInfo);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't finalize information for reading "
                "Ninf-G Remote Information File.\n");
        }
        rInfoInitialized = false;
    }

    if (xmlpCreated) {
        XML_ParserFree(xmlp);
        xmlpCreated = false;
    }

    return ret;
}

/**
 * Remote Executable Information Container: Create
 */
nginREIcontainer_t *
nginREIcontainerCreate(
    const char *filename)
{
    nginREIcontainer_t *ret = NULL;
    ngLog_t *log;
    NGEM_FNAME(nginREIcontainerCreate);

    log = ngemLogGetDefault();

    ret = NGI_ALLOCATE(nginREIcontainer_t, log, NULL);
    if (ret == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't allocate storage for Remote Information container.\n");
        goto error;
    }
    
    ret->ngrc_filename = NULL;
    ret->ngrc_filename = ngiStrdup(filename, log, NULL);
    if (ret->ngrc_filename == NULL) {
        goto error;
    }

    NGEM_LIST_INITIALIZE(
        nginRemoteExecutableInformation_t, &ret->ngrc_reInfo);

    return ret;
error:
    if (ret != NULL) {
        ngiFree(ret->ngrc_filename, log, NULL);
        NGI_DEALLOCATE(nginREIcontainer_t, ret, log, NULL);
    }

    return NULL;
}

/**
 * Remote Executable Information Container: Destroy
 */
ngemResult_t
nginREIcontainerDestroy(
    nginREIcontainer_t *container)
{
    NGEM_LIST_ITERATOR_OF(nginRemoteExecutableInformation_t) it;
    nginRemoteExecutableInformation_t *item;
    ngemResult_t ret = NGEM_SUCCESS;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(nginREIcontainerDestroy);

    NGEM_ASSERT(container != NULL);

    log = ngemLogGetDefault();

    NGEM_LIST_ERASE_EACH(
        nginRemoteExecutableInformation_t, &container->ngrc_reInfo, it, item) {
        nResult = nginlRemoteExecutableInformationDestroy(item);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't destroy remote executable information.\n");
            ret = NGEM_FAILED;
        }
    }

    NGEM_LIST_FINALIZE(
        nginRemoteExecutableInformation_t, &container->ngrc_reInfo);

    ngiFree(container->ngrc_filename, log, NULL);
    NGI_DEALLOCATE(nginREIcontainer_t, container, log, NULL);

    return ret;
}

/**
 * Remote Executable Information Container: Find Remote Executable Information
 * has hostname and classname specified by arguments.
 */
nginRemoteExecutableInformation_t *
nginREIcontainerFind(
    nginREIcontainer_t *container,
    const char *hostname,
    const char *classname)
{
    NGEM_LIST_ITERATOR_OF(nginRemoteExecutableInformation_t) it;
    nginRemoteExecutableInformation_t *item;
    NGEM_FNAME_TAG(nginREIcontainerFind);

    NGEM_LIST_FOREACH(
        nginRemoteExecutableInformation_t, &container->ngrc_reInfo, it, item) {
        if ((strcmp(hostname,  item->ngrei_hostname)  == 0) &&
            (strcmp(classname, item->ngrei_classname) == 0)) {
            /* Found */
            return item;
        }
    }
    return NULL;
}

/**
 * Remote Executable Information Container: Erase Remote Executable Information
 * has hostname and classname specified by arguments.
 */
ngemResult_t
nginREIcontainerErase(
    nginREIcontainer_t *container,
    const char *hostname,
    const char *classname)
{
    NGEM_LIST_ITERATOR_OF(nginRemoteExecutableInformation_t) it;
    nginRemoteExecutableInformation_t *item;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(nginREIcontainerErase);

    log = ngemLogGetDefault();

    NGEM_LIST_FOREACH(
        nginRemoteExecutableInformation_t, &container->ngrc_reInfo, it, item) {
        if ((strcmp(hostname,  item->ngrei_hostname)  == 0) &&
            (strcmp(classname, item->ngrei_classname) == 0)) {
            /* Found */
            NGEM_LIST_ERASE(nginRemoteExecutableInformation_t, it);
            nResult = nginlRemoteExecutableInformationDestroy(item);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't destroy remote executable information.\n");
            }
            return nResult;
        }
    }
    return NGEM_FAILED;
}

/**
 * Remote Executable Information Container: if container is empty, return
 * value is true. Otherwise return value is false.
 */
bool
nginREIcontainerIsEmpty(
    nginREIcontainer_t *container)
{
    NGEM_FNAME_TAG(nginREIcontainerIsEmpty);

    return NGEM_LIST_IS_EMPTY(
        nginRemoteExecutableInformation_t, &container->ngrc_reInfo);
}

/**
 * Remote Executable Information Container: Append new Remote Executable
 * Information to container.
 */
static ngemResult_t
nginlREIcontainerAppend(
    nginREIcontainer_t *container,
    nginRemoteExecutableInformation_t *reInfo)
{
    nginRemoteExecutableInformation_t *exist = NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(nginlREIcontainerAppend);

    log = ngemLogGetDefault();

    /* Check Remote executable information */
    if (reInfo->ngrei_hostname  == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Hostname is not specified.\n");
        return NGEM_FAILED;
    }
    if (reInfo->ngrei_classname == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "classname is not specified.\n");
        return NGEM_FAILED;
    }
    if (reInfo->ngrei_xmlString == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "XML is empty.\n");
        return NGEM_FAILED;
    }

    exist = nginREIcontainerFind(container,
        reInfo->ngrei_hostname, reInfo->ngrei_classname);
    if (exist != NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "There is already information that have "
            "this hostname and classname.\n");
        return NGEM_FAILED;
    }
    
    nResult = NGEM_LIST_INSERT_TAIL(nginRemoteExecutableInformation_t,
        &container->ngrc_reInfo, reInfo);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't insert new Remote Executable Information to container.\n");
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Remote Executable Information: Create
 */
static nginRemoteExecutableInformation_t *
nginlRemoteExecutableInformationCreate(void)
{
    nginRemoteExecutableInformation_t *reInfo = NULL;
    ngLog_t *log;
    NGEM_FNAME(nginlRemoteExecutableInformationCreate);

    log = ngemLogGetDefault();

    reInfo = NGI_ALLOCATE(nginRemoteExecutableInformation_t, log, NULL);
    if (reInfo == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't allocate storage for Remote Executable Information.\n");
        return NULL;
    }
    reInfo->ngrei_hostname  = NULL;
    reInfo->ngrei_classname = NULL;
    reInfo->ngrei_xmlString = NULL;

    return reInfo;
}

/**
 * Remote Executable Information: Destroy
 */
static ngemResult_t
nginlRemoteExecutableInformationDestroy(
    nginRemoteExecutableInformation_t *reInfo)
{
    ngLog_t *log;
    NGEM_FNAME_TAG(nginlRemoteExecutableInformationDestroy);

    log = ngemLogGetDefault();

    ngiFree(reInfo->ngrei_hostname, log, NULL);
    ngiFree(reInfo->ngrei_classname, log, NULL);
    ngiFree(reInfo->ngrei_xmlString, log, NULL);

    NGI_DEALLOCATE(nginRemoteExecutableInformation_t, reInfo, log, NULL);

    return NGEM_SUCCESS;
}

/**
 * Reading Information: Initialize
 */
static ngemResult_t
nginlReadingInfoInitialize(
    nginlReadingInfo_t *rInfo,
    XML_Parser xmlp,
    const char *filename)
{
    bool listInitialized = false;
    bool sbKeyInitialized = false;
    bool sbXMLinitialized = false;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(nginlReadingInfoInitialize);

    log = ngemLogGetDefault();

    rInfo->ngri_parser    = xmlp;
    rInfo->ngri_depth     = 0;
    rInfo->ngri_status    = NGINL_NRF_FIRST;
    rInfo->ngri_container = NULL;
    rInfo->ngri_current   = NULL;
    rInfo->ngri_error     = false;
    rInfo->ngri_nsDeclCount = 0;;
    NGEM_LIST_INITIALIZE(nginlNamespace_t, &rInfo->ngri_namespace);
    listInitialized = true;

    nResult = ngemStringBufferInitialize(&rInfo->ngri_key);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't initialize string buffer.\n");
        goto error;
    }
    sbKeyInitialized = true;
    nResult = ngemStringBufferInitialize(&rInfo->ngri_xml);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't initialize string buffer.\n");
        goto error;
    }
    sbXMLinitialized = true;

    rInfo->ngri_container = nginREIcontainerCreate(filename);
    if (rInfo->ngri_container == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't create Remote Executable Information Container.\n");
        goto error;
    }

    return NGEM_SUCCESS;
error:
    if (rInfo->ngri_container != NULL) {
        nResult = nginREIcontainerDestroy(rInfo->ngri_container);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't destroy Remote Executable Information Container.\n");
        }
    }

    if (sbXMLinitialized) {
        ngemStringBufferFinalize(&rInfo->ngri_key);
        sbKeyInitialized = false;
    }

    if (sbKeyInitialized) {
        ngemStringBufferFinalize(&rInfo->ngri_xml);
        sbXMLinitialized = false;
    }

    if (listInitialized) {
        NGEM_LIST_FINALIZE(nginlNamespace_t, &rInfo->ngri_namespace);
        listInitialized = false;
    }
    return NGEM_FAILED;
}

/**
 * Reading Information: Finalize
 */
static ngemResult_t
nginlReadingInfoFinalize(
    nginlReadingInfo_t *rInfo)
{
    NGEM_LIST_ITERATOR_OF(nginlNamespace_t) it;
    nginlNamespace_t *item;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    NGEM_FNAME(nginlReadingInfoFinalize);

    log = ngemLogGetDefault();

/*    rInfo->ngri_parser = NULL; */
    rInfo->ngri_depth     = 0;
    rInfo->ngri_status    = NGINL_NRF_END;
    rInfo->ngri_current   = NULL;
    rInfo->ngri_error     = false;
    rInfo->ngri_container = NULL;

    ngemStringBufferFinalize(&rInfo->ngri_key);
    ngemStringBufferFinalize(&rInfo->ngri_xml);

    NGEM_LIST_ERASE_EACH(nginlNamespace_t, &rInfo->ngri_namespace, it, item) {
        ngiFree(item->ngn_prefix, log, NULL);
        ngiFree(item->ngn_uri, log, NULL);

        NGI_DEALLOCATE(nginlNamespace_t, item, log, NULL);
    }
    NGEM_LIST_FINALIZE(nginlNamespace_t, &rInfo->ngri_namespace);

    if (rInfo->ngri_container != NULL) {
        nResult = nginREIcontainerDestroy(rInfo->ngri_container);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't destroy Remote Executable Information Container.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

/**
 * Reading Information: get prefix of namespace from URI
 */
static const char *
nginlReadingInfoGetPrefix(
    nginlReadingInfo_t *rInfo,
    const char *uri)
{
    NGEM_FNAME_TAG(nginlReadingInfoGetPrefix);

    return nginlReadingInfoGetPrefixN(rInfo, uri, strlen(uri));
}

/**
 * Reading Information: get prefix of namespace from URI(N)
 */
static const char *
nginlReadingInfoGetPrefixN(
    nginlReadingInfo_t *rInfo,
    const char *uri,
    size_t n)
{
    NGEM_LIST_ITERATOR_OF(nginlNamespace_t) it;
    nginlNamespace_t *item;
    NGEM_FNAME_TAG(nginlReadingInfoGetPrefixN);

    NGEM_LIST_FOREACH(nginlNamespace_t, &rInfo->ngri_namespace, it, item) {
        if (strncmp(uri, item->ngn_uri, n) == 0){
            if (item->ngn_count == 1) {
                /* FOUND */
                return item->ngn_prefix;
            } else {
                return NULL;
            }
        }
    }

    return NULL;
}

/**
 * Reading Information: pop namespace information from stack.
 */
static ngemResult_t
nginlReadingInfoPopNamespace(
    nginlReadingInfo_t *rInfo,
    const char *prefix)
{
    NGEM_LIST_ITERATOR_OF(nginlNamespace_t) it;
    NGEM_LIST_ITERATOR_OF(nginlNamespace_t) begin;
    nginlNamespace_t *ns = NULL;
    nginlNamespace_t *item;
    ngLog_t *log = NULL;
    NGEM_FNAME(nginlReadingInfoPopNamespace);

    log = ngemLogGetDefault();

    if (NGEM_LIST_IS_EMPTY(nginlNamespace_t, &rInfo->ngri_namespace)) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Stack of namespace is empty.\n");
        return NGEM_FAILED;
    }

    begin = NGEM_LIST_BEGIN(nginlNamespace_t, &rInfo->ngri_namespace);

    ns = NGEM_LIST_GET(nginlNamespace_t, begin);

    if (prefix == NULL) {
        prefix = "";
    }
    if (strcmp(ns->ngn_prefix, prefix) != 0) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Doesn't match prefix.\n");
        return NGEM_FAILED;
    }

    NGEM_LIST_FOREACH(nginlNamespace_t, &rInfo->ngri_namespace, it, item) {
        if (strcmp(ns->ngn_prefix, item->ngn_prefix) == 0){
            item->ngn_count--;
            assert(item->ngn_count >= 0);
        }
    }

    ngiFree(ns->ngn_prefix, log, NULL);
    ngiFree(ns->ngn_uri, log, NULL);
    NGI_DEALLOCATE(nginlNamespace_t, ns, log, NULL);

    NGEM_LIST_ERASE(nginlNamespace_t, begin);

    return NGEM_SUCCESS;
}

/**
 * Reading Information: push namespace information from stack.
 */
static ngemResult_t
nginlReadingInfoPushNamespace(
    nginlReadingInfo_t *rInfo,
    const char *prefix,
    const char *uri)
{
    NGEM_LIST_ITERATOR_OF(nginlNamespace_t) it;
    nginlNamespace_t *item;
    nginlNamespace_t *ns = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(nginlReadingInfoPushNamespace);

    log = ngemLogGetDefault();

    ns = NGI_ALLOCATE(nginlNamespace_t, log, NULL);
    if (ns == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, 
            "Can't allocate storage for namespace information.\n");
        goto error;
    }
    ns->ngn_prefix = NULL;
    ns->ngn_uri    = NULL;
    ns->ngn_count  = 0;

    if (prefix == NULL) {
        prefix = "";
    }

    ns->ngn_prefix = strdup(prefix);
    if (ns->ngn_prefix == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't copy string.\n");
        goto error;
    }
    ns->ngn_uri = strdup(uri);
    if (ns->ngn_uri == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't copy string.\n");
        goto error;
    }

    nResult = NGEM_LIST_INSERT_HEAD(
        nginlNamespace_t, &rInfo->ngri_namespace, ns);
    if (nResult != NGEM_SUCCESS ) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, 
            "Can't push new namespace information to stack.\n");
        goto error;
    }

    NGEM_LIST_FOREACH(nginlNamespace_t, &rInfo->ngri_namespace, it, item) {
        if (strcmp(prefix, item->ngn_prefix) == 0){
            item->ngn_count++;
        }
    }

    return NGEM_SUCCESS;

error:
    if (ns != NULL) {
        ngiFree(ns->ngn_prefix, log, NULL);
        ngiFree(ns->ngn_uri, log, NULL);
        NGI_DEALLOCATE(nginlNamespace_t, ns, log, NULL);
    }
    return NGEM_FAILED;
}

/**
 * XML Parser Callback: On element start
 */
static void
nginlElementStart(
    void *userData,
    const XML_Char *element,
    const XML_Char **attributes)
{
    nginlReadingInfo_t *rInfo = (nginlReadingInfo_t *)userData;
    NGEM_LIST_ITERATOR_OF(nginlNamespace_t) it;
    nginlNamespace_t *item = NULL;
    const char *prefix = NULL;
    nginRemoteExecutableInformation_t *reInfo = NULL;
    ngLog_t *log = NULL;
    char *name = NULL;
    const char **p;
    ngemResult_t nResult;
    NGEM_FNAME(nginlElementStart);

    log = ngemLogGetDefault();

    if (rInfo->ngri_error) {
        return;
    }

    switch (rInfo->ngri_status) {
    case NGINL_NRF_FIRST:
        if ((rInfo->ngri_depth != 0) || 
            (strcmp(NGINL_NS_NRF ":" "NinfGRemoteInformation", element) != 0)) {
            goto error;
        }
        rInfo->ngri_status = NGINL_NRF_IN_NRF;
        break;
    case NGINL_NRF_IN_NRF:
        if (rInfo->ngri_depth != 1) {
            break;
        }
        if (strcmp(
                NGINL_NS_REI ":" NGIL_TAG_REMOTE_EXECUTABLE_INFORMATION,
                element) != 0) {
            ngLogWarn(log, NGIN_LOGCAT_NRF, fName, "Unkown element <%s>.\n", element);
            break;
        }

        rInfo->ngri_status = NGINL_NRF_IN_EXECUTABLE;

        
        /* Print XML */
        prefix = nginlReadingInfoGetPrefix(rInfo, NGINL_NS_REI);
        if (prefix == NULL) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't get prefix of %s.\n", NGINL_NS_REI);
            goto error;
        }

        nResult = ngemStringBufferFormat(&rInfo->ngri_xml,
            "<?xml version=\"1.0\" encoding=\"us-ascii\" ?>\n"
            "<%s:%s", prefix, NGIL_TAG_REMOTE_EXECUTABLE_INFORMATION);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
            goto error;
        }
        NGEM_LIST_FOREACH(nginlNamespace_t, &rInfo->ngri_namespace, it, item) {
            if (item->ngn_count > 1) {
                continue;
            }
            if (strcmp(item->ngn_prefix, "") == 0) {
                nResult = ngemStringBufferFormat(
                    &rInfo->ngri_xml, " xmlns=\"%s\"", item->ngn_uri);
            } else {
                nResult = ngemStringBufferFormat(
                    &rInfo->ngri_xml, " xmlns:%s=\"%s\"",
                    item->ngn_prefix, item->ngn_uri);
            }
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
                goto error;
            }
        }

        nResult = ngemStringBufferAppend(&rInfo->ngri_xml, ">\n");
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
            goto error;
        }

        NGEM_ASSERT(rInfo->ngri_current == NULL);
        reInfo = nginlRemoteExecutableInformationCreate();
        if (reInfo == NULL) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't create Executable Information.\n");
            goto error;
        }
        rInfo->ngri_current = reInfo;

        break;
    case NGINL_NRF_IN_EXECUTABLE:
        nResult = nginlPrintElementStart(rInfo, element, attributes);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't print element.\n");
            goto error;
        }

        NGEM_ASSERT(rInfo->ngri_depth >= 2);
        if (rInfo->ngri_depth > 2) {
            break;
        }

        /* Depth is 2 */
        if (strcmp(NGINL_NS_REI ":hostName", element) == 0) {
            if (rInfo->ngri_current->ngrei_hostname != NULL) {
                ngLogWarn(log, NGIN_LOGCAT_NRF, fName,
                    "<hostName> already has appeared.\n");
            } else {
                rInfo->ngri_status = NGINL_NRF_IN_HOSTNAME;
            }
        } else if (strcmp(NGINL_NS_RCI ":class", element) == 0) {
            if (rInfo->ngri_current->ngrei_classname != NULL) {
                ngLogWarn(log, NGIN_LOGCAT_NRF, fName,
                    "<class> already has appeared.\n");
            } else {
                /* Get "name" attribute of "<class>" */
                for (p = attributes;*p != NULL; p += 2) {
                    if (strcmp(*p, "name") == 0) {
                        NGEM_ASSERT(*(p + 1) != NULL);
                        name = strdup(*(p+1));
                        if (name == NULL) {
                            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't copy string.\n");
                            goto error;
                        }
                        break;
                    }
                }
                rInfo->ngri_current->ngrei_classname = name;
            }
        }
        break;
    case NGINL_NRF_IN_HOSTNAME:
    case NGINL_NRF_END:
    default:
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Unexcept element.\n");
        goto error;
    }

    rInfo->ngri_nsDeclCount = 0;
    rInfo->ngri_depth++;

    return;
error:
    rInfo->ngri_error = true;
    return;
}

/**
 * XML Parser Callback: On element end
 */
static void
nginlElementEnd(
    void *userData,
    const XML_Char *element)
{
    nginlReadingInfo_t *rInfo = (nginlReadingInfo_t *)userData;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(nginlElementEnd);

    log = ngemLogGetDefault();

    if (rInfo->ngri_error) {
        return;
    }

    switch (rInfo->ngri_status) {
    case NGINL_NRF_FIRST:
    case NGINL_NRF_END:
        abort();
        break;
    case NGINL_NRF_IN_NRF:
        if (rInfo->ngri_depth == 1) {
            rInfo->ngri_status = NGINL_NRF_END;
        }
        break;
    case NGINL_NRF_IN_HOSTNAME:
        if (rInfo->ngri_depth != 3) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "<hostName> must not have a child element.\n");
            goto error;
        }
        NGEM_ASSERT(rInfo->ngri_current != NULL);

        /* Get Hostname */
        rInfo->ngri_current->ngrei_hostname =
            ngemStringBufferRelease(&rInfo->ngri_key);
        if (rInfo->ngri_current->ngrei_hostname == NULL) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't get string from buffer.\n");
            goto error;
        }
        ngemStringStrip(rInfo->ngri_current->ngrei_hostname);

        rInfo->ngri_status = NGINL_NRF_IN_EXECUTABLE;

        nResult = ngemStringBufferReset(&rInfo->ngri_key);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't reset string buffer.\n");
            goto error;
        }
        /* Break Through */
    case NGINL_NRF_IN_EXECUTABLE:
        nResult = nginlPrintElementEnd(rInfo, element);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't print element end.\n");
            goto error;
        }
        if ((rInfo->ngri_depth == 2) &&
            (strcmp(NGINL_NS_REI ":RemoteExecutableInformation", element) == 0)) {
            /* END */
            rInfo->ngri_status = NGINL_NRF_IN_NRF;
            
            NGEM_ASSERT(rInfo->ngri_current != NULL);

            /* XML String */
            rInfo->ngri_current->ngrei_xmlString =
                ngemStringBufferRelease(&rInfo->ngri_xml);
            if (rInfo->ngri_current->ngrei_xmlString == NULL) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't get string from buffer.\n");
                goto error;
            }

            nResult = ngemStringBufferReset(&rInfo->ngri_xml);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't reset string buffer.\n");
                goto error;
            }

            nResult = nginlREIcontainerAppend(
                rInfo->ngri_container, rInfo->ngri_current);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string buffer.\n");
                nResult = nginlRemoteExecutableInformationDestroy(
                    rInfo->ngri_current);
                if (nResult != NGEM_SUCCESS) {
                    ngLogError(log, NGIN_LOGCAT_NRF, fName,
                        "Can't destroy Remote Executable Information.\n");
                    goto error;
                }
                /* Continue */
            }
            rInfo->ngri_current = NULL;
        }
        break;
    default:
        NGEM_ASSERT_NOTREACHED();
    }
    rInfo->ngri_depth--;

    return;
error:
    rInfo->ngri_error = true;

    return;
}

/**
 * XML Parser Callback: On namespace start
 */
static void
nginlNSstart(
    void *userData,
    const XML_Char *prefix,
    const XML_Char *uri)
{
    nginlReadingInfo_t *rInfo = (nginlReadingInfo_t *)userData;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    NGEM_FNAME(nginlNSstart);

    log = ngemLogGetDefault();

    if (rInfo->ngri_error) {
        return;
    }

    rInfo->ngri_nsDeclCount++;

    nResult = nginlReadingInfoPushNamespace(rInfo, prefix, uri);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't push namespace information.\n");
        rInfo->ngri_error = true;
    }

    return;
}

static void
nginlNSend(
    void *userData,
    const XML_Char *prefix)
{
    nginlReadingInfo_t *rInfo = (nginlReadingInfo_t *)userData;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    NGEM_FNAME(nginlNSend);

    log = ngemLogGetDefault();

    if (rInfo->ngri_error) {
        return;
    }

    assert(rInfo->ngri_nsDeclCount == 0);

    nResult = nginlReadingInfoPopNamespace(rInfo, prefix);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't pop namespace information.\n");
        rInfo->ngri_error = true;
    }

    return;
}

/**
 * XML Parser Callback: On char data
 */
static void
nginlCharDataHandler(
    void *userData,
    const XML_Char *string,
    int length)
{
    nginlReadingInfo_t *rInfo = (nginlReadingInfo_t *)userData;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(nginlCharDataHandler);

    log = ngemLogGetDefault();

    if (rInfo->ngri_error) {
        return;
    }

    switch (rInfo->ngri_status) {
    case NGINL_NRF_IN_HOSTNAME:
        nResult = ngemStringBufferNappend(&rInfo->ngri_key, string, length);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
            goto error;
        }
        /* Break Through */
    case NGINL_NRF_IN_EXECUTABLE:
        /* NinfGRemoteInformation::RemoteExecutableInformation::rei:hostName */
        nResult = ngemStringBufferNappend(&rInfo->ngri_xml, string, length);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
            goto error;
        }
        break;
    case NGINL_NRF_IN_NRF:
        /* Do Nothing */
        break;
    case NGINL_NRF_FIRST:
    case NGINL_NRF_END:
    default:
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Unexcept character data.\n");
        goto error;
    }
    return;
error:
    rInfo->ngri_error = true;
    return;
}

/**
 * XML Parser Callback: default;
 */
static void
nginlDefaultHandler(
    void *userData,
    const XML_Char *string,
    int length)
{
    nginlReadingInfo_t *rInfo = (nginlReadingInfo_t *)userData;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(nginlDefaultHandler);

    log = ngemLogGetDefault();

    if (rInfo->ngri_error) {
        return;
    }

    switch (rInfo->ngri_status) {
    case NGINL_NRF_FIRST:
    case NGINL_NRF_END:
    case NGINL_NRF_IN_NRF:
    case NGINL_NRF_IN_HOSTNAME:
        /* Do Nothing */
        break;
    case NGINL_NRF_IN_EXECUTABLE:
        nResult = ngemStringBufferNappend(&rInfo->ngri_xml, string, length);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
            goto error;
        }
        break;
    }
    return;
error:
    rInfo->ngri_error = true;
    return;
}

/**
 * print element(Open)
 */
static ngemResult_t
nginlPrintElementStart(
    nginlReadingInfo_t *rInfo,
    const XML_Char *element,
    const XML_Char **attributes)
{
    char *el = NULL;
    const char *prefix = NULL;
    char *attr = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    int i;
    NGEM_LIST_ITERATOR_OF(nginlNamespace_t) it;
    nginlNamespace_t *ns;
    NGEM_FNAME(nginlPrintElementStart);

    log = ngemLogGetDefault();

    el = strrchr(element, ':');
    if (el == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "\"%s\" is invalid(does not include \":\").\n", element);
        goto error;
    }

    prefix = nginlReadingInfoGetPrefixN(rInfo, element, el - element - 1);
    if (prefix == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Invalid namespace.\n");
        goto error;
    }
    el++;

    if (strlen(prefix) == 0) {
        nResult = ngemStringBufferFormat(&rInfo->ngri_xml, "<%s", el);
    } else {
        nResult = ngemStringBufferFormat(&rInfo->ngri_xml, "<%s:%s", prefix, el);
    }
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
        goto error;
    }
    /* namespace */

    i = 0;
    NGEM_LIST_FOREACH(nginlNamespace_t, &rInfo->ngri_namespace, it, ns) {
        if (i == rInfo->ngri_nsDeclCount) {
            break;
        }
        if (ns->ngn_count > 1) {
            ngLogFatal(log, NGIN_LOGCAT_NRF, fName, "Invalid Status.\n");
            goto error;
        }

        if (strlen(ns->ngn_prefix) == 0) {
            nResult = ngemStringBufferFormat(&rInfo->ngri_xml,
                " xmlns=\"%s\"", ns->ngn_uri);
        } else {
            nResult = ngemStringBufferFormat(&rInfo->ngri_xml,
                " xmlns:%s=\"%s\"", ns->ngn_prefix, ns->ngn_uri);
        }
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
            goto error;
        }

        i++;
    }

    if (i < rInfo->ngri_nsDeclCount) {
        ngLogFatal(log, NGIN_LOGCAT_NRF, fName, "Invalid Status.\n");
        goto error;
    }
    rInfo->ngri_nsDeclCount = 0;

    for (i = 0;attributes[i] != NULL;i += 2) {
        if (attributes[i + 1] == NULL) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "The attribute does not have a value.\n");
            goto error;
        }

        attr = strrchr(attributes[i], ':');
        if (attr != NULL) {
            prefix = nginlReadingInfoGetPrefixN(rInfo, attributes[i], attr - attributes[i] - 1);
            if (prefix == NULL) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName, "Invalid namespace.\n");
                goto error;
            }
            attr++;
            if (strlen(prefix) == 0) {
                nResult = ngemStringBufferFormat(&rInfo->ngri_xml, " %s=\"%s\"", attr, attributes[i+1]);
            } else {
                nResult = ngemStringBufferFormat(&rInfo->ngri_xml, " %s:%s=\"%s\"", prefix, attr, attributes[i+1]);
            }
        } else {
            nResult = ngemStringBufferFormat(&rInfo->ngri_xml, " %s=\"%s\"", attributes[i], attributes[i+1]);
        }
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
            goto error;
        }
    }
    nResult = ngemStringBufferFormat(&rInfo->ngri_xml, ">");
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
        goto error;
    }
    return NGEM_SUCCESS;
error:
    return NGEM_FAILED;
}

/**
 * print element(close)
 */
static ngemResult_t
nginlPrintElementEnd(
    nginlReadingInfo_t *rInfo,
    const XML_Char *element)
{
    char *el = NULL;
    const char *prefix = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(nginlPrintElementEnd);

    log = ngemLogGetDefault();

    el = strrchr(element, ':');
    if (el == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "\"%s\" is invalid(does not include \":\").\n", element);
        goto error;
    }

    prefix = nginlReadingInfoGetPrefixN(rInfo, element, el - element - 1);
    if (prefix == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Invalid namespace.\n");
        goto error;
    }
    el++;

    if (strlen(prefix) == 0) {
        nResult = ngemStringBufferFormat(&rInfo->ngri_xml, "</%s>", el);
    } else {
        nResult = ngemStringBufferFormat(&rInfo->ngri_xml, "</%s:%s>", prefix, el);
    }
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't append string to buffer.\n");
        goto error;
    }
    return NGEM_SUCCESS;
error:
    return NGEM_FAILED;
}
