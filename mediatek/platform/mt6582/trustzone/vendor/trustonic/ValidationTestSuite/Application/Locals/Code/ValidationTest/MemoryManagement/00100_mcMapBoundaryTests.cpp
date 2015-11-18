/** @addtogroup TEST_MM
 * @{
 * @defgroup TEST_MM_BOUNDARY 00100_mcMapBoundaryTests
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
 * -# Trustlet TlTestMemoryManagement
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
#define LOG_TAG "Test10_mcMap"
#include <log.h>

#define PAGE_SIZE 4096
#define PAGE_MASK ~(PAGE_SIZE - 1)

//#define BULK_BUF_LEN 0x0FF000		// 1Mb - 4k
//#define BULK_BUF_LEN 0x100000		// 1Mb
#define BULK_BUF_LEN 0x004000		// 32K
//#define BULK_BUF_LEN 0x001000		// 4K

/**@description TLC maps additional Bulk memory to Trustlet and tests the access via TLC and Trustlet <br>
 * Testsequence:<br>
 * 1. TLC fills Bulk buffer with a number and notifies the Trustlet<br>
 * 2. TL reads number from the Bulk buffer and increments it<br>
 * 3. TLC reads back number after notification from the TL<br>
 * @pre 		Involved components running and functioning
 * @veri     	returns MC_DRV_OK.<br>
 * Test output is OK.
 */
