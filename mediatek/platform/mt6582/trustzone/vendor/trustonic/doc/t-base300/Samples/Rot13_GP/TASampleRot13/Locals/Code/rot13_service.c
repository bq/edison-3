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

/* ----------------------------------------------------------------------------
 *   Includes
 * ---------------------------------------------------------------------------- */
#include "taStd.h"
#include "tee_internal_api.h"

#include "taSampleRot13.h"

/* ----------------------------------------------------------------------------
 *   Global regions and defines
 * ---------------------------------------------------------------------------- */

// Reserve 2048 byte for stack.
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(2048);

// Reserve 2048 byte for heap.
DECLARE_TRUSTED_APPLICATION_MAIN_HEAP(2048);

#define TATAG "TA Rot13 "

/* ----------------------------------------------------------------------------
 *   Defines
 * ---------------------------------------------------------------------------- */
struct session {
    uint32_t calls; //count number of calls in this session
    uint32_t nEncrypted; //count number of encrypted data in this session
};

/* ----------------------------------------------------------------------------
 *   Service Entry Points
 * ---------------------------------------------------------------------------- */

/**
 *  Function TA_CreateEntryPoint:
 *  Description:
 *        The function TA_CreateEntryPoint is the service constructor, which the system
 *        calls when it creates a new instance of the service.
 *        Here this function implements nothing.
 **/
TEE_Result TA_EXPORT TA_CreateEntryPoint(void)
{
    TEE_DbgPrintLnf(TATAG "TA_CreateEntry Point");

    return TEE_SUCCESS;
}

/**
 *  Function TA_DestroyEntryPoint:
 *  Description:
 *        The function TA_DestroyEntryPoint is the service destructor, which the system
 *        calls when the instance is being destroyed.
 *        Here this function implements nothing.
 **/
void TA_EXPORT TA_DestroyEntryPoint(void)
{
    TEE_DbgPrintLnf(TATAG "TA_DestroyEntryPoint");
}

/**
 *  Function TA_OpenSessionEntryPoint:
 *  Description:
 *        The system calls the function TA_OpenSessionEntryPoint when a new client
 *        connects to the service instance.
 **/
TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(  uint32_t nParamTypes,
                                                IN OUT TEE_Param pParams[4],
                                                OUT void** ppSessionContext
)
{
    S_VAR_NOT_USED(nParamTypes);
    S_VAR_NOT_USED(pParams);
    S_VAR_NOT_USED(ppSessionContext);

    TEE_DbgPrintLnf(TATAG "TA_OpenSessionEntryPoint");

    struct session *clientSession = (struct session *)TEE_Malloc(sizeof(*clientSession), 0);
    if (clientSession == NULL) {
        TEE_DbgPrintLnf(TATAG "out of memory!");
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    TEE_DbgPrintLnf(TATAG "allocated session at %p", clientSession);
    if (TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_READ |
                                    TEE_MEMORY_ACCESS_WRITE,
                                    clientSession, sizeof(*clientSession)) != TEE_SUCCESS) {
        TEE_DbgPrintLnf(TATAG "wrong access rights!");
        return TEE_ERROR_ACCESS_CONFLICT;
    }

    clientSession->calls = 0;
    clientSession->nEncrypted = 0;

    *ppSessionContext = clientSession;

    return TEE_SUCCESS;
}

/**
 *  Function TA_CloseSessionEntryPoint:
 *  Description:
 *        The system calls this function to indicate that a session is being closed.
 **/
void TA_EXPORT TA_CloseSessionEntryPoint(IN OUT void* pSessionContext)
{
    TEE_DbgPrintLnf(TATAG "TA_CloseSessionEntryPoint");

    struct session *clientSession = (struct session *)pSessionContext;

    TEE_DbgPrintLnf(TATAG "client called %i times, encoded %i bytes.",
            clientSession->calls,
            clientSession->nEncrypted);

    TEE_Free(clientSession);
}

/**
 * Process a ROT13 command message.
 * The command data will be checked for in
 *
 * @return RET_OK if operation has been successfully completed.
 */
