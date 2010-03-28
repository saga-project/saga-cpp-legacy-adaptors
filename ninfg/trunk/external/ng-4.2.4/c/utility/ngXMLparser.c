#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngXMLparser.c,v $ $Revision: 1.10 $ $Date: 2005/07/04 08:49:47 $";
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
 * XML parser module.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ng.h"
#include "ngXML.h"

#include <stdio.h>

static void nglXMLparserInitializeMember(ngiXMLparser_t *parser);
static void nglXMLparserInitializePointer(ngiXMLparser_t *parser);
static void
nglXMLparserElementStartHandler(
    void *userData,
    const XML_Char *name,
    const XML_Char **atts);
static void
nglXMLparserElementEndHandler(
    void *userData,
    const XML_Char *name);
static void
nglXMLparserCharacterDataHandler(
    void *userData,
    const XML_Char *str,
    int len);

static nglXMLparserConstructionRemainder_t *
nglXMLparserConstructionRemainderConstruct(
    ngiXMLparser_t *parser,
    ngiXMLelement_t *currentElement,
    ngLog_t *log,
    int *error);
static int nglXMLparserConstructionRemainderDestruct(
    ngiXMLparser_t *parser,
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error);

static nglXMLparserConstructionRemainder_t *
    nglXMLparserConstructionRemainderAllocate(
    ngLog_t *log,
    int *error);
static int nglXMLparserConstructionRemainderFree(
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error);
static int nglXMLparserConstructionRemainderInitialize(
    nglXMLparserConstructionRemainder_t *pcRemainder,
    nglXMLparserConstructionRemainder_t *upperRemainder,
    ngiXMLelement_t *currentElement,
    int cdataLength,
    ngLog_t *log,
    int *error);
static int nglXMLparserConstructionRemainderFinalize(
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error);
static void nglXMLparserConstructionRemainderInitializeMember(
    nglXMLparserConstructionRemainder_t *pcRemainder);
static void nglXMLparserConstructionRemainderInitializePointer(
    nglXMLparserConstructionRemainder_t *pcRemainder);
static int nglXMLparserConstructionRemainderRegister(
    ngiXMLparser_t *parser,
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error);
static int nglXMLparserConstructionRemainderUnregister(
    ngiXMLparser_t *parser,
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error);


/**
 * Construct.
 */
ngiXMLparser_t *
ngiXMLparserConstruct(ngLog_t *log, int *error)
{
    static const char fName[] = "ngiXMLparserConstruct";
    ngiXMLparser_t *xmlParser;
    int result;

    /* Allocate */
    xmlParser = ngiXMLparserAllocate(log, error);
    if (xmlParser == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for XMLparser.\n", fName);
        return NULL;
    }

    /* Initialize */
    result = ngiXMLparserInitialize(xmlParser, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the XML parser.\n", fName);
        return NULL;
    }

    /* Success */
    return xmlParser;
}

/**
 * Destruct.
 */
