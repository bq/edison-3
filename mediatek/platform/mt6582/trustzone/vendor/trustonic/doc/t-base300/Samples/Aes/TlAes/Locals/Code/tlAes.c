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
#include "tlAes_Api.h"

// Reserve 2048 byte for stack.
DECLARE_TRUSTLET_MAIN_STACK(2048);

#define ENC_LENGTH 40

static const uint8_t enckey[]  = { 0xC0, 0xA2, 0x02, 0x67, 0xF9, 0xF1, 0xE4, 0x46, 0x9F, 0x8E, 0xB7, 0xBF, 0x45, 0x70, 0x42, 0x18 };

/**
 * Encrypt data
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t encrypt( tciMessage_t* message )
{
    tlApiCrSession_t  crSession;
    tlApiResult_t     tlRet;
    tciReturnCode_t   ret = RET_OK;
    uint8_t           key[16];

    memcpy(key, enckey, sizeof(key));

    tlApiSymKey_t symKey = {
        (uint8_t*)key,
        sizeof(key),
    };

    tlDbgPrintLnf("Proceeding with encryption");

    tlApiKey_t apiKey = { &symKey };

    uint8_t* src = (uint8_t*) message->plainData;
    uint32_t srcLen = message->cmdAes.len;
    uint8_t* dst = (uint8_t*) message->cipherData;
    uint32_t dstBufLen = message->cmdAes.len;

    // Validate all NWd buffers
    if (!tlApiIsNwdBufferValid(src,srcLen) || !tlApiIsNwdBufferValid(dst,dstBufLen)) {
	tlDbgPrintLnf("Call failed due to invalid buffers, exit");
	return RET_ERR_INVALID_BUFFERS;
    }

    tlDbgPrintLnf("Plaintext length = %d", srcLen);

    do {
        tlRet = tlApiCipherInit(&crSession, TLAPI_ALG_AES_128_CBC_NOPAD, TLAPI_MODE_ENCRYPT, &apiKey);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintLnf("tlApiCipherInit failed with ret=0x%08X, exit", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        tlRet = tlApiCipherDoFinal(crSession, src, srcLen, dst, &dstBufLen);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintLnf("tlApiCipherUpdate failed with ret=0x%08X, exit", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        tlDbgPrintLnf("Ciphertext length = %d", dstBufLen);
        message->rspAes.len = dstBufLen;

    } while (false);

    return ret;
}


/**
 * Wrap data
 *
 * @return RET_OK if operation has been successfully completed.

 */
