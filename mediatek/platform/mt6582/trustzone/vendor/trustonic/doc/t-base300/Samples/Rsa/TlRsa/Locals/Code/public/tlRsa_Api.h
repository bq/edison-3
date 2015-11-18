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

#ifndef TLRSA_API_H_
#define TLRSA_API_H_

#include "tci.h"

/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */
#define CMD_RSA   1

/**
 * Return codes
 */
#define RET_RSA_OK                    0
#define RET_ERR_RSA_INVALID_OFFSET    RET_CUSTOM_START + 1
#define RET_ERR_RSA_INVALID_LENGTH    RET_CUSTOM_START + 2

/**
 * Termination codes
 */
#define EXIT_ERROR                      ((uint32_t)(-1))

/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to processed (cleartext or ciphertext).
 */
typedef struct {
    tciCommandHeader_t  header;   /**< Command header */
    uint32_t            len;      /**< Length of data to process */
} cmd_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    tciResponseHeader_t header;   /**< Response header */
    uint32_t            len;
} rsp_t;

/**
 * TCI message data.
 */
typedef struct {
  union {
    cmd_t     cmdRsa;
    rsp_t     rspRsa;
  };
    uint32_t            inputData;
    uint32_t            outputData;
} tciMessage_t;

/**
 * Trustlet UUID.
 */
#define TL_RSA_UUID { { 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


#endif // TLRSA_API_H_
