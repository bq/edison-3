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
 * @file   drIpcHandler.c
 * @brief  Implements IPC handler of the driver.
 *
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "drApiMarshal.h"
#include "drCommon.h"
#include "drSmgmt.h"
#include "drAsyncExample_Api.h"
#include "asyncExample.h"

/* Thread specific definitions */
#define threadid_is_null(threadid)  (NILTHREAD == threadid)  /* returns true if thread id is NILTHREAD */

DECLARE_STACK(drIpchStack, 2048);

/* Static variables */
static dciMessage_t *message;

/* Global variables */
threadid_t gLastIpcClient = NILTHREAD;
extern testSession_t testSession[MAX_DR_SESSIONS];



/**
 * IPC handler loop. this is the function where IPC messages are handled
 */
 _NORETURN void drIpchLoop( void )
{
    /* Set IPC parameters for initial MSG_RD to IPCH */
    threadid_t     ipcClient = NILTHREAD;
    message_t      ipcMsg    = MSG_RD;
    uint32_t       ipcData   = 0;
    tlApiResult_t  tlRet     = E_TLAPI_DRV_UNKNOWN;
    drMarshalingParam_ptr pMarshal;
    uint32_t r;
    drSessionReg_ptr drSession;
    uint32_t sid;
    volatile uint32_t tmp;


    /**
     * Check if there is a pending client. If there is, this is an
     * indication that IPC handler thread crashed before completing
     * the request. Respond with failure.
     */
    if (!threadid_is_null(gLastIpcClient))
    {
        ipcClient = gLastIpcClient;
        ipcMsg    = MSG_RS;
        ipcData   = E_TLAPI_DRV_UNKNOWN;
    }

    for (;;)
    {
        /*
         * When called first time sends ready message to IPC server and
         * then waits for IPC requests
         */
        if (DRAPI_OK != (r = drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData)))
        {
            drDbgPrintLnf("[Driver DrAsyncExample] drIpchLoop(): drApiIpcCallToIPCH failed 0x%08X",r);
            continue;
        }

        /* Update last IPC client */
        gLastIpcClient = ipcClient;

        //drDbgPrintLnf("[Driver DrAsyncExample] drIpchLoop() ipMsg: 0x%08X",ipcMsg);
        /* Dispatch request */
        switch (ipcMsg)
        {
            case MSG_CLOSE_TRUSTLET:
                /* Close active sessions owned by trustlet, which is being shutdown */
                //For MSG_CLOSE_TRUSTLET message threadId is in ipcData parameter
                drSmgmtCloseSessionForThread(ipcData);

                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = 0;

                break;
            case MSG_CLOSE_DRIVER:
                /* Acknowledge */
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = 0;
                break;
            case MSG_RQ:
            #ifdef DR_FEATURE_TL_API

                /**
                 * Handle incoming IPC requests via TL API.
                 * Map the caller trustlet to access to the marshalling data
                 */
                pMarshal = (drMarshalingParam_ptr)drApiMapClientAndParams(
                                            ipcClient,
                                            ipcData);

                if (pMarshal) {
                    /* Process the request */
                    switch (pMarshal->functionId)
                    {
                    case FID_DR_OPEN_SESSION:
                        /**
                         * Handle open session request
                         */
                        sid= drSmgmtOpenSession(ipcClient);
                        pMarshal->sid = sid;
                        memset(&testSession[sid], 0, sizeof(testSession_t));
                        drDbgPrintf("[Driver DrAsyncExample] Open Session: 0x%08X",sid);
                        drDbgPrintLnf(" client: 0x%08X",ipcClient);
                        pMarshal->payload.retVal = TLAPI_OK;;

                        break;

                    case FID_DR_CLOSE_SESSION:
                        /**
                         * Handle close session request based on the sid provided by the client
                         */
                        drDbgPrintf("[Driver DrAsyncExample] Close Session: 0x%08X",pMarshal->sid);
                        drDbgPrintLnf(" client: 0x%08X",ipcClient);
                        drSmgmtCloseSession(pMarshal->sid);

                        pMarshal->payload.retVal = TLAPI_OK;
                        break;


                    case FID_DR_EXECUTE:
                        sid = pMarshal->sid;
                        drSession= drSmgmtGetSessionData(sid);

                        if (SESSION_STATE_ACTIVE == drSession->state) {

                               taskid_t task= drApiGetTaskid();
                               uint32_t r = drApiIpcSignal(drApiTaskidGetThreadid(task,DRIVER_THREAD_NO_IRQH));
                               //drDbgPrintLnf("[Driver DrAsyncExample] send signal return value: 0x%08X",r);
                               tlRet = TLAPI_OK;
                            }

                        else {
                            drDbgPrintLnf("[Driver DrAsyncExample] Session not open ");
                        }
                        pMarshal->payload.retVal = tlRet;

                        break;

                    case FID_DR_ASK_NOT:
                        // Trustet asks how many notifications has been sent
                        sid= pMarshal->sid;
                        /* wCounter can be incremented in any time on other thread.
                         * The value of it can change during execution of following lines
                         */
                        tmp = testSession[sid].wCounter;
                        pMarshal->payload.retVal= tmp - testSession[sid].rCounter;
                        testSession[sid].rCounter = tmp;

                        break;

                    default:
                        /* Unknown message has been received*/
                        break;
                    }
                }

                /* Update response data */
                ipcMsg  = MSG_RS;
                ipcData = 0;
            #endif // DR_FEATURE_TL_API
                break;
            default:
                /* Unknown message has been received*/
                ipcMsg = (message_t) E_TLAPI_DRV_UNKNOWN;
                ipcData = 0;
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
 * IPC handler init
 */
void drIpchInit(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    uint32_t r;
    /* ensure thread stack is clean */
    clearStack(drIpchStack);

    /* IPC message buffer */
    message = (dciMessage_t*) dciBuffer;

    /**
     * IPC buffer data can be accessed via message
     * pointer.
     */

    /**
     * Start IPC handler thread. Exception handler thread becomes local
     * exception handler of IPC handler thread
     */
    if (DRAPI_OK != (r = drApiStartThread(
                    DRIVER_THREAD_NO_IPCH,
                    FUNC_PTR(drIpch),
                    getStackTop(drIpchStack),
                    IPCH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH)))

    {
            drDbgPrintLnf("[Driver DrAsyncExample] drIpchInit(): drApiStartThread failed 0x%08X",r);
    }
}
