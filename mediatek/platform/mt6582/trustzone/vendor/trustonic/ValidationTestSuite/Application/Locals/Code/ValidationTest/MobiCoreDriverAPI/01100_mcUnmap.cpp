/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVUNMAP 01100_mcUnmap
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

#include "Common/TestUtils.h"

#define LOG_TAG "Test01100_mcUnmap"
#include <log.h>

//#define BULK_BUF_LEN 0x0FF000		// 1Mb - 4k
//#define BULK_BUF_LEN 0x100000		// 1Mb
#define BULK_BUF_LEN 0x004000		// 32K
//#define BULK_BUF_LEN 0x001000		// 4K

TEST_SCENARIO(01100_mcUnmap)
{
	mcResult_t x;
    uint8_t *tci;
    mcSessionHandle_t sessionHandle;
	void *bulkBuf;
	mcBulkMap_t mapInfo;

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

		// Allocate Heap memory
	    bulkBuf = malloc(BULK_BUF_LEN);
	    CHECK( NULL != bulkBuf );

	    // Register memory in Kernel Module and inform MobiCore
	    mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, &mapInfo);
	}
	void teardown (void)
	{
		// Unregister memory in Kernel Module and inform MobiCore
		if(MC_DRV_OK != x)
			mcUnmap(&sessionHandle, bulkBuf, &mapInfo);

		// Free allocated Heap memory
		free(bulkBuf);

		mcCloseSession(&sessionHandle);
		mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);
		mcCloseDevice(MC_DEVICE_ID_DEFAULT);
	}
};
/**@description The previous mapped bulk buffer is unmapped from Trustlet
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap unmaps successfully with the expected inputs.
 * @veri     	returns MC_DRV_OK.<br>
 * @post		
 * Test output is OK.
 */
TESTCASE(McDrvApi,01100_mcUnmap,GC00100_Unmap4k)
{
	// Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));
}
/**@description TLC calls function with a NULL pointer session
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles a null session pointer.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00101_SessionhandleNullPointer)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcUnmap(0, bulkBuf, &mapInfo));
}
/**@description TLC calls function with a NULL pointer bulkBuffer
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles a null bulk data pointer.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00102_BufNullPointer)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcUnmap(&sessionHandle, 0, &mapInfo));
}
/**@description TLC calls function with a NULL pointer mapInfo
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles a null buffer info pointer.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00103_MapinfoNullPointer)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcUnmap(&sessionHandle, bulkBuf, 0));
}
/**@description TLC calls function with a malformed handle, bad sessionId.
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles a unknown session ID.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_SESSION.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00104_SessionhandleInvalidId)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.sessionId = -1;
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_SESSION,
    		mcUnmap(&badSession, bulkBuf, &mapInfo));
}
/**@description TLC calls function with a malformed handle, bad deviceId.
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles an invalid device ID.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_DEVICE.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00105_SessionhandleInvalidDevice)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.deviceId = -1;
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcUnmap(&badSession, bulkBuf, &mapInfo));
}
/**@description TLC calls function with a bad bulkBuf.
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles an oversized bulk buffer.
 * @veri     	returns MC_DRV_ERR_BULK_UNMAPPING.<br>Test output is OK
 * @post		
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00106_BufferPointerNotManagedByLib)
{
    void *badBulkBuf = (void *)((uint32_t)bulkBuf + 4);
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_BULK_UNMAPPING,
    		mcUnmap(&sessionHandle, badBulkBuf, &mapInfo));
}
/**@description TLC calls function with a malformed mapInfo
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles a null virtual addressed map info.
 * @veri     	returns MC_DRV_ERR_BULK_UNMAPPING.<br>Test output is OK.
 * @note        This test also tests RTM ARM exception handling.
 * @post		
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00107_MapinfoAddress0)
{
	mcBulkMap_t badMapInfo = mapInfo;
	badMapInfo.sVirtualAddr = 0;

	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_INVALID_PARAMETER,
			mcUnmap(&sessionHandle, bulkBuf, &badMapInfo));
	x = MC_DRV_ERR_BULK_UNMAPPING;
}
/**@description TLC calls function with a malformed mapInfo
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles a bad virtual addressed map info.
 * @veri     	returns MC_DRV_ERR_BULK_UNMAPPING.<br>Test output is OK.
 * @note        This test also tests RTM ARM exception handling.
 * @post		
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00108_MapinfoAddressMinus1)
{
	mcBulkMap_t badMapInfo = mapInfo;
	badMapInfo.sVirtualAddr = (void*)-1;

	CHECK_DRVAPI_RETURNS(
			MC_DRV_ERR_BULK_UNMAPPING,
			mcUnmap(&sessionHandle, bulkBuf, &badMapInfo));
	x = MC_DRV_ERR_BULK_UNMAPPING;
}

/**@description TLC unmaps the same bulk buffer from Trustlet twice
 * @pre 		Involved components running and functioning
 * @focus		mcUnMap handles a  multiple unmapping on the same session.
 * @veri     	returns MC_DRV_ERR_BULK_UNMAPPING.<br>
 * @post		
 */
TESTCASE(McDrvApi,01100_mcUnmap,BC00110_UnmapAnAlreadyUnmappedBulkBuffer)
{
	// Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_BULK_UNMAPPING,
    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

    x = MC_DRV_OK; // Don't call unmap() by teardown()
}
/**@description TLC writes something in buffer and then maps and unmaps the buffer to Trustlet
 * @pre 		Involved components running and functioning
 * @focus		mcMap and mcUnMap handle an already written bulk buffer.
 * @veri     	Unmapping does not clear the buffer<br>
 * Test output is OK.
 * @post		
 */
TESTCASE(McDrvApi,01100_mcUnmap,GC00111_UnmapDoesntClear)
{
    // Register memory in Kernel Module and inform MobiCore
	int len = 3110;
    uint8_t pattern = TestUtils::Random::getByte();
    memset(bulkBuf, pattern, len);

    // Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

    uint8_t* b = (uint8_t*)bulkBuf;
    for(int i=0; i<len; i++)
	{
		CHECK_EQUAL(pattern, b[i]);
	}
}


/**
 * @}
 * @} */
