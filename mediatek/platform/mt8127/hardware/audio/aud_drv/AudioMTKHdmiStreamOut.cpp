#include "AudioMTKHdmiStreamOut.h"
#include "AudioResourceFactory.h"
#include "AudioResourceManagerInterface.h"
#include "AudioIoctl.h"
#include "AudioDigitalType.h"
#include "audio_custom_exp.h"
#ifdef MTK_INTERNAL_HDMI_SUPPORT
#include "hdmiedid.h"
#include "hdmitable.h"
#elif MTK_INTERNAL_MHL_SUPPORT
#include "mhl_edid.h"
#include "mhl_table.h"
#endif

#define LOG_TAG  "AudioMTKHdmiStreamOut"
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

#define HDMI_BASE_BUFFER_SIZE 8192
#define ENUM_TO_STRING(enum) #enum
#define CAP_TO_STR(cap) (((cap)!=0)?"Y":"N")
#define MAX_DUMP_SIZE_IN_BYTES 200000000
#define MAX_DUMP_FILE_NUM 5
#define HDMI_DEVICE_NAME "/dev/hdmitx"

static const char *streamout_dump_path = "/sdcard/mtklog/audio_dump";
static const char *streamout_hdmi_property = "streamout.hdmi.dump";

namespace android
{

AudioMTKHdmiStreamOut *AudioMTKHdmiStreamOut::mStreamOuts[] = {0};
int AudioMTKHdmiStreamOut::mDumpFileNum[] = {0};
AudioLock AudioMTKHdmiStreamOut::mStreamOutsLock;
uint32_t AudioMTKHdmiStreamOut::mHdmiSinkSupportCodec = 0;
KeyedVector<uint32_t, uint32_t> AudioMTKHdmiStreamOut::mHdmiSinkPcmSupportProfiles;

android_audio_legacy::AudioStreamOut *AudioMTKHdmiStreamOut::createAudioStreamOut(
    uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status, uint32_t output_flag)
{
    ALOGD("%s\n", __FUNCTION__);
    AudioMTKHdmiStreamOut::enableStreamOutsLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    HDMI_OUTPUT_TYPE output_type = outputFlagToOutputType(output_flag);

    if (mStreamOuts[output_type])
    {
        ALOGW("%s hdmi[%d] stream existed", __FUNCTION__, output_type);
        *status = ALREADY_EXISTS;
        AudioMTKHdmiStreamOut::disableStreamOutsLock();
        return NULL;
    }

    if (!hasValidStreamOut())
    {
        queryEdidInfo();
    }

    //  handle default parameter
    if (*format == 0)
    {
        *format = AUDIO_FORMAT_PCM_16_BIT;
    }
    if (*sampleRate == 0)
    {
        *sampleRate = 44100;
    }
    if (*channels == 0)
    {
        *channels = (HDMI_MULTI_CH == output_type) ? AUDIO_CHANNEL_OUT_5POINT1 : AUDIO_CHANNEL_OUT_STEREO;
    }

    // check bitwidth
    if (*format != AUDIO_FORMAT_PCM_16_BIT)
    {
        ALOGW("%s unsupported bitwidth %x\n", __FUNCTION__, *format);
        *status = BAD_VALUE;
        AudioMTKHdmiStreamOut::disableStreamOutsLock();
        return NULL;
    }

    // check sink capability
    if (!isSinkSupportedFormat(HDMI_SINK_AUDIO_DEC_LPCM, *sampleRate, channelMaskTChannelCount(*channels)))
    {
        ALOGW("%s hdmi[%d] unsupported format by sink\n", __FUNCTION__, output_type);
        *status = BAD_VALUE;
        AudioMTKHdmiStreamOut::disableStreamOutsLock();
        return NULL;
    }

    AudioMTKHdmiStreamOut *stream_out = new AudioMTKHdmiStreamOut(devices, format, channels,
                                                                  sampleRate, status, output_flag);
    mStreamOuts[output_type] = stream_out;
    AudioMTKHdmiStreamOut::disableStreamOutsLock();
    return stream_out;
}

void AudioMTKHdmiStreamOut::destroyAudioStreamOut(android_audio_legacy::AudioStreamOut *stream_out)
{
    AudioMTKHdmiStreamOut::enableStreamOutsLock(3000);
    if (stream_out)
    {
        for (int i = 0; i < HDMI_OUTPUT_CNT; i++)
        {
            if (stream_out == (android_audio_legacy::AudioStreamOut *)mStreamOuts[i])
            {
                mStreamOuts[i] = NULL;
                break;
            }
        }
        delete stream_out;
    }
    AudioMTKHdmiStreamOut::disableStreamOutsLock();
}

status_t AudioMTKHdmiStreamOut::enableStreamOutsLock(int timeout)
{
    status_t ret = 0;
    if (timeout != 0)
    {
        ret = mStreamOutsLock.lock_timeout(timeout);
        if (ret)
        {
            ALOGW("%s lock %d ms timeout\n", __FUNCTION__, timeout);
        }
    }
    else
    {
        ret = mStreamOutsLock.lock();
    }
    return ret;
}

void AudioMTKHdmiStreamOut::disableStreamOutsLock(void)
{
    mStreamOutsLock.unlock();
}

AudioMTKHdmiStreamOut::HDMI_OUTPUT_TYPE AudioMTKHdmiStreamOut::outputFlagToOutputType(uint32_t output_flag)
{
    return (output_flag & AUDIO_OUTPUT_FLAG_DIRECT) ? HDMI_MULTI_CH : HDMI_STEREO;
}

int AudioMTKHdmiStreamOut::sampleRateToHdmiFsType(uint32_t SampleRate)
{
    switch (SampleRate)
    {
        case 192000:
            return AudioHdmiType::HDMI_FS_192K;
        case 176400:
            return AudioHdmiType::HDMI_FS_176_4K;
        case 96000:
            return AudioHdmiType::HDMI_FS_96K;
        case 88200:
            return AudioHdmiType::HDMI_FS_88_2K;
        case 48000:
            return AudioHdmiType::HDMI_FS_48K;
        case 44100:
            return AudioHdmiType::HDMI_FS_44_1K;
        case 32000:
            return AudioHdmiType::HDMI_FS_32K;
        default:
            return AudioHdmiType::HDMI_FS_48K;
            break;
    }
}

int AudioMTKHdmiStreamOut::sampleRateToSinkSupportType(uint32_t SampleRate)
{
    switch (SampleRate)
    {
        case 192000:
            return SINK_AUDIO_192k;
        case 176400:
            return SINK_AUDIO_176k;
        case 96000:
            return SINK_AUDIO_96k;
        case 88200:
            return SINK_AUDIO_88k;
        case 48000:
            return SINK_AUDIO_48k;
        case 44100:
            return SINK_AUDIO_44k;
        case 32000:
            return SINK_AUDIO_32k;
        default:
            return 0;
            break;
    }
}

int AudioMTKHdmiStreamOut::channelMaskTChannelCount(uint32_t ChannelMask)
{
    return popcount(ChannelMask);
}

bool AudioMTKHdmiStreamOut::hasValidStreamOut(void)
{
    for (int i = 0; i < HDMI_OUTPUT_CNT; i++)
    {
        if (mStreamOuts[i] != NULL)
        {
            return true;
        }
    }
    return false;
}

void AudioMTKHdmiStreamOut::queryEdidInfo(void)
{
    ALOGD("+%s\n", __FUNCTION__);

    clearEdidInfo();

    int fd = open(HDMI_DEVICE_NAME, O_RDONLY);
    if (fd == -1)
    {
        ALOGE("%s open %s fail errno = %d\n", __FUNCTION__, HDMI_DEVICE_NAME, errno);
        return;
    }

    HDMI_EDID_T hdmi_edid;
    memset(&hdmi_edid, 0, sizeof(HDMI_EDID_T));

    int ret = ioctl(fd, MTK_HDMI_GET_EDID, &hdmi_edid);
    if (ret == 0)
    {
        mHdmiSinkSupportCodec = hdmi_edid.ui2_sink_aud_dec;
        // PCM 2ch capability
        unsigned int pcm_cap = hdmi_edid.ui4_hdmi_pcm_ch_type & 0XFF;
        if (pcm_cap)
        {
            mHdmiSinkPcmSupportProfiles.add(2, pcm_cap);
        }
        // PCM 6ch capability
        pcm_cap = (hdmi_edid.ui4_hdmi_pcm_ch_type >> 8) & 0XFF;
        if (pcm_cap)
        {
            mHdmiSinkPcmSupportProfiles.add(6, pcm_cap);
        }
        // PCM 8ch capability
        pcm_cap = (hdmi_edid.ui4_hdmi_pcm_ch_type >> 16) & 0XFF;
        if (pcm_cap)
        {
            mHdmiSinkPcmSupportProfiles.add(8, pcm_cap);
        }
        // PCM 3ch capability
        pcm_cap = hdmi_edid.ui4_hdmi_pcm_ch3ch4ch5ch7_type & 0XFF;
        if (pcm_cap)
        {
            mHdmiSinkPcmSupportProfiles.add(3, pcm_cap);
        }
        // PCM 4ch capability
        pcm_cap = (hdmi_edid.ui4_hdmi_pcm_ch3ch4ch5ch7_type >> 8) & 0XFF;
        if (pcm_cap)
        {
            mHdmiSinkPcmSupportProfiles.add(4, pcm_cap);
        }
        // PCM 5ch capability
        pcm_cap = (hdmi_edid.ui4_hdmi_pcm_ch3ch4ch5ch7_type >> 16) & 0XFF;
        if (pcm_cap)
        {
            mHdmiSinkPcmSupportProfiles.add(5, pcm_cap);
        }
        // PCM 7ch capability
        pcm_cap = (hdmi_edid.ui4_hdmi_pcm_ch3ch4ch5ch7_type >> 24) & 0XFF;
        if (pcm_cap)
        {
            mHdmiSinkPcmSupportProfiles.add(7, pcm_cap);
        }
        dumpSinkCapability();
    }
    else
    {
        ALOGE("%s ioctl MTK_HDMI_GET_EDID fail errno = %d\n", __FUNCTION__, errno);
    }

    close(fd);
    ALOGD("-%s\n", __FUNCTION__);
}

void AudioMTKHdmiStreamOut::clearEdidInfo(void)
{
    mHdmiSinkSupportCodec = 0;
    mHdmiSinkPcmSupportProfiles.clear();
}

bool AudioMTKHdmiStreamOut::isSinkSupportedFormat(uint32_t CodecFormat, uint32_t SampleRate, uint32_t ChannelCount)
{
    ALOGV("+%s codec = %x sample rate = %d channel ount = %d\n", __FUNCTION__, CodecFormat, SampleRate, ChannelCount);
    if (!(CodecFormat & mHdmiSinkSupportCodec))
    {
        ALOGW("%s unsupported codec %x\n", __FUNCTION__, CodecFormat);
        return false;
    }
    if (CodecFormat == HDMI_SINK_AUDIO_DEC_LPCM)
    {
        ssize_t index = mHdmiSinkPcmSupportProfiles.indexOfKey(ChannelCount);
        if (index >= 0)
        {
            uint32_t SupportedSampleRates = mHdmiSinkPcmSupportProfiles.valueAt(index);
            if (!(SupportedSampleRates & sampleRateToSinkSupportType(SampleRate)))
            {
                ALOGW("%s unsupported sample rate %d\n", __FUNCTION__, SampleRate);
                return false;
            }
        }
        else
        {
            ALOGW("%s unsupported channel %d\n", __FUNCTION__, ChannelCount);
            return false;
        }
    }
    ALOGV("-%s\n", __FUNCTION__);
    return true;
}

void AudioMTKHdmiStreamOut::dumpSinkCapability()
{
    // dump codec capability
    ALOGD("Sink LPCM(%s) AC3(%s) AAC(%s) DTS(%s) DDPlus(%s) DTS-HD(%s) DOLBY TRUEHD(%s)\n",
          CAP_TO_STR(HDMI_SINK_AUDIO_DEC_LPCM & mHdmiSinkSupportCodec),
          CAP_TO_STR(HDMI_SINK_AUDIO_DEC_AC3 & mHdmiSinkSupportCodec),
          CAP_TO_STR(HDMI_SINK_AUDIO_DEC_AAC & mHdmiSinkSupportCodec),
          CAP_TO_STR(HDMI_SINK_AUDIO_DEC_DTS & mHdmiSinkSupportCodec),
          CAP_TO_STR(HDMI_SINK_AUDIO_DEC_DOLBY_PLUS & mHdmiSinkSupportCodec),
          CAP_TO_STR(HDMI_SINK_AUDIO_DEC_DTS_HD & mHdmiSinkSupportCodec),
          CAP_TO_STR(HDMI_SINK_AUDIO_DEC_MAT_MLP & mHdmiSinkSupportCodec));

    // dump PCM capability
    for (int i = 2; i <= 8; i++)
    {
        uint32_t SupportedSampleRates = 0;
        ssize_t index = mHdmiSinkPcmSupportProfiles.indexOfKey(i);
        if (index >= 0)
        {
            SupportedSampleRates = mHdmiSinkPcmSupportProfiles.valueAt(index);
        }
        ALOGD("Sink PCM %dch: 192k(%s) 176k(%s) 96k(%s) 88k(%s) 48k(%s) 44k(%s) 32k(%s)\n", i,
              CAP_TO_STR(SINK_AUDIO_192k & SupportedSampleRates),
              CAP_TO_STR(SINK_AUDIO_176k & SupportedSampleRates),
              CAP_TO_STR(SINK_AUDIO_96k & SupportedSampleRates),
              CAP_TO_STR(SINK_AUDIO_88k & SupportedSampleRates),
              CAP_TO_STR(SINK_AUDIO_48k & SupportedSampleRates),
              CAP_TO_STR(SINK_AUDIO_44k & SupportedSampleRates),
              CAP_TO_STR(SINK_AUDIO_32k & SupportedSampleRates));
    }
}

AudioMTKHdmiStreamOut::AudioMTKHdmiStreamOut(uint32_t devices, int *format, uint32_t *channels,
                                             uint32_t *sampleRate, status_t *status, uint32_t output_flag)
    : mAudioResourceManager(AudioResourceManagerFactory::CreateAudioResource())
    , mAudioDigitalControl(NULL)
    , mAudioHdmiControl(NULL)
    , mStarting(false)
    , mExternalSuspend(0)
    , mInternalSuspend(false)
    , mOutputType(outputFlagToOutputType(output_flag))
    , mFd(::open(kAudioDeviceName, O_RDWR))
    , mHdmiFsType(sampleRateToHdmiFsType((sampleRate ? *sampleRate : 0)))
    , mPcmDumpFile(NULL)
    , mPresentedBytes(0)
{
    ALOGD("+%s devices = %x format = %x channels = %x sampleRate = %d output_flag = %x",
          __FUNCTION__, devices, *format, *channels, *sampleRate, output_flag);

    mChannelMask = *channels;
    mDLAttribute.mChannels = channelMaskTChannelCount(mChannelMask);
    mDLAttribute.mSampleRate = *sampleRate;
    mDLAttribute.mFormat = *format;

    // prepare supported channel
    if (mOutputType == HDMI_MULTI_CH)
    {
        if (isSinkSupportedFormat(HDMI_SINK_AUDIO_DEC_LPCM, mDLAttribute.mSampleRate, 6))
        {
            mSupportedChannelMasks.add(String8(ENUM_TO_STRING(AUDIO_CHANNEL_OUT_5POINT1)));
        }
        if (isSinkSupportedFormat(HDMI_SINK_AUDIO_DEC_LPCM, mDLAttribute.mSampleRate, 8))
        {
            mSupportedChannelMasks.add(String8(ENUM_TO_STRING(AUDIO_CHANNEL_OUT_7POINT1)));
        }
        mHdmiMemType = AudioHdmiType::HDMI_MEM_MULTI_CH_PCM;
    }
    else
    {
        if (isSinkSupportedFormat(HDMI_SINK_AUDIO_DEC_LPCM, mDLAttribute.mSampleRate, 2))
        {
            mSupportedChannelMasks.add(String8(ENUM_TO_STRING(AUDIO_CHANNEL_OUT_STEREO)));
        }
        mHdmiMemType = AudioHdmiType::HDMI_MEM_STEREO_PCM;
    }

    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);

