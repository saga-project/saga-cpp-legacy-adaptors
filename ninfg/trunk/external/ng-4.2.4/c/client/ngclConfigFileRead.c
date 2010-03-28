#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclConfigFileRead.c,v $ $Revision: 1.123 $ $Date: 2008/03/28 06:23:29 $";
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
 * Module of client configuration file reading.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <assert.h>

#include "ng.h"
#include "ngConfigFile.h"

#define NGCLL_CONFIG_DUMMY_HOSTNAME "dummy"
static char *ngcllTrueStrArray[] = {
    "set"
};
#define NGCL_ISSET_A_TRUE ngcllTrueStrArray
/* ISSET must const, and copiable */

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
static int ngcllAttrFuncClient_loglevel_gt      NGCLL_CONFIG_ATTRFUNC_ARG;
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
static int ngcllAttrFuncClient_fortran_compatible NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_handling_signals NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_listen_port          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_listen_port_authonly NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_listen_port_GSI      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_listen_port_SSL      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncClient_tcp_nodelay      NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncLocalLdifBegin          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncLocalLdifEnd            NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncLocalLdif_filename      NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncFuncInfoBegin           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfoEnd             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_hostname       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_funcname       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_staging        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_path           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_backend        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_mpicpus        NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncFuncInfo_session_timeout  NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncMDSserverBegin          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserverEnd            NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_hostname      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_tag           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_type          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_port          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_protocol      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_path          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_subject       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_vo_name       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_client_timeout  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncMDSserver_server_timeout  NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncInvokeServerBegin       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServerEnd         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_type       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_path       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_max_jobs   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_logfile    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_statusPoll NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncInvokeServer_option     NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllAttrFuncServerBegin             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServerEnd               NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_hostname         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_tag              NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_port             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_mds_hostname     NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_mds_tag          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_invoke_server    NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_invoke_server_opt  NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_mpi_runCommand   NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_mpi_runCPUs      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_gass_scheme      NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_crypt            NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllAttrFuncServer_protocol         NGCLL_CONFIG_ATTRFUNC_ARG;
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
static int ngcllAttrFuncServer_heartbeatTrans   NGCLL_CONFIG_ATTRFUNC_ARG;
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
    {"loglevel_globusToolkit",  ngcllAttrFuncClient_loglevel_gt},
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
    {"fortran_compatible",	ngcllAttrFuncClient_fortran_compatible},
    {"handling_signals",	ngcllAttrFuncClient_handling_signals},
    {"listen_port",             ngcllAttrFuncClient_listen_port},
    {"listen_port_authonly",    ngcllAttrFuncClient_listen_port_authonly},
    {"listen_port_GSI",         ngcllAttrFuncClient_listen_port_GSI},
    {"listen_port_SSL",         ngcllAttrFuncClient_listen_port_SSL},
    {"tcp_nodelay",             ngcllAttrFuncClient_tcp_nodelay},
    {NULL, NULL}
};

static ngcllAttrFuncTable_t localLdifAttrs[] = {
    {"filename",         ngcllAttrFuncLocalLdif_filename},
    {NULL, NULL}
};

static ngcllAttrFuncTable_t funcInfoAttrs[] = {
    {"hostname",         ngcllAttrFuncFuncInfo_hostname},
    {"funcname",         ngcllAttrFuncFuncInfo_funcname},
    {"path",             ngcllAttrFuncFuncInfo_path},
    {"staging",          ngcllAttrFuncFuncInfo_staging},
    {"backend",          ngcllAttrFuncFuncInfo_backend},
    {"mpi_runNoOfCPUs",  ngcllAttrFuncFuncInfo_mpicpus},
    {"session_timeout",  ngcllAttrFuncFuncInfo_session_timeout},
    {NULL, NULL}
};

