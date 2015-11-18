/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup MCDRVCOMMANDTESTS 00100_TrustletCommands
 * @{
 *
 * <b>Involved Components and their required Functionality</b>
 *
 * -# ClientLib
 *      -# Can connect to daemon
 *      -# Can reserve memory for connection
 * -# McDriverDaemon
 *      -# Daemon running
 *      -# Daemon socket accessible by TLC
 *      -# mcOpenDevice()
 * -# McDriverModule
 *      -# Module loaded
 *      -# Module socket accessible by daemon
 *      -# Module listens for connection
 * -# Trustlet TlIntegrationTest
 *      -# Trustlet copied to Android on Test device in /data/app/ as  05010000000000000000000000000000.trst
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "TrustletForwarder.h"
#include "tlTestApi.h"
#include <strings.h>

TEST_SCENARIO(00100_TrustletCommands)
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

/**@description Trustlet Connector notifies Trustlet and waits infinitely for a notification of the Trustlet (response)
 * TLC uses a bad command, with RSP_ID set.
 * Trustlet just answers back, no error message
 * @pre         Session has been opened by setup()
 * @focus       Use mcNotify() to send a valid command to the trustlet<br>
                Wait for notification from Trustlet
 * @veri        mcWaitNotification() returns MC_DRV_OK.<br>
 * No additional error code is set.<br>
 * @post        Session is closed by teardown()
 */
TESTCASE(Runtime,00100_TrustletCommands,BC00100_TrustletCommandHasRespIdSet)
{
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = RSP_ID(CMD_TEST_NOTIFY);
    tci->cmdTlTestNotify.num = 0; // Send no notification
    tci->cmdTlTestNotify.exitCode = 0; // tlApiExit will not be called by TL
    tci->cmdTlTestNotify.crash = 0; // Don't crash
    tci->cmdTlTestNotify.loop = 0; // Don't loop

    CHECK_DRVAPI_RETURNS_OK(
    		mcNotify(&sessionHandle));

    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

    // Check if response ID set
    CHECK_EQUAL( RSP_ID(CMD_TEST_NOTIFY), tci->rspTlTestNotify.rspHeader.responseId );

    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );
}

/**@description Trustlet Connector notifies Trustlet and waits infinitely for a notification of the Trustlet (response)
 * TLC uses a bad command, with RSP_ID set.
 * Trustlet just answers back, no error message
 * @pre         Session has been opened by setup()
 * @focus       Use mcNotify() to send an unknown command to the trustlet<br>
                Wait for notification from Trustlet
 * @veri        mcWaitNotification() returns MC_DRV_OK.<br>
 * check that tci->responseHeader.returncode == RET_ERR_UNKNOWN_CMD<br>
 * No additional error code is set.<br>
 * @post        Session is closed by teardown()
 */
TESTCASE(Runtime,00100_TrustletCommands,BC00101_UnknownTestCommand)
{
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_COMPARE_MEM+1;
    tci->cmdTlTestNotify.num = 0; // Send no notification
    tci->cmdTlTestNotify.exitCode = 0; // tlApiExit will not be called by TL
    tci->cmdTlTestNotify.crash = 0; // Don't crash
    tci->cmdTlTestNotify.loop = 0; // Don't loop

    CHECK_DRVAPI_RETURNS_OK(
    		mcNotify(&sessionHandle));

    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

    // Check if response ID set
    CHECK_EQUAL( RSP_ID(CMD_TEST_COMPARE_MEM+1), tci->rspTlTestNotify.rspHeader.responseId );

    CHECK_EQUAL( RET_ERR_UNKNOWN_CMD, tci->responseHeader.returnCode );

    // Request error code, must be "no error"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( 0, lastErr );
}

/**@description Trustlet does nothing (Immediate open/close session). Setup() opens the session. TestCase does nothing and Teardown() closes the session.
 * @pre         Session has been opened by setup()
 * @focus       -
 * @veri        -
 * @post        Session is closed by teardown()
 */
TESTCASE(Runtime,00100_TrustletCommands,GC00102_DoNothing)
{
}
/**
 * @}
 * @} */
