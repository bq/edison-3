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

#include "caSampleRot13.h"

#define LOG_TAG "CASampleRot13_GP"
#include "log.h"

static void rot13(const char *src, char *dst);
static void returnExitCode(int exitCode);

/**
 * Test the ROT13 function provided by the MobiCore environment.
 */
int main(int argc, char *args[])
{

    char plainText[] = "The quick brown fox jumps over the lazy dog";
    char *cipherText;

    __android_log_write(ANDROID_LOG_INFO, LOG_TAG, "Copyright (c) Trustonic Limited 2013");

    if (caOpen() != true) {
        LOG_E("open TL session failed!");
        fprintf(stderr, "Could not open Trustlet session.\n");
        returnExitCode(2);
    }

    printf("Plain text: %s\n", plainText);

    cipherText = caRot13(plainText);

    if (cipherText != NULL) {
        printf("Cipher text: %s\n", cipherText);
    } else {
        fprintf(stderr, "No response from Trustlet.\n");
        returnExitCode(2);
    }

    /////////////////////////////////////////////
    // Check if result matches expected result.
    //
    char *expectedCipherText = (char *)malloc(strlen(plainText) + 1);
    rot13(plainText, expectedCipherText);

    if (strcmp(cipherText, expectedCipherText) != 0) {
        fprintf(stderr, "Comparison failed. Expected: %s\n", expectedCipherText);
        __android_log_write(ANDROID_LOG_INFO, LOG_TAG, "Failure");
        // Let's look into the cipherText to see why comparison failed.
        //  Note that cipherText might not contain printable characters anymore.
        //  This means we also cannot use strlen(cipherText).
        LOG_I_BUF("Ciphertext", cipherText, strlen(plainText));
        LOG_I_BUF("Expected ciphertext", expectedCipherText, strlen(plainText));
        returnExitCode(1);
    }
    else __android_log_write(ANDROID_LOG_INFO, LOG_TAG, "Success");

    // Close CA. Note that this frees the cipherText/TCI pointer. Do not use this pointer after caClose().
    caClose();

    free(expectedCipherText);

    returnExitCode(0);
    return 0;
}

// Rot13 oracle. Used to verify trustlet.
static void rot13(const char *src, char *dst)
{
    const size_t len = strlen(src);
    const int alphabetLen = 'Z' - 'A';

    for (size_t i = 0; i < len; ++i) {
        int c = *src++;
        int d = c;
        if ('a' <= c && c <= 'z') {
            d = c + 13;
            if (d > 'z') {
                d = d - alphabetLen - 1;
            }
        } else if ('A' <= c && c <= 'Z') {
            d = c + 13;
            if (d > 'Z') {
                d = d - alphabetLen - 1;
            }
        }
        *dst++ = d;
    }

    *dst = '\0';
}

static void returnExitCode(int exitCode)
{
    if (0 != exitCode) {
        __android_log_write(ANDROID_LOG_ERROR, LOG_TAG, "Failure");
    } else {
        __android_log_write(ANDROID_LOG_INFO, LOG_TAG, "Success");
    }
    fprintf(stderr, "CA exit code: %08x\n", exitCode);
    exit(exitCode);
}
