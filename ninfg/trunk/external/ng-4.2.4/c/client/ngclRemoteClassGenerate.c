#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclRemoteClassGenerate.c,v $ $Revision: 1.17 $ $Date: 2006/02/02 07:04:35 $";
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
 * Function of RemoteClassInformationGenerate
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ng.h"
#include "ngXML.h"
#include "ngFunctionInformation.h"

#include <stdio.h>

/**
 *
 * XML tree traverse function call tree
 *
 * class
 *     method
 *         argument
 *             ioMode
 *             dataType
 *             subscript
 *                 expressionParent
 *                     expression
 *         methodAttribute
 *             expressionParent
 *                 expression
 *             description
 *     classAttribute
 *         description
 *
 * expression
 *     expressionTraverse
 *         expressionTraverseMono
 *         expressionTraverseBi
 *         expressionTraverseTri
 *         expressionTraverseImmediate
 *         expressionTraverseScalarref
 *         expressionTraverseNoValue
 *     expressionElement
 */

/**
 * RemoteClassInformationGenerate
 *  creates ngRemoteClassInformation_t from XML string.
 */
ngRemoteClassInformation_t *
ngcliRemoteClassInformationGenerate(
    ngclContext_t *context,
    char *buf,
    int len,
    int *error);

/**
 * Internal data structures
 */

/* convert table for string to method argument ioMode */
typedef struct ngcllRemoteClassReadXMLioMode_s {
    char *ngrmt_ioModeString;       /* each ioMode XML string */
    ngArgumentIOmode_t ngrmt_mode;  /* enum */
} ngcllRemoteClassReadXMLioMode_t;

/* convert table for string to method argument dataType */
typedef struct ngcllRemoteClassReadXMLdataType_s {
    char *ngrdt_dataTypeString;       /* each dataType XML string */
    ngArgumentDataType_t ngrdt_data;  /* enum */
} ngcllRemoteClassReadXMLdataType_t;

/* convert table for string to backend */
typedef struct ngcllRemoteClassReadXMLbackend_s {
    char *ngrbt_backendString;        /* each backend XML string */
    ngBackend_t ngrbt_backend;        /* enum */
} ngcllRemoteClassReadXMLbackend_t;

/* convert table for string to class language */
typedef struct ngcllRemoteClassReadXMLclassLang_s {
    char *ngrclt_classLangString;        /* each class language XML string */
    ngClassLanguage_t ngrclt_classLang;  /* enum */
} ngcllRemoteClassReadXMLclassLang_t;

typedef int (*ngcllExpressionTraverseFunc_t) (\
    ngclContext_t *context, \
    ngExpressionElement_t *expression,\
    int *eIndex,\
    int eSize,\
    ngiXMLelement_t *element,\
    int *error);

/* convert table for string to expression traverse functions */
typedef struct ngcllExpressionTraverseType_s {
    char *ngett_funcString;
    ngcllExpressionTraverseFunc_t ngett_func;
} ngcllExpressionTraverseType_t;

/* convert table for string to expression operators */
typedef struct ngcllExpressionOperatorType_s {
    char *ngeot_opString;
    ngExpressionOperationCode_t ngeot_opCode;
} ngcllExpressionOperatorType_t;


/**
 * local prototypes.
 */
