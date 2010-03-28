#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngStream.c,v $ $Revision: 1.59 $ $Date: 2007/05/14 05:10:45 $";
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
 * Module of managing Stream Buffer for Ninf-G Client/Executable.
 */

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */

static int nglNonImplementationStreamManagerDestroyWriteData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglNonImplementationStreamManagerDestroyReadData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglNonImplementationStreamManagerGetWritableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglNonImplementationStreamManagerWriteBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglNonImplementationStreamManagerGetReadableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglNonImplementationStreamManagerReadBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglNonImplementationStreamManagerWriteDirectly(ngiStreamManager_t *, void *, size_t, ngLog_t *, int *);
#if 0
static int nglNonImplementationStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, size_t *, ngLog_t *, int *);
static int nglNonImplementationStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);
#endif

static void nglStreamManagerInitializeMember(ngiStreamManager_t *);
static void nglStreamManagerInitializePointer(ngiStreamManager_t *);
static void nglStreamBufferInitializeMember(ngiStreamBuffer_t *stream);
static void nglStreamBufferInitializePointer(ngiStreamBuffer_t *stream);
static int nglStreamManagerReceive(ngiStreamManager_t *sMng, ngiCommunication_t *comm, size_t nBytes, int isFull, ngLog_t *log, int *error);

static void nglMemoryStreamManagerInitializeMember(ngiMemoryStreamManager_t *);
static void nglMemoryStreamManagerInitializePointer(ngiMemoryStreamManager_t *);
static int nglMemoryStreamManagerDestroyData(ngiMemoryStreamManager_t *, ngLog_t *, int *);

static int nglMemoryStreamManagerDestroyWriteData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglMemoryStreamManagerDestroyReadData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglMemoryStreamManagerGetWritableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglMemoryStreamManagerWriteBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglMemoryStreamManagerGetReadableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglMemoryStreamManagerReadBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglMemoryStreamManagerWriteDirectly(ngiStreamManager_t *sMng, void *buf, size_t nBytes, ngLog_t *log, int *error);
static int nglMemoryStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglMemoryStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

static int nglMemoryStreamManagerGrow(ngiMemoryStreamManager_t *, ngLog_t *, int *);
static int nglMemoryStreamManagerGrowWrite(ngiMemoryStreamManager_t *, ngLog_t *, int *);

ngiStreamManagerTypeInformation_t nglMemoryStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglMemoryStreamManagerDestroyWriteData,
    nglMemoryStreamManagerDestroyReadData,

    /* Buffer */
    nglMemoryStreamManagerGetWritableBuffer,
    nglMemoryStreamManagerWriteBuffer,
    nglMemoryStreamManagerGetReadableBuffer,
    nglMemoryStreamManagerReadBuffer,

    nglMemoryStreamManagerWriteDirectly,

    /* Get Size*/
    nglMemoryStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglMemoryStreamManagerDestruct
};

#define nglStreamManagerDestruct(sMng, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_destruct(sMng, log, error))

#if defined(NG_OS_IRIX) || (__INTEL_COMPILER)
#define nglAssertSizet(expr) assert(1)
#else /* defined(NG_OS_IRIX) || (__INTEL_COMPILER) */
#define nglAssertSizet(expr) assert(expr)
#endif /* defined(NG_OS_IRIX) || (__INTEL_COMPILER) */

/**
 * Memory Stream Manager: Construct
 */
ngiStreamManager_t *
ngiMemoryStreamManagerConstruct(size_t growNbytes, ngLog_t *log, int *error)
{
    int result;
    ngiMemoryStreamManager_t *sMng;
    static const char fName[] = "ngiMemoryStreamManagerConstruct";

    /* Allocate */
    sMng = ngiMemoryStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiMemoryStreamManagerInitialize(sMng, growNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert((void *)&sMng->ngmsm_base == (void *)sMng);
    return &sMng->ngmsm_base;

    /* Error occurred */
error:
    result = ngiMemoryStreamManagerFree(sMng, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
    }

    return 0;
}

/**
 * Stream Manager: Destruct all
 */
int
ngiStreamManagerDestruct(ngiStreamManager_t *sMng, ngLog_t *log, int *error)
{
    int result;
    ngiStreamManager_t *curMng, *nextMng;
    static const char fName[] = "ngiStreamManagerDestruct";

    curMng = sMng;
    while (curMng != NULL) {
        nextMng = curMng->ngsm_next;
        result = nglStreamManagerDestruct(curMng, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager.\n", fName);
            return 0;
        }
        curMng = nextMng;
    }

    /* Success */
    return 1;
}

/**
 * Stream Manager: Destruct the Stream Manager read.
 */
int
ngiStreamManagerDestructReadAlready(ngiStreamManager_t **sMng, ngLog_t *log, int *error)
{
    int result;
    size_t readableNbytes;
    void *dummy;
    ngiStreamManager_t *curMng, *nextMng;
    static const char fName[] = "ngiStreamManagerDestructReadAlready";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(*sMng != NULL);

    curMng = *sMng;
    while (curMng != NULL) {
        result = ngiStreamManagerGetReadableBuffer(
            curMng, &dummy, 0, &readableNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get readable buffer from the Stream Manager.\n", fName);
            return 0;
        }
        if (readableNbytes > 0) {
            break;
        }

        nextMng = curMng->ngsm_next;
        result = nglStreamManagerDestruct(curMng, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Stream Manager.\n", fName);
            return 0;
        }
        curMng = nextMng;
    }
    *sMng = curMng;

    /* Success */
    return 1;
}

/**
 * Memory Stream Buffer Manager: Destruct
 */
static int
nglMemoryStreamManagerDestruct(ngiStreamManager_t *sMng, ngLog_t *log, int *error)
{
    int result;
    ngiMemoryStreamManager_t *msMng;
    static const char fName[] = "nglMemoryStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);

    msMng = (ngiMemoryStreamManager_t *)sMng;

    /* Finalize */
    result = ngiMemoryStreamManagerFinalize(msMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiMemoryStreamManagerFree(msMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Memory Stream Buffer Manager: Allocate
 */
ngiMemoryStreamManager_t *
ngiMemoryStreamManagerAllocate(ngLog_t *log, int *error)
{
    ngiMemoryStreamManager_t *sMng;
    static const char fName[] = "ngiMemoryStreamManagerAllocate";

    /* Allocate */
    sMng = globus_libc_calloc(1, sizeof (ngiMemoryStreamManager_t));
    if (sMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return sMng;
}

/**
 * Memory Stream Buffer Manager: Free
 */
int
ngiMemoryStreamManagerFree(ngiMemoryStreamManager_t *msMng, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(msMng != NULL);

    /* Free */
    globus_libc_free(msMng);

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Initialize
 */
int
ngiMemoryStreamManagerInitialize(
    ngiMemoryStreamManager_t *msMng,
    size_t growNbytes,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(msMng != NULL);

    /* Initialize the members */
    ngiStreamManagerInitialize(&msMng->ngmsm_base, &nglMemoryStreamManagerTypeInfomation, log, error);
    nglMemoryStreamManagerInitializeMember(msMng);
    msMng->ngmsm_growNbytes = growNbytes;

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Finalize
 */
int
ngiMemoryStreamManagerFinalize(ngiMemoryStreamManager_t *msMng, ngLog_t *log, int *error)
{
    int result;
    ngiStreamBuffer_t *stream, *next;
    static const char fName[] = "ngiMemoryStreamManagerFinalize";

    /* Check the arguments */
    assert(msMng != NULL);

    /* Destruct the Stream Buffers */
    for (stream = msMng->ngmsm_head; stream != NULL; stream = next) {
	next = stream->ngsb_next;
    	result = ngiStreamBufferDestruct(stream, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't destruct the Stream Buffer.\n", fName);
	    return 0;
	}
    }

    /* Initialize the members */
    nglMemoryStreamManagerInitializeMember(msMng);
    result = ngiStreamManagerFinalize(&msMng->ngmsm_base, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Initialize the members.
 */
static void
nglMemoryStreamManagerInitializeMember(ngiMemoryStreamManager_t *msMng)
{
    /* Initialize the pointers */
    nglMemoryStreamManagerInitializePointer(msMng);

    /* Initialize the members */
    msMng->ngmsm_bufferNbytes = 0;
    msMng->ngmsm_growNbytes = 0;
    msMng->ngmsm_writeNbytes = 0;
    msMng->ngmsm_readNbytes = 0;
}

/**
 * Memory Stream Manager: Initialize the pointers.
 */
static void
nglMemoryStreamManagerInitializePointer(ngiMemoryStreamManager_t *msMng)
{
    /* Initialize the pointers */
    msMng->ngmsm_head = NULL;
    msMng->ngmsm_tail = NULL;
    msMng->ngmsm_write = NULL;
    msMng->ngmsm_read = NULL;
}

/**
 * Stream Manager: Append the Stream Manager at last of list.
 */
int
ngiStreamManagerAppend(
    ngiStreamManager_t *sMng,
    ngiStreamManager_t *append,
    ngLog_t *log,
    int *error)
{
    ngiStreamManager_t *curr;

    /* Check the arguments */
    assert(sMng != NULL);
    assert(append != NULL);

    /* Find the last of list */
    for (curr = sMng; curr->ngsm_next != NULL; curr = curr->ngsm_next) {
        /* Do Nothing. */
    }

    /* Append the Stream Manager */
    curr->ngsm_next = append;

    /* Success */
    return 1;
}

/**
 * Stream Manager: Delete the Stream Manager from list.
 */
int
ngiStreamManagerDelete(
    ngiStreamManager_t *sMng,
    ngiStreamManager_t *delete,
    ngLog_t *log,
    int *error)
{
    ngiStreamManager_t *curr;
    static const char fName[] = "ngiStreamManagerDelete";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(delete != NULL);

    for (curr = sMng; curr->ngsm_next != NULL; curr = curr->ngsm_next) {
    	if (curr->ngsm_next == delete) {
	    curr->ngsm_next = delete->ngsm_next;
	    delete->ngsm_next = NULL;

	    /* Success */
	    return 1;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
    	"%s: Can't find the Stream Manager.\n", fName);

    /* Failed */
    return 0;
}

/**
 * Stream Manager: Get next.
 */
ngiStreamManager_t *
ngiStreamManagerGetNext(
	ngiStreamManager_t *sMng,
	ngLog_t *log,
	int *error)
{
    /* Check the arguments */
    assert(sMng != NULL);

    return sMng->ngsm_next;
}

/**
 * Memory Stream Manager: Grow the Stream Buffer for write.
 */
static int
nglMemoryStreamManagerGrowWrite(ngiMemoryStreamManager_t *msMng, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "nglMemoryStreamManagerGrowWrite";

    /* Check the arguments */
    assert(msMng != NULL);

    result = nglMemoryStreamManagerGrow(msMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't register the Stream Buffer.\n", fName);
	return 0;
    }

    /* Update the write buffer */
    msMng->ngmsm_write = msMng->ngmsm_tail;
    if (msMng->ngmsm_read == NULL)
	msMng->ngmsm_read = msMng->ngmsm_tail;

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Grow the Stream Buffer.
 */
static int
nglMemoryStreamManagerGrow(ngiMemoryStreamManager_t *msMng, ngLog_t *log, int *error)
{
    int result;
    ngiStreamBuffer_t *stream;
    static const char fName[] = "nglMemoryStreamManagerGrow";

    /* Check the arguments */
    assert(msMng != NULL);

    assert(msMng->ngmsm_growNbytes > 0);

    /* Construct new Stream Buffer */
    stream = ngiStreamBufferConstructBuffer(msMng->ngmsm_growNbytes, log, error);
    if (stream == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't construct the Stream Buffer.\n", fName);
	return 0;
    }

    /* Register */
    result = ngiStreamBufferRegister(msMng, stream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't register the Stream Buffer.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Get writable buffer.
 */
int
nglMemoryStreamManagerGetWritableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t remainNbytes;
    ngiMemoryStreamManager_t *msMng;
    ngiStreamBuffer_t *stream;
    static const char fName[] = "nglMemoryStreamManagerGetWritableBuffer";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);
    assert(buf != NULL);
    assert(nBytes != NULL);

    msMng = (ngiMemoryStreamManager_t *)sMng;
    
    assert(msMng->ngmsm_writeNbytes >= msMng->ngmsm_readNbytes);

    if (nBytesRequired > msMng->ngmsm_growNbytes) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Bytes required is too large.\n", fName);
	return 0;
    }

    /* Is Stream Buffer exist? */
    if ((msMng->ngmsm_write != NULL) && 
        (msMng->ngmsm_write->ngsb_direct != NGI_DATA_DIRECTLY)) {
	/* Get the Stream Buffer */
	stream = msMng->ngmsm_write;

	/* Get the remain of Stream Buffer */
        assert(stream->ngsb_bufferNbytes > stream->ngsb_writeNbytes);
        remainNbytes = stream->ngsb_bufferNbytes - stream->ngsb_writeNbytes;

	/* Is there any sufficient space? */
	if ((remainNbytes > 0) &&
            (remainNbytes >= nBytesRequired)) {
	    *buf = &stream->ngsb_pointer[stream->ngsb_writeNbytes];
	    *nBytes = remainNbytes;
	    return 1;
	}
    }

    /* Grow the Stream Buffer */
    result = nglMemoryStreamManagerGrowWrite(msMng, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't grow the Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    stream = msMng->ngmsm_write;
    *buf = &stream->ngsb_pointer[0];
    *nBytes = stream->ngsb_bufferNbytes;
    return 1;
}

/**
 * Memory Stream Manager: Write data.
 * This function doesn't write in fact.
 * It advances writing index.
 */
int
nglMemoryStreamManagerWriteBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiMemoryStreamManager_t *msMng;
    ngiStreamBuffer_t *stream;
    static const char fName[] = "nglMemoryStreamManagerWriteBuffer";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);
    assert(sMng != NULL);
    nglAssertSizet(nBytes >= 0);

    msMng = (ngiMemoryStreamManager_t *)sMng;

    /* Is nBytes zero? */
    if (nBytes == 0) {
    	/* Success */
    	return 1;
    }

    /* Get the Stream Buffer */
    stream = msMng->ngmsm_write;

    /* Is nBytes valid? */
    if (nBytes + msMng->ngmsm_writeNbytes > msMng->ngmsm_bufferNbytes) {
	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Overflow the Stream Manager.\n", fName);
	return 0;
    }

    /* Is nBytes valid? */
    if (nBytes + stream->ngsb_writeNbytes > stream->ngsb_bufferNbytes) {
	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Overflow the Stream Manager.\n", fName);
	return 0;
    }

    /* Increment */
    msMng->ngmsm_writeNbytes += nBytes;
    stream->ngsb_writeNbytes += nBytes;

    /* Is Stream Buffer overflow? */
    if (stream->ngsb_writeNbytes >= stream->ngsb_bufferNbytes) {
	msMng->ngmsm_write = stream->ngsb_next;
    }

    /* Success */
    return 1;
}

/**
 * Stream Manager: Get number of total bytes of readable data.
 */
int
ngiStreamManagerGetTotalBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    size_t n;
    int result;
    static const char fName[] = "ngiStreamManagerGetTotalBytesOfReadableData";
                                                                                                                                            
    /* Check the arguments */
    assert(sMng != NULL);
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);

    *isTooLarge = 0;
    for (*nBytes = 0; sMng != NULL; sMng = sMng->ngsm_next) {
        result = ngiStreamManagerGetBytesOfReadableData(sMng, isTooLarge, &n, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                        NULL, "%s: Can't get number of bytes of readable data.\n", fName);
            return 0;
        }
        *nBytes += n;
        if (*nBytes > NGI_PROTOCOL_SIZE_MAX) {
            /* Over 2GB */
            *isTooLarge = 1;
            *nBytes = *nBytes % (NGI_PROTOCOL_SIZE_MAX + 1);
        }
    }
                                                                                                        
    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Get number of bytes of readable data.
 */
static int
nglMemoryStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiMemoryStreamManager_t *msMng;

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);

    msMng = (ngiMemoryStreamManager_t *)sMng;

    assert(msMng->ngmsm_writeNbytes >= msMng->ngmsm_readNbytes);
    *nBytes = msMng->ngmsm_writeNbytes - msMng->ngmsm_readNbytes;
    *isTooLarge = 0;

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Get readable buffer.
 */
static int
nglMemoryStreamManagerGetReadableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiMemoryStreamManager_t *msMng;
    size_t remainNbytes = 0;
    ngiStreamBuffer_t *stream;
    static const char fName[] = "nglMemoryStreamManagerGetReadableBuffer";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);
    assert(buf != NULL);
    assert(nBytes != NULL);

    msMng = (ngiMemoryStreamManager_t *)sMng;

    /* Get the Stream Buffer */
    stream = msMng->ngmsm_read;

    if (stream != NULL) {
        /* Get the remain of Stream Buffer */
        assert(stream->ngsb_writeNbytes >= stream->ngsb_readNbytes);
        remainNbytes = stream->ngsb_writeNbytes - stream->ngsb_readNbytes;
    }

    /* Is there no data? */
    if (remainNbytes < nBytesRequired) {
	NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Underflow the Stream Buffer.\n", fName);
	return  0;
    }

    /* Set the returned data */
    if (stream != NULL) {
        *buf = &stream->ngsb_pointer[stream->ngsb_readNbytes];
    }
    *nBytes = remainNbytes;

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Read data.
 */
static int
nglMemoryStreamManagerReadBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiStreamBuffer_t *stream;
    ngiMemoryStreamManager_t *msMng;
    static const char fName[] = "nglMemoryStreamManagerReadBuffer";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(nBytes > 0);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);

    msMng = (ngiMemoryStreamManager_t *)sMng;

    assert(msMng->ngmsm_read != NULL);

    /* Get the Stream Buffer */
    stream = msMng->ngmsm_read;

    /* Is nBytes valid? */
    if (nBytes + msMng->ngmsm_readNbytes > msMng->ngmsm_writeNbytes) {
	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Overflow the Stream Manager.\n", fName);
	return 0;
    }

    /* Is nBytes valid? */
    if (nBytes + stream->ngsb_readNbytes > stream->ngsb_writeNbytes) {
	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Overflow the Stream Manager.\n", fName);
	return 0;
    }

    /* Increment */
    msMng->ngmsm_readNbytes += nBytes;
    stream->ngsb_readNbytes += nBytes;

    /* Is Stream Buffer underflow */
    if (stream->ngsb_readNbytes >= stream->ngsb_bufferNbytes) {
    	msMng->ngmsm_read = stream->ngsb_next;
    }

    /* Success */
    return 1;
}

/**
 * Stream Manager: Send
 */
int
ngiStreamManagerSend(
    ngiStreamManager_t *sMng,
    ngiCommunication_t *comm,
    ngLog_t *log,
    int *error)
#ifdef NGI_AVOID_USE_WRITEV
{
    int result;
    ngiStreamManager_t *smCurr;
    void *readBuf;
    size_t canReadNbytes;
    static const char fName[] = "ngiStreamManagerSend";

    /* Check the argument */
    assert(sMng != NULL);
    assert(comm != NULL);

    smCurr = sMng;
    while (1) {
        /* Get Readable Buffer */
        canReadNbytes = 0;
        while ((smCurr != NULL) && (canReadNbytes == 0)) {
            /* Get the buffer of Stream Manager */
            result = ngiStreamManagerGetReadableBuffer(
                smCurr, &readBuf, 0, &canReadNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't get the Readable Buffer from Stream Manager.\n",
                    fName);
                return 0;
            }
            if (canReadNbytes == 0) {
                /* Check the remaining buffer of Stream Manager */
                smCurr = smCurr->ngsm_next;
            }
        }
        if (smCurr == NULL) {
            break;
        }

        result = ngiCommunicationSend(comm, readBuf, canReadNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL, "%s: Can't send data.\n", fName);
            return 0;
        }

        /* Read from Stream Buffer */
        result = ngiStreamManagerReadBuffer(
            smCurr, canReadNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR,     NULL,
                "%s: Can't read the Stream Buffer.\n", fName);
            return 0;
        }
    }

    /* Destroy the data */
    result = ngiStreamManagerDestroyWriteData(sMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destroy the data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}
#else /* NGI_AVOID_USE_WRITEV */
{
    int result;
    int i, nStreams;
    ngiStreamManager_t *smCurr;
    ngiStreamBuffer_t *stream;
    struct iovec *iov;
    static const char fName[] = "ngiStreamManagerSend";

    /* Check the argument */
    assert(sMng != NULL);
    assert(comm != NULL);

    /* How many Streams? */
    nStreams = 0;
    for (smCurr = sMng; smCurr != NULL; smCurr = smCurr->ngsm_next) {
	for (stream = smCurr->ngsm_head;
	    stream != NULL; stream = stream->ngsb_next) {
	    if (stream->ngsb_writeNbytes > 0)
	    	nStreams++;
	}
    }

    /* Allocate the iov */
    iov = globus_libc_calloc(nStreams, sizeof (struct iovec));
    if (iov == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for iov.\n", fName);
	return 0;
    }

    /* Make iov */
    i = 0;
    for (smCurr = sMng; smCurr != NULL; smCurr = smCurr->ngsm_next) {
	for (stream = smCurr->ngsm_head; stream != NULL; stream = stream->ngsb_next) {
	    if (stream->ngsb_writeNbytes > 0) {
		iov[i].iov_base = (char *)stream->ngsb_pointer;
		iov[i].iov_len = stream->ngsb_writeNbytes;
		i++;
	    }
	}
    }

    /* Send */
    result = ngiCommunicationSendIovec(comm, iov, nStreams, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send data.\n", fName);
	goto error;
    }

    /* Destroy the data */
    result = ngiStreamManagerDestroyWriteData(sMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destroy the data.\n", fName);
	goto error;
    }
    
    /* Deallocate the iov */
    globus_libc_free(iov);

    /* Success */
    return 1;

    /* Error occurred */
error:
    globus_libc_free(iov);

    return 0;
}
#endif /* NGI_AVOID_USE_WRITEV */

/**
 * Stream Manager: Receive
 *
 * Try receive the data to full the current Stream Buffer, or nBytes receive.
 */
int
ngiStreamManagerReceiveTry(
    ngiStreamManager_t *sMng,
    ngiCommunication_t *comm,
    size_t nBytes,
    size_t *receiveNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t maxNbytes;
    size_t remainNbytes;
    size_t receivedNbytes;
    void *wBuf;
    int isTooLarge;
    static const char fName[] = "ngiStreamManagerReceiveTry";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(comm != NULL);
    assert(receiveNbytes != NULL);

    result = ngiStreamManagerGetTotalBytesOfReadableData(sMng, &isTooLarge, &remainNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the total bytes of remainig data.\n", fName);
	return 0;
    }

    if (remainNbytes >= nBytes) {
        *receiveNbytes = remainNbytes;

    	/* Success */
        return 1;
    }

    /* Get the Writable Buffer */
    result = ngiStreamManagerGetWritableBuffer(sMng, &wBuf, 1, &maxNbytes,log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL, "%s: Can't grow the Stream Buffer.\n", fName);
        return 0;
    }

    if (nBytes < maxNbytes) {
        maxNbytes = nBytes;
    }

    /* Receive */
    result = ngiCommunicationReceive(comm, wBuf, nBytes, 0, &receivedNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't receive the data.\n", fName);
	return 0;
    }

    /* Wrote the date */
    result = ngiStreamManagerWriteBuffer(sMng, receivedNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write to the Stream Buffer.\n", fName);
	return 0;
    }

    *receiveNbytes = remainNbytes + receivedNbytes;

    /* Success */
    return 1;
}

