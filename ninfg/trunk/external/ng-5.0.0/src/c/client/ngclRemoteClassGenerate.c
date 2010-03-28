/*
 * $RCSfile: ngclRemoteClassGenerate.c,v $ $Revision: 1.9 $ $Date: 2008/02/07 06:22:03 $
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
 * Function of RemoteClassInformationGenerate
 */

#include "ng.h"
#include "ngXML.h"

NGI_RCSID_EMBED("$RCSfile: ngclRemoteClassGenerate.c,v $ $Revision: 1.9 $ $Date: 2008/02/07 06:22:03 $")

/**
 *
 * XML tree traverse function call tree
 *
 * Ninf-G Remote Executable Information
 *
 * namespace is http://ninf.apgrid.org/2006/12/RemoteExecutableInformation
 *
 * <RemoteExecutableInformation>
 *     <hostName> hostname text </hostname>
 *     <path> path text </text>
 *     <ngdir> ngdir text </ngdir>
 *     [Ninf-G Remote Class Information]
 *
 * 
 * [Ninf-G Remote Class Information]
 *
 * namespace is http://ninf.apgrid.org/2006/12/RemoteClassInformation
 *
 * <class>
 *     <method>
 *         <argument ioMode="ioMode" dataType="dataType">
 *             <subscript>
 *                 <size>
 *                     <expression>
 *                 <start>
 *                     <expression>
 *                 <end>
 *                     <expression>
 *                 <skip>
 *                     <expression>
 *         <methodAttribute shrink="0|1">
 *             <order>
 *                 <expression>
 *             <description>
 *     <classAttribute>
 *         <description>
 *
 * <expression>
 *     expressionTraverseMono
 *     expressionTraverseBi
 *     expressionTraverseTri
 *     expressionTraverseImmediate
 *     expressionTraverseScalarref
 *     expressionTraverseNoValue
 */

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

