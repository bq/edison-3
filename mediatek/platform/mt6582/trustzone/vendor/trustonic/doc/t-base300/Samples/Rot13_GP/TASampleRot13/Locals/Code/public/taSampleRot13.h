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

#ifndef TASAMPLEROT13API_H_
#define TASAMPLEROT13API_H_

/** Service-specific Command identifiers **/
#define CMD_SAMPLE_ROT13    1  /**< ROT13  */
#define CMD_HELLO           2

#define CMD_ROT13_PTYPES TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE)
/**
 * ROT13 parameter description.
 * The ROT13 command will apply disguise to memref data provided in parameter 1.
 * Only characters (a-z, A-Z) will be processed.
 * Other data is ignored and stays untouched.
 */

/**
 * Maximum data length.
 */
#define MAX_DATA_LEN 1024

#define CMD_HELLO_PTYPES TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE)
/**
 * Hello parameter description.
 * The Hello command will put a greeting to the memref provided in parameter 1.
 */

#endif // TASAMPLEROT13API_H_
