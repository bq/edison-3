/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVMAP 01000_mcMap
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
#include "errno.h"

#include "Common/TestUtils.h"

#define LOG_WANT_BUF
#define LOG_TAG "Test01000_mcMap"
#include <log.h>

#define PAGE_SIZE 4096
#define PAGE_MASK ~(PAGE_SIZE - 1)

//#define BULK_BUF_LEN 0x0FF000		// 1Mb - 4k
//#define BULK_BUF_LEN 0x100000		// 1Mb
#define BULK_BUF_LEN 0x004000		// 32K
//#define BULK_BUF_LEN 0x001000		// 4K

TEST_SCENARIO(01000_mcMap)
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
	}
	void teardown (void)
	{
		// Unregister memory in Kernel Module and inform MobiCore
		if(MC_DRV_OK == x)
			mcUnmap(&sessionHandle, bulkBuf, &mapInfo);

		// Free allocated Heap memory
		free(bulkBuf);

		mcCloseSession(&sessionHandle);
		mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);
		mcCloseDevice(MC_DEVICE_ID_DEFAULT);
	}
};
/**@description TLC maps additional Bulk memory to Trustlet
 * @pre 		Involved components running and functioning
 * @focus		mcMap does not map additional bulk memory.
 * @veri     	returns MC_DRV_OK.<br>
 * returned Buffer length is equal to requested length.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,01000_mcMap,GC00100_Map4k)
{
    // Register memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, &mapInfo));
	CHECK_EQUAL(BULK_BUF_LEN,  mapInfo.sVirtualLen );
}
/**@description TLC calls function with a NULL pointer session
 * @pre 		Involved components running and functioning
 * @focus		mcMap handles a null session pointer.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,01000_mcMap,BC00101_SessionhandleNullPointer)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcMap(0, bulkBuf, BULK_BUF_LEN, &mapInfo));
}
/**@description TLC calls function with a NULL pointer mapInfo
 * @pre 		Involved components running and functioning
 * @focus		mcMap handles a null map info pointer.
 * @veri     	returns MC_DRV_ERR_INVALID_PARAMETER.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,01000_mcMap,BC00102_MapinfoNullPointer)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, 0));
}
/**@description TLC calls function with a malformed handle, bad sessionId.
 * @pre 		Involved components running and functioning
 * @focus		mcMap handles an invalid session ID.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_SESSION.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,01000_mcMap,BC00103_SessionhandleInvalidId)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.sessionId = -1;
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_SESSION,
    		mcMap(&badSession, bulkBuf, BULK_BUF_LEN, &mapInfo));
}
/**@description TLC calls function with a malformed handle, bad deviceId.
 * @pre 		Involved components running and functioning
 * @focus		mcMap handles an unknown device id.
 * @veri     	returns MC_DRV_ERR_UNKNOWN_DEVICE.<br>Test output is OK
 * @post
 */
TESTCASE(McDrvApi,01000_mcMap,BC00104_SessionhandleInvalidDevice)
{
	mcSessionHandle_t badSession = sessionHandle;
	badSession.deviceId = -1;
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcMap(&badSession, bulkBuf, BULK_BUF_LEN, &mapInfo));
}
/**@description TLC maps the same additional bulk memory to one Trustlet twice
 * @pre 		Involved components running and functioning
 * @focus		mcMap handles a multiple memory mapping on same Trustlet.
 * @veri     	returns MC_DRV_ERR_BULK_MAPPING.<br>
 * The bulk buffer cannot be mapped to the same address.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,01000_mcMap,BC00105_MultipleMappingsOfSameBuffer)
{
    // Register memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, &mapInfo));
	CHECK_EQUAL(BULK_BUF_LEN,  mapInfo.sVirtualLen );

	mcBulkMap_t mapInfo2;

	mcResult_t y = mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, &mapInfo2);
	CHECK_EQUAL(MC_DRV_ERR_BULK_MAPPING, y);
 
	// Unregister memory in Kernel Module and inform MobiCore
	if(MC_DRV_OK == y)
		CHECK_DRVAPI_RETURNS_OK(
				mcUnmap(&sessionHandle, bulkBuf, &mapInfo2));

}
/**@description TLC tries to map read-only data to Trustlet
 * @pre 		Involved components running and functioning
 * @focus		mcMap handles a readonly memory area mapping.
 * @veri     	returns MC_DRV_ERR_BULK_MAPPING.<br>
 * The bulk buffer cannot be mapped because it points to readonly data.<br>
 * Test output is OK.
 * @post
 */
TESTCASE(McDrvApi,01000_mcMap,BC00106_MapAReadonlyBuffer)
{
	uint8_t *dataIn=(uint8_t *)"Plaintext";
	uint32_t dataInLen = strlen((char*)dataIn);
 
    // Register memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS(
            MAKE_MC_DRV_KMOD_WITH_ERRNO(EINVAL),
    		mcMap(&sessionHandle, dataIn, dataInLen, &mapInfo));
}

/**
 * @}
 * @} */
