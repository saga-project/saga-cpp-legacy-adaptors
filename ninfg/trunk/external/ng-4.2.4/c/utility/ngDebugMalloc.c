#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngDebugMalloc.c,v $ $Revision: 1.11 $ $Date: 2004/03/11 07:29:02 $";
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
 * Module for malloc/free debugging
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ng.h"

#define NGCLL_CHECK_BUFFER_SIZE     (1024 * sizeof(int))
#define NGCLL_CHECK_BUFFER_INITIAL  0xffaa5500

static void nglDebugMallocCheckAreaSet(void *checkAddr);
static int nglDebugMallocCheckAreaCheck(void *checkAddr);

/**
 * Note: debug malloc allocates
 *   check buffer area (head) : size=NGCLL_CHECK_BUFFER_SIZE * sizeof(int)
 *   size                     : size=sizeof(size_t)
 *   real malloc()ed address  : size=sizeof(void **)
 *   check buffer area (tail) : size=NGCLL_CHECK_BUFFER_SIZE * sizeof(int)
 *
 * check buffer area is for to check if illegal memory destruction
 *   was happened.
 */


/**
 * debug malloc
 */
void *
ngiDebugMalloc(size_t size)
{
    size_t realSize;
    void *realAddr, *returnAddr, *ckBufAddr;
    void **realAddrPtr;
    size_t *sizePtr;

    realSize = NGCLL_CHECK_BUFFER_SIZE * 2 + sizeof(size_t) +
                 sizeof(void *) + size;
    realAddr = globus_libc_malloc(realSize);
    if (realAddr == NULL) {
        return NULL;
    }

    returnAddr = (char *)realAddr + NGCLL_CHECK_BUFFER_SIZE +
        sizeof(size_t) + sizeof(void *);

    ckBufAddr = realAddr;
    nglDebugMallocCheckAreaSet(ckBufAddr);

    ckBufAddr = (char *)realAddr + NGCLL_CHECK_BUFFER_SIZE + sizeof(size_t)
               + sizeof(void *) + size;
    nglDebugMallocCheckAreaSet(ckBufAddr);

    realAddrPtr = (void **)returnAddr - 1;
    *realAddrPtr = realAddr;

    sizePtr = (size_t *)((void **)returnAddr - 2);
    *sizePtr = size;
 
    return returnAddr;
}

/**
 * debug free
 */
void
ngiDebugFree(void *addr)
{
    void *realAddr, *ckBufAddr;
    void **realAddrPtr;
    size_t *sizePtr;
    size_t size;
    int result;
 
    realAddrPtr = (void **)addr - 1;
    realAddr = *realAddrPtr;

    sizePtr = (size_t *)((void **)addr - 2);
    size = *sizePtr;

    ckBufAddr = realAddr;
    result = nglDebugMallocCheckAreaCheck(ckBufAddr);
    assert(result == 1);
    
    ckBufAddr = (char *)realAddr + NGCLL_CHECK_BUFFER_SIZE +
        sizeof(size_t) + sizeof(void *) + size;
    result = nglDebugMallocCheckAreaCheck(ckBufAddr);
    assert(result == 1);

    globus_libc_free(realAddr);
}

static void
nglDebugMallocCheckAreaSet(void *checkAddr)
{
    int i, maxCount;
    int *mallocCheckBuffer;
    int c;

    mallocCheckBuffer = (int *)checkAddr;
    maxCount = NGCLL_CHECK_BUFFER_SIZE / sizeof(int);
    c = NGCLL_CHECK_BUFFER_INITIAL;
    for (i = 0; i < maxCount; i++) {
        mallocCheckBuffer[i] = c;
    }
}

static int
nglDebugMallocCheckAreaCheck(void *checkAddr)
{
    int i, maxCount;
    int *mallocCheckBuffer;
    int c;

    mallocCheckBuffer = (int *)checkAddr;
    maxCount = NGCLL_CHECK_BUFFER_SIZE / sizeof(int);
    c = NGCLL_CHECK_BUFFER_INITIAL;
    for (i = 0; i < maxCount; i++) {
        assert(mallocCheckBuffer[i] == c);
        if (mallocCheckBuffer[i] != c) {
            return 0;
        }
    }

    return 1;
}

/**
 * calloc
 */
void *
ngiDebugCalloc(size_t nmemb, size_t size)
{
    char *addr;

    addr = ngiDebugMalloc(nmemb * size);
    if (addr == NULL) {
        return NULL;
    }
    memset(addr, 0, nmemb * size);

    return addr;
}

/**
 * realloc
 */
void *
ngiDebugRealloc(void *ptr, size_t size)
{
    void *newAddr;
    size_t *prevSizePtr;
    size_t prevSize, copySize;
 
    if (ptr == NULL) {
        return ngiDebugMalloc(size);
    }

    if (size == 0) {
        ngiDebugFree(ptr);
        return NULL;
    }

    prevSizePtr = (size_t *)((void **)ptr - 2);
    prevSize = *prevSizePtr;

    newAddr = ngiDebugMalloc(size);
    if (newAddr == NULL) {
        return NULL;
    }

    copySize = (prevSize < size ? prevSize : size);
    memcpy(newAddr, ptr, copySize);
    ngiDebugFree(ptr);

    return newAddr;
}

/**
 * strdup using debug malloc
 */
char *
ngiDebugStrdup(char *str)
{
    int newLen;
    char *newStr;

    newLen = strlen(str) + 1;
    newStr = ngiDebugMalloc(newLen * sizeof(char));
    if (newStr == NULL) {
        return NULL;
    }
    memcpy(newStr, str, newLen);

    return newStr;
}

