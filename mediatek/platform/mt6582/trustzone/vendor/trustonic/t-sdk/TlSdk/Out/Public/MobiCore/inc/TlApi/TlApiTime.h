/** @addtogroup TLAPI_SYS
 * @{
 * The MobiCore system API interface provides system information and system functions to Trustlets.
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
#ifndef __TLAPITIME_H__
#define __TLAPITIME_H__

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

#if TBASE_API_LEVEL >= 3


//------------------------------------------------------------------------------
/** Real time sources in MobiCore */
typedef enum {
        TS_SOURCE_ILLEGAL       = 0,    /**< Illegal counter source value. */
        TS_SOURCE_SOFTCNT       = 1,    /**< monotonic counter that is reset upon power cycle. */
        TS_SOURCE_SECURE_RTC    = 2,    /**< Secure real time clock that uses underlying hardware clock. */
} tsSource_t;


/** Timestamp Counter.
 * The Timestamp Counter returns global relative Timestamp value of
 * the hardware platform.
 */
typedef uint64_t timestamp_t, *timestamp_ptr;


///** Retrieve a time stamp value.
// *
// * Returns a time stamp value. The time stamp value is granted to be monotonic depending on the
// * timestamp source it is based on. The implementation takes care that no time stamp overflow occurs.
// *
// * Two subsequent calls to the function return always two different time stamp values where the
// * second value is always bigger than the first one. If the first call to the function returns
// * val1 as time stamp value, second call returns val2. For the returned values the following rules apply:
// *
// * val2 >= (val1 + 1)
// *
// * When a time stamps source is not supported by the platform. The function returns E_TLAPI_NOT_IMPLEMENTED.
// *
// * @param ts [out] After successful execution the time stamp is returned in the reference variable.
// * @param source [in] Time stamp source
// *
// * @returns TLAPI_OK if operation was successful.
// * @returns E_TLAPI_NOT_IMPLEMENTED if source is not supported by the platform.
// * @returns E_TLAPI_UNMAPPED_BUFFER if one buffer is not entirely mapped in Trustlet address space.
// */
//_TLAPI_EXTERN_C tlApiResult_t tlApiGetTimeStamp(
//    timestamp_t *ts,
//    tsSource_t  source
//);



/** Get timestamp value from the secure world, unit is device dependent.
 *
 * @param pTimestamp                    pointer to timestamp_t counter value
 * @return TLAPI_OK                     if version has been set
 * @returns E_TLAPI_INVALID_PARAMETER   pTimestamp is NULL or pointer is invalid
 * @returns E_TLAPI_NOT_IMPLEMENTED     platform or port does not support secure timestamps
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiGetSecureTimestamp(timestamp_ptr pTimestamp);

#endif /* TBASE_API_LEVEL */

#endif // __TLAPITIME_H__

/** @} */
