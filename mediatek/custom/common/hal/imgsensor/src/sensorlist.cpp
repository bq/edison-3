#include <utils/Log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>


//#include "msdk_sensor_exp.h"
#include "camera_custom_sensor.h"
#if 1// seanlin 120925 for default #if (defined(IMX073_MIPI_RAW)||defined(OV5642_RAW))
//#include "camera_calibration_eeprom.h"
#include "camera_calibration_cam_cal.h"
#endif
#include "kd_imgsensor.h"
//#include "image_sensor.h"

//TODO:remove once build system ready
//#include "camera_custom_cfg.h"

//#include "src/lib/inc/MediaLog.h"

//#define LOG_TAG "SENSORLIST"
//MUINT32 MT9P012_getCalData(PGET_SENSOR_CALIBRATION_DATA_STRUCT pGetCalData);


#define YUV_INFO(_id, name, getCalData)\
    { \
    _id, name, \
    NSFeature::YUVSensorInfo<_id>::createInstance(name, #name), \
    (NSFeature::SensorInfoBase*(*)()) \
    NSFeature::YUVSensorInfo<_id>::getInstance, \
    NSFeature::YUVSensorInfo<_id>::getDefaultData, \
    getCalData, \
	NSFeature::YUVSensorInfo<_id>::getNullFlickerPara \
    }
#define RAW_INFO(_id, name, getCalData)\
    { \
    _id, name, \
    NSFeature::RAWSensorInfo<_id>::createInstance(name, #name), \
    (NSFeature::SensorInfoBase*(*)()) \
    NSFeature::RAWSensorInfo<_id>::getInstance, \
    NSFeature::RAWSensorInfo<_id>::getDefaultData, \
    getCalData, \
	NSFeature::RAWSensorInfo<_id>::getFlickerPara \
    }


//MSDK_SENSOR_INIT_FUNCTION_STRUCT SensorList[MAX_NUM_OF_SUPPORT_SENSOR] =
MSDK_SENSOR_INIT_FUNCTION_STRUCT SensorList[] =
{
#if defined(OV16825_MIPI_RAW)
	RAW_INFO(OV16825MIPI_SENSOR_ID, SENSOR_DRVNAME_OV16825_MIPI_RAW,NULL), 
#endif
#if defined(IMX135_MIPI_RAW)
		RAW_INFO(IMX135_SENSOR_ID, SENSOR_DRVNAME_IMX135_MIPI_RAW,NULL), 
#endif
#if defined(IMX220_MIPI_RAW)
		RAW_INFO(IMX220_SENSOR_ID, SENSOR_DRVNAME_IMX220_MIPI_RAW,NULL), 
#endif

#if defined(OV8830_RAW)
    RAW_INFO(OV8830_SENSOR_ID, SENSOR_DRVNAME_OV8830_RAW, NULL),
#endif
#if defined(OV9740_MIPI_YUV)
    YUV_INFO(OV9740MIPI_SENSOR_ID, SENSOR_DRVNAME_OV9740_MIPI_YUV,NULL),
#endif
#if defined(IMX073_MIPI_RAW)
    RAW_INFO(IMX073_SENSOR_ID, SENSOR_DRVNAME_IMX073_MIPI_RAW,CAM_CALGetCalData),
#endif
#if defined(OV8825_MIPI_RAW)
    RAW_INFO(OV8825_SENSOR_ID, SENSOR_DRVNAME_OV8825_MIPI_RAW,CAM_CALGetCalData),
#endif
#if defined(OV12830_MIPI_RAW)
    RAW_INFO(OV12830_SENSOR_ID, SENSOR_DRVNAME_OV12830_MIPI_RAW,NULL),
#endif
#if defined(S5K3H2YX_MIPI_RAW)
	RAW_INFO(S5K3H2YX_SENSOR_ID, SENSOR_DRVNAME_S5K3H2YX_MIPI_RAW,NULL),
#endif
#if defined(S5K3H7Y_MIPI_RAW)
    RAW_INFO(S5K3H7Y_SENSOR_ID, SENSOR_DRVNAME_S5K3H7Y_MIPI_RAW,CAM_CALGetCalData),
#endif

#if defined(S5K4E1GA_MIPI_RAW)
    RAW_INFO(S5K4E1GA_SENSOR_ID, SENSOR_DRVNAME_S5K4E1GA_MIPI_RAW,NULL),
#endif
#if defined(OV5642_RAW)
    RAW_INFO(OV5642_SENSOR_ID, SENSOR_DRVNAME_OV5642_RAW,CAM_CALGetCalData),//aovid compilier error wo option name CAM_CALGetCalData),
#endif
#if defined(HI542_RAW)
    RAW_INFO(HI542_SENSOR_ID, SENSOR_DRVNAME_HI542_RAW, NULL),
#endif
#if defined(HI542_MIPI_RAW)
    RAW_INFO(HI542MIPI_SENSOR_ID, SENSOR_DRVNAME_HI542MIPI_RAW, NULL),
#endif
#if defined(OV5642_MIPI_YUV)
    YUV_INFO(OV5642_SENSOR_ID, SENSOR_DRVNAME_OV5642_MIPI_YUV, NULL),
#endif
#if defined(OV5642_MIPI_RGB)
    YUV_INFO(OV5642_SENSOR_ID, SENSOR_DRVNAME_OV5642_MIPI_RGB, NULL),
#endif
#if defined(OV5642_MIPI_JPG)
    YUV_INFO(OV5642_SENSOR_ID, SENSOR_DRVNAME_OV5642_MIPI_JPG, NULL),
#endif
#if defined(OV5642_YUV)
    YUV_INFO(OV5642_SENSOR_ID, SENSOR_DRVNAME_OV5642_YUV, NULL),
#endif
#if defined(OV5647_MIPI_RAW)
    RAW_INFO(OV5647MIPI_SENSOR_ID, SENSOR_DRVNAME_OV5647MIPI_RAW, CAM_CALGetCalData),
#endif
#if defined(GC2035_YUV)
    YUV_INFO(GC2035_SENSOR_ID, SENSOR_DRVNAME_GC2035_YUV, NULL), 
#endif
#if defined(OV5647_RAW)
    RAW_INFO(OV5647_SENSOR_ID, SENSOR_DRVNAME_OV5647_RAW, NULL),
#endif
#if defined(OV5648_MIPI_RAW)
    RAW_INFO(OV5648MIPI_SENSOR_ID, SENSOR_DRVNAME_OV5648_MIPI_RAW, NULL),
#endif
#if defined(OV5693_MIPI_RAW)
       RAW_INFO(OV5693_SENSOR_ID, SENSOR_DRVNAME_OV5693_MIPI_RAW, NULL), 
#endif
#if defined(OV5650_RAW)
    RAW_INFO(OV5650_SENSOR_ID, SENSOR_DRVNAME_OV5650_RAW, NULL),
#endif
#if defined(OV5650MIPI_RAW)
    RAW_INFO(OV5650MIPI_SENSOR_ID, SENSOR_DRVNAME_OV5650MIPI_RAW, NULL),
#endif
#if defined(MT9P012_RAW)
    RAW_INFO(MT9P012_SENSOR_ID, SENSOR_DRVNAME_MT9P012_RAW, NULL),
#endif
#if defined(MT9P015_RAW)
    RAW_INFO(MT9P015_SENSOR_ID, SENSOR_DRVNAME_MT9P015_RAW, NULL),
#endif
#if defined(MT9P017_RAW)
    RAW_INFO(MT9P017_SENSOR_ID, SENSOR_DRVNAME_MT9P017_RAW, NULL),
#endif
#if defined(MT9P017_MIPI_RAW)
    RAW_INFO(MT9P017MIPI_SENSOR_ID, SENSOR_DRVNAME_MT9P017_MIPI_RAW, NULL),
#endif
#if defined(MT9D115_MIPI_RAW)
    RAW_INFO(MT9D115MIPI_SENSOR_ID, SENSOR_DRVNAME_MT9D115_MIPI_RAW, NULL),
#endif
#if defined(A5141_MIPI_RAW)
    RAW_INFO(A5141MIPI_SENSOR_ID, SENSOR_DRVNAME_A5141_MIPI_RAW, NULL),
#endif
#if defined(A5142_MIPI_RAW)
    RAW_INFO(A5142MIPI_SENSOR_ID, SENSOR_DRVNAME_A5142_MIPI_RAW, CAM_CALGetCalData), 
#endif
#if defined(HM3451_RAW)
	RAW_INFO(HM3451_SENSOR_ID,SENSOR_DRVNAME_HM3451_RAW,NULL),
#endif
#if defined(AR0833_MIPI_RAW)
    RAW_INFO(AR0833_SENSOR_ID, SENSOR_DRVNAME_AR0833_MIPI_RAW,NULL),
#endif
#if defined(OV3640_RAW)
    RAW_INFO(OV3640_SENSOR_ID, SENSOR_DRVNAME_OV3640_RAW, NULL),
#endif
#if defined(OV3640_YUV)
    YUV_INFO(OV3640_SENSOR_ID, SENSOR_DRVNAME_OV3640_YUV, NULL),
#endif
#if defined(OV3640_YUV_AF)
    YUV_INFO(OV3640_SENSOR_ID, SENSOR_DRVNAME_OV3640_YUV, NULL),
#endif
#if defined(MT9T113_YUV)
    YUV_INFO(MT9T113_SENSOR_ID, SENSOR_DRVNAME_MT9T113_YUV,NULL),
#endif
#if defined(MT9T113_MIPI_YUV)
    YUV_INFO(MT9T113MIPI_SENSOR_ID, SENSOR_DRVNAME_MT9T113_MIPI_YUV,NULL),
#endif
#if defined(S5K5CAGX_YUV)
    YUV_INFO(S5K5CAGX_SENSOR_ID, SENSOR_DRVNAME_S5K5CAGX_YUV,NULL),
#endif
#if defined(S5K4ECGX_YUV)
    YUV_INFO(S5K4ECGX_SENSOR_ID, SENSOR_DRVNAME_S5K4ECGX_YUV,NULL),
#endif
#if defined(S5K4ECGX_MIPI_YUV)
    YUV_INFO(S5K4ECGX_SENSOR_ID, SENSOR_DRVNAME_S5K4ECGX_MIPI_YUV,NULL),
#endif
#if defined(S5K4ECGX_MIPI_JPG)
    YUV_INFO(S5K4ECGX_SENSOR_ID, SENSOR_DRVNAME_S5K4ECGX_MIPI_JPG,NULL), 
#endif
#if defined(OV2650_RAW)
    RAW_INFO(OV2650_SENSOR_ID, SENSOR_DRVNAME_OV2650_RAW, NULL),
#endif
#if defined(OV2655_YUV)
    YUV_INFO(OV2650_SENSOR_ID, SENSOR_DRVNAME_OV2655_YUV, NULL),
#endif
#if defined(HI253_YUV)
    YUV_INFO(HI253_SENSOR_ID, SENSOR_DRVNAME_HI253_YUV,NULL),
#endif
#if defined(OV2659_YUV)
    YUV_INFO(OV2659_SENSOR_ID, SENSOR_DRVNAME_OV2659_YUV, NULL),
#endif
#if defined(OV7675_YUV)
    YUV_INFO(OV7675_SENSOR_ID, SENSOR_DRVNAME_OV7675_YUV, NULL),
#endif
#if defined(MT9V113_YUV)
    YUV_INFO(MT9V113_SENSOR_ID, SENSOR_DRVNAME_MT9V113_YUV, NULL),
#endif
#if defined(MT9V114_YUV)
    YUV_INFO(MT9V114_SENSOR_ID, SENSOR_DRVNAME_MT9V114_YUV,NULL),
#endif
#if defined(MT9V115_YUV)
    YUV_INFO(MT9V115_SENSOR_ID, SENSOR_DRVNAME_MT9V115_YUV,NULL),
#endif
#if defined(SIV120B_YUV)
    YUV_INFO(SIV120B_SENSOR_ID, SENSOR_DRVNAME_SIV120B_YUV, NULL),
#endif
#if defined(PAS6180_SERIAL_YUV)
    YUV_INFO(PAS6180_SENSOR_ID, SENSOR_DRVNAME_PAS6180_SERIAL_YUV, NULL),
#endif
#if defined(HI704_YUV)
    YUV_INFO(HI704_SENSOR_ID, SENSOR_DRVNAME_HI704_YUV,NULL),
#endif
#if defined(IMX105_MIPI_RAW)
    RAW_INFO(IMX105_SENSOR_ID, SENSOR_DRVNAME_IMX105_MIPI_RAW,NULL),
#endif
#if defined(OV9726_RAW)
    RAW_INFO(OV9726_SENSOR_ID, SENSOR_DRVNAME_0V9726_RAW,NULL),
#endif
#if defined(OV9726_MIPI_RAW)
    RAW_INFO(OV9726MIPI_SENSOR_ID, SENSOR_DRVNAME_OV9726_MIPI_RAW,NULL),
#endif
#if defined(OV9760_MIPI_RAW)
    RAW_INFO(OV9760MIPI_SENSOR_ID, SENSOR_DRVNAME_OV9760_MIPI_RAW,NULL), 
#endif
#if defined(SIV121D_YUV)
    YUV_INFO(SIV121D_SENSOR_ID, SENSOR_DRVNAME_SIV121D_YUV,NULL),
#endif

#if defined(S5K8AAYX_MIPI_YUV)
    YUV_INFO(S5K8AAYX_MIPI_SENSOR_ID, SENSOR_DRVNAME_S5K8AAYX_MIPI_YUV,NULL),
#endif
#if defined(S5K8AAYX_YUV)
    YUV_INFO(S5K8AAYX_SENSOR_ID, SENSOR_DRVNAME_S5K8AAYX_YUV,NULL),
#endif
#if defined(SP0A19_YUV)
    YUV_INFO(SP0A19_YUV_SENSOR_ID, SENSOR_DRVNAME_SP0A19_YUV,NULL),
#endif
#if defined(GC0313_MIPI_YUV)
    YUV_INFO(GC0313MIPI_YUV_SENSOR_ID, SENSOR_DRVNAME_GC0313MIPI_YUV, NULL), 
#endif
#if defined(GC0329_YUV)
    YUV_INFO(GC0329_SENSOR_ID, SENSOR_DRVNAME_GC0329_YUV, NULL), 
#endif
#if defined(GC0330_YUV)
    YUV_INFO(GC0330_SENSOR_ID, SENSOR_DRVNAME_GC0330_YUV, NULL), 
#endif
#if defined(OV2722_MIPI_RAW)
	RAW_INFO(OV2722MIPI_SENSOR_ID, SENSOR_DRVNAME_OV2722_MIPI_RAW,NULL), 
#endif

#if defined(S5K4H5YX_2LANE_MIPI_RAW)
    RAW_INFO(S5K4H5YX_2LANE_SENSOR_ID, SENSOR_DRVNAME_S5K4H5YX_2LANE_MIPI_RAW,NULL),
#endif
/*Added by EvanZeng @2013-11-20*/
#if defined(GC2235_RAW)
    RAW_INFO(GC2235_SENSOR_ID, SENSOR_DRVNAME_GC2235_RAW, NULL),
#endif
//modify by zhanghf 2014-01-21    
#if defined(GC2235_MIPI_RAW)
    RAW_INFO(GC2235_SENSOR_ID, SENSOR_DRVNAME_GC2235_RAW, NULL),
#endif

#if defined(GC2236_MIPI_RAW)
	RAW_INFO(GC2236_SENSOR_ID, SENSOR_DRVNAME_GC2236_MIPI_RAW, NULL), 
#endif

/*  ADD sensor driver before this line */
    {0,{0},NULL, NULL, NULL}//end of list
};

UINT32 GetSensorInitFuncList(MSDK_SENSOR_INIT_FUNCTION_STRUCT **ppSensorList)
{
    if (NULL == ppSensorList) {
        ALOGE("ERROR: NULL pSensorList\n");
        return MHAL_UNKNOWN_ERROR;
    }
    *ppSensorList = &SensorList[0];
	return MHAL_NO_ERROR;
} // GetSensorInitFuncList()






