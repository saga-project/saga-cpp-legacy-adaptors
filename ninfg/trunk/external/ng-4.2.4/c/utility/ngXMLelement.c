#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngXMLelement.c,v $ $Revision: 1.9 $ $Date: 2005/07/04 08:49:47 $";
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
 * XML element module. for to set/browse element tree.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "ngXML.h"

#include <stdio.h>

static void nglXMLattributeInitializeMember(ngiXMLattribute_t *attribute);
static void nglXMLattributeInitializePointer(ngiXMLattribute_t *attribute);

static void nglXMLelementInitializeMember(ngiXMLelement_t *element);
static void nglXMLelementInitializePointer(ngiXMLelement_t *element);

/**
 * XMLattribute functions
 */

/**
 * Construct.
 */
ngiXMLattribute_t *
ngiXMLattributeConstruct(
    ngiXMLelement_t *parentElement,
    char *name,
    char *val,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributeConstruct";
    ngiXMLattribute_t *attribute;
    int result;

    /* Check the argument */
    if ((parentElement == NULL) ||
        (name == NULL) ||
        (val == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return NULL;
    }

    /* Allocate */
    attribute = ngiXMLattributeAllocate(log, error);
    if (attribute == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for XML attribute.\n", fName);
        return NULL;
    }

    /* Initialize */
    result = ngiXMLattributeInitialize(attribute, name, val, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't initialize the XML attribute.\n", fName);
        goto errorProc;

    }

    /* Register */
    result = ngiXMLattributeRegister(parentElement, attribute, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the XML attribute for Element.\n", fName);
        goto errorProc;
    }

    /* Success */
    return attribute;

    /* Error occurred */
errorProc:
    result = ngiXMLattributeFree(attribute, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL,
            "%s: Can't free the storage for the XML attribute.\n", fName);
        return NULL;
    } 
    return NULL;
}

/**
 * Destruct.
 */