typedef int (*ngcllExpressionTraverseFunc_t) (
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
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
ngcllRemoteExecutableReadXML(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLclass(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLmethod(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLargument(
    ngclContext_t *context,
    ngArgumentInformation_t *argument,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLsubscript(
    ngclContext_t *context,
    ngSubscriptInformation_t *subscript,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLexpressionParent(
    ngclContext_t *context,
    char *elementName,
    ngExpressionElement_t **expression,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLexpression(
    ngclContext_t *context,
    ngExpressionElement_t **expression,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLmethodAttribute(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLdescription(
    ngclContext_t *context,
    char **description,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLclassAttribute(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpression(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionMono(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionBi(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionTri(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionSub(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    char *tagName,
    int nArg,
    ngcllExpressionOperatorType_t *,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionImmediate(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionScalarref(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error);

static int
ngcllRemoteClassReadXMLtraverseExpressionNoValue(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
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
    ngiXMLitem_t *element,
    int *eSize,
    int *error);

static int
ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
    ngclContext_t *context,
    ngiXMLitem_t *element,
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

#define NGCLL_XML_NS_REMOTE_CLASS_INFORMATION      "http://ninf.apgrid.org/2006/12/RemoteClassInformation"
#define NGCLL_XML_NS_REMOTE_EXECUTABLE_INFORMATION "http://ninf.apgrid.org/2006/12/RemoteExecutableInformation"

#define NGCLL_XML_REI           "RemoteExecutableInformation"
#define NGCLL_XML_HOSTNAME      "hostName"
#define NGCLL_XML_PATH          "path"
#define NGCLL_XML_NGDIR         "ngdir"

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

#define NGCLL_OP_MAX_ARGS 3


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

/**
 * RemoteClassInformationGenerate
 *  creates ngRemoteClassInformation_t from XML string.
 */
int
ngcliParseRemoteExecutableInformation(
    ngclContext_t *context,
    ngiLineList_t *ll,
    ngclExecutablePathInformation_t *epInfo,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    ngiXMLparser_t *parser = NULL;
    int result;
    char *p;
    ngLog_t *log;
    ngiXMLitem_t *item = NULL;
    int ret = 0;
    static const char fName[] = "ngcliParseRemoteExecutableInformation";

    log = context->ngc_log;

    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct XML parser.\n");
        goto finalize;
    }

    p = NULL;
    while (1) {
        p = ngiLineListLineGetNext(ll, p, log, error);
        if (p == NULL) {
            break;
        }
        result = ngiXMLparserParse(parser, p, strlen(p), 0, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Fails to parse XML document.\n");
            goto finalize;
        }
    }
    result = ngiXMLparserParse(parser, NULL, 0, 1, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Fails to parse XML document.\n");
        goto finalize;
    }

    item = ngiXMLparserGetRootElement(parser, log, error);
    if (item == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get root element of XML document.\n");
        goto finalize;
    }

    result = ngcllRemoteExecutableReadXML(context, epInfo, rcInfo, item, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get th Remote Executable Information from XML document.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngiXMLparserDestruct(parser, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the XML parser.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Generate rcInfo
 */
int
ngcliRemoteClassInformationGenerate(
    ngclContext_t *context,
    char *string,
    int len,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    ngiXMLparser_t *parser = NULL;
    int result;
    ngLog_t *log;
    ngiXMLitem_t *item = NULL;
    int ret = 0;
    static const char fName[] = "ngcliRemoteClassInformationGenerate";

    assert(context != NULL);
    assert(string != NULL);
    assert(len > 0);

    log = context->ngc_log;


    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct XML parser.\n");
        goto finalize;
    }

    result = ngiXMLparserParse(parser, string, len, 1, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Fails to parse XML document.\n");
        goto finalize;
    }

    item = ngiXMLparserGetRootElement(parser, log, error);
    if (item == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get root element of XML document.\n");
        goto finalize;
    }

    result = ngcllRemoteClassReadXMLclass(context, rcInfo, item, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get th Remote Executable Information from XML document.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngiXMLparserDestruct(parser, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the XML parser.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

static int
ngcllRemoteExecutableReadXML(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLitem_t *item,
    int *error)
{
    static const char fName[] = "ngcllRemoteExecutableReadXML";
    ngiXMLitem_t *childElement;
    ngLog_t *log;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;
    char *rei = NGCLL_XML_NS_REMOTE_EXECUTABLE_INFORMATION;
    int result;

    assert(rcInfo != NULL);
    assert(item != NULL);

    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(item, rei, NGCLL_XML_REI)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    childElement = ngiXMLelementGetFirstChild(item, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rei, NGCLL_XML_HOSTNAME))) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get <%s:%s> element from XML.\n",
            rei, NGCLL_XML_HOSTNAME); 
        return 0;
    }
    epInfo->ngepi_hostName = ngiXMLelementGetText(childElement, log, error);
    if (epInfo->ngepi_hostName == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get hostname from XML.\n"); 
        return 0;
    }

    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rei, NGCLL_XML_PATH))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get path from XML.\n"); 
        return 0;
    }
    epInfo->ngepi_path = ngiXMLelementGetText(childElement, log, error);
    if (epInfo->ngepi_path == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get hostname from XML.\n"); 
        return 0;
    }

    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rei, NGCLL_XML_NGDIR))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get path from XML.\n"); 
        return 0;
    }
    /* Not used? */
    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_CLASS))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class from XML\n"); 
        return 0;
    }

    result = ngcllRemoteClassReadXMLclass(context, rcInfo, childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class from XML.\n"); 
        return 0;
    }

    epInfo->ngepi_backend   = rcInfo->ngrci_backend;
    epInfo->ngepi_className = ngiStrdup(rcInfo->ngrci_className, log, error);
    if (epInfo->ngepi_className == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't duplicate classname.\n"); 
        return 0;
    }

    /* Not check last */

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLclass(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLitem_t *item,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLclass";
    ngiXMLitem_t *childElement;
    ngLog_t *log;
    int result, countMethods;
    int i;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    assert(rcInfo != NULL);
    assert(item != NULL);

    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(item, rci, NGCLL_XML_CLASS)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get class name string */
    rcInfo->ngrci_className = ngiXMLelementDupAttributeValue(item, 
        NULL, NGCLL_XML_CLASS_NAME, log, error);
    if (rcInfo->ngrci_className == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class name in XML.\n"); 
        return 0;
    }

    /* Get version string */
    rcInfo->ngrci_version = ngiXMLelementDupAttributeValue(item, 
        NULL, NGCLL_XML_VERSION, log, error);
    if (rcInfo->ngrci_version == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class version in XML.\n"); 
        return 0;
    }

    /* Get number of methods */
    result = ngiXMLelementGetAttributeValueAsInt(item, 
        NULL, NGCLL_XML_NUM_METHODS, 
        1, INT_MAX, &rcInfo->ngrci_nMethods, log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get value of %s attribute\n.\n", NGCLL_XML_NUM_METHODS); 
        return 0;
    }

    /* Allocate array of ngRemoteMethod_t */
    rcInfo->ngrci_method = ngcliRemoteMethodInformationAllocate(
        context, rcInfo->ngrci_nMethods, error);
    if (rcInfo->ngrci_method == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Remote Method\n"); 
        return 0;
    }

    /* Initialize array of ngRemoteMethod_t */
    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
        result = ngcliRemoteMethodInformationInitialize(
            context, &rcInfo->ngrci_method[i], error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Remote Method\n"); 
            return 0;
        }
    }

    /* Set each RemoteMethod information */
    childElement = NULL;
    countMethods = 0;

    assert(item->ngxi_type == NGI_XML_ITEM_ELEMENT);

    countMethods = 0;
    childElement = ngiXMLelementGetFirstChild(item, 1, log, error);
    while ((countMethods < rcInfo->ngrci_nMethods) && (childElement != NULL)) {
        if (NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_METHOD)) {
            /* Convert method item to metod information */
            result = ngcllRemoteClassReadXMLmethod(
                context ,&rcInfo->ngrci_method[countMethods], childElement, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get method information in XML.\n"); 
                return 0;
            }
            countMethods++;
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "There is unexpect item in the XML.\n"); 
            return 0;
        }
        childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    }
    if (rcInfo->ngrci_nMethods != countMethods) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Number of method mismatch.\n");
        return 0;
    }

    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_CLASS_ATTR))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "There is not a class attribute in XML.\n"); 
        return 0;
    }

    result = ngcllRemoteClassReadXMLclassAttribute(
        context, rcInfo, childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class attribute in XML.\n"); 
        return 0;
    }

    /* Not check last */

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLmethod(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLmethod";
    ngiXMLitem_t *childElement;
    int result, countArguments;
    ngLog_t *log;
    char *str;
    int i;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(rMethod != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_METHOD)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get method name string */
    rMethod->ngrmi_methodName = ngiXMLelementDupAttributeValue(element, 
        NULL, NGCLL_XML_METHOD_NAME, log, error);
    if (rMethod->ngrmi_methodName == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get method name in XML.\n"); 
        return 0;
    }

    /* Get method ID */
    str = ngiXMLelementGetAttribute(element, 
        NULL, NGCLL_XML_METHOD_ID, log, error);
    if (str == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get method ID in XML.\n"); 
        return 0;
    }
    result = ngiStringToInt(str, &rMethod->ngrmi_methodID, log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Method ID is invalid.\n"); 
        return 0;
    }
    if (rMethod->ngrmi_methodID < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Method ID invalid.\n"); 
        return 0;
    }

    /* Get number of arguments */
    rMethod->ngrmi_nArguments = ngiXMLelementGetNumberOfChild(
          element, rci, NGCLL_XML_ARGUMENT, log, error);
    if (rMethod->ngrmi_nArguments < 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get number of <%s>.\n", NGCLL_XML_ARGUMENT); 
        return 0;
    }

    if (rMethod->ngrmi_nArguments > 0) {
        /* Allocate array of ngArgumentInformation_t */
        rMethod->ngrmi_arguments = ngcliArgumentInformationAllocate(
            context, rMethod->ngrmi_nArguments, error);
        if (rMethod->ngrmi_arguments == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate the storage for method arguments.\n"); 
            return 0;
        }

        /* Initialize array of ngArgumentInformation_t */
        for (i = 0; i < rMethod->ngrmi_nArguments; i++) {
            result = ngcliArgumentInformationInitialize(
                context, &rMethod->ngrmi_arguments[i], error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't initialize the method arguments.\n"); 
                return 0;
            }
        }
    }

    /* Set each Argument Information */
    assert(element->ngxi_type == NGI_XML_ITEM_ELEMENT);

    countArguments = 0;
    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    while ((countArguments < rMethod->ngrmi_nArguments) && (childElement != NULL)) {
        if (NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_ARGUMENT)) {
            /* Convert argument element to argument information */
            result = ngcllRemoteClassReadXMLargument(
                context, &rMethod->ngrmi_arguments[countArguments],
                childElement, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't get argument information in XML.\n"); 
                return 0;
            }
            countArguments++;
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "There is unexpect item in the XML.\n"); 
            return 0;
        }
        childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    }

    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_METHOD_ATTR))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "There is not a method attribute in XML.\n"); 
        return 0;
    }

    result = ngcllRemoteClassReadXMLmethodAttribute(
        context, rMethod, childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get method attribute in XML.\n"); 
        return 0;
    }

    /* Not check last */

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLargument(
    ngclContext_t *context,
    ngArgumentInformation_t *argument,
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLargument";
    ngiXMLitem_t *childElement;
    int countSubscripts;
    /*
    int nCallbacks;
    */
    ngLog_t *log;
    int result;
    char *str;
    int i;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(argument != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_ARGUMENT)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get mode type */
    str = ngiXMLelementGetAttribute(element, NULL, NGCLL_XML_IO_MODE, log, error);
    if (str == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get mode type in XML.\n"); 
        return 0;
    }

    /* Set mode type */
    result = ngcllRemoteClassReadXMLioModeConvert(
        context, &(argument->ngai_ioMode), str, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert mode type in XML.\n"); 
        return 0;
    }

    /* Get data type */
    str = ngiXMLelementGetAttribute(element, NULL, NGCLL_XML_DATA_TYPE, log, error);
    if (str == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get data type in XML.\n"); 
        return 0;
    }

    /* Set data type */
    result = ngcllRemoteClassReadXMLdataTypeConvert(
        context, &(argument->ngai_dataType), str, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert mode type in XML.\n"); 
        return 0;
    }

    /* Get number of dimensions */
    argument->ngai_nDimensions = ngiXMLelementGetNumberOfChild(
          element, rci, NGCLL_XML_SUBSCRIPT, log, error);
    if (argument->ngai_nDimensions < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get The number of argument dimension.\n"); 
        return 0;
    }

    argument->ngai_subscript = NULL;
    argument->ngai_callback = NULL;
    /* if data type is callback, then proceed as callback argument */
    if (argument->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
        if (argument->ngai_nDimensions > 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Callback in multi dimension.\n"); 
            return 0;
        }

        /* Set each SubscriptInformation */
        childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
        if ((childElement == NULL) ||
            (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_METHOD))) {
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "There is no Remote Method Information for callback in XML.\n"); 
            return 0;
        }

        /* Allocate ngRemoteMethod_t */
        argument->ngai_callback = ngcliRemoteMethodInformationAllocate(
            context, 1, error);
        if (argument->ngai_callback == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate the storage for Remote Method\n"); 
            return 0;
        }

        /* Initialize ngRemoteMethod_t */
        result = ngcliRemoteMethodInformationInitialize(
            context, argument->ngai_callback, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Remote Method\n"); 
            return 0;
        }

        /* Convert method element to metod information */
        result = ngcllRemoteClassReadXMLmethod(
            context, argument->ngai_callback, childElement, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get method information in XML.\n"); 
            return 0;
        }
    } else {
        /* if date type is not callback, then proceed normal subscript */
        if (argument->ngai_nDimensions > 0) {
            /**
             *  Allocate array of ngSubscriptInformation_t
             *   for each dimension expression
             */
            argument->ngai_subscript = ngcliSubscriptInformationAllocate(
                context, argument->ngai_nDimensions, error);
            if (argument->ngai_subscript == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't allocate the storage for argument subscripts.\n"); 
                return 0;
            }

            /* Initialize array of ngSubscriptInformation_t */
            for (i = 0; i < argument->ngai_nDimensions; i++) {
                result = ngcliSubscriptInformationInitialize(
                    context, &argument->ngai_subscript[i], error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "Can't initialize the argument subscripts.\n"); 
                    return 0;
                }
            }
        }
     
        /* Set each SubscriptInformation */
        countSubscripts = 0;
        childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
        while ((countSubscripts < argument->ngai_nDimensions) && 
               (childElement != NULL)) {
            if (NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_SUBSCRIPT)) {
                result = ngcllRemoteClassReadXMLsubscript(
                    context, &argument->ngai_subscript[countSubscripts],
                    childElement, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "Can't get subscript information in XML.\n"); 
                    return 0;
                }
                countSubscripts++;
            } else {
                NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "There is unexpect item in the XML.\n"); 
                return 0;
            }
            childElement = ngiXMLitemGetNext(childElement, 1, log, error);
        }
        if (countSubscripts != argument->ngai_nDimensions) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Number of subscript mismatch.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLsubscript";
    ngiXMLitem_t *childElement;
    ngLog_t *log;
    int result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(subscript != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_SUBSCRIPT)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get subscript size element */
    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_SUB_SIZE))) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript size element in XML.\n"); 
        return 0;
    }
    result = ngcllRemoteClassReadXMLexpressionParent(context,
        NGCLL_XML_SUB_SIZE, &(subscript->ngsi_size), childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript size in XML.\n"); 
        return 0;
    }
    assert(subscript->ngsi_size != NULL);

    /* Get subscript start element */
    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_SUB_START))) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript start element in XML.\n"); 
        return 0;
    }
    result = ngcllRemoteClassReadXMLexpressionParent(context,
        NGCLL_XML_SUB_START, &(subscript->ngsi_start), childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript start in XML.\n"); 
        return 0;
    }
    assert(subscript->ngsi_start != NULL);

    /* Get subscript end element */
    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_SUB_END))) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript end element in XML.\n"); 
        return 0;
    }
    result = ngcllRemoteClassReadXMLexpressionParent(context,
        NGCLL_XML_SUB_END, &(subscript->ngsi_end), childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript end in XML.\n"); 
        return 0;
    }
    assert(subscript->ngsi_end != NULL);

    /* Get subscript skip element */
    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_SUB_SKIP))) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript end element in XML.\n"); 
        return 0;
    }
    result = ngcllRemoteClassReadXMLexpressionParent(context, 
        NGCLL_XML_SUB_SKIP, &(subscript->ngsi_skip), childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get subscript skip in XML.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLexpressionParent";
    ngiXMLitem_t *childElement;
    ngLog_t *log;
    int result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(elementName != NULL);
    assert(expression != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, elementName)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get expression element */
    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_EXPRESSION))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get <%s> element in <%s> element.\n",
            NGCLL_XML_EXPRESSION, elementName); 
        return 0;
    }

    /* Convert expression element to expression information */
    result = ngcllRemoteClassReadXMLexpression(
        context, expression, childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get %s in <%s> element.\n",
            elementName, NGCLL_XML_EXPRESSION); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLmethodAttribute";
    ngiXMLitem_t *childElement;
    ngLog_t *log;
    char *str;
    int result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(rMethod != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_METHOD_ATTR)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get shrink */
    str = ngiXMLelementGetAttribute(element, NULL, NGCLL_XML_SHRINK, log, error);
    if (str == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get method attribute shrink in XML.\n"); 
        return 0;
    }

    /* Set shrink */
    if (strcmp(str, NGCLL_XML_SHRINK_YES) == 0) {
        rMethod->ngrmi_shrink = 1; /* TRUE */
    } else if (strcmp(str, NGCLL_XML_SHRINK_NO) == 0) {
        rMethod->ngrmi_shrink = 0; /* FALSE */
    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get shrink yes or no in XML.\n"); 
        return 0;
    }

    /* Get order element */
    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_ORDER))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get order element in XML.\n"); 
        return 0;
    }

    /* Convert order element to order expression */
    result = ngcllRemoteClassReadXMLexpressionParent(
        context, NGCLL_XML_ORDER, &(rMethod->ngrmi_calculationOrder),
        childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get order in XML.\n"); 
        return 0;
    }
    assert(rMethod->ngrmi_calculationOrder != NULL);

    /* Get method description element */
    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_DESCRIPTION))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get method description element in XML.\n"); 
        return 0;
    }

    /* Set method description */
    result = ngcllRemoteClassReadXMLdescription(
        context, &(rMethod->ngrmi_description), childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get method description in XML.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLdescription(
    ngclContext_t *context,
    char **description,
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLdescription";
    ngLog_t *log;
    char *str;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(description != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_DESCRIPTION)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get description */
    str = ngiXMLelementGetText(element, log, error);
    if (str == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get text data from %s element.\n", NGCLL_XML_DESCRIPTION); 
        return 0;
    }
    *description = str;

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLclassAttribute(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLclassAttribute";
    ngiXMLitem_t *childElement;
    ngLog_t *log;
    char *str;
    int result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(rcInfo != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_CLASS_ATTR)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get backend */
    str = ngiXMLelementGetAttribute(element, NULL, NGCLL_XML_BACKEND, log, error);
    if (str == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get backend in XML.\n"); 
        return 0;
    }

    /* Set backend */
    result = ngcllRemoteClassReadXMLbackendConvert(
        context, &(rcInfo->ngrci_backend), str, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert backend in XML.\n"); 
        return 0;
    }

    /* Get language */
    str = ngiXMLelementGetAttribute(element, NULL, NGCLL_XML_LANGUAGE, log, error);
    if (str == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class language in XML.\n"); 
        return 0;
    }

    /* Set language */
    result = ngcllRemoteClassReadXMLclassLangConvert(
        context, &(rcInfo->ngrci_language), str, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert backend in XML.\n"); 
        return 0;
    }

    /* Get class description element */
    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    if ((childElement == NULL) &&
        (!NGI_XML_ELEMENT_IS(childElement, rci, NGCLL_XML_DESCRIPTION))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class description element in XML.\n"); 
        return 0;
    }

    /* Set class description */
    result = ngcllRemoteClassReadXMLdescription(
        context, &(rcInfo->ngrci_description), childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get class method description in XML.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLexpression";
    int eIndex, eSize; /* Remaind expression array's index and size */
    ngLog_t *log;
    int result;
    int i;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(expression != NULL);
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_EXPRESSION)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get required expression element array size */
    result = ngcllRemoteClassReadXMLexpressionElementSizeCheck(
        context, element, &eSize, error);
    if ((result == 0) || (eSize < 1)) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get expression size.\n"); 
        return 0;
    }
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,
        "Expression total is %d.\n", eSize);

    /* Allocate array of ngExpressionElement_t */
    *expression = ngcliExpressionElementAllocate(
        context, eSize, error);
    if (*expression == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for expression.\n"); 
        return 0;
    }

    /* Initialize array of ngExpressionElement_t */
    for (i = 0; i < eSize; i++) {
        result = ngcliExpressionElementInitialize(
            context, &(*expression)[i], error);
        if (*expression == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the expression.\n"); 
            return 0;
        }
    }

    /* Traverse XML expression tree and fill out expression array */
    eIndex = 0;
    result = ngcllRemoteClassReadXMLtraverseExpression(
        context, *expression, &eIndex, eSize, element, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't traverse expression tree.\n"); 
        return 0;
    }

    /* Write final expression element (termination element) */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, *expression, &eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_END, 0, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't add expression.\n"); 
        return 0;
    }
    if (eIndex != eSize) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "expression size mismatch, incomplete expression.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLtraverseExpression";
    ngcllExpressionTraverseFunc_t traverseFunc;
    ngiXMLitem_t *childElement;
    ngLog_t *log;
    int i, result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    traverseFunc = NULL;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_EXPRESSION)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Find ExpressionElement */
    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    if (childElement != NULL) {
        for (i = 0; traverseTable[i].ngett_funcString != NULL; i++) {
            if (NGI_XML_ELEMENT_IS(childElement, rci,
                    traverseTable[i].ngett_funcString)) {
                traverseFunc = traverseTable[i].ngett_func;
                break;
            }
        }
    }
    /* Not found ExpressionElement */
    if (traverseFunc == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get expression element in XML.\n"); 
        return 0;
    }

    /* Do each traverse function */
    result = (*traverseFunc)(
        context, expression, eIndex, eSize, childElement, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get expression in XML.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    return ngcllRemoteClassReadXMLtraverseExpressionSub(
        context, expression, eIndex, eSize, element,
        NGCLL_XML_ARITH_MONO, 1, opTableMono, error);
}


