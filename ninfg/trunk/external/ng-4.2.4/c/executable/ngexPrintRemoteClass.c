#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngexPrintRemoteClass.c,v $ $Revision: 1.22 $ $Date: 2005/03/22 07:40:07 $";
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
 * Module of Ninf-G Executable.
 */

#include <stdio.h>
#include <unistd.h>
#include "ngEx.h"
#include "ngConfigFile.h"


/**
 * Prototype declaration of APIs
 */
static int ngexlPrintRemoteMethodInformation(
    ngRemoteMethodInformation_t *, int, FILE *, ngLog_t *, int *);
static int ngexlPrintArgumentInformation(
    ngArgumentInformation_t *, int, FILE *, ngLog_t *, int *);
static int ngexlPrintExpression(
    ngExpressionElement_t *, int, FILE *, ngLog_t *, int *);
static int ngexlPrintExpressionValue(
    ngExpressionElement_t *, int *, int, FILE *, ngLog_t *, int *);
static void ngexlPrintBlank(int, FILE *);
static char *ngexlGetModeTypeString(ngArgumentIOmode_t, ngLog_t *, int *);
static char *ngexlGetDataTypeString(ngArgumentDataType_t, ngLog_t *, int *);


/**
 * Print the Remote Class Information to Buffer.
 */
int
ngexiPrintRemoteClassInformationToBuffer(
    ngexiContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    char **funcInfoString,
    ngLog_t *log,
    int *error)
{
    int result;
    FILE *fp = NULL;
    int tmpFileOpen, tmpFileExist, tmpFileNameExist;
    char *tmpFile, buf[NGI_CONFIG_LINE_MAX];
    ngiStringList_t *stringList;
    static const char fName[] = "ngexiPrintRemoteClassInformationToBuffer";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfo != NULL);
    assert(funcInfoString != NULL);

    *funcInfoString = NULL;
    tmpFileNameExist = 0;
    tmpFileOpen = 0;
    tmpFileExist = 0;

    /* Create the temporary file name */
    tmpFile = ngiTemporaryFileCreate(
        context->ngc_lmInfo.nglmi_tmpDir, log, error);
    if (tmpFile == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the Temporary File Name.\n", fName);
        goto error;
    }
    tmpFileNameExist = 1;
    tmpFileExist = 1;

    /* Open the temporary file */
    fp = fopen(tmpFile, "w+");
    if (fp == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Opening temporary file \"%s\" failed.\n", fName, tmpFile);
        goto error;
    }
    tmpFileOpen = 1;

    /* Unlink the temporary file */
    result = unlink(tmpFile);
    tmpFileExist = 0;
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Deleting temporary file \"%s\" failed.\n", fName, tmpFile);
        goto error;
    }

    /* Print Remote Class Information to the temporary file */
    result = ngexiPrintRemoteClassInformation(
        rcInfo, fp, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Output Remote Class Information failed.\n", fName);
        goto error;
    }

    /* Ensure to write buffer */
    result = fflush(fp);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: flushing temporary file \"%s\" failed.\n", fName, tmpFile);
    }

    /* Return to beginning of the temporary file */
    rewind(fp);

    stringList = NULL;

    /* Reads all output */
    while (fgets(buf, NGI_CONFIG_LINE_MAX, fp) != NULL) {
        result = ngiStringListRegister(&stringList, buf);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register StringList.\n", fName);
            goto error;
        }
    }

    /* Close the temporary file */
    result = fclose(fp);
    tmpFileOpen = 0;
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing temporary file \"%s\" failed.\n", fName, tmpFile);
        goto error;
    }

    /* Was read Remote Class Information successful? */
    if (stringList == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
           NG_LOG_LEVEL_ERROR, NULL,
           "%s: No data was read for Remote Class Information.\n", fName);
        goto error;
    }

    /* Merge stringList into one buffer */
    *funcInfoString = ngiStringListMergeToString(stringList);
    if (*funcInfoString == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
           NG_LOG_LEVEL_ERROR, NULL,
           "%s: Can't allocate the storage for string list.\n", fName);
        goto error;
    }

    result = ngiStringListDestruct(stringList);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the StringList.\n", fName);
        goto error;
    }

    globus_libc_free(tmpFile);
    tmpFileNameExist = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (tmpFileOpen != 0) {
        result = fclose(fp);
        if (result != 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Closing temporary file \"%s\" failed.\n", fName, tmpFile);
        }
    }

    if (tmpFileExist != 0) {
        result = unlink(tmpFile);
        if (result != 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Deleting temporary file \"%s\" failed.\n",
               fName, tmpFile);
        }
    }

    if (tmpFileNameExist != 0) {
        globus_libc_free(tmpFile);
    }

    if (*funcInfoString != NULL) {
        globus_libc_free(*funcInfoString);
        *funcInfoString = NULL;
    }

    /* Failed */
    return 0;    
}