static int
ngcllRemoteClassReadXMLclass(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLmethod(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLargument(
    ngclContext_t *context,
    ngArgumentInformation_t *argument,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLioModeConvert(
    ngclContext_t *context,
    ngArgumentIOmode_t *modeType,
    char *modeTypeStr,
    int *error);

static int
ngcllRemoteClassReadXMLdataTypeConvert(
    ngclContext_t *context,
    ngArgumentDataType_t *dataType,
    char *dataTypeStr,
    int *error);

static int
ngcllRemoteClassReadXMLsubscript(
    ngclContext_t *context,
    ngSubscriptInformation_t *subscript,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLexpressionParent(
    ngclContext_t *context,
    char *elementName,
    ngExpressionElement_t **expression,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLexpression(
    ngclContext_t *context,
    ngExpressionElement_t **expression,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLmethodAttribute(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLdescription(
    ngclContext_t *context,
    char **description,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLclassAttribute(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpression(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionMono(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionBi(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionTri(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionImmediate(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionScalarref(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionNoValue(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLexpressionElementAdd(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngExpressionValueType_t valueType,
    int value,
    int *error);

static int
ngcllRemoteClassReadXMLexpressionElementSizeCheck(
    ngclContext_t *context,
    ngiXMLelement_t *element,
    int *eSize,
    int *error);

static int
ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
    ngclContext_t *context,
    ngiXMLelement_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLioModeConvert(
    ngclContext_t *context,
    ngArgumentIOmode_t *modeType,
    char *modeTypeStr,
    int *error);

static int
ngcllRemoteClassReadXMLdataTypeConvert(
    ngclContext_t *context,
    ngArgumentDataType_t *dataType,
    char *dataTypeStr,
    int *error);

static int
ngcllRemoteClassReadXMLbackendConvert(
    ngclContext_t *context,
    ngBackend_t *backend,
    char *backendStr,
    int *error);

static int
ngcllRemoteClassReadXMLclassLangConvert(
    ngclContext_t *context,
    ngClassLanguage_t *classLang,
    char *classLangStr,
    int *error);

static int
ngcllRemoteClassReadXMLoperatorConvert(
    ngclContext_t *context,
    ngExpressionOperationCode_t *operator,
    char *operatorStr,
    ngcllExpressionOperatorType_t *operatorTable,
    int *error);

#define NGCLL_XML_CLASS         "class"
#define NGCLL_XML_VERSION       "version"
#define NGCLL_XML_NUM_METHODS   "numMethods"
#define NGCLL_XML_CLASS_NAME    "name"
#define NGCLL_XML_CLASS_ATTR    "classAttribute"
#define NGCLL_XML_BACKEND       "backend"
#define NGCLL_XML_LANGUAGE      "language"
#define NGCLL_XML_DESCRIPTION   "description"

#define NGCLL_XML_METHOD        "method"
#define NGCLL_XML_METHOD_NAME   "name"
#define NGCLL_XML_METHOD_ID     "id"
#define NGCLL_XML_METHOD_ATTR   "methodAttribute"
#define NGCLL_XML_SHRINK        "shrink"
#define NGCLL_XML_SHRINK_YES    "yes"
#define NGCLL_XML_SHRINK_NO     "no"
#define NGCLL_XML_ORDER         "calculationOrder"

#define NGCLL_XML_ARGUMENT      "arg"
#define NGCLL_XML_IO_MODE       "ioMode"
#define NGCLL_XML_DATA_TYPE     "dataType"
#define NGCLL_XML_SUBSCRIPT     "subscript"
#define NGCLL_XML_SUB_SIZE      "size"
#define NGCLL_XML_SUB_START     "start"
#define NGCLL_XML_SUB_END       "end"
#define NGCLL_XML_SUB_SKIP      "skip"
#define NGCLL_XML_EXPRESSION    "expression"
#define NGCLL_XML_EXP_NAME      "name"
#define NGCLL_XML_EXP_VAL       "val"

#define NGCLL_XML_ARITH_MONO "monoArithmetic"
#define NGCLL_XML_ARITH_BI   "biArithmetic"
#define NGCLL_XML_ARITH_TRI  "triArithmetic"
#define NGCLL_XML_IMMEDIATE  "immediate"
#define NGCLL_XML_SCALARREF  "scalarref"
#define NGCLL_XML_NO_VALUE   "noValue"


static ngcllRemoteClassReadXMLioMode_t ioModeTable[] = {
    {"none",  NG_ARGUMENT_IO_MODE_NONE},
    {"work",  NG_ARGUMENT_IO_MODE_WORK},
    {"in",    NG_ARGUMENT_IO_MODE_IN},
    {"out",   NG_ARGUMENT_IO_MODE_OUT},
    {"inout", NG_ARGUMENT_IO_MODE_INOUT},
    {NULL, (ngArgumentIOmode_t) 0},
};

static ngcllRemoteClassReadXMLdataType_t dataTypeTable[] = {
    {"char",     NG_ARGUMENT_DATA_TYPE_CHAR},
    {"short",    NG_ARGUMENT_DATA_TYPE_SHORT},
    {"int",      NG_ARGUMENT_DATA_TYPE_INT},
    {"long",     NG_ARGUMENT_DATA_TYPE_LONG},
/*    {"longlong", ?}, */
    {"float",    NG_ARGUMENT_DATA_TYPE_FLOAT},
    {"double",   NG_ARGUMENT_DATA_TYPE_DOUBLE},
    {"scomplex", NG_ARGUMENT_DATA_TYPE_SCOMPLEX},
    {"dcomplex", NG_ARGUMENT_DATA_TYPE_DCOMPLEX},
    {"string",   NG_ARGUMENT_DATA_TYPE_STRING},
    {"filename", NG_ARGUMENT_DATA_TYPE_FILENAME},
    {"callback", NG_ARGUMENT_DATA_TYPE_CALLBACK},
    {NULL, (ngArgumentDataType_t) 0},
};

static ngcllRemoteClassReadXMLbackend_t backendTable[] = {
    {"normal",  NG_BACKEND_NORMAL},
    {"mpi",     NG_BACKEND_MPI},
    {"blacs",   NG_BACKEND_BLACS},
    {NULL, (ngBackend_t) 0},
};

static ngcllRemoteClassReadXMLclassLang_t classLangTable[] = {
    {"C",       NG_CLASS_LANGUAGE_C},
    {"FORTRAN", NG_CLASS_LANGUAGE_FORTRAN},
    {NULL, (ngClassLanguage_t) 0},
};

static ngcllExpressionTraverseType_t traverseTable[] = {
    {NGCLL_XML_ARITH_MONO, ngcllRemoteClassReadXMLtraverseExpressionMono},
    {NGCLL_XML_ARITH_BI,   ngcllRemoteClassReadXMLtraverseExpressionBi},
    {NGCLL_XML_ARITH_TRI,  ngcllRemoteClassReadXMLtraverseExpressionTri},
    {NGCLL_XML_IMMEDIATE,  ngcllRemoteClassReadXMLtraverseExpressionImmediate},
    {NGCLL_XML_SCALARREF,  ngcllRemoteClassReadXMLtraverseExpressionScalarref},
    {NGCLL_XML_NO_VALUE,   ngcllRemoteClassReadXMLtraverseExpressionNoValue},
    {NULL, NULL},
};

static ngcllExpressionOperatorType_t opTableMono[] = {
    {"neg", NG_EXPRESSION_OPCODE_UNARY_MINUS},
    {NULL, (ngExpressionOperationCode_t) 0},

};

static ngcllExpressionOperatorType_t opTableBi[] = {
    {"add", NG_EXPRESSION_OPCODE_PLUS},
    {"sub", NG_EXPRESSION_OPCODE_MINUS},
    {"mul", NG_EXPRESSION_OPCODE_MULTIPLY},
    {"div", NG_EXPRESSION_OPCODE_DIVIDE},
    {"mod", NG_EXPRESSION_OPCODE_MODULO},
    {"pow", NG_EXPRESSION_OPCODE_POWER},
    {"eq",  NG_EXPRESSION_OPCODE_EQUAL},
    {"neq", NG_EXPRESSION_OPCODE_NOT_EQUAL},
    {"gt",  NG_EXPRESSION_OPCODE_GREATER_THAN},
    {"lt",  NG_EXPRESSION_OPCODE_LESS_THAN},
    {"ge",  NG_EXPRESSION_OPCODE_GREATER_EQUAL},
    {"le",  NG_EXPRESSION_OPCODE_LESS_EQUAL},
    {NULL, (ngExpressionOperationCode_t) 0},
};

static ngcllExpressionOperatorType_t opTableTri[] = {
    {"tri", NG_EXPRESSION_OPCODE_TRI},
    {NULL, (ngExpressionOperationCode_t) 0},
};


/**
 * function definition
 */

ngRemoteClassInformation_t *
ngcliRemoteClassInformationGenerate(
    ngclContext_t *context,
    char *buf,
    int len,
    int *error)
{
    static const char fName[] = "ngcliRemoteClassInformationGenerate";
    ngRemoteClassInformation_t *rcInfo;
    ngiXMLparser_t *parser;
    ngiXMLelement_t *element;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    /* Check the arguments */
    if ((buf == NULL) || len <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return NULL;
    }

    /* Construct XML parser */
    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't allocate the storage for XMLparser.\n",
                fName);
        return NULL;
    }

    /* Parse XML */
    result = ngiXMLparserParse(parser, buf, len, 1, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: XML parse fail.\n", fName);
        return NULL;
    }

    /* Get root element tree */
    element = ngiXMLparserGetRootElement(parser, log, error);
    if (element == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get root element.\n", fName);
        return NULL;
    }

    /* Get class element tree */
    element = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_CLASS, log, error);
    if (element == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class element in XML.\n", fName);
        return NULL;
    }

    /* Allocate RemoteClassInformation */
    rcInfo = ngcliRemoteClassInformationAllocate(context, error);
    if (rcInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for Remote Class\n", fName);
        return NULL;
    }

    /* Initialize RemoteClassInformation */
    result = ngcliRemoteClassInformationInitialize(context, rcInfo, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the Remote Class\n", fName);
        return NULL;
    }

    /* Convert XML element tree to RemoteClassInformation */
    result = ngcllRemoteClassReadXMLclass(context, rcInfo, element, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Convert XML to RemoteClassInformation fail.\n", fName);
        return NULL;
    }

    /* Destruct XML parser */
    result = ngiXMLparserDestruct(parser, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct XML parser\n", fName);
        return NULL;
    }

    /* Success */
    return rcInfo;
}


static int
ngcllRemoteClassReadXMLclass(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLclass";
    ngiXMLelement_t *childElement;
    ngLog_t *log;
    char *str;
    int result, countMethods;
    int i;

    /* Check the arguments */
    assert(rcInfo != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_CLASS) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get class name string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_CLASS_NAME, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class name in XML.\n", fName);
        return 0;
    }

    /* Set class name string */
    rcInfo->ngrci_className = strdup(str);
    if (rcInfo->ngrci_className == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for the class name.\n",
            fName);
        return 0;
    }

    /* Get version string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_VERSION, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class version in XML.\n", fName);
        return 0;
    }

    /* Set version string */
    rcInfo->ngrci_version = strdup(str);
    if (rcInfo->ngrci_version == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for class version\n",
            fName);
        return 0;
    }

    /* Get number of methods */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_NUM_METHODS, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get number of methods in XML.\n", fName);
        return 0;
    }

    /* Set number of methods */
    rcInfo->ngrci_nMethods = (int)strtol(str, NULL, 10);
    if (rcInfo->ngrci_nMethods < 1) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Number of methods are %d < 1.\n",
            fName, rcInfo->ngrci_nMethods);
        return 0;
    }

    /* Allocate array of ngRemoteMethod_t */
    rcInfo->ngrci_method = ngcliRemoteMethodInformationAllocate(
        context, rcInfo->ngrci_nMethods, error);
    if (rcInfo->ngrci_method == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for Remote Method\n", fName);
        return 0;
    }

    /* Initialize array of ngRemoteMethod_t */
    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
        result = ngcliRemoteMethodInformationInitialize(
            context, &rcInfo->ngrci_method[i], error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't initialize the Remote Method\n", fName);
            return 0;
        }
    }

    /* Set each RemoteMethod information */
    childElement = NULL;
    countMethods = 0;
    while ((childElement = ngiXMLelementGetNext(
        element, childElement, NGCLL_XML_METHOD, log, error)) != NULL) {

        /* Convert method element to metod information */
        result = ngcllRemoteClassReadXMLmethod(
            context ,&rcInfo->ngrci_method[countMethods], childElement, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get method information in XML.\n", fName);
            return 0;
        }
        countMethods++;
    }
    if (countMethods != rcInfo->ngrci_nMethods) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Number of method mismatch.\n", fName);
            return 0;
    }

    /* Get class attribute element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_CLASS_ATTR, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class attribute element in XML.\n", fName);
        return 0;
    }

    /* Convert class attribute element to class information */
    result = ngcllRemoteClassReadXMLclassAttribute(
        context, rcInfo, childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class attribute in XML.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLmethod(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLmethod";
    ngiXMLelement_t *childElement;
    int result, countArguments;
    ngLog_t *log;
    char *str;
    int i;

    /* Check the arguments */
    assert(rMethod != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_METHOD) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get method name string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_METHOD_NAME, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get method name in XML.\n", fName);
        return 0;
    }

    /* Set method name string */
    rMethod->ngrmi_methodName = strdup(str);
    if (rMethod->ngrmi_methodName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for the method name.\n",
            fName);
        return 0;
    }

    /* get method ID */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_METHOD_ID, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get method ID in XML.\n", fName);
        return 0;
    }

    /* Set method ID */
    rMethod->ngrmi_methodID = (int)strtol(str, NULL, 10);
    if (rMethod->ngrmi_methodID < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Method ID invalid.\n", fName);
        return 0;
    }

    /* Get number of arguments */
    rMethod->ngrmi_nArguments =  ngiXMLelementCountElements(
                                  element, NGCLL_XML_ARGUMENT, log, error);
    if (rMethod->ngrmi_nArguments < 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: The number of method argument invalid.\n", fName);
        return 0;
    }

    if (rMethod->ngrmi_nArguments >= 1) {
        /* Allocate array of ngArgumentInformation_t */
        rMethod->ngrmi_arguments = ngcliArgumentInformationAllocate(
            context, rMethod->ngrmi_nArguments, error);
        if (rMethod->ngrmi_arguments == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't allocate the storage for method arguments.\n",
                fName);
            return 0;
        }

        /* Initialize array of ngArgumentInformation_t */
        for (i = 0; i < rMethod->ngrmi_nArguments; i++) {
            result = ngcliArgumentInformationInitialize(
                context, &rMethod->ngrmi_arguments[i], error);
            if (result != 1) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't initialize the method arguments.\n",
                    fName);
                return 0;
            }
        }
    }

    /* Set each ArgumentInformation */
    childElement = NULL;
    countArguments = 0;
    while ((childElement = ngiXMLelementGetNext(
        element, childElement, NGCLL_XML_ARGUMENT, log, error)) != NULL) {

        /* Convert argument element to argument information */
        result = ngcllRemoteClassReadXMLargument(
            context, &rMethod->ngrmi_arguments[countArguments], childElement,
            error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get argument information in XML.\n", fName);
            return 0;
        }
        countArguments++;
    }
    if (countArguments != rMethod->ngrmi_nArguments) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Number of arguments mismatch.\n", fName);
        return 0;
    }

    /* Get method attribute element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_METHOD_ATTR, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get method attribute element in XML.\n", fName);
        return 0;
    }

    /* Convert method attribute element to method information */
    result = ngcllRemoteClassReadXMLmethodAttribute(
        context, rMethod, childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get method attribute in XML.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLargument(
    ngclContext_t *context,
    ngArgumentInformation_t *argument,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLargument";
    ngiXMLelement_t *childElement;
    int countSubscripts;
    int callbacks;
    ngLog_t *log;
    int result;
    char *str;
    int i;

    /* Check the arguments */
    assert(argument != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_ARGUMENT) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get mode type */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_IO_MODE, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get mode type in XML.\n", fName);
        return 0;
    }

    /* Set mode type */
    result = ngcllRemoteClassReadXMLioModeConvert(
        context, &(argument->ngai_ioMode), str, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert mode type in XML.\n", fName);
        return 0;
    }

    /* Get data type */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_DATA_TYPE, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get data type in XML.\n", fName);
        return 0;
    }

    /* Set data type */
    result = ngcllRemoteClassReadXMLdataTypeConvert(
        context, &(argument->ngai_dataType), str, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert mode type in XML.\n", fName);
        return 0;
    }

    /* Get number of dimensions */
    argument->ngai_nDimensions =  ngiXMLelementCountElements(
                                  element, NGCLL_XML_SUBSCRIPT, log, error);
    if (argument->ngai_nDimensions < 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: The number of argument dimension invalid.\n", fName);
        return 0;
    }

    /* if data type is callback, then proceed as callback argument */
    if (argument->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
        if (argument->ngai_nDimensions >= 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: callback in multi dimension.\n",
                fName);
            return 0;
        }

        argument->ngai_subscript = NULL;

        /* Get number of callbacks */
        callbacks =  ngiXMLelementCountElements(
                                  element, NGCLL_XML_METHOD, log, error);
        if (callbacks != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: The number of callback in argument invalid.\n",
                fName);
            return 0;
        }

        /* Get method information element for callback */
        childElement = ngiXMLelementGetNext(element, NULL,
                                              NGCLL_XML_METHOD, log, error);
        if (childElement == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get method element in XML.\n", fName);
            return 0;
        }

        /* Allocate ngRemoteMethod_t */
        argument->ngai_callback = ngcliRemoteMethodInformationAllocate(
            context, 1, error);
        if (argument->ngai_callback == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't allocate the storage for Remote Method\n",
                fName);
            return 0;
        }

        /* Initialize ngRemoteMethod_t */
        result = ngcliRemoteMethodInformationInitialize(
            context, argument->ngai_callback, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't initialize the Remote Method\n", fName);
            return 0;
        }

        /* Convert method element to metod information */
        result = ngcllRemoteClassReadXMLmethod(
            context, argument->ngai_callback, childElement, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get method information in XML.\n", fName);
            return 0;
        }

    } else {
        /* if date type is not callback, then proceed normal subscript */

        argument->ngai_callback = NULL;

        if (argument->ngai_nDimensions >= 1) {
            /**
             *  Allocate array of ngSubscriptInformation_t
             *   for each dimension expression
             */
            argument->ngai_subscript = ngcliSubscriptInformationAllocate(
                context, argument->ngai_nDimensions, error);
            if (argument->ngai_subscript == NULL) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for argument subscripts.\n",
                    fName);
                return 0;
            }

            /* Initialize array of ngSubscriptInformation_t */
            for (i = 0; i < argument->ngai_nDimensions; i++) {
                result = ngcliSubscriptInformationInitialize(
                    context, &argument->ngai_subscript[i], error);
                if (result != 1) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't initialize the argument subscripts.\n",
                        fName);
                    return 0;
                }
            }
        } else {
            argument->ngai_subscript = NULL;
        }
     
        /* Set each SubscriptInformation */
        childElement = NULL;
        countSubscripts = 0;
        while ((childElement = ngiXMLelementGetNext(
            element, childElement, NGCLL_XML_SUBSCRIPT, log, error)) != NULL) {
     
            /* Convert subscript element to subscript information */
            result = ngcllRemoteClassReadXMLsubscript(
                context, &argument->ngai_subscript[countSubscripts],
                childElement, error);
            if (result != 1) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, 
                    NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't get subscript information in XML.\n", fName);
                return 0;
            }
            countSubscripts++;
        }
        if (countSubscripts != argument->ngai_nDimensions) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Number of subscript mismatch.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLsubscript(
    ngclContext_t *context,
    ngSubscriptInformation_t *subscript,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLsubscript";
    ngiXMLelement_t *childElement;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(subscript != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_SUBSCRIPT) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get subscript size element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_SUB_SIZE, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript size element in XML.\n", fName);
        return 0;
    }

    /* Convert size element to size expression */
    result = ngcllRemoteClassReadXMLexpressionParent(context,
        NGCLL_XML_SUB_SIZE, &(subscript->ngsi_size), childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript size in XML.\n", fName);
        return 0;
    }
    assert(subscript->ngsi_size != NULL);


    /* Get subscript start element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_SUB_START, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript start element in XML.\n", fName);
        return 0;
    }

    /* Convert start element to start expression */
    result = ngcllRemoteClassReadXMLexpressionParent(context,
        NGCLL_XML_SUB_START, &(subscript->ngsi_start), childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript start in XML.\n", fName);
        return 0;
    }
    assert(subscript->ngsi_start != NULL);


    /* Get subscript end element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_SUB_END, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript end element in XML.\n", fName);
        return 0;
    }

    /* Convert end element to end expression */
    result = ngcllRemoteClassReadXMLexpressionParent(context,
        NGCLL_XML_SUB_END, &(subscript->ngsi_end), childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript end in XML.\n", fName);
        return 0;
    }
    assert(subscript->ngsi_end != NULL);


    /* Get subscript skip element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_SUB_SKIP, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript skip element in XML.\n", fName);
        return 0;
    }

    /* Convert skip element to skip expression */
    result = ngcllRemoteClassReadXMLexpressionParent(context, 
        NGCLL_XML_SUB_SKIP, &(subscript->ngsi_skip), childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get subscript skip in XML.\n", fName);
        return 0;
    }
    assert(subscript->ngsi_skip != NULL);

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLexpressionParent(
    ngclContext_t *context,
    char *elementName,
    ngExpressionElement_t **expression,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLexpressionParent";
    ngiXMLelement_t *childElement;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(elementName != NULL);
    assert(expression != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, elementName) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get expression element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_EXPRESSION, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get %s element in XML.\n", fName, elementName);
        return 0;
    }

    /* Convert expression element to expression information */
    result = ngcllRemoteClassReadXMLexpression(
        context, expression, childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get %s in XML.\n", fName, elementName);
        return 0;
    }
    assert(*expression != NULL);

    /* Success */
    return 1;
}