int
ngiXMLparserDestruct(ngiXMLparser_t *parser, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiXMLparserDestruct";
    int result;

    if (parser == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Finalize */
    result = ngiXMLparserFinalize(parser, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the XML parser.\n", fName);
        return 0;
    }

    /* Deallocate */
    result = ngiXMLparserFree(parser, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Cant't deallocate the XML parser.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate.
 */
ngiXMLparser_t *
ngiXMLparserAllocate(ngLog_t *log, int *error)
{
    static const char fName[] = "ngiXMLparserAllocate";
    ngiXMLparser_t *xmlParser;

    /* Allocate new storage */
    xmlParser = (ngiXMLparser_t *)
        globus_libc_malloc(sizeof(ngiXMLparser_t));
    if (xmlParser == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for the XML parser.\n",
            fName);
        return NULL;
    }

    /* Success */
    return xmlParser;
}

/**
 * Deallocate.
 */
int
ngiXMLparserFree(ngiXMLparser_t *parser, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(parser != NULL);

    globus_libc_free(parser);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
int
ngiXMLparserInitialize(ngiXMLparser_t *parser, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiXMLparserInitialize";
    nglXMLparserConstructionRemainder_t *newRemainder;
    
    /* Check the arguments */
    assert(parser != NULL);

    /* Initialize the members */
    nglXMLparserInitializeMember(parser);

    /* Create root element */
    parser->ngxp_rootElement = ngiXMLelementConstruct(NULL, "ROOT", log, error);
    if (parser->ngxp_rootElement == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't create XML element.\n", fName);
        return 0;
    }

    parser->ngxp_pcRemainder = NULL; /* ROOT's parent is unavailable. */

    /* Create element tree construction data structure (only for ROOT) */
    /* newRemainder is already registered to parser by RemainderConstruct */
    newRemainder =
        nglXMLparserConstructionRemainderConstruct(
            parser, parser->ngxp_rootElement, log, error);
    if (newRemainder == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't create XML construction data.\n", fName);
        return 0;
    }

    parser->ngxp_handlerLog = log; /* in expat callback, use same log */
    parser->ngxp_handlerError = 0; /* initialize error by No error */

    /* Create expat XML_Parser context */
    parser->ngxp_parser = XML_ParserCreate(NULL);
    if (parser->ngxp_parser == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: XML_ParserCreate() fail. Can't create XML parser.\n",
            fName);
        return 0;
    }

    XML_SetUserData(parser->ngxp_parser, parser);

    XML_SetElementHandler(parser->ngxp_parser,
                nglXMLparserElementStartHandler,
                nglXMLparserElementEndHandler);

    XML_SetCharacterDataHandler(parser->ngxp_parser,
                nglXMLparserCharacterDataHandler);

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
int
ngiXMLparserFinalize(ngiXMLparser_t *parser, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiXMLparserFinalize";
    int result;

    /* Check the arguments */
    assert(parser != NULL);
    assert(parser->ngxp_parser != NULL);

    /* Destroy expat XML_Parser context */
    XML_ParserFree(parser->ngxp_parser);

    /* Destroy element tree constructed by parser */
    result = ngiXMLelementDestruct(NULL, parser->ngxp_rootElement, log, error);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destroy element tree.\n", fName);
        return 0;
    }

    /* Destroy element tree construction data structure */
    if (parser->ngxp_pcRemainder != NULL){

        /* making : warning should output Parse(isFinal) is not set yet */

        result = nglXMLparserConstructionRemainderDestruct(
            parser, parser->ngxp_pcRemainder, log, error);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy XML construction data.\n", fName);
            return 0;
        }
    }

    parser->ngxp_handlerLog = NULL; /* Don't free log */

    /* Initialize the members */
    nglXMLparserInitializeMember(parser);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglXMLparserInitializeMember(ngiXMLparser_t *parser)
{
    assert(parser != NULL);
    nglXMLparserInitializePointer(parser);
    parser->ngxp_bufferLength = 0;
    parser->ngxp_handlerError = 0;
}

/**
 * Initialize the pointers.
 */
static void
nglXMLparserInitializePointer(ngiXMLparser_t *parser)
{
    parser->ngxp_parser = NULL;
    parser->ngxp_rootElement = NULL;
    parser->ngxp_pcRemainder = NULL;
    parser->ngxp_handlerLog = NULL;
}

/**
 * XML Parse.
 */
int
ngiXMLparserParse(
    ngiXMLparser_t *parser,
    const char *buffer,
    int len,
    int isFinal,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLparserParse";
    int result;

    /* Check the arguments */
    if ((parser == NULL) || (buffer == NULL) || (len <= 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    assert(parser->ngxp_parser != NULL);
    parser->ngxp_bufferLength = len;
    
    /* call expat parse function */
    result = XML_Parse(parser->ngxp_parser, buffer, len, isFinal);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: XML_Parse fail. line %d (Reason %d:%s)\n", fName,
            XML_GetCurrentLineNumber(parser->ngxp_parser),
            XML_GetErrorCode(parser->ngxp_parser),
            XML_ErrorString(XML_GetErrorCode(parser->ngxp_parser)));
        return 0;
    } 

    if (parser->ngxp_handlerError != 0) {
        NGI_SET_ERROR(error, parser->ngxp_handlerError);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: XML_Parse fail.(parser internal)\n", fName);
        return 0;
    }

    if (isFinal) {
        assert(parser->ngxp_pcRemainder != NULL);
        assert(parser->ngxp_pcRemainder->ngxer_parent == NULL);

        result = nglXMLparserConstructionRemainderDestruct(
            parser, parser->ngxp_pcRemainder, log, error);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy XML construction data.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * return constructed Root Element.
 */
ngiXMLelement_t *
ngiXMLparserGetRootElement(ngiXMLparser_t *parser, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiXMLparserGetRootElement";

    if (parser == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return NULL;
    }

    return parser->ngxp_rootElement;

}

/**
 * expat XML parsing callback : callback for element start.
 */
static void
nglXMLparserElementStartHandler(
    void *userData,
    const XML_Char *name,
    const XML_Char **atts)
{
    static const char fName[] = "nglXMLparserElementStartHandler";
    ngiXMLparser_t *parser;
    nglXMLparserConstructionRemainder_t *pcRemainder, *newRemainder;
    ngiXMLelement_t *parentElement, *newElement;
    ngiXMLattribute_t *newAttribute;
    ngLog_t *log;
    int *error;
    char **attributeTable;

    assert(userData != NULL);
    assert(name != NULL);

    parser = (ngiXMLparser_t *)userData;

    error = &(parser->ngxp_handlerError);
    /* if error occurred already, then skip all callback. */
    if (*error != 0) {
        return;
    }

    log = parser->ngxp_handlerLog;

    pcRemainder = parser->ngxp_pcRemainder;
    assert(pcRemainder != NULL);

    parentElement = pcRemainder->ngxer_currentElement;
    assert(parentElement != NULL);

    /* New XML Element start, then create new Element and register */
    newElement = ngiXMLelementConstruct(
                      parentElement, (char *)name, log, error);
    if (newElement == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't create XML element.\n", fName);
        return;
    }

    /* Create Construction Remainder and register to stack */
    newRemainder = nglXMLparserConstructionRemainderConstruct(
                                  parser, newElement, log, error);
    if (newRemainder == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't create XML construction data.\n", fName);
        return;
    }

    if (atts != NULL) {
        attributeTable = (char **)atts;
        while (*attributeTable != NULL) {

            /* Create attribute and register */
            newAttribute = ngiXMLattributeConstruct(newElement,
                        *attributeTable, *(attributeTable + 1), log, error);
            if (newAttribute == NULL) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL, 
                    "%s: Can't create XML attribute.\n", fName);
                return;
            }

            attributeTable += 2;
        }
    }

    /* Success */
    return;
}

/**
 * expat XML parsing callback : callback for element end.
 */
static void
nglXMLparserElementEndHandler(
    void *userData,
    const XML_Char *name)
{
    static const char fName[] = "nglXMLparserElementEndHandler";
    ngiXMLparser_t *parser;
    nglXMLparserConstructionRemainder_t *pcRemainder;
    ngiXMLelement_t *currentElement;
    int i, found, bufferSize;
    char *buffer, c;
    ngLog_t *log;
    int *error;
    int result;

    assert(userData != NULL);
    assert(name != NULL);

    parser = (ngiXMLparser_t *)userData;

    error = &(parser->ngxp_handlerError);
    /* if error occurred already, then skip all callback. */
    if (*error != 0) {
        return;
    }

    log = parser->ngxp_handlerLog;

    pcRemainder = parser->ngxp_pcRemainder;
    assert(pcRemainder != NULL);

    /* Check name */
    currentElement = pcRemainder->ngxer_currentElement;
    assert(currentElement != NULL);
    if (strcmp(name, currentElement->ngxe_name) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Element name mismatch while proceeding XML."
                  "(\"%s\" != \"%s\"\n", fName,
                   name, currentElement->ngxe_name);
        return;
    }

    /* Add cdata */
    buffer = pcRemainder->ngxer_cdataBuffer;
    bufferSize = pcRemainder->ngxer_cdataSize;
    
    if (buffer != NULL) {
        /* Check if the non-space string are appeared. */
        found = 0;
        for (i = 0; i < bufferSize; i++) {
            c = buffer[i];
            if ((c != ' ') && (c != '\t') && (c != '\n') && (c != '\r')){
                found = 1;
                break;
            }
        }
        if (found) {
            /* append cdata into Element */
            result = ngiXMLelementAddCdata(currentElement, buffer, log, error);
            if (result != 1) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't add cdata into XML element.\n", fName);
                return;
            }
        }
    }

    /* Destruct Construction Remainder, and unregister from stack */
    result = nglXMLparserConstructionRemainderDestruct(
                parser, pcRemainder, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct XML construction data.\n", fName);
        return;
    }

    /* Success */
    return;
}

