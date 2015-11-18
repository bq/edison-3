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

#ifndef TLAES_H_
#define TLAES_H_

#include "tci.h"

/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */
#define CMD_AES_ENCRYPT   1
#define CMD_AES_DECRYPT   2
#define CMD_AES_WRAP      3
#define CMD_AES_UNWRAP    4

/**
 * Return codes
 */
#define RET_ERR_AES_INVALID_OFFSET    RET_CUSTOM_START + 1
#define RET_ERR_AES_INVALID_LENGTH    RET_CUSTOM_START + 2

/**
 * Termination codes
 */
#define EXIT_ERROR                  ((uint32_t)(-1))

/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to processed (cleartext or ciphertext).
 */
typedef struct {
    tciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process or buffer */
    uint32_t            respLen;    /**< Length of response buffer */
} cmd_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    tciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} rsp_t;

/**
 * TCI message data.
 */
typedef struct {
    union {
      cmd_t     cmdAes;
      rsp_t     rspAes;
    };
    uint32_t    plainData;
    uint32_t    cipherData;
} tciMessage_t;

/**
 * Trustlet UUID.
 */
#define TL_AES_UUID { { 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


#endif // TLAES_H_
