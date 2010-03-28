/*
 * $RCSfile: ngConfigFile.h,v $ $Revision: 1.26 $ $Date: 2006/08/17 07:21:19 $
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

#ifndef _NGCLCONFIGFILE_H_
#define _NGCLCONFIGFILE_H_

/**
 * Note: this file is required by
 *   ngConfigFileToken.c
 *   ngclConfigFileRead.c
 *   ngclConfigFileLdif.c
 *   ngexConfigFileRead.c
 */

#include "ng.h"
#include <stdarg.h>


/**
 * Token proceeding information
 */

/* get TokenInfo from TokenReadInfo */
typedef struct ngiTokenReadInfo_s {
    FILE *ntri_fp;
    char *ntri_line;   /* line oriented analysys */
                       /* line has line buffer and use it many times, */
                       /* (don't free! until fp close) */
    char *ntri_currentRead; /* NULL if not read line from *fp yet
                           or points reading place in line buffer */
    int ntri_readTokensInline;  /* read tokens in line ('till current_read) */
    char *ntri_token; /* you can use this buffer to memory token, */
                      /* (don't free) */
    long ntri_lineno; /* line number of config file */
    char *ntri_filename; /* Current reading config file name */
} ngiTokenReadInfo_t;

typedef enum ngiTokenType_e {
    NGI_TOKEN_TAG,           /* <SOME_STRING> or </SOME_STRING> */
    NGI_TOKEN_ATTR,              /* any string except <SOME_STRING> */
    NGI_TOKEN_NO_TOKEN_ANYMORE   /* EOF */
} ngiTokenType_t;

typedef struct ngiTokenInfo_s {
    ngiTokenType_t nti_type;
    int nti_tokennoInline;    /* token number in line */
    char *nti_tokenStr;       /* points data in TokenReadInfo->token */
    ngiTokenReadInfo_t *nti_readInfo; /* points TokenReadInfo */
} ngiTokenInfo_t;

typedef enum ngiGetTokenType_e {
    NGI_GETTOKEN_TOKEN,     /* request to get normal token */
    NGI_GETTOKEN_ARGS,      /* request to get argument for attribute */
                        /*  including spaces and tabs */
                        /* all strings remaining on line, except comment */
                        /* Warning : ARGS Token includes space and tabs */
    NGI_GETTOKEN_QUOTED /* request to get next line of double-quoted line, */
                        /* continued by backslash + return character */
} ngiGetTokenType_t;

#define NGI_CONFIG_LINE_MAX 1024

#define NGI_GET_TOKEN_SUCCESS 1  /* ngiGetToken() return code */
#define NGI_GET_TOKEN_EOF     2
#define NGI_GET_TOKEN_INVALID 3

#define NGI_GET_TAGNAME_BEGIN 1  /* ngiGetTagName() return code */
#define NGI_GET_TAGNAME_END   2
#define NGI_GET_TAGNAME_FAIL  3

/* Token functions */
int ngiConfigFileOpen(char *filename,
    ngiTokenReadInfo_t **tokenReadInfo, int requireAvailable,
    ngLog_t *log, int *error);
int ngiConfigFileClose(ngiTokenReadInfo_t *tokenReadInfo,
    ngLog_t *log, int *error);

int ngiGetToken(ngiTokenReadInfo_t *tokenReadInfo, ngiTokenInfo_t *token,
    ngiGetTokenType_t tokenType, ngLog_t *log, int *error);
int ngiGetTagName(char *tag, char *tagname);


/* for argument analysys */
typedef struct ngiStringList_s {
    struct ngiStringList_s *nsl_next;
    char *nsl_string;
} ngiStringList_t;

/* for conversion table for time and size */
typedef struct ngiUnitConvTable_s {
    char *nuct_unitStr;  /* string */
    int nuct_unitAmount; /* number of amount */
} ngiUnitConvTable_t;


ngiStringList_t *ngiStringListConstruct(char *string);
int ngiStringListDestruct(ngiStringList_t *stringList);
int ngiStringListRegister(ngiStringList_t **dst, char *src);
int ngiStringListRegisterList(ngiStringList_t **dst, ngiStringList_t *src);
int ngiStringListCount(ngiStringList_t *stringList, int *count);
int ngiStringListCheckIncludeSameString(
    ngiStringList_t *strings, char *str, int caseSensitive, int *isInclude);
ngiStringList_t *ngiStringListCheckListIncludeSameString(
    ngiStringList_t *stringsA, ngiStringList_t *stringsB, int caseSensitive);
int ngiStringListSubtract(ngiStringList_t *stringsA, ngiStringList_t *stringsB,
    int caseSensitive, ngiStringList_t **resultString);
