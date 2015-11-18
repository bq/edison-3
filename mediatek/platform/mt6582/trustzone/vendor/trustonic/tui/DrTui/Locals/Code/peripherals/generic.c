/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 *
 * arndaleInput.c
 *
 *  Created on: Aug 27, 2013
 *      Author: chrpit01
 *
 *      Low level touch API for ARNDALE platform
 */


#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "device.h"
#include "drTuiHal.h"


#define TAG "[PLAT TOUCH] "

drTuiHalError_t tuiHalTouchGetInfo(drTouchInfo_ptr touchSize) {
    drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}

drTuiHalError_t tuiHalTouchOpen()
{
    drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}

// It must be assumed that touch thread is STOPPED prior to calling tuiHalTouchClose().
drTuiHalError_t tuiHalTouchClose()
{
    drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}

drTuiHalError_t tuiHalTouchReset()
{
    drTuiHalError_t ret = E_TUI_HAL_INTERNAL;
    drDbgPrintLnf(TAG "%s", __func__);
    return ret;
}
