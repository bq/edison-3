/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVGETRNGSEED 01200_mcGetRndSeed
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

#include "Common/TestUtils.h"



#define LOG_WANT_BUF
#define LOG_TAG "Test01200_mcGetRngSeed"
#include <log.h>


#define BULK_BUF_LEN 32				// 32 bytes is the seed size

TEST_SCENARIO(01200_mcGetRngSeed) {
	mcResult_t x;
    uint8_t *tci;
    mcSessionHandle_t sessionHandle;
	void *bulkBuf;
	mcBulkMap_t mapInfo;
	mcUuid_t uuid;

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

		uuid = (mcUuid_t)TL_TEST_UUID;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle, &uuid, tci, (uint32_t) sizeof(tciMessage_t)));

		// Allocate Heap memory
	    bulkBuf = malloc(BULK_BUF_LEN);
	    CHECK( NULL != bulkBuf );

	    // Register memory in Kernel Module and inform MobiCore
	    // mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, &mapInfo);
	}

	void teardown (void)
	{
		// Unregister memory in Kernel Module and inform MobiCore
		// if(MC_DRV_OK != x)
		// mcUnmap(&sessionHandle, bulkBuf, &mapInfo);

		// Free allocated Heap memory
		free(bulkBuf);

		mcCloseSession(&sessionHandle);
		mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);
		mcCloseDevice(MC_DEVICE_ID_DEFAULT);
	}

	uint32_t isBulkAllZero (uint32_t len) {

		uint8_t cmpArray[len];
		memset(cmpArray, 0, len);
		return memcmp(bulkBuf, cmpArray, len);
	}
};

/**@description TLC opens a session, <br>
 * @pre
 * @focus mcNotify and mcWaitNotification return MC_DRV_OK
 * @veri  mapped area includes non-zero values after RNG seed read operation <br>
 * successfully performed.
 * @post
 */
TESTCASE(McDrvApi, 01200_mcGetRngSeed, GC01200_GetRngSeedByDriver) {

	uint8_t seed[32];
	const char * hexFile = "/data/app/HexFile.seed";
	const char * dataFile = "/data/app/asciiFile.seed";

	CHECK_DRVAPI_RETURNS_OK(
			mcMap(&sessionHandle, bulkBuf, BULK_BUF_LEN, &mapInfo));

	// Prepare command message
	((tciMessage_t *)tci)->cmdTlGetRngSeed.cmdHeader.commandId = CMD_GET_RNG_SEED_BY_DRIVER;
	((tciMessage_t *)tci)->cmdTlGetRngSeed.seedBufferAddr = (uint32_t) mapInfo.sVirtualAddr;
    
	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle));

	CHECK_DRVAPI_RETURNS_OK(
			mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

	// Checks if bulkmap is empty - Has Seed actually been read.
	CHECK_C(isBulkAllZero(BULK_BUF_LEN) != 0);
	// Log the seed
	LOG_I_BUF("The seed is:", bulkBuf, BULK_BUF_LEN);

	// Writes as concatenated byte blocks into file as text
	TestUtils::FileOperations::writeHexDataInFile(hexFile, (uint8_t *)bulkBuf, BULK_BUF_LEN);

	// Writes as comma seperated bytes into file in ASCII.
	TestUtils::FileOperations::writeDataInFile(dataFile, (uint8_t *)bulkBuf, BULK_BUF_LEN);

	// TODO -zenginon - (14_03_2012) WRITE the first RN into a file

	// Request error code, must be "no error"
	int32_t lastErr;
	CHECK_DRVAPI_RETURNS_OK(
			mcGetSessionErrorCode(&sessionHandle, &lastErr));
	CHECK_EQUAL( 0, lastErr );

	CHECK_DRVAPI_RETURNS_OK(
			mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

}