/**
 * Stream Manager: Receive
 * Receive the data to full the current Stream Buffer, or nBytes receive.
 */
int
ngiStreamManagerReceiveFull(
    ngiStreamManager_t *sMng,
    ngiCommunication_t *comm,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    return nglStreamManagerReceive(sMng, comm, nBytes, 1, log, error);
}

/**
 * Stream Manager: Receive
 * Receive the data nBytes receive.
 */
int
ngiStreamManagerReceive(
    ngiStreamManager_t *sMng,
    ngiCommunication_t *comm,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    return nglStreamManagerReceive(sMng, comm, nBytes, 0, log, error);
}

static int
nglStreamManagerReceive(
    ngiStreamManager_t *sMng,
    ngiCommunication_t *comm,
    size_t nBytes,
    int isFull,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t maxNbytes, waitNbytes, receiveNbytes, receiveTotalNbytes;
    void *wBuf;
    int isTooLarge;
    static const char fName[] = "nglStreamManagerReceive";

    /* Check the argument */
    assert(sMng != NULL);
    assert(comm != NULL);

    result = ngiStreamManagerGetTotalBytesOfReadableData(sMng, &isTooLarge, &receiveTotalNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the total bytes of remainig data.\n", fName);
	return 0;
    }

    for (; receiveTotalNbytes < nBytes; receiveTotalNbytes += receiveNbytes) {
        /* Get the Writable Buffer */
        result = ngiStreamManagerGetWritableBuffer(sMng, &wBuf, 1, &maxNbytes,log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
                NULL, "%s: Can't get writable buffer.\n", fName);
            return 0;
        }

	/* Calculate empty space */
	if (receiveTotalNbytes + maxNbytes < nBytes) {
	    /* Require >= Can receive */
	    waitNbytes = maxNbytes;
	} else {
	    /* Require < Can receive */
	    assert(receiveTotalNbytes < nBytes);
	    waitNbytes = nBytes - receiveTotalNbytes;
	}

        if (isFull == 0) {
            maxNbytes = waitNbytes;
        }
	
	/* Receive */
	result = ngiCommunicationReceive(
	    comm, wBuf,
	    maxNbytes, waitNbytes, &receiveNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
	    	"%s: Can't receive the data.\n", fName);
	    return 0;
	}

	/* Wrote the date */
	result = ngiStreamManagerWriteBuffer(sMng, receiveNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't write to the Stream Buffer.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Memory Stream Manager: Destroy the data for write.
 */
static int
nglMemoryStreamManagerDestroyWriteData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    ngiMemoryStreamManager_t *msMng;
    /* Check the argument */
    assert(sMng != NULL);
    
    msMng = (ngiMemoryStreamManager_t*)sMng;

    /* Destroy the data */
    return nglMemoryStreamManagerDestroyData(msMng, log, error);
}

/**
 * Memory Stream Manager: Destroy the data for read.
 */
static int
nglMemoryStreamManagerDestroyReadData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    ngiMemoryStreamManager_t *msMng;
    /* Check the argument */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);
    
    msMng = (ngiMemoryStreamManager_t*)sMng;

    /* Is no date remained? */
    if (msMng->ngmsm_readNbytes < msMng->ngmsm_writeNbytes) {
    	/* Do nothing. Return success */
    	return 1;
    }

    /* Destroy the data */
    return nglMemoryStreamManagerDestroyData(msMng, log, error);
}

/**
 * Memory Stream Manager: Destroy the data.
 */
static int
nglMemoryStreamManagerDestroyData(
    ngiMemoryStreamManager_t *msMng,
    ngLog_t *log,
    int *error)
{
    ngiStreamBuffer_t *stream;

    /* Check the argument */
    assert(msMng != NULL);

    /* Destroy the data */
    msMng->ngmsm_writeNbytes = 0;
    msMng->ngmsm_readNbytes = 0;
    msMng->ngmsm_write = msMng->ngmsm_head;
    msMng->ngmsm_read = msMng->ngmsm_head;
    for (stream = msMng->ngmsm_head; stream != NULL; stream = stream->ngsb_next) {
    	stream->ngsb_writeNbytes = 0;
	stream->ngsb_readNbytes = 0;
    }

    /* Success */
    return 1;
}

/**
 * Stream Buffer: Construct
 */
