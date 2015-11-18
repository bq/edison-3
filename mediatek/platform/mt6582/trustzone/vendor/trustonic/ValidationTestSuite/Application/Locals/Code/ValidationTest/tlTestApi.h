/*
 *
 */

#ifndef TLSAMPLEAPI_H_
#define TLSAMPLEAPI_H_

#include "tci.h"


/**
 * Codes for tlApiExit().
 *
 */
#define TL_TEST_EXIT_TCI    1


/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */
#define CMD_TEST_NOTIFY                     1  /**< Test command to check communication between TLC and TL with RD/WR test of the TCI buffer  */
#define CMD_TEST_FILL_MEM                   2  /**< Fill memory region */
#define CMD_TEST_COMPARE_MEM                3  /**< Compare memory region */
#define CMD_TEST_UNIMPLEMENTED              4  /**< Use unimplemented function */
#define CMD_TEST_CRYPTO                     5  /**< Use cryptographic wrapper */
#define CMD_TEST_MEMCOPY                    6  /**< Copy memory */
#define CMD_TEST_CRYPTO_CRASH               7  /**< Crash cryptographic driver */

#define CMD_SET_ANY_DRIVER_ID               8  /**< Set anydriver ID // Default = DrTestIntegration = 1024 */
#define CMD_TEST_ANY_DRIVER                 9  /**< Use anydriver wrapper */

#define CMD_TEST_SVC                       10  /**< Call MTK, aka SVC instruction */

#define CMD_TEST_ANY_DRIVER_MULTIBLE       11  /**< Use anydriver wrapper */
#define CMD_TEST_PING                      12  /**< Send acknowledge only*/
#define CMD_TEST_INITDATA                  13  /**< Test exteded data+code*/

#define CMD_GET_RNG_SEED_BY_DRIVER         14  /**< Get RNG seed by driver */
#define CMD_GET_RNG_SEED_ON_DRIVER         8   /**< Get RNG command id on driver*/

#define CMD_TEST_ROT13                     15  /**< Test ROT13 */
#define CMD_TEST_SHA256                    16  /**< Test SHA256 */
#define CMD_TEST_AES_ENCRYPT               17  /**< Test AES */
#define CMD_TEST_AES_DECRYPT               18  /**< Test AES */
#define CMD_TEST_AES_WRAP                  19  /**< Test AES */
#define CMD_TEST_AES_UNWRAP                20  /**< Test AES */

#define CMD_TEST_ALLOC_LOG_SHMEM           21 /**< Use shared mem log */

#define CMD_TEST_SHA256_INIT               22  /**< Test SHA256 */
#define CMD_TEST_SHA256_FINAL              23  /**< Test SHA256 */

#define CMD_TEST_WAKEUP_SYSTEM             24 /**< Wake the system up via driver */
#define CMD_TEST_WAKEUP_SYSTEM_ON_DRV      13 /**< Wakeup call id on driver */

#define CMD_REG_CHCK_AFTER_SLEEP           25 /**< TL restore check after sleep > */
#define CMD_REG_STATE_AFTER_SLEEP          26 /**< TL restore check after sleep > */

#define CMD_TEST_SEC_SYSCALL               27 /**< TL execute syscall cmd notify> */

#define CMD_TEST_TL_MAP_PHY_ADDR           28 /**< TL map this physical  address> */

#define CMD_TEST_CHECK_MEM_CLEAN           29 /**< TL Scan if the available address space is clean> */
#define CMD_TEST_PING_DRV                  30  /**< Ping Driver via Trustlet*/
#define CMD_TEST_ENDORSEMENT               31  /**< Create device endorsement */
/**
 * Command ID's for Trustlet startup.
 */
#define STARTUP_CMD_TEST_NONE               0x000  /**< Do nothing. */
#define STARTUP_CMD_TEST_SIGNAL_MSH         0x300  /**< Set a MTK signal in Msh. */

/** @defgroup CMD Test Commands
 * @{ */

/** @defgroup NOTIFY
 * Send multiple notifications.
 * Trustlet will send number of notifications specified in num.
 * If exit code is set to a value !=0 the TlTest will call tlapiExt(exitCode)
 * directly after issuing the desired amount of notifications.
 * If crash is set != 0 the Trustlet will crash.
 *
 * Order of execution: <BR>
 * 1. Notify num times <BR>
 * 2. call tlApiExit(exitCode) <BR>
 * 3. crash the Trustlet (if not 2.)
 *
 * @{ */

