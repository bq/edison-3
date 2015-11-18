/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup MCDRVTRUSTLETENDLESSLOOP 00301_Trustlet_Endless_Loop
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

TEST_SCENARIO(00301_Trustlet_Endless_Loop)
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
		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseDevice(MC_DEVICE_ID_DEFAULT));
	}
};
/**@description TLC tells Trustlet to go into infinite loop. Then TLC closes Session.
 * @pre 		Involved components running and functioning
 * @veri     	Session can be closed.
 * Test output is OK.
 */
TESTCASE(Runtime,00301_Trustlet_Endless_Loop,BC00100_TrustletInEndlessLoop)
{
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 0;       // No notifications
    tci->cmdTlTestNotify.exitCode = 0; // tlApiExit will not be called by TL
    tci->cmdTlTestNotify.crash = 0; // Don't crash
    tci->cmdTlTestNotify.loop = 1; // Go into endless loop

	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));

    printf("\n\tTrustlet had 2 seconds time in loop, now we close session.");
    sleep(2);
}/**
 * @}
 * @} */
