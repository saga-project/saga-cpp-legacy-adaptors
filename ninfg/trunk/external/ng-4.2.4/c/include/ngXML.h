/*
 * $RCSfile: ngXML.h,v $ $Revision: 1.3 $ $Date: 2004/04/06 11:28:51 $
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

#ifndef _NGXML_H_
#define _NGXML_H_
/**
 * XML element module. for to set/browse element tree.
 */

#include <stdio.h>
#include "ng.h"
/* xmlparse is in expat, the XML parsing utility */
#include "xmlparse.h"

/* XML attribute */
typedef struct ngiXMLattribute_s {
    struct ngiXMLattribute_s *ngxa_next; /* next attribute in element */
    char *ngxa_name;                     /* attribute name */
    char *ngxa_value;                    /* attribute value */
} ngiXMLattribute_t;

/* XML element */
typedef struct ngiXMLelement_s {
    struct ngiXMLelement_s *ngxe_next;   /* next element in parent element */
    char *ngxe_name;                     /* element name */
    char *ngxe_cdata;                    /* element cdata (always NULL) */
    ngiXMLattribute_t *ngxe_attributes;  /* element attributes */
    struct ngiXMLelement_s *ngxe_elements; /* child elements */
} ngiXMLelement_t;


/**
 * XMLattribute functions
 */
ngiXMLattribute_t *
ngiXMLattributeConstruct(
    ngiXMLelement_t *parentElement,
    char *name,
    char *val,
    ngLog_t *log,
    int *error);

int 
ngiXMLattributeDestruct(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error);

ngiXMLattribute_t *
ngiXMLattributeAllocate(
    ngLog_t *log,
    int *error);

int
ngiXMLattributeFree(
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error);
  
int
ngiXMLattributeInitialize(
    ngiXMLattribute_t *attribute,
    char *name,
    char *val,
    ngLog_t *log,
    int *error);

int
ngiXMLattributeFinalize(
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error);

int
ngiXMLattributeRegister(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error);

int
ngiXMLattributeUnregister(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error);

ngiXMLattribute_t *
ngiXMLattributeGetNext(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *current,
    char *name,
    ngLog_t *log,
    int *error);

char *
ngiXMLattributeGetValue(
    ngiXMLelement_t *parentElement,
    char *name,
    ngLog_t *log,
    int *error);

int
ngiXMLattributePrint(
    FILE *fp,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error);

/**
 * XMLelement functions
 */
ngiXMLelement_t *
ngiXMLelementConstruct(
    ngiXMLelement_t *parentElement,
    char *name,
    ngLog_t *log,
    int *error);

int 
ngiXMLelementDestruct(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error);

ngiXMLelement_t *
ngiXMLelementAllocate(
    ngLog_t *log,
    int *error);

int
ngiXMLelementFree(
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error);
  
int
ngiXMLelementInitialize(
    ngiXMLelement_t *element,
    char *name,
    ngLog_t *log,
    int *error);

int
ngiXMLelementFinalize(
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error);


int
ngiXMLelementRegister(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error);

int
ngiXMLelementUnregister(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error);

ngiXMLelement_t *
ngiXMLelementGetNext(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *current,
    char *name,
    ngLog_t *log,
    int *error);

int
ngiXMLelementAddCdata(
    ngiXMLelement_t *parentElement,
    char *cdata,
    ngLog_t *log,
    int *error);

char *
ngiXMLelementGetCdata(
    ngiXMLelement_t *parentElement,
    ngLog_t *log,
    int *error);

int
ngiXMLelementCountElements(
    ngiXMLelement_t *parentElement,
    char *name,
    ngLog_t *log,
    int *error);

int
ngiXMLelementPrint(
    FILE *fp,
    ngiXMLelement_t *element,
    int depth,
    ngLog_t *log,
    int *error);


/**
 * XML parser module.
 */

/**
 * Parser Construction Remainder, 
 *    which is used when consructing element tree
 * This struct is used as a stack.
 */
typedef struct nglXMLparserConstructionRemainder_s {
    struct nglXMLparserConstructionRemainder_s *ngxer_parent; /* parent */
    ngiXMLelement_t *ngxer_currentElement;
                                     /* element, which constructing now */
    char *ngxer_cdataBuffer;         /* appeared cdata for this element */
    int ngxer_cdataBufferSize;       /* cdata buffer size */
    int ngxer_cdataSize;             /* cdata size */
} nglXMLparserConstructionRemainder_t;


/* parser context */
typedef struct ngiXMLparser_s {
    XML_Parser *ngxp_parser;            /* expat XML context */
    ngiXMLelement_t *ngxp_rootElement;  /* rootElement construct by parse */
    nglXMLparserConstructionRemainder_t *ngxp_pcRemainder;
                                        /* temporary constructing data */
    int ngxp_bufferLength;              /* currently analyzing buffer length */
    ngLog_t *ngxp_handlerLog;           /* Log structure for callback */
    int ngxp_handlerError;              /* error code for callback */
    
} ngiXMLparser_t;


/**
 * XMLparser functions
 */
ngiXMLparser_t *
ngiXMLparserConstruct(ngLog_t *log, int *error);

int
ngiXMLparserDestruct(ngiXMLparser_t *parser, ngLog_t *log, int *error);

ngiXMLparser_t *
ngiXMLparserAllocate(ngLog_t *log, int *error);

int
ngiXMLparserFree(ngiXMLparser_t *parser, ngLog_t *log, int *error);

int
ngiXMLparserInitialize(ngiXMLparser_t *parser, ngLog_t *log, int *error);

int
ngiXMLparserFinalize(ngiXMLparser_t *parser, ngLog_t *log, int *error);

int
ngiXMLparserParse(
    ngiXMLparser_t *parser,
    const char *buffer,
    int len,
    int isFinal,
    ngLog_t *log,
    int *error);

ngiXMLelement_t *
ngiXMLparserGetRootElement(ngiXMLparser_t *parser, ngLog_t *log, int
*error);

#endif /* _NGXML_H_ */