ngiStreamBuffer_t *
ngiStreamBufferConstructBuffer(
    size_t growNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStreamBuffer_t *stream;
    static const char fName[] = "ngiStreamBufferConstructBuffer";

    /* Allocate */
    stream = ngiStreamBufferAllocate(growNbytes, log, error);
    if (stream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Buffer.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiStreamBufferInitialize(
	stream, growNbytes, growNbytes, 0,
	&stream->ngsb_buffer[0], NGI_DATA_THROUGH_BUFFER, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Buffer.\n", fName);
	goto error;
    }

    /* Success */
    return stream;

    /* Error occurred */
error:
    result = ngiStreamBufferFree(stream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Buffer.\n", fName);
    }

    return NULL;
}

/**
 * Stream Buffer: Construct Pointer
 */
ngiStreamBuffer_t *
ngiStreamBufferConstructPointer(
    void *data,
    size_t dataNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStreamBuffer_t *stream;
    static const char fName[] = "ngiStreamBufferConstructPointer";

    /* Allocate */
    stream = ngiStreamBufferAllocate(0, log, error);
    if (stream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Buffer.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiStreamBufferInitialize(
	stream, dataNbytes, 0, dataNbytes, data, NGI_DATA_DIRECTLY,
	log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Buffer.\n", fName);
	goto error;
    }

    /* Success */
    return stream;

    /* Error occurred */
error:
    result = ngiStreamBufferFree(stream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Buffer.\n", fName);
	return NULL;
    }

    return NULL;
}

#if 0 /* Is this necessary? */
/**
 * Grow the Stream Buffer.
 */
ngiStreamBuffer_t *
ngiStreamBufferGrow(ngiStreamBuffer_t *stream, ngLog_t *log, int *error)
{
    ngiStreamBuffer_t *new;
    static const char fName[] = "ngiStreamBufferGrow";

    /* Is Stream Buffer valid? */
    result = ngiStreamBufferIsValid(stream, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Stream Buffer is not valid.\n", fName);
	return NULL;
    }

    /* Is grow less equal zero? */
    if (stream->ngs_graw <= 0) {
	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW)
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Stream Buffer is overflow.\n", fName);
	return NULL;
    }

    /* Construct new Stream Buffer */
    new = ngiStreamBufferConstruct(
	stream->ngs_growNbytes, stream->ngs_growNbytes, log, error);
    if (new == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't construct the Stream Buffer.\n", fName);
	return NULL;
    }

    /* Success */
    return new;
}
#endif /* 0 */

/**
 * Destruct
 */
int
ngiStreamBufferDestruct(
    ngiStreamBuffer_t *stream,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiStreamBufferDestruct";

    /* Check the arguments */
    assert(stream != NULL);

    /* Finalize */
    result = ngiStreamBufferFinalize(stream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Stream Buffer.\n", fName);
	return 0;
    }

    result = ngiStreamBufferFree(stream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't deallocate the storage for Stream Buffer.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Stream Buffer: Allocate
 */
ngiStreamBuffer_t *
ngiStreamBufferAllocate(size_t nBytes, ngLog_t *log, int *error)
{
    ngiStreamBuffer_t *stream;
    static const char fName[] = "ngiStreamBufferAllocate";

    stream = globus_libc_calloc(1,
	sizeof (ngiStreamBuffer_t) - sizeof (stream->ngsb_buffer) + nBytes);
    if (stream == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for Stream Buffer.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return stream;
}

/**
 * Stream Buffer: Deallocate
 */
int
ngiStreamBufferFree(ngiStreamBuffer_t *stream, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(stream != NULL);

    /* Deallocate */
    globus_libc_free(stream);

    /* Success */
    return 1;
}

/**
 * Stream Buffer: Initialize
 */
int
ngiStreamBufferInitialize(
    ngiStreamBuffer_t *stream,
    size_t bufferNbytes,
    size_t growNbytes,
    size_t dataNbytes,
    void *pointer,
    ngiDataDirect_t direct,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(stream != NULL);

    /* Initialize the members */
    nglStreamBufferInitializeMember(stream);
    stream->ngsb_next = NULL;
    stream->ngsb_direct = direct;
    stream->ngsb_bufferNbytes = bufferNbytes;
    stream->ngsb_growNbytes = growNbytes;
    stream->ngsb_writeNbytes = dataNbytes;
    stream->ngsb_pointer = pointer;

    /* Success */
    return 1;
}

/**
 * Stream Buffer: Finalize
 */
int
ngiStreamBufferFinalize(
    ngiStreamBuffer_t *stream,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(stream != NULL);

    /* Initialize the members */
    nglStreamBufferInitializeMember(stream);

    /* Success */
    return 1;
}

/**
 * Stream Buffer: Initialize the members.
 */
static void
nglStreamBufferInitializeMember(ngiStreamBuffer_t *stream)
{
    /* Check the arguments */
    assert(stream != NULL);

    /* Initialize the pointers */
    nglStreamBufferInitializePointer(stream);

    /* Initialize the members */
    stream->ngsb_bufferNbytes = 0;
    stream->ngsb_growNbytes = 0;
    stream->ngsb_writeNbytes = 0;
    stream->ngsb_readNbytes = 0;
}

/**
 * Stream Buffer: Initialize the pointer.
 */
static void
nglStreamBufferInitializePointer(ngiStreamBuffer_t *stream)
{
    /* Check the arguments */
    assert(stream != NULL);

    /* Initialize the pointers */
    stream->ngsb_next = NULL;
    stream->ngsb_pointer = NULL;
}

/**
 * Stream Buffer: Register
 */
int
ngiStreamBufferRegister(
    ngiMemoryStreamManager_t *msMng,
    ngiStreamBuffer_t *stream,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(msMng != NULL);
    assert(stream != NULL);

    /* Append at last of the list */
    stream->ngsb_next = NULL;
    if (msMng->ngmsm_head == NULL) {
    	/* No Stream Buffer is registered */
	assert(msMng->ngmsm_tail == NULL);
	msMng->ngmsm_bufferNbytes = stream->ngsb_bufferNbytes;
	msMng->ngmsm_writeNbytes = stream->ngsb_writeNbytes;
	msMng->ngmsm_head = stream;
	msMng->ngmsm_tail = stream;
	msMng->ngmsm_write = stream;
	msMng->ngmsm_read = stream;
    } else {
    	/* Any Stream Buffer is already registered */
	assert(msMng->ngmsm_tail != NULL);
	assert(msMng->ngmsm_tail->ngsb_next == NULL);
	msMng->ngmsm_bufferNbytes += stream->ngsb_bufferNbytes;
	msMng->ngmsm_tail->ngsb_next = stream;
	msMng->ngmsm_tail = stream;
    }

    /* Success */
    return 1;
}

/**
 * Stream Buffer: Unregister
 */
int
ngiStreamBufferUnregister(
    ngiMemoryStreamManager_t *msMng,
    ngiStreamBuffer_t *stream,
    ngLog_t *log,
    int *error)
{
    ngiStreamBuffer_t **prev, *curr;
    static const char fName[] = "ngiStreamBufferUnregister";

    /* Delete the Stream Buffer from the list */
    prev = &msMng->ngmsm_head;
    curr = msMng->ngmsm_head;
    for (; curr != NULL; curr = curr->ngsb_next) {
	if (curr == stream) {
	    /* Unlink the Stream Buffer */
	    *prev = stream->ngsb_next;
	    stream->ngsb_next = NULL;
	    msMng->ngmsm_bufferNbytes -= stream->ngsb_bufferNbytes;

	    /* Success */
	    return 1;
	}

	prev = &curr->ngsb_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
    	"%s: Stream Buffer is not found.\n", fName);

    /* Failed */
    return 0;
}

/**
 * Stream Manager: Write to the Stream Buffer.
 */
int
ngiStreamManagerWrite(
    ngiStreamManager_t *sMng,
    char *buf,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t writeNbytes, canWriteNbytes;
    void *wBuf;
    static const char fName[] = "ngiStreamManagerWrite";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(buf != NULL);
    assert(nBytes > 0);

    for (writeNbytes = 0; writeNbytes < nBytes;
	 writeNbytes += canWriteNbytes) {
	/* Get the Stream Buffer */
        result = ngiStreamManagerGetWritableBuffer(sMng, &wBuf, 1, &canWriteNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
                NULL, "%s: Can't get writable buffer from the Stream Manager.\n", fName);
            return 0;
        }

	/* Is there any sufficient space? */
	if (writeNbytes + canWriteNbytes > nBytes) {
            assert(writeNbytes < nBytes);
	    canWriteNbytes = nBytes - writeNbytes;
	}

	/* Copy the data */
	memcpy(wBuf, &buf[writeNbytes], canWriteNbytes);

	/* Increment */
	result = ngiStreamManagerWriteBuffer(sMng, canWriteNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't write the Stream Buffer.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Stream Manager: Write to the Stream Buffer.
 */
int
nglMemoryStreamManagerWriteDirectly(
    ngiStreamManager_t *sMng,
    void *buf,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiMemoryStreamManager_t *msMng;
    ngiStreamBuffer_t *stream;
    static const char fName[] = "nglMemoryStreamManagerWriteDirectly";

    /* Check the argument */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglMemoryStreamManagerTypeInfomation);
    assert(buf != NULL);
    assert(nBytes > 0);

    msMng = (ngiMemoryStreamManager_t *)sMng;

    /* Construct new Stream Buffer */
    stream = ngiStreamBufferConstructPointer(buf, nBytes, log, error);
    if (stream == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't construct the Stream Buffer.\n", fName);
	return 0;
    }

    /* Register */
    result = ngiStreamBufferRegister(msMng, stream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't register the Stream Buffer.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

#if 0 /* Is this necessary? */
/**
 * Write to the Stream Buffer.
 */
int
ngiStreamBufferWrite(
    ngiStreamBuffer_t *stream,
    void *buf,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t writeNbytes, emptyNbytes;
    static const char fName[] = "ngiStreamBufferWrite";

    /* Check the arguments */
    assert(stream != NULL);
    assert(buf != NULL);
    assert(nBytes > 0);

    for (writeNbytes = 0; writeNbytes < nBytes; writeNbytes += emptyNbytes) {
	/* Calculate empty space. */
        assert(stream->ngsb_bufferNbytes >= stream->ngsb_dataNbytes);
	emptyNbytes = stream->ngsb_bufferNbytes - stream->ngsb_dataNbytes;

	/* Is there any sufficient space? */
	if (nBytes <= emptyNbytes) {
	    emptyNbytes = nBytes;
	}

	/* Copy the data */
	memcpy(&stream->ngsb_Buffer[stream->ngsb_dataNbytes],
	    &buff[writeNbytes], emptyNbytes);

	/* Is there any sufficient space? */
	if (nBytes > emptyNbytes) {
	    /* There is no sufficient space */
	    /* Can grow this Stream Buffer? */
	    if (stream->ngsb_growNbytes <= 0) {
		/* No, can't grow */
		NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		    NULL, "%s: Can't grow the Stream Buffer.\n", fName);
		return 0;
	    }
	}
    }

    /* Success */
    return 1;
}
#endif /* 0 */

/**
 * Stream Manager: Read from the Stream Buffer.
 */
int
ngiStreamManagerRead(
    ngiStreamManager_t *sMng,
    char *buf,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    size_t readNbytes, remainNbytes;
    int result;
    void *rBuf;
    static const char fName[] = "ngiStreamManagerRead";

    /* Check the argument */
    assert(sMng != NULL);
    assert(buf != NULL);
    assert(nBytes > 0);

    for (readNbytes = 0; readNbytes < nBytes; readNbytes += remainNbytes) {
	/* Get the Stream Buffer */
        result = ngiStreamManagerGetReadableBuffer(sMng, &rBuf, 1, &remainNbytes, log, error);
        if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't get the readable Stream Manager.\n", fName);
            return 0;
        }

	/* Is there any sufficient data? */
	if (readNbytes + remainNbytes > nBytes) {
	    assert(nBytes >= readNbytes);
	    remainNbytes = (nBytes - readNbytes);
	}

	/* Copy the data */
	memcpy(&buf[readNbytes], rBuf, remainNbytes);

	/* Increment */
	result = ngiStreamManagerReadBuffer(sMng, remainNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't read the Stream Buffer.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

int
ngiStreamManagerCopy(
    ngiStreamManager_t *sMngSrc,
    ngiStreamManager_t *sMngDest,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    size_t readNbytes, remainNbytes;
    int result;
    void *rBuf;
    static const char fName[] = "ngiStreamManagerCopy";

    /* Check the argument */
    assert(sMngSrc != NULL);
    assert(sMngDest != NULL);

    for (readNbytes = 0; readNbytes < nBytes; readNbytes += remainNbytes) {
	/* Get the Stream Buffer */
        result = ngiStreamManagerGetReadableBuffer(sMngSrc, &rBuf, 1, &remainNbytes, log, error);
        if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't get the readable Stream Manager.\n", fName);
            return 0;
        }

	/* Is there any sufficient data? */
	if (readNbytes + remainNbytes > nBytes) {
	    assert(nBytes >= readNbytes);
	    remainNbytes = (nBytes - readNbytes);
	}

        result = ngiStreamManagerWrite(sMngDest, rBuf, remainNbytes, log, error);
        if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't write the Stream Manager.\n", fName);
	    return 0;
        }

	/* Increment */
	result = ngiStreamManagerReadBuffer(sMngSrc, remainNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't read the Buffer.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}


#if 0 /* Is this necessary? */
/**
 * Read from the Stream Buffer.
 */
int
ngiStreamBufferRead(
    ngiStreamBuffer_t *stream,
    ngiStreamBuffer_t **newStream,
    void *buf,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t readNbytes, remainNbytes, readTotalNbytes;
    static const char fName[] = "ngiStreamBufferRead";

    /* Check the arguments */
    assert(stream != NULL);
    assert(buf != NULL);
    assert(nBytes > 0);

    /* Initialize the arguments */
    *newStream = NULL;
    readTotalNbytes = 0;

    for (; readTotalNbytes < nBytes; readTotalNbytes += readNbytes) {

	if (stream->ngsb_writeNbytes < stream->ngsb_readNbytes) {
	    NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Read bytes %d greater than written bytes %d.\n",
		fName, stream->ngsb_readNbytes, stream->ngsb_writeNbytes);
	    return 0;
	}
    	readNbytes = stream->ngsb_writeNbytes - stream->ngsb_readNbytes;

	/* Is data exist in this buffer? */
	if (readNbytes == 0) {
	    /* Is next Stream Buffer exist? */
	    if (stream->ngsb_next == NULL) {
	    	/* No next Stream Buffer */
		NGI_SET_ERROR(error, NGI_ERROR_UNDERFLOW);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		    NULL, "%s: Underflow the Stream Buffer.\n", fName);
		return 0;
	    }

	    /* Get the next Stream Buffer */
	    stream = stream->ngsb_next;
	    continue;	/* Goto top of the loop */
	}

	/* Calculate the number of bytes to read */
	if (readNbytes + readTotalNbytes > nBytes) {
	    assert(readTotalNbytes < nBytes);
	    readNbytes = nBytes - readTotalNbytes;
	}

	/* Copy the data */
	memcpy(buf, &stream->ngsb_pointer[stream->ngsb_readNbytes], readNbytes);
    }

    /* Success */
    *newStream = stream;
    return 1;
}
#endif /* 0 */

#if 0 /* Is this necessary? */
/**
 * Send
 */
int
ngiStreamBufferSend(
    ngiStreamBuffer_t *stream,
    ngiCommunication_t *comm,
    ngLog_t *log,
    int *error)
{
    int nStreams;
    ngiStreamBuffer_t *tmpStream;
    struct iovec *iov;

    /* Check the argument */
    assert(stream != NULL);
    assert(comm != NULL);

    /* How many Streams? */
    for (nStreams = 0, tmpStream = stream; tmpStream != NULL; nStreams++)
!!!! 2003/8/20 !!!!
#endif /* 0 */

#if 0 /* Is this necessary? */
/**
 * Get buffer from Stream Buffer.
 */
int
ngiStreamBufferGetBuffer(
    ngiStreamBuffer_t *stream,
    ngiStreamBuffer_t **newStream,
    u_char **buf,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t remainNbytes;
    ngiStreamBuffer *prev, *new;
    static const char fName[] = "ngiStreamBufferGetBuffer";

    /* Is Stream Buffer valid? */
    result = ngiStreamBufferIsValid(stream, log, error);
    if (result == 0) {
	ngLogPrintf(ngLog, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Stream Buffer is not valid.\n", fName);
	return -1;
    }

    /* Is buf NULL? */
    if (buf == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(ngLog, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Buffer is NULL.\n", fName);
	return -1;
    }

    for (; stream != NULL; stream = stream->ngsb_next) {

	/* Calculate the remaining numbers of bytes */
	if (stream->ngsb_bufferNbytes > stream->ngsb_dataNbytes) {
	    remainNbytes = stream->ngsb_bufferNbytes - stream->ngsb_dataNbytes;
	} else {
	    remainNbytes = 0;
	}
	if (remainNbytes > 0) {
	    /* Calculate the pointer of buffer */
	    buff = &stream->ngsb_pointer[stream->ngsb_dataNbytes];

	    /* Success */
	    *newStream = stream;
	    return remainNbytes;
	}

	/* Save the current Stream Buffer */
	prev = stream;
    }

    /* No space in buffer */
    new = ngiStreamBufferGrow(prev, log, error);
    if (new == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't grow the Stream Buffer.\n", fName);
	return -1;
    }

    /* Success */
    *newStream = new;
    return new->ngsb_bufferNbytes;
}
#endif /* 0 */

#if 0 /* Is this necessary? */
/**
 * Is Stream Buffer valid?
 */
int
ngiStreamBufferIsValid(
    ngiStreamBuffer_t *stream,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiStreamBufferIsValid";

    /* Is stream NULL? */
    if (stream == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(ngLog, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Stream Buffer is NULL.\n", fName);
	return 0;
    }

    /* Is number of bytes of buffer smaller equal zero? */
    if (stream->ngsb_bufferNbytes <= 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(ngLog, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The number of bytes of buffer is smaller equal zero.\n",
	    fName);
	return 0;
    }

    /* Is number of bytes of buffer valid? */
    if (stream->ngsb_bufferNbytes < stream->ngsb_dataNbytes) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(ngLog, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The buffer is smaller than nBytes written.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}
#endif /* 0 */

int
ngiStreamManagerInitialize(ngiStreamManager_t *sMng, ngiStreamManagerTypeInformation_t *typeInfo, ngLog_t *log, int *error)
{
    assert(sMng != NULL);
    assert(typeInfo != NULL);

    nglStreamManagerInitializeMember(sMng);

    sMng->ngsm_typeInfomation = typeInfo;
    sMng->ngsm_next = NULL;

    return 1;
}

static void nglStreamManagerInitializeMember(ngiStreamManager_t *sMng)
{
    assert(sMng != NULL);

    nglStreamManagerInitializePointer(sMng);
}

static void
nglStreamManagerInitializePointer(ngiStreamManager_t *sMng)
{
    assert(sMng != NULL);
    
    sMng->ngsm_typeInfomation = NULL;
    sMng->ngsm_next = NULL;
}

int
ngiStreamManagerFinalize(ngiStreamManager_t *sMng, ngLog_t *log, int *error)
{
    assert(sMng);

    nglStreamManagerInitializeMember(sMng);

    return 1;
}

/**
 * File Stream Manager
 */
static void nglFileStreamManagerInitializeMember(ngiFileStreamManager_t *);
static void nglFileStreamManagerInitializePointer(ngiFileStreamManager_t *);

static int nglFileStreamManagerDestroyWriteData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglFileStreamManagerDestroyReadData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglFileStreamManagerGetReadableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglFileStreamManagerReadBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglFileStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglFileStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

ngiStreamManagerTypeInformation_t nglFileStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglFileStreamManagerDestroyWriteData,
    nglFileStreamManagerDestroyReadData,

    /* Buffer */
    nglNonImplementationStreamManagerGetWritableBuffer,
    nglNonImplementationStreamManagerWriteBuffer,
    nglFileStreamManagerGetReadableBuffer,
    nglFileStreamManagerReadBuffer,

    nglNonImplementationStreamManagerWriteDirectly,

    /* Get Size*/
    nglFileStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglFileStreamManagerDestruct
};

/**
 * File Stream Manager: Construct
 */
ngiStreamManager_t *
ngiFileStreamManagerConstruct(
    char *fileName,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiFileStreamManager_t *sMng;
    static const char fName[] = "ngiFileStreamManagerConstruct";

    /* Allocate */
    sMng = ngiFileStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for File Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiFileStreamManagerInitialize(sMng, fileName, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert(&sMng->ngfsm_base == (ngiStreamManager_t *)sMng);
    return (ngiStreamManager_t *)sMng;

    /* Error occurred */
error:
    result = ngiFileStreamManagerFree(sMng, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
    }

    return 0;
}

/**
 * File Stream Manager: Destruct
 */
static int
nglFileStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiFileStreamManager_t *fsMng;
    static const char fName[] = "nglFileStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglFileStreamManagerTypeInfomation);

    fsMng = (ngiFileStreamManager_t *)sMng;

    /* Finalize */
    result = ngiFileStreamManagerFinalize(fsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiFileStreamManagerFree(fsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * File Stream Manager: Allocate
 */
ngiFileStreamManager_t *
ngiFileStreamManagerAllocate(
    ngLog_t *log,
    int *error)
{
    ngiFileStreamManager_t *sMng;
    static const char fName[] = "ngiFileStreamManagerAllocate";

    /* Allocate */
    sMng = globus_libc_calloc(1, sizeof (ngiFileStreamManager_t));
    if (sMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for File Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return sMng;
}

/**
 * File Stream Manager: Free
 */
int
ngiFileStreamManagerFree(
    ngiFileStreamManager_t *fsMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(fsMng != NULL);

    /* Free */
    globus_libc_free(fsMng);

    /* Success */
    return 1;
}

/**
 * File Stream Manager: Initialize
 */
int
ngiFileStreamManagerInitialize(
    ngiFileStreamManager_t *fsMng,
    char *fileName,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    struct stat buf;
    int result;
    static const char fName[] = "ngiFileStreamManagerInitialize";

    /* Check the arguments */
    assert(fsMng != NULL);

    /* Initialize the base and the members */
    result = ngiStreamManagerInitialize((ngiStreamManager_t *)fsMng,
        &nglFileStreamManagerTypeInfomation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Stream Manager.\n",
             fName);
        return 0;
    }
    nglFileStreamManagerInitializeMember(fsMng);

    /* Duplicate fileName */
    fsMng->ngfsm_fileName = strdup(fileName);
    if (fsMng->ngfsm_fileName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the file name.\n",
             fName);
        goto error;
    }

    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Open the file named \"%s\".\n", fName, fsMng->ngfsm_fileName);

    /* Open file */
    gResult = globus_io_file_open(
        fsMng->ngfsm_fileName, GLOBUS_IO_FILE_RDONLY,
        0, NULL, &fsMng->ngfsm_handle);
    if (gResult != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_io_file_open() failed.\n", fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
        goto error;
    }
    fsMng->ngfsm_handleInitialized = 1;

    /* Get file size */
    result = stat(fsMng->ngfsm_fileName, &buf);
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: stat failed: %s.\n", fName, strerror(errno));
        goto error;
    }
    fsMng->ngfsm_size = buf.st_size;

    /* Is regular file ?*/
    if (S_ISREG(buf.st_mode) == 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: \"%s\" isn't regular file.\n", fName);
        goto error;
    }

    /* Construct the Stream Buffer */
    fsMng->ngfsm_buffer = ngiStreamBufferConstructBuffer(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (fsMng->ngfsm_buffer == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't construct the Stream Buffer.\n", fName);
        goto error;
    }

    /* Success */
    return 1;
    /* Error occurred */
error:

    result = ngiFileStreamManagerFinalize(fsMng, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the File Stream Manager.\n", fName);
    }
    return 0;
}

/**
 * File Stream Manager: Finalize
 */
int
ngiFileStreamManagerFinalize(
    ngiFileStreamManager_t *fsMng,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    int result;
    int ret = 1;
    static const char fName[] = "ngiFileStreamManagerFinalize";

    assert(fsMng != NULL);

    /* Destruct the Stream Buffer */
    if (fsMng->ngfsm_buffer != NULL) {
        result = ngiStreamBufferDestruct(fsMng->ngfsm_buffer, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Stream Buffer.\n", fName);
            ret = 0;
            error = NULL;
        }
        fsMng->ngfsm_buffer = NULL;
    }
    
    /* Close file */
    if (fsMng->ngfsm_handleInitialized != 0) {
        fsMng->ngfsm_handleInitialized = 0;
        gResult = globus_io_close(&fsMng->ngfsm_handle);
        if (gResult != GLOBUS_SUCCESS) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
            ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_file_close() failed.\n", fName);
            ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, fName, gResult, NULL);
            ret = 0;
            error = NULL;
        }
    }

    /* Free filename */
    if (fsMng->ngfsm_fileName != NULL) {
        free(fsMng->ngfsm_fileName);
        fsMng->ngfsm_fileName = NULL;
    }

    nglFileStreamManagerInitializeMember(fsMng);

    /* Finalize base */
    result = ngiStreamManagerFinalize((ngiStreamManager_t *)fsMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the File Stream Manager.\n", fName);
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * File Stream Manager: Initialize member
 */
static void
nglFileStreamManagerInitializeMember(
    ngiFileStreamManager_t *fsMng)
{
    assert(fsMng != 0);

    nglFileStreamManagerInitializePointer(fsMng);

    fsMng->ngfsm_handleInitialized = 0; 
    fsMng->ngfsm_size = 0;
    fsMng->ngfsm_readNbytes = 0;
}

/**
 * File Stream Manager: Initialize pointer
 */
static void
nglFileStreamManagerInitializePointer(ngiFileStreamManager_t *fsMng)
{
    assert(fsMng != 0);

    fsMng->ngfsm_fileName = NULL;
    fsMng->ngfsm_buffer = NULL;
}

/**
 * File Stream Manager: Destroy write data
 */
static int
nglFileStreamManagerDestroyWriteData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    /* Do nothing */
    return 1;
}

/**
 * File Stream Manager: Destroy read data
 */
static int
nglFileStreamManagerDestroyReadData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    /* Do nothing */
    return 1;
}

/**
 * File Stream Manager: get readable buffer.
 */
static int
nglFileStreamManagerGetReadableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiFileStreamManager_t *fsMng;
    ngiStreamBuffer_t *stream;
    globus_result_t gResult;
    globus_size_t gReadNbytes;
    int result;
    size_t readableNbytes;
    off_t fileReadableNbytes;
    static const char fName[] = "nglFileStreamManagerGetReadableBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglFileStreamManagerTypeInfomation);
    assert(buf != NULL);
    assert(nBytes != NULL);
        
    fsMng = (ngiFileStreamManager_t *)sMng;
    stream = fsMng->ngfsm_buffer;
    assert(stream != NULL);

    /* Check number of bytes required */
    if (stream->ngsb_bufferNbytes < nBytesRequired) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Number of bytes required is too large.\n", fName);
	return  0;
    }

    /* Get readable bytes */
    assert(stream->ngsb_writeNbytes >= stream->ngsb_readNbytes);
    readableNbytes = stream->ngsb_writeNbytes - stream->ngsb_readNbytes;

    if ((readableNbytes == 0) &&
        (fsMng->ngfsm_handleInitialized != 0)) {
        /* Read from file */

        /* Stop reading data over file size */
        assert(fsMng->ngfsm_size >= fsMng->ngfsm_readNbytes);
        fileReadableNbytes = fsMng->ngfsm_size - fsMng->ngfsm_readNbytes;
        if (fileReadableNbytes > stream->ngsb_bufferNbytes) {
            fileReadableNbytes = stream->ngsb_bufferNbytes;
        }

        gResult = globus_io_read(&fsMng->ngfsm_handle, stream->ngsb_pointer,
            fileReadableNbytes, fileReadableNbytes, &gReadNbytes);
        if (gResult != GLOBUS_SUCCESS) {
            result = ngiGlobusIsIoEOF(gResult, log, error);
            if (result == 0) {
                /* Not EOF */
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: globus_io_read failed.\n", fName);

                return 0;
            }
            /* if EOF */
            if (fsMng->ngfsm_size + gReadNbytes < fsMng->ngfsm_readNbytes) {
                NGI_SET_ERROR(error, NG_ERROR_FILE);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't read all data.\n", fName);
                return 0;
            }
        }
        readableNbytes = stream->ngsb_writeNbytes = gReadNbytes;
        stream->ngsb_readNbytes = 0;
    }

    if (readableNbytes < nBytesRequired) {
	NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Underflow the Stream Buffer.\n", fName);
	return  0;
    }

    *buf    = &stream->ngsb_pointer[stream->ngsb_readNbytes];
    *nBytes = readableNbytes;

    return 1;
}

static int
nglFileStreamManagerReadBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiStreamBuffer_t *stream;
    ngiFileStreamManager_t *fsMng;
    static const char fName[] = "nglFileStreamManagerReadBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglFileStreamManagerTypeInfomation);

    fsMng = (ngiFileStreamManager_t *)sMng;
    /* Get the Stream Buffer */
    stream = fsMng->ngfsm_buffer;
    assert(stream != NULL);

    /* Is nBytes valid? */
    if (nBytes + stream->ngsb_readNbytes > stream->ngsb_writeNbytes) {
	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Overflow the Stream Manager.\n", fName);
	return 0;
    }

    /* Increment */
    stream->ngsb_readNbytes += nBytes;
    fsMng->ngfsm_readNbytes += nBytes;

    /* Success */
    return 1;
}

static int
nglFileStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiFileStreamManager_t *fsMng;
    off_t tmp;
#if 0
    static const char fName[] = "nglFileStreamManagerGetBytesOfReadableData";
#endif

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglFileStreamManagerTypeInfomation);
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);

    fsMng = (ngiFileStreamManager_t *)sMng;

    assert(fsMng->ngfsm_size >= fsMng->ngfsm_readNbytes);
    tmp = fsMng->ngfsm_size - fsMng->ngfsm_readNbytes;
    if (tmp > NGI_PROTOCOL_SIZE_MAX) {
        *isTooLarge = 1;
    } else {
        *isTooLarge = 0;
    }
    *nBytes = tmp % (NGI_PROTOCOL_SIZE_MAX + 1);

    return 1;
}

/**
 * Delimiter Stream Manager.
 */
static void nglDelimiterStreamManagerInitializeMember(ngiDelimiterStreamManager_t *);
static void nglDelimiterStreamManagerInitializePointer(ngiDelimiterStreamManager_t *);

static int nglDelimiterStreamManagerDestroyWriteData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglDelimiterStreamManagerDestroyReadData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglDelimiterStreamManagerGetReadableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglDelimiterStreamManagerReadBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglDelimiterStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglDelimiterStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

ngiStreamManagerTypeInformation_t nglDelimiterStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglDelimiterStreamManagerDestroyWriteData,
    nglDelimiterStreamManagerDestroyReadData,

    /* Buffer */
    nglNonImplementationStreamManagerGetWritableBuffer,
    nglNonImplementationStreamManagerWriteBuffer,
    nglDelimiterStreamManagerGetReadableBuffer,
    nglDelimiterStreamManagerReadBuffer,

    nglNonImplementationStreamManagerWriteDirectly,

    /* Get Size*/
    nglDelimiterStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglDelimiterStreamManagerDestruct
};

/**
 * Delimiter Stream Manager: Construct
 */
ngiStreamManager_t *
ngiDelimiterStreamManagerConstruct(
    ngLog_t *log,
    int *error)
{
    int result;
    ngiDelimiterStreamManager_t *sMng;
    static const char fName[] = "ngiDelimiterStreamManagerConstruct";

    /* Allocate */
    sMng = ngiDelimiterStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Delimiter Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiDelimiterStreamManagerInitialize(sMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Delimiter Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert(&sMng->ngdsm_base == (ngiStreamManager_t *)sMng);
    return (ngiStreamManager_t *)sMng;

    /* Error occurred */
error:
    result = ngiDelimiterStreamManagerFree(sMng, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Delimiter Stream Manager.\n", fName);
    }

    return 0;
}

/**
 * Delimiter Stream Manager: Destruct
 */
static int
nglDelimiterStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiDelimiterStreamManager_t *psMng;
    static const char fName[] = "nglDelimiterStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglDelimiterStreamManagerTypeInfomation);

    psMng = (ngiDelimiterStreamManager_t *)sMng;

    /* Finalize */
    result = ngiDelimiterStreamManagerFinalize(psMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiDelimiterStreamManagerFree(psMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Delimiter Stream Manager: Allocate
 */
ngiDelimiterStreamManager_t *
ngiDelimiterStreamManagerAllocate(
    ngLog_t *log,
    int *error)
{
    ngiDelimiterStreamManager_t *dsMng;
    static const char fName[] = "ngiDelimiterStreamManagerAllocate";

    /* Allocate */
    dsMng = globus_libc_calloc(1, sizeof (ngiDelimiterStreamManager_t));
    if (dsMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Delimiter Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return dsMng;
}

/**
 * Delimiter Stream Manager: Free
 */
int
ngiDelimiterStreamManagerFree(
    ngiDelimiterStreamManager_t *dsMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(dsMng != NULL);

    /* Free */
    globus_libc_free(dsMng);

    /* Success */
    return 1;
}

/**
 * Delimiter Stream Manager: Initialize
 */
int
ngiDelimiterStreamManagerInitialize(
    ngiDelimiterStreamManager_t *dsMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiDelimiterStreamManagerInitialize";

    /* Check the arguments */
    assert(dsMng != NULL);

    /* Initialize the base and the members */
    result = ngiStreamManagerInitialize((ngiStreamManager_t *)dsMng,
        &nglDelimiterStreamManagerTypeInfomation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Stream Manager.\n",
             fName);
        return 0;
    }
    nglDelimiterStreamManagerInitializeMember(dsMng);

    /* Success */
    return 1;
}

/**
 * Delimiter Stream Manager: Finalize
 */
int
ngiDelimiterStreamManagerFinalize(
    ngiDelimiterStreamManager_t *dsMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiDelimiterStreamManagerFinalize";

    /* Check the arguments */
    assert(dsMng != NULL);

    nglDelimiterStreamManagerInitializeMember(dsMng);

    /* Finalize base */
    result = ngiStreamManagerFinalize((ngiStreamManager_t *)dsMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Delimiter Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Delimiter Stream Manager: Initialize member
 */
static void
nglDelimiterStreamManagerInitializeMember(
    ngiDelimiterStreamManager_t *dsMng)
{
    /* Check the arguments */
    assert(dsMng != 0);

    nglDelimiterStreamManagerInitializePointer(dsMng);

    dsMng->ngdsm_dummy = 0;
    dsMng->ngdsm_readNbytes = 0;
}

/**
 * Delimiter Stream Manager: Initialize pointer
 */
static void
nglDelimiterStreamManagerInitializePointer(ngiDelimiterStreamManager_t *dsMng)
{
    assert(dsMng != NULL);
    /* Do Nothing */
}

/**
 * Delimiter Stream Manager: Destroy write data
 */
static int
nglDelimiterStreamManagerDestroyWriteData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    /* Do nothing */
    return 1;
}

/**
 * Delimiter Stream Manager: Destroy read data
 */
static int
nglDelimiterStreamManagerDestroyReadData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    /* Do nothing */
    return 1;
}

/**
 * Delimiter Stream Manager: get readable buffer.
 */
static int
nglDelimiterStreamManagerGetReadableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiDelimiterStreamManager_t *dsMng;
    size_t readableNbytes;
    unsigned char *pointer;
    static const char fName[] = "nglDelimiterStreamManagerGetReadableBuffer";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglDelimiterStreamManagerTypeInfomation);
    assert(buf != NULL);
    assert(nBytes != NULL);
        
    dsMng = (ngiDelimiterStreamManager_t *)sMng;

    assert(sizeof(dsMng->ngdsm_dummy) >= dsMng->ngdsm_readNbytes);
    readableNbytes = sizeof(dsMng->ngdsm_dummy) - dsMng->ngdsm_readNbytes;

    /* Check number of bytes required */
    if (nBytesRequired >  readableNbytes) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Number of bytes required is too large.\n", fName);
	return  0;
    }

    pointer = &dsMng->ngdsm_dummy;
    *buf    = &(pointer)[dsMng->ngdsm_readNbytes];
    *nBytes = readableNbytes;

    return 1;
}

static int
nglDelimiterStreamManagerReadBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiDelimiterStreamManager_t *dsMng;
    size_t readableNbytes;
    static const char fName[] = "nglDelimiterStreamManagerReadBuffer";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglDelimiterStreamManagerTypeInfomation);

    dsMng = (ngiDelimiterStreamManager_t *)sMng;

    assert(sizeof(dsMng->ngdsm_dummy) >= dsMng->ngdsm_readNbytes);

    readableNbytes = sizeof(dsMng->ngdsm_dummy) - dsMng->ngdsm_readNbytes;
    if (nBytes > sizeof(dsMng->ngdsm_dummy)) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Number of bytes read is too large.\n", fName);
	return  0;
    }
    
    dsMng->ngdsm_readNbytes += readableNbytes;

    /* Success */
    return 1;
}

