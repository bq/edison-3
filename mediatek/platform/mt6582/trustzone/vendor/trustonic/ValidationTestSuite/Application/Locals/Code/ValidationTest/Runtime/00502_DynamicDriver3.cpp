/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup MCDYNAMICDRIVER3 00502_DynamicDriver3
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
 * -# DrIntegrationTest Driver
 * 		-# Module loaded
 * 		-# Module socket accessible by daemon
 * 		-# Module listens for connection
 * -# Trustlet TlTestIntegrationTest
 * 		-# Trustlet copied to Android on Test device in /data/app/ as  05010000000000000000000000000000.trst
 * 		-# Trustlet runnable and bug-free
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */


#include <stdlib.h>
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "tlTestApi.h"
#include "DrIntegrationTestApi.h"
#include "Mci/mcinq.h"
#include <strings.h>

#define _TLAPI_EXTERN_C extern "C"
#include <sys/mman.h>
#include "TrustletForwarder.h"

#define LOG_WANT_BUF
#define LOG_TAG "Test_DynamicDriver"
#include <log.h>


#define TCI_OFFSET_MARSHALLING_BUFFER 32
#define BULK_BUF_LEN 0x100000		// 1 MB


TEST_SCENARIO(00502_DynamicDriver3)
{
	mcResult_t x;
	tciMessage_t *tci1;
	tciMessage_t *tci2;
	tciMessage_t *tci_driver;
    mcSessionHandle_t sessionHandle1;
    mcSessionHandle_t sessionHandle2;
    mcSessionHandle_t sessionHandle_driver;

	void setup(void) {
		FORWARDER_INIT;

		mcUuid_t uuid = TL_TEST_UUID;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenDevice(MC_DEVICE_ID_DEFAULT));

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci1, 0));

		// Clear the session handle
		bzero(&sessionHandle1, sizeof(sessionHandle1));

		// Set default device ID
		sessionHandle1.deviceId = MC_DEVICE_ID_DEFAULT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle1, &uuid, (uint8_t *)tci1, (uint32_t) sizeof(tciMessage_t)));

		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci2, 0));

		// Clear the session handle
		bzero(&sessionHandle2, sizeof(sessionHandle2));

		// Set default device ID
		sessionHandle2.deviceId = MC_DEVICE_ID_DEFAULT;

		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle2, &uuid, (uint8_t *)tci2, (uint32_t) sizeof(tciMessage_t)));

		//!!!THP faked driver as trustlet
		// Allocate WSM buffer (4k)
		CHECK_DRVAPI_RETURNS_OK(
				mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci_driver, 0));

		// Clear the session handle
		bzero(&sessionHandle_driver, sizeof(sessionHandle_driver));

		// Set default device ID
		sessionHandle_driver.deviceId = MC_DEVICE_ID_DEFAULT;

		mcUuid_t uuid_driver = DRV_DRINTEGRATIONSTEST_UUID;
		CHECK_DRVAPI_RETURNS_OK(
				mcOpenSession(&sessionHandle_driver, &uuid_driver, (uint8_t *)tci_driver, (uint32_t) sizeof(tciMessage_t)));
	}
	void teardown (void)
	{
		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle1));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci1));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle2));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci2));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseSession(&sessionHandle_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci_driver));

		CHECK_DRVAPI_RETURNS_OK(
				mcCloseDevice(MC_DEVICE_ID_DEFAULT));
	}
};



/**@description Trustlet Connector opens two Trustlets
 * @pre 		Involved components running and functioning
 * Testflow.<br>
 */