    mAudioDigitalControl = AudioDigitalControlFactory::CreateAudioDigitalControl();
    mAudioHdmiControl = AudioHdmiControlFactory::CreateAudioHdmiControl();

    // setup HW buffer size
    int DL1HwBufferSize = mAudioDigitalControl->GetMemBufferSize(AudioDigitalType::MEM_DL1);
    int HdmiHwBufferSize = (DL1HwBufferSize * mDLAttribute.mChannels / 2) * ((float)mDLAttribute.mSampleRate / 44100.0f);
    // 32 frames alignment triming (imply 8 bytes alignment)
    int SizeAlignment = 32 * frameSize();
    HdmiHwBufferSize = (HdmiHwBufferSize / SizeAlignment) * SizeAlignment;
    // allocate kernel buffer
    mAudioHdmiControl->AllocateMemBuffer(mHdmiMemType, HdmiHwBufferSize);

    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);

    // setup latency
    int HwBufferSampleCount = HdmiHwBufferSize / (mDLAttribute.mChannels * (mDLAttribute.mFormat == AUDIO_FORMAT_PCM_8_BIT ? 1 : 2));
    mLatency = (HwBufferSampleCount * 1000) / mDLAttribute.mSampleRate;
    // setup interrupt sample count
    mDLAttribute.mInterruptSample = HwBufferSampleCount >> 1;
    // 16 frames alignment
    mDLAttribute.mBufferSize = HdmiHwBufferSize >> 1;

    prepareHdmiAudioParam();

    *status = NO_ERROR;
    ALOGD("-%s HdmiHwBufferSize = %d Latency = %lu ms InterruptSample = %lu\n",
          __FUNCTION__, HdmiHwBufferSize, mLatency, mDLAttribute.mInterruptSample);
}

