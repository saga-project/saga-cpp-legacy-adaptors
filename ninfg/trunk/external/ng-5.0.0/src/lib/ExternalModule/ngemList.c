/*
 * $RCSfile: ngemList.c,v $ $Revision: 1.6 $ $Date: 2008/03/27 10:02:53 $
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

#include "ngemCallbackManager.h"
#include "ngemList.h"
#include "ngemUtility.h"
#include "ngemLog.h"

NGI_RCSID_EMBED("$RCSfile: ngemList.c,v $ $Revision: 1.6 $ $Date: 2008/03/27 10:02:53 $")

/**
 * List: Initialize
 */
void
ngemListInitialize(
    ngemList_t *list)
{
#if 0
    static const char fName[] = "ngemListInitialize";
#endif
    NGEM_ASSERT(list != NULL);

    list->ngl_dummy.ngn_next    = &list->ngl_dummy;
    list->ngl_dummy.ngn_prev    = &list->ngl_dummy;
    list->ngl_dummy.ngn_content = NULL;

    return;
}

/**
 * List: Finalize
 */
void
ngemListFinalize(
    ngemList_t *list)
{
#if 0
    static const char fName[] = "ngemListFinalize";
#endif
    ngemlListNode_t *cur = NULL;
    ngemlListNode_t *last = NULL;

    NGEM_ASSERT(list != NULL);

    cur  = ngemListBegin(list);
    NGEM_ASSERT(ngemListIteratorIsValid(cur));    
    last = ngemListEnd(list);
    while (cur != last) {
        cur = ngemListErase(cur);
        NGEM_ASSERT(ngemListIteratorIsValid(cur));
    }

    list->ngl_dummy.ngn_next    = NULL;
    list->ngl_dummy.ngn_prev    = NULL;
    list->ngl_dummy.ngn_content = NULL;

    return;
}

/**
 * List: Get iterator of head
 */
ngemListIterator_t
ngemListBegin(
    ngemList_t *list)
{
#if 0
    static const char fName[] = "ngemListBegin";
#endif
    NGEM_ASSERT(list != NULL);

    return list->ngl_dummy.ngn_next;
}

/**
 * List: Get iterator of next of tail
 */
ngemListIterator_t
ngemListEnd(
    ngemList_t *list)
{
#if 0
    static const char fName[] = "ngemListEnd";
#endif
    NGEM_ASSERT(list != NULL);

    return &list->ngl_dummy;
}

/**
 * List: Get iterator of next
 */
ngemListIterator_t
ngemListNext(
    ngemListIterator_t it)
{
#if 0
    static const char fName[] = "ngemListNext";
#endif
    NGEM_ASSERT(it != NULL);
    NGEM_ASSERT(it->ngn_content != NULL);

    return it->ngn_next;
}

/**
 * List: Get iterator of prev
 */
ngemListIterator_t
ngemListPrev(
    ngemListIterator_t it)
{
#if 0
    static const char fName[] = "ngemListPrev";
#endif
    NGEM_ASSERT(it != NULL);
    NGEM_ASSERT(it->ngn_prev->ngn_content != NULL);

    return it->ngn_prev;
}

/**
 * List: Get value
 */
void *
ngemListGet(
    ngemListIterator_t it)
{
#if 0
    static const char fName[] = "ngemListGet";
#endif
    NGEM_ASSERT(it != NULL);
    NGEM_ASSERT(it->ngn_content != NULL);

    return it->ngn_content;
}

/**
 * List: Insert head
 */
ngemResult_t
ngemListInsertHead(
    ngemList_t *list,
    void *content)
{
    ngemListIterator_t it;
#if 0
    static const char fName[] = "ngemListInsertHead";
#endif
    NGEM_ASSERT(list != NULL);

    it = ngemListBegin(list);
    it = ngemListInsert(it, content);
    
    return ngemListIteratorIsValid(it)?NGEM_SUCCESS:NGEM_FAILED;
}


/**
 * List: Insert tail
 */
ngemResult_t
ngemListInsertTail(
    ngemList_t *list,
    void *content)
{
    ngemListIterator_t it;
#if 0
    static const char fName[] = "ngemListInsertTail";
#endif
    NGEM_ASSERT(list != NULL);

    it = ngemListEnd(list);
    it = ngemListInsert(it, content);
    
    return ngemListIteratorIsValid(it)?NGEM_SUCCESS:NGEM_FAILED;
}

