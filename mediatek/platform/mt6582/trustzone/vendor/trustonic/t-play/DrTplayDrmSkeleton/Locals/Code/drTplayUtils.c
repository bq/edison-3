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

#include "drTplayCommon.h"
#include "drTplayUtils.h"



/**
 * Exchange registers of current thread or another thread.
 *
 * @param  threadNo   Thread no
 * @param  ip         ip value to be set
 * @param  sp         sp value to be set
 *
 * @retval E_OK or relevant error code.
 */
drApiResult_t drUtilsRestartThread(
    threadno_t threadNo,
    addr_t ip,
    addr_t sp )
{
    drApiResult_t ret  = E_INVALID;
    uint32_t      ctrl = THREAD_EX_REGS_IP | THREAD_EX_REGS_SP;

    /* Set ip and sp registers */
    ret = drApiThreadExRegs(threadNo,
                            ctrl,
                            ip,
                            sp);
    if (ret != E_OK)
    {
        return ret;
    }

    /* Resume thread */
    ret = drApiResumeThread(threadNo);

    return ret;
}


/**
 * Makes request to RTM for updating notification thread
 *
 * @param  threadNo   Thread no
 *
 * @retval E_OK or relevant error code.
 */
drApiResult_t drUtilsUpdateNotificationThread( threadno_t threadNo )
{
    drApiResult_t ret =  E_INVALID;
    threadid_t  notificationThread;

    /* Retrieve task id */
    taskid_t task = drApiGetTaskid();
    if (task == NILTASK)
    {
        return ret;
    }

    /* Retrieve thread id based on task id and thread no */
    notificationThread = drApiTaskidGetThreadid(task, threadNo);

    /* IPC data for setting notification handler */
    threadid_t  ipcClient = notificationThread;
    message_t   ipcMsg    = MSG_SET_NOTIFICATION_HANDLER;
    uint32_t    ipcData   = 0;

    /* Make IPC call to IPCH and wait for a response */
    ret = drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData);

    return ret;
}

