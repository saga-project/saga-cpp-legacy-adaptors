/*
 * $RCSfile: ngclConfigFileRead.c,v $ $Revision: 1.43 $ $Date: 2008/03/28 06:57:41 $
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

/**
 * Module of client configuration file reading.
 */

#include "ng.h"
#include "ngConfigFile.h"

NGI_RCSID_EMBED("$RCSfile: ngclConfigFileRead.c,v $ $Revision: 1.43 $ $Date: 2008/03/28 06:57:41 $")

#define NGCLL_CONFIG_DUMMY_HOSTNAME "dummy"
static char *ngcllTrueStrArray[] = {
    "set"
};
#define NGCL_ISSET_A_TRUE ngcllTrueStrArray
/* ISSET must const, and copiable */

static int ngcllIsSetIPtrue = 1;
#define NGCLL_ISSET_IP_TRUE (&ngcllIsSetIPtrue)


/*
 * Pairs
 * isSet's member != 0, and != NULL, then that member is enable
 * isSet should not free(), this includes only static string pointer
 */
typedef struct ngcllLocalMachineInfoPair_s {
    struct ngcllLocalMachineInfoPair_s *nlmip_next;
    ngclLocalMachineInformation_t *nlmip_entities;
    ngclLocalMachineInformation_t *nlmip_isSet;
} ngcllLocalMachineInfoPair_t;

typedef struct ngcllInvokeServerInfoPair_s {
    struct ngcllInvokeServerInfoPair_s *nisip_next;
    ngclInvokeServerInformation_t *nisip_entities;
    ngclInvokeServerInformation_t *nisip_isSet;
} ngcllInvokeServerInfoPair_t;

typedef struct ngcllCommunicationProxyInfoPair_s {
    struct ngcllCommunicationProxyInfoPair_s *ncpip_next;
    ngclCommunicationProxyInformation_t *ncpip_entities;
    ngclCommunicationProxyInformation_t *ncpip_isSet;
} ngcllCommunicationProxyInfoPair_t;

typedef struct ngcllInformationServiceInfoPair_s {
    struct ngcllInformationServiceInfoPair_s *nisip_next;
    ngclInformationServiceInformation_t *nisip_entities;
    ngclInformationServiceInformation_t *nisip_isSet;
} ngcllInformationServiceInfoPair_t;

typedef struct ngcllRemoteMachineInfoPair_s {
    struct ngcllRemoteMachineInfoPair_s *nrmip_next;
    ngclRemoteMachineInformation_t *nrmip_entities;
    ngclRemoteMachineInformation_t *nrmip_isSet;
} ngcllRemoteMachineInfoPair_t;

typedef struct ngcllExecutablePathInfoPair_s {
    struct ngcllExecutablePathInfoPair_s *nepip_next;
    ngclExecutablePathInformation_t *nepip_entities;
    ngclExecutablePathInformation_t *nepip_isSet;
} ngcllExecutablePathInfoPair_t;


/*
 * Reading Status
 */
typedef struct ngcllReadingState_s {
    ngiStringList_t *nrs_appearedConfigs;

    /* for LocalMachineInformation */
    int nrs_lmiAppeared;
    ngcllLocalMachineInfoPair_t *nrs_lmInfo;

    /* for InvokeServerInformation */
    ngiStringList_t *nrs_appearedInvokeServers;
    ngcllInvokeServerInfoPair_t *nrs_invokeServerInfos;
    ngcllInvokeServerInfoPair_t *nrs_curInvokeServerInfo;
    ngiStringList_t *nrs_curInvokeServerInfoOptions;

    /* for CommunicationProxyInformation */
    ngiStringList_t *nrs_appearedCommunicationProxys;
    ngcllCommunicationProxyInfoPair_t *nrs_cpInfos;
    ngcllCommunicationProxyInfoPair_t *nrs_curCPinfo;
    ngiStringList_t *nrs_curCPinfoOptions;

    /* for InformationServiceInformation */
    ngiStringList_t *nrs_appearedInformationServiceTags;
    ngcllInformationServiceInfoPair_t *nrs_infoServiceInfos;
    ngcllInformationServiceInfoPair_t *nrs_curInfoServiceInfo;
    ngiStringList_t *nrs_curInfoServiceInfoSources;
    ngiStringList_t *nrs_curInfoServiceInfoOptions;

    /* for RemoteMachineInformation */
    ngiStringList_t *nrs_appearedNoTagRmInfoHosts;
    ngiStringList_t *nrs_appearedRmInfoTags;
    ngiStringList_t *nrs_appearedInvokeServersInRm;
    ngiStringList_t *nrs_appearedCommunicationProxysInRm;
    ngcllRemoteMachineInfoPair_t *nrs_rmInfos;   /* store all remote machine infos read */
    ngcllRemoteMachineInfoPair_t *nrs_curRmInfo; /* current reading info */
    ngiStringList_t *nrs_curRmInfoInvokeServerOptions;  /* current opts */
    ngiStringList_t *nrs_curRmInfoCommunicationProxyOptions;  /* current opts */
    ngiStringList_t *nrs_curRmInfoRSLextensions;  /* current reading exts */
    ngiStringList_t *nrs_curRmInfoEnviron;  /* current reading environ */
    ngiStringList_t *nrs_curRmInfoHosts; /* current reading hosts */

    /* for RemoteMachineInformation default */
    int nrs_defRmInfoAppeared;
    int nrs_readingServerDefault;
    ngcllRemoteMachineInfoPair_t *nrs_defRmInfo; /* default info */

    /* ExecutablePathInformation */
    ngiStringList_t *nrs_appearedRmInfoHostsInEp;
    ngcllExecutablePathInfoPair_t *nrs_epInfos;
    ngcllExecutablePathInfoPair_t *nrs_curEpInfo;

} ngcllReadingState_t;

/**
 * Note: Configuration Priority
 *       1. client configuration file
 *       2. <SERVER_DEFAULT> section
 *       3. <INFORMATION_SOURCE>
 *       4. source code default
 */

#define NGCLL_CONFIG_ATTRFUNC_ARG \
    (char *attrName, ngiTokenInfo_t *token, \
     ngcllReadingState_t *readingState, ngclContext_t *context, \
     int *error)

typedef int (*ngcllAttrFunc_t)NGCLL_CONFIG_ATTRFUNC_ARG;


/**
 * FuncTable is for to match keyword string and attribute functions.
 * Last entry of array should be NULL.
 */

typedef struct ngcllAttrFuncTable_s {
    char *naft_attrName;
    ngcllAttrFunc_t naft_func;
} ngcllAttrFuncTable_t;

typedef struct ngcllTagFuncTable_s {
    char *ntft_tagName;
    ngcllAttrFuncTable_t *ntft_attrs;
    ngcllAttrFunc_t ntft_tagBegin, ntft_tagEnd;
} ngcllTagFuncTable_t;

/* Pair for LocalMachineInformation */
ngcllLocalMachineInfoPair_t * ngcllLocalMachineInfoPairCreate(
    ngclContext_t *context, int *error);
int ngcllLocalMachineInfoPairDestruct(ngclContext_t *context,
    ngcllLocalMachineInfoPair_t *lmInfoPair, int *error);
int ngcllLocalMachineInfoPairInitialize(ngclContext_t *context, 
    ngcllLocalMachineInfoPair_t *lmInfoPair, int *error);
int ngcllLocalMachineInformationSetPair(
    ngclLocalMachineInformation_t *dst, ngcllLocalMachineInfoPair_t *src,
    ngLog_t *log, int *error);

/* Pair for InvokeServerInformation */
ngcllInvokeServerInfoPair_t * ngcllInvokeServerInfoPairCreate(
    ngclContext_t *context, int *error);
int ngcllInvokeServerInfoPairDestruct(ngclContext_t *context,
    ngcllInvokeServerInfoPair_t *isInfoPair, int *error);
int ngcllInvokeServerInfoPairListDestruct(ngclContext_t *context,
    ngcllInvokeServerInfoPair_t *isInfoPair, int *error);
int ngcllInvokeServerInfoPairInitialize(ngclContext_t *context, 
    ngcllInvokeServerInfoPair_t *isInfoPair, int *error);
int ngcllInvokeServerInfoPairRegister(
    ngcllInvokeServerInfoPair_t **dst, ngcllInvokeServerInfoPair_t *src);
int ngcllInvokeServerInformationSetPair(
    ngclInvokeServerInformation_t *dst, ngcllInvokeServerInfoPair_t *src,
    ngLog_t *log, int *error);

/* Pair for CommunicationProxyInformation */
ngcllCommunicationProxyInfoPair_t * ngcllCommunicationProxyInfoPairCreate(
    ngclContext_t *context, int *error);
int ngcllCommunicationProxyInfoPairDestruct(ngclContext_t *context,
    ngcllCommunicationProxyInfoPair_t *cpInfoPair, int *error);
int ngcllCommunicationProxyInfoPairListDestruct(ngclContext_t *context,
    ngcllCommunicationProxyInfoPair_t *cpInfoPair, int *error);
int ngcllCommunicationProxyInfoPairInitialize(ngclContext_t *context,
    ngcllCommunicationProxyInfoPair_t *cpInfoPair, int *error);
int ngcllCommunicationProxyInfoPairRegister(
    ngcllCommunicationProxyInfoPair_t **dst,
    ngcllCommunicationProxyInfoPair_t *src);
int ngcllCommunicationProxyInformationSetPair(
    ngclCommunicationProxyInformation_t *dst,
    ngcllCommunicationProxyInfoPair_t *src, ngLog_t *log, int *error);

/* Pair for InformationServiceInformation */
ngcllInformationServiceInfoPair_t * ngcllInformationServiceInfoPairCreate(
    ngclContext_t *context, int *error);
int ngcllInformationServiceInfoPairDestruct(ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair, int *error);
int ngcllInformationServiceInfoPairListDestruct(ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair, int *error);
int ngcllInformationServiceInfoPairInitialize(ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair, int *error);
int ngcllInformationServiceInfoPairRegister(
    ngcllInformationServiceInfoPair_t **dst,
    ngcllInformationServiceInfoPair_t *src);
int ngcllInformationServiceInformationSetPair(
    ngclInformationServiceInformation_t *dst,
    ngcllInformationServiceInfoPair_t *src, ngLog_t *log, int *error);

/* Pair for RemoteMachineInformation */
ngcllRemoteMachineInfoPair_t * ngcllRemoteMachineInfoPairCreate(
    ngclContext_t *context, int *error);
int ngcllRemoteMachineInfoPairDestruct(ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair, int *error);
int ngcllRemoteMachineInfoPairListDestruct(ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair, int *error);
int ngcllRemoteMachineInfoPairInitialize(ngclContext_t *context, 
    ngcllRemoteMachineInfoPair_t *rmInfoPair, int *error);
ngcllRemoteMachineInfoPair_t * ngcllRemoteMachineInfoPairDuplicate(
    ngclContext_t *context, ngcllRemoteMachineInfoPair_t *src, int *error);
int ngcllRemoteMachineInfoPairRegister(
    ngcllRemoteMachineInfoPair_t **dst, ngcllRemoteMachineInfoPair_t *src);
int ngcllRemoteMachineInformationSetPair(
    ngclRemoteMachineInformation_t *dst, ngcllRemoteMachineInfoPair_t *src,
    ngLog_t *log, int *error);
ngcllRemoteMachineInfoPair_t * ngcllRemoteMachineInfoPairGet(
    ngcllRemoteMachineInfoPair_t *rmInfoPair, char *hostName);
ngcllRemoteMachineInfoPair_t *
ngcllRemoteMachineInfoPairGetWithTag(
    ngcllRemoteMachineInfoPair_t *rmInfoPair, char *hostName, char *tagName);

/* Pair for ExecutablePathInformation */
ngcllExecutablePathInfoPair_t *ngcllExecutablePathInfoPairCreate(
    ngclContext_t *context, int *error);
int ngcllExecutablePathInfoPairDestruct(ngclContext_t *context,
    ngcllExecutablePathInfoPair_t *rmInfoPair, int *error);
int ngcllExecutablePathInfoPairListDestruct(ngclContext_t *context,
    ngcllExecutablePathInfoPair_t *epInfoPair, int *error);
int ngcllExecutablePathInfoPairInitialize(ngclContext_t *context,
    ngcllExecutablePathInfoPair_t *rmInfoPair, int *error);
ngcllExecutablePathInfoPair_t *ngcllExecutablePathInfoPairDuplicate(
    ngclContext_t *context, ngcllExecutablePathInfoPair_t *src, int *error);
int ngcllExecutablePathInfoPairRegister(
    ngcllExecutablePathInfoPair_t **dst, ngcllExecutablePathInfoPair_t *src);
int ngcllExecutablePathInformationSetPair(
    ngclExecutablePathInformation_t *dst, ngcllExecutablePathInfoPair_t *src,
    ngLog_t *log, int *error);
ngcllExecutablePathInfoPair_t *ngcllExecutablePathInfoPairGet(
    ngcllExecutablePathInfoPair_t *epInfoPair,
    char *hostName, char *className);

/*****************************************************************/
/**
 * function table for each configuration file entity and their function
 */

static int ngcllAttrFuncIncludeBegin            NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncIncludeEnd              NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInclude_filename        NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncClientBegin             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClientEnd               NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_hostname         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_save_sessionInfo NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_loglevel         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_loglevel_ngprot  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_loglevel_ngi     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_loglevel_grpc    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_log_filePath     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_log_suffix       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_log_nFiles       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_log_maxFileSize  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_log_overWrite    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_tmpDir           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_refresh_cred     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_invoke_server_log  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_comm_proxy_log     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_info_service_log   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_fortran_compatible NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_handling_signals NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_listen_port          NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncFuncInfoBegin           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfoEnd             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_hostname       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_funcname       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_staging        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_path           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_backend        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_session_timeout     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_trans_timeout_arg   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_trans_timeout_res   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_trans_timeout_cbarg NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_trans_timeout_cbres NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncInvokeServerBegin       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServerEnd         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_type       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_path       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_max_jobs   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_logfile    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_statusPoll NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_option     NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncCommunicationProxyBegin     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncCommunicationProxyEnd       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncCommunicationProxy_type     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncCommunicationProxy_path     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncCommunicationProxy_bufsize  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncCommunicationProxy_max_jobs NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncCommunicationProxy_logfile  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncCommunicationProxy_option   NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncInformationSourceBegin    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSourceEnd      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSource_tag     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSource_type    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSource_path    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSource_logfile NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSource_timeout NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSource_source  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInformationSource_option  NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncServerBegin             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServerEnd               NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_hostname         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_tag              NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_port             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_invoke_server    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_invoke_server_opt  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_cp_type          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_cp_staging       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_cp_path          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_cp_bufferSize    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_cp_option        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_info_source_tag  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_mpi_runCPUs      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_keep_connect     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_force_xdr        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_jobmanager       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_subject          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_client_hostname  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_startTimeout NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_stopTimeout  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_maxTime      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_maxWallTime  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_maxCpuTime   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_queue        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_project      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_hostCount    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_minMemory    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_maxMemory    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_job_rslExtensions  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_heartbeat        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_heartbeatCount   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_redirect_outerr  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_tcp_nodelay      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_retryCount       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_retryBaseInterval   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_retryIncreaseRatio  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_retryRandom         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_argumentTransfer    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_compress            NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_compress_threshold  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_argument_blockSize  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_workDirectory       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_coreDumpSize        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_commLog_enable      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_commLog_filePath    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_commLog_suffix      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_commLog_nFiles      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_commLog_maxFileSize NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_commLog_overWrite   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_debug            NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_debug_display    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_debug_terminal   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_debug_debugger   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_debug_busyLoop   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_environment      NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncServerDefaultBegin      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServerDefaultEnd        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServerDefault_hostname  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServerDefault_tag       NGCLL_CONFIG_ATTRFUNC_ARG;
       /* Except hostname and tag, ServerDefault is same as Server. */
       /* ServerDefault_hostname are called from Server_hostname.   */

/* table data definition below */
static ngcllAttrFuncTable_t includeAttrs[] = {
    {"filename",         ngcllAttrFuncInclude_filename},
    {NULL, NULL}
};

static ngcllAttrFuncTable_t clientAttrs[] = {
    {"hostname",         ngcllAttrFuncClient_hostname},
    {"save_sessionInfo", ngcllAttrFuncClient_save_sessionInfo},
    {"loglevel",         ngcllAttrFuncClient_loglevel},
    {"loglevel_ninfgProtocol",  ngcllAttrFuncClient_loglevel_ngprot},
    {"loglevel_ninfgInternal",  ngcllAttrFuncClient_loglevel_ngi},
    {"loglevel_ninfgGrpc",      ngcllAttrFuncClient_loglevel_grpc},
    {"log_filePath",            ngcllAttrFuncClient_log_filePath},
    {"log_suffix",              ngcllAttrFuncClient_log_suffix},
    {"log_nFiles",              ngcllAttrFuncClient_log_nFiles},
    {"log_maxFileSize",         ngcllAttrFuncClient_log_maxFileSize},
    {"log_overwriteDirectory",  ngcllAttrFuncClient_log_overWrite},
    {"tmp_dir",                 ngcllAttrFuncClient_tmpDir},
    {"refresh_credential",      ngcllAttrFuncClient_refresh_cred},
    {"invoke_server_log",       ngcllAttrFuncClient_invoke_server_log},
    {"client_communication_proxy_log", ngcllAttrFuncClient_comm_proxy_log}, 
    {"information_service_log", ngcllAttrFuncClient_info_service_log},
    {"fortran_compatible",      ngcllAttrFuncClient_fortran_compatible},
    {"handling_signals",        ngcllAttrFuncClient_handling_signals},
    {"listen_port",             ngcllAttrFuncClient_listen_port},
    {NULL, NULL}
};

static ngcllAttrFuncTable_t funcInfoAttrs[] = {
    {"hostname",         ngcllAttrFuncFuncInfo_hostname},
    {"funcname",         ngcllAttrFuncFuncInfo_funcname},
    {"path",             ngcllAttrFuncFuncInfo_path},
    {"staging",          ngcllAttrFuncFuncInfo_staging},
    {"backend",          ngcllAttrFuncFuncInfo_backend},
    {"session_timeout",  ngcllAttrFuncFuncInfo_session_timeout},
    {"transferTimeout_argument", ngcllAttrFuncFuncInfo_trans_timeout_arg},
    {"transferTimeout_result",   ngcllAttrFuncFuncInfo_trans_timeout_res},
    {"transferTimeout_callbackArgument",
        ngcllAttrFuncFuncInfo_trans_timeout_cbarg},
    {"transferTimeout_callbackResult",
        ngcllAttrFuncFuncInfo_trans_timeout_cbres},
    {NULL, NULL}
};

static ngcllAttrFuncTable_t invokeServerAttrs[] = {
    {"type",             ngcllAttrFuncInvokeServer_type},
    {"path",             ngcllAttrFuncInvokeServer_path},
    {"max_jobs",         ngcllAttrFuncInvokeServer_max_jobs},
    {"log_filePath",     ngcllAttrFuncInvokeServer_logfile},
    {"status_polling",   ngcllAttrFuncInvokeServer_statusPoll},
    {"option",           ngcllAttrFuncInvokeServer_option},
    {NULL, NULL}
};

static ngcllAttrFuncTable_t communicationProxyAttrs[] = {
    {"type",             ngcllAttrFuncCommunicationProxy_type},     
    {"path",             ngcllAttrFuncCommunicationProxy_path},     
    {"buffer_size",      ngcllAttrFuncCommunicationProxy_bufsize},
    {"max_jobs",         ngcllAttrFuncCommunicationProxy_max_jobs}, 
    {"log_filePath",     ngcllAttrFuncCommunicationProxy_logfile},  
    {"option",           ngcllAttrFuncCommunicationProxy_option},   
    {NULL, NULL}
};

static ngcllAttrFuncTable_t informationSourceAttrs[] = {
    {"tag",              ngcllAttrFuncInformationSource_tag},     
    {"type",             ngcllAttrFuncInformationSource_type},    
    {"path",             ngcllAttrFuncInformationSource_path},    
    {"log_filePath",     ngcllAttrFuncInformationSource_logfile}, 
    {"timeout",          ngcllAttrFuncInformationSource_timeout}, 
    {"source",           ngcllAttrFuncInformationSource_source},  
    {"option",           ngcllAttrFuncInformationSource_option},  
    {NULL, NULL}
};

static ngcllAttrFuncTable_t serverAttrs[] = {
    {"hostname",         ngcllAttrFuncServer_hostname},
    {"tag",              ngcllAttrFuncServer_tag},
    {"port",             ngcllAttrFuncServer_port},
    {"invoke_server",    ngcllAttrFuncServer_invoke_server},
    {"invoke_server_option", ngcllAttrFuncServer_invoke_server_opt},
    {"communication_proxy",         ngcllAttrFuncServer_cp_type},
    {"communication_proxy_staging", ngcllAttrFuncServer_cp_staging},
    {"communication_proxy_path",    ngcllAttrFuncServer_cp_path},
    {"communication_proxy_buffer_size", ngcllAttrFuncServer_cp_bufferSize},
    {"communication_proxy_option",  ngcllAttrFuncServer_cp_option},
    {"information_source_tag",      ngcllAttrFuncServer_info_source_tag},
    {"mpi_runNoOfCPUs",  ngcllAttrFuncServer_mpi_runCPUs},
    {"keep_connection",  ngcllAttrFuncServer_keep_connect},
    {"force_xdr",        ngcllAttrFuncServer_force_xdr},
    {"jobmanager",       ngcllAttrFuncServer_jobmanager},
    {"subject",          ngcllAttrFuncServer_subject},
    {"client_hostname",  ngcllAttrFuncServer_client_hostname},
    {"job_startTimeout", ngcllAttrFuncServer_job_startTimeout},
    {"job_stopTimeout",  ngcllAttrFuncServer_job_stopTimeout},
    {"job_maxTime",      ngcllAttrFuncServer_job_maxTime},
    {"job_maxWallTime",  ngcllAttrFuncServer_job_maxWallTime},
    {"job_maxCpuTime",   ngcllAttrFuncServer_job_maxCpuTime},
    {"job_queue",        ngcllAttrFuncServer_job_queue},
    {"job_project",      ngcllAttrFuncServer_job_project},
    {"job_hostCount",    ngcllAttrFuncServer_job_hostCount},
    {"job_minMemory",    ngcllAttrFuncServer_job_minMemory},
    {"job_maxMemory",    ngcllAttrFuncServer_job_maxMemory},
    {"job_rslExtensions", ngcllAttrFuncServer_job_rslExtensions},
    {"heartbeat",        ngcllAttrFuncServer_heartbeat},
    {"heartbeat_timeoutCount",     ngcllAttrFuncServer_heartbeatCount},
    {"redirect_outerr",            ngcllAttrFuncServer_redirect_outerr},
    {"tcp_nodelay",                ngcllAttrFuncServer_tcp_nodelay},
    {"tcp_connect_retryCount",     ngcllAttrFuncServer_retryCount},
    {"tcp_connect_retryBaseInterval", ngcllAttrFuncServer_retryBaseInterval},
    {"tcp_connect_retryIncreaseRatio",ngcllAttrFuncServer_retryIncreaseRatio},
    {"tcp_connect_retryRandom",    ngcllAttrFuncServer_retryRandom},
    {"argument_transfer",          ngcllAttrFuncServer_argumentTransfer},
    {"compress",                   ngcllAttrFuncServer_compress},
    {"compress_threshold",         ngcllAttrFuncServer_compress_threshold},
    {"argument_blockSize",         ngcllAttrFuncServer_argument_blockSize},
    {"workDirectory",              ngcllAttrFuncServer_workDirectory},
    {"coreDumpSize",               ngcllAttrFuncServer_coreDumpSize},
    {"commLog_enable",             ngcllAttrFuncServer_commLog_enable},
    {"commLog_filePath",           ngcllAttrFuncServer_commLog_filePath},
    {"commLog_suffix",             ngcllAttrFuncServer_commLog_suffix},
    {"commLog_nFiles",             ngcllAttrFuncServer_commLog_nFiles},
    {"commLog_maxFileSize",        ngcllAttrFuncServer_commLog_maxFileSize},
    {"commLog_overwriteDirectory", ngcllAttrFuncServer_commLog_overWrite},
    {"debug",            ngcllAttrFuncServer_debug},
    {"debug_display",    ngcllAttrFuncServer_debug_display},
    {"debug_terminal",   ngcllAttrFuncServer_debug_terminal},
    {"debug_debugger",   ngcllAttrFuncServer_debug_debugger},
    {"debug_busyLoop",   ngcllAttrFuncServer_debug_busyLoop},
    {"environment",      ngcllAttrFuncServer_environment},
    {NULL, NULL}
};

static ngcllTagFuncTable_t tagFuncTable[] = {
    {"INCLUDE", includeAttrs,
    ngcllAttrFuncIncludeBegin,
    ngcllAttrFuncIncludeEnd},

    {"CLIENT", clientAttrs,
    ngcllAttrFuncClientBegin,
    ngcllAttrFuncClientEnd},

    {"FUNCTION_INFO", funcInfoAttrs,
    ngcllAttrFuncFuncInfoBegin,
    ngcllAttrFuncFuncInfoEnd},

    {"INVOKE_SERVER", invokeServerAttrs,
    ngcllAttrFuncInvokeServerBegin,
    ngcllAttrFuncInvokeServerEnd},

    {"CLIENT_COMMUNICATION_PROXY", communicationProxyAttrs,
    ngcllAttrFuncCommunicationProxyBegin,
    ngcllAttrFuncCommunicationProxyEnd},

    {"INFORMATION_SOURCE", informationSourceAttrs,
    ngcllAttrFuncInformationSourceBegin,
    ngcllAttrFuncInformationSourceEnd},

    {"SERVER", serverAttrs,
    ngcllAttrFuncServerBegin,
    ngcllAttrFuncServerEnd},

    {"SERVER_DEFAULT", serverAttrs,
    ngcllAttrFuncServerDefaultBegin,
    ngcllAttrFuncServerDefaultEnd},

    {NULL, NULL, NULL, NULL}
};