/**
 * Print the Remote Class Information.
 */
int
ngexiPrintRemoteClassInformation(
    ngRemoteClassInformation_t *rcInfo,
    FILE *stream,
    ngLog_t *log,
    int *error)
{
    int indent = 1;
    int result;
    int i;
    static const char fName[] = "ngexiPrintRemoteClassInformation";

    /* Check the arguments */
    assert(rcInfo != NULL);
    assert(rcInfo->ngrci_className != NULL);
    assert(rcInfo->ngrci_version != NULL);
    assert(rcInfo->ngrci_description != NULL);
    assert(rcInfo->ngrci_nMethods > 0);
    assert(rcInfo->ngrci_method != NULL);
    assert(stream != NULL);

    /* Print the Remote Class Information */
    fprintf(stream, "  <class name=\"%s\" version=\"%s\" numMethods=\"%d\">\n",
    	rcInfo->ngrci_className, rcInfo->ngrci_version,
	rcInfo->ngrci_nMethods);

    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
    	/* Print the Remote Method Information */
    	result = ngexlPrintRemoteMethodInformation(
	    &rcInfo->ngrci_method[i], indent + 1, stream, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Can't print the Remote Method Information.\n",
		fName);
	    return 0;
	}
    }

    /* Print the Remote Class Information Attribute */
    switch (rcInfo->ngrci_backend) {
    case NG_BACKEND_NORMAL:
        fprintf(stream, "    <classAttribute backend=\"normal\"");
        break;
    case NG_BACKEND_MPI:
        fprintf(stream, "    <classAttribute backend=\"mpi\"");
        break;
    case NG_BACKEND_BLACS:
        fprintf(stream, "    <classAttribute backend=\"blacs\"");
        break;
    }
    switch (rcInfo->ngrci_language) {
    case NG_CLASS_LANGUAGE_C:
        fprintf(stream, " language=\"C\">\n");
        break;
    case NG_CLASS_LANGUAGE_FORTRAN:
        fprintf(stream, " language=\"FORTRAN\">\n");
        break;
    }

    /* Print the Description of Remote Class */
    fprintf(stream, "      <description>%s</description>\n",
        rcInfo->ngrci_description);

    /* Print the end of Remote Class Information Attribute */
    fprintf(stream, "    </classAttribute>\n");

    /* Print the end of Remote Class Information */
    fprintf(stream, "  </class>");

    /* Success */
    return 1;
}

/**
 * Print the Remote Method Information.
 */
static int
ngexlPrintRemoteMethodInformation(
    ngRemoteMethodInformation_t *rmInfo,
    int indent,
    FILE *stream,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    static const char fName[] = "ngexlPrintRemoteMethodInformation";

    /* Check the arguments */
    assert(rmInfo != NULL);
    assert(rmInfo->ngrmi_methodName != NULL);
    assert(((rmInfo->ngrmi_nArguments > 0) &&
	    (rmInfo->ngrmi_arguments != NULL)) ||
	   ((rmInfo->ngrmi_nArguments == 0) &&
	    (rmInfo->ngrmi_arguments == NULL)));
    assert(stream != NULL);

    /* Print the Remote Method Information */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "<method name=\"%s\" id=\"%d\">\n",
    	rmInfo->ngrmi_methodName, rmInfo->ngrmi_methodID);

    for (i = 0; i < rmInfo->ngrmi_nArguments; i++) {
    	/* Print the Argument Information */
	result = ngexlPrintArgumentInformation(
	    &rmInfo->ngrmi_arguments[i], indent + 1, stream, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Can't print the Argument Information.\n", fName);
	    return 0;
	}
    }

    /* Print the Remote Method Attribute */
    ngexlPrintBlank(indent + 1, stream);
    fprintf(stream, "<methodAttribute shrink=\"%s\">\n",
    	(rmInfo->ngrmi_shrink == 0) ? "no" : "yes");

    /* Print the Calculation Order */
    ngexlPrintBlank(indent + 2, stream);
    fprintf(stream, "<calculationOrder>\n");
    result = ngexlPrintExpression(
    	rmInfo->ngrmi_calculationOrder, indent + 3, stream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't print the Calculation Order.\n", fName);
	return 0;
    }
    ngexlPrintBlank(indent + 2, stream);
    fprintf(stream, "</calculationOrder>\n");

    /* Print the Description of Remote Method */
    ngexlPrintBlank(indent + 2, stream);
    fprintf(stream, "<description>%s</description>\n",
	rmInfo->ngrmi_description);

    /* Print the end of Remote Method Attribute */
    ngexlPrintBlank(indent + 1, stream);
    fprintf(stream, "</methodAttribute>\n");

    /* Print the end of Remote Method Information */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "</method>\n");

    /* Success */
    return 1;
}

