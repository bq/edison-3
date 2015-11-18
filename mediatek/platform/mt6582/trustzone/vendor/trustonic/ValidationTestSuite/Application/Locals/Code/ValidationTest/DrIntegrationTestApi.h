/*
 *
 */

#ifndef DRV_H_
#define DRV_H_


#include "tci.h"
#include "dci.h"

/**
 * Return codes of Driver commands.
 */
#define RET_OK              0            /**< Set, if processing is error free */
#define RET_ERR_UNKNOWN_CMD 1            /**< Unknown command */
#define RET_CUSTOM_START    2

//** Returncodes of Driver
#define RET_ERR_NORES         RET_CUSTOM_START + 0
#define RET_ERR_NOSHRINKRES   RET_CUSTOM_START + 1
#define RET_ERR_INVALIDRES    RET_CUSTOM_START + 2
#define RET_ERR_MAP_FAILED    RET_CUSTOM_START + 3
#define RET_ERR_INVALID_TID   RET_CUSTOM_START + 4
#define RET_ERR_UNMAP_FAILED  RET_CUSTOM_START + 5
#define RET_ERR_WRITE_FAILED  RET_CUSTOM_START + 5
#define RET_ERR_READ_FAILED   RET_CUSTOM_START + 6
#define RET_ERR_UNKNOWN		  RET_CUSTOM_START + 7

//** IPCH Function codes of Driver
#define CMD_SET_NUM_RESOURCES           1
#define CMD_ALLOCATE_RESOURCE           2
#define CMD_FREE_RESOURCE               3
#define CMD_BUSY_LOOP                   4
#define CMD_ECHO                        5
#define CMD_PING                        6
#define CMD_CALIBRATE_BUSY_LOOP         7
#define CMD_GET_RNG_SEED_VADDR          8
#define CMD_CRASH_BY_ZERO_DIVIDE        9
#define CMD_ALLOC_LOG_SHMEM             10
#define CMD_CRASH_WHEN_CLOSE            11
#define CMD_CRASH_WHEN_TRUSTLET_CLOSE   12
#define CMD_CPU_WAKEUP                  13
#define CMD_TL_MAP_PHYS                 14

//
#define TRIM_CMD(x)         ((x << 1) >> 1) // Trims the message. First bit is set in driver commands

/* Command header have some arrangement here in two bytes 0S00 / 0000
 * Ś=1 indicates that a signal  wait pause take place in driver before execution
 * Ś=2 indicates that a signal  wait pause take place in driver after execution
 */
#define HALT_DRIVER_BEFORE_MASK     0x01000000
#define HALT_DRIVER_AFTER_MASK      0x02000000
#define HALT_DRIVER_COMMON_MASK     0x03000000 // For clearing the data.
#define HALT_DRIVER_BEFORE_CMD(x)   (x | HALT_DRIVER_BEFORE_MASK)
#define HALT_DRIVER_AFTER_CMD(x)    (x | HALT_DRIVER_AFTER_MASK)


// Driver DCIH Test commands
#define CMD_DR_PING                         0  // Command to see if DCI handling works.
#define CMD_DR_MAP_PHYS_CONT                1
#define CMD_DR_MAP_PHYS_FRAG                2
#define CMD_DR_SIGNAL_IPC                   3
#define CMD_DR_MAP_FOR_STORED_IPC_CLIENT    4
#define CMD_DR_THREAD_START_DUMMY           5
#define CMD_DR_TEST_1MB_EXT_MEM             6
//----------------Memory Alloc Test Cmd------

#define CMD_TESTMEMORY_CHECK_MALLOC_HINT			9
#define CMD_TESTMEMORY_MALLOC						10
#define CMD_TESTMEMORY_REALLOC						11
#define CMD_TESTMEMORY_MEMSET						12
#define CMD_TESTMEMORY_MEMCMP						13
#define CMD_TESTMEMORY_MEMMOVE						14
#define CMD_TESTMEMORY_CHECK_MALLOC_NULL_SIZE		15
#define CMD_TESTMEMORY_MALLOC_REALLOC_NULL_SIZE_1	16
#define CMD_TESTMEMORY_MALLOC_REALLOC_NULL_SIZE_2	17

/**  CMD_SET_NUM_RESOURCES Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t numRes;                /**< Count of resources */
} cmd_setNumResources_t;

/** CMD_SET_NUM_RESOURCES Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rsp_setNumResources_t;


/**  CMD_ALLOCATE_RESOURCE Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
} cmd_allocateResource_t;

/** CMD_ALLOCATE_RESOURCE Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t resId;                 /**< resource id */
    uint32_t availResCount;         /**< still available resources */
} rsp_allocateResource_t;


