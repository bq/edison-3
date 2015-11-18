/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup MCDRVDEADTRUSTLET 00300_Dead_Trustlet
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
 * -# McDriverModule
 * 		-# Module loaded
 * 		-# Module socket accessible by daemon
 * 		-# Module listens for connection
 * -# Trustlet TlIntegrationTest
 * 		-# Trustlet copied to Android on Test device in /data/app/ as  05010000000000000000000000000000.trst
 * 		-# Trustlet runnable and bug-free
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "TrustletForwarder.h"
#include "tlTestApi.h"
#include <strings.h>
#include "Mci/mcinq.h"

#define BULK_BUF_LEN 0x004000		// 32K

TEST_SCENARIO(00300_Dead_Trustlet)
{
	mcResult_t x;
	tciMessage_t *tci;
    mcSessionHandle_t sessionHandle;

    /**
     * Setup: TLC handles the "last" notification of the Trustlet after the Trustlet terminates itself with a wrong exit code.
     */
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

		//Now we tell the Trustlet to terminate itself.

	    // Prepare command message
	    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
	    tci->cmdTlTestNotify.num = 0;       // Send notification
	    tci->cmdTlTestNotify.exitCode = (uint32_t)(-10);  // negative exit codes are not allowed.
	    tci->cmdTlTestNotify.crash = 0; // Don't crash
		tci->cmdTlTestNotify.loop = 0; // Don't loop

		CHECK_DRVAPI_RETURNS_OK(
				mcNotify(&sessionHandle));

		CHECK_DRVAPI_RETURNS(
				MC_DRV_INFO_NOTIFICATION,
				mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

	    // Request error code
	    int32_t lastErr = 0;
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
	    //TODO: replace -1 with correct #define
	    CHECK_EQUAL( -1, lastErr );

	    // Trustlet is dead here and we know it.
	}
	void teardown (void)
	{
		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseDevice(MC_DEVICE_ID_DEFAULT));
	}
};
/**@description
 * Afterwards TLC tries to notify the Trustlet
 * Then it waits for the Trustlet
 * @pre 		Involved components running and functioning
 * @veri     	mcWaitNotification() returns MC_DRV_INFO_NOTIFICATION.<br>
 * The additional error code is set to -1: invalid exit code.<br>
 * mcNotify returns no error
 * mcWaitNotification returns an error
 * Test output is OK.
 */
TESTCASE(Runtime,00300_Dead_Trustlet,BC00100_NotifyDeadTrustletWaitInfinite)
{
    // Trustlet is dead here and we know it.
    // However, we try to notify it again

    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 0; // Send no notification
    tci->cmdTlTestNotify.exitCode = 0; // tlApiExit will not be called by TL
    tci->cmdTlTestNotify.crash = 0; // Don't crash
	tci->cmdTlTestNotify.loop = 0; // Don't loop

	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));

    // No we try to wait for the dead Trustlet
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_INFO_NOTIFICATION,
    		mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( ERR_SID_NOT_ACTIVE, lastErr );
}
/**@description
 * Afterwards TLC tries to map bulk memory to the Trustlet<br>
 * <br>
 * See also Test10_mcMap_BC_pagefault_on_wrong_address for a
 * Test of mcUnmap() on a dead trustlet.
 * @pre 		Involved components running and functioning
 * @veri     	mcMap() returns MC_DRV_ERR_DAEMON_UNREACHABLE.
 * @note        This test also tests RTM ARM exception handling.
 * Test output is OK.
 */
TESTCASE(Runtime,00300_Dead_Trustlet,BC00101_MapMemoryToDeadTrustlet)
{
    // Trustlet is dead here and we know it.
    // However, we try to map something to it
	// Allocate Heap memory
	void *bulkBuf;
	mcBulkMap_t mapInfo;

	bulkBuf = malloc(BULK_BUF_LEN);
    CHECK( NULL != bulkBuf );

    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_DAEMON_UNREACHABLE,
    		mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, &mapInfo));

	free(bulkBuf);
}
/**
 * @}
 * @} */
