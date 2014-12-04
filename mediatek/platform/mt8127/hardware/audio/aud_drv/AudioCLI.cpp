#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "AudioCLI"
///#define LOG_NDEBUG 0
#define MT8135_AUD_REG

#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <media/AudioSystem.h>
#include <media/IAudioPolicyService.h>
#include <math.h>

#include <system/audio.h>

#include <media/AudioParameter.h>
#ifndef ANDROID_DEFAULT_CODE
#include <cutils/xlog.h>
#ifdef MTK_AUDIO
#include <AudioIoctl.h>
#endif
#endif

#include <utils/String8.h>
#include "AudioMTKPolicyManager.h"
#include "AudioAfeReg.h"
#include "AudioAnalogReg.h"
#include "LoopbackManager.h"

//for HW testing tool
#include "AudioMeta.h"



typedef struct
{
    int id;
    char *name;
} strIdName;

strIdName StreamIdNameTable[] =
{
    {AUDIO_STREAM_VOICE_CALL, "VOICE_CALL"},
    {AUDIO_STREAM_SYSTEM, "SYSTEM"},
    {AUDIO_STREAM_RING, "RING"},
    {AUDIO_STREAM_MUSIC, "MUSIC"},
    {AUDIO_STREAM_ALARM, "ALARM"},
    {AUDIO_STREAM_NOTIFICATION, "NOTIFICATION"},
    {AUDIO_STREAM_BLUETOOTH_SCO, "BLUETOOTH_SCO"},
    {AUDIO_STREAM_ENFORCED_AUDIBLE, "ENFORCED_AUDIBLE"},
    {AUDIO_STREAM_DTMF, "DTMF"},
    {AUDIO_STREAM_TTS, "TTS"},
    {AUDIO_STREAM_FM, "FM"},
    {AUDIO_STREAM_MATV, "MATV"},
    {AUDIO_STREAM_BOOT, "BOOT"}
};

void list_audio_stream_type(void)
{
    printf("[audio_stream_type_t]\n");
    printf("  AUDIO_STREAM_DEFAULT=%d\n", AUDIO_STREAM_DEFAULT);
    printf("  AUDIO_STREAM_VOICE_CALL=%d\n", AUDIO_STREAM_VOICE_CALL);
    printf("  AUDIO_STREAM_SYSTEM=%d\n", AUDIO_STREAM_SYSTEM);
    printf("  AUDIO_STREAM_RING=%d\n", AUDIO_STREAM_RING);
    printf("  AUDIO_STREAM_MUSIC=%d\n", AUDIO_STREAM_MUSIC);
    printf("  AUDIO_STREAM_ALARM=%d\n", AUDIO_STREAM_ALARM);
    printf("  AUDIO_STREAM_NOTIFICATION=%d\n", AUDIO_STREAM_NOTIFICATION);
    printf("  AUDIO_STREAM_BLUETOOTH_SCO=%d\n", AUDIO_STREAM_BLUETOOTH_SCO);
    printf("  AUDIO_STREAM_ENFORCED_AUDIBLE=%d\n", AUDIO_STREAM_ENFORCED_AUDIBLE);
    printf("  AUDIO_STREAM_DTMF=%d\n", AUDIO_STREAM_DTMF);
    printf("  AUDIO_STREAM_TTS=%d\n", AUDIO_STREAM_TTS);
    printf("  AUDIO_STREAM_FM=%d\n", AUDIO_STREAM_FM);
    printf("  AUDIO_STREAM_MATV=%d\n", AUDIO_STREAM_MATV);
    printf("  AUDIO_STREAM_BOOT=%d\n", AUDIO_STREAM_BOOT);
    printf("  AUDIO_STREAM_CNT=%d\n", AUDIO_STREAM_CNT);
    printf("  AUDIO_STREAM_MAX=%d\n", AUDIO_STREAM_MAX);
    printf("[end]\n");
}


strIdName DeviceOutIdNameTable[] =
{
    {AUDIO_DEVICE_OUT_EARPIECE, "earpiece"},
    {AUDIO_DEVICE_OUT_SPEAKER, "speaker"},
    {AUDIO_DEVICE_OUT_WIRED_HEADSET, "wired_headset"},
    {AUDIO_DEVICE_OUT_WIRED_HEADPHONE, "wired_headphone"},
    {AUDIO_DEVICE_OUT_BLUETOOTH_SCO, "blue_sco"},
    {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET, "blue_sco_headset"},
    {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT, "blue_sco_carkit"},
    {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP, "blue_a2dp"},
    {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES, "blue_a2dp_headphone"},
    {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER, "blue_a2dp_speaker"},
    {AUDIO_DEVICE_OUT_AUX_DIGITAL, "aux_digital"},
    {AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET, "anlg_dock_headset"},
    {AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET, "dgtl_dock_headset"},
    {AUDIO_DEVICE_OUT_USB_ACCESSORY, "usb_accessory"},
    {AUDIO_DEVICE_OUT_USB_DEVICE, "usb_device"},
    {AUDIO_DEVICE_OUT_DEFAULT, "default"},
};

char *getDeviceOutName(int device)
{
    int i;
    for (i = 0; i < sizeof(DeviceOutIdNameTable) / sizeof(strIdName); i++)
    {
        if (DeviceOutIdNameTable[i].id == device)
        {
            return DeviceOutIdNameTable[i].name;
        }
    }
    return NULL;
}

