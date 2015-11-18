/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVOPENSESSION 00500_mcOpenSession
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

TEST_SCENARIO(00500_mcOpenSession)
{
	mcResult_t x;
    uint8_t *tci;
    mcSessionHandle_t sessionHandle;
	mcUuid_t uuid;

	void setup(void) {
		FORWARDER_INIT;
	    CHECK_DRVAPI_RETURNS_OK(
	            mcOpenDevice(MC_DEVICE_ID_DEFAULT));

		// Allocate WSM buffer (4k)
        CHECK_DRVAPI_RETURNS_OK(
                mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci, 0));
                
      if(TESTPARAM==1)
      {
		// Clear the session handle
		bzero(&sessionHandle, sizeof(sessionHandle));

		// Set default device ID
		sessionHandle.deviceId = MC_DEVICE_ID_DEFAULT;
      }
      
		mcUuid_t uuid2 = TL_TEST_UUID;
		uuid = uuid2;
	}
	void teardown (void)
	{
		if(TESTPARAM==1)
      {
         mcCloseSession(&sessionHandle);

		   mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);
		   mcCloseDevice(MC_DEVICE_ID_DEFAULT);
		}
		else if(TESTPARAM==2)
		{
		   CHECK_DRVAPI_RETURNS_OK(
			   mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci));
		   CHECK_DRVAPI_RETURNS_OK(
			   mcCloseDevice(MC_DEVICE_ID_DEFAULT));
		}
		else if(TESTPARAM==3)
		{
			// Close session
			CHECK_DRVAPI_RETURNS_OK(mcCloseSession(&sessionHandle));

			// free shared memory
			mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);
			// close device for other tests sake
			mcCloseDevice(MC_DEVICE_ID_DEFAULT);
		}

	}
};

/**@description TLC opens a session to a trustlet
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession opens a session successfully 
 * @veri     	mcOpenSession() returns MC_DRV_OK,<br>
 * @post		
 * sessionId is not zero and not -1,<br>
 * Most significant bit in sessionId is not set,<br>
 * Test output is OK
 */