static ngcllAttrFuncTable_t mdsServerAttrs[] = {
    {"hostname",         ngcllAttrFuncMDSserver_hostname},
    {"tag",              ngcllAttrFuncMDSserver_tag},
    {"type",             ngcllAttrFuncMDSserver_type},
    {"port",             ngcllAttrFuncMDSserver_port},
    {"protocol",         ngcllAttrFuncMDSserver_protocol},
    {"path",             ngcllAttrFuncMDSserver_path},
    {"subject",          ngcllAttrFuncMDSserver_subject},
    {"vo_name",          ngcllAttrFuncMDSserver_vo_name},
    {"client_timeout",   ngcllAttrFuncMDSserver_client_timeout},
    {"server_timeout",   ngcllAttrFuncMDSserver_server_timeout},
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

static ngcllAttrFuncTable_t serverAttrs[] = {
    {"hostname",         ngcllAttrFuncServer_hostname},
    {"tag",              ngcllAttrFuncServer_tag},
    {"port",             ngcllAttrFuncServer_port},
    {"mds_hostname",     ngcllAttrFuncServer_mds_hostname},
    {"mds_tag",          ngcllAttrFuncServer_mds_tag},
    {"invoke_server",    ngcllAttrFuncServer_invoke_server},
    {"invoke_server_option", ngcllAttrFuncServer_invoke_server_opt},
    {"mpi_runCommand",   ngcllAttrFuncServer_mpi_runCommand},
    {"mpi_runNoOfCPUs",  ngcllAttrFuncServer_mpi_runCPUs},
    {"gass_scheme",      ngcllAttrFuncServer_gass_scheme},
    {"crypt",            ngcllAttrFuncServer_crypt},
    {"protocol",         ngcllAttrFuncServer_protocol},
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
    {"heartbeat_timeoutCountOnTransfer", ngcllAttrFuncServer_heartbeatTrans},
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
  {"INCLUDE",        includeAttrs,
   ngcllAttrFuncIncludeBegin,         ngcllAttrFuncIncludeEnd},

  {"CLIENT",         clientAttrs,
         ngcllAttrFuncClientBegin,    ngcllAttrFuncClientEnd},

  {"LOCAL_LDIF",     localLdifAttrs,
   ngcllAttrFuncLocalLdifBegin,       ngcllAttrFuncLocalLdifEnd},

  {"FUNCTION_INFO",  funcInfoAttrs,
   ngcllAttrFuncFuncInfoBegin,        ngcllAttrFuncFuncInfoEnd},

  {"MDS_SERVER",     mdsServerAttrs,
         ngcllAttrFuncMDSserverBegin, ngcllAttrFuncMDSserverEnd},

  {"INVOKE_SERVER",  invokeServerAttrs,
         ngcllAttrFuncInvokeServerBegin, ngcllAttrFuncInvokeServerEnd},

  {"SERVER",         serverAttrs,
         ngcllAttrFuncServerBegin,    ngcllAttrFuncServerEnd},

  {"SERVER_DEFAULT", serverAttrs,
   ngcllAttrFuncServerDefaultBegin,   ngcllAttrFuncServerDefaultEnd},

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
static int ngcllLocalMachineInformationSignalRegister(ngclContext_t *context,
    ngcllReadingState_t *readingState, int *error);
static int ngcllMDSserverInformationsRegister(ngclContext_t *context,
    ngcllReadingState_t *readingState, int *error);
static int ngcllMDSserverInformationCheckValue(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngclMDSserverInformation_t *mdsInfo, int *error);
static int ngcllInvokeServerInformationsRegister(ngclContext_t *context,
    ngcllReadingState_t *readingState, int *error);
static int ngcllInvokeServerInformationCheckValue(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngclInvokeServerInformation_t *isInfo, int *error);
static int ngcllRemoteMachineInformationsRegister(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);
static int ngcllRemoteMachineInformationCheckValue(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngclRemoteMachineInformation_t *rmInfo, int *error);
static int ngcllExecutablePathInformationsRegister(
    ngclContext_t *context, ngcllReadingState_t *readingState, int *error);
static int ngcllExecutablePathInformationsRegisterToRemoteMachineInformation(
    ngclContext_t *context, ngcllReadingState_t *readingState,
    ngcllExecutablePathInfoPair_t *epInfoPair,
    ngcliExecutablePathInformation_t *epiCur, int *error);


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
static int ngcllRemoteMachineInfoPairSetInvokeServerOptions(
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
static int ngcllStringArrayFree(char **array, int size);

/* Set Default */
static int ngcllLocalMachineInformationSetDefault(
    ngclLocalMachineInformation_t *lmInfo, ngLog_t *log, int *error);
static int ngcllMDSserverInformationSetDefault(
    ngclMDSserverInformation_t *mdsInfo, ngLog_t *log, int *error);
static int ngcllInvokeServerInformationSetDefault(
    ngclInvokeServerInformation_t *isInfo, ngLog_t *log, int *error);
static int ngcllRemoteMachineInformationSetDefault(
    ngclRemoteMachineInformation_t *rmInfo, ngLog_t *log, int *error);
static int ngcllExecutablePathInformationSetDefault(
    ngcliExecutablePathInformation_t *epInfo, ngLog_t *log, int *error);

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
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Get configFile from argument */
    if ((configFile != NULL) && (configFile[0] != '\0')) {
        realConfigFile = configFile;

        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Using client configuration file \"%s\""
            " from initialize/configuration API argument.\n",
            fName, configFile);
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Client configuration file name was not supplied from "
            "initialize/configuration API argument.\n", fName);
    }

    /* Get configFile from environment variable */
    if (realConfigFile == NULL) {
        envConfigFile = getenv(NGCLI_ENVIRONMENT_CONFIG_FILE);
        if ((envConfigFile != NULL) && (envConfigFile[0] != '\0')) {
            realConfigFile = envConfigFile;

            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
                "%s: Using client configuration file \"%s\""
                " from environment variable.\n",
                fName, envConfigFile);
        } else {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
                "%s: Client configuration file name was not supplied from "
                "$%s environment variable.\n",
                fName, NGCLI_ENVIRONMENT_CONFIG_FILE);
        }
    }

    /* Get configFile from home directory */
    if (realConfigFile == NULL) {
        result = ngcllConfigFileGetHomeConfigFileName(
            context, &homeConfigFile, error);
        if ((result != 1) || (homeConfigFile == NULL)) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Configuration file name was not resolved.\n", fName);
            return 0;
        }

        realConfigFile = homeConfigFile;

        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Using client configuration file \"%s\""
            " from home directory.\n",
            fName, homeConfigFile);
    }

    /* Read configuration file */
    result = ngcllConfigFileRead(context, realConfigFile, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Configuration file \"%s\" read failed.\n",
            fName, realConfigFile);
    }

    if (homeConfigFile != NULL) {
        globus_libc_free(homeConfigFile);
        homeConfigFile = NULL;

        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Client configuration file name was not supplied"
                " from both initialize function argument and"
                " $%s environment variable.\n",
                fName, NGCLI_ENVIRONMENT_CONFIG_FILE);
        }
    }

    if (result != 1) {
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

    /* Check the arguments */
    assert(context != NULL);
    assert(homeConfigFile != NULL);

    log = context->ngc_log;
    homeDir = NULL;
    *homeConfigFile = NULL;
    homeConfigLength = 0;

    /* Get the passwd database */
    passwd = getpwuid(geteuid());
    if (passwd == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: getpwuid() failed.\n", fName);
        return 0;
    }

    homeDir = passwd->pw_dir;
    if ((homeDir == NULL) || (homeDir[0] == '\0')) {
        NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Home directory from getpwuid() is not valid.\n", fName);
        return 0;
    }

    assert(NGCLI_CONFIG_FILE_HOME != NULL);

    homeConfigLength =
        strlen(homeDir) + strlen(NGCLI_CONFIG_FILE_HOME) + 1;

    *homeConfigFile = globus_libc_calloc(homeConfigLength, sizeof(char));
    if (*homeConfigFile == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage for string.\n", fName);
        return 0;
    }

    snprintf(*homeConfigFile, homeConfigLength, "%s%s",
        homeDir, NGCLI_CONFIG_FILE_HOME);

    /* Success */
    return 1;
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
    int result, fileOpen;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    tokenReadInfo = NULL;
    fileOpen = 0;

    /* Check the arguments */
    if (configFile == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: configFile is NULL.\n", fName);
        goto error;
    }
    
    /* Is already reading? */
    if (context->ngc_configFileReading == 1) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Configuration file is already reading.\n", fName);
        goto error;
    }
    context->ngc_configFileReading = 1;

    /* log */
    if (context->ngc_configFileReadCount > 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL, 
            "%s: Reading the configuration file again.\n", fName);
    }

    /* Open configuration file and get tokenReadInfo */
    result = ngiConfigFileOpen(configFile, &tokenReadInfo, 1, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Opening configuration file \"%s\" failed.\n",
            fName, configFile);
        goto error;
    }
    fileOpen = 1;

    /* Parse configuration file */
    result = ngcllConfigFileParse(context, tokenReadInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Parsing configuration file \"%s\" fail.\n",
            fName, configFile);
        goto error;
    }

    /* Close configuration file */
    result = ngiConfigFileClose(tokenReadInfo, log, error);
    fileOpen = 0;
    tokenReadInfo = NULL;
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing configuration file \"%s\" failed.\n",
            fName, configFile);
        goto error;
    }

    context->ngc_configFileReadCount++;
    context->ngc_configFileReading = 0;

    /* log */
    if (context->ngc_configFileReadCount == 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL, 
            "%s: Reading the configuration file was successful.\n",
            fName);
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL, 
            "%s: Reading the configuration file was successful. (%d times)\n",
            fName, context->ngc_configFileReadCount);
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Closing configuration file \"%s\" failed.\n",
                fName, configFile);
        }
    }

    context->ngc_configFileReading = 0;

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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to initialize configuration file"
            " parsing data structure.\n", fName);
        goto error;
    }
    attrFuncInitialized = 1;

    /* Remain given config file name */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedConfigs), tokenReadInfo->ntri_filename);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the string.\n", fName);
        goto error;
    }

    /* Parse */
    result = ngcllConfigFileParseSub(
        context, tokenReadInfo, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to parse configuration file.\n", fName);
        goto error;
    }

    /* Register the Information */
    result = ngcllAttrFuncRegisterInformation(
        context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to register the configuration file data.\n",
            fName);
        goto error;
    }

    /* Finalize Attribute function data */
    result = ngcllAttrFuncFinalize(
        context, readingState, error);
    attrFuncInitialized = 0;
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to finalize configuration file"
            " parsing data structure.\n", fName);
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to finalize configuration file"
                " parsing data structure.\n", fName);
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
                if (result != 1) {
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: %s section open process fail.\n",
                        fName, tagName);
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
                if (result != 1) {
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: %s section close process fail.\n",
                        fName, tagName);
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
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: attribute \"%s\" process fail.\n",
                    fName, attrName);
                return 0;
            }

        } else {
            /**
             * (GetToken() == GET_TOKEN_SUCCESS) &&
             *    (token->nti_type != NG_TOKEN_TAG) &&
             *    (token->nti_type != NG_TOKEN_ATTR)
             * won't be happen
             */
            ngLogPrintf(log,
                 NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                 "%s: GetToken failed.\n",
                 fName);
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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to construct configuration file"
            " parsing data structure.\n", fName);
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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to destruct configuration file"
            " parsing data structure.\n", fName);
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
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* Register LocalMachineInformation */
    result = ngcllLocalMachineInformationsRegister(
        context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to register LocalMachineInformation.\n", fName);
        return 0;
    }

    /* Register MDSserverInformation */
    result = ngcllMDSserverInformationsRegister(
        context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to register MDSserverInformations.\n", fName);
        return 0;
    }

    /* Register InvokeServerInformation */
    result = ngcllInvokeServerInformationsRegister(
        context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to register InvokeServerInformations.\n", fName);
        return 0;
    }

    /* Register RemoteMachineInformation */
    result = ngcllRemoteMachineInformationsRegister(
        context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to register RemoteMachineInformations.\n", fName);
        return 0;
    }

    /* Register ExecutablePathInformation */
    result = ngcllExecutablePathInformationsRegister(
        context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to register ExecutablePathInformations.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
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
    newState = (ngcllReadingState_t *)
        globus_libc_malloc(sizeof(ngcllReadingState_t));
    if (newState == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage for configuration file"
            " parsing data structure.\n", fName);
        goto error;
    }

    /* Configuration file */
    newState->nrs_appearedConfigs = NULL;
    newState->nrs_appearedLocalLdifs = NULL;

    /* LocalMachineInformation */
    newState->nrs_lmiAppeared = 0; /* FALSE */
    newState->nrs_lmInfo = ngcllLocalMachineInfoPairCreate(
        context, error);
    if (newState->nrs_lmInfo == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage for Local Machine Information.\n",
             fName);
        goto error;
    }
    newState->nrs_signalAppeared = 0;
    newState->nrs_appearedSignals = NULL;

    result = ngcllLocalMachineInfoPairInitialize(
        context, newState->nrs_lmInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize Local Machine Information.\n",
            fName);
        goto error;
    }

    /* MDSserverInformation */
    newState->nrs_appearedMDSservers = NULL;
    newState->nrs_appearedNoTagMDSservers = NULL;
    newState->nrs_appearedMDStags = NULL;
    newState->nrs_mdsInfos = NULL;
    newState->nrs_curMDSinfo = NULL;

    /* InvokeServerInformation */
    newState->nrs_appearedInvokeServers = NULL;
    newState->nrs_isInfos = NULL;
    newState->nrs_curISinfo = NULL;
    newState->nrs_curISinfoOptions = NULL;

    /* RemoteMachineInformation */
    newState->nrs_appearedRmInfoHosts = NULL;
    newState->nrs_appearedNoTagRmInfoHosts = NULL;
    newState->nrs_appearedRmInfoTags = NULL;
    newState->nrs_appearedMDSserversInRm = NULL;
    newState->nrs_appearedInvokeServersInRm = NULL;
    newState->nrs_rmInfos = NULL;
    newState->nrs_rmInfoEps = NULL;

    newState->nrs_curRmInfo = NULL;
    newState->nrs_curRmInfoEps = NULL;
    newState->nrs_curRmInfoInvokeServerOptions = NULL;
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

    /* LocalLDIF RemoteMachineInformation */
    newState->nrs_appearedLocalRmInfoHosts = NULL;
    newState->nrs_localRmInfos = NULL;
    newState->nrs_localCurRmInfo = NULL;

    /* LocalLDIF ExecutablePathInformation */
    newState->nrs_localEpInfos = NULL;
    newState->nrs_localCurEpInfo = NULL;

    newState->nrs_localCurEpInfoStub = NULL;

    /* LocalLDIF RemoteClassInformation */
    newState->nrs_appearedClasses = NULL;

    *readingState = newState;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* LocalMachineInformation */
    if ((newState != NULL) && (newState->nrs_lmInfo != NULL)) {
        result = ngcllLocalMachineInfoPairDestruct(
            context, newState->nrs_lmInfo, NULL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't destruct the storage for Local Machine Information.\n",
             fName);
        newState->nrs_lmInfo = NULL;
    }

    if (newState != NULL) {
        globus_libc_free(newState);
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
            macroResult = ngiStringListDestruct((stringList)); \
            if (macroResult != 1) { \
                (retResult) = 0; \
                ngLogPrintf(log, \
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, \
                    "%s: Can't destruct the string list.\n", \
                     fName); \
                /* Not return */ \
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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid config data (appeared configuration file is NULL).\n",
             fName);
    }
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedConfigs, retResult)

    /* Local LDIF file */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedLocalLdifs, retResult)

    /* LocalMachineInformation */
    if (readingState->nrs_lmInfo != NULL) {
        result = ngcllLocalMachineInfoPairDestruct(
            context, readingState->nrs_lmInfo, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Local Machine Information Pair.\n",
                 fName);
        }
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Local Machine Information Pair is not available.\n",
             fName);
    }

    if(readingState->nrs_appearedSignals != NULL) {
        globus_libc_free(readingState->nrs_appearedSignals);
    }

    /* MDSserverInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedMDSservers, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedNoTagMDSservers, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedMDStags, retResult)

    if (readingState->nrs_mdsInfos != NULL) {
        result = ngcllMDSserverInfoPairListDestruct(
            context, readingState->nrs_mdsInfos, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the MDS Server Information"
                " Pair list.\n",
                fName);
        }
    }

    if (readingState->nrs_curMDSinfo != NULL) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid config data"
            " (current MDS Server Information is not NULL).\n",
            fName);
    }

    /* InvokeServerInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedInvokeServers, retResult)

    if (readingState->nrs_isInfos != NULL) {
        result = ngcllInvokeServerInfoPairListDestruct(
            context, readingState->nrs_isInfos, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Invoke Server Information"
                " Pair list.\n",
                fName);
        }
    }

    if ((readingState->nrs_curISinfo != NULL) ||
        (readingState->nrs_curISinfoOptions != NULL)) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid config data"
            " (current Invoke Server Information is not NULL).\n",
            fName);
    }

    /* RemoteMachineInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedRmInfoHosts, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedNoTagRmInfoHosts, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedRmInfoTags, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedMDSserversInRm, retResult)

    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedInvokeServersInRm, retResult)

    if (readingState->nrs_rmInfos != NULL) {
        result = ngcllRemoteMachineInfoPairListDestruct(
            context, readingState->nrs_rmInfos, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Remote Machine Information"
                " Pair list.\n",
                fName);
        }
    }

    if (readingState->nrs_rmInfoEps != NULL) {
        result = ngcllExecutablePathInfoPairListDestruct(
            context, readingState->nrs_rmInfoEps, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Executable Path Information"
                " Pair list.\n",
                fName);
        }
    }

    if ((readingState->nrs_curRmInfo != NULL) ||
        (readingState->nrs_curRmInfoEps != NULL) ||
        (readingState->nrs_curRmInfoInvokeServerOptions != NULL) ||
        (readingState->nrs_curRmInfoRSLextensions != NULL) ||
        (readingState->nrs_curRmInfoEnviron != NULL) ||
        (readingState->nrs_curRmInfoHosts != NULL)) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid config data"
            " (current Remote Machine Information is not NULL).\n",
            fName);
    }

    /* DefaultRemoteMachineInformation */
    if (readingState->nrs_defRmInfo != NULL) {
        result = ngcllRemoteMachineInfoPairDestruct(
            context, readingState->nrs_defRmInfo, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Default Remote Machine Information"
                " Pair.\n",
                fName);
        }
    }

    /* ExecutablePathInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedRmInfoHostsInEp, retResult)

    if (readingState->nrs_epInfos != NULL) {
        result = ngcllExecutablePathInfoPairListDestruct(
            context, readingState->nrs_epInfos, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Executable Path Information"
                " Pair list.\n",
                fName);
        }
    }

    if (readingState->nrs_curEpInfo != NULL) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid config data"
            " (current Executable Path Information is not NULL).\n",
            fName);
    }

    /* LocalLDIF RemoteMachineInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedLocalRmInfoHosts, retResult)

    if (readingState->nrs_localRmInfos != NULL) {
        result = ngcllRemoteMachineInfoPairListDestruct(
            context, readingState->nrs_localRmInfos, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Remote Machine Information"
                " Pair list on the Local LDIF file.\n",
                fName);
        }
    }

    if (readingState->nrs_localCurRmInfo != NULL) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid config data"
            " (current Remote Machine Information in Local LDIF file"
            " is not NULL).\n",
            fName);
    }

    /* LocalLDIF ExecutablePathInformation */
    if (readingState->nrs_localEpInfos != NULL) {
        result = ngcllExecutablePathInfoPairListDestruct(
            context, readingState->nrs_localEpInfos, error);
        if (result != 1) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Executable Path Information"
                " Pair list on the Local LDIF file.\n",
                fName);
        }
    }

    if ((readingState->nrs_localCurEpInfo != NULL) ||
        (readingState->nrs_localCurEpInfoStub != NULL)) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid config data"
            " (current Executable Path Information in Local LDIF file"
            " is not NULL).\n",
            fName);
    }

    /* LocalLDIF RemoteClassInformation */
    NGCLL_STRING_LIST_DESTRUCT(
        readingState->nrs_appearedClasses, retResult)

    globus_libc_free(readingState);

    if (retResult != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to destruct the storage for configuration file"
            " parsing data structure.\n", fName);

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
    ngclLocalMachineInformation_t *lmiCur;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* Skip if this call is not the first time configuration file read */
    if (context->ngc_configFileReadCount > 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL, 
            "%s: Skip the Local Machine Information update.\n",
             fName);
        /* Success */
        return 1;
    }

    /* Create temporary area for register */
    lmiCur = ngcliLocalMachineInformationAllocate(context, error);
    if (lmiCur == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage to "
            "register Local Machine Information.\n",
             fName);
        return 0;
    }

    /* clear to zero */
    result = ngcliLocalMachineInformationInitialize(context, lmiCur, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Local Machine Information.\n", fName);
        return 0;
    }

    /* set to application default */
    result = ngcllLocalMachineInformationSetDefault(lmiCur, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't set the Local Machine Information to default.\n",
            fName);
        return 0;
    }

    /* set to config file client section */
    result = ngcllLocalMachineInformationSetPair(
        lmiCur, readingState->nrs_lmInfo, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't set the Local Machine Information Pair.\n", fName);
        return 0;
    }

    /* Register to context */
    result = ngcliLocalMachineInformationCacheRegister(
        context, lmiCur, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the Local Machine Information.\n", fName);
        return 0;
    }

    result = ngclLocalMachineInformationRelease(context, lmiCur, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Local Machine Information.\n", fName);
        return 0;
    }

    result = ngcliLocalMachineInformationFree(context, lmiCur, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't deallocate the Local Machine Information.\n", fName);
        return 0;
    }

    /* Register the signal */
    result = ngcllLocalMachineInformationSignalRegister(
        context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the Signal.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register the signal.
 */
static int
ngcllLocalMachineInformationSignalRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllLocalMachineInformationSignalRegister";
    int *signalTable, size;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    size = 0;
    signalTable = readingState->nrs_appearedSignals;

    if (readingState->nrs_signalAppeared == 1) {
        assert(signalTable != NULL);

        /* Find the tail */
        for (; signalTable[size] != 0; size++);
    } else {
        signalTable = NULL; /* NULL is default, size 0 is no-signal */
    }

    result = ngcliNinfgManagerSignalRegister(
        signalTable, size, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the Signal.\n", fName);
        return 0;
    }

    result = ngcliNinfgManagerSignalManagerStart(log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't start the Signal Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register all MDS Server information.
 */
static int
ngcllMDSserverInformationsRegister(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInformationsRegister";
    ngiStringList_t *mdsInfoHostsInRm, *curMdsHostName;
    ngcllMDSserverInfoPair_t *curMdsInfoPair;
    int doNextRegister, registerCurRmInfo;
    ngclMDSserverInformation_t mdsInfo;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;
    mdsInfoHostsInRm = NULL;

    /* Get MDSservers only in Remote Machine Information */
    result = ngiStringListSubtract(
        readingState->nrs_appearedMDSserversInRm,
        readingState->nrs_appearedMDSservers,
        NGI_HOSTNAME_CASE_SENSITIVE, &mdsInfoHostsInRm);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to subtract StringList.\n", fName);
        return 0;
    }

    /* first MDS info */
    doNextRegister = 0;
    curMdsInfoPair = NULL;
    curMdsHostName = NULL;

    if (readingState->nrs_mdsInfos != NULL) {
        doNextRegister = 1;
        curMdsInfoPair = readingState->nrs_mdsInfos;
    }

    if (mdsInfoHostsInRm != NULL) {
        doNextRegister = 1;
        curMdsHostName = mdsInfoHostsInRm;
    }
    
    while (doNextRegister != 0) {
        /* Determine which, MDSinfo or MDS host name. */
        registerCurRmInfo = 0;
        if (curMdsInfoPair != NULL) {
            registerCurRmInfo = 1;
        } else if (curMdsHostName != NULL) {
            registerCurRmInfo = 0;
        } else {
            abort();
        }

        /* Initialize */
        result = ngcliMDSserverInformationInitialize(
            context, &mdsInfo, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the MDS Server Information.\n",
                fName);
            return 0;
        }

        /* Set default */
        result = ngcllMDSserverInformationSetDefault(
            &mdsInfo, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the MDS Server Information to default.\n",
                fName);
            return 0;
        }

        if (registerCurRmInfo != 0) {
            assert(curMdsInfoPair != NULL);

            /* set to config file */
            result = ngcllMDSserverInformationSetPair(
                &mdsInfo, curMdsInfoPair, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the MDS Server Information.\n",
                    fName);
                return 0;
            }
        } else {
            /* Set host name */
            assert(curMdsHostName->nsl_string != NULL);
            assert(mdsInfo.ngmsi_hostName == NULL);
            mdsInfo.ngmsi_hostName = strdup(curMdsHostName->nsl_string);
            if (mdsInfo.ngmsi_hostName == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string.\n", fName);
                return 0;
            }
        }

        /* Check the setting of MDS Server */
        result = ngcllMDSserverInformationCheckValue(
            context, readingState, &mdsInfo, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to check the MDS Server Information.\n", fName);
            return 0;
        }

        /* Register to context */
        result = ngcliMDSserverInformationCacheRegister(
            context, &mdsInfo, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the MDS Server Information.\n",
                fName);
            return 0;
        }

        /* Release */
        result = ngclMDSserverInformationRelease(context, &mdsInfo, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't release MDS Server Information.\n", fName);
            return 0;
        }

        /* Next MDS info */
        if (registerCurRmInfo != 0) {
            curMdsInfoPair = curMdsInfoPair->nmsip_next;
            doNextRegister = 0;
            if ((curMdsInfoPair != NULL) || (curMdsHostName != NULL)) {
                doNextRegister = 1;
            }
        } else {
            curMdsHostName = curMdsHostName->nsl_next;
            doNextRegister = ((curMdsHostName != NULL) ? 1 : 0);
        }
    }

    if (mdsInfoHostsInRm != NULL) {
        result = ngiStringListDestruct(mdsInfoHostsInRm);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the StringList.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

static int
ngcllMDSserverInformationCheckValue(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInformationCheckValue";
    char *tagName, *hostName;
    ngLog_t *log;

    /* Check arguments */
    assert(context != NULL);
    assert(readingState != NULL);
    assert(mdsInfo != NULL);

    tagName = mdsInfo->ngmsi_tagName;
    hostName = mdsInfo->ngmsi_hostName;
    log = context->ngc_log;

#ifdef NGI_NO_MDS2_MODULE
    if (mdsInfo->ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2) {
        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS2 is not configured for this Ninf-G installation.\n",
            fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS Server Information"
            " (tag name = \"%s\", host name = \"%s\") invalid.\n",
            fName, ((tagName != NULL) ? tagName : "null"),
            ((hostName != NULL) ? hostName : "null"));
        return 0;
    }
#endif /* NGI_NO_MDS2_MODULE */

#ifdef NGI_NO_MDS4_MODULE
    if (mdsInfo->ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS4) {
        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS4 is not configured for this Ninf-G installation.\n",
            fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: MDS Server Information"
            " (tag name = \"%s\", host name = \"%s\") invalid.\n",
            fName, ((tagName != NULL) ? tagName : "null"),
            ((hostName != NULL) ? hostName : "null"));
        return 0;
    }
#endif /* NGI_NO_MDS4_MODULE */

    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: MDS Server Information"
        " (tag name = \"%s\", host name = \"%s\") is valid.\n",
        fName, ((tagName != NULL) ? tagName : "null"),
        ((hostName != NULL) ? hostName : "null"));

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
        NGI_FILENAME_CASE_SENSITIVE, &isInfosInRm);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to subtract StringList.\n", fName);
        return 0;
    }

    /* first Invoke Server info */
    doNextRegister = 0;
    curIsInfoPair = NULL;
    curIsName = NULL;

    if (readingState->nrs_isInfos != NULL) {
        doNextRegister = 1;
        curIsInfoPair = readingState->nrs_isInfos;
    }

    if (isInfosInRm != NULL) {
        doNextRegister = 1;
        curIsName = isInfosInRm;
    }
    
    while (doNextRegister != 0) {
        /* Determine which, ISinfo or IS host name. */
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the Invoke Server Information.\n",
                fName);
            return 0;
        }

        /* Set default */
        result = ngcllInvokeServerInformationSetDefault(
            &isInfo, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the Invoke Server Information to default.\n",
                fName);
            return 0;
        }

        if (registerCurRmInfo != 0) {
            assert(curIsInfoPair != NULL);

            /* set to config file */
            result = ngcllInvokeServerInformationSetPair(
                &isInfo, curIsInfoPair, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Invoke Server Information.\n",
                    fName);
                return 0;
            }
        } else {
            /* Set type name */
            assert(curIsName->nsl_string != NULL);
            assert(isInfo.ngisi_type == NULL);
            isInfo.ngisi_type = strdup(curIsName->nsl_string);
            if (isInfo.ngisi_type == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string.\n", fName);
                return 0;
            }
        }

        /* Check the setting of Invoke Server */
        result = ngcllInvokeServerInformationCheckValue(
            context, readingState, &isInfo, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to check the Invoke Server Information.\n", fName);
            return 0;
        }

        /* Register to context */
        result = ngcliInvokeServerInformationCacheRegister(
            context, &isInfo, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the Invoke Server Information.\n",
                fName);
            return 0;
        }

        /* Release */
        result = ngclInvokeServerInformationRelease(context, &isInfo, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't release Invoke Server Information.\n", fName);
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
        result = ngiStringListDestruct(isInfosInRm);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the StringList.\n", fName);
            return 0;
        }
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

