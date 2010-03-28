/*
 * $RCSfile: ngisList.h,v $ $Revision: 1.3 $ $Date: 2008/05/15 05:54:02 $
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
#ifndef _NGIS_LIST_H_
#define _NGIS_LIST_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

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

#define NGIS_CHECK_TYPE(type, val) (1 ? (val): (type)0)
#define NGIS_CHECK_LIST(type, val) (&(NGIS_CHECK_TYPE(NGIS_LIST_OF(type) *, val)->ngl_list))
#define NGIS_CHECK_ITER(type, val) (&(NGIS_CHECK_TYPE(NGIS_LIST_ITERATOR_OF(type), val)->ngn_node))

/* Macros for linked list(typesafe) */
#define NGIS_LIST_INITIALIZE(type, list) \
    (ngisListInitialize(NGIS_CHECK_LIST(type, list)))
#define NGIS_LIST_FINALIZE(type, list) \
    (ngisListFinalize(NGIS_CHECK_LIST(type, list)))

#define NGIS_LIST_BEGIN(type, list) \
    ((NGIS_LIST_ITERATOR_OF(type))ngisListBegin(NGIS_CHECK_LIST(type, (list))))
#define NGIS_LIST_END(type, list) \
    ((NGIS_LIST_ITERATOR_OF(type))ngisListEnd(NGIS_CHECK_LIST(type, (list))))
#define NGIS_LIST_IS_EMPTY(type, list) \
    (NGIS_LIST_BEGIN(type, (list)) == NGIS_LIST_END(type, (list)))

#define NGIS_LIST_NEXT(type, it) \
    ((NGIS_LIST_ITERATOR_OF(type)) ngisListNext(NGIS_CHECK_ITER(type, (it))))
#define NGIS_LIST_PREV(type, it) \
    ((NGIS_LIST_ITERATOR_OF(type)) ngisListPrev(NGIS_CHECK_ITER(type, (it))))
#define NGIS_LIST_GET(type, it) \
    ((type *) ngisListGet(NGIS_CHECK_ITER(type, (it))))
#define NGIS_LIST_ITERATOR_IS_VALID(type, it) \
    (ngisListIteratorIsValid(NGIS_CHECK_ITER(type, (it))))

#define NGIS_LIST_INSERT_HEAD(type, list, content) \
    (ngisListInsertHead(NGIS_CHECK_LIST(type, (list)), NGIS_CHECK_TYPE(type *, (content))))
#define NGIS_LIST_INSERT_TAIL(type, list, content) \
    (ngisListInsertTail(NGIS_CHECK_LIST(type, (list)), NGIS_CHECK_TYPE(type *, (content))))
#define NGIS_LIST_INSERT(type, it, content) \
    (ngisListInsert(NGIS_CHECK_ITER(type, (it)), NGIS_CHECK_TYPE(type *, (content))))
#define NGIS_LIST_ERASE(type, it) \
    ((NGIS_LIST_ITERATOR_OF(type))ngisListErase(NGIS_CHECK_ITER(type, (it))))
#define NGIS_LIST_ERASE_BY_ADDRESS(type, list, content) \
    (ngisListEraseByAddress(NGIS_CHECK_LIST(type, (list)), NGIS_CHECK_TYPE(type *,(content))))
#define NGIS_LIST_SPLICE1(type, it, src) \
    (ngisListSplice1(NGIS_CHECK_ITER(type, (it)), NGIS_CHECK_ITER(type, (src))))
#define NGIS_LIST_SPLICE2(type, it, first, last) \
    (ngisListSplice2(NGIS_CHECK_ITER(type, (it)), NGIS_CHECK_ITER(type, (first)), NGIS_CHECK_ITER(type, (last))))

#endif /* _NGIS_LIST_H_ */
