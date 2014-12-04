#include "AudioPlatformDevice.h"
#include "AudioAnalogType.h"
#include "audio_custom_exp.h"

#define LOG_TAG "AudioPlatformDevice"
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

status_t AudioPlatformDevice::InitCheck()
{
    ALOGD("InitCheck");
    return NO_ERROR;
}

AudioPlatformDevice::AudioPlatformDevice()
{
    ALOGD("AudioPlatformDevice constructor");
    mAudioAnalogReg = NULL;
    mAudioAnalogReg = AudioAnalogReg::getInstance();
    if (!mAudioAnalogReg)
    {
        ALOGW("mAudioAnalogReg = %p", mAudioAnalogReg);
    }
    // init analog part.
    for (int i = 0; i < AudioAnalogType::DEVICE_MAX; i++)
    {
        memset((void *)&mBlockAttribute[i], 0, sizeof(AnalogBlockAttribute));
    }
    for (int i = 0; i < AudioAnalogType::VOLUME_TYPE_MAX; i++)
    {
        memset((void *)&mVolumeAttribute[i], 0, sizeof(AnalogVolumeAttribute));
    }
}

/**
* a basic function for SetAnalogGain for different Volume Type
* @param VoleumType value want to set to analog volume
* @param volume function of analog gain , value between 0 ~ 255
* @return status_t
*/
status_t AudioPlatformDevice::SetAnalogGain(AudioAnalogType::VOLUME_TYPE VoleumType, int volume)
{
    ALOGD("SetAnalogGain VOLUME_TYPE = %d volume = %d ", VoleumType, volume);
    return NO_ERROR;
}

/**
* a basic function fo SetAnalogMute, if provide mute function of hardware.
* @param VoleumType value want to set to analog volume
* @param mute of volume type
* @return status_t
*/
status_t AudioPlatformDevice::SetAnalogMute(AudioAnalogType::VOLUME_TYPE VoleumType, bool mute)
{
    ALOGD("AudioPlatformDevice SetAnalogMute VOLUME_TYPE = %d mute = %d ", VoleumType, mute);
    return NO_ERROR;
}


status_t AudioPlatformDevice::SetFrequency(AudioAnalogType::DEVICE_SAMPLERATE_TYPE DeviceType, unsigned int frequency)
{
    ALOGD("AudioPlatformDevice SetFrequency");
    mBlockSampleRate[DeviceType] = frequency;
    return NO_ERROR;
}

uint32 AudioPlatformDevice::GetDLFrequency(unsigned int frequency)
{
    ALOGD("AudioPlatformDevice ApplyDLFrequency ApplyDLFrequency = %d", frequency);
    uint32 Reg_value = 0;
    switch (frequency)
    {
        case 8000:
            Reg_value = 0 << 12;
            break;
        case 11025:
            Reg_value = 1 << 12;
            break;
        case 12000:
            Reg_value = 2 << 12;
            break;
        case 16000:
            Reg_value = 3 << 12;
            break;
        case 22050:
            Reg_value = 4 << 12;
            break;
        case 24000:
            Reg_value = 5 << 12;
            break;
        case 32000:
            Reg_value = 6 << 12;
            break;
        case 44100:
            Reg_value = 7 << 12;
            break;
        case 48000:
            Reg_value = 8 << 12;
        default:
            ALOGW("ApplyDLFrequency with frequency = %d", frequency);
    }
    return Reg_value;
}


uint32 AudioPlatformDevice::GetULFrequency(unsigned int frequency)
{
    ALOGD("AudioPlatformDevice GetULFrequency ApplyDLFrequency = %d", frequency);
    uint32 Reg_value = 0;
    switch (frequency)
    {
        case 8000:
            Reg_value = 0x0 << 1;
            break;
        case 16000:
            Reg_value = 0x5 << 1;
            break;
        case 32000:
            Reg_value = 0xa << 1;
            break;
        case 48000:
            Reg_value = 0xf << 1;
        default:
            ALOGW("GetULFrequency with frequency = %d", frequency);
    }
    ALOGD("AudioPlatformDevice GetULFrequency Reg_value = %d", Reg_value);
    return Reg_value;
}

uint32 AudioPlatformDevice::GetULNewIFFrequency(unsigned int frequency)
{
    ALOGD("AudioPlatformDevice GetULNewIFFrequency ApplyULNewIFFrequency = %d", frequency);
    uint32 Reg_value = 0;
    switch (frequency)
    {
        case 8000:
        case 16000:
        case 32000:
            Reg_value = 1;
            break;
        case 48000:
            Reg_value = 3;
        default:
            ALOGW("GetULNewIFFrequency with frequency = %d", frequency);
    }
    ALOGD("AudioPlatformDevice GetULNewIFFrequency Reg_value = %d", Reg_value);
    return Reg_value;
}

