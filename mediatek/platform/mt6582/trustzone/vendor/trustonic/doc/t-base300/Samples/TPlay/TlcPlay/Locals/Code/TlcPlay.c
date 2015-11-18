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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* t-base includes */
#include "MobiCoreDriverApi.h"

/* Tplay driver includes */
#include "tlTplayDci.h"

/* Tplay TA includes */
#include "tci.h"
#include "tlDrm_Api.h"

/* log */
#define LOG_TAG "TLC SAMPLE TPLAY"
#include "log.h"


/* utilities */
static size_t tlcGetFileContent(const char* pPath, uint8_t** ppContent);
static mcResult_t tlcOpenSession(const char *name,
                                 uint32_t deviceId,
                                 mcSessionHandle_t *pSessionHandle,
                                 uint8_t **ppWsm,
                                 int32_t wsmSize);
static mcResult_t tlcCloseSession(const char *name,
                               uint32_t deviceId,
                               mcSessionHandle_t *pSessionHandle,
                               uint8_t **ppWsm);
static mcResult_t tlcNotifyAndWait(mcSessionHandle_t *pSessionHandle,
                                const char *name);
static void tlcInitCryptoBuffer(uint8_t* cryptoBuffer);

#define TRUSTLET_PATH "/data/app/mcRegistry"

#define CHECK_TCI_RES(name, pTci, exp, err)	{						        \
        LOG_I(name " => response_id=%x, result=%x", 				        \
                pTci->message.response.header.responseId,			        \
                pTci->message.response.header.returnCode);			        \
                if (pTci->message.response.header.returnCode != exp) {	 	\
                    LOG_E(name ": ERROR");				 			  	\
                    err++;												  	\
                }															\
}

#define KEY_SIZE			16
#define IV_SIZE				16
#define CRYPTO_DATA_SIZE 	1024
#define CRYPTO_BUFFER_SIZE	(CRYPTO_DATA_SIZE + IV_SIZE)

/**
 * SAMPLE: __FOR TESTING PURPOSE___
 * The encryption key is not known by TLC.
 * It is just put in the sample to "encrypt" data on the fly
 * then pass data to driver for decryption.
 */
uint8_t aesApiKey[KEY_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
        0x0d, 0x0e, 0x0f, 0x10 };

/**
 * SAMPLE:
 * The IV may be passed from Nwd to Swd
 */
uint8_t aesApiIv[IV_SIZE] = {
        0x51, 0xf9, 0x45, 0x5a, 0x91, 0x2d, 0xA1, 0x07, 0x33, 0x1a, 0x05, 0x1a,
        0x11, 0xe1, 0x6f, 0x21 };

/**
 * SAMPLE: used to fill cryptoBuffer
 */
uint8_t dataBuf[] = { 'T', 'l', 'c', 'P', 'l', 'a', 'y', 'T', 'c', 'i', 0 };