static int ngcllAttrFuncInitialize(ngclContext_t *context,
    ngcllReadingState_t **readingState, int *error);
static int ngcllAttrFuncFinalize(ngclContext_t *context,
    ngcllReadingState_t *readingState, int *error);
static int ngcllAttrFuncRegisterInformation(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);
static int ngcllReadingStateConstruct(ngclContext_t *context,
    ngcllReadingState_t **readingState, int *error);
static int ngcllReadingStateDestruct(ngclContext_t *context,
    ngcllReadingState_t *readingState, int *error);
static int ngcllLocalMachineInformationsRegister(ngclContext_t *context,
    ngcllReadingState_t *readingState, int *error);
static int ngcllInvokeServerInformationsRegister(ngclContext_t *context,
    ngcllReadingState_t *readingState, int *error);
static int ngcllInvokeServerInformationCheckValue(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngclInvokeServerInformation_t *isInfo, int *error);
static int ngcllCommunicationProxyInformationsRegister(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);
static int ngcllCommunicationProxyInformationCheckValue(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngclCommunicationProxyInformation_t *cpInfo, int *error);
static int ngcllInformationServiceInformationsRegister(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);
static int ngcllInformationServiceInformationCheckValue(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngclInformationServiceInformation_t *isInfo, int *error);
static int ngcllRemoteMachineInformationsRegister(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);
static int ngcllDefaultRemoteMachineInformationsRegister(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);
static int ngcllRemoteMachineInformationCheckValue(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngclRemoteMachineInformation_t *rmInfo, int *error);
static int ngcllExecutablePathInformationsRegister(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);


/*  function table section end */
/*****************************************************************/

/**
 * Conversion table for time, size
 */
extern ngiUnitConvTable_t ngiTimeUnitTable[];
extern ngiUnitConvTable_t ngiSizeUnitTable[];

/**
 * internal function prototypes
 */

/* parsing functions */
static int ngcllConfigFileRead(ngclContext_t *context,
    char *configFile, int *error);
static int ngcllConfigFileGetHomeConfigFileName(ngclContext_t *context,
    char **homeConfigFile, int *error);
static int ngcllConfigFileParse(ngclContext_t *context,
    ngiTokenReadInfo_t *tokenReadInfo, int *error);
static int ngcllConfigFileParseSub(ngclContext_t *context,
    ngiTokenReadInfo_t *tokenReadInfo,
    ngcllReadingState_t *readingState, int *error);
static int ngcllTagNameIsEqual(char *tag1, char *tag2, size_t max);
static ngcllTagFuncTable_t *ngcllGetTagFuncInfo(
    ngcllTagFuncTable_t *tagTable, char *tagName);
static ngcllAttrFunc_t ngcllGetAttrFuncPtr(
    ngcllAttrFuncTable_t *attrTable, char *attrName);

/* utility functions that is used from attr functions */
static int ngcllInvokeServerInfoPairSetOptions(
    ngclContext_t *context, ngcllInvokeServerInfoPair_t *isInfoPair,
    ngiStringList_t *options, int *error);
static int ngcllCommunicationProxyInfoPairSetOptions(
    ngclContext_t *context, ngcllCommunicationProxyInfoPair_t *cpInfoPair,
    ngiStringList_t *options, int *error);
static int ngcllInformationServiceInfoPairSetSources(
    ngclContext_t *context, ngcllInformationServiceInfoPair_t *isInfoPair,
    ngiStringList_t *sources, int *error);
static int ngcllInformationServiceInfoPairSetOptions(
    ngclContext_t *context, ngcllInformationServiceInfoPair_t *isInfoPair,
    ngiStringList_t *options, int *error);
static int ngcllRemoteMachineInfoPairSetInvokeServerOptions(
    ngclContext_t *context, ngcllRemoteMachineInfoPair_t *rmInfoPair,
    ngiStringList_t *options, int *error);
static int ngcllRemoteMachineInfoPairSetCommunicationProxyOptions(
    ngclContext_t *context, ngcllRemoteMachineInfoPair_t *rmInfoPair,
    ngiStringList_t *options, int *error);
static int ngcllRemoteMachineInfoPairSetRSLextensions(
    ngclContext_t *context, ngcllRemoteMachineInfoPair_t *rmInfoPair,
    ngiStringList_t *rslExtensions, int *error);
static int ngcllRemoteMachineInfoPairSetEnviron(ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair, ngiStringList_t *environ,
    int *error);
static int ngcllStringArrayAdd(char ***dstArray, int *dstSize,
    char **srcArray, int srcSize, ngLog_t *log, int *error);
static int ngcllStringArrayFree(
    char **array, int size, ngLog_t *log, int *error);

/* Set Default */
static int ngcllLocalMachineInformationSetDefault(
    ngclLocalMachineInformation_t *lmInfo, ngLog_t *log, int *error);
static int ngcllInvokeServerInformationSetDefault(
    ngclInvokeServerInformation_t *isInfo, ngLog_t *log, int *error);
static int ngcllCommunicationProxyInformationSetDefault(
    ngclCommunicationProxyInformation_t *cpInfo, ngLog_t *log, int *error);
static int ngcllInformationServiceInformationSetDefault(
    ngclInformationServiceInformation_t *isInfo, ngLog_t *log, int *error);
static int ngcllRemoteMachineInformationSetDefault(
    ngclRemoteMachineInformation_t *rmInfo, ngLog_t *log, int *error);
static int ngcllExecutablePathInformationSetDefault(
    ngclExecutablePathInformation_t *epInfo, ngLog_t *log, int *error);

/**
 * functions
 */

/**
 * ConfigFileRead() reads configFile and set appropriate data
 * in context
 */
int
ngcliConfigFileRead(ngclContext_t *context, char *configFile, int *error)
{
    static const char fName[] = "ngcliConfigFileRead";
    char *realConfigFile, *envConfigFile, *homeConfigFile;
    ngLog_t *log;
    int result;

    realConfigFile = NULL;
    envConfigFile = NULL;
    homeConfigFile = NULL;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Get configFile from argument */
    if ((configFile != NULL) && (configFile[0] != '\0')) {
        realConfigFile = configFile;

        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Using client configuration file \"%s\""
            " from initialize/configuration API argument.\n", configFile); 
    } else {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Client configuration file name was not supplied from "
            "initialize/configuration API argument.\n"); 
    }

    /* Get configFile from environment variable */
    if (realConfigFile == NULL) {
        envConfigFile = getenv(NGCLI_ENVIRONMENT_CONFIG_FILE);
        if ((envConfigFile != NULL) && (envConfigFile[0] != '\0')) {
            realConfigFile = envConfigFile;

            ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Using client configuration file \"%s\""
                " from environment variable.\n", envConfigFile); 
        } else {
            ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Client configuration file name was not supplied from "
                "$%s environment variable.\n", NGCLI_ENVIRONMENT_CONFIG_FILE); 
        }
    }

    /* Get configFile from home directory */
    if (realConfigFile == NULL) {
        result = ngcllConfigFileGetHomeConfigFileName(
            context, &homeConfigFile, error);
        if ((result == 0) || (homeConfigFile == NULL)) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Configuration file name was not resolved.\n"); 
            return 0;
        }

        realConfigFile = homeConfigFile;

        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Using client configuration file \"%s\""
            " from home directory.\n", homeConfigFile); 
    }

    /* Read configuration file */
    result = ngcllConfigFileRead(context, realConfigFile, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Configuration file \"%s\" read failed.\n", realConfigFile); 
    }

    if (homeConfigFile != NULL) {
        ngiFree(homeConfigFile, log, error);
        homeConfigFile = NULL;

        if (result == 0) {
            ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Client configuration file name was not supplied"
                " from both initialize function argument and"
                " $%s environment variable.\n",
                NGCLI_ENVIRONMENT_CONFIG_FILE); 
        }
    }

    if (result == 0) {
        /* Failed */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the Home Configuration file name.
 * for grpc_initialize(NULL) and $NG_CONFIG_FILE not defined.
 */
static int
ngcllConfigFileGetHomeConfigFileName(
    ngclContext_t *context,
    char **homeConfigFile,
    int *error)
{
    static const char fName[] = "ngcllConfigFileGetHomeConfigFileName";
    size_t homeConfigLength;
    struct passwd *passwd;
    char *homeDir;
    ngLog_t *log;
    int result;
    char *buf;

    /* Check the arguments */
    assert(context != NULL);
    assert(homeConfigFile != NULL);

    log = context->ngc_log;
    homeDir = NULL;
    *homeConfigFile = NULL;
    homeConfigLength = 0;
    passwd = NULL;
    buf = NULL;

    /* Get the passwd database */
    result = ngiGetpwuid(geteuid(), &passwd, &buf, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "ngiGetpwuid() failed.\n");
        return 0;
    }

    homeDir = passwd->pw_dir;
    if ((homeDir == NULL) || (homeDir[0] == '\0')) {
        NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Home directory from getpwuid() is not valid.\n"); 
        goto error;
    }

    assert(NGCLI_CONFIG_FILE_HOME != NULL);

    homeConfigLength =
        strlen(homeDir) + strlen(NGCLI_CONFIG_FILE_HOME) + 1;

    *homeConfigFile = ngiCalloc(homeConfigLength, sizeof(char), log, error);
    if (*homeConfigFile == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for string.\n"); 
        goto error;
    }

    snprintf(*homeConfigFile, homeConfigLength, "%s%s",
        homeDir, NGCLI_CONFIG_FILE_HOME);

    result = ngiReleasePasswd(passwd, buf, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Release the password buffer failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if ((passwd != NULL) || (buf != NULL)) {
    result = ngiReleasePasswd(passwd, buf, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Release the password buffer failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * ConfigFileRead()
 */
static int
ngcllConfigFileRead(
    ngclContext_t *context,
    char *configFile,
    int *error)
{
    static const char fName[] = "ngcllConfigFileRead";
    ngiTokenReadInfo_t *tokenReadInfo;
    int result, fileOpen, mutexLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    mutexLocked = 0;
    tokenReadInfo = NULL;
    fileOpen = 0;

    /* Check the arguments */
    if (configFile == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "configFile is NULL.\n"); 
        goto error;
    }
    
    /* Mutex Lock */
    result = ngiMutexLock(&context->ngc_mutexConfig, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Lock the mutex failed.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /* Is already reading? */
    if (context->ngc_configFileReading == 1) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Configuration file is already reading.\n"); 
        goto error;
    }
    context->ngc_configFileReading = 1;

    /* log */
    if (context->ngc_configFileReadCount > 0) {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Reading the configuration file again.\n"); 
    }

    /* Open configuration file and get tokenReadInfo */
    result = ngiConfigFileOpen(configFile, &tokenReadInfo, 1, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Opening configuration file \"%s\" failed.\n", configFile); 
        goto error;
    }
    fileOpen = 1;

    /* Parse configuration file */
    result = ngcllConfigFileParse(context, tokenReadInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Parsing configuration file \"%s\" fail.\n", configFile); 
        goto error;
    }

    /* Close configuration file */
    result = ngiConfigFileClose(tokenReadInfo, log, error);
    fileOpen = 0;
    tokenReadInfo = NULL;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Closing configuration file \"%s\" failed.\n", configFile); 
        goto error;
    }

    context->ngc_configFileReadCount++;
    context->ngc_configFileReading = 0;

    /* log */
    if (context->ngc_configFileReadCount == 1) {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Reading the configuration file was successful.\n"); 
    } else {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Reading the configuration file was successful. (%d times)\n",
            context->ngc_configFileReadCount); 
    }

    /* Mutex Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&context->ngc_mutexConfig, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Close configuration file */
    if ((tokenReadInfo != NULL) && (fileOpen == 1)) {
        result = ngiConfigFileClose(tokenReadInfo, log, NULL);
        fileOpen = 0;
        tokenReadInfo = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Closing configuration file \"%s\" failed.\n", configFile); 
        }
    }

    context->ngc_configFileReading = 0;

    /* Mutex Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&context->ngc_mutexConfig, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Parsing functions
 *  This function reads sequential tokens,
 *  parse, check syntax, and call appropriate attribute function
 */
static int
ngcllConfigFileParse(
    ngclContext_t *context,
    ngiTokenReadInfo_t *tokenReadInfo,
    int *error)
{
    static const char fName[] = "ngcllConfigFileParse";
    ngcllReadingState_t *readingState;
    int result, attrFuncInitialized;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(tokenReadInfo != NULL);

    log = context->ngc_log;
    readingState = NULL;
    attrFuncInitialized = 0;

    /* Initialize Attribute function data */
    result = ngcllAttrFuncInitialize(context, &readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to initialize configuration file"
            " parsing data structure.\n"); 
        goto error;
    }
    attrFuncInitialized = 1;

    /* Remain given config file name */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedConfigs), tokenReadInfo->ntri_filename,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        goto error;
    }

    /* Parse */
    result = ngcllConfigFileParseSub(
        context, tokenReadInfo, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to parse configuration file.\n"); 
        goto error;
    }

    /* Register the Information */
    result = ngcllAttrFuncRegisterInformation(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register the configuration file data.\n"); 
        goto error;
    }

    /* Finalize Attribute function data */
    result = ngcllAttrFuncFinalize(
        context, readingState, error);
    attrFuncInitialized = 0;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to finalize configuration file"
            " parsing data structure.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize Attribute function data */
    if ((readingState != NULL) && (attrFuncInitialized == 1)) {
        result = ngcllAttrFuncFinalize(
            context, readingState, NULL);
        attrFuncInitialized = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to finalize configuration file"
                " parsing data structure.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/* enum for where ConfigFileParse reading */
typedef enum ngcllConfigFileParseReadingType_e {
    READING_GLOBAL,   /* reading not in SECTION */
    READING_SECTION   /* reading in SECTION     */
} ngcllConfigFileParseReadingType_t;

/**
 * main parsing function.
 *  This function is called recursively from <INCLUDE> section.
 */
static int
ngcllConfigFileParseSub(
    ngclContext_t *context,
    ngiTokenReadInfo_t *tokenReadInfo,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllConfigFileParseSub";
    ngiTokenInfo_t token_entity, *token;
    ngiTokenInfo_t argToken_entity, *argToken;
    ngcllConfigFileParseReadingType_t reading;
    char currentReadingSectionName[NGI_CONFIG_LINE_MAX];
    char tagName[NGI_CONFIG_LINE_MAX], attrName[NGI_CONFIG_LINE_MAX];
    ngcllTagFuncTable_t *tagTable, *tagItem;
    ngcllAttrFuncTable_t *attrTable;
    long prevLineNo;
    ngcllAttrFunc_t attrfnc; /* attribute (or section) function */
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(tokenReadInfo != NULL);
    assert(readingState != NULL);

    token = &token_entity;
    argToken = &argToken_entity;
    reading = READING_GLOBAL;
    tagTable = tagFuncTable;  /* use table in static global func table */
    tagItem = NULL;
    attrTable = NULL;
    log = context->ngc_log;

    prevLineNo = 0;

    while((result = ngiGetToken(
        tokenReadInfo, token, NGI_GETTOKEN_TOKEN, log, error))
            == NGI_GET_TOKEN_SUCCESS) {

        if (token->nti_readInfo->ntri_lineno == prevLineNo) {
            ngiConfigFileSyntaxError(log, token,
                "multiple keyword appear in same line",
                fName, NULL, token->nti_tokenStr, error);
            return 0;
        }
        prevLineNo = token->nti_readInfo->ntri_lineno;

        if (token->nti_type == NGI_TOKEN_TAG) {

            result = ngiGetTagName(token->nti_tokenStr, tagName);

            if (reading == READING_GLOBAL) {
                if (result != NGI_GET_TAGNAME_BEGIN) {
                    ngiConfigFileSyntaxError(log, token,
                        "Not starting section",
                        fName, NULL, token->nti_tokenStr, error);
                    return 0;
                }
                reading = READING_SECTION;
                strncpy(
                    currentReadingSectionName, tagName, NGI_CONFIG_LINE_MAX);

                tagItem = ngcllGetTagFuncInfo(tagTable, tagName);

                if (tagItem == NULL) {
                    ngiConfigFileSyntaxError(log, token,
                        "No such section", fName, NULL, tagName, error);
                    return 0;
                }

                attrTable = tagItem->ntft_attrs;
                /* This attrTable is used continuously until section ends */
                assert(attrTable != NULL);

                attrfnc = tagItem->ntft_tagBegin;
                assert(attrfnc != NULL);

                result = (*attrfnc)(tagName, token, readingState,
                    context, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "%s section open process fail.\n", tagName); 
                    return 0;
                }

            } else { /* reading == READING_SECTION */
                if (result != NGI_GET_TAGNAME_END) {
                    ngiConfigFileSyntaxError(log, token,
                        "Nested section not allowed",
                        fName, NULL, tagName, error);
                    return 0;
                }

                if (ngcllTagNameIsEqual(tagName, currentReadingSectionName,
                                          NGI_CONFIG_LINE_MAX) != 1) {
                    ngiConfigFileSyntaxError(log, token,
                        "Not the end of section", fName, NULL,
                        currentReadingSectionName, error);
                    return 0;
                }

                reading = READING_GLOBAL;
                strncpy(currentReadingSectionName, "", NGI_CONFIG_LINE_MAX);

                attrTable = NULL;

                /* reuse tagItem which set when SECTION was started */
                assert(tagItem != NULL);

                attrfnc = tagItem->ntft_tagEnd;
                assert(attrfnc != NULL);

                result = (*attrfnc)(tagName, token, readingState,
                    context, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "%s section close process fail.\n", tagName); 
                    return 0;
                }
            }

        } else if (token->nti_type == NGI_TOKEN_ATTR) {

            strncpy(attrName, token->nti_tokenStr, NGI_CONFIG_LINE_MAX);

            if (reading != READING_SECTION) {
                ngiConfigFileSyntaxError(log, token,
                    "Not starting with section",
                    fName, NULL, attrName, error);
                return 0;
            }

            assert(attrTable != NULL);
            attrfnc = ngcllGetAttrFuncPtr(attrTable, attrName);
            if (attrfnc == NULL) {
                ngiConfigFileSyntaxError(log, token,
                    "No such attribute", fName, NULL, attrName, error);
                return 0;
            }

            /* get attribute value (attribute argument) */
            result = ngiGetToken(tokenReadInfo, argToken,
                NGI_GETTOKEN_ARGS, log, error);
            if ((result != NGI_GET_TOKEN_SUCCESS)
                        || (argToken->nti_type != NGI_TOKEN_ATTR)) {
                ngiConfigFileSyntaxError(log, token,
                    "No argument for the attribute",
                    fName, attrName, NULL, error);
                return 0;
            }

            result = (*attrfnc)(attrName, argToken, readingState,
                context, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "attribute \"%s\" process fail.\n", attrName); 
                return 0;
            }

        } else {
            /**
             * (GetToken() == GET_TOKEN_SUCCESS) &&
             *    (token->nti_type != NG_TOKEN_TAG) &&
             *    (token->nti_type != NG_TOKEN_ATTR)
             * won't be happen
             */
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "GetToken failed.\n"); 
            abort();
            return 0;
        }
    }
    if (result != NGI_GET_TOKEN_EOF) {
        ngiConfigFileSyntaxError(log, token,
            "Invalid word", fName, NULL, token->nti_tokenStr, error);
        return 0;
    }

    if (reading != READING_GLOBAL) {
            ngiConfigFileSyntaxError(log, token,
                "Not yet finished section",
                 fName, NULL, currentReadingSectionName, error);
            return 0;
    }

    return 1;
}

/**
 * Check the tagname is equal.
 */
