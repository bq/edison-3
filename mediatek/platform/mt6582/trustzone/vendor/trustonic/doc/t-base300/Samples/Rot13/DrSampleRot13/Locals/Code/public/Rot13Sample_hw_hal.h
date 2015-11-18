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
 
#ifndef __ROT13SAMPLE_HW_HAL_H__
#define __ROT13SAMPLE_HW_HAL_H__

#define MAXIMUM_DATA_LENGTH 1024

void HAL_Rot13encrypt(uint32_t len, uint8_t *cleartext);

#endif // __ROT13SAMPLE_HW_HAL_H__
