/**
 * Marshalling of Trustlet API crypto
 * Function calls are mapped to a flat call.
 */

//#include <mcstd.h>
//#include <mcutil.h>
#include <stdint.h>
#include <stdio.h>
typedef enum {
	false = 0,
	true  = 1
} bool;

#define _TLAPI_EXTERN_C
#include "TlApiMarshal.h"
#include "TlApi/TlApiCrypto.h"
#include "mcDriverId.h"


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiCrAbort(
    tlApiCrSession_t    sessionHandle
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_SESSION_ABORT,
        .payload    = {
            .crAbort = {
                .sessionHandle = sessionHandle
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiGenerateKeyPair(
    tlApiKeyPair_t      *keyPair,
    tlApiKeyPairType_t  type,
    size_t              len
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_GENERATE_KEY_PAIR,
        .payload    = {
            .crGenerateKeyPair = {
                .keyPair   = keyPair,
                .type      = type,
                .len       = len,
                .buffer    = NULL,
                .bufferLen = 0
            }                
        }
    };
    
    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiGenerateKeyPairIntoBuffer(
    tlApiKeyPair_t      *keyPair,
    tlApiKeyPairType_t  type,
    size_t              len,
    uint8_t             *buffer,
    size_t              bufferLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_GENERATE_KEY_PAIR_BUFFER,
        .payload    = {
            .crGenerateKeyPair = {
                .keyPair   = keyPair,
                .type      = type,
                .len       = len,
                .buffer    = buffer,
                .bufferLen = bufferLen,
            }
        }            
    };
    
    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiCipherInit(
    tlApiCrSession_t    *pSessionHandle,
    tlApiCipherAlg_t    alg,
    tlApiCipherMode_t   mode,
    const tlApiKey_t    *key
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_CIPHER_INIT,
        .payload    = {
            .cipherInit = {
                .pSessionHandle = pSessionHandle,
                .alg            = alg,
                .mode           = mode,
                .key            = key,
                .buffer         = NULL,
                .bufferLen      = 0
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiCipherInitWithData(
    tlApiCrSession_t    *pSessionHandle,
    tlApiCipherAlg_t    alg,
    tlApiCipherMode_t   mode,
    const tlApiKey_t    *key,
    const uint8_t       *buffer,
    size_t              bufferLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_CIPHER_INIT_WITH_DATA,
        .payload    =  {
            .cipherInit = {
                .pSessionHandle = pSessionHandle,
                .alg            = alg,
                .mode           = mode,
                .key            = key,
                .buffer         = buffer,
                .bufferLen      = bufferLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiCipherUpdate(
    tlApiCrSession_t    sessionHandle,
    const uint8_t       *srcData,
    size_t              srcLen,
    uint8_t             *destData,
    size_t              *destLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_CIPHER_UPDATE,
        .payload    = {
            .cipherUpdate = {
                .sessionHandle = sessionHandle,
                .srcData       = srcData,
                .srcLen        = srcLen,
                .destData      = destData,
                .destLen       = destLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiCipherDoFinal(
    tlApiCrSession_t    sessionHandle,
    const uint8_t       *srcData,
    size_t              srcLen,
    uint8_t             *destData,
    size_t              *destLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_CIPHER_DOFINAL,
        .payload    = {
            .cipherDoFinal = {
                .sessionHandle = sessionHandle,
                .srcData       = srcData,
                .srcLen        = srcLen,
                .destData      = destData,
                .destLen       = destLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiSignatureInit(
    tlApiCrSession_t    *pSessionHandle,
    const tlApiKey_t    *key,
    tlApiSigMode_t      mode,
    tlApiSigAlg_t       alg
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_SIG_INIT,
        .payload    = {
            .signatureInit = {
                .pSessionHandle = pSessionHandle,
                .key            = (tlApiKey_t *)key,
                .mode           = mode,
                .alg            = alg,
                .buffer         = NULL,
                .bufferLen      = 0
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiSignatureInitWithData(
    tlApiCrSession_t    *pSessionHandle,
    const tlApiKey_t    *key,
    tlApiSigMode_t      mode,
    tlApiSigAlg_t       alg,
    const uint8_t       *buffer,
    size_t              bufferLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_SIG_INIT_WITH_DATA,
        .payload    = {
            .signatureInit = {
                .pSessionHandle = pSessionHandle,
                .key            = (tlApiKey_t *)key,
                .mode           = mode,
                .alg            = alg,
                .buffer         = buffer,
                .bufferLen      = bufferLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiSignatureUpdate(
    tlApiCrSession_t    sessionHandle,
    const uint8_t       *message,
    size_t              messageLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_SIG_UPDATE,
        .payload    = {
            .signatureUpdate = {
                .sessionHandle = sessionHandle,
                .message       = message,
                .messageLen    = messageLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiSignatureSign(
    tlApiCrSession_t    sessionHandle,
    const uint8_t       *message,
    size_t              messageLen,
    uint8_t             *signature,
    size_t              *signatureLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_SIG_SIGN,
        .payload    = {
            .signatureSign = {
                .sessionHandle = sessionHandle,
                .message       = message,
                .messageLen    = messageLen,
                .signature     = signature,
                .signatureLen  = signatureLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiSignatureVerify(
    tlApiCrSession_t    sessionHandle,
    const uint8_t       *message,
    size_t              messageLen,
    const uint8_t       *signature,
    size_t              signatureLen,
    bool                *validity
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_SIG_VERIFY,
        .payload    = {
            .signatureVerify = {
                .sessionHandle = sessionHandle,
                .message       = message,
                .messageLen    = messageLen,
                .signature     = signature,
                .signatureLen  = signatureLen,
                .validity      = validity
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiMessageDigestInit(
    tlApiCrSession_t    *pSessionHandle,
    tlApiMdAlg_t        alg
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_MD_INIT,
        .payload    = {
            .messageDigestInit = {
                .pSessionHandle   = pSessionHandle,
                .alg              = alg,
                .buffer           = NULL,
                .lenPreHashedData = 0
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiMessageDigestInitWithData(
    tlApiCrSession_t    *pSessionHandle,
    tlApiMdAlg_t        alg,
    const uint8_t       *buffer,
    const uint8_t       lenPreHashedData[8]
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_MD_INIT_WITH_DATA,
        .payload    = {
            .messageDigestInit = {
                .pSessionHandle   = pSessionHandle,
                .alg              = alg,
                .buffer           = buffer,
                .lenPreHashedData = lenPreHashedData
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiMessageDigestUpdate(
    tlApiCrSession_t    sessionHandle,
    const uint8_t       *message,
    size_t              messageLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_MD_UPDATE,
        .payload    = {
            .messageDigestUpdate = {
                .sessionHandle = sessionHandle,
                .message       = message,
                .messageLen    = messageLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiMessageDigestDoFinal(
    tlApiCrSession_t    sessionHandle,
    const uint8_t       *message,
    size_t              messageLen,
    uint8_t             *hash,
    size_t              *hashLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_MD_DOFINAL,
        .payload    = {
            .messageDigestDoFinal = {
                .sessionHandle = sessionHandle,
                .message       = message,
                .messageLen    = messageLen,
                .hash          = hash,
                .hashLen       = hashLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}


//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiRandomGenerateData(
    tlApiRngAlg_t   alg,
    uint8_t         *randomBuffer,
    size_t          *randomLen
) {
    marshalingParam_t marParam = {
        .functionId = FID_CRYPTO_RNG_GENERATE_DATA,
        .payload    = {
            .randomGenerateData = {
                .alg          = alg,
                .randomBuffer = randomBuffer,
                .randomLen    = randomLen
            }
        }
    };

    return callCryptoDriver(&marParam);
}

/** @} */

/** @addtogroup TLAPI_SEC
 * @{
 * @file
 * TlApi security functions.
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2009-2012 -->
 */

//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiWrapObjectExt(
    const void                *src,
    size_t                    plainLen,
    size_t                    encryptedLen,
    void                      *dest,
    size_t                    *destLen,
    mcSoContext_t             context,
    mcSoLifeTime_t            lifetime,
    const tlApiSpTrustletId_t *consumer,
    uint32_t flags)
{
    // TODO-2012-08-07-paaterol flags should be encoded into high bits of lifetime or context.
    // as we are at the marshalling parameter limit of 8.
    marshalingParam_t marParam = {
        .functionId = FID_SECURITY_WRAP_OBJECT,
        .payload    = {
            .wrapObject = {
                .src        = src,
                .plainLen   = plainLen,
                .encryptedLen = encryptedLen,
                .dest       = dest,
                .destLen    = destLen,
                .context    = context,
                .lifetime   = lifetime,
                .consumer   = consumer
            }
        }
    };

    return callCryptoDriver(&marParam);
}

//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiUnwrapObjectExt(
    void                *src,
    size_t              srcLen,
    void                *dest,
    size_t              *destLen,
    uint32_t            flags
) {
    // Value srcLen==0 i used to detect old trustlets (eg srcLen parameter was missing).
    // This prevents accidental use of this in new code.
    if (srcLen==0)
        return E_TLAPI_BUFFER_TOO_SMALL;
    marshalingParam_t marParam = {
        .functionId = FID_SECURITY_UNWRAP_OBJECT,
        .payload    = {
            .unwrapObject = {
                .src        = src,
                .dest       = dest,
                .destLen    = destLen,
                .srcLen     = srcLen,
                .flags      = flags,
            }
        }
    };

    return callCryptoDriver(&marParam);
}

//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiGetSuid(
    mcSuid_t            *suid
) {
    marshalingParam_t marParam = {
        .functionId = FID_SECURITY_GET_SUID,
        .payload    = {
            .getSuid = {
                .suid       = suid,
            }
        }
    };

    return callCryptoDriver(&marParam);
}
//------------------------------------------------------------------------------
static tlApiResult_t callSecurityDriver(
    const marshalingParam_ptr pMarParam
) {
    return callCryptoDriver(pMarParam);
}

//------------------------------------------------------------------------------
static tlApiResult_t callSystemDriver(
    const marshalingParam_ptr pMarParam
) {
    return callCryptoDriver(pMarParam);
}

//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiGetMobicoreVersion(
    mcVersionInfo_t *mcVersionInfo
) {
    marshalingParam_t marParam = {
        .functionId = FID_SYSTEM_GET_VERSION,
        .payload    = {
            .getVersion = {
                .mcVersionInfo   = mcVersionInfo,
            }
        }
    };

    return callSystemDriver(&marParam);
}

//------------------------------------------------------------------------------
_TLAPI_EXTERN_C tlApiResult_t tlApiGetVersion(
    uint32_t *tlApiVersion
) {
    tlApiResult_t ret = E_TLAPI_COM_ERROR;

    // Set-up IPC call to driver
    uint32_t partner = 0x00030001;//IPCH

    // tlApi is implemented by CR driver, so we get CR version
    mcDriverId_t crDrvId = MC_DRV_ID_CRYPTO;

    //message_t   ipcMsg = MSG_GET_DRIVER_VERSION;
    uint32_t ipcMsg = 11;

    // version is returned in last parameter.
    uint32_t    ipcData = 0;

    uint32_t timeout = 0x0C000000;
    uint32_t ipcReturn;

    // send message to IPCH and wait for answer.
    ipcReturn = TrustletApiForwarder_ipc(
                    &partner,
                    (uint32_t*)&crDrvId,
                    (uint32_t*)&ipcMsg,
                    &ipcData,
                    timeout);

    if(0 == ipcReturn)
    {
        *tlApiVersion = ipcData;
        ret = TLAPI_OK; 
    } else {
        printf("\ntlApiGetVersion: ERROR, TrustletApiForwarder_ipc(...) returned 0x%x",ipcReturn);
    }
    // Set return value from MTK as return
    return ret;
}
//------------------------------------------------------------------------------
/** @} */
