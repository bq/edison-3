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

#ifndef TLCAES_H_
#define TLCAES_H_

#include "MobiCoreDriverApi.h"

mcResult_t tlcOpen(uint8_t* pTAData,  uint32_t nTASize);

mcResult_t aes(uint8_t *srcbuf, int srcbuflen, uint8_t *dstbuf, int dstbuflen);
mcResult_t secureObject(uint8_t *srcbuf, int srcbuflen, uint8_t *dstbuf, int dstbuflen);

void tlcClose(void);

#endif // TLCAES_H_
