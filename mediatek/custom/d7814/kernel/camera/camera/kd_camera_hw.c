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
#define PK_DBG_FUNC(fmt, arg...)    printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)

#define DEBUG_CAMERA_HW_K
#ifdef  DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_XLOG_INFO(fmt, args...) \
        do {    \
                    xlog_printk(ANDROID_LOG_INFO, "kd_camera_hw", fmt, ##args); \
        } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
u32 pinSetIdx = 0;//default main sensor

    #define IDX_PS_CMRST 0
    #define IDX_PS_CMPDN 4

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3
u32 pinSet[3][8] = {
                    //for main sensor 
                    {   GPIO_CAMERA_CMRST1_PIN,
                        GPIO_CAMERA_CMRST1_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                        GPIO_CAMERA_CMPDN_PIN,
                        GPIO_CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    },
                    //for sub sensor 
                    {   GPIO_CAMERA_CMRST1_PIN,
                        GPIO_CAMERA_CMRST1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                        GPIO_CAMERA_CMPDN1_PIN,
                        GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
                        GPIO_OUT_ZERO,
                        GPIO_OUT_ONE,
                    }
 
                  };

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }
    else if (DUAL_CAMERA_MAIN_SECOND_SENSOR == SensorIdx) {
        pinSetIdx = 2;
    }
 
    //power ON
    printk("shone enter here SensorIdx=%d currSensorName=%s on=%d\n",SensorIdx,currSensorName,On);
    if (On) {
        if(pinSetIdx == 0) {//main on

            if(mt_set_gpio_out(GPIO_CAMERA_CMPDN_PIN,1)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN_PIN down failed!! \n");}
            if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,0)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMRST1_PIN down failed!! \n");}

            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1800,mode_name))
			{
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                //return -EIO;
                 goto _kdCISModulePowerOn_exit_;
            }	
			mdelay(1);	
			
		 	if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_2800,mode_name))
        	{
            	PK_DBG("[CAMERA SENSOR] Fail to enable io digital power\n");
           		 //return -EIO;
           		 goto _kdCISModulePowerOn_exit_;
        	}	
			mdelay(1);	
                 
            
        	if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
      	    {
            	PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
          		 //return -EIO;
           		goto _kdCISModulePowerOn_exit_;
			}
            mdelay(1);  

           
           
          
            //disable the sub  
            if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,1)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN1_PIN down failed!! \n");}
            //main power on
            if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2235_RAW,currSensorName)))
            {//gc2035
                if(mt_set_gpio_out(GPIO_CAMERA_CMPDN_PIN,0)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN_PIN down failed!! \n");}
                if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,1)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMRST1_PIN up failed!! \n");}
            }
            
          }
        else if(pinSetIdx == 1){//sub on
        
            if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,1)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN1_PIN down failed!! \n");}
         
            if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
             }

	        mdelay(1);	
		 	if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_2800,mode_name))
        	{
            	PK_DBG("[CAMERA SENSOR] Fail to enable io digital power\n");
           		 //return -EIO;
           		 goto _kdCISModulePowerOn_exit_;
        	}	
			mdelay(1);	
             //diable main  
            if(mt_set_gpio_out(GPIO_CAMERA_CMPDN_PIN,0)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN_PIN down failed!! \n");}
            
            if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0329_YUV,currSensorName)))
            { //gc0329  
                if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,0)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN1_PIN up failed!! \n");}
                if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,1)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN_PIN down failed!! \n");}
            }
                
        }
    }
    else{

               
            if(mt_set_gpio_out(GPIO_CAMERA_CMPDN_PIN,1)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN_PIN down failed!! \n");}
            if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,1)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMPDN1_PIN down failed!! \n");}
            if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,0)){PK_DBG("[CAMERA LENS] set GPIO_CAMERA_CMRST1_PIN down failed!! \n");}
            
                        
            if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	            //return -EIO;
	            goto _kdCISModulePowerOn_exit_;
	        }     	
	        mdelay(1);
            
            if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2235_RAW,currSensorName)))
            {//gc2235
            if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
	            PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
	            //return -EIO;
	            goto _kdCISModulePowerOn_exit_;
	        }
            mdelay(1);
          		 //return -EIO;
            

            }
            
                //return -EIO;
            if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name)) 
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }        
            mdelay(1);
                //return -EIO;
            }        
            
                

            


	return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}


EXPORT_SYMBOL(kdCISModulePowerOn);

//!--
//