int main(int argc, char *args[]) {
    uint32_t deviceId = MC_DEVICE_ID_DEFAULT;
    dciMessage_t *pDci = NULL;
    tci_t *pTci = NULL;
    int8_t *pCryptoBuffer = NULL;
    mcSessionHandle_t drSessionHandle;
    mcSessionHandle_t tlSessionHandle;
    mcBulkMap_t mapInfo;
    mcResult_t mcRet;
    int err = 1;

    LOG_I("Copyright (c) Trustonic Limited 2013");
    do {
        //put wrong device id to remember that tl session is not openned
        tlSessionHandle.deviceId = deviceId + 1;
        mapInfo.sVirtualLen = 0;

        /* Open MobiCore device */
        mcRet = mcOpenDevice(deviceId);
        if (MC_DRV_OK != mcRet) {
            LOG_E("mcOpenDevice returned: %d", mcRet);
            exit(127);
        }

        LOG_I("mcOpenDevice returned: %d", mcRet);

        /*
         * Open DRIVER SESSION
         * We won't use DCI here, but we just open session to load driver in case it is not loaded.
         * If openSession gives error (14,15) we ignore it.
         */
        mcRet = tlcOpenSession("070b0000000000000000000000000000.tlbin",
                            deviceId,
                            &drSessionHandle,
                            (uint8_t**) &pDci,
                            sizeof(dciMessage_t));
        if (MC_DRV_ERR_SESSION_PENDING == mcRet
                || MC_DRV_ERR_DAEMON_UNREACHABLE == mcRet) {
            LOG_W("driver session already opened. Ignoring...");
        } else if (MC_DRV_OK != mcRet) {
            break;
        } else {
            /*
             * The Driver will send a DCI notification when initialized.
             */
            LOG_I("wait for notification from driver dci handler");
            if (MC_DRV_OK != mcWaitNotification(&drSessionHandle,
                                                MC_INFINITE_TIMEOUT)) {
                LOG_E("mcWaitNotification returned: %d", mcRet);
                break;
            }
        }

        /*
         * Open TA SESSION
         */
        mcRet = tlcOpenSession("07080000000000000000000000000000.tlbin",
                            deviceId,
                            &tlSessionHandle,
                            (uint8_t**) &pTci,
                            sizeof(tci_t));
        if (MC_DRV_OK != mcRet) {
            break;
        }

        /*
         * Allocate Buffer to exchange encrypted data and Map it to TA
         */
        if ((pCryptoBuffer = (int8_t*) malloc(CRYPTO_BUFFER_SIZE)) == NULL) {
            LOG_E("ERROR: Malloc cryptoBuffer");
            break;
        }
        if ((mcRet = mcMap(&tlSessionHandle,
                           pCryptoBuffer,
                           CRYPTO_BUFFER_SIZE,
                           &mapInfo)) != MC_DRV_OK) {
            LOG_E("mcMap(cryptoBuffer) returned: %d", mcRet);
            break;
        }
        LOG_I("mcMap(ptr=%p,sec_ptr=%p,size=%d) returned: %d",
              pCryptoBuffer,
              mapInfo.sVirtualAddr,
              mapInfo.sVirtualLen,
              mcRet);

        /* Fill crypto buffer with iv and data */
        tlcInitCryptoBuffer(pCryptoBuffer);

        /*
         * Communicate with TA via TCI
         * ---------------------------- */

        /*
         * Prepare TCI: openSession
         * */
        pTci->message.command.header.commandId
                = CMD_ID_RUN_DRM_TEST_OPEN_SESSION;

        /* notify, check result */
        mcRet = tlcNotifyAndWait(&tlSessionHandle, "drm OpenSession");
        if (MC_DRV_OK != mcRet) {
            break;
        }
        CHECK_TCI_RES("drm OpenSession", pTci, RET_OK, err);

        /*
         * Prepare TCI: checkLink
         */
        pTci->message.command.header.commandId = CMD_ID_RUN_DRM_TEST_CHECK_LINK;

        /* notify, check result */
        mcRet = tlcNotifyAndWait(&tlSessionHandle, "drm CheckLink");
        if (MC_DRV_OK != mcRet) {
            break;
        }
        CHECK_TCI_RES("drm CheckLink", pTci, RET_OK, err);

        /*
         * Prepare TCI: processDrmContent
         */
        pTci->message.command.header.commandId
                = CMD_ID_RUN_DRM_TEST_PROCESS_DRM_CONTENT;
        pTci->message.command.len = mapInfo.sVirtualLen;
        pTci->message.command.data = (uint32_t) mapInfo.sVirtualAddr;

        /* notify, check result */
        mcRet = tlcNotifyAndWait(&tlSessionHandle, "drm ProcessDrmContent");
        if (MC_DRV_OK != mcRet) {
            break;
        }
        CHECK_TCI_RES("drm ProcessDrmContent", pTci, RET_OK, err);

        /*
         * Prepare TCI: closeSession
         */
        pTci->message.command.header.commandId
                = CMD_ID_RUN_DRM_TEST_CLOSE_SESSION;

        /* notify, check result */
        mcRet = tlcNotifyAndWait(&tlSessionHandle, "drm CloseSession");
        if (MC_DRV_OK != mcRet) {
            break;
        }
        CHECK_TCI_RES("drm CloseSession", pTci, RET_OK, err);

        err--;
    } while (false);

    /* Unmap cryptoBuffer */
    if (tlSessionHandle.deviceId == deviceId && mapInfo.sVirtualLen != 0) {
        mcRet = mcUnmap(&tlSessionHandle, pCryptoBuffer, &mapInfo);
        if (MC_DRV_OK != mcRet) {
            LOG_E("mcUnmap returned: %d", mcRet);
        }
    }

    /* Close Driver Session */
    mcRet = tlcCloseSession("DR", deviceId, &drSessionHandle, (uint8_t**) &pDci);

    /* Close TA Session */
    mcRet = tlcCloseSession("TL", deviceId, &tlSessionHandle, (uint8_t**) &pTci);

    /* free resources */
    if (pCryptoBuffer != NULL) {
        free(pCryptoBuffer);
    }

    /* Close MobiCore device */
    mcRet = mcCloseDevice(deviceId);
    if (MC_DRV_OK != mcRet) {
        LOG_E("mcCloseDevice returned: %d", mcRet);
    } else {
        LOG_I("mcCloseDevice returned: %d", mcRet);
    }

    LOG_I("main finished with %d error(s)", err);
    if(err == 1) {
        LOG_I("Success");
    }        

    exit(err);
    return 0;
}

