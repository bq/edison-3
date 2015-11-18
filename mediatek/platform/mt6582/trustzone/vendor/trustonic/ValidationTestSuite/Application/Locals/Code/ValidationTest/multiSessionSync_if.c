/**
 * <!-- Copyright Giesecke & Devrient GmbH 2012 -->
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include <android/log.h>

#include "multiSessionSync_if.h"
#include "MobiCoreDriverApi.h"
#include "tlTestApi.h"
#include "DrIntegrationTestApi.h"
#include "TlApi/TlApiError.h"

/* Logging macros */
#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG , "TlcIntegrationTest", __VA_ARGS__)
#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO  , "TlcIntegrationTest", __VA_ARGS__)
#define LOG_W(...) __android_log_print(ANDROID_LOG_WARN  , "TlcIntegrationTest", __VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR , "TlcIntegrationTest", __VA_ARGS__)
#define DEFAULT_SLEEP_SEC   2
#define WAIT_TIMEOUT_MSEC    10000
#define MAX_LISTENER_THREADS 20


/* Global definitions */
static const uint32_t        DEVICE_ID = MC_DEVICE_ID_DEFAULT;
static const mcUuid_t        uuidTl = TL_TEST_UUID;
static const mcUuid_t        uuidDr = DRV_DRINTEGRATIONSTEST_UUID;
static ASYNC_IF_EventHandler gEventHandler = NULL;
static uint32_t              gEventMask = 0x0;
static pthread_cond_t        gThreadCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t       gMutexLock = PTHREAD_MUTEX_INITIALIZER;
static bool                  gAsyncEventsEnabled = false;
static mcSessionHandle_t     gSessionHandle[MAX_LISTENER_THREADS];
static pthread_t             gListenerThreads[MAX_LISTENER_THREADS];
static uint32_t listenerThreadCounter = 0;

static dciMessageDrIntegrationTest_t			*dci;			// For driver cmds
static tciMessage_t  		 					*tci; 			// For trustlet cmds


/*!
*   ASYNC_IF_listenDci
*
*   DCI listener function
*
*   @param  param     Session id
*/
static void ASYNC_IF_listenDci(
    void *param
){
    uint32_t    sessionId = (uint32_t)param;
    mcResult_t  mcRet;
    uint32_t    eventType;
    mcSessionHandle_t session;


    LOG_I("ASYNC_IF_listenDci(): DCI listener thread started for session 0x%.8x\n", sessionId);

    // Create session data.
    session.sessionId = sessionId;
    session.deviceId = DEVICE_ID;

    for(;;)
    {
        LOG_I("ASYNC_IF_listenDci(): Waiting for notification\n");

        /* Wait for notification from Swd */
        if (MC_DRV_OK != mcWaitNotification(&session, 5000)) // MC_INFINITE_TIMEOUT
        {
            LOG_E("ASYNC_IF_listenDci(): mcWaitNotification failed\n");
            break;
        }

        /* This is not exception. Process it as command response */
        // TODO -zenginon-07_11_12 Mutex should be unlocked after thread condition update
        pthread_mutex_lock(&gMutexLock);
        /* Signal the receiver thread */
        pthread_cond_signal(&gThreadCond);
        /* Unlock mutex until */
        pthread_mutex_unlock(&gMutexLock);


        /* Call event handler if events are enabled and event handler is set */
        if ((eventType != RET_ERR_UNKNOWN_CMD) &&
            (gEventHandler) &&
            (gAsyncEventsEnabled)) {
            LOG_I("ASYNC_IF_listenDci(): Calling event handler for event id %d\n", eventType);
            (*gEventHandler)(sessionId, eventType, NULL, 0, &dci->dciResponseHeader.returnCode);
        }
    }
}

