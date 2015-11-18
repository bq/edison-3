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
#include "tlSampleRot13_Api.h"

// Trusted Application (TA) tlSampleRot13.

// Reserve 4096 bytes for stack. This size is enough to cipher data and unwrap
// the maximum size of a Data container. However, it should provide some space
// for modification on the code and leave another 2048 byte for global
// variables. Note that the data/bss segment uses full 4KB pages, same for
// the code segment.
DECLARE_TRUSTLET_MAIN_STACK(4096);

// SWd buffer where crypto operation happens
uint8_t swdData[MAX_DATA_LEN];

/**
 * Process a ROT13 CIPHER command message.
 * The command data will be checked for in.
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t processCmdRot13Cipher(tciMessage_t* message) {
    // Check parameters.
    uint32_t len = message->cmdRot13.len;
    char *data = (char*)message->data;
    // Verify that NWd buffer is fully in NWd, and does not extend too far.
    if(!tlApiIsNwdBufferValid(data, len)) {
        tlApiLogPrintf("TA tlSampleRot13: Error, invalid cipher data.\n");
        return RET_ERR_ROT13_INVALID_LENGTH;
    }

    // We use the simply light weight debug output to create as mall TA.
    // tlDbgPrintf gives smallest size and best performance.
    tlDbgPrintf("TA tlSampleRot13: Clear text: %s.\n", data);
    tlDbgPrintf("TA tlSampleRot13: Length %d.\n", len);

    if(len > MAX_DATA_LEN) {
        tlApiLogPrintf("TA tlSampleRot13: Error, invalid cipher data length "
                "(> %d).\n", MAX_DATA_LEN);
        return RET_ERR_ROT13_INVALID_LENGTH;
    }

    // Copy plain data to SWd.
    memcpy(&swdData, data, len);

    // ROT13 encryption.
    uint32_t i;
    for(i = 0; i < len; i++) {
        // Get char from message buffer.
        char c = swdData[i];
        // Convert lower case letter to upper case.
        char upper = (char)(c & (char)32);
        c &= ~upper;
        // Do ROT13 on A-Z, leave any other bytes untouched.
        if((c >= 'A') && (c <= 'Z')) {
            // This implementation uses a division. However, ARM CPUs do
            // not support this natively, to the library has to provide
            // code for this. For the RVDS 4.1 library, the additional
            // div code increases the TA's code segment size by
            // about 396 byte.
            //    c = ((c - 'A' + 13) % 26 + 'A');
            // This implementation avoids a division.
            c += 13;
            if(c > 'Z') { c -= 26; }
        }
        // Restore lower case letter.
        c |= upper;
        // Write ROT13 encrypted char back to message buffer.
        swdData[i] = c;
    }
    // Copy encrypted data to NWd.
    memcpy(data, &swdData, len);

    tlDbgPrintf("TA tlSampleRot13: Cipher text: %s.\n", data);

    return RET_OK;
}

/**
 * Process a ROT13 UNWRAP command message.
 * The command data will be checked for in.
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t processCmdRot13Unwrap(tciMessage_t* message) {
    // Check parameters.
    uint32_t len = message->cmdRot13.len;
    char *data = (char*)message->data;
    // Verify that NWd buffer is fully in NWd, and does not extend too far.
    if(!tlApiIsNwdBufferValid(data, len)) {
        tlApiLogPrintf("TA tlSampleRot13: Error, invalid unwrap data.\n");
        return RET_ERR_ROT13_INVALID_LENGTH;
    }

    tlDbgPrintf("TA tlSampleRot13: Wrapped data (length %d):\n", len);
    for(uint32_t i = 0; i < len; i++) {
        tlDbgPrintf("%02x ", data[i]);
    }
    tlDbgPrintf("\n");

    if(len > MAX_DATA_LEN) {
        tlApiLogPrintf("TA tlSampleRot13: Error, invalid unwrap data length "
                "(> %d).\n", MAX_DATA_LEN);
        return RET_ERR_ROT13_INVALID_LENGTH;
    }

    // Copy data container to SWd.
    memcpy(&swdData, data, len);

    tciReturnCode_t ret = tlApiUnwrapObjectExt(&swdData, len,
            NULL, &len, TLAPI_UNWRAP_PERMIT_DELEGATED | TLAPI_UNWRAP_DEFAULT);
    if(TLAPI_OK != ret) {
        tlApiLogPrintf("TA tlSampleRot13: Error, unwrap data container failed "
                "with ret=0x%08X.\n", ret);
        return RET_ERR_ROT13_UNWRAP;
    }

    // Copy unwrapped data container to NWd.
    mcSoDataCont_t* soDataCont = (mcSoDataCont_t*)&swdData;
    memcpy(data, &soDataCont->cont.co, soDataCont->soHeader.encryptedLen);
    message->rspRot13.len = soDataCont->soHeader.encryptedLen;

    tlDbgPrintf("TA tlSampleRot13: Unwrapped data (length %d):\n",
            soDataCont->soHeader.encryptedLen);
    for(uint32_t i = 0; i < soDataCont->soHeader.encryptedLen; i++) {
        tlDbgPrintf("%02x ", data[i]);
    }
    tlDbgPrintf("\n");

    return RET_OK;
}

/**
 * TA entry.
 */
