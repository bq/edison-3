//s_add new sensor driver here
//export funtions

UINT32 OV5693_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 OV5648MIPISensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 OV5647MIPISensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC2235MIPISensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC2235SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC0339SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC0329_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC0328_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);


//! Add Sensor Init function here
//! Note:
//! 1. Add by the resolution from ""large to small"", due to large sensor
//!    will be possible to be main sensor.
//!    This can avoid I2C error during searching sensor.
//! 2. This file should be the same as mediatek\custom\common\hal\imgsensor\src\sensorlist.cpp
ACDK_KD_SENSOR_INIT_FUNCTION_STRUCT kdSensorList[MAX_NUM_OF_SUPPORT_SENSOR+1] =
{

#if defined(OV5693_MIPI_RAW)
    {OV5693_SENSOR_ID, SENSOR_DRVNAME_OV5693_MIPI_RAW, OV5693_MIPI_RAW_SensorInit}, 
#endif

#if defined(OV5648_MIPI_RAW)
    {OV5648MIPI_SENSOR_ID, SENSOR_DRVNAME_OV5648_MIPI_RAW, OV5648MIPISensorInit}, 
#endif

#if defined(OV5647_MIPI_RAW)
    {OV5647MIPI_SENSOR_ID, SENSOR_DRVNAME_OV5647MIPI_RAW, OV5647MIPISensorInit}, 
#endif

#if defined(GC2235_MIPI_RAW)
		{GC2235_SENSOR_ID, SENSOR_DRVNAME_GC2235_MIPI_RAW, GC2235MIPISensorInit}, 
#endif

#if defined(GC2235_RAW)
    {GC2235_SENSOR_ID, SENSOR_DRVNAME_GC2235_RAW, GC2235SensorInit},
#endif

#if defined(GC0339_MIPI_RAW)
		{GC0339_SENSOR_ID, SENSOR_DRVNAME_GC0339_MIPI_RAW, GC0339SensorInit}, 
#endif

#if defined(GC0329_YUV)
    {GC0329_SENSOR_ID, SENSOR_DRVNAME_GC0329_YUV, GC0329_YUV_SensorInit},
#endif

#if defined(GC0328_YUV)
    {GC0328_SENSOR_ID, SENSOR_DRVNAME_GC0328_YUV, GC0328_YUV_SensorInit},
#endif

/*  ADD sensor driver before this line */
    {0,{0},NULL}, //end of list
};
//e_add new sensor driver here

