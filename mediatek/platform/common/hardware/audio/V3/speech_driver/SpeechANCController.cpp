#include "SpeechANCController.h"
#include "AudioALSAStreamManager.h"
#include "audio_custom_exp.h"
#include "AudioCustParam.h"
#include "AudioType.h"
#include "AudioALSAHardware.h"
#include <pthread.h>
#include <linux/rtpm_prio.h>
#include <sys/prctl.h>

#define LOG_TAG "SpeechANCController"
#define param_anc_add
namespace android
{

static const char     kPrefixOfANCFileName[] = "/sdcard/mtklog/audio_dump/ANCLog";
static const uint32_t kSizeOfPrefixOfANCFileName = sizeof(kPrefixOfANCFileName) - 1;

static const uint32_t kMaxSizeOfANCFileName = 128;

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

SpeechANCController *SpeechANCController::UniqueSpeechANCController = NULL;


SpeechANCController *SpeechANCController::getInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);
    ALOGD("%s()", __FUNCTION__);

    if (UniqueSpeechANCController == NULL)
    {
        UniqueSpeechANCController = new SpeechANCController();
    }
    ASSERT(UniqueSpeechANCController != NULL);
    return UniqueSpeechANCController;
}
/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechANCController::SpeechANCController()
{
    ALOGD("%s()", __FUNCTION__);
    mEnabled       = false;
    mGroupANC      = false;
#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
    Init();
#endif
}

SpeechANCController::~SpeechANCController()
{
    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
    if (mFd)
    {
        ::close(mFd);
        mFd = 0;
    }
#endif
}

/*==============================================================================
 *                     AudioANCControl Imeplementation
 *============================================================================*/
bool SpeechANCController::GetANCSupport(void)
{
    ALOGD("%s(), GetANCSupport:%d", __FUNCTION__, mApply);
    //TODO(Tina): return by project config

#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
    return true;
#else
    return false;
#endif

}

#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)

void SpeechANCController::Init()
{
    ALOGD("%s()", __FUNCTION__);
    mFd            = ::open(kANCDeviceName, O_RDWR);
    if (mFd < 0)
    {
        ALOGE("%s() fail to open %s", __FUNCTION__, kANCDeviceName);
    }
    else
    {
        ALOGD("%s() open %s success!", __FUNCTION__, kANCDeviceName);

        ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Init);
    }
#ifdef param_anc_add
    AUDIO_ANC_CUSTOM_PARAM_STRUCT pSphParamAnc;
    Mutex::Autolock _l(mMutex);
    GetANCSpeechParamFromNVRam(&pSphParamAnc);
    mLogEnable     = pSphParamAnc.ANC_log;
    mLogDownSample = pSphParamAnc.ANC_log_downsample;
    mApply         = pSphParamAnc.ANC_apply;

    SetCoefficients(pSphParamAnc.ANC_para);
#else
    mLogEnable     = false;
    mLogDownSample = false;
    mApply         = false;

#endif
}


void SpeechANCController::SetCoefficients(void *buf)
{
    ALOGD("%s(), SetCoefficients:%d", __FUNCTION__);
    ::ioctl(mFd, SET_ANC_PARAMETER, buf);
}

void SpeechANCController::SetApplyANC(bool apply)
{
    //if mmi selected, set flag and enable/disable anc
    ALOGD("%s(), SetApply:%d", __FUNCTION__, apply);

    if (apply ^ mApply)
    {
#ifdef param_anc_add
        AUDIO_ANC_CUSTOM_PARAM_STRUCT pSphParamAnc;
        Mutex::Autolock _l(mMutex);
        GetANCSpeechParamFromNVRam(&pSphParamAnc);
        pSphParamAnc.ANC_apply = apply;
        SetANCSpeechParamToNVRam(&pSphParamAnc);
#endif
        mApply = apply;

    }

}


bool SpeechANCController::GetApplyANC(void)
{
    //get compile option and return
    ALOGD("%s(), mApply:%d", __FUNCTION__, mApply);
    return mApply;
}

void SpeechANCController::SetEanbleANCLog(bool enable, bool downsample)
{
    ALOGD("%s(), enable:%d, downsample(%d)", __FUNCTION__, enable, downsample);
    if (enable ^ mLogEnable || mLogDownSample ^ downsample)
    {
#ifdef param_anc_add
        AUDIO_ANC_CUSTOM_PARAM_STRUCT pSphParamAnc;
        Mutex::Autolock _l(mMutex);
        GetANCSpeechParamFromNVRam(&pSphParamAnc);
        pSphParamAnc.ANC_log = enable;
        pSphParamAnc.ANC_log_downsample = downsample;
        SetANCSpeechParamToNVRam(&pSphParamAnc);
#endif
        mLogEnable = enable;
        mLogDownSample = downsample;
        if (enable)
        {
            ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_EnableLog);
        }
        else
        {
            ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_DisableLog);
        }
    }
}

