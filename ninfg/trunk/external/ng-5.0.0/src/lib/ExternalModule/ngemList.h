/*
 * $RCSfile: ngemList.h,v $ $Revision: 1.3 $ $Date: 2008/02/25 05:21:46 $
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
#ifndef _NGEM_LIST_H_
#define _NGEM_LIST_H_

#include "ngemEnvironment.h"
#include "ngemType.h"

#define NGEM_LOGCAT_LIST   "EM List"

/**
 * Node of linked list
 */
typedef struct ngemlListNode_s {
    struct ngemlListNode_s *ngn_next;
    struct ngemlListNode_s *ngn_prev;
    void                   *ngn_content;
} ngemlListNode_t;

/**
 * Linked list
 */
typedef struct ngemList_s {
    ngemlListNode_t ngl_dummy;
} ngemList_t;

/**
 * Iterator of linked list
 */
typedef ngemlListNode_t *ngemListIterator_t;

/* Functions */
void ngemListInitialize(ngemList_t *);
void ngemListFinalize(ngemList_t *);

ngemListIterator_t ngemListBegin(ngemList_t *);
ngemListIterator_t ngemListEnd(ngemList_t *);
ngemListIterator_t ngemListNext(ngemListIterator_t);
ngemListIterator_t ngemListPrev(ngemListIterator_t);
void *ngemListGet(ngemListIterator_t);
bool ngemListIteratorIsValid(ngemListIterator_t);

ngemResult_t ngemListInsertHead(ngemList_t *, void *);
ngemResult_t ngemListInsertTail(ngemList_t *, void *);
ngemListIterator_t ngemListInsert(ngemListIterator_t, void *);
ngemListIterator_t ngemListErase(ngemListIterator_t);
ngemResult_t ngemListEraseByAddress(ngemList_t *, void *);
void ngemListSplice1(ngemListIterator_t, ngemListIterator_t);
void ngemListSplice2(ngemListIterator_t,
    ngemListIterator_t, ngemListIterator_t);
void *ngemListHead(ngemList_t *);
void *ngemListTail(ngemList_t *);

/**
 * Linked list(typesafe).
 */
#define NGEM_LIST_OF(type) struct ngemListOf##type##_s

/**
 * Iterator of linked list(typesafe).
 */
#define NGEM_LIST_ITERATOR_OF(type) ngemListIteratorOf##type##_t

/**
 * declare of linked list(typesafe).
 */
#define NGEM_DECLARE_LIST_OF(type)                 \
NGEM_LIST_OF(type) {                               \
    ngemList_t ngl_list;                           \
};                                                 \
typedef struct ngemlListNodeOf##type##_s {         \
    ngemlListNode_t ngn_node;                      \
} * NGEM_LIST_ITERATOR_OF(type)

#define NGEM_LIST_SET_INVALID_VALUE(list)           \
    do {                                            \
        (list)->ngl_list.ngl_dummy.ngn_next = NULL; \
        (list)->ngl_list.ngl_dummy.ngn_prev = NULL; \
    } while (0)
#define NGEM_LIST_IS_INVALID_VALUE(list) \
    (((list)->ngl_list.ngl_dummy.ngn_next == NULL) ||\
     ((list)->ngl_list.ngl_dummy.ngn_prev == NULL))

typedef void (*ngemVoidFuncPtr)(void);

/* Macros for linked list(typesafe) */
#define NGEM_LIST_INITIALIZE(type, list) \
    ((void (*)(NGEM_LIST_OF(type) *))(ngemVoidFuncPtr)ngemListInitialize)(list)
#define NGEM_LIST_FINALIZE(type, list) \
    ((void (*)(NGEM_LIST_OF(type) *))(ngemVoidFuncPtr)ngemListFinalize)(list)

#define NGEM_LIST_BEGIN(type, list) \
    ((NGEM_LIST_ITERATOR_OF(type) (*)(NGEM_LIST_OF(type) *))(ngemVoidFuncPtr)\
     ngemListBegin)(list)
#define NGEM_LIST_END(type, list) \
    ((NGEM_LIST_ITERATOR_OF(type) (*)(NGEM_LIST_OF(type) *))(ngemVoidFuncPtr)\
     ngemListEnd)(list)
