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

#include "tlcSampleRsa.h"

#define LOG_TAG "TlcSampleRsa"
#include "log.h"

// See for more help about JNI:
// http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html

#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C
#endif

EXTERN_C JNIEXPORT jint JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRsa_rsa(
	JNIEnv *env,
	jobject obj,
	jbyteArray dstbuf,
    jint dstbuflen,
    jintArray dstlength
) {
	jint ret;
	jbyte* dst = env->GetByteArrayElements(dstbuf, 0);
	jint* len = env->GetIntArrayElements(dstlength, 0);

    LOG_I("%s", __func__);

    *len = dstbuflen;
    ret = rsa((uint8_t*)dst, len);

    env->ReleaseByteArrayElements(dstbuf, dst, 0);
    env->ReleaseIntArrayElements(dstlength, len, 0);

    return ret;
}

EXTERN_C JNIEXPORT jint JNICALL
Java_com_trustonic_tsdk_sample_TlcSampleRsa_open(
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
Java_com_trustonic_tsdk_sample_TlcSampleRsa_close(
	JNIEnv *env,
	jobject obj
) {
	LOG_I("%s", __func__);

	tlcClose();
}
