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
#include "tlRsa_Api.h"

#define LOG_TAG "TLC RSA"
#include "log.h"

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
tciMessage_t *tci;
mcSessionHandle_t sessionHandle;

// -------------------------------------------------------------
mcResult_t tlcOpen(uint8_t* pTAData,  uint32_t nTASize)
{
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;

    LOG_I("Opening <t-base device");
    mcRet = mcOpenDevice(DEVICE_ID);
    if (MC_DRV_OK != mcRet) {
        LOG_E("Error opening device: %d", mcRet);
        return mcRet;
    }

    mcRet = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if (MC_DRV_OK != mcRet) {
        LOG_E("mcGetMobiCoreVersion failed %d", mcRet);
        mcCloseDevice(DEVICE_ID);
        return mcRet;
    }
    LOG_I("productId        = %s", versionInfo.productId);
    LOG_I("versionMci       = 0x%08X", versionInfo.versionMci);
    LOG_I("versionSo        = 0x%08X", versionInfo.versionSo);
    LOG_I("versionMclf      = 0x%08X", versionInfo.versionMclf);
    LOG_I("versionContainer = 0x%08X", versionInfo.versionContainer);
    LOG_I("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    LOG_I("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    LOG_I("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    LOG_I("versionCmp       = 0x%08X", versionInfo.versionCmp);

    tci = (tciMessage_t*)malloc(sizeof(tciMessage_t));
    if (tci == NULL) {
        LOG_E("Allocation of TCI failed");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(tci, 0, sizeof(tciMessage_t));

    LOG_I("Opening the session");
    memset(&sessionHandle, 0, sizeof(sessionHandle));
    sessionHandle.deviceId = DEVICE_ID; // The device ID (default device is used)
    mcRet = mcOpenTrustlet(
            &sessionHandle,
            MC_SPID_TRUSTONIC_TEST, /* mcSpid_t */
            pTAData,
            nTASize,
            (uint8_t *) tci,
            sizeof(tciMessage_t));
    if (MC_DRV_OK != mcRet) {
        LOG_E("Open session failed: %d", mcRet);
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
    }
    else {
        LOG_I("open() succeeded");
    }

    return mcRet;
}

// -------------------------------------------------------------
mcResult_t rsa(uint8_t *dstbuf, int *dstbuflen)
{
    mcResult_t ret;
    mcBulkMap_t mapInfo;

    // -------------------------------------------------------------
    // Map additional memory to Trusted Application (session)

    ret = mcMap(&sessionHandle, dstbuf, *dstbuflen, &mapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("mcMap failed: Error during memory registration in Kernel Module, %d", ret);
        return ret;
    }

    LOG_I("Output Buffer mapped");

    // -------------------------------------------------------------
    // Call the RSA Trusted Application
    //

    // Prepare command message in TCI: CMD_RSA
    tci->cmdRsa.header.commandId = CMD_RSA;
    tci->outputData = (uint32_t)mapInfo.sVirtualAddr;
    tci->cmdRsa.len = tci->rspRsa.len = *dstbuflen;

    // -------------------------------------------------------------
    // Signal the Trusted Application
    ret = mcNotify(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Notify failed: %d", ret);
        goto unmap;
    }

    LOG_I("Trusted Application notified");

    // -------------------------------------------------------------
    // Wait for the Trusted Application response
    ret = mcWaitNotification(&sessionHandle, -1);
    if (MC_DRV_OK != ret) {
        LOG_E("Wait for response notification failed: %d", ret);
        goto unmap;
    }

    LOG_I("Trusted Application answered");

    // -------------------------------------------------------------
    // Verify that the Trusted Application sent a response
    LOG_I("Verifying that the Trusted Application sent a response.\n");
    if ( RSP_ID(CMD_RSA) != tci->rspRsa.header.responseId) {
        LOG_E("Trusted Application did not send a valid response! responseId: %d",
        tci->rspRsa.header.responseId);
        ret = MC_DRV_ERR_INVALID_RESPONSE;
        goto unmap;
    }

    if (RET_RSA_OK != tci->rspRsa.header.returnCode) {
        LOG_E("Trusted Application did not send a valid return code: %d",
            tci->rspRsa.header.returnCode);
        ret = MC_DRV_ERR_INVALID_RESPONSE;
        goto unmap;
    }

    // -------------------------------------------------------------
    // Unregister memory from Trusted Application
    ret = mcUnmap(&sessionHandle, dstbuf, &mapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("Error during memory unmapping");
    }

    LOG_I("Output Buffer unmapped");

    *dstbuflen = tci->rspRsa.len;

    return ret;

unmap:
    mcUnmap(&sessionHandle, dstbuf, &mapInfo);
    return ret;
}

// -------------------------------------------------------------
void tlcClose(void)
{
    mcResult_t ret;

    LOG_I("Closing the session");
    ret = mcCloseSession(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing session failed: %d", ret);
        /* continue even in case of error */
    }

    LOG_I("Closing <t-base device");
    ret = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing <t-base device failed: %d", ret);
        /* continue even in case of error */;
    }
    free(tci);
    tci = NULL;
}