/**
 * expat XML parsing callback : callback for character data (string).
 */
static void
nglXMLparserCharacterDataHandler(
    void *userData,
    const XML_Char *str,
    int len)
{
    static const char fName[] = "nglXMLparserCharacterDataHandler";
    ngiXMLparser_t *parser;
    nglXMLparserConstructionRemainder_t *pcRemainder;
    int curSize, newSize;
    char *buffer;
    ngLog_t *log;
    int *error;

    assert(userData != NULL);

    parser = (ngiXMLparser_t *)userData;

    error = &(parser->ngxp_handlerError);
    /* if error occurred already, then skip all callback. */
    if (*error != 0) {
        return;
    }

    log = parser->ngxp_handlerLog;

    pcRemainder = parser->ngxp_pcRemainder;
    assert(pcRemainder != NULL);

    buffer = pcRemainder->ngxer_cdataBuffer;
    curSize = pcRemainder->ngxer_cdataSize;
    newSize = curSize + len;

    if (buffer == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: No cdata buffer for XML parse.\n", fName);
        return;
    }

    if (newSize > pcRemainder->ngxer_cdataBufferSize) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: No cdata buffer for XML parse anymore.\n", fName);
        return;
    }

    /* Add string into cdata buffer */
    strncpy(&buffer[curSize], (char *)str, len);
    buffer[newSize] = '\0';
    pcRemainder->ngxer_cdataSize = newSize;

    /* Success */
    return;
}