/**  Notify Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t num;                   /**< Number of notifications to send */
    uint32_t exitCode;              /**< exit code for tlapiExit */
    uint32_t crash;                 /**< crash Trustlet if != 0 */
    uint32_t loop;                  /**< Trustlet goes into an endless loop if != 0 */
} cmdTlTestNotify_t, *cmdTlTestNotify_ptr;

/** Notify Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
} rspTlTestNotify_t, *rspTlTestNotify_ptr;
/** @} */// End NOTIFY


/** Ping Driver Command */
typedef struct {
    tciCommandHeader_t cmdHeader;  /**< Command header. */
    uint32_t driverId;
} cmdTlPingDriver_t, *cmdTlPingDriver_ptr;

/** Ping Driver Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
} rspTlPingDriver_t, *rspTlPingDriver_ptr;

/** @defgroup FILLMEM
 * Fill memory region.
 * Trustlet fills the specified memory region with the given value.
 *
 * @{ */

/**  Fill Memory Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t dest;                  /**< Start address */
    uint32_t len;                   /**< Length of the memory region */
    uint32_t  value;                 /**< LSB represent the (byte( value used for filling */
} cmdTlTestFillMem_t, *cmdTlTestFillMem_ptr;

/** Fill Memory Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
} rspTlTestFillMem_t, *rspTlTestFillMem_ptr;
/** @} */// End FILLMEM

/** @defgroup COMPAREMEM
 * Fill memory region.
 * Trustlet fills the specified memory region with the given value.
 *
 * @{ */

/**  Compare Memory Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t dest;                  /**< Start address */
    uint32_t len;                   /**< Length of the memory region */
    uint32_t  value;                 /**< LSB represent the (byte( value used for filling */
} cmdTlTestCompareMem_t, *cmdTlTestCompareMem_ptr;

/** Compare Memory Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ofsMismatch;          /**< Offset of mismatch */
} rspTlTestCompareMem_t, *rspTlTestCompareMem_ptr;
/** @} */// End COMPAREMEM

/** Get RNG Seed By Driver Command */
typedef struct {
    tciCommandHeader_t cmdHeader;  /**< Command header. */
    uint32_t seedBufferAddr;
} cmdTlGetRngSeed_t, *cmdTlGetRngSeed_ptr;

/** Get RNG Seed By Driver Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
} rspTlGetRngSeed_t, *rspTlGetRngSeed_ptr;


/** @defgroup UNIMPLEMENTED
 *
 *
 * @{ */
/**  Unimplemented Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t apiCode;               /**< Function code to call */
} cmdTlTestUnimplemented_t;
/** @} */// End UNIMPLEMENTED

/** @defgroup CRYPTO
 *
 *
 * @{ */
/**  Crypto Command */
typedef struct {
    union {
        // For more convenient reading --------
        tciCommandHeader_t      cmdHeader;    /**< Command header. */
        tciResponseHeader_t     responseHeader;
    };
    uint32_t sVirtualAddr;          /**< The virtual address of the Bulk buffer regarding the address space of the Trustlet, already includes a possible offset! */
} cmdTlTestCrypto_t;
/** @} */// End CRYPTO

/** @defgroup MEMCOPY
 * Trustlet executes memcpy.
 *
 * @{ */

/**  Memcopy Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t dest;                  /**< Destination address */
    uint32_t src;                   /**< Source address */
    uint32_t n;                     /**< Length of the memory region  */
} cmdTlTestMemcopy_t, *cmdTlTestMemcopy_ptr;

/** @} */// End MEMCOPY

/** @defgroup SET_ANY_DRIVER_ID
 * Trustlet set ID any driver.
 *
 * @{ */

/**  Set anydriver ID */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t driver_Id;             /**< Driver ID */
} cmdTlSetAnyDriverId_t, *cmdTlSetAnyDriverId_ptr;


/** @} */// End SET_ANY_DRIVER_ID


/** @defgroup LOAD_ANY_DRIVER
 * Load dynamical driver.
 * Trustlet loads dynamical driver due last Set-anydriver-ID-Command.
 *
 * @{ */



/** @defgroup TEST_ANY_DRIVER
 *
 *
 * @{ */
/**  TEST_ANY_DRIVER Command */
typedef struct {
    union {
        // For more convenient reading --------
        tciCommandHeader_t      cmdHeader;    /**< Command header. */
        tciResponseHeader_t     responseHeader;
    };
    uint32_t sVirtualAddr;          /**< The virtual address of the Bulk buffer regarding the address space of the Trustlet, already includes a possible offset! */
    uint32_t p0;                    /**< additional parameter */
} cmdTlTestAnyDriver_t;
/** @} */// End TEST_ANY_DRIVER