static size_t tlcGetFileContent(
    const char* pPath,
    uint8_t** ppContent)
{
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

   /* Open the file */
   pStream = fopen(pPath, "rb");
   if (pStream == NULL)
   {
      fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
      return 0;
   }

   if (fseek(pStream, 0L, SEEK_END) != 0)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   filesize = ftell(pStream);
   if (filesize < 0)
   {
      fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
      goto error;
   }

   if (filesize == 0)
   {
      fprintf(stderr, "Error: Empty file: %s.\n", pPath);
      goto error;
   }

   /* Set the file pointer at the beginning of the file */
   if (fseek(pStream, 0L, SEEK_SET) != 0)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   /* Allocate a buffer for the content */
   content = (uint8_t*)malloc(filesize);
   if (content == NULL)
   {
      fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
      goto error;
   }

   /* Read data from the file into the buffer */
   if (fread(content, (size_t)filesize, 1, pStream) != 1)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   /* Close the file */
   fclose(pStream);
   *ppContent = content;

   /* Return number of bytes read */
   return (size_t)filesize;

error:
   if (content  != NULL)
   {
       free(content);
   }
   fclose(pStream);
   return 0;
}

#define PATH_MAX 4096
static mcResult_t tlcOpenSession(const char *name,
                                 uint32_t deviceId,
                                 mcSessionHandle_t *pSessionHandle,
                                 uint8_t **ppWsm,
                                 int32_t wsmSize) {
    mcResult_t  mcRet = MC_DRV_ERR_UNKNOWN;
    uint32_t    nTrustedAppLength;
    uint8_t*    pTrustedAppData = NULL;
    char        sPath[4096];
    int         n;

    do {
        if (ppWsm == NULL || pSessionHandle == NULL) {
            break;
        }

        n = snprintf(sPath, PATH_MAX, "%s/%s", TRUSTLET_PATH, name);
        if (n <= 0) {
            break ;
        }

        nTrustedAppLength = tlcGetFileContent(sPath, &pTrustedAppData);
        if (nTrustedAppLength == 0) {
            LOG_E("%s not found", sPath);
            break ;
        }

        /* Initialize session handle data */
        memset(pSessionHandle, 0, sizeof(mcSessionHandle_t));

        /* Allocating TCI */
        if ((*ppWsm = malloc(wsmSize)) == NULL) {
            LOG_E("malloc(%s) returned null", name);
            break;
        }

        /* Open the session */
        pSessionHandle->deviceId = deviceId;
        mcRet = mcOpenTrustlet(
                        pSessionHandle,
                        MC_SPID_SYSTEM, /* mcSpid_t */
                        pTrustedAppData,
                        nTrustedAppLength,
                        *ppWsm,
                        wsmSize);

        if (MC_DRV_OK != mcRet) {
            LOG_E("mcOpenTrustlet(%s) returned: %d", name, mcRet);
            //put wrong deviceId in handle to remember that session was not openned.
            pSessionHandle->deviceId = deviceId + 1;
            free(*ppWsm);
            *ppWsm = NULL;
            break;
        }

        LOG_I("mcOpenTrustlet(%s) returned: %d", name, mcRet);

    } while (0);

    if (pTrustedAppData != NULL) {
        free(pTrustedAppData);
    }

    return mcRet;
}