static int
ngcllRemoteClassReadXMLtraverseExpressionBi(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error)
{
    return ngcllRemoteClassReadXMLtraverseExpressionSub(
        context, expression, eIndex, eSize, element,
        NGCLL_XML_ARITH_BI, 2, opTableBi, error);
}

static int
ngcllRemoteClassReadXMLtraverseExpressionTri(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    int *error)
{
    return ngcllRemoteClassReadXMLtraverseExpressionSub(
        context, expression, eIndex, eSize, element,
        NGCLL_XML_ARITH_TRI, 3, opTableTri, error);
}

static int
ngcllRemoteClassReadXMLtraverseExpressionSub(
    ngclContext_t *context,
    ngExpressionElement_t *expression,
    int *eIndex,
    int eSize,
    ngiXMLitem_t *element,
    char *tagName,
    int nArg,
    ngcllExpressionOperatorType_t *opTable,
    int *error)
{
    static const char fName[] = "ngcllRemoteClassReadXMLtraverseExpressionSub";
    ngiXMLitem_t *childElements[NGCLL_OP_MAX_ARGS];
    ngiXMLitem_t *ce;
    ngExpressionOperationCode_t op;
    ngLog_t *log;
    char *str;
    int result;
    int i;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    assert(tagName != NULL);
    assert(nArg >= 0);
    assert(nArg <= NGCLL_OP_MAX_ARGS);

    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, tagName)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get operator string */
    str = ngiXMLelementGetAttribute(element, NULL, NGCLL_XML_EXP_NAME, log, error);
    if (str == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get expression operator in XML.\n"); 
        return 0;
    }

    /* Get operator number */
    result = ngcllRemoteClassReadXMLoperatorConvert(
        context, &op, str, opTable, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert expression operator in XML.\n"); 
        return 0;
    }

    /* Get 1st element for operator's argument */
    i = 0;
    ce = ngiXMLelementGetFirstChild(element, 1, log, error);
    while (i < nArg) {
        if ((ce == NULL) || 
            (!NGI_XML_ELEMENT_IS(ce, rci, NGCLL_XML_EXPRESSION))) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get expression element in XML.\n"); 
            return 0;
        }
        childElements[i] = ce;
        ce = ngiXMLitemGetNext(ce, 1, log, error);
    }

    while (i > 0) {
        i--;
        result = ngcllRemoteClassReadXMLtraverseExpression(
            context, expression, eIndex, eSize, childElements[i], error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't traverse expression tree.\n"); 
            return 0;
        }
    }

    /* Add operator to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_OPCODE, op, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't add expression.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLtraverseExpressionImmediate";
    ngLog_t *log;
    int value;
    int result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_IMMEDIATE)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get value string */
    result = ngiXMLelementGetAttributeValueAsInt(element,
        NULL, NGCLL_XML_EXP_VAL, INT_MIN, INT_MAX, &value, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get expression value in XML.\n"); 
        return 0;
    }

    /* Add value to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_CONSTANT, value, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't add expression.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLtraverseExpressionScalarref";
    ngLog_t *log;
    int value;
    int result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;


    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_SCALARREF)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngiXMLelementGetAttributeValueAsInt(element,
        NULL, NGCLL_XML_EXP_VAL, INT_MIN, INT_MAX, &value, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get expression value in XML.\n"); 
        return 0;
    }

    /* Add value to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, value, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't add expression.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLtraverseExpressionNoValue";
    ngLog_t *log;
    int result;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    /* Check the arguments */
    assert(expression != NULL);
    assert(eSize > 0);
    assert((eIndex != NULL) && (*eIndex < eSize));
    assert(element != NULL);
    log = context->ngc_log;

    /* Check given element is correct */
    if (!NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_NO_VALUE)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Add value to expression array */
    result = ngcllRemoteClassReadXMLexpressionElementAdd(
        context, expression, eIndex, eSize,
        NG_EXPRESSION_VALUE_TYPE_NONE, 0, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't add expression.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "expression array overflow.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteClassReadXMLexpressionElementSizeCheck(
    ngclContext_t *context,
    ngiXMLitem_t *element,
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't count expression operators and values.\n"); 
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
    ngiXMLitem_t *element,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassReadXMLexpressionElementSizeCheckSub";
    ngiXMLitem_t *childElement, *last;
    ngLog_t *log;
    int count, sum;
    char *rci = NGCLL_XML_NS_REMOTE_CLASS_INFORMATION;

    log = context->ngc_log;

    if (NGI_XML_ELEMENT_IS(element, rci, NGCLL_XML_EXPRESSION)) {
        /* If expression, child is 1 */

        /* Get child */
        childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
        if (childElement == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Expression element has no child element.\n"); 
            return -1;
        }

        /* No more child */
        last = ngiXMLitemGetNext(childElement, 1, log, error);
        if (last != NULL) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Expression element has several child element.\n"); 
            return -1;
        }

        count = ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
            context, childElement, error);
        if (count < 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't count expression operators and values.\n"); 
            return -1;
        }

        /* Success */
        return count;
    }

    /* else, Not expression */

    /* count children */
    sum = 1; /* This element */
    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    while (childElement != NULL) {

        /* count each child operators and values */
        count = ngcllRemoteClassReadXMLexpressionElementSizeCheckSub(
            context, childElement, error);
        if (count < 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't count expression operators and values.\n"); 
            return -1;
        }

        sum += count;

        childElement = ngiXMLitemGetNext(childElement, 1, log, error);
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
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "\"%s\" No such mode type string.\n", ioModeStr); 
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
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "\"%s\" No such data type string.\n", dataTypeStr); 
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
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "\"%s\" No such backend string.\n", backendStr); 
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
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "\"%s\" No such class language string.\n", classLangStr); 
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
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "\"%s\" No such operator string.\n", operatorStr); 
    return 0;
}