/**  CMD_FREE_RESOURCE Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t resId;                 /**< resource id */
} cmd_freeResource_t;

/** CMD_FREE_RESOURCE Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t availResCount;         /**< still available resources */
} rsp_freeResource_t;

/** CMD_BUSY_LOOP Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t timeout;               /**< timeout in ms */
} cmd_waitingLoop_t;


/** CMD_BUSY_LOOP Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rsp_waitingLoop_t;


/**  CMD_ECHO Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t src;                   /**< source adresse */
    uint32_t len;                   /**< length of bytes to copy */
    uint32_t dest;                  /**< destinateion adresse */
} cmd_echo_t;

/** CMD_ECHO Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rsp_echo_t;

/**  CMD_PING */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
} cmd_ping_t;

/** CMD_SET_NUM_RESOURCES Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rsp_ping_t;

/** CMD_CALIBRATE1000 Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t calibrate1000;         /**< calibration value  */
} cmd_calibrate_waitingLoop_t;


/** CMD_CALIBRATE Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rsp_calibrate_waitingLoop_t;

/**  CMD_ALLOCATE_RNG_SEED Command*/
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint8_t *bufferAddress;
} cmd_readRngSeedVaddr_t;

/** CMD_ALLOCATE_RNG_SEED Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t rngVirtualAddress;     /**< still available resources */
} rsp_readRngSeedVaddr_t;


/**  CMD_CRASH_BY_ZERO_DIVIDE Command*/
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
} cmd_crashByZeroDevide_t;

/** CMD_CRASH_BY_ZERO_DIVIDE Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rsp_crashByZeroDevide_t;


/**  CMD_ALLOC_LOG_SHMEM Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t logmem;                /**< log mem adresse */
    uint32_t bwritethrough;         /** */
} cmd_allocLogShmem_t;

/** CMD_ALLOC_LOG_SHMEM Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rsp_allocLogShmem_t;


/**  CMD_CRASH_WHEN_CLOSE */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t bcrashwhenclose;       /** */
} cmd_crashwhenclose_t;

/**  CMD_CRASH_WHEN_CLOSE */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Command header. */
} rsp_crashwhenclose_t;


/**  CMD_CPU_WAKEUP */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t seconds;               /** */
} cmd_cpuWakeup_t;

/**  CMD_CPU_WAKEUP */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Command header. */
} rsp_cpuWakeup_t;


/** CMD_TL_MAP_PHYS */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint64_t phyAddr;               /**< Physical Address */
    uint32_t tlVirtAddr;            /**< Allocated virtual address in tl */
} cmd_tlMapPhys_t;

typedef struct {
    tciResponseHeader_t rspHeader;   /**< Response header. */
} rsp_tlMapPhys_t;


/**
 * TCI message data.
 */
typedef union {
    // For more convenient reading --------
    tciCommandHeader_t          commandHeader;
    tciResponseHeader_t         responseHeader;

    // Dedicated messages --------
    cmd_setNumResources_t       cmd_setNumResources;
    rsp_setNumResources_t       rsp_setNumResources;
    cmd_allocateResource_t      cmd_allocateResource;
    rsp_allocateResource_t      rsp_allocateResource;
    cmd_freeResource_t          cmd_freeResource;
    rsp_freeResource_t          rsp_freeResource;
    cmd_waitingLoop_t           cmd_waitingLoop;
    rsp_waitingLoop_t           rsp_waitingLoop;
    cmd_echo_t                  cmd_echo;
    rsp_echo_t                  rsp_echo;
    cmd_ping_t                  cmd_ping;
    rsp_ping_t                  rsp_ping;
    cmd_calibrate_waitingLoop_t cmd_calibrate_waitingLoop;
    rsp_calibrate_waitingLoop_t rsp_calibrate_waitingLoop;
    cmd_readRngSeedVaddr_t      cmd_readRngSeed;
    rsp_readRngSeedVaddr_t      rsp_readRngSeed;
    cmd_crashByZeroDevide_t     cmd_crashByZeroDevide;
    rsp_crashByZeroDevide_t     rsp_crashByZeroDevide;
    cmd_allocLogShmem_t         cmd_allocLogShmem;
    rsp_allocLogShmem_t         rsp_allocLogShmem;
    cmd_crashwhenclose_t        cmd_crashwhenclose;
    rsp_crashwhenclose_t        rsp_crashwhenclose;
    cmd_cpuWakeup_t             cmd_cpuWakeup;
    rsp_cpuWakeup_t             rsp_cpuWakeup;
    cmd_tlMapPhys_t             cmd_tlMapPhys;
    rsp_tlMapPhys_t             rsp_tlMapPhys;
} tciMessageDrIntegrationTest_t;


