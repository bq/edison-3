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

#include "tlSampleSha256_Api.h"

DECLARE_TRUSTLET_MAIN_STACK(2048);

/**
 * Process a SHA256 command.
 * The command data will be checked for in
 *
 * @param tci pointer to TCI buffer
 * @return RET_OK if operation has been successfully completed.
 */
tciReturnCode_t processCmdSha256(tci_t* tci) {
    tciReturnCode_t  ret = RET_OK;
    tlApiCrSession_t crSession;

    cmdSha256_t *cmdSha256 = &(tci->message.cmdSha256);

    // Copy parameters
    uint8_t* src = cmdSha256->bulkBuffer + cmdSha256->srcOffs;
    uint32_t srcLen = cmdSha256->srcLen;
    uint8_t* dest = (uint8_t*) tci->message.rspSha256.hash;
    uint32_t destLen = 32;

    tlApiResult_t tlRet;

    tlDbgPrintf("Clear text data: %s\n", src);

    do {
        // Check parameters
        // Do not allow access to secure memory located in first 1MB.
        if (NULL == src || !tlApiIsNwdBufferValid(src, srcLen)) {
            tlDbgPrintf ("SAMPLE_SHA256: processCmdSha256(), RET_ERR_INVALID_BUFFER, exit\n");
            ret = RET_ERR_INVALID_BUFFER;
            break;
        }

        // Finally, do the SHA256
        tlRet = tlApiMessageDigestInit(&crSession, TLAPI_ALG_SHA256);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintf("SAMPLE_SHA256: processCmdSha256(), tlApiMessageDigestInit ret=0x%08X, exit\n", tlRet);
            ret = RET_ERR_ALGORITHM;
            break;
        }

        tlRet = tlApiMessageDigestDoFinal(crSession, src, srcLen, dest,
                (size_t*)&destLen);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintf("SAMPLE_SHA256: processCmdSha256(), tlApiMessageDigestDoFinal ret=0x%08X, exit\n", tlRet);
            ret = RET_ERR_ALGORITHM;
            break;
        }

        tci->message.rspSha256.hashLen = destLen;

    } while (false);

    return ret;
}

/**
 * Trustlet entry.
 */
_TLAPI_ENTRY void tlMain(
    const addr_t tciBuffer,
    const uint32_t tciBufferLen)
{
    tciReturnCode_t ret;
    tciCommandId_t commandId;

    dbgSN("SAMPLE_SHA256: running");

    //tlDbgPrintf("SAMPLE_SHA256: running\n");

    // Check if the size of the given TCI is sufficient
    if ((NULL == tciBuffer) || (sizeof(tci_t) > tciBufferLen)) {
        // TCI too small -> end Trustlet
        // tlDbgPrintf("SAMPLE_SHA256: invalid TCI (got 0x%08X, %d, need %d), exit\n", tciBuffer, tciBufferLen, sizeof(tci_t));
        tlDbgPrintf("SAMPLE_SHA256: invalid TCI\n");
        tlDbgPrintf("TCI buffer: %x\n", tciBuffer);
        tlDbgPrintf("TCI buffer length: %x\n", tciBufferLen);
        tlDbgPrintf("sizeof(tciMessage_t): %d\n", sizeof(tci_t));

        //tlDbgPrintf("SAMPLE_SHA256: ignoring TCI error.\n");
        tlApiExit(EXIT_ERROR);
    }

    tci_t* tci = (tci_t*) tciBuffer;

    // The Trustlet main loop running infinitely
    for (;;) {

        tlDbgPrintf("SAMPLE_SHA256: waiting for notification\n");

        // Wait for a notification to arrive (INFINITE timeout is recommended -> not polling!)
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        // Dereference commandId once for further usage
        commandId = tci->message.commandHeader.commandId;

        // Check if the message received is (still) a response
        if (!IS_CMD(commandId)) {
            // Tell the NWd a response is still pending (optional)
            tlApiNotify();
            continue;
        }

        // Process command message
        switch (commandId) {
        case CMD_SAMPLE_SHA256:
            // SHA256 call
            ret = processCmdSha256(tci);
            break;
        default:
            // Unknown command ID
            tlDbgPrintf("SAMPLE_SHA256: Unknown command ID, ignore\n");
            ret = RET_ERR_UNKNOWN_CMD;
            break;
        }

        // Set up response header -> mask response ID and set return code
        tci->message.responseHeader.responseId = RSP_ID(commandId);
        tci->message.responseHeader.returnCode = ret;

        tlDbgPrintf("SAMPLE_SHA256: notify TLC\n");

        // Notify back the TLC
        tlApiNotify();
    }
}

/** @} */
