#ifdef NGIS_NO_WARN_RCSID
static const char rcsid[] = "$RCSfile$ $Revision$ $Date$";
#endif /* NGIS_NO_WARN_RCSID */
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ngisUtility.h"
#include "ngInvokeServer.h"

/**
 * Invoke Server Option Container: Create
 */
ngisOptionContainer_t *
ngisOptionContainerCreate(void)
{
    ngisOptionContainer_t *new = NULL;
    static const char fName[] = "ngisOptionContainerCreate";
    
    new = NGIS_ALLOC(ngisOptionContainer_t);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName,
            "Can't allocate storage for invoke server options container.\n");
        goto error;
    }
    NGIS_LIST_INITIALIZE(ngisOptionElement_t, &new->ngoc_list);

    return new;
error:
    NGIS_NULL_CHECK_AND_FREE(new);

    return NULL;    
}

/**
 * Invoke Server Option Container: Destroy
 */
int
ngisOptionContainerDestroy(
    ngisOptionContainer_t *opts)
{
    ngisOptionElement_t *element = NULL;
    ngisOption_t it;
    ngisOption_t last;
    int ret = 1;
#if 0    
    static const char fName[] = "ngisOptionContainerDestroy";
#endif

    NGIS_ASSERT(opts != NULL);
    
    it = ngisOptionContainerBegin(opts);
    last = ngisOptionContainerEnd(opts);
    while (it != last) {
        element = NGIS_LIST_GET(ngisOptionElement_t, it);
        it = NGIS_LIST_ERASE(ngisOptionElement_t, it);

        NGIS_ASSERT(element != NULL);
        NGIS_ASSERT_STRING(element->ngoe_name);
        NGIS_NULL_CHECK_AND_FREE(element->ngoe_name);
        NGIS_NULL_CHECK_AND_FREE(element->ngoe_value);

        free(element);
    }
    
    NGIS_LIST_FINALIZE(ngisOptionElement_t, &opts->ngoc_list);
    NGIS_FREE(opts); 

    return ret;
}

/**
 * Invoke Server Option Container: Add a option from line
 */
int
ngisOptionContainerAdd(
    ngisOptionContainer_t *opts,
     char *line)
{
    char *value = NULL;
    char *name = NULL;    
    size_t name_len = 0;
    int result;
    ngisOptionElement_t *element = NULL;
    static const char fName[] = "ngisOptionContainerAdd";
    
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(line != NULL);
    NGIS_ASSERT_STRING(line);

    /* Get the name of option */    
    name_len = 0;   
    while ((line[name_len] != '\0') && (!isspace((int)line[name_len]))) {
        name_len++;
    }
    if (name_len == 0) {
        ngisErrorPrint(NULL, fName, "Option's name is empty.\n");
        goto error;
    }
    
    name = ngisStrndup(line, name_len);
    if (name == NULL) {
        ngisErrorPrint(NULL, fName, "Can't copy string of option's name.\n");
        goto error;
    }
    
    /* Get the value of option */
    if (strlen(&line[name_len]) > 1) {
        value = strdup(line + name_len + 1);
    } else {
        value = strdup("");
    }
    if (value == NULL) {
        ngisErrorPrint(NULL, fName,
            "Can't copy string of option's value.\n");
        goto error;
    }
    ngisStringStrip(value);
    
    /* Create new element */
    element = NGIS_ALLOC(ngisOptionElement_t);
    if (element == NULL) {
        ngisErrorPrint(NULL, fName,
            "Can't allocate storage for option element.\n");
        goto error;
    }
    element->ngoe_name  = name;
    element->ngoe_value = value;
    
    result = NGIS_LIST_INSERT_TAIL(ngisOptionElement_t,
        &opts->ngoc_list, element);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "Can't append option element to container.\n");
        goto error;
    }
    return 1;
    
error:
    NGIS_NULL_CHECK_AND_FREE(element);
    NGIS_NULL_CHECK_AND_FREE(value);
    NGIS_NULL_CHECK_AND_FREE(name);

    return 0;
}

ngisOption_t
ngisOptionContainerFindFirst(
    ngisOptionContainer_t *opts,
    char *name)
{
    ngisOption_t it;
    ngisOption_t last;    
    static const char fName[] = "ngisOptionContainerFindFirst";

    NGIS_ASSERT(opts != NULL);    
    NGIS_ASSERT_STRING(name);

    it = ngisOptionContainerBegin(opts);
    last = ngisOptionContainerEnd(opts);

    it = ngisOptionFind(it, last, name);
    if (it == last) {
        ngisDebugPrint(NULL, fName,
            "There is no option whoes name is \"%s\" in container.\n", name);
    }

    return it;
}

