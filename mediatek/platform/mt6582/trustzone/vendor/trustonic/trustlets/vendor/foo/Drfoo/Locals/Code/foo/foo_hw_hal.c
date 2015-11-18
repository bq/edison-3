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
#include "foo_hw_hal.h"
#include "DrApi/DrApi.h"
#include "drApiMarshal.h"


void HAL_FooAdd(tlApiFoo_ptr FooData)
{
    drDbgPrintLnf("[Driver Drfoo] HAL_FooAdd: x(%d), y(%d)", FooData->x, FooData->y);
    FooData->result = FooData->x + FooData->y;
}

void HAL_FooSub(tlApiFoo_ptr FooData)
{
    drDbgPrintLnf("[Driver Drfoo] HAL_FooSub: x(%d), y(%d)", FooData->x, FooData->y);
    FooData->result = FooData->y - FooData->x;
}