bool AudioPlatformDevice::GetDownLinkStatus(void)
{
    for (int i = 0; i <= AudioAnalogType::DEVICE_2IN1_SPK; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            return true;
        }
    }
    return false;
}

bool AudioPlatformDevice::GetULinkStatus(void)
{
    for (int i = AudioAnalogType::DEVICE_2IN1_SPK; i <= AudioAnalogType::DEVICE_IN_DIGITAL_MIC; i++)
    {
        if (mBlockAttribute[i].mEnable == true)
        {
            return true;
        }
    }
    return false;
}

/**
* a basic function fo AnalogOpen, open analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioPlatformDevice::AnalogOpen(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogOpen DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();
    if (mBlockAttribute[DeviceType].mEnable == true)
    {
        ALOGW("AudioPlatformDevice AnalogOpen bypass with DeviceType = %d", DeviceType);
        mLock.unlock();
        return NO_ERROR;;
    }
    mBlockAttribute[DeviceType].mEnable = true;

    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_CLR , 0x0003, 0xffff);

            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0 , GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0xf000);

            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0006, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON0 , 0xc3a1, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x000b, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SDM_CON1 , 0x001e, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_H , 0x0300 | GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0 , 0x007f, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L , 0x1801, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON1_H , 0x00e1, 0xffff);
            break;

        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_CLR , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0 , GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0xf000);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0006, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON0 , 0xc3a1, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x000b, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SDM_CON1 , 0x001e, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_H , 0x0300 | GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0x0ffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0 , 0x007f, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L , 0x1801, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON1_H , 0x00e1, 0xffff);
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
#ifdef USING_EXTAMP_HP
            mLock.unlock();
            AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();

#else
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_CLR , 0x0607, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0 , GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0xf000);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0006, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON0 , 0xc3a1, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x000b, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SDM_CON1 , 0x001e, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_H , 0x0300 | GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0x0ffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0 , 0x007f, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L , 0x1801, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON1_H , 0x00e1, 0xffff);
#endif

            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_CLR , 0x0607, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0 , GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0xf000);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0006, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON0 , 0xc3a1, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x000b, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SDM_CON1 , 0x001e, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_H , 0x0300 | GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0x0ffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0 , 0x007f, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L , 0x1801, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON1_H , 0x00e1, 0xffff);
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            ALOGD("AudioPlatformDevice::DEVICE_IN_ADC2:");
            mAudioAnalogReg->SetAnalogReg(AUDCLKGEN_CFG0 , 0x0000, 0x0002);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_L , 0x0000, 0xffff);
            //mAfeReg->SetAfeReg(AFE_ADDA_UL_SRC_CON0, 0x0000, 0x07a3);
            mAudioAnalogReg->SetAnalogReg(AUDCLKGEN_CFG0 , 0x0002, 0x0002);

            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_CLR , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(ANA_AUDIO_TOP_CON0 , 0x0000, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_H , 0x0000 | GetULFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]), 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0 , 0x007f, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_L , 0x0001, 0xffff);
            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            mAudioAnalogReg->SetAnalogReg(AUDCLKGEN_CFG0 , 0x0000, 0x0002);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_L , 0x0000, 0xffff);
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_CLR , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(ANA_AUDIO_TOP_CON0 , 0x0000, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_H , 0x00e0 | GetULFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_IN_ADC]), 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0 , 0x007f, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_L , 0x0023, 0xffff);
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_CLR , 0x0607, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_PMIC_NEWIF_CFG0 , GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0xf000);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0006, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON0 , 0xc3a1, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x0003, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFUNC_AUD_CON2 , 0x000b, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SDM_CON1 , 0x001e, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_H , 0x0300 | GetDLFrequency(mBlockSampleRate[AudioAnalogType::DEVICE_OUT_DAC]), 0x0ffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0 , 0x007f, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L , 0x1801, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON1_H , 0x00e1, 0xffff);
            break;
    }
    mLock.unlock();
    return NO_ERROR;
}

/**
* a basic function fo AnalogClose, ckose analog power
* @param DeviceType analog part power
* @return status_t
*/
status_t AudioPlatformDevice::AnalogClose(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogClose DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();
    mBlockAttribute[DeviceType].mEnable = false;
    // here to open pmic digital part
    switch (DeviceType)
    {
        case AudioAnalogType::DEVICE_OUT_EARPIECER:
        case AudioAnalogType::DEVICE_OUT_EARPIECEL:
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L, 0x1800, 0xffff);
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0, 0x0000, 0xffff);
            }
            break;
        case AudioAnalogType::DEVICE_OUT_HEADSETR:
        case AudioAnalogType::DEVICE_OUT_HEADSETL:
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L, 0x1800, 0xffff);
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0, 0x0000, 0xffff);
            }
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKERR:
        case AudioAnalogType::DEVICE_OUT_SPEAKERL:
