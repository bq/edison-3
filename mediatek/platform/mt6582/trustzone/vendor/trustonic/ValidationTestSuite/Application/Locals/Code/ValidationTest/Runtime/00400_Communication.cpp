/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup MCDRVCOMMUNICATIONS 00400_Communication
 * Parallel Notifications and Mappings
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

#define NOTIFY_SESSION1 1
#define NOTIFY_SESSION2 1
#define DONT_NOTIFY_SESSION1 0
#define DONT_NOTIFY_SESSION2 0

TEST_SCENARIO(00400_Communication)
{
	mcResult_t x,y;
	tciMessage_t *tci,*tci2;
    mcSessionHandle_t session1,session2;

	void setup(void) {
		FORWARDER_INIT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenDevice(MC_DEVICE_ID_DEFAULT));

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci, 0));

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci2, 0));

		// Clear the session handle
		bzero(&session1, sizeof(session1));

		// Set default device ID
		session1.deviceId = MC_DEVICE_ID_DEFAULT;

		session2 = session1;

		mcUuid_t uuid = TL_TEST_UUID;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&session1, &uuid, (uint8_t *)tci, (uint32_t) sizeof(tciMessage_t)));

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&session2, &uuid, (uint8_t *)tci2, (uint32_t) sizeof(tciMessage_t)));
	}
	void teardown (void)
	{
		// Close session2
		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&session1));
		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&session2));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci));
		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci2));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseDevice(MC_DEVICE_ID_DEFAULT));
	}
	void callTemplate(bool notifySession1, bool notifySession2, bool notifySession1again) {
		cmdTlTestNotify_t cmd = {
				// C++ does not support C99 designated initializers
				/* .cmdHeader = */ {
					/* .commandId = */ CMD_TEST_NOTIFY
				},
		};
	    // Prepare command message
	    tci->cmdTlTestNotify = cmd;
	    tci2->cmdTlTestNotify = cmd;

	    
	    if(notifySession1) {
	    	CHECK_DRVAPI_RETURNS_OK(
	    			mcNotify(&session1));
	    }
	    if(notifySession2) {
	    	CHECK_DRVAPI_RETURNS_OK(
	    			mcNotify(&session2));
	    }
	    if(notifySession1again) {
	    	CHECK_DRVAPI_RETURNS_OK(
	    			mcNotify(&session1));
	    }

	    // Wait for notification from first Trustlet
	    x = mcWaitNotification(&session1, 1000);
	    if(notifySession1 || notifySession1again) {
	    	//we notified Trustlet 1, so we expect an answer
	    	CHECK_DRVAPI_RETURNS_OK( x);

			// Check if response ID set
			CHECK_EQUAL( RSP_ID(CMD_TEST_NOTIFY), tci->rspTlTestNotify.rspHeader.responseId );

			// Check if Trustlet says ok
			CHECK_EQUAL( RET_OK, tci->responseHeader.returnCode );
	    } else {
	    	//we didn't notify Trustlet 1, so we don't expect an answer
	    	CHECK_EQUAL(MC_DRV_ERR_TIMEOUT, x);
	    }

		// Wait for notification from second Trustlet
		x = mcWaitNotification(&session2, 1000);
	    if(notifySession2) {
	    	//we notified Trustlet 2, so we expect an answer
			CHECK_DRVAPI_RETURNS_OK( x);

			// Check if response ID set
			CHECK_EQUAL( RSP_ID(CMD_TEST_NOTIFY), tci2->rspTlTestNotify.rspHeader.responseId );

			// Check if Trustlet says ok
			CHECK_EQUAL( RET_OK, tci2->responseHeader.returnCode );
	    } else {
	    	//we didn't notify Trustlet 2, so we don't expect an answer
	    	CHECK_EQUAL(MC_DRV_ERR_TIMEOUT, x);
	    }
	}
};
/**@description TLC sends and receives notifications from two Trustlets.<br>
 * In all cases it waits for both trustlet.<br>
 * But it notifies only none, one, or both<br>
 * @pre 		Involved components running and functioning
 * @veri     	mcWaitNotification() returns MC_DRV_OK or MC_DRV_ERR_TIMEOUT.<br>
 * Response ID is set in message header on TCI.<br>
 * Exit code is OK.<br>
 * Test output is OK.
 * @note        This test also tests RTM ARM exception handling.
 */
