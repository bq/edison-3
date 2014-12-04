#ifndef _AUDIO_HDMI_CONTROL_INTERFACE_H
#define _AUDIO_HDMI_CONTROL_INTERFACE_H

#include "AudioType.h"
#include "AudioHdmiType.h"

//! HDMI control interface
/*!
  this class defines HDMI part control interface
*/
namespace android
{

class AudioHdmiControlInterface
{
    public:
        /**
        * a destuctor for AudioHdmiControlInterface
        */
        virtual ~AudioHdmiControlInterface() {};

        /**
        * a function of allocate memory for HDMI
        * @param MemType
        * @return status_t
        */
        virtual status_t AllocateMemBuffer(uint32 MemType, uint32 BufferSzie) = 0;

        /**
        * a function of free memory for HDMI
        * @param MemType
        * @return status_t
        */
        virtual status_t FreeMemBuffer(uint32 MemType) = 0;

        /**
        * a function fo SetHdmiOutControl
        * @param HdmiOutControl
        * @return status_t
        */
        virtual status_t SetHdmiOutControl(AudioHdmiOutControl *HdmiOutControl) = 0;

        /**
        * a function fo SetHdmiOutControlEnable
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHdmiOutControlEnable(bool Enable) = 0;

        /**
        * a function fo SetHdmiI2S
        * @param AudioHdmiI2S
        * @return status_t
        */
        virtual status_t SetHdmiI2S(AudioHdmiI2S *AudioHdmiI2S) = 0;

        /**
        * a function fo SetHdmiI2SEnable
        * @param Enable
        * @return status_t
        */
        virtual status_t SetHdmiI2SEnable(bool Enable) = 0;

        /**
        * a function fo SetHdmiClockSource
        * @param FsType
        * @return status_t
        */
        virtual status_t SetHdmiClockSource(uint32 FsType) = 0;

        /**
        * a function fo SetHdmiBckDiv
        * @param FsType
        * @return status_t
        */
        virtual status_t SetHdmiBckDiv(uint32 FsType) = 0;

        /**
        * a function fo SetHdmiFrameSize
        * @param FrameSize
        * @return status_t
        */
        virtual status_t SetHdmiFrameSize(uint32 FrameSize) = 0;

};

}
#endif
