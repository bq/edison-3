/**
 * @addtogroup TEST_TLAPI
 *
 * <h2>Introduction</h2>
 * Tests checking the basic functionality of the Trustlet interface.
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */

#include "CppUTest/TestHarness.h"
#include "MobiCoreDriverApi.h"
#include "tlTestApi.h"
#include "Mci/mcinq.h"
#include <strings.h>

#define _TLAPI_EXTERN_C extern "C"
#include "TlApiMarshal.h"
#include "TrustletForwarder.h"
#include <sys/mman.h>
#include <errno.h>

uint32_t data_symKey[4] 	= {0x80000000,00000000,00000000,00000000};
uint32_t data_plainText[4] 	= {00000000,00000000,00000000,00000000};
uint32_t data_cipherText[4] = {0x9c8e8479,0xcdf8c230,0xf725b3a8,0x39b1d2fe};

//save important pointers outside so we can access them coming from c.
static TrustletApiForwarder *globalForwarder;
TrustletApiForwarder *TrustletApiForwarder::getInstance(void) {
    if(0 == globalForwarder) {
        globalForwarder     = new TrustletApiForwarder();
    }

    return globalForwarder;
}


TrustletApiForwarder::TrustletApiForwarder(void) {
    state = state_uninitialized;
    setup();
};

void TrustletApiForwarder::setup(void) {
    if(state_uninitialized != state) {
        FAIL("Calling setup twice!");
        return;
    }

    CHECK_DRVAPI_RETURNS_OK(
            mcOpenDevice(MC_DEVICE_ID_DEFAULT));

    // Allocate WSM buffer (4k)
    CHECK_DRVAPI_RETURNS_OK(
            mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, 4096, (uint8_t **) &tci, 0));

    // Allocate Heap memory
    //bulkBuf = (uint8_t*)memalign(4096, TRUSTLET_SHARED_MEM_D2_SIZE);
    //CHECK( NULL != bulkBuf );

    // Allocate Heap memory 1-1 with Trustlet so pointers work both sides.
    bulkBuf = (uint8_t*)mmap(TRUSTLET_SHARED_MEM_D2, TRUSTLET_SHARED_MEM_D2_SIZE,
            PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    CHECK_EQUAL( TRUSTLET_SHARED_MEM_D2, bulkBuf );

    bulkBuf2 = (uint8_t*)mmap(TRUSTLET_SHARED_MEM_D3, TRUSTLET_SHARED_MEM_D3_SIZE,
            PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    //printf("\n\tJust to make sure, buffer is 4k aligned: %x",bulkBuf);

    state = state_setup;

    openTrustletSession();
}

void TrustletApiForwarder::openTrustletSession(void) {
    if(state_setup != state) {
        printf("TrustletApiForwarder.state=%i",state);
        FAIL("state mismatch.");
        return;
    }

    // Clear the session handle
    bzero(&sessionHandle, sizeof(sessionHandle));

    // Set default device ID
    sessionHandle.deviceId = MC_DEVICE_ID_DEFAULT;

    mcUuid_t uuid = TL_TEST_UUID;

    CHECK_DRVAPI_RETURNS_OK(
            mcOpenSession(&sessionHandle, &uuid, (uint8_t *)tci, (uint32_t) sizeof(tciMessage_t)));

    // Register memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
            mcMap(&sessionHandle, bulkBuf, TRUSTLET_SHARED_MEM_D2_SIZE, &mapInfo));
    CHECK_EQUAL( TRUSTLET_SHARED_MEM_D2, mapInfo.sVirtualAddr );

    //Tell Trustlet about mapped memory
    tci->cmdTlTestCrypto.sVirtualAddr = (uint32_t)mapInfo.sVirtualAddr;

    state = state_trustlet_opened;

    mapD3ForKey();

    freePointer = 0;
    freePointer2 = 0;
    freeAll();
}

void TrustletApiForwarder::mapD3ForKey(void) {
    if(state_trustlet_opened != state) {
        printf("TrustletApiForwarder.state=%i",state);
        FAIL("state mismatch.");
        return;
    }

    // Register memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
            mcMap(&sessionHandle, bulkBuf2, TRUSTLET_SHARED_MEM_D3_SIZE, &mapInfo2));
    CHECK_EQUAL( TRUSTLET_SHARED_MEM_D3, mapInfo2.sVirtualAddr );

    state = state_D3_mapped;
}

void TrustletApiForwarder::unmapD3ForKey(void) {
    if(state_D3_mapped != state) {
        printf("TrustletApiForwarder.state=%i",state);
        FAIL("state mismatch.");
        return;
    }
    CHECK_DRVAPI_RETURNS_OK(
            mcUnmap(&sessionHandle, bulkBuf2, &mapInfo2));
    state = state_trustlet_opened;
}