static int
nglDelimiterStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiDelimiterStreamManager_t *dsMng;

    /* Check the arguments */
    assert(sMng != NULL);
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);

    assert(sMng->ngsm_typeInfomation == &nglDelimiterStreamManagerTypeInfomation);

    dsMng = (ngiDelimiterStreamManager_t *)sMng;

    assert(sizeof(dsMng->ngdsm_dummy) >= dsMng->ngdsm_readNbytes);

    *nBytes = sizeof(dsMng->ngdsm_dummy) - dsMng->ngdsm_readNbytes;
    isTooLarge = 0;

    return 1;
}

/**
 * Partial Stream Manager
 */
static void nglPartialStreamManagerInitializeMember(ngiPartialStreamManager_t *);
static void nglPartialStreamManagerInitializePointer(ngiPartialStreamManager_t *);

static int nglPartialStreamManagerDestroyWriteData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglPartialStreamManagerDestroyReadData(ngiStreamManager_t *, ngLog_t *, int *);
static int nglPartialStreamManagerGetReadableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglPartialStreamManagerReadBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglPartialStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglPartialStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

ngiStreamManagerTypeInformation_t nglPartialStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglPartialStreamManagerDestroyWriteData,
    nglPartialStreamManagerDestroyReadData,

    /* Buffer */
    nglNonImplementationStreamManagerGetWritableBuffer,
    nglNonImplementationStreamManagerWriteBuffer,
    nglPartialStreamManagerGetReadableBuffer,
    nglPartialStreamManagerReadBuffer,

    nglNonImplementationStreamManagerWriteDirectly,

    /* Get Size*/
    nglPartialStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglPartialStreamManagerDestruct
};