static int 
ngcllTagNameIsEqual(char *tag1, char *tag2, size_t max)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);

    cmp = (NGI_SECTION_NAME_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    if((*cmp)(tag1, tag2, max) != 0) {
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * This function gets TagTable element from
 *  argument given global variable tagTable[].
 */
ngcllTagFuncTable_t *
ngcllGetTagFuncInfo(ngcllTagFuncTable_t *tagTable, char *tagName)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    int i;

    cmp = (NGI_SECTION_NAME_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    for (i = 0; tagTable[i].ntft_tagName != NULL; i++) {
        if((*cmp)(
            tagTable[i].ntft_tagName, tagName, NGI_CONFIG_LINE_MAX) == 0) {

            /* Success */
            return &tagTable[i];
        }
    }

    /* Failed */
    return NULL;
}

/**
 * This function gets function pointer corresponding keyword attrName
 * in AttrTable
 */
ngcllAttrFunc_t
ngcllGetAttrFuncPtr(ngcllAttrFuncTable_t *attrTable, char *attrName)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    int i;

    cmp = (NGI_ATTR_NAME_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    for (i = 0; attrTable[i].naft_attrName != NULL; i++) {
        if((*cmp)(
            attrTable[i].naft_attrName, attrName, NGI_CONFIG_LINE_MAX) == 0) {

            /* Success */
            return attrTable[i].naft_func;
        }
    }

    /* Failed */
    return NULL;
}



/*****************************************************************/
/**
 * attribute functions below
 */

/**
 * attribute functions can get attribute value by calling
 *   ngiReadStringFromArg,
 *   ngiReadIntFromArg,
 *   ngiReadStringListFromArg, ...
 *    giving them with argument token string
 *
 * input : token (including attribute's value in string)
 * processing state keeping : ReadingInfo
 * output : context
 *
 * ngcllConfigFileParse calls following sequence
 * at first, ngcllAttrFuncInitialize
 * next, attribute functions (including section begin, end functions)
 * then, ngcllAttrFuncRegisterInformation
 * at last, ngcllAttrFuncFinalize
 */
static int
ngcllAttrFuncInitialize(
    ngclContext_t *context,
    ngcllReadingState_t **readingState,
    int *error)
{
    static const char fName[] = "ngcllAttrFuncInitialize";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    /* Construct */
    result = ngcllReadingStateConstruct(context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to construct configuration file"
            " parsing data structure.\n"); 
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncFinalize(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllAttrFuncFinalize";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    result = ngcllReadingStateDestruct(context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to destruct configuration file"
            " parsing data structure.\n"); 
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncRegisterInformation(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllAttrFuncRegisterInformation";
    ngLog_t *log;
    int qmLocked = 0;
    int result;
    int ret = 0;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* Register LocalMachineInformation */
    result = ngcllLocalMachineInformationsRegister(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register LocalMachineInformation.\n"); 
        return 0;
    }

    /* Register InvokeServerInformation */
    result = ngcllInvokeServerInformationsRegister(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register InvokeServerInformations.\n"); 
        return 0;
    }

    /* Register CommunicationProxyInformation */
    result = ngcllCommunicationProxyInformationsRegister(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register CommunicationProxyInformations.\n"); 
        return 0;
    }

    /* Register InformationServiceInformation */
    result = ngcllInformationServiceInformationsRegister(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register InformationServiceInformations.\n"); 
        return 0;
    }

    /* Register RemoteMachineInformation */
    result = ngcllRemoteMachineInformationsRegister(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register RemoteMachineInformations.\n"); 
        return 0;
    }

    /* Register DefaultRemoteMachineInformation */
    result = ngcllDefaultRemoteMachineInformationsRegister(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register DefaultRemoteMachineInformation.\n"); 
        return 0;
    }

    if (context->ngc_queryManager != NULL) {
        result = ngcliQueryManagerLockForReread(context->ngc_queryManager, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to lock the query manager for reading.\n"); 
            goto finalize;
        }
        qmLocked = 1;
    }

    /* Register ExecutablePathInformation */
    result = ngcllExecutablePathInformationsRegister(
        context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to register ExecutablePathInformations.\n"); 
        goto finalize;
    }

    /* Inactivate all Remote Class Information */
    result = ngcliRemoteClassInformationCacheInactivate(
        context, NULL, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to inactivate the RemoteClassInformation.\n"); 
        goto finalize;
    }

    if (context->ngc_queryManager != NULL) {
        result = ngcliQueryManagerReconstruct(context->ngc_queryManager, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't reconstruct the query manager.\n"); 
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (context->ngc_queryManager != NULL) {
        result = ngcliQueryManagerUnlockForReread(context->ngc_queryManager, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to unlock the query manager for reading.\n"); 
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

static int
ngcllReadingStateConstruct(
    ngclContext_t *context,
    ngcllReadingState_t **readingState,
    int *error)
{
    static const char fName[] = "ngcllReadingStateConstruct";
    ngcllReadingState_t *newState;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    *readingState = NULL;
    newState = NULL;

    /* Allocate */
    newState = NGI_ALLOCATE(ngcllReadingState_t, log, error);
    if (newState == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for configuration file"
            " parsing data structure.\n"); 
        goto error;
    }

    /* Configuration file */
    newState->nrs_appearedConfigs = NULL;

    /* LocalMachineInformation */
    newState->nrs_lmiAppeared = 0; /* FALSE */
    newState->nrs_lmInfo = ngcllLocalMachineInfoPairCreate(
        context, error);
    if (newState->nrs_lmInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Local Machine Information.\n"); 
        goto error;
    }

    result = ngcllLocalMachineInfoPairInitialize(
        context, newState->nrs_lmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Local Machine Information.\n"); 
        goto error;
    }

    /* InvokeServerInformation */
    newState->nrs_appearedInvokeServers = NULL;
    newState->nrs_invokeServerInfos = NULL;
    newState->nrs_curInvokeServerInfo = NULL;
    newState->nrs_curInvokeServerInfoOptions = NULL;

    /* CommunicationProxyInformation */
    newState->nrs_appearedCommunicationProxys = NULL;
    newState->nrs_cpInfos = NULL;
    newState->nrs_curCPinfo = NULL;
    newState->nrs_curCPinfoOptions = NULL;

    /* InformationServiceInformation */
    newState->nrs_appearedInformationServiceTags = NULL;
    newState->nrs_infoServiceInfos = NULL;
    newState->nrs_curInfoServiceInfo = NULL;
    newState->nrs_curInfoServiceInfoSources = NULL;
    newState->nrs_curInfoServiceInfoOptions = NULL;

    /* RemoteMachineInformation */
    newState->nrs_appearedNoTagRmInfoHosts = NULL;
    newState->nrs_appearedRmInfoTags = NULL;
    newState->nrs_appearedInvokeServersInRm = NULL;
    newState->nrs_appearedCommunicationProxysInRm = NULL;
    newState->nrs_rmInfos = NULL;

    newState->nrs_curRmInfo = NULL;
    newState->nrs_curRmInfoInvokeServerOptions = NULL;
    newState->nrs_curRmInfoCommunicationProxyOptions = NULL;
    newState->nrs_curRmInfoRSLextensions = NULL;
    newState->nrs_curRmInfoEnviron = NULL;
    newState->nrs_curRmInfoHosts = NULL;

    /* DefaultRemoteMachineInformation */
    newState->nrs_defRmInfoAppeared = 0; /* FALSE */
    newState->nrs_readingServerDefault = 0; /* FALSE */
    newState->nrs_defRmInfo = NULL;

    /* ExecutablePathInformation */
    newState->nrs_appearedRmInfoHostsInEp = NULL;
    newState->nrs_epInfos = NULL;
    newState->nrs_curEpInfo = NULL;

    *readingState = newState;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* LocalMachineInformation */
    if ((newState != NULL) && (newState->nrs_lmInfo != NULL)) {
        result = ngcllLocalMachineInfoPairDestruct(
            context, newState->nrs_lmInfo, NULL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct the storage for Local Machine Information.\n"); 
        newState->nrs_lmInfo = NULL;
    }

    if (newState != NULL) {
        result = NGI_DEALLOCATE(
            ngcllReadingState_t, newState, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't deallocate the storage for configuration file"
                " parsing data structure.\n"); 
        }
        newState = NULL;
    }

    /* Failed */
    return 0;
}

#define NGCLL_STRING_LIST_DESTRUCT(stringList, retResult) \
    { \
        int macroResult; \
         \
        if ((stringList) != NULL) { \
            macroResult = ngiStringListDestruct( \
                (stringList), log, error); \
            if (macroResult != 1) { \
                (retResult) = 0; \
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  \
                    "Can't destruct the string list.\n"); \
                    /* Not return */\
            } \
        } \
    }

static int
ngcllReadingStateDestruct(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllReadingStateDestruct"; 
    int result, retResult;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    retResult = 1;

    /* Configuration file */
    if (readingState->nrs_appearedConfigs == NULL) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid config data (appeared configuration file is NULL).\n"); 
    }
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedConfigs, retResult)

    /* LocalMachineInformation */
    if (readingState->nrs_lmInfo != NULL) {
        result = ngcllLocalMachineInfoPairDestruct(
            context, readingState->nrs_lmInfo, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Local Machine Information Pair.\n"); 
        }
    } else {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Local Machine Information Pair is not available.\n"); 
    }

    /* InvokeServerInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedInvokeServers, retResult)

    if (readingState->nrs_invokeServerInfos != NULL) {
        result = ngcllInvokeServerInfoPairListDestruct(
            context, readingState->nrs_invokeServerInfos, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Invoke Server Information"
                " Pair list.\n"); 
        }
    }

    if ((readingState->nrs_curInvokeServerInfo != NULL) ||
        (readingState->nrs_curInvokeServerInfoOptions != NULL)) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid config data"
            " (current Invoke Server Information is not NULL).\n"); 
    }

    /* CommunicationProxyInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedCommunicationProxys, retResult)

    if (readingState->nrs_cpInfos != NULL) {
        result = ngcllCommunicationProxyInfoPairListDestruct(
            context, readingState->nrs_cpInfos, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Communication Proxy Information"
                " Pair list.\n"); 
        }
    }

    if ((readingState->nrs_curCPinfo != NULL) ||
        (readingState->nrs_curCPinfoOptions != NULL)) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid config data"
            " (current Communication Proxy Information is not NULL).\n"); 
    }

    /* InformationServiceInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedInformationServiceTags, retResult)

    if (readingState->nrs_infoServiceInfos != NULL) {
        result = ngcllInformationServiceInfoPairListDestruct(
            context, readingState->nrs_infoServiceInfos, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Information Service Information"
                " Pair list.\n"); 
        }
    }

    if ((readingState->nrs_curInfoServiceInfo != NULL) ||
        (readingState->nrs_curInfoServiceInfoSources != NULL) ||
        (readingState->nrs_curInfoServiceInfoOptions != NULL)) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid config data"
            " (current Information Service Information is not NULL).\n"); 
    }

    /* RemoteMachineInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedNoTagRmInfoHosts, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedRmInfoTags, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedInvokeServersInRm, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedCommunicationProxysInRm, retResult)

    if (readingState->nrs_rmInfos != NULL) {
        result = ngcllRemoteMachineInfoPairListDestruct(
            context, readingState->nrs_rmInfos, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Remote Machine Information"
                " Pair list.\n"); 
        }
    }

    if ((readingState->nrs_curRmInfo != NULL) ||
        (readingState->nrs_curRmInfoInvokeServerOptions != NULL) ||
        (readingState->nrs_curRmInfoCommunicationProxyOptions != NULL) ||
        (readingState->nrs_curRmInfoRSLextensions != NULL) ||
        (readingState->nrs_curRmInfoEnviron != NULL) ||
        (readingState->nrs_curRmInfoHosts != NULL)) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid config data"
            " (current Remote Machine Information is not NULL).\n"); 
    }

    /* DefaultRemoteMachineInformation */
    if (readingState->nrs_defRmInfo != NULL) {
        result = ngcllRemoteMachineInfoPairDestruct(
            context, readingState->nrs_defRmInfo, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Default Remote Machine Information"
                " Pair.\n"); 
        }
    }

    /* ExecutablePathInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedRmInfoHostsInEp, retResult)

    if (readingState->nrs_epInfos != NULL) {
        result = ngcllExecutablePathInfoPairListDestruct(
            context, readingState->nrs_epInfos, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Executable Path Information"
                " Pair list.\n"); 
        }
    }

    if (readingState->nrs_curEpInfo != NULL) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid config data"
            " (current Executable Path Information is not NULL).\n"); 
    }

    result = NGI_DEALLOCATE(
            ngcllReadingState_t, readingState, log, error);
    if (result == 0) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the storage for configuration file"
            " parsing data structure.\n"); 
    }

    if (retResult != 1) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to destruct the storage for configuration file"
            " parsing data structure.\n"); 

        /* Failed */
        return 0;
    }

    /* Success */
    return 1;
}

#undef NGCLL_STRING_LIST_DESTRUCT

/**
 * Register LocalMachineInformation
 */
static int
ngcllLocalMachineInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllLocalMachineInformationsRegister";
    ngclLocalMachineInformation_t lmInfo;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* Skip if this call is not the first time configuration file read */
    if (context->ngc_configFileReadCount > 0) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Skip the Local Machine Information update.\n"); 
        /* Success */
        return 1;
    }

    /* clear to zero */
    result = ngcliLocalMachineInformationInitialize(context, &lmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Local Machine Information.\n"); 
        return 0;
    }

    /* set to application default */
    result = ngcllLocalMachineInformationSetDefault(&lmInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Local Machine Information to default.\n"); 
        return 0;
    }

    /* set to config file client section */
    result = ngcllLocalMachineInformationSetPair(
        &lmInfo, readingState->nrs_lmInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Local Machine Information Pair.\n"); 
        return 0;
    }

    /* Register to context */
    result = ngcliLocalMachineInformationCacheRegister(
        context, &lmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the Local Machine Information.\n"); 
        return 0;
    }

    result = ngclLocalMachineInformationRelease(context, &lmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Local Machine Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register all Invoke Server information.
 */
static int
ngcllInvokeServerInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInformationsRegister";
    ngiStringList_t *isInfosInRm, *curIsName;
    ngcllInvokeServerInfoPair_t *curIsInfoPair;
    int doNextRegister, registerCurRmInfo;
    ngclInvokeServerInformation_t isInfo;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;
    isInfosInRm = NULL;

    /* Get InvokeServers only in Remote Machine Information */
    result = ngiStringListSubtract(
        readingState->nrs_appearedInvokeServersInRm,
        readingState->nrs_appearedInvokeServers,
        NGI_FILENAME_CASE_SENSITIVE, &isInfosInRm, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to subtract StringList.\n"); 
        return 0;
    }

    /* first Invoke Server info */
    doNextRegister = 0;
    curIsInfoPair = NULL;
    curIsName = NULL;

    if (readingState->nrs_invokeServerInfos != NULL) {
        doNextRegister = 1;
        curIsInfoPair = readingState->nrs_invokeServerInfos;
    }

    if (isInfosInRm != NULL) {
        doNextRegister = 1;
        curIsName = isInfosInRm;
    }
    
    while (doNextRegister != 0) {
        /* Determine which, ISinfo or IS name. */
        registerCurRmInfo = 0;
        if (curIsInfoPair != NULL) {
            registerCurRmInfo = 1;
        } else if (curIsName != NULL) {
            registerCurRmInfo = 0;
        } else {
            abort();
        }

        /* Initialize */
        result = ngcliInvokeServerInformationInitialize(
            context, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Invoke Server Information.\n"); 
            return 0;
        }

        /* Set default */
        result = ngcllInvokeServerInformationSetDefault(
            &isInfo, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Invoke Server Information to default.\n"); 
            return 0;
        }

        if (registerCurRmInfo != 0) {
            assert(curIsInfoPair != NULL);

            /* set to config file */
            result = ngcllInvokeServerInformationSetPair(
                &isInfo, curIsInfoPair, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't set the Invoke Server Information.\n"); 
                return 0;
            }
        } else {
            /* Set type name */
            assert(curIsName->nsl_string != NULL);
            assert(isInfo.ngisi_type == NULL);
            isInfo.ngisi_type = strdup(curIsName->nsl_string);
            if (isInfo.ngisi_type == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't duplicate the string.\n"); 
                return 0;
            }
        }

        /* Check the setting of Invoke Server */
        result = ngcllInvokeServerInformationCheckValue(
            context, readingState, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to check the Invoke Server Information.\n"); 
            return 0;
        }

        /* Register to context */
        result = ngcliInvokeServerInformationCacheRegister(
            context, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the Invoke Server Information.\n"); 
            return 0;
        }

        /* Release */
        result = ngclInvokeServerInformationRelease(
            context, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release Invoke Server Information.\n"); 
            return 0;
        }

        /* Next Invoke Server info */
        if (registerCurRmInfo != 0) {
            curIsInfoPair = curIsInfoPair->nisip_next;
            doNextRegister = 0;
            if ((curIsInfoPair != NULL) || (curIsName != NULL)) {
                doNextRegister = 1;
            }
        } else {
            curIsName = curIsName->nsl_next;
            doNextRegister = ((curIsName != NULL) ? 1 : 0);
        }
    }

    if (isInfosInRm != NULL) {
        result = ngiStringListDestruct(isInfosInRm, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
    }

    /* Retire the current Invoke Servers. */
    result = ngcliInvokeServerRetire(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't retire the current Invoke Server.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllInvokeServerInformationCheckValue(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInformationCheckValue";
    ngLog_t *log;
    int result;

    /* Check arguments */
    assert(context != NULL);
    assert(readingState != NULL);
    assert(isInfo != NULL);

    log = context->ngc_log;

    /* Check the Invoke Server executable file */
    result = ngiExternalModuleProgramCheckAccess(
        NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER,
        NGI_EXTERNAL_MODULE_SUB_TYPE_NORMAL,
        isInfo->ngisi_type,
        isInfo->ngisi_path, /* NULL or path */
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invoke Server %s executable was not found.\n",
            isInfo->ngisi_type);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register all Communication Proxy information.
 */
static int
ngcllCommunicationProxyInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInformationsRegister";
    ngiStringList_t *cpInfosInRm, *curCpName;
    ngcllCommunicationProxyInfoPair_t *curCpInfoPair;
    int doNextRegister, registerCurRmInfo;
    ngclCommunicationProxyInformation_t cpInfo;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;
    cpInfosInRm = NULL;

    /* Get CommunicationProxys only in Remote Machine Information */
    result = ngiStringListSubtract(
        readingState->nrs_appearedCommunicationProxysInRm,
        readingState->nrs_appearedCommunicationProxys,
        NGI_FILENAME_CASE_SENSITIVE, &cpInfosInRm, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to subtract StringList.\n"); 
        return 0;
    }

    /* first Communication Proxy info */
    doNextRegister = 0;
    curCpInfoPair = NULL;
    curCpName = NULL;

    if (readingState->nrs_cpInfos != NULL) {
        doNextRegister = 1;
        curCpInfoPair = readingState->nrs_cpInfos;
    }

    if (cpInfosInRm != NULL) {
        doNextRegister = 1;
        curCpName = cpInfosInRm;
    }
    
    while (doNextRegister != 0) {
        /* Determine which, CPinfo or CP name. */
        registerCurRmInfo = 0;
        if (curCpInfoPair != NULL) {
            registerCurRmInfo = 1;
        } else if (curCpName != NULL) {
            registerCurRmInfo = 0;
        } else {
            abort();
        }

        /* Initialize */
        result = ngcliCommunicationProxyInformationInitialize(
            context, &cpInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Communication Proxy Information.\n"); 
            return 0;
        }

        /* Set default */
        result = ngcllCommunicationProxyInformationSetDefault(
            &cpInfo, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Communication Proxy Information to default.\n"); 
            return 0;
        }

        if (registerCurRmInfo != 0) {
            assert(curCpInfoPair != NULL);

            /* set to config file */
            result = ngcllCommunicationProxyInformationSetPair(
                &cpInfo, curCpInfoPair, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't set the Communication Proxy Information.\n"); 
                return 0;
            }
        } else {
            /* Set type name */
            assert(curCpName->nsl_string != NULL);
            assert(cpInfo.ngcpi_type == NULL);
            cpInfo.ngcpi_type = strdup(curCpName->nsl_string);
            if (cpInfo.ngcpi_type == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't duplicate the string.\n"); 
                return 0;
            }
        }

        /* Check the setting of Communication Proxy */
        result = ngcllCommunicationProxyInformationCheckValue(
            context, readingState, &cpInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to check the Communication Proxy Information.\n"); 
            return 0;
        }

        /* Register to context */
        result = ngcliCommunicationProxyInformationCacheRegister(
            context, &cpInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the Communication Proxy Information.\n"); 
            return 0;
        }

        /* Release */
        result = ngclCommunicationProxyInformationRelease(
            context, &cpInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release Communication Proxy Information.\n"); 
            return 0;
        }

        /* Next Communication Proxy info */
        if (registerCurRmInfo != 0) {
            curCpInfoPair = curCpInfoPair->ncpip_next;
            doNextRegister = 0;
            if ((curCpInfoPair != NULL) || (curCpName != NULL)) {
                doNextRegister = 1;
            }
        } else {
            curCpName = curCpName->nsl_next;
            doNextRegister = ((curCpName != NULL) ? 1 : 0);
        }
    }

    if (cpInfosInRm != NULL) {
        result = ngiStringListDestruct(cpInfosInRm, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
    }

    /* Retire the current Communication Proxy. */
    result = ngcliCommunicationProxyManagerRetire(
        context->ngc_communicationProxyManager, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't retire the Communication Proxy.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllCommunicationProxyInformationCheckValue(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInformationCheckValue";
    ngLog_t *log;
    int result;

    /* Check arguments */
    assert(context != NULL);
    assert(readingState != NULL);
    assert(cpInfo != NULL);

    log = context->ngc_log;

    /* Check the Client Communication Proxy executable file */
    result = ngiExternalModuleProgramCheckAccess(
        NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY,
        NGI_EXTERNAL_MODULE_SUB_TYPE_CLIENT_COMMUNICATION_PROXY,
        cpInfo->ngcpi_type,
        cpInfo->ngcpi_path, /* NULL or path */
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Communication Proxy %s executable was not found.\n",
            cpInfo->ngcpi_type);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register all Information Service information.
 */
static int
ngcllInformationServiceInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInformationsRegister";
    ngcllInformationServiceInfoPair_t *curIsInfoPair;
    ngclInformationServiceInformation_t isInfo;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    curIsInfoPair = readingState->nrs_infoServiceInfos;
    while (curIsInfoPair != NULL) {

        /* Initialize */
        result = ngcliInformationServiceInformationInitialize(
            context, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Information Service Information.\n"); 
            return 0;
        }

        /* Set default */
        result = ngcllInformationServiceInformationSetDefault(
            &isInfo, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Information Service Information to default.\n"); 
            return 0;
        }

        /* Set config file value */
        result = ngcllInformationServiceInformationSetPair(
            &isInfo, curIsInfoPair, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Information Service Information.\n"); 
            return 0;
        }

        /* Check the setting of Information Service */
        result = ngcllInformationServiceInformationCheckValue(
            context, readingState, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to check the Information Service Information.\n"); 
            return 0;
        }

        /* Register to context */
        result = ngcliInformationServiceInformationCacheRegister(
            context, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the Information Service Information.\n"); 
            return 0;
        }

        /* Release */
        result = ngclInformationServiceInformationRelease(
            context, &isInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release Information Service Information.\n"); 
            return 0;
        }

        curIsInfoPair = curIsInfoPair->nisip_next;
    }

    /* Success */
    return 1;
}

static int
ngcllInformationServiceInformationCheckValue(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInformationCheckValue";
    ngLog_t *log;
    int result;

    /* Check arguments */
    assert(context != NULL);
    assert(readingState != NULL);
    assert(isInfo != NULL);

    log = context->ngc_log;

    /* Check the Information Service executable file */
    result = ngiExternalModuleProgramCheckAccess(
        NGI_EXTERNAL_MODULE_TYPE_INFORMATION_SERVICE,
        NGI_EXTERNAL_MODULE_SUB_TYPE_NORMAL,
        isInfo->ngisi_type,
        isInfo->ngisi_path, /* NULL or path */
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Information Service %s executable was not found.\n",
            isInfo->ngisi_type);
        return 0;
    }

    /* Note: multiple same type definition is allowed. */

    /* Success */
    return 1;
}

/**
 * Register all SERVER section informations to context.
 */
static int
ngcllRemoteMachineInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInformationsRegister";
    ngclRemoteMachineInformation_t rmiCur; /* for to register to context */
    int rmiCurInitialized = 0;
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* Register RemoteMachineInformations in rmInfos */
    for (rmInfoPair = readingState->nrs_rmInfos;
         rmInfoPair != NULL;
         rmInfoPair = rmInfoPair->nrmip_next) {

        /* clear to zero */
        result = ngcliRemoteMachineInformationInitialize(
            context, &rmiCur, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Remote Machine Information.\n"); 
            goto error;
        }
        rmiCurInitialized = 1;

        /* set application default */
        result = ngcllRemoteMachineInformationSetDefault(&rmiCur, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Remote Machine Information to default.\n"); 
            goto error;
        }

        /* Sets values of <SERVER_DEFAULT> section in config file */
        if (readingState->nrs_defRmInfo != NULL) {
            result = ngcllRemoteMachineInformationSetPair(
                &rmiCur, readingState->nrs_defRmInfo, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't set the Remote Machine Information.\n"); 
                goto error;
            }
        }

        /* Set values of <SERVER> section in config file */
        result = ngcllRemoteMachineInformationSetPair(
            &rmiCur, rmInfoPair, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Remote Machine Information.\n"); 
            goto error;
        }

        /* Check the setting of server default section */
        result = ngcllRemoteMachineInformationCheckValue(
            context, readingState, &rmiCur, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to check the Remote Machine Information.\n"); 
            goto error;
        }

        /* Register to context */
        result = ngcliRemoteMachineInformationCacheRegister(
            context, &rmiCur, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the Remote Machine Information.\n"); 
            goto error;
        }

        /* Release */
        result = ngclRemoteMachineInformationRelease(context, &rmiCur, error);
        rmiCurInitialized = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release the Remote Machine Information.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;
error:
    /* Release */
    if (rmiCurInitialized != 0) {
        result = ngclRemoteMachineInformationRelease(context, &rmiCur, NULL);
        rmiCurInitialized = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release the Remote Machine Information.\n"); 
        }
    }
    
    return 0;
}

/**
 * Register information in readingState to default remote machine
 * information in Context.
 */
static int
ngcllDefaultRemoteMachineInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllDefaultRemoteMachineInformationsRegister";
    ngclRemoteMachineInformation_t drmInfo; /* for to register to context */
    int drmInfoInitialized = 0;
    ngLog_t *log;
    int ret = 0;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* clear to zero */
    result = ngcliRemoteMachineInformationInitialize(
        context, &drmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Remote Machine Information.\n"); 
        goto finalize;
    }
    drmInfoInitialized = 1;

    /* Sets to default */
    result = ngcllRemoteMachineInformationSetDefault(&drmInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Remote Machine Information to default.\n"); 
        goto finalize;
    }

    /* Sets to values of <SERVER_DEFAULT> section in config file */
    if (readingState->nrs_defRmInfo != NULL) {
        result = ngcllRemoteMachineInformationSetPair(
            &drmInfo, readingState->nrs_defRmInfo, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Default Remote Machine Information.\n"); 
            goto finalize;
        }
    }

    /* Check the setting of <SERVER_DEFAULT> section */
    result = ngcllRemoteMachineInformationCheckValue(
        context, readingState, &drmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to check the Remote Machine Information.\n"); 
        goto finalize;
    }

    /* Register to context */
    result = ngcliDefaultRemoteMachineInformationCacheRegister(
        context, &drmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the Remote Machine Information.\n"); 
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    /* Release */
    if (drmInfoInitialized != 0) {
        result = ngclRemoteMachineInformationRelease(context, &drmInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release the Remote Machine Information.\n"); 
            ret = 0;
            error = NULL;
        }
        drmInfoInitialized = 0;
    }

    return ret;
}

static int
ngcllRemoteMachineInformationCheckValue(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInformationCheckValue";
    char *hostname, *section;
    int result, isRegistered;
    ngLog_t *log;

    /* Check arguments */
    assert(context != NULL);
    assert(readingState != NULL);
    assert(rmInfo != NULL);

    log = context->ngc_log;
    isRegistered = 0;

    /* For log output */
    if (rmInfo->ngrmi_hostName == NULL) {
        /* server default section */
        section = "SERVER_DEFAULT section";
        hostname = "";
    } else {
        section = "SERVER section ";
        hostname = rmInfo->ngrmi_hostName;
    }

    if ((rmInfo->ngrmi_redirectEnable != 0) &&
        (rmInfo->ngrmi_jobEndTimeout == 0)) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "In %s%s, "
            "redirect_outerr may not work completely "
            "when job_stopTimeout is zero.\n", section, hostname);
    }

    /* Check Information Service Tag for this rmInfo registered */
    if (rmInfo->ngrmi_infoServiceTag != NULL) {
        result = ngiStringListCheckIncludeSameString(
            readingState->nrs_appearedInformationServiceTags,
            rmInfo->ngrmi_infoServiceTag,
            NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the StringList.\n"); 
            return 0;
        }

        if (!isRegistered) {
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "INFORMATION_SOURCE Tag \"%s\" for %s%s was not registered.\n",
                rmInfo->ngrmi_infoServiceTag, section, hostname); 
            return 0;
        }
    }

    /* Check Invoke Server. */
    if ((rmInfo->ngrmi_hostName != NULL) &&
        (rmInfo->ngrmi_invokeServerType == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invoke Server was not set for the server \"%s\".\n",
            hostname); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register all ExecutablePath section informations to context.
 */
static int
ngcllExecutablePathInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInformationsRegister";
    ngclExecutablePathInformation_t epInfo;
    ngcllExecutablePathInfoPair_t *epInfoPair;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* Inactivate all Executable Path Information. */
    result = ngcliExecutablePathInformationCacheInactivate(
        context, NULL, NULL, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't inactivate all Executable Path Information.\n"); 
        return 0;
    }

    for (epInfoPair = readingState->nrs_epInfos;
         epInfoPair != NULL;
         epInfoPair = epInfoPair->nepip_next) {

        assert(epInfoPair->nepip_entities->ngepi_hostName != NULL);
        assert(epInfoPair->nepip_entities->ngepi_className != NULL);

        /* clear to zero */
        result = ngcliExecutablePathInformationInitialize(
            context, &epInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Executable Path Information.\n"); 
            return 0;
        }

        /* set to application default */
        result = ngcllExecutablePathInformationSetDefault(&epInfo, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Executable Path Information to default.\n"); 
            return 0;
        }

        /* set to config file func info section */
        result = ngcllExecutablePathInformationSetPair(
            &epInfo, epInfoPair, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Executable Path Information.\n"); 
            return 0;
        }

        /* Register to RemoteMachineInformation */
        result = ngcliExecutablePathInformationCacheRegister(
            context, &epInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the Executable Path Information.\n"); 
            return 0;
        }

        /* Release */
        result = ngclExecutablePathInformationRelease(context, &epInfo, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release the Executable Path Information.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * template for each AttributeFunction members
 * first argument should member of
 *   ngclLocalMachineInformation_t or
 *   ngcliInvokeServerInformation_t or
 *   ngcliCommunicationProxyInformation_t or
 *   ngcliInformationServiceInformation_t or
 *   ngclRemoteMachineInformation_t
 * AF means ngcllAttrFunc
 */

#define NGCLL_AF_MEMBER_SET_STR(member, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
         \
        macroLog = context->ngc_log; \
         \
        if ((isset)->member != NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        if ((entity)->member != NULL) { \
            ngiFree((entity)->member, macroLog, error); \
        } \
         \
        (entity)->member = ngiReadStringFromArg( \
            (arg), macroLog, error); \
        if ((entity)->member == NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset)->member = NGI_ISSET_S_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_QSTR(member, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
         \
        macroLog = context->ngc_log; \
         \
        if ((isset)->member != NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        if ((entity)->member != NULL) { \
            ngiFree((entity)->member, macroLog, error); \
        } \
         \
        (entity)->member = ngiReadQuotedStringFromArg( \
            (arg), 0, NULL, 0, macroLog, error); \
        if ((entity)->member == NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset)->member = NGI_ISSET_S_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_BOOL(member, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
        char *true_false; \
        int result; \
         \
        macroLog = context->ngc_log; \
         \
        if ((isset)->member != 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        true_false = ngiReadStringFromArg( \
            (arg), macroLog, error); \
        if (true_false == NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadEnumFromArg( \
            true_false, NGI_ATTR_ARG_CASE_SENSITIVE, \
            2, "true", "false"); \
        if (!((result >= 1) && (result <= 2))) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            ngiFree(true_false, macroLog, error); \
            return 0; \
        } \
        if (result == 1) { \
            (entity)->member = 1; \
        } else if (result == 2) { \
            (entity)->member = 0; \
        } else { \
            abort(); \
        } \
        (isset)->member = NGI_ISSET_I_TRUE; \
        ngiFree(true_false, macroLog, error); \
    }

#define NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
        int resultValue; \
        int result; \
         \
        macroLog = context->ngc_log; \
        resultValue = 0; \
         \
        if ((isset)->member != 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        result = ngiReadIntFromArg((arg), &resultValue); \
        if (result == 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid (not integer):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (entity)->member = resultValue; \
        (isset)->member = NGI_ISSET_I_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_DOUBLE(member, minValue, arg, entity, isset) \
    { \
        double resultValue; \
        ngLog_t *macroLog; \
        int result; \
         \
        macroLog = context->ngc_log; \
        resultValue = 0.0; \
         \
        if (((isset)->member > 0.001) || \
            ((isset)->member < -0.001)) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        result = ngiReadDoubleFromArg((arg), &resultValue); \
        if (result == 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid (not integer):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        if (resultValue < ((minValue) - 0.000001)) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (entity)->member = resultValue; \
        (isset)->member = NGI_ISSET_D_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_LOGLEVEL(member, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
        char *levelStr; \
        int result; \
        \
        macroLog = context->ngc_log; \
         \
        if ((isset)->member != (ngLogLevel_t) 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        levelStr = ngiReadStringFromArg((arg), macroLog, error); \
        if (levelStr == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                 fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadEnumFromArg( \
            levelStr, NGI_ATTR_ARG_CASE_SENSITIVE, \
            12, "0", "1", "2", "3", "4", "5", \
            "Off", "Fatal", "Error", "Warning", "Information", "Debug"); \
         \
        if ((result >= 1) && (result <= 6)) { \
            (entity)->member = (ngLogLevel_t) (result - 1); \
        } else if ((result >= 7) && (result <= 12)) { \
            (entity)->member = (ngLogLevel_t) (result - 7); \
        } else { \
            ngiFree(levelStr, macroLog, error); \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                 fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset)->member = (ngLogLevel_t) NGI_ISSET_I_TRUE; \
        ngiFree(levelStr, macroLog, error); \
    }

#define NGCLL_AF_MEMBER_SET_UNIT( \
    member, minValue, arg, entity, isset, unitTable) \
    { \
        ngLog_t *macroLog; \
        char *unitString; \
        int resultValue; \
        int result; \
         \
        macroLog = context->ngc_log; \
        resultValue = 0; \
         \
        if ((isset)->member != 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        unitString = ngiReadStringFromArg( \
            (arg), macroLog, error); \
        if (unitString == NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadUnitNumFromArg(unitString, &resultValue, \
             unitTable, NGI_ATTR_ARG_CASE_SENSITIVE); \
        if (result == 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            ngiFree(unitString, macroLog, error); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            ngiFree(unitString, macroLog, error); \
            return 0; \
        } \
        (entity)->member = resultValue; \
        (isset)->member = NGI_ISSET_I_TRUE; \
        ngiFree(unitString, macroLog, error); \
    }

#define NGCLL_CONFIG_ATTRFUNC_ASSERTS \
    { \
        assert(attrName != NULL); \
        assert(token != NULL); \
        assert(token->nti_tokenStr != NULL); \
        assert(readingState != NULL); \
        assert(context != NULL); \
    }

/**
 * TAG / Attribute functions below
 */

/**
 * INCLUDE  section
 */

static int
ngcllAttrFuncIncludeBegin            NGCLL_CONFIG_ATTRFUNC_ARG
{
    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    /* Do nothing */
    return 1;
}

static int
ngcllAttrFuncIncludeEnd              NGCLL_CONFIG_ATTRFUNC_ARG
{
    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    /* Do nothing */
    return 1;
}

static int
ngcllAttrFuncInclude_filename        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInclude_filename";
    ngiTokenReadInfo_t *newTokenReadInfo;
    int result, isRegistered, fileOpen;
    char *newConfigFile;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    log = context->ngc_log;
    newConfigFile = NULL;
    newTokenReadInfo = NULL;
    fileOpen = 0;

    /* Get filename from argument */
    newConfigFile = ngiReadStringFromArg(token->nti_tokenStr, log, error);
    if (newConfigFile == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "No filename was specified", fName, attrName, NULL, error);
        goto error;
    }

    /* Check re-open */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedConfigs, newConfigFile,
        NGI_FILENAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        goto error;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "reading the same file again :",
            fName, attrName, newConfigFile, error);
        goto error;
    }

    /* register appeared config file names */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedConfigs), newConfigFile, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register StringList.\n"); 
        goto error;
    }

    result = ngiConfigFileOpen(
        newConfigFile, &newTokenReadInfo, 1, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Opening configuration file \"%s\" failed.\n", newConfigFile); 
        goto error;
    }
    fileOpen = 1;

    result = ngcllConfigFileParseSub(context,
        newTokenReadInfo, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to parse configuration file.\n"); 
        goto error;
    }

    result = ngiConfigFileClose(newTokenReadInfo, log, error);
    fileOpen = 0;
    newTokenReadInfo = NULL;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Closing configuration file fail.\n"); 
        goto error;
    }

    ngiFree(newConfigFile, log, error);
    newConfigFile = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if ((newTokenReadInfo != NULL) && (fileOpen == 1)) {
        result = ngiConfigFileClose(newTokenReadInfo, log, NULL);
        fileOpen = 0;
        newTokenReadInfo = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Closing configuration file fail.\n"); 
        }
    }

    if (newConfigFile != NULL) {
        ngiFree(newConfigFile, log, error);
        newConfigFile = NULL;
    }

    /* Failed */
    return 0;
}


/**
 * CLIENT  section
 */

static int
ngcllAttrFuncClientBegin             NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClientBegin";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    if (readingState->nrs_lmiAppeared) {
        ngiConfigFileSyntaxError(context->ngc_log, token,
                "redefining section:", fName, NULL, attrName, error);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncClientEnd               NGCLL_CONFIG_ATTRFUNC_ARG
{
    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    readingState->nrs_lmiAppeared = 1; /* TRUE */

    return 1;
}

/**
 * template for each ngclLocalMachineInformation_t members
 * first argument should member of ngclLocalMachineInformation_t
 * AFC means ngcllAttrFuncClient
 */

#define NGCLL_AFC_MEMBER_SET_STR(member, arg) \
         NGCLL_AF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_lmInfo->nlmip_entities, \
             readingState->nrs_lmInfo->nlmip_isSet)

#define NGCLL_AFC_MEMBER_SET_BOOL(member, arg) \
         NGCLL_AF_MEMBER_SET_BOOL(member, arg, \
             readingState->nrs_lmInfo->nlmip_entities, \
             readingState->nrs_lmInfo->nlmip_isSet)

#define NGCLL_AFC_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_lmInfo->nlmip_entities, \
             readingState->nrs_lmInfo->nlmip_isSet)

#define NGCLL_AFC_MEMBER_SET_LOGLEVEL(member, arg) \
         NGCLL_AF_MEMBER_SET_LOGLEVEL(member, arg, \
             readingState->nrs_lmInfo->nlmip_entities, \
             readingState->nrs_lmInfo->nlmip_isSet)

#define NGCLL_AFC_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_lmInfo->nlmip_entities, \
             readingState->nrs_lmInfo->nlmip_isSet, unitTable)

static int
ngcllAttrFuncClient_hostname         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_hostname";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(nglmi_hostName, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_save_sessionInfo NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_save_sessionInfo";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_INT(nglmi_saveNsessions, -1, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_loglevel         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logLevels.nglli_level, token->nti_tokenStr)
    return 1;
}

static int
ngcllAttrFuncClient_loglevel_ngprot  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel_ngprot";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logLevels.nglli_ninfgProtocol, token->nti_tokenStr)
    return 1;
}

static int
ngcllAttrFuncClient_loglevel_ngi     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel_ngi";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logLevels.nglli_ninfgInternal, token->nti_tokenStr)
    return 1;
}

