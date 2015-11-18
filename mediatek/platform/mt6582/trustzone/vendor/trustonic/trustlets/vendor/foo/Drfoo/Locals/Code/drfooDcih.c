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
 * @file   drDciHandler.c
 * @brief  Implements DCI handler of the driver.
 *
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "drCommon.h"
#include "drUtils.h"
#include "drTemplate_Api.h"



DECLARE_STACK(drDcihStack, 2048);


/* Static variables */
static dciMessage_t *message;


/**
 * DCI handler loop. this is the function where notifications from Nwd are handled
 */
_NORETURN void drDcihLoop( void )
{
    dciCommandId_t  commandId;
    dciReturnCode_t ret;
    taskid_t        taskid = drApiGetTaskid();

    drDbgPrintLnf("[Driver DrTemplate] drDcihLoop(): DCI handler thread is running");

    for(;;)
    {
        /* Initialize return value */
        ret = RET_OK;
        
        drDbgPrintLnf("[Driver DrTemplate] Koshi drDcihLoop(): start to wait the signal from normal world");

        /* Wait for IPC signal */
        if (DRAPI_OK != drApiIpcSigWait())
        {
            drDbgPrintLnf("[Driver DrTemplate] drDcihLoop(): drApiIpcSigWait failed");
            continue;
        }

        /* Get commandid */
        commandId = message->command.header.commandId;
        
        drDbgPrintLnf("[Driver DrTemplate] Koshi drDcihLoop(): commandId(%d)", commandId);

        /* Get & process DCI command */
        switch( commandId )
        {
            /**
             * TODO: Add new command ids and and update existing ones when needed
             *
             */

            //--------------------------------------
            case CMD_ID_01:
                break;
            //--------------------------------------
            case CMD_ID_02:
                break;
            //--------------------------------------
            default:
                break;
        }

        message->response.header.responseId = RSP_ID(commandId);
        message->response.header.returnCode = ret;

        /* Notify Nwd */
        if (DRAPI_OK != drApiNotify())
        {
            drDbgPrintLnf("[Driver DrTemplate] drDcihLoop(): drApiNotify failed");
        }
        drDbgPrintLnf("[Driver DrTemplate] Koshi drDcihLoop(): drApiNotify success");
    }
}


/**
 * DCI handler thread entry point
 */
_THREAD_ENTRY void drDcih( void )
{
    drDcihLoop();
}


/**
 * DCI handler init
 */
void drDcihInit(
    const addr_t    dciBuffer,
    const uint32_t  dciBufferLen
){
    /* Ensure thread stack is clean */
    clearStack( drDcihStack );

    /* DCI message buffer */
    message = (dciMessage_t*) dciBuffer;

    /**
     * Update DCI handler thread so that notifications will be delivered
     * to DCI handler thread
     */
    if (DRAPI_OK != drUtilsUpdateNotificationThread( DRIVER_THREAD_NO_DCIH ))
    {
        drDbgPrintLnf("[Driver DrTemplate] drDcihInit(): Updating notification thread failed");
    }

    /**
     * Start DCI handler thread. EXH thread becomes local
     * exception handler of DCIH thread
     */
    if (DRAPI_OK != drApiStartThread(
                    DRIVER_THREAD_NO_DCIH,
                    FUNC_PTR(drDcih),
                    getStackTop(drDcihStack),
                    DCIH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH))

    {
            drDbgPrintLnf("[Driver DrTemplate] drDcihInit(): drApiStartThread failed");
    }
}
