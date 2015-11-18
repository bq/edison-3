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
#include <string.h>

#include "tlcSampleAes.h"

#define LOG_TAG "TLC AES"
#include "log.h"

// -------------------------------------------------------------
static size_t getFileContent(
    const char* pPath,
    uint8_t** ppContent)
{
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

   /*
    * The stat function is not used (not available in WinCE).
    */

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

int main(int argc, char *argv[])
{
    bool    wrapData = false;
    int     srcDataLength  = 16;
    int     dstDataLength  = 0;
    uint8_t *srcData  = NULL;
    uint8_t *dstData = NULL;
    mcResult_t ret;
    uint8_t* pTAData = NULL;
    uint32_t nTASize;
	int i;

    LOG_i("Copyright (c) Trustonic Limited 2013");

    LOG_I("AES TLC called");

	for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            i++;
            if (i == argc)
            {
                LOG_E("Error: Missing value for %s parameter\n", argv[i-1]);
                return 1;
            }
            if (sscanf(argv[i], "%i", &srcDataLength) != 1)
            {
                LOG_E("Error: Invalid value for %s parameter\n", argv[i-1]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            i++;
            if (i == argc)
            {
                LOG_E("Error: Missing value for %s parameter\n", argv[i-1]);
                return 1;
            }
            if (sscanf(argv[i], "%i", &srcDataLength) != 1)
            {
                LOG_E("Error: Invalid value for %s parameter\n", argv[i-1]);
                return 1;
            }
            wrapData = true;
        }
        else
        {
            LOG_E("Unknown option");
            return 1;
        }
    }

    nTASize = getFileContent(
                        "07020000000000000000000000000000.tlbin",
                        &pTAData);
    if (nTASize == 0) {
        LOG_E("Trusted Application not found");
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }

    ret = tlcOpen(pTAData, nTASize);

    free(pTAData);

    if (MC_DRV_OK != ret) {
        LOG_E("tlcOpen failed: %d", ret);
        return ret;
    }

    srcData = (uint8_t *)malloc(srcDataLength);
    if (srcData == NULL) {
        ret = MC_DRV_ERR_NO_FREE_MEMORY;
        LOG_E("Allocation of source buffer failed: %d", ret);
        goto exit;
    }
    memset(srcData, 0xab, srcDataLength);

    dstDataLength = srcDataLength + 512;
    dstData = (uint8_t *)malloc(dstDataLength);
    if (dstData == NULL) {
        ret = MC_DRV_ERR_NO_FREE_MEMORY;
        LOG_E("Allocation of destination buffer failed: %d", ret);
        goto exit;
    }
    memset(dstData, 0x00, dstDataLength);

    if (wrapData == false) {
        // AES encrypt/decrypt operation
        ret = aes(srcData, srcDataLength, dstData, dstDataLength);
    } else {
        // Secure object wrap/unwrap operation
        ret = secureObject(srcData, srcDataLength, dstData, dstDataLength);
    }

exit:
    if (srcData != NULL) {
        free(srcData);
    }
    if (dstData != NULL) {
        free(dstData);
    }
    if (MC_DRV_OK != ret) {
        LOG_e("Failure");
    }
    else {
        LOG_i("Success");
    }
    tlcClose();
    return ret;
}
