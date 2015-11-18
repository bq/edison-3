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

/** <t-play Drm API */

#ifndef __TPLAY_H__
#define __TPLAY_H__

#if TBASE_API_LEVEL >= 3

#include "TlApi/TlApiError.h"

/******************************************************************************
 *                               ERROR CODES                                  *
 ******************************************************************************/

/** Success */
#define TLAPI_DRM_OK                            TLAPI_OK

/** Invalid parameter for cipher operation */
#define E_TLAPI_DRM_INVALID_PARAMS              0x00000601

/** Internal error in the driver */
#define E_TLAPI_DRM_INTERNAL                    0x00000602

/** Driver mapping error */
#define E_TLAPI_DRM_MAP                         0x00000603

/** Permission denied */
#define E_TLAPI_DRM_PERMISSION_DENIED           0x00000604

/** If the output address is not protected. */
#define E_TLAPI_DRM_REGION_NOT_SECURE           0x00000605

/** If a single session implementation is already active, or a multi session implementation has no free sessions. */
#define E_TLAPI_DRM_SESSION_NOT_AVAILABLE       0x00000606

/** Invalid Command ID. */
#define E_TLAPI_DRM_INVALID_COMMAND             0x00000607

/** If algorithm is not supported by the driver. */
#define E_TLAPI_DRM_ALGORITHM_NOT_SUPPORTED     0x00000608

/** If the functions have not been implemented. */
#define E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED      0x00000609


/******************************************************************************
 *                               DRM KEYS                                     *
 ******************************************************************************/

/** DRM 128 bits key size */
#define TLAPI_DRM_KEY_SIZE_128            16
/** DRM 192 bits key size */
#define TLAPI_DRM_KEY_SIZE_192            24
/** DRM 256 bits key size */
#define TLAPI_DRM_KEY_SIZE_256            32


/******************************************************************************
 *                               DRM PROCESS MODE                             *
 ******************************************************************************/

/** DRM data given to driver is encrypted, the driver shall decrypt. */
#define TLAPI_DRM_PROCESS_ENCRYPTED_DATA   1
/** DRM data given to driver is decrypted, the driver shall only decode. */
#define TLAPI_DRM_PROCESS_DECRYPTED_DATA   2


/******************************************************************************
 *                               DRM TYPES                                    *
 ******************************************************************************/

/** Number of offset/size pair in input descriptor */
#define TLAPI_DRM_INPUT_PAIR_NUMBER     10

/** DRM Encryption Algoritms */
typedef enum {
    TLAPI_DRM_ALG_NONE,
    TLAPI_DRM_ALG_AES_ECB,
    TLAPI_DRM_ALG_AES_CBC,
    TLAPI_DRM_ALG_AES_CTR32,
    TLAPI_DRM_ALG_AES_CTR64,
    TLAPI_DRM_ALG_AES_CTR96,
    TLAPI_DRM_ALG_AES_CTR128,
    TLAPI_DRM_ALG_AES_XTS,
    TLAPI_DRM_ALG_AES_CBCCTS
} tlApiDrmAlg_t;

/** DRM External Links */
typedef enum {
    TLAPI_DRM_LINK_HDCP_1,
    TLAPI_DRM_LINK_HDCP_2,
    TLAPI_DRM_LINK_AIRPLAY,
    TLAPI_DRM_LINK_DTCP
} tlApiDrmLink_t;

/** DRM frame/block definition */
typedef struct {
    /** size of encrypted block */
    uint32_t nSize;
    /** offset from start of buffer to start of encrypted block */
    uint32_t nOffset;
} tlApiDrmOffsetSizePair_t;

/** DRM Input data */
typedef struct {
    /** size of whole data (plain + encrypted) */
    uint32_t                    nTotalSize;
    /** number of blocks of encrypted data within the buffer */
    uint32_t                    nNumBlocks;
    /** Encrypted blocks */
    tlApiDrmOffsetSizePair_t    aPairs[TLAPI_DRM_INPUT_PAIR_NUMBER];
} tlApiDrmInputSegmentDescriptor_t;


/**
 * For DRM cipher/copy operations
 *
 * Parameters
 * @param  key          [in]  content key
 * @param  key_len      [in]  key length in bytes (16,24,32)
 * @param  iv           [in]  initialization vector. Always 16 bytes.
 * @param  ivlen        [in]  length initialization vector.
 * @param  alg          [in]  algorithm
 * @param  outputoffset [in]  output data offset
 *
 */
typedef struct {
    uint8_t                *key;
    int32_t                keylen;
    uint8_t                *iv;
    uint32_t          	   ivlen;
    tlApiDrmAlg_t          alg;
    uint32_t               outputoffet;
} tlApiDrmDecryptContext_t;


/******************************************************************************
 *                               DRM FUNCTIONS                                *
 ******************************************************************************/

/**
 * For DRM cipher/copy operations
 *
 * Parameters
 * @param  sHandle      [in]  Handler for multisession
 * @param  cmdId        [in]  Command ID
 * @param  decryptCtx   [in]  DRM Cipher data
 * @param  input        [in]  virtual address of contiguous memory
 * @param  blocksInfo   [in]  number of blocks and offset data in the input array to be decrypted.
 * @param  processmode  [in]  cipher mode. E.g., encrypt/decrypt/plain
 * @param  rfu          [in]  reserved for future use
 *
 * @return TLAPI_DRM_OK on success or error code.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiDrmProcessContent(
        uint8_t                             sHandle,
        tlApiDrmDecryptContext_t            decryptCtx,
        uint8_t                             *input,
        tlApiDrmInputSegmentDescriptor_t    inputDesc,
        uint16_t                            processMode,
        uint8_t                             *rfu);

/**
 * Opens session to the driver
 *
 * @param  sHandle      [out]  Handle for multisession
 *
 * @return TLAPI_DRM_OK on success or error code.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiDrmOpenSession(uint8_t *sHandle);


/**
 * Closes session to the driver
 *
 * @param  sHandle      [in]  Handle for multisession
 *
 * @return TLAPI_DRM_OK on success or error code.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiDrmCloseSession(uint8_t sHandle);

/**
 * Check the external link(HDCPv1, HDCPv2, AirPlay, and DTCP) to the driver
 *
 * @param  sHandle      [in]  Handle for multisession
 * @param  link         [in]  external link to check
 *
 * @return TLAPI_DRM_OK on success or error code.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiDrmCheckLink(uint8_t sHandle, tlApiDrmLink_t link);

#endif /* TBASE_API_LEVEL */

#endif // __TPLAY_H__