/**
 * Partial Stream Manager: Construct
 */
ngiStreamManager_t *
ngiPartialStreamManagerConstruct(
    ngiStreamManager_t *smWhole,
    size_t size,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiPartialStreamManager_t *sMng;
    static const char fName[] = "ngiPartialStreamManagerConstruct";

    /* Allocate */
    sMng = ngiPartialStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Partial Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiPartialStreamManagerInitialize(sMng, smWhole, size, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Partial Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert(&sMng->ngpsm_base == (ngiStreamManager_t *)sMng);
    return (ngiStreamManager_t *)sMng;

    /* Error occurred */
error:
    result = ngiPartialStreamManagerFree(sMng, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Partial Stream Manager.\n", fName);
    }

    return 0;
}

/**
 * Partial Stream Manager: Destruct
 */
static int
nglPartialStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiPartialStreamManager_t *psMng;
    static const char fName[] = "nglPartialStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglPartialStreamManagerTypeInfomation);

    psMng = (ngiPartialStreamManager_t *)sMng;

    /* Finalize */
    result = ngiPartialStreamManagerFinalize(psMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiPartialStreamManagerFree(psMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Partial Stream Manager: Allocate
 */
ngiPartialStreamManager_t *
ngiPartialStreamManagerAllocate(
    ngLog_t *log,
    int *error)
{
    ngiPartialStreamManager_t *psMng;
    static const char fName[] = "ngiPartialStreamManagerAllocate";

    /* Allocate */
    psMng = globus_libc_calloc(1, sizeof (ngiPartialStreamManager_t));
    if (psMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Partial Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return psMng;
}

/**
 * Partial Stream Manager: Free
 */
int
ngiPartialStreamManagerFree(
    ngiPartialStreamManager_t *psMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(psMng != NULL);

    /* Free */
    globus_libc_free(psMng);

    /* Success */
    return 1;
}

/**
 * Partial Stream Manager: Initialize
 */
int
ngiPartialStreamManagerInitialize(
    ngiPartialStreamManager_t *psMng,
    ngiStreamManager_t *smWhole,
    size_t size,
    ngLog_t *log,
    int *error)
{
    int result;
    void *buf;
    size_t readableNbytes;
    static const char fName[] = "ngiPartialStreamManagerInitialize";

    /* Check the arguments */
    assert(psMng != NULL);

    /* Initialize the base and the members */
    result = ngiStreamManagerInitialize((ngiStreamManager_t *)psMng,
        &nglPartialStreamManagerTypeInfomation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Stream Manager.\n",
             fName);
        return 0;
    }
    nglPartialStreamManagerInitializeMember(psMng);

    /* Delimiter */
    if ((smWhole != NULL) &&
        (smWhole->ngsm_typeInfomation
            == &nglDelimiterStreamManagerTypeInfomation)) {
        result = ngiStreamManagerGetReadableBuffer(
            smWhole, &buf, 0, &readableNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get readable buffer from the Stream Manager.\n",
                 fName);
            return 0;
        }
        result = ngiStreamManagerReadBuffer(
            smWhole, readableNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read buffer from the Stream Manager.\n",
                 fName);
            return 0;
        }
    }
    psMng->ngpsm_smWhole = smWhole;
    psMng->ngpsm_maxSize = size;

    /* Success */
    return 1;
}

/**
 * Partial Stream Manager: Finalize
 */
int
ngiPartialStreamManagerFinalize(
    ngiPartialStreamManager_t *psMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiPartialStreamManagerFinalize";

    assert(psMng != NULL);
    nglPartialStreamManagerInitializeMember(psMng);

    /* Finalize base */
    result = ngiStreamManagerFinalize((ngiStreamManager_t *)psMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Partial Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Partial Stream Manager: Initialize member
 */
static void
nglPartialStreamManagerInitializeMember(
    ngiPartialStreamManager_t *psMng)
{
    assert(psMng != 0);

    nglPartialStreamManagerInitializePointer(psMng);

    psMng->ngpsm_maxSize = 0;
    psMng->ngpsm_readNbytes = 0;
}

/**
 * Partial Stream Manager: Initialize pointer
 */
static void
nglPartialStreamManagerInitializePointer(ngiPartialStreamManager_t *psMng)
{
    assert(psMng != 0);

    psMng->ngpsm_smWhole = NULL;
}

/**
 * Partial Stream Manager: Destroy write data
 */
static int
nglPartialStreamManagerDestroyWriteData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    /* Do Nothing */
    return 1;
}

/**
 * Partial Stream Manager: Destroy read data
 */
static int
nglPartialStreamManagerDestroyReadData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    /* Do Nothing */
    return 1;
}

/**
 * Partial Stream Manager: get readable buffer.
 */
static int
nglPartialStreamManagerGetReadableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiPartialStreamManager_t *psMng;
    ngiStreamManager_t *smReadable;
    int result;
    size_t remainNbytes;
    size_t readableNbytes = 0;
    static const char fName[] = "nglPartialStreamManagerGetReadableBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglPartialStreamManagerTypeInfomation);
    assert(buf != NULL);
    assert(nBytes != NULL);
        
    psMng = (ngiPartialStreamManager_t *)sMng;

    assert(psMng->ngpsm_maxSize >= psMng->ngpsm_readNbytes);
    remainNbytes = psMng->ngpsm_maxSize - psMng->ngpsm_readNbytes;

    /* Check number of bytes required */
    if (remainNbytes < nBytesRequired) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Number of bytes required is too large.\n", fName);
	return  0;
    }

    for (smReadable = psMng->ngpsm_smWhole;
         (smReadable != NULL) &&
         (smReadable->ngsm_typeInfomation != &nglDelimiterStreamManagerTypeInfomation);
         smReadable = smReadable->ngsm_next) {

        /* Get readable buffer from whole Stream Manager */
        result = ngiStreamManagerGetReadableBuffer(
            smReadable, buf, 0, &readableNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get readable buffer from whole Stream Buffer.\n", fName);
            return  0;
        }

        if (readableNbytes != 0) {
            break;
        }
    }
    psMng->ngpsm_smWhole = smReadable;

    if (readableNbytes > remainNbytes) {
        readableNbytes = remainNbytes;
    }

    /* Check Underflow */
    if (readableNbytes < nBytesRequired) {
	NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Underflow the Stream Buffer.\n", fName);
	return  0;
    }

    *nBytes = readableNbytes;

    return 1;
}

static int
nglPartialStreamManagerReadBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiPartialStreamManager_t *psMng;
    size_t remainNbytes;
    int result;
    static const char fName[] = "nglPartialStreamManagerReadBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglPartialStreamManagerTypeInfomation);

    psMng = (ngiPartialStreamManager_t *)sMng;

    assert(psMng->ngpsm_maxSize >= psMng->ngpsm_readNbytes);
    remainNbytes = psMng->ngpsm_maxSize - psMng->ngpsm_readNbytes;

    if (nBytes == 0) {
        return 1;
    }

    if ((remainNbytes < nBytes) ||
        (psMng->ngpsm_smWhole == NULL) ||
        (psMng->ngpsm_smWhole->ngsm_typeInfomation ==
         &nglDelimiterStreamManagerTypeInfomation)) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Number of bytes read is too large.\n", fName);
	return  0;
    }

    /* Read buffer from whole Stream Manager */
    result = ngiStreamManagerReadBuffer(
        psMng->ngpsm_smWhole, nBytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't read buffer from whole Stream Buffer.\n", fName);
	return  0;
    }
    psMng->ngpsm_readNbytes += nBytes;

    /* Success */
    return 1;
}

static int
nglPartialStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiPartialStreamManager_t *psMng;
    ngiStreamManager_t *smCur;
    size_t readableNbytes = 0;
    size_t wholeNbytes;
    size_t wholeTotalBytes;
    int result;
    int tmpIsTooLarge = 0;
    static const char fName[] = "nglPartialStreamManagerGetBytesOfReadableData";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglPartialStreamManagerTypeInfomation);
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);

    psMng = (ngiPartialStreamManager_t *)sMng;

    assert(psMng->ngpsm_maxSize >= psMng->ngpsm_readNbytes);
    readableNbytes = psMng->ngpsm_maxSize - psMng->ngpsm_readNbytes;

    wholeTotalBytes = 0;
    for (smCur = psMng->ngpsm_smWhole;
         smCur != NULL;
         smCur = smCur->ngsm_next) {
        result = ngiStreamManagerGetBytesOfReadableData(
            smCur, &tmpIsTooLarge, &wholeNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get number of bytes of readable.\n",
                fName);
            return 0;
        }

        if (smCur->ngsm_typeInfomation
            == &nglDelimiterStreamManagerTypeInfomation) {
            break;
        }
        if (tmpIsTooLarge != 0) {
            break;
        }
        wholeTotalBytes += wholeNbytes;
    }

    if ((tmpIsTooLarge != 0) ||
        (readableNbytes < wholeTotalBytes)) {
        wholeTotalBytes = readableNbytes;
    }

    *nBytes = wholeTotalBytes;
    *isTooLarge = 0;

    return 1;
}

/**
 * Receiving Stream Manager
 */
static void nglReceivingStreamManagerInitializeMember(ngiReceivingStreamManager_t *);
static void nglReceivingStreamManagerInitializePointer(ngiReceivingStreamManager_t *);

static int nglReceivingStreamManagerGetWritableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglReceivingStreamManagerWriteBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglReceivingStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglReceivingStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

ngiStreamManagerTypeInformation_t nglReceivingStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglNonImplementationStreamManagerDestroyWriteData,
    nglNonImplementationStreamManagerDestroyReadData,

    /* Buffer */
    nglReceivingStreamManagerGetWritableBuffer,
    nglReceivingStreamManagerWriteBuffer,
    nglNonImplementationStreamManagerGetReadableBuffer,
    nglNonImplementationStreamManagerReadBuffer,

    nglNonImplementationStreamManagerWriteDirectly,

    /* Get Size*/
    nglReceivingStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglReceivingStreamManagerDestruct
};

ngiStreamManager_t *
ngiReceivingStreamManagerConstruct(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    void *pointer,
    ngLog_t *log,
    int *error) 
{
    int result;
    ngiReceivingStreamManager_t *sMng;
    static const char fName[] = "ngiReceivingStreamManagerConstruct";

    /* Allocate */
    sMng = ngiReceivingStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiReceivingStreamManagerInitialize(sMng, argElement, protocol, argHead, pointer, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert((void *)&sMng->ngrsm_base == (void *)sMng);
    return &sMng->ngrsm_base;

    /* Error occurred */
error:
    result = ngiReceivingStreamManagerFree(sMng, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
    }

    return 0;
}
ngiReceivingStreamManager_t *
ngiReceivingStreamManagerAllocate(
    ngLog_t *log,
    int *error)
{
    ngiReceivingStreamManager_t *sMng;
    static const char fName[] = "ngiReceivingStreamManagerAllocate";

    /* Allocate */
    sMng = globus_libc_calloc(1, sizeof (ngiReceivingStreamManager_t));
    if (sMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return sMng;
}

int
ngiReceivingStreamManagerFree(
    ngiReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(rsMng != NULL);

    /* Free */
    globus_libc_free(rsMng);

    /* Success */
    return 1;
}
int
ngiReceivingStreamManagerInitialize(
    ngiReceivingStreamManager_t *rsMng,
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    void *pointer,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiReceivingStreamManagerInitialize";

    /* Check the arguments */
    assert(rsMng != 0);
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHead != NULL);

    /* Initialize the base and the members */
    result = ngiStreamManagerInitialize((ngiStreamManager_t *)rsMng,
        &nglReceivingStreamManagerTypeInfomation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Stream Manager.\n",
             fName);
        return 0;
    }
    nglReceivingStreamManagerInitializeMember(rsMng);

    rsMng->ngrsm_argElement = argElement;
    rsMng->ngrsm_protocol   = protocol;
    rsMng->ngrsm_argHead    = argHead;
    rsMng->ngrsm_pointer    = pointer;

    rsMng->ngrsm_writeNbytes = 0;
    rsMng->ngrsm_size =
        argHead->ngpad_nElements * argElement->ngae_nativeDataNbytes;
    rsMng->ngrsm_paddingNbytes = rsMng->ngrsm_size % BYTES_PER_XDR_UNIT;
    if (rsMng->ngrsm_paddingNbytes > 0) {
        rsMng->ngrsm_paddingNbytes =
            BYTES_PER_XDR_UNIT - rsMng->ngrsm_paddingNbytes;
    }

    /* Success */
    return 1;
}

static void
nglReceivingStreamManagerInitializeMember(
    ngiReceivingStreamManager_t *rsMng)
{
    int i;
    assert(rsMng != NULL);

    nglReceivingStreamManagerInitializePointer(rsMng);

    rsMng->ngrsm_writeNbytes = 0;
    rsMng->ngrsm_size        = 0;
    rsMng->ngrsm_paddingNbytes = 0;

    for (i = 0;i < BYTES_PER_XDR_UNIT;i++) {
        rsMng->ngrsm_padding[i] = 0;
    }
}

static void
nglReceivingStreamManagerInitializePointer(
    ngiReceivingStreamManager_t *rsMng)
{
    assert(rsMng != NULL);

    rsMng->ngrsm_argElement = NULL;
    rsMng->ngrsm_protocol   = NULL;
    rsMng->ngrsm_argHead    = NULL;
    rsMng->ngrsm_pointer    = NULL;
}

int
ngiReceivingStreamManagerFinalize(
    ngiReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiReceivingStreamManagerFinalize";
    assert(rsMng != NULL);

    /* Initialize the members */
    nglReceivingStreamManagerInitializeMember(rsMng);

    result = ngiStreamManagerFinalize((ngiStreamManager_t *)rsMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the Stream Manager.\n", fName);
        return 0;
    }

    return 1;
}

static int
nglReceivingStreamManagerGetWritableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiReceivingStreamManager_t *rsMng;
    size_t writableNbytes;
    size_t sizeWithPadding;
    unsigned char *pointer;
    static const char fName[] = "nglReceivingStreamManagerGetWritableBuffer";

    assert(nBytes != NULL);
    assert(buf != NULL);
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglReceivingStreamManagerTypeInfomation);

    rsMng = (ngiReceivingStreamManager_t *)sMng;

    sizeWithPadding = rsMng->ngrsm_size + rsMng->ngrsm_paddingNbytes;

    assert(sizeWithPadding >= rsMng->ngrsm_writeNbytes);

    if (rsMng->ngrsm_writeNbytes < rsMng->ngrsm_size) {
        writableNbytes = rsMng->ngrsm_size - rsMng->ngrsm_writeNbytes;
        pointer = (u_char *)rsMng->ngrsm_pointer;
        pointer = &pointer[rsMng->ngrsm_writeNbytes];
    } else {
        /* Padding */
        writableNbytes = sizeWithPadding - rsMng->ngrsm_writeNbytes;
        assert(writableNbytes < BYTES_PER_XDR_UNIT);
        pointer = rsMng->ngrsm_padding;
    }

    if (writableNbytes < nBytesRequired) {
	NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Underflow buffer.\n", fName);
	return  0;
    }

    *buf = pointer;
    *nBytes = writableNbytes;

    return 1;
}