static int
ngcllRemoteClassReadXMLmethodAttribute(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLmethodAttribute";
    ngiXMLelement_t *childElement;
    ngLog_t *log;
    char *str;
    int result;

    /* Check the arguments */
    assert(rMethod != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_METHOD_ATTR) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get shrink */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_SHRINK, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get method attribute shrink in XML.\n", fName);
        return 0;
    }

    /* Set shrink */
    if (strcmp(str, NGCLL_XML_SHRINK_YES) == 0) {
        rMethod->ngrmi_shrink = 1; /* TRUE */
    } else if (strcmp(str, NGCLL_XML_SHRINK_NO) == 0) {
        rMethod->ngrmi_shrink = 0; /* FALSE */
    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get shrink yes or no in XML.\n", fName);
        return 0;
    }

#ifdef NINFGVI_STYLE
    /* Get info type */
    str = ngiXMLattributeGetValue(element, NGCLL_INFO_TYPE, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get mode type in XML.\n", fName);
        return 0;
    }

    /* Set info type */
    result = ngcllRemoteClassReadXMLclassLangConvert(
        context, &(rMethod->ngrmi_infoType), str, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert mode type in XML.\n", fName);
        return 0;
    }
#endif /* NINFGVI_STYLE */

    /* Get order element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_ORDER, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get order element in XML.\n", fName);
        return 0;
    }

    /* Convert order element to order expression */
    result = ngcllRemoteClassReadXMLexpressionParent(
        context, NGCLL_XML_ORDER, &(rMethod->ngrmi_calculationOrder),
        childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get order in XML.\n", fName);
        return 0;
    }
    assert(rMethod->ngrmi_calculationOrder != NULL);

    /* Get method description element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_DESCRIPTION, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get method description element in XML.\n", fName);
        return 0;
    }

    /* Set method description */
    result = ngcllRemoteClassReadXMLdescription(
        context, &(rMethod->ngrmi_description), childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get method description in XML.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}