/**
 * ROT13 command message.
 * The ROT13 command will apply disguise data provided in the payload
 * buffer of the TCI. Only characters (a-z, A-Z) will be processed. Other
 * data is ignored and stays untouched.
 *
 * @param len Lenght of the data to process.
 * @param data Data to processed (clear text).
 */
typedef struct {
    union {
       tciCommandHeader_t cmdHeader;
       tciResponseHeader_t rspHeader;
    };
    uint32_t            len;                         /**< Length of data to process */
    uint8_t*            bulkBuffer;                  /**< data: The virtual address of the Bulk buffer regarding the address space of the Trustlet, already includes a possible offset! */
} cmdTlTestRot13_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    union {
       tciCommandHeader_t cmdHeader;
       tciResponseHeader_t rspHeader;
    };
} rspTlTestRot13_t;

/**
 * SHA256 command message.
 */
typedef struct {
    union {
       tciCommandHeader_t cmdHeader;
       tciResponseHeader_t rspHeader;
    };
    uint32_t sVirtualAddr;      /**< The virtual address of the bulk buffer regarding the address space of the Trustlet. */
    uint32_t srcOffs;           /**< The offset of the source data within the bulk buffer. */
    uint32_t srcLen;            /**< Length of the source data. */
} cmdTlTestSha256_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    union {
       tciCommandHeader_t cmdHeader;
       tciResponseHeader_t rspHeader;
    };
    uint32_t hashLen;           /**< The length of the ouput hash (should be 32 byte). */
    uint32_t hash[32];          /**< The 256 bit hash. */
} rspTlTestSha256_t;

/**
 * AES command message.
 */
typedef struct {
    union {
       tciCommandHeader_t cmdHeader;
       tciResponseHeader_t rspHeader;
    };
    uint32_t    len;
    uint32_t    plainData;
    uint32_t    cipherData;
    uint32_t    counter;
} cmdTlTestAes_t, rspTlTestAes_t;;

/**  CMD_TEST_ALLOC_LOG_SHMEM Command */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t ID;                    /**< Test-ID */
    uint32_t logmem;                /**< log mem adresse */
} cmdTlAllocLogShmem_t;

/**  CMD_TEST_ALLOC_LOG_SHMEM Response */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
    uint32_t ID;                    /**< Test-ID */
} rspTlAllocLogShmem_t;

/**  CMD_TEST_SHA256_INIT */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
} cmdTlTestSha256init_t;
typedef struct {
    tciResponseHeader_t rspHeader;   /**< Command header. */
    uint32_t index;
} rspTlTestSha256init_t;

/**  CMD_TEST_SHA256_FINAL */
typedef struct {
    tciCommandHeader_t cmdHeader;
    uint32_t sVirtualAddr;      /**< The virtual address of the bulk buffer regarding the address space of the Trustlet. */
    uint32_t srcOffs;           /**< The offset of the source data within the bulk buffer. */
    uint32_t srcLen;            /**< Length of the source data. */
    uint32_t index;             /**< Length of the source data. */
} cmdTlTestSha256Final_t;
typedef struct {
    tciResponseHeader_t rspHeader;
    uint32_t hashLen;          /**< The length of the ouput hash (should be 32 byte). */
    uint32_t hash[32];         /**< The 256 bit hash. */
} rspTlTestSha256Final_t;

/** CMD_TEST_WAKEUP_SYSTEM */
typedef struct {
    tciCommandHeader_t cmdHeader;
    uint32_t seconds;
} cmdTlWakeUpSystem_t;

/** CMD_TEST_WAKEUP_SYSTEM */
typedef struct {
    tciResponseHeader_t rspHeader;
} rspTlWakeUpSystem_t;


/** CMD_REG_CHCK_AFTER_SLEEP */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
} cmdTlRegChckSleep_t;

/** CMD_REG_CHCK_AFTER_SLEEP */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
} rspTlRegChckSleep_t;

/** CMD_REG_STATE_AFTER_SLEEP */
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
} cmdTlRegStateSleep_t;

/** CMD_REG_STATE_AFTER_SLEEP */
typedef struct {
   tciResponseHeader_t rspHeader;  /**< Response header. */
} rspTlRegStateSleep_t;

/** CMD_TRY_MAP_PHY*/
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
    uint32_t phyAddress;
} cmdTlMapPhyAddress_t;

/** CMD_TRY_MAP_PHY */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
} rspTlMapPhyAddress_t;

/** CMD_TEST_CHECK_MEM_CLEAN*/
typedef struct {
    tciCommandHeader_t cmdHeader;   /**< Command header. */
} cmdCheckMemClean_t;