static int
nglReceivingStreamManagerWriteBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiReceivingStreamManager_t *rsMng;
    size_t writableNbytes;
    size_t sizeWithPadding;
    static const char fName[] = "nglReceivingStreamManagerWriteBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglReceivingStreamManagerTypeInfomation);

    rsMng = (ngiReceivingStreamManager_t *)sMng;

    sizeWithPadding = rsMng->ngrsm_size + rsMng->ngrsm_paddingNbytes;

    assert(sizeWithPadding >= rsMng->ngrsm_writeNbytes);
    if (rsMng->ngrsm_writeNbytes < rsMng->ngrsm_size) {
        writableNbytes = rsMng->ngrsm_size - rsMng->ngrsm_writeNbytes;
    } else {
        /* Padding */
        writableNbytes = sizeWithPadding - rsMng->ngrsm_writeNbytes;
        assert(writableNbytes < BYTES_PER_XDR_UNIT);
    }

    if (writableNbytes < nBytes) {
	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Overflow buffer.\n", fName);
        return 0;
    }
    rsMng->ngrsm_writeNbytes += nBytes;

    return 1;
}

static int
nglReceivingStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);
    *nBytes = 0;
    *isTooLarge = 0;
    return 1;
}

static int
nglReceivingStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiReceivingStreamManager_t *rsMng;
    static const char fName[] = "nglReceivingStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglReceivingStreamManagerTypeInfomation);

    rsMng = (ngiReceivingStreamManager_t *)sMng;

    /* Finalize */
    result = ngiReceivingStreamManagerFinalize(rsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiReceivingStreamManagerFree(rsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * String Receiving Stream Manager
 */
static void nglStringReceivingStreamManagerInitializeMember(ngiStringReceivingStreamManager_t *);
static void nglStringReceivingStreamManagerInitializePointer(ngiStringReceivingStreamManager_t *);

static int nglStringReceivingStreamManagerGetWritableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglStringReceivingStreamManagerWriteBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglStringReceivingStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglStringReceivingStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

static size_t nglStringReceivingStreamManagerGetBytesOfWritableData(ngiStringReceivingStreamManager_t *);

ngiStreamManagerTypeInformation_t nglStringReceivingStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglNonImplementationStreamManagerDestroyWriteData,
    nglNonImplementationStreamManagerDestroyReadData,

    /* Buffer */
    nglStringReceivingStreamManagerGetWritableBuffer,
    nglStringReceivingStreamManagerWriteBuffer,
    nglNonImplementationStreamManagerGetReadableBuffer,
    nglNonImplementationStreamManagerReadBuffer,

    nglNonImplementationStreamManagerWriteDirectly,

    /* Get Size*/
    nglStringReceivingStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglStringReceivingStreamManagerDestruct
};

#define NGL_STRING_RECEIVING_STATE_SIZE    1
#define NGL_STRING_RECEIVING_STATE_STRING  2
#define NGL_STRING_RECEIVING_STATE_PADDING 3

/**
 * String Receiving Stream Manager: Construct
 */
ngiStreamManager_t *
ngiStringReceivingStreamManagerConstruct(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    ngLog_t *log,
    int *error) 
{
    int result;
    ngiStringReceivingStreamManager_t *sMng;
    static const char fName[] = "ngiStringReceivingStreamManagerConstruct";

    /* Allocate */
    sMng = ngiStringReceivingStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiStringReceivingStreamManagerInitialize(sMng, argElement, protocol, argHead, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert((void *)&sMng->ngsrsm_base == (void *)sMng);
    return &sMng->ngsrsm_base;

    /* Error occurred */
error:
    if (sMng != NULL) {
        result = ngiStringReceivingStreamManagerFree(sMng, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't deallocate the storage for Stream Manager.\n", fName);
        }
        sMng = NULL;
    }

    return 0;
}

/**
 * String Receiving Stream Manager: Allocate
 */
ngiStringReceivingStreamManager_t *
ngiStringReceivingStreamManagerAllocate(
    ngLog_t *log,
    int *error)
{
    ngiStringReceivingStreamManager_t *sMng;
    static const char fName[] = "ngiStringReceivingStreamManagerAllocate";

    /* Allocate */
    sMng = globus_libc_calloc(1, sizeof (ngiStringReceivingStreamManager_t));
    if (sMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return sMng;
}

/**
 * String Receiving Stream Manager: Free
 */
int
ngiStringReceivingStreamManagerFree(
    ngiStringReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(rsMng != NULL);

    /* Free */
    globus_libc_free(rsMng);

    /* Success */
    return 1;
}

/**
 * String Receiving Stream Manager: initialize
 */
int
ngiStringReceivingStreamManagerInitialize(
    ngiStringReceivingStreamManager_t *rsMng,
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiStringReceivingStreamManagerInitialize";

    /* Check the arguments */
    assert(rsMng != 0);
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHead != NULL);

    /* Initialize the base and the members */
    result = ngiStreamManagerInitialize((ngiStreamManager_t *)rsMng,
        &nglStringReceivingStreamManagerTypeInfomation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Stream Manager.\n",
             fName);
        return 0;
    }
    nglStringReceivingStreamManagerInitializeMember(rsMng);

    rsMng->ngsrsm_argElement = argElement;
    rsMng->ngsrsm_protocol   = protocol;
    rsMng->ngsrsm_argHead    = argHead;

    rsMng->ngsrsm_state = NGL_STRING_RECEIVING_STATE_SIZE;
    rsMng->ngsrsm_pointer = rsMng->ngsrsm_buffer;

    /* Initialize Net Communicator */
    result = ngiNetCommunicatorInitialize(&rsMng->ngsrsm_netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        goto error;
    }
    rsMng->ngsrsm_netCommInitialized = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngiStringReceivingStreamManagerFinalize(rsMng, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the String Receiving Stream Manager.\n", fName);
    }
        
    return 0;
}

/**
 * String Receiving Stream Manager: initialize member
 */
static void
nglStringReceivingStreamManagerInitializeMember(
    ngiStringReceivingStreamManager_t *rsMng)
{
    assert(rsMng != NULL);

    nglStringReceivingStreamManagerInitializePointer(rsMng);

    rsMng->ngsrsm_state = 0;
    rsMng->ngsrsm_length = 0;
    rsMng->ngsrsm_paddingNbytes= 0;

    rsMng->ngsrsm_writeNbytes = 0;

    rsMng->ngsrsm_netCommInitialized = 0;

    rsMng->ngsrsm_number = 0;
    memset(rsMng->ngsrsm_buffer, 0, BYTES_PER_XDR_UNIT);
}

/**
 * String Receiving Stream Manager: Initialize pointer
 */
static void
nglStringReceivingStreamManagerInitializePointer(
    ngiStringReceivingStreamManager_t *rsMng)
{
    assert(rsMng != NULL);

    rsMng->ngsrsm_argElement = NULL;
    rsMng->ngsrsm_protocol   = NULL;
    rsMng->ngsrsm_argHead    = NULL;
    rsMng->ngsrsm_pointer    = NULL;
}

/**
 * String Receiving Stream Manager: Finalize
 */
int
ngiStringReceivingStreamManagerFinalize(
    ngiStringReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiStringReceivingStreamManagerFinalize";

    assert(rsMng != NULL);

    /* Finalize the NET Communicator */
    if (rsMng->ngsrsm_netCommInitialized != 0) {
        rsMng->ngsrsm_netCommInitialized = 0;
        result = ngiNetCommunicatorFinalize(&rsMng->ngsrsm_netComm, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the NET Communicator.\n", fName);
            error = NULL;
            ret = 0;
        }
    }

    /* Initialize the members */
    nglStringReceivingStreamManagerInitializeMember(rsMng);

    result = ngiStreamManagerFinalize((ngiStreamManager_t *)rsMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the Stream Manager.\n", fName);
        error = NULL;
        ret = 0;
    }

    return ret;
}

static int
nglStringReceivingStreamManagerGetWritableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiStringReceivingStreamManager_t *rsMng;
    size_t writableNbytes;
    static const char fName[] = "nglStringReceivingStreamManagerGetWritableBuffer";

    assert(nBytes != NULL);
    assert(buf != NULL);
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglStringReceivingStreamManagerTypeInfomation);

    rsMng = (ngiStringReceivingStreamManager_t *)sMng;

    /* Get Writable Size */
    writableNbytes = nglStringReceivingStreamManagerGetBytesOfWritableData(rsMng);

    if (writableNbytes < nBytesRequired) {
        NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Underflow buffer.\n", fName);
        goto error;
    }

    *buf = &((u_char *)rsMng->ngsrsm_pointer)[rsMng->ngsrsm_writeNbytes];
    *nBytes = writableNbytes;

    return 1;

    /* Error occurred */
error:
    return 0;
}

static int
nglStringReceivingStreamManagerWriteBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiStringReceivingStreamManager_t *rsMng;
    size_t writableNbytes;
    int result;
    int stateChange = 0;
    long paddingNbytes;
    static const char fName[] = "nglStringReceivingStreamManagerWriteBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglStringReceivingStreamManagerTypeInfomation);

    rsMng = (ngiStringReceivingStreamManager_t *)sMng;

    /* Get Writable Size */
    writableNbytes = nglStringReceivingStreamManagerGetBytesOfWritableData(rsMng);

    if (writableNbytes < nBytes) {
        NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Underflow buffer.\n", fName);
        goto error;
    }

    rsMng->ngsrsm_writeNbytes += nBytes;

    do {
        stateChange = 0;
        switch (rsMng->ngsrsm_state) {
        case NGL_STRING_RECEIVING_STATE_SIZE:
            if (rsMng->ngsrsm_writeNbytes == rsMng->ngsrsm_protocol->ngp_xdrDataSize.ngds_long) {
                /* Get String Size */
                result = ngiNetCommunicatorReadArray(
                    &rsMng->ngsrsm_netComm,
                    NG_ARGUMENT_DATA_TYPE_LONG,
                    rsMng->ngsrsm_pointer, rsMng->ngsrsm_writeNbytes,
                    &rsMng->ngsrsm_length, 1,
                    log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't read the data from Net Communicator.\n",
                        fName);
                    goto error;
                }
                if (rsMng->ngsrsm_length < 0) {
                    NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: The string length is less than zero.\n",
                        fName);
                    goto error;
                } 
                if (rsMng->ngsrsm_length == 0) {
                    /* Null String */
                    stateChange = 1;
                    rsMng->ngsrsm_state = NGL_STRING_RECEIVING_STATE_STRING;
                    rsMng->ngsrsm_writeNbytes = 0;
                    rsMng->ngsrsm_argElement->ngae_pointer.ngap_stringArray[rsMng->ngsrsm_number] = NULL;
                    break;
                } 
                stateChange = 1;

                rsMng->ngsrsm_state = NGL_STRING_RECEIVING_STATE_STRING;
                rsMng->ngsrsm_writeNbytes = 0;
                paddingNbytes = rsMng->ngsrsm_length % BYTES_PER_XDR_UNIT;
                if (paddingNbytes > 0) {
                    paddingNbytes = BYTES_PER_XDR_UNIT - paddingNbytes;
                }
                rsMng->ngsrsm_paddingNbytes = paddingNbytes;

                /* Allocate the storage for string */
                rsMng->ngsrsm_pointer = malloc(rsMng->ngsrsm_length + 1);
                if (rsMng->ngsrsm_pointer == NULL) {
                    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                        NG_LOG_LEVEL_ERROR,
                        NULL, "%s: Can't allocate the storage for string.\n",
                        fName);
                    goto error;
                }

                /* Set to argument element */
                assert(rsMng->ngsrsm_number < rsMng->ngsrsm_argHead->ngpad_nElements);
                rsMng->ngsrsm_argElement->ngae_pointer.ngap_stringArray[rsMng->ngsrsm_number] =
                    rsMng->ngsrsm_pointer;
            }
            break;
        case NGL_STRING_RECEIVING_STATE_STRING:
            /* Write String */
            if (rsMng->ngsrsm_writeNbytes == rsMng->ngsrsm_length) {
                if (rsMng->ngsrsm_pointer != NULL) {
                    ((char *)rsMng->ngsrsm_pointer)[rsMng->ngsrsm_length] = '\0';
                }
                rsMng->ngsrsm_state = NGL_STRING_RECEIVING_STATE_PADDING;
                rsMng->ngsrsm_writeNbytes = 0;
                rsMng->ngsrsm_pointer = rsMng->ngsrsm_buffer;
                stateChange = 1;
            }
            break;
        case NGL_STRING_RECEIVING_STATE_PADDING:
            /* Write String */
            if (rsMng->ngsrsm_writeNbytes == rsMng->ngsrsm_paddingNbytes) {
                rsMng->ngsrsm_number++;
                rsMng->ngsrsm_state = NGL_STRING_RECEIVING_STATE_SIZE;
                rsMng->ngsrsm_writeNbytes = 0;
                rsMng->ngsrsm_pointer = rsMng->ngsrsm_buffer;
                rsMng->ngsrsm_paddingNbytes = 0;
            }
            break;
        default:
            /* NOTREACHED */
            abort();
        }
    } while (stateChange != 0);

    return 1;

    /* Error occurred */
error:
    return 0;
}

static int
nglStringReceivingStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    /* Check argument */
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);

    *nBytes = 0;
    *isTooLarge = 0;
    return 1;
}

static int
nglStringReceivingStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStringReceivingStreamManager_t *rsMng;
    static const char fName[] = "nglStringReceivingStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglStringReceivingStreamManagerTypeInfomation);

    rsMng = (ngiStringReceivingStreamManager_t *)sMng;

    /* Finalize */
    result = ngiStringReceivingStreamManagerFinalize(rsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiStringReceivingStreamManagerFree(rsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}


/**
 * String Receiving Stream Manager: Get number of bytes of writable data.
 */
static size_t
nglStringReceivingStreamManagerGetBytesOfWritableData(
    ngiStringReceivingStreamManager_t *rsMng)
{
    size_t size;
    size_t writableNbytes;

    assert(rsMng != NULL);

    /* Get Writable Size */
    switch (rsMng->ngsrsm_state) {
    case NGL_STRING_RECEIVING_STATE_SIZE:
        size= rsMng->ngsrsm_protocol->ngp_xdrDataSize.ngds_long;
        break;
    case NGL_STRING_RECEIVING_STATE_STRING:  
        size = rsMng->ngsrsm_length;
        break;
    case NGL_STRING_RECEIVING_STATE_PADDING: 
        size = rsMng->ngsrsm_paddingNbytes;
        break;
    default:
        /* NOTREACHED */
        abort();
    }

    assert(rsMng->ngsrsm_writeNbytes <= size);
    writableNbytes = size - rsMng->ngsrsm_writeNbytes;

    if (rsMng->ngsrsm_number >= rsMng->ngsrsm_argHead->ngpad_nElements) {
        writableNbytes = 0;
    }

    return writableNbytes;
}
 
/**
 * File Receiving Stream Manager
 */
static void nglFileReceivingStreamManagerInitializeMember(ngiFileReceivingStreamManager_t *);
static void nglFileReceivingStreamManagerInitializePointer(ngiFileReceivingStreamManager_t *);

static int nglFileReceivingStreamManagerGetWritableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglFileReceivingStreamManagerWriteBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglFileReceivingStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglFileReceivingStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

static int nglFileReceivingStreamManagerOpenFile(ngiFileReceivingStreamManager_t *, ngLog_t *, int *);
static int nglFileReceivingStreamManagerCloseFile(ngiFileReceivingStreamManager_t *, ngLog_t *, int *);
static size_t nglFileReceivingStreamManagerGetBytesOfWritableData(ngiFileReceivingStreamManager_t *);

ngiStreamManagerTypeInformation_t nglFileReceivingStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglNonImplementationStreamManagerDestroyWriteData,
    nglNonImplementationStreamManagerDestroyReadData,

    /* Buffer */
    nglFileReceivingStreamManagerGetWritableBuffer,
    nglFileReceivingStreamManagerWriteBuffer,
    nglNonImplementationStreamManagerGetReadableBuffer,
    nglNonImplementationStreamManagerReadBuffer,

    nglNonImplementationStreamManagerWriteDirectly,

    /* Get Size*/
    nglFileReceivingStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglFileReceivingStreamManagerDestruct
};

/**
 * File Receiving Stream Manager: Construct
 */
