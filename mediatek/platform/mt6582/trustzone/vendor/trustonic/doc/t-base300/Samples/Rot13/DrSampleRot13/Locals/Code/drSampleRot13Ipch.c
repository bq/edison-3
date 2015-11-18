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

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "TlApiMarshal_Rot13.h"
#include "TlApiRot13Error.h"
#include "Rot13Sample_hw_hal.h"
#include "drSampleRot13Common.h"


DECLARE_STACK(drIpchStack, 2048);



/**
 * IPC handler loop. this is the function where IPC messages are handled
 */
_NORETURN void drIpchLoop( void )
{
    /* Set IPC parameters for initial MSG_RD to IPCH */
    threadid_t            ipcClient = NILTHREAD;
    message_t             ipcMsg    = MSG_RD;
    uint32_t              ipcData   = 0;
    tlApiResult_t         tlRet     = TLAPI_OK;
    Rot13MarshalingParam_ptr pMarshal;
    taskid_t              taskid = drApiGetTaskid();
    uint8_t               *text = NULL;
    uint32_t              length;
    tlApiEncodeRot13_ptr  encodeRot13 = NULL;

    for (;;)
    {

        /*
         * When called first time sends ready message to IPC server and
         * then waits for IPC requests
         */
        if (DRAPI_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData))
        {
            drDbgPrintLnf("[Driver DrSampleRot13] drIpchLoop(): drApiIpcCallToIPCH failed");
            continue;
        }

        /* Dispatch request */
        switch (ipcMsg)
        {
            case MSG_CLOSE_TRUSTLET:
                /**
                 * Trustlet close message
                 */
                drDbgPrintLnf("[Driver DrSampleRot13] drIpchLoop(): Acknowledging trustlet close");
                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_CLOSE_DRIVER:
                /**
                 * Driver close message
                 */
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_RQ:

                /* init tlRet value */
                tlRet = TLAPI_OK;

                /**
                 * Handle incoming IPC requests via TL API.
                 * Map the caller trustlet to access to the marshalling data
                 */
                pMarshal = (Rot13MarshalingParam_ptr)drApiMapClientAndParams(
                                            ipcClient,
                                            ipcData);

                if (pMarshal)
                {
                    /* Process the request */
                    switch (pMarshal->functionId)
                    {
                    case FID_DRV_ENCODE_ROT13:
                        /**
                         * Handle cipher request
                         */
                        tlRet = drApiNotify();
                        if (tlRet != TLAPI_OK) {
                            break;
                        }

                        drDbgPrintLnf("[Driver DrSampleRot13] drIpchLoop(): Handling cipher request");
                        encodeRot13 = drApiAddrTranslateAndCheck(pMarshal->payload.encodeRot13);
                        length=encodeRot13->length;
                        text = drApiAddrTranslateAndCheck(encodeRot13->text);
                        if (!text)
                         {
                            tlRet = E_TLAPI_UNKNOWN_FUNCTION;
                            break;
                         }

                        drDbgPrintLnf((char *)text);
                        HAL_Rot13encrypt(length, text);
                        break;
                    default:
                        /* Unknown message has been received*/
                        tlRet = E_TLAPI_UNKNOWN_FUNCTION;
                        break;
                    }
                }

                /* Update response data */
                ipcMsg  = MSG_RS;
                ipcData = tlRet;
                break;
            default:
                /* Unknown message has been received*/
                ipcMsg  = MSG_RS;
                ipcData = E_TLAPI_DRV_UNKNOWN;
                break;
        }
    }
}


/**
 * IPC handler thread entry point
 */
_THREAD_ENTRY void drIpch( void )
{
    drIpchLoop();
}


/**
 * IPC handler thread init
 */
void drIpchInit( void )
{
    /* ensure thread stack is clean */
    clearStack(drIpchStack);

    /**
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
        drDbgPrintLnf("[Driver DrSampleRot13] drIpchInit(): drApiStartThread failed");
    }
}