TEST_SCENARIO(00100_mcMapBoundaryTests)
{
	mcResult_t x;
    volatile tciMessage_t *tci;
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
		// Close session
		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseDevice(MC_DEVICE_ID_DEFAULT));
	}
	
	/*
	* Executes predefined Tci Commands and verifies if the command has been
	* executed successfully
	*/
	void executeTciCommand(uint32_t commandId){

	    CHECK_DRVAPI_RETURNS_OK(
	    		mcNotify(&sessionHandle));

	    // Wait for notification from Trustlet
	    CHECK_DRVAPI_RETURNS_OK(
	            mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

	    // Check if response ID set
	    CHECK_EQUAL( RSP_ID(commandId), tci->rspTlTestNotify.rspHeader.responseId );

	    // Check if Trustlet says ok
	    CHECK_EQUAL( RET_OK, tci->responseHeader.returnCode );

	    // Request error code, must be "no error"
	    int32_t lastErr;
        CHECK_DRVAPI_RETURNS_OK(
                mcGetSessionErrorCode(&sessionHandle, &lastErr));
	    CHECK_EQUAL( 0, lastErr );
	}
	
	/*
 	* Writes data to NWd via MC and checks integrity of the data.
	*/
	void writeAndCheck(void *buf, uint32_t offset, uint32_t len, mcBulkMap_t mapInfo){

	    // SWd address to modify
	    uint32_t bufInTrustlet = (uint32_t)mapInfo.sVirtualAddr + offset;
	    void* bufLocal = (void*)( (uint32_t)buf + offset );

	    LOG_I("Buffer info -> Len: %i, NWd adr: %08X, SWd adr: %08X", 1, (uint32_t)bufLocal, bufInTrustlet);

        uint8_t pattern[2];
        pattern[0] = TestUtils::Random::getByte();
        pattern[1] = ~pattern[0];

	    // ----------------------------------------------------------------------
	    // NWd to SWd write check

	    // Set test value in NWd
	    LOG_I("NWd -> SWd test - Filling buffer - Patterns used: %02X %02X", pattern[0], pattern[1]);
	    memset(bufLocal, pattern[0], len);

	    // Check if address has been set
	    LOG_I("Checking memory on SWd side (CMD_TEST_COMPARE_MEM)");
	    tci->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
	    tci->cmdTlTestCompareMem.dest = bufInTrustlet;
	    tci->cmdTlTestCompareMem.len = len;
	    tci->cmdTlTestCompareMem.value = pattern[0];
	    executeTciCommand(CMD_TEST_COMPARE_MEM);
	    LOG_I("SUCCESS");

	    // ----------------------------------------------------------------------
	    // SWd to NWd write check
	    LOG_I("SWd -> NWd test");

	    LOG_I("Filling memory on SWd side (CMD_TEST_FILL_MEM)");
	    tci->cmdTlTestFillMem.cmdHeader.commandId = CMD_TEST_FILL_MEM;
	    tci->cmdTlTestFillMem.dest = bufInTrustlet;
	    tci->cmdTlTestFillMem.len = len;
	    tci->cmdTlTestFillMem.value = pattern[1];
	    executeTciCommand(CMD_TEST_FILL_MEM);

	    // Check memory region in NWd
	    LOG_I("Checking memory on NWd side: PASS 1");
	    for(uint32_t i = 0; i < len; i++){
	        uint8_t val = ((uint8_t *)bufLocal)[i];
	        if (pattern[1] != val)
	        {
	            LOG_I("Comparison failed at address 0x%08x", (uint32_t)bufLocal+i);
	            LOG_I_Buf(NULL,bufLocal,len);
	        }
	        CHECK_EQUAL( pattern[1], val );
	    }

	    LOG_I("Reading back memory from SWd side (CMD_TEST_COMPARE_MEM)");
	    // Check if byte has been set in SWd
	    tci->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
	    tci->cmdTlTestCompareMem.dest = bufInTrustlet;
	    tci->cmdTlTestCompareMem.len = len;
	    tci->cmdTlTestCompareMem.value = pattern[1];
	    executeTciCommand(CMD_TEST_COMPARE_MEM);

	    // Verify that value written by SWd has reached the NWd
	    LOG_I("Checking memory on NWd side: PASS 2");
	    for(uint32_t i = 0; i < len; i++){
	        uint8_t val = ((uint8_t *)bufLocal)[i];
	        if (pattern[1] != val)
	        {
	            LOG_I("Comparison failed at address 0x%08x", (uint32_t)bufLocal+i);
	            LOG_I_Buf(NULL,bufLocal,len);
	        }
	        CHECK_EQUAL( pattern[1], val );
	    }
	    LOG_I("SUCCESS");
	}
	
	/*
	* Checks big sized buffers with page borders respective to buffer boundaries
	*/ 
	void checkBufferBoundaries(uint32_t bufferSize){
	    mcResult_t x;
	    mcBulkMap_t mapInfo;

	    LOG_I("-------------------------------------------------------------------------");
	    LOG_I("START> WSM bulk mapping boundary test. Buffersize is %i",bufferSize);

	    // Allocate Heap memory
	    void *bulkBuf = malloc(bufferSize);

	    // Map bulk memory to Trustlet
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcMap(&sessionHandle, bulkBuf, bufferSize, &mapInfo));

	    LOG_I("Len: %i, NWd adr: %08X, SWd adr: %08X", bufferSize, (uint32_t)bulkBuf, (uint32_t)mapInfo.sVirtualAddr);

	    uint32_t start = (uint32_t) bulkBuf;
	    uint32_t end = start + bufferSize;

	    uint32_t pagesAffected = (((end & PAGE_MASK) + PAGE_SIZE) - (start & PAGE_MASK))/PAGE_SIZE;
	    LOG_I("Affected pages: %i",pagesAffected);

	    // Check beginning of buffer
	    LOG_I("Checking first byte of buffer");
	    LOG_I("---------------------------");
	    writeAndCheck(bulkBuf, 0, 1, mapInfo);

	    // Check page borders
	    uint32_t ofsInPage = (uint32_t)bulkBuf & (PAGE_SIZE - 1);
	    uint32_t ofsBorder =   PAGE_SIZE - ofsInPage - 1;

	    // Check n-1 boundaries
	    for(int i = 0; i < (pagesAffected -1); i++){

	        LOG_I("Checking page boundary %i", i+1);
	        LOG_I("---------------------------");
	        //Check upper bound of page
	        writeAndCheck(bulkBuf, ofsBorder, 1, mapInfo);

	        //Check lower bound of next page
	        writeAndCheck(bulkBuf, ofsBorder + 1, 1, mapInfo);

	        // Warp to next boundary
	        ofsBorder += PAGE_SIZE;
	    }

	    // Check end of buffer
	    LOG_I("Checking last byte of buffer");
	    LOG_I("---------------------------");
	    writeAndCheck(bulkBuf, bufferSize -1, 1, mapInfo);

	    // Unregister memory in Kernel Module and inform MobiCore
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

	    // Free allocate Heap memory
	    free(bulkBuf);

	    LOG_I("END TEST",bufferSize);
	    LOG_I("-------------------------------------------------------------------------");

	}

	/*
	* Writes and checks the buffer for a certain size and reads in back that is
	* stored on NWd via MC.
	*/
	void checkBufferFillAndReadback(uint32_t bufferSize){
	    mcResult_t x;
	    mcBulkMap_t mapInfo;

	    LOG_I("-------------------------------------------------------------------------");
	    LOG_I("START> WSM bulk mapping fill and readback test. Buffersize is %i",bufferSize);

	    // Allocate Heap memory
	    void *bulkBuf = malloc(bufferSize);

	    // Map bulk memory to Trustlet
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcMap(&sessionHandle, bulkBuf, bufferSize, &mapInfo));

	    LOG_I("Len: %i, NWd adr: %08X, SWd adr: %08X", bufferSize, (uint32_t)bulkBuf, (uint32_t)mapInfo.sVirtualAddr);

	    uint32_t start = (uint32_t) bulkBuf;
	    uint32_t end = start + bufferSize;

	    uint32_t pagesAffected = (((end & PAGE_MASK) + PAGE_SIZE) - (start & PAGE_MASK))/PAGE_SIZE;
	    LOG_I("Affected pages: %i",pagesAffected);

	    // Check beginning of buffer
	    writeAndCheck(bulkBuf, 0, bufferSize, mapInfo);

	    // Unregister memory in Kernel Module and inform MobiCore
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

	    // Free allocate Heap memory
	    free(bulkBuf);

	    LOG_I("END TEST",bufferSize);
	    LOG_I("-------------------------------------------------------------------------");

	}

	/*
	* Writes multiple blocks in NWd via MC and reads them back 
	* to see if there is any corruption in the data.
	*/
	void mapAllFillAndReadback(uint32_t bufferSize){
	    mcBulkMap_t mapInfoD3;
	    mcBulkMap_t mapInfoD4;
	    mcBulkMap_t mapInfoD5;
	    mcBulkMap_t mapInfoD6;

	    LOG_I("-------------------------------------------------------------------------");
	    LOG_I("START> WSM multi-mapping fill and readback test. Buffersize is %i",bufferSize);

	    // Allocate Heap memory
	    void *bulkBufD3 = malloc(bufferSize); // D3
	    void *bulkBufD4 = malloc(bufferSize); // D4
	    void *bulkBufD5 = malloc(bufferSize); // D5
	    void *bulkBufD6 = malloc(bufferSize); // D6

	    // Map bulk memory to Trustlet
	    mcResult_t x;
	    LOG_I("Mapping D3");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcMap(&sessionHandle, bulkBufD3, bufferSize, &mapInfoD3));
	    LOG_I("D3: %i, NWd adr: %08X, SWd adr: %08X", bufferSize, (uint32_t)bulkBufD3, (uint32_t)mapInfoD3.sVirtualAddr);

	    LOG_I("Mapping D4");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcMap(&sessionHandle, bulkBufD4, bufferSize, &mapInfoD4));
	    LOG_I("D4: %i, NWd adr: %08X, SWd adr: %08X", bufferSize, (uint32_t)bulkBufD4, (uint32_t)mapInfoD4.sVirtualAddr);

	    LOG_I("Mapping D5");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcMap(&sessionHandle, bulkBufD5, bufferSize, &mapInfoD5));
	    LOG_I("D5: %i, NWd adr: %08X, SWd adr: %08X", bufferSize, (uint32_t)bulkBufD5, (uint32_t)mapInfoD5.sVirtualAddr);

	    LOG_I("Mapping D6");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcMap(&sessionHandle, bulkBufD6, bufferSize, &mapInfoD6));
	    LOG_I("D6: %i, NWd adr: %08X, SWd adr: %08X", bufferSize, (uint32_t)bulkBufD6, (uint32_t)mapInfoD6.sVirtualAddr);

	    /*
	    uint32_t start = (uint32_t) bulkBufD3;
	    uint32_t end = start + bufferSize;

	    uint32_t pagesAffected = (((end & PAGE_MASK) + PAGE_SIZE) - (start & PAGE_MASK))/PAGE_SIZE;
	    LOG_I("D3 Affected pages: %i",pagesAffected);
	    */

	    // Check beginning of buffer
	    LOG_I("writeAndCheck D3");
	    writeAndCheck(bulkBufD3, 0, bufferSize, mapInfoD3);
	    LOG_I("writeAndCheck D4");
	    writeAndCheck(bulkBufD4, 0, bufferSize, mapInfoD4);
	    LOG_I("writeAndCheck D5");
	    writeAndCheck(bulkBufD5, 0, bufferSize, mapInfoD5);
	    LOG_I("writeAndCheck D6");
	    writeAndCheck(bulkBufD6, 0, bufferSize, mapInfoD6);

	    // Unregister memory in Kernel Module and inform MobiCore
	    LOG_I("Unmapping D3");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcUnmap(&sessionHandle, bulkBufD3, &mapInfoD3));

	    LOG_I("Unmapping D4");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcUnmap(&sessionHandle, bulkBufD4, &mapInfoD4));

	    LOG_I("Unmapping D5");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcUnmap(&sessionHandle, bulkBufD5, &mapInfoD5));

	    LOG_I("Unmapping D6");
	    CHECK_DRVAPI_RETURNS_OK(
	    		mcUnmap(&sessionHandle, bulkBufD6, &mapInfoD6));

	    // Free allocate Heap memory
	    free(bulkBufD3);
	    free(bulkBufD4);
	    free(bulkBufD5);
	    free(bulkBufD6);

	    LOG_I("END TEST",bufferSize);
	    LOG_I("-------------------------------------------------------------------------");

	}

};

