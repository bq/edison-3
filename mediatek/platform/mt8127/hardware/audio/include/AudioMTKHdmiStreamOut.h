#ifndef AUDIO_MTK_HDMI_STREAM_OUT_H
#define AUDIO_MTK_HDMI_STREAM_OUT_H

#include <hardware_legacy/AudioHardwareInterface.h>
#include "AudioStreamAttribute.h"
#include "AudioResourceManagerInterface.h"
#include "AudioDigitalControlFactory.h"
#include "AudioHdmiControlFactory.h"
#include "hdmitx.h"

namespace android
{

class AudioMTKHdmiStreamOut : public android_audio_legacy::AudioStreamOut
{
    public:
        virtual ~AudioMTKHdmiStreamOut();

        /** return audio sampling rate in hz - eg. 44100 */
        virtual uint32_t sampleRate() const;

        /** returns size of output buffer - eg. 4800 */
        virtual size_t bufferSize() const;

        /** returns the output channel mask */
        virtual uint32_t channels() const;

        /**
         * return audio format in 8bit or 16bit PCM format -
         * eg. AudioSystem:PCM_16_BIT
         */
        virtual int format() const;

        /**
         * return the frame size (number of bytes per sample).
         */
        uint32_t frameSize() const
        {
            return popcount(channels()) * ((format() == AUDIO_FORMAT_PCM_16_BIT) ? sizeof(int16_t) : sizeof(int8_t));
        }

        /**
         * return the audio hardware driver latency in milli seconds.
         */
        virtual uint32_t latency() const;

        /**
         * Use this method in situations where audio mixing is done in the
         * hardware. This method serves as a direct interface with hardware,
         * allowing you to directly set the volume as apposed to via the framework.
         * This method might produce multiple PCM outputs or hardware accelerated
         * codecs, such as MP3 or AAC.
         */
        virtual status_t setVolume(float left, float right);

        /** write audio buffer to driver. Returns number of bytes written */
        virtual ssize_t write(const void *buffer, size_t bytes);

        /**
         * Put the audio hardware output into standby mode. Returns
         * status based on include/utils/Errors.h
         */
        virtual status_t standby();

        /** dump the state of the audio output device */
        virtual status_t dump(int fd, const Vector<String16> &args);

        // set/get audio output parameters. The function accepts a list of parameters
        // key value pairs in the form: key1=value1;key2=value2;...
        // Some keys are reserved for standard parameters (See AudioParameter class).
        // If the implementation does not accept a parameter change while the output is
        // active but the parameter is acceptable otherwise, it must return INVALID_OPERATION.
        // The audio flinger will put the output in standby and then change the parameter value.
        virtual status_t setParameters(const String8 &keyValuePairs);
        virtual String8 getParameters(const String8 &keys);

        // return the number of audio frames written by the audio dsp to DAC since
        // the output has exited standby
        virtual status_t getRenderPosition(uint32_t *dspFrames);

        virtual status_t getPresentationPosition(uint64_t *frames, struct timespec *timestamp);

        bool getStreamRunning(void);

        status_t forceStandby(void);

        status_t internalStandby(void);

        status_t setSuspend(bool suspend);

        // static method
        static android_audio_legacy::AudioStreamOut *createAudioStreamOut(uint32_t devices, int *format, uint32_t *channels,
                                                                          uint32_t *sampleRate, status_t *status, uint32_t output_flag);
        static void destroyAudioStreamOut(android_audio_legacy::AudioStreamOut *stream_out);

    private:
        // private declaration
        enum HDMI_OUTPUT_TYPE
        {
            HDMI_STEREO = 0,
            HDMI_MULTI_CH,
            HDMI_OUTPUT_CNT
        };

        // private method
        AudioMTKHdmiStreamOut(uint32_t devices, int *format, uint32_t *channels,
                              uint32_t *sampleRate, status_t *status, uint32_t output_flag);

        void setStreamRunning(bool enable);

        status_t enableStreamLock(int timeout = 0);

        void disableStreamLock(void);

        void requestPlaybackClock(void);

        void releasePlaybackClock(void);

        void checkSuspendOutput(void);

        void forceHDMIStereoStandy(void);

        void outputStandby(void);

        status_t enableMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode, bool Enable);

        void turnOnAfeHdmi(void);

        void turnOffAfeHdmi(void);

        status_t setHdmiOutControl(void);

        status_t setHdmiOutControlEnable(bool Enable);

        status_t setHdmiBckDiv(void);

        status_t setHdmiI2SAttribute(void);

        status_t setHdmiI2SEnable(bool Enable);

        void setHdmiInterConnection(uint32_t ConnectionState);

        size_t writeDataToKernel(const void *buffer, size_t bytes);

        void openPcmDumpFile(void);

        void closePcmDumpFile(void);

        uint32_t dataToDurationUs(size_t bytes);

        void prepareHdmiAudioParam(void);

        void notifyAudioSettingToHdmiTx(void);

        // static method
        static status_t enableStreamOutsLock(int timeout = 0);
        static void disableStreamOutsLock(void);
        static HDMI_OUTPUT_TYPE outputFlagToOutputType(uint32_t output_flag);
        static int sampleRateToHdmiFsType(uint32_t SampleRate);
        static int sampleRateToSinkSupportType(uint32_t SampleRate);
        static int channelMaskTChannelCount(uint32_t ChannelMask);
        static bool hasValidStreamOut(void);
        static void queryEdidInfo(void);
        static void clearEdidInfo(void);
        static bool isSinkSupportedFormat(uint32_t CodecFormat, uint32_t SampleRate = 0, uint32_t ChannelCount = 0);
        static void dumpSinkCapability();

        // private member
        AudioResourceManagerInterface *mAudioResourceManager;
        AudioDigitalControlInterface *mAudioDigitalControl;
        AudioHdmiControlInterface *mAudioHdmiControl;
        uint32_t mLatency;
        uint32_t mChannelMask;
        bool mStarting;
        int mExternalSuspend;
        bool mInternalSuspend;
        HDMI_OUTPUT_TYPE mOutputType;
        int mFd;
        int mHdmiFsType;
        int mHdmiMemType;
        AudioStreamAttribute mDLAttribute;
        AudioLock mStreamLock;
        Vector<String8> mSupportedChannelMasks;
        HDMITX_AUDIO_PARA mHdmiAudioParam;
        FILE *mPcmDumpFile;
        uint64_t mPresentedBytes;
        timespec mPresentedTime;

        // static member
        static AudioMTKHdmiStreamOut *mStreamOuts[HDMI_OUTPUT_CNT];
        static int mDumpFileNum[HDMI_OUTPUT_CNT];
        static AudioLock mStreamOutsLock;
        static uint32_t mHdmiSinkSupportCodec;
        // <channel, samplerate>
        static KeyedVector<uint32_t, uint32_t> mHdmiSinkPcmSupportProfiles;
};

}

#endif
