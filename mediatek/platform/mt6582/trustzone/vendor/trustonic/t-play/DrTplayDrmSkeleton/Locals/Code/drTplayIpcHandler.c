/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include "drStd.h"
#include "DrApi/DrApiLogging.h"
#include "DrApi/DrApiMm.h"
#include "DrApi/DrApiThread.h"
#include "DrApi/DrApiIpcMsg.h"

#include "drTplayApiMarshal.h"
#include "drTplayCommon.h"
#include "drTplayUtils.h"


#include "tplay_marshal.h"
#include "TlApi/TPlay.h"


DECLARE_STACK(drIpchStack, 2048);


/* Global variables */
threadid_t gLastIpcClient = NILTHREAD;
uint32_t   gLastPageNum   = 0;


/**
 * Performs security checks to find out output address is
 * in the range of secure protected memory block
 */
static int32_t drIpchSecurityCheck(
        addr_t   physAddr,
        uint32_t length ) {
    /**
     * Update the function accordingly
     */
    return TLAPI_DRM_OK;
}


/**
 * IPC handler loop. this is the function where IPC messages are handled
 */
_NORETURN void drIpchLoop( void ) {

    /* Set IPC parameters for initial MSG_RD to IPCH */
    threadid_t              ipcClient = NILTHREAD;
    message_t               ipcMsg    = MSG_RD;
    uint32_t                ipcData   = 0;
    tlApiResult_t           tlRet     = TLAPI_OK;
    tplayMarshalingParam_ptr   pMarshal;
    taskid_t                taskid = drApiGetTaskid();
    tlDrmApiDrmContent_t    drmContent;
    tlDrmApiLink_t          link;
    uint32_t                physOutAddr;
    uint32_t                callerRootId;
    uint32_t                callerSpId;
    uint8_t                 i;

    /**
     * Check if there is a pending client. If there is, this is an
     * indication that IPC handler thread crashed before completing
     * the request. Respond with failure.
     */
    if (!threadid_is_null(gLastIpcClient)) {
        ipcClient = gLastIpcClient;
        ipcMsg    = MSG_RS;
        ipcData   = E_TLAPI_DRV_UNKNOWN;
    }

    dbgSN("[DRM Driver] drIpchLoop(): IPC handler thread started");
    for (;;) {

        /* Reset last IPC client */
        gLastIpcClient = NILTHREAD;

        /*
         * When called first time sends ready message to IPC server and
         * then waits for IPC requests
         */
        if (E_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData)) {
            dbgSN("[DRM Driver] drIpchLoop(): drApiIpcCallToIPCH failed");
            continue;
        }

        /* Update last IPC client */
        gLastIpcClient = ipcClient;

        /* Dispatch request */
        switch (ipcMsg) {
            case MSG_CLOSE_TRUSTLET:
                /**
                 * Trusted Application close message
                 */
                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_CLOSE_DRIVER:
                /**
                 * Driver close message
                 */
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = TLAPI_OK;
                break;
            case MSG_GET_DRIVER_VERSION:
                /**
                 * Driver version message
                 */
                ipcMsg = (message_t) TLAPI_OK;
                ipcData = DRIVER_VERSION  ;
                break;
            case MSG_RQ:

                /* init tlRet value */
                tlRet = E_TLAPI_DRM_INTERNAL;

                /* Before proceeding further, check if the caller is system Trusted Application */
                if (E_OK != drApiGetClientRootAndSpId(
                        &callerRootId,
                        &callerSpId,
                        ipcClient)) {
                    dbgSN("[DRM Driver] drIpchLoop(): drApiGetClientRootAndSpId! failed");
                    continue;
                }
                /**
                 * Handle incoming IPC requests via TL API.
                 * Map the caller Trusted Application to access to the marshalling data
                 */
                pMarshal = (tplayMarshalingParam_ptr)drApiMapClientAndParams(
                        ipcClient,
                        ipcData);

                if (pMarshal) {

                    /* Before proceeding further, check if the caller is system Trusted Application*/
                    if (E_OK != drApiGetClientRootAndSpId(
                            &callerRootId,
                            &callerSpId,
                            ipcClient)) {
                        dbgSN("[DRM Driver] drIpchLoop(): drApiGetClientRootAndSpId failed!");
                        pMarshal->payload.retVal = E_TLAPI_DRM_INTERNAL;
                        break;
                    }

                    if ((callerRootId != ROOTID_SYSTEM) && (callerSpId != SPID_SYSTEM)) {
                        dbgSN("[DRM Driver] drIpchLoop(): Called not a system Trusted Application!");
                        pMarshal->payload.retVal = E_TLAPI_DRM_PERMISSION_DENIED;
                        break;
                    }

                    /* Process the request */
                    switch (pMarshal->functionId) {

                        case FID_DR_PROCESS_DRM_CONTENT:
                            /**
                             * Handle secure cipher operation
                             */

                            drmContent.sHandle                     = pMarshal->payload.drmContent.sHandle;
                            drmContent.decryptCtx.key              = drApiAddrTranslateAndCheck(pMarshal->payload.drmContent.decryptCtx.key);
                            drmContent.decryptCtx.keylen           = pMarshal->payload.drmContent.decryptCtx.keylen;
                            drmContent.decryptCtx.iv               = drApiAddrTranslateAndCheck(pMarshal->payload.drmContent.decryptCtx.iv);
                            drmContent.decryptCtx.ivlen            = pMarshal->payload.drmContent.decryptCtx.ivlen;
                            drmContent.decryptCtx.alg              = pMarshal->payload.drmContent.decryptCtx.alg;
                            drmContent.decryptCtx.outputoffet      = pMarshal->payload.drmContent.decryptCtx.outputoffet;
                            drmContent.input                       = drApiAddrTranslateAndCheck(pMarshal->payload.drmContent.input);
                            drmContent.inputDesc.nTotalSize        = pMarshal->payload.drmContent.inputDesc.nTotalSize;
                            drmContent.inputDesc.nNumBlocks        = pMarshal->payload.drmContent.inputDesc.nNumBlocks;

                            for(i = 0; i < TLAPI_DRM_INPUT_PAIR_NUMBER; i++) {
                                drmContent.inputDesc.aPairs[i].nSize   = pMarshal->payload.drmContent.inputDesc.aPairs[i].nSize;
                                drmContent.inputDesc.aPairs[i].nOffset = pMarshal->payload.drmContent.inputDesc.aPairs[i].nOffset;
                                /* Check input overflow */
                                if (i < drmContent.inputDesc.nNumBlocks &&
                                        (drmContent.inputDesc.aPairs[i].nOffset >= drmContent.inputDesc.nTotalSize ||
                                         drmContent.inputDesc.aPairs[i].nSize > (int32_t)(drmContent.inputDesc.nTotalSize
                                                                                          - drmContent.inputDesc.aPairs[i].nOffset))) {
                                    break ;
                                }
                            }
                            if (i != TLAPI_DRM_INPUT_PAIR_NUMBER) {
                                dbgSN("[DRM Driver] drIpchLoop(): ERROR - Invalid parameters received (inputDesc overflow).");
                                pMarshal->payload.retVal = E_TLAPI_DRM_INVALID_PARAMS;
                                break ;
                            }

                            drmContent.processMode                 = pMarshal->payload.drmContent.processMode;
                            drmContent.rfu                         = drApiAddrTranslateAndCheck(pMarshal->payload.drmContent.rfu);


                            /**
                             * Segment must not exceed DR_MAX_INPUT_LENGTH. To be Check the DR_MAX_INPUT_LENGTH
                             */

                            if ((!drmContent.input) ||
                                    (!drmContent.decryptCtx.key) ||
                                    (!drmContent.decryptCtx.iv) ||
                                    (drmContent.decryptCtx.ivlen == 0) ||
                                    (drmContent.inputDesc.nTotalSize == 0) ||
                                    (drmContent.inputDesc.nNumBlocks == 0) ||
                                    (drmContent.inputDesc.nNumBlocks > TLAPI_DRM_INPUT_PAIR_NUMBER)) {
                                dbgSN("[DRM Driver] drIpchLoop(): ERROR - Invalid parameters received.");
                                pMarshal->payload.retVal = E_TLAPI_DRM_INVALID_PARAMS;
                            } else {
                                /**
                                 * Initialize crypto hardware if not already done.
                                 */

                                /**
                                 * Do security check. This is to ensure that
                                 * Output buffer is within the range of protected memory.
                                 * Protected memory region is really protected.
                                 */
                                if (TLAPI_DRM_OK != drIpchSecurityCheck((addr_t)(physOutAddr + drmContent.decryptCtx.outputoffet), drmContent.inputDesc.nTotalSize)) {
                                    dbgSN("[DRM Driver] drIpchLoop(): ERROR - Security check failed");
                                    pMarshal->payload.retVal = E_TLAPI_DRM_PERMISSION_DENIED;
                                    break;
                                }

                                /**
                                 * Check the HDCP link if necessary.
                                 */

                                /**
                                 * Decrypt the data according to the input parameters passed in.
                                 * and set return value accordingly E.g pMarshal->payload.retVal = DR_E_OK
                                 */

                                drDbgPrintf("[DRM Driver] drIpchLoop(): ready for decrypt. input=%p, totalSize=%u, blocks=%u\n",
                                            drmContent.input, drmContent.inputDesc.nTotalSize, drmContent.inputDesc.nNumBlocks);

                                /*
                                 * SAMPLE: process each frame
                                 *
                                 * ___JUST FOR TEST PURPOSE___, we display the 16 first bytes of each frame/block.
                                 *
                                 */
                                for(i=0; i<drmContent.inputDesc.nNumBlocks; i++) {
                                    drDbgPrintf("[DRM Driver] drIpchLoop(): + block #%u, offset=%u, size=%u, value: ",
                                            i, drmContent.inputDesc.aPairs[i].nOffset, drmContent.inputDesc.aPairs[i].nSize);

                                    /*
                                     * SAMPLE: DUMMY decrypt and display 16 first bytes
                                     *
                                     * ___JUST FOR TEST PURPOSE__ and in alignment with TlcPlayTci, the dummy "encryption"
                                     * done by TLC is removed and the "decrypted" content is displayed.
                                     * -> data[i] = data[i] - key[i%key_size] + iv[i%iv_size]
                                     *
                                     */
                                    for (int j=0; j < drmContent.inputDesc.aPairs[i].nSize && j < 16; j++) {
                                        uint8_t c;
                                        if (drmContent.processMode == TLAPI_DRM_PROCESS_ENCRYPTED_DATA) {
                                            c =  (uint8_t)(drmContent.input[drmContent.inputDesc.aPairs[i].nOffset+j]
                                                           - drmContent.decryptCtx.key[j % drmContent.decryptCtx.keylen]
                                                           + drmContent.decryptCtx.iv[j % drmContent.decryptCtx.ivlen]);
                                        } else {
                                            c = (uint8_t)(drmContent.input[drmContent.inputDesc.aPairs[i].nOffset+j]);
                                        }
                                        drDbgPrintf("%x ", c);
                                    }
                                    drDbgPrintf("...\n");
                                }

                                tlRet = E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED;
                                dbgSN("[Drm API Driver] : FID_DR_PROCESS_DRM_CONTENT - DRIVER_NOT_IMPLEMENTED");

                            }
                            break;

                        case FID_DR_OPEN_SESSION: {
                            /**
                             * Load and authenticate video firmare if required.
                             */

                            /**
                             * Initialize security features, such as protected buffers.
                             */

                            /**
                             *  assign a new session ID
                             */
                            uint8_t sHandle = 1;
                            uint8_t *psHandle = drApiAddrTranslateAndCheck(pMarshal->payload.returned_sHandle);
                            if (psHandle != NULL) {
                                *psHandle = sHandle;
                            }

                            /**
                             * Set return value accordingly. E.g pMarshal->payload.retVal = DR_E_OK
                             */

                            tlRet = E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED;
                            dbgSN("[Drm API Driver] : FID_DR_OPEN_SESSION - DRIVER_NOT_IMPLEMENTED");
                            break;
                        }
                        case FID_DR_CLOSE_SESSION: {
                            /**
                             *  close respective session
                             */
                            uint8_t sHandle = pMarshal->payload.sHandle_to_close;
                            sHandle = sHandle;

                            /**
                             * Cleanup buffers used in previous session, i.e. fill with 0x00
                             */

                            /**
                             * Disable any security features that are no longer required
                             */

                            /**
                             * Set return value accordingly. E.g pMarshal->payload.retVal = DR_E_OK
                             */

                            tlRet = E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED;
                            dbgSN("[Drm API Driver] IPC - FID_DR_CLOSE_SESSION - DRIVER_NOT_IMPLEMENTED");
                            break;
                        }
                        case FID_DR_CHECK_LINK:
                            /**
                             * Check the external link (HDCPv1, HDCPv2, AirPlay, and DTCP) status
                             */
                            link = pMarshal->payload.link;
                            tlRet = E_TLAPI_DRM_DRIVER_NOT_IMPLEMENTED;

                            switch(link.link) {
                                case TLAPI_DRM_LINK_HDCP_1:
                                    dbgSN("[Drm API Driver] IPC - FID_DR_CHECK_LINK : HDCPv1   - DRIVER_NOT_IMPLEMENTED");
                                    break;
                                case TLAPI_DRM_LINK_HDCP_2:
                                    dbgSN("[Drm API Driver] IPC - FID_DR_CHECK_LINK : HDCPv2   - DRIVER_NOT_IMPLEMENTED");
                                    break;
                                case TLAPI_DRM_LINK_AIRPLAY:
                                    dbgSN("[Drm API Driver] IPC - FID_DR_CHECK_LINK : AIR PLAY - DRIVER_NOT_IMPLEMENTED");
                                    break;
                                case TLAPI_DRM_LINK_DTCP:
                                    dbgSN("[Drm API Driver] IPC - FID_DR_CHECK_LINK : DTCP     - DRIVER_NOT_IMPLEMENTED");
                                    break;
                                default:
                                    dbgSN("[Drm API Driver] IPC - FID_DR_CHECK_LINK : UNKNOWN  - DRIVER_NOT_IMPLEMENTED");
                                    break;
                            }
                            break;

                        default:
                            /* Unknown message has been received*/
                            tlRet = E_TLAPI_DRM_INVALID_COMMAND;
                            dbgSN("[Drm API Driver] IPC - TL_DRM_E_INVALID_COMMAND");
                            break;
                    }
                }

                /* Update response data */
                ipcMsg  = MSG_RS;
                ipcData =  TLAPI_OK;
                pMarshal->payload.retVal = tlRet;				
                break;

            default:
                drApiIpcUnknownMessage(&ipcClient, &ipcMsg, &ipcData);
                /* Unknown message has been received*/
                ipcMsg = (message_t) E_TLAPI_DRV_UNKNOWN;
                ipcData = 0;
                break;
        }

    }
}


//------------------------------------------------------------------------------
_THREAD_ENTRY void drIpch( void )
{
    drIpchLoop();
}


//------------------------------------------------------------------------------
void drIpchInit( void )
{
    /* ensure thread stack is clean */
    clearStack(drIpchStack);

    /**
     * Start IPC handler thread. Exception handler thread becomes local
     * exception handler of IPC handler thread
     */
    if (E_OK != drApiStartThread(
            DRIVER_THREAD_NO_IPCH,
            FUNC_PTR(drIpch),
            getStackTop(drIpchStack),
            IPCH_PRIORITY,
            DRIVER_THREAD_NO_EXCH))
    {
        dbgSN("[DRM Driver] drIpchInit(): drApiStartThread failed");
    }
}