TESTCASE(McDrvApi,00500_mcOpenSession,GC00100_05010000000000000000000000000000,1)
{
	// Open a session on default MobiCore device
	// Session IDs must be according to [LowInt]
    CHECK_DRVAPI_RETURNS_OK(
            mcOpenSession(&sessionHandle, &uuid, tci, (uint32_t) sizeof(tciMessage_t)));
    // 0 is reserved for MCP session
	CHECK( 0 != sessionHandle.sessionId );
    // 0xffffffff is invalid
	CHECK( 0xffffffff != sessionHandle.sessionId );
    // MSB must not be set
    CHECK_EQUAL( 0,(sessionHandle.sessionId & (1U << 31)) );
}
/**@description TLC opens a session with NULL pointer parameter for session
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for a null session parameter.
 * @veri     	mcOpenSession() returns MC_DRV_ERR_INVALID_PARAMETER, Test output is OK
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00101_SessionNullPointer,1)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_INVALID_PARAMETER,
            mcOpenSession(0, &uuid, tci, (uint32_t) sizeof(tciMessage_t)));
}
/**@description TLC opens a session with NULL pointer parameter for uuid
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for a null uuid parameter.
 * @veri     	mcOpenSession() returns MC_DRV_ERR_INVALID_PARAMETER, Test output is OK
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00102_UuidNullPointer,1)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_INVALID_PARAMETER,
            mcOpenSession(&sessionHandle, 0, (uint8_t *) tci, (uint32_t) sizeof(tciMessage_t)));
}
/**@description TLC opens a session with NULL pointer parameter for tci
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for a null tci parameter.
 * @veri     	mcOpenSession() returns MC_DRV_ERR_INVALID_PARAMETER, Test output is OK
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00103_TciNullPointer,1)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_INVALID_PARAMETER,
            mcOpenSession(&sessionHandle, &uuid, 0, (uint32_t) sizeof(tciMessage_t)));
}
/**@description TLC opens a session with an invalid len parameter (len=MC_MAX_TCI_LEN+1)
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for a buffer exceeding length parameter.
 * @veri     	mcOpenSession() returns MC_DRV_ERR_INVALID_PARAMETER, Test output is OK
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00104_LenBiggerMax,1)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_INVALID_PARAMETER,
            mcOpenSession(&sessionHandle, &uuid, (uint8_t *) tci, MC_MAX_TCI_LEN+1));
}
/**@description TLC opens a session with an invalid len parameter (len=-1)
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for a negative length parameter.
 * @veri     	mcOpenSession() returns MC_DRV_ERR_INVALID_PARAMETER, Test output is OK
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00105_LenMinus1,1)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_INVALID_PARAMETER,
            mcOpenSession(&sessionHandle, &uuid, (uint8_t *) tci, -1));
}
/**@description TLC opens a session with an invalid len parameter (len=0)
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for a zero length parameter.
 * @veri     	mcOpenSession() returns MC_DRV_OK, Test output is OK.<br>
 * @note 		The outcome is rather unexpected, is a len of 0 ok?
 * Actually, the session is created, but the Trustlet exits immediately.
 * Exit can be recognized with mcWaitNotification() and mcGetSessionErrorCode()
 * However, the session has to be closed anyhow
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00106_Len0,1)
{
    CHECK_DRVAPI_RETURNS(MC_DRV_ERR_INVALID_PARAMETER,
            mcOpenSession(&sessionHandle, &uuid, (uint8_t *) tci, 0));
//    CHECK_EQUAL( MC_DRV_ERR_INVALID_PARAMETER, x );
}
/**@description TLC opens a session with an invalid device id (-1)
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for an unknown device ID.
 * @veri     	mcOpenSession() returns MC_DRV_ERR_UNKNOWN_DEVICE.<br> Test output is OK.
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00107_SessionhandleInvalid,1)
{
	sessionHandle.deviceId = -1;
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_UNKNOWN_DEVICE,
            mcOpenSession(&sessionHandle, &uuid, (uint8_t *) tci, (uint32_t) sizeof(tciMessage_t)));
}
/**@description TLC opens a session with an invalid UUID and that should be impossible.
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession  returns an error for an invalid uuid parameter.
 * @veri     	mcOpenSession() returns MC_DRV_ERR_UNKNOWN_DEVICE.<br> Test output is OK.
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00108_UuidUnknown1234123412341234,1)
{
	const mcUuid_t uuid2 = { { 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4 } };
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_INVALID_DEVICE_FILE,
            mcOpenSession(&sessionHandle, &uuid2, (uint8_t *) tci, (uint32_t) sizeof(tciMessage_t)));
	//LOG_I("Wait 2 sec ...  (should be sufficient on most platforms!)");
}


/**@description TLC opens 2 sessions, then out of memory
 * @pre 		Involved components running and functioning
 * @focus		mcMallocWsm  and mcOpenSession return errors to avoid memory exhaustion.
 * @veri     	Returns 2 times MC_DRV_OK, then MC_DRV_ERR_INVALID_DEVICE_FILE.<br> Test output is OK.
 * @note        This test also tests RTM ARM exception handling, at least on some boards.
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,GC00109_MaximumSessions,2)
{
	printf("\n");
#define MAX_SESSIONS 3
	mcSessionHandle_t a[MAX_SESSIONS];
    tciMessage_t* b[MAX_SESSIONS];
	for(int y=0; y<3; y++) {
		printf("Starting round %i...\n",y);
		int max = y+1;
		x = MC_DRV_OK;
		int j = 0;
		do {
			a[j].deviceId = MC_DEVICE_ID_DEFAULT;

			// Allocate WSM buffer for the TCI
	        CHECK_DRVAPI_RETURNS_OK(
                    mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, sizeof(tciMessage_t), (uint8_t **) &b[j], 0));

			x = mcOpenSession(&a[j], &uuid, (uint8_t *) b[j], (uint32_t) sizeof(tciMessage_t));

			j++;

			if(MC_DRV_OK == x)
				printf("Stress-test: mcOpenSession #%i.\n",j-1);
			else if(MC_DRV_ERR_DAEMON_UNREACHABLE == x)
				break;
			else
				CHECK_EQUAL(MC_DRV_ERR_DAEMON_UNREACHABLE, x);
		}
		while(j < max);//MAX_SESSIONS);

		j--;//undo last increment
		printf("Stress-test: mcOpenSession called %i times, opened %i sessions.\n",j+1,j+(MC_DRV_OK == x));
		//note: currently mcOpenSession gets called 3 times
		if(MC_DRV_OK != x)
			j--; //the last one was not opened
		for(;j>-1;j--) {
			printf("Stress-test: mcCloseSession #%i.\n",j);
	        CHECK_DRVAPI_RETURNS_OK(
                    mcCloseSession(&a[j]));

			// Free WSM buffer for TCI
	        CHECK_DRVAPI_RETURNS_OK(
                    mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t*)b[j]));
		}
	}
	x = MC_DRV_ERR_UNKNOWN;
}
/**@description TLC tries to close a device with an open session attached to it
 * @pre 		Involved components running and functioning
 * @focus		mcCloseDevice should return an error of a  pending session.
 * @veri     	Returns MC_DRV_ERR_SESSION_PENDING, device and session stay open.<br> Test output is OK.
 * @post		-
 */
TESTCASE(McDrvApi,00500_mcOpenSession,BC00110_CloseDeviceWithOpenSession,1)
{
    CHECK_DRVAPI_RETURNS_OK(
            mcOpenSession(&sessionHandle, &uuid, (uint8_t *) tci, (uint32_t) sizeof(tciMessage_t)));

	mcResult_t y = x;
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_SESSION_PENDING,
            mcCloseDevice(MC_DEVICE_ID_DEFAULT));
    x = y;
}

/**@description TLC tries to open a session and leave it open
 * @pre 		Involved components running and functioning
 * @focus		mcOpenSession should open persistent sessions.
 * @veri     	Test output is OK.
 * @post		-
 */
TESTCASE(McDrvApi, 00500_mcOpenSession, BC00111_LeaveSession,3)
{
	// Open a session on default MobiCore device
	// Session IDs must be according to [LowInt]
    CHECK_DRVAPI_RETURNS_OK(
    		mcOpenSession(&sessionHandle, &uuid, tci, (uint32_t) sizeof(tciMessage_t)));
    // 0 is reserved for MCP session
	CHECK( 0 != sessionHandle.sessionId );
    // 0xffffffff is invalid
	CHECK( 0xffffffff != sessionHandle.sessionId );
    // MSB must not be set
    CHECK_EQUAL( 0,(sessionHandle.sessionId & (1U << 31)) );

    // Lets see if the session pending affects.
    // A crash  should not happen
    CHECK_DRVAPI_RETURNS(MC_DRV_ERR_SESSION_PENDING,
    				mcCloseDevice(MC_DEVICE_ID_DEFAULT));

}
/**
 * @}
 * @} */