void list_audio_output_devices(void)
{
    printf("[audio_devices_t output]\n");
    printf("  AUDIO_DEVICE_OUT_EARPIECE = 0x%x\n", AUDIO_DEVICE_OUT_EARPIECE);
    printf("  AUDIO_DEVICE_OUT_SPEAKER = 0x%x\n", AUDIO_DEVICE_OUT_SPEAKER);
    printf("  AUDIO_DEVICE_OUT_WIRED_HEADSET = 0x%x\n", AUDIO_DEVICE_OUT_WIRED_HEADSET);
    printf("  AUDIO_DEVICE_OUT_WIRED_HEADPHONE = 0x%x\n", AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
    printf("  AUDIO_DEVICE_OUT_BLUETOOTH_SCO = 0x%x\n", AUDIO_DEVICE_OUT_BLUETOOTH_SCO);
    printf("  AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET = 0x%x\n", AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET);
    printf("  AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT = 0x%x\n", AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT);
    printf("  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP = 0x%x\n", AUDIO_DEVICE_OUT_BLUETOOTH_A2DP);
    printf("  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES = 0x%x\n", AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES);
    printf("  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER = 0x%x\n", AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER);
    printf("  AUDIO_DEVICE_OUT_AUX_DIGITAL = 0x%x\n", AUDIO_DEVICE_OUT_AUX_DIGITAL);
    printf("  AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET = 0x%x\n", AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET);
    printf("  AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET = 0x%x\n", AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET);
    printf("  AUDIO_DEVICE_OUT_USB_ACCESSORY = 0x%x\n", AUDIO_DEVICE_OUT_USB_ACCESSORY);
    printf("  AUDIO_DEVICE_OUT_USB_DEVICE = 0x%x\n", AUDIO_DEVICE_OUT_USB_DEVICE);
    printf("  AUDIO_DEVICE_OUT_DEFAULT = 0x%x\n", AUDIO_DEVICE_OUT_DEFAULT);
    printf("[end]\n");
}

void list_audio_input_devices(void)
{
    printf("[audio_devices_t input]\n");
    printf("  AUDIO_DEVICE_IN_COMMUNICATION = 0x%x\n", AUDIO_DEVICE_IN_COMMUNICATION);
    printf("  AUDIO_DEVICE_IN_AMBIENT = 0x%x\n", AUDIO_DEVICE_IN_AMBIENT);
    printf("  AUDIO_DEVICE_IN_BUILTIN_MIC = %x\n", AUDIO_DEVICE_IN_BUILTIN_MIC);
    printf("  AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET = 0x%x\n", AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET);
    printf("  AUDIO_DEVICE_IN_WIRED_HEADSET = 0x%x\n", AUDIO_DEVICE_IN_WIRED_HEADSET);
    printf("  AUDIO_DEVICE_IN_AUX_DIGITAL = 0x%x\n", AUDIO_DEVICE_IN_AUX_DIGITAL);
    printf("  AUDIO_DEVICE_IN_VOICE_CALL = 0x%x\n", AUDIO_DEVICE_IN_VOICE_CALL);
    printf("  AUDIO_DEVICE_IN_BACK_MIC = 0x%x\n", AUDIO_DEVICE_IN_BACK_MIC);
    printf("  AUDIO_DEVICE_IN_FM = 0x%x\n", AUDIO_DEVICE_IN_FM);
    printf("  AUDIO_DEVICE_IN_DEFAULT = 0x%x\n", AUDIO_DEVICE_IN_DEFAULT);
    printf("[end]\n");
}

void list_audio_devices(void)
{
    list_audio_output_devices();
    list_audio_input_devices();
}



void list_getStreamVolumeIndex(int device)
{
    int i;
    int index;
    char *device_name = getDeviceOutName(device);

    if (device_name)
    {
        printf("[test AudioSystem::getStreamVolumeIndex device=0x%x(%s)]\n", device, device_name);
        for (i = AUDIO_STREAM_VOICE_CALL; i < AUDIO_STREAM_CNT; i++)
        {
            android::AudioSystem::getStreamVolumeIndex((audio_stream_type_t)i, &index, (audio_devices_t)device);
            printf("  stream: %s volume_index=%d\n", StreamIdNameTable[i].name, index);
        }
        printf("[end]\n");
    }
    else
    {
        printf("unsuppored device=0x%x", device);
    }
}


void list_output_status(void)
{
    uint32_t sample_rate;
    uint32_t frame_count;
    uint32_t latency;
    printf("[output status]\n");
    android::AudioSystem::getOutputSamplingRate(&sample_rate, AUDIO_STREAM_DEFAULT);
    android::AudioSystem::getOutputFrameCount(&frame_count, AUDIO_STREAM_DEFAULT);
    android::AudioSystem::getOutputLatency(&latency, AUDIO_STREAM_DEFAULT);
    printf("  sample_rate: %d\n", sample_rate);
    printf("  frame_count: %d\n", frame_count);
    printf("  latency: %d\n", latency);
    printf("[end]\n");
}


void list_output_device_status(void)
{
    int i;
    audio_policy_dev_state_t state;
    char device_addr[256] = {0};

    printf("[output device status]\n");
    for (i = 0; i < sizeof(DeviceOutIdNameTable) / sizeof(strIdName); i++)
    {
        state = android::AudioSystem::getDeviceConnectionState((audio_devices_t)DeviceOutIdNameTable[i].id, device_addr);
        printf("  %s: %d\n", DeviceOutIdNameTable[i].name, state);
    }

    printf("[end]\n");
}


strIdName ForcedConfigTable[] =
{
    {AUDIO_POLICY_FORCE_NONE, "FORCE_NONE"},
    {AUDIO_POLICY_FORCE_SPEAKER, "FORCE_SPEAKER"},
    {AUDIO_POLICY_FORCE_HEADPHONES, "FORCE_HEADPHONES"},
    {AUDIO_POLICY_FORCE_BT_SCO, "FORCE_BT_SCO"},
    {AUDIO_POLICY_FORCE_BT_A2DP, "FORCE_BT_A2DP"},
    {AUDIO_POLICY_FORCE_WIRED_ACCESSORY, "FORCE_WIRED_ACCESSORY"},
    {AUDIO_POLICY_FORCE_BT_CAR_DOCK, "FORCE_BT_CAR_DOCK"},
    {AUDIO_POLICY_FORCE_BT_DESK_DOCK, "FORCE_BT_DESK_DOCK"},
    {AUDIO_POLICY_FORCE_ANALOG_DOCK, "FORCE_ANALOG_DOCK"},
    {AUDIO_POLICY_FORCE_DIGITAL_DOCK, "FORCE_DIGITAL_DOCK"},
    {AUDIO_POLICY_FORCE_NO_BT_A2DP, "FORCE_NO_BT_A2DP"},
    {AUDIO_POLICY_FORCE_SYSTEM_ENFORCED, "FORCE_SYSTEM_ENFORCED"},
    {AUDIO_POLICY_FORCE_NO_SYSTEM_ENFORCED, "FORCE_NO_SYSTEM_ENFORCED"},
};

strIdName ForcedUseTable[] =
{
    {AUDIO_POLICY_FORCE_FOR_COMMUNICATION, "FOR_COMMUNICATION"},
    {AUDIO_POLICY_FORCE_FOR_MEDIA, "FOR_MEDIA"},
    {AUDIO_POLICY_FORCE_FOR_RECORD, "FOR_RECORD"},
    {AUDIO_POLICY_FORCE_FOR_DOCK, "FOR_DOCK"},
    {AUDIO_POLICY_FORCE_FOR_SYSTEM, "FOR_SYSTEM"},
    {AUDIO_POLICY_FORCE_FOR_PROPRIETARY, "FOR_PROPRIETARY"},
};

void list_forced_use_status(void)
{
    int i;
    int forced_config;
    printf("[forced use status]\n");
    for (i = 0; i < sizeof(ForcedUseTable) / sizeof(strIdName); i++)
    {
        forced_config = (int)android::AudioSystem::getForceUse((audio_policy_force_use_t)i);
        printf("  %s: %s\n", ForcedUseTable[i].name, ForcedConfigTable[forced_config].name);
    }
    printf("[end]\n");
}


void list_get_output_status(void)
{
    int i;
    int output;
    printf("[get output status]\n");
    for (i = 0; i < sizeof(StreamIdNameTable) / sizeof(strIdName); i++)
    {
        output = android::AudioSystem::getOutput((audio_stream_type_t)i);
        printf("  %s: %d\n", StreamIdNameTable[i].name, output);
    }
    printf("[end]\n");
}


char *StrategyNameTable[] =
{
    "STRATEGY_MEDIA",
    "STRATEGY_PHONE",
    "STRATEGY_SONIFICATION",
    "STRATEGY_SONIFICATION_RESPECTFUL",
    "STRATEGY_DTMF",
    "STRATEGY_ENFORCED_AUDIBLE",
    "STRATEGY_PROPRIETARY"
};

void list_routing_strategy(void)
{
    int i;
    printf("[routing_strategy]\n");
    for (i = 0; i < sizeof(StrategyNameTable) / sizeof(char *); i++)
    {
        printf("  %s: %d\n", StrategyNameTable[i], i);
    }
    printf("[end]\n");
}

void list_routing_strategy_by_stream(void)
{
    int i;
    int strategy;
    printf("[routing_strategy by stream]\n");
    for (i = 0; i < sizeof(StreamIdNameTable) / sizeof(strIdName); i++)
    {
        strategy = (int)android::AudioSystem::getStrategyForStream((audio_stream_type_t)i);
        printf("  %s: %s\n", StreamIdNameTable[i].name, StrategyNameTable[strategy]);
    }
    printf("[end]\n");

}

void list_device_by_stream(void)
{
    int i, j;
    int device;
    printf("[device by stream]\n");
    for (i = 0; i < sizeof(StreamIdNameTable) / sizeof(strIdName); i++)
    {
        device = (int)android::AudioSystem::getDevicesForStream((audio_stream_type_t)i);
        printf("  %s:", StreamIdNameTable[i].name);
        for (j = 0; j < sizeof(DeviceOutIdNameTable) / sizeof(strIdName); j++)
        {
            if (device & DeviceOutIdNameTable[j].id)
            {
                printf(" %s", getDeviceOutName(DeviceOutIdNameTable[j].id));
            }
        }
        printf("\n");
    }
    printf("[end]\n");
}


strIdName AudioSourceIdNameTable[] =
{
    {AUDIO_SOURCE_DEFAULT, "default"},
    {AUDIO_SOURCE_MIC, "mic"},
    {AUDIO_SOURCE_VOICE_UPLINK, "voice_uplink"},
    {AUDIO_SOURCE_VOICE_DOWNLINK, "voice_downlink"},
    {AUDIO_SOURCE_VOICE_CALL, "voice_call"},
    {AUDIO_SOURCE_CAMCORDER, "camcorder"},
    {AUDIO_SOURCE_VOICE_RECOGNITION, "voice_recognition"},
    {AUDIO_SOURCE_VOICE_COMMUNICATION, "voice_communication"},
    {AUDIO_SOURCE_MATV, "matv"},
    {AUDIO_SOURCE_FM, "fm"},
};

void list_audio_source(void)
{
    int i;
    printf("[audio_source_t]\n");
    for (i = 0; i < sizeof(AudioSourceIdNameTable) / sizeof(strIdName); i++)
    {
        printf("  %s: %d\n", AudioSourceIdNameTable[i].name, AudioSourceIdNameTable[i].id);
    }
    printf("[end]\n");
}


void list_get_input_status(void)
{
    int i;
    int input;
    printf("[get input status]\n");
    for (i = 0; i < sizeof(AudioSourceIdNameTable) / sizeof(strIdName); i++)
    {
        input = (int)android::AudioSystem::getInput((audio_source_t)AudioSourceIdNameTable[i].id, 0, AUDIO_FORMAT_DEFAULT, (audio_channel_mask_t)0, 0);
        printf("  %s(%d): %d\n", AudioSourceIdNameTable[i].name, AudioSourceIdNameTable[i].id, input);
    }
    printf("[end]\n");
}

void dump_audio_policy(void)
{
    printf("================================================================================\n");
    android::AudioSystem::setParameters(0, android::String8("AudioPolicyDump=1"));
    {
        FILE *fp = NULL;
        char buf[256];
        fp = fopen("/sdcard/audio_policy.dump", "rt");
        if (fp)
        {
            fgets(buf, sizeof(buf), fp);
            while (!feof(fp))
            {
                printf("%s", buf);
                fgets(buf, sizeof(buf), fp);
            }
            fclose(fp);
        }
    }
    printf("================================================================================\n");
}

void dump_audio_flinger(void)
{
    printf("================================================================================\n");
    android::AudioSystem::setParameters(0, android::String8("AudioFlingerDump=1"));
    {
        FILE *fp = NULL;
        char buf[256];
        fp = fopen("/sdcard/audio_flinger.dump", "rt");
        if (fp)
        {
            fgets(buf, sizeof(buf), fp);
            while (!feof(fp))
            {
                printf("%s", buf);
                fgets(buf, sizeof(buf), fp);
            }
            fclose(fp);
        }
    }
    printf("================================================================================\n");
}

typedef struct
{
    int bit_addr;
    char *name;
} rConnBitName;

typedef struct
{
    int reg_addr;
    rConnBitName bit_name[32];
} rConnTable;

#define CONN_TAB_NUM 8

rConnTable conn_table[CONN_TAB_NUM] =
{
    {
        AFE_CONN0,
        {   {31, "I08_O01_R"},
            {30, "I07_O01_R"},
            {29, "I06_O01_R"},
            {28, "I05_O01_R"},
            {27, "I01_O01_R"},
            {26, "I00_O01_R"},
            {25, "I09_O01_S"},
            {24, "I08_O01_S"},
            {23, "I07_O01_S"},
            {22, "I06_O01_S"},
            {21, "I05_O01_S"},
            {20, "I04_O01_S"},
            {19, "I03_O01_S"},
            {18, "I02_O01_S"},
            {17, "I01_O01_S"},
            {16, "I00_O01_S"},
            {15, "I08_O00_R"},
            {14, "I07_O00_R"},
            {13, "I06_O00_R"},
            {12, "I05_O00_R"},
            {11, "I01_O00_R"},
            {10, "I00_O00_R"},
            {9, "I09_O00_S"},
            {8, "I08_O00_S"},
            {7, "I07_O00_S"},
            {6, "I06_O00_S"},
            {5, "I05_O00_S"},
            {4, "I04_O00_S"},
            {3, "I03_O00_S"},
            {2, "I02_O00_S"},
            {1, "I01_O00_S"},
            {0, "I00_O00_S"}
        }
    },
    {
        AFE_CONN1,
        {   {31, "I08_O03_R"},
            {30, "I07_O03_R"},
            {29, "I06_O03_R"},
            {28, "I05_O03_R"},
            {27, "I01_O03_R"},
            {26, "I00_O03_R"},
            {25, "I09_O03_S"},
            {24, "I08_O03_S"},
            {23, "I07_O03_S"},
            {22, "I06_O03_S"},
            {21, "I05_O03_S"},
            {20, "I04_O03_S"},
            {19, "I03_O03_S"},
            {18, "I02_O03_S"},
            {17, "I01_O03_S"},
            {16, "I00_O03_S"},
            {15, "I08_O02_R"},
            {14, "I07_O02_R"},
            {13, "I06_O02_R"},
            {12, "I05_O02_R"},
            {11, "I01_O02_R"},
            {10, "I00_O02_R"},
            {9, "I09_O02_S"},
            {8, "I08_O02_S"},
            {7, "I07_O02_S"},
            {6, "I06_O02_S"},
            {5, "I05_O02_S"},
            {4, "I04_O02_S"},
            {3, "I03_O02_S"},
            {2, "I02_O02_S"},
            {1, "I01_O02_S"},
            {0, "I00_O02_S"}
        }
    },
    {
        AFE_CONN2,
        {   {31, "I08_O08_S"},
            {30, "I06_O08_S"},
            {29, "I04_O08_S"},
            {28, "I07_O07_S"},
            {27, "I05_O07_S"},
            {26, "I03_O07_S"},
            {25, "I08_O06_S"},
            {24, "I06_O06_S"},
            {23, "I04_O06_S"},
            {22, "I01_O06_S"},
            {21, "I09_O05_S"},
            {20, "I07_O05_S"},
            {19, "I05_O05_S"},
            {18, "I03_O05_S"},
            {17, "I02_O05_S"},
            {16, "I00_O05_S"},
            {15, "I08_O04_R"},
            {14, "I07_O04_R"},
            {13, "I06_O04_R"},
            {12, "I05_O04_R"},
            {11, "I01_O04_R"},
            {10, "I00_O04_R"},
            {9, "I09_O04_S"},
            {8, "I08_O04_S"},
            {7, "I07_O04_S"},
            {6, "I06_O04_S"},
            {5, "I05_O04_S"},
            {4, "I04_O04_S"},
            {3, "I03_O04_S"},
            {2, "I02_O04_S"},
            {1, "I01_O04_S"},
            {0, "I00_O04_S"}
        }
    },
    {
        AFE_CONN3,
        {   {31, "I16_O03_R"},
            {30, "I15_O03_R"},
            {29, "I16_O03_S"},
            {28, "I15_O03_S"},
            {27, "I14_O03_S"},
            {26, "I16_O02_R"},
            {25, "I15_O02_R"},
            {24, "I16_O02_S"},
            {23, "I15_O02_S"},
            {22, "I14_O02_S"},
            {21, "I16_O01_R"},
            {20, "I15_O01_R"},
            {19, "I16_O01_S"},
            {18, "I15_O01_S"},
            {17, "I14_O01_S"},
            {16, "I16_O00_R"},
            {15, "I15_O00_R"},
            {14, "I16_O00_S"},
            {13, "I15_O00_S"},
            {12, "I14_O00_S"},
            {11, "I09_O12_S"},
            {10, "I08_O12_S"},
            {9, "I06_O12_S"},
            {8, "I07_O11_S"},
            {7, "I05_O11_S"},
            {6, "I02_O11_S"},
            {5, "I08_O10_S"},
            {4, "I06_O10_S"},
            {3, "I04_O10_S"},
            {2, "I07_O09_S"},
            {1, "I05_O09_S"},
            {0, "I03_O09_S"}
        }
    },
    {
        AFE_CONN4,
        {   { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
#ifdef MT8135_AUD_REG
            {22, "I02_O17_S"},
            {21, "I02_O07_S"},
#else
            { -1, ""},
            { -1, ""},
#endif
            {20, "I01_O06_R"},
            {19, "I00_O05_R"},
            {18, "I008_O18_S"},
            {17, "I06_O18_S"},
            {16, "I04_O18_S"},
            {15, "I07_O17_S"},
            {14, "I05_O17_S"},
            {13, "I03_O17_S"},
            {12, "I14_O12_S"},
            {11, "I16_O10_S"},
            {10, "I15_O09_S"},
            {9, "I16_O06_R"},
            {8, "I16_O06_S"},
            {7, "I15_O05_R"},
            {6, "I15_O05_S"},
            {5, "I14_O05_S"},
            {4, "I16_O04_R"},
            {3, "I15_O04_R"},
            {2, "I16_O04_S"},
            {1, "I15_O04_S"},
            {0, "I14_O04_S"}
        }
    },
    {
        AFE_GAIN1_CONN,
        {   { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            {25, "I11_O18_S"},
            {24, "I10_O17_S"},
            {23, "I08_O14_S"},
            {22, "I07_O14_S"},
            {21, "I06_O14_S"},
            {20, "I05_O14_S"},
            {19, "I08_O13_S"},
            {18, "I07_O13_S"},
            {17, "I06_O13_S"},
            {16, "I05_O13_S"},
            {15, "I11_O08_S"},
            {14, "I10_O07_S"},
            {13, "I11_O06_S"},
            {12, "I10_O05_S"},
            {11, "I11_O04_R"},
            {10, "I11_O04_S"},
            {9, "I10_O03_R"},
            {8, "I10_O03_S"},
            {7, "I11_O02_R"},
            {6, "I11_O02_S"},
            {5, "I10_O02_R"},
            {4, "I10_O02_S"},
            {3, "I11_O01_R"},
            {2, "I11_O01_S"},
            {1, "I10_O00_R"},
            {0, "I10_O00_S"}
        }
    },
    {
        AFE_GAIN2_CONN,
        {   {31, "I00_O15_R"},
            {30, "I16_O15_S"},
            {29, "I15_O15_S"},
            {28, "I14_O15_S"},
            {27, "I09_O16_S"},
            {26, "I04_O16_S"},
            {25, "I03_O16_S"},
            {24, "I02_O16_S"},
            {23, "I01_O16_S"},
            {22, "I00_O16_S"},
            {21, "I09_O15_S"},
            {20, "I04_O15_S"},
            {19, "I03_O15_S"},
            {18, "I02_O15_S"},
            {17, "I01_O15_S"},
            {16, "I00_O15_S"},
            {15, "I13_O08_S"},
            {14, "I12_O07_S"},
            {13, "I13_O06_S"},
            {12, "I12_O05_S"},
            {11, "I13_O04_R"},
            {10, "I13_O04_S"},
            {9, "I12_O03_R"},
            {8, "I12_O03_S"},
            {7, "I13_O02_R"},
            {6, "I13_O02_S"},
            {5, "I12_O02_R"},
            {4, "I12_O02_S"},
            {3, "I13_O01_R"},
            {2, "I13_O01_S"},
            {1, "I12_O00_R"},
            {0, "I12_O00_S"}
        }
    },
    {
        AFE_GAIN2_CONN2,
        {   { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            { -1, ""},
            {7, "I13_O18_S"},
            {6, "I12_O17_S"},
            {5, "I16_O16_R"},
            {4, "I01_O16_R"},
            {3, "I16_O16_S"},
            {2, "I15_O16_S"},
            {1, "I14_O16_S"},
            {0, "I15_O15_R"}
        }
    },
};

char *mem_sr_name[11] =
{
    "8k",
    "11.025k",
    "12k",
    "",
    "16k",
    "22.05k",
    "24k",
    "",
    "32k",
    "44.1k",
    "48k"
};

char *pmic_sr_name[9] =
{
    "8k",
    "11.025k",
    "12k",
    "16k",
    "22.05k",
    "24k",
    "32k",
    "44.1k",
    "48k"
};

void init_audio_meta(void)
{
    static int init = 0;
    if (!init)
    {
        META_Audio_init();
        init = 1;
    }
}

void list_usage(void)
{
    printf("[audiocli usage]\n");
    printf("  h/help/usage/l/ls: list usage\n");
    printf("  audio_stream_type: list audio_stream_type_t\n");
    printf("  audio_source:      list audio_source_t\n");
    printf("  audio_devices:     list audio_devices_t\n");
    printf("  routing_strategy:  list routing_stragegy\n");
    printf("  vi 0x??:           list volume index for each stream for device\n");
    printf("  out_status:        show output status\n");
    printf("  out_dev_status:    show output device status\n");
    printf("  connect/disconnect [output devices]\n");
    printf("  forced_use_status: show forced use status\n");
    printf("  get_output_status: show getOutput id for each stream\n");
    printf("  get_input_status:  show getInput id for each audio source\n");
    printf("  dump_audio_policy: dump audio policy info\n");
    printf("  dump_audio_flinger: dump audio fligner info\n");
#ifdef MT8135_AUD_REG
    printf("  r [reg] [cnt]:     read AP-side register (hex num wo 0x prefix)\n");
    printf("  w [reg] [data]:    write AP-side register (hex num wo 0x prefix)\n");
#endif
    printf("  ra [reg] [cnt]:    read AFE register (hex num wo 0x prefix)\n");
    printf("  wa [reg] [data]:   write AFE register (hex num wo 0x prefix)\n");
    printf("  rp [reg] [cnt]:    read PMIC register (hex num wo 0x prefix)\n");
    printf("  wp [reg] [data]:   write PMIC register (hex num wo 0x prefix)\n");
    printf("  hwq [module name]: show HW module reg status\n");
    printf("  hwtest [item]:     Factory Mode/HW Testing tool item\n");
    printf("  loopback [input: 1~3] [output: 1~3]\n");
    printf("  audio_dump [item]: audio dump item toggle\n");
    printf("  eng:               trigger engineering mode\n");
    printf("  exit:              exit program\n");
    printf("[end]\n");
}

void process_audio_dump_cmd(char *cmd);

int main(int argc, char *argv[])
{
    char cmd[100];
    android::AudioAfeReg *mAfeReg = android::AudioAfeReg::getInstance();
    android::AudioAnalogReg *mAnalogReg = android::AudioAnalogReg::getInstance();
    //META_Audio_init();
    list_usage();

    while (1)
    {
        printf("Please input cmd:\n");
        memset(cmd, 0, 100);
        fgets(cmd, sizeof(cmd), stdin);
        // remove new line character
        size_t cmdLen = strlen(cmd);
        if (cmdLen > 0 && cmd[cmdLen-1] == '\n')
            cmd[cmdLen-1] = '\0';

        //scanf("%s", cmd);
        if (!strcmp(cmd, "h") || !strcmp(cmd, "help") || !strcmp(cmd, "usage") || !strcmp(cmd, "l") || !strcmp(cmd, "ls"))
        {
            list_usage();
        }
        else if (!strcmp(cmd, "audio_stream_type"))
        {
            list_audio_stream_type();
        }
        else if (!strcmp(cmd, "audio_source"))
        {
            list_audio_source();
        }
        else if (!strcmp(cmd, "audio_devices"))
        {
            list_audio_devices();
        }
        else if (!strcmp(cmd, "routing_strategy"))
        {
            list_routing_strategy();
            list_routing_strategy_by_stream();
            list_device_by_stream();
        }
        else if (!strncmp(cmd, "vi", 2))
        {
            int device = (int)AUDIO_DEVICE_OUT_SPEAKER;
            if (strlen(cmd) == 2)
            {
                list_getStreamVolumeIndex(device);
            }
            else
            {
                sscanf(cmd, "vi 0x%x\n", &device);
                list_getStreamVolumeIndex(device);
            }
        }
        else if (!strncmp(cmd, "ra ", 3))
        {
            int addr = 0;
            int val = 0;
            int cnt = 0;
            int ret;
            ret = sscanf(cmd, "ra %x %d\n", &addr, &cnt);

            if (addr & 0x3)
            {
                printf("[error] addr 0x%x should be aligned with 4\n", addr);
            }
            else
            {
                int i;
                if (ret == 1)
                {
                    cnt = 1;
                }
                else if (cnt == 0)
                {
                    cnt = 1;
                }

                if ((addr & 15) != 0)
                {
                    printf("\n%04xh: ", (addr & (~15)));
                    for (i = 0; i < (addr & 15); i += 4)
                    {
                        printf("         ");
                    }
                }
                for (i = 0; i < cnt; i++)
                {
                    if (((addr + i * 4) & 15) == 0)
                    {
                        printf("\n%04xh: ", (addr + i * 4));
                    }
                    val = mAfeReg->GetAfeReg(addr + i * 4);
                    printf("%08x ", val);
                }
                printf("\n");
            }
        }
        else if (!strncmp(cmd, "wa ", 3))
        {
            int addr = 0;
            int val = 0;
            int ret;
            ret = sscanf(cmd, "wa %x %x\n", &addr, &val);
            if (ret != 2)
            {
                printf("usage: wa [addr] [val]\n");
            }
            else
            {
                mAfeReg->SetAfeReg(addr, val, 0xffffffff);
            }
        }
        else if (!strncmp(cmd, "rp ", 3))
        {
            int addr = 0;
            int val = 0;
            int cnt = 0;
            int ret;
            ret = sscanf(cmd, "rp %x %d\n", &addr, &cnt);

            if (addr & 0x1)
            {
                printf("[error] addr 0x%x should be aligned with 2\n", addr);
            }
            else
            {
                int i;
                if (ret == 1)
                {
                    cnt = 1;
                }
                else if (cnt == 0)
                {
                    cnt = 1;
                }

                if ((addr & 7) != 0)
                {
                    printf("\n%04xh: ", (addr & (~7)));
                    for (i = 0; i < (addr & 7); i += 2)
                    {
                        printf("     ");
                    }
                }
                for (i = 0; i < cnt; i++)
                {
                    if (((addr + i * 2) & 7) == 0)
                    {
                        printf("\n%04xh: ", (addr + i * 2));
                    }
                    val = mAnalogReg->GetAnalogReg(addr + i * 2);
                    printf("%04x ", val);
                }
                printf("\n");
            }

        }
        else if (!strncmp(cmd, "wp ", 3))
        {
            int addr = 0;
            int val = 0;
            int ret;
            ret = sscanf(cmd, "wp %x %x\n", &addr, &val);
            if (ret != 2)
            {
                printf("usage: wp [addr] [val]\n");
            }
            else
            {
                mAnalogReg->SetAnalogReg(addr, val, 0xffffffff);
            }
        }
#ifdef MT8135_AUD_REG
        else if (!strncmp(cmd, "r ", 2))
        {
            int addr = 0;
            int val = 0;
            int cnt = 0;
            int ret;
            ret = sscanf(cmd, "r %x %d\n", &addr, &cnt);

            if (addr & 0x3)
            {
                printf("[error] addr 0x%x should be aligned with 4\n", addr);
            }
            else
            {
                int i;
                if (ret == 1)
                {
                    cnt = 1;
                }
                else if (cnt == 0)
                {
                    cnt = 1;
                }

                if ((addr & 15) != 0)
                {
                    printf("\n%04xh: ", (addr & (~15)));
                    for (i = 0; i < (addr & 15); i += 4)
                    {
                        printf("         ");
                    }
                }
                for (i = 0; i < cnt; i++)
                {
                    if (((addr + i * 4) & 15) == 0)
                    {
                        printf("\n%04xh: ", (addr + i * 4));
                    }
                    val = mAfeReg->GetApReg(addr + i * 4);
                    printf("%08x ", val);
                }
                printf("\n");
            }
        }
        else if (!strncmp(cmd, "w ", 2))
        {
            int addr = 0;
            int val = 0;
            int ret;
            ret = sscanf(cmd, "w %x %x\n", &addr, &val);
            if (ret != 2)
            {
                printf("usage: wa [addr] [val]\n");
            }
            else
            {
                mAfeReg->SetApReg(addr, val, 0xffffffff);
            }
        }
#endif
        else if (!strncmp(cmd, "hwq", 3))
        {
            if (!strcmp(cmd + 4, "intcon"))
            {
                int i, j, val;

                val = mAfeReg->GetAfeReg(AFE_CONN0);
                printf("AFE_CONN0(%04xh) = %08x\n", AFE_CONN0, val);
                val = mAfeReg->GetAfeReg(AFE_CONN1);
                printf("AFE_CONN1(%04xh) = %08x\n", AFE_CONN1, val);
                val = mAfeReg->GetAfeReg(AFE_CONN2);
                printf("AFE_CONN2(%04xh) = %08x\n", AFE_CONN2, val);
                val = mAfeReg->GetAfeReg(AFE_CONN3);
                printf("AFE_CONN3(%04xh) = %08x\n", AFE_CONN3, val);
                val = mAfeReg->GetAfeReg(AFE_CONN4);
                printf("AFE_CONN4(%04xh) = %08x\n", AFE_CONN4, val);
                val = mAfeReg->GetAfeReg(AFE_GAIN1_CONN);
                printf("AFE_GAIN1_CONN(%04xh) = %08x\n", AFE_GAIN1_CONN, val);
                val = mAfeReg->GetAfeReg(AFE_GAIN2_CONN);
                printf("AFE_GAIN2_CONN(%04xh) = %08x\n", AFE_GAIN2_CONN, val);
                val = mAfeReg->GetAfeReg(AFE_GAIN2_CONN2);
                printf("AFE_GAIN2_CONN2(%04xh) = %08x\n", AFE_GAIN2_CONN2, val);

                for (i = 0; i < CONN_TAB_NUM; i++)
                {
                    val = mAfeReg->GetAfeReg(conn_table[i].reg_addr);
                    for (j = 0; j < 32; j++)
                    {
                        if (conn_table[i].bit_name[j].bit_addr != -1)
                        {
                            if (val & (1 << conn_table[i].bit_name[j].bit_addr))
                            {
                                printf("  %s\n", conn_table[i].bit_name[j].name);
                            }
                        }
                    }
                }
            }
            else if (!strcmp(cmd + 4, "pdn"))
            {
                int val;

                val = mAfeReg->GetAfeReg(AUDIO_TOP_CON0);
                printf("AUDIO_TOP_CON0(%04xh) = %08x\n", AUDIO_TOP_CON0, val);
                printf("  PDN_SPDF_CK: %s\n", (val & (1 << 21)) ? "power down" : "power on");
                printf("  PDN_HDMI_CK: %s\n", (val & (1 << 20)) ? "power down" : "power on");
                printf("  PDN_APLL_TUNER: %s\n", (val & (1 << 19)) ? "power down" : "power on");
                printf("  AFE_CK_DIV_RST: %s\n", (val & (1 << 16)) ? "reset" : "No reset");
                printf("  APB3_SEL: %s\n", (val & (1 << 14)) ? "use APB 3.0" : "Don't use APB 3.0");
                printf("  PDN_I2S(input): %s\n", (val & (1 << 6)) ? "power down" : "power on");
                printf("  PDN_AFE: %s\n", (val & (1 << 2)) ? "power down" : "power on");
                printf("  PDN_APB: %s\n", (val & (1 << 1)) ? "power down" : "power on");
                val = mAfeReg->GetAfeReg(AFE_DAC_CON0);
                printf("AFE_DAC_CON0(%04xh) = %08x\n", AFE_DAC_CON0, val);
                printf("  MOD_PCM_ON: %s\n", (val & (1 << 7)) ? "on" : "off");
                printf("  AWB_ON: %s\n", (val & (1 << 6)) ? "on" : "off");
#ifndef MT8135_AUD_REG
                printf("  I2S_ON: %s\n", (val & (1 << 5)) ? "on" : "off");
#endif
                printf("  DAI_ON: %s\n", (val & (1 << 4)) ? "on" : "off");
                printf("  VUL_ON: %s\n", (val & (1 << 3)) ? "on" : "off");
                printf("  DL2_ON: %s\n", (val & (1 << 2)) ? "on" : "off");
                printf("  DL1_ON: %s\n", (val & (1 << 1)) ? "on" : "off");
                printf("  AFE_ON: %s\n", (val & (1 << 0)) ? "on" : "off");
            }
            else if (!strcmp(cmd + 4, "mem"))
            {
                int val;

                val = mAfeReg->GetAfeReg(AFE_DAC_CON1);
                printf("AFE_DAC_CON1(%04xh) = %08x\n", AFE_DAC_CON1, val);
                printf("  DL1_MODE: %s\n", mem_sr_name[(val >> 0) & 0xf]);
                printf("  DL2_MODE: %s\n", mem_sr_name[(val >> 4) & 0xf]);
                printf("  I2S_MODE: %s\n", mem_sr_name[(val >> 8) & 0xf]);
                printf("  AWB_MODE: %s\n", mem_sr_name[(val >> 12) & 0xf]);
                printf("  VUL_MODE: %s\n", mem_sr_name[(val >> 16) & 0xf]);
                printf("  DAI_MODE: %s\n", (val & (1 << 20)) ? "16k" : "8k");
                printf("  DL1_DATA: %s\n", (val & (1 << 21)) ? "mono" : "stereo");
                printf("  DL2_DATA: %s\n", (val & (1 << 22)) ? "mono" : "stereo");
                printf("  AWB_DATA: %s\n", (val & (1 << 24)) ? "mono" : "stereo");
                printf("  AWB_R_MONO: %s\n", (val & (1 << 25)) ? "mono use R" : "mono use L");
                printf("  VUL_DATA: %s\n", (val & (1 << 27)) ? "mono" : "stereo");
                printf("  VUL_R_MONO: %s\n", (val & (1 << 28)) ? "mono use R" : "mono use L");
                printf("  MOD_PCM_MODE: %s\n", (val & (1 << 30)) ? "16k" : "8k");
            }
            else if (!strcmp(cmd + 4, "2ndi2s"))
            {
                int val;

                val = mAfeReg->GetAfeReg(AFE_I2S_CON);
                printf("AFE_I2S_CON(%04xh) = %08x\n", AFE_I2S_CON, val);
                printf("  INV_LRCK: %s\n", (val & (1 << 5)) ? "inverse" : "not inverse");
                printf("  I2S_DIR: %s\n", (val & (1 << 4)) ? "input" : "output");
                printf("  I2S_FMT: %s\n", (val & (1 << 3)) ? "I2S" : "EIAJ");
                printf("  I2S_SRC: %s\n", (val & (1 << 2)) ? "slave" : "master");
                printf("  I2S_WLEN: %s\n", (val & (1 << 1)) ? "32bits" : "16bits");
                printf("  I2S_EN: %s\n", (val & (1 << 0)) ? "enable" : "disable");
            }
            else if (!strcmp(cmd + 4, "i2s"))
            {
                int val;

                val = mAfeReg->GetAfeReg(AFE_I2S_CON1);
                printf("(DAC) AFE_I2S_CON1(%04xh) = %08x\n", AFE_I2S_CON1, val);
                printf("  I2S_OUT_MODE: %s\n", mem_sr_name[(val >> 8) & 0xf]);
                printf("  INV_LRCK: %s\n", (val & (1 << 5)) ? "inverse" : "not inverse");
                printf("  I2S_FMT: %s\n", (val & (1 << 3)) ? "I2S" : "EIAJ");
                printf("  I2S_WLEN: %s\n", (val & (1 << 1)) ? "32bits" : "16bits");
                printf("  I2S_EN: %s\n", (val & (1 << 0)) ? "enable" : "disable");

                val = mAfeReg->GetAfeReg(AFE_I2S_CON2);
                printf("(ADC) AFE_I2S_CON2(%04xh) = %08x\n", AFE_I2S_CON2, val);
                printf("  I2S_OUT_MODE: %s\n", mem_sr_name[(val >> 8) & 0xf]);
                printf("  I2S_FMT: %s\n", (val & (1 << 3)) ? "I2S" : "EIAJ");
                printf("  I2S_WLEN: %s\n", (val & (1 << 1)) ? "32bits" : "16bits");
                printf("  I2S_EN: %s\n", (val & (1 << 0)) ? "enable" : "disable");
            }
            else if (!strcmp(cmd + 4, "mcu"))
            {
                int val;

                val = mAfeReg->GetAfeReg(AFE_IRQ_MCU_CON);
                printf("AFE_IRQ_MCU_CON(%04xh) = %08x\n", AFE_IRQ_MCU_CON, val);
                printf("  IRQ1_MCU_ON(DL/UL): %s\n", (val & (1 << 0)) ? "on" : "off");
                printf("  IRQ2_MCU_ON(AWB,VUL,DAI,MOD_DAI): %s\n", (val & (1 << 1)) ? "on" : "off");
                printf("  IRQ3_MCU_ON(DAI): %s\n", (val & (1 << 2)) ? "on" : "off");
                printf("  IRQ1_MCU_MODE: %s\n", mem_sr_name[(val >> 4) & 0xf]);
                printf("  IRQ2_MCU_MODE: %s\n", mem_sr_name[(val >> 8) & 0xf]);
                printf("  IRQ5_MCU_ON(HDMI): %s\n", (val & (1 << 12)) ? "on" : "off");
                printf("  IRQ6_MCU_ON(SPDF): %s\n", (val & (1 << 13)) ? "on" : "off");
                val = mAfeReg->GetAfeReg(AFE_IRQ_MCU_CNT1);
                printf("  AFE_IRQ_MCU_CNT1: %d\n", (val & 0x3ffff));
                val = mAfeReg->GetAfeReg(AFE_IRQ_MCU_CNT2);
                printf("  AFE_IRQ_MCU_CNT2: %d\n", (val & 0x3ffff));
                val = mAfeReg->GetAfeReg(AFE_IRQ_MCU_CNT5);
                printf("  AFE_IRQ_MCU_CNT5: %d\n", (val & 0x3ffff));
            }
            else if (!strcmp(cmd + 4, "gain12"))
            {
                int val;
                val = mAfeReg->GetAfeReg(AFE_GAIN1_CON0);

                printf("AFE_GAIN1_CON0(%04xh) = %08x\n", AFE_GAIN1_CON0, val);
                printf("  GAIN1_ON: %s\n", (val & 0x1) ? "on" : "off");
                printf("  GAIN1_MODE: %s\n", mem_sr_name[(val >> 4) & 0xf]);

                val = mAfeReg->GetAfeReg(AFE_GAIN2_CON0);

                printf("AFE_GAIN2_CON0(%04xh) = %08x\n", AFE_GAIN2_CON0, val);
                printf("  GAIN2_ON: %s\n", (val & 0x1) ? "on" : "off");
                printf("  GAIN2_MODE: %s\n", mem_sr_name[(val >> 4) & 0xf]);
            }
            else if (!strcmp(cmd + 4, "mrgif"))
            {
#ifdef SUPPORT_MRGIF
                int val;
                val = mAfeReg->GetAfeReg(AFE_MRGIF_CON);
                printf("AFE_MRGIF_CON(%04xh) = %08x\n", AFE_MRGIF_CON, val);
                printf("  MRGIF_I2S_MODE: %s\n", mem_sr_name[(val >> 20) & 0xf]);
                printf("  MRGIF_I2S_EN: %s\n", (val & (1 << 16)) ? "on" : "off");
                printf("  MRGIF_EN: %s\n", (val & (1 << 0)) ? "on" : "off");
#else
                printf("AFE_MRGIF_CON is not supported!\n");
#endif
            }
            else if (!strcmp(cmd + 4, "dai"))
            {
#ifdef SUPPORT_MRGIF
                int val;
                val = mAfeReg->GetAfeReg(AFE_DAIBT_CON0);
                printf("AFE_DAIBT_CON0(%04xh) = %08x\n", AFE_DAIBT_CON0, val);
                printf("  USE_MRGIF_INPUT: %s\n", (val & (1 << 12)) ? "on" : "off");
                printf("  DAIBT_MODE: %s\n", (val & (1 << 9)) ? "16k" : "8k");
                printf("  BT_ON: %s\n", (val & (1 << 1)) ? "on" : "off");
                printf("  DAIBT_ON: %s\n", (val & (1 << 0)) ? "on" : "off");
#else
                printf("AFE_DAIBT_CON0 is not supported!\n");
#endif
            }
            else if (!strcmp(cmd + 4, "mux"))
            {
                int val;
                printf("=== Output Mux ===\n");
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AUDBUF_CFG0);
#endif
                printf("  HPR inputmux: %s\n", (((val >> 9) & 0xf) == 0) ? "open/open" :
                       (((val >> 9) & 0xf) == 1) ? "Line Left/Line Left Common" :
                       (((val >> 9) & 0xf) == 2) ? "Line Right/Line Right Common" :
                       (((val >> 9) & 0xf) == 3) ? "Line Left/Line Right" :
                       (((val >> 9) & 0xf) == 4) ? "IDACLP / IDACLN" :
                       (((val >> 9) & 0xf) == 5) ? "IDACLP + Line Left" :
                       (((val >> 9) & 0xf) == 8) ? "IV buffer in" : "Unknown");
                printf("  HPL inputmux: %s\n", (((val >> 5) & 0xf) == 0) ? "open/open" :
                       (((val >> 5) & 0xf) == 1) ? "Line Left/Line Left Common" :
                       (((val >> 5) & 0xf) == 2) ? "Line Right/Line Right Common" :
                       (((val >> 5) & 0xf) == 3) ? "Line Left/Line Right" :
                       (((val >> 5) & 0xf) == 4) ? "IDACLP / IDACLN" :
                       (((val >> 5) & 0xf) == 5) ? "IDACLP + Line Left" :
                       (((val >> 5) & 0xf) == 8) ? "IV buffer in" : "Unknown");
                printf("  HS inputmux: %s\n", (((val >> 3) & 0x3) == 0) ? "open/open" :
                       (((val >> 3) & 0x3) == 1) ? "MUTE" :
                       (((val >> 3) & 0x3) == 2) ? "Voice playback" : "test mode");
#ifdef MTK_PMIC_MT6397                       
                val = mAnalogReg->GetAnalogReg(AUD_IV_CFG0);
#endif
                printf("  IVL inputmux: %s\n", (((val >> 2) & 0x7) == 0) ? "open/open" :
                       (((val >> 2) & 0x7) == 1) ? "FM stereo mode" :
                       (((val >> 2) & 0x7) == 2) ? "FM mono mode" :
                       (((val >> 2) & 0x7) == 3) ? "open/open" :
                       (((val >> 2) & 0x7) == 4) ? "Audio playback mode" :
                       (((val >> 2) & 0x7) == 5) ? "Audio + FM stereo mode" :
                       (((val >> 2) & 0x7) == 6) ? "Audio + FM mono mode" : "Audio playback");
#ifdef MT8135_AUD_REG
                printf("  IVR inputmux: %s\n", (((val >> 10) & 0x7) == 0) ? "open/open" :
                       (((val >> 10) & 0x7) == 1) ? "FM stereo mode" :
                       (((val >> 10) & 0x7) == 2) ? "FM mono mode" :
                       (((val >> 10) & 0x7) == 3) ? "open/open" :
                       (((val >> 10) & 0x7) == 4) ? "Audio playback mode" :
                       (((val >> 10) & 0x7) == 5) ? "Audio + FM stereo mode" :
                       (((val >> 10) & 0x7) == 6) ? "Audio + FM mono mode" : "Audio playback");
#endif
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(ZCD_CON0);
#endif
                printf("  ZCD inputmux: %s\n", (((val >> 8) & 0x7) == 0) ? "Line-in L/R" :
                       (((val >> 8) & 0x7) == 1) ? "HPL/HPR" :
                       (((val >> 8) & 0x7) == 2) ? "HS" :
                       (((val >> 8) & 0x7) == 3) ? "IV buffer" : "Bypass ZCD");
                printf("=== Input Mux ===\n");
#ifdef MTK_PMIC_MT6397                
                val = mAnalogReg->GetAnalogReg(AUDADC_CON0);
#endif
                printf("  ADC-R inputmux: %s\n", (((val >> 5) & 0x7) == 0) ? "idle" :
                       (((val >> 5) & 0x7) == 1) ? "AIN0" :
                       (((val >> 5) & 0x7) == 2) ? "idle" :
                       (((val >> 5) & 0x7) == 3) ? "idle" :
                       (((val >> 5) & 0x7) == 4) ? "Preamp-R" :
                       (((val >> 5) & 0x7) == 5) ? "LSB" :
                       (((val >> 5) & 0x7) == 6) ? "idle" : "idle");
                printf("  ADC-L inputmux: %s\n", (((val >> 2) & 0x7) == 0) ? "idle" :
                       (((val >> 2) & 0x7) == 1) ? "AIN0" :
                       (((val >> 2) & 0x7) == 2) ? "idle" :
                       (((val >> 2) & 0x7) == 3) ? "idle" :
                       (((val >> 2) & 0x7) == 4) ? "Preamp-L" :
                       (((val >> 2) & 0x7) == 5) ? "LSB" :
                       (((val >> 2) & 0x7) == 6) ? "idle" : "idle");
#ifdef MTK_PMIC_MT6397                       
                val = mAnalogReg->GetAnalogReg(AUDLSBUF_CON1);
#endif
                printf("  LSB-R inputmux: %s\n", (((val >> 3) & 0x7) == 0) ? "open/open" :
                       (((val >> 3) & 0x7) == 1) ? "Line Left/Line Left Common" :
                       (((val >> 3) & 0x7) == 2) ? "Line Right/Line Right Common" :
                       (((val >> 3) & 0x7) == 3) ? "Line Left/Line Right" :
                       (((val >> 3) & 0x7) == 4) ? "HPR-P/Vref0V" :
                       (((val >> 3) & 0x7) == 5) ? "HPR-P/N" :
                       (((val >> 3) & 0x7) == 6) ? "No use" : "HS-P/N");
                printf("  LSB-L inputmux: %s\n", (((val >> 0) & 0x7) == 0) ? "open/open" :
                       (((val >> 0) & 0x7) == 1) ? "Line Left/Line Left Common" :
                       (((val >> 0) & 0x7) == 2) ? "Line Right/Line Right Common" :
                       (((val >> 0) & 0x7) == 3) ? "Line Left/Line Right" :
                       (((val >> 0) & 0x7) == 4) ? "HPL-P/Vref0V" :
                       (((val >> 0) & 0x7) == 5) ? "HPL-P/N" :
                       (((val >> 0) & 0x7) == 6) ? "No use" : "HS-P/N");
#ifdef MTK_PMIC_MT6397                       
                val = mAnalogReg->GetAnalogReg(AUDPREAMP_CON0);
#endif
                printf("  Preamp-R inputmux: %s\n", (((val >> 5) & 0x7) == 0) ? "None" :
                       (((val >> 5) & 0x7) == 1) ? "AIN0" :
                       (((val >> 5) & 0x7) == 2) ? "AIN1" :
                       (((val >> 5) & 0x7) == 3) ? "AIN2" : "None");
                printf("  Preamp-L inputmux: %s\n", (((val >> 2) & 0x7) == 0) ? "None" :
                       (((val >> 2) & 0x7) == 1) ? "AIN0" :
                       (((val >> 2) & 0x7) == 2) ? "AIN1" :
                       (((val >> 2) & 0x7) == 3) ? "AIN2" : "None");
            }
            else if (!strcmp(cmd + 4, "anapdn"))
            {
                int val;
#ifdef MTK_PMIC_MT6397                
                val = mAnalogReg->GetAnalogReg(AFE_UL_DL_CON0);
#endif
                printf("  [PM]afe_on: %s\n", (val & (1 << 0)) ? "on" : "off");
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AFE_DL_SRC2_CON0_H);
#endif
                printf("  [PM]dl_2_input_mode_ctl: %s\n", (((val >> 12) & 0xf) > 8) ? "8k" : pmic_sr_name[((val >> 12) & 0xf)]);
                printf("  [PM]dl_2_output_sel_ctl: %s\n", (((val >> 8) & 0x3) == 0) ?  "x1 rate" :
                       (((val >> 8) & 0x3) == 1) ?  "x2 rate" :
                       (((val >> 8) & 0x3) == 2) ?  "x4 rate" : "x8 rate");
#ifdef MT8135_AUD_REG
                val = mAfeReg->GetAfeReg(AFE_ADDA_DL_SRC2_CON0);
                printf("  [AP]dl_2_input_mode_ctl: %s\n", (((val >> 28) & 0xf) > 8) ? "8k" : pmic_sr_name[((val >> 28) & 0xf)]);
                printf("  [AP]dl_2_output_sel_ctl: %s\n", (((val >> 24) & 0x3) == 0) ?  "x1 rate" :
                       (((val >> 24) & 0x3) == 1) ?  "x2 rate" :
                       (((val >> 24) & 0x3) == 2) ?  "x4 rate" : "x8 rate");
#endif
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AFE_DL_SRC2_CON0_L);
#endif
                printf("  [PM]DL: %s\n", (val & (1 << 0)) ? "on" : "off");
#ifdef MT8135_AUD_REG
                val = mAfeReg->GetAfeReg(AFE_ADDA_DL_SRC2_CON0);
                printf("  [AP]DL: %s\n", (val & (1 << 0)) ? "on" : "off");
#endif
#ifdef MT8135_AUD_REG
                val = mAnalogReg->GetAnalogReg(AFE_PMIC_NEWIF_CFG0);
                printf("  [PM] up8x_rxif_dl_2_input_mode: %s\n", (((val >> 12) & 0xf) == 0) ? "64k" :
                       (((val >> 12) & 0xf) == 1) ? "88.2k" :
                       (((val >> 12) & 0xf) == 2) ? "96k" :
                       (((val >> 12) & 0xf) == 3) ? "128k" :
                       (((val >> 12) & 0xf) == 4) ? "176.4k" :
                       (((val >> 12) & 0xf) == 5) ? "192k" :
                       (((val >> 12) & 0xf) == 6) ? "256k" :
                       (((val >> 12) & 0xf) == 7) ? "352.8k" :
                       (((val >> 12) & 0xf) == 8) ? "384k" : "NA");
#else
                val = mAnalogReg->GetAnalogReg(AFE_I2S_FIFO_DL_CFG0);
                printf("    wlen: %s\n", (val & (1 << 7)) ? "32bits" : "16bits");
                printf("    fmt: %s\n", (val & (1 << 6)) ? "I2S" : "EIAJ");
                printf("    fifo enable: %s\n", (val & (1 << 1)) ? "enable" : "disable");
                printf("    fifo resetb: %s\n", (val & (1 << 0)) ? "on" : "off");
#endif
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AFE_UL_SRC_CON0_L);
#endif
                printf("  [PM]UL: %s\n", (val & (1 << 0)) ? "on" : "off");
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AFE_UL_SRC_CON0_H);
#endif
                printf("  [PM]ul_voice_mode_ch2_ctl: %s\n", (((val >> 3) & 0x3) == 0) ? "8k" :
                       (((val >> 3) & 0x3) == 1) ? "16k" :
                       (((val >> 3) & 0x3) == 2) ? "32k" : "48k");
                printf("  [PM]ul_voice_mode_ch1_ctl: %s\n", (((val >> 1) & 0x3) == 0) ? "8k" :
                       (((val >> 1) & 0x3) == 1) ? "16k" :
                       (((val >> 1) & 0x3) == 2) ? "32k" : "48k");
#ifdef MT8135_AUD_REG
                val = mAfeReg->GetAfeReg(AFE_ADDA_UL_SRC_CON0);
                printf("  [AP]UL: %s\n", (val & (1 << 0)) ? "on" : "off");
                printf("  [AP]ul_voice_mode_ch2_ctl: %s\n", (((val >> 19) & 0x3) == 0) ? "8k" :
                       (((val >> 19) & 0x3) == 1) ? "16k" :
                       (((val >> 19) & 0x3) == 2) ? "32k" : "48k");
                printf("  [AP]ul_voice_mode_ch1_ctl: %s\n", (((val >> 17) & 0x3) == 0) ? "8k" :
                       (((val >> 17) & 0x3) == 1) ? "16k" :
                       (((val >> 17) & 0x3) == 2) ? "32k" : "48k");
                val = mAfeReg->GetAfeReg(AFE_ADDA_NEWIF_CFG1);
                printf("  [AP]up8x_rxif_adc_voice_mode: %d\n", ((val >> 10) & 0x3));
#endif
#ifdef MT8135_AUD_REG
                val = mAnalogReg->GetAnalogReg(AFE_PMIC_NEWIF_CFG2);
                printf("  [PM]rg_up8x_rxif_adc_voice_mode: %d\n", ((val >> 10) & 0x3));
#else
                val = mAnalogReg->GetAnalogReg(AFE_I2S_FIFO_UL_CFG0);
                printf("    wlen: %s\n", (val & (1 << 8)) ? "32bits" : "16bits");
                printf("    fmt: %s\n", (val & (1 << 6)) ? "I2S" : "EIAJ");
                printf("    fifo enable: %s\n", (val & (1 << 1)) ? "enable" : "disable");
                printf("    fifo resetb: %s\n", (val & (1 << 0)) ? "on" : "off");
#endif
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AUDBUF_CFG0);
#endif
                printf("  HPR: %s\n", (val & (1 << 2)) ? "power up" : "power down");
                printf("  HPL: %s\n", (val & (1 << 1)) ? "power up" : "power down");
                printf("  HS:  %s\n", (val & (1 << 0)) ? "power up" : "power down");
#ifdef MTK_PMIC_MT6397                
                val = mAnalogReg->GetAnalogReg(AUD_IV_CFG0);
#endif
                printf("  IVL buffer: startup %s\n", (val & (1 << 1)) ? "enable" : "disable");
                printf("  IVL buffer: power %s\n", (val & (1 << 0)) ? "up" : "down");
#ifdef MT8135_AUD_REG
                printf("  IVR buffer: startup %s\n", (val & (1 << 9)) ? "enable" : "disable");
                printf("  IVR buffer: power %s\n", (val & (1 << 8)) ? "up" : "down");
#endif
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(ZCD_CON0);
#endif
                printf("  ZCD: %s\n", (val & (1 << 0)) ? "enable" : "disable");
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AUDADC_CON0);
#endif
                printf("  ADC-R: %s\n", (val & (1 << 1)) ? "power up" : "power down");
                printf("  ADC-L: %s\n", (val & (1 << 0)) ? "power up" : "power down");
#ifdef MTK_PMIC_MT6397                
                val = mAnalogReg->GetAnalogReg(AUDLSBUF_CON0);
#endif
                printf("  LSB-R: %s\n", (val & (1 << 1)) ? "power up" : "power down");
                printf("  LSB-L: %s\n", (val & (1 << 0)) ? "power up" : "power down");
#ifdef MTK_PMIC_MT6397                
                val = mAnalogReg->GetAnalogReg(AUDPREAMP_CON0);
#endif
                printf("  Preamp-R: %s\n", (val & (1 << 1)) ? "enable" : "disable");
                printf("  Preamp-L: %s\n", (val & (1 << 0)) ? "enable" : "disable");
                val = mAnalogReg->GetAnalogReg(SPK_CON0);
                printf("  SPK_EN_L: %s\n", (val & (1 << 0)) ? "enable" : "disable");
#ifdef MTK_PMIC_MT6397                
                val = mAnalogReg->GetAnalogReg(SPK_CON3);
#endif
                printf("  SPK_EN_R: %s\n", (val & (1 << 0)) ? "enable" : "disable");
                val = mAnalogReg->GetAnalogReg(SPK_CON11);
                printf("  SPK_OUTSTG_EN_L_SW: %s\n", (val & (1 << 11)) ? "enable" : "disable");
                printf("  SPK_OUTSTG_EN_R_SW: %s\n", (val & (1 << 10)) ? "enable" : "disable");
                printf("  SPK_EN_L_SW: %s\n", (val & (1 << 9)) ? "enable" : "disable");
                printf("  SPK_EN_L_SW: %s\n", (val & (1 << 8)) ? "enable" : "disable");
#ifdef MTK_PMIC_MT6397                                
                val = mAnalogReg->GetAnalogReg(AUDDAC_CON0);
#endif
                printf("  DAC-L biasgen: %s\n", (val & (1 << 3)) ? "enable" : "disable");
                printf("  DAC-R biasgen: %s\n", (val & (1 << 2)) ? "enable" : "disable");
                printf("  DAC-R: %s\n", (val & (1 << 1)) ? "enable" : "disable");
                printf("  DAC-L: %s\n", (val & (1 << 0)) ? "enable" : "disable");
            }
            else if (!strcmp(cmd + 4, "anagain"))
            {
                int val;
                printf("=== Analog Gain ===\n");
#ifdef MTK_PMIC_MT6397                
                val = mAnalogReg->GetAnalogReg(AUDLSBUF_CON0);
                printf("  LSB-R mute(%04xh[15]): %s\n", AUDLSBUF_CON0, (val & (1 << 15)) ? "mute" : "no mute");
                printf("  LSB-R gain(%04xh[11:9]: %s\n", AUDLSBUF_CON0, (((val >> 9) & 0x7) == 0) ? "-3dB" :              	
                       (((val >> 9) & 0x7) == 1) ? "0dB" :
                       (((val >> 9) & 0x7) == 2) ? "3dB" :
                       (((val >> 9) & 0x7) == 3) ? "6dB" :
                       (((val >> 9) & 0x7) == 4) ? "9dB" :
                       (((val >> 9) & 0x7) == 5) ? "12dB" :
                       (((val >> 9) & 0x7) == 6) ? "15dB" : "18dB");
                printf("  LSB-L mute(%04xh[8]): %s\n", AUDLSBUF_CON0, (val & (1 << 8)) ? "mute" : "no mute");
                printf("  LSB-L gain(%04xh[4:2]): %s\n", AUDLSBUF_CON0, (((val >> 2) & 0x7) == 0) ? "-3dB" :
                       (((val >> 2) & 0x7) == 1) ? "0dB" :
                       (((val >> 2) & 0x7) == 2) ? "3dB" :
                       (((val >> 2) & 0x7) == 3) ? "6dB" :
                       (((val >> 2) & 0x7) == 4) ? "9dB" :
                       (((val >> 2) & 0x7) == 5) ? "12dB" :
                       (((val >> 2) & 0x7) == 6) ? "15dB" : "18dB");
#endif                         
                val = mAnalogReg->GetAnalogReg(SPK_CON0);
                printf("  SPK L-Ch Vol(%04xh[13:12]): %s\n", SPK_CON0, (((val >> 12) & 0x3) == 0) ? "mute" :
                       (((val >> 12) & 0x3) == 1) ? "-6dB" :
                       (((val >> 12) & 0x3) == 2) ? "-3dB" : "0dB");

                val = mAnalogReg->GetAnalogReg(SPK_CON9);
                printf("  SPK L PGA_GAIN(%04xh[11:8]): %s\n", SPK_CON9, (((val >> 8) & 0xf) == 0) ? "mute" :
                       (((val >> 8) & 0xf) == 1) ? "0dB" :
                       (((val >> 8) & 0xf) == 2) ? "4dB" :
                       (((val >> 8) & 0xf) == 3) ? "5dB" :
                       (((val >> 8) & 0xf) == 4) ? "6dB" :
                       (((val >> 8) & 0xf) == 5) ? "7dB" :
                       (((val >> 8) & 0xf) == 6) ? "8dB" :
                       (((val >> 8) & 0xf) == 7) ? "9dB" :
                       (((val >> 8) & 0xf) == 8) ? "10dB" :
                       (((val >> 8) & 0xf) == 9) ? "11dB" :
                       (((val >> 8) & 0xf) == 10) ? "12dB" :
                       (((val >> 8) & 0xf) == 11) ? "13dB" :
                       (((val >> 8) & 0xf) == 12) ? "14dB" :
                       (((val >> 8) & 0xf) == 13) ? "15dB" :
                       (((val >> 8) & 0xf) == 14) ? "16dB" : "17dB");
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(SPK_CON3);
                printf("  SPK R-Ch Vol(%04xh[13:12]): %s\n", SPK_CON3, (((val >> 12) & 0x3) == 0) ? "mute" :
                       (((val >> 12) & 0x3) == 1) ? "-6dB" :
                       (((val >> 12) & 0x3) == 2) ? "-3dB" : "0dB");
#endif
#ifdef MT8135_AUD_REG
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(SPK_CON5);
                printf("  SPK R PGA_GAIN(%04x[14:11]): %s\n", SPK_CON5, (((val >> 11) & 0xf) == 0) ? "mute" :
                       (((val >> 11) & 0xf) == 1) ? "0dB" :
                       (((val >> 11) & 0xf) == 2) ? "4dB" :
                       (((val >> 11) & 0xf) == 3) ? "5dB" :
                       (((val >> 11) & 0xf) == 4) ? "6dB" :
                       (((val >> 11) & 0xf) == 5) ? "7dB" :
                       (((val >> 11) & 0xf) == 6) ? "8dB" :
                       (((val >> 11) & 0xf) == 7) ? "9dB" :
                       (((val >> 11) & 0xf) == 8) ? "10dB" :
                       (((val >> 11) & 0xf) == 9) ? "11dB" :
                       (((val >> 11) & 0xf) == 10) ? "12dB" :
                       (((val >> 11) & 0xf) == 11) ? "13dB" :
                       (((val >> 11) & 0xf) == 12) ? "14dB" :
                       (((val >> 11) & 0xf) == 13) ? "15dB" :
                       (((val >> 11) & 0xf) == 14) ? "16dB" : "17dB");
#endif                       
#endif
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(AUDPREAMPGAIN_CON0);

                printf("  Preamp-R gain(%04xh[6:4]): %s\n", AUDPREAMPGAIN_CON0, (((val >> 4) & 0x7) == 0) ? "2dB" :
                       (((val >> 4) & 0x7) == 1) ? "8dB" :
                       (((val >> 4) & 0x7) == 2) ? "14dB" :
                       (((val >> 4) & 0x7) == 3) ? "20dB" :
                       (((val >> 4) & 0x7) == 4) ? "26dB" :
                       (((val >> 4) & 0x7) == 5) ? "32dB" : "NA");
                printf("  Preamp-L gain(%04xh[2:0]): %s\n", AUDPREAMPGAIN_CON0, (((val >> 0) & 0x7) == 0) ? "2dB" :
                       (((val >> 0) & 0x7) == 1) ? "8dB" :
                       (((val >> 0) & 0x7) == 2) ? "14dB" :
                       (((val >> 0) & 0x7) == 3) ? "20dB" :
                       (((val >> 0) & 0x7) == 4) ? "26dB" :
                       (((val >> 0) & 0x7) == 5) ? "32dB" : "NA");
#endif                       
#ifdef MTK_PMIC_MT6397
                val = mAnalogReg->GetAnalogReg(ZCD_CON1);
                printf("  Line-In gian(%04xh[3:0]): %s\n", ZCD_CON1, (((val >> 0) & 0xf) == 0) ? "10dB" :
                       (((val >> 0) & 0xf) == 1) ? "8dB" :
                       (((val >> 0) & 0xf) == 2) ? "6dB" :
                       (((val >> 0) & 0xf) == 3) ? "4dB" :
                       (((val >> 0) & 0xf) == 4) ? "2dB" :
                       (((val >> 0) & 0xf) == 5) ? "0dB" :
                       (((val >> 0) & 0xf) == 6) ? "-2dB" :
                       (((val >> 0) & 0xf) == 7) ? "-4dB" :
                       (((val >> 0) & 0xf) == 8) ? "-6dB" :
                       (((val >> 0) & 0xf) == 9) ? "-8dB" :
                       (((val >> 0) & 0xf) == 10) ? "-10dB" :
                       (((val >> 0) & 0xf) == 11) ? "-12dB" :
                       (((val >> 0) & 0xf) == 12) ? "-14dB" :
                       (((val >> 0) & 0xf) == 13) ? "-16dB" :
                       (((val >> 0) & 0xf) == 14) ? "-18dB" : "-20dB");
                val = mAnalogReg->GetAnalogReg(ZCD_CON2);
                printf("  HPR gain(%04xh[11:8]): %s\n", ZCD_CON2, (((val >> 8) & 0xf) == 0) ? "8dB" :
                       (((val >> 8) & 0xf) == 1) ? "7dB" :
                       (((val >> 8) & 0xf) == 2) ? "6dB" :
                       (((val >> 8) & 0xf) == 3) ? "5dB" :
                       (((val >> 8) & 0xf) == 4) ? "4dB" :
                       (((val >> 8) & 0xf) == 5) ? "3dB" :
                       (((val >> 8) & 0xf) == 6) ? "2dB" :
                       (((val >> 8) & 0xf) == 7) ? "1dB" :
                       (((val >> 8) & 0xf) == 8) ? "0dB" :
                       (((val >> 8) & 0xf) == 9) ? "-1dB" :
                       (((val >> 8) & 0xf) == 10) ? "-2dB" :
                       (((val >> 8) & 0xf) == 11) ? "-3dB" :
                       (((val >> 8) & 0xf) == 12) ? "-4dB" :
                       (((val >> 8) & 0xf) == 13) ? "NA" :
                       (((val >> 8) & 0xf) == 14) ? "NA" : "-40dB(Mute)");
                printf("  HPL gain(%04xh[3:0]): %s\n", ZCD_CON2, (((val >> 0) & 0xf) == 0) ? "8dB" :
                       (((val >> 0) & 0xf) == 1) ? "7dB" :
                       (((val >> 0) & 0xf) == 2) ? "6dB" :
                       (((val >> 0) & 0xf) == 3) ? "5dB" :
                       (((val >> 0) & 0xf) == 4) ? "4dB" :
                       (((val >> 0) & 0xf) == 5) ? "3dB" :
                       (((val >> 0) & 0xf) == 6) ? "2dB" :
                       (((val >> 0) & 0xf) == 7) ? "1dB" :
                       (((val >> 0) & 0xf) == 8) ? "0dB" :
                       (((val >> 0) & 0xf) == 9) ? "-1dB" :
                       (((val >> 0) & 0xf) == 10) ? "-2dB" :
                       (((val >> 0) & 0xf) == 11) ? "-3dB" :
                       (((val >> 0) & 0xf) == 12) ? "-4dB" :
                       (((val >> 0) & 0xf) == 13) ? "NA" :
                       (((val >> 0) & 0xf) == 14) ? "NA" : "-40dB(Mute)");
                val = mAnalogReg->GetAnalogReg(ZCD_CON3);
                printf("  HS gain(%04xh[3:0]): %s\n", ZCD_CON3, (((val >> 0) & 0xf) == 0) ? "8dB" :
                       (((val >> 0) & 0xf) == 1) ? "7dB" :
                       (((val >> 0) & 0xf) == 2) ? "6dB" :
                       (((val >> 0) & 0xf) == 3) ? "5dB" :
                       (((val >> 0) & 0xf) == 4) ? "4dB" :
                       (((val >> 0) & 0xf) == 5) ? "3dB" :
                       (((val >> 0) & 0xf) == 6) ? "2dB" :
                       (((val >> 0) & 0xf) == 7) ? "1dB" :
                       (((val >> 0) & 0xf) == 8) ? "0dB" :
                       (((val >> 0) & 0xf) == 9) ? "-1dB" :
                       (((val >> 0) & 0xf) == 10) ? "-2dB" :
                       (((val >> 0) & 0xf) == 11) ? "-3dB" :
                       (((val >> 0) & 0xf) == 12) ? "-4dB" :
                       (((val >> 0) & 0xf) == 13) ? "NA" :
                       (((val >> 0) & 0xf) == 14) ? "NA" : "-40dB(Mute)");
                val = mAnalogReg->GetAnalogReg(ZCD_CON4);
                printf("  IV-R gain(%04xh[10:8]): %s\n", ZCD_CON4, (((val >> 8) & 0x7) == 0) ? "5dB" :
                       (((val >> 8) & 0x7) == 1) ? "4dB" :
                       (((val >> 8) & 0x7) == 2) ? "3dB" :
                       (((val >> 8) & 0x7) == 3) ? "2dB" :
                       (((val >> 8) & 0x7) == 4) ? "1dB" :
                       (((val >> 8) & 0x7) == 5) ? "0dB" :
                       (((val >> 8) & 0x7) == 6) ? "-1dB" : "-2dB");
                printf("  IV-L gain(%04xh[2:0]): %s\n", ZCD_CON4, (((val >> 0) & 0x7) == 0) ? "5dB" :
                       (((val >> 0) & 0x7) == 1) ? "4dB" :
                       (((val >> 0) & 0x7) == 2) ? "3dB" :
                       (((val >> 0) & 0x7) == 3) ? "2dB" :
                       (((val >> 0) & 0x7) == 4) ? "1dB" :
                       (((val >> 0) & 0x7) == 5) ? "0dB" :
                       (((val >> 0) & 0x7) == 6) ? "-1dB" : "-2dB");
#endif                       
            }
            else
            {
                printf("usage: hwq [module name]\n"
                       "  intcon\n"
                       "  pdn\n"
                       "  mem\n"
                       "  2ndi2s\n"
                       "  i2s\n"
                       "  gain12\n"
                       "  mrgif\n"
                       "  dai\n"
                       "  mcu\n"
                       "  mux\n"
                       "  anapdn\n"
                       "  anagain\n");
            }
        }
        else if (!strncmp(cmd, "hwtest", 6))
        {
            if (!strcmp(cmd + 7, "recvon"))
            {
                init_audio_meta();
                RecieverTest(1);
            }
            else if (!strcmp(cmd + 7, "recvoff"))
            {
                init_audio_meta();
                RecieverTest(0);
            }
            else if (!strcmp(cmd + 7, "hpon"))
            {
                init_audio_meta();
                EarphoneTest(1);
            }
            else if (!strcmp(cmd + 7, "hpoff"))
            {
                init_audio_meta();
                EarphoneTest(0);
            }
            else if (!strcmp(cmd + 7, "spkon"))
            {
                init_audio_meta();
                LouderSPKTest(1, 1);
            }
            else if (!strcmp(cmd + 7, "spkoff"))
            {
                init_audio_meta();
                LouderSPKTest(0, 0);
            }
            else if (!strcmp(cmd + 7, "recvloopon"))
            {
                init_audio_meta();
                RecieverLoopbackTest(1);
            }
            else if (!strcmp(cmd + 7, "recvloopoff"))
            {
                init_audio_meta();
                RecieverLoopbackTest(0);
            }
            else if (!strcmp(cmd + 7, "recvloop2on"))
            {
                init_audio_meta();
                RecieverLoopbackTest_Mic2(1);
            }
            else if (!strcmp(cmd + 7, "recvloop2off"))
            {
                init_audio_meta();
                RecieverLoopbackTest_Mic2(0);
            }
            else if (!strcmp(cmd + 7, "hploopon"))
            {
                init_audio_meta();
                HeadsetMic_EarphoneLR_Loopback(1);
            }
            else if (!strcmp(cmd + 7, "hploopoff"))
            {
                init_audio_meta();
                HeadsetMic_EarphoneLR_Loopback(0);
            }
            else
            {
                printf("Usage: hwtest [option]\n");
                printf("  recvon/recvoff\n");
                printf("  hpon/hpoff\n");
                printf("  spkon/spkoff\n");
                printf("  recvloopon/recvloopoff\n");
                printf("  recvloop2on/recvloop2off\n");
                printf("  hploopon/hploopoff\n");
            }
        }
        else if (!strncmp(cmd, "loopback ", 9))
        {
            int input = 1;
            int output = 1;
            sscanf(cmd, "loopback %d %d\n", &input, &output);
            if ((input == 0) || (output == 0) || (input > 3) || (output > 3))
            {
                android::LoopbackManager::GetInstance()->SetLoopbackOff();
                printf("Disable loopback\n");
            }
            else
            {
                android::LoopbackManager::GetInstance()->SetLoopbackOn((android::loopback_t)input, (android::loopback_output_device_t)output);
                printf("Enable loopback: %d %d\n", input, output);
            }
        }
        else if (!strcmp(cmd, "out_status"))
        {
            list_output_status();
        }
        else if (!strcmp(cmd, "out_dev_status"))
        {
            list_output_device_status();
        }
        else if (!strncmp(cmd, "connect", 7))
        {
            int out_device;
            int ret;
            ret = sscanf(cmd, "connect %x", &out_device);
            if (ret == 1)
            {
                android::AudioSystem::setDeviceConnectionState((audio_devices_t)out_device, AUDIO_POLICY_DEVICE_STATE_AVAILABLE, NULL);
            }
            else
            {
                list_audio_output_devices();
            }
        }
        else if (!strncmp(cmd, "disconnect", 10))
        {
            int out_device;
            int ret;
            ret = sscanf(cmd, "disconnect %x", &out_device);
            if (ret == 1)
            {
                android::AudioSystem::setDeviceConnectionState((audio_devices_t)out_device, AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE, NULL);
            }
            else
            {
                list_audio_output_devices();
            }
        }
        else if (!strcmp(cmd, "forced_use_status"))
        {
            list_forced_use_status();
        }
        else if (!strcmp(cmd, "get_output_status"))
        {
            list_get_output_status();
        }
        else if (!strcmp(cmd, "get_input_status"))
        {
            list_get_input_status();
        }
        else if (!strcmp(cmd, "dump_audio_policy"))
        {
            dump_audio_policy();
        }
        else if (!strcmp(cmd, "dump_audio_flinger"))
        {
            dump_audio_flinger();
        }
        else if (!strncmp(cmd, "audio_dump", 10))
        {
            process_audio_dump_cmd(cmd);
        }
        else if (!strcmp(cmd, "eng"))
        {
            system("am start com.mediatek.engineermode/com.mediatek.engineermode.EngineerMode");
        }
        else if (!strcmp(cmd, "exit"))
        {
            break;
        }
        else
        {
            if (strcmp(cmd, ""))
            {
                printf("unknown command: <%s>\n", cmd);
            }
            list_usage();
        }
    }
    //scanf("%s", cmd);
    //printf("  echo: %s\n", cmd);

    //list_audio_stream_type();
    //list_audio_devices();
    //list_getStreamVolumeIndex();

    return 0;
}