#ifdef NG_PTHREAD
    /* Check the Invoke Server executable file */
    result = ngcliInvokeServerProgramCheckAccess(
        context, isInfo->ngisi_type,
        isInfo->ngisi_path, /* NULL or path */
        error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s executable was not found.\n",
            fName, isInfo->ngisi_type);
        return 0;
    }
#else /* NG_PTHREAD */
    /* Do Nothing */
    result = 1;
    assert(result != 0);
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s executable check suppressed"
        " for this NonThread version of Ninf-G.\n",
        fName, isInfo->ngisi_type);

#endif /* NG_PTHREAD */

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
    ngclRemoteMachineInformation_t *rmiCur; /* for to register to context */
    ngcllRemoteMachineInfoPair_t *rmInfoPair, *foundPair;
    ngiStringList_t *rmInfoHostNames1, *rmInfoHostNames2, *cur_hostName;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    /* Create temporary area for register */
    rmiCur = ngcliRemoteMachineInformationAllocate(context, error);
    if (rmiCur == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage to "
            "register Remote Machine Information.\n",
             fName);
        return 0;
    }

    /* Register Default RemoteMachineInformation */
    {
        /* clear to zero */
        result = ngcliRemoteMachineInformationInitialize(
            context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't initialize the Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* set to application default */
        result = ngcllRemoteMachineInformationSetDefault(rmiCur, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't set the Remote Machine Information to default.\n",
                fName);
            return 0;
        }

        /* set to config file server default section */
        if (readingState->nrs_defRmInfo != NULL) {
            result = ngcllRemoteMachineInformationSetPair(
                rmiCur, readingState->nrs_defRmInfo, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Remote Machine Information.\n", fName);
                return 0;
            }
        }

        /* Check the setting of server default section */
        result = ngcllRemoteMachineInformationCheckValue(
            context, readingState, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to check the Remote Machine Information.\n", fName);
            return 0;
        }

        /* Register to context */
        result = ngcliDefaultRemoteMachineInformationCacheRegister(
            context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* Release */
        result = ngclRemoteMachineInformationRelease(context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't release the Remote Machine Information.\n", fName);
            return 0;
        }
    }

    /* Register RemoteMachineInformations in rmInfos, localRmInfos */
    rmInfoPair = NULL; /* retrieve head item */
    while((rmInfoPair = ngcllRemoteMachineInfoPairGetNextInTwo(
            readingState->nrs_rmInfos, readingState->nrs_localRmInfos,
            rmInfoPair)) != NULL) {

        /* clear to zero */
        result = ngcliRemoteMachineInformationInitialize(
            context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't initialize the Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* set to application default */
        result = ngcllRemoteMachineInformationSetDefault(rmiCur, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't set the Remote Machine Information to default.\n",
                fName);
            return 0;
        }

        /* set to local ldif value */
        assert(rmInfoPair->nrmip_entities->ngrmi_hostName != NULL);
        foundPair = ngcllRemoteMachineInfoPairGet(
            readingState->nrs_localRmInfos,
            rmInfoPair->nrmip_entities->ngrmi_hostName);
        if (foundPair != NULL) {
            result = ngcllRemoteMachineInformationSetPair(
                rmiCur, foundPair, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Remote Machine Information.\n", fName);
                return 0;
            }
        }

        /* set to config file server default section */
        if (readingState->nrs_defRmInfo != NULL) {
            result = ngcllRemoteMachineInformationSetPair(
                rmiCur, readingState->nrs_defRmInfo, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Remote Machine Information.\n",
                    fName);
                return 0;
            }
        }

        /* set to config file server section */
        foundPair = ngcllRemoteMachineInfoPairGetWithTag(
            readingState->nrs_rmInfos,
            rmInfoPair->nrmip_entities->ngrmi_hostName,
            rmInfoPair->nrmip_entities->ngrmi_tagName);
        if (foundPair != NULL) {
            result = ngcllRemoteMachineInformationSetPair(
                rmiCur, foundPair, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Remote Machine Information.\n",
                    fName);
                return 0;
            }
        }

        /* Check the setting of server default section */
        result = ngcllRemoteMachineInformationCheckValue(
            context, readingState, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to check the Remote Machine Information.\n", fName);
            return 0;
        }

        /* Register to context */
        result = ngcliRemoteMachineInformationCacheRegister(
            context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* Release */
        result = ngclRemoteMachineInformationRelease(context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't release the Remote Machine Information.\n", fName);
            return 0;
        }
    }

    /* Register RemoteMachineInformations appearing in only FUNC_INFO */
    result = ngiStringListSubtract(
        readingState->nrs_appearedRmInfoHostsInEp,
        readingState->nrs_appearedRmInfoHosts,
        NGI_HOSTNAME_CASE_SENSITIVE, &rmInfoHostNames1);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to subtract StringList.\n", fName);
        return 0;
    }
    result = ngiStringListSubtract(
        rmInfoHostNames1,
        readingState->nrs_appearedLocalRmInfoHosts,
        NGI_HOSTNAME_CASE_SENSITIVE, &rmInfoHostNames2);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to subtract StringList.\n", fName);
        return 0;
    }
    if (rmInfoHostNames1 != NULL) {
        result = ngiStringListDestruct(rmInfoHostNames1);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to destruct the StringList.\n", fName);
            return 0;
        }
    }

    cur_hostName = rmInfoHostNames2; /* may be always NULL */
    while (cur_hostName != NULL) {

        /* clear to zero */
        result = ngcliRemoteMachineInformationInitialize(
            context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't initialize the Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* set to application default */
        result = ngcllRemoteMachineInformationSetDefault(rmiCur, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't set the Remote Machine Information to default.\n",
                fName);
            return 0;
        }

        /* set to config file server default section */
        if (readingState->nrs_defRmInfo != NULL) {
            result = ngcllRemoteMachineInformationSetPair(
                rmiCur, readingState->nrs_defRmInfo, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Remote Machine Information.\n", fName);
                return 0;
            }
        }

        /* set hostname */
        assert(cur_hostName->nsl_string != NULL);
        assert(rmiCur->ngrmi_hostName == NULL);
        rmiCur->ngrmi_hostName = strdup(cur_hostName->nsl_string);
        if (rmiCur->ngrmi_hostName == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string.\n", fName);
            return 0;
        }

        /* Register to context */
        result = ngcliRemoteMachineInformationCacheRegister(
            context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* Release */
        result = ngclRemoteMachineInformationRelease(context, rmiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't release the Remote Machine Information.\n",
                fName);
            return 0;
        }

        cur_hostName = cur_hostName->nsl_next;
    }
    if (rmInfoHostNames2 != NULL) {
        result = ngiStringListDestruct(rmInfoHostNames2);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to destruct the StringList.\n", fName);
            return 0;
        }
    }

    /* Deallocate */
    result = ngcliRemoteMachineInformationFree(context, rmiCur, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't deallocate the Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
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
        section = "SERVER section of ";
        hostname = rmInfo->ngrmi_hostName;
    }

    if ((rmInfo->ngrmi_redirectEnable != 0) &&
        (rmInfo->ngrmi_jobEndTimeout == 0)) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: In %s%s, "
            "redirect_outerr may not work completely "
            "when job_stopTimeout is zero.\n",
            fName, section, hostname);
    }

    /* Check MDS Tag for this rmInfo registered */
    if (rmInfo->ngrmi_mdsTag != NULL) {
        result = ngiStringListCheckIncludeSameString(
            readingState->nrs_appearedMDStags,
            rmInfo->ngrmi_mdsTag,
            NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't check the StringList.\n", fName);
            return 0;
        }

        if (!isRegistered) {
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: MDS Tag \"%s\" for %s%s was not registered.\n",
                fName, rmInfo->ngrmi_mdsTag, section, hostname);
            return 0;
        }
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
    ngcliExecutablePathInformation_t *epiCur;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    ngcllExecutablePathInfoPair_t *epInfoPair;
    ngcllExecutablePathInfoPair_t *foundPair;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(readingState != NULL);

    log = context->ngc_log;

    epiCur = ngcliExecutablePathInformationAllocate(context, error);
    if (epiCur == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage to "
            "register Executable Path Information.\n",
             fName);
        return 0;
    }

    epInfoPair = NULL; /* retrieve head item */
    while((epInfoPair = ngcllExecutablePathInfoPairGetNextInTwo(
            readingState->nrs_epInfos, readingState->nrs_localEpInfos,
            epInfoPair)) != NULL) {

        /* clear to zero */
        result = ngcliExecutablePathInformationInitialize(
            context, epiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't initialize the Executable Path Information.\n",
                fName);
            return 0;
        }

        /* set to application default */
        result = ngcllExecutablePathInformationSetDefault(epiCur, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't set the Executable Path Information to default.\n",
                fName);
            return 0;
        }

        /* set to local ldif value */
        assert(epInfoPair->nepip_entities->ngepi_hostName != NULL);
        assert(epInfoPair->nepip_entities->ngepi_className != NULL);
        foundPair = ngcllExecutablePathInfoPairGet(
            readingState->nrs_localEpInfos,
            epInfoPair->nepip_entities->ngepi_hostName,
            epInfoPair->nepip_entities->ngepi_className);
        if (foundPair != NULL) {
            result = ngcllExecutablePathInformationSetPair(
                epiCur, foundPair, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Executable Path Information.\n",
                    fName);
                return 0;
            }
        }

        /* set to config file func info section */
        foundPair = ngcllExecutablePathInfoPairGet(
            readingState->nrs_epInfos,
            epInfoPair->nepip_entities->ngepi_hostName,
            epInfoPair->nepip_entities->ngepi_className);
        if (foundPair != NULL) {
            result = ngcllExecutablePathInformationSetPair(
                epiCur, foundPair, log, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't set the Executable Path Information.\n",
                    fName);
                return 0;
            }
        }

        /* Register to RemoteMachineInformation */
        result =
            ngcllExecutablePathInformationsRegisterToRemoteMachineInformation(
            context, readingState, epInfoPair, epiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the Executable Path Information.\n",
                fName);
            return 0;
        }

        /* Release */
        result = ngclExecutablePathInformationRelease(context, epiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't release the Executable Path Information.\n",
                fName);
            return 0;
        }
    }

    /* Register ExecutablePathInformations appearing in only SERVER mpi */
    for (epInfoPair = readingState->nrs_rmInfoEps; epInfoPair != NULL;
        epInfoPair = epInfoPair->nepip_next) {

        /* Skip if epInfo is available in <FUNCTION_INFO> */
        foundPair = ngcllExecutablePathInfoPairGet(
            readingState->nrs_epInfos,
            epInfoPair->nepip_entities->ngepi_hostName,
            epInfoPair->nepip_entities->ngepi_className);
        if (foundPair != NULL) {
            continue;
        }

        /* Skip if epInfo is available in <LOCAL_LDIF> */
        foundPair = ngcllExecutablePathInfoPairGet(
            readingState->nrs_localEpInfos,
            epInfoPair->nepip_entities->ngepi_hostName,
            epInfoPair->nepip_entities->ngepi_className);
        if (foundPair != NULL) {
            continue;
        }

        /* clear to zero */
        result = ngcliExecutablePathInformationInitialize(
            context, epiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't initialize the Executable Path Information.\n",
                fName);
            return 0;
        }

        /* set to application default */
        result = ngcllExecutablePathInformationSetDefault(epiCur, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't set the Executable Path Information to default.\n",
                fName);
            return 0;
        }

        /* set value */
        result = ngcllExecutablePathInformationSetPair(
            epiCur, epInfoPair, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't set the Executable Path Information.\n", fName);
            return 0;
        }

        /* Get target RemoteMachineInformation */
        rmInfoMng = ngcliRemoteMachineInformationCacheGetWithTag(
            context,
            epInfoPair->nepip_entities->ngepi_hostName,
            epInfoPair->nepip_tagName, error);
        if (rmInfoMng == NULL) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't get the Remote Machine Information.\n", fName);
            return 0;
        }

        /* Register to RemoteMachineInformation */
        result = ngcliExecutablePathInformationCacheRegister(
            context, rmInfoMng, epiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the Executable Path Information.\n",
                fName);
            return 0;
        }

        /* Release */
        result = ngclExecutablePathInformationRelease(context, epiCur, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't release the Executable Path Information.\n",
                fName);
            return 0;
        }
    }

    /* Deallocate */
    result = ngcliExecutablePathInformationFree(context, epiCur, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't deallocate the Executable Path Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register specified ExecutablePathInformation to context.
 * epiCur is registered to all RemoteMachineInformation,
 * which has same hostName.
 */
static int
ngcllExecutablePathInformationsRegisterToRemoteMachineInformation(
    ngclContext_t *context,
    ngcllReadingState_t *readingState,
    ngcllExecutablePathInfoPair_t *epInfoPair,
    ngcliExecutablePathInformation_t *epiCur,
    int *error)
{
    static const char fName[] =
        "ngcllExecutablePathInformationsRegisterToRemoteMachineInformation";
    ngcliExecutablePathInformation_t *epiRegist, epiCopy;
    ngclRemoteMachineInformationManager_t *cur;
    int (*cmp)(const char *s1, const char *s2);
    ngcllExecutablePathInfoPair_t *foundPair;
    char *hostName;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(epiCur != NULL);

    cmp = (NGI_HOSTNAME_CASE_SENSITIVE ? strcmp : strcasecmp);
    assert(cmp != NULL);

    log = context->ngc_log;
    hostName = epiCur->ngepi_hostName;

    cur = NULL; /* Retrieve head item */
    while((cur = ngcliRemoteMachineInformationCacheGetNext(
         context, cur, error)) != NULL) {

        if ((*cmp)(hostName, cur->ngrmim_info.ngrmi_hostName) != 0) {
            continue; 
        }

        epiRegist = epiCur;

        /* set config file SERVER section mpi_runNoOfCPUs value */
        foundPair = ngcllExecutablePathInfoPairGetWithTag(
            readingState->nrs_rmInfoEps,
            epInfoPair->nepip_entities->ngepi_hostName,
            cur->ngrmim_info.ngrmi_tagName,
            epInfoPair->nepip_entities->ngepi_className);

        if (foundPair != NULL) {
            /* clear to zero */
            result = ngcliExecutablePathInformationInitialize(
                context, &epiCopy, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't initialize the Executable Path Information.\n",
                    fName);
                return 0;
            }

            /* copy */
            result = ngcliExecutablePathInformationCopy(
                context, epiCur, &epiCopy, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't copy the Executable Path Information.\n",
                    fName);
                return 0;
            }

            assert(foundPair->nepip_isSet->ngepi_mpiNcpus != 0);
            epiCopy.ngepi_mpiNcpus =
                foundPair->nepip_entities->ngepi_mpiNcpus;

            epiRegist = &epiCopy;
        }

        /* Register to RemoteMachineInformation */
        result = ngcliExecutablePathInformationCacheRegister(
            context, cur, epiRegist, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the Executable Path Information.\n",
                fName);
            return 0;
        }

        if (foundPair != NULL) {
            /* Release */
            result = ngclExecutablePathInformationRelease(
                context, &epiCopy, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't release the Executable Path Information.\n",
                    fName);
                return 0;
            }
        }
    }

    return 1;
}

/**
 * template for each AttributeFunction members
 * first argument should member of
 *   ngclLocalMachineInformation_t or
 *   ngclMDSserverInformation_t or
 *   ngcliInvokeServerInformation_t or
 *   ngclRemoteMachineInformation_t
 * AF means ngcllAttrFunc
 */