bool SpeechANCController::GetEanbleANCLog(void)
{
    ALOGD("%s(), mLogEnable:%d", __FUNCTION__, mLogEnable);
    return mLogEnable;
}

bool SpeechANCController::GetEanbleANCLogDownSample(void)
{
    ALOGD("%s(), mLogDownSample:%d", __FUNCTION__, mLogDownSample);
    return mLogDownSample;
}

bool SpeechANCController::EanbleANC(bool enable)
{
    int ret;

    ALOGD("%s(), mEnabled(%d), enable(%d)", __FUNCTION__, mEnabled, enable);


    if (!mGroupANC)
    {
        ALOGD("%s(), EnableError, Not ANC group", __FUNCTION__);
        return false;
    }
    if (enable ^ mEnabled)
    {
        Mutex::Autolock _l(mMutex);
        if (enable)
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Enable);
        }
        else
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Disable);
        }
        if (ret == -1)
        {
            ALOGD("%s(), EnableFail:%d", __FUNCTION__, ret);
            return false;
        }
        mEnabled = enable;
    }
    return true;
}

bool SpeechANCController::CloseANC(void)
{
    int ret;
    ALOGD("%s()", __FUNCTION__);
    if (!mGroupANC)
    {
        ALOGD("%s(), CloseError, Not ANC group", __FUNCTION__);
        return false;
    }
    Mutex::Autolock _l(mMutex);
    ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Close);
    if (ret == -1)
    {
        ALOGD("%s(), EnableFail:%d", __FUNCTION__, ret);
        return false;
    }
    mEnabled = false;
    return true;
}

bool SpeechANCController::SwapANC(bool swap2anc)
{
    int ret;
    ALOGD("%s(), mGroupANC(%d), swap2anc(%d)", __FUNCTION__, mGroupANC, swap2anc);
    if (mGroupANC ^ swap2anc)
    {
        if (swap2anc)
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_SwapToANC);
        }
        else
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_SwapFromANC);
        }

#if 0 //tina test       
        mGroupANC = swap2anc;
#endif

        if (ret == -1)
        {
            ALOGD("%s(), SWAPFail:%d", __FUNCTION__, ret);
            return false;
        }
        mGroupANC = swap2anc;
    }
    return true;
}

status_t SpeechANCController::OpenFile(FILE *mDumpFile, int mType_ANC)
{
    char ANC_file_path[kMaxSizeOfANCFileName];
    memset((void *)ANC_file_path, 0, kMaxSizeOfANCFileName);

    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strcpy(ANC_file_path, kPrefixOfANCFileName);
    sprintf(ANC_file_path, "%s%d", kPrefixOfANCFileName, mType_ANC);
    if (mType_ANC == 5)
    {
        strftime(ANC_file_path + kSizeOfPrefixOfANCFileName, kMaxSizeOfANCFileName - kSizeOfPrefixOfANCFileName - 1, "_%Y_%m_%d_%H%M%S.log", timeinfo);
    }
    else
    {
        strftime(ANC_file_path + kSizeOfPrefixOfANCFileName, kMaxSizeOfANCFileName - kSizeOfPrefixOfANCFileName - 1, "_%Y_%m_%d_%H%M%S.pcm", timeinfo);
    }
    ALOGD("%s(), ANC_file_path: \"%s\"", __FUNCTION__, ANC_file_path);

    // check vm_file_path is valid
    int ret = AudiocheckAndCreateDirectory(ANC_file_path);
    if (ret < 0)
    {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ANC_file_path);
        return UNKNOWN_ERROR;
    }

    // open VM file
    mDumpFile = fopen(ANC_file_path, "wb");
    if (mDumpFile == NULL)
    {
        ALOGE("%s(), fopen(%s) fail!!", __FUNCTION__, ANC_file_path);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}


