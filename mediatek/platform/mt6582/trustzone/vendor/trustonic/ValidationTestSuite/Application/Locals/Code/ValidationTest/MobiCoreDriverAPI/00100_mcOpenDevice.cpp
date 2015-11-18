/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVOPENDEVICE 00100_mcOpenDevice
 * @{
 * Test Scenario for McDriverAPI function mcOpenDevice()<br>
 *
 * <b>Involved Components and their required Functionality</b>
 *
 * -# ClientLib
 *      -# Can connect to daemon
 *      -# Can reserve memory for connection
 * -# McDriverDaemon
 *      -# Daemon running
 *      -# Daemon socket accessible by TLC
 * -# McDriverModule
 *      -# Module loaded
 *      -# Module socket accessible by daemon
 *      -# Module listens for connection
 *
 * <b>Generic preconditions (setup)</b> <br>
 * Generic Preconditions are the same for every test case.
 * They are executed always in front of every single test case.
 * - none
 *
 * <b>Generic post processing (teardown)</b> <br>
 * Generic post processing is the same for every test case.
 * It is executed always after every single test case.
 * - close Device by calling mcCloseDevice()

 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "TrustletForwarder.h"

TEST_SCENARIO(00100_mcOpenDevice)
{
    mcResult_t x;

    void setup(void) {
    	FORWARDER_INIT;
        x = MC_DRV_OK;
    }
    void teardown (void)
    {
        if(MC_DRV_OK == x) {
            CHECK_DRVAPI_RETURNS_OK(
                    mcCloseDevice(MC_DEVICE_ID_DEFAULT));
        }
    }
};
/**@description TLC opens a new default device (MC_DEVICE_ID_DEFAULT)
 * @pre         -
 * @focus       mcOpenDevice(MC_DEVICE_ID_DEFAULT)
 * @veri        MC_DRV_OK is returned
 * @post        -
 */
TESTCASE(McDrvApi,00100_mcOpenDevice,GC00100_IdDefault)
{
    CHECK_DRVAPI_RETURNS_OK(
            mcOpenDevice(MC_DEVICE_ID_DEFAULT));
}
/**@description TLC opens an invalid device (1)
 * @pre         -
 * @focus       mcOpenDevice(1)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00100_mcOpenDevice,BC00101_InvalidDevice1)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_UNKNOWN_DEVICE,
            mcOpenDevice(1));
}
/**@description TLC opens an invalid device (2)
 * @pre         -
 * @focus       mcOpenDevice(2)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00100_mcOpenDevice,BC00102_InvalidDevice2)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_UNKNOWN_DEVICE,
            mcOpenDevice(2));
}
/**@description TLC opens an invalid device (-1)
 * @pre         -
 * @focus       mcOpenDevice(-1)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00100_mcOpenDevice,BC00103_InvalidDeviceMinus1)
{
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_UNKNOWN_DEVICE,
            mcOpenDevice(-1));
}
/**@description TLC opens same device a second time
 * @pre         1. mcOpenDevice(MC_DEVICE_ID_DEFAULT) => returns MC_DRV_OK
 * @focus       2. mcOpenDevice(MC_DEVICE_ID_DEFAULT)
 * @veri        MC_DRV_ERR_INVALID_OPERATION is returned for second time
 * @post        -
 */
TESTCASE(McDrvApi,00100_mcOpenDevice,BC00104_OpenRepeatedly)
{
    CHECK_DRVAPI_RETURNS_OK(
            mcOpenDevice(MC_DEVICE_ID_DEFAULT));

    mcResult_t y = x;
    CHECK_DRVAPI_RETURNS(
            MC_DRV_ERR_INVALID_OPERATION,
            mcOpenDevice(MC_DEVICE_ID_DEFAULT));
    x=y;
}
/**
 * @}
 * @} */
