/**
 * <!-- Copyright Giesecke & Devrient GmbH 2009-2012 -->
 */
#ifndef __TRUSTLET_FORWARDER_H__
#define __TRUSTLET_FORWARDER_H__

#include "MobiCoreDriverApi.h"
#include "tlTestApi.h"
#include "mcErrorStrings.h"
#include "tlApiErrorStrings.h"

#define _TLAPI_EXTERN_C extern "C"
#include "TlApiMarshal.h"

#define TRUSTLET_SHARED_MEM_D1  (void*)0x100000
#define TRUSTLET_SHARED_MEM_D2  (void*)0x200000
#define TRUSTLET_SHARED_MEM_D3  (void*)0x300000
#define TRUSTLET_SHARED_MEM_D4  (void*)0x400000
#define TRUSTLET_SHARED_MEM_INVALID TRUSTLET_SHARED_MEM_D4
#define POINTER_VALID(x) \
        (NULL                        != x && \
         TRUSTLET_SHARED_MEM_INVALID != x)

#define TRUSTLET_SHARED_MEM_D2_SIZE 0x00F000        // 124K
#define TRUSTLET_SHARED_MEM_D3_SIZE 4096

class TrustletApiForwarder
{
private:
    mcResult_t x;
    tciMessage_t *tci;
    mcSessionHandle_t sessionHandle;
    uint8_t *bulkBuf;
    uint8_t *bulkBuf2;
    mcBulkMap_t mapInfo;
    mcBulkMap_t mapInfo2;
    uint32_t freePointer;
    uint32_t freePointer2;
    enum {
        state_uninitialized,
        state_setup,
        state_trustlet_opened,
        state_D3_mapped
    } state;

    TrustletApiForwarder(void);
    ~TrustletApiForwarder(void);

    /** Opens the Mobicore device and allocates memory in TLC at addresses 2MB and 3MB.
     * This corresponds to Trustlet virtual address space D2 and D3.
     * Then starts Trustlet session.
     * Set global variable to this instance.
     */
    void setup(void);

public:

    /** Get the instance of the Trustlet forwarder.
     * If necessary, creates one Instance.
     */
    static TrustletApiForwarder *getInstance(void);

    /** Ensure that no MobiCore resources are bound by TrustletApiForwarder
     */
    static void ensureNotUsed(void);

    /** Starts a Trustlet session and establishes shared memory at D2.
     * Then calls helper for shared memory at D3.
     * Then initializes the allocators.
     */
    void openTrustletSession(void);

    /** Maps shared memory at D3.
     * This helper function is used in between Tests to reestablish mappings.
     */
    void mapD3ForKey(void);

    /** Unmaps shared memory at D3.
     * This helper function is used to remove a mapping in between function calls.
     * If function 1 verifies the mapping and function 2 uses it without verification,
     * Function 2 will trigger a page fault.
     */
    void unmapD3ForKey(void);

    /** Close a Trustlet session and unmap existing mappings beforehand.
     * This helper function is used to close a Trustlet session inside a Test.
     * This is used to test if crypto sessions are correctly closed when a Trustlet exits.
     */
    void teardownTrustletSession (void);

    /** Close the Trustlet session and free all allocated resources.
     */
    void teardown (void);

    /** Send a message to Trustlet, instructing it to use Crypto.
     * Wait for response and unmarshal result.
     */
    tlApiResult_t executeCrypto(void);

    /** Send a message to Trustlet, instructing it to exit.
     * Wait for Notification from MC Session management and verify exit code.
     * This helper function is used to close a Trustlet session inside a Test.
     * This is used to test if crypto sessions are correctly closed when a Trustlet exits.
     */
    void executeExitTrustlet(void);

    /** Send a message to Trustlet, instructing it to crash.
     * Wait for Notification from MC Session management and verify exit reason.
     * This helper function is used to close a Trustlet session inside a Test.
     * This is used to test if crypto sessions are correctly closed when a Trustlet crashes.
     */
    void executeCrashTrustlet(void);

