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

#include <string.h>
#include <stdint.h>
#include <jni.h>

#include "tlcSampleRot13.h"

#define LOG_TAG "TlcSampleRot13"
#include "log.h"

// See for more help about JNI:
// http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C JNIEXPORT jstring JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRot13_getDefaultPlainText(
        JNIEnv *env,
        jobject obj
        ) {
    const char *plainText = PLAINTEXT;

    LOG_I("default plainText: %s", plainText);

    return env->NewStringUTF(plainText);
}

EXTERN_C JNIEXPORT jstring JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRot13_rot13NWd(
        JNIEnv *env,
        jobject obj,
        jstring plainText
        ) {
    char *cipher;
    char *str;

    str = (char*)env->GetStringUTFChars(plainText, NULL);
    if (str == NULL) {
        return NULL; /* OutOfMemoryError already thrown */
    }
    LOG_I("plainText: %s", str);

    cipher = tlcRot13NWd(str);
    LOG_I("cipherText: %s", cipher);

    env->ReleaseStringUTFChars(plainText, str);

    return env->NewStringUTF(cipher);
}

EXTERN_C JNIEXPORT jint JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRot13_open(
        JNIEnv *env,
        jobject obj,
        jint spid,
        jbyteArray pTAData,
        jint nTASize
        ) {
    jint ret;
    jbyte* data = env->GetByteArrayElements(pTAData, 0);
    LOG_I("%s", __func__);

    ret = tlcOpen(spid, (uint8_t*)data, nTASize);

    env->ReleaseByteArrayElements(pTAData, data, 0);

    return ret;
}

EXTERN_C JNIEXPORT jstring JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRot13_rot13SWd(
        JNIEnv *env,
        jobject obj,
        jstring plainText
        ) {
    char *cipher;
    char *str;

    str = (char*) env->GetStringUTFChars(plainText, NULL);
    if (str == NULL) {
        return NULL; /* OutOfMemoryError already thrown */
    }
    LOG_I("plainText: %s", str);

    cipher = tlcRot13SWd(str);
    LOG_I("cipherText: %s", cipher);

    env->ReleaseStringUTFChars(plainText, str);

    return env->NewStringUTF(cipher);
}

EXTERN_C JNIEXPORT jstring JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRot13_unwrapDataSWd(
        JNIEnv *env,
        jobject obj,
        jbyteArray pData,
        jint nDataSize
        ) {
    jbyte *data = env->GetByteArrayElements(pData, 0);

    char *unwrap = tlcUnwrapDataSWd((uint8_t*)data, nDataSize);
    LOG_I("unwrapDataText: %s", unwrap);

    env->ReleaseByteArrayElements(pData, data, 0);

    return env->NewStringUTF(unwrap);
}

EXTERN_C JNIEXPORT void JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRot13_close(
        JNIEnv *env,
        jobject obj
        ) {
    LOG_I("%s", __func__);

    tlcClose();
}
