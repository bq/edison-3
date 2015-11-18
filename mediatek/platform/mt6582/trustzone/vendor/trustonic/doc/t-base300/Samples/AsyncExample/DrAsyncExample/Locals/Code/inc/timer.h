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
 * @brief  HW Timer declarations
 *
 */

#ifndef __TIMER_H__
#define __TIMER_H__

void timerInit(void);
void timerShutdown(void);
void timerIsr(void);

uint32_t plat_getTimerBase(void);
uint32_t plat_getIrqNumber(void);
void plat_timerInit(uint32_t virt_timer_base_ext);
void plat_timerShutdown(void);
void plat_clearInterrupt(void);

#define writel(v,a) (*(volatile unsigned int *)(a) = (v))
#define readl(a)    (*(volatile unsigned int *)(a))

#endif // __TIMER_H__

/** @} */
