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

#include "tlStd.h"
#include "TlApi/TlApi.h"

#include "tlCommon.h"
#include "tci.h"
#include "tlDrm_Api.h"


/* Trusted Application stack definition */
DECLARE_TRUSTLET_MAIN_STACK(16384);

/* External functions */
extern _NORETURN void tlTciHandler_Loop(
                                    const addr_t tciBuffer,
                                    const uint32_t tciBufferLen);

/**
 * Trusted Application entry function
 */
_TLAPI_ENTRY void tlMain(
                    const addr_t tciBuffer,
                    const uint32_t tciBufferLen)
{

    /* Check TCI buffer and its length */
    if ((NULL == tciBuffer) || (sizeof(tci_t) > tciBufferLen))
    {
        /* TCI buffer is too small */
        dbgSN("[DRM Test Truslet] tlMain(): TCI buffer too small. Exiting..");
        tlApiExit(EXIT_ERROR);
    }

    /* TCI handler loop */
    tlTciHandler_Loop(tciBuffer, tciBufferLen);
}
