/**
 * @addtogroup TLAPI_MEM
 * @{
 * The MobiCore OS provides Trustlets with access to Dynamic allocation.
 * Communication is based on world-shared memory buffers and notifications without message payload.
 * <BR>
 * In MobiCore OS, message-formatting is application specific.
 * Messages are interchanged on world shared memory buffers that the
 * TrustletConnector (TLC) specifies in mcOpenSession() and mcMap().
 * The MobiCore driver and MobiCore OS set up this shared memory buffer
 * between TLC and Trustlet and forward notifications between the two.
 *
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#ifndef __TLAPIHEAP_H__
#define __TLAPIHEAP_H__

#if TBASE_API_LEVEL >= 3

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

/**
 * Allocates a block of memory from the heap.
 * The address of the allocated block is aligned on a 8-bytes boundary. A block
 * allocated by tlApiMalloc must be freed by tlApiFree.
 * If the size of the space requested is zero, the value returned is still a
 * non-NULL pointer that the Trusted Application must not attempt to access.
 *
 * @param   size: [in] the number of bytes to be allocated.
 *
 * @param   hint: [in] must be 0
 *
 * @return  Upon successful completion, with size not equal to zero, the
 *          function returns a pointer to the allocated space.
 *          Otherwise, a NULL pointer is returned.
 **/
_TLAPI_EXTERN_C void* tlApiMalloc(uint32_t size, uint32_t hint);

/**
 * Reallocates a block of memory from a heap.
 * This function allows resizing a memory block.
 * If buffer is NULL, tlApiRealloc is equivalent to tlApiMalloc.
 * If buffer is not NULL and newSize is 0, then tlApiRealloc is equivalent to
 * tlApiFree and returns a non-NULL pointer that the Trusted Application must
 * not attempt to access.
 * If newSize is less or equal to the current size of the block, the block is
 * truncated, the content of the block is left unchanged and the function
 * returns buffer.
 * If newSize is greater than the current size of the block, the size of the
 * block is increased. The whole content of the block is copied at the beginning
 * of the new block. If possible, the block is enlarged in place and the
 * function returns buffer. If this is not possible, a new block is allocated
 * with the new size, the content of the current block is copied, the current
 * block is freed and the function returns the pointer on the new block.
 *
 * @param   buffer: [in] Pointer to the block of memory that the function
 *                       reallocates.
 *                       This value may be null or returned by an earlier call
 *                       to tlApiMalloc or tlApiRealloc.
 *
 * @param   newSize: [in] size of the memory block in bytes.
 *                        This value may be zero.
 *
 * @return  A pointer to the reallocated memory block, a non-NULL pointer if the
 *          newSize is zero or NULL if an error is detected.
 **/
_TLAPI_EXTERN_C void* tlApiRealloc(void* buffer, uint32_t newSize);

/**
 * Frees a memory block allocated from a heap by tlApiMalloc or tlApiRealloc.
 * This function does nothing if buffer is NULL.
 *
 * @param   buffer: [in] Pointer to the block of memory to be freed.
 **/
_TLAPI_EXTERN_C void tlApiFree(void *buffer);

#endif /* TBASE_API_LEVEL */

#endif  /* !defined(__TLAPIHEAP_H__) */
/** @} */