//IGNORE_TESTCASE
TESTCASE(Runtime,00502_DynamicDriver3,GC00100_DriverMemoryMapping)
{
	uint8_t *bulkBuf21;
	uint8_t *bulkBuf22;
	uint8_t *bulkBuf31;
	uint8_t *bulkBuf32;
    mcBulkMap_t mapInfo21;
    mcBulkMap_t mapInfo22;
    mcBulkMap_t mapInfo31;
    mcBulkMap_t mapInfo32;
    uint32_t index;
    char *cptr;

	mcUuid_t uuid = TL_TEST_UUID;

	uint8_t *marshalling1 = &((uint8_t*)tci1)[TCI_OFFSET_MARSHALLING_BUFFER];
	uint8_t *marshalling2 = &((uint8_t*)tci2)[TCI_OFFSET_MARSHALLING_BUFFER];

	cmd_echo_t        *cmd_echo1 =  (cmd_echo_t*) marshalling1;
	rsp_echo_t        *rsp_echo1 =  (rsp_echo_t*) marshalling1;
	cmd_echo_t        *cmd_echo2 =  (cmd_echo_t*) marshalling2;
	rsp_echo_t        *rsp_echo2 =  (rsp_echo_t*) marshalling2;

	// Allocate Heap memory 1-1 with Trustlet so pointers work both sides.
	bulkBuf21 = (uint8_t*) memalign(4096, BULK_BUF_LEN);
	CHECK(bulkBuf21 != 0);
	bulkBuf22 = (uint8_t*) memalign(4096, BULK_BUF_LEN);
	CHECK(bulkBuf22 != 0);
	bulkBuf31 = (uint8_t*) memalign(4096, BULK_BUF_LEN);
	CHECK(bulkBuf31 != 0);
	bulkBuf32 = (uint8_t*) memalign(4096, BULK_BUF_LEN);
	CHECK(bulkBuf32 != 0);


    // map trustlet1 shared memory
    CHECK_DRVAPI_RETURNS_OK(
    	mcMap(&sessionHandle1, bulkBuf21, BULK_BUF_LEN, &mapInfo21));
    CHECK_DRVAPI_RETURNS_OK(
    	mcMap(&sessionHandle1, bulkBuf22, BULK_BUF_LEN, &mapInfo22));

    // map trustlet2 shared memory
    CHECK_DRVAPI_RETURNS_OK(
    	mcMap(&sessionHandle2, bulkBuf31, BULK_BUF_LEN, &mapInfo31));
    CHECK_DRVAPI_RETURNS_OK(
    	mcMap(&sessionHandle2, bulkBuf32, BULK_BUF_LEN, &mapInfo32));

    memset(bulkBuf21,'1',BULK_BUF_LEN);

	////////////////// Copy

    // Prepare command message
    tci1->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
    tci1->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
    cmd_echo1->cmdHeader.commandId = CMD_ECHO;
    cmd_echo1->src  = (uint32_t) mapInfo21.sVirtualAddr;
    cmd_echo1->dest = (uint32_t) mapInfo22.sVirtualAddr;
    cmd_echo1->len  = BULK_BUF_LEN;

    
	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle1));

	
    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle1, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_ECHO) , rsp_echo1->rspHeader.responseId);
    CHECK_EQUAL( RET_OK , rsp_echo1->rspHeader.returnCode);

    // check correct copy
    cptr = (char*) bulkBuf22;
    for(index=0; index<BULK_BUF_LEN; index++)
    	if(cptr[index] != '1')
    	    CHECK_EQUAL( '1' , cptr[index]);


    memset(bulkBuf31,'2',BULK_BUF_LEN);

	////////////////// Copy

    // Prepare command message
    tci2->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
    tci2->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
    cmd_echo2->cmdHeader.commandId = CMD_ECHO;
    cmd_echo2->src  = (uint32_t) mapInfo31.sVirtualAddr;
    cmd_echo2->dest = (uint32_t) mapInfo32.sVirtualAddr;
    cmd_echo2->len  = BULK_BUF_LEN;


    
	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle2));

	
    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle2, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_ECHO) , rsp_echo2->rspHeader.responseId);
    CHECK_EQUAL( RET_OK , rsp_echo2->rspHeader.returnCode);


//    CHECK_EQUAL( 0 , 1);

    // check correct copy
    cptr = (char*) bulkBuf32;
    for(index=0; index<BULK_BUF_LEN; index++)
    	if(cptr[index] != '2')
    	    CHECK_EQUAL( '2' , cptr[index]);

    // check trustlet 1 not effected
    cptr = (char*) bulkBuf21;
    for(index=0; index<BULK_BUF_LEN; index++)
    	if(cptr[index] != '1')
    	    CHECK_EQUAL( '1' , cptr[index]);
    cptr = (char*) bulkBuf22;
    for(index=0; index<BULK_BUF_LEN; index++)
    	if(cptr[index] != '1')
    	    CHECK_EQUAL( '1' , cptr[index]);


    memset(bulkBuf21,'3',BULK_BUF_LEN);

	////////////////// Copy

    // Prepare command message
    tci1->cmdTlTestAnyDriver.cmdHeader.commandId = CMD_TEST_ANY_DRIVER;
    tci1->cmdTlTestAnyDriver.sVirtualAddr = (uint32_t)TRUSTLET_SHARED_MEM_D1 + TCI_OFFSET_MARSHALLING_BUFFER;
    cmd_echo1->cmdHeader.commandId = CMD_ECHO;
    cmd_echo1->src  = (uint32_t) mapInfo21.sVirtualAddr;
    cmd_echo1->dest = (uint32_t) mapInfo22.sVirtualAddr;
    cmd_echo1->len  = BULK_BUF_LEN;

    
	CHECK_DRVAPI_RETURNS_OK(
			mcNotify(&sessionHandle1));

	
    CHECK_DRVAPI_RETURNS_OK(
    		mcWaitNotification(&sessionHandle1, MC_INFINITE_TIMEOUT));
    CHECK_EQUAL( RSP_ID(CMD_ECHO) , rsp_echo1->rspHeader.responseId);
    CHECK_EQUAL( RET_OK , rsp_echo1->rspHeader.returnCode);

    // check correct copy
    cptr = (char*) bulkBuf22;
    for(index=0; index<BULK_BUF_LEN; index++)
    	if(cptr[index] != '3')
    	    CHECK_EQUAL( '3' , cptr[index]);


    // check trustlet 2 not effected
    cptr = (char*) bulkBuf31;
    for(index=0; index<BULK_BUF_LEN; index++)
    	if(cptr[index] != '2')
    	    CHECK_EQUAL( '2' , cptr[index]);
    cptr = (char*) bulkBuf32;
    for(index=0; index<BULK_BUF_LEN; index++)
    	if(cptr[index] != '2')
    	    CHECK_EQUAL( '2' , cptr[index]);


    // Unmap Trustlets shared memory
    CHECK_DRVAPI_RETURNS_OK(
    	mcUnmap(&sessionHandle1, bulkBuf21, &mapInfo21));
    CHECK_DRVAPI_RETURNS_OK(
    	mcUnmap(&sessionHandle1, bulkBuf22, &mapInfo22));
    CHECK_DRVAPI_RETURNS_OK(
    	mcUnmap(&sessionHandle2, bulkBuf31, &mapInfo31));
    CHECK_DRVAPI_RETURNS_OK(
    	mcUnmap(&sessionHandle2, bulkBuf32, &mapInfo32));


    free(bulkBuf21);
    free(bulkBuf22);
    free(bulkBuf31);
    free(bulkBuf32);

}

/**
 * @}
 * @} */