void TrustletApiForwarder::teardownTrustletSession (void) {
    if(state_trustlet_opened != state && state_D3_mapped != state) {
        printf("TrustletApiForwarder.state=%i",state);
        FAIL("state mismatch.");
        return;
    }
    // Unregister memory in Kernel Module and inform MobiCore
    CHECK_DRVAPI_RETURNS_OK(
            mcUnmap(&sessionHandle, bulkBuf, &mapInfo));

    if(state_D3_mapped == state) {
        unmapD3ForKey();
    }

    
    CHECK_DRVAPI_RETURNS_OK(
            mcCloseSession(&sessionHandle));

    state = state_setup;
}

void TrustletApiForwarder::restartTrustletSession(void) {
    teardownTrustletSession();
    openTrustletSession();
}

void TrustletApiForwarder::teardown (void)
{
    if(state_uninitialized == state) {
        printf("TrustletApiForwarder.state=%i",state);
        FAIL("state mismatch.");
        return;
    }

    if(state_trustlet_opened == state || state_D3_mapped == state) {
        teardownTrustletSession();
    }

    munmap(bulkBuf, TRUSTLET_SHARED_MEM_D2_SIZE);

    munmap(bulkBuf2, TRUSTLET_SHARED_MEM_D3_SIZE);

    CHECK_DRVAPI_RETURNS_OK(
            mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)tci));

    CHECK_DRVAPI_RETURNS_OK(
            mcCloseDevice(MC_DEVICE_ID_DEFAULT));
}

TrustletApiForwarder::~TrustletApiForwarder(void) {
    teardown();
};

void TrustletApiForwarder::ensureNotUsed(void) {
    if(0 != globalForwarder) {
        delete globalForwarder;
        globalForwarder = 0;
    }
};

