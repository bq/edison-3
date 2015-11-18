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

/**
 * @file   timer.h
 * @brief  Timer auxiliary functions
 *
 */

#ifndef __TIMERUTILS_H__
#define __TIMERUTILS_H__

#define STACK_DBG_INIT_VALUE 0xCC

#define fillStack(_name_, _val_) \
        memset(_STACK_ARRAY_VAR(_name_), _val_, getStackSize(_name_))

#define writel(v,a) (*(volatile unsigned int *)(a) = (v))
#define readl(a)    (*(volatile unsigned int *)(a))

#endif // __TIMERUTILS_H__

/** @} */