//call by speech driver
bool SpeechANCController::StartANCLog()
{    
    if (!mGroupANC)
    {
        ALOGD("%s(), EnableError, Not ANC group", __FUNCTION__);
        return false;
    }

    ALOGD("%s()", __FUNCTION__);
    if (mLogEnable)
    {
        // create 5 reading thread

        //ANC Log1
        mEnable_ANCLog1 = true;
        int ret = pthread_create(&hReadThread_ANCLog1, NULL, SpeechANCController::readThread_ANCLog1, (void *)this);
        if (ret != 0)
        {
            ALOGE("%s() create thread fail!!", __FUNCTION__);
            return UNKNOWN_ERROR;
        }

        //ANC Log5: MD32 coefficients
        mEnable_ANCLog5 = true;
        int ret5 = pthread_create(&hReadThread_ANCLog5, NULL, SpeechANCController::readThread_ANCLog5, (void *)this);
        if (ret5 != 0)
        {
            ALOGE("%s() create thread fail!!", __FUNCTION__);
            return UNKNOWN_ERROR;
        }

        OpenFile(mDumpFile1, 1);
       // OpenFile(mDumpFile2, 2);
       // OpenFile(mDumpFile3, 3);
       // OpenFile(mDumpFile4, 4);
        OpenFile(mDumpFile5, 5);

    }




    return true;
}


//call by speech driver
bool SpeechANCController::StopANCLog()
{
    int ret;
    if (!mGroupANC)
    {
        ALOGD("%s(), EnableError, Not ANC group", __FUNCTION__);
        return false;
    }

    ALOGD("%s()", __FUNCTION__);
    if (mLogEnable)
    {
        // exist 5 reading threads
        mEnable_ANCLog1 = false;
        mEnable_ANCLog2 = false;
        mEnable_ANCLog3 = false;
        mEnable_ANCLog4 = false;
        mEnable_ANCLog5 = false;
    }

    return true;
}


void *SpeechANCController::readThread_ANCLog1(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    SpeechANCController *pSpeechANCController = (SpeechANCController *)arg;

    // force to set priority
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_RECORD + 1;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
    {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    }
    else
    {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }
    ALOGD("+%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;
    int format = AUDIO_FORMAT_PCM_16_BIT;
    uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
    uint32_t sampleRate = 16000;
    status_t status = 0;
    ssize_t bytes_read, bytes, write_bytes = 0;

    status_t retval = NO_ERROR;
    AudioALSAStreamIn *StreamInANCLog1 = (AudioALSAStreamIn *)AudioALSAStreamManager::getInstance() ->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
    StreamInANCLog1->setParameters(String8("input_source=97"));//AUDIO_SOURCE_ANC
    StreamInANCLog1->open();

    // read raw data from stream manager
    char *buffer = new char[1024];

    memset((void *)&buffer, 0, sizeof(buffer));

    while (pSpeechANCController->mEnable_ANCLog1 == true)
    {
        if (pSpeechANCController->mEnable_ANCLog1 == false)
        {
            break;
        }
        bytes_read = StreamInANCLog1->read(buffer, bytes);

        // write data to sd card
        write_bytes += fwrite((void *)buffer, sizeof(char), bytes_read, pSpeechANCController->mDumpFile1);

        if (retval != 0)
        {
            ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
        }

    }

    // close file
    if (pSpeechANCController->mDumpFile1 != NULL)
    {
        fflush(pSpeechANCController->mDumpFile1);
        fclose(pSpeechANCController->mDumpFile1);
        pSpeechANCController->mDumpFile1 = NULL;
    }
    StreamInANCLog1->close();

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}


void *SpeechANCController::readThread_ANCLog5(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    SpeechANCController *pSpeechANCController = (SpeechANCController *)arg;

    // force to set priority
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_RECORD + 1;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
    {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    }
    else
    {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }
    ALOGD("+%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    ssize_t bytes_read, bytes, write_bytes = 0;

    status_t retval = NO_ERROR;

    // read raw data from anc kernel driver
    char *buffer = new char[1024];
    memset((void *)&buffer, 0, sizeof(buffer));

    while (pSpeechANCController->mEnable_ANCLog5 == true)
    {
        if (pSpeechANCController->mEnable_ANCLog5 == false)
        {
            break;
        }
        //TODO: ANC KERNEL READ API
        //bytes_read, buffer

        // write data to sd card
        //write_bytes += fwrite((void *)buffer, sizeof(char), bytes_read, pSpeechANCController->mDumpFile5);

        if (retval != 0)
        {
            ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
        }

    }
    // close file
    if (pSpeechANCController->mDumpFile5 != NULL)
    {
        fflush(pSpeechANCController->mDumpFile5);
        fclose(pSpeechANCController->mDumpFile5);
        pSpeechANCController->mDumpFile5 = NULL;
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}


#endif

}







//namespace android
