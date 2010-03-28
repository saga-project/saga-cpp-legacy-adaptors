/*
 * $RCSfile: ngexPrintRemoteClass.c,v $ $Revision: 1.6 $ $Date: 2007/11/27 02:27:41 $
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
 * Module of Ninf-G Executable.
 */

#include "ngEx.h"
#include "ngConfigFile.h"

NGI_RCSID_EMBED("$RCSfile: ngexPrintRemoteClass.c,v $ $Revision: 1.6 $ $Date: 2007/11/27 02:27:41 $")

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

#define NGCLL_RCI_PREFIX "rci"


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
    int tmpFileExist;
    char *tmpFile = NULL;
    char *string = NULL;
    long filesize;
    size_t sum;
    size_t nRead;
    static const char fName[] = "ngexiPrintRemoteClassInformationToBuffer";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfo != NULL);
    assert(funcInfoString != NULL);

    *funcInfoString = NULL;
    tmpFileExist = 0;

    /* Create the temporary file name */
    tmpFile = ngiTemporaryFileCreate(
        context->ngc_lmInfo.nglmi_tmpDir, log, error);
    if (tmpFile == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't create the Temporary File Name.\n"); 
        goto error;
    }
    tmpFileExist = 1;

    /* Open the temporary file */
    fp = fopen(tmpFile, "w+");
    if (fp == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Opening temporary file \"%s\" failed.\n", tmpFile); 
        goto error;
    }

    /* Unlink the temporary file */
    result = unlink(tmpFile);
    tmpFileExist = 0;
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Deleting temporary file \"%s\" failed.\n", tmpFile); 
        goto error;
    }

    /* Print Remote Class Information to the temporary file */
    result = ngexiPrintRemoteClassInformation(
        rcInfo, fp, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Output Remote Class Information failed.\n"); 
        goto error;
    }

    /* Ensure to write buffer */
    result = fflush(fp);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "flushing temporary file \"%s\" failed.\n", tmpFile); 
    }

    filesize = ftell(fp);
    if (filesize < 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "ftell(): %s\n", strerror(errno));
        goto error;
    }
    if (filesize == 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "File is empty.\n");
        goto error;
    }
    /* Return to beginning of the temporary file */
    rewind(fp);

    string = ngiCalloc(filesize + 1, sizeof(char), log, error);
    if (string == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate storage for the XML document.\n");
        goto error;
    }

    sum = 0;
    while (sum < filesize) {
        nRead = fread(string, sizeof(char), filesize-sum, fp);
        if (nRead == 0) {
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't read all date in file.\n");
            goto error;
        }
        sum += nRead;
    }
    assert(sum == filesize);
    string[filesize] = '\0';

    /* Close the temporary file */
    result = fclose(fp);
    fp = NULL;
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Closing temporary file \"%s\" failed.\n", tmpFile); 
        goto error;
    }

    ngiFree(tmpFile, log, error);
    tmpFile = NULL;

    *funcInfoString = string;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (fp != NULL) {
        result = fclose(fp);
        if (result != 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Closing temporary file \"%s\" failed.\n", tmpFile); 
        }
    }

    if (tmpFileExist != 0) {
        result = unlink(tmpFile);
        if (result != 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Deleting temporary file \"%s\" failed.\n", tmpFile); 
        }
    }

    if (tmpFile != NULL) {
        ngiFree(tmpFile, log, NULL);
        tmpFile = NULL;
    }

    if (string != NULL) {
        ngiFree(string, log, NULL);
        string = NULL;
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
    char *backend = NULL;
    char *lang = NULL;
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
    fprintf(stream, "  <%s:class xmlns:%s=\"%s\" name=\"%s\" version=\"%s\" numMethods=\"%d\">\n",
        NGCLL_RCI_PREFIX, NGCLL_RCI_PREFIX,
        "http://ninf.apgrid.org/2006/12/RemoteClassInformation",
    	rcInfo->ngrci_className, rcInfo->ngrci_version,
	rcInfo->ngrci_nMethods);

    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
    	/* Print the Remote Method Information */
    	result = ngexlPrintRemoteMethodInformation(
	    &rcInfo->ngrci_method[i], indent + 1, stream, log, error);
	if (result == 0) {
	    ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't print the Remote Method Information.\n"); 
	    return 0;
	}
    }

    /* Print the Remote Class Information Attribute */
    switch (rcInfo->ngrci_backend) {
    case NG_BACKEND_NORMAL:
        backend = "normal";
        break;
    case NG_BACKEND_MPI:
        backend = "mpi";
        break;
    case NG_BACKEND_BLACS:
        backend = "blacs";
        break;
    }
    switch (rcInfo->ngrci_language) {
    case NG_CLASS_LANGUAGE_C:
        lang = "C";
        break;
    case NG_CLASS_LANGUAGE_FORTRAN:
        lang = "FORTRAN";
        break;
    }
    assert(backend != NULL);
    assert(lang != NULL);

    fprintf(stream, "    <%s:classAttribute backend=\"%s\" language=\"%s\">\n",
        NGCLL_RCI_PREFIX, backend, lang);

    /* Print the Description of Remote Class */
    fprintf(stream, "      <%s:description>%s</%s:description>\n",
        NGCLL_RCI_PREFIX, rcInfo->ngrci_description, NGCLL_RCI_PREFIX);

    /* Print the end of Remote Class Information Attribute */
    fprintf(stream, "    </%s:classAttribute>\n", NGCLL_RCI_PREFIX);

    /* Print the end of Remote Class Information */
    fprintf(stream, "  </%s:class>\n", NGCLL_RCI_PREFIX);

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
    fprintf(stream, "<%s:method name=\"%s\" id=\"%d\">\n",
        NGCLL_RCI_PREFIX, rmInfo->ngrmi_methodName, rmInfo->ngrmi_methodID);

    for (i = 0; i < rmInfo->ngrmi_nArguments; i++) {
    	/* Print the Argument Information */
	result = ngexlPrintArgumentInformation(
	    &rmInfo->ngrmi_arguments[i], indent + 1, stream, log, error);
	if (result == 0) {
	    ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't print the Argument Information.\n"); 
	    return 0;
	}
    }

    /* Print the Remote Method Attribute */
    ngexlPrintBlank(indent + 1, stream);
    fprintf(stream, "<%s:methodAttribute shrink=\"%s\">\n",
        NGCLL_RCI_PREFIX, (rmInfo->ngrmi_shrink == 0) ? "no" : "yes");

    /* Print the Calculation Order */
    ngexlPrintBlank(indent + 2, stream);
    fprintf(stream, "<%s:calculationOrder>\n", NGCLL_RCI_PREFIX);
    result = ngexlPrintExpression(
    	rmInfo->ngrmi_calculationOrder, indent + 3, stream, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't print the Calculation Order.\n"); 
	return 0;
    }
    ngexlPrintBlank(indent + 2, stream);
    fprintf(stream, "</%s:calculationOrder>\n", NGCLL_RCI_PREFIX);

    /* Print the Description of Remote Method */
    ngexlPrintBlank(indent + 2, stream);
    fprintf(stream, "<%s:description>%s</%s:description>\n",
        NGCLL_RCI_PREFIX, rmInfo->ngrmi_description, NGCLL_RCI_PREFIX);

    /* Print the end of Remote Method Attribute */
    ngexlPrintBlank(indent + 1, stream);
    fprintf(stream, "</%s:methodAttribute>\n", NGCLL_RCI_PREFIX);

    /* Print the end of Remote Method Information */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "</%s:method>\n", NGCLL_RCI_PREFIX);

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
    	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't print the Mode Type.\n"); 
	return 0;
    }

    /* Get string of data type */
    dataType = ngexlGetDataTypeString(argInfo->ngai_dataType, log, error);
    if (dataType == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't print the Data Type.\n"); 
	return 0;
    }

    /* Print the Argument Information */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "<%s:arg ioMode=\"%s\" dataType=\"%s\">\n",
        NGCLL_RCI_PREFIX, modeType, dataType);

    if (argInfo->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
     	/* Print the Callback Information */
    	result = ngexlPrintRemoteMethodInformation(
            argInfo->ngai_callback, indent + 1, stream, log, error);
	if (result == 0) {
	    ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't print Callback Information.\n"); 
	    return 0;
	}
    } else {
        for (i = 0; i < argInfo->ngai_nDimensions; i++) {
            /* Print the Subscript Information */
            ngexlPrintBlank(indent + 1, stream);
            fprintf(stream, "<%s:subscript>\n", NGCLL_RCI_PREFIX);

            /* Print the Size of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<%s:size>\n", NGCLL_RCI_PREFIX);
            result = ngexlPrintExpression(argInfo->ngai_subscript[i].ngsi_size,
                indent + 3, stream, log, error);
            if (result == 0) {
                ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't print the Size of Subscript.\n"); 
                return 0;
            }
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "</%s:size>\n", NGCLL_RCI_PREFIX);

            /* Print the Start of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<%s:start>\n", NGCLL_RCI_PREFIX);
            result = ngexlPrintExpression(argInfo->ngai_subscript[i].ngsi_start,
                indent + 3, stream, log, error);
            if (result == 0) {
                ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't print the Start of Subscript.\n"); 
                return 0;
            }
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "</%s:start>\n", NGCLL_RCI_PREFIX);

            /* Print the End of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<%s:end>\n", NGCLL_RCI_PREFIX);
            result = ngexlPrintExpression(
                argInfo->ngai_subscript[i].ngsi_end, indent + 3, stream, log,
                error);
            if (result == 0) {
                ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't print the End of Subscript.\n"); 
                return 0;
            }
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "</%s:end>\n", NGCLL_RCI_PREFIX);

            /* Print the Skip of Subscript */
            ngexlPrintBlank(indent + 2, stream);
            fprintf(stream, "<%s:skip>\n", NGCLL_RCI_PREFIX);
            result = ngexlPrintExpression(argInfo->ngai_subscript[i].ngsi_skip,
                indent + 3, stream, log, error);
            if (result == 0) {
                ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't print the Skip of Subscript.\n"); 
                return 0;
             }
             ngexlPrintBlank(indent + 2, stream);
             fprintf(stream, "</%s:skip>\n", NGCLL_RCI_PREFIX);

             /* Print the end of Subscript Information */
             ngexlPrintBlank(indent + 1, stream);
             fprintf(stream, "</%s:subscript>\n", NGCLL_RCI_PREFIX);
        }
    }

    /* Print the end of Argument Information */
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "</%s:arg>\n", NGCLL_RCI_PREFIX);

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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "There is no data to Expression Element.\n"); 
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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Stack underflow of Expression Element.\n"); 
        return 0;
    }

    /* Print value of Expression and check type of Expression */
    result = ngexlPrintExpressionValue(exp, &index, indent, stream, log,
        error);
    if (result == 0) {
      ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
          "Can't print the Expression Element.\n"); 
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
    fprintf(stream, "<%s:expression>\n", NGCLL_RCI_PREFIX);
    switch (exp_tmp->ngee_valueType) {
    /* None */
    case NG_EXPRESSION_VALUE_TYPE_NONE:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<%s:noValue/>\n", NGCLL_RCI_PREFIX);
        break;

    /* Constant value */
    case NG_EXPRESSION_VALUE_TYPE_CONSTANT:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<%s:immediate val=\"%d\"/>\n", NGCLL_RCI_PREFIX, exp_tmp->ngee_value);
        break;

    /* Specified by scaler argument of IN */
    case NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<%s:scalarref val=\"%d\"/>\n", NGCLL_RCI_PREFIX, exp_tmp->ngee_value);
        break;

    /* Operation code */
    case NG_EXPRESSION_VALUE_TYPE_OPCODE:
        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "<%s:%s name=\"%s\">\n",
            NGCLL_RCI_PREFIX,
            operationClause[operationClauseNumber[exp_tmp->ngee_value]],
            operationCodeString[exp_tmp->ngee_value]);

        for (i = 0; i < operationClauseNumber[exp_tmp->ngee_value]; i++) {
             (*index) -= 1;
             if (*index < 0) {
                 NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
                 ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                     "Stack underflow of Expression Element.\n"); 
                 return 0;
             }

             /* Print the Expression elements value */
             result = ngexlPrintExpressionValue(exp, index, indent + 2,
                 stream, log, error);
             if (result == 0) {
                 ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                     "Can't print the Expression Element.\n"); 
                 return 0;
             }
        }

        ngexlPrintBlank(indent + 1, stream);
        fprintf(stream, "</%s:%s>\n",
            NGCLL_RCI_PREFIX,
            operationClause[operationClauseNumber[exp_tmp->ngee_value]]);
        break;

    /* Other Value are error */
    default:
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't print the Expression Element.\n"); 
        return 0;        
    }
    ngexlPrintBlank(indent, stream);
    fprintf(stream, "</%s:expression>\n", NGCLL_RCI_PREFIX);

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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "No such mode type.\n"); 
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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Undefined data type.\n"); 
    }

    return NULL;
}
