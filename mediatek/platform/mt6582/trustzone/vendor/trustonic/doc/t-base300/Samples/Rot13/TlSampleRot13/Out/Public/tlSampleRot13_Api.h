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

#ifndef TL_SAMPLE_ROT13_API_H_
#define TL_SAMPLE_ROT13_API_H_

#include "tci.h"
#include "mcContainer.h"

/**
 * Command ID's for communication Trusted Application Connector (TAC)
 * -> Trusted Application (TA).
 */
#define CMD_SAMPLE_ROT13_CIPHER 1 /**< cipher in ROT13 TA  */
#define CMD_SAMPLE_ROT13_UNWRAP 2 /**< Unwrap data container in ROT13 TA */

/**
 * Return codes
 */
#define RET_ERR_ROT13_INVALID_LENGTH RET_CUSTOM_START + 1
#define RET_ERR_ROT13_UNWRAP RET_CUSTOM_START + 3

/**
 * Termination codes
 */
#define EXIT_ERROR ((uint32_t)(-1))

/**
 * Maximum data length.
 */
#define MAX_DATA_LEN sizeof(mcSoDataCont_t)

/**
 * ROT13 CIPHER and ROT13 UNWRAP command messages.
 *
 * The ROT13 CIPHER command will apply disguise data provided in the payload
 * buffer of the TCI. Only characters (a-z, A-Z) will be processed. Other
 * data is ignored and stays untouched.

 * The ROT13 UNWRAP command will unwrap in the TA the Secure Object Data
 * container provided in the buffer of the TCI.
 *
 * @param len Lenght of the data to process.
 * @param data Data to process (text to cipher or data container to unwrap).
 */
typedef struct {
    tciCommandHeader_t header; /**< Command header */
    uint32_t len; /**< Length of data to process */
} cmdRot13_t;

/**
 * Response structure TA -> TAC.
 */
typedef struct {
    tciResponseHeader_t header; /**< Response header */
    uint32_t len; /**< Length of data processed */
} rspRot13_t;

/**
 * TCI message data.
 */
typedef struct {
    union {
        tciCommandHeader_t command; /**< Command header */
        tciResponseHeader_t response; /**< Response header */
        cmdRot13_t cmdRot13;
        rspRot13_t rspRot13;
    };
    /**< Clear/Cipher text  or Wrap/Unwrap Data container */
    uint8_t data[MAX_DATA_LEN];
} tciMessage_t;

/**
 * TA UUID.
 */
#define TL_SAMPLE_ROT13_UUID { { 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // TL_SAMPLE_ROT13_API_H_