#define NGCLL_AF_MEMBER_SET_STR(member, arg, entity, isset) \
    { \
        if ((isset)->member != NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        if ((entity)->member != NULL) { \
            globus_libc_free((entity)->member); \
        } \
         \
        (entity)->member = ngiReadStringFromArg((arg)); \
        if ((entity)->member == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset)->member = NGCL_ISSET_S_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_QSTR(member, arg, entity, isset) \
    { \
        if ((isset)->member != NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        if ((entity)->member != NULL) { \
            globus_libc_free((entity)->member); \
        } \
         \
        (entity)->member = ngiReadQuotedStringFromArg((arg), 0, NULL, 0); \
        if ((entity)->member == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset)->member = NGCL_ISSET_S_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_BOOL(member, arg, entity, isset) \
    { \
        char *true_false; \
        int result; \
         \
        if ((isset)->member != 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        true_false = ngiReadStringFromArg((arg)); \
        if (true_false == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadEnumFromArg( \
            true_false, NGI_ATTR_ARG_CASE_SENSITIVE, \
            2, "true", "false"); \
        if (!((result >= 1) && (result <= 2))) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            globus_libc_free(true_false); \
            return 0; \
        } \
        (entity)->member = (result == 1 ? 1 : 0); \
        (isset)->member = NGCL_ISSET_I_TRUE; \
        globus_libc_free(true_false); \
    }

#define NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, entity, isset) \
    { \
        int resultValue; \
        int result; \
         \
        resultValue = 0; \
         \
        if ((isset)->member != 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        result = ngiReadIntFromArg((arg), &resultValue); \
        if (result != 1) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid (not integer):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (entity)->member = resultValue; \
        (isset)->member = NGCL_ISSET_I_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_DOUBLE(member, minValue, arg, entity, isset) \
    { \
        double resultValue; \
        int result; \
         \
        resultValue = 0.0; \
         \
        if (((isset)->member > 0.001) || \
            ((isset)->member < -0.001)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        result = ngiReadDoubleFromArg((arg), &resultValue); \
        if (result != 1) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid (not integer):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        if (resultValue < ((minValue) - 0.000001)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (entity)->member = resultValue; \
        (isset)->member = NGCL_ISSET_D_TRUE; \
    }

#define NGCLL_AF_MEMBER_SET_LOGLEVEL(member, arg, entity, isset) \
    { \
        char *levelStr; \
        int result; \
         \
        if ((isset)->member != (ngLogLevel_t) 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        levelStr = ngiReadStringFromArg((arg)); \
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
            globus_libc_free(levelStr); \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                 fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset)->member = (ngLogLevel_t) NGCL_ISSET_I_TRUE; \
        globus_libc_free(levelStr); \
    }

#define NGCLL_AF_MEMBER_SET_UNIT( \
    member, minValue, arg, entity, isset, unitTable) \
    { \
        char *unitString; \
        int resultValue; \
        int result; \
         \
        resultValue = 0; \
         \
        if ((isset)->member != 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        unitString = ngiReadStringFromArg((arg)); \
        if (unitString == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadUnitNumFromArg(unitString, &resultValue, \
             unitTable, NGI_ATTR_ARG_CASE_SENSITIVE); \
        if (result != 1) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            globus_libc_free(unitString); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            globus_libc_free(unitString); \
            return 0; \
        } \
        (entity)->member = resultValue; \
        (isset)->member = NGCL_ISSET_I_TRUE; \
        globus_libc_free(unitString); \
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
    newConfigFile = ngiReadStringFromArg(token->nti_tokenStr);
    if (newConfigFile == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "No filename was specified", fName, attrName, NULL, error);
        goto error;
    }

    /* Check re-open */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedConfigs, newConfigFile,
        NGI_FILENAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
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
        &(readingState->nrs_appearedConfigs), newConfigFile);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register StringList.\n", fName);
        goto error;
    }

    result = ngiConfigFileOpen(
        newConfigFile, &newTokenReadInfo, 1, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Opening configuration file \"%s\" failed.\n",
            fName, newConfigFile);
        goto error;
    }
    fileOpen = 1;

    result = ngcllConfigFileParseSub(context,
        newTokenReadInfo, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to parse configuration file.\n", fName);
        goto error;
    }

    result = ngiConfigFileClose(newTokenReadInfo, log, error);
    fileOpen = 0;
    newTokenReadInfo = NULL;
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing configuration file fail.\n", fName);
        goto error;
    }

    globus_libc_free(newConfigFile);
    newConfigFile = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if ((newTokenReadInfo != NULL) && (fileOpen == 1)) {
        result = ngiConfigFileClose(newTokenReadInfo, log, NULL);
        fileOpen = 0;
        newTokenReadInfo = NULL;
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Closing configuration file fail.\n", fName);
        }
    }

    if (newConfigFile != NULL) {
        globus_libc_free(newConfigFile);
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
        nglmi_logInfo.ngli_level, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_loglevel_gt      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel_gt";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logInfo.ngli_levelGlobusToolkit, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_loglevel_ngprot  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel_ngprot";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logInfo.ngli_levelNinfgProtocol, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_loglevel_ngi     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel_ngi";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logInfo.ngli_levelNinfgInternal, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncClient_loglevel_grpc    NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncClient_loglevel_grpc";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_LOGLEVEL(
        nglmi_logInfo.ngli_levelGrpc, token->nti_tokenStr)

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
    ngcllLocalMachineInfoPair_t *lmInfoPair;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    lmInfoPair = readingState->nrs_lmInfo;

    NGCLL_AFC_MEMBER_SET_UNIT(
        nglmi_refreshInterval, 0, token->nti_tokenStr, ngiTimeUnitTable)

    if (lmInfoPair->nlmip_entities->nglmi_refreshInterval == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: disabling refresh credential for the client.\n", fName);
    } else {
#ifndef NG_PTHREAD
        /* Disabling refresh credential */
        lmInfoPair->nlmip_entities->nglmi_refreshInterval = 0;

        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: refresh credential not supported for this GlobusToolkit flavor.\n",
            fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: refresh credential is supported only for pthread version.\n", fName);
#endif /* NG_PTHREAD */
    }

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
    char **signalNames, *end, *endptr;
    int nParamSignals, *paramTable;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    paramSignalStrings = NULL;
    nParamSignals = 0;
    paramTable = NULL;
    nSignals = 0;
    signalNames = NULL;
    signalNumbers = NULL;

    cmp = (NGI_ATTR_ARG_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    if (readingState->nrs_signalAppeared != 0) {
        ngiConfigFileSyntaxError(context->ngc_log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0; 
    }

    assert(readingState->nrs_appearedSignals == NULL);
    paramSignalStrings = ngiReadStringListFromArg(token->nti_tokenStr);
    /* NULL is valid */

    result = ngiStringListCount(paramSignalStrings, &nParamSignals);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Count the string list failed.\n", fName);
        goto error;
    }

    paramTable = globus_libc_calloc(sizeof(int), nParamSignals + 1);
    if (paramTable == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the signal table.\n", fName);
        goto error;
    }

    for (i = 0; i < (nParamSignals + 1); i++) {
        paramTable[i] = 0; /* 0 is to terminate */
    }

    /* Get the signal names */
    result = ngiSignalManagerSignalNamesGet(
        &signalNames, &signalNumbers, &nSignals, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the signal table.\n", fName);
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
                ngiConfigFileSyntaxError(context->ngc_log, token,
                    "invalid signal name: ",
                    fName, attrName, cur->nsl_string, error);
                goto error;
            }
            found = 1;
        }

        /* Is this first appearance? */
        for (j = 0; j < i; j++) {
            if (paramTable[j] == foundNumber) {
                ngiConfigFileSyntaxError(context->ngc_log, token,
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

    readingState->nrs_signalAppeared = 1;
    readingState->nrs_appearedSignals = paramTable;
    paramTable = NULL;

    if (paramSignalStrings != NULL) {
        result = ngiStringListDestruct(paramSignalStrings);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the StringList.\n", fName);
            paramSignalStrings = NULL;
            goto error;
        }
        paramSignalStrings = NULL;
    }

    result = ngiSignalManagerSignalNamesDestruct(
        signalNames, signalNumbers, nSignals, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Signal Names.\n", fName);
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
        result = ngiStringListDestruct(paramSignalStrings);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the StringList.\n", fName);
            paramSignalStrings = NULL;
        }
    }

    if (paramTable != NULL) {
        globus_libc_free(paramTable);
        paramTable = NULL;
    }

    if ((signalNames != NULL) || (signalNumbers != NULL)) {
        result = ngiSignalManagerSignalNamesDestruct(
            signalNames, signalNumbers, nSignals, log, NULL);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Signal Names.\n", fName);
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

static int
ngcllAttrFuncClient_listen_port_authonly NGCLL_CONFIG_ATTRFUNC_ARG
{
    ngLog_t *log;
    static const char fName[] = "ngcllAttrFuncClient_listen_port_authonly";

    log = context->ngc_log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_INT(nglmi_listenPortAuthOnly, 0, token->nti_tokenStr)

    if (readingState->nrs_lmInfo->nlmip_entities->nglmi_listenPortAuthOnly >
        NGI_PORT_MAX) {
        ngiConfigFileSyntaxError(log, token,
            "attribute value invalid (too large):",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncClient_listen_port_GSI NGCLL_CONFIG_ATTRFUNC_ARG
{
    ngLog_t *log;
    static const char fName[] = "ngcllAttrFuncClient_listen_port_GSI";

    log = context->ngc_log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_INT(nglmi_listenPortGSI, 0, token->nti_tokenStr)

    if (readingState->nrs_lmInfo->nlmip_entities->nglmi_listenPortGSI >
        NGI_PORT_MAX) {
        ngiConfigFileSyntaxError(log, token,
            "attribute value invalid (too large):",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncClient_listen_port_SSL NGCLL_CONFIG_ATTRFUNC_ARG
{
    ngLog_t *log;
    static const char fName[] = "ngcllAttrFuncClient_listen_port_SSL";

    log = context->ngc_log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_INT(nglmi_listenPortSSL, 0, token->nti_tokenStr)

    if (readingState->nrs_lmInfo->nlmip_entities->nglmi_listenPortSSL >
        NGI_PORT_MAX) {
        ngiConfigFileSyntaxError(log, token,
            "attribute value invalid (too large):",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncClient_tcp_nodelay NGCLL_CONFIG_ATTRFUNC_ARG
{
    ngLog_t *log;
    static const char fName[] = "ngcllAttrFuncClient_tcp_nodelay";

    log = context->ngc_log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFC_MEMBER_SET_BOOL(nglmi_tcpNodelay, token->nti_tokenStr)

    return 1;
}

#undef NGCLL_AFC_MEMBER_SET_STR
#undef NGCLL_AFC_MEMBER_SET_BOOL
#undef NGCLL_AFC_MEMBER_SET_INT
#undef NGCLL_AFC_MEMBER_SET_LOGLEVEL
#undef NGCLL_AFC_MEMBER_SET_UNIT

/**
 * LOCAL_LDIF  section
 */

static int
ngcllAttrFuncLocalLdifBegin          NGCLL_CONFIG_ATTRFUNC_ARG
{
    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    /* Do nothing */
    return 1;
}

static int
ngcllAttrFuncLocalLdifEnd            NGCLL_CONFIG_ATTRFUNC_ARG
{
    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    /* Do nothing */
    return 1;
}

static int
ngcllAttrFuncLocalLdif_filename      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncLocalLdif_filename";
    char *localLdifFileName;
    int result, isRegistered;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    localLdifFileName = ngiReadStringFromArg(token->nti_tokenStr);
    if (localLdifFileName == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "No ldif filename was specified", fName, attrName, NULL, error);
    }

    /* Check re-appearing */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedLocalLdifs, localLdifFileName,
        NGI_FILENAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "reading the same Local LDIF file again :",
            fName, attrName, localLdifFileName, error);
    }

    /* Register appeared local ldif file names */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedLocalLdifs), localLdifFileName);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register StringList.\n", fName);
        return 0;
    }
    globus_libc_free(localLdifFileName);

    /* Read Local LDIF file */
    result = ngcllConfigFileReadLocalLdif(attrName, token, readingState,
        context, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Processing LocalLDIF file fail.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage to "
            "register Executable Path Information.\n",
             fName);
        return 0;
    }

    result = ngcllExecutablePathInfoPairInitialize(context,
        readingState->nrs_curEpInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Executable Path Info Pair.\n", fName);
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
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }
    if (!isRegistered) {
        result = ngiStringListRegister(
            &(readingState->nrs_appearedRmInfoHostsInEp),
            epInfoPair->nepip_entities->ngepi_hostName);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register StringList.\n", fName);
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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register Executable Path Info.\n", fName);
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
    backendStr = ngiReadStringFromArg(token->nti_tokenStr);
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
        globus_libc_free(backendStr);
        return 0;
    }
    globus_libc_free(backendStr);

    if (result == 1) {
        epInfoPair->nepip_entities->ngepi_backend = NG_BACKEND_NORMAL;
    } else if (result == 2) {
        epInfoPair->nepip_entities->ngepi_backend = NG_BACKEND_MPI;
    } else if (result == 3) {
        epInfoPair->nepip_entities->ngepi_backend = NG_BACKEND_BLACS;
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: reading backend string fail. result is %d\n",
            fName, result);
        return 0;
    }
    epInfoPair->nepip_isSet->ngepi_backend = (ngBackend_t) NGCL_ISSET_I_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncFuncInfo_mpicpus        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_mpicpus";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFF_MEMBER_SET_INT(ngepi_mpiNcpus, 1, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncFuncInfo_session_timeout  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncFuncInfo_session_timeout";
    ngcllExecutablePathInfoPair_t *epInfoPair;
    ngLog_t *log;

    log = context->ngc_log;
    epInfoPair = readingState->nrs_curEpInfo;

    NGCLL_AFF_MEMBER_SET_UNIT(
        ngepi_sessionTimeout, 0, token->nti_tokenStr, ngiTimeUnitTable)

    if (epInfoPair->nepip_entities->ngepi_sessionTimeout == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: disabling session timeout for the server.\n", fName);
    } else {
#ifndef NG_PTHREAD
        /* Disabling session_timeout */
        epInfoPair->nepip_entities->ngepi_sessionTimeout = 0;
     
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: session timeout not supported for this GlobusToolkit flavor.\n",
            fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: session timeout is supported only for pthread version.\n",
            fName);
#endif /* NG_PTHREAD */
    }

    /* Success */
    return 1;
}

#undef NGCLL_AFF_MEMBER_SET_STR
#undef NGCLL_AFF_MEMBER_SET_BOOL
#undef NGCLL_AFF_MEMBER_SET_INT
#undef NGCLL_AFF_MEMBER_SET_UNIT

/**
 * MDS_SERVER section
 */

static int
ngcllAttrFuncMDSserverBegin          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserverBegin";
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_curMDSinfo == NULL);

    log = context->ngc_log;

    /* Construct MDSserverInfoPair */
    readingState->nrs_curMDSinfo = ngcllMDSserverInfoPairCreate(
        context, error);
    if (readingState->nrs_curMDSinfo == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate MDS Server Information.\n", fName);
        return 0;
    }

    result = ngcllMDSserverInfoPairInitialize(
        context, readingState->nrs_curMDSinfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize MDS Server Information.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllAttrFuncMDSserverEnd            NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserverEnd";
    ngcllMDSserverInfoPair_t *mdsInfoPair;
    char *mdsHostName, *mdsTagName;
    int result, isRegistered;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    mdsInfoPair = NULL;

    if (readingState->nrs_curMDSinfo->nmsip_isSet->ngmsi_hostName == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "No hostname in section:",
                 fName, NULL, attrName, error);
        return 0;
    }

    mdsHostName = readingState->nrs_curMDSinfo->nmsip_entities->ngmsi_hostName;
    assert(mdsHostName != NULL);
    mdsTagName = readingState->nrs_curMDSinfo->nmsip_entities->ngmsi_tagName;

    /* assert mdsHostName not registered in NoTagMDSservers */
    if (mdsTagName == NULL) {
        result = ngiStringListCheckIncludeSameString(
            readingState->nrs_appearedNoTagMDSservers, mdsHostName,
            NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't check the StringList.\n", fName);
            return 0;
        }
        if (isRegistered) {
            ngiConfigFileSyntaxError(log, token,
                "No-Tag MDS Server information already registered for:",
                fName, NULL, mdsHostName, error);
            return 0;
        }

        /* Register to NoTagMDSservers */
        result = ngiStringListRegister(
            &(readingState->nrs_appearedNoTagMDSservers), mdsHostName);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register StringList.\n", fName);
            return 0;
        }
    }

    /* check available of mdsHostName in appearedMDSservers */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedMDSservers, mdsHostName,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }

    /* Register to appearedMDSservers */
    if (!isRegistered) {
        result = ngiStringListRegister(
            &(readingState->nrs_appearedMDSservers), mdsHostName);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register StringList.\n", fName);
            return 0;
        }
    }

    /* Move to curMDSinfo to mdsInfos */
    mdsInfoPair = readingState->nrs_curMDSinfo;
    readingState->nrs_curMDSinfo = NULL;

    result = ngcllMDSserverInfoPairRegister(
        &(readingState->nrs_mdsInfos), mdsInfoPair);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register MDS Server Info.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * template for each ngclMDSserverInformation_t members
 * first argument should member of ngclMDSserverInformation_t
 * AFM means ngcllAttrFuncMDSserver
 */

#define NGCLL_AFM_MEMBER_SET_STR(member, arg) \
         NGCLL_AF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_curMDSinfo->nmsip_entities, \
             readingState->nrs_curMDSinfo->nmsip_isSet)

#define NGCLL_AFM_MEMBER_SET_QSTR(member, arg) \
         NGCLL_AF_MEMBER_SET_QSTR(member, arg, \
             readingState->nrs_curMDSinfo->nmsip_entities, \
             readingState->nrs_curMDSinfo->nmsip_isSet)

#define NGCLL_AFM_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_curMDSinfo->nmsip_entities, \
             readingState->nrs_curMDSinfo->nmsip_isSet)