static tciReturnCode_t wrap( tciMessage_t* message )
{
    tlApiResult_t     tlRet;
    tciReturnCode_t   ret = RET_OK;

    uint8_t* src = (uint8_t*) message->plainData;
    uint32_t srcLen = message->cmdAes.len;
    uint8_t* dst = (uint8_t*) message->cipherData;
    uint32_t dstLen = message->cmdAes.respLen;

    // Validate all NWd buffers
    if (!tlApiIsNwdBufferValid(src,srcLen) || !tlApiIsNwdBufferValid(dst,dstLen)) {
	tlDbgPrintLnf("Call failed due to invalid buffers, exit");
	return RET_ERR_INVALID_BUFFERS;
    }

    size_t soLen = dstLen;

    tlDbgPrintLnf("Proceeding with wrapping data %d -> %d", srcLen, dstLen);

    do {
        // Generally neither source nor destination of wrapping / unwrapping
        // should be in NWd memory. Here both are in NWd memory to make this example simple.
        tlRet = tlApiWrapObject(src,
                                srcLen,
                                ENC_LENGTH,
                                dst,
                                &soLen,
                                MC_SO_CONTEXT_TLT,
                                MC_SO_LIFETIME_PERMANENT,
                                NULL,
                                TLAPI_WRAP_DEFAULT);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintLnf("tlApiWrapObject failed with ret=0x%08X, exit", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        tlDbgPrintLnf("Wrapped data length = %d", srcLen);

        message->rspAes.len = soLen;

    } while (false);

    return ret;
}


/**
 * Unwrap data
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t unwrap( tciMessage_t* message )
{
    tlApiResult_t     tlRet;
    tciReturnCode_t   ret = RET_OK;

    uint8_t* src = (uint8_t*) message->plainData;
    uint32_t srcLen = message->cmdAes.len;
    uint8_t* dst = (uint8_t*) message->cipherData;
    uint32_t dstLen = message->cmdAes.respLen;

    // Validate all NWd buffers
    if (!tlApiIsNwdBufferValid(src,srcLen) || !tlApiIsNwdBufferValid(dst,dstLen)) {
	tlDbgPrintLnf("Call failed due to invalid buffers, exit");
	return RET_ERR_INVALID_BUFFERS;
    }

    size_t            soLen = dstLen;

    tlDbgPrintLnf("Proceeding with unwrapping data");

    do {
        // Generally neither source nor destination of wrapping / unwrapping
        // should be in NWd memory. Here both are in NWd memory to make this example simple.
        tlRet = tlApiUnwrapObject(src,
                                srcLen,
                                dst,
                                &soLen,
                                TLAPI_UNWRAP_DEFAULT);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintLnf("tlApiUnwrapObject failed with ret=0x%08X, exit", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        tlDbgPrintLnf("Wrapped data length = %d", srcLen);

        message->rspAes.len = soLen;

    } while (false);

    return ret;
}


/**
 * Decrypt data
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t decrypt( tciMessage_t* message )
{
    tlApiCrSession_t  crSession;
    tlApiResult_t     tlRet;
    tciReturnCode_t   ret = RET_OK;
    uint8_t           key[16];

    memcpy(key, enckey, sizeof(key));

    tlApiSymKey_t symKey = {
        (uint8_t*)key,
        sizeof(key)
    };

    tlDbgPrintLnf("Proceeding with decryption");

    tlApiKey_t apiKey = { &symKey };

    uint8_t* src = (uint8_t*) message->plainData;
    uint32_t srcLen = message->cmdAes.len;
    uint8_t* dst = (uint8_t*) message->cipherData;
    uint32_t dstBufLen = message->rspAes.len;

    // Validate all NWd buffers
    if (!tlApiIsNwdBufferValid(src,srcLen) || !tlApiIsNwdBufferValid(dst,dstBufLen)) {
	tlDbgPrintLnf("Call failed due to invalid buffers, exit");
	return RET_ERR_INVALID_BUFFERS;
    }

    tlDbgPrintLnf("Ciphertext length = %d", srcLen);

    do {
        tlRet = tlApiCipherInit(&crSession, TLAPI_ALG_AES_128_CBC_NOPAD, TLAPI_MODE_DECRYPT, &apiKey);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintLnf("tlApiCipherInit failed with ret=0x%08X, exit", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        tlRet = tlApiCipherDoFinal(crSession, src, srcLen, dst, &dstBufLen);
        if (TLAPI_OK != tlRet) {
            tlDbgPrintLnf("tlApiCipherUpdate failed with ret=0x%08X, exit", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

      message->rspAes.len = dstBufLen;

    } while (false);

    return ret;
}


/**
 * Trustlet entry.
 */
_TLAPI_ENTRY void tlMain( const addr_t tciBuffer, const uint32_t tciBufferLen )
{
    tciReturnCode_t ret;
    tciCommandId_t commandId;

    dbgSN("AES trustlet is starting");

    {
        uint32_t tlApiVersion;
        mcVersionInfo_t versionInfo;

        ret = tlApiGetVersion(&tlApiVersion);
        if (TLAPI_OK != ret) {
            tlDbgPrintLnf("tlApiGetVersion failed with ret=0x%08X, exit", ret);
            tlApiExit(ret);
        }
        tlDbgPrintLnf("tlApi version 0x%08X, exit", tlApiVersion);

        ret = tlApiGetMobicoreVersion(&versionInfo);
        if (TLAPI_OK != ret) {
            tlDbgPrintLnf("tlApiGetMobicoreVersion failed with ret=0x%08X, exit", ret);
            tlApiExit(ret);
        }
        tlDbgPrintLnf("productId        = %s",     versionInfo.productId);
        tlDbgPrintLnf("versionMci       = 0x%08X", versionInfo.versionMci);
        tlDbgPrintLnf("versionSo        = 0x%08X", versionInfo.versionSo);
        tlDbgPrintLnf("versionMclf      = 0x%08X", versionInfo.versionMclf);
        tlDbgPrintLnf("versionContainer = 0x%08X", versionInfo.versionContainer);
        tlDbgPrintLnf("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
        tlDbgPrintLnf("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
        tlDbgPrintLnf("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
        tlDbgPrintLnf("versionCmp       = 0x%08X", versionInfo.versionCmp);
    }

    /* Check if the size of the given TCI is sufficient */
    if ((NULL == tciBuffer) || (sizeof(tciMessage_t) > tciBufferLen))
    {
          dbgSN("TCI error");
          dbgSPN("TCI buffer: ", tciBuffer);
          dbgSPN("TCI buffer length: ", tciBufferLen);
          dbgSDN("sizeof(tciMessage_t): ", sizeof(tciMessage_t));
          tlApiExit(EXIT_ERROR);
    }

    dbgSN("AES trustlet is processing the command");
    tciMessage_t* message = (tciMessage_t*) tciBuffer;

    for (;;)
    {

        dbgSN("AES trustlet is waiting for a notification to arrive");

        /* Wait for a notification to arrive */
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        /* Dereference commandId once for further usage */
        commandId = message->cmdAes.header.commandId;

        dbgSN("AES trustlet received command");

        /* Check if the message received is (still) a response */
        if (!IS_CMD(commandId)) {
            tlApiNotify();
            continue;
        }

        /* Process command message */
        switch (commandId) {
            case CMD_AES_ENCRYPT:
              ret = encrypt(message);
              break;
            case CMD_AES_DECRYPT:
              ret = decrypt(message);
              break;
            case CMD_AES_WRAP:
              ret = wrap(message);
              break;
            case CMD_AES_UNWRAP:
              ret = unwrap(message);
              break;
            default:
              /* Unknown command ID */
              ret = RET_ERR_UNKNOWN_CMD;
              break;
    }

      /* Set up response header */
      message->rspAes.header.responseId = RSP_ID(commandId);
      message->rspAes.header.returnCode = ret;

      dbgSDN("AES trustlet is exiting with status ", ret);

      /* Notify back the TLC */
      tlApiNotify();
    }
}
