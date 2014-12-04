#include "AudioHdmiControl.h"

#define LOG_TAG "AudioHdmiControl"
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

namespace android
{

AudioHdmiControl *AudioHdmiControl::UniqueControlInstance = 0;

AudioHdmiControl *AudioHdmiControl::getInstance()
{
    static Mutex sLock;
    AutoMutex lock(sLock);
    if (UniqueControlInstance == 0)
    {
        ALOGD("+%s()\n", __FUNCTION__);
        UniqueControlInstance = new AudioHdmiControl();
        ALOGD("-%s()\n", __FUNCTION__);
    }
    return UniqueControlInstance;
}

const AudioHDMIClockSetting AudioHdmiControl::mHdmiClockSettings[AudioHdmiType::HDMI_FS_TYPE_COUNT] =
{
    {32000,  AudioHDMIClockSetting::APLL_D24, 0}, // 32k
    {44100,  AudioHDMIClockSetting::APLL_D16, 0}, // 44.1k
    {48000,  AudioHDMIClockSetting::APLL_D16, 0}, // 48k
    {88200,  AudioHDMIClockSetting::APLL_D8,  0}, // 88.2k
    {96000,  AudioHDMIClockSetting::APLL_D8,  0}, // 96k
    {176400, AudioHDMIClockSetting::APLL_D4,  0}, // 176.4k
    {192000, AudioHDMIClockSetting::APLL_D4,  0}  // 192k
};

AudioHdmiControl::AudioHdmiControl()
    : mAfeReg(NULL)
    , mFd(-1)
{
    mAfeReg = AudioAfeReg::getInstance();
    if (mAfeReg)
    {
        mFd = mAfeReg->GetAfeFd();
        mAfeReg->SetAfeReg(AFE_HDMI_CONN0, 0x7fffffff, 0xffffffff);
    }
    else
    {
        ALOGE("%s invalid mAfeReg\n", __FUNCTION__);
    }
}

status_t AudioHdmiControl::AllocateMemBuffer(uint32 MemType, uint32 BufferSzie)
{
    int ret = 0;
    switch (MemType)
    {
        case AudioHdmiType::HDMI_MEM_STEREO_PCM:
            ret = ::ioctl(mFd, ALLOCATE_MEMIF_HDMI_STEREO_PCM, BufferSzie);
            break;
        case AudioHdmiType::HDMI_MEM_MULTI_CH_PCM:
            ret = ::ioctl(mFd, ALLOCATE_MEMIF_HDMI_MULTI_CH_PCM, BufferSzie);
            break;
        default:
            ret = BAD_VALUE;
            break;
    }
    return ret;
}

status_t AudioHdmiControl::FreeMemBuffer(uint32 MemType)
{
    int ret = 0;
    switch (MemType)
    {
        case AudioHdmiType::HDMI_MEM_STEREO_PCM:
            ret = ::ioctl(mFd, FREE_MEMIF_HDMI_STEREO_PCM);
            break;
        case AudioHdmiType::HDMI_MEM_MULTI_CH_PCM:
            ret = ::ioctl(mFd, FREE_MEMIF_HDMI_MULTI_CH_PCM);
            break;
        default:
            ret = BAD_VALUE;
            break;
    }
    return ret;
}

status_t AudioHdmiControl::SetHdmiOutControl(AudioHdmiOutControl *HdmiOutControl)
{
    uint32 RegisterValue = 0;
    RegisterValue |= (HdmiOutControl->mHDMI_OUT_CH_NUM << 4);
    RegisterValue |= (HdmiOutControl->mHDMI_OUT_BIT_WIDTH << 1);
    return mAfeReg->SetAfeReg(AFE_HDMI_OUT_CON0, RegisterValue, AFE_MASK_ALL);
}

status_t AudioHdmiControl::SetHdmiOutControlEnable(bool Enable)
{
    return mAfeReg->SetAfeReg(AFE_HDMI_OUT_CON0, Enable, 0x1);
}

status_t AudioHdmiControl::SetHdmiI2S(AudioHdmiI2S *AudioHdmiI2S)
{
    uint32 RegisterValue = 0;
    RegisterValue |= (AudioHdmiI2S->mI2S_WLEN << 4);
    RegisterValue |= (AudioHdmiI2S->mI2S_DELAY_DATA << 3);
    RegisterValue |= (AudioHdmiI2S->mI2S_LRCK_INV << 2);
    RegisterValue |= (AudioHdmiI2S->mI2S_BCLK_INV << 1);
    return mAfeReg->SetAfeReg(AFE_8CH_I2S_OUT_CON, RegisterValue, AFE_MASK_ALL);
}

status_t AudioHdmiControl::SetHdmiI2SEnable(bool Enable)
{
    return mAfeReg->SetAfeReg(AFE_8CH_I2S_OUT_CON, Enable, 0x1);
}

status_t AudioHdmiControl::SetHdmiClockSource(uint32 FsType)
{
    Hdmi_Clock_Control ClockControl;
    ClockControl.SampleRate = mHdmiClockSettings[FsType].SAMPLE_RATE;
    ClockControl.ClkApllSel = mHdmiClockSettings[FsType].CLK_APLL_SEL;
    return ::ioctl(mFd, SET_HDMI_CLOCK_SOURCE, &ClockControl);
}

status_t AudioHdmiControl::SetHdmiBckDiv(uint32 FsType)
{
    uint32 RegisterValue = 0;
    RegisterValue |= (mHdmiClockSettings[FsType].HDMI_BCK_DIV) << 8;
    return mAfeReg->SetAfeReg(AUDIO_TOP_CON3, RegisterValue, 0x3F00);
}

status_t AudioHdmiControl::SetHdmiFrameSize(uint32 FrameSize)
{
    return ::ioctl(mFd, SET_HDMI_FRAME_SIZE, FrameSize);
}

}//namespace android