ngiStreamManager_t *
ngiFileReceivingStreamManagerConstruct(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    int allocate,
    ngLog_t *log,
    int *error) 
{
    int result;
    ngiFileReceivingStreamManager_t *sMng;
    static const char fName[] = "ngiFileReceivingStreamManagerConstruct";

    /* Allocate */
    sMng = ngiFileReceivingStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiFileReceivingStreamManagerInitialize(sMng, argElement, protocol, argHead, allocate, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert((void *)&sMng->ngfrsm_base == (void *)sMng);
    return &sMng->ngfrsm_base;

    /* Error occurred */
error:
    if (sMng != NULL) {
        result = ngiFileReceivingStreamManagerFree(sMng, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't deallocate the storage for Stream Manager.\n", fName);
        }
        sMng = NULL;
    }

    return 0;
}

/**
 * File Receiving Stream Manager: Allocate
 */
ngiFileReceivingStreamManager_t *
ngiFileReceivingStreamManagerAllocate(
    ngLog_t *log,
    int *error)
{
    ngiFileReceivingStreamManager_t *sMng;
    static const char fName[] = "ngiFileReceivingStreamManagerAllocate";

    /* Allocate */
    sMng = globus_libc_calloc(1, sizeof (ngiFileReceivingStreamManager_t));
    if (sMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return sMng;
}

/**
 * File Receiving Stream Manager: Free
 */
int
ngiFileReceivingStreamManagerFree(
    ngiFileReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(rsMng != NULL);

    /* Free */
    globus_libc_free(rsMng);

    /* Success */
    return 1;
}

/**
 * File Receiving Stream Manager: initialize
 */
int
ngiFileReceivingStreamManagerInitialize(
    ngiFileReceivingStreamManager_t *rsMng,
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    int allocate,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiFileReceivingStreamManagerInitialize";

    /* Check the arguments */
    assert(rsMng != 0);
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHead != NULL);

    /* Initialize the base and the members */
    result = ngiStreamManagerInitialize((ngiStreamManager_t *)rsMng,
        &nglFileReceivingStreamManagerTypeInfomation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Stream Manager.\n",
             fName);
        return 0;
    }
    nglFileReceivingStreamManagerInitializeMember(rsMng);

    rsMng->ngfrsm_argElement = argElement;
    rsMng->ngfrsm_protocol   = protocol;
    rsMng->ngfrsm_argHead    = argHead;
    rsMng->ngfrsm_allocate   = allocate;

    /* Allocate Buffer */
    rsMng->ngfrsm_bufferSize = NGI_PROTOCOL_STREAM_NBYTES;
    rsMng->ngfrsm_buffer = globus_libc_malloc(rsMng->ngfrsm_bufferSize);
    if (rsMng->ngfrsm_buffer == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for receiving files.\n", fName);
        goto error;
    }

    /* Initialize Net Communicator */
    result = ngiNetCommunicatorInitialize(&rsMng->ngfrsm_netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        goto error;
    }
    rsMng->ngfrsm_netCommInitialized = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngiFileReceivingStreamManagerFinalize(rsMng, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the File Receiving Stream Manager.\n", fName);
    }
        
    return 0;
}

/**
 * File Receiving Stream Manager: initialize member
 */
static void
nglFileReceivingStreamManagerInitializeMember(
    ngiFileReceivingStreamManager_t *rsMng)
{
    assert(rsMng != NULL);

    nglFileReceivingStreamManagerInitializePointer(rsMng);

    rsMng->ngfrsm_handleInitialized  = 0;
    rsMng->ngfrsm_netCommInitialized = 0;

    rsMng->ngfrsm_allocate = 0;
    rsMng->ngfrsm_fileSize = 0;
    rsMng->ngfrsm_fileType = 0;
    rsMng->ngfrsm_fileSizeWithPadding = 0;
    rsMng->ngfrsm_fileHeaderRead = 0;

    rsMng->ngfrsm_number = 0;

    rsMng->ngfrsm_fileWriteNbytes   = 0;
    rsMng->ngfrsm_bufferWriteNbytes = 0;
    rsMng->ngfrsm_bufferSize = 0;
}

/**
 * File Receiving Stream Manager: Initialize pointer
 */
static void
nglFileReceivingStreamManagerInitializePointer(
    ngiFileReceivingStreamManager_t *rsMng)
{
    assert(rsMng != NULL);

    rsMng->ngfrsm_argElement = NULL;
    rsMng->ngfrsm_protocol   = NULL;
    rsMng->ngfrsm_argHead    = NULL;
    rsMng->ngfrsm_buffer     = NULL;
}

/**
 * File Receiving Stream Manager: Finalize
 */
int
ngiFileReceivingStreamManagerFinalize(
    ngiFileReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiFileReceivingStreamManagerFinalize";

    assert(rsMng != NULL);

    if (rsMng->ngfrsm_buffer != NULL) {
        globus_libc_free(rsMng->ngfrsm_buffer);
        rsMng->ngfrsm_buffer = NULL;
    }

    /* Close File */
    result = nglFileReceivingStreamManagerCloseFile(rsMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't close file.\n", fName);
        error = NULL;
        ret = 0;
    }

    /* Finalize the NET Communicator */
    if (rsMng->ngfrsm_netCommInitialized != 0) {
        rsMng->ngfrsm_netCommInitialized = 0;
        result = ngiNetCommunicatorFinalize(&rsMng->ngfrsm_netComm, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the NET Communicator.\n", fName);
            error = NULL;
            ret = 0;
        }
    }

    /* Initialize the members */
    nglFileReceivingStreamManagerInitializeMember(rsMng);

    result = ngiStreamManagerFinalize((ngiStreamManager_t *)rsMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the Stream Manager.\n", fName);
        error = NULL;
        ret = 0;
    }

    return ret;
}

static int
nglFileReceivingStreamManagerGetWritableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiFileReceivingStreamManager_t *rsMng;
    size_t writableNbytes;
    int result;
    static const char fName[] = "nglFileReceivingStreamManagerGetWritableBuffer";

    assert(nBytes != NULL);
    assert(buf != NULL);
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglFileReceivingStreamManagerTypeInfomation);

    rsMng = (ngiFileReceivingStreamManager_t *)sMng;

    /* This File is end? */
    assert((rsMng->ngfrsm_number     == rsMng->ngfrsm_argElement->ngae_fileNumber) ||
           (rsMng->ngfrsm_number + 1 == rsMng->ngfrsm_argElement->ngae_fileNumber));
    if ((rsMng->ngfrsm_fileHeaderRead != 0) &&
        (rsMng->ngfrsm_fileSize >= 0)) {
        if ((rsMng->ngfrsm_fileSizeWithPadding == rsMng->ngfrsm_fileWriteNbytes) &&
            (rsMng->ngfrsm_number == rsMng->ngfrsm_argElement->ngae_fileNumber)){
            rsMng->ngfrsm_argElement->ngae_fileNumber++;
        }
    }

    /* Next File*/
    if (rsMng->ngfrsm_number < rsMng->ngfrsm_argElement->ngae_fileNumber) {
        result = nglFileReceivingStreamManagerCloseFile(rsMng, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't close file.\n", fName);
            goto error;
        }

        if (rsMng->ngfrsm_fileHeaderRead == 0) {
            NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Underflow buffer.\n", fName);
            goto error;
        }
        rsMng->ngfrsm_number++;

        if (rsMng->ngfrsm_number >= rsMng->ngfrsm_argHead->ngpad_nElements) {
            NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Underflow buffer.\n", fName);
            goto error;
        }

        rsMng->ngfrsm_fileHeaderRead = 0;
    }

    /* Get Writable Size */
    writableNbytes = nglFileReceivingStreamManagerGetBytesOfWritableData(rsMng);

    if (writableNbytes < nBytesRequired) {
        NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Underflow buffer.\n", fName);
        return  0;
    }

    *buf = &((u_char *)rsMng->ngfrsm_buffer)[rsMng->ngfrsm_bufferWriteNbytes];
    *nBytes = writableNbytes;

    return 1;

    /* Error occurred */
error:
    return 0;
}

static int
nglFileReceivingStreamManagerWriteBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiFileReceivingStreamManager_t *rsMng;
    size_t writableNbytes;
    globus_result_t gResult;
    globus_size_t writeNbytes;
    int result;
    long paddingNbytes;
    size_t headerSize;
    long fileHeader[NGI_PROTOCOL_FILE_TRANSFER_HEADER_SIZE];
    int index;
    static const char fName[] = "nglFileReceivingStreamManagerWriteBuffer";

    /* Check arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglFileReceivingStreamManagerTypeInfomation);

    rsMng = (ngiFileReceivingStreamManager_t *)sMng;

    /* Get Writable Size */
    writableNbytes = nglFileReceivingStreamManagerGetBytesOfWritableData(rsMng);

    if (writableNbytes < nBytes) {
        NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Underflow buffer.\n", fName);
        goto error;
    }

    if (rsMng->ngfrsm_fileHeaderRead == 0) {
        rsMng->ngfrsm_bufferWriteNbytes += nBytes;
        /* Write File Size */
        headerSize = rsMng->ngfrsm_protocol->ngp_xdrDataSize.ngds_long 
            * NGI_PROTOCOL_FILE_TRANSFER_HEADER_SIZE;
        if (rsMng->ngfrsm_bufferWriteNbytes ==  headerSize) {
            /* Get File Size */
            result = ngiNetCommunicatorReadArray(
                &rsMng->ngfrsm_netComm,
                NG_ARGUMENT_DATA_TYPE_LONG,
                rsMng->ngfrsm_buffer, rsMng->ngfrsm_bufferWriteNbytes,
                fileHeader, NGI_PROTOCOL_FILE_TRANSFER_HEADER_SIZE,
                log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't read the data from Net Communicator.\n",
                    fName);
                goto error;
            }
            index = 0;
            rsMng->ngfrsm_fileType = fileHeader[index++];
            rsMng->ngfrsm_fileSize = fileHeader[index++];
            rsMng->ngfrsm_fileSizeWithPadding = rsMng->ngfrsm_fileSize;

            /* Is Data contained */
            switch (rsMng->ngfrsm_fileType) {
            case NGI_PROTOCOL_FILE_TRANSFER_FILE_DATA:
                result = nglFileReceivingStreamManagerOpenFile(rsMng, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't open file.\n", fName);
                    goto error;
                }

                if (rsMng->ngfrsm_fileSize >= 0) {
                    paddingNbytes = rsMng->ngfrsm_fileSize % BYTES_PER_XDR_UNIT;
                    if (paddingNbytes > 0) {
                        paddingNbytes = BYTES_PER_XDR_UNIT - paddingNbytes;
                    }
                    rsMng->ngfrsm_fileSizeWithPadding = 
                        rsMng->ngfrsm_fileSize + paddingNbytes;
                }

                rsMng->ngfrsm_fileHeaderRead = 1;
                rsMng->ngfrsm_bufferWriteNbytes = 0;
                break;
            case NGI_PROTOCOL_FILE_TRANSFER_NULL:
                if (rsMng->ngfrsm_allocate != 0) {
                    rsMng->ngfrsm_argElement->ngae_tmpFileNameTable[rsMng->ngfrsm_number] = NULL;
                    if (rsMng->ngfrsm_argElement->ngae_nDimensions == 0) {
                        assert(rsMng->ngfrsm_number == 0);
                        rsMng->ngfrsm_argElement->ngae_pointer.ngap_fileName = NULL;
                    } else {
                        rsMng->ngfrsm_argElement->ngae_pointer.
                            ngap_fileNameArray[rsMng->ngfrsm_number] = NULL;
                    }
                }
                break;
            case NGI_PROTOCOL_FILE_TRANSFER_EMPTY_FILENAME:
                if (rsMng->ngfrsm_allocate != 0) {
                    rsMng->ngfrsm_argElement->ngae_tmpFileNameTable[rsMng->ngfrsm_number] = "";
                    if (rsMng->ngfrsm_argElement->ngae_nDimensions == 0) {
                        assert(rsMng->ngfrsm_number == 0);
                        rsMng->ngfrsm_argElement->ngae_pointer.ngap_fileName = "";
                    } else {
                        rsMng->ngfrsm_argElement->ngae_pointer.
                            ngap_fileNameArray[rsMng->ngfrsm_number] = "";
                    }
                }
                break;
            default:
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Invalid file type.\n", fName);
                goto error;
            }
        }
    } else {
        /* Write File */
        assert(rsMng->ngfrsm_bufferSize >= rsMng->ngfrsm_bufferWriteNbytes);
        if (rsMng->ngfrsm_handleInitialized == 0) {
            NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Underflow buffer.\n", fName);
            goto error;
        }

        writableNbytes = rsMng->ngfrsm_bufferSize;

        rsMng->ngfrsm_fileWriteNbytes += nBytes;
        if (rsMng->ngfrsm_fileSize >= 0) {
            if (rsMng->ngfrsm_fileWriteNbytes > rsMng->ngfrsm_fileSize) {
                paddingNbytes = rsMng->ngfrsm_fileWriteNbytes - rsMng->ngfrsm_fileSize;
                nBytes -= paddingNbytes;
            }
        }
        if (nBytes > 0) {
            gResult = globus_io_write(&rsMng->ngfrsm_handle,
                (void *)rsMng->ngfrsm_buffer, nBytes, &writeNbytes);
            if (gResult != GLOBUS_SUCCESS) {
                NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
                ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                    NG_LOG_LEVEL_FATAL, NULL,
                    "%s: globus_io_file_write() failed.\n", fName);
                    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                    NG_LOG_LEVEL_FATAL, fName, gResult, NULL);
                goto error;
            }
        }
    }

    return 1;

    /* Error occurred */
error:
    return 0;
}

static int
nglFileReceivingStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    assert(nBytes != NULL);
    assert(isTooLarge != NULL);
    *nBytes = 0;
    *isTooLarge = 0;
    return 1;
}

static int
nglFileReceivingStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiFileReceivingStreamManager_t *rsMng;
    static const char fName[] = "nglFileReceivingStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglFileReceivingStreamManagerTypeInfomation);

    rsMng = (ngiFileReceivingStreamManager_t *)sMng;

    /* Finalize */
    result = ngiFileReceivingStreamManagerFinalize(rsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiFileReceivingStreamManagerFree(rsMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * File Receiving Stream Manager: Open File 
 */
static int
nglFileReceivingStreamManagerOpenFile(
    ngiFileReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    char *fileName;
    int result;
    static const char fName[] = "nglFileReceivingStreamManagerOpenFile";

    /* Check Arguments */
    assert(rsMng != NULL);
    assert(rsMng->ngfrsm_handleInitialized == 0);

    if (rsMng->ngfrsm_allocate == 0) {
        /* Get filename */
        if (rsMng->ngfrsm_argElement->ngae_nDimensions == 0) {
            assert(rsMng->ngfrsm_number == 0);
            fileName = rsMng->ngfrsm_argElement->ngae_pointer.ngap_fileName;
        } else {
            fileName = rsMng->ngfrsm_argElement->
                ngae_pointer.ngap_fileNameArray[rsMng->ngfrsm_number];
        }
    } else {
        /* Create Temporary File */
        fileName = ngiTemporaryFileCreate(rsMng->ngfrsm_protocol->ngp_attr.ngpa_tmpDir, log, error);
        if (fileName == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't create temporary file.\n", fName);
            goto error;
        }
        rsMng->ngfrsm_argElement->ngae_tmpFileNameTable[rsMng->ngfrsm_number] = fileName;
        if (rsMng->ngfrsm_argElement->ngae_nDimensions == 0) {
            assert(rsMng->ngfrsm_number == 0);
            rsMng->ngfrsm_argElement->ngae_pointer.ngap_fileName = fileName;
        } else {
            rsMng->ngfrsm_argElement->ngae_pointer.
                ngap_fileNameArray[rsMng->ngfrsm_number] = fileName;
        }
    }

    /* Open File */
    /* Specified permission for OUT mode file  */
    gResult = globus_io_file_open(fileName,
        GLOBUS_IO_FILE_CREAT | GLOBUS_IO_FILE_WRONLY | GLOBUS_IO_FILE_TRUNC,
        GLOBUS_IO_FILE_IRUSR | GLOBUS_IO_FILE_IWUSR | GLOBUS_IO_FILE_IROTH |
        GLOBUS_IO_FILE_IWOTH | GLOBUS_IO_FILE_IRGRP | GLOBUS_IO_FILE_IWGRP,
        NULL, &rsMng->ngfrsm_handle);
    if (gResult != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                    NG_LOG_LEVEL_FATAL, NULL,
                    "%s: globus_io_file_open() failed.\n", fName);
        ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
        NG_LOG_LEVEL_FATAL, fName, gResult, NULL);

        goto error;
    }
    rsMng->ngfrsm_handleInitialized = 1;

    rsMng->ngfrsm_fileWriteNbytes = 0;

    return 1;
error:
    if ((rsMng->ngfrsm_allocate != 0) && (fileName != NULL)) {
        result = ngiTemporaryFileDestroy(fileName, log, NULL);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destroy the Temporary File Name.\n", fName);
        }
        if (rsMng->ngfrsm_argElement->ngae_nDimensions == 0) {
            assert(rsMng->ngfrsm_number == 0);
            rsMng->ngfrsm_argElement->ngae_pointer.ngap_fileName = NULL;
        } else {
            rsMng->ngfrsm_argElement->ngae_pointer.
                ngap_fileNameArray[rsMng->ngfrsm_number] = NULL;
        }
        fileName = NULL;
    }

    return 0;
}

