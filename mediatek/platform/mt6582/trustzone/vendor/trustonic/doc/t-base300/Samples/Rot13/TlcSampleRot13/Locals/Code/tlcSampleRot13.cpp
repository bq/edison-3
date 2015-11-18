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

#include <stdlib.h>
#include <string.h>

#include "MobiCoreDriverApi.h"
#include "tlSampleRot13_Api.h"
#include "tlcSampleRot13.h"
#include "log.h"

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
const mcUuid_t uuid = TL_SAMPLE_ROT13_UUID;
tciMessage_t *tci;
mcSessionHandle_t sessionHandle;

// -------------------------------------------------------------
// NWd Rot13 oracle. Used to verify SWd Trusted Application result.
char* tlcRot13NWd(const char* plainText) {
    int len = strlen(plainText);
    int alphabetLen = 'Z' - 'A';
    char* expectedCipherText = (char*)malloc(len + 1);
    int i;

    for(i = 0; i < len; i++) {
        int c = *plainText++;
        int d = c;
        if('a' <= c && c <= 'z') {
            d = c + 13;
            if(d > 'z') {
                d = d - alphabetLen - 1;
            }
        } else if('A' <= c && c <= 'Z') {
            d = c + 13;
            if (d > 'Z') {
                d = d - alphabetLen - 1;
            }
        }
        expectedCipherText[i] = d;
    }

    expectedCipherText[i] = '\0';
    return expectedCipherText;
}

// -------------------------------------------------------------
mcResult_t tlcOpen(mcSpid_t spid, uint8_t* pTAData,  uint32_t nTASize)
{
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;

    LOG_I("Opening <t-base device.");
    mcRet = mcOpenDevice(DEVICE_ID);
    if(MC_DRV_OK != mcRet) {
        LOG_E("Error opening device: %d.", mcRet);
        return mcRet;
    }

    mcRet = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if(MC_DRV_OK != mcRet) {
        LOG_E("mcGetMobiCoreVersion failed %d.", mcRet);
        mcCloseDevice(DEVICE_ID);
        return mcRet;
    }
    LOG_I("productId        = %s",     versionInfo.productId);
    LOG_I("versionMci       = 0x%08X", versionInfo.versionMci);
    LOG_I("versionSo        = 0x%08X", versionInfo.versionSo);
    LOG_I("versionMclf      = 0x%08X", versionInfo.versionMclf);
    LOG_I("versionContainer = 0x%08X", versionInfo.versionContainer);
    LOG_I("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    LOG_I("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    LOG_I("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    LOG_I("versionCmp       = 0x%08X", versionInfo.versionCmp);

    tci = (tciMessage_t*)malloc(sizeof(tciMessage_t));
    if(tci == NULL) {
        LOG_E("Allocation of TCI failed.");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(tci, 0, sizeof(tciMessage_t));

    LOG_I("Opening the session.");
    memset(&sessionHandle, 0, sizeof(mcSessionHandle_t));
    // The device ID (default device is used).
    sessionHandle.deviceId = DEVICE_ID;
    LOG_I_BUF("tlcSampleRot13 Trusted Application:", pTAData, nTASize);
    mcRet = mcOpenTrustlet(
            &sessionHandle,
            spid,
            pTAData,
            nTASize,
            (uint8_t*)tci,
            sizeof(tciMessage_t));

    if(MC_DRV_OK != mcRet) {
        LOG_E("Open session failed: %d.", mcRet);
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
    } else {
        LOG_I("open() succeeded.");
    }

    return mcRet;
}

// -------------------------------------------------------------
char* tlcRot13SWd(const char* plainText) {
    mcResult_t ret;
    char* cipher = NULL;

    LOG_I("rot13() called with plainText: %s.", plainText);

    if(NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting.");
        return NULL;
    }

    do {
        // Prepare command message in TCI.
        tci->cmdRot13.header.commandId = CMD_SAMPLE_ROT13_CIPHER;
        tci->cmdRot13.len = strlen(plainText);

        // Copy data to TCI buffer.
        strcpy((char*) tci->data, plainText);
        LOG_I("Plain text for TA: %s.", tci->data);

        // Notify the TA.
        ret = mcNotify(&sessionHandle);
        if(MC_DRV_OK != ret) {
            LOG_E("Notify failed: %d.", ret);
            break;
        }

        // Wait for the TA response.
        ret = mcWaitNotification(&sessionHandle, -1);
        if(MC_DRV_OK != ret) {
            LOG_E("Wait for response notification failed: %d.", ret);
            break;
        }

        // Verify that the TA sent a response.
        if((RSP_ID(CMD_SAMPLE_ROT13_CIPHER) != tci->response.responseId)) {
            LOG_E("TA did not send a response: %d.",
                    tci->response.responseId);
            break;
        }

        // Check the TA return code.
        if(RET_OK != tci->response.returnCode) {
            LOG_E("TA did not send a valid return code: %d.",
                    tci->response.returnCode);
            break;
        }

        // Read result from TCI buffer.
        cipher = (char*) tci->data;

        LOG_I("Cipher text from TA: %s.", cipher);

    } while (false);

    return cipher;
}

// -------------------------------------------------------------
char* tlcUnwrapDataSWd(uint8_t* pData,  uint32_t nDataSize) {
    mcResult_t ret;
    char* unwrap = NULL;

    LOG_I("unwrapData() called.");

    if(NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting.");
        return NULL;
    }

    do {
        // Prepare command message in TCI.
        tci->cmdRot13.header.commandId = CMD_SAMPLE_ROT13_UNWRAP;
        memcpy(&tci->data, pData, nDataSize);
        tci->cmdRot13.len = nDataSize;

        // Copy data to TCI buffer.
        memcpy(tci->data, pData, nDataSize);
        LOG_I_BUF("Data container to unwrap by the TA", tci->data,
                tci->cmdRot13.len);

        // Notify the TA.
        ret = mcNotify(&sessionHandle);
        if(MC_DRV_OK != ret) {
            LOG_E("Notify failed: %d.", ret);
            break;
        }

        // Wait for the TA response.
        ret = mcWaitNotification(&sessionHandle, -1);
        if(MC_DRV_OK != ret) {
            LOG_E("Wait for response notification failed: %d.", ret);
            break;
        }

        // Verify that the TA sent a response.
        if((RSP_ID(CMD_SAMPLE_ROT13_UNWRAP) != tci->response.responseId)) {
            LOG_E("TA did not send a response: %d.",
                    tci->response.responseId);
            break;
        }

        // Check the TA return code.
        if(RET_OK != tci->response.returnCode) {
            LOG_E("TA did not send a valid return code: %d.",
                    tci->response.returnCode);
            break;
        }

        // Read result from TCI buffer.
        unwrap = (char*) tci->data;
        unwrap[tci->rspRot13.len] = '\0';

        LOG_I("Unwrap data from TA: %s", unwrap);
    } while (false);

    return unwrap;
}

// -------------------------------------------------------------
void tlcClose(void)
{
    mcResult_t ret;

    LOG_I("Closing the session.");
    ret = mcCloseSession(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing session failed: %d.", ret);
        // Continue even in case of error.
    }

    LOG_I("Closing <t-base device.");
    ret = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing <t-base device failed: %d.", ret);
        // Continue even in case of error.
    }
    free(tci);
    tci = NULL;
}
