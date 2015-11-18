/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup MCDYNAMICDRIVER4 00503_DynamicDriver4
 * @{
 *
 * <b>Involved Components and their required Functionality</b>
 *
 * -# ClientLib
 * 		-# Can connect to daemon
 * 		-# Can reserve memory for connection
 * -# McDriverDaemon
 * 		-# Daemon running
 * 		-# Daemon socket accessible by TLC
 * 		-# mcOpenDevice()
 * -# DrIntegrationTest Driver
 * 		-# Module loaded
 * 		-# Module socket accessible by daemon
 * 		-# Module listens for connection
 * -# Trustlet TlTestIntegrationTest
 * 		-# Trustlet copied to Android on Test device in /data/app/ as  05010000000000000000000000000000.trst
 * 		-# Trustlet runnable and bug-free
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */


#include <stdlib.h>
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "tlTestApi.h"
#include "DrIntegrationTestApi.h"
#include "Mci/mcinq.h"
#include <strings.h>

#define _TLAPI_EXTERN_C extern "C"
#include <sys/mman.h>
#include "TrustletForwarder.h"

#define LOG_WANT_BUF
#define LOG_TAG "Test_DynamicDriver"
#include <log.h>


#define TCI_OFFSET_MARSHALLING_BUFFER 32
#define BULK_BUF_LEN 0x100000		// 1 MB


TEST_SCENARIO(00503_DynamicDriver4)
{
	mcResult_t x;
	tciMessage_t *tci1;
	tciMessage_t *tci2;
	tciMessage_t *tci_driver;
    mcSessionHandle_t sessionHandle1;
    mcSessionHandle_t sessionHandle2;
    mcSessionHandle_t sessionHandle_driver;

	void setup(void) {
		FORWARDER_INIT;


	}
	void teardown (void)
	{
		// Close session
	}

    bool checkdebug()
    {
    #ifndef NDEBUG
        {
        	return true;
        }
    #endif
        return false;
    }

};



/**@description Trustlet Connector reopens multiple time trustlet+driver with Ping to trustlet only
 * @pre 		Involved components running and functioning
 * Testflow.<br>
 */

TESTCASE(Runtime,00503_DynamicDriver4,GC00100_MultipleDriverLoading_WithoutCall)
{
	mcUuid_t uuid = TL_TEST_UUID;
	uint32_t maxIndex;

	if(checkdebug())
		maxIndex = 100;
	else
		maxIndex = 1000;

    printf("\n\r"); fflush(stdout);

	for(uint32_t index=0; index<maxIndex; index++)
	{
		if(checkdebug())
		{
		   printf("\r\tMultiple Driver Loading %d from %d (Debug)",index+1,maxIndex);
		   fflush(stdout);
		}
		else
		{
		   printf("\r\tMultiple Driver Loading %d from %d (Release)",index+1,maxIndex);
		   fflush(stdout);
		}

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenDevice(MC_DEVICE_ID_DEFAULT));

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci1, 0));

		// Clear the session handle
		bzero(&sessionHandle1, sizeof(sessionHandle1));

		// Set default device ID
		sessionHandle1.deviceId = MC_DEVICE_ID_DEFAULT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle1, &uuid, (uint8_t *)tci1, (uint32_t) sizeof(tciMessage_t)));


		//!!!THP faked driver as trustlet
		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci_driver, 0));


		// Clear the session handle
		bzero(&sessionHandle_driver, sizeof(sessionHandle_driver));

		// Set default device ID
		sessionHandle_driver.deviceId = MC_DEVICE_ID_DEFAULT;

		mcUuid_t uuid_driver = DRV_DRINTEGRATIONSTEST_UUID;
		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle_driver, &uuid_driver, (uint8_t *)tci_driver, (uint32_t) sizeof(tciMessage_t)));


		// Prepare command message
		tci1->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_PING;

		CHECK_DRVAPI_RETURNS_OK(
				mcNotify(&sessionHandle1));

		CHECK_DRVAPI_RETURNS_OK(
				mcWaitNotification(&sessionHandle1, MC_INFINITE_TIMEOUT));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle1));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci1));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseDevice(MC_DEVICE_ID_DEFAULT));
	}
    //printf("\n\r");

}



/**@description Trustlet Connector reopens multible time trustlet+driver with ping trustlet->driver
 * @pre 		Involved components running and functioning
 * Testflow.<br>
 */