AudioMTKHdmiStreamOut::~AudioMTKHdmiStreamOut()
{
    ALOGD("+%s[%d]\n", __FUNCTION__, mOutputType);
    // free kernel buffer
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    mAudioHdmiControl->FreeMemBuffer(mHdmiMemType);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    mSupportedChannelMasks.clear();
    if (mFd != -1)
    {
        ::close(mFd);
    }
    ALOGD("-%s[%d]\n", __FUNCTION__, mOutputType);
}

uint32_t AudioMTKHdmiStreamOut::sampleRate() const
{
    return mDLAttribute.mSampleRate;
}

size_t AudioMTKHdmiStreamOut::bufferSize() const
{
    return mDLAttribute.mBufferSize;
}

uint32_t AudioMTKHdmiStreamOut::channels() const
{
    return mChannelMask;
}

int AudioMTKHdmiStreamOut::format() const
{
    return mDLAttribute.mFormat;
}

uint32_t AudioMTKHdmiStreamOut::latency() const
{
    return mLatency;
}

status_t AudioMTKHdmiStreamOut::setVolume(float left, float right)
{
    return NO_ERROR;
}

ssize_t AudioMTKHdmiStreamOut::write(const void *buffer, size_t bytes)
{
    ALOGV("+%s[%d] bytes = %d\n", __FUNCTION__, mOutputType, bytes);

    if (mExternalSuspend > 0)
    {
        usleep(dataToDurationUs(bytes));
        ALOGD("%s[%d] suspend write\n", __FUNCTION__, mOutputType);
        return bytes;
    }

    status_t ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        ALOGW("%s[%d] EnableAudioLock AUDIO_HARDWARE_LOCK fail\n", __FUNCTION__, mOutputType);
        usleep(dataToDurationUs(bytes));
        return bytes;
    }
    ret = AudioMTKHdmiStreamOut::enableStreamOutsLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        ALOGW("%s[%d] enableStreamOutsLock fail\n", __FUNCTION__, mOutputType);
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        usleep(dataToDurationUs(bytes));
        return bytes;
    }
    ret = enableStreamLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        ALOGW("%s[%d] enableStreamLock fail\n", __FUNCTION__, mOutputType);
        AudioMTKHdmiStreamOut::disableStreamOutsLock();
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        usleep(dataToDurationUs(bytes));
        return bytes;
    }

    if (getStreamRunning() == false)
    {
        checkSuspendOutput();
        setStreamRunning(true);
        openPcmDumpFile();
        if (mInternalSuspend == false)
        {
            // enable audio clock
            requestPlaybackClock();
            // start HDMI memory
            ::ioctl(mFd, START_HDMI_MEMIF_TYPE, mHdmiMemType);
            mAudioHdmiControl->SetHdmiFrameSize(frameSize());
            //setup interrupt count
            mAudioDigitalControl->SetIrqMcuCounter(AudioDigitalType::IRQ5_MCU_MODE, mDLAttribute.mInterruptSample);
            // write data to kernel buffer
            bytes = writeDataToKernel(buffer, bytes);
            // notify audio setting to HDMI Tx
            notifyAudioSettingToHdmiTx();
            // enable IRQ5
            enableMcuIRQ(AudioDigitalType::IRQ5_MCU_MODE, true);
            // turn on AFE HDMI
            turnOnAfeHdmi();
            // enable digital block
            mAudioDigitalControl->SetMemIfEnable(AudioDigitalType::HDMI, true);
            // enable AFE module
            mAudioDigitalControl->SetAfeEnable(true);
            disableStreamLock();
            AudioMTKHdmiStreamOut::disableStreamOutsLock();
            mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
            return bytes;
        }
    }

    AudioMTKHdmiStreamOut::disableStreamOutsLock();
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);

    if (!mInternalSuspend)
    {
        // write data to kernel buffer
        bytes = writeDataToKernel(buffer, bytes);
    }
    else
    {
        ALOGD("%s[%d] internal suspend\n", __FUNCTION__, mOutputType);
        usleep(dataToDurationUs(bytes));
    }

    mPresentedBytes += bytes;
    clock_gettime(CLOCK_MONOTONIC, &mPresentedTime);

    disableStreamLock();
    ALOGV("-%s[%d] bytes = %d\n", __FUNCTION__, mOutputType, bytes);
    return bytes;
}