static const char *gAudioDumpList[] =
{
    "streamout.pcm.dump",
    "streamout.hdmi.dump",
    "a2dp.streamout.pcm",
    "streamin.pcm.dump",
    "SPE.pcm.dump",
    "SPEOut.pcm.dump"
    "SPEIn.pcm.dump",
    "APVM.dump",
    "af.mixer.pcm",
    "af.track.pcm",
    "af.record.dump.pcm",
    "omx_audio_dump",
    "audio.dumpenc.aac",
    "audio.dumpenc.vorbis",
    "audio.dumpenc.amr",
    "audio.dumpenc.awb",
    "audio.dumpenc.adpcm"
};

#define AUDIO_DUMP_LIST_LEN ((int)(sizeof(gAudioDumpList)/sizeof(gAudioDumpList[0])))

void process_audio_dump_cmd(char *cmd)
{
    int item_no;
    int ret;
    char value[PROPERTY_VALUE_MAX];
    int result;
    ret = sscanf(cmd, "audio_dump %d\n", &item_no);
    if (ret == 1)
    {
        if (item_no < AUDIO_DUMP_LIST_LEN && item_no >= 0)
        {
            property_get(gAudioDumpList[item_no], value, "0");
            result = atoi(value);
            sprintf(value, "%d", 1 - result);
            property_set(gAudioDumpList[item_no], value);
        }
        else
        {
            printf("item no %d invalid\n", item_no);
        }
    }
    else
    {
        for (int i = 0; i < AUDIO_DUMP_LIST_LEN; i++)
        {
            property_get(gAudioDumpList[i], value, "0");
            result = atoi(value);
            printf("%d: %s=%d\n", i, gAudioDumpList[i], result);
        }
    }
}

