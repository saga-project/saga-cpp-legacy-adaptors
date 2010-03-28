/*
 * $RCSfile: ngisList.h,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $
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
#ifndef _NGIS_LIST_H_
#define _NGIS_LIST_H_

#include "ngEnvironment.h"

/**
 * Node of linked list
 */
typedef struct ngislListNode_s {
    struct ngislListNode_s *ngn_next;
    struct ngislListNode_s *ngn_prev;
    void                   *ngn_content;
} ngislListNode_t;

/**
 * Linked list
 */
typedef struct ngisList_s {
    ngislListNode_t ngl_dummy;
} ngisList_t;

/**
 * Iterator of linked list
 */
typedef ngislListNode_t *ngisListIterator_t;

/* Functions */
void ngisListInitialize(ngisList_t *);
void ngisListFinalize(ngisList_t *);

ngisListIterator_t ngisListBegin(ngisList_t *);
ngisListIterator_t ngisListEnd(ngisList_t *);
ngisListIterator_t ngisListNext(ngisListIterator_t);
ngisListIterator_t ngisListPrev(ngisListIterator_t);
void *ngisListGet(ngisListIterator_t);
int ngisListIteratorIsValid(ngisListIterator_t);

int ngisListInsertHead(ngisList_t *, void *);
int ngisListInsertTail(ngisList_t *, void *);
ngisListIterator_t ngisListInsert(ngisListIterator_t, void *);
ngisListIterator_t ngisListErase(ngisListIterator_t);
int ngisListEraseByAddress(ngisList_t *, void *);
void ngisListSplice1(ngisListIterator_t, ngisListIterator_t);
void ngisListSplice2(ngisListIterator_t,
    ngisListIterator_t, ngisListIterator_t);

/**
 * Linked list(typesafe).
 */
#define NGIS_LIST_OF(type) struct ngisListOf##type##_s

/**
 * Iterator of linked list(typesafe).
 */
#define NGIS_LIST_ITERATOR_OF(type) ngisListIteratorOf##type##_t

/**
 * declare of linked list(typesafe).
 */
#define NGIS_DECLARE_LIST_OF(type)                 \
NGIS_LIST_OF(type) {                               \
    ngisList_t ngl_list;                           \
};                                                 \
typedef struct ngislListNodeOf##type##_s {         \
    ngislListNode_t ngn_node;                      \
} * NGIS_LIST_ITERATOR_OF(type)

#define NGIS_LIST_SET_INVALID_VALUE(list)           \
    do {                                            \
        (list)->ngl_list.ngl_dummy.ngn_next = NULL; \
        (list)->ngl_list.ngl_dummy.ngn_prev = NULL; \
    } while (0)
#define NGIS_LIST_IS_INVALID_VALUE(list) \
    (((list)->ngl_list.ngl_dummy.ngn_next == NULL) ||\
     ((list)->ngl_list.ngl_dummy.ngn_prev == NULL))

typedef void (*ngisVoidFuncPtr)(void);

/* Macros for linked list(typesafe) */
#define NGIS_LIST_INITIALIZE(type, list) \
    ((void (*)(NGIS_LIST_OF(type) *))(ngisVoidFuncPtr)ngisListInitialize)(list)
#define NGIS_LIST_FINALIZE(type, list) \
    ((void (*)(NGIS_LIST_OF(type) *))(ngisVoidFuncPtr)ngisListFinalize)(list)

#define NGIS_LIST_BEGIN(type, list) \
    ((NGIS_LIST_ITERATOR_OF(type) (*)(NGIS_LIST_OF(type) *))(ngisVoidFuncPtr)\
     ngisListBegin)(list)
#define NGIS_LIST_END(type, list) \
    ((NGIS_LIST_ITERATOR_OF(type) (*)(NGIS_LIST_OF(type) *))(ngisVoidFuncPtr)\
     ngisListEnd)(list)
#define NGIS_LIST_IS_EMPTY(type, list) \
    (NGIS_LIST_BEGIN(type, (list)) == NGIS_LIST_END(type, (list)))

#define NGIS_LIST_NEXT(type, it) \
    ((NGIS_LIST_ITERATOR_OF(type) (*)(NGIS_LIST_ITERATOR_OF(type)))\
     (ngisVoidFuncPtr)ngisListNext)(it)
#define NGIS_LIST_PREV(type, it) \
    ((NGIS_LIST_ITERATOR_OF(type) (*)(NGIS_LIST_ITERATOR_OF(type)))\
     (ngisVoidFuncPtr)ngisListPrev)(it)
#define NGIS_LIST_GET(type, it) \
    ((type* (*)(NGIS_LIST_ITERATOR_OF(type)))(ngisVoidFuncPtr)ngisListGet)(it)
#define NGIS_LIST_ITERATOR_IS_VALID(type, it) \
    ((int (*)(NGIS_LIST_ITERATOR_OF(type)))(ngisVoidFuncPtr)\
     ngisListIteratorIsValid)(it)

#define NGIS_LIST_INSERT_HEAD(type, list, content) \
    ((int (*)(NGIS_LIST_OF(type) *, type *))(ngisVoidFuncPtr)\
     ngisListInsertHead)(list, content)
#define NGIS_LIST_INSERT_TAIL(type, list, content) \
    ((int (*)(NGIS_LIST_OF(type) *, type *))(ngisVoidFuncPtr)\
     ngisListInsertTail)(list, content)
#define NGIS_LIST_INSERT(type, it, content) \
    ((NGIS_LIST_ITERATOR_OF(type) (*)(NGIS_LIST_ITERATOR_OF(type), type *))\
     (ngisVoidFuncPtr)ngisListInsert)(it, content)
#define NGIS_LIST_ERASE(type, it) \
    ((NGIS_LIST_ITERATOR_OF(type) (*)(NGIS_LIST_ITERATOR_OF(type)))\
     (ngisVoidFuncPtr)ngisListErase)(it)
#define NGIS_LIST_ERASE_BY_ADDRESS(type, list, content) \
    ((int (*)(NGIS_LIST_OF(type) *, type *))(ngisVoidFuncPtr)\
     ngisListEraseByAddress)(list, content)
#define NGIS_LIST_SPLICE1(type, it, src) \
    ((void (*)(NGIS_LIST_ITERATOR_OF(type), NGIS_LIST_ITERATOR_OF(type)))\
     (ngisVoidFuncPtr)ngisListSplice1)(it, src)
#define NGIS_LIST_SPLICE2(type, it, first, last) \
    ((void (*)(NGIS_LIST_ITERATOR_OF(type), NGIS_LIST_ITERATOR_OF(type),\
    NGIS_LIST_ITERATOR_OF(type)))(ngisVoidFuncPtr)ngisListSplice2)\
    (it, first, last)

#endif /* _NGIS_LIST_H_ */
