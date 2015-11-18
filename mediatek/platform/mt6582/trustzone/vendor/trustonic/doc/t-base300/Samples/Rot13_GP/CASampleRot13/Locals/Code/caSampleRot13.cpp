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

#include <stdlib.h>

#include "taSampleRot13.h"
#include "taSampleRot13_uuid.h"

#define LOG_TAG "CASampleRot13:lib"
#include "log.h"

#include "tee_client_api.h"

const TEEC_UUID uuid = taSampleRot13_UUID;
TEEC_Context  *context;
TEEC_Session  *session;
char cipher[100];

/**
*  Function r13Initialize:
*  Description:
*           Initialize: create a device context.
*  Output : TEEC_Context **context     = points to the device context
*
**/
static TEEC_Result r13Initialize(OUT TEEC_Context **context)
{
    TEEC_Result    nError;
    TEEC_Context  *pContext;

    LOG_I("[CASampleRot13] %s", __func__);

    *context = NULL;

    *context = (TEEC_Context *)malloc(sizeof(TEEC_Context));
    if (*context == NULL) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }
    memset(*context, 0, sizeof(TEEC_Context));
    /* Create Device context  */
    nError = TEEC_InitializeContext(NULL, *context);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[CASampleRot13] %s: TEEC_InitializeContext failed (%08x), %d", __func__, nError, __LINE__);
        if (nError == TEEC_ERROR_COMMUNICATION) {
            LOG_E("[CASampleRot13] %s: The client could not communicate with the service, %d", __func__, __LINE__);
        }
        free(*context);
    }

    LOG_I("[CASampleRot13] %s finished", __func__);
    return nError;
}

/**
*  Function r13Finalize:
*  Description:
*           Finalize: delete the device context.
*  Input :  TEEC_Context *context     = the device context
*
**/

static TEEC_Result r13Finalize(IN OUT TEEC_Context *context)
{

    LOG_I("[CASampleRot13] %s", __func__);

    if (context == NULL) {
        LOG_E("[CASampleRot13] %s: Device handle invalid, %d", __func__, __LINE__);
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    TEEC_FinalizeContext(context);
    free(context);
    return TEEC_SUCCESS;
}

/**
*  Function r13CloseSession:
*  Description:
*           Close the client session.
*  Input :  TEEC_Session *session - session handler
*
**/
static TEEC_Result r13CloseSession(IN TEEC_Session *session)
{
    LOG_I("[CASampleRot13] %s", __func__);

    if (session == NULL) {
        LOG_E("[CASampleRot13] %s: Invalid session handle, %d", __func__, __LINE__);
        return TEEC_ERROR_BAD_PARAMETERS;
    }
    TEEC_CloseSession(session);
    free(session);
    return TEEC_SUCCESS;
}

/**
*  Function r13OpenSession:
*  Description:
*           Open a client session with a specified service.
*  Input :  TEEC_Context  *context,    = the device context
*  Output:  TEEC_Session  **session   = points to the session handle
*
**/
static TEEC_Result r13OpenSession(
    IN    TEEC_Context  *context,
    OUT   TEEC_Session  **session)
{
    TEEC_Operation sOperation;
    TEEC_Result    nError;
    uint32_t returnOrigin;

    LOG_I("[CASampleRot13] %s", __func__);

    *session = (TEEC_Session *)malloc(sizeof(TEEC_Session));
    if (*session == NULL) {
        return TEEC_ERROR_OUT_OF_MEMORY;
    }
    memset(*session, 0, sizeof(TEEC_Session));
    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = 0;
    nError = TEEC_OpenSession(context,
                              *session,                    /* OUT session */
                              &uuid,                      /* destination UUID */
                              TEEC_LOGIN_PUBLIC,          /* connectionMethod */
                              NULL,                       /* connectionData */
                              &sOperation,                /* IN OUT operation */
                              NULL                        /* OUT returnOrigin, optional */
                             );
    if (nError != TEEC_SUCCESS) {
        LOG_E("[CASampleRot13] %s: TEEC_OpenSession failed (%08x), %d", __func__, nError, __LINE__);
        free(*session);
        return nError;
    }
    return TEEC_SUCCESS;
}

/**
*  Function r13Run:
*  Description:
*           This function just send a command CMD_HELLO to the service.
*  Input:   TEEC_Session *session   = the session handle
**/
static TEEC_Result r13Run(IN TEEC_Session *session,
                          char *pData,
                          uint32_t nSize)
{
    TEEC_Result    nError;
    TEEC_Operation sOperation;
    char          *pMsg;

    LOG_I("[CASampleRot13] %s", __func__);

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(
                                TEEC_MEMREF_TEMP_INOUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    sOperation.params[0].tmpref.buffer = pData;
    sOperation.params[0].tmpref.size   = nSize;

    LOG_I("[CASampleRot13] %s: clear text for Trustlet: %s", __func__, sOperation.params[0].tmpref.buffer);
    nError = TEEC_InvokeCommand(session,
                                CMD_SAMPLE_ROT13,
                                &sOperation,       /* IN OUT operation */
                                NULL               /* OUT returnOrigin, optional */
                               );
    if (nError != TEEC_SUCCESS) {
        LOG_E("[CASampleRot13] %s: TEEC_InvokeCommand failed (%08x), %d", __func__, nError, __LINE__);
    }

    return nError;
}

// -------------------------------------------------------------
bool caOpen(void)
{
    TEEC_Result     nError;

    LOG_I("[CASampleRot13] %s", __func__);

    nError = r13Initialize(&context);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[CASampleRot13] %s: r13Initialize failed (%08x), %d", __func__, nError, __LINE__);
        return false;
    }

    /* Open a session */
    nError = r13OpenSession(context, &session);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[CASampleRot13] %s: r13OpenSession failed (%08x), %d", __func__, nError, __LINE__);
        return false;
    }

    return true;
}

// -------------------------------------------------------------
char *caRot13(const char *plainText)
{
    strcpy(cipher, plainText);
    r13Run(session, cipher, strlen(plainText));
    return cipher;
}
// -------------------------------------------------------------
void caClose(void)
{
    TEEC_Result       nError;

    LOG_I("[CASampleRot13] %s", __func__);

    /* Close the session */
    nError = r13CloseSession(session);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[CASampleRot13] %s: r13CloseSession failed (%08x), %d", __func__, nError, __LINE__);
    }

    /* Finalize */
    nError = r13Finalize(context);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[CASampleRot13] %s: r13Finalize failed (%08x), %d", __func__, nError, __LINE__);
    }
}