/*!
*   ASYNC_IF_listenTci
*
*   TCI listener function
*
*   @param  param     Session id
*/
static tlApiResult_t ASYNC_IF_listenTci(
    void *param

){
    uint32_t    sessionId = (uint32_t)param;
    mcResult_t  mcRet;
    uint32_t    eventType;

    mcSessionHandle_t session;


    LOG_I("ASYNC_IF_listenTci(): TCI listener thread started for session 0x%.8x\n", sessionId);

    // Create session data.
    session.sessionId = sessionId;
    session.deviceId = DEVICE_ID;

    for(;;)
    {
        LOG_I("ASYNC_IF_listenTci(): Waiting for notification\n");

        /* Wait for notification from Swd for 2 secs */
        tlApiResult_t res = mcWaitNotification(&session, WAIT_TIMEOUT_MSEC);
        if ((MC_DRV_OK != res) && (MC_DRV_ERR_TIMEOUT != res))
        {
            LOG_E("ASYNC_IF_listenTci(): mcWaitNotification failed %08x \n", res);
            decreaseListenerThreadCounter();
            return res;
        }

        if (MC_DRV_ERR_TIMEOUT == res) {
        	LOG_E("ASYNC_IF_listenTci(): mcWaitNotification TIMED OUT: %08x\n", res);
        }
        /* NOTE: Exception handling of debug agent is removed. Process it as command response */
        // TODO -zenginon-07_11_12 Mutex should be unlocked after thread condition update
        pthread_mutex_lock(&gMutexLock);
        /* Signal the receiver thread */
        pthread_cond_signal(&gThreadCond);
        /* Unlock mutex until */
        pthread_mutex_unlock(&gMutexLock);

        decreaseListenerThreadCounter();

        return res;

    }
}

/*!
*   ASYNC_IF_notificationListener
*
*   Listens notifications from Swd
*
*   @param  ulSessionId     Session id
*/
static tlApiResult_t ASYNC_IF_dciNotificationListener(
    uint32_t ulSessionId
){
	tlApiResult_t  ret = ASYNC_ERR_NONE;
    uint32_t     res;

    LOG_E("ASYNC_IF_dciNotificationListener(): Creating notification listener thread for session (%d)\n", ulSessionId);

    res = increaseListenerThreadCounter(MULTI_SESSION_TYPE_DRIVER, ulSessionId);
    if (0 != res)
    {
        LOG_E("ASYNC_IF_dciNotificationListener(): Cannot create listener thread (%d)\n", res);
        ret = ASYNC_ERR_NOTIFY;
    }

    return ret;
}

/*!
*   ASYNC_IF_tciNotificationListener
*
*   Listens notifications from Swd
*
*   @param  ulSessionId     Session id
*/
static tlApiResult_t ASYNC_IF_tciNotificationListener(
    uint32_t ulSessionId
){
    tlApiResult_t	ret = TLAPI_OK;
    uint32_t     	res;

    LOG_I("ASYNC_IF_tciNotificationListener(): Creating notification listener thread for session (%d)\n", ulSessionId);
    res = increaseListenerThreadCounter(MULTI_SESSION_TYPE_TRUSTLET, ulSessionId);

    if (0 != res)
    {
        LOG_E("ASYNC_IF_tciNotificationListener(): Cannot create listener thread (%d)\n", res);
        ret = ASYNC_ERR_NOTIFY; // TODO -zenginon: 06-11-12 change when error handler is defined.
    }
    return ret;
}


/*!
*   ASYNC_IF_notificationListener
*
*   Listens notifications from Swd
*
*   @param  ulSessionId     Session id
*   @param  sessionType     tl or dr session? tl = 0, dr = 1
*/

static ASYNC_ERR ASYNC_IF_notificationListener(
    uint32_t ulSessionId,
    uint32_t sessionType
) {
	if (sessionType == MULTI_SESSION_TYPE_DRIVER) {
		return ASYNC_IF_dciNotificationListener(ulSessionId);
	} else {
		return ASYNC_IF_tciNotificationListener(ulSessionId);
	}
}