/**
 * Print the Argument Information.
 */
static int
ngexlPrintArgumentInformation(
    ngArgumentInformation_t *argInfo,
    int indent,
    FILE *stream,
    ngLog_t *log,
    int *error)
{
    char *modeType = NULL, *dataType = NULL;
    int result;
    int i;
    static const char fName[] = "ngexlPrintArgumentInformation";

    /* Check the argument */
    assert(argInfo->ngai_nDimensions >= 0);
    assert(((argInfo->ngai_dataType != NG_ARGUMENT_DATA_TYPE_CALLBACK) &&
	    (argInfo->ngai_callback == NULL)) ||
           ((argInfo->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) &&
            (argInfo->ngai_callback != NULL)));
    assert(stream != NULL);

    /* Get string of mode type */
    modeType = ngexlGetModeTypeString(argInfo->ngai_ioMode, log, error);
    if (modeType == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't print the Mode Type.\n", fName);
	return 0;
    }

    /* Get string of data type */
    dataType = ngexlGetDataTypeString(argInfo->ngai_dataType, log, error);
    if (dataType == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't print the Data Type.\n", fName);
	return 0;
    }

    /* Print the Argument Information */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "<arg ioMode=\"%s\" dataType=\"%s\">\n",
        modeType, dataType);

    if (argInfo->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
     	/* Print the Callback Information */
    	result = ngexlPrintRemoteMethodInformation(
            argInfo->ngai_callback, indent + 1, stream, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Can't print Callback Information.\n",
		fName);
	    return 0;
	}
    } else {
        for (i = 0; i < argInfo->ngai_nDimensions; i++) {
            /* Print the Subscript Information */
            ngexlPrintBlank(indent + 1, stream);
            fprintf(stream, "<subscript>\n");

            /* Print the Size of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<size>\n");
            result = ngexlPrintExpression(argInfo->ngai_subscript[i].ngsi_size,
                indent + 3, stream, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                     NG_LOG_LEVEL_FATAL, NULL,
                     "%s: Can't print the Size of Subscript.\n", fName);
                return 0;
            }
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "</size>\n");

            /* Print the Start of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<start>\n");
            result = ngexlPrintExpression(argInfo->ngai_subscript[i].ngsi_start,
                indent + 3, stream, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't print the Start of Subscript.\n", fName);
                return 0;
            }
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "</start>\n");

            /* Print the End of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<end>\n");
            result = ngexlPrintExpression(
                argInfo->ngai_subscript[i].ngsi_end, indent + 3, stream, log,
                error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't print the End of Subscript.\n", fName);
                return 0;
            }
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "</end>\n");

            /* Print the Skip of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<skip>\n");
            result = ngexlPrintExpression(argInfo->ngai_subscript[i].ngsi_skip,
                indent + 3, stream, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't print the Skip of Subscript.\n", fName);
                return 0;
             }
             ngexlPrintBlank(indent + 2, stream);
             fprintf(stream, "</skip>\n");

             /* Print the end of Subscript Information */
             ngexlPrintBlank(indent + 1, stream);
             fprintf(stream, "</subscript>\n");
        }
    }

    /* Print the end of Argument Information */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "</arg>\n");

    /* Success */
    return 1;
}

/**
 * Print the Expression Element.
 */
static int
ngexlPrintExpression(
    ngExpressionElement_t *exp,
    int indent,
    FILE *stream,
    ngLog_t *log,
    int *error)
{
    ngExpressionElement_t *exp_tmp = exp;
    int index;
    int result;
    static const char fName[] = "ngexlPrintExpression";

    /* Check the arguments */
    assert(exp != NULL);
    assert(stream != NULL);

    /* Check the Expression data type */
    if (exp->ngee_valueType == NG_EXPRESSION_VALUE_TYPE_END) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: There is no data to Expression Element.\n", fName);
        return 0;
    }

    /* Get last index of Expression Element */
    for (index = 0;
         exp_tmp->ngee_valueType != NG_EXPRESSION_VALUE_TYPE_END;
         index++, exp_tmp++) {
         ;
    }
    index -= 1;

    /* Check the Stack underflow */
    if (index < 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Stack underflow of Expression Element.\n", fName);
        return 0;
    }

    /* Print value of Expression and check type of Expression */
    result = ngexlPrintExpressionValue(exp, &index, indent, stream, log,
        error);
    if (result == 0) {
      ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
          NULL, "%s: Can't print the Expression Element.\n", fName);
      return 0;
    }

    /* Success */
    return 1;
}

/**
 * Print the Expression Element Value.
 */
