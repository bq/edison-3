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
 * @file   drExcHandler.c
 * @brief  Implements exception handler of the driver.
 *
 * Responsible for handling exceptions cause by IPC and DCI handler threads.
 * Restarts the thread that causes specific exception. runs with higher priority
 * than IPC and DCI handler threads
 *
 * <Copyright goes here>
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "drCommon.h"
#include "drSmgmt.h"
#include "drUtils.h"

#ifdef DR_FEATURE_DCI_HANDLER
/* DCI handler stack */
EXTERNAL_STACK(drDcihStack)
#endif // DR_FEATURE_DCI_HANDLER

/* IPC handler stack */
EXTERNAL_STACK(drIpchStack)
EXTERNAL_STACK(timerIsrStack)

#ifdef DR_FEATURE_DCI_HANDLER
/* DCI handler entry function */
extern _NORETURN void drDcihLoop( void );
#endif // DR_FEATURE_DCI_HANDLER

/* IPC handler entry function */
extern _NORETURN void drIpchLoop( void );
extern _NORETURN void timerIsr( void );


/**
 * Cleanup function
 */
static void doCleanup( void )
{
    /* Close all sessions */
    drSmgmtCloseAllSessions();

    /**
     * TODO: Add cleanup code here
     *
     */
}


/**
 * Exception handler loop.
 */
_NORETURN void drExchLoop(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    threadno_t      faultedThread;
    threadid_t      ipcPartner;
    uint32_t        mr0, mr1, mr2;
    addr_t          ip = NULL;
    addr_t          sp = NULL;

    drDbgPrintLnf("[Driver DrAsyncExample] drExchLoop(): Exception handler thread is running");

    for (;;)
    {
        // wait for exception
        if ( DRAPI_OK != drApiIpcWaitForMessage(
                 &ipcPartner,
                 &mr0,
                 &mr1,
                 &mr2) )
        {
            /* Unable to receive IPC message */
            drDbgPrintLnf("[Driver DrAsyncExample] drExchLoop(): drApiIpcWaitForMessage failed");
            continue;
        }

        /* mr0 holds threadid value of thread that caused the exception */
        faultedThread = GET_THREADNO(mr0);

        /* Process exception */
        switch(mr1)
        {
            //--------------------------------------
            case TRAP_SEGMENTATION:
                /* Update ip and sp accordingly and restart the thread */
                switch(faultedThread)
                {
                    //--------------------------------------
                    case DRIVER_THREAD_NO_IPCH:
                        ip = FUNC_PTR(drIpchLoop);
                        sp = getStackTop(drIpchStack);
                        break;
                #ifdef DR_FEATURE_DCI_HANDLER
                    //--------------------------------------
                    case DRIVER_THREAD_NO_DCIH:
                        ip = FUNC_PTR(drDcihLoop);
                        sp = getStackTop(drDcihStack);
                        break;
                #endif // DR_FEATURE_DCI_HANDLER
                    //--------------------------------------
                    case DRIVER_THREAD_NO_IRQH:
                        ip = FUNC_PTR(timerIsr);
                        sp = getStackTop(timerIsrStack);
                        break;
                    default:
                        drDbgPrintLnf("[Driver DrAsyncExample] drExchLoop(): Unknown thread. This should never happen");
                        break;
                }

                if ((ip != NULL) && (sp != NULL))
                {
                    /* Set sp and ip accordingly and resume execution if DCIH thread */
                    if (DRAPI_OK != drUtilsRestartThread(
                                    faultedThread,
                                    ip,
                                    sp))
                    {
                        drDbgPrintLnf("[Driver DrAsyncExample] drExchLoop(): restarting thread failed");
                    }
                }

                break;
            //--------------------------------------
            case TRAP_ALIGNMENT:
            case TRAP_UNDEF_INSTR:
                /**
                 * This should never happen. If it does, do the cleanup and exit gracefully
                 */
                doCleanup();

            #ifdef DR_FEATURE_DCI_HANDLER
                /* Stop DCI handler thread */
                if (DRAPI_OK != drApiStopThread(DRIVER_THREAD_NO_DCIH))
                {
                    drDbgPrintLnf("[Driver DrAsyncExample] drExchLoop(): Unable to stop DCI handler thread");
                }
            #endif // DR_FEATURE_DCI_HANDLER

                /* Stop IPC handler thread */
                if (DRAPI_OK != drApiStopThread(DRIVER_THREAD_NO_IPCH))
                {
                    drDbgPrintLnf("[Driver DrAsyncExample] drExchLoop(): Unable to stop IPC handler thread");
                }

                /* Stop main thread */
                if (DRAPI_OK != drApiStopThread(NILTHREAD))
                {
                    drDbgPrintLnf("[Driver DrAsyncExample] drExchLoop(): Unable to stop main thread");
                }

                /* Will not come to this point */
                break;
            //--------------------------------------
            default:
                /**
                 * TODO: Update this sestion accordingly
                 *
                 * Unknown exception occured.Do cleanup in case
                 */
                doCleanup();
                break;
        }
    }
}

