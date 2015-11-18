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

#include "drSampleRot13Utils.h"
#include "drSampleRot13Common.h"


/* IPC handler stack */
EXTERNAL_STACK(drIpchStack)

/* External functions */
extern _NORETURN void drIpchLoop( void );


/**
 * Cleanup function
 */
static void doCleanup( void )
{
}


/**
 * Exception handler loop.
 */
_NORETURN void drExchLoop( void )
{
    threadno_t      faultedThread;
    threadid_t      ipcPartner;
    uint32_t        mr0, mr1, mr2;
    addr_t          ip;
    addr_t          sp;

    drDbgPrintLnf("[Driver DrSampleRot13] drExchLoop(): Exception handler thread is running");

    for (;;)
    {
        /* Wait for exception */
        if ( DRAPI_OK != drApiIpcWaitForMessage(
                     &ipcPartner,
                     &mr0,
                     &mr1,
                     &mr2) )
        {
            /* Unable to receive IPC message */
            drDbgPrintLnf("[Driver DrSampleRot13] drExchLoop(): drApiIpcWaitForMessage failed");
            continue;
        }

        /* mr0 holds threadid value of thread that caused the exception */
        faultedThread = GET_THREADNO(mr0);

        /* Process exception */
        switch(mr1)
        {
            //--------------------------------------
            case TRAP_SEGMENTATION:
                /* Check which thread caused exception */
                switch(faultedThread)
                {
                    //--------------------------------------
                    case DRIVER_THREAD_NO_IPCH:
                        /* Update sp and ip accordingly */
                        ip = FUNC_PTR(drIpchLoop);
                        sp = getStackTop(drIpchStack);

                        /* Resume thread execution */
                        if (DRAPI_OK != drUtilsRestartThread(
                                        faultedThread,
                                        ip,
                                        sp))
                        {
                            drDbgPrintLnf("[Driver DrSampleRot13] drExchLoop(): restarting thread failed");
                        }

                        break;
                    //--------------------------------------
                    default:
                        drDbgPrintLnf("[Driver DrSampleRot13] drExchLoop(): Unknown thread. This should never happen");
                        break;
                }

                break;
            //--------------------------------------
            case TRAP_ALIGNMENT:
            case TRAP_UNDEF_INSTR:
                /**
                 * This should never happen. If it does, do the cleanup and exit gracefully
                 */
                doCleanup();

                /* Stop IPC handler thread */
                if (DRAPI_OK != drApiStopThread(DRIVER_THREAD_NO_IPCH))
                {
                    drDbgPrintLnf("[Driver DrSampleRot13] drExchLoop(): Unable to stop IPC handler thread");
                }

                /* Stop main thread */
                if (DRAPI_OK != drApiStopThread(NILTHREAD))
                {
                    drDbgPrintLnf("[Driver DrSampleRot13] drExchLoop(): Unable to stop main thread");
                }

                /* Will not come to this point */
                break;
            //--------------------------------------
            default:
                /**
                 * Unknown exception occurred. Do cleanup in case.
                 */
                doCleanup();
                break;
        }
    }
}