#define NGEM_LIST_IS_EMPTY(type, list) \
    (NGEM_LIST_BEGIN(type, (list)) == NGEM_LIST_END(type, (list)))

#define NGEM_LIST_NEXT(type, it) \
    ((NGEM_LIST_ITERATOR_OF(type) (*)(NGEM_LIST_ITERATOR_OF(type)))\
     (ngemVoidFuncPtr)ngemListNext)(it)
#define NGEM_LIST_PREV(type, it) \
    ((NGEM_LIST_ITERATOR_OF(type) (*)(NGEM_LIST_ITERATOR_OF(type)))\
     (ngemVoidFuncPtr)ngemListPrev)(it)
#define NGEM_LIST_GET(type, it) \
    ((type* (*)(NGEM_LIST_ITERATOR_OF(type)))(ngemVoidFuncPtr)ngemListGet)(it)
#define NGEM_LIST_ITERATOR_IS_VALID(type, it) \
    ((bool (*)(NGEM_LIST_ITERATOR_OF(type)))(ngemVoidFuncPtr)\
     ngemListIteratorIsValid)(it)

#define NGEM_LIST_INSERT_HEAD(type, list, content) \
    ((ngemResult_t (*)(NGEM_LIST_OF(type) *, type *))(ngemVoidFuncPtr)\
     ngemListInsertHead)(list, content)
#define NGEM_LIST_INSERT_TAIL(type, list, content) \
    ((ngemResult_t (*)(NGEM_LIST_OF(type) *, type *))(ngemVoidFuncPtr)\
     ngemListInsertTail)(list, content)
#define NGEM_LIST_INSERT(type, it, content) \
    ((NGEM_LIST_ITERATOR_OF(type) (*)(NGEM_LIST_ITERATOR_OF(type), type *))\
     (ngemVoidFuncPtr)ngemListInsert)(it, content)
#define NGEM_LIST_ERASE(type, it) \
    ((NGEM_LIST_ITERATOR_OF(type) (*)(NGEM_LIST_ITERATOR_OF(type)))\
     (ngemVoidFuncPtr)ngemListErase)(it)
#define NGEM_LIST_ERASE_BY_ADDRESS(type, list, content) \
    ((ngemResult_t (*)(NGEM_LIST_OF(type) *, type *))(ngemVoidFuncPtr)\
     ngemListEraseByAddress)(list, content)
#define NGEM_LIST_SPLICE1(type, it, src) \
    ((void (*)(NGEM_LIST_ITERATOR_OF(type), NGEM_LIST_ITERATOR_OF(type)))\
     (ngemVoidFuncPtr)ngemListSplice1)(it, src)
#define NGEM_LIST_SPLICE2(type, it, first, last) \
    ((void (*)(NGEM_LIST_ITERATOR_OF(type), NGEM_LIST_ITERATOR_OF(type),\
    NGEM_LIST_ITERATOR_OF(type)))(ngemVoidFuncPtr)ngemListSplice2)\
    (it, first, last)
#define NGEM_LIST_HEAD(type, list) \
    ((type *(*)(NGEM_LIST_OF(type) *))(ngemVoidFuncPtr)ngemListHead)(list)
#define NGEM_LIST_TAIL(type, list) \
    ((type *(*)(NGEM_LIST_OF(type) *))(ngemVoidFuncPtr)ngemListTail)(list)

/* FOREACHs */
#define NGEM_LIST_FOREACH(type, list, it, var) \
    for (it = NGEM_LIST_BEGIN(type, list);     \
         (it != NGEM_LIST_END(type, list)) &&  \
         (var = NGEM_LIST_GET(type, it));      \
         it = NGEM_LIST_NEXT(type, it))

#define NGEM_LIST_ERASE_EACH(type, list, it, var) \
    for (it = NGEM_LIST_BEGIN(type, list);     \
         (it != NGEM_LIST_END(type, list)) &&  \
         (var = NGEM_LIST_GET(type, it));      \
         it = NGEM_LIST_ERASE(type, it))


#endif /* _NGEM_LIST_H_ */
