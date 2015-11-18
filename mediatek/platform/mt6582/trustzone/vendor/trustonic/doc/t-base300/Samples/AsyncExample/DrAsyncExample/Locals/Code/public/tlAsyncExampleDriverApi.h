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
 * @file   tlAsyncExampleDriverApi.h
 * @brief  Contains trustlet API definitions
 *
 */

#ifndef __TLDRIVERAPI_H__
#define __TLDRIVERAPI_H__

#include "tlStd.h"
#include "TlApi/TlApiError.h"


/**
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlApi_Async_OpenSession( void );


/**
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApi_Async_CloseSession( uint32_t sid );



/**
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApi_Async_Start(
        uint32_t sid
        );

/**
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C uint32_t tlApi_Async_NotificationQuery(
        uint32_t sid);

#endif // __TLDRIVERAPI_H__
