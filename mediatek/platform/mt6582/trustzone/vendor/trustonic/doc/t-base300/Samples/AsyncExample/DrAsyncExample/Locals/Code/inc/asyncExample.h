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


/**
 * @file   asyncExample.h
 * @brief  Contains common definitions and data structures
 *
 */


#ifndef ASYNCEXAMPLE_H_
#define ASYNCSAMPLE_H_

typedef enum  {
    TEST_NONE = 0,   // No tests
    TEST_RUNNING,
    } asyncExample_t;


typedef struct {
    uint32_t countOfNotifications; // How many notification are required to send
    uint32_t intervalOfNotifications; // How often notification will be send
} testParams_t;



typedef struct {
    threadid_t   client;
    uint32_t     rCounter;          //How many notifications was told in  previous query
    volatile uint32_t     wCounter; //How many notifications are send totally
} testSession_t, *testSession_ptr;

#endif /* ASYNCSAMPLE_H_ */
