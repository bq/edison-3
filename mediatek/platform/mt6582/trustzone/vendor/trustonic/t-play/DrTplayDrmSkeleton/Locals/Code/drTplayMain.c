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


DECLARE_DRIVER_MAIN_STACK(4096);


/* External functions */
extern void drIpchInit( void );
extern _NORETURN void drExchLoop( void );


/**
 * Initialization function
 */
static drApiResult_t doInitialization( void )
{
    /**
     * Update the function accordingly if required
     */
    return E_OK;
}

/**
 * Main routine.
 * Starts the required threads.
 */
_DRAPI_ENTRY void drMain(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen )
    {
    dbgSN("[DRM Driver] drMain(): driver started");

    /* Initialization */
    if (E_OK != doInitialization())
    {
        dbgSN("[DRM Driver] drMain(): Initialization failed");

        /* No need to proceed further. Shutdown the main thread */
        if (E_OK != drApiStopThread(NILTHREAD))
        {
            dbgSN("[DRM Driver] drMain(): Unable to stop main thread");
        }
    }

    /* Start IPC handler */
    drIpchInit();

    /* Start DCI handler */
    drDcihInit(dciBuffer, dciBufferLen);

    /* Start exception handler */
    drExchLoop();

}
