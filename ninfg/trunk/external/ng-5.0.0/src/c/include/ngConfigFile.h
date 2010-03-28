/*
 * $RCSfile: ngConfigFile.h,v $ $Revision: 1.6 $ $Date: 2008/02/07 10:26:15 $
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

#ifndef _NGCONFIGFILE_H_
#define _NGCONFIGFILE_H_

/**
 * Note: this file is required by
 *   ngConfigFileToken.c
 *   ngclConfigFileRead.c
 *   ngexConfigFileRead.c
 */

#include "ng.h"

/**
 * Token proceeding information
 */

/* get TokenInfo from TokenReadInfo */
typedef struct ngiTokenReadInfo_s {
    FILE *ntri_fp;
    char *ntri_line;   /* line oriented analysis */
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


/* for argument analysis */
typedef struct ngiStringList_s {
    struct ngiStringList_s *nsl_next;
    char *nsl_string;
} ngiStringList_t;

/* for conversion table for time and size */
typedef struct ngiUnitConvTable_s {
    char *nuct_unitStr;  /* string */
    int nuct_unitAmount; /* number of amount */
} ngiUnitConvTable_t;


ngiStringList_t *ngiStringListConstruct(
    char *string, ngLog_t *log, int *error);
int ngiStringListDestruct(
    ngiStringList_t *stringList, ngLog_t *log, int *error);
int ngiStringListRegister(
    ngiStringList_t **dst, char *src, ngLog_t *log, int *error);
int ngiStringListRegisterList(
    ngiStringList_t **dst, ngiStringList_t *src, ngLog_t *log, int *error);
int ngiStringListCount(
    ngiStringList_t *stringList, int *count, ngLog_t *log, int *error);
int ngiStringListCheckIncludeSameString(
    ngiStringList_t *strings, char *str, int caseSensitive,
    int *isInclude, ngLog_t *log, int *error);
ngiStringList_t *ngiStringListCheckListIncludeSameString(
    ngiStringList_t *stringsA, ngiStringList_t *stringsB,
    int caseSensitive, ngLog_t *log, int *error);
int ngiStringListSubtract(
    ngiStringList_t *stringsA, ngiStringList_t *stringsB,
    int caseSensitive, ngiStringList_t **resultString,
    ngLog_t *log, int *error);
ngiStringList_t *ngiStringListDuplicate(
    ngiStringList_t *stringList, ngLog_t *log, int *error);
char *ngiStringListMergeToString(
    ngiStringList_t *strings, ngLog_t *log, int *error);
int ngiStringListToStringArray(char ***dstArray, int *dstSize,
    ngiStringList_t *strings, ngLog_t *log, int *error);


/* utility functions that is used from attribute functions */
int ngiReadIntFromArg(char *arg, int *num);
int ngiReadDoubleFromArg(char *arg, double *num);
char *ngiReadStringFromArg(char *arg, ngLog_t *log, int *error);
char *ngiReadQuotedStringFromArg(
    char *arg, int allowContinue, int *valueContinueToNextLine,
    int continuedValue, ngLog_t *log, int *error);
int ngiReadEnumFromArg(char *arg, int caseSensitive, int num, ...);
int ngiReadUnitNumFromArg(char *arg, int *num, ngiUnitConvTable_t *table,
    int caseSensitive);
ngiStringList_t *ngiReadStringListFromArg(
    char *arg, ngLog_t *log, int *error);
char *ngiReadEnvStringFromArg(char *arg, ngLog_t *log, int *error);
int ngiReadStrEqualNumberFromArg(
    char *arg, char **str, int *num, ngLog_t *log, int *error);

int ngiConfigFileSyntaxError(ngLog_t *log, ngiTokenInfo_t *token,
    char *errMessage, const char *fName,
    char *attrName, char *keyword, int *error); 

#define NGI_ISSET_S_TRUE  "set" /* if ISSET_S_TRUE is set into        */
#define NGI_ISSET_I_TRUE  1     /* string pointer in isSet, then this */
#define NGI_ISSET_D_TRUE  1.0   /* member in entities is enabled.     */

#define NGI_SECTION_NAME_CASE_SENSITIVE 1
#define NGI_ATTR_NAME_CASE_SENSITIVE 1
#define NGI_ATTR_ARG_CASE_SENSITIVE 1
#define NGI_FILENAME_CASE_SENSITIVE 1
#define NGI_HOSTNAME_CASE_SENSITIVE 0
       /* It is available for true/false ...  */
       /* no effect for filenames */

#endif /* _NGCONFIGFILE_H_ */