/**
 * Parser Construction Remainder,
 *     which is used when constructing element tree.
 */

/**
 * Construct.
 */
static nglXMLparserConstructionRemainder_t *
nglXMLparserConstructionRemainderConstruct(
    ngiXMLparser_t *parser,
    ngiXMLelement_t *currentElement,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglXMLparserConstructionRemainderConstruct";
    nglXMLparserConstructionRemainder_t *pcRemainder;
    int result;

    /* Check the argument */
    assert(parser != NULL);
    assert(currentElement != NULL);

    /* Allocate */
    pcRemainder = nglXMLparserConstructionRemainderAllocate(log, error);
    if (pcRemainder == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for XML construction data.\n",
            fName);
        return NULL;
    }

    /* Initialize */
    result = nglXMLparserConstructionRemainderInitialize(
        pcRemainder, parser->ngxp_pcRemainder, currentElement,
        parser->ngxp_bufferLength, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't initialize the XML construction data.\n", fName);
        goto error_proc;
        
    }

    /* Register */
    result = nglXMLparserConstructionRemainderRegister(
                parser, pcRemainder, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the XML construction data for parser.\n",
            fName);
        goto error_proc;
    }

    /* Success */
    return pcRemainder;

    /* Error occurred */
error_proc:
    result = nglXMLparserConstructionRemainderFree(pcRemainder, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL,
            "%s: Can't free the storage for the XML construction data.\n",
            fName);
        return NULL;
    } 
    return NULL;
}


/**
 * Destruct.
 */