static int
ngcllRemoteClassReadXMLdescription(
    ngclContext_t *context,
    char **description,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLdescription";
    ngLog_t *log;
    char *str;

    /* Check the arguments */
    assert(description != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_DESCRIPTION) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get description */
    str = ngiXMLelementGetCdata(element, log, error);

    /* Set description */
    if (str == NULL) {

        /* description can suppress */     
        *description = NULL;
    } else {
        *description = strdup(str);
        if (*description == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't allocate the storage for the description.\n",
                fName);
            return 0;
        } 
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLclassAttribute(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLclassAttribute";
    ngiXMLelement_t *childElement;
    ngLog_t *log;
    char *str;
    int result;

    /* Check the arguments */
    assert(rcInfo != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_CLASS_ATTR) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get backend */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_BACKEND, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get backend in XML.\n", fName);
        return 0;
    }

    /* Set backend */
    result = ngcllRemoteClassReadXMLbackendConvert(
        context, &(rcInfo->ngrci_backend), str, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert backend in XML.\n", fName);
        return 0;
    }

    /* Get language */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_LANGUAGE, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class language in XML.\n", fName);
        return 0;
    }

    /* Set language */
    result = ngcllRemoteClassReadXMLclassLangConvert(
        context, &(rcInfo->ngrci_language), str, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert backend in XML.\n", fName);
        return 0;
    }

    /* Get class description element */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_DESCRIPTION, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class description element in XML.\n", fName);
        return 0;
    }

    /* Set class description */
    result = ngcllRemoteClassReadXMLdescription(
        context, &(rcInfo->ngrci_description), childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get class method description in XML.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}