status_t AudioMTKHdmiStreamOut::standby()
{
    ALOGD("+%s[%d]\n", __FUNCTION__, mOutputType);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    AudioMTKHdmiStreamOut::enableStreamOutsLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    enableStreamLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    outputStandby();
    disableStreamLock();
    AudioMTKHdmiStreamOut::disableStreamOutsLock();
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
    ALOGD("-%s[%d]\n", __FUNCTION__, mOutputType);
    return NO_ERROR;
}

status_t AudioMTKHdmiStreamOut::dump(int fd, const Vector<String16> &args)
{
    return NO_ERROR;
}

status_t AudioMTKHdmiStreamOut::setParameters(const String8 &keyValuePairs)
{
    ALOGD("%s[%d] keyValuePairs: %s\n", __FUNCTION__, mOutputType, keyValuePairs.string());
    return NO_ERROR;
}

String8 AudioMTKHdmiStreamOut::getParameters(const String8 &keys)
{
    ALOGD("+%s[%d] keys: %s\n", __FUNCTION__, mOutputType, keys.string());
    AudioParameter param = AudioParameter(keys);
    String8 value;
    if (param.get(String8(AUDIO_PARAMETER_STREAM_SUP_CHANNELS), value) == NO_ERROR)
    {
        String8 support_channel;
        for (size_t i = 0; i < mSupportedChannelMasks.size(); i++)
        {
            support_channel.append(mSupportedChannelMasks[i]);
            if (i != (mSupportedChannelMasks.size() - 1))
            {
                support_channel.append("|");
            }
        }
        param.add(String8(AUDIO_PARAMETER_STREAM_SUP_CHANNELS), support_channel);
    }
    ALOGD("-%s[%d] keys: %s\n", __FUNCTION__, mOutputType, param.toString().string());
    return param.toString();
}