ngiStringList_t * ngiStringListDuplicate(ngiStringList_t *stringList);
char *ngiStringListMergeToString(ngiStringList_t *strings);
int ngiStringListToStringArray(char ***dstArray, int *dstSize,
    ngiStringList_t *strings);


/* utility functions that is used from attribute functions */
int ngiReadIntFromArg(char *arg, int *num);
int ngiReadDoubleFromArg(char *arg, double *num);
char *ngiReadStringFromArg(char *arg);
char *ngiReadQuotedStringFromArg(char *arg, int, int *, int);
int ngiReadEnumFromArg(char *arg, int caseSensitive, int num, ...);
int ngiReadUnitNumFromArg(char *arg, int *num, ngiUnitConvTable_t *table,
    int caseSensitive);
ngiStringList_t *ngiReadStringListFromArg(char *arg);
char *ngiReadEnvStringFromArg(char *arg);
int ngiReadStrEqualNumberFromArg(char *arg, char **str, int *num);

int ngiConfigFileSyntaxError(ngLog_t *log, ngiTokenInfo_t *token,
    char *errMessage, const char *fName,
    char *attrName, char *keyword, int *error); 


/**
 * below is for ngclConfigFileRead.c, ngclConfigFileLdif.c
 *   inter processing data structure ReadingState
 *   while parsing client config file.
 */

typedef struct ngcllLocalMachineInfoPair_s {
    struct ngcllLocalMachineInfoPair_s *nlmip_next;
    ngclLocalMachineInformation_t *nlmip_entities;
    ngclLocalMachineInformation_t *nlmip_isSet;
    /* isSet's member != 0, and != NULL, then that member is enable */
    /* isSet should not free(), this includes only static string pointer */
} ngcllLocalMachineInfoPair_t;

typedef struct ngcllMDSserverInfoPair_s {
    struct ngcllMDSserverInfoPair_s *nmsip_next;
    ngclMDSserverInformation_t *nmsip_entities;
    ngclMDSserverInformation_t *nmsip_isSet;
} ngcllMDSserverInfoPair_t;

typedef struct ngcllInvokeServerInfoPair_s {
    struct ngcllInvokeServerInfoPair_s *nisip_next;
    ngclInvokeServerInformation_t *nisip_entities;
    ngclInvokeServerInformation_t *nisip_isSet;
} ngcllInvokeServerInfoPair_t;

typedef struct ngcllRemoteMachineInfoPair_s {
    struct ngcllRemoteMachineInfoPair_s *nrmip_next;
    ngclRemoteMachineInformation_t *nrmip_entities;
    ngclRemoteMachineInformation_t *nrmip_isSet;
} ngcllRemoteMachineInfoPair_t;

typedef struct ngcllExecutablePathInfoPair_s {
    struct ngcllExecutablePathInfoPair_s *nepip_next;
    ngcliExecutablePathInformation_t *nepip_entities;
    ngcliExecutablePathInformation_t *nepip_isSet;
    char *nepip_tagName; /* Used by <SERVER> mpi_runNoOfCPUs */
} ngcllExecutablePathInfoPair_t;

#define NGCL_ISSET_S_TRUE  "set" /* if ISSET_S_TRUE is set into        */
#define NGCL_ISSET_I_TRUE  1     /* string pointer in isSet, then this */
#define NGCL_ISSET_D_TRUE  1.0   /* member in entities is enabled.     */


typedef struct ngcllReadingState_s {
    ngiStringList_t *nrs_appearedConfigs;
    ngiStringList_t *nrs_appearedLocalLdifs;

    /* for LocalMachineInformation */
    int nrs_lmiAppeared;
    ngcllLocalMachineInfoPair_t *nrs_lmInfo;
    int nrs_signalAppeared;
    int *nrs_appearedSignals;

    /* for MDSserverInformation */
    ngiStringList_t *nrs_appearedMDSservers;
    ngiStringList_t *nrs_appearedNoTagMDSservers;
    ngiStringList_t *nrs_appearedMDStags;
    ngcllMDSserverInfoPair_t *nrs_mdsInfos;
    ngcllMDSserverInfoPair_t *nrs_curMDSinfo;

    /* for InvokeServerInformation */
    ngiStringList_t *nrs_appearedInvokeServers;
    ngcllInvokeServerInfoPair_t *nrs_isInfos;
    ngcllInvokeServerInfoPair_t *nrs_curISinfo;
    ngiStringList_t *nrs_curISinfoOptions;

    /* for RemoteMachineInformation */
    ngiStringList_t *nrs_appearedRmInfoHosts;
    ngiStringList_t *nrs_appearedNoTagRmInfoHosts;
    ngiStringList_t *nrs_appearedRmInfoTags;
    ngiStringList_t *nrs_appearedMDSserversInRm;
    ngiStringList_t *nrs_appearedInvokeServersInRm;
    ngcllRemoteMachineInfoPair_t *nrs_rmInfos;
                                  /* store all remote machine infos read */
    ngcllExecutablePathInfoPair_t *nrs_rmInfoEps; /* mpiNcpus */
    ngcllRemoteMachineInfoPair_t *nrs_curRmInfo; /* current reading info */
    ngcllExecutablePathInfoPair_t *nrs_curRmInfoEps; /* mpiNcpus */
    ngiStringList_t *nrs_curRmInfoInvokeServerOptions;  /* current opts */
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

    /* for Local LDIF RemoteMachineInformation */
    ngiStringList_t *nrs_appearedLocalRmInfoHosts;
    ngcllRemoteMachineInfoPair_t *nrs_localRmInfos;
    ngcllRemoteMachineInfoPair_t *nrs_localCurRmInfo;

    /* for Local LDIF ExecutablePathInformation */
    ngcllExecutablePathInfoPair_t *nrs_localEpInfos;
    ngcllExecutablePathInfoPair_t *nrs_localCurEpInfo;
    ngiStringList_t *nrs_localCurEpInfoStub;

    /* for RemoteClassInformation (in Local LDIF) */
    ngiStringList_t *nrs_appearedClasses;

} ngcllReadingState_t;

