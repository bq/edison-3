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
#include "tlSampleSha256_Api.h"

#define LOG_TAG "TLC SAMPLE SHA256"
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
mcResult_t sha256(uint8_t *srcbuf, int srcbuflen, uint8_t *dstbuf, int dstbuflen)
{
    mcResult_t  ret;
    mcBulkMap_t mapInfo;

    if (dstbuflen != 32)
    {
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    ret = mcMap(&sessionHandle, srcbuf, sizeof(srcbuflen), &mapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("Error during memory registration in Kernel Module");
        return ret;
    }

    LOG_I("Buffer mapped");

    // -------------------------------------------------------------
    // Call the SHA256 Trusted Application
    // Prepare command message in TCI
    tci->cmdSha256.header.commandId = CMD_SAMPLE_SHA256;
    tci->cmdSha256.bulkBuffer = (uint8_t*) mapInfo.sVirtualAddr;
    tci->cmdSha256.srcOffs = 0;
    tci->cmdSha256.srcLen = srcbuflen;

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
    if ((RSP_ID(CMD_SAMPLE_SHA256) != tci->responseHeader.responseId)) {
        LOG_E("Trusted Application did not send a valid response! responseId: %d",
                tci->responseHeader.responseId);
        ret = MC_DRV_ERR_INVALID_RESPONSE;
        goto unmap;
	}

    // -------------------------------------------------------------
    // Check the Trusted Application return code
    if (RET_OK != tci->responseHeader.returnCode) {
        LOG_E("Trusted Application failed! Return code: %d",
                tci->responseHeader.returnCode);
        ret = tci->responseHeader.returnCode;
        goto unmap;
    }

    // -------------------------------------------------------------
    // Unregister memory from Trusted Application
    ret = mcUnmap(&sessionHandle, srcbuf, &mapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("Error during memory unmapping");
    }

    LOG_I("Output Buffer unmapped");

    if (tci->rspSha256.hashLen != 32)
    {
        return MC_DRV_ERR_INVALID_PARAMETER;
    }

    memcpy(dstbuf, tci->rspSha256.hash, dstbuflen);

    return ret;

unmap:
    mcUnmap(&sessionHandle, srcbuf, &mapInfo);
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