TESTCASE(Runtime,00400_Communication,GC00100_NoNotifyThenWait)
{
	callTemplate(
			DONT_NOTIFY_SESSION1,
			DONT_NOTIFY_SESSION2,
			DONT_NOTIFY_SESSION1);
}
TESTCASE(Runtime,00400_Communication,GC00101_NotifyT2ThenWait)
{
	callTemplate(
			DONT_NOTIFY_SESSION1,
			NOTIFY_SESSION2,
			DONT_NOTIFY_SESSION1);
}
TESTCASE(Runtime,00400_Communication,GC00102_NotifyT1ThenWait)
{
	callTemplate(
			NOTIFY_SESSION1,
			DONT_NOTIFY_SESSION2,
			DONT_NOTIFY_SESSION1);
}
TESTCASE(Runtime,00400_Communication,GC00103_NotifyT1T2ThenWait)
{
	callTemplate(
			NOTIFY_SESSION1,
			NOTIFY_SESSION2,
			DONT_NOTIFY_SESSION1);
}
TESTCASE(Runtime,00400_Communication,GC00104_NotifyT2T1ThenWait)
{
	callTemplate(
			DONT_NOTIFY_SESSION1,
			NOTIFY_SESSION2,
			NOTIFY_SESSION1);
}
//TODO: make a separate file and split test scenario, even if setup is the same
/**@description TLC maps buffer to two Trustlets<br>
 * Trustlet 1 writes in buffer.<br>
 * Trustlet 2 reads the buffer.
 * @pre 		Involved components running and functioning
 * @veri     	Trustlet 2 reads what Trustlet 1 wrote.<br>
 * Test output is OK.
 */
TESTCASE(Runtime,00400_Communication,GC00105_SharedMemoryT1WritesT2Reads)
{
	int one_page  = 4096;
    mcBulkMap_t mapInfo1,mapInfo2;

    // Allocate Heap memory
    uint8_t* bulkBuf = (uint8_t*)malloc(one_page);
    CHECK( NULL != bulkBuf );

    // Register memory in Kernel Module and inform MobiCore
	int len = 3110;
#define BYTE_PATTERN_1    0x5a
#define BYTE_PATTERN_2    0x6d
//    memset(bulkBuf, BYTE_PATTERN_1, one_page);

    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&session1, bulkBuf, one_page, &mapInfo1));
	CHECK_EQUAL(one_page,  mapInfo1.sVirtualLen);

    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&session2, bulkBuf, one_page, &mapInfo2));
	CHECK_EQUAL(one_page,  mapInfo2.sVirtualLen );

	//Let Trustlet 1 fill the memory
    tci->cmdTlTestFillMem.cmdHeader.commandId = CMD_TEST_FILL_MEM;
    tci->cmdTlTestFillMem.dest = (uint32_t)mapInfo1.sVirtualAddr;
    tci->cmdTlTestFillMem.len = len;
    tci->cmdTlTestFillMem.value = BYTE_PATTERN_2;
	CHECK_DRVAPI_RETURNS_OK( mcNotify(&session1));
    CHECK_DRVAPI_RETURNS_OK( mcWaitNotification(&session1, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_TEST_FILL_MEM), tci->responseHeader.responseId );
    CHECK_EQUAL( RET_OK, tci->responseHeader.returnCode)

    //Let Trustlet 2 compare the memory
    tci2->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
    tci2->cmdTlTestCompareMem.dest = (uint32_t)mapInfo2.sVirtualAddr;
    tci2->cmdTlTestCompareMem.len = len;
    tci2->cmdTlTestCompareMem.value = BYTE_PATTERN_2;
	CHECK_DRVAPI_RETURNS_OK( mcNotify(&session2));
    CHECK_DRVAPI_RETURNS_OK( mcWaitNotification(&session2, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_TEST_COMPARE_MEM), tci2->responseHeader.responseId );
    CHECK_EQUAL( RET_OK, tci2->responseHeader.returnCode)

    //Let Trustlet 2 compare the rest of the memory
    tci2->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
    tci2->cmdTlTestCompareMem.dest = (uint32_t)mapInfo2.sVirtualAddr+len;
    tci2->cmdTlTestCompareMem.len = 1;
    tci2->cmdTlTestCompareMem.value = BYTE_PATTERN_2;
	CHECK_DRVAPI_RETURNS_OK( mcNotify(&session2));
    CHECK_DRVAPI_RETURNS_OK( mcWaitNotification(&session2, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_TEST_COMPARE_MEM), tci2->responseHeader.responseId );
    CHECK_EQUAL( RET_ERR_COMPARE_FAILED, tci2->responseHeader.returnCode)

    // Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&session1, bulkBuf, &mapInfo1));

    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&session2, bulkBuf, &mapInfo2));

    free(bulkBuf);
}
/**@description TLC writes something in buffer and then maps it to two Trustlets.<br>
 * Then TLC unmaps the buffer from one Trustlet.<br>
 * The other Trustlet can still read the buffer.
 * @pre 		Involved components running and functioning
 * @veri     	Unmapping does not unmap the buffer everywhere.<br>
 * Test output is OK.
 */