_TLAPI_ENTRY void tlMain(const addr_t tciBuffer, const uint32_t tciBufferLen) {
    tciReturnCode_t ret;
    tlApiLogPrintf("TA tlSampleRot13: Starting.\n");

    // Show tbase version.
    uint32_t tlApiVersion;
    mcVersionInfo_t versionInfo;

    tlApiLogPrintf("TA tlSampleRot13: tbase info.\n");

    ret = tlApiGetVersion(&tlApiVersion);
    if(TLAPI_OK != ret) {
        tlApiLogPrintf("TA tlSampleRot13: Error, tlApiGetVersion failed with "
                "ret=0x%08X, exit.\n", ret);
        tlApiExit(ret);
    }
    tlApiLogPrintf("tlApi version    =  0x%08X\n", tlApiVersion);

    ret = tlApiGetMobicoreVersion(&versionInfo);
    if(TLAPI_OK != ret) {
        tlApiLogPrintf("TA tlSampleRot13: Error, tlApiGetMobicoreVersion "
                "failed with ret=0x%08X, exit.\n", ret);
        tlApiExit(ret);
    }
    tlApiLogPrintf("productId        = %s\n", versionInfo.productId);
    tlApiLogPrintf("versionMci       = 0x%08X\n", versionInfo.versionMci);
    tlApiLogPrintf("versionSo        = 0x%08X\n", versionInfo.versionSo);
    tlApiLogPrintf("versionMclf      = 0x%08X\n", versionInfo.versionMclf);
    tlApiLogPrintf("versionContainer = 0x%08X\n", versionInfo.versionContainer);
    tlApiLogPrintf("versionMcConfig  = 0x%08X\n", versionInfo.versionMcConfig);
    tlApiLogPrintf("versionTlApi     = 0x%08X\n", versionInfo.versionTlApi);
    tlApiLogPrintf("versionDrApi     = 0x%08X\n", versionInfo.versionDrApi);
    tlApiLogPrintf("versionCmp       = 0x%08X\n", versionInfo.versionCmp);

    // Check if the size of the given TCI is sufficient.
    if((NULL == tciBuffer) || (sizeof(tciMessage_t) > tciBufferLen)) {
        tlApiLogPrintf("TA tlSampleRot13: Error, invalid TCI size.\n");
        tlApiLogPrintf("TCI buffer: %x.\n", tciBuffer);
        tlApiLogPrintf("TCI buffer length: %d.\n", tciBufferLen);
        tlApiLogPrintf("sizeof(tciMessage_t): %d.\n", sizeof(tciMessage_t));
        tlApiExit(EXIT_ERROR);
    }

    tciMessage_t* message = (tciMessage_t*) tciBuffer;

    // TA main loop running infinitely.
    for(;;) {
        // Wait for a notification to arrive
        // (INFINITE timeout is recommended -> not polling!)
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        // Copy commandId in SWd.
        tciCommandId_t commandId = message->command.commandId;

        tlDbgPrintf("TA tlSampleRot13: Got a message!, commandId=%x.\n",
                commandId);

        // Check if the message received is (still) a response.
        if(!IS_CMD(commandId)) {
            // Tell the NWd a response is still pending (optional).
            tlApiNotify();
            continue;
        }

        // Process command message.
        switch(commandId) {
            //-----------------------------------------------
            case CMD_SAMPLE_ROT13_CIPHER:
                // Rot13 cipher call.
                ret = processCmdRot13Cipher(message);
                break;
                //-----------------------------------------------
            case CMD_SAMPLE_ROT13_UNWRAP:
                // Rot13 unwrap call.
                ret = processCmdRot13Unwrap(message);
                break;
                //-----------------------------------------------
            default:
                // Unknown commandId.
                tlApiLogPrintf("TA tlSampleRot13: Error, unknow commandId=%x.\n");
                dbgBlob("TCI:", tciBuffer, 32);
                ret = RET_ERR_UNKNOWN_CMD;
                break;
        } // end switch (commandId).

        // Set up response header -> mask response ID and set return code.
        message->response.responseId = RSP_ID(commandId);
        message->response.returnCode = ret;

        tlDbgPrintf("TA tlSampleRot13: returning %d.\n",ret);
        // Notify back the TLC
        tlApiNotify();
    }
}