static uint32_t increaseListenerThreadCounter(uint32_t sessionType, uint32_t ulSessionId) {

	uint32_t res = 0;

	LOG_I("increaseListenerThreadCounter(): Listener thread Index: %d.", listenerThreadCounter);

	if (listenerThreadCounter == MAX_LISTENER_THREADS) {
		LOG_I("increaseListenerThreadCounter(): Listener thread number will be out of boundaries: %d.", listenerThreadCounter+1);
		return 1;
	}

	pthread_mutex_lock(&gMutexLock);
	listenerThreadCounter++;
	pthread_mutex_unlock(&gMutexLock);

	if (sessionType == MULTI_SESSION_TYPE_DRIVER) {
		res = pthread_create(&gListenerThreads[listenerThreadCounter],
							NULL,
							(void *) &ASYNC_IF_listenDci,
							(void *) ulSessionId);
	} else {
		res = pthread_create(&gListenerThreads[listenerThreadCounter],
							NULL,
							(void *) &ASYNC_IF_listenTci,
							(void *) ulSessionId);

	}

	LOG_I("increaseListenerThreadCounter(): FINALIZED Listener thread number is now : %d.", listenerThreadCounter);
	return res;
}

static uint32_t decreaseListenerThreadCounter() {

	LOG_I("decreaseListenerThreadCounter(): Listener thread number: %d.", listenerThreadCounter);
	if (listenerThreadCounter == MAX_LISTENER_THREADS) {
		LOG_I("decreaseListenerThreadCounter(): Listener thread number will be out of boundaries: %d.", listenerThreadCounter -1);
		return 1;
	}

	pthread_mutex_lock(&gMutexLock);
	listenerThreadCounter--;
	pthread_mutex_unlock(&gMutexLock);

	return 0;
}


static ASYNC_ERR ASYNC_IF_string2Uuid(
    const char *trustletId,
    mcUuid_t *uuid
){
    ASYNC_ERR  ret = ASYNC_ERR_NONE;

    return ret;
}




/*!
*   ASYNC_IF_executeCommand
*
*   Register a handler for asynchronous events from the trustlet connector
*
*   @param  session     Trustlet debug session ID
*/
static mcResult_t ASYNC_IF_executeCommand(
		mcSessionHandle_t session,
		uint32_t threadNo
){
    mcResult_t        mcRet = TLAPI_OK;
    struct timespec   ts;
    struct timeval    tp;

    /* Lock mutex so that event handler thread waits until condition wait state */
    pthread_mutex_lock(&gMutexLock);

    do {
        /**
         * Check if API is called by the listener thread.
         * If it is, return failure as APIs cannot be directly
         * called in the listener thread
         */
        if (gListenerThreads[threadNo] == pthread_self())
        {
        	LOG_E("ASYNC_IF_tciExecuteCommand(): no Listener thread:");
            mcRet = E_TLAPI_UNKNOWN;
            break;
        }

        /* Update the message type  to indicate that this is a command message */
        // tci->commandHeader.commandId = cmdId;

        /* Notify trustlet */
        mcRet = mcNotify(&session);
        if (MC_DRV_OK != mcRet)
        {
            LOG_E("ASYNC_IF_tciExecuteCommand(): mcNotify returned: %d\n", mcRet);
            return mcRet;
        }

        if (ASYNC_ERR_NONE != gettimeofday(&tp, NULL))
        {
            LOG_E("ASYNC_IF_tciExecuteCommand(): gettimeofday failed\n");
            mcRet = E_TLAPI_UNKNOWN_FUNCTION;
            break;
        }

        /* Convert from timeval to timespec and add timeout value */
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += WAIT_TIMEOUT_MSEC / 1000;

        if (ASYNC_ERR_NONE != pthread_cond_timedwait(&gThreadCond, &gMutexLock, &ts))
        {
            LOG_E("ASYNC_IF_tciExecuteCommand(): pthread_cond_wait failed\n");
            mcRet = E_TLAPI_TIMEOUT;
            break;
        }

        LOG_E("ASYNC_IF_tciExecuteCommand(): response return code: %d\n", tci->responseHeader.returnCode);

    } while(false);

    /* Unlock mutex */
    pthread_mutex_unlock(&gMutexLock);
    return mcRet;
}

