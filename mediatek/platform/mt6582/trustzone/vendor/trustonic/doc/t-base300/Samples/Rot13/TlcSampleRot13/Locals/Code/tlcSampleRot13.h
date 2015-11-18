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

/*
 * tlcSampleRot13.h
 */

#ifndef TLC_SAMPLE_ROT13_H_
#define TLC_SAMPLE_ROT13_H_

#include "MobiCoreDriverApi.h"

#define LOG_TAG "tlcSampleRot13"

#define PLAINTEXT "The quick brown fox jumps over the lazy dog"

char* tlcRot13NWd(const char* plainText);

mcResult_t tlcOpen(mcSpid_t spid, uint8_t* pTAData, uint32_t nTASize);
char* tlcRot13SWd(const char* plainText);
char* tlcUnwrapDataSWd(uint8_t* pData, uint32_t nDataSize);
void tlcClose(void);

#endif // TLC_SAMPLE_ROT13_H_