/*
 * -------------- The data types are needed for tlc build --------
 * They are used in  DrTestIntr driver.
 * We just did not need to define another separate header.
 */
/** CMD_DR_TEST_INTR */
typedef struct {
    dciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t value;                 /**< Value checking if interrupt was triggered. */
} rsp_drTestIntr_t;

/** CMD_DR_MAP_PHYS_CONT */
typedef struct {
    dciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t phyAddr;               /**< Physical Address (Normally not necessary) */
    uint32_t numberOfPages;         /**< Number of pages (4k) to check continuity*/
} cmd_mapDrPhysCont_t;

typedef struct {
    dciResponseHeader_t rspHeader;   /**< Response header. */
} rsp_mapDrPhysCont_t;

/** CMD_DR_TEST_1MB_EXT_MEM */
typedef struct {
    dciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t virtAddr;         /**< Virtual address where 1MB should be mapped*/
} cmd_test1MbExtendedMemory_t;

/** CMD_DR_MAP_PHYS_FRAG */
typedef struct {
    dciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t phyAddr;               /**< Physical Address (Normally not necessary) */
    uint32_t indexPreMapPage;       /**< Which page should be mapped in advance */
    uint32_t numberOfPages;         /**< Number of pages (4k) to check continuity*/
} cmd_mapDrPhysFrag_t;

typedef struct {
    dciResponseHeader_t rspHeader;   /**< Response header. */
} rsp_mapDrPhysFrag_t;

/** CMD_DR_SIGNAL_IPC */
typedef struct {
    dciCommandHeader_t cmdHeader;   /**< Command header. */
} cmd_signalIpc_t;

typedef struct {
    dciResponseHeader_t rspHeader;   /**< Response header. */
} rsp_signalIpc_t;

/** CMD_DR_MAP_FOR_STORED_IPC_CLIENT */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t phyAddr;               /**< Physical Address */
} cmd_drMapForStoredIpcClient_t;

typedef struct {
    tciResponseHeader_t rspHeader;   /**< Response header. */
} rsp_drMapForStoredIpcClient_t;


/** CMD_DR_THREAD_START_DUMMY */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t dummyThreadNo;         /**< Dummy thread to start [0-5] */
} cmd_dcThreadStartDummy_t;

typedef struct {
    tciResponseHeader_t rspHeader;   /**< Response header. */
} rsp_dcThreadStartDummy_t;


/**
 * DCI message data.
 */
typedef union {
    // For more convenient reading --------
    dciCommandHeader_t            dciCommandHeader;   /**< Command header. */
    dciResponseHeader_t           dciResponseHeader;  /**< Response header. */

    // Dedicated messages ----------------
    rsp_drTestIntr_t                    rsp_drTestIntr;
    cmd_mapDrPhysCont_t                 cmd_mapDrPhysCont;
    rsp_mapDrPhysCont_t                 rsp_mapDrPhysCont;
    cmd_mapDrPhysFrag_t                 cmd_mapDrPhysFrag;
    rsp_mapDrPhysFrag_t                 rsp_mapDrPhysFrag;
    cmd_signalIpc_t                     cmd_signalIpc;
    rsp_signalIpc_t                     rsp_signalIpc;
    cmd_drMapForStoredIpcClient_t       cmd_drMapForStoredIpcClient;
    rsp_drMapForStoredIpcClient_t       rsp_drMapForStoredIpcClient;
    cmd_dcThreadStartDummy_t            cmd_threadStartDummy;
    rsp_dcThreadStartDummy_t            rsp_threadStartDummy;
    cmd_test1MbExtendedMemory_t         cmd_map1MbExtMem;
} dciMessageDrIntegrationTest_t;

/*
 * --------------------------------------------------------------
 */

/**
 * Driver UUID.
 */
#define DRV_DRINTEGRATIONSTEST_UUID { { 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

/**
 * Shmem-Logstruct.
 */
#define LogShmem_Dim   100
typedef struct {
    uint32_t ID;
    uint8_t  typ; // T=Trustlet, D=Driver
    uint32_t ipcMsg;
    uint32_t functionId;
} LogShmemEntry_t;

typedef struct {
    uint16_t index;
    LogShmemEntry_t entry[LogShmem_Dim];
} LogShmem_t;


#endif // DRV_H_