/*!
*   ASYNC_IF_RegisterEventHandler
*
*   Register a handler for asynchronous events from the trustlet connector
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  eventHandler    Event handler function
*   @param  pulEventMask    Pointer to the initial event mask
*/
ASYNC_ERR ASYNC_IF_RegisterEventHandler(
    uint32_t ulSessionId,
    ASYNC_IF_EventHandler eventHandler,
    uint32_t *pulEventMask
){
     /* TODO-R-2012-03-26-guerelil Support registering event hadles for multipble sessions */
    LOG_I("ASYNC_IF_RegisterEventHandler(): Registering event handler\n");
    gEventHandler = eventHandler;
    return ASYNC_ERR_NONE;
}


/*!
*   ASYNC_IF_SetEventMask
*
*   Set the mask for asynchronous events
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  pulEventMask    Pointer to the event mask arrary to apply
*/
ASYNC_ERR ASYNC_IF_SetEventMask(
    uint32_t ulSessionId,
    uint32_t *pulEventMask
){
    /* TODO-R-2012-03-26-guerelil Support event hadles for multiple sessions */
    LOG_I("ASYNC_IF_SetEventMask(): Setting event mask 0x%.8x\n", *pulEventMask);
    gEventMask = *pulEventMask;

    return ASYNC_ERR_NONE;
}


/*!
*   ASYNC_IF_GetEventMask
*
*   Return the current asynchronous events mask
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  pulEventMask    Return pointer for the current event mask array
*/
ASYNC_ERR ASYNC_IF_GetEventMask(
    uint32_t ulSessionId,
    uint32_t *pulEventMask
){
    /* TODO-R-2012-03-26-guerelil Support event hadles for multiple sessions */
    *pulEventMask = gEventMask;
    LOG_I("ASYNC_IF_GetEventMask(): Getting event mask 0x%.8x\n", *pulEventMask);
    return ASYNC_ERR_NONE;
}


/*!
*   ASYNC_IF_EnableAsyncEvents
*
*   Enable or disable asynchronous events from the connector
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  enable          Enable (1) or disable (0) asyncronous events for this session
*/
ASYNC_ERR ASYNC_IF_EnableAsyncEvents(
    uint32_t ulSessionId,
    bool     enable
){
    /* TODO-R-2012-03-26-guerelil Support multiple sessions */
    LOG_I("ASYNC_IF_EnableAsyncEvents(): Async events enable (%d) for session id %d\n", enable, ulSessionId);
    gAsyncEventsEnabled = enable;
    return ASYNC_ERR_NONE;
}


/*!
*   ASYNC_IF_GetVersion
*
*   Return the interface version string
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  sVersionString  The returned interface version string
*/
ASYNC_ERR ASYNC_IF_GetVersion(
    uint32_t ulSessionId,
    char     *sVersionString
){
    LOG_W("ASYNC_IF_GetVersion(): Not supported\n");
    return ASYNC_ERR_NOT_SUPPORTED;
}



/*!
*   ASYNC_IF_DciOpen
*
*   Open a new test session
*
*   @param  pulSessionId    Return pointer for the new debug session ID
*/
tlApiResult_t ASYNC_IF_DciOpen(
    uint32_t *pulSessionId,
    uint32_t *dciBuffer
){
	tlApiResult_t  ret = TLAPI_OK;
	mcSessionHandle_t session;

    // Create session data.
    session.sessionId = *pulSessionId;
    session.deviceId = DEVICE_ID;



    bzero(&gSessionHandle, sizeof(mcSessionHandle_t));

    do {

        /* Open session to the debug driver trustlet */
        ret = mcOpenSession(&session,
                              &uuidDr,
                              (uint8_t *) dciBuffer,
                              (uint32_t) sizeof(dciMessageDrIntegrationTest_t));
        if (MC_DRV_OK != ret)
        {
            LOG_E("ASYNC_IF_Open(): mcOpenSession returned: %d\n", ret);
            ret = E_TLAPI_UNMAPPED_BUFFER;
            break;
        }

        /* Launch notification listener */
        ASYNC_IF_notificationListener((uint32_t) session.sessionId, MULTI_SESSION_TYPE_DRIVER);

    } while (false);

    LOG_I("ASYNC_IF_Open(): returning: 0x%.8x\n", ret);

    return ret;
}