/**
 * List: Insert
 */
ngemListIterator_t
ngemListInsert(
    ngemListIterator_t it,
    void *content)
{
    ngemlListNode_t *new = NULL;
    ngemListIterator_t it2;
    int i = 0;
    ngLog_t *log;
    static const char fName[] = "ngemListInsert";

    NGEM_ASSERT(it != NULL);
    NGEM_ASSERT(content != NULL);

    log = ngemLogGetDefault();

    new = NGI_ALLOCATE(ngemlListNode_t, log, NULL);
    if (new == NULL) {
        ngLogError(log, NGEM_LOGCAT_LIST, fName,
            "Can't allocate storage for new list node\n");
        return NULL;
    }

    new->ngn_content = content;
    new->ngn_prev = it->ngn_prev;
    new->ngn_next = it;

    it->ngn_prev->ngn_next = new;
    it->ngn_prev = new;

    it2 = it->ngn_next;
    while (it2 != it) {
        it2 = it2->ngn_next;
        i++;
    }

    return it;
}

/**
 * List: Erase
 */
ngemListIterator_t
ngemListErase(
    ngemListIterator_t it)
{
    ngemListIterator_t next;
    ngemListIterator_t prev;
    ngLog_t *log;
#if 0
    static const char fName[] = "ngemListErase";
#endif

    log = ngemLogGetDefault();

    NGEM_ASSERT(it != NULL);
    NGEM_ASSERT(it->ngn_content != NULL);

    next = it->ngn_next;
    prev = it->ngn_prev;

    next->ngn_prev = prev;
    prev->ngn_next = next;

    it->ngn_next = NULL;
    it->ngn_prev = NULL;
    it->ngn_content = NULL;
    ngiFree(it, log, NULL);

    return next;
}

/**
 * List: Erase item of address specified.
 */
ngemResult_t
ngemListEraseByAddress(
    ngemList_t *list,
    void *content)
{
    ngemListIterator_t cur;
    ngemListIterator_t last;
    ngemListIterator_t itResult;    
    ngLog_t *log;
    static const char fName[] = "ngemListEraseByAddress";

    NGEM_ASSERT(list != NULL);
    NGEM_ASSERT(content != NULL);

    log = ngemLogGetDefault();

    cur  = ngemListBegin(list);
    last = ngemListEnd(list);
    while (cur != last) {
        if(ngemListGet(cur) == content) {
            itResult = ngemListErase(cur);
            return ngemListIteratorIsValid(itResult)?NGEM_SUCCESS:NGEM_FAILED;
        }
        cur = ngemListNext(cur);
    }
    
    ngLogError(log, NGEM_LOGCAT_LIST, fName,
        "There is not item of address specified in list.\n");

    return NGEM_FAILED;
}

/**
 * List: Erase item of address specified.
 */
bool
ngemListIteratorIsValid(
    ngemListIterator_t it)
{
#if 0
    static const char fName[] = "ngemListIteratorIsValid";
#endif
    return it != NULL;
}

void
ngemListSplice1(
    ngemListIterator_t it,
    ngemListIterator_t src)
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
ngemListSplice2(
    ngemListIterator_t it,
    ngemListIterator_t first,
    ngemListIterator_t last)
{
    ngemlListNode_t *end = last->ngn_prev;

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

void *
ngemListHead(
    ngemList_t *list)
{
    ngemListIterator_t first;
    ngemListIterator_t last;
    NGEM_FNAME_TAG(ngemListHead);

    first = ngemListBegin(list);
    last  = ngemListEnd(list);
    if (first == last) {
        return NULL;
    } else {
        return ngemListGet(first);
    }
}

void *
ngemListTail(
    ngemList_t *list)
{
    ngemListIterator_t first;
    ngemListIterator_t last;
    NGEM_FNAME_TAG(ngemListTail);

    first = ngemListBegin(list);
    last  = ngemListEnd(list);
    if (first == last) {
        return NULL;
    } else {
        return ngemListGet(ngemListPrev(last));
    }
}
