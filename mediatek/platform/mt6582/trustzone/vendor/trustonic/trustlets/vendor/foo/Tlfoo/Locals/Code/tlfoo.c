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
 * @file   tlfoo.c
 * @brief  Trustlet implementation
 *
 * TCI data is processed and handled according to the command id and data
 * given by the associated TLC. Normal world is notified accordingly with
 * relevant return code after processing TCI command data
 *
 */

#include "tlStd.h"
#include "TlApi/TlApi.h"
#include "tlfoo_Api.h"

#include "tlDriverApi.h"
#include "tlApifoo.h"

// Reserve 2048 byte for stack.
DECLARE_TRUSTLET_MAIN_STACK(2048);

/*
 * Add data
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t add(tciMessage_t* message)
{
    tlApiCrSession_t  crSession;
    tlApiResult_t     tlRet;
    tciReturnCode_t   ret = RET_OK;
    tlApiFoo_t        FooData;

    tlDbgPrintLnf("Proceeding with addition in SWd");
    tlDbgPrintLnf("FOO: (%d + %d)", message->Num1, message->Num2);

    FooData.commandId = FID_DRV_ADD_FOO;
    FooData.x = message->Num1;
    FooData.y = message->Num2;

    do
    {
        // Calling driver with IPC. 
      	// Call waits for the answer and blocks the trustlet. 
      	// The function is located inside drfoo.lib
      	// that is implemented and built within drfoo 	
        tlDbgPrintLnf("[Trustlet Tlfoo] &FooData %p", &FooData);

        crSession = tlApiOpenSession();

      	tlApiExecute(crSession, &FooData);

        tlApiCloseSession(crSession);

        tlDbgPrintLnf("[Trustlet Tlfoo] result %d", FooData.result);

        message->ResultData = FooData.result;
        
		break;
    }while(false);
    
    tlDbgPrintLnf("FOO: Result (%d)", message->ResultData);

    return ret;
}

/*
 * Minus data
 *
 * @return RET_OK if operation has been successfully completed.
 */
static tciReturnCode_t minus(tciMessage_t* message)
{
    tlApiCrSession_t  crSession;
    tlApiResult_t     tlRet;
    tciReturnCode_t   ret = RET_OK;
    tlApiFoo_t        FooData;

    tlDbgPrintLnf("Proceeding with minus in SWd");
    tlDbgPrintLnf("FOO: (%d - %d)", message->Num2, message->Num1);
        
    FooData.commandId = FID_DRV_SUB_FOO;
    FooData.x = message->Num1;
    FooData.y = message->Num2;
    
    do
    {
        // Calling driver with IPC. 
      	// Call waits for the answer and blocks the trustlet. 
      	// The function is located inside drfoo.lib
      	// that is implemented and built within drfoo 	
        tlDbgPrintLnf("[Trustlet Tlfoo] &FooData %p", &FooData);

        crSession = tlApiOpenSession();

      	tlApiExecute(crSession, &FooData);

        tlApiCloseSession(crSession);

        tlDbgPrintLnf("[Trustlet Tlfoo] result %d", FooData.result);

        message->ResultData = FooData.result;
        
		break;
    }while(false);
    
    tlDbgPrintLnf("FOO TL: Result (%d)", message->ResultData);

    return ret;
}


/**
 * Trustlet entry.
 */
_TLAPI_ENTRY void tlMain( const addr_t tciBuffer, const uint32_t tciBufferLen )
{
    tciReturnCode_t ret;
    tciCommandId_t commandId;

    tlApiLogPrintf("[<t Trustlet TlSampleRot13WithDriver], Build " __DATE__ ", " __TIME__ EOL);

    dbgSN("FOO trustlet is starting");

    {
        uint32_t tlApiVersion;
        mcVersionInfo_t versionInfo;

        ret = tlApiGetVersion(&tlApiVersion);
        if (TLAPI_OK != ret) 
        {
            tlDbgPrintLnf("tlApiGetVersion failed with ret=0x%08X, exit", ret);
            tlApiExit(ret);
        }
        tlDbgPrintLnf("tlApi version 0x%08X, exit", tlApiVersion);

        ret = tlApiGetMobicoreVersion(&versionInfo);
        if (TLAPI_OK != ret) 
        {
            tlDbgPrintLnf("tlApiGetMobicoreVersion failed with ret=0x%08X, exit", ret);
            tlApiExit(ret);
        }
        tlDbgPrintLnf("productId        = %s",     versionInfo.productId);
        tlDbgPrintLnf("versionMci       = 0x%08X", versionInfo.versionMci);
        tlDbgPrintLnf("versionSo        = 0x%08X", versionInfo.versionSo);
        tlDbgPrintLnf("versionMclf      = 0x%08X", versionInfo.versionMclf);
        tlDbgPrintLnf("versionContainer = 0x%08X", versionInfo.versionContainer);
        tlDbgPrintLnf("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
        tlDbgPrintLnf("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
        tlDbgPrintLnf("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
        tlDbgPrintLnf("versionCmp       = 0x%08X", versionInfo.versionCmp);
    }

    /* Check if the size of the given TCI is sufficient */
    if ((NULL == tciBuffer) || (sizeof(tciMessage_t) > tciBufferLen))
    {
        dbgSN("TCI error");
        dbgSPN("TCI buffer: ", tciBuffer);
        dbgSPN("TCI buffer length: ", tciBufferLen);
        dbgSDN("sizeof(tciMessage_t): ", sizeof(tciMessage_t));
        tlApiExit(EXIT_ERROR);
    }

    dbgSN("FOO trustlet is processing the command");
    tciMessage_t* message = (tciMessage_t*) tciBuffer;

    for (;;)
    {
        dbgSN("FOO trustlet is waiting for a notification to arrive");

        /* Wait for a notification to arrive */
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        /* Dereference commandId once for further usage */
        commandId = message->cmdfoo.header.commandId;

        tlDbgPrintLnf("[Trustlet Tlfoo]Got a message, commandId=0x%08X", commandId);

        /* Check if the message received is (still) a response */
        if (!IS_CMD(commandId)) 
        {
            tlApiNotify();
            continue;
        }

        /* Process command message */
        switch (commandId) 
        {
            case CMD_FOO_ADD:
              ret = add(message);
              break;
            case CMD_FOO_SUB:
              ret = minus(message);
              break;
            default:
              /* Unknown command ID */
              ret = RET_ERR_UNKNOWN_CMD;
              break;
	    }
	
		/* Set up response header */
		message->rspfoo.header.responseId = RSP_ID(commandId);
		message->rspfoo.header.returnCode = ret;
		
		dbgSDN("FOO trustlet is exiting with status ", ret);
		
		/* Notify back the TLC */
		tlApiNotify();
	}
}