static TEE_Result rot13(char *txt, uint32_t len) {
    // Print text in debug version
    TEE_DbgPrintLnf(TATAG "Cleartext: %s\n", txt);
    TEE_DbgPrintLnf(TATAG "len %d\n", len);

    // Check length parameter
    if (len > MAX_DATA_LEN) {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    // ROT13 encryption
    uint32_t i;
    for (i = 0; i < len; i++) {
        // get char from message buffer
        char c = txt[i];
        // convert lower case letter to upper case
        char upper = (char) (c & (char) 32);
        c &= ~upper;
        // do ROT13 on A-Z, leave any other bytes untouched
        if ((c >= 'A') && (c <= 'Z')) {

            // following implementation uses a division. However, ARM CPUs do
            // not support this natively, to the library has to provide
            // code for this. For the RVDS 4.1 library, the additional
            // div code increases the Trusted App's code segment size by
            // about 396 byte.
            //    c = ((c - 'A' + 13) % 26 + 'A');

            // this implementation avoids a division
            c += 13;
            if (c > 'Z') {
                c -= 26;
            }
        }
        // resore lower case letter
        c |= upper;
        // write ROT13 encrypted char back to message buffer
        txt[i] = c;
    }

    TEE_DbgPrintLnf(TATAG "Ciphertext: %s\n", txt);

    return TEE_SUCCESS;
}

/**
 *  Function TA_InvokeCommandEntryPoint:
 *  Description:
 *        The system calls this function when the client invokes a command within
 *        a session of the instance.
 *        Here this function perfoms a switch on the command Id received as input,
 *        and returns the main function matching to the command id.
 **/
TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(IN OUT void* pSessionContext,
                                                uint32_t nCommandID,
                                                uint32_t nParamTypes,
                                                TEE_Param pParams[4])
{
    uint8_t* pOutput;
    uint32_t nOutputSize;
    struct session *clientSession = (struct session *)pSessionContext;

    TEE_DbgPrintLnf(TATAG "TA_InvokeCommandEntryPoint");
    clientSession->calls++;

    switch(nCommandID)
    {
        case CMD_SAMPLE_ROT13:
        if (nParamTypes != CMD_ROT13_PTYPES)
        {
            TEE_DbgPrintLnf(TATAG "bad params");
            return TEE_ERROR_BAD_PARAMETERS;
        }

        pOutput = pParams[0].memref.buffer;
        nOutputSize = (uint32_t)pParams[0].memref.size;

        if ((pOutput != NULL) && (nOutputSize != 0))
        {
            // Verify that NWd buffer is fully in NWd, and does not extend too far.
            if (TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_READ |
                                            TEE_MEMORY_ACCESS_WRITE |
                                            TEE_MEMORY_ACCESS_ANY_OWNER,
                                            pOutput, nOutputSize) != TEE_SUCCESS) {
                TEE_DbgPrintLnf(TATAG "wrong access rights!");
                return TEE_ERROR_ACCESS_CONFLICT;
            }
            dbgBlob("my blog", NULL, 0);
            // Rot13 call
            TEE_Result ret = rot13((char*)pOutput, nOutputSize);
            if (ret == TEE_SUCCESS) {
                clientSession->nEncrypted += nOutputSize;
            }
            return ret;
        }
        return TEE_SUCCESS;
        case CMD_HELLO:
        if (nParamTypes != CMD_HELLO_PTYPES)
        {
            return TEE_ERROR_BAD_PARAMETERS;
        }

        pOutput = pParams[0].memref.buffer;
        nOutputSize = (uint32_t)pParams[0].memref.size;

        if ((pOutput != NULL) && (nOutputSize != 0))
        {
            char msg[] = "Hello, World !";
            if (nOutputSize < sizeof(msg) + 1)
            {
                return TEE_ERROR_SHORT_BUFFER;
            }

            nOutputSize = sizeof(msg) + 1;

            TEE_MemFill(pOutput, nOutputSize, 1);

            TEE_MemMove(pOutput, msg, nOutputSize);
            pParams[0].memref.size = nOutputSize;

            int32_t c = TEE_MemCompare(pOutput, msg, nOutputSize);
            if (c != 0) {
                return TEE_ERROR_GENERIC;
            }
        }
        return TEE_SUCCESS;

        case CMD_HELLO*2:
        TEE_Panic(2);
        default:
        TEE_DbgPrintLnf(TATAG "invalid command ID: 0x%X", nCommandID);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}