/**
 * File Receiving Stream Manager: Close File 
 */
static int
nglFileReceivingStreamManagerCloseFile(
    ngiFileReceivingStreamManager_t *rsMng,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    static const char fName[] = "nglFileReceivingStreamManagerCloseFile";

    assert(rsMng != NULL);

    if (rsMng->ngfrsm_handleInitialized != 0) {
        rsMng->ngfrsm_handleInitialized = 0;
        /* Close */
        gResult = globus_io_close(&rsMng->ngfrsm_handle);
        if (gResult != GLOBUS_SUCCESS) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
                ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_file_close() failed.\n", fName);
            ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, fName, gResult, NULL);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * File Receiving Stream Manager: Get number of bytes of writable data.
 */
static size_t
nglFileReceivingStreamManagerGetBytesOfWritableData(
    ngiFileReceivingStreamManager_t *rsMng)
{
    size_t writableNbytes;
    size_t remainNbytes;
    size_t headerSize;

    assert(rsMng != NULL);

    /* Get Writable Size */
    if (rsMng->ngfrsm_fileHeaderRead== 0) {
        /* Write File Size */
        headerSize = rsMng->ngfrsm_protocol->ngp_xdrDataSize.ngds_long
            * NGI_PROTOCOL_FILE_TRANSFER_HEADER_SIZE;

        assert(headerSize >  rsMng->ngfrsm_bufferWriteNbytes);
        assert(headerSize <= rsMng->ngfrsm_bufferSize);
        writableNbytes = headerSize - rsMng->ngfrsm_bufferWriteNbytes;
    } else {
        /* Write File Content */
        assert(rsMng->ngfrsm_bufferSize >= rsMng->ngfrsm_bufferWriteNbytes);
        writableNbytes = rsMng->ngfrsm_bufferSize;
        if (rsMng->ngfrsm_fileSize >= 0) {
            assert(rsMng->ngfrsm_fileSizeWithPadding >= rsMng->ngfrsm_fileWriteNbytes);
            remainNbytes = rsMng->ngfrsm_fileSizeWithPadding - rsMng->ngfrsm_fileWriteNbytes;
            if (remainNbytes < writableNbytes) {
                writableNbytes = remainNbytes;
            }
        }
    }

    return writableNbytes;
}


/**
 * Conversion Method Stream Manager
 */

static void nglConversionMethodStreamManagerInitializeMember(ngiConversionMethodStreamManager_t *);
static void nglConversionMethodStreamManagerInitializePointer(ngiConversionMethodStreamManager_t *);

static int nglConversionMethodStreamManagerGetWritableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglConversionMethodStreamManagerWriteBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglConversionMethodStreamManagerGetReadableBuffer(ngiStreamManager_t *, void **, size_t, size_t *, ngLog_t *, int *);
static int nglConversionMethodStreamManagerReadBuffer(ngiStreamManager_t *, size_t, ngLog_t *, int *);
static int nglConversionMethodStreamManagerGetBytesOfReadableData(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);
static int nglConversionMethodStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);

ngiStreamManagerTypeInformation_t nglConversionMethodStreamManagerTypeInfomation = {
    /* Destroy Data */
    nglNonImplementationStreamManagerDestroyWriteData,
    nglNonImplementationStreamManagerDestroyReadData,

    /* Buffer */
    nglConversionMethodStreamManagerGetWritableBuffer,
    nglConversionMethodStreamManagerWriteBuffer,
    nglConversionMethodStreamManagerGetReadableBuffer,
    nglConversionMethodStreamManagerReadBuffer,

    nglNonImplementationStreamManagerWriteDirectly,

    /* Get Size*/
    nglConversionMethodStreamManagerGetBytesOfReadableData,

    /* Destruct */
    nglConversionMethodStreamManagerDestruct
};

ngiStreamManager_t *
ngiConversionMethodStreamManagerConstruct(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMngArg,
    ngLog_t *log,
    int *error) 
{
    int result;
    ngiConversionMethodStreamManager_t *sMng;
    static const char fName[] = "ngiConversionMethodStreamManagerConstruct";

    /* Allocate */
    sMng = ngiConversionMethodStreamManagerAllocate(log, error);
    if (sMng == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiConversionMethodStreamManagerInitialize(sMng, protocol, sMngArg, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stream Manager.\n", fName);
	goto error;
    }

    /* Success */
    assert((void *)&sMng->ngcsm_base == (void *)sMng);
    return &sMng->ngcsm_base;

    /* Error occurred */
error:
    result = ngiConversionMethodStreamManagerFree(sMng, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
    }

    return 0;
}
ngiConversionMethodStreamManager_t *
ngiConversionMethodStreamManagerAllocate(
    ngLog_t *log,
    int *error)
{
    ngiConversionMethodStreamManager_t *sMng;
    static const char fName[] = "ngiConversionMethodStreamManagerAllocate";

    /* Allocate */
    sMng = globus_libc_calloc(1, sizeof (ngiConversionMethodStreamManager_t));
    if (sMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stream Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return sMng;
}

int
ngiConversionMethodStreamManagerFree(
    ngiConversionMethodStreamManager_t *csMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(csMng != NULL);

    /* Free */
    globus_libc_free(csMng);

    /* Success */
    return 1;
}
int
ngiConversionMethodStreamManagerInitialize(
    ngiConversionMethodStreamManager_t *csMng,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMngArg,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiConversionMethodStreamManagerInitialize";

    /* Check the arguments */
    assert(csMng != 0);
    assert(protocol != NULL);

    /* Initialize the base and the members */
    result = ngiStreamManagerInitialize((ngiStreamManager_t *)csMng,
        &nglConversionMethodStreamManagerTypeInfomation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Stream Manager.\n",
             fName);
        return 0;
    }
    nglConversionMethodStreamManagerInitializeMember(csMng);

    csMng->ngcsm_protocol = protocol;
    csMng->ngcsm_sMng     = sMngArg;

    csMng->ngcsm_xdrSize = csMng->ngcsm_protocol->ngp_xdrDataSize.ngds_long;
    csMng->ngcsm_xdrConversionMethod = globus_libc_malloc(csMng->ngcsm_xdrSize * 3);
    if (csMng->ngcsm_xdrConversionMethod == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage.\n", fName);
        goto error;
    }

    /* Success */
    return 1;
error:
    result = ngiConversionMethodStreamManagerFinalize(csMng, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the ConversionMethod Stream Manager.\n", fName);
    }
    return 0;
}

static void
nglConversionMethodStreamManagerInitializeMember(
    ngiConversionMethodStreamManager_t *csMng)
{
    assert(csMng != NULL);

    nglConversionMethodStreamManagerInitializePointer(csMng);

    csMng->ngcsm_conversionMethod = NGI_BYTE_STREAM_CONVERSION_RAW;

    csMng->ngcsm_xdrSize        = 0;
    csMng->ngcsm_xdrWriteNbytes = 0;
    csMng->ngcsm_xdrReadNbytes  = 0;
}

static void
nglConversionMethodStreamManagerInitializePointer(
    ngiConversionMethodStreamManager_t *csMng)
{
    assert(csMng != NULL);

    csMng->ngcsm_xdrConversionMethod = NULL;
    csMng->ngcsm_sMng                = NULL;
}

int
ngiConversionMethodStreamManagerFinalize(
    ngiConversionMethodStreamManager_t *csMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiConversionMethodStreamManagerFinalize";
    assert(csMng != NULL);

    if (csMng->ngcsm_xdrConversionMethod != NULL) {
        globus_libc_free(csMng->ngcsm_xdrConversionMethod);
    }

    /* Initialize the members */
    nglConversionMethodStreamManagerInitializeMember(csMng);

    result = ngiStreamManagerFinalize((ngiStreamManager_t *)csMng, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the Stream Manager.\n", fName);
        return 0;
    }

    return 1;
}

static int
nglConversionMethodStreamManagerGetWritableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiConversionMethodStreamManager_t *csMng;
    size_t writableNbytes;
    int result;
    static const char fName[] = "nglConversionMethodStreamManagerGetWritableBuffer";

    assert(nBytes != NULL);
    assert(buf != NULL);
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglConversionMethodStreamManagerTypeInfomation);

    csMng = (ngiConversionMethodStreamManager_t *)sMng;

    if (csMng->ngcsm_xdrSize > csMng->ngcsm_xdrWriteNbytes) {
        /* Conversion Method */
        assert(csMng->ngcsm_xdrSize >= csMng->ngcsm_xdrWriteNbytes);
        writableNbytes = csMng->ngcsm_xdrSize - csMng->ngcsm_xdrWriteNbytes;

        if (writableNbytes < nBytesRequired) {
            NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Underflow buffer.\n", fName);
            return  0;
        }

        /* Conversion Method */
        *buf = &((u_char *)csMng->ngcsm_xdrConversionMethod)[csMng->ngcsm_xdrWriteNbytes];
        *nBytes = writableNbytes;
    } else {
        result = ngiStreamManagerGetWritableBuffer(csMng->ngcsm_sMng, buf, nBytesRequired, nBytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get writable buffer from the Stream Buffer.\n", fName);
            return  0;
        }
    }

    return 1;
}

static int
nglConversionMethodStreamManagerWriteBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiConversionMethodStreamManager_t *csMng;
    size_t writableNbytes;
    NET_Communicator netComm;
    int netCommInitialized = 0;
    int result;
    static const char fName[] = "nglConversionMethodStreamManagerWriteBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglConversionMethodStreamManagerTypeInfomation);

    csMng = (ngiConversionMethodStreamManager_t *)sMng;

    if (csMng->ngcsm_xdrWriteNbytes < csMng->ngcsm_xdrSize) {
        /* Conversion Method */
        assert(csMng->ngcsm_xdrSize >= csMng->ngcsm_xdrWriteNbytes);
        writableNbytes = csMng->ngcsm_xdrSize - csMng->ngcsm_xdrWriteNbytes;

        if (writableNbytes < nBytes) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Overflow buffer.\n", fName);
            goto error;
        }

        csMng->ngcsm_xdrWriteNbytes += nBytes;
        if ((csMng->ngcsm_xdrSize == csMng->ngcsm_xdrWriteNbytes) &&
            (csMng->ngcsm_xdrSize == csMng->ngcsm_protocol->ngp_xdrDataSize.ngds_long)) {
            /* Initialize the NET Communicator */
            result = ngiNetCommunicatorInitialize(&netComm, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't initialize the NET Communicator.\n", fName);
                goto error;
            }
            netCommInitialized = 1;

            /* Decode from XDR to native */
            result = ngiNetCommunicatorReadArray(
                &netComm, NG_ARGUMENT_DATA_TYPE_LONG,
                csMng->ngcsm_xdrConversionMethod,
                csMng->ngcsm_xdrSize,
                &csMng->ngcsm_conversionMethod,
                1, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't read the data from Net Communicator.\n",
                    fName);
                goto error;
            }

            /* Finalize the NET Communicator */
            netCommInitialized = 0;
            result = ngiNetCommunicatorFinalize(&netComm, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't finalize the NET Communicator.\n", fName);
                goto error;
            }

            switch(csMng->ngcsm_conversionMethod) {
            case NGI_BYTE_STREAM_CONVERSION_DIVIDE:
                csMng->ngcsm_xdrSize = csMng->ngcsm_protocol->ngp_xdrDataSize.ngds_long * 3;
                break;
            case NGI_BYTE_STREAM_CONVERSION_RAW:
            case NGI_BYTE_STREAM_CONVERSION_ZLIB:
                csMng->ngcsm_xdrSize = csMng->ngcsm_protocol->ngp_xdrDataSize.ngds_long * 2;
                break;
            default:
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't invalid conversion method.\n", fName);
                goto error;
                break;
            }
        }
    } else {
        result = ngiStreamManagerWriteBuffer(csMng->ngcsm_sMng, nBytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get write buffer to the Stream Buffer.\n", fName);
            goto error;
        }
    }

    return 1;
error:

    if (netCommInitialized != 0) {
        /* Finalize the NET Communicator */
        netCommInitialized = 0;
        result = ngiNetCommunicatorFinalize(&netComm, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        }
    }
    return 0;
}

static int
nglConversionMethodStreamManagerGetReadableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    ngiConversionMethodStreamManager_t *csMng;
    size_t readableNbytes;
    int result;
    static const char fName[] = "nglConversionMethodStreamManagerGetReadableBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglConversionMethodStreamManagerTypeInfomation);

    csMng = (ngiConversionMethodStreamManager_t *)sMng;

    if ((csMng->ngcsm_xdrWriteNbytes < csMng->ngcsm_xdrSize) ||
        (csMng->ngcsm_xdrReadNbytes  < csMng->ngcsm_xdrWriteNbytes)) {

        /* Conversion Method */
        assert(csMng->ngcsm_xdrSize >= csMng->ngcsm_xdrWriteNbytes);
        readableNbytes = csMng->ngcsm_xdrWriteNbytes - csMng->ngcsm_xdrReadNbytes;

        if (readableNbytes < nBytesRequired) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Overflow buffer.\n", fName);
            goto error;
        }

        *buf    = &((u_char*)csMng->ngcsm_xdrConversionMethod)[csMng->ngcsm_xdrReadNbytes];
        *nBytes = readableNbytes;
    } else {
        result = ngiStreamManagerGetReadableBuffer(csMng->ngcsm_sMng, buf, nBytesRequired, nBytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get readable buffer to the Stream Buffer.\n", fName);
            goto error;
        }
    }
    return 1;
error:
    return 0;
}

static int
nglConversionMethodStreamManagerReadBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    ngiConversionMethodStreamManager_t *csMng;
    size_t readableNbytes;
    int result;
    static const char fName[] = "nglConversionMethodStreamManagerReadBuffer";

    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglConversionMethodStreamManagerTypeInfomation);
    
    csMng = (ngiConversionMethodStreamManager_t *)sMng;

    if ((csMng->ngcsm_xdrWriteNbytes < csMng->ngcsm_xdrSize) ||
        (csMng->ngcsm_xdrReadNbytes  < csMng->ngcsm_xdrWriteNbytes)) {

        assert(csMng->ngcsm_xdrSize >= csMng->ngcsm_xdrWriteNbytes);
        readableNbytes = csMng->ngcsm_xdrWriteNbytes - csMng->ngcsm_xdrReadNbytes;

        if (readableNbytes < nBytes) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Overflow buffer.\n", fName);
            goto error;
        }

        csMng->ngcsm_xdrReadNbytes += nBytes;
    } else {
        result = ngiStreamManagerReadBuffer(csMng->ngcsm_sMng, nBytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get read buffer to the Stream Buffer.\n", fName);
            goto error;
        }
    }

    return 1;
error:
    return 0;
}

static int
nglConversionMethodStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    int *isTooLarge,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    /* Check argument */
    assert(nBytes != NULL);
    assert(isTooLarge!= NULL);

    *nBytes = 0;
    *isTooLarge = 0;

    return 1;
}

static int
nglConversionMethodStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiConversionMethodStreamManager_t *csMng;
    static const char fName[] = "nglConversionMethodStreamManagerDestruct";

    /* Check the arguments */
    assert(sMng != NULL);
    assert(sMng->ngsm_typeInfomation == &nglConversionMethodStreamManagerTypeInfomation);

    csMng = (ngiConversionMethodStreamManager_t *)sMng;

    /* Finalize */
    result = ngiConversionMethodStreamManagerFinalize(csMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stream Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiConversionMethodStreamManagerFree(csMng, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Not implement
 */ 
static int
nglNonImplementationStreamManagerDestroyWriteData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

static int
nglNonImplementationStreamManagerDestroyReadData(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

static int
nglNonImplementationStreamManagerGetWritableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

static int
nglNonImplementationStreamManagerWriteBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

static int
nglNonImplementationStreamManagerGetReadableBuffer(
    ngiStreamManager_t *sMng,
    void **buf,
    size_t nBytesRequired,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

static int
nglNonImplementationStreamManagerReadBuffer(
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

static int
nglNonImplementationStreamManagerWriteDirectly(
    ngiStreamManager_t *sMng,
    void *buf,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

#if 0
static int
nglNonImplementationStreamManagerGetBytesOfReadableData(
    ngiStreamManager_t *sMng,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}

static int
nglNonImplementationStreamManagerDestruct(
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    assert(0);
    return 0;
}
#endif
