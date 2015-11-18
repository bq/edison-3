/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVGETSESSIONERRORCODE 00900_mcGetSessionErrorCode
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
#include "Mci/mcinq.h"

TEST_SCENARIO(00900_mcGetSessionErrorCode)
{
	mcResult_t x;
	tciMessage_t *tci;
    mcSessionHandle_t sessionHandle;

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
/**@description Ensures that no session error is returned if there is none.
 * @pre         Involved components running and functioning
 * @focus       mcGetSessionErrorCode() returns MC_DRV_OK if there is no error.
 * @veri        mcGetSessionErrorCode() returns MC_DRV_OK.<br>
 * No additional error code is set.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,GC00050_NoNotifications)
{
    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );
}
/**@description Ensures that trustlet Connector handles multiple notification of test trustlet successfully.
 * @pre 		Involved components running and functioning
 * @focus		mcWaitNotification handles 3 notifications of trustlet.
 * @veri     	mcWaitNotification() returns MC_DRV_OK.<br>
 * Response ID is set in message header on TCI.<br>
 * No additional error code is set.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,GC00100_MultipleNotifications)
{
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 3;       // Send notification
    tci->cmdTlTestNotify.exitCode = 0; // tlApiExit will not be called by TL
    tci->cmdTlTestNotify.crash = 0; // Don't crash
	tci->cmdTlTestNotify.loop = 0; // Don't loop

	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));

	int i=1;
	do {
		CHECK_DRVAPI_RETURNS_OK(
				mcWaitNotification(&sessionHandle, 1000));

		printf("\n\tReceived Notification #%i.",i);

		i++;
	} while (	(i<5)
			// Check if response ID set
			&& 	(RSP_ID(CMD_TEST_NOTIFY) != tci->rspTlTestNotify.rspHeader.responseId));

	// Check if Trustlet says ok
	CHECK_EQUAL( RET_OK, tci->responseHeader.returnCode );

	// Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );
}
/**@description Trustlet Connector handles the "last" notification of the Trustlet after the Trustlet terminates itself with a wrong exit code.
 * @pre 		Involved components running and functioning
 * @focus		mcGetSessionErrorCode is ERR_INVALID_EXIT_CODE as the last error.
 * @veri     	mcWaitNotification() returns MC_DRV_INFO_NOTIFICATION.<br>
 * The additional error code is set to -1: invalid exit code.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,GC00101_TrustletExits)
{
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
    CHECK_EQUAL( ERR_INVALID_EXIT_CODE, lastErr );
}
/**@description Trustlet Connector handles the notification of the TL after the TL causes a kernel exception.
 * @pre 		Involved components running and functioning
 * @focus		mcGetSessionErrorCode() returns "ERR_INVALID_OPERATION" 
 * @veri     	mcWaitNotification() returns MC_DRV_INFO_NOTIFICATION.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,GC00103_TrustletCrashes)
{
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 0;                   // Send no notifications
    tci->cmdTlTestNotify.exitCode = 0;              // do not exit
    tci->cmdTlTestNotify.crash = 1;                 // make trustlet cause a crash
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
    // Kernel exception shall be signaled
    CHECK_EQUAL( ERR_INVALID_OPERATION, lastErr );
}
/**@description TLC calls function with a NULL pointer session
 * @pre 		Involved components running and functioning
 * @focus		mcGetSessionErrorCode handles a null session. 
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,BC00104_SessionhandleNullPointer)
{
    int32_t lastErr;
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_INVALID_PARAMETER,
			mcGetSessionErrorCode(0, &lastErr));
}
/**@description TLC calls function with a NULL pointer lastErr
 * @pre 		Involved components running and functioning
 * @focus		mcGetSessionErrorCode handles a null last error pointer.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,BC00105_LastErrNullPointer)
{
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_INVALID_PARAMETER,
			mcGetSessionErrorCode(&sessionHandle, 0));
}
/**@description TLC calls function with a malformed handle, bad sessionId.
 * @pre 		Involved components running and functioning
 * @focus		mcGetSessionErrorCode handles an unknown session ID.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_SESSION.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,BC00106_SessionhandleInvalidId)
{
    int32_t lastErr;
	mcSessionHandle_t badSession = sessionHandle;
	badSession.sessionId = -1;
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_UNKNOWN_SESSION,
			mcGetSessionErrorCode(&badSession, &lastErr));
}
/**@description TLC calls function with a malformed handle, bad deviceId.
 * @pre 		Involved components running and functioning
 * @focus		mcGetSessionErrorCode handles an unknown device ID.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_DEVICE.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,00900_mcGetSessionErrorCode,BC00107_SessionhandleInvalidDevice)
{
    int32_t lastErr;
	mcSessionHandle_t badSession = sessionHandle;
	badSession.deviceId = -1;
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_UNKNOWN_DEVICE,
			mcGetSessionErrorCode(&badSession,&lastErr));
}
/**
 * @}
 * @} */
