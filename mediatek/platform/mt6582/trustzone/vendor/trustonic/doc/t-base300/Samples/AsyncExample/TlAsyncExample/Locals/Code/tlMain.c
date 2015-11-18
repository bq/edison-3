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

/**
 * @file   tlMain.c
 * @brief  Implements the entry point of the trustlet
 *
 */

#include "tlStd.h"
#include "TlApi/TlApi.h"

#include "tlCommon.h"
#include "tlAsyncExample_Api.h"


/* Trustlet stack definition */
DECLARE_TRUSTLET_MAIN_STACK(16384);

/* External functions */
extern _NORETURN void tlTciHandler_Loop(
    const addr_t tciBuffer,
    const uint32_t tciBufferLen);

/**
 * Trustlet entry function
 */
_TLAPI_ENTRY void tlMain(
    const addr_t tciBuffer,
    const uint32_t tciBufferLen)
{

    tlApiLogPrintf("[<t Trustlet TlAsyncExample], Build " __DATE__ ", " __TIME__ EOL);

    /* Check TCI buffer and its length */
    if ((NULL == tciBuffer) || (sizeof(tci_t) > tciBufferLen))
    {
        /* TCI buffer is too small */
        tlDbgPrintLnf("[Trustlet TlAsyncExample] tlMain(): TCI buffer too small. Exiting..");
        tlApiExit(EXIT_ERROR);
    }

    /* TCI handler loop */
    tlTciHandler_Loop(tciBuffer, tciBufferLen);
}