/**
* Verifies that the bidirectional NWd, SWd read write operations are successful <br>
* on the buffer and page boundaries for a buffer size which is smaller that 1 page.  
*/

// Boundary tests
/**@description	Verifies that the bidirectional NWd, SWd read write operations are successful <br>
* on the buffer and page boundaries for a buffer size which is smaller than 1 page.  
 * @pre			Involved components running and functioning   
 * @focus		CMD_TEST_FILL_MEM command is executable for single byte memory insertion
 * @veri        Page and buffer boundaries are written from SWd to NWd and verified <br>
 * Data is written from NWd to SWd and verified.
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */

TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00100_CheckBufferBoundariesSize1)
{
    checkBufferBoundaries(1);
}

/**@description Verifies that the bidirectional NWd, SWd read write operations are successful <br>
* on the buffer and page boundaries for a buffer sized smaller than 2 pages.  
 * Proves that memory allocation does not cause a boundary related problems with 2 pages affected. 
 * @pre			Involved components running and functioning
 * @focus		CMD_TEST_FILL_MEM command is executable for over 1 page size memory insertion
 * @veri        Page and buffer boundaries are written from SWd to NWd and verified <br>
 * Data is written from NWd to SWd and verified.<br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00101_CheckBufferBoundariesSize1Page)
{
    checkBufferBoundaries((1 * PAGE_SIZE) + 1);
}

/**@description Verifies that the bidirectional NWd, SWd read write operations are successful <br>
* on the buffer and page boundaries for a buffer sized smaller than 3 pages.
 * @pre			Involved components running and functioning
 * @focus		CMD_TEST_FILL_MEM command is executable for over 2 page size memory insertion
 * @veri        Page and buffer boundaries are written from SWd to NWd and verified <br>
 * Data is written from NWd to SWd and verified.
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00102_CheckBufferBoundariesSize2Pages)
{
    checkBufferBoundaries((2 * PAGE_SIZE) + 1);
}

/**@description Verifies that the bidirectional NWd, SWd read write operations are successful <br>
* on the buffer and page boundaries for a buffer sized smaller than 4 pages.
 * @pre			Involved components running and functioning  
 * @focus		CMD_TEST_FILL_MEM command is executable for over 3 page size memory insertion
 * @veri        Page and buffer boundaries are written from SWd to NWd and verified <br>
 * Data is written from NWd to SWd and verified.
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00103_CheckBufferBoundariesSize3Pages)
{
    checkBufferBoundaries((3 * PAGE_SIZE) + 1);
}

/**@description Verifies that the bidirectional NWd, SWd read write operations are successful <br>
* on the buffer and page boundaries for a buffer sized smaller than 5 pages.
 * @pre			Involved components running and functioning
 * @focus		CMD_TEST_FILL_MEM command is executable for over 4 page size memory insertion
 * @veri        Page and buffer boundaries are written from SWd to NWd and verified <br>
 * Data is written from NWd to SWd and verified.<br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00104_CheckBufferBoundariesSize4Pages)
{
    checkBufferBoundaries((4 * PAGE_SIZE) + 1);
}

/**@description Verifies that the bidirectional NWd, SWd read write operations are successful <br>
 * on the buffer and page boundaries for a buffer sized smaller than 9 pages.
 * @pre			Involved components running and functioning
 * @focus		CMD_TEST_FILL_MEM command is executable for over 8 page size memory insertion
 * @veri        Page and buffer boundaries are written from SWd to NWd and verified <br>
 * Data is written from NWd to SWd and verified.
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00105_CheckBufferBoundariesSize8Pages)
{
    checkBufferBoundaries((8 * PAGE_SIZE) + 1);
}


// Check buffer fill and readback tests
/**@description <br>
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer. 
 * @veri        Data block is written to the NWd via SWd.<br>
 * Data is written from NWd to SWd and verified.<br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Bindings are unMap'ped and allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00106_CheckBufferFillAndReadbackSize1)
{
    checkBufferFillAndReadback(1);
}

/**@description Verifies that the integrity of  randomly generated data written <br>
 * into the NWd via SWd is kept totally. Data consistency check is tested for data <br>
 * larger than a page size. 
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        Data block which is larger than a page size is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00107_CheckBufferFillAndReadbackSize1Page)
{
    checkBufferFillAndReadback((1 * PAGE_SIZE) + 1);
}

/**@description <br>
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        Data block which is larger than 2 pages' size is written to the NWd via SWd.<br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00108_CheckBufferFillAndReadbackSize2Pages)
{
    checkBufferFillAndReadback((2 * PAGE_SIZE) + 1);
}

/**@description <br>
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        Data block which is larger than 3 pages' size is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00109_CheckBufferFillAndReadbackSize3Pages)
{
    checkBufferFillAndReadback((3 * PAGE_SIZE) + 1);
}

/**@description <br>
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        Data block which is larger than 4 pages' size is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00110_CheckBufferFillAndReadbackSize4Pages)
{
    checkBufferFillAndReadback((4 * PAGE_SIZE) + 1);
}

/**@description <br>
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        Data block which is larger than 8 pages' size is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00111_CheckBufferFillAndReadbackSize8Pages)
{
    checkBufferFillAndReadback((8 * PAGE_SIZE) + 1);
}


// Map all fill and readback tests
/**@description Verifies the SWd to NWd memory access with multiple buffers allocated, read and written.<br>
 * allocates 4 buffers with 1 byte, writes the randomly generated info to NWd memory via SWd and verifies the data.
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        4 identical data blocks which sized 1 byte is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00112_MapAllAndReadbackSize1)
{
    mapAllFillAndReadback(1);
}

/**@description Verifies the SWd to NWd memory access with multiple buffers allocated, read and written.<br>
 * allocates 4 buffers with data sized larger than 1 page size, writes the randomly generated info to <br>
 * NWd memory via SWd and verifies the data.
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        4 identical data blocks which are sized larger than 1 pages is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00113_MapAllAndReadbackSize1Page)
{
    mapAllFillAndReadback((1 * PAGE_SIZE) + 1);
}

/**@description Verifies the SWd to NWd memory access with multiple buffers allocated, read and written.<br>
 * allocates 4 buffers with data sized larger than 2 pages' size, writes the randomly generated info to <br>
 * NWd memory via SWd and verifies the data.
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        4 identical data blocks which are sized larger than 2 pages is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00114_MapAllAndReadbackSize2Pages)
{
    mapAllFillAndReadback((2 * PAGE_SIZE) + 1);
}

/**@description Verifies the SWd to NWd memory access with multiple buffers allocated, read and written.<br>
 * allocates 4 buffers with data sized larger than 3 pages' size, writes the randomly generated info to <br>
 * NWd memory via SWd and verifies the data.
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        4 identical data blocks which are sized larger than 3 pages is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00115_MapAllAndReadbackSize3Pages)
{
    mapAllFillAndReadback((3 * PAGE_SIZE) + 1);
}

/**@description Verifies the SWd to NWd memory access with multiple buffers allocated, read and written.<br>
 * allocates 4 buffers with data sized larger than 4 pages' size, writes the randomly generated info to <br>
 * NWd memory via SWd and verifies the data.
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        4 identical data blocks which are sized larger than 4 pages is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00116_MapAllAndReadbackSize4Pages)
{
    mapAllFillAndReadback((4 * PAGE_SIZE) + 1);
}

/**@description Verifies the SWd to NWd memory access with multiple buffers allocated, read and written.<br>
 * allocates 4 buffers with data sized larger than 8 pages' size, writes the randomly generated info to <br>
 * NWd memory via SWd and verifies the data.
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        4 identical data blocks which are sized larger than 8 pages is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00117_MapAllAndReadbackSize8Pages)
{
    mapAllFillAndReadback((8 * PAGE_SIZE) + 1);
}

/**@description Verifies the SWd to NWd memory access with multiple buffers allocated, read and written.<br>
 * allocates 4 buffers with data sized 255 pages' size, writes the randomly generated info to <br>
 * NWd memory via SWd and verifies the data.
 * @pre         Involved components running and functioning
 * @focus		mcMap call maps the SWd memory synchronizes with NWd buffer.
 * @veri        4 identical data blocks which are sized as big as 255 pages is written to the NWd via SWd. <br>
 * Data is written from NWd to SWd and verified. <br>
 * CMD_TEST_FILL_MEM and CMD_TEST_COMPARE_MEM commands are successfuly executed.
 * @post		Allocated fields are freed.
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00118_MapAllAndReadbackSize255Pages)
{
    mapAllFillAndReadback((255 * PAGE_SIZE));
}

/**@description TLC writes something in buffer and then maps the buffer to Trustlet<br>
 * right after the moment of mapping buffer is verified to be cleared totally.
 * @pre 		Involved components running and functioning
 * @focus		mcMap clears the buffer when mapping takes place
 * @veri     	Mapping does not clear the buffer<br>
 * Test output is OK.
 * @post		Allocated fields are freed.	
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,GC00119_MapDoesntClear)
{
	int one_page  = 4096;
    mcBulkMap_t mapInfo;

    // Allocate Heap memory
    uint8_t* bulkBuf = (uint8_t*)malloc(one_page);
    CHECK( NULL != bulkBuf );

    // Register memory in Kernel Module and inform MobiCore
	int len = 3110;
    uint8_t pattern = TestUtils::Random::getByte();
    memset(bulkBuf, pattern, len);

    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&sessionHandle, bulkBuf, one_page, &mapInfo));
	CHECK_EQUAL(one_page,  mapInfo.sVirtualLen );

	for(int i=0; i<len; i++)
	{
		CHECK_EQUAL(pattern, bulkBuf[i]);
	}

    // Check if address has been set
    LOG_I("Checking memory on SWd side (CMD_TEST_COMPARE_MEM)");
    tci->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
    tci->cmdTlTestCompareMem.dest = (uint32_t)mapInfo.sVirtualAddr;
    tci->cmdTlTestCompareMem.len = len;
    tci->cmdTlTestCompareMem.value = pattern;
    executeTciCommand(CMD_TEST_COMPARE_MEM);
    LOG_I("SUCCESS");

    // Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

    free(bulkBuf);
}
/**@description TLC maps a buffer to Trustlet and then orders Trustlet to
 * write next to the buffer.
 * @note		This is more a test of mcUnmap(), bad we keep it here because
 * it resembles the other tests here.
 * TODO: move test in separate file or to unmap scenario
 * @pre 		Involved components running and functioning
 * @focus		mcNotify informs TL to swtich the access offset in the buffer. 
 * @veri     	Trustlet pagefaults<br>
 * mcUnmap() returns MC_DRV_OK, even if trustlet is dead.
 * Test output is OK.
 * @post
 */
