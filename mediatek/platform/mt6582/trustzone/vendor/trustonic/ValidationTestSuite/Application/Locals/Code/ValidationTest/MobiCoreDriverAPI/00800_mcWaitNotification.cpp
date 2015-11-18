/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVWAITNOTIFICATION 00800_mcWaitNotification
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
 * <b>Generic preconditions (setup)</b><br>
 * Generic Preconditions are the same for every test case.
 * They are executed always in front of every single test case.
 * - open device using the default ID by calling mcOpenDevice()
 * - allocate a WSM buffer (4k)
 * - open a session to the Trustlet
 *
 * <b>Generic post processing (teardown)</b><br>
 * Generic post processing is the same for every test case.
 * It is executed always after every single test case.
 * - close session to the Trustlet
 * - free allocated memory of WSM buffer
 * - close Device by calling mcCloseDevice()
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "TrustletForwarder.h"
#include "tlTestApi.h"
#include <strings.h>

TEST_SCENARIO(00800_mcWaitNotification)
{
	tciMessage_t *tci;
    mcSessionHandle_t sessionHandle;
	mcResult_t x;

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
	}
	void teardown (void)
	{
		mcCloseSession(&sessionHandle);
		mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci);
		mcCloseDevice(MC_DEVICE_ID_DEFAULT);
	}
};
/**@description Trustlet Connector notifies Trustlet and waits infinitely for a notification of the Trustlet (response)
 * @pre 		Involved components running and functioning
 * @focus		mcWaitNotification arbitrarily awaits for a notfication of an empty message.
 * @veri     	mcWaitNotification() returns MC_DRV_OK.<br>
 * Response ID is set in message header on TCI.<br>
 * No additional error code is set.<br>
 * Test output is OK.
 * @post		
 */
TESTCASE(McDrvApi,00800_mcWaitNotification,GC00100_NotifyThenTimeoutInfinite)
{
    // Prepare command message
	// TODO: TEST these inputs for arbitrary numbers
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 0; // Send no notification
    tci->cmdTlTestNotify.exitCode = 0; // tlApiExit will not be called by TL
    tci->cmdTlTestNotify.crash = 0; // Don't crash
	tci->cmdTlTestNotify.loop = 0; // Don't loop

	CHECK_DRVAPI_RETURNS_OK(
		mcNotify(&sessionHandle));

	// Wait for notification from Trustlet
	mcResult_t x = mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT);

    // Handle a bad case that happened during integration testing
    if(MC_DRV_INFO_NOTIFICATION == x)
    {
        printf("\n\nHandling a bad case that happened during integration testing\n");
        printf("CHECK_EQUAL( RSP_ID(CMD_TEST_NOTIFY)=%x, tci->rspTlTestNotify.rspHeader.responseId=%x\n",
        		RSP_ID(CMD_TEST_NOTIFY),
        		tci->rspTlTestNotify.rspHeader.responseId);
        printf("CHECK_EQUAL( RET_OK =%x, tci->responseHeader.returnCode=%x );\n",
        		RET_OK,
        		tci->responseHeader.returnCode );
        int32_t lastErr;
        x = mcGetSessionErrorCode(&sessionHandle, &lastErr);
        printf("x=%x = mcGetSessionErrorCode(&sessionHandle, &lastErr=%i);\n",
        		x,
        		lastErr);
        FAIL("Aborting now.");
    }
    CHECK_DRVAPI_RETURNS_OK( x);

    // Check if response ID set
    CHECK_EQUAL( RSP_ID(CMD_TEST_NOTIFY), tci->rspTlTestNotify.rspHeader.responseId );

    // Check if Trustlet says ok
    CHECK_EQUAL( RET_OK, tci->responseHeader.returnCode );

    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    	mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );
}
/**@description Trustlet Connector notifies Trustlet and waits 1 second for a notification of the Trustlet (response)
 * @pre 		Involved components running and functioning
 * @focus		mcWaitNotification awaits 1 sec for a rnotification without an early timeout.
 * @veri     	mcWaitNotification() returns MC_DRV_OK.<br>
 * Response ID is set in message header on TCI.<br>
 * No additional error code is set.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,00800_mcWaitNotification,GC00101_NotifyThenTimeoutFinite)
{
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 0; // Send no notification
    tci->cmdTlTestNotify.exitCode = 0; // tlApiExit will not be called by TL
    tci->cmdTlTestNotify.crash = 0; // Don't crash
	tci->cmdTlTestNotify.loop = 0; // Don't loop

	CHECK_DRVAPI_RETURNS_OK(
		mcNotify(&sessionHandle));

	// TODO: TEST check the time elapsed (Really ~1 sec?)

	// Wait for notification from Trustlet
	CHECK_DRVAPI_RETURNS_OK(
		mcWaitNotification(&sessionHandle, 1000));
	
	// Check if response ID set
    CHECK_EQUAL( RSP_ID(CMD_TEST_NOTIFY), tci->rspTlTestNotify.rspHeader.responseId );

    // Check if Trustlet says ok
    CHECK_EQUAL( RET_OK, tci->responseHeader.returnCode );

    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    	mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );
}
/**@description Trustlet Connector does not notify Trustlet and waits 1 second for a notification of the Trustlet (response)
 * @pre 		Involved components running and functioning
 * @focus		
 * @veri     	mcWaitNotification() returns MC_DRV_ERR_TIMEOUT.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,00800_mcWaitNotification,GC00102_NoNotifyThenTimeoutFinite)
{
	// Wait for notification from Trustlet
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_TIMEOUT,
			mcWaitNotification(&sessionHandle, 1000));
}
/**@description Trustlet Connector does not notify Trustlet and waits infinitely for a notification of the Trustlet (response)
 * @pre         Involved components running and functioning
 * @focus		mcWaitNotification handles a no notification case.
 * @veri        mcWaitNotification() returns MC_DRV_ERR_NOTIFICATION.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,00800_mcWaitNotification,BC00103_NoNotifyThenThenNoTimeout)
{
	// Wait for notification from Trustlet
	// TODO: TEST check the time elapsed (Really ~1 sec?)
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_TIMEOUT,
			mcWaitNotification(&sessionHandle, MC_NO_TIMEOUT));
}
/**@description TLC calls function with a NULL pointer
 * @pre 		Involved components running and functioning
 * @focus		mcWaitNotification handles a NULL parameter case.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00800_mcWaitNotification,BC00104_SessionhandleNullPointer)
{
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_INVALID_PARAMETER,
			mcWaitNotification(0, MC_INFINITE_TIMEOUT));
}
/**@description TLC calls function with a malformed handle, bad sessionId.
 * @pre 		Involved components running and functioning
 * @focus		mcWaitNotification handles an invalid session ID case.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_SESSION.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00800_mcWaitNotification,BC00105_SessionhandleInvalidId)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.sessionId = -1;
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_UNKNOWN_SESSION,
			mcWaitNotification(&badSession,MC_INFINITE_TIMEOUT));
}
/**@description TLC calls function with a malformed handle, bad deviceId.
 * @pre 		Involved components running and functioning
 * @focus		mcWaitNotification handles an invalid device ID case.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_DEVICE.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00800_mcWaitNotification,BC00106_SessionhandleInvalidDevice)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.deviceId = -1;
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_UNKNOWN_DEVICE,
			mcWaitNotification(&badSession,MC_INFINITE_TIMEOUT));
}
/**
 * @}
 * @} */
