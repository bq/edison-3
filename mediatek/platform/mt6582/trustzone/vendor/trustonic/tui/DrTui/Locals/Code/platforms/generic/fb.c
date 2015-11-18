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

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drTuiHal.h"

#define TAG "[TUI HAL FB] "

/*
 * Initialize the FrameBuffer structure info.
 */
drTuiHalError_t tuiHalFBOpen(tuiFbInfo_t *pFbInfo) {
    drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}

/**
 *  tuiHalDisplayProtectFB()
 *      uint32_t phys_base :    Physical address of memory region to be secured. Must be aligned on size. Only 32-bit supported.
 *      uint32_t size : size of the region (must be 2^n)
 *        It seems we must configure ALL TZASC modules identically (one per CPU?). Cf SLSI DRM secdrv example.
 *        Check availability only on 1st module, assuming all others are sync'ed.
 *        Note 1: Assuming 32-bit address ONLY (discard TZASC_HI)
 *        Note 2: Dealing with regions only, no subregion.
 *        Note 3: Assume "default" general config (no "init" function implemented by us).
 */
drTuiHalError_t tuiHalFBProtect(uint32_t physAddr, uint32_t size) {
    drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}


/**
 *  tuiHalDisplayUnprotectFB()
 *      uint32_t physAddr : Physical address of memory region to be secured. Must be aligned on size. Only 32-bit supported.
 *      uint32_t size :   size of the region
 */
drTuiHalError_t tuiHalFBUnprotect(uint32_t physAddr, uint32_t size) {
    drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}

/*
 * Copy an image to the framebuffer.
 */
void tuiHalFBImageBlt(tuiFbInfo_t *pFbInfo, tuiImage_t *pImage, tuiOrientation_t orientation) {

    drDbgPrintLnf(TAG "%s", __func__);
}
