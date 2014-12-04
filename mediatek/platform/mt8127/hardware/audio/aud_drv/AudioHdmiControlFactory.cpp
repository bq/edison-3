#include "AudioHdmiControlFactory.h"
#include "AudioHdmiControl.h"

namespace android
{

AudioHdmiControlInterface *AudioHdmiControlFactory::CreateAudioHdmiControl()
{
    AudioHdmiControlInterface *instance = AudioHdmiControl::getInstance();
    return instance;
}

}