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

#include "tlcSampleAes.h"

#define LOG_TAG "TlcSampleAes"
#include "log.h"

// See for more help about JNI:
// http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html

#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C
#endif

EXTERN_C JNIEXPORT jint JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleAes_secureObject(
	JNIEnv *env,
	jobject obj,
	jbyteArray srcbuf,
    jint srcbuflen,
    jbyteArray dstbuf,
    jint dstbuflen
) {
	jint ret;
	jbyte* src = env->GetByteArrayElements(srcbuf, 0);
	jbyte* dst = env->GetByteArrayElements(dstbuf, 0);

    LOG_I("%s", __func__);

    ret = secureObject((uint8_t*)src, srcbuflen, (uint8_t*)dst, dstbuflen);

    env->ReleaseByteArrayElements(srcbuf, src, 0);
    env->ReleaseByteArrayElements(dstbuf, dst, 0);

    return ret;
}

EXTERN_C JNIEXPORT jint JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleAes_aes(
	JNIEnv *env,
	jobject obj,
	jbyteArray srcbuf,
    jint srcbuflen,
    jbyteArray dstbuf,
    jint dstbuflen
) {
	jint ret;
	jbyte* src = env->GetByteArrayElements(srcbuf, 0);
	jbyte* dst = env->GetByteArrayElements(dstbuf, 0);

    LOG_I("%s", __func__);

    ret = aes((uint8_t*)src, srcbuflen, (uint8_t*)dst, dstbuflen);

    env->ReleaseByteArrayElements(srcbuf, src, 0);
    env->ReleaseByteArrayElements(dstbuf, dst, 0);

    return ret;
}

EXTERN_C JNIEXPORT jint JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleAes_open(
	JNIEnv *env,
	jobject obj,
	jbyteArray pTAData,
    jint nTASize
) {
	jint ret;
	jbyte* data = env->GetByteArrayElements(pTAData, 0);
	LOG_I("%s", __func__);

	ret = tlcOpen((uint8_t*)data, nTASize);

	env->ReleaseByteArrayElements(pTAData, data, 0);

	return ret;
}

EXTERN_C JNIEXPORT void JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleAes_close(
	JNIEnv *env,
	jobject obj
) {
	LOG_I("%s", __func__);

	tlcClose();
}