#define NGCLL_AFM_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_curMDSinfo->nmsip_entities, \
             readingState->nrs_curMDSinfo->nmsip_isSet, unitTable)

static int
ngcllAttrFuncMDSserver_hostname      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_hostname";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_STR(ngmsi_hostName, token->nti_tokenStr)

    /* Success */
    return 1;
}

static int
ngcllAttrFuncMDSserver_tag           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_tag";
    char *tagName;
    int result, isRegistered;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_STR(ngmsi_tagName, token->nti_tokenStr)

    log = context->ngc_log;

    tagName = readingState->nrs_curMDSinfo->nmsip_entities->ngmsi_tagName;
    assert(tagName != NULL);

    /* tag name registered check */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedMDStags, tagName,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }
    if (isRegistered) {
        ngiConfigFileSyntaxError(log, token,
            "MDS Tag already registered for:",
            fName, NULL, tagName, error);
        return 0;
    }

    /* Register to appearedMDStags */
    result = ngiStringListRegister(
        &(readingState->nrs_appearedMDStags), tagName);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register StringList.\n", fName);
        return 0;
    }

    return 1;
}

static int
ngcllAttrFuncMDSserver_type          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_type";
    ngcllMDSserverInfoPair_t *mdsInfoPair;
    char *mdsTypeStr;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    mdsInfoPair = readingState->nrs_curMDSinfo;

    if (mdsInfoPair->nmsip_isSet->ngmsi_type != 0) {
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0;
    }

    /* Get argument string */
    mdsTypeStr = ngiReadStringFromArg(token->nti_tokenStr);
    if (mdsTypeStr == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal type [MDS2/MDS4]:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* Set argument enum */
    result = ngiReadEnumFromArg(
        mdsTypeStr, NGI_ATTR_ARG_CASE_SENSITIVE,
        2, "MDS2", "MDS4");
    if (result == 0) {
        ngiConfigFileSyntaxError(log, token,
            "No such MDS type [MDS2/MDS4]:",
            fName, attrName, mdsTypeStr, error);
        globus_libc_free(mdsTypeStr);
        return 0;
    }
    globus_libc_free(mdsTypeStr);

    if (result == 1) {
        mdsInfoPair->nmsip_entities->ngmsi_type = NGCL_MDS_SERVER_TYPE_MDS2;
    } else if (result == 2) {
        mdsInfoPair->nmsip_entities->ngmsi_type = NGCL_MDS_SERVER_TYPE_MDS4;
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: reading type for MDS string fail. result is %d\n",
            fName, result);
        return 0;
    }
    mdsInfoPair->nmsip_isSet->ngmsi_type =
        (ngclMDSserverInformationType_t) NGCL_ISSET_I_TRUE;

#ifdef NGI_NO_MDS2_MODULE
    if (mdsInfoPair->nmsip_entities->ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS2) {
        ngiConfigFileSyntaxError(log, token,
            "MDS2 is not configured for this Ninf-G installation",
            fName, attrName, NULL, error);
        return 0;
    }
#endif /* NGI_NO_MDS2_MODULE */

#ifdef NGI_NO_MDS4_MODULE
    if (mdsInfoPair->nmsip_entities->ngmsi_type == NGCL_MDS_SERVER_TYPE_MDS4) {
        ngiConfigFileSyntaxError(log, token,
            "MDS4 is not configured for this Ninf-G installation",
            fName, attrName, NULL, error);
        return 0;
    }
#endif /* NGI_NO_MDS4_MODULE */

    /* Success */
    return 1;
}

static int
ngcllAttrFuncMDSserver_port          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_port";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_INT(ngmsi_portNo, 0, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncMDSserver_protocol      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_protocol";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_STR(ngmsi_protocol, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncMDSserver_path          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_path";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_STR(ngmsi_path, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncMDSserver_subject       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_subject";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_QSTR(ngmsi_subject, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncMDSserver_vo_name       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_vo_name";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_STR(ngmsi_voName, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncMDSserver_client_timeout  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_client_timeout";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_UNIT(
        ngmsi_clientTimeout, 0, token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

static int
ngcllAttrFuncMDSserver_server_timeout  NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncMDSserver_server_timeout";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFM_MEMBER_SET_UNIT(
        ngmsi_serverTimeout, 0, token->nti_tokenStr, ngiTimeUnitTable)

    return 1;
}

#undef NGCLL_AFM_MEMBER_SET_STR
#undef NGCLL_AFM_MEMBER_SET_QSTR
#undef NGCLL_AFM_MEMBER_SET_INT
#undef NGCLL_AFM_MEMBER_SET_UNIT

/**
 * INVOKE_SERVER section
 */

static int
ngcllAttrFuncInvokeServerBegin       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServerBegin";
    int result, isPthread;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    assert(readingState->nrs_curISinfo == NULL);

    log = context->ngc_log;

    isPthread = 0;
#ifdef NG_PTHREAD
    isPthread = 1;
#endif /* NG_PTHREAD */

    if (isPthread == 0) {
        /* NonThread version of Invoke Server is not supported. */

        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: invoke_server is not supported for"
            " this GlobusToolkit flavor.\n", fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: invoke_server is supported only for pthread version.\n",
            fName);

        return 0;
    }

    /* Construct InvokeServerInfoPair */
    readingState->nrs_curISinfo = ngcllInvokeServerInfoPairCreate(
        context, error);
    if (readingState->nrs_curISinfo == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate Invoke Server Information.\n", fName);
        return 0;
    }

    result = ngcllInvokeServerInfoPairInitialize(
        context, readingState->nrs_curISinfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize Invoke Server Information.\n", fName);
        return 0;
    }

    assert(readingState->nrs_curISinfoOptions == NULL);

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
    isInfoPair = NULL;

    if (readingState->nrs_curISinfo->nisip_isSet->ngisi_type == NULL) {
        ngiConfigFileSyntaxError(log, token,
                "No type name in section:",
                 fName, NULL, attrName, error);
        return 0;
    }

    isTypeName = readingState->nrs_curISinfo->nisip_entities->ngisi_type;
    assert(isTypeName != NULL);

    /* assert isTypeName not registered */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedInvokeServers, isTypeName,
        NGI_FILENAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
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
        &(readingState->nrs_appearedInvokeServers), isTypeName);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register StringList.\n", fName);
        return 0;
    }

    /* Set options */
    if (readingState->nrs_curISinfoOptions != NULL) {
        result = ngcllInvokeServerInfoPairSetOptions(
            context, readingState->nrs_curISinfo,
            readingState->nrs_curISinfoOptions, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set options to InvokeServerInfoPair.\n",
                fName);
            return 0;
        }

        result = ngiStringListDestruct(readingState->nrs_curISinfoOptions);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the StringList.\n", fName);
            return 0;
        }
        readingState->nrs_curISinfoOptions = NULL;
    }

    /* Move to curISinfo to isInfos */
    isInfoPair = readingState->nrs_curISinfo;
    readingState->nrs_curISinfo = NULL;

    result = ngcllInvokeServerInfoPairRegister(
        &(readingState->nrs_isInfos), isInfoPair);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register Invoke Server Info.\n", fName);
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
             readingState->nrs_curISinfo->nisip_entities, \
             readingState->nrs_curISinfo->nisip_isSet)

#define NGCLL_AFI_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_curISinfo->nisip_entities, \
             readingState->nrs_curISinfo->nisip_isSet)

#define NGCLL_AFI_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGCLL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->nrs_curISinfo->nisip_entities, \
             readingState->nrs_curISinfo->nisip_isSet, unitTable)


static int
ngcllAttrFuncInvokeServer_type       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncInvokeServer_type";
    int result, isRegistered;
    char *typeName;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFI_MEMBER_SET_STR(ngisi_type, token->nti_tokenStr)

    log = context->ngc_log;

    typeName = readingState->nrs_curISinfo->nisip_entities->ngisi_type;
    assert(typeName != NULL);

    /* type name registered check */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedInvokeServers, typeName,
        NGI_ATTR_NAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
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
    optString = ngiReadQuotedStringFromArg(token->nti_tokenStr, 0, NULL, 0);
    if (optString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curISinfoOptions), optString);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the string.\n", fName);
        return 0;
    }
    globus_libc_free(optString);

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage to "
            "register Remote Machine Information.\n", fName);
        return 0;
    }

    result = ngcllRemoteMachineInfoPairInitialize(context,
        readingState->nrs_curRmInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize Remote Machine Info Pair.\n", fName);
        return 0;
    }

    assert(readingState->nrs_curRmInfoEps == NULL);
    assert(readingState->nrs_curRmInfoHosts == NULL);
    assert(readingState->nrs_curRmInfoInvokeServerOptions == NULL);
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
    ngcllExecutablePathInfoPair_t *curEp, *newEp;
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
        /* Check if the hostname is 1 or not */
        result = ngiStringListCount(
            readingState->nrs_curRmInfoHosts, &count);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't count the StringList.\n", fName);
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
            NGI_HOSTNAME_CASE_SENSITIVE);
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set InvokeServerOptions to "
                "RemoteMachineInfoPair.\n",
                fName);
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoInvokeServerOptions);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the StringList.\n", fName);
            return 0;
        }
        readingState->nrs_curRmInfoInvokeServerOptions = NULL;
    }

    /* Set RSL Extensions to curRmInfo */
    if (readingState->nrs_curRmInfoRSLextensions != NULL) {
        result = ngcllRemoteMachineInfoPairSetRSLextensions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoRSLextensions, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set RSL Extensions to "
                "RemoteMachineInfoPair.\n",
                fName);
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoRSLextensions);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the StringList.\n", fName);
            return 0;
        }
        readingState->nrs_curRmInfoRSLextensions = NULL;
    }

    /* Set environ to curRmInfo */
    if (readingState->nrs_curRmInfoEnviron != NULL) {
        result = ngcllRemoteMachineInfoPairSetEnviron(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoEnviron, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set environ to Remote Machine Info Pair.\n",
                fName);
            return 0;
        }

        result = ngiStringListDestruct(readingState->nrs_curRmInfoEnviron);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the StringList.\n", fName);
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
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate Remote Machine Info Pair.\n", fName);
            return 0;
        }

        assert(host_cur->nsl_string != NULL);
        hostName = strdup(host_cur->nsl_string);
        if (hostName == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string.\n", fName);
            return 0;
        }

        assert(newPair->nrmip_entities->ngrmi_hostName == NULL);

        newPair->nrmip_entities->ngrmi_hostName = hostName;
        newPair->nrmip_isSet->ngrmi_hostName = NGCL_ISSET_S_TRUE;

        result = ngcllRemoteMachineInfoPairRegister(
            &(readingState->nrs_rmInfos), newPair);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register Remote Machine Info.\n", fName);
            return 0;
        }

        /* Register curRmInfoEps to rmInfoEps */
        curEp = readingState->nrs_curRmInfoEps;
        while (curEp != NULL) {

            newEp = ngcllExecutablePathInfoPairDuplicate(
                context, curEp, error);
            if (newEp == NULL) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate Executable PathInfo Pair.\n", fName);
                return 0;
            }

            assert(host_cur->nsl_string != NULL);
            hostName = strdup(host_cur->nsl_string);
            if (hostName == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string.\n", fName);
                return 0;
            }
            globus_libc_free(
                newEp->nepip_entities->ngepi_hostName); /* free dummy */
            newEp->nepip_entities->ngepi_hostName = hostName;
            newEp->nepip_isSet->ngepi_hostName = NGCL_ISSET_S_TRUE;

            if (tagName != NULL) {
                newEp->nepip_tagName = strdup(tagName);
                if (newEp->nepip_tagName == NULL) {
                    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't duplicate the string.\n", fName);
                    return 0;
                }
            }

            result = ngcllExecutablePathInfoPairRegister(
                &(readingState->nrs_rmInfoEps), newEp);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't register Executable PathInfo.\n", fName);
                return 0;
            }

            curEp = curEp->nepip_next;
        }

        host_cur = host_cur->nsl_next;
    }

    result = ngcllRemoteMachineInfoPairDestruct(context,
        readingState->nrs_curRmInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't destruct the Remote Machine Info Pair.\n", fName);
        return 0;
    }

    if (readingState->nrs_curRmInfoEps != NULL) {
        result = ngcllExecutablePathInfoPairListDestruct(context,
            readingState->nrs_curRmInfoEps, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Executable Path Info Pair.\n", fName);
            return 0;
        }
    }

    /* Register No-Tag host names */
    if (tagSet == NULL) {
        assert(readingState->nrs_curRmInfoHosts != NULL);

        newHosts = ngiStringListDuplicate(
            readingState->nrs_curRmInfoHosts);
        if (newHosts == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't duplicate the StringList.\n", fName);
            return 0;
        }

        result = ngiStringListRegisterList(
            &(readingState->nrs_appearedNoTagRmInfoHosts), newHosts);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the StringList.\n", fName);
            return 0;
        }
    }

    /* Register host names */
    result = ngiStringListRegisterList(
        &(readingState->nrs_appearedRmInfoHosts),
        readingState->nrs_curRmInfoHosts);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the StringList.\n", fName);
        return 0;
    }

    readingState->nrs_curRmInfoHosts = NULL;
    readingState->nrs_curRmInfoEps = NULL;
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
    hostNames = ngiReadStringListFromArg(token->nti_tokenStr);
    if (hostNames == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal host name list:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* Check host name already defined (current section) */
    foundHost = ngiStringListCheckListIncludeSameString(hostNames,
        readingState->nrs_curRmInfoHosts, NGI_HOSTNAME_CASE_SENSITIVE);
    if (foundHost != NULL) {
        ngiConfigFileSyntaxError(log, token,
            "server information already registered for:",
            fName, attrName, foundHost->nsl_string, error);
        return 0;
    }
    foundHost = NULL;

    /* Register host names */
    result = ngiStringListRegisterList(
        &(readingState->nrs_curRmInfoHosts), hostNames);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the StringList.\n", fName);
        return 0;
    }

    readingState->nrs_curRmInfo->nrmip_isSet->ngrmi_hostName =
        NGCL_ISSET_S_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_tag              NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_tag";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    int result, isRegistered;
    char *tagName, *p;
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

    /* Tag name validity check */
    p = tagName;
    while (*p != '\0') {
        /* Just ensure the Resource Manager Contact splitable. */
        if ((*p == ':') || (*p == '/')) {
            ngiConfigFileSyntaxError(log, token,
                "Tag name includes invalid character:",
                fName, NULL, tagName, error);
                return 0;
        }

        p++;
    }
    p = NULL;

    /* Tag name registered check */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedRmInfoTags, tagName,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't check the StringList.\n", fName);
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
        &(readingState->nrs_appearedRmInfoTags), tagName);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register StringList.\n", fName);
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
ngcllAttrFuncServer_mds_hostname     NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_mds_hostname";
    int result, isRegistered;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_mdsServer, token->nti_tokenStr)

    log = context->ngc_log;

    /* Check mds_tag */
    if (readingState->nrs_curRmInfo->nrmip_isSet->ngrmi_mdsTag != NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "mutual mds_tag already defined ",
            fName, attrName, NULL, error);
        return 0;
    }

    /* Register to appearedMDSserversInRm */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedMDSserversInRm,
        readingState->nrs_curRmInfo->nrmip_entities->ngrmi_mdsServer,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }
    if (!isRegistered) {
        result = ngiStringListRegister(
            &(readingState->nrs_appearedMDSserversInRm),
            readingState->nrs_curRmInfo->nrmip_entities->ngrmi_mdsServer);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register StringList.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_mds_tag          NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_mds_tag";
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_mdsTag, token->nti_tokenStr)

    log = context->ngc_log;

    /* Check mds_hostname */
    if (readingState->nrs_curRmInfo->nrmip_isSet->ngrmi_mdsServer != NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "mutual mds_hostname already defined ",
            fName, attrName, NULL, error);
        return 0;
    }

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

    /* "none" is for GT2 GRAM (Do not use Invoke Server), Type is NULL */
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
            NGI_ATTR_NAME_CASE_SENSITIVE, &isRegistered);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't check the StringList.\n", fName);
            return 0;
        }
        if (!isRegistered) {
            result = ngiStringListRegister(
                &(readingState->nrs_appearedInvokeServersInRm), type);
            if (result != 1) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't register StringList.\n", fName);
                return 0;
            }
        }