status_t AudioMTKHdmiStreamOut::getRenderPosition(uint32_t *dspFrames)
{
    return NO_ERROR;
}

status_t AudioMTKHdmiStreamOut::getPresentationPosition(uint64_t *frames, struct timespec *timestamp)
{
    *frames = mPresentedBytes / frameSize();
    *timestamp = mPresentedTime;
    return NO_ERROR;
}

void AudioMTKHdmiStreamOut::setStreamRunning(bool enable)
{
    ALOGD("%s[%d] enable = %d\n", __FUNCTION__, mOutputType, enable);
    mStarting = enable;
}

bool AudioMTKHdmiStreamOut::getStreamRunning(void)
{
    return mStarting;
}

status_t AudioMTKHdmiStreamOut::forceStandby(void)
{
    ALOGD("+%s[%d]\n", __FUNCTION__, mOutputType);
    AudioMTKHdmiStreamOut::enableStreamOutsLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    enableStreamLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    outputStandby();
    disableStreamLock();
    AudioMTKHdmiStreamOut::disableStreamOutsLock();
    ALOGD("-%s[%d]\n", __FUNCTION__, mOutputType);
    return NO_ERROR;
}

status_t AudioMTKHdmiStreamOut::internalStandby(void)
{
    ALOGD("+%s[%d]\n", __FUNCTION__, mOutputType);
    enableStreamLock(AUDIO_LOCK_TIMEOUT_VALUE_MS);
    outputStandby();
    disableStreamLock();
    ALOGD("-%s[%d]\n", __FUNCTION__, mOutputType);
    return NO_ERROR;
}