static int
ngcllAttrFuncClient_loglevel_grpc    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel_grpc";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logLevels.nglli_grpc, token->nti_tokenStr)
    return 1;
}

static int
ngcllAttrFuncClient_log_filePath     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_log_filePath";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(nglmi_logInfo.ngli_filePath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_log_suffix       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_log_suffix";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(nglmi_logInfo.ngli_suffix, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_log_nFiles       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_log_nFiles";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_INT(nglmi_logInfo.ngli_nFiles, 0, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_log_maxFileSize  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_log_maxFileSize";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_UNIT(
        nglmi_logInfo.ngli_maxFileSize, 0,
        token->nti_tokenStr, ngiSizeUnitTable)

    return 1;
}

static int
ngcllAttrFuncClient_log_overWrite    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_log_overWrite";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_BOOL(
        nglmi_logInfo.ngli_overWriteDir, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_tmpDir           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_tmpDir";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(nglmi_tmpDir, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_refresh_cred     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_refresh_cred";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_UNIT(
        nglmi_refreshInterval, 0, token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

static int
ngcllAttrFuncClient_invoke_server_log  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_invoke_server_log";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(nglmi_invokeServerLog, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_comm_proxy_log     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_comm_proxy_log";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(nglmi_commProxyLog, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_info_service_log   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_info_service_log";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(nglmi_infoServiceLog, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_fortran_compatible    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_fortran_compatible";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_BOOL(nglmi_fortranCompatible, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_handling_signals NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_handling_signals";
    int (*cmp)(const char *s1, const char *s2, size_t n);
    int nSignals, *signalNumbers, found, foundNumber, i, j;
    ngiStringList_t *paramSignalStrings, *cur;
    ngcllLocalMachineInfoPair_t *lmInfoPair;
    char **signalNames, *end, *endptr;
    int nParamSignals, *paramTable;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    lmInfoPair = readingState->nrs_lmInfo;
    paramSignalStrings = NULL;
    nParamSignals = 0;
    paramTable = NULL;
    nSignals = 0;
    signalNames = NULL;
    signalNumbers = NULL;

    cmp = (NGI_ATTR_ARG_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    if (lmInfoPair->nlmip_isSet->nglmi_signals != NULL) {
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0; 
    }

    paramSignalStrings = ngiReadStringListFromArg(
        token->nti_tokenStr, log, error);
    /* NULL is valid */

    result = ngiStringListCount(
        paramSignalStrings, &nParamSignals, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Count the string list failed.\n"); 
        goto error;
    }

    paramTable = ngiCalloc(sizeof(int), nParamSignals + 1, log, error);
    if (paramTable == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the signal table.\n"); 
        goto error;
    }

    for (i = 0; i < (nParamSignals + 1); i++) {
        paramTable[i] = 0; /* 0 is to terminate */
    }

    /* Get the signal names */
    result = ngiSignalManagerSignalNamesGet(
        &signalNames, &signalNumbers, &nSignals, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the signal table.\n"); 
        goto error;
    }

    cur = paramSignalStrings;
    i = 0;
    while (cur != NULL) {
        /* Is attribute value "none"? */
        if ((i == 0) &&
            (nParamSignals == 1) &&
            ((*cmp)(cur->nsl_string, "none", strlen(cur->nsl_string) + 1)
                == 0)) {
            /* Signal is empty */
            break;
        }

        /* Decode the attribute value by name */
        found = 0;
        foundNumber = 0;
        for (j = 0; j < nSignals; j++) {
            /* The case sensitive search. */
            result = (*cmp)(signalNames[j], cur->nsl_string,
                strlen(signalNames[j]) + 1);
            if (result == 0) {
                /* Found */
                found = 1;
                foundNumber = signalNumbers[j];
                break;
            }
        }
        
        /* Decode the attribute value by number */
        if (found != 1) {
            end = &(cur->nsl_string[strlen(cur->nsl_string)]);
            foundNumber = (int)strtol(cur->nsl_string, &endptr, 10);
            if ((foundNumber <= 0) || (endptr != end)) {
                ngiConfigFileSyntaxError(log, token,
                    "invalid signal name: ",
                    fName, attrName, cur->nsl_string, error);
                goto error;
            }
            found = 1;
        }

        /* Is this first appearance? */
        for (j = 0; j < i; j++) {
            if (paramTable[j] == foundNumber) {
                ngiConfigFileSyntaxError(log, token,
                    "signal already appeared: ",
                    fName, attrName, cur->nsl_string, error);
                goto error;
            }
        }
        
        assert(found == 1);
        paramTable[i] = foundNumber;

        cur = cur->nsl_next;
        i++;
    }

    lmInfoPair->nlmip_entities->nglmi_signals = paramTable;
    lmInfoPair->nlmip_isSet->nglmi_signals = NGCLL_ISSET_IP_TRUE;
    paramTable = NULL;

    if (paramSignalStrings != NULL) {
        result = ngiStringListDestruct(
            paramSignalStrings, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            paramSignalStrings = NULL;
            goto error;
        }
        paramSignalStrings = NULL;
    }

    result = ngiSignalManagerSignalNamesDestruct(
        signalNames, signalNumbers, nSignals, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct the Signal Names.\n"); 
        signalNames = NULL;
        signalNumbers = NULL;
        goto error;
    }
    signalNames = NULL;
    signalNumbers = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (paramSignalStrings != NULL) {
        result = ngiStringListDestruct(
            paramSignalStrings, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            paramSignalStrings = NULL;
        }
    }

    if (paramTable != NULL) {
        ngiFree(paramTable, log, NULL);
        paramTable = NULL;
    }

    if ((signalNames != NULL) || (signalNumbers != NULL)) {
        result = ngiSignalManagerSignalNamesDestruct(
            signalNames, signalNumbers, nSignals, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Signal Names.\n"); 
        }
    }

    /* Failed */
    return 0;
}

static int
ngcllAttrFuncClient_listen_port NGCLL_CONFIG_ATTRFUNC_ARG
{
    ngLog_t *log;
    static const char fName[] = "ngcllAttrFuncClient_listen_port";

    log = context->ngc_log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_INT(nglmi_listenPort, 0, token->nti_tokenStr)

    if (readingState->nrs_lmInfo->nlmip_entities->nglmi_listenPort >
        NGI_PORT_MAX) {
        ngiConfigFileSyntaxError(log, token,
            "attribute value invalid (too large):",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    return 1;
}

#undef NGCLL_AFC_MEMBER_SET_STR
#undef NGCLL_AFC_MEMBER_SET_BOOL
#undef NGCLL_AFC_MEMBER_SET_INT
#undef NGCLL_AFC_MEMBER_SET_LOGLEVEL
#undef NGCLL_AFC_MEMBER_SET_UNIT

/**
 * <FUNCTION_INFO> section
 */

static int
ngcllAttrFuncFuncInfoBegin           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfoBegin";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_curEpInfo == NULL);

    log = context->ngc_log;

    /* Create ExecutablePathInfoPair to register */
    readingState->nrs_curEpInfo = ngcllExecutablePathInfoPairCreate(
        context, error);
    if (readingState->nrs_curEpInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage to "
            "register Executable Path Information.\n"); 
        return 0;
    }

    result = ngcllExecutablePathInfoPairInitialize(context,
        readingState->nrs_curEpInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Executable Path Info Pair.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllAttrFuncFuncInfoEnd             NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfoEnd";
    ngcllExecutablePathInfoPair_t *epInfoPair, *foundPair;
    int result, isRegistered;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    epInfoPair = readingState->nrs_curEpInfo;

    /* Validity check */
    if (epInfoPair->nepip_isSet->ngepi_hostName == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "No hostname in section:", fName, NULL, attrName, error);
        return 0;
    }
    if (epInfoPair->nepip_isSet->ngepi_className == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "No funcname in section:", fName, NULL, attrName, error);
        return 0;
    }
    if (epInfoPair->nepip_isSet->ngepi_path == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "No path in section:", fName, NULL, attrName, error);
        return 0;
    }

    /* Register to appearedRmInfoHostsInEp */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedRmInfoHostsInEp,
        epInfoPair->nepip_entities->ngepi_hostName,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (!isRegistered) {
        result = ngiStringListRegister(
            &(readingState->nrs_appearedRmInfoHostsInEp),
            epInfoPair->nepip_entities->ngepi_hostName, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register StringList.\n"); 
            return 0;
        } 
    }

    /* Check if already registered */
    foundPair = ngcllExecutablePathInfoPairGet(
        readingState->nrs_epInfos, 
        epInfoPair->nepip_entities->ngepi_hostName,
        epInfoPair->nepip_entities->ngepi_className);
    if (foundPair != NULL) {
        ngiConfigFileSyntaxError(log, token,
            "Already registered function in the host:", fName, NULL,
                epInfoPair->nepip_entities->ngepi_className, error);
        return 0;
    }

    /* Register to epInfos */
    result = ngcllExecutablePathInfoPairRegister(&(readingState->nrs_epInfos),
            epInfoPair);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register Executable Path Info.\n"); 
        return 0;
    }
    readingState->nrs_curEpInfo = NULL;

    /* Success */
    return 1;
}

/**
 * template for each ngclExecutablePathInformation_t members
 * first argument should member of ngclExecutablePathInformation_t
 * AFF means ngcllAttrFuncFuncInfo
 */

#define NGCLL_AFF_MEMBER_SET_STR(member, arg) \
         NGCLL_AF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_curEpInfo->nepip_entities, \
             readingState->nrs_curEpInfo->nepip_isSet)

#define NGCLL_AFF_MEMBER_SET_BOOL(member, arg) \
         NGCLL_AF_MEMBER_SET_BOOL(member, arg, \
             readingState->nrs_curEpInfo->nepip_entities, \
             readingState->nrs_curEpInfo->nepip_isSet)

#define NGCLL_AFF_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_curEpInfo->nepip_entities, \
             readingState->nrs_curEpInfo->nepip_isSet)

#define NGCLL_AFF_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_curEpInfo->nepip_entities, \
             readingState->nrs_curEpInfo->nepip_isSet, unitTable)

static int
ngcllAttrFuncFuncInfo_hostname       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_hostname";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_STR(ngepi_hostName, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncFuncInfo_funcname       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_funcname";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_STR(ngepi_className, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncFuncInfo_path           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_path";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_STR(ngepi_path, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncFuncInfo_staging        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_staging";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_BOOL(ngepi_stagingEnable, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncFuncInfo_backend        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_backend";
    ngcllExecutablePathInfoPair_t *epInfoPair;
    char *backendStr;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    epInfoPair = readingState->nrs_curEpInfo;

    if (epInfoPair->nepip_isSet->ngepi_backend != 0) {
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0;
    }

    /* Get argument string */
    backendStr = ngiReadStringFromArg(token->nti_tokenStr, log, error);
    if (backendStr == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal backend [normal/mpi/blacs]:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* Set argument enum */
    result = ngiReadEnumFromArg(
        backendStr, NGI_ATTR_ARG_CASE_SENSITIVE,
        3, "normal", "mpi", "blacs");
    if (result == 0) {
        ngiConfigFileSyntaxError(log, token,
            "No such backend [normal/mpi/blacs]:",
            fName, attrName, backendStr, error);
        ngiFree(backendStr, log, error);
        return 0;
    }
    ngiFree(backendStr, log, error);

    if (result == 1) {
        epInfoPair->nepip_entities->ngepi_backend = NG_BACKEND_NORMAL;
    } else if (result == 2) {
        epInfoPair->nepip_entities->ngepi_backend = NG_BACKEND_MPI;
    } else if (result == 3) {
        epInfoPair->nepip_entities->ngepi_backend = NG_BACKEND_BLACS;
    } else {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "reading backend string fail. result is %d\n", result); 
        return 0;
    }
    epInfoPair->nepip_isSet->ngepi_backend = (ngBackend_t) NGI_ISSET_I_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncFuncInfo_session_timeout  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_session_timeout";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_UNIT(
        ngepi_sessionTimeout, 0, token->nti_tokenStr, ngiTimeUnitTable)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncFuncInfo_trans_timeout_arg   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_trans_timeout_arg";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_UNIT(
        ngepi_transferTimeout_argument, 0,
        token->nti_tokenStr, ngiTimeUnitTable)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncFuncInfo_trans_timeout_res   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_trans_timeout_res";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_UNIT(
        ngepi_transferTimeout_result, 0,
        token->nti_tokenStr, ngiTimeUnitTable)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncFuncInfo_trans_timeout_cbarg NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_trans_timeout_cbarg";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_UNIT(
        ngepi_transferTimeout_cbArgument, 0,
        token->nti_tokenStr, ngiTimeUnitTable)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncFuncInfo_trans_timeout_cbres NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_trans_timeout_cbres";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_UNIT(
        ngepi_transferTimeout_cbResult, 0,
        token->nti_tokenStr, ngiTimeUnitTable)

    /* Success */
    return 1;
}

#undef NGCLL_AFF_MEMBER_SET_STR
#undef NGCLL_AFF_MEMBER_SET_BOOL
#undef NGCLL_AFF_MEMBER_SET_INT
#undef NGCLL_AFF_MEMBER_SET_UNIT

/**
 * INVOKE_SERVER section
 */

static int
ngcllAttrFuncInvokeServerBegin       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServerBegin";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_curInvokeServerInfo == NULL);

    log = context->ngc_log;

    /* Construct InvokeServerInfoPair */
    readingState->nrs_curInvokeServerInfo = ngcllInvokeServerInfoPairCreate(
        context, error);
    if (readingState->nrs_curInvokeServerInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate Invoke Server Information.\n"); 
        return 0;
    }

    result = ngcllInvokeServerInfoPairInitialize(
        context, readingState->nrs_curInvokeServerInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Invoke Server Information.\n"); 
        return 0;
    }

    assert(readingState->nrs_curInvokeServerInfoOptions == NULL);

    /* Success */
    return 1;
}

static int
ngcllAttrFuncInvokeServerEnd         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServerEnd";
    ngcllInvokeServerInfoPair_t *isInfoPair;
    int result, isRegistered;
    char *isTypeName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    isInfoPair = readingState->nrs_curInvokeServerInfo;

    if (isInfoPair->nisip_isSet->ngisi_type == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "No type name in section:",
                 fName, NULL, attrName, error);
        return 0;
    }

    isTypeName = isInfoPair->nisip_entities->ngisi_type;
    assert(isTypeName != NULL);

    /* assert isTypeName not registered */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedInvokeServers, isTypeName,
        NGI_FILENAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "Invoke Server information already registered for:",
            fName, NULL, isTypeName, error);
        return 0;
    }

    /* Register to appearedInvokeServers */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedInvokeServers), isTypeName,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register StringList.\n"); 
        return 0;
    }

    /* Set options */
    if (readingState->nrs_curInvokeServerInfoOptions != NULL) {
        result = ngcllInvokeServerInfoPairSetOptions(
            context, readingState->nrs_curInvokeServerInfo,
            readingState->nrs_curInvokeServerInfoOptions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set options to InvokeServerInfoPair.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curInvokeServerInfoOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
        readingState->nrs_curInvokeServerInfoOptions = NULL;
    }

    /* Move to curISinfo to isInfos */
    isInfoPair = readingState->nrs_curInvokeServerInfo;
    readingState->nrs_curInvokeServerInfo = NULL;

    result = ngcllInvokeServerInfoPairRegister(
        &(readingState->nrs_invokeServerInfos), isInfoPair);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register Invoke Server Info.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * template for each ngclInvokeServerInformation_t members
 * first argument should member of ngclInvokeServerInformation_t
 * AFI means ngcllAttrFuncInvokeServer
 */

#define NGCLL_AFI_MEMBER_SET_STR(member, arg) \
         NGCLL_AF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_curInvokeServerInfo->nisip_entities, \
             readingState->nrs_curInvokeServerInfo->nisip_isSet)

#define NGCLL_AFI_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_curInvokeServerInfo->nisip_entities, \
             readingState->nrs_curInvokeServerInfo->nisip_isSet)

#define NGCLL_AFI_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_curInvokeServerInfo->nisip_entities, \
             readingState->nrs_curInvokeServerInfo->nisip_isSet, unitTable)