#ifndef NG_PTHREAD
        /* NonThread version of Invoke Server is not supported. */

        /* Set back to NULL. */
        free(type);
        rmInfoPair->nrmip_entities->ngrmi_invokeServerType = NULL;

        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: invoke_server is not supported for"
            " this GlobusToolkit flavor.\n", fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: invoke_server is supported only for pthread version.\n",
            fName);

        return 0;
#endif /* NG_PTHREAD */
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
    optString = ngiReadQuotedStringFromArg(token->nti_tokenStr, 0, NULL, 0);
    if (optString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get string from ",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curRmInfoInvokeServerOptions), optString);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the string.\n", fName);
        return 0;
    }
    globus_libc_free(optString);

    /* Success */
    return 1;
}

static int 
ngcllAttrFuncServer_mpi_runCommand   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_mpi_runCommand";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    ngLogPrintf(context->ngc_log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s:%s:line %ld: "
        "Obsolete syntax \"%s\". Ignoring this setting, continue.\n", fName,
        token->nti_readInfo->ntri_filename,
        token->nti_readInfo->ntri_lineno, attrName);

    return 1;
}

static int 
ngcllAttrFuncServer_mpi_runCPUs      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_mpi_runCPUs";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    ngcllExecutablePathInfoPair_t *epInfoPair;
    char *className, *hostName;
    int result, cpus;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    result = ngiReadStrEqualNumberFromArg(token->nti_tokenStr,
         &className, &cpus);
    if (result != 1) {
        ngiConfigFileSyntaxError(log, token,
            "Unable to get mpi_runNoOfCPUs",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }
    if (cpus < 1) {
        ngiConfigFileSyntaxError(log, token,
            "attribute value invalid (too small):",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* If no name specified, it means to set rmInfo's all mpiNcpus */
    if (className == NULL) {
        if (rmInfoPair->nrmip_isSet->ngrmi_mpiNcpus != 0) {
            ngiConfigFileSyntaxError(log, token,
                "attribute redefined", fName, attrName, NULL, error);
            return 0;
        }

        rmInfoPair->nrmip_entities->ngrmi_mpiNcpus = cpus;
        rmInfoPair->nrmip_isSet->ngrmi_mpiNcpus = NGCL_ISSET_I_TRUE;

        /* Success */
        return 1;
    }

    /* else, class specified mpiNcpus */

    if (readingState->nrs_readingServerDefault) {
        ngiConfigFileSyntaxError(log, token,
            "not allowed attribute in this section",
             fName, attrName, NULL, error);
        return 0;
    }

    /* duplication check */
    epInfoPair = ngcllExecutablePathInfoPairGet(
        readingState->nrs_curRmInfoEps,
        NGCLL_CONFIG_DUMMY_HOSTNAME, className);
    if (epInfoPair != NULL) {
        ngiConfigFileSyntaxError(log, token,
            "mpi_runNoOfCPUs redefined for function:",
            fName, attrName, className, error);
        return 0;
    }

    /**
     * Set dummy hostname to success RemoteExecutableInformationCopy()
     * True name is set when ServerEnd()
     */
    hostName = strdup(NGCLL_CONFIG_DUMMY_HOSTNAME);
    if (hostName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string.\n", fName);
        return 0;
    }

    epInfoPair = ngcllExecutablePathInfoPairCreate(context, error);
    if (epInfoPair == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage to "
            "register Executable Path Information.\n",
             fName);
        return 0;
    }

    result = ngcllExecutablePathInfoPairInitialize(
        context, epInfoPair, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize Executable Path Info Pair.\n", fName);
        return 0;
    }

    epInfoPair->nepip_entities->ngepi_hostName = hostName;
    epInfoPair->nepip_entities->ngepi_className = className;
    epInfoPair->nepip_entities->ngepi_mpiNcpus = cpus;
    epInfoPair->nepip_isSet->ngepi_hostName = NGCL_ISSET_S_TRUE;
    epInfoPair->nepip_isSet->ngepi_className = NGCL_ISSET_S_TRUE;
    epInfoPair->nepip_isSet->ngepi_mpiNcpus = NGCL_ISSET_I_TRUE;

    result = ngcllExecutablePathInfoPairRegister(
        &(readingState->nrs_curRmInfoEps), epInfoPair);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register Executable Path Info Pair.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_gass_scheme      NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_gass_scheme";

    NGCLL_CONFIG_ATTRFUNC_ASSERTS
    NGCLL_AFS_MEMBER_SET_STR(ngrmi_gassScheme, token->nti_tokenStr)

    return 1;
}

static int
ngcllAttrFuncServer_crypt            NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_crypt";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    char *cryptTypeStr;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    if (rmInfoPair->nrmip_isSet->ngrmi_crypt != 0) {
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0;
    }

    /* Get argument string */
    cryptTypeStr = ngiReadStringFromArg(token->nti_tokenStr);
    if (cryptTypeStr == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal crypt type [false/authonly/SSL/GSI]:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* Set argument enum */
    result = ngiReadEnumFromArg(
        cryptTypeStr, NGI_ATTR_ARG_CASE_SENSITIVE,
        4, "false", "authonly", "GSI", "SSL");
    if (result == 0) {
        ngiConfigFileSyntaxError(log, token,
            "No such crypt [false/authonly/SSL/GSI]:",
            fName, attrName, cryptTypeStr, error);
        globus_libc_free(cryptTypeStr);
        return 0;
    }
    globus_libc_free(cryptTypeStr);

    if (result == 1) {
        rmInfoPair->nrmip_entities->ngrmi_crypt = NG_PROTOCOL_CRYPT_NONE;
    } else if (result == 2) {
        rmInfoPair->nrmip_entities->ngrmi_crypt = NG_PROTOCOL_CRYPT_AUTHONLY;
    } else if (result == 3) {
        rmInfoPair->nrmip_entities->ngrmi_crypt = NG_PROTOCOL_CRYPT_GSI;
    } else if (result == 4) {
        rmInfoPair->nrmip_entities->ngrmi_crypt = NG_PROTOCOL_CRYPT_SSL;
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: reading crypt string fail. result is %d\n",
            fName, result);
        return 0;
    }
    rmInfoPair->nrmip_isSet->ngrmi_crypt =
        (ngProtocolCrypt_t) NGCL_ISSET_I_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_protocol         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_protocol";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    char *protocolType;
    ngLog_t *log;
    int result;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    if (rmInfoPair->nrmip_isSet->ngrmi_protocol != 0) {
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0;
    }

    /* Get argument string */
    protocolType = ngiReadStringFromArg(token->nti_tokenStr);
    if (protocolType == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "illegal protocol [XML/binary]:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    /* Set argument enum */
    result = ngiReadEnumFromArg(
        protocolType, NGI_ATTR_ARG_CASE_SENSITIVE,
        2, "XML", "binary");
    if (result == 0) {
        ngiConfigFileSyntaxError(log, token,
            "No such protocol [XML/binary]:",
            fName, attrName, protocolType, error);
        globus_libc_free(protocolType);
        return 0;
    }
    globus_libc_free(protocolType);

    if (result == 1) {
        ngiConfigFileSyntaxError(log, token,
            "Protocol type XML is not supported yet",
            fName, attrName, NULL, error);
        return 0;
    } else if (result == 2) {
        rmInfoPair->nrmip_entities->ngrmi_protocol = NG_PROTOCOL_TYPE_BINARY;
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: reading protocol string fail. result is %d\n",
            fName, result);
        return 0;
    }
    rmInfoPair->nrmip_isSet->ngrmi_protocol =
        (ngProtocolType_t) NGCL_ISSET_I_TRUE;

    /* Success */
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
            token->nti_tokenStr, 1, &valueContinue, continuedLine);
        if (extString == NULL) {
            ngiConfigFileSyntaxError(log, token, 
                "Unable to get string from ",
                fName, attrName, token->nti_tokenStr, error);
            return 0;
        }

        result = ngiStringListRegister(
            &(readingState->nrs_curRmInfoRSLextensions), extString);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the string.\n", fName);
            return 0;
        }
        globus_libc_free(extString);
        extString = NULL;

        continuedLine = 1;
    }

    rmInfoPair->nrmip_isSet->ngrmi_rslExtensionsSize = NGCL_ISSET_I_TRUE;

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_heartbeat        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_heartbeat";
    ngcllRemoteMachineInfoPair_t *rmInfoPair;
    ngLog_t *log;

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_curRmInfo;

    NGCLL_AFS_MEMBER_SET_UNIT(
        ngrmi_heartBeat, 0, token->nti_tokenStr, ngiTimeUnitTable)

    if (rmInfoPair->nrmip_entities->ngrmi_heartBeat == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: disabling heartbeat for the server.\n", fName);
    } else {
#ifndef NG_PTHREAD
        /* Disabling heartbeat */
        rmInfoPair->nrmip_entities->ngrmi_heartBeat = 0;
     
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: heartbeat not supported for this GlobusToolkit flavor.\n",
            fName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: heartbeat is supported only for pthread version.\n", fName);
#endif /* NG_PTHREAD */
    }

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_heartbeatCount   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_heartbeatCount";
#ifndef NG_PTHREAD
    ngLog_t *log;
#endif /* NG_PTHREAD */

    NGCLL_CONFIG_ATTRFUNC_ASSERTS


    NGCLL_AFS_MEMBER_SET_INT(
        ngrmi_heartBeatTimeoutCount, 1, token->nti_tokenStr)

#ifndef NG_PTHREAD
    log = context->ngc_log;

    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: heartbeat/heartbeat_timeoutCount is not supported for"
        " this GlobusToolkit flavor.\n", fName);
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: heartbeat is supported only for pthread version.\n", fName);
#endif /* NG_PTHREAD */

    /* Success */
    return 1;
}

static int
ngcllAttrFuncServer_heartbeatTrans   NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllAttrFuncServer_heartbeatTrans";
#ifndef NG_PTHREAD
    ngLog_t *log;
#endif /* NG_PTHREAD */

    NGCLL_CONFIG_ATTRFUNC_ASSERTS

    NGCLL_AFS_MEMBER_SET_INT(
        ngrmi_heartBeatTimeoutCountOnTransfer, 0, token->nti_tokenStr)

#ifndef NG_PTHREAD
    log = context->ngc_log;

    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: heartbeat_timeoutCountOnTransfer is not supported for"
        " this GlobusToolkit flavor.\n", fName);
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: heartbeat is supported only for pthread version.\n", fName);
#endif /* NG_PTHREAD */

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

    ngLogPrintf(context->ngc_log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s:%s:line %ld: "
        "Ignore \"%s\" attribute in <SERVER> section, it is obsolete attribute."
        "Please use tcp_nodelay attribute in <CLIENT> section.\n",
        fName,
        token->nti_readInfo->ntri_filename,
        token->nti_readInfo->ntri_lineno, attrName,
        token->nti_tokenStr);

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
    argTransType = ngiReadStringFromArg(token->nti_tokenStr);
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
        globus_libc_free(argTransType);
        return 0;
    }
    globus_libc_free(argTransType);

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: reading argument transfer string fail. result is %d\n",
            fName, result);
        return 0;
    }
    rmInfoPair->nrmip_isSet->ngrmi_argumentTransfer =
        (ngArgumentTransfer_t) NGCL_ISSET_I_TRUE;

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
    compressType = ngiReadStringFromArg(token->nti_tokenStr);
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
        globus_libc_free(compressType);
        return 0;
    }
    globus_libc_free(compressType);

    if (result == 1) {
        rmInfoPair->nrmip_entities->ngrmi_compressionType =
            NG_COMPRESSION_TYPE_RAW;
    } else if (result == 2) {

#ifndef NGI_NO_ZLIB
        rmInfoPair->nrmip_entities->ngrmi_compressionType =
            NG_COMPRESSION_TYPE_ZLIB;

#else /* NGI_NO_ZLIB */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: %s: zlib compression is not available for this client.\n",
            fName, attrName);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G configure was determined as no-zlib for this host.\n",
            fName);

        rmInfoPair->nrmip_entities->ngrmi_compressionType =
            NG_COMPRESSION_TYPE_RAW;

#endif /* NGI_NO_ZLIB */

    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: reading compress string fail. result is %d\n",
            fName, result);
        return 0;
    }
    rmInfoPair->nrmip_isSet->ngrmi_compressionType =
        (ngCompressionType_t) NGCL_ISSET_I_TRUE;

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

#ifdef NGI_NO_ZLIB
    ngLogPrintf(context->ngc_log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: %s: zlib compression is not available for this client.\n",
        fName, "compress_threshold");
    ngLogPrintf(context->ngc_log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: %s: thus, threshold has no effect.\n",
        fName);

#endif /* NGI_NO_ZLIB */

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
        ngrmi_commLogInfo.ngli_enable, token->nti_tokenStr)

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

    envString = ngiReadEnvStringFromArg(token->nti_tokenStr);
    if (envString == NULL) {
        ngiConfigFileSyntaxError(log, token, 
            "Unable to get environment from:",
            fName, attrName, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_curRmInfoEnviron), envString);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the string.\n", fName);
        return 0;
    }
    globus_libc_free(envString);

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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to execute attribute function.\n", fName);
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set InvokeServerOptions to "
                "RemoteMachineInformation.\n",
                fName);
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoInvokeServerOptions);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct StringList.\n", fName);
            return 0;
        }
        readingState->nrs_curRmInfoInvokeServerOptions = NULL;
    }

    /* Treat RSL Extensions */
    if (readingState->nrs_curRmInfoRSLextensions != NULL) {
        result = ngcllRemoteMachineInfoPairSetRSLextensions(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoRSLextensions, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set RSL Extensions to "
                "RemoteMachineInformation.\n",
                fName);
            return 0;
        }

        result = ngiStringListDestruct(
            readingState->nrs_curRmInfoRSLextensions);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct StringList.\n", fName);
            return 0;
        }
        readingState->nrs_curRmInfoRSLextensions = NULL;
    }

    /* Treat environment variable */
    if (readingState->nrs_curRmInfoEnviron != NULL) {
        result = ngcllRemoteMachineInfoPairSetEnviron(
            context, readingState->nrs_curRmInfo,
            readingState->nrs_curRmInfoEnviron, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set environ to Remote Machine Information.\n",
                fName);
            return 0;
        }

        result = ngiStringListDestruct(readingState->nrs_curRmInfoEnviron);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct StringList.\n", fName);
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

    /* making : funcname change Create -> Allocate */

    log = context->ngc_log;

    ret = (ngcllLocalMachineInfoPair_t *)
              globus_libc_malloc(sizeof(ngcllLocalMachineInfoPair_t));
    if (ret == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Local Machine Info Pair.\n", fName);
        return NULL;
    }

    ret->nlmip_entities = ngcliLocalMachineInformationAllocate(
        context, error);
    if (ret->nlmip_entities == NULL) {
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Local Machine Information.\n", fName);
        return NULL;
    }

    ret->nlmip_isSet = ngcliLocalMachineInformationAllocate(
        context, error);
    if (ret->nlmip_isSet == NULL) {
        globus_libc_free(ret->nlmip_entities);
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Local Machine Information.\n", fName);
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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngclLocalMachineInformationRelease(context,
        lmInfoPair->nlmip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Local Machine Info Pair.\n", fName);
        return 0;
    }

    /* nlmip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliLocalMachineInformationFree(context,
        lmInfoPair->nlmip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Local Machine Information.\n", fName);
        return 0;
    }

    result = ngcliLocalMachineInformationFree(context,
        lmInfoPair->nlmip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Local Machine Information.\n", fName);
        return 0;
    }

    globus_libc_free(lmInfoPair);

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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Local Machine Information.\n", fName);
        return 0;
    }
    result = ngcllLocalMachineInformationSetDefault(
        lmInfoPair->nlmip_entities, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Local Machine Information to default.\n",
            fName);
        return 0;
    }

    result = ngcliLocalMachineInformationInitialize(context,
        lmInfoPair->nlmip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Local Machine Information.\n",
            fName);
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
                globus_libc_free(dst->member); \
            } \
            if ((entity)->member != NULL) { \
                dst->member = strdup((entity)->member); \
                if (dst->member == NULL) { \
                    NGI_SET_ERROR(error, NG_ERROR_MEMORY); \
                    ngLogPrintf(log, \
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, \
                        "%s: Can't allocate the storage for string.\n", \
                         fName); \
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

/**
 * This function sets dst from src member which isSet flag is true.
 */
int
ngcllLocalMachineInformationSetPair(
    ngclLocalMachineInformation_t *dst,
    ngcllLocalMachineInfoPair_t *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllLocalMachineInformationSetPair";

    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_hostName)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_saveNsessions)

    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_enable)

    if (src->nlmip_isSet->nglmi_logInfo.ngli_level != 0) {
        dst->nglmi_logInfo.ngli_level =
            dst->nglmi_logInfo.ngli_levelGlobusToolkit =
            dst->nglmi_logInfo.ngli_levelNinfgProtocol =
            dst->nglmi_logInfo.ngli_levelNinfgInternal =
            dst->nglmi_logInfo.ngli_levelGrpc =
            src->nlmip_entities->nglmi_logInfo.ngli_level;
    }

    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_levelGlobusToolkit)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_levelNinfgProtocol)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_levelNinfgInternal)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_levelGrpc)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_logInfo.ngli_filePath)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_logInfo.ngli_suffix)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_nFiles)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_maxFileSize)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_overWriteDir)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_tmpDir)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_refreshInterval)
    NGCLL_LMISP_MEMBER_SET_STR(dst, src, nglmi_invokeServerLog)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_fortranCompatible)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_listenPort)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_listenPortAuthOnly)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_listenPortGSI)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_listenPortSSL)
    NGCLL_LMISP_MEMBER_SET_INT(dst, src, nglmi_tcpNodelay)

    /* Success */
    return 1;
}

