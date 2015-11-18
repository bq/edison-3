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

#ifndef TL_SAMPLE_SHA256_API_H_
#define TL_SAMPLE_SHA256_API_H_

#include "tci.h"

/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */
#define CMD_SAMPLE_SHA256	1	/**< SHA256 */

/**
 * Return codes
 */
#define RET_ERR_INVALID_BUFFER	RET_CUSTOM_START + 1
#define RET_ERR_ALGORITHM		RET_CUSTOM_START + 3

/**
 * Termination codes
 */
#define EXIT_ERROR	((uint32_t)(-1))

/**
 * SHA256 command message.
 */
typedef struct {
    tciCommandHeader_t header;	/**< Command header. */
    uint8_t* bulkBuffer;		/**< The virtual address of the bulk buffer regarding the address space of the Trustlet. */
    uint32_t srcOffs;			/**< The offset of the source data within the bulk buffer. */
	uint32_t srcLen;			/**< Length of the source data. */
} cmdSha256_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    tciResponseHeader_t header;	/**< Response header. */
    uint32_t hashLen;			/**< The length of the output hash (should be 32 byte). */
    uint8_t  hash[32];          /**< The 256 bit hash. */
} rspSha256_t;

/**
 * TCI message data.
 */
typedef union {
    tciCommandHeader_t	commandHeader;
    tciResponseHeader_t	responseHeader;
    cmdSha256_t         cmdSha256;
    rspSha256_t         rspSha256;
} tciMessage_t;

/**
 * Overall TCI structure.
 */
#define TCI_PAYLOAD_LEN 1024
typedef struct {
	tciMessage_t message;	/**< TCI message */
} tci_t;

/**
 * Trustlet UUID.
 */
#define TL_SAMPLE_SHA256_UUID { { 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // TL_SAMPLE_SHA256_API_H_
