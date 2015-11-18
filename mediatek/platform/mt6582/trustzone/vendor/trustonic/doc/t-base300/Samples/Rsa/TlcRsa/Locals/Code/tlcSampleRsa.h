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

#ifndef TLCRSA_H_
#define TLCRSA_H_

#include "MobiCoreDriverApi.h"

mcResult_t tlcOpen(uint8_t* pTAData,  uint32_t nTASize);

mcResult_t rsa(uint8_t *dstbuf, int *dstbuflen);

void tlcClose(void);

#endif // TLCRSA_H_