/**
 *  Expression tree traversing functions.
 */
static int
ngcllRemoteClassReadXMLexpression(
    ngclContext_t *context,
    ngExpressionElement_t **expression,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLexpression";
    int eIndex, eSize; /* Remaind expression array's index and size */
    ngLog_t *log;
    int result;
    int i;

    /* Check the arguments */
    assert(expression != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_EXPRESSION) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get required expression element array size */
    result = ngcllRemoteClassReadXMLexpressionElementSizeCheck(
        context, element, &eSize, error);
    if ((result != 1) || (eSize < 1)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression size.\n", fName);
        return 0;
    }

    /* Allocate array of ngExpressionElement_t */
    *expression = ngcliExpressionElementAllocate(
        context, eSize, error);
    if (*expression == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for expression.\n",
            fName);
        return 0;
    }

    /* Initialize array of ngExpressionElement_t */
    for (i = 0; i < eSize; i++) {
        result = ngcliExpressionElementInitialize(
            context, &(*expression)[i], error);
        if (*expression == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't initialize the expression.\n",
                fName);
            return 0;
        }
    }

    /* Traverse XML expression tree and fill out expression array */
    eIndex = 0;
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, *expression, &eIndex, eSize, element, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't traverse expression tree.\n",
            fName);
        return 0;
    }

    /* Write final expression element (termination element) */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, *expression, &eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_END, 0, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't add expression.\n",
            fName);
        return 0;
    }
    if (eIndex != eSize) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: expression size mismatch, incomplete expression.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLtraverseExpression(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLtraverseExpression";
    ngcllExpressionTraverseFunc_t traverseFunc;
    ngiXMLelement_t *childElement;
    ngLog_t *log;
    int i, result;

    traverseFunc = NULL;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_EXPRESSION) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Find ExpressionElement */
    childElement = NULL;
    for (i = 0; traverseTable[i].ngett_funcString != NULL; i++) {
        childElement = ngiXMLelementGetNext(element, NULL,
                             traverseTable[i].ngett_funcString, log, error);

        /* found ExpressionElement */
        if (childElement != NULL) {
            traverseFunc = traverseTable[i].ngett_func;
            break;
        }
    }

    /* Not found ExpressionElement */
    if (childElement == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression element in XML.\n", fName);
        return 0;
    }
    assert(traverseFunc != NULL);

    /* Do each traverse function */
    result = (*traverseFunc)(
        context, expression, eIndex, eSize, childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get expression in XML.\n", fName);
            return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLtraverseExpressionMono(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLtraverseExpressionMono";
    ngExpressionOperationCode_t op;
    ngiXMLelement_t *childElement;
    ngLog_t *log;
    char *str;
    int result;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_ARITH_MONO) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get operator string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_EXP_NAME, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression operator in XML.\n", fName);
        return 0;
    }

    /* Get operator number */
    result = ngcllRemoteClassReadXMLoperatorConvert(
        context, &op, str, opTableMono, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert expression operator in XML.\n", fName);
        return 0;
    }

    /* Get element for operator's argument */
    childElement = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_EXPRESSION, log, error);
    if (childElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression element in XML.\n", fName);
        return 0;
    }

    /* Traverse child XML expression */
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, expression, eIndex, eSize, childElement, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't traverse expression tree.\n",
            fName);
        return 0;
    }

    /* Add operator to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_OPCODE, op, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't add expression.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}


static int
ngcllRemoteClassReadXMLtraverseExpressionBi(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLtraverseExpressionBi";
    ngiXMLelement_t *childElement1, *childElement2;
    ngExpressionOperationCode_t op;
    ngLog_t *log;
    char *str;
    int result;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eIndex != NULL);
    assert(eSize > 0);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_ARITH_BI) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get operator string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_EXP_NAME, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression operator in XML.\n", fName);
        return 0;
    }

    /* Get operator number */
    result = ngcllRemoteClassReadXMLoperatorConvert(
        context, &op, str, opTableBi, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert expression operator in XML.\n", fName);
        return 0;
    }

    /* Get 1st element for operator's argument */
    childElement1 = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_EXPRESSION, log, error);
    if (childElement1 == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression element in XML.\n", fName);
        return 0;
    }

    /* Get 2nd element for operator's argument */
    childElement2 = ngiXMLelementGetNext(element, childElement1,
                                     NGCLL_XML_EXPRESSION, log, error);
    if (childElement2 == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression element in XML.\n", fName);
        return 0;
    }

    /* Traverse 2nd child XML expression */
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, expression, eIndex, eSize, childElement2, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't traverse expression tree.\n",
            fName);
        return 0;
    }

    /* Traverse 1st child XML expression */
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, expression, eIndex, eSize, childElement1, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't traverse expression tree.\n",
            fName);
        return 0;
    }

    /* Add operator to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_OPCODE, op, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't add expression.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLtraverseExpressionTri(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLtraverseExpressionTri";
    ngiXMLelement_t *childElement1, *childElement2, *childElement3;
    ngExpressionOperationCode_t op;
    ngLog_t *log;
    char *str;
    int result;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_ARITH_TRI) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get operator string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_EXP_NAME, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression operator in XML.\n", fName);
        return 0;
    }

    /* Get operator number */
    result = ngcllRemoteClassReadXMLoperatorConvert(
        context, &op, str, opTableTri, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert expression operator in XML.\n", fName);
        return 0;
    }

    /* Get 1st element for operator's argument */
    childElement1 = ngiXMLelementGetNext(element, NULL,
                                     NGCLL_XML_EXPRESSION, log, error);
    if (childElement1 == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression element in XML.\n", fName);
        return 0;
    }

    /* Get 2nd element for operator's argument */
    childElement2 = ngiXMLelementGetNext(element, childElement1,
                                     NGCLL_XML_EXPRESSION, log, error);
    if (childElement2 == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression element in XML.\n", fName);
        return 0;
    }

    /* Get 3rd element for operator's argument */
    childElement3 = ngiXMLelementGetNext(element, childElement2,
                                     NGCLL_XML_EXPRESSION, log, error);
    if (childElement3 == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression element in XML.\n", fName);
        return 0;
    }

    /* Traverse 3rd child XML expression */
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, expression, eIndex, eSize, childElement3, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't traverse expression tree.\n",
            fName);
        return 0;
    }

    /* Traverse 2nd child XML expression */
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, expression, eIndex, eSize, childElement2, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't traverse expression tree.\n",
            fName);
        return 0;
    }

    /* Traverse 1st child XML expression */
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, expression, eIndex, eSize, childElement1, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't traverse expression tree.\n",
            fName);
        return 0;
    }

    /* Add operator to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_OPCODE, op, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't add expression.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLtraverseExpressionImmediate(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLtraverseExpressionImmediate";
    ngLog_t *log;
    char *str;
    int value;
    int result;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_IMMEDIATE) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get value string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_EXP_VAL, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression value in XML.\n", fName);
        return 0;
    }

    /* Set value */
    value = (int)strtol(str, NULL, 10);

    /* Add value to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_CONSTANT, value, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't add expression.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLtraverseExpressionScalarref(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLtraverseExpressionScalarref";
    ngLog_t *log;
    char *str;
    int value;
    int result;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_SCALARREF) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Get value string */
    str = ngiXMLattributeGetValue(element, NGCLL_XML_EXP_VAL, log, error);
    if (str == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get expression value in XML.\n", fName);
        return 0;
    }

    /* Set value */
    value = (int)strtol(str, NULL, 10);

    /* Add value to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, value, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't add expression.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLtraverseExpressionNoValue(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLtraverseExpressionNoValue";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (strcmp(element->ngxe_name, NGCLL_XML_NO_VALUE) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Add value to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_NONE, 0, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't add expression.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLexpressionElementAdd(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngExpressionValueType_t valueType,
    int value,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLexpressionElementAdd";
    ngLog_t *log;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    log = context->ngc_log;

    /* Register value data */
    (expression + *eIndex)->ngee_valueType = valueType;
    (expression + *eIndex)->ngee_value = value;

    (*eIndex)++;
    if (*eIndex > eSize) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: expression array overflow.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLexpressionElementSizeCheck(
    ngclContext_t *context,
    ngiXMLelement_t *element,
    int *eSize,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLexpressionElementSizeCheck";
    ngLog_t *log;
    int count;

    /* Check the arguments */
    assert(element != NULL);
    assert(eSize != NULL);
    log = context->ngc_log;

    count = ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
                context, element, error);
    if (count < 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't count expression operators and values.\n", fName);
        return 0;
    }

    *eSize = count + 1; /* + 1 is for VALUE_TYPE_END */

    /* Success */
    return 1;
}

