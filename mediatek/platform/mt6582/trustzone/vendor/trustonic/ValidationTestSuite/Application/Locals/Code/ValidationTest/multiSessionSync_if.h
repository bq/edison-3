/*!
* @file    multiSessionSync.h
* @brief   MobiCore Trustlet Debug Interface Definition
*
* This file defines the interface between the trustlet debug agent and the
* trustlet connector
*
* The intended usage, from the agent's, is as follows:
*
* - Open a new debug session with ASYNC_Open()
* - Register for asynchronous events from the connector with ASYNC_RegisterEventHandler()
* - Connect the session to the specified trustlet using ASYNC_Connect()
*       - During the connect call, the agent expects to receive an event from
*         the connector to indicate trustlet status
*
* - To end the session the agent should disconnect from the trustlet with ASYNC_Disconnect()
*       - During the disconnect call, the agent expects to receive an event from
*         the connector with the final status
* - Close the session with ASYNC_Close()
*/

#if !defined(ASYNC_H)
#define ASYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "MobiCoreDriverApi.h"
// #include "tlTestApi.h"
#include "TlApi/TlApiError.h"


// Session type constants
#define MULTI_SESSION_TYPE_TRUSTLET		0
#define MULTI_SESSION_TYPE_DRIVER		1

// UUID Length
#define UUID_LENGTH 					20

/* size of the event mask array, in 32-bit words (in case we ever need more than 32 event types) */
#define EVENT_MASK_SIZE (1)

/* Event masks */
#define ASYNC_EVENT_MASK_EXE               (1U << 0) /* Event mask for execution state changes */
#define ASYNC_EVENT_MASK_CRASHED           (1U << 1) /* Event mask for crash events */
#define ASYNC_EVENT_MASK_REMOVED           (1U << 2) /* Event mask for trustlet unload */
#define ASYNC_EVENT_MASK_BP                (1U << 3) /* Event mask for breakpoint events */


/* error codes */
typedef enum
{
    ASYNC_ERR_NONE          = 0,
    ASYNC_ERR_FAIL          = 1,
    ASYNC_ERR_NOT_SUPPORTED = 2,
    ASYNC_ERR_MC_SESSION    = 3,
    ASYNC_ERR_MAP_ERROR     = 4,
    ASYNC_ERR_CMD_EXECUTE   = 5,
    ASYNC_ERR_NOTIFY        = 6
    // more can be added as required
} ASYNC_ERR;


/* asynchronous events */
typedef enum
{
    TLDBG_EVENT_TRUSTLET_EXE,       /* trustlet execution state changed */
    TLDBG_EVENT_TRUSTLET_CRASHED,   /* trustlet has crashed */
    TLDBG_EVENT_TRUSTLET_REMOVED,   /* trustlet has been removed/unloaded */
    TLDBG_EVENT_TRUSTLET_BP,        /* trustlet breakpoint is hit */
    TLDBG_EVENT_LOG_COMMS,          /* new debug logging data to be passed upwards */
    TLDBG_EVENT_UNKNOWN             /* unknown event */
    // more can be added as required
} TLDBG_IF_Event;


/* callback definition for asynchronous event handling */
typedef ASYNC_ERR (*ASYNC_IF_EventHandler)(
    uint32_t ulSessionId,
    uint32_t ulEventId,
    uint8_t  *pEventData,
    uint32_t ulEventDataSize,
    uint32_t *pulEventReturn
);


/*!
*   ASYNC_RegisterEventHandler
*
*  Register a handler for asynchronous events from the trustlet connector
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  eventHandler    Event handler function
*   @param  pulEventMask    Pointer to the initial event mask
*/
ASYNC_ERR ASYNC_RegisterEventHandler(
    uint32_t ulSessionId,
    ASYNC_IF_EventHandler eventHandler,
    uint32_t *pulEventMask
);


/*!
*   ASYNC_SetEventMask
*
*   Set the mask for asynchronous events
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  pulEventMask    Pointer to the event mask arrary to apply
*/
ASYNC_ERR ASYNC_SetEventMask(
    uint32_t ulSessionId,
    uint32_t *pulEventMask
);


/*!
*   ASYNC_GetEventMask
*
*   Return the current asynchronous events mask
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  pulEventMask    Return pointer for the current event mask array
*/
ASYNC_ERR ASYNC_GetEventMask(
    uint32_t ulSessionId,
    uint32_t *pulEventMask
);


/*!
*   ASYNC_EnableAsyncEvents
*
*   Enable or disable asynchronous events from the connector
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  enable          Enable (1) or disable (0) asyncronous events for this session
*/
ASYNC_ERR ASYNC_EnableAsyncEvents(
    uint32_t ulSessionId,
    bool     enable
);


/*!
*   ASYNC_GetVersion
*
*   Return the interface version string
*
*   @param  ulSessionId     Trustlet debug session ID
*   @param  sVersionString  The returned interface version string
*/
ASYNC_ERR ASYNC_GetVersion(
    uint32_t ulSessionId,
    char     *sVersionString
);

/*!
*   ASYNC_DciOpen
*
*   Open a new debug session
*
*   @param  pulSessionId    Return pointer for the new debug session ID
*/
tlApiResult_t ASYNC_DciOpen(
    uint32_t *pulSessionId,
    uint32_t *dciBuffer
);



/*!
*   ASYNC_TciOpen
*
*   Open a new debug session
*
*   @param  pulSessionId    Return pointer for the new debug session ID
*/
tlApiResult_t ASYNC_TciOpen(
    uint32_t *pulSessionId,
    uint32_t *tciBuffer
);



/*!
*   ASYNC_Open
*
*   Open a new debug session
*
*   @param  pulSessionId    Return pointer for the new debug session ID
*/
tlApiResult_t ASYNC_Open(
    uint32_t *pulSessionId,
    uint32_t sessionType,
    uint32_t *bufferAddr
);

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
);


/*!
*   ASYNC_Close
*
*   Close the debug session
*
*   @param  ulSessionId     Trustlet debug session ID
*/
tlApiResult_t ASYNC_Close(
    uint32_t ulSessionId,
    uint32_t sessionType,
    uint32_t *buffer
);


/*!
 * Create  and increase  the global  thread counter.
 *
 * @param sessionType type of the session
 * @@param ulSessionId id of the session for listener thread creation
 */

static uint32_t increaseListenerThreadCounter(
		uint32_t sessionType,
		uint32_t ulSessionId
);

/*!
 * Decrease the thread information while shutting down.
 *
 */
static uint32_t decreaseListenerThreadCounter(
		void
);

#ifdef __cplusplus
}
#endif

#endif