#undef NGCLL_LMISP_MEMBER_SET_STR
#undef NGCLL_LMISP_MEMBER_SET_INT

/**
 * MDSserverInfoPair
 */
ngcllMDSserverInfoPair_t *
ngcllMDSserverInfoPairCreate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInfoPairCreate";
    ngcllMDSserverInfoPair_t *ret;
    ngLog_t *log;

    /* making : funcname change Create -> Allocate */

    log = context->ngc_log;

    ret = (ngcllMDSserverInfoPair_t *)
              globus_libc_malloc(sizeof(ngcllMDSserverInfoPair_t));
    if (ret == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the MDS Server Info Pair.\n", fName);
        return NULL;
    }

    ret->nmsip_entities = ngcliMDSserverInformationAllocate(
        context, error);
    if (ret->nmsip_entities == NULL) {
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the MDS Server Information.\n", fName);
        return NULL;
    }

    ret->nmsip_isSet = ngcliMDSserverInformationAllocate(
        context, error);
    if (ret->nmsip_isSet == NULL) {
        globus_libc_free(ret->nmsip_entities);
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the MDS Server Information.\n", fName);
        return NULL;
    }

    /* Success */
    return ret;
}

int
ngcllMDSserverInfoPairDestruct(
    ngclContext_t *context,
    ngcllMDSserverInfoPair_t *mdsInfoPair,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInfoPairDestruct";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (mdsInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngclMDSserverInformationRelease(context,
        mdsInfoPair->nmsip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the storage to "
            "register MDS Server Information.\n", fName);
        return 0;
    }

    /* nmsip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliMDSserverInformationFree(context,
        mdsInfoPair->nmsip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the MDS Server Information.\n", fName);
        return 0;
    }

    result = ngcliMDSserverInformationFree(context,
        mdsInfoPair->nmsip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the MDS Server Information.\n", fName);
        return 0;
    }

    globus_libc_free(mdsInfoPair);

    /* Success */
    return 1;
}

int
ngcllMDSserverInfoPairListDestruct(
    ngclContext_t *context,
    ngcllMDSserverInfoPair_t *mdsInfoPair,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInfoPairListDestruct";
    ngcllMDSserverInfoPair_t *curMds, *nextMds;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    if (mdsInfoPair == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    curMds = mdsInfoPair;
    do {
        nextMds = curMds->nmsip_next;
        result = ngcllMDSserverInfoPairDestruct(context, curMds, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to destruct MDSserverInfoPair.\n",
                fName);
            return 0;
        }

        curMds = nextMds;
    } while (curMds != NULL);

    /* Success */
    return 1;
}

int
ngcllMDSserverInfoPairInitialize(
    ngclContext_t *context,
    ngcllMDSserverInfoPair_t *mdsInfoPair,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInfoPairInitialize";
    ngLog_t *log;
    int result;

    assert(mdsInfoPair != NULL);
    assert(mdsInfoPair->nmsip_entities != NULL);
    assert(mdsInfoPair->nmsip_isSet != NULL);

    log = context->ngc_log;

    mdsInfoPair->nmsip_next = NULL;

    result = ngcliMDSserverInformationInitialize(context,
        mdsInfoPair->nmsip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the MDS Server Information.\n", fName);
        return 0;
    }
    result = ngcllMDSserverInformationSetDefault(
        mdsInfoPair->nmsip_entities, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the MDS Server Information to default.\n",
            fName);
        return 0;
    }

    result = ngcliMDSserverInformationInitialize(context,
        mdsInfoPair->nmsip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the MDS Server Information.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllMDSserverInfoPairRegister(
    ngcllMDSserverInfoPair_t **dst,
    ngcllMDSserverInfoPair_t *src
)
{
    ngcllMDSserverInfoPair_t *cur;

    /* making : add argument for context and log */

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    src->nmsip_next = NULL;

    if (*dst == NULL) {
        *dst = src;
        return 1;
    }

    cur = *dst;
    while (cur->nmsip_next != NULL) {
        cur = cur->nmsip_next;
    }

    cur->nmsip_next = src;

    /* Success */
    return 1;
}


/**
 * This macro sets dst->member if src->isSet->member flag is on.
 * argument member should be the member variable
 * of ngclMDSserverInformation_t.
 * RMISP means MDSserverInformationSetPair.
 */

#define NGCLL_MSISP_MEMBER_SET_STR(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_STR( \
            dst, (src)->nmsip_entities, (src)->nmsip_isSet, member)

#define NGCLL_MSISP_MEMBER_SET_INT(dst, src, member) \
        NGCLL_ISP_MEMBER_SET_INT( \
            dst, (src)->nmsip_entities, (src)->nmsip_isSet, member)

/**
 * This function sets dst from src member which isSet flag is true.
 */
int
ngcllMDSserverInformationSetPair(
    ngclMDSserverInformation_t *dst,
    ngcllMDSserverInfoPair_t *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInformationSetPair";

    NGCLL_MSISP_MEMBER_SET_STR(dst, src, ngmsi_hostName)
    NGCLL_MSISP_MEMBER_SET_STR(dst, src, ngmsi_tagName)
    NGCLL_MSISP_MEMBER_SET_INT(dst, src, ngmsi_type)
    NGCLL_MSISP_MEMBER_SET_INT(dst, src, ngmsi_portNo)
    NGCLL_MSISP_MEMBER_SET_STR(dst, src, ngmsi_protocol)
    NGCLL_MSISP_MEMBER_SET_STR(dst, src, ngmsi_path)
    NGCLL_MSISP_MEMBER_SET_STR(dst, src, ngmsi_subject)
    NGCLL_MSISP_MEMBER_SET_STR(dst, src, ngmsi_voName)
    NGCLL_MSISP_MEMBER_SET_INT(dst, src, ngmsi_clientTimeout)
    NGCLL_MSISP_MEMBER_SET_INT(dst, src, ngmsi_serverTimeout)

    /* Success */
    return 1;
}

#undef NGCLL_MSISP_MEMBER_SET_STR
#undef NGCLL_MSISP_MEMBER_SET_INT

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

    /* making : funcname change Create -> Allocate */

    log = context->ngc_log;

    ret = (ngcllInvokeServerInfoPair_t *)
              globus_libc_malloc(sizeof(ngcllInvokeServerInfoPair_t));
    if (ret == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Invoke Server Info Pair.\n", fName);
        return NULL;
    }

    ret->nisip_entities = ngcliInvokeServerInformationAllocate(
        context, error);
    if (ret->nisip_entities == NULL) {
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Invoke Server Information.\n", fName);
        return NULL;
    }

    ret->nisip_isSet = ngcliInvokeServerInformationAllocate(
        context, error);
    if (ret->nisip_isSet == NULL) {
        globus_libc_free(ret->nisip_entities);
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Invoke Server Information.\n", fName);
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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngclInvokeServerInformationRelease(context,
        isInfoPair->nisip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the storage to "
            "register Invoke Server Information.\n", fName);
        return 0;
    }

    /* nisip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliInvokeServerInformationFree(context,
        isInfoPair->nisip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Invoke Server Information.\n", fName);
        return 0;
    }

    result = ngcliInvokeServerInformationFree(context,
        isInfoPair->nisip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Invoke Server Information.\n", fName);
        return 0;
    }

    globus_libc_free(isInfoPair);

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    curIs = isInfoPair;
    do {
        nextIs = curIs->nisip_next;
        result = ngcllInvokeServerInfoPairDestruct(context, curIs, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to destruct InvokeServerInfoPair.\n",
                fName);
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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Invoke Server Information.\n", fName);
        return 0;
    }
    result = ngcllInvokeServerInformationSetDefault(
        isInfoPair->nisip_entities, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Invoke Server Information to default.\n",
            fName);
        return 0;
    }

    result = ngcliInvokeServerInformationInitialize(context,
        isInfoPair->nisip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Invoke Server Information.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

int
ngcllInvokeServerInfoPairRegister(
    ngcllInvokeServerInfoPair_t **dst,
    ngcllInvokeServerInfoPair_t *src
)
{
    ngcllInvokeServerInfoPair_t *cur;

    /* making : add argument for context and log */

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
 * RMISP means InvokeServerInformationSetPair.
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't add StringArray.\n", fName);
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
        &optArray, &optSize, options);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't convert StringList to StringArray.\n", fName);
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    isInfoPair->nisip_entities->ngisi_nOptions = optSize;
    isInfoPair->nisip_entities->ngisi_options = optArray;

    isInfoPair->nisip_isSet->ngisi_nOptions = NGCL_ISSET_I_TRUE;
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

    /* making : funcname change Create -> Allocate */

    log = context->ngc_log;

    ret = (ngcllRemoteMachineInfoPair_t *)
              globus_libc_malloc(sizeof(ngcllRemoteMachineInfoPair_t));
    if (ret == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Remote Machine Info Pair.\n", fName);
        return NULL;
    }

    ret->nrmip_entities = ngcliRemoteMachineInformationAllocate(
        context, error);
    if (ret->nrmip_entities == NULL) {
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Remote Machine Information.\n", fName);
        return NULL;
    }

    ret->nrmip_isSet = ngcliRemoteMachineInformationAllocate(
        context, error);
    if (ret->nrmip_isSet == NULL) {
        globus_libc_free(ret->nrmip_entities);
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Remote Machine Information.\n", fName);
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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngclRemoteMachineInformationRelease(context,
        rmInfoPair->nrmip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the storage to "
            "register Remote Machine Information.\n", fName);
        return 0;
    }

    /* nrmip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    result = ngcliRemoteMachineInformationFree(context,
        rmInfoPair->nrmip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Remote Machine Information.\n", fName);
        return 0;
    }

    result = ngcliRemoteMachineInformationFree(context,
        rmInfoPair->nrmip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Remote Machine Information.\n", fName);
        return 0;
    }

    globus_libc_free(rmInfoPair);

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    curRm = rmInfoPair;
    do {
        nextRm = curRm->nrmip_next;
        result = ngcllRemoteMachineInfoPairDestruct(context, curRm, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to destruct RemoteMachineInfoPair.\n",
                fName);
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
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Remote Machine Information.\n", fName);
        return 0;
    }

    result = ngcllRemoteMachineInformationSetDefault(
        rmInfoPair->nrmip_entities, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Remote Machine Information to default.\n",
            fName);
        return 0;
    }

    result = ngcliRemoteMachineInformationInitialize(context,
        rmInfoPair->nrmip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Remote Machine Information.\n", fName);
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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't create the Remote Machine Info Pair.\n", fName);
        return NULL;
    }

    result = ngcllRemoteMachineInfoPairInitialize(context, newPair, error);
    if (result != 1) {
        ngcllRemoteMachineInfoPairDestruct(context, newPair, error);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize Remote Machine Info Pair.\n", fName);
        return NULL;
    }

    result = ngclRemoteMachineInformationRelease(
        context, newPair->nrmip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Remote Machine Information.\n", fName);
        return NULL;
    }
    result = ngcliRemoteMachineInformationCopy(context,
        src->nrmip_entities, newPair->nrmip_entities, error);
    if (result != 1) {
        ngcllRemoteMachineInfoPairDestruct(context, newPair, error);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't copy the Remote Machine Information.\n", fName);
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
    ngcllRemoteMachineInfoPair_t *src
)
{
    ngcllRemoteMachineInfoPair_t *cur;

    /* making : add argument for context and log */

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

    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_hostName)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_tagName)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_portNo)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_mdsServer)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_mdsTag)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_invokeServerType)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_mpiNcpus)
    NGCLL_RMISP_MEMBER_SET_STR(dst, src, ngrmi_gassScheme)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_crypt)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_protocol)
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
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_heartBeatTimeoutCountOnTransfer)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_redirectEnable)
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
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commLogInfo.ngli_enable)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src, ngrmi_commLogInfo.ngli_level)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src,
                                    ngrmi_commLogInfo.ngli_levelGlobusToolkit)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src,
                                    ngrmi_commLogInfo.ngli_levelNinfgProtocol)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src,
                                    ngrmi_commLogInfo.ngli_levelNinfgInternal)
    NGCLL_RMISP_MEMBER_SET_INT(dst, src,
                                    ngrmi_commLogInfo.ngli_levelGrpc)
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

    /* Set ngrmi_invokeServerOptions (add to dst) */
    if (src->nrmip_isSet->ngrmi_invokeServerNoptions != 0) {
        assert(src->nrmip_isSet->ngrmi_invokeServerOptions != NULL);

        result = ngcllStringArrayAdd(
            &dst->ngrmi_invokeServerOptions,
            &dst->ngrmi_invokeServerNoptions,
            src->nrmip_entities->ngrmi_invokeServerOptions,
            src->nrmip_entities->ngrmi_invokeServerNoptions, log, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't add StringArray.\n", fName);
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't add StringArray.\n", fName);
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
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't add StringArray.\n", fName);
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

/**
 * This function returns Next item from current,
 *  if current reached end of first Pair list, then
 *   return next Pair list item, but if item has same hostname
 *   as one of first Pair list, then skip.
 * example:
 *  first Pair list   a b c d e
 *  next Pair list    a   c   e f g
 *  and current is pointing e, then return f.
 *   returning order is a b c d e f g NULL.
 */
ngcllRemoteMachineInfoPair_t *
ngcllRemoteMachineInfoPairGetNextInTwo(
    ngcllRemoteMachineInfoPair_t *rmInfoPair1,
    ngcllRemoteMachineInfoPair_t *rmInfoPair2,
    ngcllRemoteMachineInfoPair_t *current)
{
    ngcllRemoteMachineInfoPair_t *curRmInfo;
    ngcllRemoteMachineInfoPair_t *prevRmInfo;
    ngcllRemoteMachineInfoPair_t *foundRmInfo;
    ngcllRemoteMachineInfoPair_t *tailFind;

    if (current == NULL) {
        if (rmInfoPair1 != NULL) {
            return rmInfoPair1;
        } else {
            return rmInfoPair2;
        }
    }

    prevRmInfo = current;
    curRmInfo = current->nrmip_next;
    while (1) {
        /**
         * if curRmInfo reached tail of rmInfoPair1, then continue
         *   beginning of rmInfoPair2
         * else, return NULL to finish (reached rmInfoPair2 tail).
         */
        if (curRmInfo == NULL) {
            if (rmInfoPair1 == NULL) {
                /* not found */
                goto notFound;
            }

            /* find rmInfoPair1 list tail */
            tailFind = rmInfoPair1;
            while (tailFind->nrmip_next != NULL) {
                tailFind = tailFind->nrmip_next;
            }

            if (prevRmInfo == tailFind) {
                /* found curRmInfo was reached tail of rmInfoPair1 */

                if (rmInfoPair2 == NULL) {
                    /* not found */
                    goto notFound;
                } else {
                    curRmInfo = rmInfoPair2;
                }
            } else {
                /* found curRmInfo was reached tail of rmInfoPair2 */
                goto notFound;
            }
        }

        /* uniqueness check. was curRmInfo->hostName appeared before? */
        assert(curRmInfo != NULL);
        foundRmInfo = ngcllRemoteMachineInfoPairGetWithTag(rmInfoPair1,
            curRmInfo->nrmip_entities->ngrmi_hostName,
            curRmInfo->nrmip_entities->ngrmi_tagName);
        if (foundRmInfo == curRmInfo) {
            /* found unique next item */
            return curRmInfo;
        }
        if (foundRmInfo == NULL) {
            /* curRmInfo is in rmInfoPair2 */

            foundRmInfo = ngcllRemoteMachineInfoPairGetWithTag(rmInfoPair2,
                curRmInfo->nrmip_entities->ngrmi_hostName,
                curRmInfo->nrmip_entities->ngrmi_tagName);
            if (foundRmInfo == curRmInfo) {
                /* found unique next item */
                return curRmInfo;
            }
            assert(foundRmInfo != NULL);
        }

        /* curRmInfo was not unique, so try to check next item unique */
        prevRmInfo = curRmInfo;
        curRmInfo = curRmInfo->nrmip_next;
    }

notFound:
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
        &optArray, &optSize, options);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't convert StringList to StringArray.\n", fName);
        return 0;
    }
    assert(optSize > 0);
    assert(optArray != NULL);

    rmInfoPair->nrmip_entities->ngrmi_invokeServerNoptions = optSize;
    rmInfoPair->nrmip_entities->ngrmi_invokeServerOptions = optArray;

    rmInfoPair->nrmip_isSet->ngrmi_invokeServerNoptions = NGCL_ISSET_I_TRUE;
    rmInfoPair->nrmip_isSet->ngrmi_invokeServerOptions = NGCL_ISSET_A_TRUE;
    
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
        &extArray, &extSize, rslExtensions);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't convert StringList to StringArray.\n", fName);
        return 0;
    }
    assert(extSize > 0);
    assert(extArray != NULL);

    rmInfoPair->nrmip_entities->ngrmi_rslExtensionsSize = extSize;
    rmInfoPair->nrmip_entities->ngrmi_rslExtensions = extArray;

    rmInfoPair->nrmip_isSet->ngrmi_rslExtensionsSize = NGCL_ISSET_I_TRUE;
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
        &envArray, &envSize, environ);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't convert StringList to StringArray.\n", fName);
        return 0;
    }
    assert(envSize > 0);
    assert(envArray != NULL);

    rmInfoPair->nrmip_entities->ngrmi_nEnvironments = envSize;
    rmInfoPair->nrmip_entities->ngrmi_environment = envArray;

    rmInfoPair->nrmip_isSet->ngrmi_nEnvironments = NGCL_ISSET_I_TRUE;
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
              globus_libc_malloc(sizeof(ngcllExecutablePathInfoPair_t));
    if (ret == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Executable Path Info Pair.\n", fName);
        return NULL;
    }

    ret->nepip_entities = ngcliExecutablePathInformationAllocate(
        context, error);
    if (ret->nepip_entities == NULL) {
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Executable Path Information.\n", fName);
        return NULL;
    }

    ret->nepip_isSet = ngcliExecutablePathInformationAllocate(
        context, error);
    if (ret->nepip_isSet == NULL) {
        globus_libc_free(ret->nepip_entities);
        globus_libc_free(ret);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Executable Path Information.\n", fName);
        return NULL;
    }

    ret->nepip_tagName = NULL;

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngclExecutablePathInformationRelease(context,
        epInfoPair->nepip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't release the Executable Path Info Pair.\n", fName);
        return 0;
    }

    /* nepip_isSet includes static (non free()able) variable,
        should not free member, and no need to free member also */

    globus_libc_free(epInfoPair->nepip_entities);
    globus_libc_free(epInfoPair->nepip_isSet);

    if (epInfoPair->nepip_tagName != NULL) {
        globus_libc_free(epInfoPair->nepip_tagName);
    }

    globus_libc_free(epInfoPair);

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    curEp = epInfoPair;
    do {
        nextEp = curEp->nepip_next;
        result = ngcllExecutablePathInfoPairDestruct(context, curEp, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Failed to destruct ExecutablePathInfoPair.\n",
                fName);
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
    epInfoPair->nepip_tagName = NULL;

    result = ngcliExecutablePathInformationInitialize(context,
        epInfoPair->nepip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Executable Path Information.\n",
            fName);
        return 0;
    }
    result = ngcllExecutablePathInformationSetDefault(
        epInfoPair->nepip_entities, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Executable Path Information"
            " to default.\n", fName);
        return 0;
    }

    result = ngcliExecutablePathInformationInitialize(context,
        epInfoPair->nepip_isSet, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Executable Path Information.\n",
            fName);
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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the Executable Path Info Pair.\n", fName);
        return NULL;
    }

    result = ngcllExecutablePathInfoPairInitialize(context, newPair, error);
    if (result != 1) {
        ngcllExecutablePathInfoPairDestruct(context, newPair, error);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Executable Path Info Pair.\n", fName);
        return NULL;
    }

    result = ngclExecutablePathInformationRelease(
        context, newPair->nepip_entities, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Executable Path Info Pair.\n", fName);
        return NULL;
    }
    result = ngcliExecutablePathInformationCopy(context,
             src->nepip_entities, newPair->nepip_entities, error);
    if (result != 1) {
        ngcllExecutablePathInfoPairDestruct(context, newPair, error);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Executable Path Info Pair.\n", fName);
        return NULL;
    }

    /* make shallow copy (string member is not duplicated) */
    *(newPair->nepip_isSet) = *(src->nepip_isSet);

    if (src->nepip_tagName != NULL) {
        newPair->nepip_tagName = strdup(src->nepip_tagName);
        if (newPair->nepip_tagName == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string.\n", fName);
            return NULL;
        }
    }

    /* Success */
    return newPair;
}

