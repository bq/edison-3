/** @addtogroup TEST_RUNTIME
 * @{
 * @defgroup CPUCOREAVAILABILITY 00600_CpuCoreAvailability
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
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */

#include <stdlib.h>
#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "Mci/mcinq.h"
#include <sys/syscall.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// #define __NR_sched_setaffinity 122 // Our own set affinity syscall id
// #define __NR_sched_getaffinity 123 // Our own get affinity syscall id
#define LOG_WANT_BUF
#define LOG_TAG "Test_CpuPlugging"
#include <log.h>

#define TCI_OFFSET_MARSHALLING_BUFFER 32
#define BULK_BUF_LEN 0x100000		// 1 MB


TEST_SCENARIO(00600_CpuCoreAvailability)
{

	void setup(void) {


	}
	void teardown (void)
	{
		// Close session
	}

    bool checkdebug()
    {
    #ifndef NDEBUG
        {
        	return true;
        }
    #endif
        return false;
    }

};

/**@description Mobicore caused other secondary cpu boots to fail when the process runs  on Cpu1  <br>
 * this test has been developed to make sure if a process over Cpu1 shuts down another secondary  <br>
 * cpu can hotplug  the core back up again.
 * Important note: Since Power Management (PM) aggressively takes the un-required cores down <br>
 * this test results as passed when PM is enabled, yet the test is not run actually.
 * @pre Running linux kernel
 * @focus fread returns 1 for Cpu2 from /sys/devices/system/cpu/cpu2/online
 * @veri read ingredients of /sys/devices/system/cpu/cpu2/online after plugging out and plugging in.
 * @post
 */


TESTCASE(Runtime,00600_CpuCoreAvailability, GC00100_PlugOut_and_PlugIn_Cpu1)
{
	int err, syscallres, mask = 0x2;

	const char * proc1loc = "/sys/devices/system/cpu/cpu1/online";
	const char * proc2loc = "/sys/devices/system/cpu/cpu2/online";
	const char * cpuActive = "1";
	const char * cpuInActive = "0";
	char * cpustate = (char *)alloca(5);


	// Lets  see if we have more than two cores
	FILE * fpCpu2 = fopen(proc2loc, "w");
	if (fpCpu2 == 0) {
		return;
	}

	// Lets see if PM is enabled
	FILE * fpCpu1 = fopen(proc1loc, "w");
	fputs(cpuActive, fpCpu1);
	fclose(fpCpu1);
		// Sleep for a while
	sleep(1);
		// Read the new state
	fpCpu1 = fopen(proc1loc, "r");
	fread(cpustate, 5, 1, fpCpu1);
	if (strncmp(cpustate, cpuInActive, 1) == 0) {
		LOG_E("Test GC00100_PlugOut_and_PlugIn_Cpu1 has been skipped power management is enabled");
		return;
	}



	// Switch to cpu 1
	pid_t pid = getpid();

	syscallres = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
	if (syscallres) {
	        err = errno;
	        LOG_E("Error in the syscall setaffinity: mask=%d=0x%x err=%d=0x%x", mask, mask, err, err);
	}

	// If it is 0 then we are on cpu1
	CHECK_EQUAL(0, syscallres);

	// Inactivate the cpu 2
	fputs(cpuInActive, fpCpu2);
	fclose(fpCpu2);

	// Sleep for a while
	sleep(1);

	// Activate cpu 2
	fpCpu2 = fopen(proc2loc, "w");
	fputs(cpuActive, fpCpu2);
	fclose(fpCpu2);

	// Sleep for a while
	sleep(1);

	// Read the new state
	fpCpu2 = fopen(proc2loc, "r");
	fread(cpustate, 5, 1, fpCpu2);

	// Check the char. Needs to be 1
	CHECK_EQUAL(0, strncmp(cpustate, cpuActive, 1));

	fclose(fpCpu2);

}

/**
 * @}
 * @} */


