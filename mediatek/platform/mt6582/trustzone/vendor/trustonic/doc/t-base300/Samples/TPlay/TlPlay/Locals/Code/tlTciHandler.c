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


#define IV_SIZE        16
#define KEY_SIZE        16
#define DATA_SIZE      1024
#define OUTPUT_ADDR    0xE0203000
#define TEST_MEM_ADDR  0x10000000


uint8_t sHandle = 0;
uint8_t aes_data[DATA_SIZE];

/* Test in sections F.1.1 and F.1.2 */
uint8_t aesApiKey[KEY_SIZE] __attribute__((aligned(0x4))) = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
};


uint8_t aesApiIv[IV_SIZE] __attribute__((aligned(0x4))) = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
};

#define LOG_PREFFIX "[TPLAY  TA]"

#define RUNTEST(a, b, c) \
        dbgS(LOG_PREFFIX " Test Description : "); \
        dbgSN(a); \
        if (b == c) \
        dbgSN(LOG_PREFFIX " Test Result : PASS"); \
        else \
        dbgSN(LOG_PREFFIX " Test Result : FAIL");


static tciReturnCode_t convertDrmErrorToTciError(tlApiResult_t err) {
    tciReturnCode_t ret;

    switch (err) {
        case TLAPI_DRM_OK:
        case E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED:
            ret = RET_OK;
            break ;
        case E_TLAPI_DRM_INVALID_COMMAND:
            ret = RET_ERR_UNKNOWN_CMD;
            break ;
        case E_TLAPI_DRM_ALGORITHM_NOT_SUPPORTED:
            ret = RET_ERR_NOT_SUPPORTED;
            break ;
        default:
            ret = RET_ERR_INTERNAL_ERROR;
    }
    return ret;
}

static tlApiResult_t testAESDecryptionCtr128(
        uint8_t *pAesData,
        uint32_t dataSize,
        uint8_t *pIv,
        uint32_t ivSize) {

    uint8_t i;
    tlApiDrmInputSegmentDescriptor_t inputDesc;
    tlApiResult_t ret;

    /*
     * SAMPLE: Here the frames are setup in TA but could be passed
     * from TLC in Nwd.
     * For Testing purpose we set up 2 frames.
     */
    inputDesc.nTotalSize = dataSize;
    inputDesc.nNumBlocks = 2;
    i = dataSize < 16 ? dataSize : 16;
    inputDesc.aPairs[0].nSize = i;
    inputDesc.aPairs[0].nOffset = 0;
    inputDesc.aPairs[1].nSize = i;
    inputDesc.aPairs[1].nOffset = dataSize - i;
    for(i = 2; i < TLAPI_DRM_INPUT_PAIR_NUMBER; i++) {
        inputDesc.aPairs[i].nSize = 0;
        inputDesc.aPairs[i].nOffset = 0;
    }

    tlApiDrmDecryptContext_t decryptCtx;
    decryptCtx.key = &aesApiKey[0];
    decryptCtx.keylen = TLAPI_DRM_KEY_SIZE_128;
    decryptCtx.iv = pIv;
    decryptCtx.ivlen = ivSize;
    decryptCtx.alg = TLAPI_DRM_ALG_AES_CTR128;
    decryptCtx.outputoffet = 0;

    RUNTEST(
            "AES CTR 128 decryption",
            E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED,
            (ret = tlApiDrmProcessContent(
                    sHandle,
                    decryptCtx,
                    pAesData,
                    inputDesc,
                    TLAPI_DRM_PROCESS_ENCRYPTED_DATA,
                    NULL))
    );
    return ret;
}


static tlApiResult_t testProcessDrmContent(tci_t *pTci) {
    uint8_t *pBuf = (uint8_t*)pTci->message.command.data;
    uint32_t size;
    uint8_t *pIv;

    if (pBuf == 0) {
        pBuf = &aes_data[0];
        pIv = &aesApiIv[0];
        size = DATA_SIZE;
        /* Fill memory buffer */
        memset(pBuf, 0x12, size);
    } else {
        pIv = pBuf;
        pBuf += IV_SIZE;
        size = pTci->message.command.len - IV_SIZE;
    }

    // Prepare marshal and call driver
    return testAESDecryptionCtr128(pBuf, size, pIv, IV_SIZE);
}


static tlApiResult_t testOpenSession() {
    tlApiResult_t ret;

    RUNTEST(
            "Open Session",
            E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED,
            (ret = tlApiDrmOpenSession(&sHandle))
    );

    return ret;
}


static int32_t testCloseSession() {
    tlApiResult_t ret;

    RUNTEST(
            "Close Session",
            E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED,
            (ret = tlApiDrmCloseSession(sHandle))
    );

    return ret;
}


static tlApiResult_t testCheckLink() {
    tlApiDrmLink_t link = TLAPI_DRM_LINK_HDCP_1;
    tlApiResult_t ret;

    RUNTEST(
            "Check Link",
            E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED,
            (ret = tlApiDrmCheckLink(sHandle,link))
    );

    return ret;
}


static tciReturnCode_t runOpenSession() {
    tlApiResult_t ret;

    ret = testOpenSession();
    return convertDrmErrorToTciError(ret);
}


static tciReturnCode_t runCloseSession() {
    tlApiResult_t ret;

    ret = testCloseSession();
    return convertDrmErrorToTciError(ret);
}

static tciReturnCode_t runProcessDrmContent(tci_t *pTci) {
    tlApiResult_t ret;

    ret = testProcessDrmContent(pTci);
    return convertDrmErrorToTciError(ret);
}


static tciReturnCode_t runCheckLink() {
    tlApiResult_t ret;

    ret = testCheckLink();
    return convertDrmErrorToTciError(ret);
}


/**
 * TCI handler loop
 */
_NORETURN void tlTciHandler_Loop(
        const addr_t tciBuffer,
        const uint32_t tciBufferLen) {
    tciReturnCode_t  ret = RET_OK;
    tciCommandId_t   commandId;
    tci_t* pTci = (tci_t*) tciBuffer;

    /* The Trusted Application main loop */
    for (;;) {

        /* Wait for notification infinitely */
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        /* Retrieve command id */
        commandId = pTci->message.command.header.commandId;

        /* Process command message */
        switch (commandId) {
            case CMD_ID_RUN_DRM_TEST_OPEN_SESSION:
                dbgSN(LOG_PREFFIX " Running the Secure Playback driver tests: Open Session");
                ret = runOpenSession();
                break;

            case CMD_ID_RUN_DRM_TEST_PROCESS_DRM_CONTENT:
                dbgSN(LOG_PREFFIX " Running the Secure Playback driver tests: Process Drm Content");
                ret = runProcessDrmContent(pTci);
                break;

            case CMD_ID_RUN_DRM_TEST_CLOSE_SESSION:
                dbgSN(LOG_PREFFIX " Running the Secure Playback driver tests: Close Session");
                ret = runCloseSession();
                break;

            case CMD_ID_RUN_DRM_TEST_CHECK_LINK:
                dbgSN(LOG_PREFFIX " Running the Secure Playback driver tests: Check Link");
                ret = runCheckLink();
                break;

            default:
                /* Unknown command */
                dbgSN(LOG_PREFFIX " Running the Secure Playback driver tests: Unknown command");
                ret = RET_ERR_UNKNOWN_CMD;
                break;
        }

        /* Set response header */
        pTci->message.response.header.responseId = RSP_ID(commandId);
        pTci->message.response.header.returnCode = ret;

        /* Notify the TLC */
        tlApiNotify();
    }
}


