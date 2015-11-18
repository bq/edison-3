/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVFREEWSM 00400_mcFreeWsm
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
 * <b>Generic preconditions (setup)</b><br>
 * Generic Preconditions are the same for every test case.
 * They are executed always in front of every single test case.
 * - open device using the default ID by calling mcOpenDevice()
 * - allocate a WSM buffer (4k)
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

TEST_SCENARIO(00400_mcFreeWsm)
{
	mcResult_t x;
    uint8_t *tci;
      
	void setup(void) {
	   if(TESTPARAM==1) //Nothing to do if the test parameter is 2 (BC00104_OnClosedDevice)
	   {
		   FORWARDER_INIT;

		   CHECK_DRVAPI_RETURNS_OK(
				   mcOpenDevice(MC_DEVICE_ID_DEFAULT));

		   // Allocate WSM buffer (4k)
		   CHECK_DRVAPI_RETURNS_OK(
				   mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci, 0));
		}
	}
	void teardown (void)
	{
	   if(TESTPARAM==1)
	   {
		   if(MC_DRV_OK != x)
			   mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);

		   mcCloseDevice(MC_DEVICE_ID_DEFAULT);
		}
	}
};
/**@description 1. TLC allocates a block of WSM. <br>
 * 				2. TLC frees the block again
 * @pre 		Involved components running and functioning
 * @focus		mcFreeWsm works correctly.
 * @veri     	All checks pass, Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00400_mcFreeWsm,GC00100_DeviceDefault,1)
{
	CHECK_DRVAPI_RETURNS_OK(
			mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci));
}
/**@description TLC calls mcFreeeWsm with an unknown device id (1)
 * @pre 		Involved components running and functioning
 * @focus		mcFreeWsm returns an error for an unknown device id.
 * @veri     	mcFreeWsm() returns MC_DRV_ERR_UNKNOWN_DEVICE, Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00400_mcFreeWsm,BC00101_DeviceInvalid,1)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcFreeWsm(1, tci));
}
/**@description TLC calls mcFreeeWsm with a NULL pointer
 * @pre 		Involved components running and functioning
 * @focus 		mcFreeWsm returns an error for a null device id parameter.
 * @veri     	mcFreeWsm() returns MC_DRV_ERR_INVALID_PARAMETER, Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00400_mcFreeWsm,BC00102_WsmNullPointer,1)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcFreeWsm(MC_DEVICE_ID_DEFAULT, 0));
}
/**@description TLC calls mcFreeeWsm with memory not managed by MobiCore Driver
 * @pre 		Involved components running and functioning
 * @focus		mcFreeWsm returns an error for a parameter points a memory block that is not controlled by driver.
 * @veri     	mcFreeWsm() returns MC_DRV_ERR_INVALID_PARAMETER, Test output is OK
 * @post 
 */
TESTCASE(McDrvApi,00400_mcFreeWsm,BC00103_MemoryNotManagedByLib,1)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t*)&tci));
}
/**@description TLC calls mcFreeeWsm on a not yet opened device
 * @pre 		Involved components running and functioning
 * @focus		mcFreeWsm returns an error for a parameter points an unknown device.
 * @veri     	mcFreeWsm() returns MC_DRV_ERR_UNKNOWN_DEVICE, Test output is OK
 * @post
 */
TESTCASE(McDrvApi,00400_mcFreeWsm,BC00104_OnClosedDevice,2)
{
	mcResult_t x;
	uint8_t mem;
	uint8_t* tci = &mem;
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci));
}

/**
 * @}
 * @} */