status_t AudioMTKHdmiStreamOut::setSuspend(bool suspend)
{
    if (suspend)
    {
        mExternalSuspend++;
    }
    else
    {
        mExternalSuspend--;
        if (mExternalSuspend < 0)
        {
            ALOGW("%s[%d] unexpected suspend reset\n", __FUNCTION__, mOutputType);
            mExternalSuspend = 0;
        }
    }
    return NO_ERROR;
}

status_t AudioMTKHdmiStreamOut::enableStreamLock(int timeout)
{
    status_t ret = 0;
    if (timeout != 0)
    {
        ret = mStreamLock.lock_timeout(timeout);
        if (ret)
        {
            ALOGW("%s[%d] lock %d ms timeout\n", __FUNCTION__, mOutputType, timeout);
        }
    }
    else
    {
        ret = mStreamLock.lock();
    }
    return ret;
}

void AudioMTKHdmiStreamOut::disableStreamLock(void)
{
    mStreamLock.unlock();
}

void AudioMTKHdmiStreamOut::requestPlaybackClock(void)
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    mAudioHdmiControl->SetHdmiClockSource(mHdmiFsType);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_APLL_TUNER_CLOCK, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_HDMI, true);
}

void AudioMTKHdmiStreamOut::releasePlaybackClock(void)
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_HDMI, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_APLL_TUNER_CLOCK, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
}

void AudioMTKHdmiStreamOut::checkSuspendOutput(void)
{
    ALOGD("+%s[%d]\n", __FUNCTION__, mOutputType);
    if (mOutputType == HDMI_MULTI_CH)
    {
        forceHDMIStereoStandy();
    }
    else if (mStreamOuts[HDMI_MULTI_CH] &&
             mStreamOuts[HDMI_MULTI_CH]->getStreamRunning() == true)
    {
        mInternalSuspend = true;
    }
    else
    {
        mInternalSuspend = false;
    }
    ALOGD("-%s[%d]\n", __FUNCTION__, mOutputType);
}

void AudioMTKHdmiStreamOut::forceHDMIStereoStandy(void)
{
    if (mStreamOuts[HDMI_STEREO])
    {
        ALOGD("+%s[%d]\n", __FUNCTION__, mOutputType);
        mStreamOuts[HDMI_STEREO]->internalStandby();
        ALOGD("-%s[%d]\n", __FUNCTION__, mOutputType);
    }
}

void AudioMTKHdmiStreamOut::outputStandby(void)
{
    if (getStreamRunning() == true)
    {
        ALOGD("+%s[%d]\n", __FUNCTION__, mOutputType);
        closePcmDumpFile();
        setStreamRunning(false);
        if (mInternalSuspend == false)
        {
            // turn off AFE HDMI
            turnOffAfeHdmi();
            // disable IRQ5
            enableMcuIRQ(AudioDigitalType::IRQ5_MCU_MODE, false);
            // disable digital block
            mAudioDigitalControl->SetMemIfEnable(AudioDigitalType::HDMI, false);
            // disable AFE module
            mAudioDigitalControl->SetAfeEnable(false);
            // standy HDMI memory
            ::ioctl(mFd, STANDBY_HDMI_MEMIF_TYPE, mHdmiMemType);
            // disable clock
            releasePlaybackClock();
        }
        if (mOutputType == HDMI_MULTI_CH)
        {
            // let stereo stream have chance to leave internal suspend
            forceHDMIStereoStandy();
        }
        ALOGD("-%s[%d]\n", __FUNCTION__, mOutputType);
    }
}

status_t AudioMTKHdmiStreamOut::enableMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode, bool Enable)
{
    ALOGD("%s[%d] IRQ_mode = %d Enable = %d\n", __FUNCTION__, mOutputType, IRQ_mode, Enable);
    return mAudioDigitalControl->SetIrqMcuEnable(IRQ_mode, Enable);
}

void AudioMTKHdmiStreamOut::turnOnAfeHdmi(void)
{
    // setup interconnection
    setHdmiInterConnection(AudioDigitalType::Connection);
    // set HDMI Output Control
    setHdmiOutControl();
    setHdmiOutControlEnable(true);
    // set HDMI I2S
    setHdmiI2SAttribute();
    setHdmiBckDiv();
    setHdmiI2SEnable(true);
}

void AudioMTKHdmiStreamOut::turnOffAfeHdmi(void)
{
    // reset interconnection
    setHdmiInterConnection(AudioDigitalType::DisConnect);
    // turn off HDMI I2S
    setHdmiI2SEnable(false);
    // turn off memory interface
    setHdmiOutControlEnable(false);
}

status_t AudioMTKHdmiStreamOut::setHdmiOutControl(void)
{
    AudioHdmiOutControl HdmiOutControl;
    HdmiOutControl.mHDMI_OUT_CH_NUM = mDLAttribute.mChannels;
    HdmiOutControl.mHDMI_OUT_BIT_WIDTH = AudioHdmiOutControl::HDMI_INPUT_16BIT;
    return mAudioHdmiControl->SetHdmiOutControl(&HdmiOutControl);
}