static int
ngcllAttrFuncInvokeServer_type       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServer_type";
    ngcllInvokeServerInfoPair_t *isInfoPair;
    int result, isRegistered;
    char *typeName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_type, token->nti_tokenStr)

    log = context->ngc_log;
    isInfoPair = readingState->nrs_curInvokeServerInfo;

    typeName = isInfoPair->nisip_entities->ngisi_type;
    assert(typeName != NULL);

    /* type name registered check */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedInvokeServers, typeName,
        NGI_ATTR_NAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "Invoke Server information already registered for:",
            fName, attrName, typeName, error);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncInvokeServer_path       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServer_path";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_path, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncInvokeServer_max_jobs   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServer_max_jobs";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_INT(ngisi_maxJobs, 0, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncInvokeServer_logfile    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServer_logfile";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_logFilePath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncInvokeServer_statusPoll NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServer_statusPoll";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_UNIT(
        ngisi_statusPoll, 0, token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

static int
ngcllAttrFuncInvokeServer_option     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServer_option";
    char *optString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /**
     * option is able to appear multiple times in one section
     * so don't check member set or not.
     */
    optString = ngiReadQuotedStringFromArg(
        token->nti_tokenStr, 0, NULL, 0, log, error);
    if (optString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curInvokeServerInfoOptions), optString,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        return 0;
    }
    ngiFree(optString, log, error);

    return 1;
}

#undef NGCLL_AFI_MEMBER_SET_STR
#undef NGCLL_AFI_MEMBER_SET_INT
#undef NGCLL_AFI_MEMBER_SET_UNIT

/**
 * CLIENT_COMMUNICATION_PROXY section
 */


static int
ngcllAttrFuncCommunicationProxyBegin     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxyBegin";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_curCPinfo == NULL);

    log = context->ngc_log;

    /* Construct CommunicationProxyInfoPair */
    readingState->nrs_curCPinfo = ngcllCommunicationProxyInfoPairCreate(
        context, error);
    if (readingState->nrs_curCPinfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate Communication Proxy Information.\n"); 
        return 0;
    }

    result = ngcllCommunicationProxyInfoPairInitialize(
        context, readingState->nrs_curCPinfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Communication Proxy Information.\n"); 
        return 0;
    }

    assert(readingState->nrs_curCPinfoOptions == NULL);

    /* Success */
    return 1;
}

static int
ngcllAttrFuncCommunicationProxyEnd       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxyEnd";
    ngcllCommunicationProxyInfoPair_t *cpInfoPair;
    int result, isRegistered;
    char *cpTypeName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    cpInfoPair = readingState->nrs_curCPinfo;

    if (cpInfoPair->ncpip_isSet->ngcpi_type == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "No type name in section:",
                 fName, NULL, attrName, error);
        return 0;
    }

    cpTypeName = cpInfoPair->ncpip_entities->ngcpi_type;
    assert(cpTypeName != NULL);

    /* assert cpTypeName not registered */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedCommunicationProxys, cpTypeName,
        NGI_FILENAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "Communication Proxy information already registered for:",
            fName, NULL, cpTypeName, error);
        return 0;
    }

    /* Register to appearedCommunicationProxys */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedCommunicationProxys), cpTypeName,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register StringList.\n"); 
        return 0;
    }

    /* Set options */
    if (readingState->nrs_curCPinfoOptions != NULL) {
        result = ngcllCommunicationProxyInfoPairSetOptions(
            context, readingState->nrs_curCPinfo,
            readingState->nrs_curCPinfoOptions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set options to CommunicationProxyInfoPair.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curCPinfoOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
        readingState->nrs_curCPinfoOptions = NULL;
    }

    /* Move to curCPinfo to cpInfos */
    cpInfoPair = readingState->nrs_curCPinfo;
    readingState->nrs_curCPinfo = NULL;

    result = ngcllCommunicationProxyInfoPairRegister(
        &(readingState->nrs_cpInfos), cpInfoPair);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register Communication Proxy Info.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * template for each ngclCommunicationProxyInformation_t members
 * first argument should member of ngclCommunicationProxyInformation_t
 * AFI means ngcllAttrFuncCommunicationProxy
 */

#define NGCLL_AFC_MEMBER_SET_STR(member, arg) \
         NGCLL_AF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_curCPinfo->ncpip_entities, \
             readingState->nrs_curCPinfo->ncpip_isSet)

#define NGCLL_AFC_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_curCPinfo->ncpip_entities, \
             readingState->nrs_curCPinfo->ncpip_isSet)

#define NGCLL_AFC_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_curCPinfo->ncpip_entities, \
             readingState->nrs_curCPinfo->ncpip_isSet, unitTable)


static int
ngcllAttrFuncCommunicationProxy_type     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxy_type";
    ngcllCommunicationProxyInfoPair_t *cpInfoPair;
    int result, isRegistered;
    char *typeName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(ngcpi_type, token->nti_tokenStr)

    log = context->ngc_log;
    cpInfoPair = readingState->nrs_curCPinfo;

    typeName = cpInfoPair->ncpip_entities->ngcpi_type;
    assert(typeName != NULL);

    /* type name registered check */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedCommunicationProxys, typeName,
        NGI_ATTR_NAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "Communication Proxy information already registered for:",
            fName, attrName, typeName, error);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncCommunicationProxy_path     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxy_path";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(ngcpi_path, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncCommunicationProxy_bufsize  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxy_bufsize";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_UNIT(ngcpi_bufferSize, 0, token->nti_tokenStr,
        ngiSizeUnitTable)

    return 1;
}

static int
ngcllAttrFuncCommunicationProxy_max_jobs NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxy_max_jobs";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_INT(ngcpi_maxJobs, 0, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncCommunicationProxy_logfile  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxy_logfile";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_STR(ngcpi_logFilePath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncCommunicationProxy_option   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncCommunicationProxy_option";
    char *optString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /**
     * option is able to appear multiple times in one section
     * so don't check member set or not.
     */
    optString = ngiReadQuotedStringFromArg(
        token->nti_tokenStr, 0, NULL, 0, log, error);
    if (optString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curCPinfoOptions), optString, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        return 0;
    }
    ngiFree(optString, log, error);

    return 1;
}

#undef NGCLL_AFC_MEMBER_SET_STR
#undef NGCLL_AFC_MEMBER_SET_INT
#undef NGCLL_AFC_MEMBER_SET_UNIT

/**
 * INFORMATION_SOURCE section
 */

static int
ngcllAttrFuncInformationSourceBegin    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSourceBegin";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_curInfoServiceInfo == NULL);

    log = context->ngc_log;

    /* Construct InformationServiceInfoPair */
    readingState->nrs_curInfoServiceInfo =
        ngcllInformationServiceInfoPairCreate(context, error);
    if (readingState->nrs_curInfoServiceInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate Information Service Information.\n"); 
        return 0;
    }

    result = ngcllInformationServiceInfoPairInitialize(
        context, readingState->nrs_curInfoServiceInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Information Service Information.\n"); 
        return 0;
    }

    assert(readingState->nrs_curInfoServiceInfoSources == NULL);
    assert(readingState->nrs_curInfoServiceInfoOptions == NULL);

    /* Success */
    return 1;
}

static int
ngcllAttrFuncInformationSourceEnd      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSourceEnd";
    ngcllInformationServiceInfoPair_t *isInfoPair;
    int result, isRegistered;
    char *isTagName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    isInfoPair = readingState->nrs_curInfoServiceInfo;
    isTagName = NULL;

    /* Check type. */
    if (isInfoPair->nisip_isSet->ngisi_type == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "No type name in section:",
                 fName, NULL, attrName, error);
        return 0;
    }
    assert(isInfoPair->nisip_entities->ngisi_type != NULL);
    /* Note: Duplicated type entry is allowed. */

    /* Check tag. */

    /**
     * Note: Since duplicated type entry allowed,
     *       the tag name must be set,
     *       for to replace precise entry on reloading configuration file. 
     */

    if (isInfoPair->nisip_isSet->ngisi_tag == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "No tag name in section:",
                 fName, NULL, attrName, error);
        return 0;
    }

    isTagName = isInfoPair->nisip_entities->ngisi_tag;
    assert(isTagName != NULL);

    /* Check isTagName not registered */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedInformationServiceTags,
        isTagName,
        NGI_FILENAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "Information Service information already registered for tag:",
            fName, NULL, isTagName, error);
        return 0;
    }

    /* Register to appearedInformationServiceTags */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedInformationServiceTags),
        isTagName, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register StringList.\n"); 
        return 0;
    }

    /* Check sources */
    if (readingState->nrs_curInfoServiceInfoSources == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "No source in section:",
                 fName, NULL, attrName, error);
        return 0;
    }

    /* Set sources */
    assert(readingState->nrs_curInfoServiceInfoSources != NULL);
    result = ngcllInformationServiceInfoPairSetSources(
        context, readingState->nrs_curInfoServiceInfo,
        readingState->nrs_curInfoServiceInfoSources, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set sources to InformationServiceInfoPair.\n"); 
        return 0;
    }

    result = ngiStringListDestruct(
        readingState->nrs_curInfoServiceInfoSources, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct the StringList.\n"); 
        return 0;
    }
    readingState->nrs_curInfoServiceInfoSources = NULL;

    /* Set options */
    if (readingState->nrs_curInfoServiceInfoOptions != NULL) {
        result = ngcllInformationServiceInfoPairSetOptions(
            context, readingState->nrs_curInfoServiceInfo,
            readingState->nrs_curInfoServiceInfoOptions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set options to InformationServiceInfoPair.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curInfoServiceInfoOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
        readingState->nrs_curInfoServiceInfoOptions = NULL;
    }

    /* Move to curISinfo to isInfos */
    isInfoPair = readingState->nrs_curInfoServiceInfo;
    readingState->nrs_curInfoServiceInfo = NULL;

    result = ngcllInformationServiceInfoPairRegister(
        &(readingState->nrs_infoServiceInfos), isInfoPair);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register Information Service Info.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * template for each ngclInformationServiceInformation_t members
 * first argument should member of ngclInformationServiceInformation_t
 * AFI means ngcllAttrFuncInformationService
 */

#define NGCLL_AFI_MEMBER_SET_STR(member, arg) \
         NGCLL_AF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_curInfoServiceInfo->nisip_entities, \
             readingState->nrs_curInfoServiceInfo->nisip_isSet)

#define NGCLL_AFI_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_curInfoServiceInfo->nisip_entities, \
             readingState->nrs_curInfoServiceInfo->nisip_isSet)

#define NGCLL_AFI_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_curInfoServiceInfo->nisip_entities, \
             readingState->nrs_curInfoServiceInfo->nisip_isSet, unitTable)


static int
ngcllAttrFuncInformationSource_tag     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSource_tag";
    ngcllInformationServiceInfoPair_t *isInfoPair;
    int result, isRegistered;
    char *tagName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_tag, token->nti_tokenStr)

    log = context->ngc_log;
    isInfoPair = readingState->nrs_curInfoServiceInfo;

    tagName = isInfoPair->nisip_entities->ngisi_tag;
    assert(tagName != NULL);

    /* tag name registered check */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedInformationServiceTags, tagName,
        NGI_ATTR_NAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "Information Service information already registered for tag:",
            fName, attrName, tagName, error);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncInformationSource_type    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSource_type";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_type, token->nti_tokenStr)

    /* The type duplication is acceptable. */

    return 1;
}

static int
ngcllAttrFuncInformationSource_path    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSource_path";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_path, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncInformationSource_logfile NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSource_logfile";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_logFilePath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncInformationSource_timeout NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSource_timeout";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_UNIT(
        ngisi_timeout, 0, token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

static int
ngcllAttrFuncInformationSource_source  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSource_source";
    char *srcString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /**
     * option is able to appear multiple times in one section
     * so don't check member set or not.
     */
    srcString = ngiReadQuotedStringFromArg(
        token->nti_tokenStr, 0, NULL, 0, log, error);
    if (srcString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curInfoServiceInfoSources), srcString,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        return 0;
    }
    ngiFree(srcString, log, error);

    return 1;
}

static int
ngcllAttrFuncInformationSource_option  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInformationSource_option";
    char *optString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /**
     * option is able to appear multiple times in one section
     * so don't check member set or not.
     */
    optString = ngiReadQuotedStringFromArg(
        token->nti_tokenStr, 0, NULL, 0, log, error);
    if (optString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curInfoServiceInfoOptions), optString,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        return 0;
    }
    ngiFree(optString, log, error);

    return 1;
}

#undef NGCLL_AFI_MEMBER_SET_STR
#undef NGCLL_AFI_MEMBER_SET_INT
#undef NGCLL_AFI_MEMBER_SET_UNIT

/**
 * SERVER section
 */

static int
ngcllAttrFuncServerBegin             NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServerBegin";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_curRmInfo == NULL);

    log = context->ngc_log;

    readingState->nrs_curRmInfo = ngcllRemoteMachineInfoPairCreate(
        context, error);
    if (readingState->nrs_curRmInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage to "
            "register Remote Machine Information.\n"); 
        return 0;
    }

    result = ngcllRemoteMachineInfoPairInitialize(context,
        readingState->nrs_curRmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Remote Machine Info Pair.\n"); 
        return 0;
    }

    assert(readingState->nrs_curRmInfoHosts == NULL);
    assert(readingState->nrs_curRmInfoInvokeServerOptions == NULL);
    assert(readingState->nrs_curRmInfoCommunicationProxyOptions == NULL);
    assert(readingState->nrs_curRmInfoRSLextensions == NULL);
    assert(readingState->nrs_curRmInfoEnviron == NULL);

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServerEnd               NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServerEnd";
    char *hostName, *tagSet, *tagName;
    ngcllRemoteMachineInfoPair_t *newPair;
    ngiStringList_t *host_cur, *foundHost, *newHosts;
    int result, count;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /* Check the hostname available */
    if (readingState->nrs_curRmInfoHosts == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "at least one hostname should appear in section",
                 fName, NULL, attrName, error);
        return 0;
    }

    /* Check Tag */
    tagSet = readingState->nrs_curRmInfo->nrmip_isSet->ngrmi_tagName;
    tagName = readingState->nrs_curRmInfo->nrmip_entities->ngrmi_tagName;
    if (tagSet != NULL) {
        /* Check number hostname attribute in this <SERVER> section.
         * When tagName is set, number of hostname attribute must be
         * one in <SERVER> section. */
        result = ngiStringListCount(
            readingState->nrs_curRmInfoHosts, &count, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't count the StringList.\n"); 
            return 0;
        }
        if (count != 1) {
            ngiConfigFileSyntaxError(log, token,
                "Only one hostname should be set for the section of tag",
                 fName, NULL, tagName, error);
            return 0;
        }
    }

    /* Check host name already defined */
    if (tagSet == NULL) {
        foundHost = ngiStringListCheckListIncludeSameString(
            readingState->nrs_curRmInfoHosts,
            readingState->nrs_appearedNoTagRmInfoHosts,
            NGI_HOSTNAME_CASE_SENSITIVE, log, error);
        if (foundHost != NULL) {
            ngiConfigFileSyntaxError(log, token,
                "The Server information already registered for:",
                fName, NULL, foundHost->nsl_string, error);
            return 0;
        }
        foundHost = NULL;
    }

    /* Set Invoke Server Options to curRmInfo */
    if (readingState->nrs_curRmInfoInvokeServerOptions != NULL) {
        result = ngcllRemoteMachineInfoPairSetInvokeServerOptions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoInvokeServerOptions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set InvokeServerOptions to "
                "RemoteMachineInfoPair.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoInvokeServerOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoInvokeServerOptions = NULL;
    }

    /* Set Communication Proxy Options to curRmInfo */
    if (readingState->nrs_curRmInfoCommunicationProxyOptions != NULL) {
        result = ngcllRemoteMachineInfoPairSetCommunicationProxyOptions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoCommunicationProxyOptions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set CommunicationProxyOptions to "
                "RemoteMachineInfoPair.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoCommunicationProxyOptions,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoCommunicationProxyOptions = NULL;
    }

    /* Set RSL Extensions to curRmInfo */
    if (readingState->nrs_curRmInfoRSLextensions != NULL) {
        result = ngcllRemoteMachineInfoPairSetRSLextensions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoRSLextensions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set RSL Extensions to "
                "RemoteMachineInfoPair.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoRSLextensions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoRSLextensions = NULL;
    }

    /* Set environ to curRmInfo */
    if (readingState->nrs_curRmInfoEnviron != NULL) {
        result = ngcllRemoteMachineInfoPairSetEnviron(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoEnviron, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set environ to Remote Machine Info Pair.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoEnviron, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoEnviron = NULL;
    }

    /* Create ngcllRemoteMachineInfoPair for all hosts. */
    host_cur = readingState->nrs_curRmInfoHosts;
    while (host_cur != NULL) {

        newPair = ngcllRemoteMachineInfoPairDuplicate(
            context, readingState->nrs_curRmInfo, error);
        if (newPair == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate Remote Machine Info Pair.\n"); 
            return 0;
        }

        assert(host_cur->nsl_string != NULL);
        hostName = ngiStrdup(host_cur->nsl_string, log, error);
        if (hostName == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the string.\n"); 
            return 0;
        }

        assert(newPair->nrmip_entities->ngrmi_hostName == NULL);

        newPair->nrmip_entities->ngrmi_hostName = hostName;
        newPair->nrmip_isSet->ngrmi_hostName = NGI_ISSET_S_TRUE;

        result = ngcllRemoteMachineInfoPairRegister(
            &(readingState->nrs_rmInfos), newPair);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register Remote Machine Info.\n"); 
            return 0;
        }

        host_cur = host_cur->nsl_next;
    }

    result = ngcllRemoteMachineInfoPairDestruct(context,
        readingState->nrs_curRmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct the Remote Machine Info Pair.\n"); 
        return 0;
    }

    /* Register No-Tag host names */
    if (tagSet == NULL) {
        assert(readingState->nrs_curRmInfoHosts != NULL);

        newHosts = ngiStringListDuplicate(
            readingState->nrs_curRmInfoHosts, log, error);
        if (newHosts == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the StringList.\n"); 
            return 0;
        }

        result = ngiStringListRegisterList(
            &(readingState->nrs_appearedNoTagRmInfoHosts), newHosts,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the StringList.\n"); 
            return 0;
        }
    }

    if (readingState->nrs_curRmInfoHosts != NULL) {
        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoHosts,
            log, error);
        readingState->nrs_curRmInfoHosts = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            return 0;
        }
    }

    readingState->nrs_curRmInfo = NULL;

    /* Success */
    return 1;
}


/**
 * template for each RemoteMachineInfo members
 * first argument should member of ngclRemoteMachineInformation_t
 * AFS means ngcllAttrFuncServer
 */

#define NGCLL_AFS_MEMBER_SET_STR(member, arg) \
         NGCLL_AF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_curRmInfo->nrmip_entities, \
             readingState->nrs_curRmInfo->nrmip_isSet)

#define NGCLL_AFS_MEMBER_SET_QSTR(member, arg) \
         NGCLL_AF_MEMBER_SET_QSTR(member, arg, \
             readingState->nrs_curRmInfo->nrmip_entities, \
             readingState->nrs_curRmInfo->nrmip_isSet)

#define NGCLL_AFS_MEMBER_SET_BOOL(member, arg) \
         NGCLL_AF_MEMBER_SET_BOOL(member, arg, \
             readingState->nrs_curRmInfo->nrmip_entities, \
             readingState->nrs_curRmInfo->nrmip_isSet)

#define NGCLL_AFS_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_curRmInfo->nrmip_entities, \
             readingState->nrs_curRmInfo->nrmip_isSet)

#define NGCLL_AFS_MEMBER_SET_DOUBLE(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_DOUBLE(member, minValue, arg, \
             readingState->nrs_curRmInfo->nrmip_entities, \
             readingState->nrs_curRmInfo->nrmip_isSet)

#define NGCLL_AFS_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_curRmInfo->nrmip_entities, \
             readingState->nrs_curRmInfo->nrmip_isSet, unitTable)

static int
ngcllAttrFuncServer_hostname         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_hostname";
    ngiStringList_t *hostNames, *foundHost;
    ngLog_t *log;
    int result;
    int ret = 0;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    if (readingState->nrs_readingServerDefault) {
        result = ngcllAttrFuncServerDefault_hostname(
            attrName, token, readingState, context, error);

        return result;
    }

    assert(readingState != NULL);
    assert(readingState->nrs_lmInfo != NULL);
    assert(token != NULL);
    assert(token->nti_tokenStr != NULL);

    /* multiple appearance are allowed, so, don't check isSet */

    /* Get argument strings */
    hostNames = ngiReadStringListFromArg(
        token->nti_tokenStr, log, error);
    if (hostNames == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal host name list:",
            fName, attrName, token->nti_tokenStr, error);
        goto finalize;
    }

    /* Check host name already defined (current section) */
    foundHost = ngiStringListCheckListIncludeSameString(hostNames,
        readingState->nrs_curRmInfoHosts, NGI_HOSTNAME_CASE_SENSITIVE,
        log, error);
    if (foundHost != NULL) {
        ngiConfigFileSyntaxError(log, token,
            "server information already registered for:",
            fName, attrName, foundHost->nsl_string, error);
        goto finalize;
    }
    foundHost = NULL;

    /* Register host names */
    result = ngiStringListRegisterList(
        &(readingState->nrs_curRmInfoHosts), hostNames, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the StringList.\n"); 
        goto finalize;
    }
    hostNames = NULL;

    readingState->nrs_curRmInfo->nrmip_isSet->ngrmi_hostName =
        NGI_ISSET_S_TRUE;

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (hostNames != NULL) {
        result = ngiStringListDestruct(hostNames, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct StringList.\n"); 
            error = NULL;
            ret = 0;
        }
    }

    return ret;
}

static int
ngcllAttrFuncServer_tag              NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_tag";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    int result, isRegistered;
    char *tagName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    if (readingState->nrs_readingServerDefault) {
        result = ngcllAttrFuncServerDefault_tag(
            attrName, token, readingState, context, error);

        return result;
    }

    NGCLL_AFS_MEMBER_SET_STR(ngrmi_tagName, token->nti_tokenStr)

    tagName = rmInfoPair->nrmip_entities->ngrmi_tagName;
    assert(tagName != NULL);

    /* Tag name registered check */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedRmInfoTags, tagName,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the StringList.\n"); 
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "Server tag already registered for:",
            fName, NULL, tagName, error);
        return 0;
    }
 
    /* Register to appearedRmInfoTags */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedRmInfoTags), tagName, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register StringList.\n"); 
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncServer_port             NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_port";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_portNo, 0, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_invoke_server    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_invoke_server";
    int (*cmp)(const char *s1, const char *s2, size_t n);
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    int isRegistered, result;
    ngLog_t *log;
    char *type;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    NGCLL_AFS_MEMBER_SET_STR(ngrmi_invokeServerType, token->nti_tokenStr)

    /**
     * Do not check validity of Invoke Server Type.
     * Because, Unknown Invoke Server Type should be acceptable.
     * The new Invoke Server should be added without changing
     * Ninf-G code.
     */

    /**
     * For future use, "none" is used to avoid Invoke Server.
     * Super Computer may not have fork(), thus External Module
     * won't work.
     */

    type = rmInfoPair->nrmip_entities->ngrmi_invokeServerType;
    assert(type != NULL);

    cmp = (NGI_ATTR_ARG_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    if ((*cmp)(type, "none", strlen(type) + 1) == 0) {
        /* Set back to NULL */
        free(type);
        rmInfoPair->nrmip_entities->ngrmi_invokeServerType = NULL;

    } else {

        /* Register to nrs_appearedInvokeServersInRm */
        result = ngiStringListCheckIncludeSameString(
            readingState->nrs_appearedInvokeServersInRm, type,
            NGI_ATTR_NAME_CASE_SENSITIVE, &isRegistered, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the StringList.\n"); 
            return 0;
        }
        if (!isRegistered) {
            result = ngiStringListRegister(
                &(readingState->nrs_appearedInvokeServersInRm), type,
                log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't register StringList.\n"); 
                return 0;
            }
        }
    }

    return 1;
}

static int
ngcllAttrFuncServer_invoke_server_opt  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_invoke_server_opt";
    char *optString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /**
     * invoke_server_option is able to appear multiple times in one section
     * so don't check member set or not.
     */
    optString = ngiReadQuotedStringFromArg(
        token->nti_tokenStr, 0, NULL, 0, log, error);
    if (optString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curRmInfoInvokeServerOptions), optString,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        return 0;
    }
    ngiFree(optString, log, error);

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_cp_type          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_cp_type";
    int (*cmp)(const char *s1, const char *s2, size_t n);
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    int isRegistered, result;
    ngLog_t *log;
    char *type;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    NGCLL_AFS_MEMBER_SET_STR(ngrmi_commProxyType, token->nti_tokenStr)

    /**
     * Do not check validity of Communication Proxy Type.
     * Because, Unknown Communication Proxy Type should be acceptable.
     * The new Communication Proxy should be added without changing
     * Ninf-G code.
     */

    /* "none" is for not to use Communication Proxy, Type is NULL. */
    type = rmInfoPair->nrmip_entities->ngrmi_commProxyType;
    assert(type != NULL);

    cmp = (NGI_ATTR_ARG_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    if ((*cmp)(type, "none", strlen(type) + 1) == 0) {
        /* Set back to NULL */
        free(type);
        rmInfoPair->nrmip_entities->ngrmi_commProxyType = NULL;

    } else {

        /* Register to nrs_appearedCommunicationProxysInRm */
        result = ngiStringListCheckIncludeSameString(
            readingState->nrs_appearedCommunicationProxysInRm, type,
            NGI_ATTR_NAME_CASE_SENSITIVE, &isRegistered, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the StringList.\n"); 
            return 0;
        }
        if (!isRegistered) {
            result = ngiStringListRegister(
                &(readingState->nrs_appearedCommunicationProxysInRm), type,
                log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't register StringList.\n"); 
                return 0;
            }
        }
    }

    return 1;
}

static int
ngcllAttrFuncServer_cp_staging       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_cp_staging";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(ngrmi_commProxyStaging, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_cp_path          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_cp_path";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_commProxyPath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_cp_bufferSize    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_cp_bufferSize";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_commProxyBufferSize, 0, token->nti_tokenStr,
        ngiSizeUnitTable)

    return 1;
}

