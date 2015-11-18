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

#ifndef __DRDRMUTILS_H__
#define __DRDRMUTILS_H__

#include "DrApi/DrApiThread.h"
#include "DrApi/DrApiMm.h"

#include "drTplayCommon.h"



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
    addr_t sp );
    

/**
 * Makes request to RTM for updating notification thread
 *
 * @param  threadNo   Thread no
 *
 * @retval E_OK or relevant error code.
 */
drApiResult_t drUtilsUpdateNotificationThread( threadno_t threadNo );


#endif // __DRDRMUTILS_H__