status_t AudioMTKHdmiStreamOut::setHdmiOutControlEnable(bool Enable)
{
    return mAudioHdmiControl->SetHdmiOutControlEnable(Enable);
}

status_t AudioMTKHdmiStreamOut::setHdmiBckDiv(void)
{
    return mAudioHdmiControl->SetHdmiBckDiv(mHdmiFsType);
}

status_t AudioMTKHdmiStreamOut::setHdmiI2SAttribute(void)
{
    AudioHdmiI2S HdmiI2SAttr;
    HdmiI2SAttr.mI2S_WLEN = AudioHdmiI2S::HDMI_I2S_32BIT;
    HdmiI2SAttr.mI2S_DELAY_DATA = AudioHdmiI2S::HDMI_I2S_FIRST_BIT_1T_DELAY;
    HdmiI2SAttr.mI2S_LRCK_INV = AudioHdmiI2S::HDMI_I2S_LRCK_NOT_INVERSE;
    HdmiI2SAttr.mI2S_BCLK_INV = AudioHdmiI2S::HDMI_I2S_BCLK_INVERSE;
    return mAudioHdmiControl->SetHdmiI2S(&HdmiI2SAttr);
}

status_t AudioMTKHdmiStreamOut::setHdmiI2SEnable(bool Enable)
{
    return mAudioHdmiControl->SetHdmiI2SEnable(Enable);
}

void AudioMTKHdmiStreamOut::setHdmiInterConnection(uint32_t ConnectionState)
{
    // O20~O27: L/R/LS/RS/C/LFE/CH7/CH8
    switch (mChannelMask)
    {
        case AUDIO_CHANNEL_OUT_7POINT1:
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I26, AudioDigitalType::O26);
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I27, AudioDigitalType::O27);
            // fall through
        case AUDIO_CHANNEL_OUT_5POINT1:
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I24, AudioDigitalType::O22);
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I25, AudioDigitalType::O23);
            // fall through
        case AUDIO_CHANNEL_OUT_QUAD:
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I22, AudioDigitalType::O24);
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I23, AudioDigitalType::O25);
            // fall through
        case AUDIO_CHANNEL_OUT_STEREO:
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I20, AudioDigitalType::O20);
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I21, AudioDigitalType::O21);
            break;
        case AUDIO_CHANNEL_OUT_SURROUND:
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I20, AudioDigitalType::O20);
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I21, AudioDigitalType::O21);
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I22, AudioDigitalType::O24);
            // route RC channel to LS speaker
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I23, AudioDigitalType::O22);
            break;
        case AUDIO_CHANNEL_OUT_MONO:
            mAudioDigitalControl->SetinputConnection(ConnectionState, AudioDigitalType::I20, AudioDigitalType::O20);
            break;
        default:
            ALOGW("%s[%d] unknown channel mask 0x%x\n", __FUNCTION__, mOutputType, mChannelMask);
            break;
    }
}

size_t AudioMTKHdmiStreamOut::writeDataToKernel(const void *buffer, size_t bytes)
{
    if (mPcmDumpFile)
    {
        if (ftell(mPcmDumpFile) > MAX_DUMP_SIZE_IN_BYTES)
        {
            ALOGW("%s[%d] rewind dump file\n", __FUNCTION__, mOutputType);
            rewind(mPcmDumpFile);
        }
        if (bytes != fwrite((void *)buffer, 1, bytes, mPcmDumpFile))
        {
            ALOGW("%s[%d] dump file fail\n", __FUNCTION__, mOutputType);
        }
    }

    return ::write(mFd, buffer, bytes);
}

void AudioMTKHdmiStreamOut::openPcmDumpFile(void)
{
    if (!mPcmDumpFile)
    {
        String8 dump_file_name;
        dump_file_name.appendFormat("%s/streamout[%d]_hdmi_%d_%dch_%d.pcm", streamout_dump_path, mOutputType,
                                    mDLAttribute.mSampleRate, mDLAttribute.mChannels, mDumpFileNum[mOutputType]);
        mPcmDumpFile = AudioOpendumpPCMFile(dump_file_name.string(), streamout_hdmi_property);
        mDumpFileNum[mOutputType]++;
        mDumpFileNum[mOutputType] %= MAX_DUMP_FILE_NUM;
    }
}

void AudioMTKHdmiStreamOut::closePcmDumpFile(void)
{
    if (mPcmDumpFile)
    {
        AudioCloseDumpPCMFile(mPcmDumpFile);
        mPcmDumpFile = NULL;
    }
}

uint32_t AudioMTKHdmiStreamOut::dataToDurationUs(size_t bytes)
{
    return (bytes * 1000 / (mDLAttribute.mChannels * mDLAttribute.mSampleRate * 2 / 1000));
}