tlApiResult_t TrustletApiForwarder::executeCrypto(void) {
    // Prepare command message
    tci->commandHeader.commandId = CMD_TEST_CRYPTO;

    //Tell Trustlet about mapped memory
    tci->cmdTlTestCrypto.sVirtualAddr = (uint32_t)mapInfo.sVirtualAddr;

    
    CHECK_DRVAPI_RETURNS_OK(
            mcNotify(&sessionHandle));;

    
    CHECK_DRVAPI_RETURNS_OK(
            mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

    // Check if response ID set
    CHECK_EQUAL( RSP_ID(CMD_TEST_CRYPTO), tci->responseHeader.responseId );

    return (tlApiResult_t)tci->responseHeader.returnCode;
}

void TrustletApiForwarder::executeExitTrustlet(void) {
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 0;       // Send notification
    tci->cmdTlTestNotify.exitCode = (uint32_t)(-10);  // negative exit codes are not allowed.
    tci->cmdTlTestNotify.crash = 0; // Don't crash

    CHECK_DRVAPI_RETURNS_OK(
            mcNotify(&sessionHandle));

    CHECK_DRVAPI_RETURNS(
            MC_DRV_INFO_NOTIFICATION,
            mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

    // Request error code
    int32_t lastErr = 0;
    CHECK_DRVAPI_RETURNS_OK(
            mcGetSessionErrorCode(&sessionHandle, &lastErr));
    CHECK_EQUAL( ERR_INVALID_EXIT_CODE, lastErr );
}

void TrustletApiForwarder::executeCrashTrustlet(void) {
    // Prepare command message
    tci->cmdTlTestNotify.cmdHeader.commandId = CMD_TEST_NOTIFY;
    tci->cmdTlTestNotify.num = 0;       // Send notification
    tci->cmdTlTestNotify.exitCode = 0;  // negative exit codes are not allowed.
    tci->cmdTlTestNotify.crash = 1; // Don't crash

    CHECK_DRVAPI_RETURNS_OK(
            mcNotify(&sessionHandle));

    CHECK_DRVAPI_RETURNS(
            MC_DRV_INFO_NOTIFICATION,
            mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT));

    // Request error code
    int32_t lastErr = 0;
    CHECK_DRVAPI_RETURNS_OK(
            mcGetSessionErrorCode(&sessionHandle, &lastErr));
    // Kernel exception shall be signaled
    CHECK_EQUAL( ERR_INVALID_OPERATION, lastErr );
}
#define is_null(ptr)    ((ptr) == NULL) /**< returns true if pointer is NULL. */

uint32_t TrustletApiForwarder::svc(
        uint32_t syscallno,
        uint32_t *reg0,
        uint32_t *reg1,
        uint32_t *reg2,
        uint32_t *reg3,
        uint32_t *reg4,
        mcResult_t *fromMcWaitNotification,
        int32_t *sessionErrorCode
) {
    // Prepare command message
    tci->commandHeader.commandId = CMD_TEST_SVC;
    tci->cmdTlTestSvc.syscallno = syscallno;
    tci->cmdTlTestSvc.reg0 = 0;
    tci->cmdTlTestSvc.reg1 = 0;
    tci->cmdTlTestSvc.reg2 = 0;
    tci->cmdTlTestSvc.reg3 = 0;
    tci->cmdTlTestSvc.reg4 = 0;

    if(!is_null(reg0)) { tci->cmdTlTestSvc.reg0 = *reg0; }
    if(!is_null(reg1)) { tci->cmdTlTestSvc.reg1 = *reg1; }
    if(!is_null(reg2)) { tci->cmdTlTestSvc.reg2 = *reg2; }
    if(!is_null(reg3)) { tci->cmdTlTestSvc.reg3 = *reg3; }
    if(!is_null(reg4)) { tci->cmdTlTestSvc.reg4 = *reg4; }

    
    CHECK_DRVAPI_RETURNS_OK(
            mcNotify(&sessionHandle));;

    
    mcResult_t res = mcWaitNotification(&sessionHandle, MC_INFINITE_TIMEOUT);
    if(!is_null(fromMcWaitNotification)) {
        *fromMcWaitNotification = res;
    }
    // Request error code
    mcGetSessionErrorCode(&sessionHandle, sessionErrorCode);

    if(MC_DRV_OK == res) {
        // Check if response ID set
        CHECK_EQUAL( RSP_ID(CMD_TEST_SVC), tci->responseHeader.responseId );

        if(!is_null(reg0)) { *reg0 = tci->cmdTlTestSvc.reg0; }
        if(!is_null(reg1)) { *reg1 = tci->cmdTlTestSvc.reg1; }
        if(!is_null(reg2)) { *reg2 = tci->cmdTlTestSvc.reg2; }
        if(!is_null(reg3)) { *reg3 = tci->cmdTlTestSvc.reg3; }
        if(!is_null(reg4)) { *reg4 = tci->cmdTlTestSvc.reg4; }
    }

    return tci->responseHeader.returnCode;
}

uint32_t TrustletApiForwarder::ipc(
        uint32_t *partner,
        uint32_t *mr0,
        uint32_t *mr1,
        uint32_t *mr2,
        uint32_t ctrl
) {
    uint32_t reg4 = ctrl;
    mcResult_t res = MC_DRV_OK;
    int32_t lastErr = 0;
    uint32_t ret = 0;

    ret = svc(SYSCALLNO_IPC, partner, mr0, mr1, mr2, &reg4, &res, &lastErr);
    CHECK_DRVAPI_RETURNS_OK( res);
    CHECK_EQUAL(0, lastErr);

    // Stupid register moving
    if (!is_null(partner) && !is_null(mr0)) { *partner = *mr0; }
    if (!is_null(mr0) && !is_null(mr1)) { *mr0 = *mr1; }
    if (!is_null(mr1) && !is_null(mr2)) { *mr1 = *mr2; }
    if (!is_null(mr2)) { *mr2 = reg4; }

    return ret; //== *partner
}

void* TrustletApiForwarder::allocOnSharedMemD2(size_t size) {
    if(freePointer + size > (uint32_t)mapInfo.sVirtualLen) {
        FAIL("Requested too much memory");
        return 0;
    }
    uint32_t ret = (uint32_t)freePointer + (uint32_t)bulkBuf;//mapInfo.sVirtualAddr;
    uint32_t align = ret % sizeof(uint32_t);
    if(0 != align) {
        ret     += 4-align;
        size    += 4-align;
    }
    //bzero((void*)ret, size);
    freePointer += size;
    //printf("\n\t allocated address: %x\n",ret);
    return (void*)ret;
}

void* TrustletApiForwarder::allocOnSharedMemD3(size_t size) {
    if(freePointer2 + size > (uint32_t)mapInfo2.sVirtualLen) {
        FAIL("Requested too much memory");
        return 0;
    }
    uint32_t ret = (uint32_t)freePointer2 + (uint32_t)bulkBuf2;//mapInfo.sVirtualAddr;
    //bzero((void*)ret, size);
    freePointer2 += size;
    //printf("\n\t allocated address: %x\n",ret);
    return (void*)ret;
}

uint8_t *TrustletApiForwarder::getBuf() {
    return bulkBuf;
}

void TrustletApiForwarder::freeAll() {
    bzero(bulkBuf,freePointer);
    freePointer = 0;
    allocOnSharedMemD2(sizeof(marshalingParam_t));

    bzero(bulkBuf2,freePointer2);
    freePointer2 = 0;
}

_TLAPI_EXTERN_C tlApiResult_t callCryptoDriver(
    const marshalingParam_ptr pMarParam
) {
	//printf("\n\tcalling function %i\n",pMarParam->functionId);
	//printf("\t\tbase address: %x\n",globalForwarder->getBuf());
	memcpy((void*)globalForwarder->getBuf(),(void*)pMarParam,sizeof(*pMarParam));
	tlApiResult_t ret = globalForwarder->executeCrypto();
	if(TLAPI_OK == ret) {
		ret = ((marshalingParam_ptr)globalForwarder->getBuf())->payload.retVal;
	}
	return ret;
}

_TLAPI_EXTERN_C uint32_t TrustletApiForwarder_ipc(
        uint32_t *partner,
        uint32_t *mr0,
        uint32_t *mr1,
        uint32_t *mr2,
        uint32_t ctrl
) {
    return globalForwarder->ipc(partner, mr0, mr1, mr2, ctrl);
}
/** @} */
