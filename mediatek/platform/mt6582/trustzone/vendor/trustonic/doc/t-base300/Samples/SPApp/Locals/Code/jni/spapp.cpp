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
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <jni.h>
#include <errno.h>
#include <sys/wait.h>

#include "log.h"

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

// Service Provider Application (SPApp) JNI example.
EXTERN_C JNIEXPORT jboolean JNICALL
Java_com_trustonic_tbase_android_ota_spapp_TBaseSPApp_foo(JNIEnv *env,
        jobject obj) {
    return true;
}