/**
 * Note: Configuration Priority
 *       1. client configuration file
 *       2. <SERVER_DEFAULT> section
 *       3. local LDIF
 *       4. MDS
 *       5. source code default
 */

#define NGCLL_CONFIG_ATTRFUNC_ARG \
    (char *attrName, ngiTokenInfo_t *token, \
     ngcllReadingState_t *readingState, ngclContext_t *context, \
     int *error)

typedef int (*ngcllAttrFunc_t)NGCLL_CONFIG_ATTRFUNC_ARG;

#define NGI_SECTION_NAME_CASE_SENSITIVE 1
#define NGI_ATTR_NAME_CASE_SENSITIVE 1
#define NGI_ATTR_ARG_CASE_SENSITIVE 1
#define NGI_FILENAME_CASE_SENSITIVE 1
#define NGI_HOSTNAME_CASE_SENSITIVE 0
#define NGI_LDIFTAG_CASE_SENSITIVE 1
       /* It is available for true/false ...  */
       /* no effect for filenames */

/**
 * FuncTable is for to match keyword string and attribute functions.
 * Last entry of array should NULL.
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

int ngcllConfigFileReadLocalLdif         NGCLL_CONFIG_ATTRFUNC_ARG;

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


/* Pair for MDSserverInformation */
ngcllMDSserverInfoPair_t * ngcllMDSserverInfoPairCreate(
    ngclContext_t *context, int *error);
int ngcllMDSserverInfoPairDestruct(ngclContext_t *context,
    ngcllMDSserverInfoPair_t *mdsInfoPair, int *error);
int ngcllMDSserverInfoPairListDestruct(ngclContext_t *context,
    ngcllMDSserverInfoPair_t *mdsInfoPair, int *error);
int ngcllMDSserverInfoPairInitialize(ngclContext_t *context, 
    ngcllMDSserverInfoPair_t *mdsInfoPair, int *error);
int ngcllMDSserverInfoPairRegister(
    ngcllMDSserverInfoPair_t **dst, ngcllMDSserverInfoPair_t *src);
int ngcllMDSserverInformationSetPair(
    ngclMDSserverInformation_t *dst, ngcllMDSserverInfoPair_t *src,
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
ngcllRemoteMachineInfoPair_t * ngcllRemoteMachineInfoPairGetNextInTwo(
    ngcllRemoteMachineInfoPair_t *rmInfoPair1,
    ngcllRemoteMachineInfoPair_t *rmInfoPair2,
    ngcllRemoteMachineInfoPair_t *current);

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
    ngcliExecutablePathInformation_t *dst, ngcllExecutablePathInfoPair_t *src,
    ngLog_t *log, int *error);
ngcllExecutablePathInfoPair_t *ngcllExecutablePathInfoPairGet(
    ngcllExecutablePathInfoPair_t *epInfoPair,
    char *hostName, char *className);
ngcllExecutablePathInfoPair_t * ngcllExecutablePathInfoPairGetWithTag(
    ngcllExecutablePathInfoPair_t *epInfoPair,
    char *hostName, char *tagName, char *className);
ngcllExecutablePathInfoPair_t *ngcllExecutablePathInfoPairGetNextInTwo(
    ngcllExecutablePathInfoPair_t *epInfoPair1,
    ngcllExecutablePathInfoPair_t *epInfoPair2,
    ngcllExecutablePathInfoPair_t *current);

#endif /* _NGCLCONFIGFILE_H_ */