static int
ngexlPrintExpressionValue(
    ngExpressionElement_t *exp,
    int *index,
    int indent,
    FILE *stream,
    ngLog_t *log,
    int *error)
{
    static const char *operationCodeString[] = {
        NULL, "add", "sub", "mul", "div", "mod", "neg", "pow", "eq", "neq",
        "gt", "lt", "ge", "le", "tri"};
    static const char *operationClause[] = {
        NULL, "monoArithmetic", "biArithmetic", "triArithmetic"};
    static const int operationClauseNumber[] = {
        -1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 3};
    ngExpressionElement_t *exp_tmp = exp;
    int result;
    int i;
    static const char fName[] = "ngexlPrintExpressionValue";

    /* Check the arguments */
    assert(exp != NULL);
    assert(index != NULL);
    assert(stream != NULL);

    /* Get Expression Element number of index */
    for (i = 0; i < *index; i++, exp_tmp++) {
        ;
    }

    /* Print the Start of Expression */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "<expression>\n");
    switch (exp_tmp->ngee_valueType) {
    /* None */
    case NG_EXPRESSION_VALUE_TYPE_NONE:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<noValue/>\n");
        break;

    /* Constant value */
    case NG_EXPRESSION_VALUE_TYPE_CONSTANT:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<immediate val=\"%d\"/>\n", exp_tmp->ngee_value);
        break;

    /* Specified by scaler argument of IN */
    case NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<scalarref val=\"%d\"/>\n", exp_tmp->ngee_value);
        break;

    /* Operation code */
    case NG_EXPRESSION_VALUE_TYPE_OPCODE:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<%s name=\"%s\">\n",
        operationClause[operationClauseNumber[exp_tmp->ngee_value]],
        operationCodeString[exp_tmp->ngee_value]);

        for (i = 0; i < operationClauseNumber[exp_tmp->ngee_value]; i++) {
             (*index) -= 1;
             if (*index < 0) {
                 NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
                 ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                     NG_LOG_LEVEL_FATAL, NULL,
                     "%s: Stack underflow of Expression Element.\n", fName);
                 return 0;
             }

             /* Print the Expression elements value */
             result = ngexlPrintExpressionValue(exp, index, indent + 2,
                 stream, log, error);
             if (result == 0) {
                 ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                     NG_LOG_LEVEL_FATAL, NULL,
                     "%s: Can't print the Expression Element.\n", fName);
                 return 0;
             }
        }

        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "</%s>\n",
            operationClause[operationClauseNumber[exp_tmp->ngee_value]]);
        break;

    /* Other Value are error */
    default:
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't print the Expression Element.\n", fName);
        return 0;        
    }
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "</expression>\n");

    /* Success */
    return 1;
}

/**
 * Print the blank for indent.
 */
static void
ngexlPrintBlank(int indent, FILE *stream)
{
    int i;

    /* Check the arguments */
    assert(stream != NULL);

    /* Print two blanks */
    for (i = 0; i < indent; i++) {
         fprintf(stream, "  ");
    }
}

/**
 * Get string of Mode Type
 */
static char *
ngexlGetModeTypeString(
    ngArgumentIOmode_t mode_type,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngexlGetModeTypeString";

    switch (mode_type) {
    /* None */
    case NG_ARGUMENT_IO_MODE_NONE:
        return "none";

    /* IN: Transfer from client to remote */
    case NG_ARGUMENT_IO_MODE_IN:
        return "in";

    /* OUT: Transfer from remote to client */
    case NG_ARGUMENT_IO_MODE_OUT:
        return "out";

    /* INOUT: Transfer both directions */
    case NG_ARGUMENT_IO_MODE_INOUT:
        return "inout";

    /* WORK: Allocated at remote side */
    case NG_ARGUMENT_IO_MODE_WORK:
        return "work";

    default:
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: No such mode type.\n", fName);
    }
    return NULL;
}

/**
 * Get string of Data Type
 */
static char *
ngexlGetDataTypeString(
    ngArgumentDataType_t data_type,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngexlGetDataTypeString";

    switch (data_type) {
    /* char */
    case NG_ARGUMENT_DATA_TYPE_CHAR:
        return "char";

    /* short */
    case NG_ARGUMENT_DATA_TYPE_SHORT:
        return "short";

    /* int */
    case NG_ARGUMENT_DATA_TYPE_INT:
        return "int";

    /* long */
    case NG_ARGUMENT_DATA_TYPE_LONG:
        return "long";

    /* float */
    case NG_ARGUMENT_DATA_TYPE_FLOAT:
        return "float";

    /* double */
    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
        return "double";

    /* scomplex */
    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
        return "scomplex";

    /* dcomplex */
    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
        return "dcomplex";

    /* string */
    case NG_ARGUMENT_DATA_TYPE_STRING:
        return "string";

    /* filename */
    case NG_ARGUMENT_DATA_TYPE_FILENAME:
        return "filename";

    /* callback function */
    case NG_ARGUMENT_DATA_TYPE_CALLBACK:
        return "callback";

    default:
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Undefined data type.\n", fName);
    }

    return NULL;
}