static int
nglXMLparserConstructionRemainderDestruct(
    ngiXMLparser_t *parser,
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglXMLparserConstructionRemainderDestruct";
    int result;

    /* Check the argument */
    assert(pcRemainder != NULL);

    /* Unregister */
    result = nglXMLparserConstructionRemainderUnregister(
                    parser, pcRemainder, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unregister the XML construction data.\n", fName);
        return 0;
    }

    /* Finalize */
    result = nglXMLparserConstructionRemainderFinalize(
                         pcRemainder, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the XML construction data.\n", fName);
        return 0;
    }

    /* Deallocate */
    result = nglXMLparserConstructionRemainderFree(pcRemainder, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't deallocate the XML construction data.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate.
 */
static nglXMLparserConstructionRemainder_t *
nglXMLparserConstructionRemainderAllocate(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglXMLparserConstructionRemainderAllocate";
    nglXMLparserConstructionRemainder_t *pcRemainder;

    /* Allocate new storage */
    pcRemainder = (nglXMLparserConstructionRemainder_t *)
        globus_libc_malloc(sizeof(nglXMLparserConstructionRemainder_t));
    if (pcRemainder == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for the XML construction data.\n",
            fName);
        return NULL;
    }

    /* Success */
    return pcRemainder;
}

/**
 * Deallocate.
 */
static int
nglXMLparserConstructionRemainderFree(
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error)
{
    /* Check the argument */
    assert(pcRemainder != NULL);

    globus_libc_free(pcRemainder);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
nglXMLparserConstructionRemainderInitialize(
    nglXMLparserConstructionRemainder_t *pcRemainder,
    nglXMLparserConstructionRemainder_t *upperRemainder,
    ngiXMLelement_t *currentElement,
    int cdataLength,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglXMLparserConstructionRemainderInitialize";

    /* Check the argument */
    assert(pcRemainder != NULL);
    assert(currentElement != NULL);

    /* Initialize the members */
    nglXMLparserConstructionRemainderInitializeMember(pcRemainder);

    pcRemainder->ngxer_parent = upperRemainder;
    pcRemainder->ngxer_currentElement = currentElement;

    pcRemainder->ngxer_cdataBuffer = NULL;
    pcRemainder->ngxer_cdataBufferSize = 0;
    pcRemainder->ngxer_cdataSize = 0;

    if (cdataLength > 0) {
        pcRemainder->ngxer_cdataBufferSize = cdataLength;
        pcRemainder->ngxer_cdataBuffer = (char *)
            globus_libc_calloc(cdataLength + 1, sizeof(char));
        if (pcRemainder->ngxer_cdataBuffer == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't allocate the storage for the XML parser.\n",
                fName);
            return 0;
        }

        /* clear the string (for safety) */
        pcRemainder->ngxer_cdataBuffer[0] = '\0';
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
nglXMLparserConstructionRemainderFinalize(
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error)
{
    /* Check the argument */
    assert(pcRemainder != NULL);

    if (pcRemainder->ngxer_cdataBuffer != NULL) {
        globus_libc_free(pcRemainder->ngxer_cdataBuffer);
    }

    /* Initialize the members */
    nglXMLparserConstructionRemainderInitializeMember(pcRemainder);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglXMLparserConstructionRemainderInitializeMember(
    nglXMLparserConstructionRemainder_t *pcRemainder)
{
    assert(pcRemainder != NULL);
    nglXMLparserConstructionRemainderInitializePointer(pcRemainder);
    pcRemainder->ngxer_cdataBufferSize = 0;
    pcRemainder->ngxer_cdataSize = 0;
}

/**
 * Initialize the pointers.
 */
static void
nglXMLparserConstructionRemainderInitializePointer(
    nglXMLparserConstructionRemainder_t *pcRemainder)
{
    pcRemainder->ngxer_parent = NULL;
    pcRemainder->ngxer_currentElement = NULL;
    pcRemainder->ngxer_cdataBuffer = NULL;
}

/**
 * Register
 */
static int
nglXMLparserConstructionRemainderRegister(
    ngiXMLparser_t *parser,
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(parser != NULL);
    assert(pcRemainder != NULL);

    assert(parser->ngxp_pcRemainder == pcRemainder->ngxer_parent);

    parser->ngxp_pcRemainder = pcRemainder;

    /* Success */
    return 1;
}

/**
 * Unregister
 */
static int
nglXMLparserConstructionRemainderUnregister(
    ngiXMLparser_t *parser,
    nglXMLparserConstructionRemainder_t *pcRemainder,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(parser != NULL);
    assert(pcRemainder != NULL);

    assert(parser->ngxp_pcRemainder == pcRemainder);
    
    parser->ngxp_pcRemainder = pcRemainder->ngxer_parent;

    /* Success */
    return 1;
}

