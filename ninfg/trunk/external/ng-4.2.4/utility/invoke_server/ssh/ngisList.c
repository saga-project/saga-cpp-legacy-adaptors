#ifdef NGIS_NO_WARN_RCSID
static const char rcsid[] = "$RCSfile: ngisList.c,v $ $Revision: 1.3 $ $Date: 2006/08/21 11:25:37 $";
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
#include <errno.h>
#include <string.h>

#include "ngisList.h"
#include "ngisLog.h"

/**
 * List: Initialize
 */
void
ngisListInitialize(
    ngisList_t *list)
{
#if 0
    static const char fName[] = "ngisListInitialize";
#endif
    NGIS_ASSERT(list != NULL);

    list->ngl_dummy.ngn_next    = &list->ngl_dummy;
    list->ngl_dummy.ngn_prev    = &list->ngl_dummy;
    list->ngl_dummy.ngn_content = NULL;

    return;
}

/**
 * List: Finalize
 */
void
ngisListFinalize(
    ngisList_t *list)
{
#if 0
    static const char fName[] = "ngisListFinalize";
#endif
    ngislListNode_t *cur = NULL;
    ngislListNode_t *last = NULL;

    NGIS_ASSERT(list != NULL);

    cur  = ngisListBegin(list);
    NGIS_ASSERT(ngisListIteratorIsValid(cur));    
    last = ngisListEnd(list);
    while (cur != last) {
        cur = ngisListErase(cur);
        NGIS_ASSERT(ngisListIteratorIsValid(cur));
    }

    list->ngl_dummy.ngn_next    = NULL;
    list->ngl_dummy.ngn_prev    = NULL;
    list->ngl_dummy.ngn_content = NULL;

    return;
}

/**
 * List: Get iterator of head
 */
ngisListIterator_t
ngisListBegin(
    ngisList_t *list)
{
#if 0
    static const char fName[] = "ngisListBegin";
#endif
    NGIS_ASSERT(list != NULL);

    return list->ngl_dummy.ngn_next;
}

/**
 * List: Get iterator of next of tail
 */
ngisListIterator_t
ngisListEnd(
    ngisList_t *list)
{
#if 0
    static const char fName[] = "ngisListEnd";
#endif
    NGIS_ASSERT(list != NULL);

    return &list->ngl_dummy;
}

/**
 * List: Get iterator of next
 */
ngisListIterator_t
ngisListNext(
    ngisListIterator_t it)
{
#if 0
    static const char fName[] = "ngisListNext";
#endif
    NGIS_ASSERT(it != NULL);
    NGIS_ASSERT(it->ngn_content != NULL);

    return it->ngn_next;
}

/**
 * List: Get iterator of prev
 */
ngisListIterator_t
ngisListPrev(
    ngisListIterator_t it)
{
#if 0
    static const char fName[] = "ngisListPrev";
#endif
    NGIS_ASSERT(it != NULL);
    NGIS_ASSERT(it->ngn_prev->ngn_content != NULL);

    return it->ngn_prev;
}

/**
 * List: Get value
 */
void *
ngisListGet(
    ngisListIterator_t it)
{
#if 0
    static const char fName[] = "ngisListGet";
#endif
    NGIS_ASSERT(it != NULL);
    NGIS_ASSERT(it->ngn_content != NULL);

    return it->ngn_content;
}

/**
 * List: Insert head
 */
int
ngisListInsertHead(
    ngisList_t *list,
    void *content)
{
    ngisListIterator_t it;
#if 0
    static const char fName[] = "ngisListInsertHead";
#endif
    NGIS_ASSERT(list != NULL);

    it = ngisListBegin(list);
    it = ngisListInsert(it, content);
    
    return ngisListIteratorIsValid(it);
}

/**
 * List: Insert tail
 */
int
ngisListInsertTail(
    ngisList_t *list,
    void *content)
{
    ngisListIterator_t it;
#if 0
    static const char fName[] = "ngisListInsertTail";
#endif
    NGIS_ASSERT(list != NULL);

    it = ngisListEnd(list);
    it = ngisListInsert(it, content);
    
    return ngisListIteratorIsValid(it);
}

/**
 * List: Insert
 */
ngisListIterator_t
ngisListInsert(
    ngisListIterator_t it,
    void *content)
{
    ngislListNode_t *new = NULL;
    static const char fName[] = "ngisListInsert";

    NGIS_ASSERT(it != NULL);
    NGIS_ASSERT(content != NULL);

    new = NGIS_ALLOC(ngislListNode_t);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName,
            "Can't allocate storage for new list node\n");
        return NULL;
    }

    new->ngn_content = content;
    new->ngn_prev = it->ngn_prev;
    new->ngn_next = it;

    it->ngn_prev->ngn_next = new;
    it->ngn_prev = new;

    return it;
}

/**
 * List: Erase
 */
ngisListIterator_t
ngisListErase(
    ngisListIterator_t it)
{
    ngisListIterator_t next;
    ngisListIterator_t prev;
#if 0
    static const char fName[] = "ngisListErase";
#endif

    NGIS_ASSERT(it != NULL);
    NGIS_ASSERT(it->ngn_content != NULL);

    next = it->ngn_next;
    prev = it->ngn_prev;

    next->ngn_prev = prev;
    prev->ngn_next = next;

    it->ngn_next = NULL;
    it->ngn_prev = NULL;
    it->ngn_content = NULL;
    free(it);

    return next;
}

/**
 * List: Erase item of address specified.
 */
int
ngisListEraseByAddress(
    ngisList_t *list,
    void *content)
{
    ngisListIterator_t cur;
    ngisListIterator_t last;
    ngisListIterator_t itResult;    
    static const char fName[] = "ngisListEraseByAddress";

    NGIS_ASSERT(list != NULL);
    NGIS_ASSERT(content != NULL);

    cur  = ngisListBegin(list);
    last = ngisListEnd(list);
    while (cur != last) {
        if(ngisListGet(cur) == content) {
            itResult = ngisListErase(cur);
            return ngisListIteratorIsValid(itResult);
        }
        cur = ngisListNext(cur);
    }
    
    ngisErrorPrint(NULL, fName,
        "There is not item of address specified in list.\n");
    return 0;
}

/**
 * List: Erase item of address specified.
 */
int
ngisListIteratorIsValid(
    ngisListIterator_t it)
{
#if 0
    static const char fName[] = "ngisListIteratorIsValid";
#endif
    return it != NULL;
}

void
ngisListSplice1(
    ngisListIterator_t it,
    ngisListIterator_t src)
{
    /* Remove source list*/
    src->ngn_prev->ngn_next = src->ngn_next;
    src->ngn_next->ngn_prev = src->ngn_prev;

    /* Add dest list */
    src->ngn_prev = it->ngn_prev;
    src->ngn_next = it;
    it->ngn_prev->ngn_next = src;
    it->ngn_prev           = src;

    return;
}

void
ngisListSplice2(
    ngisListIterator_t it,
    ngisListIterator_t first,
    ngisListIterator_t last)
{
    ngislListNode_t *end = last->ngn_prev;

    if (first == last) {
        return;
    }

    /* Remove source list*/
    first->ngn_prev->ngn_next = last;
    last->ngn_prev            = first->ngn_prev;

    /* Add dest list */
    first->ngn_prev = it->ngn_prev;
    end->ngn_next   = it;
    it->ngn_prev->ngn_next = first;
    it->ngn_prev           = end;

    return;
}