int 
ngiXMLattributeDestruct(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributeDestruct";
    int result;

    /* Check the argument */
    if ((parentElement == NULL) ||
        (attribute == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Unregister */
    result = ngiXMLattributeUnregister(parentElement, attribute, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unregister the XML attribute.\n", fName);
        return 0;
    }

    /* Finalize */
    result = ngiXMLattributeFinalize(attribute, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the XML attribute.\n", fName);
        return 0;
    }

    /* Deallocate */
    result = ngiXMLattributeFree(attribute, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't deallocate the XML attribute.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate.
 */
ngiXMLattribute_t *
ngiXMLattributeAllocate(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributeAllocate";
    ngiXMLattribute_t *attribute;

    /* Allocate new storage */
    attribute = globus_libc_malloc(sizeof(ngiXMLattribute_t));
    if (attribute == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for the XML attribute.\n", fName);
        return NULL;
    }

    /* Success */
    return attribute;
}

/**
 * Deallocate.
 */
int
ngiXMLattributeFree(
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error)
{
    /* Check the argument */
    assert(attribute != NULL);

    globus_libc_free(attribute);

    /* Success */
    return 1;
}
  
/**
 * Initialize.
 */
int
ngiXMLattributeInitialize(
    ngiXMLattribute_t *attribute,
    char *name,
    char *val,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributeInitialize";

    /* Check the argument */
    assert(attribute != NULL);
    assert(name != NULL);
    assert(val != NULL);

    /* Initialize the members */
    nglXMLattributeInitializeMember(attribute);

    attribute->ngxa_name = strdup(name);
    if (attribute->ngxa_name == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for the XML attribute name.\n",
            fName);
        return 0;
    }

    attribute->ngxa_value = strdup(val);
    if (attribute->ngxa_value == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for the XML attribute value.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
int
ngiXMLattributeFinalize(
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error)
{
    /* Check the argument */
    assert(attribute != NULL);
    assert(attribute->ngxa_name != NULL);
    assert(attribute->ngxa_value != NULL);

    /* Destruct the members */
    globus_libc_free(attribute->ngxa_name);
    globus_libc_free(attribute->ngxa_value);

    /* Initialize the members */
    nglXMLattributeInitializeMember(attribute);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglXMLattributeInitializeMember(ngiXMLattribute_t *attribute)
{
    assert(attribute != NULL);
    nglXMLattributeInitializePointer(attribute);
}

/**
 * Initialize the pointers.
 */
static void
nglXMLattributeInitializePointer(ngiXMLattribute_t *attribute)
{
    attribute->ngxa_next = NULL;
    attribute->ngxa_name = NULL;
    attribute->ngxa_value = NULL;
}

/**
 * Register attribute into parentElement.
 */
int
ngiXMLattributeRegister(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error)
{
    ngiXMLattribute_t **cur;

    /* Check the arguments */
    assert(parentElement != NULL);
    assert(attribute != NULL);
    assert(attribute->ngxa_next == NULL);

    cur = &(parentElement->ngxe_attributes);

    /* find list tail */
    while (*cur != NULL) {
        cur = &((*cur)->ngxa_next);
    }

    *cur = attribute;

    /* Success */
    return 1;
}

/**
 * Unregister attribute from parentElement.
 */
int
ngiXMLattributeUnregister(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributeUnregister";
    ngiXMLattribute_t **cur;
    int found;

    /* Check the arguments */
    assert(parentElement != NULL);
    assert(attribute != NULL);

    cur = &(parentElement->ngxe_attributes);

    /* find attribute in list */
    found = 0;
    while (*cur != NULL) {
        if (*cur == attribute) {
            *cur = (*cur)->ngxa_next;
            found = 1;
            break;
        }
        cur = &((*cur)->ngxa_next);
    }

    /* not found */
    if (!found) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unregister attribute.(not found)\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * GetNext
 * if current != NULL, return attribute after current.
 * if current == NULL, return first attribute in parentElement.
 * if name != NULL, search attribute specified by name in parentElement,
 * if name == NULL, search exactly next attribute in parentElement.
 */
ngiXMLattribute_t *
ngiXMLattributeGetNext(
    ngiXMLelement_t *parentElement,
    ngiXMLattribute_t *current,
    char *name,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributeGetNext";
    ngiXMLattribute_t *cur;

    /* Check the arguments */
    if (parentElement == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return NULL;
    }

    if (current != NULL) {
        cur = current->ngxa_next;
    } else {
        cur = parentElement->ngxe_attributes;
    }

    if (name == NULL) {
        return cur;
    }

    /* find attribute from list */
    while (cur != NULL) {
        assert(cur->ngxa_name != NULL);
        if (strcmp(cur->ngxa_name, name) == 0) {
            break;
        }
        cur = cur->ngxa_next;
    }

    return cur;
}

/**
 * Get attribute value specified by name
 */
char *
ngiXMLattributeGetValue(
    ngiXMLelement_t *parentElement,
    char *name,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributeGetValue";
    ngiXMLattribute_t *attribute;

    /* Check the arguments */
    if ((parentElement == NULL) ||
        (name == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return NULL;
    }

    /* Get attribute */
    attribute = ngiXMLattributeGetNext(parentElement, NULL, name, log, error);
    if (attribute == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get XML attribute.\n", fName);
        return NULL;
    }

    assert(attribute->ngxa_value != NULL);

    /* Success */
    return attribute->ngxa_value;
}

/**
 * Debug printing.
 */
int
ngiXMLattributePrint(
    FILE *fp,
    ngiXMLattribute_t *attribute,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLattributePrint";

    /* Check the arguments */
    if (attribute == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    assert(attribute->ngxa_name != NULL);
    assert(attribute->ngxa_value != NULL);

    fprintf(fp," (%s=%s)", attribute->ngxa_name, attribute->ngxa_value);

    return 1;
}

/**
 * XMLelement functions
 */

/**
 * Construct.
 *  if parentElement == NULL, then no registration to parentElement
 *  performed. (for ROOT element only).
 */
ngiXMLelement_t *
ngiXMLelementConstruct(
    ngiXMLelement_t *parentElement,
    char *name,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementConstruct";
    ngiXMLelement_t *element;
    int result;

    /* Check the argument */
    if (name == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return NULL;
    }

    /* Allocate */
    element = ngiXMLelementAllocate(log, error);
    if (element == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for XML element.\n", fName);
        return NULL;
    }

    /* Initialize */
    result = ngiXMLelementInitialize(element, name, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't initialize the XML element.\n", fName);
        goto errorProc;

    }

    /* Register */
    if (parentElement != NULL) { /* ROOT element has no parent */
        result = ngiXMLelementRegister(parentElement, element, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the XML element.\n", fName);
            goto errorProc;
        }
    }

    /* Success */
    return element;

    /* Error occurred */
errorProc:
    result = ngiXMLelementFree(element, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL,
            "%s: Can't free the storage for the XML element.\n", fName);
        return NULL;
    } 
    return NULL;

}

/**
 * Destruct.
 */
int 
ngiXMLelementDestruct(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementDestruct";
    int result;

    /* Check the argument */
    if (element == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Unregister */
    if (parentElement != NULL) { /* ROOT element has no parent */
        result = ngiXMLelementUnregister(parentElement, element, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't unregister the XML element.\n", fName);
            return 0;
        }
    }

    /* Finalize */
    result = ngiXMLelementFinalize(element, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the XML element.\n", fName);
        return 0;
    }

    /* Deallocate */
    result = ngiXMLelementFree(element, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't deallocate the XML element.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate.
 */
ngiXMLelement_t *
ngiXMLelementAllocate(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementAllocate";
    ngiXMLelement_t *element;

    /* Allocate new storage */
    element = globus_libc_malloc(sizeof(ngiXMLelement_t));
    if (element == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for the XML element.\n", fName);
        return NULL;
    }

    /* Success */
    return element;
}

/**
 * Deallocate.
 */
int
ngiXMLelementFree(
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error)
{
    /* Check the argument */
    assert(element != NULL);

    globus_libc_free(element);

    /* Success */
    return 1;
}
  
/**
 * Initialize.
 */
int
ngiXMLelementInitialize(
    ngiXMLelement_t *element,
    char *name,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementInitialize";

    /* Check the argument */
    assert(element != NULL);
    assert(name != NULL);

    /* Initialize the members */
    nglXMLelementInitializeMember(element);

    element->ngxe_name = strdup(name);
    if (element->ngxe_name == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for the XML element name.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
int
ngiXMLelementFinalize(
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementFinalize";
    ngiXMLelement_t *currentElement, *nextElement;
    ngiXMLattribute_t *currentAttribute, *nextAttribute;
    int result;

    /* Check the argument */
    assert(element != NULL);
    assert(element->ngxe_name != NULL);

    /* Destruct name */
    globus_libc_free(element->ngxe_name);

    /* Destruct cdata */
    if (element->ngxe_cdata != NULL) {
        globus_libc_free(element->ngxe_cdata);
    }

    /* Destruct all attributes in element */
    currentAttribute = ngiXMLattributeGetNext(element, NULL, NULL, log, error);

    while (currentAttribute != NULL) {
        nextAttribute = ngiXMLattributeGetNext(
                      element, currentAttribute, NULL, log, error);

        result = ngiXMLattributeDestruct(element, currentAttribute, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                 NULL, "%s: Can't destruct XML attribute.\n", fName);
        }
        currentAttribute = nextAttribute;
    }

    /* Destruct all child elements in element */
    currentElement = ngiXMLelementGetNext(element, NULL, NULL, log, error);

    while (currentElement != NULL) {
        nextElement = ngiXMLelementGetNext(
                      element, currentElement, NULL, log, error);

        result = ngiXMLelementDestruct(element, currentElement, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                 NULL, "%s: Can't destruct XML element.\n", fName);
        }
        currentElement = nextElement;
    }

    /* Initialize the members */
    nglXMLelementInitializeMember(element);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglXMLelementInitializeMember(ngiXMLelement_t *element)
{   
    assert(element != NULL);
    nglXMLelementInitializePointer(element);
}

/**
 * Initialize the pointers.
 */
static void
nglXMLelementInitializePointer(ngiXMLelement_t *element)
{
    element->ngxe_next = NULL;
    element->ngxe_name = NULL;
    element->ngxe_cdata = NULL;
    element->ngxe_attributes = NULL;
    element->ngxe_elements = NULL;
}

/**
 * Register element into parentElement.
 */
int
ngiXMLelementRegister(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error)
{
    ngiXMLelement_t **cur;

    /* Check the arguments */
    assert(parentElement != NULL);
    assert(element != NULL);
    assert(element->ngxe_next == NULL);

    cur = &(parentElement->ngxe_elements);

    /* find list tail */
    while (*cur != NULL) {
        cur = &((*cur)->ngxe_next);
    }

    *cur = element;

    /* Success */
    return 1;
}

/**
 * Unregister element from parentElement.
 */
int
ngiXMLelementUnregister(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *element,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementUnregister";
    ngiXMLelement_t **cur;
    int found;

    /* Check the arguments */
    assert(parentElement != NULL);
    assert(element != NULL);

    cur = &(parentElement->ngxe_elements);

    /* find element in list */
    found = 0;
    while (*cur != NULL) {
        if (*cur == element) {
            *cur = (*cur)->ngxe_next;
            found = 1;
            break;
        }
        cur = &((*cur)->ngxe_next);
    }

    /* not found */
    if (!found) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unregister element.(not found)\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * GetNext
 * if current != NULL, return element after current.
 * if current == NULL, return first element in parentElement.
 * if name != NULL, search element specified by name in parentElement,
 * if name == NULL, search exactly next element in parentElement.
 */
ngiXMLelement_t *
ngiXMLelementGetNext(
    ngiXMLelement_t *parentElement,
    ngiXMLelement_t *current,
    char *name,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementGetNext";
    ngiXMLelement_t *cur;

    /* Check the arguments */
    if (parentElement == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return NULL;
    }

    if (current != NULL) {
        cur = current->ngxe_next;
    } else {
        cur = parentElement->ngxe_elements;
    }

    if (name == NULL) {
        return cur;
    }

    /* find element from list */
    while (cur != NULL) {
        assert(cur->ngxe_name != NULL);
        if (strcmp(cur->ngxe_name, name) == 0) {
            break;
        }
        cur = cur->ngxe_next;
    }

    return cur;
}

/**
 * register Cdata into parentElement.
 */
int
ngiXMLelementAddCdata(
    ngiXMLelement_t *parentElement,
    char *cdata,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementAddCdata";
    char *oldCdata, *newCdata;
    size_t newLength;

    /* Check the argument */
    if ((parentElement == NULL) ||
        (cdata == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    oldCdata = parentElement->ngxe_cdata;
    newCdata = NULL;
    newLength = 0;

    /* Allocate */
    if (oldCdata == NULL) {
        newLength = strlen(cdata) + 1; /* not used, but set */
        newCdata = strdup(cdata);
    } else {
        newLength = strlen(oldCdata) + strlen(cdata) + 1;
        newCdata = (char *)globus_libc_malloc(sizeof(char) * newLength);
    }
    if (newCdata == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for the XML element cdata.\n",
            fName);
        return 0;
    }

    /* Copy string */
    if (oldCdata != NULL) {
        snprintf(newCdata, newLength, "%s%s", oldCdata, cdata);
        globus_libc_free(oldCdata);
    }

    parentElement->ngxe_cdata = newCdata;

    /* Success */
    return 1;
}

char *
ngiXMLelementGetCdata(
    ngiXMLelement_t *parentElement,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementGetCdata";

    if (parentElement == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    return parentElement->ngxe_cdata;
}

/**
 * Count how many child elements are in the parentElement.
 * if name != NULL, count elements specified by name.
 * if name == NULL, count all elements.
 * return -1, if error occurred.
 */
int
ngiXMLelementCountElements(
    ngiXMLelement_t *parentElement,
    char *name,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementCountElements";
    ngiXMLelement_t *cur;
    int count;

    if (parentElement == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return -1;
    }

    /* count elements */
    cur = parentElement->ngxe_elements;
    count = 0;
    while (cur != NULL) {
        assert(cur->ngxe_name != NULL);
        if ((name == NULL) || (strcmp(cur->ngxe_name, name) == 0)) {
            count++;
        }
        cur = cur->ngxe_next;
    }

    return count;
}

/**
 * Debug printing
 *   API user should call depth as 0.
 */
int
ngiXMLelementPrint(
    FILE *fp,
    ngiXMLelement_t *element,
    int depth,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiXMLelementPrint";
    ngiXMLattribute_t *childAttribute;
    ngiXMLelement_t *childElement;
    int i;

    /* Check the arguments */
    if (element == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    if (depth == 0) {
        fprintf(fp, "Dumping elemnt start\n");
    }

    /* print indent */
    for (i = 0; i < depth; i++) {
       fprintf(fp, "    ");
    }
    
    /* print name */
    fprintf(fp, "%s ", element->ngxe_name);

    /* print cdata */
    if (element->ngxe_cdata != NULL) {
        fprintf(fp, "cdata=\"%s\"", element->ngxe_cdata);
    }

    /* print attributes */
    childAttribute = NULL;
    while ((childAttribute = ngiXMLattributeGetNext(element,
                              childAttribute, NULL, log, error)) != NULL){
        ngiXMLattributePrint(fp, childAttribute, log, error);
    }
    fprintf(fp, "\n");

    /* print elements */
    childElement = NULL;
    while ((childElement = ngiXMLelementGetNext(element,
                              childElement, NULL, log, error)) != NULL){
        ngiXMLelementPrint(fp, childElement, depth + 1, log, error);
    }

    if (depth == 0) {
        fprintf(fp, "Dumping elemnt end\n");
    }

    return 1;
}