int
ngcllExecutablePathInfoPairRegister(
    ngcllExecutablePathInfoPair_t **dst,
    ngcllExecutablePathInfoPair_t *src
)
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
    ngcliExecutablePathInformation_t *dst,
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
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_mpiNcpus)
    NGCLL_EPISP_MEMBER_SET_INT(dst, src, ngepi_sessionTimeout)

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
 * This function finds item which have same hostName, tagName, and className
 *    in member as argument.
 */
ngcllExecutablePathInfoPair_t *
ngcllExecutablePathInfoPairGetWithTag(
    ngcllExecutablePathInfoPair_t *epInfoPair,
    char *hostName,
    char *tagName,
    char *className)
{
    ngcllExecutablePathInfoPair_t *cur;
    int (*cmp)(const char *s1, const char *s2);

    cmp = (NGI_HOSTNAME_CASE_SENSITIVE ? strcmp : strcasecmp);
    assert(cmp != NULL);

    assert(hostName != NULL);
    assert(className != NULL);

    for (cur = epInfoPair; cur != NULL; cur = cur->nepip_next) {
        assert(cur->nepip_entities->ngepi_hostName != NULL);
        assert(cur->nepip_entities->ngepi_className != NULL);

        if ((*cmp)(hostName, cur->nepip_entities->ngepi_hostName) != 0) {
            continue;
        }

        if (tagName != NULL) {
            if (cur->nepip_tagName == NULL) {
                continue;
            }
            if (strcmp(tagName, cur->nepip_tagName) != 0) {
                continue;
            }
        } else {
            if (cur->nepip_tagName != NULL) {
                continue;
            }
        }

        if (strcmp(className, cur->nepip_entities->ngepi_className) != 0) {
            continue;
        }

        /* found */
        return cur;
    }

    /* not found */
    return NULL;
}

/**
 * This function returns Next item from current,
 *  if current reached end of first Pair list, then
 *   return next Pair list item, but if item has same host/class name
 *   as one of first Pair list, then skip.
 */
ngcllExecutablePathInfoPair_t *
ngcllExecutablePathInfoPairGetNextInTwo(
    ngcllExecutablePathInfoPair_t *epInfoPair1,
    ngcllExecutablePathInfoPair_t *epInfoPair2,
    ngcllExecutablePathInfoPair_t *current)
{
    ngcllExecutablePathInfoPair_t *curEpInfo;
    ngcllExecutablePathInfoPair_t *prevEpInfo;
    ngcllExecutablePathInfoPair_t *foundEpInfo;
    ngcllExecutablePathInfoPair_t *tailFind;

    if (current == NULL) {
        if (epInfoPair1 != NULL) {
            return epInfoPair1;
        } else {
            return epInfoPair2;
        }
    }

    prevEpInfo = current;
    curEpInfo = current->nepip_next;
    while (1) {
        /**
         * if curEpInfo reached tail of epInfoPair1, then continue
         *   beginning of epInfoPair2
         * else, return NULL to finish (reached rmInfoPair2 tail).
         */
        if (curEpInfo == NULL) {
            if (epInfoPair1 == NULL) {
                /* not found */
                goto notFound;
            }

            /* find epInfoPair1 list tail */
            tailFind = epInfoPair1;
            while (tailFind->nepip_next != NULL) {
                tailFind = tailFind->nepip_next;
            }

            if (prevEpInfo == tailFind) {
                /* found curEpInfo was reached tail of epInfoPair1 */

                if (epInfoPair2 == NULL) {
                    /* not found */
                    goto notFound;
                } else {
                    curEpInfo = epInfoPair2;
                }
            } else {
                /* found curEpInfo was reached tail of epInfoPair1 */
                goto notFound;
            }
        }

        /* uniqueness check, was curEmInfo->*Name appeared before? */
        assert(curEpInfo != NULL);
        foundEpInfo = ngcllExecutablePathInfoPairGet(epInfoPair1,
            curEpInfo->nepip_entities->ngepi_hostName,
            curEpInfo->nepip_entities->ngepi_className);
        if (foundEpInfo == curEpInfo) {
            /* found unique next item */
            return curEpInfo;
        }
        if (foundEpInfo == NULL) {
            /* curEpInfo is in epInfoPair2 */

            foundEpInfo = ngcllExecutablePathInfoPairGet(epInfoPair2,
                curEpInfo->nepip_entities->ngepi_hostName,
                curEpInfo->nepip_entities->ngepi_className);
            if (foundEpInfo == curEpInfo) {
                /* found unique next item */
                return curEpInfo;
            }
            assert(foundEpInfo != NULL);
        }

        /* curEpInfo was not unique, so try to check next item unique */
        prevEpInfo = curEpInfo;
        curEpInfo = curEpInfo->nepip_next;
    }

notFound:
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
    newArray = (char **)globus_libc_malloc(sizeof(char *) * newSize);
    if (newArray == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the StringArray.\n", fName);
        return 0;
    }

    i = 0;
    /* Copy from dst */
    for (j = 0; j < *dstSize; j++, i++) {
        assert((*dstArray)[j] != NULL);

        newArray[i] = strdup((*dstArray)[j]);
        if (newArray[i] == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't duplicate the string.\n", fName);
            return 0;
        }
    }

    /* Copy from src */
    for (j = 0; j < srcSize; j++, i++) {
        assert(srcArray[j] != NULL);

        newArray[i] = strdup(srcArray[j]);
        if (newArray[i] == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't duplicate the string.\n", fName);
            return 0;
        }
    }
    assert(i == newSize);

    if (*dstArray != NULL) {
        result = ngcllStringArrayFree(*dstArray, *dstSize);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't deallocate the StringArray.\n", fName);
            return 0;
        }
    }

    *dstSize = newSize;
    *dstArray = newArray;

    /* Success */
    return 1;
}

static int 
ngcllStringArrayFree(char **array, int size)
{
    int i;

    assert(size > 0);
    assert(array != NULL);

    for (i = 0; i < size; i++) {
        assert(array[i] != NULL);
        globus_libc_free(array[i]);
    }

    globus_libc_free(array);

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
    static const char fName[] = "ngcllLocalMachineInformationSetDefault";
    int result;

    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngiLogInformationSetDefault(
        &lmInfo->nglmi_logInfo, NG_LOG_TYPE_GENERIC, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set default value for Log Information.\n", fName);
        return 0;
    }

    lmInfo->nglmi_hostName = NULL;
    lmInfo->nglmi_saveNsessions = 256;
    lmInfo->nglmi_tmpDir = NULL; /* Use $TMPDIR or /tmp */
    lmInfo->nglmi_refreshInterval = 0; /* disable although pthread version */
    lmInfo->nglmi_invokeServerLog = NULL; /* no log */
    lmInfo->nglmi_fortranCompatible = 0; /* false */
    lmInfo->nglmi_listenPort = 0U;
    lmInfo->nglmi_listenPortAuthOnly = 0U;
    lmInfo->nglmi_listenPortGSI = 0U;
    lmInfo->nglmi_listenPortSSL = 0U;
    lmInfo->nglmi_tcpNodelay = 0;/* false */
    /* signal handling default is set by Register */

    /* Success */
    return 1;
}

static int
ngcllMDSserverInformationSetDefault(
    ngclMDSserverInformation_t *mdsInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInformationSetDefault";

    if (mdsInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    mdsInfo->ngmsi_hostName = NULL; /* no default hostname */
    mdsInfo->ngmsi_tagName = NULL;
    mdsInfo->ngmsi_type = NGCL_MDS_SERVER_TYPE_MDS2;
    mdsInfo->ngmsi_portNo = 0; /* 2135 for MDS2, 8443 for MDS4 */
    mdsInfo->ngmsi_protocol = NULL;
    mdsInfo->ngmsi_path = NULL;
    mdsInfo->ngmsi_subject = NULL;
    mdsInfo->ngmsi_voName = strdup("local");
    mdsInfo->ngmsi_clientTimeout = 0;
    mdsInfo->ngmsi_serverTimeout = 0;

    if (mdsInfo->ngmsi_voName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the string.\n",
            fName);
        return 0;
    }

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
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
ngcllRemoteMachineInformationSetDefault(
    ngclRemoteMachineInformation_t *rmInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationSetDefault";

    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngiLogInformationSetDefault(
        &rmInfo->ngrmi_commLogInfo, NG_LOG_TYPE_COMMUNICATION, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set default value for Comm Log Information.\n", fName);
        return 0;
    }

    rmInfo->ngrmi_hostName  = NULL; /* no default hostname */
    rmInfo->ngrmi_tagName  = NULL;
    rmInfo->ngrmi_portNo = 0;
    rmInfo->ngrmi_mdsServer = NULL;
    rmInfo->ngrmi_mdsTag = NULL;
    rmInfo->ngrmi_invokeServerType = NULL;
    rmInfo->ngrmi_invokeServerNoptions = 0;
    rmInfo->ngrmi_invokeServerOptions = NULL;
    rmInfo->ngrmi_mpiNcpus = 0; /* to retrieve MDS */
    rmInfo->ngrmi_gassScheme = strdup("http");
    rmInfo->ngrmi_crypt = NG_PROTOCOL_CRYPT_NONE;
    rmInfo->ngrmi_protocol = NG_PROTOCOL_TYPE_BINARY; /* temporary setting */
                          /* NG_PROTOCOL_TYPE_XML unimplemented yet.  making */
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
#ifdef NG_PTHREAD
    rmInfo->ngrmi_heartBeat = 60; /* Only pthread version can handle */
#else /* NG_PTHREAD */
    rmInfo->ngrmi_heartBeat = 0;  /* disable heartbeat for other flavor */
#endif /* NG_PTHREAD */
    rmInfo->ngrmi_heartBeatTimeoutCount = 5;
    rmInfo->ngrmi_heartBeatTimeoutCountOnTransfer = -1; /* Same as count */
    rmInfo->ngrmi_redirectEnable = 1; /* true */
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
    rmInfo->ngrmi_debug.ngdi_enable = 0; /* false */
    rmInfo->ngrmi_debug.ngdi_terminalPath = NULL; /* making : xterm */
    rmInfo->ngrmi_debug.ngdi_display = NULL; /* send environ? always :0.0 */
    rmInfo->ngrmi_debug.ngdi_debuggerPath = NULL; /* making : gdb */
    rmInfo->ngrmi_debugBusyLoop = 0; /* false */
    rmInfo->ngrmi_nEnvironments = 0;
    rmInfo->ngrmi_environment = NULL;

    if (rmInfo->ngrmi_gassScheme == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the string.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllExecutablePathInformationSetDefault(
    ngcliExecutablePathInformation_t *epInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInformationSetDefault";

    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    epInfo->ngepi_hostName = NULL;
    epInfo->ngepi_className = NULL;
    epInfo->ngepi_path = NULL;
    epInfo->ngepi_stagingEnable = 0; /* false */
    epInfo->ngepi_backend = NG_BACKEND_NORMAL;
    epInfo->ngepi_mpiNcpus = 0; /* undefined on default */
    epInfo->ngepi_sessionTimeout = 0; /* disable */

    /* Success */
    return 1;
}

