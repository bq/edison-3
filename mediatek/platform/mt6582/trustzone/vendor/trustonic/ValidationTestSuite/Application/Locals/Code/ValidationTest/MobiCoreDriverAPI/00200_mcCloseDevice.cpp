/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVCLOSEDEVICE 00200_mcCloseDevice
 * @{
 * Test Scenario for McDriverAPI function mcCloseDevice()<br>
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
 *
 * <b>Generic preconditions (setup)</b><br>
 * Generic Preconditions are the same for every test case.
 * They are executed always in front of every single test case.
 * - open device using the default ID by calling mcOpenDevice()
 *
 * <b>Generic post processing (teardown)</b><br>
 * Generic post processing is the same for every test case.
 * It is executed always after every single test case.
 * - close Device by calling mcCloseDevice() if it is not already closed

 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "TrustletForwarder.h"

TEST_SCENARIO(00200_mcCloseDevice)
{
    mcResult_t x;

    void setup(void) {
    	FORWARDER_INIT;
	    CHECK_DRVAPI_RETURNS_OK(
	            mcOpenDevice(MC_DEVICE_ID_DEFAULT));
    }
    void teardown (void)
    {
        if(MC_DRV_OK != x)
            mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    }
};
/**@description TLC closes a device session using the default device ID (MC_DEVICE_ID_DEFAULT)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcCloseDevice(MC_DEVICE_ID_DEFAULT)
 * @veri        MC_DRV_OK is returned
 * @post        -
 */
TESTCASE(McDrvApi,00200_mcCloseDevice,GC00100_IdDefault)
{
    CHECK_DRVAPI_RETURNS_OK(
            mcCloseDevice(MC_DEVICE_ID_DEFAULT));
}
/**@description TLC closes a device with an invalid device ID (1)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcCloseDevice(1)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00200_mcCloseDevice,BC00101_InvalidDevice1)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcCloseDevice(1));
}
/**@description TLC closes a device with an invalid device ID (2)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcCloseDevice(2)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00200_mcCloseDevice,BC00102_InvalidDevice2)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcCloseDevice(2));
}
/**@description TLC closes a device with an invalid device ID (-1)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcOpenDevice(-1)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00200_mcCloseDevice,BC00103_InvalidDeviceMinus1)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcCloseDevice(-1));
}
/**@description TLC closes a device which is already closed
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)<br>1. mcCloseDevice(MC_DEVICE_ID_DEFAULT) => returns MC_DRV_OK
 * @focus       2. mcCloseDevice(MC_DEVICE_ID_DEFAULT)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned for second time
 * @post        -
 */
TESTCASE(McDrvApi,00200_mcCloseDevice,BC00103_CloseRepeatedly)
{
    CHECK_DRVAPI_RETURNS_OK(
    		mcCloseDevice(MC_DEVICE_ID_DEFAULT));

    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN,
    		mcCloseDevice(MC_DEVICE_ID_DEFAULT));
}
/**
 * @}
 * @} */
