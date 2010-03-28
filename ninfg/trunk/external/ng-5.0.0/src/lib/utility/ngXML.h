/*
 * $RCSfile: ngXML.h,v $ $Revision: 1.3 $ $Date: 2008/02/07 06:22:04 $
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

#ifndef _NGXML_H_
#define _NGXML_H_
/**
 * XML element module. for to set/browse element tree.
 */

#include "ngUtility.h"

/* xmlparse is in expat, the XML parsing utility */
#include "xmlparse.h"
#include "queue.h"

/* XML attribute */
typedef struct ngiXMLattribute_s {
    LIST_ENTRY(ngiXMLattribute_s)  ngxa_entry;
    char                           *ngxa_namespace; /* namespace */
    char                           *ngxa_name;      /* attribute name */
    char                           *ngxa_value;     /* attribute value */
} ngiXMLattribute_t;
LIST_HEAD(ngiXMLattributeList_s, ngiXMLattribute_s);
typedef struct ngiXMLattributeList_s ngiXMLattributeList_t;

typedef struct ngiXMLnsDecl_s {
    LIST_ENTRY(ngiXMLnsDecl_s) ngxn_entry;
    char                      *ngxn_prefix;
    char                      *ngxn_uri;
} ngiXMLnsDecl_t;
LIST_HEAD(ngiXMLnsDeclList_s, ngiXMLnsDecl_s);
typedef struct ngiXMLnsDeclList_s ngiXMLnsDeclList_t;

typedef enum ngiXMLitemType_e {
    NGI_XML_ITEM_NULL,
    NGI_XML_ITEM_DUMMY,
    NGI_XML_ITEM_ELEMENT,
    NGI_XML_ITEM_TEXT
} ngiXMLitemType_t;

/* XML item */
struct ngiXMLitem_s;
typedef struct ngiXMLitem_s ngiXMLitem_t;
LIST_HEAD(ngiXMLitemList_s, ngiXMLitem_s);
typedef struct ngiXMLitemList_s ngiXMLitemList_t;

struct ngiXMLitem_s {
    LIST_ENTRY(ngiXMLitem_s)  ngxi_entry;
    ngiXMLitem_t             *ngxi_parent;
    ngiXMLitemType_t          ngxi_type;

    /* Uses when ngxi_type == NGI_ITEM_TEXT */
    char                     *ngxi_text; /* element characters data */

    /* Uses when ngxi_type == NGI_ITEM_ELEMENT */
    char                     *ngxi_namespace; /* namespace */
    char                     *ngxi_name;      /* element name */
    ngiXMLattributeList_t     ngxi_attributes;/* element attributes */
    ngiXMLitemList_t          ngxi_children;  /* child elements */

    ngiXMLnsDeclList_t        ngxi_nsDecls;

    /* Uses for dummy */
    ngiXMLitem_t             *ngxi_root;
};

/* XML document */
typedef struct ngiXMLdocument_s {
    ngiXMLitem_t          *ngxd_dummy;
} ngiXMLdocument_t;

/**
 * XML document functions
 */
ngiXMLdocument_t *ngiXMLdocumentConstruct(ngLog_t *, int *);
int ngiXMLdocumentDestruct(ngiXMLdocument_t *, ngLog_t *, int *);
ngiXMLitem_t *ngiXMLdocumentGetRootElement(ngiXMLdocument_t *, ngLog_t *, int *);
ngiXMLitem_t *ngiXMLdocumentCreateRoot(ngiXMLdocument_t *, char *, ngLog_t *, int *);

int ngiXMLdocumentPrint(FILE *, ngiXMLdocument_t *, ngLog_t *, int *);
int ngiXMLdocumentPrintDebug(FILE *, ngiXMLdocument_t *, ngLog_t *, int *);

/**
 * XMLelement functions
 */
ngiXMLitem_t *ngiXMLelementConstruct(ngiXMLitem_t *, char *, ngLog_t *, int *);
ngiXMLitem_t *ngiXMLtextConstruct(ngiXMLitem_t *, char *, size_t, ngLog_t *, int *);
int ngiXMLitemDestruct(ngiXMLitem_t *, ngLog_t *, int *);

ngiXMLitem_t *ngiXMLelementGetFirstChild(ngiXMLitem_t *, int, ngLog_t *, int *);
ngiXMLitem_t *ngiXMLitemGetNext(ngiXMLitem_t *, int, ngLog_t *, int *);
int ngiXMLelementGetNumberOfChild(ngiXMLitem_t *, char *, char *, ngLog_t *, int *);
char *ngiXMLelementGetAttribute(ngiXMLitem_t *, char *, char *, ngLog_t *, int *);
char *ngiXMLelementDupAttributeValue(ngiXMLitem_t *, char *, char *, ngLog_t *, int *);
int ngiXMLelementGetAttributeValueAsInt(ngiXMLitem_t *, char *, char *, int, int, int *, ngLog_t *, int *);
int ngiXMLelementAppendAttribute(ngiXMLitem_t *, char *, char *, ngLog_t *, int *);
int ngiXMLelementAppendNSdecl(ngiXMLitem_t *, char *, char *, ngLog_t *, int *);
char *ngiXMLelementGetText(ngiXMLitem_t *, ngLog_t *, int *);

#define NGI_XML_ELEMENT_IS(item, ns, ln) \
    (((item)->ngxi_type == NGI_XML_ITEM_ELEMENT) && \
     (strcmp((item)->ngxi_namespace, ns) == 0) && \
     (strcmp((item)->ngxi_name, ln) == 0))

/**
 * XML parser module.
 */

/* parser context */
typedef struct ngiXMLparser_s {
    XML_Parser        *ngxp_parser;      /* expat XML context */
    ngiXMLdocument_t  *ngxp_document;    /* XML document */
    ngiXMLitem_t      *ngxp_current;     /* XML current item*/
    ngiXMLnsDeclList_t ngxp_nsDecls;
    ngLog_t           *ngxp_handlerLog;  /* Log structure for callback */
    int                ngxp_handlerError;/* error code for callback */
} ngiXMLparser_t;

/**
 * XMLparser functions
 */
ngiXMLparser_t * ngiXMLparserConstruct(ngLog_t *, int *);
int ngiXMLparserDestruct(ngiXMLparser_t *, ngLog_t *, int *);
int ngiXMLparserParse(ngiXMLparser_t *, const char *, int, int, ngLog_t *, int *);
ngiXMLitem_t * ngiXMLparserGetRootElement(ngiXMLparser_t *, ngLog_t *, int *);
ngiXMLdocument_t *ngiXMLparserGetDocument(ngiXMLparser_t *, ngLog_t *, int *);

#endif /* _NGXML_H_ */
