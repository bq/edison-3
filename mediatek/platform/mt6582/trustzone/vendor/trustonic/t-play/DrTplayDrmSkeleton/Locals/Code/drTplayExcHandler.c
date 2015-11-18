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
#include "DrApi/DrApiLogging.h"
#include "DrApi/DrApiMm.h"
#include "DrApi/DrApiThread.h"
#include "DrApi/DrApiIpcMsg.h"

#include "drTplayCommon.h"
#include "drTplayUtils.h"


/* IPC handler stack */
EXTERNAL_STACK(drIpchStack)
/* DCI handler stack */
EXTERNAL_STACK(drDcihStack)

/* DCI handler entry function */
extern _NORETURN void drDcihLoop( void );

/* External functions */
extern _NORETURN void drIpchLoop( void );

extern void deinit_hw( void );


/**
 * Cleanup function
 */
static void doCleanup( void )
{
    /**
     * Update the function accordingly if cleanup is required.
     */
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

    dbgSN("[DRM Driver] drExchLoop(): Exception handler thread is running");

    for (;;)
    {
        /* Wait for exception */
        if ( E_OK != drApiIpcWaitForMessage(
                &ipcPartner,
                &mr0,
                &mr1,
                &mr2) )
        {
            /* Unable to receive IPC message */
            dbgSN("[DRM Driver] drExchLoop(): drApiIpcWaitForMessage failed");
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
                        if (E_OK != drUtilsRestartThread(
                                        faultedThread,
                                        ip,
                                        sp))
                        {
                            dbgSN("[DRM Driver] drExchLoop(): IPC restarting thread failed");
                        }
					//--------------------------------------
					case DRIVER_THREAD_NO_DCIH:
						/* Update sp and ip accordingly */
						ip = FUNC_PTR(drDcihLoop);
						sp = getStackTop(drDcihStack);

						/* Resume thread execution */
						if (E_OK != drUtilsRestartThread(
										faultedThread,
										ip,
										sp))
						{
							dbgSN("[DRM Driver] drExchLoop(): DCI restarting thread failed");
						}
						break;
                    //--------------------------------------
                    default:
                        dbgSN("[DRM Driver] drExchLoop(): Unknown thread. This should never happen");
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
                if (E_OK != drApiStopThread(DRIVER_THREAD_NO_IPCH))
                {
                    dbgSN("[DRM Driver] drExchLoop(): Unable to stop IPC handler thread");
                }

                /* Stop main thread */
                if (E_OK != drApiStopThread(NILTHREAD))
                {
                    dbgSN("[DRM Driver] drExchLoop(): Unable to stop main thread");
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

