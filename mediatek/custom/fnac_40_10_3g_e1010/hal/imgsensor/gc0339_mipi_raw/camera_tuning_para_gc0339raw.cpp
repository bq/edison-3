#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_gc0339raw.h"
#include "camera_info_gc0339raw.h"
#include "camera_custom_AEPlinetable.h"
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,
    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    }
    },
    ISPPca:{
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
        },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        70300,    // i4R_AVG
        8906,    // i4R_STD
        92267,    // i4B_AVG
        24788,    // i4B_STD
        {  // i4P00[9]
            5383333, -2373333, -453333, -790000, 3470000, -120000, 120000, -1833333, 4270000
        },
        {  // i4P10[9]
            639683, -691536, 50691, 25718, -78211, 52494, 39217, -43419, 3040
        },
        {  // i4P01[9]
            534236, -502884, -26376, -173683, -14681, 188364, -40808, -131880, 177664
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1024,    // u4MinGain, 1024 base = 1x
            4096,    // u4MaxGain, 16x
            100,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            80,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            80,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            80,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            28,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            2,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {86, 108, 128, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 34}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            TRUE,    // bEnableCaptureThres
            TRUE,    // bEnableVideoThres
            TRUE,    // bEnableStrobeThres
            47,    // u4AETarget
            47,    // u4StrobeAETarget
            20,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -10,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            32,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
            50,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            50,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            75    // u4FlatnessStrength
        }
    },
    // AWB NVRAM
    {
        // AWB calibration data
        {
            // rUnitGain (unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rGoldenGain (golden sample gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                784,    // i4R
                643,    // i4G
                512    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                202,    // i4X
                -22    // i4Y
            },
            // Horizon
            {
                -394,    // i4X
                63    // i4Y
            },
            // A
            {
                -269,    // i4X
                25    // i4Y
            },
            // TL84
            {
                -89,    // i4X
                -62    // i4Y
            },
            // CWF
            {
                -41,    // i4X
                -96    // i4Y
            },
            // DNP
            {
                30,    // i4X
                -28    // i4Y
            },
            // D65
            {
                157,    // i4X
                11    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                203,    // i4X
                -15    // i4Y
            },
            // Horizon
            {
                -396,    // i4X
                49    // i4Y
            },
            // A
            {
                -270,    // i4X
                16    // i4Y
            },
            // TL84
            {
                -87,    // i4X
                -65    // i4Y
            },
            // CWF
            {
                -38,    // i4X
                -97    // i4Y
            },
            // DNP
            {
                31,    // i4X
                -27    // i4Y
            },
            // D65
            {
                157,    // i4X
                17    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                884,    // i4R
                653,    // i4G
                512    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                951,    // i4G
                1489    // i4B
            },
            // A 
            {
                512,    // i4R
                762,    // i4G
                1062    // i4B
            },
            // TL84 
            {
                512,    // i4R
                531,    // i4G
                652    // i4B
            },
            // CWF 
            {
                552,    // i4R
                512,    // i4G
                616    // i4B
            },
            // DNP 
            {
                555,    // i4R
                513,    // i4G
                512    // i4B
            },
            // D65 
            {
                784,    // i4R
                643,    // i4G
                512    // i4B
            },
            // DF 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            }
        },
        // Rotation matrix parameter
        {
            -2,    // i4RotationAngle
            256,    // i4Cos
            -9    // i4Sin
        },
        // Daylight locus parameter
        {
            -117,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -137,    // i4RightBound
            -787,    // i4LeftBound
            83,    // i4UpperBound
            -17    // i4LowerBound
            },
            // Warm fluorescent
            {
            -137,    // i4RightBound
            -787,    // i4LeftBound
            -17,    // i4UpperBound
            -137    // i4LowerBound
            },
            // Fluorescent
            {
            -19,    // i4RightBound
            -137,    // i4LeftBound
            90,    // i4UpperBound
            -81    // i4LowerBound
            },
            // CWF
            {
            -19,    // i4RightBound
            -137,    // i4LeftBound
            -81,    // i4UpperBound
            -147    // i4LowerBound
            },
            // Daylight
            {
            182,    // i4RightBound
            -19,    // i4LeftBound
            97,    // i4UpperBound
            -100    // i4LowerBound
            },
            // Shade
            {
            542,    // i4RightBound
            182,    // i4LeftBound
            97,    // i4UpperBound
            -100    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            542,    // i4RightBound
            -787,    // i4LeftBound
            97,    // i4UpperBound
            -147    // i4LowerBound
            },
            // Daylight
            {
            207,    // i4RightBound
            -19,    // i4LeftBound
            97,    // i4UpperBound
            -100    // i4LowerBound
            },
            // Cloudy daylight
            {
            307,    // i4RightBound
            132,    // i4LeftBound
            97,    // i4UpperBound
            -100    // i4LowerBound
            },
            // Shade
            {
            407,    // i4RightBound
            132,    // i4LeftBound
            97,    // i4UpperBound
            -100    // i4LowerBound
            },
            // Twilight
            {
            -19,    // i4RightBound
            -179,    // i4LeftBound
            97,    // i4UpperBound
            -100    // i4LowerBound
            },
            // Fluorescent
            {
            207,    // i4RightBound
            -187,    // i4LeftBound
            67,    // i4UpperBound
            -147    // i4LowerBound
            },
            // Warm fluorescent
            {
            -170,    // i4RightBound
            -370,    // i4LeftBound
            67,    // i4UpperBound
            -147    // i4LowerBound
            },
            // Incandescent
            {
            -170,    // i4RightBound
            -370,    // i4LeftBound
            97,    // i4UpperBound
            -100    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            585,    // i4R
            512,    // i4G
            454    // i4B
            },
            // Cloudy daylight
            {
            698,    // i4R
            512,    // i4G
            385    // i4B
            },
            // Shade
            {
            748,    // i4R
            512,    // i4G
            361    // i4B
            },
            // Twilight
            {
            447,    // i4R
            512,    // i4G
            584    // i4B
            },
            // Fluorescent
            {
            547,    // i4R
            512,    // i4G
            534    // i4B
            },
            // Warm fluorescent
            {
            370,    // i4R
            512,    // i4G
            770    // i4B
            },
            // Incandescent
            {
            352,    // i4R
            512,    // i4G
            730    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            50,    // i4SliderValue
            4007    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            50,    // i4SliderValue
            4007    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            341    // i4OffsetThr
            },
            // Daylight WB gain
            {
            558,    // i4R
            546,    // i4G
            512    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -553,    // i4RotatedXCoordinate[0]
                -427,    // i4RotatedXCoordinate[1]
                -244,    // i4RotatedXCoordinate[2]
                -126,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T)};

    if (CameraDataType > CAMERA_DATA_AE_PLINETABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        default:
            break;
    }
    return 0;
}}; // NSFeature


