/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __TLAPIROT13_H__
#define __TLAPIROT13_H__

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

/** Encode cleartext by calling Rot13 driver.
 *
 * @param length       Lentgh of the data to be encrypted.
 * @param cleartext    Pointer to the data to be encrypted.
 * @return TLAPI_OK if data has successfully been encrypted.
 */

_TLAPI_EXTERN_C tlApiResult_t tlApiEncodeRot13(
    uint32_t   length,
    uint8_t   *cleartext);

#endif // __TLAPIROT13_H__
