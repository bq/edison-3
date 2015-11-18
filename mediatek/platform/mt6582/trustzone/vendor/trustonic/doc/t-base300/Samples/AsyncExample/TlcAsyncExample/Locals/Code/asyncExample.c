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

/*
 * @file   asyncExample.c
 * @brief  Async trustlet connector main file
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>


#include "MobiCoreDriverApi.h"
#include "tlAsyncExample_Api.h"


#define LOG_TAG "TlcAsyncExample"
#include "log.h"


/* error codes */
typedef enum
{
    ASYNC_EXAMPLE_ERR_NONE          = 0,
    ASYNC_EXAMPLE_NOTIFY,
    ASYNC_EXAMPLE_ERR_MC_SESSION,
    // more can be added as required
} ASYNC_EXAMPLE_ERR;


static mcSessionHandle_t   gSessionHandle;
static pthread_t           gListenerThread;
static pthread_mutex_t     gMutexLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t      gThreadCond = PTHREAD_COND_INITIALIZER;


/**
 * getFileContent is used to read trustlet binary data and put it to buffer
 */
static size_t getFileContent(
    const char* pPath,
    uint8_t** ppContent)
{
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

   /* Open the file */
   pStream = fopen(pPath, "rb");
   if (pStream == NULL)
   {
      fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
      return 0;
   }

   if (fseek(pStream, 0L, SEEK_END) != 0)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   filesize = ftell(pStream);
   if (filesize < 0)
   {
      fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
      goto error;
   }

   if (filesize == 0)
   {
      fprintf(stderr, "Error: Empty file: %s.\n", pPath);
      goto error;
   }

   /* Set the file pointer at the beginning of the file */
   if (fseek(pStream, 0L, SEEK_SET) != 0)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   /* Allocate a buffer for the content */
   content = (uint8_t*)malloc(filesize);
   if (content == NULL)
   {
      fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
      goto error;
   }

   /* Read data from the file into the buffer */
   if (fread(content, (size_t)filesize, 1, pStream) != 1)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   /* Close the file */
   fclose(pStream);
   *ppContent = content;

   /* Return number of bytes read */
   return (size_t)filesize;

error:
   if (content  != NULL)
   {
       free(content);
   }
   fclose(pStream);
   return 0;

}

/**
 *   readThread reads notifications from trustlet
 */
static void readThread(void *param)
{
    tciMessage_ptr  pTci= (tciMessage_ptr)param;
    uint32_t        irqCountLocal, tmp;

    //LOG_I("readThread(): started \n");
    irqCountLocal =  pTci->response.irqCount;

    for(;;)
    {
        LOG_I("readThread(): Wait response for command\n");
        do {
            /* Wait for notification from Swd */
            if (MC_DRV_OK != mcWaitNotification(&gSessionHandle, MC_INFINITE_TIMEOUT)) {
                LOG_E("readThread(): mcWait failed\n");
                return;
            }

            // pTci->response.irqCount can be increased in any time by trustlet
            // so it must not been change here.
            tmp = pTci->response.irqCount;
        } while (tmp == irqCountLocal);

        LOG_I("readThread(): writing OK, count of notification from trustlet: %x\n",tmp - irqCountLocal);
        irqCountLocal= tmp;

        pthread_mutex_lock(&gMutexLock);
        /* Signal the write thread */
        pthread_cond_signal(&gThreadCond);

        pthread_mutex_unlock(&gMutexLock);
    }
}

/**
 *   notificationListener creates a listening thread
 */
static ASYNC_EXAMPLE_ERR notificationListener(tciMessage_ptr pTci)
{
    ASYNC_EXAMPLE_ERR  ret = ASYNC_EXAMPLE_ERR_NONE;
    uint32_t     res;

    //LOG_I("notificationListener(): Creating Read Thread\n");

    res = pthread_create(&gListenerThread,
                         NULL,
                         (void *) &readThread,
                         (void *) pTci);
    if (0 != res)
    {
        LOG_E("notificationListener(): Cannot create Read thread (%d)\n", res);
        ret = ASYNC_EXAMPLE_NOTIFY;
    }

    return ret;
}

/**
 * initSession is used to open session to trustlet
 */
int initSession(tciMessage_ptr *pTci)
{
    uint32_t            deviceId = MC_DEVICE_ID_DEFAULT;
    mcResult_t          mcRet;
    uint8_t*            pTrustletData = NULL;
    uint32_t            nTrustletSize;

    /* Initialize session handle data */
    bzero(&gSessionHandle, sizeof(mcSessionHandle_t));

    /* Open <t-base device */
    LOG_I("Opening <t-base device");
    mcRet = mcOpenDevice(deviceId);
    if (MC_DRV_OK != mcRet)
    {
        LOG_E("mcOpenDevice returned: %d\n", mcRet);
        return -1;
    }

    /* Allocating WSM for TCI */
    *pTci = (tciMessage_t*)malloc(sizeof(tciMessage_t));
    if (*pTci == NULL) {
        LOG_E("Allocation of TCI failed");
        mcCloseDevice(deviceId);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(*pTci, 0, sizeof(tciMessage_t));

    nTrustletSize = getFileContent("07080000000000000000000000000000.tlbin", &pTrustletData);
    if (nTrustletSize == 0) {
        LOG_E("Trustlet not found");
        free(*pTci);
        *pTci = NULL;
        mcCloseDevice(deviceId);
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }

    LOG_I("Opening the session");
    memset(&gSessionHandle, 0, sizeof(gSessionHandle));
    gSessionHandle.deviceId = deviceId; // The device ID (default device is used)
    mcRet = mcOpenTrustlet(
            &gSessionHandle,
            MC_SPID_RESERVED_TEST, /* mcSpid_t */
            pTrustletData,
            nTrustletSize,
            (uint8_t *) *pTci,
            sizeof(tciMessage_t));

    // Whatever the result is, free the buffer
    free(pTrustletData);

    if (MC_DRV_OK != mcRet) {
        LOG_E("Open session failed: %d", mcRet);
        free(*pTci);
        *pTci = NULL;
        mcCloseDevice(deviceId);
        return -1;
    }
    else {
        LOG_I("open() succeeded");
    }

    return 0;

}

/**
 * main function
 */
int main(int argc, char *args[])
{
    tciMessage_ptr      pTci   = NULL;
    mcResult_t          mcRet;
    int                 i, num;

    if (0 != initSession(&pTci)) {
        return 0;
    }

    LOG_I("Start Read thread");
    if (ASYNC_EXAMPLE_ERR_NONE != notificationListener(pTci)){
        LOG_E("Read thread start failed\n");
    }

    num=5;
    if (argc>1){
        num = atoi(args[1]);
    }

    printf("We are running timer %d times\n", num);
    for (i=0; i<num; i++) {
        printf("Run timer, cycle %d ...", i);
        pTci->command.header.commandId = CMD_ID_RUN_ASYNC;

        /* Notify the trustlet */
        mcRet = mcNotify(&gSessionHandle);
        if (MC_DRV_OK != mcRet)
        {
            LOG_E("mcNotify returned: %d\n", mcRet);
            break;
        }

        //wait until writeOK
        if (ASYNC_EXAMPLE_ERR_NONE != pthread_cond_wait(&gThreadCond, &gMutexLock))
        {
            LOG_E("pthread_cond_wait failed\n");
            break;
        }
        printf("The timer has elapsed - the notification from driver\n");
    }
    return 0;
}