/*!
*   ASYNC_IF_Open
*
*   Open a new test session
*
*   @param  pulSessionId    Return pointer for the new debug session ID
*/
tlApiResult_t ASYNC_IF_TciOpen(
    uint32_t *pulSessionId,
    uint32_t *tciBuffer
){
    tlApiResult_t  ret = TLAPI_OK;
    mcSessionHandle_t session;

    // Create session data.
    session.sessionId = *pulSessionId;
    session.deviceId = DEVICE_ID;

    do {
        /* Open session to the debug driver trustlet */

        ret = mcOpenSession(&session,
                              &uuidTl,
                              (uint8_t *) tciBuffer,
                              (uint32_t) sizeof(tciMessage_t));


        LOG_I("!!! ASYNC_IF_TciOpen(): Opening session  for DEV ID: 0x%08x, SESSION ID: 0x%08x", session.deviceId, session.sessionId );

        if (MC_DRV_OK != ret)
        {
            LOG_E("ASYNC_IF_Open(): mcOpenSession returned error: %d\n", ret);
            ret = E_TLAPI_UNMAPPED_BUFFER;
            break;
        }

        /* Launch notification listener */
        LOG_I("!!! ASYNC_IF_TciOpen(): Session ID : 0x%08x",(uint32_t)session.sessionId );
        sleep(2);

        // return session ID
        *pulSessionId = session.sessionId;
        ASYNC_IF_notificationListener(*pulSessionId, MULTI_SESSION_TYPE_TRUSTLET);

    } while (false);

    LOG_I("!!! ASYNC_IF_Open(): returning: 0x%.8x\n", ret);

    return ret;
}

/*!
*   ASYNC_IF_Open
*
*   Open a new test session
*
*   @param  pulSessionId    Return pointer for the new debug session ID
*/
tlApiResult_t ASYNC_Open(
    uint32_t *pulSessionId,
    uint32_t sessionType,
    uint32_t * buffer
){
    if (sessionType == MULTI_SESSION_TYPE_DRIVER) {
    	return ASYNC_IF_DciOpen(pulSessionId, (uint32_t *)buffer);
    } else {
    	return ASYNC_IF_TciOpen(pulSessionId, (uint32_t*)buffer);
    }
}


/*!
*   ASYNC_IF_Close
*
*   Close the debug session
*
*   @param  ulSessionId     Trustlet debug session ID
*/
tlApiResult_t ASYNC_Close(
    uint32_t ulSessionId,
    uint32_t sessionType,
    uint32_t *buffer
){
    ASYNC_ERR  ret = MC_DRV_OK;
    mcResult_t   mcRet;

    mcSessionHandle_t session;

    // Create session data.
    session.sessionId = ulSessionId;
    session.deviceId = DEVICE_ID;

    do {
    	LOG_I("ASYNC_IF_Close(): mcCloseSession received session id: 0x%x\n", ulSessionId);

    	/* buffer already freed*/
    	if (buffer == NULL) {
    		break;
    	}


    	/* Clear DCI message buffer */
    	if (sessionType == MULTI_SESSION_TYPE_DRIVER) {
    		memset(buffer, 0, sizeof(dciMessageDrIntegrationTest_t));
    	} else {
    		memset(buffer, 0, sizeof(tciMessage_t));
    	}

        /* Close session to the debug driver trustlet */
    	mcRet = mcCloseSession(&session);
        if (MC_DRV_OK != mcRet)
        {
            LOG_E("ASYNC_IF_Close(): mcCloseSession failed with status: %d\n", mcRet);
            return mcRet;
        }

        LOG_I("!!!  ASYNC_IF_Close(): mcCloseSession returned success\n");

        mcRet = mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t *)buffer);
        if (MC_DRV_OK != mcRet)
        {
        	LOG_E("ASYNC_IF_Close(): mcFreeWsm failed with status: %d\n", mcRet);
            return mcRet;
        }

        LOG_I("ASYNC_IF_Close(): mcFreeWsm returned success\n");

    } while (false);

    LOG_I("ASYNC_IF_Close(): returning: 0x%.8x\n", ret);

    return ret;
}