/** CMD_TRY_MAP_PHY */
typedef struct {
    tciResponseHeader_t rspHeader;  /**< Response header. */
} rspCheckMemClean_t;


/** @defgroup TEST_SVC
 *
 *
 * @{ */
/**  SVC Command */
//int svc(
//    uint32_t syscallno,
//    uint32_t *reg0,
//    uint32_t *reg1,
//    uint32_t *reg2,
//    uint32_t *reg3,
//    uint32_t *reg4
//);
typedef struct {
    union {
        // For more convenient reading --------
        tciCommandHeader_t      cmdHeader;    /**< Command header. */
        tciResponseHeader_t     responseHeader;
    };
    uint32_t syscallno; /**< The MTK system call number to be executed. */
    uint32_t reg0;      /**< Parameter / register 0 of system call. */
    uint32_t reg1;      /**< Parameter / register 1 of system call. */
    uint32_t reg2;      /**< Parameter / register 2 of system call. */
    uint32_t reg3;      /**< Parameter / register 3 of system call. */
    uint32_t reg4;      /**< Parameter / register 4 of system call. */
} cmdTlTestSvc_t;

#define SYSCALLNO_TASK_DONATE            0
#define SYSCALLNO_TASK_ACTIVATE          1
#define SYSCALLNO_IPCRIGHT_ATTACH        2
#define SYSCALLNO_IPCRIGHT_DETACH        3
#define SYSCALLNO_TASK_ATTR              4
#define SYSCALLNO_TASK_START             5
#define SYSCALLNO_TASK_TERMINATE         6
#define SYSCALLNO_MAP_PHYS               7
#define SYSCALLNO_UNMAP                  8
#define SYSCALLNO_THREAD_START           9
#define SYSCALLNO_THREAD_STOP           10
#define SYSCALLNO_TIME_DONATE           11
#define SYSCALLNO_THREAD_EX_REGS        12
#define SYSCALLNO_MAP                   13
#define SYSCALLNO_THREAD_SUSPEND        14
#define SYSCALLNO_THREAD_GET_ATTR       15
#define SYSCALLNO_THREAD_SET_ATTR       16
#define SYSCALLNO_IPC                   17
#define SYSCALLNO_INTR_ATTACH           18
#define SYSCALLNO_INTR_DETACH           19
#define SYSCALLNO_SIGNAL_WAIT           20
#define SYSCALLNO_SIGNAL_SET            21
#define SYSCALLNO_SIGNAL_CLEAR          22
#define SYSCALLNO_CONTROL               23
/** @} */// End TEST_SVC

/**
 * Create endorsement command and parameters
 *
 * @param ret     return code
 * @param scope   the scope of the endorsement to create
 * @param inlen   length of data to create endorsement from
 * @param outlen  length of created endorsement (if successful)
 * @param in      data to add to endorsement
 * @param out     the endorsement
 * @param ho      the hash offset within the returned blob
 */
typedef struct {
    tciCommandHeader_t cmdHeader;
    uint32_t           ret;
    uint32_t           scope;
    uint32_t           inlen;
    uint32_t           outlen;
    uint8_t            *in;
    uint8_t            *out;
    uint32_t           ho;
    /* out is organized as follows:
     * ho is the hash offset within the decrypted endorsement
     * <decrypted endorsement> - 140 bytes
     * <Encrypted endorsement + header> - 260 bytes
     */
} cmdCreateEndorsement_t;

/**
 * Response structure TA -> CA
 *
 * @param att the actual endorsement on success
 * @param len Length of endorsement
 */
typedef struct {
} rspCreateEndorsement_t;

/**
 * TCI message data.
 */
