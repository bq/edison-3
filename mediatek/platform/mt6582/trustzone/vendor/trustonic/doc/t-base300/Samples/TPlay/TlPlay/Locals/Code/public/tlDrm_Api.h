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

#ifndef __TLDRMAPI_H__
#define __TLDRMAPI_H__


/**
 * Command ID's
 */
#define CMD_ID_RUN_DRM_TEST_OPEN_SESSION        1
#define CMD_ID_RUN_DRM_TEST_PROCESS_DRM_CONTENT 2
#define CMD_ID_RUN_DRM_TEST_CLOSE_SESSION       3
#define CMD_ID_RUN_DRM_TEST_CHECK_LINK          4
#define CMD_ID_RUN_DRM_TEST_UNKNOWN_CMD       100
/*... add more command ids when needed */


/**
 * Command message.
 *
 * @param len Length of the data to process.
 * @param data Data to be processed
 */
typedef struct {
    tciCommandHeader_t  header;     /**< Command header */

    /**
     * Data: Here a simple block is given, but the TLC can still pass to TA
     * frames for multiple encrypted areas defined as in TA<->Driver
     *  interface (tlApiDrmOffsetSizePair_t).
     */
    uint32_t            len;        /**< Length of data to process */
    uint32_t			data;		/**< data to process */

    /*
     * Crypto and Hw parameters:
     * SAMPLE: In this Sample, the IV is given at head of data, and
     * other parameters like algo/outputOffset/... are not used.
     */

    /*
     * Algorithm is implicitly know in this sample but could be
     * passed to TA from Nwd.
     */
} command_t;


/**
 * Response structure
 */
typedef struct {
    tciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} response_t;



/**
 * TCI message data.
 */
typedef struct {
    union {
        command_t     command;
        response_t    response;
    };
} tciMessageData_t, *tciMessage_ptr;


/**
 * Overall TCI structure.
 */
typedef struct {
    tciMessageData_t message;   /**< TCI message */
} tci_t;


/**
 * Trusted Application UUID
 */
#define DRM_TL_UUID { { 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


#endif // __TLDRMAPI_H__
