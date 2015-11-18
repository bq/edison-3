/** @addtogroup TEST_MCDRVAPI
 * @{
 * @defgroup MCDRVMALLOCWSM 00300_mcMallocWsm
 * @{
 * Test Scenario for McDriverAPI function mcMallocWsm()<br>
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
 * - free allocated memory by calling mcFreeWsm() if it has been allocated
 * - close Device by calling mcCloseDevice()

 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "TrustletForwarder.h"
#include "errno.h"

TEST_SCENARIO(00300_mcMallocWsm)
{
    mcResult_t x;
    uint8_t *tci;

    void setup(void) {
    	FORWARDER_INIT;
        x = mcOpenDevice(MC_DEVICE_ID_DEFAULT);
        CHECK_DRVAPI_RETURNS_OK( x);
    }
    void teardown (void)
    {
        if(MC_DRV_OK == x)
            mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci);

        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    }
};

/**@description TLC allocates 4kB of WSM
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, &tci, 0)
 * @veri        MC_DRV_OK is returned
 * @post        -
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,GC00100_IdDefault4k)
{
    CHECK_DRVAPI_RETURNS_OK(
    		mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, &tci, 0));
}
/**@description TLC calls mcMallocWsm with an invalid device id (1)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcMallocWsm(1, 0, 4096, &tci, 0)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,BC00101_InvalidDevice1)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcMallocWsm(1, 0, 4096, &tci, 0));
}
/**@description TLC calls mcMallocWsm with an invalid device id (4)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcMallocWsm(4, 0, 4096, &tci, 0)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,BC00101_InvalidDevice4)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcMallocWsm(4, 0, 4096, &tci, 0));
}
/**@description TLC calls mcMallocWsm with an invalid device id (-1)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcMallocWsm(-1, 0, 4096, &tci, 0)
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        -
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,BC00102_InvalidDeviceMinus1)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_UNKNOWN_DEVICE,
    		mcMallocWsm(-1, 0, 4096, &tci, 0));
}
/**@description TLC calls mcMallocWsm with an unsupported len parameter (0)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 0, &tci, 0)
 * @veri        MC_DRV_ERR_NO_FREE_MEMORY is returned
 * @post        -
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,BC00103_Len0)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_NO_FREE_MEMORY,
    		mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 0, &tci, 0));
}
/**@description TLC calls mcMallocWsm with an unsupported len parameter (-1)
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, -1, &tci, 0)
 * @veri        MC_DRV_ERR_NO_FREE_MEMORY is returned
 * @post        -
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,BC00104_LenMinus1)
{
    CHECK_DRVAPI_RETURNS(
            MAKE_MC_DRV_KMOD_WITH_ERRNO(EFAULT),
    		mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, -1, &tci, 0));
}
/**@description TLC calls mcMallocWsm with a NULL pointer
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, 0, 0)
 * @veri        MC_DRV_ERR_INVALID_PARAMETER is returned
 * @post        -
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,BC00105_WsmNullPointer)
{
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_INVALID_PARAMETER,
    		mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, 0, 0));
}
/**@description TLC calls mcMallocWsm with a not yet opened device
 * @pre         Close device using the default ID
 * @focus       mcMallocWsm
 * @veri        MC_DRV_ERR_UNKNOWN_DEVICE is returned
 * @post        Open device again using the default ID
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,BC00106_ClosedDevice)
{
    CHECK_DRVAPI_RETURNS_OK(mcCloseDevice(MC_DEVICE_ID_DEFAULT));

    ::sleep(2);
    CHECK_DRVAPI_RETURNS(
    		MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN,
    		mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, 0, 0));
}
/**@description TLC calls mcMallocWsm 16 times<br>
 * After that, the memory (available to the TLC) is exhausted.<br>
 * After that, the TLC frees the memory again.<br>
 * Several runs of this test should not leak any other memory
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       call mcMallocWsm several times allocating blocks of 4kB
 * @veri        MC_DRV_OK is returned until no more memory is available then<br>
 * MC_DRV_ERR_NO_FREE_MEMORY is returned.
 * @post        All memory is freed again by calling mcFreeWsm(), no memory leaks occur
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,GC00107_StresstestMultipleAllocations)
{
#define MAX_ALLOCS 100
    uint8_t* a[MAX_ALLOCS];
for(int y=0; y<1; y++) {
    int j = 0;
    do {
        x = mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, &a[j], 0);

        if(MC_DRV_OK == x)
            ;
        else if(MC_DRV_ERR_NO_FREE_MEMORY == x)
            break;
        else
            CHECK_EQUAL(MC_DRV_ERR_NO_FREE_MEMORY, x);
        j++;
    }
    while(j < MAX_ALLOCS);
    printf("\n\tStress-test: mcMallocWsm called %i times, allocated %i chunks.\n",j,j-1);
    j--;//the last one was not allocated.
    //note: currently malloc gets called 16 times
    for(;j+1;j--) {
		CHECK_DRVAPI_RETURNS_OK( mcFreeWsm(MC_DEVICE_ID_DEFAULT, a[j]));
    }
}
    x = MC_DRV_ERR_NO_FREE_MEMORY;
}
/**@description TLC calls mcMallocWsm and iteratively increases the size.<br>
 * The TLC frees the memory after each allocation.<br>
 * Several runs of this test should not leak any other memory
 * @pre         A device has been opened using the default ID in Generic preconditions (setup)
 * @focus       call mcMallocWsm several times allocating blocks with increasing block size
 * @veri        MC_DRV_OK is returned until no more memory is available then<br>
 * MC_DRV_ERR_NO_FREE_MEMORY is returned.
 * @post        memory is not freed explicitely, but device is closed by generic post processing
 */
TESTCASE(McDrvApi,00300_mcMallocWsm,GC00108_StresstestMultipleSizes)
{
    printf("\n");
    int size = 4096/2,j=0;
    do {
        size *=2;
        j++;
        printf("\tStress-test: Calling mcMallocWsm with size %i == 0x%x. ",size,size);
        x = mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, size, &tci, 0);

        if(MC_DRV_OK == x) {
            printf("ok.");
            CHECK_DRVAPI_RETURNS_OK(
            	mcFreeWsm(MC_DEVICE_ID_DEFAULT, tci));
        }
        else if(MC_DRV_ERR_NO_FREE_MEMORY == x) {
            printf("no more memory.");
            size/=2;
            break;
        }
        else
            CHECK_EQUAL(MC_DRV_ERR_NO_FREE_MEMORY, x);
        printf("\n");
    }
    while(size < MC_MAX_TCI_LEN);
    printf("\n\tStress-test: mcMallocWsm called %i times, \n"
            "\t\tBiggest allocated chunk: %i byte.\n",
            j,size);

    //do not call free:
    x = MC_DRV_ERR_NO_FREE_MEMORY;
}
/**
 * @}
 * @} */