#ifdef USING_EXTAMP_HP
            mLock.unlock();
            AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR);
            mLock.lock();
#else
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L, 0x1800, 0xffff);
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0, 0x0000, 0xffff);
            }
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_SET , 0x0604, 0xffff);
#endif
            break;
        case AudioAnalogType::DEVICE_2IN1_SPK:
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L, 0x1800, 0xffff);
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0, 0x0000, 0xffff);
            }
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_SET , 0x0604, 0xffff);
            break;
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R:
        case AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L:
            mAudioAnalogReg->SetAnalogReg(AFE_DL_SRC2_CON0_L, 0x1800, 0xffff);
            if (GetULinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0, 0x0000, 0xffff);
            }
            mAudioAnalogReg->SetAnalogReg(TOP_CKPDN_SET , 0x0604, 0xffff);
            break;
        case AudioAnalogType::DEVICE_IN_ADC1:
        case AudioAnalogType::DEVICE_IN_ADC2:
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_L , 0x0000, 0xffff);
            if (GetDownLinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0, 0x0000, 0xffff);
            }
            //mAfeReg->SetAfeReg(AFE_ADDA_UL_SRC_CON0, 0x0000, 0x07a3);
            break;
        case AudioAnalogType::DEVICE_IN_DIGITAL_MIC:
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_H , 0x0000, 0xffff);
            mAudioAnalogReg->SetAnalogReg(AFE_UL_SRC_CON0_L , 0x0000, 0xffff);
            if (GetDownLinkStatus() == false)
            {
                mAudioAnalogReg->SetAnalogReg(AFE_UL_DL_CON0, 0x0000, 0xffff);
            }
            //mAfeReg->SetAfeReg(AFE_ADDA_UL_SRC_CON0, 0x00000000, 0xffffffff);
            break;
    }
    mLock.unlock();
    return NO_ERROR;
}

/**
* a basic function fo select mux of device type, not all device may have mux
* if select a device with no mux support , report error.
* @param DeviceType analog part
* @param MuxType analog mux selection
* @return status_t
*/
status_t AudioPlatformDevice::AnalogSetMux(AudioAnalogType::DEVICE_TYPE DeviceType, AudioAnalogType::MUX_TYPE MuxType)
{
    ALOGD("AAudioPlatformDevice nalogSetMux DeviceType = %s MuxType = %s", kAudioAnalogDeviceTypeName[DeviceType], kAudioAnalogMuxTypeName[MuxType]);
    return NO_ERROR;
}

/**
* a  function for setParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return status_t
*/
status_t AudioPlatformDevice::setParameters(int command1 , int command2 , unsigned int data)
{
    return NO_ERROR;
}

/**
* a function for setParameters , provide wide usage of analog control
* @param command1
* @param data
* @return status_t
*/
status_t AudioPlatformDevice::setParameters(int command1 , void *data)
{
    return NO_ERROR;
}

/**
* a function fo getParameters , provide wide usage of analog control
* @param command1
* @param command2
* @param data
* @return copy_size
*/
int AudioPlatformDevice::getParameters(int command1 , int command2 , void *data)
{
    return 0;
}

static const uint32_t kFadeSamples[4] = {4096, 2048, 1024, 512};

status_t AudioPlatformDevice::FadeOutDownlink(uint16_t sample_rate)
{
    return NO_ERROR;
}

status_t AudioPlatformDevice::FadeInDownlink(uint16_t sample_rate)
{
    return NO_ERROR;
}

status_t AudioPlatformDevice::SetDcCalibration(AudioAnalogType::DEVICE_TYPE DeviceType, int dc_cali_value)
{  // TODO: Need to add MT6397 code here
    return NO_ERROR;
}

status_t AudioPlatformDevice::AnalogOpenForAddSPK(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogOpenForAddSPK DeviceType = %s ", kAudioAnalogDeviceTypeName[DeviceType]);
 //   uint32 ulFreq, dlFreq;
    mLock.lock();

        mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = false;
        mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETL].mEnable = false;
        mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R].mEnable = true;   
        
     mLock.unlock();
    return NO_ERROR;
}

status_t AudioPlatformDevice::AnalogCloseForSubSPK(AudioAnalogType::DEVICE_TYPE DeviceType)
{
    ALOGD("AudioPlatformDevice AnalogCloseForSubSPK DeviceType = %s", kAudioAnalogDeviceTypeName[DeviceType]);
    mLock.lock();

    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_R].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_SPEAKER_HEADSET_L].mEnable = false;
    mBlockAttribute[AudioAnalogType::DEVICE_OUT_HEADSETR].mEnable = true;
  
    mLock.unlock();
    return NO_ERROR;
}


}

