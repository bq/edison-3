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
 * @file   tlTciHandler.c
 * @brief  Implements TCI handler functionality
 *
 * TCI data is processed and handled according to the command id and data
 * given by the associated TLC. Normal world is notified accordingly with
 * relevant return code after processing TCI command data
 *
 */

#include "tlStd.h"
#include "TlApi/TlApi.h"


#include "tlCommon.h"
#include "tlAsyncExample_Api.h"
#include "tlAsyncExampleDriverApi.h"

/**
 * TCI handler loop
 */
_NORETURN void tlTciHandler_Loop(
    const addr_t tciBuffer,
    const uint32_t tciBufferLen)
{
    tciReturnCode_t  ret = RET_OK;
    tciCommandId_t   commandId;
    uint32_t         sessionId = 0;
    uint32_t         count = 0;
    tci_t* pTci = (tci_t*) tciBuffer;

    /* The Trustlet main loop */
    for (;;) {

        //tlDbgPrintLnf("[Trustlet TlAsyncExample] Loop Start");
        /* Wait for notification from Driver or TLC */
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        /*
         * We got a notification. It can be 
           - from TLC only
           - from Driver only
           - from TLC and from Driver.
         */

	/*
	 * Check if it was a notification from TLC. Is there a command?
	 */

        /* Retrieve command id */
        commandId = pTci->message.command.header.commandId;
        //tlDbgPrintLnf("[Trustlet TlAsyncExample] command: 0x%08X",commandId);

        /* Process command message */
        switch (commandId)
            {
            case CMD_ID_RUN_ASYNC:
                if (0 == sessionId) {
                    sessionId= tlApi_Async_OpenSession();
                    if (DR_SID_INVALID == sessionId) {
                        tlDbgPrintLnf("[Trustlet TlAsyncExample] Open session failed");
                        break;
                    }
                    tlDbgPrintLnf("[Trustlet TlAsyncExample] Open session");
                }
                //tlDbgPrintLnf("[Trustlet TlAsyncExample] Start driver session: 0x%08X",sessionId);
                tlApi_Async_Start(sessionId); //Call driver
                /* clear command ID so we do not execute this command again: */
                pTci->message.command.header.commandId = 0; // command done
                ret = RET_OK;
                break;

            default:

            	/*
            	 * No command found in TCI - notification did not come from TLC.
            	 */

                /* Unknown command */
                ret = RET_ERR_UNKNOWN_CMD;
                break;
            }

        pTci->message.response.header.responseId = RSP_ID(commandId);

	/*
	 * Check if is was a notification from driver. Is there some data?
	 */
	// Do a callDriver here again to transfer data from driver to Trustlet.
	// See the count value that is now available in Trustlet after driver 
	// notification.
        count = tlApi_Async_NotificationQuery(sessionId);
        tlDbgPrintLnf("[Trustlet TlAsyncExample] Notification count: %d",count);
        if (count > 0) {
            pTci->message.response.irqCount += count;

        }
        else if (RET_ERR_UNKNOWN_CMD == ret) {
            /*
             * Notification was not command from TLC nor notification from driver
             */
            tlDbgPrintLnf("[Trustlet TlAsyncExample] Unknow or old Notification");
            // Here we continue and wait for next notification.
            // TLC is only notified when the driver sends at least one notification.
            continue;
        }

        pTci->message.response.header.returnCode = ret;
        /* Notify the TLC */
        tlDbgPrintLnf("[Trustlet TlAsyncExample] Notify NWd");
        tlApiNotify(); //Notify TLC
    }
}

