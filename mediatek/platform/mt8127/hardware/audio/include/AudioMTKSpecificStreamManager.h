#ifndef AUDIO_MTK_SPECIFIC_STREAM_MANAGER_H
#define AUDIO_MTK_SPECIFIC_STREAM_MANAGER_H

#include "AudioMTKStreamManager.h"

namespace android
{

class AudioMTKSpecificStreamManager : public AudioMTKStreamManager
{
    public:
        /**
        * virtual destrutor
        */
        virtual ~AudioMTKSpecificStreamManager() {};

        /** This method creates and opens the audio hardware output stream */
        virtual android_audio_legacy::AudioStreamOut *openOutputStream(
            uint32_t devices,
            int *format = 0,
            uint32_t *channels = 0,
            uint32_t *sampleRate = 0,
            status_t *status = 0,
            uint32_t output_flag = 0);

        /**
        * do closeOutputStream
        * @return status_t*/
        virtual status_t closeOutputStream(android_audio_legacy::AudioStreamOut *out);

        virtual status_t ForceAllStandby();

        virtual status_t SetOutputStreamSuspend(bool bEnable);

        static AudioMTKSpecificStreamManager *createInstance();

    protected:
        AudioMTKSpecificStreamManager();

    private:
        status_t enableVectorLock(int timeout = 0);
        void disableVectorLock(void);
        KeyedVector<uint32_t, android_audio_legacy::AudioStreamOut *> mHdmiStreamOutVector;
        AudioLock mStreamOutVectorLock;

};

}

#endif