static int
ngcllAttrFuncServer_cp_option        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_cp_option";
    char *optString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /**
     * this option attribute is able to appear multiple times in one section
     * so don't check member set or not.
     */
    optString = ngiReadQuotedStringFromArg(
        token->nti_tokenStr, 0, NULL, 0, log, error);
    if (optString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curRmInfoCommunicationProxyOptions), optString,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        return 0;
    }
    ngiFree(optString, log, error);

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_info_source_tag  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_info_source_tag";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_infoServiceTag, token->nti_tokenStr)

    return 1;
}

static int 
ngcllAttrFuncServer_mpi_runCPUs      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_mpi_runCPUs";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    char *className = NULL;
    int result, cpus;
    int ret = 0;
    int sub_error;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    result = ngiReadStrEqualNumberFromArg(token->nti_tokenStr,
         &className, &cpus, log, error);
    if (result == 0) {
        ngiConfigFileSyntaxError(log, token,
            "Unable to get mpi_runNoOfCPUs",
            fName, attrName, token->nti_tokenStr, error);
        goto finalize;
    }
    if (cpus < 1) {
        ngiConfigFileSyntaxError(log, token,
            "attribute value is invalid (too small):",
            fName, attrName, token->nti_tokenStr, error);
        goto finalize;
    }

    if (className == NULL) {
        /* If no name specified, it means to set rmInfo's all mpiNcpus */
        if (rmInfoPair->nrmip_isSet->ngrmi_mpiNcpus != 0) {
            ngiConfigFileSyntaxError(log, token,
                "attribute redefined", fName, attrName, NULL, error);
            goto finalize;
        }

        rmInfoPair->nrmip_entities->ngrmi_mpiNcpus = cpus;
        rmInfoPair->nrmip_isSet->ngrmi_mpiNcpus = NGI_ISSET_I_TRUE;

    } else {
        /* Class specified mpiNcpus */
        if (readingState->nrs_readingServerDefault) {
            ngiConfigFileSyntaxError(log, token,   
                "Not allowed attribute in this section",
                fName, attrName, NULL, error);         
            goto finalize;
        }
        NGI_SET_ERROR(&sub_error, NG_ERROR_NO_ERROR);
        result = ngcliRemoteMachineInformationAppendMPInCPUs(
            rmInfoPair->nrmip_entities, className, cpus, log, &sub_error);
        if (result == 0) {
            if (sub_error == NG_ERROR_ALREADY) {
                ngiConfigFileSyntaxError(log, token,
                    "mpi_runNoOfCPUs redefined for function:",
                    fName, attrName, className, error);
            } else {
                NGI_SET_ERROR(error, sub_error);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't append MPI number of CPUs to "
                    "Remote Machine Information.\n");
            }
            goto finalize;
        }
    }
    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngiFree(className, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the string.\n");
        error = NULL;
        ret = 0;
    }

    return ret;
}

static int
ngcllAttrFuncServer_keep_connect     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_keep_connect";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(ngrmi_keepConnect, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_force_xdr        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_force_xdr";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(ngrmi_forceXDR, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_jobmanager       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_jobmanager";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_jobManager, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_subject          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_subject";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_QSTR(ngrmi_subject, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_client_hostname  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_client_hostname";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_clientHostName, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_job_startTimeout NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_startTimeout";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_jobStartTimeout, 0, token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

static int
ngcllAttrFuncServer_job_stopTimeout  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_stopTimeout";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_jobEndTimeout, -1, token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

static int
ngcllAttrFuncServer_job_maxTime      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_maxTime";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_jobMaxTime, 0, token->nti_tokenStr)

    /* Success */
    return 1;
}


static int
ngcllAttrFuncServer_job_maxWallTime  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_maxWallTime";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_jobMaxWallTime, 0, token->nti_tokenStr)

    /* Success */
    return 1;
}


static int
ngcllAttrFuncServer_job_maxCpuTime   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_maxCpuTime";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_jobMaxCpuTime, 0, token->nti_tokenStr)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_job_queue        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_queue";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_jobQueue, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_job_project      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_project";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_jobProject, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_job_hostCount    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_hostCount";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_jobHostCount, 0, token->nti_tokenStr)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_job_minMemory    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_minMemory";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_jobMinMemory, 0, token->nti_tokenStr)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_job_maxMemory    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_maxMemory";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_jobMaxMemory, 0, token->nti_tokenStr)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_job_rslExtensions  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_job_rslExtensions";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    int valueContinue, continuedLine;
    char *extString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    /**
     * job_rslExtensions is able to appear multiple times in one section
     * thus, don't check member set or not.
     */

    valueContinue = 1;
    continuedLine = 0;
    while (valueContinue == 1) {
        if (continuedLine == 1) {
            result = ngiGetToken(
                token->nti_readInfo, token, NGI_GETTOKEN_QUOTED, log, error);
            if (result != NGI_GET_TOKEN_SUCCESS) {
                ngiConfigFileSyntaxError(log, token,
                    "Failed to get quoted string line",
                    fName, NULL, NULL, error);
                return 0;
            }

            if (token->nti_type != NGI_TOKEN_ATTR) {
                ngiConfigFileSyntaxError(log, token,
                    "Failed to get double quoted string line",
                    fName, NULL, token->nti_tokenStr, error);
                return 0;
            }
        }

        extString = ngiReadQuotedStringFromArg(
            token->nti_tokenStr, 1, &valueContinue, continuedLine,
            log, error);
        if (extString == NULL) {
            ngiConfigFileSyntaxError(log, token, 
                "Unable to get string from ",
                fName, attrName, token->nti_tokenStr, error);
            return 0;
        }

        result = ngiStringListRegister(
            &(readingState->nrs_curRmInfoRSLextensions), extString,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the string.\n"); 
            return 0;
        }
        ngiFree(extString, log, error);
        extString = NULL;

        continuedLine = 1;
    }

    rmInfoPair->nrmip_isSet->ngrmi_rslExtensionsSize = NGI_ISSET_I_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_heartbeat        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_heartbeat";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_heartBeat, 0, token->nti_tokenStr, ngiTimeUnitTable)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_heartbeatCount   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_heartbeatCount";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(
        ngrmi_heartBeatTimeoutCount, 1, token->nti_tokenStr)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_redirect_outerr  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_redirect_outerr";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(ngrmi_redirectEnable, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_tcp_nodelay      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_tcp_nodelay";


    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(ngrmi_tcpNodelay, token->nti_tokenStr)

    /* Success */
    return 1;
}

static int 
ngcllAttrFuncServer_retryCount       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_retryCount";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    NGCLL_AFS_MEMBER_SET_INT(
        ngrmi_retryInfo.ngcri_count, 0, token->nti_tokenStr)

    return 1;
}

static int 
ngcllAttrFuncServer_retryBaseInterval   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_retryBaseInterval";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_retryInfo.ngcri_interval, 0,
        token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

static int 
ngcllAttrFuncServer_retryIncreaseRatio  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_retryIncreaseRatio";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    NGCLL_AFS_MEMBER_SET_DOUBLE(
        ngrmi_retryInfo.ngcri_increase, 1.0, token->nti_tokenStr)

    return 1;
}


static int 
ngcllAttrFuncServer_retryRandom         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_retryRandom";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(
        ngrmi_retryInfo.ngcri_useRandom, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_argumentTransfer    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_argumentTransfer";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    char *argTransType;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    if (rmInfoPair->nrmip_isSet->ngrmi_argumentTransfer != 0) {
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0;
    }

    /* Get argument string */
    argTransType = ngiReadStringFromArg(
        token->nti_tokenStr, log, error);
    if (argTransType == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal argument transfer [wait/nowait/copy]:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* Set argument enum */
    result = ngiReadEnumFromArg(
        argTransType, NGI_ATTR_ARG_CASE_SENSITIVE,
        3, "wait", "nowait", "copy");
    if (result == 0) {
        ngiConfigFileSyntaxError(log, token,
            "No such argument transfer [wait/nowait/copy]:",
            fName, attrName, argTransType, error);
        ngiFree(argTransType, log, error);
        return 0;
    }
    ngiFree(argTransType, log, error);

    if (result == 1) {
        rmInfoPair->nrmip_entities->ngrmi_argumentTransfer =
            NG_ARGUMENT_TRANSFER_WAIT;
    } else if (result == 2) {
        rmInfoPair->nrmip_entities->ngrmi_argumentTransfer =
            NG_ARGUMENT_TRANSFER_NOWAIT;
    } else if (result == 3) {
        rmInfoPair->nrmip_entities->ngrmi_argumentTransfer =
            NG_ARGUMENT_TRANSFER_COPY;
    } else {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "reading argument transfer string fail. result is %d\n", result); 
        return 0;
    }
    rmInfoPair->nrmip_isSet->ngrmi_argumentTransfer =
        (ngArgumentTransfer_t) NGI_ISSET_I_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_compress            NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_compress";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    char *compressType;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    if (rmInfoPair->nrmip_isSet->ngrmi_compressionType != 0) {
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0;
    }

    /* Get argument string */
    compressType = ngiReadStringFromArg(
        token->nti_tokenStr, log, error);
    if (compressType == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal compression type [raw/zlib]:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* Set argument enum */
    result = ngiReadEnumFromArg(
        compressType, NGI_ATTR_ARG_CASE_SENSITIVE,
        2, "raw", "zlib");
    if (result == 0) {
        ngiConfigFileSyntaxError(log, token,
            "No such compression type [raw/zlib]:",
            fName, attrName, compressType, error);
        ngiFree(compressType, log, error);
        return 0;
    }
    ngiFree(compressType, log, error);

    if (result == 1) {
        rmInfoPair->nrmip_entities->ngrmi_compressionType =
            NG_COMPRESSION_TYPE_RAW;
    } else if (result == 2) {

#ifdef NGI_ZLIB_ENABLED
        rmInfoPair->nrmip_entities->ngrmi_compressionType =
            NG_COMPRESSION_TYPE_ZLIB;

#else /* NGI_ZLIB_ENABLED */
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s: zlib compression is not available for this client.\n",
            attrName); 
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G configure was determined as no-zlib for this host.\n"); 

        rmInfoPair->nrmip_entities->ngrmi_compressionType =
            NG_COMPRESSION_TYPE_RAW;

#endif /* NGI_ZLIB_ENABLED */

    } else {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "reading compress string fail. result is %d\n", result); 
        return 0;
    }
    rmInfoPair->nrmip_isSet->ngrmi_compressionType =
        (ngCompressionType_t) NGI_ISSET_I_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_compress_threshold  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_compress_threshold";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_compressionThresholdNbytes, 0,
        token->nti_tokenStr, ngiSizeUnitTable)

#ifndef NGI_ZLIB_ENABLED
    ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "%s: zlib compression is not available for this client.\n",
        "compress_threshold"); 
    ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "thus, threshold has no effect.\n"); 

#endif /* NGI_ZLIB_ENABLED */

    return 1;
}

static int
ngcllAttrFuncServer_argument_blockSize  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_argument_blockSize";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_argumentBlockSize, 0, token->nti_tokenStr,
        ngiSizeUnitTable)

    return 1;
}

static int
ngcllAttrFuncServer_workDirectory       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_workDirectory";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_workDirectory, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_coreDumpSize        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_coreDumpSize";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(ngrmi_coreDumpSize, -1, token->nti_tokenStr)

    return 1;
}

static int 
ngcllAttrFuncServer_commLog_enable      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_commLog_enable";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(
        ngrmi_commLogEnable, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_commLog_filePath    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_commLog_filePath";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(
        ngrmi_commLogInfo.ngli_filePath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_commLog_suffix      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_commLog_suffix";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(
        ngrmi_commLogInfo.ngli_suffix, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_commLog_nFiles      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_commLog_nFiles";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_INT(
        ngrmi_commLogInfo.ngli_nFiles, 0, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_commLog_maxFileSize NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_commLog_maxFileSize";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_commLogInfo.ngli_maxFileSize, 0, token->nti_tokenStr,
        ngiSizeUnitTable)

    return 1;
}

static int 
ngcllAttrFuncServer_commLog_overWrite   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_commLog_overWrite";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(
        ngrmi_commLogInfo.ngli_overWriteDir, token->nti_tokenStr)

    return 1;
}


static int
ngcllAttrFuncServer_debug            NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_debug";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(ngrmi_debug.ngdi_enable, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_debug_display    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_debug_display";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(
        ngrmi_debug.ngdi_display, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_debug_terminal   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_debug_terminal";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(
        ngrmi_debug.ngdi_terminalPath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_debug_debugger   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_debug_debugger";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(
        ngrmi_debug.ngdi_debuggerPath, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_debug_busyLoop   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_debug_busyLoop";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_BOOL(ngrmi_debugBusyLoop, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_environment      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_environment";
    char *envString;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    /**
     * environment is able to appear multiple times in one section
     * so don't check member set or not.
     */

    envString = ngiReadEnvStringFromArg(
        token->nti_tokenStr, log, error);
    if (envString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get environment from:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curRmInfoEnviron), envString, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the string.\n"); 
        return 0;
    }
    ngiFree(envString, log, error);

    /* Success */
    return 1;
}

#undef NGCLL_AFS_MEMBER_SET_STR
#undef NGCLL_AFS_MEMBER_SET_QSTR
#undef NGCLL_AFS_MEMBER_SET_BOOL
#undef NGCLL_AFS_MEMBER_SET_INT
#undef NGCLL_AFS_MEMBER_SET_DOUBLE
#undef NGCLL_AFS_MEMBER_SET_UNIT

/**
 * SERVER_DEFAULT section
 */

static int
ngcllAttrFuncServerDefaultBegin      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServerDefaultBegin";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    if (readingState->nrs_defRmInfoAppeared) {
        ngiConfigFileSyntaxError(log, token, "redefining section",
            fName, NULL, attrName, error);
        return 0;
    }

    /* treat as same as SERVER section */
    result = ngcllAttrFuncServerBegin(
        attrName, token, readingState, context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to execute attribute function.\n"); 
        return 0;
    }

    readingState->nrs_readingServerDefault = 1; /* TRUE */

    assert(readingState->nrs_curRmInfoHosts == NULL);

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServerDefaultEnd        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServerDefaultEnd";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_defRmInfo == NULL);
    assert(readingState->nrs_curRmInfo != NULL);

    log = context->ngc_log;

    /* hostName is not appear in SERVER_DEFAULT section */
    assert(readingState->nrs_curRmInfo->nrmip_isSet->ngrmi_hostName == NULL);

    /* Treat Invoke Server Options */
    if (readingState->nrs_curRmInfoInvokeServerOptions != NULL) {
        result = ngcllRemoteMachineInfoPairSetInvokeServerOptions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoInvokeServerOptions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set InvokeServerOptions to "
                "RemoteMachineInformation.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoInvokeServerOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoInvokeServerOptions = NULL;
    }

    /* Treat Communication Proxy Options */
    if (readingState->nrs_curRmInfoCommunicationProxyOptions != NULL) {
        result = ngcllRemoteMachineInfoPairSetCommunicationProxyOptions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoCommunicationProxyOptions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set CommunicationProxyOptions to "
                "RemoteMachineInformation.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoCommunicationProxyOptions,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoCommunicationProxyOptions = NULL;
    }

    /* Treat RSL Extensions */
    if (readingState->nrs_curRmInfoRSLextensions != NULL) {
        result = ngcllRemoteMachineInfoPairSetRSLextensions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoRSLextensions, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set RSL Extensions to "
                "RemoteMachineInformation.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoRSLextensions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoRSLextensions = NULL;
    }

    /* Treat environment variable */
    if (readingState->nrs_curRmInfoEnviron != NULL) {
        result = ngcllRemoteMachineInfoPairSetEnviron(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoEnviron, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set environ to Remote Machine Information.\n"); 
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoEnviron, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct StringList.\n"); 
            return 0;
        }
        readingState->nrs_curRmInfoEnviron = NULL;
    }

    /* Move to curRmInfo -> defRmInfo */
    readingState->nrs_defRmInfo = readingState->nrs_curRmInfo;
    readingState->nrs_curRmInfo = NULL;

    readingState->nrs_readingServerDefault = 0; /* FALSE */
    readingState->nrs_defRmInfoAppeared = 1; /* TRUE */

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServerDefault_hostname  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServerDefault_hostname";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    ngiConfigFileSyntaxError(context->ngc_log, token,
        "not allowed attribute in this section",
        fName, attrName, NULL, error);

    return 0;
}

static int
ngcllAttrFuncServerDefault_tag       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServerDefault_tag";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    ngiConfigFileSyntaxError(context->ngc_log, token,
        "not allowed attribute in this section",
        fName, attrName, NULL, error);

    return 0;
}

#undef NGCLL_CONFIG_ATTRFUNC_ASSERTS

#undef NGCLL_AF_MEMBER_SET_STR
#undef NGCLL_AF_MEMBER_SET_QSTR
#undef NGCLL_AF_MEMBER_SET_BOOL
#undef NGCLL_AF_MEMBER_SET_INT
#undef NGCLL_AF_MEMBER_SET_DOUBLE
#undef NGCLL_AF_MEMBER_SET_LOGLEVEL
#undef NGCLL_AF_MEMBER_SET_UNIT

/************************************************************************/

/* utility functions */

/**
 * LocalMachineInfoPair
 */
ngcllLocalMachineInfoPair_t *
ngcllLocalMachineInfoPairCreate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllLocalMachineInfoPairCreate";
    ngcllLocalMachineInfoPair_t *ret;
    ngLog_t *log;

    log = context->ngc_log;

    ret = NGI_ALLOCATE(ngcllLocalMachineInfoPair_t, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Local Machine Info Pair.\n"); 
        return NULL;
    }

    ret->nlmip_entities = ngcliLocalMachineInformationAllocate(
        context, error);
    if (ret->nlmip_entities == NULL) {
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Local Machine Information.\n"); 
        return NULL;
    }

    ret->nlmip_isSet = ngcliLocalMachineInformationAllocate(
        context, error);
    if (ret->nlmip_isSet == NULL) {
        ngiFree(ret->nlmip_entities, log, error);
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Local Machine Information.\n"); 
        return NULL;
    }

    /* Success */
    return ret;
}

int
ngcllLocalMachineInfoPairDestruct(
    ngclContext_t *context,
    ngcllLocalMachineInfoPair_t *lmInfoPair,
    int *error)
{
    static const char fName[] = "ngcllLocalMachineInfoPairDestruct";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (lmInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngclLocalMachineInformationRelease(context,
        lmInfoPair->nlmip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Local Machine Info Pair.\n"); 
        return 0;
    }

    /* nlmip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliLocalMachineInformationFree(context,
        lmInfoPair->nlmip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Local Machine Information.\n"); 
        return 0;
    }

    result = ngcliLocalMachineInformationFree(context,
        lmInfoPair->nlmip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Local Machine Information.\n"); 
        return 0;
    }

    result = NGI_DEALLOCATE(
        ngcllLocalMachineInfoPair_t, lmInfoPair, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the Local Machine Info Pair.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllLocalMachineInfoPairInitialize(
    ngclContext_t *context,
    ngcllLocalMachineInfoPair_t *lmInfoPair,
    int *error)
{
    static const char fName[] = "ngcllLocalMachineInfoPairInitialize";
    ngLog_t *log;
    int result;

    assert(lmInfoPair != NULL);
    assert(lmInfoPair->nlmip_entities != NULL);
    assert(lmInfoPair->nlmip_isSet != NULL);

    log = context->ngc_log;

    lmInfoPair->nlmip_next = NULL;

    result = ngcliLocalMachineInformationInitialize(context,
        lmInfoPair->nlmip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Local Machine Information.\n"); 
        return 0;
    }
    result = ngcllLocalMachineInformationSetDefault(
        lmInfoPair->nlmip_entities, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Local Machine Information to default.\n"); 
        return 0;
    }

    result = ngcliLocalMachineInformationInitialize(context,
        lmInfoPair->nlmip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Local Machine Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * This macro sets dst->member if src->isSet->member flag is on.
 * argument member should be the member variable
 * of ngcl*Information_t.
 * ISP means *InformationSetPair.
 */

#define NGCLL_ISP_MEMBER_SET_STR(dst, entity, isset, member) \
    { \
        if ((isset)->member != NULL) { \
            if (dst->member != NULL) { \
                ngiFree(dst->member, log, error); \
            } \
            if ((entity)->member != NULL) { \
                dst->member = strdup((entity)->member); \
                if (dst->member == NULL) { \
                    NGI_SET_ERROR(error, NG_ERROR_MEMORY); \
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  \
                        "Can't allocate the storage for string.\n"); \
                    return 0; \
                } \
            } else { \
                dst->member = NULL; \
            } \
        } \
    }
#define NGCLL_ISP_MEMBER_SET_INT(dst, entity, isset, member) \
    { \
        if ((isset)->member != 0) { \
            dst->member = (entity)->member; \
        } \
    }

#define NGCLL_ISP_MEMBER_SET_DOUBLE(dst, entity, isset, member) \
    { \
        if (((isset)->member > 0.001) || \
            ((isset)->member < -0.001)) { \
            dst->member = (entity)->member; \
        } \
    }

#define NGCLL_LMISP_MEMBER_SET_STR(dst, src, member) \
    NGCLL_ISP_MEMBER_SET_STR( \
            dst, (src)->nlmip_entities, (src)->nlmip_isSet, member)

#define NGCLL_LMISP_MEMBER_SET_INT(dst, src, member) \
    NGCLL_ISP_MEMBER_SET_INT( \
            dst, (src)->nlmip_entities, (src)->nlmip_isSet, member)


/** * This function sets dst from src member which isSet flag is true. */
int
ngcllLocalMachineInformationSetPair(
    ngclLocalMachineInformation_t *dst,
    ngcllLocalMachineInfoPair_t *src,
    ngLog_t *log, int *error)
{
    static const char fName[] = "ngcllLocalMachineInformationSetPair";
    int *origTable, *newTable, size, i;

    size = 0;
    origTable = NULL;
    newTable = NULL;

    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_hostName)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_saveNsessions)

    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_logInfo.ngli_filePath)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_logInfo.ngli_suffix)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_nFiles)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_maxFileSize)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_overWriteDir)

    if (src->nlmip_isSet->nglmi_logLevels.nglli_level != 0) {
        dst->nglmi_logLevels.nglli_level =
            dst->nglmi_logLevels.nglli_ninfgProtocol =
            dst->nglmi_logLevels.nglli_ninfgInternal =
            dst->nglmi_logLevels.nglli_grpc =
            src->nlmip_entities->nglmi_logLevels.nglli_level;
    }

    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logLevels.nglli_ninfgProtocol)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logLevels.nglli_ninfgInternal)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logLevels.nglli_grpc)

    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_tmpDir)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_refreshInterval)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_invokeServerLog)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_commProxyLog)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_infoServiceLog)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_fortranCompatible)

    if (src->nlmip_isSet->nglmi_signals != NULL) {
        /* Clear old data and set new data */
        if (dst->nglmi_signals != NULL) {
            ngiFree(dst->nglmi_signals, log, error);
            dst->nglmi_signals = NULL;
        }

        dst->nglmi_signals = NULL;
        if (src->nlmip_entities->nglmi_signals != NULL) {
             origTable = src->nlmip_entities->nglmi_signals;
             for (size = 0; origTable[size] != 0; size++);

             newTable = ngiCalloc(sizeof(int), size + 1, log, error);
             if (newTable == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't allocate the signal table.\n"); 
                return 0;
             }

             for (i = 0; i < (size + 1); i++) {
                 newTable[i] = origTable[i];
             }

             dst->nglmi_signals = newTable;
        }
    }

    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_listenPort)

    /* Success */
    return 1;
}

#undef NGCLL_LMISP_MEMBER_SET_STR
#undef NGCLL_LMISP_MEMBER_SET_INT

/**
 * InvokeServerInfoPair
 */
ngcllInvokeServerInfoPair_t *
ngcllInvokeServerInfoPairCreate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInfoPairCreate";
    ngcllInvokeServerInfoPair_t *ret;
    ngLog_t *log;

    log = context->ngc_log;

    ret = NGI_ALLOCATE(ngcllInvokeServerInfoPair_t, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Invoke Server Info Pair.\n"); 
        return NULL;
    }

    ret->nisip_entities = ngcliInvokeServerInformationAllocate(
        context, error);
    if (ret->nisip_entities == NULL) {
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Invoke Server Information.\n"); 
        return NULL;
    }

    ret->nisip_isSet = ngcliInvokeServerInformationAllocate(
        context, error);
    if (ret->nisip_isSet == NULL) {
        ngiFree(ret->nisip_entities, log, error);
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Invoke Server Information.\n"); 
        return NULL;
    }

    /* Success */
    return ret;
}

int
ngcllInvokeServerInfoPairDestruct(
    ngclContext_t *context,
    ngcllInvokeServerInfoPair_t *isInfoPair,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInfoPairDestruct";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (isInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngclInvokeServerInformationRelease(context,
        isInfoPair->nisip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the storage to "
            "register Invoke Server Information.\n"); 
        return 0;
    }

    /* nisip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliInvokeServerInformationFree(context,
        isInfoPair->nisip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Invoke Server Information.\n"); 
        return 0;
    }

    result = ngcliInvokeServerInformationFree(context,
        isInfoPair->nisip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Invoke Server Information.\n"); 
        return 0;
    }

    result = NGI_DEALLOCATE(
        ngcllInvokeServerInfoPair_t, isInfoPair, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the Invoke Server Info Pair.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllInvokeServerInfoPairListDestruct(
    ngclContext_t *context,
    ngcllInvokeServerInfoPair_t *isInfoPair,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInfoPairListDestruct";
    ngcllInvokeServerInfoPair_t *curIs, *nextIs;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (isInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    curIs = isInfoPair;
    do {
        nextIs = curIs->nisip_next;
        result = ngcllInvokeServerInfoPairDestruct(context, curIs, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct InvokeServerInfoPair.\n"); 
            return 0;
        }

        curIs = nextIs;
    } while (curIs != NULL);

    /* Success */
    return 1;
}