/**
 * return count of operators and values.
 * if error occur, return -1
 */
static int
ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
    ngclContext_t *context,
    ngiXMLelement_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLexpressionElementSizeCheckSub";
    ngiXMLelement_t *childElement, *childElement2;
    ngLog_t *log;
    int count, sum;

    log = context->ngc_log;

    if (strcmp(element->ngxe_name, NGCLL_XML_EXPRESSION) == 0) {

        /* If expression, child is 1 */
        /* Get child */
        childElement = ngiXMLelementGetNext(element, NULL, NULL, log, error);
        if (childElement == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Expression element has no child element.\n", fName);
            return -1;
        }

        childElement2 = ngiXMLelementGetNext(element,
                                        childElement, NULL, log, error);
        if (childElement2 != NULL) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Expression element has several child element.\n",
                fName);
            return -1;
        }

        count = ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
            context, childElement, error);
        if (count < 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't count expression operators and values.\n",
                fName);
            return -1;
        }

        /* Success */
        return count;
    }

    /* else, Not expression */

    /* count children */
    sum = 1; /* This element */
    childElement = NULL;
    while ((childElement = ngiXMLelementGetNext(
        element, childElement, NULL, log, error)) != NULL) {

        /* count each child operators and values */
        count = ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
            context, childElement, error);
        if (count < 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't count expression operators and values.\n",
                fName);
            return -1;
        }

        sum += count;
    }

    /* Success */
    return sum;
}