TESTCASE(Runtime,00503_DynamicDriver4,GC00101_MultipleDriverLoading_WithCall)
{
	mcUuid_t uuid = TL_TEST_UUID;
	uint32_t maxIndex;

	if(checkdebug())
		maxIndex = 100;
	else
		maxIndex = 1000;

	CHECK_DRVAPI_RETURNS_OK(
			mcOpenDevice(MC_DEVICE_ID_DEFAULT));

    printf("\n\r"); fflush(stdout);

	for(uint32_t index=0; index<maxIndex; index++)
	{
		if(checkdebug())
		{
		   printf("\r\tMultiple Driver Loading %d from %d (Debug)",index+1,maxIndex);
		   fflush(stdout);
		}
		else
		{
		   printf("\r\tMultiple Driver Loading %d from %d (Release)",index+1,maxIndex);
		   fflush(stdout);
		}

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci1, 0));

		uint8_t *marshalling = &((uint8_t*)tci1)[TCI_OFFSET_MARSHALLING_BUFFER];
		cmd_ping_t       *cmd_ping = (cmd_ping_t*) marshalling;
		rsp_ping_t       *rsp_ping = (rsp_ping_t*) marshalling;

		// Clear the session handle
		bzero(&sessionHandle1, sizeof(sessionHandle1));

		// Set default device ID
		sessionHandle1.deviceId = MC_DEVICE_ID_DEFAULT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle1, &uuid, (uint8_t *)tci1, (uint32_t) sizeof(tciMessage_t)));


		//!!!THP faked driver as trustlet
		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci_driver, 0));


		// Clear the session handle
		bzero(&sessionHandle_driver, sizeof(sessionHandle_driver));

		// Set default device ID
		sessionHandle_driver.deviceId = MC_DEVICE_ID_DEFAULT;

		mcUuid_t uuid_driver = DRV_DRINTEGRATIONSTEST_UUID;
		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle_driver, &uuid_driver, (uint8_t *)tci_driver, (uint32_t) sizeof(tciMessage_t)));


        // Prepare command message
        tci1->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
        tci1->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
        cmd_ping->cmdHeader.commandId = CMD_PING;

		CHECK_DRVAPI_RETURNS_OK(
				mcNotify(&sessionHandle1));

		CHECK_DRVAPI_RETURNS_OK(
				mcWaitNotification(&sessionHandle1, MC_INFINITE_TIMEOUT));

	    CHECK_EQUAL( RSP_ID(CMD_PING) , rsp_ping->rspHeader.responseId);
	    CHECK_EQUAL( rsp_ping->rspHeader.returnCode , RET_OK);


		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle1));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci1));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci_driver));

	}
    //printf("\n\r");
	CHECK_DRVAPI_RETURNS_OK(mcCloseDevice(MC_DEVICE_ID_DEFAULT));

}

/**@description Trustlet Connector reopens multible time trustlet+driver with ping trustlet->driver with 2 trustlets
 * @pre 		Involved components running and functioning
 * Testflow.<br>
 */

TESTCASE(Runtime,00503_DynamicDriver4,GC00102_MultipleDriverLoading_WithCall_2Trustlets)
{
	mcUuid_t uuid = TL_TEST_UUID;
	uint32_t maxIndex;

	if(checkdebug())
		maxIndex = 50;
	else
		maxIndex = 500;

	CHECK_DRVAPI_RETURNS_OK(
			mcOpenDevice(MC_DEVICE_ID_DEFAULT));

    printf("\n\r"); fflush(stdout);

	for(uint32_t index=0; index<maxIndex; index++)
	{
		if(checkdebug())
		{
		   printf("\r\tMultiple Driver Loading %d from %d (Debug)",index+1,maxIndex);
		   fflush(stdout);
		}
		else
		{
		   printf("\r\tMultiple Driver Loading %d from %d (Release)",index+1,maxIndex);
		   fflush(stdout);
		}


		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci1, 0));

		uint8_t *marshalling1 = &((uint8_t*)tci1)[TCI_OFFSET_MARSHALLING_BUFFER];
		cmd_ping_t       *cmd_ping1 = (cmd_ping_t*) marshalling1;
		rsp_ping_t       *rsp_ping1 = (rsp_ping_t*) marshalling1;

		// Clear the session handle
		bzero(&sessionHandle1, sizeof(sessionHandle1));

		// Set default device ID
		sessionHandle1.deviceId = MC_DEVICE_ID_DEFAULT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle1, &uuid, (uint8_t *)tci1, (uint32_t) sizeof(tciMessage_t)));

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci2, 0));

		uint8_t *marshalling2 = &((uint8_t*)tci2)[TCI_OFFSET_MARSHALLING_BUFFER];
		cmd_ping_t       *cmd_ping2 = (cmd_ping_t*) marshalling2;
		rsp_ping_t       *rsp_ping2 = (rsp_ping_t*) marshalling2;

		// Clear the session handle
		bzero(&sessionHandle2, sizeof(sessionHandle1));

		// Set default device ID
		sessionHandle2.deviceId = MC_DEVICE_ID_DEFAULT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle2, &uuid, (uint8_t *)tci2, (uint32_t) sizeof(tciMessage_t)));

		//!!!THP faked driver as trustlet
		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci_driver, 0));

		// Clear the session handle
		bzero(&sessionHandle_driver, sizeof(sessionHandle_driver));

		// Set default device ID
		sessionHandle_driver.deviceId = MC_DEVICE_ID_DEFAULT;

		mcUuid_t uuid_driver = DRV_DRINTEGRATIONSTEST_UUID;
		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle_driver, &uuid_driver, (uint8_t *)tci_driver, (uint32_t) sizeof(tciMessage_t)));


        // Prepare command message
        tci1->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
        tci1->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
        cmd_ping1->cmdHeader.commandId = CMD_PING;

		
		CHECK_DRVAPI_RETURNS_OK(
				mcNotify(&sessionHandle1));

		
		CHECK_DRVAPI_RETURNS_OK(
				mcWaitNotification(&sessionHandle1, MC_INFINITE_TIMEOUT));


        // Prepare command message
        tci2->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
        tci2->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
        cmd_ping2->cmdHeader.commandId = CMD_PING;

		CHECK_DRVAPI_RETURNS_OK(
				mcNotify(&sessionHandle2));

		CHECK_DRVAPI_RETURNS_OK(
				mcWaitNotification(&sessionHandle2, MC_INFINITE_TIMEOUT));


		CHECK_EQUAL( RSP_ID(CMD_PING) , rsp_ping2->rspHeader.responseId);
	    CHECK_EQUAL( rsp_ping2->rspHeader.returnCode , RET_OK);


		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle1));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci1));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle2));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci2));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci_driver));

	}
    //printf("\n\r");
	CHECK_DRVAPI_RETURNS_OK(
			mcCloseDevice(MC_DEVICE_ID_DEFAULT));
}
/**
 * @}
 * @} */
