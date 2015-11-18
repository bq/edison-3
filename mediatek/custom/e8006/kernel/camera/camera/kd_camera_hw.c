#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         xlog_printk(ANDROID_LOG_ERR, PFX , fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

/*[A] define GPIO PIN xmcwy@20140702*/
#define GPIO_CAMERA_CMRST_PIN                   GPIO80
#define GPIO_CAMERA_CMPDN_PIN                   GPIO83
#define GPIO_CAMERA_CMRST1_PIN                 GPIO82
#define GPIO_CAMERA_CMPDN1_PIN                 GPIO81
/*[E] define GPIO PIN xmcwy@20140702*/

int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
u32 pinSetIdx = 0;//default main sensor

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 1

u32 pinSet[2][2] = {
                    //for main sensor
                    {GPIO_CAMERA_CMRST_PIN,
                     GPIO_CAMERA_CMPDN_PIN,
                    },
                    //for sub sensor
                    {GPIO_CAMERA_CMRST1_PIN,
                     GPIO_CAMERA_CMPDN1_PIN,
                    }
                   };


    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }

    PK_DBG("kdCISModulePowerOn SensorIdx = %d  currSensorName = %s  On = %d mode_name = %s \n", \
        SensorIdx, currSensorName, On, mode_name);

    //power ON
    if (On) {

#if defined(GC2235_MIPI_RAW)        
        if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2235_MIPI_RAW,currSensorName)))
        {

            PK_DBG("xmcwy_camera_power gc2235_mipi_raw  on  SensorIdx = %d \n",SensorIdx);
			
            //reset active sensor
            //RST pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);
            //PDN pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);

            //DVDD
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1800,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_D \n");
                 goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);
		
            //IOVDD
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_D2 \n");
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);
			
            //AVDD
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_A \n");
                goto _kdCISModulePowerOn_exit_;
            }
	     mdelay(10);
		 
            #if 0
            //AF_VCC
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_A2 \n");
                goto _kdCISModulePowerOn_exit_;
            }
            #endif

            //enable active sensor
            //PDN pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);            
            //RST pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);
			
            //disable inactive sensor
            //PDN pin
	     if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	     if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}	
	     if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	     mdelay(10); 
            //RST pin
            if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);

        }
#endif

#if defined(GC0339_MIPI_RAW) 
        if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0339_MIPI_RAW,currSensorName)))
        {

            PK_DBG("xmcwy_camera_power gc0339_mipi_raw  on  SensorIdx = %d \n",SensorIdx);
			
            //reset active sensor
            //RST pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);
            //PDN pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);

			
            //IOVDD
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_D2 \n");
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);

            //AVDD
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_A \n");
                goto _kdCISModulePowerOn_exit_;
            }
	     mdelay(10);

            //DVDD
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1800,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_D \n");
                 goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);
		 
            #if 0
            //AF_VCC
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable CAMERA_POWER_VCAM_A2 \n");
                goto _kdCISModulePowerOn_exit_;
            }
            #endif

            //enable active sensor
            //PDN pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);            
            //RST pin
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);
			
            //disable inactive sensor
            //PDN pin
	     if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	     if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}	
	     if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	     mdelay(10); 
            //RST pin
            if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio out failed!! \n");}
            mdelay(10);

        }
#endif

    }
    else {//power OFF

#if defined(GC2235_MIPI_RAW)
        if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2235_MIPI_RAW,currSensorName))){

	 PK_DBG("xmcwy_camera_power gc2235_mipi_raw off SensorIdx = %d \n",SensorIdx);

        //disable inactive sensor
        //PDN pin
	 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}	
	 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	 mdelay(10); 

        //RST pin	 
	 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	 mdelay(10); 

	 //AVDD
    	 if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to off AVDD \n");
            goto _kdCISModulePowerOn_exit_;
        }

	 //IOVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2, mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to off IOVDD \n");
            goto _kdCISModulePowerOn_exit_;
        }

	 //DVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to off DVDD \n");
            goto _kdCISModulePowerOn_exit_;
        }

        }	
#endif

#if defined(GC0339_MIPI_RAW)
        if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0339_MIPI_RAW,currSensorName))){

	 PK_DBG("xmcwy_camera_power gc0339_mipi_raw off SensorIdx = %d \n",SensorIdx);

        //disable inactive sensor
        //PDN pin
	 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}	
	 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_OUT_ONE)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	 mdelay(10); 

        //RST pin	 
	 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_MODE_00)){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_OUT_ZERO)){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	 mdelay(10);

	 //IOVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2, mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to off IOVDD \n");
            goto _kdCISModulePowerOn_exit_;
        }

	 //AVDD
    	 if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to off AVDD \n");
            goto _kdCISModulePowerOn_exit_;
        }

	 //DVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to off DVDD \n");
            goto _kdCISModulePowerOn_exit_;
        }

        }	
#endif

    }

	return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);

