/**
 * @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVNOTIFY 00700_mcNotify
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

TEST_SCENARIO(00700_mcNotify)
{
	mcResult_t x;
    uint8_t *tci;
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
				mcOpenSession(&sessionHandle, &uuid, tci, (uint32_t) sizeof(tciMessage_t)));
	}
	void teardown (void)
	{
		
		mcCloseSession(&sessionHandle);
		mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);
		mcCloseDevice(MC_DEVICE_ID_DEFAULT);
	}
};
/**@description Trustlet Connector notifies Trustlet of a new command.<BR>
 *Ensures the noitification has been performed successfully.
 * @pre 		Involved components running and functioning
 * @focus		mcNotify is performed for normal parameters.
 * @veri     	returns MC_DRV_OK.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,00700_mcNotify,GC00100_notifyOnce)
{
	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));
}
/**@description TLC calls function with a NULL pointer
 * @pre 		Involved components running and functioning
 * @focus		mcNotify handles a NULL parameter.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00700_mcNotify,BC00101_SessionhandleNullPointer)
{
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_INVALID_PARAMETER,
			mcNotify(0));
}
/**@description TLC calls function with an unknown sessionId.
 * @pre 		Involved components running and functioning
 * @focus		mcNotify handles an unknown  session ID.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_SESSION.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,00700_mcNotify,BC00102_SessionhandleInvalidId)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.sessionId = -1;
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_UNKNOWN_SESSION,
			mcNotify(&badSession));
}
/**@description TLC calls function with a malformed handle, bad deviceId.
 * @pre 		Involved components running and functioning
 * @focus		mcNotify handles an unknown  device ID.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_DEVICE.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,00700_mcNotify,BC00103_SessionhandleInvalidDevice)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.deviceId = -1;
	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_UNKNOWN_DEVICE,
			mcNotify(&badSession));
}
/**
 * @}
 * @} */