void AudioMTKHdmiStreamOut::prepareHdmiAudioParam(void)
{
    mHdmiAudioParam.e_hdmi_aud_in = SV_I2S;
    mHdmiAudioParam.e_aud_code = AVD_LPCM;
    mHdmiAudioParam.e_I2sFmt = HDMI_I2S_24BIT;
    mHdmiAudioParam.u1HdmiI2sMclk = MCLK_128FS;

    switch (mDLAttribute.mSampleRate)
    {
        case 192000:
            mHdmiAudioParam.e_iec_frame = IEC_192K;
            mHdmiAudioParam.e_hdmi_fs = HDMI_FS_192K;
            // channel status byte 3: sampling frequency and clock accuracy
            mHdmiAudioParam.bhdmi_LCh_status[3] = 0xE;
            break;
        case 176400:
            mHdmiAudioParam.e_iec_frame = IEC_176K;
            mHdmiAudioParam.e_hdmi_fs = HDMI_FS_176K;
            // channel status byte 3: sampling frequency and clock accuracy
            mHdmiAudioParam.bhdmi_LCh_status[3] = 0xC;
            break;
        case 96000:
            mHdmiAudioParam.e_iec_frame = IEC_96K;
            mHdmiAudioParam.e_hdmi_fs = HDMI_FS_96K;
            // channel status byte 3: sampling frequency and clock accuracy
            mHdmiAudioParam.bhdmi_LCh_status[3] = 0xA;
            break;
        case 88200:
            mHdmiAudioParam.e_iec_frame = IEC_88K;
            mHdmiAudioParam.e_hdmi_fs = HDMI_FS_88K;
            // channel status byte 3: sampling frequency and clock accuracy
            mHdmiAudioParam.bhdmi_LCh_status[3] = 0x8;
            break;
        case 48000:
            mHdmiAudioParam.e_iec_frame = IEC_48K;
            mHdmiAudioParam.e_hdmi_fs = HDMI_FS_48K;
            // channel status byte 3: sampling frequency and clock accuracy
            mHdmiAudioParam.bhdmi_LCh_status[3] = 0x2;
            break;
        case 44100:
            mHdmiAudioParam.e_iec_frame = IEC_44K;
            mHdmiAudioParam.e_hdmi_fs = HDMI_FS_44K;
            // channel status byte 3: sampling frequency and clock accuracy
            mHdmiAudioParam.bhdmi_LCh_status[3] = 0x0;
            break;
        case 32000:
            mHdmiAudioParam.e_iec_frame = IEC_32K;
            mHdmiAudioParam.e_hdmi_fs = HDMI_FS_32K;
            // channel status byte 3: sampling frequency and clock accuracy
            mHdmiAudioParam.bhdmi_LCh_status[3] = 0x3;
            break;
        default:
            ALOGW("%s[%d] unsupported sample rate %d\n", __FUNCTION__, mOutputType, mDLAttribute.mSampleRate);
            break;
    }

    switch (mChannelMask)
    {
        case AUDIO_CHANNEL_OUT_MONO:
            mHdmiAudioParam.u1Aud_Input_Chan_Cnt = AUD_INPUT_1_0;
            break;
        case AUDIO_CHANNEL_OUT_STEREO:
            mHdmiAudioParam.u1Aud_Input_Chan_Cnt = AUD_INPUT_2_0;
            break;
        case AUDIO_CHANNEL_OUT_SURROUND:
            mHdmiAudioParam.u1Aud_Input_Chan_Cnt = AUD_INPUT_4_0_CLRS;
            break;
        case AUDIO_CHANNEL_OUT_QUAD:
            mHdmiAudioParam.u1Aud_Input_Chan_Cnt = AUD_INPUT_4_0;
            break;
        case AUDIO_CHANNEL_OUT_5POINT1:
            mHdmiAudioParam.u1Aud_Input_Chan_Cnt = AUD_INPUT_5_1;
            break;
        case AUDIO_CHANNEL_OUT_7POINT1:
            mHdmiAudioParam.u1Aud_Input_Chan_Cnt = AUD_INPUT_7_1;
            break;
        default:
            ALOGW("%s[%d] unknown channel mask %x\n", __FUNCTION__, mOutputType, mChannelMask);
            break;
    }

    // channel status byte 0: general control ande mode information
    mHdmiAudioParam.bhdmi_LCh_status[0] = CHANNEL_STATUS_COPY_BIT << 2;
    // channel status byte 1: category code
    mHdmiAudioParam.bhdmi_LCh_status[1] = CHANNEL_STATUS_CATEGORY_CODE;
    // channel status byte 2: source and channel number
    mHdmiAudioParam.bhdmi_LCh_status[2] = 0;
    // channel status byte 4: word length and original sampling frequency
    mHdmiAudioParam.bhdmi_LCh_status[4] = 0x2;
    memcpy(mHdmiAudioParam.bhdmi_RCh_status, mHdmiAudioParam.bhdmi_LCh_status, sizeof(mHdmiAudioParam.bhdmi_LCh_status));
}

void AudioMTKHdmiStreamOut::notifyAudioSettingToHdmiTx(void)
{
    int fd = open(HDMI_DEVICE_NAME, O_RDONLY);
    if (fd == -1)
    {
        ALOGE("%s[%d] open %s fail errno = %d\n", __FUNCTION__, mOutputType, HDMI_DEVICE_NAME, errno);
        return;
    }

    int ret = ioctl(fd, MTK_HDMI_AUDIO_SETTING, &mHdmiAudioParam);
    if (ret != 0)
    {
        ALOGE("%s[%d] ioctl MTK_HDMI_AUDIO_SETTING fail errno = %d\n", __FUNCTION__, mOutputType, errno);
    }

    close(fd);
}

}