    /** Send a message to Trustlet, instructing it to call svc with given parameters.
     * Wait for Notification from Mobicore. Usually, Trustlet tells us that svc was not successful.
     * However, also, Trustlet could terminate or maybe crash.
     */
    uint32_t svc(
            uint32_t syscallno,
            uint32_t *reg0,
            uint32_t *reg1,
            uint32_t *reg2,
            uint32_t *reg3,
            uint32_t *reg4,
            mcResult_t *fromMcWaitNotification,
            int32_t *sessionErrorCode
    );

    /** Send a message to Trustlet, instructing it to call ipc with given parameters.
     * Based on svc(), but does not expect any abnormal behavior.
     */
    uint32_t ipc(
            uint32_t *partner,
            uint32_t *mr0,
            uint32_t *mr1,
            uint32_t *mr2,
            uint32_t ctrl
    );
    /** Close a Trustlet session and then starts a Trustlet session.
     */
    void restartTrustletSession(void);

    /**
     * Reserve a portion of shared memory D2. The returned address can be used in Trustlet and TLC.
     * Reservations are stored using a freepointer.
     * */
    void* allocOnSharedMemD2(size_t size);

    /**
     * Reserve a portion of shared memory D3. The returned address can be used in Trustlet and TLC.
     * Reservations are stored using a freepointer.
     * */
    void* allocOnSharedMemD3(size_t size);

    /**
     * Return address of bulkBuf
     * */
    uint8_t *getBuf();

    /**
     * Reset reservations of shared memory.
     * */
    void freeAll();
};

#define NEW(X) (X*)myForwarder->allocOnSharedMemD2(sizeof(X))
#define MAL(X) (uint8_t*)myForwarder->allocOnSharedMemD2(X)

#define NEW2(X) (X*)myForwarder->allocOnSharedMemD3(sizeof(X))
#define MAL2(X) (uint8_t*)myForwarder->allocOnSharedMemD3(X)

void freeAll();

void restartTrustletSession();
void exitTrustlet();
void crashTrustlet();

void unmapD3ForKey();
void mapD3ForKey();

extern uint32_t data_symKey[4];
extern uint32_t data_plainText[4];
extern uint32_t data_cipherText[4];

enum allocProblem {
	TEST_INVALID_POINTER_NULL,
	TEST_INVALID_POINTER_UNMAPPED
};

/**
 * New Two part error codes have been introduced
 * We have to  mask  and compare those parts with
 * Two parts.
 */
#define CHECK_DRVAPI_RETURNS_TWO_PART(error, source, calling) \
    CHECK_EQUAL_CODE(error, (calling >> 16), mcDrvApiCodeToString(calling >> 16)); \
    CHECK_EQUAL_CODE(source, (calling & 0x0000FFFF), mcDrvApiCodeToString(calling & 0x0000FFFF))


/**
 * Check return code of driver.
 * NOTE: requires the caller to have mcResult_t x to be defined.
 */
#define CHECK_DRVAPI_RETURNS(expected,calling) \
    CHECK_EQUAL_CODE(expected,x=calling, mcDrvApiCodeToString(x))

#define CHECK_DRVAPI_RETURNS_OK(calling) \
    CHECK_DRVAPI_RETURNS(MC_DRV_OK,calling)

/**
 * Check return code of driver.
 * NOTE: requires the caller to have mcResult_t res to be defined.
 */
#define CHECK_TLAPI_RETURNS(expected,calling) \
    CHECK_EQUAL_CODE(expected,res=calling,tlApiCodeToString(res))

#define CHECK_TLAPI_OK(calling) \
    CHECK_TLAPI_RETURNS(TLAPI_OK,calling)

#define FORWARDER_INIT 	TrustletApiForwarder::ensureNotUsed();

/** Find the maximum value. Never pass function calls here. */
#if !defined(MAX)
    #define MAX(x,y)    (((x)<(y))?(y):(x))
#endif

#endif // __TRUSTLET_FORWARDER_H__

/** */