static mcResult_t tlcCloseSession(const char *name,
                               uint32_t deviceId,
                               mcSessionHandle_t *pSessionHandle,
                               uint8_t **ppWsm) {
    mcResult_t mcRet = MC_DRV_OK;

    if (pSessionHandle && pSessionHandle->deviceId == deviceId) {
        mcRet = mcCloseSession(pSessionHandle);
        if (MC_DRV_OK != mcRet) {
            LOG_E("mcCloseSession(%s) returned: %d", name, mcRet);
        }
    }
    if (ppWsm && *ppWsm) {
        free(*ppWsm);
        *ppWsm = NULL;
    }
    return mcRet;
}

static mcResult_t tlcNotifyAndWait(mcSessionHandle_t *pSessionHandle,
                                const char *name) {
    mcResult_t mcRet = MC_DRV_ERR_UNKNOWN;

    do {
        if (pSessionHandle == NULL) {
            break;
        }

        LOG_I("notifying (%s)", name);

        /* Notify the trustlet */
        mcRet = mcNotify(pSessionHandle);
        if (MC_DRV_OK != mcRet) {
            LOG_E("mcNotify returned: %d", mcRet);
            break;
        }

        /* Wait for response from the trustlet */
        mcRet = mcWaitNotification(pSessionHandle, MC_INFINITE_TIMEOUT);
        if (MC_DRV_OK != mcRet) {
            LOG_E("mcWaitNotification returned: %d", mcRet);
            break;
        }

    } while (0);

    return mcRet;
}

static void tlcInitCryptoBuffer(uint8_t* cryptoBuffer) {
    uint32_t i;
    char *buf = dataBuf;

    /* Init Buffer to fixed value */
    memset(cryptoBuffer, 0x5A, CRYPTO_BUFFER_SIZE);

    /* Insert IV at beginning of Buffer */
    memcpy(cryptoBuffer, aesApiIv, IV_SIZE);

    /*
     * Append Data from offset IV_SIZE
     */
    for (i = IV_SIZE; *buf != 0; i++) {
        cryptoBuffer[i] = *buf++;
    }

    /*
     * ___JUST FOR TEST PURPOSE___, we perform a DUMMY "encryption" of buffer using Key and IV
     * in order to show that tlc/TL/driver take all elements(iv/key/data/offset) into account.
     *
     * -> data[i] = data[i] + key[i%key_size] - iv[i%iv_size]
     *
     */
    for (i = 0; i < CRYPTO_DATA_SIZE; i++) {
        cryptoBuffer[IV_SIZE + i] = (uint8_t) (cryptoBuffer[IV_SIZE + i]
                + aesApiKey[i % KEY_SIZE] - aesApiIv[i % IV_SIZE]);
    }

}