ngisOption_t
ngisOptionErase(
    ngisOption_t opt)
{
    ngisOption_t it;
    ngisOptionElement_t *element = NULL;
#if 0
    static const char fName[] = "ngisOptionErase";
#endif

    element = NGIS_LIST_GET(ngisOptionElement_t, opt);
    it = NGIS_LIST_ERASE(ngisOptionElement_t, opt);

    NGIS_ASSERT(element != NULL);
    NGIS_ASSERT_STRING(element->ngoe_name);
    NGIS_ASSERT(element->ngoe_value != NULL);

    NGIS_NULL_CHECK_AND_FREE(element->ngoe_name);
    NGIS_NULL_CHECK_AND_FREE(element->ngoe_value);

    free(element);

    return it;
}


/**
 * Invoke Server Option Container: Find option by name
 */
ngisOption_t
ngisOptionFind(
    ngisOption_t first,
    ngisOption_t last,
    char *name)
{
    ngisOptionElement_t *element = NULL;
#if 0
    static const char fName[] = "ngisOptionFind";
#endif

    NGIS_ASSERT(NGIS_LIST_ITERATOR_IS_VALID(ngisOptionElement_t, first));
    NGIS_ASSERT(NGIS_LIST_ITERATOR_IS_VALID(ngisOptionElement_t, last));    
    NGIS_ASSERT_STRING(name);
    
    while (first != last) {
        element = NGIS_LIST_GET(ngisOptionElement_t, first);
        if (strcmp(element->ngoe_name, name) == 0) {
            return first;
        }
        
        first = ngisOptionNext(first);
    }
    /* Not Found */
    return first;
}

char *
ngisOptionContainerGet(
    ngisOptionContainer_t *opts,
    char *name)
{
    ngisOption_t it;
    ngisOption_t last;    
    char *value = NULL;
    static const char fName[] = "ngisOptionContainerGet";

    NGIS_ASSERT(opts != NULL);    
    NGIS_ASSERT_STRING(name);

    it = ngisOptionContainerBegin(opts);
    last = ngisOptionContainerEnd(opts);

    it = ngisOptionFind(it, last, name);
    if (it == last) {
        ngisDebugPrint(NULL, fName,
            "There is no option whoes name is \"%s\" in container.\n", name);
        return NULL;
    }

    value = ngisOptionValue(it); 

    return value;
}

/**
 * Invoke Server Option Container: Get first option
 */
ngisOption_t
ngisOptionContainerBegin(
    ngisOptionContainer_t *opts)
{
    NGIS_ASSERT(opts != NULL);
    return NGIS_LIST_BEGIN(ngisOptionElement_t, &opts->ngoc_list);
}

/**
 * Invoke Server Option Container: Get last option
 */
ngisOption_t
ngisOptionContainerEnd(
    ngisOptionContainer_t *opts)
{
#if 0
    static const char fName[] = "ngisOptionContainerEnd";
#endif
    NGIS_ASSERT(opts != NULL);
    return NGIS_LIST_END(ngisOptionElement_t, &opts->ngoc_list);
}

ngisOption_t
ngisOptionNext(
    ngisOption_t opt)
{
#if 0
    static const char fName[] = "ngisOptionNext";
#endif
    NGIS_ASSERT(NGIS_LIST_ITERATOR_IS_VALID(ngisOptionElement_t, opt));
    return NGIS_LIST_NEXT(ngisOptionElement_t, opt);
}

char *
ngisOptionName(
    ngisOption_t opt)
{
#if 0
    static const char fName[] = "ngisOptionName";
#endif
    NGIS_ASSERT(NGIS_LIST_ITERATOR_IS_VALID(ngisOptionElement_t, opt));
    return NGIS_LIST_GET(ngisOptionElement_t, opt)->ngoe_name;
}

char *
ngisOptionValue(
    ngisOption_t opt)
{
#if 0
    static const char fName[] = "ngisOptionValue";
#endif
    NGIS_ASSERT(NGIS_LIST_ITERATOR_IS_VALID(ngisOptionElement_t, opt));
    return NGIS_LIST_GET(ngisOptionElement_t, opt)->ngoe_value;
}

int
ngisOptionIs(
    ngisOption_t opt,
    char *name)
{
    NGIS_ASSERT(NGIS_LIST_ITERATOR_IS_VALID(ngisOptionElement_t, opt));
    return strcmp(ngisOptionName(opt), name) == 0;
}