int
ngcllInvokeServerInfoPairInitialize(
    ngclContext_t *context,
    ngcllInvokeServerInfoPair_t *isInfoPair,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInfoPairInitialize";
    ngLog_t *log;
    int result;

    assert(isInfoPair != NULL);
    assert(isInfoPair->nisip_entities != NULL);
    assert(isInfoPair->nisip_isSet != NULL);

    log = context->ngc_log;

    isInfoPair->nisip_next = NULL;

    result = ngcliInvokeServerInformationInitialize(context,
        isInfoPair->nisip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Invoke Server Information.\n"); 
        return 0;
    }
    result = ngcllInvokeServerInformationSetDefault(
        isInfoPair->nisip_entities, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Invoke Server Information to default.\n"); 
        return 0;
    }

    result = ngcliInvokeServerInformationInitialize(context,
        isInfoPair->nisip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Invoke Server Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllInvokeServerInfoPairRegister(
    ngcllInvokeServerInfoPair_t **dst,
    ngcllInvokeServerInfoPair_t *src)
{
    ngcllInvokeServerInfoPair_t *cur;

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    src->nisip_next = NULL;

    if (*dst == NULL) {
        *dst = src;
        return 1;
    }

    cur = *dst;
    while (cur->nisip_next != NULL) {
        cur = cur->nisip_next;
    }

    cur->nisip_next = src;

    /* Success */
    return 1;
}


/**
 * This macro sets dst->member if src->isSet->member flag is on.
 * argument member should be the member variable
 * of ngclInvokeServerInformation_t.
 * ISISP means InvokeServerInformationSetPair.
 */

#define NGCLL_ISISP_MEMBER_SET_STR(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_STR( \
            dst, (src)->nisip_entities, (src)->nisip_isSet, member)

#define NGCLL_ISISP_MEMBER_SET_INT(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_INT( \
            dst, (src)->nisip_entities, (src)->nisip_isSet, member)

/**
 * This function sets dst from src member which isSet flag is true.
 */
int
ngcllInvokeServerInformationSetPair(
    ngclInvokeServerInformation_t *dst,
    ngcllInvokeServerInfoPair_t *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInformationSetPair";
    int result;

    NGCLL_ISISP_MEMBER_SET_STR(dst, src, ngisi_type)
    NGCLL_ISISP_MEMBER_SET_STR(dst, src, ngisi_path)
    NGCLL_ISISP_MEMBER_SET_INT(dst, src, ngisi_maxJobs)
    NGCLL_ISISP_MEMBER_SET_STR(dst, src, ngisi_logFilePath)
    NGCLL_ISISP_MEMBER_SET_INT(dst, src, ngisi_statusPoll)

    /* Set ngisi_options (add to dst) */
    if (src->nisip_isSet->ngisi_nOptions != 0) {
        assert(src->nisip_isSet->ngisi_options != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngisi_options,
            &dst->ngisi_nOptions,
            src->nisip_entities->ngisi_options,
            src->nisip_entities->ngisi_nOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

#undef NGCLL_ISISP_MEMBER_SET_STR
#undef NGCLL_ISISP_MEMBER_SET_INT

static int
ngcllInvokeServerInfoPairSetOptions(
    ngclContext_t *context,
    ngcllInvokeServerInfoPair_t *isInfoPair,
    ngiStringList_t *options,
    int *error)
{
    static const char fName[] =
        "ngcllInvokeServerInfoPairSetOptions";
    char **optArray;
    int optSize;
    ngLog_t *log;
    int result;

    assert(options != NULL);
    assert(isInfoPair != NULL);

    assert(isInfoPair->nisip_isSet->ngisi_nOptions == 0);
    assert(isInfoPair->nisip_isSet->ngisi_options == NULL);

    optSize = 0;
    optArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &optArray, &optSize, options, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    isInfoPair->nisip_entities->ngisi_nOptions = optSize;
    isInfoPair->nisip_entities->ngisi_options = optArray;

    isInfoPair->nisip_isSet->ngisi_nOptions = NGI_ISSET_I_TRUE;
    isInfoPair->nisip_isSet->ngisi_options = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}


/**
 * CommunicationProxyInfoPair
 */
ngcllCommunicationProxyInfoPair_t *
ngcllCommunicationProxyInfoPairCreate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInfoPairCreate";
    ngcllCommunicationProxyInfoPair_t *ret;
    ngLog_t *log;

    log = context->ngc_log;

    ret = NGI_ALLOCATE(ngcllCommunicationProxyInfoPair_t, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Communication Proxy Info Pair.\n"); 
        return NULL;
    }

    ret->ncpip_entities = ngcliCommunicationProxyInformationAllocate(
        context, error);
    if (ret->ncpip_entities == NULL) {
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Communication Proxy Information.\n"); 
        return NULL;
    }

    ret->ncpip_isSet = ngcliCommunicationProxyInformationAllocate(
        context, error);
    if (ret->ncpip_isSet == NULL) {
        ngiFree(ret->ncpip_entities, log, error);
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Communication Proxy Information.\n"); 
        return NULL;
    }

    /* Success */
    return ret;
}

int
ngcllCommunicationProxyInfoPairDestruct(
    ngclContext_t *context,
    ngcllCommunicationProxyInfoPair_t *cpInfoPair,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInfoPairDestruct";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (cpInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngclCommunicationProxyInformationRelease(context,
        cpInfoPair->ncpip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the storage to "
            "register Communication Proxy Information.\n"); 
        return 0;
    }

    /* ncpip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliCommunicationProxyInformationFree(context,
        cpInfoPair->ncpip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Communication Proxy Information.\n"); 
        return 0;
    }

    result = ngcliCommunicationProxyInformationFree(context,
        cpInfoPair->ncpip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Communication Proxy Information.\n"); 
        return 0;
    }

    result = NGI_DEALLOCATE(
        ngcllCommunicationProxyInfoPair_t, cpInfoPair, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the Communication Proxy Info Pair.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllCommunicationProxyInfoPairListDestruct(
    ngclContext_t *context,
    ngcllCommunicationProxyInfoPair_t *cpInfoPair,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInfoPairListDestruct";
    ngcllCommunicationProxyInfoPair_t *curCp, *nextCp;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (cpInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    curCp = cpInfoPair;
    do {
        nextCp = curCp->ncpip_next;
        result = ngcllCommunicationProxyInfoPairDestruct(context, curCp, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct CommunicationProxyInfoPair.\n"); 
            return 0;
        }

        curCp = nextCp;
    } while (curCp != NULL);

    /* Success */
    return 1;
}

int
ngcllCommunicationProxyInfoPairInitialize(
    ngclContext_t *context,
    ngcllCommunicationProxyInfoPair_t *cpInfoPair,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInfoPairInitialize";
    ngLog_t *log;
    int result;

    assert(cpInfoPair != NULL);
    assert(cpInfoPair->ncpip_entities != NULL);
    assert(cpInfoPair->ncpip_isSet != NULL);

    log = context->ngc_log;

    cpInfoPair->ncpip_next = NULL;

    result = ngcliCommunicationProxyInformationInitialize(context,
        cpInfoPair->ncpip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Communication Proxy Information.\n"); 
        return 0;
    }
    result = ngcllCommunicationProxyInformationSetDefault(
        cpInfoPair->ncpip_entities, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Communication Proxy Information"
            " to default.\n"); 
        return 0;
    }

    result = ngcliCommunicationProxyInformationInitialize(context,
        cpInfoPair->ncpip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Communication Proxy Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllCommunicationProxyInfoPairRegister(
    ngcllCommunicationProxyInfoPair_t **dst,
    ngcllCommunicationProxyInfoPair_t *src)
{
    ngcllCommunicationProxyInfoPair_t *cur;

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    src->ncpip_next = NULL;

    if (*dst == NULL) {
        *dst = src;
        return 1;
    }

    cur = *dst;
    while (cur->ncpip_next != NULL) {
        cur = cur->ncpip_next;
    }

    cur->ncpip_next = src;

    /* Success */
    return 1;
}


/**
 * This macro sets dst->member if src->isSet->member flag is on.
 * argument member should be the member variable
 * of ngclCommunicationProxyInformation_t.
 * CPISP means CommunicationProxyInformationSetPair.
 */

#define NGCLL_CPISP_MEMBER_SET_STR(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_STR( \
            dst, (src)->ncpip_entities, (src)->ncpip_isSet, member)

#define NGCLL_CPISP_MEMBER_SET_INT(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_INT( \
            dst, (src)->ncpip_entities, (src)->ncpip_isSet, member)

/**
 * This function sets dst from src member which isSet flag is true.
 */
int
ngcllCommunicationProxyInformationSetPair(
    ngclCommunicationProxyInformation_t *dst,
    ngcllCommunicationProxyInfoPair_t *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInformationSetPair";
    int result;

    NGCLL_CPISP_MEMBER_SET_STR(dst, src, ngcpi_type)
    NGCLL_CPISP_MEMBER_SET_STR(dst, src, ngcpi_path)
    NGCLL_CPISP_MEMBER_SET_INT(dst, src, ngcpi_bufferSize)
    NGCLL_CPISP_MEMBER_SET_INT(dst, src, ngcpi_maxJobs)
    NGCLL_CPISP_MEMBER_SET_STR(dst, src, ngcpi_logFilePath)

    /* Set ngcpi_options (add to dst) */
    if (src->ncpip_isSet->ngcpi_nOptions != 0) {
        assert(src->ncpip_isSet->ngcpi_options != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngcpi_options,
            &dst->ngcpi_nOptions,
            src->ncpip_entities->ngcpi_options,
            src->ncpip_entities->ngcpi_nOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

#undef NGCLL_CPISP_MEMBER_SET_STR
#undef NGCLL_CPISP_MEMBER_SET_INT

static int
ngcllCommunicationProxyInfoPairSetOptions(
    ngclContext_t *context,
    ngcllCommunicationProxyInfoPair_t *cpInfoPair,
    ngiStringList_t *options,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInfoPairSetOptions";
    char **optArray;
    int optSize;
    ngLog_t *log;
    int result;

    assert(options != NULL);
    assert(cpInfoPair != NULL);

    assert(cpInfoPair->ncpip_isSet->ngcpi_nOptions == 0);
    assert(cpInfoPair->ncpip_isSet->ngcpi_options == NULL);

    optSize = 0;
    optArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &optArray, &optSize, options, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    cpInfoPair->ncpip_entities->ngcpi_nOptions = optSize;
    cpInfoPair->ncpip_entities->ngcpi_options = optArray;

    cpInfoPair->ncpip_isSet->ngcpi_nOptions = NGI_ISSET_I_TRUE;
    cpInfoPair->ncpip_isSet->ngcpi_options = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}


/**
 * InformationServiceInfoPair
 */
ngcllInformationServiceInfoPair_t *
ngcllInformationServiceInfoPairCreate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInfoPairCreate";
    ngcllInformationServiceInfoPair_t *ret;
    ngLog_t *log;

    log = context->ngc_log;

    ret = NGI_ALLOCATE(ngcllInformationServiceInfoPair_t, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Information Service Info Pair.\n"); 
        return NULL;
    }

    ret->nisip_entities = ngcliInformationServiceInformationAllocate(
        context, error);
    if (ret->nisip_entities == NULL) {
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Information Service Information.\n"); 
        return NULL;
    }

    ret->nisip_isSet = ngcliInformationServiceInformationAllocate(
        context, error);
    if (ret->nisip_isSet == NULL) {
        ngiFree(ret->nisip_entities, log, error);
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Information Service Information.\n"); 
        return NULL;
    }

    /* Success */
    return ret;
}

int
ngcllInformationServiceInfoPairDestruct(
    ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInfoPairDestruct";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (isInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngclInformationServiceInformationRelease(context,
        isInfoPair->nisip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the storage to "
            "register Information Service Information.\n"); 
        return 0;
    }

    /* nisip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliInformationServiceInformationFree(context,
        isInfoPair->nisip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Information Service Information.\n"); 
        return 0;
    }

    result = ngcliInformationServiceInformationFree(context,
        isInfoPair->nisip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Information Service Information.\n"); 
        return 0;
    }

    result = NGI_DEALLOCATE(
        ngcllInformationServiceInfoPair_t, isInfoPair, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the Information Service Info Pair.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllInformationServiceInfoPairListDestruct(
    ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInfoPairListDestruct";
    ngcllInformationServiceInfoPair_t *curIs, *nextIs;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (isInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    curIs = isInfoPair;
    do {
        nextIs = curIs->nisip_next;
        result = ngcllInformationServiceInfoPairDestruct(context, curIs, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct InformationServiceInfoPair.\n"); 
            return 0;
        }

        curIs = nextIs;
    } while (curIs != NULL);

    /* Success */
    return 1;
}

int
ngcllInformationServiceInfoPairInitialize(
    ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInfoPairInitialize";
    ngLog_t *log;
    int result;

    assert(isInfoPair != NULL);
    assert(isInfoPair->nisip_entities != NULL);
    assert(isInfoPair->nisip_isSet != NULL);

    log = context->ngc_log;

    isInfoPair->nisip_next = NULL;

    result = ngcliInformationServiceInformationInitialize(context,
        isInfoPair->nisip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Information Service Information.\n"); 
        return 0;
    }
    result = ngcllInformationServiceInformationSetDefault(
        isInfoPair->nisip_entities, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Information Service Information"
            " to default.\n"); 
        return 0;
    }

    result = ngcliInformationServiceInformationInitialize(context,
        isInfoPair->nisip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Information Service Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllInformationServiceInfoPairRegister(
    ngcllInformationServiceInfoPair_t **dst,
    ngcllInformationServiceInfoPair_t *src)
{
    ngcllInformationServiceInfoPair_t *cur;

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    src->nisip_next = NULL;

    if (*dst == NULL) {
        *dst = src;
        return 1;
    }

    cur = *dst;
    while (cur->nisip_next != NULL) {
        cur = cur->nisip_next;
    }

    cur->nisip_next = src;

    /* Success */
    return 1;
}


/**
 * This macro sets dst->member if src->isSet->member flag is on.
 * argument member should be the member variable
 * of ngclInformationServiceInformation_t.
 * ISISP means InformationServiceInformationSetPair.
 */

#define NGCLL_ISISP_MEMBER_SET_STR(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_STR( \
            dst, (src)->nisip_entities, (src)->nisip_isSet, member)

#define NGCLL_ISISP_MEMBER_SET_INT(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_INT( \
            dst, (src)->nisip_entities, (src)->nisip_isSet, member)

/**
 * This function sets dst from src member which isSet flag is true.
 */