TESTCASE(MemoryManagement,00100_mcMapBoundaryTests,BC00120_TrustletPagefaultOnWrongAddress)
{
	int one_page  = 4096;
    mcBulkMap_t mapInfo;

    // Allocate Heap memory
    uint8_t* bulkBuf = (uint8_t*)malloc(one_page);
    CHECK( NULL != bulkBuf );

    // Register memory in Kernel Module and inform MobiCore
	int len = 3110;
    uint8_t pattern = TestUtils::Random::getByte();
    memset(bulkBuf, pattern, len);

    CHECK_DRVAPI_RETURNS_OK(
    		mcMap(&sessionHandle, bulkBuf, one_page, &mapInfo));
	CHECK_EQUAL(one_page,  mapInfo.sVirtualLen );

    // Let the Trustlet "Check" the memory
    LOG_I("Calling compare on invalid address (CMD_TEST_COMPARE_MEM)");
    tci->cmdTlTestCompareMem.cmdHeader.commandId = CMD_TEST_COMPARE_MEM;
    tci->cmdTlTestCompareMem.dest = (uint32_t)mapInfo.sVirtualAddr-one_page;
    tci->cmdTlTestCompareMem.len = len;
    tci->cmdTlTestCompareMem.value = pattern;

    CHECK_DRVAPI_RETURNS_OK(
    		mcNotify(&sessionHandle));

    // Wait for notification from Trustlet
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_INFO_NOTIFICATION,
    		mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

    // Request error code, must be "pagefault"
    int32_t lastErr;
    CHECK_DRVAPI_RETURNS_OK(
    		mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( ERR_INVALID_OPERATION, lastErr );
    LOG_I("SUCCESS");

    // Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
    		mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

    free(bulkBuf);
}
/**
 * @}
 * @} */
