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

/*
 * @file   drfooIpch.c
 * @brief  Implements IPC handler of the driver.
 *
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "DrError.h"
#include "foo_hw_hal.h"
#include "drCommon.h"
#include "drfoo_Api.h"

#include "drApiMarshal.h"
#include "drSmgmt.h"

DECLARE_STACK(drIpchStack, 2048);

/* Static variables */
static dciMessage_t *message;

/*
 * IPC handler loop. this is the function where IPC messages are handled
 */
_NORETURN void drIpchLoop( void )
{
    /* Set IPC parameters for initial MSG_RD to IPCH */
    threadid_t              ipcClient = NILTHREAD;
    message_t               ipcMsg    = MSG_RD;
    uint32_t                ipcData   = 0;
    tlApiResult_t           tlRet     = TLAPI_OK;
    drMarshalingParam_ptr   pMarshal;
    taskid_t                taskid = drApiGetTaskid();
    uint8_t                 *text = NULL;
    uint32_t                x, y, result;
    tlApiFoo_ptr            FooData = NULL;

    for(;;)
    {
        /*
         * When called first time sends ready message to IPC server and
         * then waits for IPC requests
         */
        if (DRAPI_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData))
        {
            drDbgPrintLnf("[Driver Drfoo] drIpchLoop(): drApiIpcCallToIPCH failed");
            continue;
        }

        /* Dispatch request */
        switch(ipcMsg)
        {
            case MSG_CLOSE_TRUSTLET:
                /*
                 * Trustlet close message
                 */
                drDbgPrintLnf("[Driver Drfoo] drIpchLoop(): Acknowledging trustlet close");
                /* Close active sessions owned by trustlet, which is being shutdown */
                //For MSG_CLOSE_TRUSTLET message threadId is in ipcData parameter
                drSmgmtCloseSessionForThread(ipcData);
                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_CLOSE_DRIVER:
                /*
                 * Driver close message
                 */
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_RQ:
#ifdef DR_FEATURE_TL_API                
                /*
                 * Handle incoming IPC requests via TL API.
                 * Map the caller trustlet to access to the marshalling data
                 */
                pMarshal = (drMarshalingParam_ptr)drApiMapClientAndParams(
                                            ipcClient,
                                            ipcData);

                if(pMarshal)
                {
                    /* Process the request */
                    switch (pMarshal->functionId)
                    {

                    case FID_DR_OPEN_SESSION:
                        /*
                         * Handle open session request
                         */

                        pMarshal->sid = drSmgmtOpenSession(ipcClient);
                        /*
                         * Update IPC status as success. If there is an error with opening session
                         * invalid sid will indicatethat opening session failed
                         */
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_CLOSE_SESSION:
                        /*
                         * Handle close session request based on the sid provided by the client
                         */
                        drSmgmtCloseSession(pMarshal->sid);
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_INIT_DATA:
                        /*
                         * Initialize sesion data
                         */
                        if (E_DR_SMGMT_OK == drSmgmtSetSessionData(
                                pMarshal->sid,
                                ipcClient,
                                &(pMarshal->payload.FooData)))
                        {
                            tlRet = TLAPI_OK;
                        }
                        break;
                    case FID_DR_EXECUTE:
                        /*
                         * TODO: Read registry data compare threadids to make sure that client is allowed to use
                         * registry data. Then execute the command and update tlRet accordingly
                         */

                        drApiNotify();

                        FooData = drApiAddrTranslateAndCheck(pMarshal->payload.FooData); 
                        
                        if(FID_DRV_ADD_FOO == FooData->commandId)
                        {
                            drDbgPrintLnf("[Driver Drfoo] FID_DRV_ADD_FOO: x(%d), y(%d)", FooData->x, FooData->y);
                            HAL_FooAdd(FooData);
                        }

                        if(FID_DRV_SUB_FOO == FooData->commandId)
                        {
                            drDbgPrintLnf("[Driver Drfoo] FID_DRV_SUB_FOO: x(%d), y(%d)", FooData->x, FooData->y);
                            HAL_FooSub(FooData);
                        }

                        break;                    
                    default:
                        /* Unknown message has been received*/
                        break;
                    }
                }

                /* Update response data */
                ipcMsg  = MSG_RS;
                ipcData = 0;
                pMarshal->payload.retVal = tlRet;
#endif                
                break;
            default:
                drApiIpcUnknownMessage(&ipcClient, &ipcMsg, &ipcData);
                /* Unknown message has been received*/
                ipcMsg = (message_t) E_TLAPI_DRV_UNKNOWN;
                ipcData = 0;
                break;
        }
    }
}

/*
 * IPC handler thread entry point
 */
_THREAD_ENTRY void drIpch( void )
{
    drIpchLoop();
}

/**
 * IPC handler thread init
 */
void drIpchInit(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    /* ensure thread stack is clean */
    clearStack(drIpchStack);

    /* DCI message buffer */
    message = (dciMessage_t*) dciBuffer;

    /*
     * DCI buffer data can be accessed via message
     * pointer.
     */

    /*
     * Start IPC handler thread. Exception handler thread becomes local
     * exception handler of IPC handler thread
     */
    if (DRAPI_OK != drApiStartThread(
                    DRIVER_THREAD_NO_IPCH,
                    FUNC_PTR(drIpch),
                    getStackTop(drIpchStack),
                    IPCH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH))

    {
        drDbgPrintLnf("[Driver Drfoo] drIpchInit(): drApiStartThread failed");
    }
}
