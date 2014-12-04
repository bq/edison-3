
#include <utils/String16.h>
#include "AudioMTKSpecificStreamManager.h"
#include "AudioUtility.h"
#include "AudioMTKStreamOut.h"
#include "AudioMTKHdmiStreamOut.h"

#define LOG_TAG "AudioMTKSpecificStreamManager"
#ifndef ANDROID_DEFAULT_CODE
#include <cutils/xlog.h>
#ifdef ALOGE
#undef ALOGE
#endif
#ifdef ALOGW
#undef ALOGW
#endif
#ifdef ALOGI
#undef ALOGI
#endif
#ifdef ALOGD
#undef ALOGD
#endif
#ifdef ALOGV
#undef ALOGV
#endif
#define ALOGE XLOGE
#define ALOGW XLOGW
#define ALOGI XLOGI
#define ALOGD XLOGD
#define ALOGV XLOGV
#else
#include <utils/Log.h>
#endif

#define STREAM_VECTOR_LOCK_TIMEOUT_IN_MS 3000


namespace android
{

AudioMTKSpecificStreamManager *AudioMTKSpecificStreamManager::createInstance()
{
    return new AudioMTKSpecificStreamManager();
}

AudioMTKSpecificStreamManager::AudioMTKSpecificStreamManager()
{
}

android_audio_legacy::AudioStreamOut *AudioMTKSpecificStreamManager::openOutputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    uint32_t output_flag)
{
    ALOGD("+%s devices = 0x%x format = 0x%x channels = 0x%x rate = %d\n", __FUNCTION__, devices, *format, *channels, *sampleRate);
    android_audio_legacy::AudioStreamOut *out = NULL;
    if (devices == AUDIO_DEVICE_OUT_AUX_DIGITAL)
    {
        out = AudioMTKHdmiStreamOut::createAudioStreamOut(devices, format, channels, sampleRate, status, output_flag);
        enableVectorLock(STREAM_VECTOR_LOCK_TIMEOUT_IN_MS);
        if (out)
        {
            mHdmiStreamOutVector.add((unsigned int)out, out);
        }
        disableVectorLock();
    }
    else
    {
        out = AudioMTKStreamManager::openOutputStream(devices, format, channels, sampleRate, status, output_flag);
    }
    ALOGD("-%s out = %p\n", __FUNCTION__, out);
    return out;
}

status_t AudioMTKSpecificStreamManager::closeOutputStream(android_audio_legacy::AudioStreamOut *out)
{
    ALOGD("%s out = %p\n", __FUNCTION__, out);
    int index = mHdmiStreamOutVector.indexOfKey((unsigned int)out);
    if (index >= 0)
    {
        enableVectorLock(STREAM_VECTOR_LOCK_TIMEOUT_IN_MS);
        mHdmiStreamOutVector.removeItem((unsigned int)out);
        disableVectorLock();
        AudioMTKHdmiStreamOut::destroyAudioStreamOut(out);
        return NO_ERROR;
    }
    else
    {
        return AudioMTKStreamManager::closeOutputStream(out);
    }
}

status_t AudioMTKSpecificStreamManager::ForceAllStandby()
{
    ALOGD("+%s\n", __FUNCTION__);
    status_t ret = AudioMTKStreamManager::ForceAllStandby();
    enableVectorLock(STREAM_VECTOR_LOCK_TIMEOUT_IN_MS);
    if (mHdmiStreamOutVector.size())
    {
        for (size_t i = 0; i < mHdmiStreamOutVector.size() ; i++)
        {
            AudioMTKHdmiStreamOut *pStreamOut = (AudioMTKHdmiStreamOut *)mHdmiStreamOutVector.valueAt(i);
            if (pStreamOut)
            {
                pStreamOut->forceStandby();
            }
        }
    }
    disableVectorLock();
    ALOGD("-%s\n", __FUNCTION__);
    return ret;
}

status_t AudioMTKSpecificStreamManager::SetOutputStreamSuspend(bool bEnable)
{
    ALOGD("+%s\n", __FUNCTION__);
    status_t ret = AudioMTKStreamManager::SetOutputStreamSuspend(bEnable);
    enableVectorLock(STREAM_VECTOR_LOCK_TIMEOUT_IN_MS);
    if (mHdmiStreamOutVector.size())
    {
        for (size_t i = 0; i < mHdmiStreamOutVector.size() ; i++)
        {
            AudioMTKHdmiStreamOut *pStreamOut = (AudioMTKHdmiStreamOut *)mHdmiStreamOutVector.valueAt(i);
            if (pStreamOut)
            {
                pStreamOut->standby();
            }
        }
    }
    disableVectorLock();
    ALOGD("-%s\n", __FUNCTION__);
    return ret;
}

status_t AudioMTKSpecificStreamManager::enableVectorLock(int timeout)
{
    status_t ret = 0;
    if (timeout != 0)
    {
        ret = mStreamOutVectorLock.lock_timeout(timeout);
        if (ret)
        {
            ALOGW("%s lock %d ms timeout\n", __FUNCTION__, timeout);
        }
    }
    else
    {
        ret = mStreamOutVectorLock.lock();
    }
    return ret;
}

void AudioMTKSpecificStreamManager::disableVectorLock(void)
{
    mStreamOutVectorLock.unlock();
}

extern "C" AudioMTKStreamManager *createSpecificStreamManager()
{
    return AudioMTKSpecificStreamManager::createInstance();
}

}

