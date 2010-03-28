/*
 * $RCSfile: ngXMLparser.c,v $ $Revision: 1.6 $ $Date: 2008/02/14 10:57:33 $
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
 * XML parser module.
 */

#include "ngInternal.h"
#include "ngXML.h"

NGI_RCSID_EMBED("$RCSfile: ngXMLparser.c,v $ $Revision: 1.6 $ $Date: 2008/02/14 10:57:33 $")

/* Functions */

/* XML document */
static int nglXMLdocumentInitialize(ngiXMLdocument_t *, ngLog_t *, int *);
static int nglXMLdocumentFinalize(ngiXMLdocument_t *, ngLog_t *, int *);
static void nglXMLdocumentInitializeMember(ngiXMLdocument_t *);

static int nglXMLdocumentPrintItem(FILE *, ngiXMLdocument_t *, ngiXMLitem_t *, int, ngLog_t *, int *);
static int nglXMLdocumentPrintItemDebug(FILE *,ngiXMLdocument_t *, ngiXMLitem_t *, int, ngLog_t *, int *);

/* XML namespace */
static ngiXMLnsDecl_t *nglXMLnsDeclConstruct(char *, char *, ngLog_t *, int *);
static int nglXMLnsDeclDestruct(ngiXMLnsDecl_t *, ngLog_t *, int *);
static int nglXMLnsDeclInitialize(ngiXMLnsDecl_t *, char *, char *, ngLog_t *, int *);
static int nglXMLnsDeclFinalize(ngiXMLnsDecl_t *, ngLog_t *, int *);
static void nglXMLnsDeclInitializeMember(ngiXMLnsDecl_t *);

/* XML items */
static ngiXMLitem_t *nglXMLdummyConstruct(ngLog_t *, int *);
static int nglXMLelementInitialize(ngiXMLitem_t *, ngiXMLitem_t *, char *, ngLog_t *, int *);
static int nglXMLtextInitialize(ngiXMLitem_t *, ngiXMLitem_t *, char *, size_t, ngLog_t *, int *);
static int nglXMLdummyInitialize(ngiXMLitem_t *, ngLog_t *, int *);
static int nglXMLitemFinalize(ngiXMLitem_t *, ngLog_t *, int *);
static int nglXMLelementFinalize(ngiXMLitem_t *, ngLog_t *, int *);
static int nglXMLtextFinalize(ngiXMLitem_t *, ngLog_t *, int *);
static int nglXMLdummyFinalize(ngiXMLitem_t *, ngLog_t *, int *);
static void nglXMLitemInitializeMember(ngiXMLitem_t *);

static int nglXMLitemDivideQualifiedName(char *, char **, char **, ngLog_t *, int *);
static int nglXMLitemAppendChild(ngiXMLitem_t *, ngiXMLitem_t *, ngLog_t *, int *);
static int nglXMLitemRemoveChild(ngiXMLitem_t *, ngiXMLitem_t *, ngLog_t *, int *);
static int nglXMLelementGetPrefix(ngiXMLitem_t *, char *, char **, ngLog_t *, int *);

/* XML attribute */
static ngiXMLattribute_t *nglXMLattributeConstruct(char *, char *, ngLog_t *, int *);
static int nglXMLattributeDestruct(ngiXMLattribute_t *, ngLog_t *, int *);
static int nglXMLattributeInitialize(ngiXMLattribute_t *, char *, char *, ngLog_t *, int *);
static int nglXMLattributeFinalize(ngiXMLattribute_t *, ngLog_t *, int *);
static void nglXMLattributeInitializeMember(ngiXMLattribute_t *);

/* XML parser */
static int nglXMLparserInitialize(ngiXMLparser_t *parser, ngLog_t *log, int *error);
static int nglXMLparserFinalize(ngiXMLparser_t *parser, ngLog_t *log, int *error);
static void nglXMLparserInitializeMember(ngiXMLparser_t *parser);
static void nglXMLparserInitializePointer(ngiXMLparser_t *parser);

static void nglXMLparserElementStartHandler(void *, const XML_Char *, const XML_Char **);
static void nglXMLparserElementEndHandler(void *, const XML_Char *);
static void nglXMLparserCharacterDataHandler(void *, const XML_Char *, int);
static void nglXMLparserStartNamespaceDeclHandler(void *, const XML_Char *, const XML_Char *);
static void nglXMLparserEndNamespaceDeclHandler(void *, const XML_Char *);

/**
 * XML document: Constructs
 */
ngiXMLdocument_t *
ngiXMLdocumentConstruct(
    ngLog_t *log,
    int *error)
{
    ngiXMLdocument_t *doc = NULL;
    int result;
    static const char fName[] = "ngiXMLdocumentConstruct";

    doc = NGI_ALLOCATE(ngiXMLdocument_t, log, error);
    if (doc == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for the XML document.\n");
        goto error;
    }

    result = nglXMLdocumentInitialize(doc, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the XML document.\n");
        goto error;
    }

    return doc;
error:
    result = ngiXMLdocumentDestruct(doc, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destruct the XML document.\n");
        goto error;
    }
    return NULL;
}

/**
 * XML document: Destruct
 */
int
ngiXMLdocumentDestruct(
    ngiXMLdocument_t *doc,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "ngiXMLdocumentDestruct";
    
    if (doc == NULL) {
        return 1;
    }

    result = nglXMLdocumentFinalize(doc, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize the XML document.\n");
        error = NULL;
        ret   = 0;
    }

    result = NGI_DEALLOCATE(ngiXMLdocument_t, doc, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for the XML document.\n");
        error = NULL;
        ret   = 0;
    }

    return ret;
}

/**
 * XML document: Initialize
 */
static int
nglXMLdocumentInitialize(
    ngiXMLdocument_t *doc,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglXMLdocumentInitialize";
    assert(doc != NULL);

    nglXMLdocumentInitializeMember(doc);

    doc->ngxd_dummy = nglXMLdummyConstruct(log, error);
    if (doc->ngxd_dummy == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't construct the XML dummy item.\n");
        return 0;
    }

    return 1;
}

/**
 * XML document: Finalize
 */
static int
nglXMLdocumentFinalize(
    ngiXMLdocument_t *doc,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
/*
    ngiXMLnamespace_t *ns;
    */
    static const char fName[] = "nglXMLdocumentFinalize";

    assert(doc != NULL);

    /* Elements */
    result = ngiXMLitemDestruct(doc->ngxd_dummy, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destruct the XML items.\n");
        ret = 0;
        error = NULL;
    }
    nglXMLdocumentInitializeMember(doc);

    return ret;
}

/**
 * XML document: Zero clear
 */
static void
nglXMLdocumentInitializeMember(
    ngiXMLdocument_t *doc)
{
    assert(doc != NULL);

    doc->ngxd_dummy = NULL;

    return;
}

/**
 * XML document: Returns root element.
 */
