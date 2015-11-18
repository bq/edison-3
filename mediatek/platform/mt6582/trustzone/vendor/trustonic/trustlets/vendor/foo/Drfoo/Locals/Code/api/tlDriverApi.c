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
 * @file   tlDriverApi.c
 * @brief  Implements driver APIs
 *
 * The APIs allow trustlets to make requests to the driver
 *
 */


#include "tlDriverApi.h"
#include "drApiMarshal.h"



/* Sends a MSG_RQ message via IPC to a MobiCore driver.
 *
 * @param driverID The driver to send the IPC to.
 * @param pMarParam MPointer to marshaling parameters to send to the driver.
 *
 * @return TLAPI_OK
 * @return E_TLAPI_COM_ERROR in case of an IPC error.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApi_callDriver(
        uint32_t driver_ID,
        void* pMarParam);



/**
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlApiOpenSession()
{
    uint32_t sid = DR_SID_INVALID;

    drMarshalingParam_t marParam = {
			.functionId = FID_DR_OPEN_SESSION,
	};

	if (TLAPI_OK == tlApi_callDriver(FOO_DRV_ID, &marParam))
	{
	    /* Retrieve session id provided by the driver */
	    sid = marParam.sid;
	}

	return sid;
}

/**
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiCloseSession(
        uint32_t sid)
{
    drMarshalingParam_t marParam = {
            .functionId = FID_DR_CLOSE_SESSION,
            .sid = sid
    };

    tlApiResult_t ret = tlApi_callDriver(FOO_DRV_ID, &marParam);
    return ret;
}

/**
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiExecute(
        uint32_t sid,
        tlApiFoo_ptr FooData)
{
    drMarshalingParam_t marParam = 
    {
        .functionId = FID_DR_EXECUTE,
        .sid        = sid,
        .payload    = 
        {
            .FooData = FooData
        }
    };

    tlApiResult_t ret = tlApi_callDriver(FOO_DRV_ID, &marParam);
    return ret;
}

#ifdef NEVER
_TLAPI_EXTERN_C tlApiResult_t tlApiAddFoo(tlApiFoo_ptr FooData)
{
    drMarshalingParam_t marParam = 
    {
        .functionId = FID_DRV_ADD_FOO,
        .payload    = 
        {
            .FooData = 
            {
                .x = FooData->x,
                .y = FooData->y,
                .result = FooData->result
            }
        }
    };
    
    tlApiResult_t ret = tlApi_callDriver(FOO_DRV_ID, &marParam);
}

_TLAPI_EXTERN_C tlApiResult_t tlApiSubFoo(tlApiFoo_ptr FooData)
{
    drMarshalingParam_t marParam = 
    {
        .functionId = FID_DRV_SUB_FOO,
        .payload    = 
        {
            .FooData = 
            {
                .x = FooData->x,
                .y = FooData->y,
                .result = FooData->result
            }
        }
    };
    
    tlApiResult_t ret = tlApi_callDriver(FOO_DRV_ID, &marParam);
}
#endif /* NEVER */