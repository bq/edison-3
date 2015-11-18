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

/** Trusted User Interface API - TUI Base API */

#ifndef __TLAPITUI_H__
#define __TLAPITUI_H__

#if TBASE_API_LEVEL >= 3

#include "TlApi/TlApiError.h"

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

/** Screen information. */
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
_TLAPI_EXTERN_C tlApiResult_t tlApiTuiGetScreenInfo(
    tlApiTuiScreenInfo_ptr screenInfo);

/******************************************************************************
 *                                 SESSION                                    *
 ******************************************************************************/

/**
 * Open a session to the TUI driver.
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiTuiOpenSession(void);

/**
 * Close the session to the TUI driver.
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiTuiCloseSession(void);

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
_TLAPI_EXTERN_C tlApiResult_t tlApiTuiSetImage(
    tlApiTuiImage_ptr image,
    tlApiTuiCoordinates_t coordinates);

/**
 * Get a touch event from TUI driver.
 * This is non blocking call. It shall be called when a the TL is notified.
 *
 * @param touchEvent: the touch event that occurred.
 * @return  TLAPI_OK upon success or specific error.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiTuiGetTouchEvent(
    tlApiTuiTouchEvent_ptr touchEvent);

#endif  /* TBASE_API_LEVEL */

#endif /* __TLAPITUI_H__ */

