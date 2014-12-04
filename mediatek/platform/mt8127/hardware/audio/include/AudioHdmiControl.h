#ifndef _AUDIO_HDMI_CONTROL_H
#define _AUDIO_HDMI_CONTROL_H

#include "AudioHdmiControlInterface.h"
#include "AudioAfeReg.h"

namespace android
{
//this class only control digital part  , interconnection is not incldue.
class AudioHdmiControl : public AudioHdmiControlInterface
{
    public:

        static AudioHdmiControl *getInstance();
        ~AudioHdmiControl() {};

        /**
        * a function of allocate memory for HDMI
        * @param MemType
        * @return status_t
        */
        virtual status_t AllocateMemBuffer(uint32 MemType, uint32 BufferSzie);

        /**
        * a function of free memory for HDMI
        * @param MemType
        * @return status_t
        */
        virtual status_t FreeMemBuffer(uint32 MemType);

        /**
        * a function fo SetHdmiOutControl
        * @param HdmiOutControl
        * @return status_t
        */
        virtual status_t SetHdmiOutControl(AudioHdmiOutControl *HdmiOutControl);

        /**
        * a function fo SetHdmiOutControlEnable
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHdmiOutControlEnable(bool Enable);

        /**
        * a function fo SetHdmiI2S
        * @param AudioHdmiI2S
        * @return status_t
        */
        virtual status_t SetHdmiI2S(AudioHdmiI2S *AudioHdmiI2S);

        /**
        * a function fo SetHdmiI2SEnable
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHdmiI2SEnable(bool Enable);

        /**
        * a function fo SetHdmiClockSource
        * @param FsType
        * @return status_t
        */
        virtual status_t SetHdmiClockSource(uint32 FsType);

        /**
        * a function fo SetHdmiBckDiv
        * @param FsType
        * @return status_t
        */
        virtual status_t SetHdmiBckDiv(uint32 FsType);

        /**
        * a function fo SetHdmiFrameSize
        * @param FrameSize
        * @return status_t
        */
        virtual status_t SetHdmiFrameSize(uint32 FrameSize);

    private:

        static AudioHdmiControl *UniqueControlInstance;
        AudioHdmiControl();

        AudioAfeReg *mAfeReg;
        uint32 mFd;
        static const AudioHDMIClockSetting mHdmiClockSettings[AudioHdmiType::HDMI_FS_TYPE_COUNT];
};

}

#endif