TESTCASE(Runtime,00400_Communication,GC00106_UnmapSharedMemoryFromT1ThenCheckT2CanStillRead)
{
	int one_page  = 4096;
    mcBulkMap_t mapInfo1,mapInfo2;

    // Allocate Heap memory
    uint8_t* bulkBuf = (uint8_t*)malloc(one_page);
    CHECK( NULL != bulkBuf );

    // Register memory in Kernel Module and inform MobiCore
	int len = 3110;
#define BYTE_PATTERN_2    0x6d
    memset(bulkBuf, BYTE_PATTERN_2, len);

    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&session1, bulkBuf, one_page, &mapInfo1));
	CHECK_EQUAL(one_page,  mapInfo1.sVirtualLen);

    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&session2, bulkBuf, one_page, &mapInfo2));
	CHECK_EQUAL(one_page,  mapInfo2.sVirtualLen );

    // Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&session1, bulkBuf, &mapInfo1));

    //Let Trustlet 2 compare the memory
    tci2->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
    tci2->cmdTlTestCompareMem.dest = (uint32_t)mapInfo2.sVirtualAddr;
    tci2->cmdTlTestCompareMem.len = len;
    tci2->cmdTlTestCompareMem.value = BYTE_PATTERN_2;
	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&session2));
    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&session2, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_TEST_COMPARE_MEM), tci2->responseHeader.responseId );
    CHECK_EQUAL( RET_OK, tci2->responseHeader.returnCode)

    //Let Trustlet 2 compare the rest of the memory
    tci2->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
    tci2->cmdTlTestCompareMem.dest = (uint32_t)mapInfo2.sVirtualAddr+len;
    tci2->cmdTlTestCompareMem.len = 1;
    tci2->cmdTlTestCompareMem.value = BYTE_PATTERN_2;
	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&session2));
    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&session2, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_TEST_COMPARE_MEM), tci2->responseHeader.responseId );
    CHECK_EQUAL( RET_ERR_COMPARE_FAILED, tci2->responseHeader.returnCode)

    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&session2, bulkBuf, &mapInfo2));

    free(bulkBuf);
}
/**
 * @}
 * @} */
