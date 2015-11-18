/**
 * @addtogroup TEST_TLAPI
 *
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#include "mcErrorStrings.h"

/**
 * NOTE: this needs to be updated whenever new error codes are defined
 *       in MobiCoreDriverApi.h
 */
const char* mcDrvApiCodeToString(mcResult_t result) {

    static const char* strings[] = {
        "MC_DRV_OK",
        "MC_DRV_NO_NOTIFICATION",
        "MC_DRV_ERR_NOTIFICATION",
        "MC_DRV_ERR_NOT_IMPLEMENTED",
        "MC_DRV_ERR_OUT_OF_RESOURCES",
        "MC_DRV_ERR_INIT",
        "MC_DRV_ERR_UNKNOWN",
        "MC_DRV_ERR_UNKNOWN_DEVICE",
        "MC_DRV_ERR_UNKNOWN_SESSION",
        "MC_DRV_ERR_INVALID_OPERATION",
        "MC_DRV_ERR_INVALID_RESPONSE",
        "MC_DRV_ERR_TIMEOUT",
        "MC_DRV_ERR_NO_FREE_MEMORY",
        "MC_DRV_ERR_FREE_MEMORY_FAILED",
        "MC_DRV_ERR_SESSION_PENDING",
        "MC_DRV_ERR_DAEMON_UNREACHABLE",
		"MC_DRV_ERR_INVALID_DEVICE_FILE",
		"MC_DRV_ERR_INVALID_PARAMETER",
		"MC_DRV_ERR_KERNEL_MODULE",
		"MC_DRV_ERR_BULK_MAPPING",
		"MC_DRV_ERR_BULK_UNMAPPING",
		"MC_DRV_INFO_NOTIFICATION",
		"MC_DRV_ERR_NQ_FAILED",
		"MC_DRV_ERR_DAEMON_VERSION",
		"MC_DRV_ERR_CONTAINER_VERSION",
		"MC_DRV_ERR_WRONG_PUBLIC_KEY",			/** 0x00000019 < System Trustlet public key is wrong. */
		"MC_DRV_ERR_CONTAINER_TYPE_MISMATCH", 	/** 0x0000001a < Wrong containter type(s). */
		"MC_DRV_ERR_CONTAINER_LOCKED",        	/** 0x0000001b < Container is locked (or not activated). */
		"MC_DRV_ERR_SP_NO_CHILD",				/** 0x0000001c < SPID is not registered with root container. */
		"MC_DRV_ERR_TL_NO_CHILD",				/** 0x0000001d < UUID is not registered with sp container. */
		"MC_DRV_ERR_UNWRAP_ROOT_FAILED",		/** 0x0000001e /**< Unwrapping of root container failed. */
		"MC_DRV_ERR_UNWRAP_SP_FAILED",          /**0x0000001f /**< Unwrapping of service provider container failed. */
		"MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED",
		"MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN"
    };
    if((result & 0x0000FFFF) > MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN)
        return "!!! ERROR CODE UNKNOWN !!!";
    return strings[result & 0x0000FFFF];
}
