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
#include "tlRsa_Api.h"

/* Reserve 2048 byte for stack */
DECLARE_TRUSTLET_MAIN_STACK(2048);

/**
 * Needed for 1024 bits RSA key generation. 556 bytes is the amount total
 * length of key pair structure for 1024 bits RSA key
 */
#define RSA_KEY_STRUCT_SIZE     556
#define RSA_KEY_DATA_SIZE       512
#define RSA_PRI_KEY_DATA_SIZE   256
#define RSA_PUB_KEY_DATA_SIZE   256
#define RSA_SIGNATURE_SIZE      128

#define STRUCT_M(data,type,buf)  ({ data = (type*) buf; buf=buf+sizeof(type); })
#define STRUCT_D(data,buf,lenth) ({ data = (uint8_t *) buf; buf=buf+lenth; })
#define COPY_KEY_DATA(dst,src,length) ({ memcpy(dst, src, length); dst=dst+length; })


static const uint8_t exp_3[]     = { 0x03 };

/**
 * Generate RSA key pair
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t generateKeyPair( tciMessage_t* message )
{
    tlApiResult_t     tlRet;
    tciReturnCode_t   ret = RET_OK;
    uint32_t          modlen = (1024/8);
    uint8_t           sbuf[RSA_KEY_STRUCT_SIZE];
    uint8_t           keydata[RSA_KEY_DATA_SIZE];
    uint8_t*          kbuf = &keydata[0];  /* key data buffer  */
    uint8_t*          pSbuf  = &sbuf[0];   /* structure buffer */
    uint8_t*          dstBuf    = (uint8_t*) message->outputData;
    uint32_t          dstBufLen = MC_SO_SIZE(RSA_PUB_KEY_DATA_SIZE, RSA_PRI_KEY_DATA_SIZE);
    tlApiCrSession_t  sessionHandle;
    tlApiKey_t        key;
    tlApiRsaKey_t*    rsaKeyPair = NULL;
    tlApiRsaKey_t     rsaKeySign;
    tlApiRsaKey_t     rsaKeyVerify;
    tlApiKeyPair_t    keyPair;
    uint8_t           sigData[RSA_SIGNATURE_SIZE];
    uint32_t          sigLen = sizeof(sigData);
    bool              isValidSig = false;

    // Validate all NWd buffers
    if (!tlApiIsNwdBufferValid(dstBuf,dstBufLen)) {
        tlDbgPrintLnf("Call failed due to invalid buffer, exit");
        return RET_ERR_INVALID_BUFFERS;
    }
    
    /**
     * We need to use internal memory for creating keys
     * if private key is placed to shared memory during
     * key generation, it can be compromised to NWd.
     * We will only copy encrypted data to
     * the shared memory. Further explanation on modulus
     * and exponent lengths (for 1024 bits RSA key):
     * RSA private key exponent length is 128 bytes
     * RSA public key modulus length is 128 bytes
     * RSA public key exponent length is 128 bytes
     *
     * Set addresses in the key pair structure. The generated
     * values will be written to these addresses
     */
    STRUCT_M(rsaKeyPair, tlApiRsaKey_t, pSbuf);
    STRUCT_D(rsaKeyPair->exponent.value, pSbuf, modlen);
    STRUCT_D(rsaKeyPair->modulus.value, pSbuf, modlen);
    rsaKeyPair->exponent.len = modlen;
    rsaKeyPair->modulus.len  = modlen;

    /* Private key part*/
    STRUCT_D(rsaKeyPair->privateExponent.value, pSbuf, modlen);
    rsaKeyPair->privateExponent.len = modlen;

    /* P,Q, DP, DQ and Qinv are all set to 0 */
    memset(&rsaKeyPair->privateCrtKey, 0, sizeof(rsaKeyPair->privateCrtKey));

    /* Public exponent data */
    memcpy(rsaKeyPair->exponent.value, (uint8_t *) exp_3, sizeof(exp_3));
    rsaKeyPair->exponent.len = sizeof(exp_3);

    keyPair.rsaKeyPair = rsaKeyPair;

    do {

        dbgSN("RSA: generating RSA key pair");

        /**
         * STEP 1: Generate RSA key pair
         */

        /* Generate RSA key pair */
        tlRet = tlApiGenerateKeyPair(&keyPair, TLAPI_RSA, modlen);
        if (TLAPI_OK != tlRet) {
            dbgSHN("RSA: tlApiGenerateKeyPair failed with ret=", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        dbgSN("RSA: Proceeding with sign operation");

        /* hardcoded data for signing */
        uint8_t messageData[] = { 0x01, 0xc8, 0x7d, 0xa2, 0x23, 0xd7, 0x86, 0xdf,
                0x3b, 0x45, 0xe0, 0xbb, 0xbc, 0x72, 0x13, 0x26, 0xd1, 0xee, 0x2a,
                0xf8, 0x06, 0xcc, 0x31, 0x54, 0x75, 0xcc, 0x6f, 0x0d, 0x9c, 0x66,
                0xe1, 0xb6, 0x23, 0x71, 0xd4, 0x5c, 0xe2, 0x39, 0x2e, 0x1a, 0xc9,
                0x28, 0x44, 0xc3, 0x10, 0x10, 0x2f, 0x15, 0x6a, 0x0d, 0x8d, 0x52,
                0xc1, 0xf4, 0xc4, 0x0b, 0xa3, 0xaa, 0x65, 0x09, 0x57, 0x86, 0xcb,
                0x76, 0x97, 0x57, 0xa6, 0x56, 0x3b, 0xa9, 0x58, 0xfe, 0xd0, 0xbc,
                0xc9, 0x84, 0xe8, 0xb5, 0x17, 0xa3, 0xd5, 0xf5, 0x15, 0xb2, 0x3b,
                0x8a, 0x41, 0xe7, 0x4a, 0xa8, 0x67, 0x69, 0x3f, 0x90, 0xdf, 0xb0,
                0x61, 0xa6, 0xe8, 0x6d, 0xfa, 0xae, 0xe6, 0x44, 0x72, 0xc0, 0x0e,
                0x5f, 0x20, 0x94, 0x57, 0x29, 0xcb, 0xeb, 0xe7, 0x7f, 0x06, 0xce,
                0x78, 0xe0, 0x8f, 0x40, 0x98, 0xfb, 0xa4, 0x1f, 0x9d, 0x61, 0x93,
                0xc0, 0x31, 0x7e, 0x8b, 0x60, 0xd4, 0xb6, 0x08, 0x4a, 0xcb, 0x42,
                0xd2, 0x9e, 0x38, 0x08, 0xa3, 0xbc, 0x37, 0x2d, 0x85, 0xe3, 0x31,
                0x17, 0x0f, 0xcb, 0xf7, 0xcc, 0x72, 0xd0, 0xb7, 0x1c, 0x29, 0x66,
                0x48, 0xb3, 0xa4, 0xd1, 0x0f, 0x41, 0x62, 0x95, 0xd0, 0x80, 0x7a,
                0xa6, 0x25, 0xca, 0xb2, 0x74, 0x4f, 0xd9, 0xea, 0x8f, 0xd2, 0x23,
                0xc4, 0x25, 0x37, 0x02, 0x98, 0x28, 0xbd, 0x16, 0xbe, 0x02, 0x54,
                0x6f, 0x13, 0x0f, 0xd2, 0xe3, 0x3b, 0x93, 0x6d, 0x26, 0x76, 0xe0,
                0x8a, 0xed, 0x1b, 0x73, 0x31, 0x8b, 0x75, 0x0a, 0x01, 0x67, 0xd0 };

        uint32_t messageLen = sizeof(messageData);

        /**
         * STEP 2: Sign data
         */

        /* Use private key data for signing data */
        rsaKeySign.modulus.value  = rsaKeyPair->modulus.value;
        rsaKeySign.modulus.len    = rsaKeyPair->modulus.len;
        rsaKeySign.exponent.value = rsaKeyPair->exponent.value;
        rsaKeySign.exponent.len   = rsaKeyPair->exponent.len;
        rsaKeySign.privateExponent.value = rsaKeyPair->privateExponent.value;
        rsaKeySign.privateExponent.len   = rsaKeyPair->privateExponent.len;
        memset(&rsaKeySign.privateCrtKey, 0, sizeof(rsaKeySign.privateCrtKey));
        key.rsaKey = &rsaKeySign;

        /* Do signature init */
        dbgSN("RSA: Init data for signing with TLAPI_SIG_RSA_SHA_PKCS1 type signature");
        tlRet = tlApiSignatureInit(&sessionHandle,
                                   &key,
                                   TLAPI_MODE_SIGN,
                                   TLAPI_SIG_RSA_SHA_PKCS1);
        if (TLAPI_OK != tlRet) {
            dbgSHN("RSA: tlApiSignatureInit failed with ret=", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        /* Proceed with signing data */
        dbgSN("RSA: Signing data");
        tlRet =  tlApiSignatureSign(sessionHandle,
                                    messageData,
                                    messageLen,
                                    sigData,
                                    &sigLen);

        if (TLAPI_OK != tlRet) {
            dbgSHN("RSA: tlApiSignatureInit failed with ret=", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        dbgSDN("RSA: signature data length: ", sigLen);

        /**
         * STEP 3: Verify signature
         */


        /**
         * Proceed with verifying signature
         * Use private key for verifying data
         */
        rsaKeyVerify.modulus.value  = rsaKeyPair->modulus.value;
        rsaKeyVerify.modulus.len    = rsaKeyPair->modulus.len;
        rsaKeyVerify.exponent.value = rsaKeyPair->exponent.value;
        rsaKeyVerify.exponent.len   = rsaKeyPair->exponent.len;
        /* P,Q, DP, DQ and Qinv are all set to 0 */
        memset(&rsaKeyVerify.privateCrtKey, 0, sizeof(rsaKeyVerify.privateCrtKey));
        rsaKeyVerify.privateExponent.value = NULL;
        rsaKeyVerify.privateExponent.len = 0;

        key.rsaKey = &rsaKeyVerify;

        dbgSN("RSA: Init data for verifying with TLAPI_SIG_RSA_SHA_PKCS1 type signature");
        tlRet = tlApiSignatureInit(&sessionHandle,
                                   &key,
                                   TLAPI_MODE_VERIFY,
                                   TLAPI_SIG_RSA_SHA_PKCS1);
        if (TLAPI_OK != tlRet) {
            dbgSHN("RSA: tlApiSignatureInit failed with ret=", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        dbgSN("RSA: Verifying data");
        tlRet =  tlApiSignatureVerify(sessionHandle,
                                      messageData,
                                      messageLen,
                                      sigData,
                                      sigLen,
                                      &isValidSig);

        if (TLAPI_OK != tlRet) {
            dbgSHN("RSA: tlApiSignatureVerify failed with ret=", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        dbgSDN("RSA: Signature verification status: ", isValidSig);

        /**
         * STEP 4: Wrap key data (to be returned to TLC)
         */

        /**
         * We need to copy private/public key modulus and
         * exponent data into a buffer as below. This buffer will be wrapped.
         *
         * |--Public key modulus--|--Public key exponent--|--Private key exponent--|

         * |------------------plaintext-------------------|-------encrypted--------|
         *
         * When wrapping data, first 256 bytes will be wrapped as plaintext and another part will be encrypted.
         * Encrypted part contains private key modulus and exponent data
         */
        COPY_KEY_DATA(kbuf, rsaKeyPair->modulus.value,   modlen);
        COPY_KEY_DATA(kbuf, rsaKeyPair->exponent.value,  modlen);
        COPY_KEY_DATA(kbuf, rsaKeyPair->privateExponent.value,  modlen);
        kbuf = &keydata[0];

        /* Proceed with wrapping key data */
        dbgSN("RSA: wrap key data");
        tlRet = tlApiWrapObject(kbuf,
                                RSA_PUB_KEY_DATA_SIZE,
                                RSA_PRI_KEY_DATA_SIZE,
                                dstBuf,
                                &dstBufLen,
                                MC_SO_CONTEXT_TLT,
                                MC_SO_LIFETIME_PERMANENT,
                                NULL,
                                TLAPI_WRAP_DEFAULT);
        if (TLAPI_OK != tlRet) {
            dbgSHN("RSA: tlApiWrapObject failed with ret=", tlRet);
            ret = RET_ERR_CRYPTO;
            break;
        }

        if (MC_SO_SIZE(RSA_PUB_KEY_DATA_SIZE, RSA_PRI_KEY_DATA_SIZE) != dstBufLen) {
            dbgSN("RSA: Problem with SO length");
            message->rspRsa.len = 0;
            ret = RET_ERR_CRYPTO;
            break;
        }

        message->rspRsa.len = dstBufLen;

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

    dbgSN("RSA: trustlet is starting");

    /* Check if the size of the given TCI is sufficient */
    if ((NULL == tciBuffer) || (sizeof(tciMessage_t) > tciBufferLen)) {
          dbgSN("TCI error");
          dbgSPN("TCI buffer: ", tciBuffer);
          dbgSPN("TCI buffer length: ", tciBufferLen);
          dbgSDN("sizeof(tciMessage_t): ", sizeof(tciMessage_t));
          tlApiExit(EXIT_ERROR);
    }

    tciMessage_t* message = (tciMessage_t*) tciBuffer;

    for (;;) {

        /* Wait for a notification to arrive */
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        /* Dereference commandId once for further usage */
        commandId = message->cmdRsa.header.commandId;

        dbgSN("RSA: trustlet received command");

        /* Check if the message received is (still) a response */
        if (!IS_CMD(commandId)) {
            tlApiNotify();
            continue;
        }

        /* Process command message */
        switch (commandId) {
            case CMD_RSA:
                ret = generateKeyPair(message);
                break;
            default:
                /* Unknown command ID */
                ret = RET_ERR_UNKNOWN_CMD;
                break;
      }

      /* Set up response header */
      message->rspRsa.header.responseId = RSP_ID(commandId);
      message->rspRsa.header.returnCode = ret;
      dbgSDN("RSA: trustlet is exiting with status ", ret);

      /* Notify back the TLC */
      tlApiNotify();
    }
}
