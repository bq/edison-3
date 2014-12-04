#ifndef AUDIO_HDMI_CONTROL_FACTORY_H
#define AUDIO_HDMI_CONTROL_FACTORY_H

#include "AudioHdmiControlInterface.h"

namespace android
{

class AudioHdmiControlFactory
{
    public:
        static AudioHdmiControlInterface *CreateAudioHdmiControl();
};

}
#endif