/**
 * Utility functions.
 */

static int
ngcllRemoteClassReadXMLioModeConvert(
    ngclContext_t *context,
    ngArgumentIOmode_t *ioMode,
    char *ioModeStr,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLioModeConvert";
    ngLog_t *log;
    int i;

    /* Check the arguments */
    assert(ioMode != NULL);
    assert(ioModeStr != NULL);
    log = context->ngc_log;

    /* Convert string to enum (ngArgumentIOmode_t) */
    for (i = 0; ioModeTable[i].ngrmt_ioModeString != NULL; i++) {
        if (strcmp(ioModeTable[i].ngrmt_ioModeString, ioModeStr) == 0) {
            *ioMode = ioModeTable[i].ngrmt_mode;

            /* Success */
            return 1;
        }
    }

    /* Not found */
    *ioMode = (ngArgumentIOmode_t) -1; /* NG_ARGUMENT_IO_MODE_ERROR */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
        NULL, "%s: \"%s\" No such mode type string.\n",
        fName, ioModeStr);
    return 0;
}

static int
ngcllRemoteClassReadXMLdataTypeConvert(
    ngclContext_t *context,
    ngArgumentDataType_t *dataType,
    char *dataTypeStr,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLdataTypeConvert";
    ngLog_t *log;
    int i;

    /* Check the arguments */
    assert(dataType != NULL);
    assert(dataTypeStr != NULL);
    log = context->ngc_log;

    /* Convert string to enum (ngArgumentDataType_t) */
    for (i = 0; dataTypeTable[i].ngrdt_dataTypeString != NULL; i++) {
        if (strcmp(dataTypeTable[i].ngrdt_dataTypeString, dataTypeStr) == 0) {

            /* found */
            *dataType = dataTypeTable[i].ngrdt_data;

            /* Success */
            return 1;
        }
    }

    /* Not found */
    *dataType = (ngArgumentDataType_t) -1; /* NG_ARGUMENT_DATA_TYPE_ERROR */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
        NULL, "%s: \"%s\" No such data type string.\n",
        fName, dataTypeStr);
    return 0;
}