ngiXMLitem_t *
ngiXMLdocumentGetRootElement(
    ngiXMLdocument_t *doc,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLdocumentGetRootElement";

    if ((doc == NULL) || (doc->ngxd_dummy == NULL) ||
        (doc->ngxd_dummy->ngxi_type != NGI_XML_ITEM_DUMMY) ||
        (doc->ngxd_dummy->ngxi_root == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    return doc->ngxd_dummy->ngxi_root;
}

/**
 * XML document: Creates root element.
 */
ngiXMLitem_t *
ngiXMLdocumentCreateRoot(
    ngiXMLdocument_t *doc,
    char *tagName,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLdocumentCreateRoot";

    if ((doc == NULL) || (doc->ngxd_dummy == NULL) ||
        (doc->ngxd_dummy->ngxi_root != NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    return ngiXMLelementConstruct(doc->ngxd_dummy, tagName, log, error);
}


/**
 * XML document: Prints to "fp"
 */
int
ngiXMLdocumentPrint(
    FILE *fp,
    ngiXMLdocument_t *doc,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiXMLdocumentPrint";

    if ((doc == NULL) || (doc->ngxd_dummy == NULL) ||
        (doc->ngxd_dummy->ngxi_type != NGI_XML_ITEM_DUMMY)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }
    if (doc->ngxd_dummy->ngxi_root == NULL) {
        /* If empty, does nothing */
        return 1;
    }

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"us-ascii\"?>\n");

    result = nglXMLdocumentPrintItem(fp, doc, doc->ngxd_dummy->ngxi_root, 0, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't print the element.\n"); 
        goto error;
    }
    return 1;
error:
    return 0;
}

/**
 * XML document: Prints to "fp" for debug
 */
int
ngiXMLdocumentPrintDebug(
    FILE *fp,
    ngiXMLdocument_t *doc,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLdocumentPrintDebug";

    if ((doc == NULL) || (doc->ngxd_dummy == NULL) ||
        (doc->ngxd_dummy->ngxi_type != NGI_XML_ITEM_DUMMY)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }
    if (doc->ngxd_dummy->ngxi_root == NULL) {
        /* If empty, does nothing */
        fprintf(fp, "XML document is empty.\n");
        return 1;
    }

    return nglXMLdocumentPrintItemDebug(fp, doc, doc->ngxd_dummy->ngxi_root, 0, log, error);
}

/**
 * XML document: print item
 */
static int
nglXMLdocumentPrintItem(
    FILE *fp,
    ngiXMLdocument_t *doc,
    ngiXMLitem_t *item,
    int indentLevel,
    ngLog_t *log,
    int *error)
{
    int result;
    char *p;
    char *prefix;
    char *attrPrefix;
    ngiXMLitem_t *it = NULL;
    ngiXMLattribute_t *attrIt = NULL;
    ngiXMLnsDecl_t *nsIt = NULL;
    static const char fName[] = "nglXMLdocumentPrintItem";

    switch (item->ngxi_type) {
    case NGI_XML_ITEM_ELEMENT:
        assert(item->ngxi_namespace != NULL);
        result = nglXMLelementGetPrefix(item, item->ngxi_namespace, &prefix, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't get prefix of the XML element.\n");
            return 0;
        }
        fprintf(fp, "<");
        if (prefix != NULL) {
            fprintf(fp, "%s:", prefix);
        }
        fprintf(fp, "%s", item->ngxi_name);

        LIST_FOREACH(nsIt, &item->ngxi_nsDecls, ngxn_entry) {
            attrPrefix = NULL;
            fprintf(fp, " xmlns");
            if (nsIt->ngxn_prefix != NULL) {
                fprintf(fp, ":%s", nsIt->ngxn_prefix);
            }
            fprintf(fp, "=\"%s\"", nsIt->ngxn_uri);
        }

        /* Print Attribute */
        LIST_FOREACH(attrIt, &item->ngxi_attributes, ngxa_entry) {
            attrPrefix = NULL;
            if (attrIt->ngxa_namespace != NULL) {
                result = nglXMLelementGetPrefix(
                    item, attrIt->ngxa_namespace, &attrPrefix, log, error);
                if ((result == 0) || (attrPrefix == NULL)) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                        "Can't get prefix of the XML element.\n");
                    return 0;
                }
            }

            fprintf(fp, " ");
            if (attrPrefix != NULL) {
                fprintf(fp, "%s:", attrPrefix);
            }
            fprintf(fp, "%s=\"%s\"", attrIt->ngxa_name, attrIt->ngxa_value);
        }

        if (!LIST_EMPTY(&item->ngxi_children)) {
            fprintf(fp, ">");
            LIST_FOREACH(it, &item->ngxi_children, ngxi_entry) {
                result = nglXMLdocumentPrintItem(
                    fp, doc, it, indentLevel+1, log, error);
                if (result == 0) {
                    /* No message */
                    return 0;
                }
            }

            fprintf(fp, "</");
            if (prefix != NULL) {
                fprintf(fp, "%s:", prefix);
            }
            fprintf(fp, "%s>", item->ngxi_name);
        } else {
            fprintf(fp, "/>");
        }

        break;
    case NGI_XML_ITEM_TEXT:
        for (p = item->ngxi_text;*p != '\0';++p) {
            switch (*p) {
            case '&': fprintf(fp, "&amp;"); break;
            case '<': fprintf(fp, "&lt;"); break;
            case '>': fprintf(fp, "&gt;"); break;
            case '"': fprintf(fp, "&quot;"); break;
            default:  fprintf(fp, "%c", *p);
            }
        }
        break;
    case NGI_XML_ITEM_NULL:
    case NGI_XML_ITEM_DUMMY:
    default:
        assert(0);
    }

    return 1;
}
/**
 * XML document: print item for debug
 */
static int
nglXMLdocumentPrintItemDebug(
    FILE *fp,
    ngiXMLdocument_t *doc,
    ngiXMLitem_t *item,
    int indentLevel,
    ngLog_t *log,
    int *error)
{
    switch (item->ngxi_type) {
    case NGI_XML_ITEM_ELEMENT:
#if 0
        prefix = nglXMLdocumentGetPrefix(doc, item->ngxi_namespace, log, error);
#endif
        fprintf(fp, "%*s", indentLevel * 2, "");
        if (item->ngxi_namespace != NULL) {
            fprintf(fp, "%s:", item->ngxi_namespace);
        }
        fprintf(fp, "%s", item->ngxi_name);
        break;
    case NGI_XML_ITEM_TEXT:
        fprintf(fp, "%s", item->ngxi_text);
        break;
    case NGI_XML_ITEM_NULL:
    case NGI_XML_ITEM_DUMMY:
    default:
        assert(0);
    }

    return 1;
}

/**
 * XML namespace declaration: Construct
 */
static ngiXMLnsDecl_t *
nglXMLnsDeclConstruct(
    char *prefix,
    char *uri,
    ngLog_t *log,
    int *error)
{
    ngiXMLnsDecl_t *nsDecl = NULL;
    int result;
    static const char fName[] = "nglXMLnsDeclConstruct";

    nsDecl = NGI_ALLOCATE(ngiXMLnsDecl_t, log, error);
    if (nsDecl == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for the XML namespace declaration.\n");
        goto error;
    }

    result = nglXMLnsDeclInitialize(nsDecl, prefix, uri, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the XML namespace declaration.\n");
        goto error;
    }

    return nsDecl;
error:
    result = nglXMLnsDeclDestruct(nsDecl, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destruct the XML namespace declaration.\n");
    }
    return NULL;
}

/**
 * XML namespace declaration: Destruct
 */
static int
nglXMLnsDeclDestruct(
    ngiXMLnsDecl_t *nsDecl,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "nglXMLnsDeclDestruct";

    result = nglXMLnsDeclFinalize(nsDecl, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize the XML namespace declaration.\n");
        error = NULL;
        ret = 0;
    }

    result = NGI_DEALLOCATE(ngiXMLnsDecl_t, nsDecl, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for the XML namespace declaration.\n");
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * XML namespace declaration: Initialize
 */
static int
nglXMLnsDeclInitialize(
    ngiXMLnsDecl_t *nsDecl,
    char *prefix,
    char *uri,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglXMLnsDeclInitialize";

    assert(uri != NULL);

    nglXMLnsDeclInitializeMember(nsDecl);

    if (prefix != NULL) {
        nsDecl->ngxn_prefix = ngiStrdup(prefix, log, error);
        if (nsDecl->ngxn_prefix == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
    }

    nsDecl->ngxn_uri = ngiStrdup(uri, log, error);
    if (nsDecl->ngxn_uri == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't duplicate the string.\n");
        goto error;
    }

    return 1;
error:
    result = nglXMLnsDeclFinalize(nsDecl, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize the namespace declaration.\n");
    }

    return 0;
}

/**
 * XML namespace declaration: Finalize
 */
static int
nglXMLnsDeclFinalize(
    ngiXMLnsDecl_t *nsDecl,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "nglXMLnsDeclFinalize";

    assert(nsDecl != NULL);

    result = ngiFree(nsDecl->ngxn_prefix, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }

    result = ngiFree(nsDecl->ngxn_uri, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }
    nglXMLnsDeclInitializeMember(nsDecl);

    return ret;
}

/**
 * XML namespace declaration: Zero clear
 */
static void
nglXMLnsDeclInitializeMember(
    ngiXMLnsDecl_t *nsDecl)
{
    assert(nsDecl != NULL);

    nsDecl->ngxn_prefix = NULL;
    nsDecl->ngxn_uri = NULL;
}

/**
 * XML Element: Construct
 */
ngiXMLitem_t *
ngiXMLelementConstruct(
    ngiXMLitem_t *parent,
    char *tagName,
    ngLog_t *log,
    int *error)
{
    ngiXMLitem_t *element = NULL;
    int result;
    static const char fName[] = "ngiXMLelementConstruct";

    if ((parent == NULL) ||
        ((tagName == NULL) || (strlen(tagName) == 0))) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    element = NGI_ALLOCATE(ngiXMLitem_t, log, error);
    if (element == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for the XML element.\n");
        goto error;
    }

    result = nglXMLelementInitialize(element, parent, tagName, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the XML element.\n");
        goto error;
    }

    return element;
error:
    result = ngiXMLitemDestruct(element, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destruct the XML element.\n");
    }
    return NULL;
}

/**
 * XML text data: Construct
 */
ngiXMLitem_t *
ngiXMLtextConstruct(
    ngiXMLitem_t *parent,
    char *string,
    size_t len,
    ngLog_t *log,
    int *error)
{
    ngiXMLitem_t *text = NULL;
    int result;
    static const char fName[] = "ngiXMLtextConstruct";

    if ((parent == NULL) ||
        ((string == NULL) || (strlen(string) == 0))) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    text = NGI_ALLOCATE(ngiXMLitem_t, log, error);
    if (text == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for the XML text.\n");
        goto error;
    }

    result = nglXMLtextInitialize(text , parent, string, len, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the XML text.\n");
        goto error;
    }

    return text;
error:
    result = ngiXMLitemDestruct(text, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destruct the XML text.\n");
    }
    return NULL;
}

/**
 * XML character data: Construct
 */
static ngiXMLitem_t *
nglXMLdummyConstruct(
    ngLog_t *log,
    int *error) 
{
    ngiXMLitem_t *dummy= NULL;
    int result;
    static const char fName[] = "nglXMLdummyConstruct";

    dummy = NGI_ALLOCATE(ngiXMLitem_t, log, error);
    if (dummy == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for the XML dummy.\n");
        goto error;
    }

    result = nglXMLdummyInitialize(dummy, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the XML text.\n");
        goto error;
    }

    return dummy;
error:
    result = ngiXMLitemDestruct(dummy, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destruct the XML text.\n");
    }
    return NULL;
}

/**
 * XML item: Destruct
 */
int
ngiXMLitemDestruct(
    ngiXMLitem_t *item,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "ngiXMLitemDestruct";

    result = nglXMLitemFinalize(item, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize the XML item.\n");
        error = NULL;
        ret = 0;
    }

    result = NGI_DEALLOCATE(ngiXMLitem_t, item, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for the XML item.\n");
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * XML element: Initialize
 */
static int
nglXMLelementInitialize(
    ngiXMLitem_t *element,
    ngiXMLitem_t *parent,
    char *tagName,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglXMLelementInitialize";

    assert(element != NULL);
    assert(parent != NULL);
    assert(tagName != NULL);
    assert(strlen(tagName) > 0 );

    nglXMLitemInitializeMember(element);

    element->ngxi_type = NGI_XML_ITEM_ELEMENT;
    LIST_INIT(&element->ngxi_children);
    LIST_INIT(&element->ngxi_attributes);
    LIST_INIT(&element->ngxi_nsDecls);

    result = nglXMLitemDivideQualifiedName(tagName,
        &element->ngxi_namespace, &element->ngxi_name, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't divide qualified name to prefix and local part.\n");
        goto error;
    }

    result = nglXMLitemAppendChild(parent, element, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't append the XML item to the XML item.\n");
        goto error;
    }

    return 1;
error:
    result = nglXMLitemFinalize(element, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for the XML item.\n");
    }
    return 0;
}

/**
 * XML text: Initialize
 */
static int
nglXMLtextInitialize(
    ngiXMLitem_t *text,
    ngiXMLitem_t *parent,
    char *string,
    size_t len,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglXMLtextInitialize";

    assert(text != NULL);
    assert(parent != NULL);
    assert(string != NULL);
    assert(strlen(string) > 0 );

    nglXMLitemInitializeMember(text);

    text->ngxi_type = NGI_XML_ITEM_TEXT;
    text->ngxi_text = ngiStrndup(string, len, log, error);
    if (text->ngxi_text == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't duplicate the string.\n");
        goto error;
    }

    result = nglXMLitemAppendChild(parent, text, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't append the XML item to the XML item.\n");
        goto error;
    }

    return 1;
error:
    result = nglXMLitemFinalize(text, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for the XML item.\n");
    }
    return 0;
}

/**
 * XML element: Initialize
 */
static int
nglXMLdummyInitialize(
    ngiXMLitem_t *dummy,
    ngLog_t *log,
    int *error)
{
    assert(dummy!= NULL);

    nglXMLitemInitializeMember(dummy);
    dummy->ngxi_type = NGI_XML_ITEM_DUMMY;

    return 1;
}

/**
 * XML item: Finalize
 */
static int
nglXMLitemFinalize(
    ngiXMLitem_t *item,
    ngLog_t *log,
    int *error)
{
    switch(item->ngxi_type) {
    case NGI_XML_ITEM_DUMMY:
        return nglXMLdummyFinalize(item, log, error);
    case NGI_XML_ITEM_TEXT:
        return nglXMLtextFinalize(item, log, error);
    case NGI_XML_ITEM_ELEMENT:
        return nglXMLelementFinalize(item, log, error);
    default:
        assert(0);
        /* NOTREACHED */
        return 0;
    }
}

/**
 * XML element: Finalize
 */
static int
nglXMLelementFinalize(
    ngiXMLitem_t *element,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    ngiXMLitem_t *child;
    ngiXMLattribute_t *attr;
    ngiXMLnsDecl_t *nsDecl;
    static const char fName[] = "nglXMLelementFinalize";

    assert(element != NULL);
    assert(element->ngxi_type == NGI_XML_ITEM_ELEMENT);

    /* Destructs the children */
    while (!LIST_EMPTY(&element->ngxi_children)) {
        child = LIST_FIRST(&element->ngxi_children);
        assert(child != NULL);
        result = ngiXMLitemDestruct(child, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't destruct the child XML item.\n");
            ret = 0;
            error = NULL;
        }
    }

    /* Removes from the parent */
    if (element->ngxi_parent != NULL) {
        result = nglXMLitemRemoveChild(element->ngxi_parent, element, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't remove the element from the parent item.\n");
            ret = 0;
            error = NULL;
        }
    }

    /* Destructs the NS decl */
    while (!LIST_EMPTY(&element->ngxi_nsDecls)) {
        nsDecl = LIST_FIRST(&element->ngxi_nsDecls);
        assert(nsDecl != NULL);
        LIST_REMOVE(nsDecl, ngxn_entry);
        result = nglXMLnsDeclDestruct(nsDecl, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't destruct the XML namespace declaration.\n");
            ret = 0;
            error = NULL;
        }
    }

    /* Destructs the attributes */
    while (!LIST_EMPTY(&element->ngxi_attributes)) {
        attr = LIST_FIRST(&element->ngxi_attributes);
        assert(attr != NULL);
        LIST_REMOVE(attr, ngxa_entry);
        result = nglXMLattributeDestruct(attr, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't destruct the XML attribute.\n");
            ret = 0;
            error = NULL;
        }
    }

    result = ngiFree(element->ngxi_namespace, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free the string.\n");
        ret = 0;
        error = NULL;
    }

#if 0
    result = ngiFree(element->ngxi_prefix, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free the string.\n");
        ret = 0;
        error = NULL;
    }
#endif

    result = ngiFree(element->ngxi_name, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free the string.\n");
        ret = 0;
        error = NULL;
    }

    nglXMLitemInitializeMember(element);

    return ret;
}

/**
 * XML text: Finalize
 */
static int
nglXMLtextFinalize(
    ngiXMLitem_t *text,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "nglXMLtextFinalize";

    assert(text != NULL);
    assert(text->ngxi_type == NGI_XML_ITEM_TEXT);

    /* Removes from the parent */
    if (text->ngxi_parent != NULL) {
        result = nglXMLitemRemoveChild(text->ngxi_parent, text, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't remove the text from the parent item.\n");
            ret = 0;
            error = NULL;
        }
    }

    result = ngiFree(text->ngxi_text, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free the string.\n");
        ret = 0;
        error = NULL;
    }

    nglXMLitemInitializeMember(text);

    return ret;
}

/**
 * XML dummy: Finalize
 */
static int
nglXMLdummyFinalize(
    ngiXMLitem_t *dummy,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "nglXMLdummyFinalize";

    assert(dummy != NULL);
    assert(dummy->ngxi_type == NGI_XML_ITEM_DUMMY);

    /* Removes from the parent */
    if (dummy->ngxi_root != NULL) {
        result = ngiXMLitemDestruct(dummy->ngxi_root, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't destruct the XML root element.\n");
            ret = 0;
            error = NULL;
        }
    }
    dummy->ngxi_root = NULL;

    nglXMLitemInitializeMember(dummy);

    return ret;
}

/**
 * XML item: Zero clear
 */
static void
nglXMLitemInitializeMember(
    ngiXMLitem_t *item)
{
    assert(item != NULL);

    item->ngxi_parent = NULL;
    item->ngxi_type = NGI_XML_ITEM_NULL;

    /* Uses when item->ngxi_type == NGI_XML_ITEM_TEXT */
    item->ngxi_text = NULL;

    /* Uses when item->ngxi_type == NGI_XML_ITEM_ELEMENT */
    item->ngxi_namespace = NULL;
    item->ngxi_name = NULL;
    LIST_INIT(&item->ngxi_attributes);
    LIST_INIT(&item->ngxi_children);
    LIST_INIT(&item->ngxi_nsDecls);

    /* Uses when item->ngxi_type == NGI_XML_ITEM_DUMMY*/
    item->ngxi_root = NULL;
    
}

/**
 * XML item: Zero clear
 */
static int
nglXMLitemDivideQualifiedName(
    char *qualifiedName,
    char **prefix,
    char **localPart,
    ngLog_t *log,
    int *error)
{
    char *prfx = NULL;
    char *lp = NULL;
    char *p;
    int result;
    static const char fName[] = "nglXMLitemDivideQualifiedName";

    assert(qualifiedName != NULL);
    assert(strlen(qualifiedName) > 0);
    assert(prefix != NULL);
    assert(localPart != NULL);

    p = strrchr(qualifiedName, ':');
    if (p != NULL) {
        if (strlen(p+1) == 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
        lp = ngiStrdup(p+1, log, error);
        if (lp == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't duplicate the string.\n");
            goto error;
        }

        assert(p - qualifiedName);
        prfx = ngiStrndup(qualifiedName, p-qualifiedName, log, error);
        if (prfx == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
        assert(strlen(lp) > 0);
        assert(strlen(prfx) > 0);
    } else {
        lp = ngiStrdup(qualifiedName, log, error);
        if (lp == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
        assert(strlen(lp) > 0);
    }

    *prefix    = prfx;
    *localPart = lp;

    return 1;
error:
    result = ngiFree(lp, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free storage for the string.\n");
    }
    result = ngiFree(prfx, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free storage for the string.\n");
    }

    return 0;
}

/**
 * XML item: Appends new child
 */
static int
nglXMLitemAppendChild(
    ngiXMLitem_t *item,
    ngiXMLitem_t *newChild,
    ngLog_t *log,
    int *error)
{
    ngiXMLitem_t *it;
    ngiXMLitem_t *last = NULL;

    assert(item != NULL);
    assert((item->ngxi_type == NGI_XML_ITEM_ELEMENT) ||
           (item->ngxi_type == NGI_XML_ITEM_DUMMY));
    assert(newChild != NULL);
    assert((newChild->ngxi_type == NGI_XML_ITEM_ELEMENT) ||
           (newChild->ngxi_type == NGI_XML_ITEM_TEXT));
    assert(newChild->ngxi_parent == NULL);

    switch (item->ngxi_type) {
    case NGI_XML_ITEM_ELEMENT:
        LIST_FOREACH(it, &item->ngxi_children, ngxi_entry) {
            last = it;
        }
        if (last == NULL) {
            LIST_INSERT_HEAD(&item->ngxi_children, newChild, ngxi_entry);
        } else {
            LIST_INSERT_AFTER(last, newChild, ngxi_entry);
        }
        newChild->ngxi_parent = item;
        break;
    case NGI_XML_ITEM_DUMMY:
        assert(item->ngxi_root == NULL);
        item->ngxi_root = newChild;
        newChild->ngxi_parent = item;
        break;
    default:
        assert(0);
    }

    return 1;
}

/**
 * XML item: Remove new child
 */
static int
nglXMLitemRemoveChild(
    ngiXMLitem_t *item,
    ngiXMLitem_t *child,
    ngLog_t *log,
    int *error)
{
    assert(item != NULL);
    assert((item->ngxi_type == NGI_XML_ITEM_ELEMENT) ||
           (item->ngxi_type == NGI_XML_ITEM_DUMMY));
    assert(child != NULL);
    assert((child->ngxi_type == NGI_XML_ITEM_ELEMENT) ||
           (child->ngxi_type == NGI_XML_ITEM_TEXT));
    assert(child->ngxi_parent == item);

    switch (item->ngxi_type) {
    case NGI_XML_ITEM_ELEMENT:
        LIST_REMOVE(child, ngxi_entry);
        child->ngxi_parent = NULL;
        break;
    case NGI_XML_ITEM_DUMMY:
        assert(item->ngxi_root == child);
        item->ngxi_root = NULL;
        child->ngxi_parent = NULL;
        break;
    default:
        assert(0);
    }

    return 1;
}

/**
 * XML element: Get first child.
 */
ngiXMLitem_t *
ngiXMLelementGetFirstChild(
    ngiXMLitem_t *element,
    int ignoreEmptyText,
    ngLog_t *log,
    int *error)
{
    ngiXMLitem_t *item = NULL;
    ngiXMLitem_t *it = NULL;
    char *p;
    static const char fName[] = "ngiXMLelementGetFirstChild";

    if ((element == NULL) || (element->ngxi_type != NGI_XML_ITEM_ELEMENT)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    LIST_FOREACH(it, &element->ngxi_children, ngxi_entry) {
        if (it->ngxi_type != NGI_XML_ITEM_TEXT) {
            item = it;
            break;
        }

        for (p = it->ngxi_text;*p != '\0';++p) {
            if (isgraph((int)*p)) {
                item = it;
                goto out_of_loop;
            }
        }
    }
out_of_loop:

    return item;
}

/**
 * XML element: Get next item
 */
ngiXMLitem_t *
ngiXMLitemGetNext(
    ngiXMLitem_t *item,
    int ignoreEmptyText,
    ngLog_t *log,
    int *error)
{
    ngiXMLitem_t *next = NULL;
    ngiXMLitem_t *it = NULL;
    char *p;
    static const char fName[] = "ngiXMLitemGetNext";

    if ((item == NULL) || (item->ngxi_parent == NULL) ||
        (item->ngxi_parent->ngxi_type != NGI_XML_ITEM_ELEMENT)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    it = item;
    while ((it = LIST_NEXT(it, ngxi_entry)) != NULL) {
        if (it->ngxi_type != NGI_XML_ITEM_TEXT) {
            next = it;
            break;
        }

        for (p = it->ngxi_text;*p != '\0';++p) {
            if (isgraph((int)*p)) {
                next = it;
                goto out_of_loop;
            }
        }
    }
out_of_loop:

    return next;
}

/**
 * XML element: Get number of child elements.
 */
int
ngiXMLelementGetNumberOfChild(
    ngiXMLitem_t *element,
    char *namespace,
    char *name,
    ngLog_t *log,
    int *error)
{
    int num = 0;
    ngiXMLitem_t *it;
    static const char fName[] = "ngiXMLelementGetNumberOfChild";

    if ((element == NULL) || (element->ngxi_type != NGI_XML_ITEM_ELEMENT)
        || (name == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return -1;
    }

    LIST_FOREACH(it, &element->ngxi_children, ngxi_entry) {
        if ((it->ngxi_type == NGI_XML_ITEM_ELEMENT) && 
            (((namespace == NULL) && (it->ngxi_namespace == NULL)) ||
             ((namespace != NULL) && (it->ngxi_namespace != NULL) &&
              (strcmp(namespace, it->ngxi_namespace) == 0))) &&
            (strcmp(name, it->ngxi_name) == 0)) {
            num++;
        }
    }
    return num;
}

/**
 * XML element: Get attribute
 */
char *
ngiXMLelementGetAttribute(
    ngiXMLitem_t *element,
    char *namespace,
    char *attrName,
    ngLog_t *log,
    int *error)
{
    ngiXMLattribute_t *it = NULL;
    static const char fName[] = "ngiXMLelementGetAttribute";

    if ((element== NULL) || (element->ngxi_type != NGI_XML_ITEM_ELEMENT) ||
        (attrName == NULL) || (strlen(attrName) == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    LIST_FOREACH(it, &element->ngxi_attributes, ngxa_entry) {
        if (strcmp(attrName, it->ngxa_name) == 0) {
            if (((namespace == NULL) && (it->ngxa_namespace == NULL)) ||
                ((namespace != NULL) && (it->ngxa_namespace != NULL) &&
                 (strcmp(namespace, it->ngxa_namespace) == 0))) {
                return it->ngxa_value;
            }
        }
    }

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);

    return NULL;
}

/**
 * XML element: Get copy of attribute value 
 */
char *
ngiXMLelementDupAttributeValue(
    ngiXMLitem_t *element,
    char *namespace,
    char *attrName,
    ngLog_t *log,
    int *error)
{
    char *value = NULL;
    char *copy = NULL;
    static const char fName[] = "ngiXMLelementDupAttributeValue";

    value = ngiXMLelementGetAttribute(element, namespace, attrName, log, error);
    if (value == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get attribute value.\n"); 
        return NULL;
    }

    copy = ngiStrdup(value, log, error);
    if (value == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't duplicate the string.\n"); 
        return NULL;
    }
    return copy;
}

/**
 * Get attribute value as integer
 */
int
ngiXMLelementGetAttributeValueAsInt(
    ngiXMLitem_t *element,
    char *namespace,
    char *attrName,
    int min,
    int max,
    int *intVal,
    ngLog_t *log,
    int *error)
{
    char *value = NULL;
    int ival;
    int result;
    static const char fName[] = "ngiXMLelementGetAttributeValueAsInt";

    if (min > max) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "min is greater than max.\n"); 
        return 0;
    }
    if (intVal == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    value = ngiXMLelementGetAttribute(element, namespace, attrName, log, error);
    if (value == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get attribute value.\n"); 
        return 0;
    }

    result = ngiStringToInt(value, &ival, log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "The value of %s attribute is invalid as integer.\n", attrName);
        return 0;
    }

    if (ival < min) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "The value of %s attribute is too small.\n", attrName);
        return 0;
    }

    if (ival > max) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "The value of %s attribute is too large.\n", attrName);
        return 0;
    }

    *intVal = ival;

    return 1;
}

/**
 * XML element: Appends attribute
 */
int
ngiXMLelementAppendAttribute(
    ngiXMLitem_t *element,
    char *attrName,
    char *value,
    ngLog_t *log,
    int *error)
{
    ngiXMLattribute_t *attr = NULL;
    ngiXMLattribute_t *it = NULL;
    ngiXMLattribute_t *last = NULL;
    static const char fName[] = "ngiXMLelementAppendAttribute";

    if ((element== NULL) || (element->ngxi_type != NGI_XML_ITEM_ELEMENT) ||
        (attrName == NULL) || (strlen(attrName) == 0) || (value == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    attr = nglXMLattributeConstruct(attrName, value, log, error);
    if (attr == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't construct .\n"); 
        return 0;
    }

    LIST_FOREACH(it, &element->ngxi_attributes, ngxa_entry) {
        last = it;
    }
    if (last == NULL) {
        LIST_INSERT_HEAD(&element->ngxi_attributes, attr, ngxa_entry);
    } else {
        LIST_INSERT_AFTER(last, attr, ngxa_entry);
    }

    return  1;
}

/**
 * XML element: Appends the namespace declaration
 */
int
ngiXMLelementAppendNSdecl(
    ngiXMLitem_t *element,
    char *prefix,
    char *uri,
    ngLog_t *log,
    int *error)
{
    ngiXMLnsDecl_t *nsDecl = NULL;
    ngiXMLnsDecl_t *it = NULL;
    ngiXMLnsDecl_t *last = NULL;
    static const char fName[] = "ngiXMLelementAppendNSdecl";

    if ((element == NULL) || (element->ngxi_type != NGI_XML_ITEM_ELEMENT) ||
        (uri == NULL) || (strlen(uri) == 0) ||
        ((prefix != NULL) && (strlen(prefix) == 0))) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    nsDecl = nglXMLnsDeclConstruct(prefix, uri, log, error);
    if (nsDecl == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't construct .\n"); 
        return 0;
    }

    LIST_FOREACH(it, &element->ngxi_nsDecls, ngxn_entry) {
        last = it;
    }
    if (last == NULL) {
        LIST_INSERT_HEAD(&element->ngxi_nsDecls, nsDecl, ngxn_entry);
    } else {
        LIST_INSERT_AFTER(last, nsDecl, ngxn_entry);
    }

    return  1;
}

/**
 * Element: Get Text Data(dup)
 * This function expects "element" having no <element>.
 */
char *
ngiXMLelementGetText(
    ngiXMLitem_t *element,
    ngLog_t *log,
    int *error)
{
    ngiXMLitem_t *it;
    size_t len;
    char *ret = NULL;
    char *p = NULL;
    static const char fName[] = "ngiXMLelementGetText";

    if ((element == NULL) ||
        (element->ngxi_type != NGI_XML_ITEM_ELEMENT)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    LIST_FOREACH(it, &element->ngxi_children, ngxi_entry) {
        if (it->ngxi_type != NGI_XML_ITEM_TEXT) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Element has child elements.\n"); 
            return NULL;
        }
    }

    /* Get Length */
    len = 0;
    LIST_FOREACH(it, &element->ngxi_children, ngxi_entry) {
        assert(it->ngxi_type == NGI_XML_ITEM_TEXT);
        len += strlen(it->ngxi_text);
    }

    ret = ngiMalloc(len + 1, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate storage for the string.\n"); 
        return NULL;
    }
    strcpy(ret, "");
    p = ret;
    LIST_FOREACH(it, &element->ngxi_children, ngxi_entry) {
        assert(it->ngxi_type == NGI_XML_ITEM_TEXT);
        strcpy(p, it->ngxi_text);
        p += strlen(it->ngxi_text);
    }

    return ret;
}

/**
 * Element: Get prefix of namespace
 * Set the namespace declaration of the XML element to *pprefix
 * Thus "pprefix" must be release with free()!
 */
static int
nglXMLelementGetPrefix(
    ngiXMLitem_t *element,
    char *namespace,
    char **pprefix,
    ngLog_t *log,
    int *error)
{
    ngiXMLnsDecl_t *it;
    ngiXMLitem_t *el = NULL;
    char *prefix = NULL;
    int found = 0;
    static const char fName[] = "nglXMLelementGetPrefix";

    assert(element != NULL);
    assert(namespace != NULL);
    assert(strlen(namespace) > 0);
    assert(pprefix != NULL);

    el = element;
    while ((el != NULL) && (el->ngxi_type == NGI_XML_ITEM_ELEMENT)) {
        LIST_FOREACH(it, &el->ngxi_nsDecls, ngxn_entry) {
            if (strcmp(it->ngxn_uri, namespace) == 0) {
                prefix = it->ngxn_prefix;
                found = 1;
            }
        }
        el = el->ngxi_parent;
    }
    if (found == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unkown namespace: \"%s\"\n", namespace);
        return 0;
    }
    *pprefix = prefix;

    return 1;
}

/**
 * XML attribute: Construct
 */
static ngiXMLattribute_t *
nglXMLattributeConstruct(
    char *name,
    char *value,
    ngLog_t *log,
    int *error)
{
    ngiXMLattribute_t *attr = NULL;
    int result;
    static const char fName[] = "nglXMLattributeConstruct";

    attr = NGI_ALLOCATE(ngiXMLattribute_t, log, error);
    if (attr == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for the XML attribute.\n");
        goto error;
    }

    result = nglXMLattributeInitialize(attr, name, value, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the XML attribute.\n");
        goto error;
    }
    return attr;
error:
    result = nglXMLattributeDestruct(attr, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destruct the XML attribute.\n");
    }
    return NULL;
}

/**
 * XML attribute: Destruct
 */
static int
nglXMLattributeDestruct(
    ngiXMLattribute_t *attr,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "nglXMLattributeDestruct";

    if (attr == NULL) {
        /* Do nothing */
        return 1;
    }

    result = nglXMLattributeFinalize(attr, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize the XML attribute.\n");
        ret = 0;
        error = NULL;
    }

    result = NGI_DEALLOCATE(ngiXMLattribute_t, attr, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for the XML attribute.\n");
        ret = 0;
        error = NULL;
    }
    return ret;
}

/**
 * XML attribute: Initialize
 */
static int
nglXMLattributeInitialize(
    ngiXMLattribute_t *attr,
    char *name,
    char *value,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglXMLattributeInitialize";

    assert(attr != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(value != NULL);

    nglXMLattributeInitializeMember(attr);

    result = nglXMLitemDivideQualifiedName(name,
        &attr->ngxa_namespace, &attr->ngxa_name, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't divide qualified name.\n");
        goto error;
    }
    assert(attr->ngxa_name != NULL);

    attr->ngxa_value = ngiStrdup(value, log, error);
    if (attr->ngxa_value == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't duplicate the string.\n");
        goto error;
    }

    return 1;
error:
    result = nglXMLattributeFinalize(attr, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize the XML attribute.\n");
    }
    return 0;
}

/**
 * XML attribute: Finalize
 */
static int
nglXMLattributeFinalize(
    ngiXMLattribute_t *attr,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "nglXMLattributeFinalize";

    result = ngiFree(attr->ngxa_value, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free storage for the string.\n");
        ret = 0;
        error = NULL;
    }

    result = ngiFree(attr->ngxa_name, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free storage for the string.\n");
        ret = 0;
        error = NULL;
    }

    result = ngiFree(attr->ngxa_namespace, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free storage for the string.\n");
        ret = 0;
        error = NULL;
    }
    nglXMLattributeInitializeMember(attr);

    return ret;
}

/**
 * XML attribute: Zero clear
 */
static void
nglXMLattributeInitializeMember(
    ngiXMLattribute_t *attr)
{
    attr->ngxa_namespace = NULL;
    attr->ngxa_name      = NULL;
    attr->ngxa_value     = NULL;
    return;
}

/**
 * XML parse: Construct.
 */
ngiXMLparser_t *
ngiXMLparserConstruct(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLparserConstruct";
    ngiXMLparser_t *xmlParser;
    int result;

    /* Allocate */
    xmlParser = NGI_ALLOCATE(ngiXMLparser_t, log, error);
    if (xmlParser == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for XMLparser.\n"); 
        goto error;
    }

    /* Initialize */
    result = nglXMLparserInitialize(xmlParser, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the XML parser.\n"); 
        goto error;
    }

    /* Success */
    return xmlParser;
error:
    result = ngiXMLparserDestruct(xmlParser, log, NULL);
    if (xmlParser == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct the XML parser.\n"); 
    }

    return NULL;
}

/**
 * Destruct.
 */
int
ngiXMLparserDestruct(
    ngiXMLparser_t *parser,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLparserDestruct";
    int result;
    int ret = 1;

    if (parser == NULL) {
        return 1;
    }

    /* Finalize */
    result = nglXMLparserFinalize(parser, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the XML parser.\n"); 
        ret = 0;
        error = NULL;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiXMLparser_t, parser, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Cant't deallocate the XML parser.\n"); 
        ret = 0;
        error = NULL;
    }

    /* Success */
    return ret;
}


/**
 * XML Parser: Initialize
 */
static int
nglXMLparserInitialize(
    ngiXMLparser_t *parser,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglXMLparserInitialize";
    
    /* Check the arguments */
    assert(parser != NULL);

    /* Initialize the members */
    nglXMLparserInitializeMember(parser);

    /* Create root element */
    parser->ngxp_document = ngiXMLdocumentConstruct(log, error);
    if (parser->ngxp_document== NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't create XML document.\n"); 
        goto error;
    }

    parser->ngxp_handlerLog = log; /* in expat callback, use same log */
    parser->ngxp_handlerError = 0; /* initialize error by No error */

    /* Create expat XML_Parser context */
    parser->ngxp_parser = XML_ParserCreateNS(NULL, ':');
    if (parser->ngxp_parser == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "XML_ParserCreateNS() fail. Can't create XML parser.\n"); 
        goto error;
    }

    XML_SetUserData(parser->ngxp_parser, parser);
    XML_SetElementHandler(parser->ngxp_parser,
                nglXMLparserElementStartHandler,
                nglXMLparserElementEndHandler);
    XML_SetCharacterDataHandler(parser->ngxp_parser,
                nglXMLparserCharacterDataHandler);
    XML_SetCharacterDataHandler(parser->ngxp_parser,
                nglXMLparserCharacterDataHandler);
    XML_SetNamespaceDeclHandler(parser->ngxp_parser,
                nglXMLparserStartNamespaceDeclHandler,
                nglXMLparserEndNamespaceDeclHandler);

    /* Success */
    return 1;
error:
    result = nglXMLparserFinalize(parser, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the XML parser.\n"); 
    }
    return 0;
}

/**
 * Finalize.
 */
static int
nglXMLparserFinalize(
    ngiXMLparser_t *parser,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglXMLparserFinalize";
    ngiXMLnsDecl_t *it;
    int result;
    int ret = 1;

    /* Check the arguments */
    assert(parser != NULL);
    assert(parser->ngxp_parser != NULL);

    /* Destroy expat XML_Parser context */
    XML_ParserFree(parser->ngxp_parser);

    /* Destroy element tree constructed by parser */
    result = ngiXMLdocumentDestruct(parser->ngxp_document, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destroy the XML document.\n"); 
        ret = 0;
        error = NULL;
    }

    while (!LIST_EMPTY(&parser->ngxp_nsDecls)) {
        it = LIST_FIRST(&parser->ngxp_nsDecls);

        LIST_REMOVE(it, ngxn_entry);

        result = nglXMLnsDeclDestruct(it, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destroy the XML declaration.\n"); 
            ret = 0;
            error = NULL;
        }
    }

    parser->ngxp_handlerLog = NULL; /* Don't free log */

    /* Initialize the members */
    nglXMLparserInitializeMember(parser);

    return ret;
}

/**
 * Initialize the members.
 */
static void
nglXMLparserInitializeMember(ngiXMLparser_t *parser)
{
    assert(parser != NULL);
    nglXMLparserInitializePointer(parser);
    LIST_INIT(&parser->ngxp_nsDecls);
    parser->ngxp_handlerError = 0;
}

/**
 * Initialize the pointers.
 */
static void
nglXMLparserInitializePointer(ngiXMLparser_t *parser)
{
    parser->ngxp_parser     = NULL;
    parser->ngxp_document   = NULL;
    parser->ngxp_current    = NULL;
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
    if ((parser == NULL) ||
        ((len > 0) && (buffer == NULL))) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    assert(parser->ngxp_parser != NULL);
    
    /* call expat parse function */
    result = XML_Parse(parser->ngxp_parser, buffer, len, isFinal);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "XML_Parse fail. line %d (Reason %d:%s)\n",
            XML_GetCurrentLineNumber(parser->ngxp_parser),
            XML_GetErrorCode(parser->ngxp_parser),
            XML_ErrorString(XML_GetErrorCode(parser->ngxp_parser))); 
        return 0;
    } 

    if (parser->ngxp_handlerError != 0) {
        NGI_SET_ERROR(error, parser->ngxp_handlerError);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "XML_Parse fail.(parser internal)\n"); 
        return 0;
    }

    if (isFinal != 0) {
        assert(parser->ngxp_current == parser->ngxp_document->ngxd_dummy);
        /* check namespace declaration is empty */
        if (!LIST_EMPTY(&parser->ngxp_nsDecls)) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Namespace declaration list is not empty, invalid status.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * return constructed Root Element.
 */
ngiXMLitem_t *
ngiXMLparserGetRootElement(ngiXMLparser_t *parser, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiXMLparserGetRootElement";

    if ((parser == NULL) || (parser->ngxp_document == NULL) ||
        (parser->ngxp_document->ngxd_dummy == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    return parser->ngxp_document->ngxd_dummy->ngxi_root;
}

/**
 * return constructed document.
 */
ngiXMLdocument_t *
ngiXMLparserGetDocument(
    ngiXMLparser_t *parser,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLparserGetDocument";

    if ((parser == NULL) || (parser->ngxp_document == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return NULL;
    }

    return parser->ngxp_document;
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
    ngiXMLnsDecl_t *nsDecl;
    ngiXMLnsDecl_t *last = NULL;
    ngiXMLparser_t *parser;
    ngiXMLitem_t *newElement;
    ngLog_t *log;
    int *error;
    char **attributeTable;
    int result;

    assert(userData != NULL);
    assert(name != NULL);

    parser = (ngiXMLparser_t *)userData;
    log = parser->ngxp_handlerLog;

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Enter, name = \"%s\"\n", name);

    error = &(parser->ngxp_handlerError);

    /* if error occurred already, then skip all callback. */
    if (*error != 0) {
        return;
    }

    if (parser->ngxp_current == NULL) {
        /* Root */
        newElement = ngiXMLdocumentCreateRoot(
            parser->ngxp_document, (char *)name, log, error);
    } else {
        newElement = ngiXMLelementConstruct(
            parser->ngxp_current, (char *)name, log, error);
        /* WARNING: const cast is not good? */
    }
    if (newElement == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't create the XML element.\n"); 
        goto error;
    }

    while (!LIST_EMPTY(&parser->ngxp_nsDecls)) {
        nsDecl = LIST_FIRST(&parser->ngxp_nsDecls);
        LIST_REMOVE(nsDecl, ngxn_entry);
        if (last == NULL) {
            LIST_INSERT_HEAD(&newElement->ngxi_nsDecls, nsDecl, ngxn_entry);
        } else {
            LIST_INSERT_AFTER(last, nsDecl, ngxn_entry);
        }
        last = nsDecl;
    }

    if (atts != NULL) {
        attributeTable = (char **)atts;
        while (*attributeTable != NULL) {
            assert(*(attributeTable + 1) != NULL);

            /* Create attribute and register */
            result = ngiXMLelementAppendAttribute(newElement,
                        *attributeTable, *(attributeTable + 1), log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't create XML attribute.\n"); 
                goto error;
            }

            attributeTable += 2;
        }
    }
    parser->ngxp_current = newElement;

    /* Success */
    return;
error:
    assert(*error != NG_ERROR_NO_ERROR);
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
    ngLog_t *log;
    char *ns;
    char *ln;
    int *error;

    assert(userData != NULL);
    assert(name != NULL);

    parser = (ngiXMLparser_t *)userData;
    log = parser->ngxp_handlerLog;
    error = &(parser->ngxp_handlerError);
    /* if error occurred already, then skip all callback. */
    if (*error != 0) {
        return;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Enter, name = \"%s\"\n", name);

    /* Check name */
    assert(parser->ngxp_current != NULL);
    assert(parser->ngxp_current->ngxi_type == NGI_XML_ITEM_ELEMENT);

    ns = parser->ngxp_current->ngxi_namespace;
    ln = parser->ngxp_current->ngxi_name;

    if ((strncmp(name, ns, strlen(ns)) != 0) ||
        (name[strlen(ns)] != ':') ||
        (strcmp(&name[strlen(ns) + 1], ln) != 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Element name mismatch while proceeding XML."
            "(\"%s\" != \"%s:%s\"\n", name, ns, ln); 
        goto error;
    }

    parser->ngxp_current = parser->ngxp_current->ngxi_parent;

    if (parser->ngxp_current == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid status: Close tag is more than open tag.\n");
        goto error;
    }

    /* Success */
    return;
error:
    assert(*error != NG_ERROR_NO_ERROR);

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
    ngiXMLitem_t *text;
    ngLog_t *log;
    int *error;

    assert(userData != NULL);
    assert(str != NULL);

    parser = (ngiXMLparser_t *)userData;
    log = parser->ngxp_handlerLog;
    error = &(parser->ngxp_handlerError);
    /* if error occurred already, then skip all callback. */
    if (*error != 0) {
        return;
    }

    if (parser->ngxp_current == NULL) {
        /* Root */
        NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "The XML document has text data on the root.\n");
    } else {
        text = ngiXMLtextConstruct(
            parser->ngxp_current, (char *)str, len, log, error);
        /* WARNING: const cast is not good? */
        if (text == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't create the text data.\n"); 
            goto error;
        }
    }

    /* Success */
    return;
error:
    assert(*error != NG_ERROR_NO_ERROR);

    return;
}

static void
nglXMLparserStartNamespaceDeclHandler(
    void *userData,
    const XML_Char *prefix,
    const XML_Char *uri)
{
    ngiXMLparser_t *parser;
    ngLog_t *log;
    int *error;
    ngiXMLnsDecl_t *nsDecl = NULL;
    ngiXMLnsDecl_t *it = NULL;
    ngiXMLnsDecl_t *last = NULL;
    static const char fName[] = "nglXMLparserStartNamespaceDeclHandler";

    assert(userData != NULL);

    parser = (ngiXMLparser_t *)userData;
    log = parser->ngxp_handlerLog;
    error = &(parser->ngxp_handlerError);

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Enter, prefix = \"%s\" uri = \"%s\"\n", prefix, uri);

    if (*error != 0) {
        return;
    }

    nsDecl = nglXMLnsDeclConstruct((char *)prefix, (char *)uri, log, error);
    if (nsDecl == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't construct .\n"); 
        goto error;
    }

    LIST_FOREACH(it, &parser->ngxp_nsDecls, ngxn_entry) {
        last = it;
    }
    if (last == NULL) {
        LIST_INSERT_HEAD(&parser->ngxp_nsDecls, nsDecl, ngxn_entry);
    } else {
        LIST_INSERT_AFTER(last, nsDecl, ngxn_entry);
    }

    return;
error:
    assert(*error != NG_ERROR_NO_ERROR);
    return;
}

static void
nglXMLparserEndNamespaceDeclHandler(
    void *userData,
    const XML_Char *prefix)
{
    ngiXMLparser_t *parser;
    ngLog_t *log;
    int *error;
    static const char fName[] = "nglXMLparserEndNamespaceDeclHandler";
    
    assert(userData != NULL);

    parser = (ngiXMLparser_t *)userData;
    log = parser->ngxp_handlerLog;
    error = &(parser->ngxp_handlerError);

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Enter, prefix = \"%s\"\n", prefix);

    /* Do nothing */

    return;
}
