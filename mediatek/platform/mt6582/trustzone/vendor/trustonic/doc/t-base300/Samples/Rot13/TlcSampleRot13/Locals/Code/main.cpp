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

#include "tlcSampleRot13.h"
#include "log.h"

static void returnExitCode(int exitCode);
static size_t getFileContent(const char* pPath, uint8_t** ppContent);

/**
 * Test the ROT13 function provided by the TBase environment.
 */
int main(int argc, char *args[]) {

	mcResult_t ret;
	char plainText[] = PLAINTEXT;
	char *cipherText;
    char* expectedCipherText;
    uint8_t* pTAData = NULL;
    uint32_t nTASize;

	LOG_i("Copyright (c) Trustonic Limited 2013");

    nTASize = getFileContent(
                        "04010000000000000000000000000000.tlbin",
                        &pTAData);
    if (nTASize == 0) {
        fprintf(stderr, "Trusted Application not found.\n");
		returnExitCode(2);
    }

	ret = tlcOpen(MC_SPID_TRUSTONIC_TEST, pTAData, nTASize);
	if (MC_DRV_OK != ret) {
		LOG_E("open TL session failed!");
        fprintf(stderr, "Could not open Trusted Application session.\n");
        free(pTAData);
		returnExitCode(2);
	}

    printf("Plain text: %s\n", plainText);

	cipherText = tlcRot13SWd(plainText);

    if (cipherText != NULL) {
        printf("Cipher text: %s\n", cipherText);
    } else {
        fprintf(stderr, "No response from Trusted Application.\n");
        free(pTAData);
        returnExitCode(2);
    }

    /////////////////////////////////////////////
    // Check if result matches expected result.
    //
    expectedCipherText = tlcRot13NWd(plainText);

    if (strcmp(cipherText, expectedCipherText) != 0) {
        fprintf(stderr, "Comparison failed. Expected: %s\n", expectedCipherText);
        // Let's look into the cipherText to see why comparison failed.
        //  Note that cipherText might not contain printable characters anymore.
        //  This means we also cannot use strlen(cipherText).
        LOG_I_BUF("Ciphertext", cipherText, strlen(plainText));
        LOG_I_BUF("Expected ciphertext", expectedCipherText, strlen(plainText));
        free(expectedCipherText);
        free(pTAData);
        returnExitCode(1);
    }

    // Close tlc. Note that this frees the cipherText/TCI pointer. Do not use this pointer after tlcClose().
    tlcClose();

    free(pTAData);
    free(expectedCipherText);

    returnExitCode(0);
	return 0;
}

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

static void returnExitCode(int exitCode) {
    if (0 != exitCode) {
        LOG_e("Failure");
    }
    else {
        LOG_i("Success");
    }
    fprintf(stderr, "TLC exit code: %08x\n", exitCode);
    exit(exitCode);
}
