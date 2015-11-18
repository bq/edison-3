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
#include <memory.h>

#include "MobiCoreDriverApi.h"
#include "tlAes_Api.h"

#define LOG_TAG "TLC AES"
#include "log.h"

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
tciMessage_t *tci;
mcSessionHandle_t sessionHandle;

// -------------------------------------------------------------
static mcResult_t executeCmd(uint8_t *srcbuf, int srcbuflen, uint8_t *dstbuf, int *dstbuflen, tciCommandId_t cmd)
{
    mcResult_t  ret;
    mcBulkMap_t srcMapInfo;
    mcBulkMap_t dstMapInfo;

    if(NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting");
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    LOG_I("srcbuflen: %d", srcbuflen);
    LOG_I("dstbuflen: %d", *dstbuflen);

    tci->cmdAes.header.commandId = cmd;
    tci->cmdAes.len = srcbuflen;
    tci->cmdAes.respLen = *dstbuflen;

    LOG_I("Preparing command message in TCI");

    ret = mcMap(&sessionHandle, srcbuf, srcbuflen, &srcMapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("mcMap failed: %d", ret);
        return ret;
    }
    tci->plainData = (uint32_t)srcMapInfo.sVirtualAddr;

    ret = mcMap(&sessionHandle, dstbuf, *dstbuflen, &dstMapInfo);
    if (MC_DRV_OK != ret) {
        LOG_E("mcMap failed: %d", ret);
        mcUnmap(&sessionHandle, srcbuf, &srcMapInfo);
        return ret;
    }
    tci->cipherData = (uint32_t)dstMapInfo.sVirtualAddr;

    LOG_I("Notifying the Trusted Application");
    ret = mcNotify(&sessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Notify failed: %d", ret);
        goto exit;
    }

    LOG_I("Waiting for the Trusted Application response");
    ret = mcWaitNotification(&sessionHandle, -1);
    if (MC_DRV_OK != ret) {
        LOG_E("Wait for response notification failed: %d", ret);
        goto exit;
    }
    *dstbuflen = tci->rspAes.len;
    LOG_I("Response length=%d", *dstbuflen);

    LOG_I("Verifying that the Trusted Application sent a response.");
    if (RSP_ID(cmd) != tci->rspAes.header.responseId) {
        LOG_E("Trusted Application did not send a response: %d",
            tci->rspAes.header.responseId);
        ret = MC_DRV_ERR_INVALID_RESPONSE;
        goto exit;
    }

    if (RET_OK != tci->rspAes.header.returnCode) {
        LOG_E("Trusted Application did not send a valid return code: %d",
            tci->rspAes.header.returnCode);
        ret = tci->rspAes.header.returnCode;
    }

exit:
    mcUnmap(&sessionHandle, srcbuf, &srcMapInfo);
    mcUnmap(&sessionHandle, dstbuf, &dstMapInfo);

    return ret;
}

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
mcResult_t aes(uint8_t *srcbuf, int srcbuflen, uint8_t *dstbuf, int dstbuflen)
{
    mcResult_t  ret;
    uint8_t     *tmpData = NULL;

    tmpData = (uint8_t *)malloc(dstbuflen);
    if (tmpData == NULL) {
        LOG_E("Allocation of tmpData buffer failed");
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    ret = executeCmd(srcbuf, srcbuflen, tmpData, &dstbuflen, CMD_AES_ENCRYPT);
    if (ret != RET_OK) {
        LOG_E("Unable to execute CMD_AES_ENCRYPT command: %d", ret);
        goto exit;
    }

    ret = executeCmd(tmpData, dstbuflen, dstbuf, &dstbuflen, CMD_AES_DECRYPT);
    if (ret != RET_OK) {
        LOG_E("Unable to execute CMD_AES_DECRYPT command: %d", ret);
        goto exit;
    }

    if (0 == memcmp(srcbuf, dstbuf, srcbuflen)) {
      LOG_I("Data match");
    } else {
      LOG_E("Data does NOT match");
    }

exit:
    free(tmpData);
    return ret;
}


// -------------------------------------------------------------
mcResult_t secureObject(uint8_t *srcbuf, int srcbuflen, uint8_t *dstbuf, int dstbuflen)
{
    mcResult_t  ret;
    uint8_t     *tmpData = NULL;

    tmpData = (uint8_t *)malloc(dstbuflen);
    if (tmpData == NULL) {
        LOG_E("Allocation of tmpData buffer failed");
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    ret = executeCmd(srcbuf, srcbuflen, tmpData, &dstbuflen, CMD_AES_WRAP);
    if (ret != RET_OK) {
        LOG_E("Unable to execute CMD_AES_ENCRYPT command: %d", ret);
        goto exit;
    }

    ret = executeCmd(tmpData, dstbuflen, dstbuf, &dstbuflen, CMD_AES_UNWRAP);
    if (ret != RET_OK) {
        LOG_E("Unable to execute CMD_AES_DECRYPT command: %d", ret);
        goto exit;
    }

    if (0 == memcmp(srcbuf, dstbuf, srcbuflen)) {
      LOG_I("Data match");
    } else {
      LOG_E("Data does NOT match");
    }

exit:
    free(tmpData);
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
