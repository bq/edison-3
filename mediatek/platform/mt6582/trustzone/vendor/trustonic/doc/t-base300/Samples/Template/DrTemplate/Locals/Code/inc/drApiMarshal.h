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
 * @file   drApiMarshal.h
 * @brief  Contains marshalling definitions and data structures
 *
 */

#ifndef __DRAPIMARSHAL_H__
#define __DRAPIMARSHAL_H__


#include "DrApi/DrApiCommon.h"
#include "TlApi/TlApiError.h"


/**
 * Driver ID. Update accordingly
 */
#define SAMPLE_DR_ID DRIVER_ID

/**
 * Function id definitions
 */
#define FID_DR_OPEN_SESSION     1
#define FID_DR_CLOSE_SESSION    2
#define FID_DR_INIT_DATA        3
#define FID_DR_EXECUTE          4
/* .. add more when needed */

/** Invalid session id. Returned in case of an error. */
#define DR_SID_INVALID      0xffffffff

/**
 * Sample 01 data structure
 */
typedef struct {
    uint32_t len;
    uint32_t addr;
} tlApiSampleData01_t, *tlApiSampleData01_ptr;


/**
 * Sample 02 data structure
 */
typedef struct {
    uint32_t data;
} tlApiSampleData02_t, *tlApiSampleData02_ptr;


/**
 * Union of marshaling parameters. */
/* If adding any function, add the marshaling structure here
 */
typedef struct {
    uint32_t commandId;
    union {
        tlApiSampleData01_t sampleData01;
        tlApiSampleData02_t sampleData02;
    };
} drMarshalingData_t, *drMarshalingData_ptr;


/**
 * Union of marshaling parameters. */
/* If adding any function, add the marshaling structure here
 */
typedef struct {
    uint32_t     functionId;    /** Function identifier. */
    union {
        drMarshalingData_t  sampleData;  /** Data */
        tlApiResult_t       retVal;      /** Return value */
    } payload;
    uint32_t     sid;           /** Session identifier */
} drMarshalingParam_t, *drMarshalingParam_ptr;


#endif // __DRAPIMARSHAL_H__

