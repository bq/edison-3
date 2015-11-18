/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup MCDYNAMICDRIVER 00500_DynamicDriver
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
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "TrustletForwarder.h"
#include "tlTestApi.h"
#include "DrIntegrationTestApi.h"
#include <strings.h>

#define TRUSTLET_SHARED_MEM_D1  (void*)0x100000
#define TCI_OFFSET_MARSHALLING_BUFFER 32

TEST_SCENARIO(00500_DynamicDriver)
{
	mcResult_t x;
	tciMessage_t *tci;
	tciMessage_t *tci_driver;
    mcSessionHandle_t sessionHandle;
    mcSessionHandle_t sessionHandle_driver;


	void setup(void) {
		FORWARDER_INIT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenDevice(MC_DEVICE_ID_DEFAULT));

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci, 0));

		// Clear the session handle
		bzero(&sessionHandle, sizeof(sessionHandle));

		// Set default device ID
		sessionHandle.deviceId = MC_DEVICE_ID_DEFAULT;

		mcUuid_t uuid = TL_TEST_UUID;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle, &uuid, (uint8_t *)tci, (uint32_t) sizeof(tciMessage_t)));

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

	}
	void teardown (void)
	{
		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseDevice(MC_DEVICE_ID_DEFAULT));
	}
};

/**@description Trustlet Connector notifies Trustlet and allocate 1 resource
 * Trustlet just answers with accept message, no error message
 * @pre 		Involved components running and functioning
 * @veri     	mcWaitNotification() returns MC_DRV_OK.<br>
 * @veri        returnCode == RET_OK
 * No additional error code is set.<br>
 * Test output is OK.
 */
TESTCASE(Runtime,00500_DynamicDriver,GC00100_DriverAllocOk)
{
	uint8_t *marshalling = &((uint8_t*)tci)[TCI_OFFSET_MARSHALLING_BUFFER];
	cmd_allocateResource_t       *cmd_allocateResource = (cmd_allocateResource_t*) marshalling;
	rsp_allocateResource_t       *rsp_allocateResource = (rsp_allocateResource_t*) marshalling;

    // Prepare command message
    tci->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
    tci->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
	cmd_allocateResource->cmdHeader.commandId = CMD_ALLOCATE_RESOURCE;

	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));

    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));


    CHECK_EQUAL( RSP_ID(CMD_ALLOCATE_RESOURCE) , rsp_allocateResource->rspHeader.responseId);
    CHECK_EQUAL( rsp_allocateResource->rspHeader.returnCode , RET_OK);
    printf("rsp_allocateResource->resId=%x\n",rsp_allocateResource->resId);
    CHECK( rsp_allocateResource->resId != 0);

    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );
}

/**@description Trustlet Connector notifies Trustlet and allocate 2 resource
 * Trustlet answers first with accept message, no error message
 * Trustlet answers first with not accept message, error message
 * @pre 		Involved components running and functioning
 * @veri     	mcWaitNotification() returns MC_DRV_OK.<br>
 * @veri        first returnCode == RET_OK
 * No additional error code is set.<br>
 * Test output is OK.
 */
TESTCASE(Runtime,00500_DynamicDriver,BC00101_DriverAllocTooMuch)
{
	uint8_t *marshalling = &((uint8_t*)tci)[TCI_OFFSET_MARSHALLING_BUFFER];
	cmd_allocateResource_t       *cmd_allocateResource = (cmd_allocateResource_t*) marshalling;
	rsp_allocateResource_t       *rsp_allocateResource = (rsp_allocateResource_t*) marshalling;

    // Prepare command message
    tci->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
    tci->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
	cmd_allocateResource->cmdHeader.commandId = CMD_ALLOCATE_RESOURCE;

	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));

    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));


    CHECK_EQUAL( RSP_ID(CMD_ALLOCATE_RESOURCE) , rsp_allocateResource->rspHeader.responseId);
    CHECK_EQUAL( rsp_allocateResource->rspHeader.returnCode , RET_OK);
    CHECK( rsp_allocateResource->resId != 0);

    // Prepare command message 2.time allocate
    tci->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
    tci->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
	cmd_allocateResource->cmdHeader.commandId = CMD_ALLOCATE_RESOURCE;

	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));

    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));


    CHECK_EQUAL( RSP_ID(CMD_ALLOCATE_RESOURCE) , rsp_allocateResource->rspHeader.responseId);
    CHECK( rsp_allocateResource->rspHeader.returnCode != RET_OK);
    CHECK_EQUAL( rsp_allocateResource->resId , 0);

    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );

}


/**@description Testcase without doing anything
 * Open/CloseSession without any duration between
 * @pre 		Involved components running and functioning
 * @veri     	mcWaitNotification() returns MC_DRV_OK.<br>
 * @veri        first returnCode == RET_OK
 * No additional error code is set.<br>
 * Test output is OK.
 */

TESTCASE(Runtime,00500_DynamicDriver,GC00102_DoNothing)
{

}

/**
 * @}
 * @} */
