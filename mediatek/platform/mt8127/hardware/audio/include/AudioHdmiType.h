#ifndef _AUDIO_HDMI_TYPE_H
#define _AUDIO_HDMI_TYPE_H

class AudioHdmiType
{
    public:
        enum HDMI_MEM_TYPE
        {
            HDMI_MEM_STEREO_PCM = 0,
            HDMI_MEM_MULTI_CH_PCM,
            HDMI_MEM_TYPE_COUNT
        };

        enum HDMI_FS_TYPE
        {
            HDMI_FS_32K = 0,
            HDMI_FS_44_1K,
            HDMI_FS_48K,
            HDMI_FS_88_2K,
            HDMI_FS_96K,
            HDMI_FS_176_4K,
            HDMI_FS_192K,
            HDMI_FS_TYPE_COUNT
        };
};

class AudioHdmiOutControl
{
    public:
        enum HDMI_OUTPUT_CHANNEL
        {
            HDMI_NO_OUTPUT_CHANNEL = 0,
            HDMI_OUTPUT_1CH,
            HDMI_OUTPUT_2CH,
            HDMI_OUTPUT_3CH,
            HDMI_OUTPUT_4CH,
            HDMI_OUTPUT_5CH,
            HDMI_OUTPUT_6CH,
            HDMI_OUTPUT_7CH,
            HDMI_OUTPUT_8CH
        };

        enum HDMI_INPUT_DATA_BIT_WIDTH
        {
            HDMI_INPUT_16BIT = 0,
            HDMI_INPUT_32BIT
        };

        int mHDMI_OUT_CH_NUM;
        int mHDMI_OUT_BIT_WIDTH;
};

class AudioHdmiI2S
{
    public:
        enum HDMI_I2S_WLEN
        {
            HDMI_I2S_8BIT = 0,
            HDMI_I2S_16BIT,
            HDMI_I2S_24BIT,
            HDMI_I2S_32BIT
        };

        enum HDMI_I2S_DELAY_DATA
        {
            HDMI_I2S_NOT_DELAY = 0,
            HDMI_I2S_FIRST_BIT_1T_DELAY
        };

        enum HDMI_I2S_LRCK_INV
        {
            HDMI_I2S_LRCK_NOT_INVERSE = 0,
            HDMI_I2S_LRCK_INVERSE
        };

        enum HDMI_I2S_BCLK_INV
        {
            HDMI_I2S_BCLK_NOT_INVERSE = 0,
            HDMI_I2S_BCLK_INVERSE
        };

        int mI2S_WLEN;
        bool mI2S_DELAY_DATA;
        bool mI2S_LRCK_INV;
        bool mI2S_BCLK_INV;
};

class AudioHDMIClockSetting
{
    public:
        enum HDMI_CLK_APLL_SEL
        {
            CLSQ_MUX_CK = 0,
            AD_APLL_CK,
            APLL_D4,
            APLL_D8,
            APLL_D16,
            APLL_D24
        };

        int SAMPLE_RATE;
        int CLK_APLL_SEL;
        int HDMI_BCK_DIV;
};

#endif