static int
ngcllRemoteClassReadXMLbackendConvert(
    ngclContext_t *context,
    ngBackend_t *backend,
    char *backendStr,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLbackendConvert";
    ngLog_t *log;
    int i;

    /* Check the arguments */
    assert(backend != NULL);
    assert(backendStr != NULL);
    log = context->ngc_log;

    /* Convert string to enum (ngBackend_t) */
    for (i = 0; backendTable[i].ngrbt_backendString != NULL; i++) {
        if (strcmp(backendTable[i].ngrbt_backendString, backendStr) == 0) {

            /* found */
            *backend = backendTable[i].ngrbt_backend;

            /* Success */
            return 1;
        }
    }

    /* Not found */
    *backend = (ngBackend_t) -1; /* NG_BACKEND_ERROR */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
        NULL, "%s: \"%s\" No such backend string.\n",
        fName, backendStr);
    return 0;
}

static int
ngcllRemoteClassReadXMLclassLangConvert(
    ngclContext_t *context,
    ngClassLanguage_t *classLang,
    char *classLangStr,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLclassLangConvert";
    ngLog_t *log;
    int i;

    /* Check the arguments */
    assert(classLang != NULL);
    assert(classLangStr != NULL);
    log = context->ngc_log;

    /* Convert string to enum (ngClassLanguage_t) */
    for (i = 0; classLangTable[i].ngrclt_classLangString != NULL; i++) {
        if (strcmp(classLangTable[i].ngrclt_classLangString, classLangStr)
             == 0) {

            /* found */
            *classLang = classLangTable[i].ngrclt_classLang;

            /* Success */
            return 1;
        }
    }

    /* Not found */
    *classLang = (ngClassLanguage_t) -1; /* NG_CLASS_LANGUAGE_ERROR */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
        NULL, "%s: \"%s\" No such class language string.\n",
        fName, classLangStr);
    return 0;
}

static int
ngcllRemoteClassReadXMLoperatorConvert(
    ngclContext_t *context,
    ngExpressionOperationCode_t *operator,
    char *operatorStr,
    ngcllExpressionOperatorType_t *operatorTable,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLoperatorConvert";
    ngLog_t *log;
    int i;

    /* Check the arguments */
    assert(operator != NULL);
    assert(operatorStr != NULL);
    assert(operatorTable != NULL);
    log = context->ngc_log;

    /* Convert string to enum (ngClassLanguage_t) */
    for (i = 0; operatorTable[i].ngeot_opString != NULL; i++) {
        if (strcmp(operatorTable[i].ngeot_opString, operatorStr) == 0) {

            /* found */
            *operator = operatorTable[i].ngeot_opCode;

            /* Success */
            return 1;
        }
    }

    /* Not found */
    *operator = (ngExpressionOperationCode_t) -1;
        /* NG_EXPRESSION_OPCODE_ERROR */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
        NULL, "%s: \"%s\" No such operator string.\n",
        fName, operatorStr);
    return 0;
}

