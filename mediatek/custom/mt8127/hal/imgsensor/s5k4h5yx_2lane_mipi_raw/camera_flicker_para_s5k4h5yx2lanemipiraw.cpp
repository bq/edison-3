#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_info_s5k4h5yx2laneraw.h"
#include "camera_custom_AEPlinetable.h"

#include "camera_custom_flicker_para.h"
#include <cutils/xlog.h>

static void get_flicker_para_by_preview(FLICKER_CUST_PARA* para)
{
}

static void get_flicker_para_by_ZSD(FLICKER_CUST_PARA* para)
{
}


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;
namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetFlickerPara(MINT32 sensorMode, MVOID*const pDataBuf) const
{
	return 0;
}
}