typedef union {
    // For more convenient reading --------
    tciCommandHeader_t       commandHeader;
    tciResponseHeader_t      responseHeader;

    // Dedicated messages --------
    cmdTlTestNotify_t        cmdTlTestNotify;
    rspTlTestNotify_t        rspTlTestNotify;
    cmdTlPingDriver_t        cmdTlPingDriver;
    rspTlPingDriver_t        rspTlPingDriver;
    cmdTlTestFillMem_t       cmdTlTestFillMem;
    rspTlTestFillMem_t       rspTlTestFillMem;
    cmdTlTestCompareMem_t    cmdTlTestCompareMem;
    rspTlTestCompareMem_t    rspTlTestCompareMem;
    cmdTlTestUnimplemented_t cmdTlTestUnimplemented;
    cmdTlTestCrypto_t        cmdTlTestCrypto;
    cmdTlTestMemcopy_t       cmdTlTestMemcopy;
    cmdTlSetAnyDriverId_t    cmdTlSetAnyDriverId;
    cmdTlTestAnyDriver_t     cmdTlTestAnyDriver;
    cmdTlTestSvc_t           cmdTlTestSvc;
    cmdTlGetRngSeed_t        cmdTlGetRngSeed;
    rspTlGetRngSeed_t        rspTlGetRngSeed;
    cmdTlTestRot13_t         cmdTlTestRot13;
    rspTlTestRot13_t         rspTlTestRot13;
    cmdTlTestSha256_t        cmdTlTestSha256;
    rspTlTestSha256_t        rspTlTestSha256;
    cmdTlTestAes_t           cmdTlTestAes;
    rspTlTestAes_t           rspTlTestAes;
    cmdTlAllocLogShmem_t     cmdTlAllocLogShmem;
    rspTlAllocLogShmem_t     rspTlAllocLogShmem;
    cmdTlTestSha256init_t    cmdTlTestSha256init;
    rspTlTestSha256init_t    rspTlTestSha256init;
    cmdTlTestSha256Final_t   cmdTlTestSha256Final;
    rspTlTestSha256Final_t   rspTlTestSha256Final;
    cmdTlWakeUpSystem_t      cmdTlWakeUpSystem;
    rspTlWakeUpSystem_t      rspTlWakeUpSystem;
    cmdTlRegChckSleep_t      cmdTlRegChckSleep;
    rspTlRegChckSleep_t      rspTlRegChckSleep;
    cmdTlRegStateSleep_t     cmdTlRegStateSleep;
    rspTlRegStateSleep_t     rspTlRegStateSleep;
    cmdTlMapPhyAddress_t     cmdTlMapPhyAddress;
    rspTlMapPhyAddress_t     rspTlMapPhyAddress;
    cmdCheckMemClean_t       cmdCheckMemClean;
    rspCheckMemClean_t       rspCheckMemClean;
    cmdCreateEndorsement_t   cmdCreateEndorsement;
} tciMessage_t;


/**
 * Trustlet UUID.
 */
#define TL_TEST_UUID { { 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


/** TeeKeymaster command header */
typedef struct {
    tciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process */
} tciCommandHeaderTeeKeyMaster_t;

/** TeeKeymaster response header */
typedef struct {
    tciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} tciResponseHeaderTeeKeyMaster_t;

/** TeeKeymaster command/response for generating a key */
typedef struct {
    uint32_t type;                  /**< Key pair type. RSA or RSACRT */
    uint32_t keysize;               /**< Key size in bits, e.g. 1024, 2048,.. */
    uint32_t exponent;              /**< Exponent number */
    uint32_t keydata;               /**< Key data buffer passed by TLC  */
    uint32_t keydatalen;            /**< Length of key data buffer */
    uint32_t solen;                 /**< Secure object length  (of key data) (provided by the trustlet)  */
} cmdRspTeeKeymasterRsaGenKey_t;

/** TeeKeymaster command/response for signing */
typedef struct {
    uint32_t keydata;               /**< Key data buffer */
    uint32_t keydatalen;            /**< Length of key data buffer */
    uint32_t plaindata;             /**< Plaintext data buffer */
    uint32_t plaindatalen;          /**< Length of plaintext data buffer */
    uint32_t signaturedata;         /**< Signature data buffer */
    uint32_t signaturedatalen;      /**< Length of signature data buffer */
    uint32_t algorithm;             /**< Signing algorithm */
} cmdRspTeeKeymasterRsaSign_t;

/** TeeKeymaster command/response for verifying a signature */
typedef struct {
    uint32_t keydata;               /**< Key data buffer */
    uint32_t keydatalen;            /**< Length of key data buffer */
    uint32_t plaindata;             /**< Plaintext data buffer */
    uint32_t plaindatalen;          /**< Length of plaintext data buffer */
    uint32_t signaturedata;         /**< Signature data buffer */
    uint32_t signaturedatalen;      /**< Length of signature data buffer */
    uint32_t algorithm;             /**< Signing algorithm */
    bool     validity;              /**< Signature validity */
} cmdRspTeeKeymasterRsaVerify_t;

/** Trustlet TeeKeymaster message */
typedef struct {
    union {
        tciCommandHeaderTeeKeyMaster_t  command;
        tciResponseHeaderTeeKeyMaster_t response;
    };

    union {
        cmdRspTeeKeymasterRsaGenKey_t  rsagenkey;
        cmdRspTeeKeymasterRsaSign_t    rsasign;
        cmdRspTeeKeymasterRsaVerify_t  rsaverify;
    };
} tciMessageTeeKeymaster_t;

#endif // TLSAMPLEAPI_H_
