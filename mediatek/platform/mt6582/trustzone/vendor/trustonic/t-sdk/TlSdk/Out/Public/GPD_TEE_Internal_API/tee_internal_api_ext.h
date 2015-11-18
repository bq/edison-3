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
#ifndef   __TEE_INTERNAL_API_EXT_H__
#define   __TEE_INTERNAL_API_EXT_H__

#if TBASE_API_LEVEL >= 3

#include "tee_type.h"
#include "tee_error.h"
#include "tee_internal_api.h"

//------------------------------------------------------------------------------
TEE_Result TEE_TBase_UnwrapObject(
        void                *src,
        size_t              srcLen,
        void                *dest,
        size_t              destLen);

//------------------------------------------------------------------------------
TEE_Result TEE_TBase_DeriveKey(
        const void          *salt,
        size_t              saltLen,
        void                *dest,
        size_t 				destLen);

/*------------------------------------------------------------------------------
      Trusted User Interface Functions
------------------------------------------------------------------------------*/

#ifndef __TLAPITUI_H__
#define __TLAPITUI_H__
/******************************************************************************
 *                               ERROR CODES                                  *
 ******************************************************************************/

/** The session to TUI driver cannot be found. It was not opened or has been
 * closed. */
#define E_TLAPI_TUI_NO_SESSION      0x00000501

/** TUI driver is busy. Another session may be open. */
#define E_TLAPI_TUI_BUSY            0x00000502

/** TUI event queue is empty. */
#define E_TLAPI_TUI_NO_EVENT        0x00000503

/** TUI driver can't display the image because it is out of screen. */
#define E_TLAPI_TUI_OUT_OF_DISPLAY  0x00000504

/** TUI driver can't decode the image. */
#define E_TLAPI_TUI_IMG_BAD_FORMAT  0x00000505


/******************************************************************************
 *                                CONFIGURATION                               *
 ******************************************************************************/

typedef struct {
    uint32_t    grayscaleBitDepth;  /**< Available grayscale depth. */
    uint32_t    redBitDepth;        /**< Available red bit depth. */
    uint32_t    greenBitDepth;      /**< Available green bit depth. */
    uint32_t    blueBitDepth;       /**< Available blue bit depth. */
    uint32_t    width;              /**< Width of screen in pixel. */
    uint32_t    height;             /**< Height of screen in pixel. */
    uint32_t    wDensity;           /**< Density of screen in dots-per-inch. */
    uint32_t    hDensity;           /**< Density of screen in dots-per-inch. */
} tlApiTuiScreenInfo_t, *tlApiTuiScreenInfo_ptr;

/**
 * Get screen information.
 * This should be called before opening TUI session
 *
 * @param screenInfo screen Information.
 *
 * @return  TLAPI_OK upon success or specific error
 */
TEE_Result TEE_TBase_TUI_GetScreenInfo(
    tlApiTuiScreenInfo_ptr screenInfo);

/******************************************************************************
 *                                 SESSION                                    *
 ******************************************************************************/

/**
 * Open a session to the TUI driver.
 *
 * @return  TLAPI_OK upon success or specific error
 */
TEE_Result TEE_TBase_TUI_OpenSession(void);

/**
 * Close the session to the TUI driver.
 *
 * @return  TLAPI_OK upon success or specific error
 */
TEE_Result TEE_TBase_TUI_CloseSession(void);

/******************************************************************************
 *                            INPUT / OUTPUT                                  *
 ******************************************************************************/

/** Coordinates
 * These are related to the top-left corner of the screen. */
typedef struct {
    uint32_t    xOffset;   /**< x coordinate. */
    uint32_t    yOffset;   /**< y coordinate. */
} tlApiTuiCoordinates_t, *tlApiTuiCoordinates_ptr;

/** Type of touch event */
typedef enum {
    TUI_TOUCH_EVENT_RELEASED = 0,   /**< A pressed gesture has finished. */
    TUI_TOUCH_EVENT_PRESSED  = 1,   /**< A pressed gesture has occurred. */
} tlApiTuiTouchEventType_t;

/** Touch event */
typedef struct {
    tlApiTuiTouchEventType_t   type;        /**< Type of touch event. */
    tlApiTuiCoordinates_t      coordinates; /**< Coordinates of the touch event
                                             *   in the screen. */
} tlApiTuiTouchEvent_t, *tlApiTuiTouchEvent_ptr;

/** Image file */
typedef struct {
    void*       imageFile;         /**< a buffer containing the image file. */
    uint32_t    imageFileLength;   /**< size of the buffer. */
} tlApiTuiImage_t, *tlApiTuiImage_ptr;

/**
 * Draw an image in secure display.
 * @param image:        image to be displayed.
 * @param coordinates:  coordinates where to display the image in the screen.
 * @return  TLAPI_OK upon success or specific error.
 */
TEE_Result TEE_TBase_TUI_SetImage(
    tlApiTuiImage_ptr image,
    tlApiTuiCoordinates_t coordinates);

/**
 * Get a touch event from TUI driver.
 * This a blocking call.
 *
 * @param touchEvent: the touch event that occurred.
 * @return  TLAPI_OK upon success or specific error.
 */
TEE_Result TEE_TBase_TUI_GetTouchEvent(
    tlApiTuiTouchEvent_ptr touchEvent);

#endif //__TLAPITUI_H__
/*------------------------------------------------------------------------------
      Trusted Play Functions
------------------------------------------------------------------------------*/
#ifndef __TPLAY_H__
#define __TPLAY_H__

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
    uint32_t               ivlen;
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
 * @param  sHandle      [in]  Handle for multisession
 * @param  cmdId        [in]  Command ID
 * @param  decryptCtx   [in]  DRM Cipher data
 * @param  input        [in]  virtual address of contiguous memory
 * @param  blocksInfo   [in]  number of blocks and offset data in the input array to be decrypted.
 * @param  processmode  [in]  cipher mode. E.g., encrypt/decrypt/plain
 * @param  rfu          [in]  reserved for future use
 *
 * @return TLAPI_DRM_OK on success or error code.
 */
_TLAPI_EXTERN_C TEE_Result TEE_TBase_DRM_ProcessContent(
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
_TLAPI_EXTERN_C TEE_Result TEE_TBase_DRM_OpenSession(uint8_t *sHandle);


/**
 * Closes session to the driver
 *
 * @param  sHandle      [in]  Handle for multisession
 *
 * @return TLAPI_DRM_OK on success or error code.
 */
_TLAPI_EXTERN_C TEE_Result TEE_TBase_DRM_CloseSession(uint8_t sHandle);

/**
 * Check the external link(HDCPv1, HDCPv2, AirPlay, and DTCP) to the driver
 *
 * @param  sHandle      [in]  Handle for multisession
 * @param  link         [in]  external link to check
 *
 * @return TLAPI_DRM_OK on success or error code.
 */
_TLAPI_EXTERN_C TEE_Result TEE_TBase_DRM_CheckLink(uint8_t sHandle, tlApiDrmLink_t link);


#endif // __TPLAY_H__

#endif /* TBASE_API_LEVEL */

#endif /* __TEE_INTERNAL_API_EXT_H__ */

