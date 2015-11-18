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
 * @file   tlAsynctest_Api.h
 * @brief  Contains TCI command definitions and data structures
 *
 */

#ifndef __TLDRMAPI_H__
#define __TLDRMAPI_H__

#include "tci.h"



/**
 * Command ID's
 */

#define CMD_ID_RUN_ASYNC   1

#define RESPONSE_NOTIFY    0x10

/*... add more command ids when needed */
/** Invalid session id. Returned in case of an error. */
#define DR_SID_INVALID      0xffffffff
//#define MAX_DATA_LEN 128
/**
 * Command message.
 *
 * @param len Length of the data to process.
 * @param data Data to be processed
 */
typedef struct {
    tciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process */
} command_t;


/**
 * Response structure
 */
typedef struct {
    tciResponseHeader_t header;     /**< Response header */
    uint32_t            irqCount;
} response_t;



/**
 * TCI message data.
 */
typedef struct {
    struct {
        command_t     command;
        response_t    response;
    };
} tciMessage_t, *tciMessage_ptr;


/**
 * Overall TCI structure.
 */
typedef struct {
    tciMessage_t message;   /**< TCI message */
} tci_t;


/**
 * Trustlet UUID
 */
#define ASYNC_EXAMPLE_TL_UUID  { { 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // __TLDRMAPI_H__