int
ngcllInformationServiceInformationSetPair(
    ngclInformationServiceInformation_t *dst,
    ngcllInformationServiceInfoPair_t *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInformationSetPair";
    int result;

    NGCLL_ISISP_MEMBER_SET_STR(dst, src, ngisi_tag)
    NGCLL_ISISP_MEMBER_SET_STR(dst, src, ngisi_type)
    NGCLL_ISISP_MEMBER_SET_STR(dst, src, ngisi_path)
    NGCLL_ISISP_MEMBER_SET_STR(dst, src, ngisi_logFilePath)
    NGCLL_ISISP_MEMBER_SET_INT(dst, src, ngisi_timeout)

    /* Set ngisi_sources (add to dst) */
    if (src->nisip_isSet->ngisi_nSources != 0) {
        assert(src->nisip_isSet->ngisi_sources != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngisi_sources,
            &dst->ngisi_nSources,
            src->nisip_entities->ngisi_sources,
            src->nisip_entities->ngisi_nSources, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Set ngisi_options (add to dst) */
    if (src->nisip_isSet->ngisi_nOptions != 0) {
        assert(src->nisip_isSet->ngisi_options != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngisi_options,
            &dst->ngisi_nOptions,
            src->nisip_entities->ngisi_options,
            src->nisip_entities->ngisi_nOptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

#undef NGCLL_ISISP_MEMBER_SET_STR
#undef NGCLL_ISISP_MEMBER_SET_INT

static int
ngcllInformationServiceInfoPairSetSources(
    ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair,
    ngiStringList_t *sources,
    int *error)
{
    static const char fName[] =
        "ngcllInformationServiceInfoPairSetSources";
    char **optArray;
    int optSize;
    ngLog_t *log;
    int result;

    assert(sources != NULL);
    assert(isInfoPair != NULL);

    assert(isInfoPair->nisip_isSet->ngisi_nSources == 0);
    assert(isInfoPair->nisip_isSet->ngisi_sources == NULL);

    optSize = 0;
    optArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &optArray, &optSize, sources, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    isInfoPair->nisip_entities->ngisi_nSources = optSize;
    isInfoPair->nisip_entities->ngisi_sources = optArray;

    isInfoPair->nisip_isSet->ngisi_nSources = NGI_ISSET_I_TRUE;
    isInfoPair->nisip_isSet->ngisi_sources = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}

static int
ngcllInformationServiceInfoPairSetOptions(
    ngclContext_t *context,
    ngcllInformationServiceInfoPair_t *isInfoPair,
    ngiStringList_t *options,
    int *error)
{
    static const char fName[] =
        "ngcllInformationServiceInfoPairSetOptions";
    char **optArray;
    int optSize;
    ngLog_t *log;
    int result;

    assert(options != NULL);
    assert(isInfoPair != NULL);

    assert(isInfoPair->nisip_isSet->ngisi_nOptions == 0);
    assert(isInfoPair->nisip_isSet->ngisi_options == NULL);

    optSize = 0;
    optArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &optArray, &optSize, options, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    isInfoPair->nisip_entities->ngisi_nOptions = optSize;
    isInfoPair->nisip_entities->ngisi_options = optArray;

    isInfoPair->nisip_isSet->ngisi_nOptions = NGI_ISSET_I_TRUE;
    isInfoPair->nisip_isSet->ngisi_options = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}


/**
 * RemoteMachineInfoPair
 */
ngcllRemoteMachineInfoPair_t *
ngcllRemoteMachineInfoPairCreate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInfoPairCreate";
    ngcllRemoteMachineInfoPair_t *ret;
    ngLog_t *log;

    log = context->ngc_log;

    ret = NGI_ALLOCATE(ngcllRemoteMachineInfoPair_t, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Remote Machine Info Pair.\n"); 
        return NULL;
    }

    ret->nrmip_entities = ngcliRemoteMachineInformationAllocate(
        context, error);
    if (ret->nrmip_entities == NULL) {
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Remote Machine Information.\n"); 
        return NULL;
    }

    ret->nrmip_isSet = ngcliRemoteMachineInformationAllocate(
        context, error);
    if (ret->nrmip_isSet == NULL) {
        ngiFree(ret->nrmip_entities, log, error);
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Remote Machine Information.\n"); 
        return NULL;
    }

    /* Success */
    return ret;
}

int
ngcllRemoteMachineInfoPairDestruct(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInfoPairDestruct";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (rmInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngclRemoteMachineInformationRelease(context,
        rmInfoPair->nrmip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the storage to "
            "register Remote Machine Information.\n"); 
        return 0;
    }

    /* nrmip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliRemoteMachineInformationFree(context,
        rmInfoPair->nrmip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Machine Information.\n"); 
        return 0;
    }

    result = ngcliRemoteMachineInformationFree(context,
        rmInfoPair->nrmip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Machine Information.\n"); 
        return 0;
    }

    result = NGI_DEALLOCATE(
        ngcllRemoteMachineInfoPair_t, rmInfoPair, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the Remote Machine Info Pair.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllRemoteMachineInfoPairListDestruct(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInfoPairListDestruct";
    ngcllRemoteMachineInfoPair_t *curRm, *nextRm;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (rmInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    curRm = rmInfoPair;
    do {
        nextRm = curRm->nrmip_next;
        result = ngcllRemoteMachineInfoPairDestruct(context, curRm, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct RemoteMachineInfoPair.\n"); 
            return 0;
        }

        curRm = nextRm;
    } while (curRm != NULL);

    /* Success */
    return 1;
}

int
ngcllRemoteMachineInfoPairInitialize(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInfoPairInitialize";
    ngLog_t *log;
    int result;

    assert(rmInfoPair != NULL);
    assert(rmInfoPair->nrmip_entities != NULL);
    assert(rmInfoPair->nrmip_isSet != NULL);

    log = context->ngc_log;

    rmInfoPair->nrmip_next = NULL;

    result = ngcliRemoteMachineInformationInitialize(context,
        rmInfoPair->nrmip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Remote Machine Information.\n"); 
        return 0;
    }

    result = ngcllRemoteMachineInformationSetDefault(
        rmInfoPair->nrmip_entities, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Remote Machine Information to default.\n"); 
        return 0;
    }

    result = ngcliRemoteMachineInformationInitialize(context,
        rmInfoPair->nrmip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Remote Machine Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

ngcllRemoteMachineInfoPair_t *
ngcllRemoteMachineInfoPairDuplicate(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *src,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInfoPairDuplicate";
    ngcllRemoteMachineInfoPair_t *newPair;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    newPair = ngcllRemoteMachineInfoPairCreate(context, error);
    if (newPair == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't create the Remote Machine Info Pair.\n"); 
        return NULL;
    }

    result = ngcllRemoteMachineInfoPairInitialize(context, newPair, error);
    if (result == 0) {
        ngcllRemoteMachineInfoPairDestruct(context, newPair, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Remote Machine Info Pair.\n"); 
        return NULL;
    }

    result = ngclRemoteMachineInformationRelease(
        context, newPair->nrmip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Machine Information.\n"); 
        return NULL;
    }
    result = ngcliRemoteMachineInformationCopy(context,
        src->nrmip_entities, newPair->nrmip_entities, error);
    if (result == 0) {
        ngcllRemoteMachineInfoPairDestruct(context, newPair, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Remote Machine Information.\n"); 
        return NULL;
    }

    /* make shallow copy (string member is not duplicated) */
    *(newPair->nrmip_isSet) = *(src->nrmip_isSet);

    /* Success */
    return newPair;
}

int
ngcllRemoteMachineInfoPairRegister(
    ngcllRemoteMachineInfoPair_t **dst,
    ngcllRemoteMachineInfoPair_t *src)
{
    ngcllRemoteMachineInfoPair_t *cur;

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    src->nrmip_next = NULL;

    if (*dst == NULL) {
        *dst = src;
        return 1;
    }

    cur = *dst;
    while (cur->nrmip_next != NULL) {
        cur = cur->nrmip_next;
    }

    cur->nrmip_next = src;

    /* Success */
    return 1;
}


/**
 * This macro sets dst->member if src->isSet->member flag is on.
 * argument member should be the member variable
 * of ngclRemoteMachineInformation_t.
 * RMISP means RemoteMachineInformationSetPair.
 */

#define NGCLL_RMISP_MEMBER_SET_STR(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_STR( \
            dst, (src)->nrmip_entities, (src)->nrmip_isSet, member)

#define NGCLL_RMISP_MEMBER_SET_INT(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_INT( \
            dst, (src)->nrmip_entities, (src)->nrmip_isSet, member)

#define NGCLL_RMISP_MEMBER_SET_DOUBLE(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_DOUBLE( \
            dst, (src)->nrmip_entities, (src)->nrmip_isSet, member)

/**
 * This function sets dst from src member which isSet flag is true.
 */
int
ngcllRemoteMachineInformationSetPair(
    ngclRemoteMachineInformation_t *dst,
    ngcllRemoteMachineInfoPair_t *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInformationSetPair";
    int result;
    ngclMPInCPUsInformation_t *ncpuInfo;

    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_hostName)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_tagName)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_portNo)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_invokeServerType)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_commProxyType)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commProxyStaging)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_commProxyPath)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commProxyBufferSize)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_infoServiceTag)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_mpiNcpus)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_keepConnect)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_forceXDR)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_jobManager)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_subject)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_clientHostName)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobStartTimeout)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobEndTimeout)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobMaxTime)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobMaxWallTime)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobMaxCpuTime)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_jobQueue)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_jobProject)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobHostCount)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobMinMemory)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_jobMaxMemory)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_heartBeat)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_heartBeatTimeoutCount)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_redirectEnable)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_tcpNodelay)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_retryInfo.ngcri_count)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_retryInfo.ngcri_interval)
    NGCLL_RMISP_MEMBER_SET_DOUBLE(dst, src, ngrmi_retryInfo.ngcri_increase)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_retryInfo.ngcri_useRandom)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_argumentTransfer)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_compressionType)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_compressionThresholdNbytes)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_argumentBlockSize)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_workDirectory)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_coreDumpSize)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commLogEnable)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_commLogInfo.ngli_filePath)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_commLogInfo.ngli_suffix)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commLogInfo.ngli_nFiles)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commLogInfo.ngli_maxFileSize)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commLogInfo.ngli_overWriteDir)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_debug.ngdi_enable)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_debug.ngdi_terminalPath)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_debug.ngdi_display)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_debug.ngdi_debuggerPath)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_debugBusyLoop)

    /* Copy the MPI number of CPUs information list*/
    if (!SLIST_EMPTY(&src->nrmip_entities->ngrmi_mpiNcpusList)) {
        SLIST_FOREACH(ncpuInfo, &src->nrmip_entities->ngrmi_mpiNcpusList, ngmni_entry) {
            result = ngcliRemoteMachineInformationAppendMPInCPUs(
                dst, ncpuInfo->ngmni_className, ncpuInfo->ngmni_nCPUs, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't copy the MPI number of CPUs Information.\n");
                return 0;
            }
        }
    }

    /* Set ngrmi_invokeServerOptions (add to dst) */
    if (src->nrmip_isSet->ngrmi_invokeServerNoptions != 0) {
        assert(src->nrmip_isSet->ngrmi_invokeServerOptions != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngrmi_invokeServerOptions,
            &dst->ngrmi_invokeServerNoptions,
            src->nrmip_entities->ngrmi_invokeServerOptions,
            src->nrmip_entities->ngrmi_invokeServerNoptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Set ngrmi_commProxyOptions (add to dst) */
    if (src->nrmip_isSet->ngrmi_commProxyNoptions != 0) {
        assert(src->nrmip_isSet->ngrmi_commProxyOptions != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngrmi_commProxyOptions,
            &dst->ngrmi_commProxyNoptions,
            src->nrmip_entities->ngrmi_commProxyOptions,
            src->nrmip_entities-> ngrmi_commProxyNoptions, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Set ngrmi_rslExtensions (add to dst) */
    if (src->nrmip_isSet->ngrmi_rslExtensionsSize != 0) {
       assert(src->nrmip_isSet->ngrmi_rslExtensions != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngrmi_rslExtensions,
            &dst->ngrmi_rslExtensionsSize,
            src->nrmip_entities->ngrmi_rslExtensions,
            src->nrmip_entities->ngrmi_rslExtensionsSize, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Set ngrmi_nEnvironments and ngrmi_environment (add to dst) */
    if (src->nrmip_isSet->ngrmi_nEnvironments != 0) {
        assert(src->nrmip_isSet->ngrmi_environment != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngrmi_environment, &dst->ngrmi_nEnvironments,
            src->nrmip_entities->ngrmi_environment,
            src->nrmip_entities->ngrmi_nEnvironments, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't add StringArray.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

#undef NGCLL_RMISP_MEMBER_SET_STR
#undef NGCLL_RMISP_MEMBER_SET_INT
#undef NGCLL_RMISP_MEMBER_SET_DOUBLE

/**
 * This function finds item which have same hostName in member as argument.
 */
ngcllRemoteMachineInfoPair_t *
ngcllRemoteMachineInfoPairGet(
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    char *hostName)
{
    ngcllRemoteMachineInfoPair_t *cur;
    int (*cmp)(const char *s1, const char *s2);

    cmp = (NGI_HOSTNAME_CASE_SENSITIVE ? strcmp : strcasecmp);
    assert(cmp != NULL);

    for (cur = rmInfoPair; cur != NULL; cur = cur->nrmip_next) {
        assert(cur->nrmip_entities->ngrmi_hostName != NULL);
        if ((*cmp)(hostName, cur->nrmip_entities->ngrmi_hostName) == 0) {
            /* found */
            return cur;
        }
    }

    /* not found */
    return NULL;
}

/**
 * This function finds item which have same hostName and tagName
 * in member as argument.
 */
ngcllRemoteMachineInfoPair_t *
ngcllRemoteMachineInfoPairGetWithTag(
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    char *hostName,
    char *tagName)
{
    ngcllRemoteMachineInfoPair_t *cur;
    int (*cmp)(const char *s1, const char *s2);

    cmp = (NGI_HOSTNAME_CASE_SENSITIVE ? strcmp : strcasecmp);
    assert(cmp != NULL);

    assert(hostName != NULL);

    for (cur = rmInfoPair; cur != NULL; cur = cur->nrmip_next) {
        assert(cur->nrmip_entities->ngrmi_hostName != NULL);
        if ((*cmp)(hostName, cur->nrmip_entities->ngrmi_hostName) != 0) {
            continue;
        }

        if (tagName != NULL) {
            if (cur->nrmip_entities->ngrmi_tagName == NULL) {
                continue;
            }
            if ((*cmp)(tagName, cur->nrmip_entities->ngrmi_tagName) != 0) {
                continue;
            }
        } else {
            if (cur->nrmip_entities->ngrmi_tagName != NULL) {
                continue;
            }
        }
        
        /* found */
        return cur;
    }

    /* not found */
    return NULL;
}

static int
ngcllRemoteMachineInfoPairSetInvokeServerOptions(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    ngiStringList_t *options,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteMachineInfoPairSetInvokeServerOptions";
    char **optArray;
    int optSize;
    ngLog_t *log;
    int result;

    assert(options != NULL);
    assert(rmInfoPair != NULL);

    assert(rmInfoPair->nrmip_isSet->ngrmi_invokeServerNoptions == 0);
    assert(rmInfoPair->nrmip_isSet->ngrmi_invokeServerOptions == NULL);

    optSize = 0;
    optArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &optArray, &optSize, options, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    rmInfoPair->nrmip_entities->ngrmi_invokeServerNoptions = optSize;
    rmInfoPair->nrmip_entities->ngrmi_invokeServerOptions = optArray;

    rmInfoPair->nrmip_isSet->ngrmi_invokeServerNoptions = NGI_ISSET_I_TRUE;
    rmInfoPair->nrmip_isSet->ngrmi_invokeServerOptions = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}

static int
ngcllRemoteMachineInfoPairSetCommunicationProxyOptions(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    ngiStringList_t *options,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteMachineInfoPairSetCommunicationProxyOptions";
    ngclRemoteMachineInformation_t *rmInfo;
    char **optArray;
    int optSize;
    ngLog_t *log;
    int result;

    assert(options != NULL);
    assert(rmInfoPair != NULL);

    assert(rmInfoPair->nrmip_isSet->ngrmi_commProxyNoptions == 0);
    assert(rmInfoPair->nrmip_isSet->ngrmi_commProxyOptions == NULL);

    optSize = 0;
    optArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &optArray, &optSize, options, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    rmInfo = rmInfoPair->nrmip_entities;
    rmInfo->ngrmi_commProxyNoptions = optSize;
    rmInfo->ngrmi_commProxyOptions = optArray;

    rmInfo = rmInfoPair->nrmip_isSet;
    rmInfo->ngrmi_commProxyNoptions = NGI_ISSET_I_TRUE;
    rmInfo->ngrmi_commProxyOptions = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}
static int
ngcllRemoteMachineInfoPairSetRSLextensions(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    ngiStringList_t *rslExtensions,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteMachineInfoPairSetRSLextensions";
    char **extArray;
    int extSize;
    ngLog_t *log;
    int result;

    assert(rslExtensions != NULL);
    assert(rmInfoPair != NULL);

    /* rmInfoPair->nrmip_isSet->ngrmi_rslExtensionsSize == 0 or 1 */
    assert(rmInfoPair->nrmip_isSet->ngrmi_rslExtensions == NULL);

    extSize = 0;
    extArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &extArray, &extSize, rslExtensions, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(extSize > 0);
    assert(extArray != NULL);

    rmInfoPair->nrmip_entities->ngrmi_rslExtensionsSize = extSize;
    rmInfoPair->nrmip_entities->ngrmi_rslExtensions = extArray;

    rmInfoPair->nrmip_isSet->ngrmi_rslExtensionsSize = NGI_ISSET_I_TRUE;
    rmInfoPair->nrmip_isSet->ngrmi_rslExtensions = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}

static int
ngcllRemoteMachineInfoPairSetEnviron(
    ngclContext_t *context,
    ngcllRemoteMachineInfoPair_t *rmInfoPair,
    ngiStringList_t *environ,
    int *error)
{
    static const char fName[] = "ngcllRemoteMachineInfoPairSetEnviron";
    char **envArray;
    int envSize;
    ngLog_t *log;
    int result;

    assert(environ != NULL);
    assert(rmInfoPair != NULL);

    assert(rmInfoPair->nrmip_isSet->ngrmi_nEnvironments == 0);
    assert(rmInfoPair->nrmip_isSet->ngrmi_environment == NULL);

    envSize = 0;
    envArray = NULL;
    log = context->ngc_log;

    result = ngiStringListToStringArray(
        &envArray, &envSize, environ, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert StringList to StringArray.\n"); 
        return 0;
    }
    assert(envSize > 0);
    assert(envArray != NULL);

    rmInfoPair->nrmip_entities->ngrmi_nEnvironments = envSize;
    rmInfoPair->nrmip_entities->ngrmi_environment = envArray;

    rmInfoPair->nrmip_isSet->ngrmi_nEnvironments = NGI_ISSET_I_TRUE;
    rmInfoPair->nrmip_isSet->ngrmi_environment = NGCL_ISSET_A_TRUE;
    
    /* Success */
    return 1;
}

/**
 * ExecutablePathInfoPair
 */
ngcllExecutablePathInfoPair_t *
ngcllExecutablePathInfoPairCreate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInfoPairCreate";
    ngcllExecutablePathInfoPair_t *ret;
    ngLog_t *log;

    log = context->ngc_log;

    ret = (ngcllExecutablePathInfoPair_t *)
              ngiCalloc(1, sizeof(ngcllExecutablePathInfoPair_t), log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Executable Path Info Pair.\n"); 
        return NULL;
    }

    ret->nepip_entities = ngcliExecutablePathInformationAllocate(
        context, error);
    if (ret->nepip_entities == NULL) {
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Executable Path Information.\n"); 
        return NULL;
    }

    ret->nepip_isSet = ngcliExecutablePathInformationAllocate(
        context, error);
    if (ret->nepip_isSet == NULL) {
        ngiFree(ret->nepip_entities, log, error);
        ngiFree(ret, log, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the Executable Path Information.\n"); 
        return NULL;
    }

    /* Success */
    return ret;
}

int
ngcllExecutablePathInfoPairDestruct(
    ngclContext_t *context,
    ngcllExecutablePathInfoPair_t *epInfoPair,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInfoPairDestruct";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (epInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngclExecutablePathInformationRelease(context,
        epInfoPair->nepip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Executable Path Info Pair.\n"); 
        return 0;
    }

    /* nepip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    ngiFree(epInfoPair->nepip_entities, log, error);
    ngiFree(epInfoPair->nepip_isSet, log, error);

    ngiFree(epInfoPair, log, error);

    return 1;
}

int
ngcllExecutablePathInfoPairListDestruct(
    ngclContext_t *context,
    ngcllExecutablePathInfoPair_t *epInfoPair,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInfoPairListDestruct";
    ngcllExecutablePathInfoPair_t *curEp, *nextEp;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (epInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    curEp = epInfoPair;
    do {
        nextEp = curEp->nepip_next;
        result = ngcllExecutablePathInfoPairDestruct(context, curEp, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct ExecutablePathInfoPair.\n"); 
            return 0;
        }

        curEp = nextEp;
    } while (curEp != NULL);

    /* Success */
    return 1;
}

int
ngcllExecutablePathInfoPairInitialize(
    ngclContext_t *context,
    ngcllExecutablePathInfoPair_t *epInfoPair,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInfoPairInitialize";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    assert(epInfoPair != NULL);
    assert(epInfoPair->nepip_entities != NULL);
    assert(epInfoPair->nepip_isSet != NULL);

    epInfoPair->nepip_next = NULL;

    result = ngcliExecutablePathInformationInitialize(context,
        epInfoPair->nepip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Executable Path Information.\n"); 
        return 0;
    }
    result = ngcllExecutablePathInformationSetDefault(
        epInfoPair->nepip_entities, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Executable Path Information"
            " to default.\n"); 
        return 0;
    }

    result = ngcliExecutablePathInformationInitialize(context,
        epInfoPair->nepip_isSet, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Executable Path Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

ngcllExecutablePathInfoPair_t *
ngcllExecutablePathInfoPairDuplicate(
    ngclContext_t *context,
    ngcllExecutablePathInfoPair_t *src,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInfoPairDuplicate";
    ngcllExecutablePathInfoPair_t *newPair;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    newPair = ngcllExecutablePathInfoPairCreate(context, error);
    if (newPair == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't create the Executable Path Info Pair.\n"); 
        return NULL;
    }

    result = ngcllExecutablePathInfoPairInitialize(context, newPair, error);
    if (result == 0) {
        ngcllExecutablePathInfoPairDestruct(context, newPair, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Executable Path Info Pair.\n"); 
        return NULL;
    }

    result = ngclExecutablePathInformationRelease(
        context, newPair->nepip_entities, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Executable Path Info Pair.\n"); 
        return NULL;
    }
    result = ngcliExecutablePathInformationCopy(context,
             src->nepip_entities, newPair->nepip_entities, error);
    if (result == 0) {
        ngcllExecutablePathInfoPairDestruct(context, newPair, error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Executable Path Info Pair.\n"); 
        return NULL;
    }

    /* make shallow copy (string member is not duplicated) */
    *(newPair->nepip_isSet) = *(src->nepip_isSet);

    /* Success */
    return newPair;
}

int
ngcllExecutablePathInfoPairRegister(
    ngcllExecutablePathInfoPair_t **dst,
    ngcllExecutablePathInfoPair_t *src)
{
    ngcllExecutablePathInfoPair_t *cur;

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    src->nepip_next = NULL;

    if (*dst == NULL) {
        *dst = src;
        return 1;
    }

    cur = *dst;
    while (cur->nepip_next != NULL) {
        cur = cur->nepip_next;
    }

    cur->nepip_next = src;

    /* Success */
    return 1;
}

/**
 * This macro sets dst->member if src->isSet->member flag is on.
 * argument member should be the member variable
 * of ngclExecutablePathInformation_t.
 * EPISP means ExecutablePathInformationSetPair.
 */

#define NGCLL_EPISP_MEMBER_SET_STR(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_STR( \
            dst, (src)->nepip_entities, (src)->nepip_isSet, member)

#define NGCLL_EPISP_MEMBER_SET_INT(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_INT( \
            dst, (src)->nepip_entities, (src)->nepip_isSet, member)

/**
 * This function sets dst from src member which isSet flag is true.
 */
int
ngcllExecutablePathInformationSetPair(
    ngclExecutablePathInformation_t *dst,
    ngcllExecutablePathInfoPair_t *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInformationSetPair";

    NGCLL_EPISP_MEMBER_SET_STR(dst, src, ngepi_hostName)
    NGCLL_EPISP_MEMBER_SET_STR(dst, src, ngepi_className)
    NGCLL_EPISP_MEMBER_SET_STR(dst, src, ngepi_path)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_stagingEnable)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_backend)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_sessionTimeout)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_transferTimeout_argument)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_transferTimeout_result)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_transferTimeout_cbArgument)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_transferTimeout_cbResult)

    /* Success */
    return 1;
}

#undef NGCLL_EPISP_MEMBER_SET_STR
#undef NGCLL_EPISP_MEMBER_SET_INT

#undef NGCLL_ISP_MEMBER_SET_STR
#undef NGCLL_ISP_MEMBER_SET_INT
#undef NGCLL_ISP_MEMBER_SET_DOUBLE

/**
 * This function finds item which have same hostName and className
 *    in member as argument.
 */
ngcllExecutablePathInfoPair_t *
ngcllExecutablePathInfoPairGet(
    ngcllExecutablePathInfoPair_t *epInfoPair,
    char *hostName,
    char *className)
{
    ngcllExecutablePathInfoPair_t *cur;
    int (*cmp)(const char *s1, const char *s2);

    cmp = (NGI_HOSTNAME_CASE_SENSITIVE ? strcmp : strcasecmp);
    assert(cmp != NULL);

    cur = epInfoPair;
    while (cur != NULL) {
        assert(cur->nepip_entities->ngepi_hostName != NULL);
        assert(cur->nepip_entities->ngepi_className != NULL);
        if (((*cmp)(hostName, cur->nepip_entities->ngepi_hostName) == 0) &&
            (strcmp(className, cur->nepip_entities->ngepi_className) == 0)) {
            /* found */
            return cur;
        }

        cur = cur->nepip_next;
    }

    /* not found */
    return NULL;
}

/**
 * If dst is empty, then just copy src,
 * if dst is not empty, then add src string array to dst.
 */
static int
ngcllStringArrayAdd(
    char ***dstArray,
    int *dstSize,
    char **srcArray,
    int srcSize,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllStringArrayAdd";
    int newSize, i, j;
    char **newArray;
    int result;

    assert(dstArray != NULL);
    assert(dstSize != NULL);
    assert(*dstSize >= 0);
    assert((*dstSize == 0) || (*dstArray != NULL));
    assert(srcArray != NULL);
    assert(srcSize > 0);

    newSize = *dstSize + srcSize;
    newArray = (char **)ngiCalloc(newSize, sizeof(char *), log, error);
    if (newArray == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the StringArray.\n"); 
        return 0;
    }

    i = 0;
    /* Copy from dst */
    for (j = 0; j < *dstSize; j++, i++) {
        assert((*dstArray)[j] != NULL);

        newArray[i] = strdup((*dstArray)[j]);
        if (newArray[i] == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the string.\n"); 
            return 0;
        }
    }

    /* Copy from src */
    for (j = 0; j < srcSize; j++, i++) {
        assert(srcArray[j] != NULL);

        newArray[i] = strdup(srcArray[j]);
        if (newArray[i] == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the string.\n"); 
            return 0;
        }
    }
    assert(i == newSize);

    if (*dstArray != NULL) {
        result = ngcllStringArrayFree(*dstArray, *dstSize, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't deallocate the StringArray.\n"); 
            return 0;
        }
    }

    *dstSize = newSize;
    *dstArray = newArray;

    /* Success */
    return 1;
}

static int 
ngcllStringArrayFree(
    char **array,
    int size,
    ngLog_t *log,
    int *error)
{
    int i;

    assert(size > 0);
    assert(array != NULL);

    for (i = 0; i < size; i++) {
        assert(array[i] != NULL);
        ngiFree(array[i], log, error);
    }

    ngiFree(array, log, error);

    /* Succsess */
    return 1;
}

/**
 * Below 4 functions (*SetDefault) is used to set
 * application-deafult values.
 *   (NOTICE: If member was added *Information, don't forget to add
 *              *SetPair also.)
 */
static int
ngcllLocalMachineInformationSetDefault(
    ngclLocalMachineInformation_t *lmInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllLocalMachineInformationSetDefault";

    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    lmInfo->nglmi_hostName = NULL;
    lmInfo->nglmi_saveNsessions = 256;

    result = ngiLogInformationSetDefault(&lmInfo->nglmi_logInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
            "Can't set default value to the log information.\n");
        return 0;
    }
    result = ngiLogLevelInformationSetDefault(
        &lmInfo->nglmi_logLevels, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
            "Can't set default value to the log level information.\n");
        return 0;
    }

    lmInfo->nglmi_tmpDir = NULL; /* Use $TMPDIR or /tmp */
    lmInfo->nglmi_refreshInterval = 0; /* disable although pthread version */
    lmInfo->nglmi_invokeServerLog = NULL; /* no log */
    lmInfo->nglmi_commProxyLog = NULL; /* no log */
    lmInfo->nglmi_infoServiceLog = NULL; /* no log */
    lmInfo->nglmi_fortranCompatible = 0; /* false */
    lmInfo->nglmi_signals = NULL; /* default */
    lmInfo->nglmi_listenPort = 0; /* unspecified */
    /* signal handling default is set by Register */

    /* Success */
    return 1;
}

static int
ngcllInvokeServerInformationSetDefault(
    ngclInvokeServerInformation_t *isInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInformationSetDefault";

    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    isInfo->ngisi_type = NULL; /* no default type */
    isInfo->ngisi_path = NULL; /* Use default path */
    isInfo->ngisi_maxJobs = 0; /* Not use multiple Invoke Server */
    isInfo->ngisi_logFilePath = NULL; /* No specified log */
    isInfo->ngisi_statusPoll = 0;
    isInfo->ngisi_nOptions = 0;
    isInfo->ngisi_options = NULL;

    /* Success */
    return 1;
}

static int
ngcllCommunicationProxyInformationSetDefault(
    ngclCommunicationProxyInformation_t *cpInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllCommunicationProxyInformationSetDefault";

    if (cpInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    cpInfo->ngcpi_type = NULL; /* no default type */
    cpInfo->ngcpi_path = NULL; /* Use default path */
    cpInfo->ngcpi_bufferSize = 0; /* Use default buffer size */
    cpInfo->ngcpi_maxJobs = 0; /* Not use multiple Communication Proxy */
    cpInfo->ngcpi_logFilePath = NULL; /* No specified log */
    cpInfo->ngcpi_nOptions = 0;
    cpInfo->ngcpi_options = NULL;

    /* Success */
    return 1;
}

static int
ngcllInformationServiceInformationSetDefault(
    ngclInformationServiceInformation_t *isInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInformationSetDefault";

    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    isInfo->ngisi_tag = NULL;  /* no default tag */
    isInfo->ngisi_type = NULL; /* no default type */
    isInfo->ngisi_path = NULL; /* Use default path */
    isInfo->ngisi_logFilePath = NULL; /* No specified log */
    isInfo->ngisi_timeout = 0;
    isInfo->ngisi_nSources = 0;
    isInfo->ngisi_sources = NULL;
    isInfo->ngisi_nOptions = 0;
    isInfo->ngisi_options = NULL;

    /* Success */
    return 1;
}

static int
ngcllRemoteMachineInformationSetDefault(
    ngclRemoteMachineInformation_t *rmInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationSetDefault";

    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    rmInfo->ngrmi_hostName  = NULL; /* no default hostname */
    rmInfo->ngrmi_tagName  = NULL;
    rmInfo->ngrmi_portNo = 0;

    rmInfo->ngrmi_invokeServerType = NULL;
    rmInfo->ngrmi_invokeServerNoptions = 0;
    rmInfo->ngrmi_invokeServerOptions = NULL;

    rmInfo->ngrmi_commProxyType = NULL;
    rmInfo->ngrmi_commProxyStaging = 0; /* false */
    rmInfo->ngrmi_commProxyPath = NULL; /* use default path */
    rmInfo->ngrmi_commProxyBufferSize = 0; /* use default buffer size */
    rmInfo->ngrmi_commProxyNoptions = 0;
    rmInfo->ngrmi_commProxyOptions = NULL;

    rmInfo->ngrmi_infoServiceTag = NULL;

    rmInfo->ngrmi_mpiNcpus = 0;
    SLIST_INIT(&rmInfo->ngrmi_mpiNcpusList);
    rmInfo->ngrmi_keepConnect = 1; /* true */
    rmInfo->ngrmi_forceXDR = 0; /* false */
    rmInfo->ngrmi_jobManager = NULL;
    rmInfo->ngrmi_subject = NULL;
    rmInfo->ngrmi_clientHostName = NULL; /* Use <CLIENT> hostname */
    rmInfo->ngrmi_jobStartTimeout = 0;
    rmInfo->ngrmi_jobEndTimeout = -1; /* Wait forever */
    rmInfo->ngrmi_jobMaxTime = -1; /* undefined */
    rmInfo->ngrmi_jobMaxWallTime = -1; /* undefined */
    rmInfo->ngrmi_jobMaxCpuTime = -1; /* undefined */
    rmInfo->ngrmi_jobQueue = NULL;
    rmInfo->ngrmi_jobProject = NULL;
    rmInfo->ngrmi_jobHostCount = -1; /* undefined */
    rmInfo->ngrmi_jobMinMemory = -1; /* undefined */
    rmInfo->ngrmi_jobMaxMemory = -1; /* undefined */
    rmInfo->ngrmi_rslExtensionsSize = 0;
    rmInfo->ngrmi_rslExtensions = NULL;
    rmInfo->ngrmi_heartBeat = 60;
    rmInfo->ngrmi_heartBeatTimeoutCount = 5;
    rmInfo->ngrmi_redirectEnable = 1; /* true */
    rmInfo->ngrmi_tcpNodelay = NGI_TCP_NODELAY_DEFAULT; 
    rmInfo->ngrmi_retryInfo.ngcri_count = 4;
    rmInfo->ngrmi_retryInfo.ngcri_interval = 1;
    rmInfo->ngrmi_retryInfo.ngcri_increase = 2.0;
    rmInfo->ngrmi_retryInfo.ngcri_useRandom = 1; /* true */
    rmInfo->ngrmi_argumentTransfer = NG_ARGUMENT_TRANSFER_WAIT;
    rmInfo->ngrmi_compressionType = NG_COMPRESSION_TYPE_RAW;
    rmInfo->ngrmi_compressionThresholdNbytes = 64 * 1024; /* 64kB */
    rmInfo->ngrmi_argumentBlockSize = NGI_DEFAULT_BLOCK_SIZE;
    rmInfo->ngrmi_workDirectory = NULL;
    rmInfo->ngrmi_coreDumpSize = -2; /* -2 means undefined in config */

    rmInfo->ngrmi_commLogEnable = 0; /* false */
    result = ngiLogInformationSetDefault(&rmInfo->ngrmi_commLogInfo, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
            "Can't set default value to the communication log information.\n");
        return 0;
    }

    rmInfo->ngrmi_debug.ngdi_enable = 0; /* false */
    rmInfo->ngrmi_debug.ngdi_terminalPath = NULL; /* It may xterm */
    rmInfo->ngrmi_debug.ngdi_display = NULL; /* send environ? always :0.0 */
    rmInfo->ngrmi_debug.ngdi_debuggerPath = NULL; /* It may gdb */
    rmInfo->ngrmi_debugBusyLoop = 0; /* false */
    rmInfo->ngrmi_nEnvironments = 0;
    rmInfo->ngrmi_environment = NULL;

    /* Success */
    return 1;
}

static int
ngcllExecutablePathInformationSetDefault(
    ngclExecutablePathInformation_t *epInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInformationSetDefault";

    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    epInfo->ngepi_hostName = NULL;
    epInfo->ngepi_className = NULL;
    epInfo->ngepi_path = NULL;
    epInfo->ngepi_stagingEnable = 0; /* false */
    epInfo->ngepi_backend = NG_BACKEND_NORMAL;
    epInfo->ngepi_sessionTimeout = 0; /* disable */
    epInfo->ngepi_transferTimeout_argument = 0; /* disable */
    epInfo->ngepi_transferTimeout_result = 0; /* disable */
    epInfo->ngepi_transferTimeout_cbArgument = 0; /* disable */
    epInfo->ngepi_transferTimeout_cbResult = 0; /* disable */

    /* Success */
    return 1;
}

