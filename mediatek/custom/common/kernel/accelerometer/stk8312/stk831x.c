/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/* drivers/i2c/chips/stk831x.c - stk831x motion sensor driver
 *
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define SENSOR_STK8312 
//#define SENSOR_STK8313

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/version.h>
#include <linux/module.h>

#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/fs.h> 
#include <linux/fcntl.h>
#include <linux/syscalls.h>

#include "stk8312.h"
#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <linux/hwmsen_helper.h>
#ifdef MT6516
#include <mach/mt6516_devs.h>
#include <mach/mt6516_typedefs.h>
#include <mach/mt6516_gpio.h>
#include <mach/mt6516_pll.h>
#endif

#ifdef MT6573
#include <mach/mt6573_devs.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_pll.h>
#endif
#if 0
#include <mach/mt6575_devs.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_gpio.h>
#include <mach/mt6575_pm_ldo.h>
#endif
#ifdef MT6577
#include <mach/mt6577_devs.h>
#include <mach/mt6577_typedefs.h>
#include <mach/mt6577_gpio.h>
#include <mach/mt6577_pm_ldo.h>
#endif
#if (defined(MT6589) || defined(MT6572) || defined(MT6575)|| defined(MT6582))
//#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

#ifdef MT6516
#define POWER_NONE_MACRO MT6516_POWER_NONE
#endif

#ifdef MT6573
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif
#ifdef MT6575
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif
#ifdef MT6577
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif
#if (defined(MT6589) || defined(MT6572)|| defined(MT6582))
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif
/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_STK8313 345
/*----------------------------------------------------------------------------*/
#define DEBUG 1
//#define STK_DEBUG_PRINT
/*----------------------------------------------------------------------------*/
#define STK831X_HOLD_ODR
#define STK831X_INIT_ODR 3
#define STK_PERMISSION_THREAD
#define CONFIG_STK831X_LOWPASS   /*apply low pass filter on output*/       
//#define STK_ZG_FILTER
#ifdef SENSOR_STK8312
	#define STK_ZG_COUNT	1
#elif defined (SENSOR_STK8313)
	#define STK_ZG_COUNT	4
#endif

#define STK_TUNE
#ifdef SENSOR_STK8312
	#define STK_TUNE_XYOFFSET 4
	#define STK_TUNE_ZOFFSET 6
	#define STK_TUNE_NOISE 5
	#define STK_LSB_1G		21
#elif defined (SENSOR_STK8313)
	#define STK_TUNE_XYOFFSET 35
	#define STK_TUNE_ZOFFSET 75
	#define STK_TUNE_NOISE 20	
	#define STK_LSB_1G		256
#endif
//#define STK_TUNE_NUM 125
//#define STK_TUNE_DELAY 125
#define STK_TUNE_NUM 60
#define STK_TUNE_DELAY 60
/*----------------------------------------------------------------------------*/
#define STK831X_DRIVER_VERSION	"0.9"
#define STK831X_AXIS_X          0
#define STK831X_AXIS_Y          1
#define STK831X_AXIS_Z          2
#define STK831X_AXES_NUM        3
#ifdef SENSOR_STK8312
	#define STK831X_DATA_LEN        3
#elif defined SENSOR_STK8313
	#define STK831X_DATA_LEN        6
#endif
#define STK831X_DEV_NAME        "stk831x"
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id stk831x_i2c_id[] = {{STK831X_DEV_NAME,0},{}};
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,0,0))
	#ifdef SENSOR_STK8312	
		static struct i2c_board_info __initdata i2c_stk831x={ I2C_BOARD_INFO(STK831X_DEV_NAME, 0x3D)};
	#elif defined SENSOR_STK8313
		static struct i2c_board_info __initdata i2c_stk831x={ I2C_BOARD_INFO(STK831X_DEV_NAME, 0x22)};
	#endif	/* #ifdef SENSOR_STK8312	 */	
#else
	#ifdef SENSOR_STK8312
		static unsigned short stk8312_force[] = {0x00, STK8312_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
		static const unsigned short *const stk8312_forces[] = { stk8312_force, NULL };
		static struct i2c_client_address_data stk8312_addr_data = { .forces = stk8312_forces,};
	#elif defined SENSOR_STK8313
		static unsigned short stk8313_force[] = {0x00, STK8313_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
		static const unsigned short *const stk8313_forces[] = { stk8313_force, NULL };
		static struct i2c_client_address_data stk8313_addr_data = { .forces = stk8313_forces,};
	#endif	/* 	#ifdef SENSOR_STK8312 */
#endif	/* #if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,0,0))	 */
/*----------------------------------------------------------------------------*/
static int stk831x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int stk831x_i2c_remove(struct i2c_client *client);
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))	
static int stk831x_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
#endif
#if !defined(CONFIG_HAS_EARLYSUSPEND)   

static int stk831x_suspend(struct i2c_client *client, pm_message_t msg) ;
static int stk831x_resume(struct i2c_client *client);
#endif

static int STK831X_ReadData(struct i2c_client *client, s16 data[]);
static int STK831X_ReadSensorData(struct i2c_client *client, char *buf, int bufsize);
static int STK831X_Init(struct i2c_client *client, int reset_cali);
static int STK831X_SetDelay(struct i2c_client *client, u8 delay);

static int STK831X_SetPowerModeToWrite(struct i2c_client *client, bool enable);
static int STK831X_SetPowerMode(struct i2c_client *client, bool enable);
static int STK831X_SetVD(struct i2c_client *client);
static int stk_store_in_file(char offset[], char mode);
static int stk_store_in_ic( struct i2c_client *client, char otp_offset[], char FT_index, uint32_t delay_ms);
static int STK831x_ReadByteOTP(char rReg, char *value);
static int STK831x_WriteByteOTP(char wReg, char value);
static int STK831x_WriteOffsetOTP(struct i2c_client *client, int FT, char offsetData[]);
static int STK831X_VerifyCali(struct i2c_client *client, unsigned char en_dis, uint32_t delay_ms);
static int STK831x_GetDelay(struct i2c_client *client, uint32_t* gdelay_ns);


/*------------------------------------------------------------------------------*/
typedef enum {
    ADX_TRC_FILTER  = 0x01,
    ADX_TRC_RAWDATA = 0x02,
    ADX_TRC_IOCTL   = 0x04,
    ADX_TRC_CALI	= 0X08,
    ADX_TRC_INFO	= 0X10,
    ADX_TRC_REGXYZ	= 0X20,
} ADX_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};
/*----------------------------------------------------------------------------*/
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
    //float                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][STK831X_AXES_NUM];
    int sum[STK831X_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct stk831x_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;

    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[STK831X_AXES_NUM+1];

    /*data*/
    s8                      offset[STK831X_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[STK831X_AXES_NUM+1];

#if defined(CONFIG_STK831X_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif 
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
	atomic_t				event_since_en;
	atomic_t				event_since_en_limit;
	atomic_t				recv_reg;
};
/*----------------------------------------------------------------------------*/
#define STK_DEBUG_CALI
#define STK_SAMPLE_NO				10
#define STK_ACC_CALI_VER0			0x3D
#define STK_ACC_CALI_VER1			0x01
#define STK_ACC_CALI_FILE 			"/data/misc/stkacccali.conf"
#define STK_ACC_CALI_FILE_SIZE 		10

#define STK_K_SUCCESS_TUNE			0x04
#define STK_K_SUCCESS_FT2			0x03
#define STK_K_SUCCESS_FT1			0x02
#define STK_K_SUCCESS_FILE			0x01
#define STK_K_NO_CALI				0xFF
#define STK_K_RUNNING				0xFE
#define STK_K_FAIL_LRG_DIFF			0xFD
#define STK_K_FAIL_OPEN_FILE			0xFC
#define STK_K_FAIL_W_FILE				0xFB
#define STK_K_FAIL_R_BACK				0xFA
#define STK_K_FAIL_R_BACK_COMP		0xF9
#define STK_K_FAIL_I2C				0xF8
#define STK_K_FAIL_K_PARA				0xF7
#define STK_K_FAIL_OTP_OUT_RG		0xF6
#define STK_K_FAIL_ENG_I2C			0xF5
#define STK_K_FAIL_FT1_USD			0xF4
#define STK_K_FAIL_FT2_USD			0xF3
#define STK_K_FAIL_WRITE_NOFST		0xF2
#define STK_K_FAIL_OTP_5T				0xF1
#define STK_K_FAIL_PLACEMENT			0xF0


#define POSITIVE_Z_UP		0
#define NEGATIVE_Z_UP	1
#define POSITIVE_X_UP		2
#define NEGATIVE_X_UP	3
#define POSITIVE_Y_UP		4
#define NEGATIVE_Y_UP	5
static unsigned char stk831x_placement = POSITIVE_Z_UP;
#ifdef STK_TUNE
static char stk_tune_offset_record[3] = {0};
static int stk_tune_offset[3] = {0};
static int stk_tune_sum[3] = {0};
static int stk_tune_max[3] = {0};
static int stk_tune_min[3] = {0};
static int stk_tune_index = 0;
static int stk_tune_done = 0;
#endif
static bool first_enable;
static atomic_t cali_status;
/*----------------------------------------------------------------------------*/


static struct i2c_driver stk831x_i2c_driver = {
    .driver = {
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))			
        .owner          = THIS_MODULE,
#endif		
        .name           = STK831X_DEV_NAME,
    },
	.probe      		= stk831x_i2c_probe,
	.remove    			= stk831x_i2c_remove,
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))		
	.detect				= stk831x_i2c_detect,
#endif	
#if !defined(CONFIG_HAS_EARLYSUSPEND)    
    .suspend            = stk831x_suspend,
    .resume             = stk831x_resume,
#endif
	.id_table = stk831x_i2c_id,
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))		
	#ifdef SENSOR_STK8312
		.address_data = &stk8312_addr_data,
	#elif defined SENSOR_STK8313
		.address_data = &stk8313_addr_data,
	#endif
#endif	/* #if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0)) */
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *stk831x_i2c_client = NULL;
static struct platform_driver stk831x_gsensor_driver;
static struct stk831x_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = false;
static GSENSOR_VECTOR3D gsensor_gain, gsensor_offset;
//static char selftestRes[10] = {0};



/*----------------------------------------------------------------------------*/
#if 1
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk(KERN_INFO GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_INFO GSE_TAG fmt, ##args)
#else
#define GSE_TAG
#define GSE_FUN(f)               do {} while (0)
#define GSE_ERR(fmt, args...)    do {} while (0)
#define GSE_LOG(fmt, args...)    do {} while (0)
#endif
/*----------------------------------------------------------------------------*/
#ifdef SENSOR_STK8312
static struct data_resolution stk831x_data_resolution[] = {
	/*3 combination by {FULL_RES,RANGE}*/
	{{ 3, 9}, 21.33},   /*+/-1.5g  in 6-bit resolution:  46.88 mg/LSB*/    
	{{ 3, 9}, 21.33},   /*+/-6g  in 8-bit resolution:  46.88 mg/LSB*/
	{{3, 9},  8},   /*+/-16g  in 8-bit resolution: 125 mg/LSB*/
};
#elif defined SENSOR_STK8313
static struct data_resolution stk831x_data_resolution[] = {
 /*4 combination by {FULL_RES,RANGE}*/
    {{ 3, 9}, 256},   /*+/-2g  in 10-bit resolution:  3.9 mg/LSB*/    
    {{ 3, 9}, 256},   /*+/-4g  in 11-bit resolution:  3.9 mg/LSB*/
    {{3, 9},  256},   /*+/-8g  in 12-bit resolution: 3.9 mg/LSB*/
    {{ 7, 8}, 128},   /*+/-16g  in 12-bit resolution:  7.8 mg/LSB (full-resolution)*/       
};
#endif

/*----------------------------------------------------------------------------*/
#ifdef SENSOR_STK8312
static struct data_resolution stk831x_offset_resolution = {{3, 9}, 21.33};
#elif defined SENSOR_STK8313
static struct data_resolution stk831x_offset_resolution = {{3, 9}, 256};
#endif


int stk831x_hwmsen_read_block(struct i2c_client *client, u8 addr, u8 *data, u8 len)
{
	u8 beg = addr; 
	struct i2c_msg msgs[2] = 
	{
		{
			.addr = client->addr,	 
			.flags = 0,
			.len = 1,				 
			.buf= &beg
		},
		{
			.addr = client->addr,	 
			.flags = I2C_M_RD,
			.len = len, 			 
			.buf = data,
		}
	};
	int err;

	if (!client)
		return -EINVAL;
	else if (len > C_I2C_FIFO_SIZE) 
	{		 
		GSE_LOG(" length %d exceeds %d\n", len, C_I2C_FIFO_SIZE);
		return -EINVAL;
	}

	err = i2c_transfer(client->adapter, msgs, sizeof(msgs)/sizeof(msgs[0]));
	if (err != 2) 
	{
		GSE_LOG("i2c_transfer error: (%d %p %d) %d\n", addr, data, len, err);
		err = -EIO;
	}
	else 
	{
		err = 0;/*no error*/
	}
	return err;
}


int hwmsen_read_byte_sr(struct i2c_client *client, u8 addr, u8 *data)
{	
	return stk831x_hwmsen_read_block(client, addr, data, 1);			
#if 0
/*
   u8 buf;
    int ret = 0;
	
    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
    buf = addr;
	ret = i2c_master_send(client, (const char*)&buf, 1<<8 | 1);
    //ret = i2c_master_send(client, (const char*)&buf, 1);
    if (ret < 0) {
        HWM_ERR("send command error!!\n");
        return -EFAULT;
    }

    *data = buf;
	client->addr = client->addr& I2C_MASK_FLAG;
    return 0;
//#else
    return hwmsen_read_byte(client, addr, data);
*/
#endif
}

static int STK_i2c_Rx(char *rxData, int length)
{
	struct i2c_client *client = stk831x_i2c_client; 
	u8 addr = rxData[0];
	
	return stk831x_hwmsen_read_block(client, addr, rxData, length);
}

static int STK_i2c_Tx(char *txData, int length)
{
	struct i2c_client *client = stk831x_i2c_client; 
	
	return i2c_master_send(client, txData, length);	
}

/*
void dumpReg(struct i2c_client *client)
{
  int i=0;
  u8 addr = 0x00;
  u8 regdata=0;
  for(i=0; i<49 ; i++)
  {
    //dump all
    hwmsen_read_byte_sr(client,addr,&regdata);
	HWM_LOG("Reg addr=%x regdata=%x\n",addr,regdata);
	addr++;
	if(addr ==01)
		addr=addr+0x06;
	if(addr==0x09)
		addr++;
	if(addr==0x0A)
		addr++;
  }
  
 
  // for(i=0; i<5 ; i++)
  // {
   // dump ctrol_reg1~control_reg5
    // hwmsen_read_byte_sr(client,addr,regdata);
	// HWM_LOG("Reg addr=%x regdata=%x\n",addr,regdata);
	// addr++;
  // }
  
  // addr = STK831X_REG_OFSX;
  // for(i=0; i<5 ; i++)
  // {
  //  dump offset
    // hwmsen_read_byte_sr(client,addr,regdata);
	// HWM_LOG("Reg addr=%x regdata=%x\n",addr,regdata);
	// addr++
  // }

}

int hwmsen_read_block_sr(struct i2c_client *client, u8 addr, u8 *data)
{
	u8 buf[10];
    int ret = 0;
	memset(buf, 0, sizeof(u8)*10); 
	
    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
    buf[0] = addr;
	ret = i2c_master_send(client, (const char*)&buf, 6<<8 | 1);
    //ret = i2c_master_send(client, (const char*)&buf, 1);
    if (ret < 0) {
        HWM_ERR("send command error!!\n");
        return -EFAULT;
    }

    *data = buf;
	client->addr = client->addr& I2C_MASK_FLAG;
    return 0;
}
*/

static void STK831X_power(struct acc_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{        
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "STK831X"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "STK831X"))
			{
				GSE_ERR("power off fail!!\n");
			}			  
		}
	}
	power_on = on;    
}
/*----------------------------------------------------------------------------*/
#ifdef STK_TUNE
static int STK831x_SetOffset(char buf[])
{
	int result;
	char buffer[4] = "";
	
	buffer[0] = STK831X_REG_OFSX;	
	buffer[1] = buf[0];
	buffer[2] = buf[1];
	buffer[3] = buf[2];
	result = STK_i2c_Tx(buffer, 4);
	if (result < 0) 
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}	
	return 0;
}

static int STK831x_GetOffset(char buf[])
{
	int result;
	char buffer[3] = "";
	
	buffer[0] = STK831X_REG_OFSX;
	result = STK_i2c_Rx(buffer, 3);	
	if (result < 0) 
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}		
	buf[0] = buffer[0];
	buf[1] = buffer[1];
	buf[2] = buffer[2];
	return 0;
}

static int stk_get_ic_content(void)
{
	int result;
	char regR;
		
	result = STK831x_ReadByteOTP(0x7F, &regR);
	if(result < 0)
	{
		printk(KERN_ERR "%s: read/write eng i2c error, result=0x%x\n", __func__, result);	
		return result;
	}
	
	if(regR&0x20)
	{
		atomic_set(&cali_status, STK_K_SUCCESS_FT2);	
		printk(KERN_INFO "%s: OTP 2 used\n", __func__);
		return 2;	
	}
	if(regR&0x10)	
	{
		atomic_set(&cali_status, STK_K_SUCCESS_FT1);	
		printk(KERN_INFO "%s: OTP 1 used\n", __func__);		
		return 1;	
	}
	return 0;
}

static int stk_store_in_ic( struct i2c_client *client, char otp_offset[], char FT_index, uint32_t delay_ms)
{
	int result;
	char buffer[2] = "";

	buffer[0] = STK831X_REG_MODE;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto ic_err_i2c_rw;
	}			
	buffer[1] = (buffer[0] & 0xF8) | 0x01;
	buffer[0] = STK831X_REG_MODE;	
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto ic_err_i2c_rw;
	}		
	STK831X_SetVD(client);
	
	buffer[0] = 0x2B;	
	buffer[1] = otp_offset[0];
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto ic_err_i2c_rw;
	}
	buffer[0] = 0x2F;	
	buffer[1] = otp_offset[2];
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto ic_err_i2c_rw;
	}
	buffer[0] = 0x33;	
	buffer[1] = otp_offset[1];
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto ic_err_i2c_rw;
	}		
	

#ifdef STK_DEBUG_CALI	
	//printk(KERN_INFO "%s:Check All OTP Data after write 0x2B 0x2F 0x33\n", __func__);
	//STK831x_ReadAllOTP();
#endif	
	
	msleep(delay_ms*15);		
	result = STK831X_VerifyCali(client, 0, 0);
	if(result)
	{
		printk(KERN_ERR "%s: calibration check1 fail, FT_index=%d\n", __func__, FT_index);				
		goto ic_err_misc;
	}
#ifdef STK_DEBUG_CALI		
	//printk(KERN_INFO "\n%s:Check All OTP Data before write OTP\n", __func__);
	//STK831x_ReadAllOTP();

#endif	
	//Write OTP	
	printk(KERN_INFO "\n%s:Write offset data to FT%d OTP\n", __func__, FT_index);
	result = STK831x_WriteOffsetOTP(client, FT_index, otp_offset);
	if(result < 0)
	{
		printk(KERN_INFO "%s: write OTP%d fail\n", __func__, FT_index);
		goto ic_err_misc;
	}
	
	buffer[0] = STK831X_REG_MODE;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto ic_err_i2c_rw;
	}			
	buffer[1] = (buffer[0] & 0xF8);
	buffer[0] = STK831X_REG_MODE;	
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto ic_err_i2c_rw;
	}	
	
	msleep(1);
	result = STK831X_Init(client, 0);

#ifdef STK_DEBUG_CALI		
	//printk(KERN_INFO "\n%s:Check All OTP Data after write OTP and reset\n", __func__);
	//STK831x_ReadAllOTP();
#endif
		
	result = STK831X_VerifyCali(client, 1, delay_ms);
	if(result)
	{
		printk(KERN_ERR "%s: calibration check2 fail\n", __func__);
		goto ic_err_misc;
	}
	return 0;

ic_err_misc:
	result = STK831X_Init(client, 0);	
	msleep(1);
	atomic_set(&cali_status, -result);	
	return result;
	
ic_err_i2c_rw:	
	printk(KERN_ERR "%s: i2c read/write error, err=0x%x\n", __func__, result);
	msleep(1);
	result = STK831X_Init(client, 0);	
	atomic_set(&cali_status, STK_K_FAIL_I2C);	
	return result;	
}

static int32_t stk_get_file_content(char * r_buf, int8_t buf_size)
{
	struct file  *cali_file;
	mm_segment_t fs;	
	ssize_t ret;
	
    cali_file = filp_open(STK_ACC_CALI_FILE, O_RDONLY,0);
    if(IS_ERR(cali_file))
	{
        printk("%s: filp_open error, no offset file!\n", __func__);
        return -ENOENT;
	}
	else
	{
		fs = get_fs();
		set_fs(get_ds());
		ret = cali_file->f_op->read(cali_file,r_buf, STK_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret < 0)
		{
			printk("%s: read error, ret=%d\n", __func__, ret);
			filp_close(cali_file,NULL);
			return -EIO;
		}		
		set_fs(fs);
    }
	
    filp_close(cali_file,NULL);	
	return 0;	
}

static void stk_handle_first_en(struct i2c_client *client)
{
	char r_buf[STK_ACC_CALI_FILE_SIZE] = {0};
	char offset[3];	
	char mode;

	if(stk_tune_offset_record[0]!=0 || stk_tune_offset_record[1]!=0 || stk_tune_offset_record[2]!=0)
	{
		STK831x_SetOffset(stk_tune_offset_record);
		stk_tune_done = 1;				
		atomic_set(&cali_status, STK_K_SUCCESS_TUNE);	
		printk("%s: set offset:%d,%d,%d\n", __func__, stk_tune_offset_record[0], stk_tune_offset_record[1],stk_tune_offset_record[2]);	
	}		
	else if ((stk_get_file_content(r_buf, STK_ACC_CALI_FILE_SIZE)) == 0)
	{
		if(r_buf[0] == STK_ACC_CALI_VER0 && r_buf[1] == STK_ACC_CALI_VER1)
		{
			offset[0] = r_buf[2];
			offset[1] = r_buf[3];
			offset[2] = r_buf[4];
			mode = r_buf[5];
			STK831x_SetOffset(offset);
//#ifdef STK_TUNE			
			stk_tune_offset_record[0] = offset[0];
			stk_tune_offset_record[1] = offset[1];
			stk_tune_offset_record[2] = offset[2];
//#endif 			
			printk("%s: set offset:%d,%d,%d, mode=%d\n", __func__, offset[0], offset[1], offset[2], mode);
			atomic_set(&cali_status, mode);								
		}
		else
		{
			printk("%s: cali version number error! r_buf=0x%x,0x%x,0x%x,0x%x,0x%x\n", 
				__func__, r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4]);						
			//return -EINVAL;
		}
	}
	else
	{
		offset[0] = offset[1] = offset[2] = 0;
		stk_store_in_file(offset, STK_K_NO_CALI);
		atomic_set(&cali_status, STK_K_NO_CALI);			
	}
	return;
}

static void STK831x_ResetPara(void)
{
	int ii;
	for(ii=0;ii<3;ii++)
	{
		stk_tune_sum[ii] = 0;
		stk_tune_min[ii] = 4096;
		stk_tune_max[ii] = -4096;
	}
	return;
}

static void STK831x_Tune(struct i2c_client *client, int acc[])
{	
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int ii;
	char offset[3];		
	char mode_reg;
	int result;
	char buffer[2] = "";
	
	if (stk_tune_done==0)
	{	
		if( atomic_read(&obj->event_since_en) >= STK_TUNE_DELAY)
		{	
			if ((abs(acc[0]) <= STK_TUNE_XYOFFSET) && (abs(acc[1]) <= STK_TUNE_XYOFFSET)
				&& (abs(abs(acc[2])-STK_LSB_1G) <= STK_TUNE_ZOFFSET))				
				stk_tune_index++;
			else
				stk_tune_index = 0;

			if (stk_tune_index==0)			
				STK831x_ResetPara();			
			else
			{
				for(ii=0;ii<3;ii++)
				{
					stk_tune_sum[ii] += acc[ii];
					if(acc[ii] > stk_tune_max[ii])
						stk_tune_max[ii] = acc[ii];
					if(acc[ii] < stk_tune_min[ii])
						stk_tune_min[ii] = acc[ii];						
				}	
			}			

			if(stk_tune_index == STK_TUNE_NUM)
			{
				for(ii=0;ii<3;ii++)
				{
					if((stk_tune_max[ii] - stk_tune_min[ii]) > STK_TUNE_NOISE)
					{
						stk_tune_index = 0;
						STK831x_ResetPara();
						return;
					}
				}
				buffer[0] = STK831X_REG_MODE;
				result = STK_i2c_Rx(buffer, 1);	
				if (result < 0) 
				{
					printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
					return;
				}
				mode_reg = buffer[0];
				buffer[1] = mode_reg & 0xF8;
				buffer[0] = STK831X_REG_MODE;	
				result = STK_i2c_Tx(buffer, 2);
				if (result < 0) 
				{
					printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);			
					return;
				}				
				
				stk_tune_offset[0] = stk_tune_sum[0]/STK_TUNE_NUM;
				stk_tune_offset[1] = stk_tune_sum[1]/STK_TUNE_NUM;
				if (acc[2] > 0)
					stk_tune_offset[2] = stk_tune_sum[2]/STK_TUNE_NUM - STK_LSB_1G;
				else
					stk_tune_offset[2] = stk_tune_sum[2]/STK_TUNE_NUM - (-STK_LSB_1G);				
				
				offset[0] = (char) (-stk_tune_offset[0]);
				offset[1] = (char) (-stk_tune_offset[1]);
				offset[2] = (char) (-stk_tune_offset[2]);
				STK831x_SetOffset(offset);
				stk_tune_offset_record[0] = offset[0];
				stk_tune_offset_record[1] = offset[1];
				stk_tune_offset_record[2] = offset[2];
				
				buffer[1] = mode_reg | 0x1;
				buffer[0] = STK831X_REG_MODE;	
				result = STK_i2c_Tx(buffer, 2);
				if (result < 0) 
				{
					printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);			
					return;
				}
				
				STK831X_SetVD(client);			
				stk_store_in_file(offset, STK_K_SUCCESS_TUNE);		
				stk_tune_done = 1;				
				atomic_set(&cali_status, STK_K_SUCCESS_TUNE);				
				atomic_set(&obj->event_since_en, 0);			
				printk(KERN_INFO "%s:TUNE done, %d,%d,%d\n", __func__, 
					offset[0], offset[1],offset[2]);		
			}	
		}		
	}

	return;
}
#endif
/*----------------------------------------------------------------------------*/
static int STK831x_ReadByteOTP(char rReg, char *value)
{
	int redo = 0;
	int result;
	char buffer[2] = "";
	*value = 0;
	
	buffer[0] = 0x3D;
	buffer[1] = rReg;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto eng_i2c_r_err;
	}
	buffer[0] = 0x3F;
	buffer[1] = 0x02;
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto eng_i2c_r_err;
	}
	
	do {
		msleep(2);
		buffer[0] = 0x3F;
		result = STK_i2c_Rx(buffer, 1);	
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed\n", __func__);
			goto eng_i2c_r_err;
		}
		if(buffer[0]& 0x80)
		{
			break;
		}		
		redo++;
	}while(redo < 10);
	
	if(redo == 10)
	{
		printk(KERN_ERR "%s:OTP read repeat read 10 times! Failed!\n", __func__);
		return -STK_K_FAIL_OTP_5T;
	}	
	buffer[0] = 0x3E;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		printk(KERN_ERR "%s:failed\n", __func__);
		goto eng_i2c_r_err;
	}	
	*value = buffer[0];
#ifdef STK_DEBUG_CALI		
	printk(KERN_INFO "%s: read 0x%x=0x%x\n", __func__, rReg, *value);
#endif	
	return 0;
	
eng_i2c_r_err:	
	return -STK_K_FAIL_ENG_I2C;	
}

static int STK831x_WriteByteOTP(char wReg, char value)
{
	int finish_w_check = 0;
	int result;
	char buffer[2] = "";
	char read_back, value_xor = value;
	int re_write = 0;
	
	do
	{
		finish_w_check = 0;
		buffer[0] = 0x3D;
		buffer[1] = wReg;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);
			goto eng_i2c_w_err;
		}
		buffer[0] = 0x3E;
		buffer[1] = value_xor;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);
			goto eng_i2c_w_err;
		}				
		buffer[0] = 0x3F;
		buffer[1] = 0x01;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);			
			goto eng_i2c_w_err;
		}				
		
		do 
		{
			msleep(1);
			buffer[0] = 0x3F;
			result = STK_i2c_Rx(buffer, 1);	
			if (result < 0) 
			{
				printk(KERN_ERR "%s:failed, err=0x%x\n", __func__, result);			
				goto eng_i2c_w_err;
			}
			if(buffer[0]& 0x80)
			{
				result = STK831x_ReadByteOTP(wReg, &read_back);
				if(result < 0)
				{
					printk(KERN_ERR "%s: read back error, result=%d\n", __func__, result);
					goto eng_i2c_w_err;
				}
				
				if(read_back == value)				
				{
#ifdef STK_DEBUG_CALI					
					printk(KERN_INFO "%s: write 0x%x=0x%x successfully\n", __func__, wReg, value);
#endif			
					re_write = 0xFF;
					break;
				}
				else
				{
					printk(KERN_ERR "%s: write 0x%x=0x%x, read 0x%x=0x%x, try again\n", __func__, wReg, value_xor, wReg, read_back);
					value_xor = read_back ^ value;
					re_write++;
					break;
				}
			}
			finish_w_check++;		
		} while (finish_w_check < 5);
	} while(re_write < 10);
	
	if(re_write == 10)
	{
		printk(KERN_ERR "%s: write 0x%x fail, read=0x%x, write=0x%x, target=0x%x\n", __func__, wReg, read_back, value_xor, value);
		return -STK_K_FAIL_OTP_5T;
	}	
	
	return 0;

eng_i2c_w_err:	
	return -STK_K_FAIL_ENG_I2C;
}

static int STK831x_WriteOffsetOTP(struct i2c_client *client, int FT, char offsetData[])
{
	char regR[6], reg_comp[3];
	char mode; 
	int result;
	char buffer[2] = "";
	int ft_pre_trim = 0;
	
	if(FT==1)
	{
		result = STK831x_ReadByteOTP(0x7F, &regR[0]);
		if(result < 0)
			goto eng_i2c_err;
		
		if(regR[0]&0x10)
		{
			printk(KERN_ERR "%s: 0x7F=0x%x\n", __func__, regR[0]);
			return -STK_K_FAIL_FT1_USD;
		}
	}
	else if (FT == 2)
	{
		result = STK831x_ReadByteOTP(0x7F, &regR[0]);
		if(result < 0)
			goto eng_i2c_err;
			
		if(regR[0]&0x20)
		{
			printk(KERN_ERR "%s: 0x7F=0x%x\n", __func__, regR[0]);
			return -STK_K_FAIL_FT2_USD;
		}		
	}
	
	buffer[0] = STK831X_REG_MODE;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	mode = buffer[0];
	buffer[1] = (mode | 0x01);
	buffer[0] = STK831X_REG_MODE;	
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	msleep(2);


	if(FT == 1)
	{
		result = STK831x_ReadByteOTP(0x40, &reg_comp[0]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831x_ReadByteOTP(0x41, &reg_comp[1]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831x_ReadByteOTP(0x42, &reg_comp[2]);
		if(result < 0)
			goto eng_i2c_err;	
	}
	else if (FT == 2)
	{
		result = STK831x_ReadByteOTP(0x50, &reg_comp[0]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831x_ReadByteOTP(0x51, &reg_comp[1]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831x_ReadByteOTP(0x52, &reg_comp[2]);
		if(result < 0)
			goto eng_i2c_err;					
	}

	result = STK831x_ReadByteOTP(0x30, &regR[0]);
	if(result < 0)
		goto eng_i2c_err;
	result = STK831x_ReadByteOTP(0x31, &regR[1]);
	if(result < 0)
		goto eng_i2c_err;
	result = STK831x_ReadByteOTP(0x32, &regR[2]);
	if(result < 0)
		goto eng_i2c_err;
		
	if(reg_comp[0] == regR[0] && reg_comp[1] == regR[1] && reg_comp[2] == regR[2])
	{
		printk(KERN_INFO "%s: ft pre-trimmed\n", __func__);
		ft_pre_trim = 1;
	}
	
	if(!ft_pre_trim)
	{
		if(FT == 1)
		{		
			result = STK831x_WriteByteOTP(0x40, regR[0]);
			if(result < 0)
				goto eng_i2c_err;
			result = STK831x_WriteByteOTP(0x41, regR[1]);
			if(result < 0)
				goto eng_i2c_err;		
			result = STK831x_WriteByteOTP(0x42, regR[2]);
			if(result < 0)
				goto eng_i2c_err;		
		}
		else if (FT == 2)
		{
			result = STK831x_WriteByteOTP(0x50, regR[0]);
			if(result < 0)
				goto eng_i2c_err;
			result = STK831x_WriteByteOTP(0x51, regR[1]);
			if(result < 0)
				goto eng_i2c_err;		
			result = STK831x_WriteByteOTP(0x52, regR[2]);
			if(result < 0)
				goto eng_i2c_err;		
		}
	}
#ifdef STK_DEBUG_CALI
	printk(KERN_INFO "%s:OTP step1 Success!\n", __func__);
#endif
	buffer[0] = 0x2A;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	else
	{
		regR[0] = buffer[0];
	}
	buffer[0] = 0x2B;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	else
	{
		regR[1] = buffer[0];
	}
	buffer[0] = 0x2E;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	else
	{
		regR[2] = buffer[0];
	}
	buffer[0] = 0x2F;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	else
	{
		regR[3] = buffer[0];
	}
	buffer[0] = 0x32;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	else
	{
		regR[4] = buffer[0];
	}
	buffer[0] = 0x33;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto common_i2c_error;
	}
	else
	{
		regR[5] = buffer[0];
	}
	
	regR[1] = offsetData[0];
	regR[3] = offsetData[2];
	regR[5] = offsetData[1];
	if(FT==1)
	{
		result = STK831x_WriteByteOTP(0x44, regR[1]);
		if(result < 0)
			goto eng_i2c_err;		
		result = STK831x_WriteByteOTP(0x46, regR[3]);
		if(result < 0)
			goto eng_i2c_err;				
		result = STK831x_WriteByteOTP(0x48, regR[5]);
		if(result < 0)
			goto eng_i2c_err;				
		
		if(!ft_pre_trim)
		{
			result = STK831x_WriteByteOTP(0x43, regR[0]);
			if(result < 0)
				goto eng_i2c_err;		
			result = STK831x_WriteByteOTP(0x45, regR[2]);
			if(result < 0)
				goto eng_i2c_err;				
			result = STK831x_WriteByteOTP(0x47, regR[4]);
			if(result < 0)
				goto eng_i2c_err;				
		}
	}
	else if (FT == 2)
	{
		result = STK831x_WriteByteOTP(0x54, regR[1]);
		if(result < 0)
			goto eng_i2c_err;
		result = STK831x_WriteByteOTP(0x56, regR[3]);
		if(result < 0)
			goto eng_i2c_err;				
		result = STK831x_WriteByteOTP(0x58, regR[5]);
		if(result < 0)
			goto eng_i2c_err;				

		if(!ft_pre_trim)
		{		
			result = STK831x_WriteByteOTP(0x53, regR[0]);
			if(result < 0)
				goto eng_i2c_err;								
			result = STK831x_WriteByteOTP(0x55, regR[2]);
			if(result < 0)
				goto eng_i2c_err;				
			result = STK831x_WriteByteOTP(0x57, regR[4]);
			if(result < 0)
				goto eng_i2c_err;				
		}
	}
#ifdef STK_DEBUG_CALI	
	printk(KERN_INFO "%s:OTP step2 Success!\n", __func__);
#endif
	result = STK831x_ReadByteOTP(0x7F, &regR[0]);
	if(result < 0)
		goto eng_i2c_err;
	
	if(FT==1)
		regR[0] = regR[0]|0x10;
	else if(FT==2)
		regR[0] = regR[0]|0x20;

	result = STK831x_WriteByteOTP(0x7F, regR[0]);
	if(result < 0)
		goto eng_i2c_err;
#ifdef STK_DEBUG_CALI	
	printk(KERN_INFO "%s:OTP step3 Success!\n", __func__);
#endif	
	return 0;
	
eng_i2c_err:
	printk(KERN_ERR "%s: read/write eng i2c error, result=0x%x\n", __func__, result);	
	return result;
	
common_i2c_error:
	printk(KERN_ERR "%s: read/write common i2c error, result=0x%x\n", __func__, result);
	return result;	
}

static int STK831X_VerifyCali(struct i2c_client *client, unsigned char en_dis, uint32_t delay_ms)
{
	unsigned char axis, state;	
	int acc_ave[3] = {0, 0, 0};
	s16 acc_adc[STK831X_BUFSIZE] = {0};
	const unsigned char verify_sample_no = 3;		
#ifdef SENSOR_STK8313
	const unsigned char verify_diff = 25;	
#elif defined SENSOR_STK8312
	const unsigned char verify_diff = 2;		
#endif		
	int result;
	char buffer[2] = "";
	int ret = 0;
	
	if(en_dis)
	{
		STK831X_SetDelay(client, 2);
		buffer[0] = STK831X_REG_MODE;
		result = STK_i2c_Rx(buffer, 1);	
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);
			return -STK_K_FAIL_I2C;
		}			
		buffer[1] = (buffer[0] & 0xF8) | 0x01;
		buffer[0] = STK831X_REG_MODE;	
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);			
			return -STK_K_FAIL_I2C;
		}
		STK831X_SetVD(client);			
		msleep(delay_ms*15);	
	}
	
	for(state=0;state<verify_sample_no;state++)
	{
		STK831X_ReadData(client, acc_adc);
		for(axis=0;axis<3;axis++)			
			acc_ave[axis] += (int)acc_adc[axis];	
#ifdef STK_DEBUG_CALI				
		printk(KERN_INFO "%s: acc=%d,%d,%d\n", __func__, acc_adc[0], acc_adc[1], acc_adc[2]);	
#endif
		msleep(delay_ms);		
	}		
	
	for(axis=0;axis<3;axis++)
		acc_ave[axis] /= verify_sample_no;
	
	switch(stk831x_placement)
	{
	case POSITIVE_X_UP:
		acc_ave[0] -= STK_LSB_1G;
		break;
	case NEGATIVE_X_UP:
		acc_ave[0] += STK_LSB_1G;		
		break;
	case POSITIVE_Y_UP:
		acc_ave[1] -= STK_LSB_1G;
		break;
	case NEGATIVE_Y_UP:
		acc_ave[1] += STK_LSB_1G;
		break;
	case POSITIVE_Z_UP:
		acc_ave[2] -= STK_LSB_1G;
		break;
	case NEGATIVE_Z_UP:
		acc_ave[2] += STK_LSB_1G;
		break;
	default:
		printk("%s: invalid stk831x_placement=%d\n", __func__, stk831x_placement);
		ret = -STK_K_FAIL_PLACEMENT;
		break;
	}	
	if(abs(acc_ave[0]) > verify_diff || abs(acc_ave[1]) > verify_diff || abs(acc_ave[2]) > verify_diff)
	{
		printk(KERN_INFO "%s:Check data x:%d, y:%d, z:%d\n", __func__,acc_ave[0],acc_ave[1],acc_ave[2]);		
		printk(KERN_ERR "%s:Check Fail, Calibration Fail\n", __func__);
		ret = -STK_K_FAIL_LRG_DIFF;
	}	
#ifdef STK_DEBUG_CALI
	else
		printk(KERN_INFO "%s:Check data pass\n", __func__);
#endif	
	if(en_dis)
	{
		buffer[0] = STK831X_REG_MODE;
		result = STK_i2c_Rx(buffer, 1);	
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);			
			return -STK_K_FAIL_I2C;
		}			
		buffer[1] = (buffer[0] & 0xF8);
		buffer[0] = STK831X_REG_MODE;	
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			printk(KERN_ERR "%s:failed, result=0x%x\n", __func__, result);			
			return -STK_K_FAIL_I2C;
		}		
	}	
	
	return ret;
}

static int STK831x_SetCali(struct i2c_client *client, char sstate)
{
	bool org_enable;
	int acc_ave[3] = {0, 0, 0};
	int state, axis;
	int new_offset[3];
	char char_offset[3] = {0};
	int result;
	char buffer[2] = "";
	char reg_offset[3] = {0};
	s16 acc_adc[STK831X_BUFSIZE] = {0};
	char store_location = sstate;
	uint32_t gdelay_ns = 0;
	uint32_t real_delay_ms = 0;
	char offset[3];	
	
	atomic_set(&cali_status, STK_K_RUNNING);	
	//sstate=1, STORE_OFFSET_IN_FILE
	//sstate=2, STORE_OFFSET_IN_IC		
#ifdef STK_DEBUG_CALI		
	printk(KERN_INFO "%s:store_location=%d\n", __func__, store_location);
#endif	
	if((store_location != 3 && store_location != 2 && store_location != 1) || (stk831x_placement < 0 || stk831x_placement > 5) )
	{
		printk(KERN_ERR "%s, invalid parameters\n", __func__);
		atomic_set(&cali_status, STK_K_FAIL_K_PARA);	
		return -STK_K_FAIL_K_PARA;
	}	
	STK831x_GetDelay(client, &gdelay_ns);
	org_enable = sensor_power;
	if(org_enable)
		STK831X_SetPowerMode(client, 0);
	STK831X_SetDelay(client, 2);
	msleep(1);
	STK831x_GetDelay(client, &real_delay_ms);
	real_delay_ms = (real_delay_ms + (NSEC_PER_MSEC / 2)) / NSEC_PER_MSEC;
	printk(KERN_INFO "%s: delay =%d ms\n", __func__, real_delay_ms);
	
	STK831x_SetOffset(reg_offset);
	buffer[0] = STK831X_REG_MODE;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto err_i2c_rw;
	}			
	buffer[1] = (buffer[0] & 0xF8) | 0x01;
	buffer[0] = STK831X_REG_MODE;	
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto err_i2c_rw;
	}

	STK831X_SetVD(client);
	if(store_location >= 2)
	{
		buffer[0] = 0x2B;	
		buffer[1] = 0x0;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			goto err_i2c_rw;
		}
		buffer[0] = 0x2F;	
		buffer[1] = 0x0;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			goto err_i2c_rw;
		}
		buffer[0] = 0x33;	
		buffer[1] = 0x0;
		result = STK_i2c_Tx(buffer, 2);
		if (result < 0) 
		{
			goto err_i2c_rw;
		}
	}	
				
	msleep(real_delay_ms*20);				
	for(state=0;state<STK_SAMPLE_NO;state++)
	{
		STK831X_ReadData(client, acc_adc);
		for(axis=0;axis<3;axis++)			
			acc_ave[axis] += (int)acc_adc[axis];	
#ifdef STK_DEBUG_CALI				
		printk(KERN_INFO "%s: acc=%d,%d,%d\n", __func__, acc_adc[0], acc_adc[1], acc_adc[2]);	
#endif		
		msleep(real_delay_ms);		
	}		
	buffer[0] = STK831X_REG_MODE;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{
		goto err_i2c_rw;
	}			
	buffer[1] = (buffer[0] & 0xF8);
	buffer[0] = STK831X_REG_MODE;	
	result = STK_i2c_Tx(buffer, 2);
	if (result < 0) 
	{
		goto err_i2c_rw;
	}	
	
	for(axis=0;axis<3;axis++)
	{
		if(acc_ave[axis] >= 0)
			acc_ave[axis] = (acc_ave[axis] + STK_SAMPLE_NO / 2) / STK_SAMPLE_NO;
		else
			acc_ave[axis] = (acc_ave[axis] - STK_SAMPLE_NO / 2) / STK_SAMPLE_NO;
	
	}
	if(acc_ave[2] <= -1)
		stk831x_placement = NEGATIVE_Z_UP;
	else if((acc_ave[2] >= 1))
		stk831x_placement = POSITIVE_Z_UP;
#ifdef STK_DEBUG_CALI		
	printk(KERN_INFO "%s:stk831x_placement=%d\n", __func__, stk831x_placement);
#endif	
	
	switch(stk831x_placement)
	{
	case POSITIVE_X_UP:
		acc_ave[0] -= STK_LSB_1G;
		break;
	case NEGATIVE_X_UP:
		acc_ave[0] += STK_LSB_1G;		
		break;
	case POSITIVE_Y_UP:
		acc_ave[1] -= STK_LSB_1G;
		break;
	case NEGATIVE_Y_UP:
		acc_ave[1] += STK_LSB_1G;
		break;
	case POSITIVE_Z_UP:
		acc_ave[2] -= STK_LSB_1G;
		break;
	case NEGATIVE_Z_UP:
		acc_ave[2] += STK_LSB_1G;
		break;
	default:
		printk("%s: invalid stk831x_placement=%d\n", __func__, stk831x_placement);
		atomic_set(&cali_status, STK_K_FAIL_PLACEMENT);	
		return -STK_K_FAIL_K_PARA;
		break;
	}		
	
	for(axis=0;axis<3;axis++)
	{
		acc_ave[axis] = -acc_ave[axis];
		new_offset[axis] = acc_ave[axis];
		char_offset[axis] = new_offset[axis];
	}				
#ifdef STK_DEBUG_CALI	
	printk(KERN_INFO "%s: New offset:%d,%d,%d\n", __func__, new_offset[0], new_offset[1], new_offset[2]);	
#endif	
	if(store_location == 1)
	{
		STK831x_SetOffset(char_offset);
		msleep(1);
		STK831x_GetOffset(reg_offset);
		for(axis=0;axis<3;axis++)
		{
			if(char_offset[axis] != reg_offset[axis])		
			{
				printk(KERN_ERR "%s: set offset to register fail!, char_offset[%d]=%d,reg_offset[%d]=%d\n", 
					__func__, axis,char_offset[axis], axis, reg_offset[axis]);
				atomic_set(&cali_status, STK_K_FAIL_WRITE_NOFST);				
				return -STK_K_FAIL_WRITE_NOFST;
			}
		}
		
		
		result = STK831X_VerifyCali(client, 1, real_delay_ms);
		if(result)
		{
			printk(KERN_ERR "%s: calibration check fail, result=0x%x\n", __func__, result);
			atomic_set(&cali_status, -result);
		}
		else
		{
			result = stk_store_in_file(char_offset, STK_K_SUCCESS_FILE);
			if(result)
			{
				printk(KERN_INFO "%s:write calibration failed\n", __func__);
				atomic_set(&cali_status, -result);				
			}
			else
			{
				printk(KERN_INFO "%s successfully\n", __func__);
				atomic_set(&cali_status, STK_K_SUCCESS_FILE);
			}		
			
		}
	}
	else if(store_location >= 2)
	{
		for(axis=0; axis<3; axis++)
		{
#ifdef SENSOR_STK8313
			new_offset[axis]>>=2;
#endif				
			char_offset[axis] = (char)new_offset[axis];
			if( (char_offset[axis]>>7)==0)
			{
				if(char_offset[axis] >= 0x20 )
				{
					printk(KERN_ERR "%s: offset[%d]=0x%x is too large, limit to 0x1f\n", 
									__func__, axis, char_offset[axis] );
					char_offset[axis] = 0x1F;
					//atomic_set(&cali_status, STK_K_FAIL_OTP_OUT_RG);						
					//return -STK_K_FAIL_OTP_OUT_RG;
				}
			}	
			else
			{
				if(char_offset[axis] <= 0xDF)
				{
					printk(KERN_ERR "%s: offset[%d]=0x%x is too large, limit to 0x20\n", 
									__func__, axis, char_offset[axis]);				
					char_offset[axis] = 0x20;
					//atomic_set(&cali_status, STK_K_FAIL_OTP_OUT_RG);			
					//return -STK_K_FAIL_OTP_OUT_RG;					
				}
				else
					char_offset[axis] = char_offset[axis] & 0x3f;
			}			
		}

		printk(KERN_INFO "%s: OTP offset:0x%x,0x%x,0x%x\n", __func__, char_offset[0], char_offset[1], char_offset[2]);
		if(store_location == 2)
		{
			result = stk_store_in_ic( client, char_offset, 1, real_delay_ms);
			if(result == 0)
			{
				printk(KERN_INFO "%s successfully\n", __func__);
				atomic_set(&cali_status, STK_K_SUCCESS_FT1);
			}
			else
			{
				printk(KERN_ERR "%s fail, result=%d\n", __func__, result);
			}
		}
		else if(store_location == 3)
		{
			result = stk_store_in_ic( client, char_offset, 2, real_delay_ms);
			if(result == 0)
			{
				printk(KERN_INFO "%s successfully\n", __func__);
				atomic_set(&cali_status, STK_K_SUCCESS_FT2);
			}
			else
			{
				printk(KERN_ERR "%s fail, result=%d\n", __func__, result);
			}
		}
		offset[0] = offset[1] = offset[2] = 0;
		stk_store_in_file(offset, store_location);				
	}
#ifdef STK_TUNE	
	stk_tune_offset_record[0] = 0;
	stk_tune_offset_record[1] = 0;
	stk_tune_offset_record[2] = 0;
	stk_tune_done = 1;
#endif	
	first_enable = false;		
	STK831X_SetDelay(client, STK831X_INIT_ODR);
	
	if(org_enable)
		STK831X_SetPowerMode(client, 1);		
	return 0;
	
err_i2c_rw:
	first_enable = false;		
	if(org_enable)
		STK831X_SetPowerMode(client, 1);				
	printk(KERN_ERR "%s: i2c read/write error, err=0x%x\n", __func__, result);
	atomic_set(&cali_status, STK_K_FAIL_I2C);	
	return result;
}


static int STK831x_GetCali(struct i2c_client *client)
{
	char r_buf[STK_ACC_CALI_FILE_SIZE] = {0};
	char offset[3], mode;	
	int cnt, result;
	char regR[6];
	
#ifdef STK_TUNE		
	printk(KERN_INFO "%s: stk_tune_done=%d, stk_tune_index=%d, stk_tune_offset=%d,%d,%d\n", __func__, 
		stk_tune_done, stk_tune_index, stk_tune_offset_record[0], stk_tune_offset_record[1], 
		stk_tune_offset_record[2]);
#endif		
	if ((stk_get_file_content(r_buf, STK_ACC_CALI_FILE_SIZE)) == 0)
	{
		if(r_buf[0] == STK_ACC_CALI_VER0 && r_buf[1] == STK_ACC_CALI_VER1)
		{
			offset[0] = r_buf[2];
			offset[1] = r_buf[3];
			offset[2] = r_buf[4];
			mode = r_buf[5];
			printk(KERN_INFO "%s:file offset:%#02x,%#02x,%#02x,%#02x\n", 
				__func__, offset[0], offset[1], offset[2], mode);							
		}
		else
		{
			printk(KERN_ERR "%s: cali version number error! r_buf=0x%x,0x%x,0x%x,0x%x,0x%x\n", 
				__func__, r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4]);						
		}
	}
	else
		printk(KERN_INFO "%s: No file offset\n", __func__);
	
	for(cnt=0x43;cnt<0x49;cnt++)
	{
		result = STK831x_ReadByteOTP(cnt, &(regR[cnt-0x43]));
		if(result < 0)
			printk(KERN_ERR "%s: STK831x_ReadByteOTP failed, ret=%d\n", __func__, result);		
	}
	printk(KERN_INFO "%s: OTP 0x43-0x49:%#02x,%#02x,%#02x,%#02x,%#02x,%#02x\n", __func__, regR[0], 
		regR[1], regR[2],regR[3], regR[4], regR[5]);
		
	for(cnt=0x53;cnt<0x59;cnt++)
	{
		result = STK831x_ReadByteOTP(cnt, &(regR[cnt-0x53]));
		if(result < 0)
			printk(KERN_ERR "%s: STK831x_ReadByteOTP failed, ret=%d\n", __func__, result);
	}
	printk(KERN_INFO "%s: OTP 0x53-0x59:%#02x,%#02x,%#02x,%#02x,%#02x,%#02x\n", __func__, regR[0], 
		regR[1], regR[2],regR[3], regR[4], regR[5]);
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int STK831X_SetPowerModeToWrite(struct i2c_client *client, bool enable)
{
	u8 databuf[2];    
	int res = 0;
	//u8 addr = STK831X_REG_MODE;
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int k_status = atomic_read(&cali_status);
	
	//GSE_FUN();	
	printk("%s: enable=%d", __func__, enable);	
	if(enable == true)
	{
		databuf[1] = STK831X_ACTIVE_MODE;
	}
	else
	{
		databuf[1] = STK831X_STANDBY_MODE;
	}
	
	databuf[0] = STK831X_REG_MODE;
	res = i2c_master_send(client, databuf, 0x2);
	
	if(res <= 0)
	{
		GSE_LOG("fwq set power mode failed!\n");
		return STK831X_ERR_I2C;
	}
	else if(atomic_read(&obj->trace) & ADX_TRC_INFO)
	{
		GSE_LOG("fwq set power mode ok %d!\n", databuf[1]);
	}
	
	if(enable)
	{
		STK831X_SetVD(client);
		atomic_set(&obj->event_since_en, 0);
#ifdef STK_TUNE		
		if((k_status&0xF0) != 0 && stk_tune_done == 0)
		{
			stk_tune_index = 0;
			STK831x_ResetPara();
		}
#endif
	}
	return STK831X_SUCCESS;    	
}

static int STK831X_SetPowerMode(struct i2c_client *client, bool enable)
{
	u8 databuf[2];    
	int res = 0;
	//u8 addr = STK831X_REG_MODE;
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int k_status = atomic_read(&cali_status);
	
	//GSE_FUN();
	printk("%s: enable=%d", __func__, enable);
	if(enable == sensor_power)
	{
		GSE_LOG("Sensor power status need not to be set again!!!\n");
		return STK831X_SUCCESS;
	}
	
	
	if(first_enable && k_status != STK_K_RUNNING)
		stk_handle_first_en(client);


	if(enable)
	{
		databuf[0] = STK831X_REG_MODE;	
		databuf[1] = 0x00;
		res = i2c_master_send(client, databuf, 0x2);	

		databuf[0] = STK831X_REG_XYZ_DATA_CFG;	
#ifdef SENSOR_STK8312	
		databuf[1] = 0x42;
#elif defined SENSOR_STK8313
		databuf[1] = 0x82;
#endif	

		res = i2c_master_send(client, databuf, 0x2);
		GSE_LOG("%s: force write reg 0x13 = 0x42 when enable\n", __func__);
	}
	
	if(enable == true)
	{
		databuf[1] = STK831X_ACTIVE_MODE;
		atomic_set(&obj->event_since_en, 0);
#ifdef STK_TUNE		
		if((k_status&0xF0) != 0 && stk_tune_done == 0)
		{
			stk_tune_index = 0;
			STK831x_ResetPara();
		}
#endif
	}
	else
	{
		databuf[1] = STK831X_STANDBY_MODE;
	}
	databuf[0] = STK831X_REG_MODE;
	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		GSE_LOG("fwq set power mode failed!\n");
		return STK831X_ERR_I2C;
	}
	else if(atomic_read(&obj->trace) & ADX_TRC_INFO)
	{
		GSE_LOG("fwq set power mode ok %d!\n", databuf[1]);
	}
	
	if(first_enable && k_status != STK_K_RUNNING)
	{
		first_enable = false;	
		msleep(2);
		res = stk_get_ic_content();			
	}	
	
	if(enable)
	{
		STK831X_SetVD(client);
	}
	sensor_power = enable;
	
	msleep(5);
	return STK831X_SUCCESS;    
}

static int STK831X_SBtoWriteReg(struct i2c_client *client, bool standby)
{
	if(sensor_power)
		STK831X_SetPowerModeToWrite(client, !standby);		
	
	return STK831X_SUCCESS;
}

static int STK831X_SetVD(struct i2c_client *client)
{
	u8 databuf[2];    
	int err, redo = 0;
	//struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	u8 reg24, dat;
	
	msleep(2);
	databuf[0] = 0x3D;
	databuf[1] = 0x70;
	err = i2c_master_send(client, databuf, 0x2);	
	if(err <= 0)
	{
		GSE_LOG("%s: failed!\n", __func__);
		return STK831X_ERR_I2C;
	}	

	databuf[0] = 0x3F;
	databuf[1] = 0x02;
	err = i2c_master_send(client, databuf, 0x2);	
	if(err <= 0)
	{
		GSE_LOG("%s: failed!\n", __func__);
		return STK831X_ERR_I2C;
	}	
	msleep(1);	
	do {
		if((err = hwmsen_read_byte_sr(client, 0x3F, &dat)))
			GSE_ERR("error: %d\n", err);
	
		if(dat & 0x80)
		{
			break;
		}		
		msleep(1);
		redo++;
	}while(redo < 5);	
	
	if(redo == 5)
	{
		GSE_ERR("%s:OTP read repeat read 5 times! Failed!\n", __func__);
		return STK831X_ERR_I2C;
	}	
	
	if((err = hwmsen_read_byte_sr(client, 0x3E, &reg24)))
		GSE_ERR("error: %d\n", err);
	

	if(reg24 != 0)
	{
		databuf[0] = 0x24;
		databuf[1] = reg24;
		err = i2c_master_send(client, databuf, 0x2);	
		if(err <= 0)
		{
			GSE_LOG("%s: failed!\n", __func__);
			return STK831X_ERR_I2C;
		}			
	}	
	else
	{
		//GSE_LOG("%s: reg24=0, do nothing\n", __func__);
		return 0;
	}
	
	if((err = hwmsen_read_byte_sr(client, 0x24, &dat)))
		GSE_ERR("%s:error: %d\n", __func__, err);
		
	if(dat != reg24)	
	{
		GSE_ERR("%s: error, reg24=0x%x, read=0x%x\n", __func__, reg24, dat);
		return STK831X_ERR_SETUP_FAILURE;
	}
	GSE_LOG("%s: successfully\n", __func__);
	
	return 0;
}


static int STK831X_SetDelay(struct i2c_client *client, u8 delay)
{
	u8 databuf[2];    
	int err;
	u8 dat;
	
	STK831X_SBtoWriteReg(client, true);
	if((err = hwmsen_read_byte_sr(client, STK831X_REG_SR, &dat)))
		GSE_ERR("%s:error: %d\n", __func__, err);	
		
	databuf[0] = STK831X_REG_SR;
	databuf[1] = (dat & 0xF8) | (delay & 0x07);
	err = i2c_master_send(client, databuf, 0x2);	
	if(err <= 0)
	{
		GSE_LOG("%s: failed!\n", __func__);
		return STK831X_ERR_I2C;
	}	
	STK831X_SBtoWriteReg(client, false);	
	return STK831X_SUCCESS;	
}

static int STK831x_GetDelay(struct i2c_client *client, uint32_t *gdelay_ns)
{
	int result;
	char buffer[2] = "";
	
	buffer[0] =  STK831X_REG_SR;
	result = STK_i2c_Rx(buffer, 1);	
	if (result < 0) 
	{	
		printk(KERN_ERR "%s:failed\n", __func__);
		return result;
	}	
	*gdelay_ns = (uint32_t) STK831X_SAMPLE_TIME[(int)buffer[0]] * 1000;
	return 0;	
}
/*----------------------------------------------------------------------------*/
//this function here use to set resolution and choose sensitivity
static int STK831X_SetDataResolution(struct i2c_client *client ,u8 dataresolution)
{
	int err;
	u8  dat, reso=0;
    //int res = 0;
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
    GSE_LOG("fwq set resolution  dataresolution= %d!\n", dataresolution);	
    //choose sensitivity depend on resolution and detect range
	//read detect range
	if((err = hwmsen_read_byte_sr(client, STK831X_REG_XYZ_DATA_CFG, &dat)))
	{
		GSE_ERR("read detect range  fail!!\n");
		return err;
	}	
	GSE_LOG("dat=%x!! OK \n",dat);
	dat = (dat&0xc0)>>6;
	GSE_LOG("dat=%x!! OK \n",dat);
#ifdef SENSOR_STK8312	
	switch(dat)
	{
	case STK831X_RANGE_1_5G:
		reso = STK831X_RANGE_1_5G;
		break;
	case STK831X_RANGE_6G:
		reso = STK831X_RANGE_6G;
		break;
	case STK831X_RANGE_16G:
		reso = STK831X_RANGE_16G;
		break;		
	}

	GSE_LOG("reso=%x!! OK \n",reso);	
	if(reso < sizeof(stk831x_data_resolution)/sizeof(stk831x_data_resolution[0]))
	{        
		obj->reso = &stk831x_data_resolution[reso];
		GSE_LOG("reso=%x!! OK \n",reso);
		return 0;
	}
	else
	{   
	    GSE_ERR("choose sensitivity  fail!!\n");
		return -EINVAL;
	}
#elif defined SENSOR_STK8313	
	switch(dat)
	{
	case STK831X_RANGE_2G:
		reso = STK831X_RANGE_2G;
		break;
	case STK831X_RANGE_4G:
		reso = STK831X_RANGE_4G;
		break;
	case STK831X_RANGE_8G:
		reso = STK831X_RANGE_8G;
		break;		
	case STK831X_RANGE_16G:
		reso = STK831X_RANGE_16G;
		break;	
	}

	GSE_LOG("reso=%x!! OK \n",reso);	
	if(reso < sizeof(stk831x_data_resolution)/sizeof(stk831x_data_resolution[0]))
	{        
		obj->reso = &stk831x_data_resolution[reso];
		GSE_LOG("reso=%x!! OK \n",reso);
		return 0;
	}
	else
	{   
	    GSE_ERR("choose sensitivity  fail!!\n");
		return -EINVAL;
	}
#endif
}
/*----------------------------------------------------------------------------*/
static int STK831X_CheckReading(int acc[], bool clear)
{	
	struct i2c_client *client = stk831x_i2c_client;  
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	static int check_result = 0;
#ifdef SENSOR_STK8312
	int verifing[] = {-128,127};	
#elif defined SENSOR_STK8313
	int verifing[] = {-2048,2047};		
#endif	
	
	if(acc[0] == verifing[0] || acc[0] == verifing[1] || acc[1] == verifing[0] || acc[1] == verifing[1] || 
			acc[2] == verifing[0] || acc[2] == verifing[1])
	{
		printk(KERN_INFO "%s: acc:%o,%o,%o\n", __func__, acc[0], acc[1], acc[2]);
		check_result++;		
	}	
	if(clear)
	{
		if(check_result == 3)
		{
			atomic_set(&obj->event_since_en_limit, 10000); 
			printk(KERN_INFO "%s: incorrect reading\n", __func__);		
			check_result = 0;
			return 1;
		}
		check_result = 0;
	}
	return 0;		
}
/*----------------------------------------------------------------------------*/
static int STK831X_ReadData(struct i2c_client *client, s16 data[STK831X_AXES_NUM])
{
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);     
	//u8 addr = STK831X_REG_DATAX0;
	u8 buf[STK831X_DATA_LEN] = {0};
	int err = 0;
	int k_status = atomic_read(&cali_status);
	int acc_xyz[3] = {0};
#ifdef STK_ZG_FILTER	
	s16 zero_fir = 0;	
#endif
	//s16 tmp;//breaze
	
	//u8 cnt;
	if(NULL == client)
	{
		err = -EINVAL;
	}
	else		
	{
		/*
		buf[0] = STK831X_REG_DATAX0;
	    client->addr = client->addr& I2C_MASK_FLAG | I2C_WR_FLAG |I2C_RS_FLAG;
        i2c_master_send(client, (const char*)&buf, 6<<8 | 1);
	    client->addr = client->addr& I2C_MASK_FLAG;
		*/	
	
		if((err = stk831x_hwmsen_read_block(client, STK831X_REG_DATAX0, buf, STK831X_DATA_LEN)))
		{
			GSE_ERR("%s: error!\n", __func__);
			return err;			
		}
	/*
		for(cnt=0;cnt<STK831X_DATA_LEN;cnt++)
		{
			if((err = hwmsen_read_byte_sr(client, STK831X_REG_DATAX0 + cnt, &(buf[cnt]))))
			{
				GSE_ERR("%s: error!\n", __func__);
				return err;
			}	
		}
	*/
#ifdef STK_DEBUG_PRINT
	#ifdef SENSOR_STK8312
		GSE_LOG("buf[STK831X_AXIS_X] = %d\n",buf[STK831X_AXIS_X]);
		GSE_LOG("buf[STK831X_AXIS_Y] = %d\n",buf[STK831X_AXIS_Y]);
		GSE_LOG("buf[STK831X_AXIS_Z] = %d\n",buf[STK831X_AXIS_Z]);
	#elif defined SENSOR_STK8313
		GSE_LOG("buf[STK831X_AXIS_X] = %d\n",(buf[STK831X_AXIS_X*2]<<4) + (buf[STK831X_AXIS_X*2+1]>>4));
		GSE_LOG("buf[STK831X_AXIS_Y] = %d\n",(buf[STK831X_AXIS_Y*2]<<4) + (buf[STK831X_AXIS_Y*2+1]>>4));
		GSE_LOG("buf[STK831X_AXIS_Z] = %d\n",(buf[STK831X_AXIS_Z*2]<<4) + (buf[STK831X_AXIS_Z*2+1]>>4));
	#endif
#endif
	
#ifdef SENSOR_STK8312
		if (buf[0] & 0x80)
			data[STK831X_AXIS_X] = (s16) buf[STK831X_AXIS_X] - 256;
		else
			data[STK831X_AXIS_X] = (s16) buf[STK831X_AXIS_X];
			
		if (buf[1] & 0x80)
			data[STK831X_AXIS_Y] = (s16) buf[STK831X_AXIS_Y] - 256;
		else
			data[STK831X_AXIS_Y] = (s16) buf[STK831X_AXIS_Y];
			
		if (buf[2] & 0x80)
			data[STK831X_AXIS_Z] = (s16) buf[STK831X_AXIS_Z] - 256;
		else
			data[STK831X_AXIS_Z] = (s16) buf[STK831X_AXIS_Z];				
#elif defined SENSOR_STK8313
		if (buf[0] & 0x80)
			data[STK831X_AXIS_X] = (s16) ((buf[STK831X_AXIS_X*2]<<4) + (buf[STK831X_AXIS_X*2+1]>>4) - 4096);
		else
			data[STK831X_AXIS_X] = (s16) ((buf[STK831X_AXIS_X*2]<<4) + (buf[STK831X_AXIS_X*2+1]>>4));
			
		if (buf[2] & 0x80)
			data[STK831X_AXIS_Y] = (s16) ((buf[STK831X_AXIS_Y*2]<<4) + (buf[STK831X_AXIS_Y*2+1]>>4) - 4096);
		else
			data[STK831X_AXIS_Y] = (s16) ((buf[STK831X_AXIS_Y*2]<<4) + (buf[STK831X_AXIS_Y*2+1]>>4));
			
		if (buf[4] & 0x80)
			data[STK831X_AXIS_Z] = (s16) ((buf[STK831X_AXIS_Z*2]<<4) + (buf[STK831X_AXIS_Z*2+1]>>4) - 4096);
		else
			data[STK831X_AXIS_Z] = (s16) ((buf[STK831X_AXIS_Z*2]<<4) + (buf[STK831X_AXIS_Z*2+1]>>4));	

//		data[STK831X_AXIS_X] = (s16)((buf[STK831X_AXIS_X*2] << 8) |
//		(buf[STK831X_AXIS_X*2+1]));
//		data[STK831X_AXIS_Y] = (s16)((buf[STK831X_AXIS_Y*2] << 8) |
//		(buf[STK831X_AXIS_Y*2+1]));
//		data[STK831X_AXIS_Z] = (s16)((buf[STK831X_AXIS_Z*2] << 8) |
//		(buf[STK831X_AXIS_Z*2+1]));

#endif		 
		
		if(atomic_read(&obj->trace) & ADX_TRC_REGXYZ)
		{
			GSE_LOG("raw from reg(SR) [%08X %08X %08X] => [%5d %5d %5d]\n", data[STK831X_AXIS_X], data[STK831X_AXIS_Y], data[STK831X_AXIS_Z],
		                               data[STK831X_AXIS_X], data[STK831X_AXIS_Y], data[STK831X_AXIS_Z]);
		}
		//GSE_LOG("raw from reg(SR) [%08X %08X %08X] => [%5d %5d %5d]\n", data[MMA8452Q_AXIS_X], data[MMA8452Q_AXIS_Y], data[MMA8452Q_AXIS_Z],
		  //                             data[MMA8452Q_AXIS_X], data[MMA8452Q_AXIS_Y], data[MMA8452Q_AXIS_Z]);
		//add to fix data, refer to datasheet
		
		
		acc_xyz[STK831X_AXIS_X] = (int) data[STK831X_AXIS_X];
		acc_xyz[STK831X_AXIS_Y] = (int) data[STK831X_AXIS_Y];
		acc_xyz[STK831X_AXIS_Z] = (int) data[STK831X_AXIS_Z];
		if(atomic_read(&obj->event_since_en) == 16 || atomic_read(&obj->event_since_en) == 17)
			STK831X_CheckReading(acc_xyz, false);
		else if(atomic_read(&obj->event_since_en) == 18)
			STK831X_CheckReading(acc_xyz, true);
		
		
#ifdef CONFIG_STK831X_LOWPASS
		if(atomic_read(&obj->filter))
		{
			if(atomic_read(&obj->fir_en) && !atomic_read(&obj->suspend))
			{
				int idx, firlen = atomic_read(&obj->firlen);   
				if(obj->fir.num < firlen)
				{                
					obj->fir.raw[obj->fir.num][STK831X_AXIS_X] = data[STK831X_AXIS_X];
					obj->fir.raw[obj->fir.num][STK831X_AXIS_Y] = data[STK831X_AXIS_Y];
					obj->fir.raw[obj->fir.num][STK831X_AXIS_Z] = data[STK831X_AXIS_Z];
					obj->fir.sum[STK831X_AXIS_X] += data[STK831X_AXIS_X];
					obj->fir.sum[STK831X_AXIS_Y] += data[STK831X_AXIS_Y];
					obj->fir.sum[STK831X_AXIS_Z] += data[STK831X_AXIS_Z];
					if(atomic_read(&obj->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d]\n", obj->fir.num,
						obj->fir.raw[obj->fir.num][STK831X_AXIS_X], obj->fir.raw[obj->fir.num][STK831X_AXIS_Y], obj->fir.raw[obj->fir.num][STK831X_AXIS_Z],
						obj->fir.sum[STK831X_AXIS_X], obj->fir.sum[STK831X_AXIS_Y], obj->fir.sum[STK831X_AXIS_Z]);
					}
					obj->fir.num++;
					obj->fir.idx++;
				}
				else
				{
					idx = obj->fir.idx % firlen;
					obj->fir.sum[STK831X_AXIS_X] -= obj->fir.raw[idx][STK831X_AXIS_X];
					obj->fir.sum[STK831X_AXIS_Y] -= obj->fir.raw[idx][STK831X_AXIS_Y];
					obj->fir.sum[STK831X_AXIS_Z] -= obj->fir.raw[idx][STK831X_AXIS_Z];
					obj->fir.raw[idx][STK831X_AXIS_X] = data[STK831X_AXIS_X];
					obj->fir.raw[idx][STK831X_AXIS_Y] = data[STK831X_AXIS_Y];
					obj->fir.raw[idx][STK831X_AXIS_Z] = data[STK831X_AXIS_Z];
					obj->fir.sum[STK831X_AXIS_X] += data[STK831X_AXIS_X];
					obj->fir.sum[STK831X_AXIS_Y] += data[STK831X_AXIS_Y];
					obj->fir.sum[STK831X_AXIS_Z] += data[STK831X_AXIS_Z];
					obj->fir.idx++;

					data[STK831X_AXIS_X] = obj->fir.sum[STK831X_AXIS_X]/firlen;
					data[STK831X_AXIS_Y] = obj->fir.sum[STK831X_AXIS_Y]/firlen;
					data[STK831X_AXIS_Z] = obj->fir.sum[STK831X_AXIS_Z]/firlen;
					
					if(atomic_read(&obj->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d] : [%5d %5d %5d]\n", idx,
						obj->fir.raw[idx][STK831X_AXIS_X], obj->fir.raw[idx][STK831X_AXIS_Y], obj->fir.raw[idx][STK831X_AXIS_Z],
						obj->fir.sum[STK831X_AXIS_X], obj->fir.sum[STK831X_AXIS_Y], obj->fir.sum[STK831X_AXIS_Z],
						data[STK831X_AXIS_X], data[STK831X_AXIS_Y], data[STK831X_AXIS_Z]);
					}
				}
			}
		}	
#endif  		
		

#ifdef STK_TUNE
		if((k_status&0xF0) != 0)
		{
			acc_xyz[STK831X_AXIS_X] = (int) data[STK831X_AXIS_X];
			acc_xyz[STK831X_AXIS_Y] = (int) data[STK831X_AXIS_Y];
			acc_xyz[STK831X_AXIS_Z] = (int) data[STK831X_AXIS_Z];
		
			STK831x_Tune(client, acc_xyz);		
			
			data[STK831X_AXIS_X] = (s16) acc_xyz[STK831X_AXIS_X];
			data[STK831X_AXIS_Y] = (s16) acc_xyz[STK831X_AXIS_Y];
			data[STK831X_AXIS_Z] = (s16) acc_xyz[STK831X_AXIS_Z];
		}	
#endif	

/*
		tmp = data[STK831X_AXIS_X];//breaze 20130822
		data[STK831X_AXIS_X] = -data[STK831X_AXIS_Y];//breaze
		data[STK831X_AXIS_Y] = -tmp;//breaze

		data[STK831X_AXIS_Z] = -data[STK831X_AXIS_Z];
*/
		
#ifdef STK_ZG_FILTER	
		if( abs(data[STK831X_AXIS_X]) <= STK_ZG_COUNT)	
			data[STK831X_AXIS_X] = (data[STK831X_AXIS_X]*zero_fir);	
		if( abs(data[STK831X_AXIS_Y]) <= STK_ZG_COUNT)
			data[STK831X_AXIS_Y] = (data[STK831X_AXIS_Y]*zero_fir);
		if( abs(data[STK831X_AXIS_Z]) <= STK_ZG_COUNT)
			data[STK831X_AXIS_Z] = (data[STK831X_AXIS_Z]*zero_fir);	
#endif			

		if(atomic_read(&obj->trace) & ADX_TRC_RAWDATA)
		{
			GSE_LOG("raw >>6it:[%08X %08X %08X] => [%5d %5d %5d]\n", data[STK831X_AXIS_X], data[STK831X_AXIS_Y], data[STK831X_AXIS_Z],
		                               data[STK831X_AXIS_X], data[STK831X_AXIS_Y], data[STK831X_AXIS_Z]);
		}
		    
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int STK831X_ReadOffset(struct i2c_client *client, s8 ofs[STK831X_AXES_NUM])
{    
	int err;
    GSE_ERR("fwq read offset+: \n");
	if((err = hwmsen_read_byte_sr(client, STK831X_REG_OFSX, &ofs[STK831X_AXIS_X])))
	{
		GSE_ERR("error: %d\n", err);
	}
	if((err = hwmsen_read_byte_sr(client, STK831X_REG_OFSY, &ofs[STK831X_AXIS_Y])))
	{
		GSE_ERR("error: %d\n", err);
	}
	if((err = hwmsen_read_byte_sr(client, STK831X_REG_OFSZ, &ofs[STK831X_AXIS_Z])))
	{
		GSE_ERR("error: %d\n", err);
	}
	GSE_LOG("fwq read off:  offX=%x ,offY=%x ,offZ=%x\n",ofs[STK831X_AXIS_X],ofs[STK831X_AXIS_Y],ofs[STK831X_AXIS_Z]);
	
	return err;    
}
/*----------------------------------------------------------------------------*/
static int STK831X_ResetCalibration(struct i2c_client *client)
{
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	s8 ofs[STK831X_AXES_NUM] = {0x00, 0x00, 0x00};
	int err;

	//goto standby mode to clear cali
	STK831X_SetPowerModeToWrite(obj->client,false);
	if((err = hwmsen_write_block(client, STK831X_REG_OFSX, ofs, STK831X_AXES_NUM)))
	{
		GSE_ERR("error: %d\n", err);
	}
    STK831X_SetPowerModeToWrite(obj->client,true);
	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	return err;    
}
/*----------------------------------------------------------------------------*/
static int STK831X_ReadCalibration(struct i2c_client *client, int dat[STK831X_AXES_NUM])
{
    struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
    int err;
    int mul;
    
    if ((err = STK831X_ReadOffset(client, obj->offset))) {
        GSE_ERR("read offset fail, %d\n", err);
        return err;
    }    
    
    //mul = obj->reso->sensitivity/mma8452q_offset_resolution.sensitivity;
    mul = stk831x_offset_resolution.sensitivity/obj->reso->sensitivity;
    dat[obj->cvt.map[STK831X_AXIS_X]] = obj->cvt.sign[STK831X_AXIS_X]*(obj->offset[STK831X_AXIS_X]/mul);
    dat[obj->cvt.map[STK831X_AXIS_Y]] = obj->cvt.sign[STK831X_AXIS_Y]*(obj->offset[STK831X_AXIS_Y]/mul);
    dat[obj->cvt.map[STK831X_AXIS_Z]] = obj->cvt.sign[STK831X_AXIS_Z]*(obj->offset[STK831X_AXIS_Z]/mul);                        
    GSE_LOG("fwq:read cali offX=%x ,offY=%x ,offZ=%x\n",obj->offset[STK831X_AXIS_X],obj->offset[STK831X_AXIS_Y],obj->offset[STK831X_AXIS_Z]);
	//GSE_LOG("fwq:read cali swX=%x ,swY=%x ,swZ=%x\n",obj->cali_sw[MMA8452Q_AXIS_X],obj->cali_sw[MMA8452Q_AXIS_Y],obj->cali_sw[MMA8452Q_AXIS_Z]);
    return 0;
}
/*----------------------------------------------------------------------------*/
static int STK831X_ReadCalibrationEx(struct i2c_client *client, int act[STK831X_AXES_NUM], int raw[STK831X_AXES_NUM])
{  
	/*raw: the raw calibration data; act: the actual calibration data*/
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int mul;

	if((err = STK831X_ReadOffset(client, obj->offset)))
	{
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}    

	mul = stk831x_offset_resolution.sensitivity/obj->reso->sensitivity;
	raw[STK831X_AXIS_X] = obj->offset[STK831X_AXIS_X]/mul + obj->cali_sw[STK831X_AXIS_X];
	raw[STK831X_AXIS_Y] = obj->offset[STK831X_AXIS_Y]/mul + obj->cali_sw[STK831X_AXIS_Y];
	raw[STK831X_AXIS_Z] = obj->offset[STK831X_AXIS_Z]/mul + obj->cali_sw[STK831X_AXIS_Z];

	act[obj->cvt.map[STK831X_AXIS_X]] = obj->cvt.sign[STK831X_AXIS_X]*raw[STK831X_AXIS_X];
	act[obj->cvt.map[STK831X_AXIS_Y]] = obj->cvt.sign[STK831X_AXIS_Y]*raw[STK831X_AXIS_Y];
	act[obj->cvt.map[STK831X_AXIS_Z]] = obj->cvt.sign[STK831X_AXIS_Z]*raw[STK831X_AXIS_Z];                        
	                       
	return 0;
}
/*----------------------------------------------------------------------------*/
static int STK831X_WriteCalibration(struct i2c_client *client, int dat[STK831X_AXES_NUM])
{
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	//u8 testdata=0;
	int err;
	int cali[STK831X_AXES_NUM], raw[STK831X_AXES_NUM];
	int lsb = stk831x_offset_resolution.sensitivity;
	//u8 databuf[2]; 
	//int res = 0;
	//int divisor = obj->reso->sensitivity/lsb;
	int divisor = lsb/obj->reso->sensitivity;
	GSE_LOG("fwq obj->reso->sensitivity=%d\n", obj->reso->sensitivity);
	GSE_LOG("fwq lsb=%d\n", lsb);
	

	if((err = STK831X_ReadCalibrationEx(client, cali, raw)))	/*offset will be updated in obj->offset*/
	{ 
		GSE_ERR("read offset fail, %d\n", err);
		return err;
	}

	GSE_LOG("OLDOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		raw[STK831X_AXIS_X], raw[STK831X_AXIS_Y], raw[STK831X_AXIS_Z],
		obj->offset[STK831X_AXIS_X], obj->offset[STK831X_AXIS_Y], obj->offset[STK831X_AXIS_Z],
		obj->cali_sw[STK831X_AXIS_X], obj->cali_sw[STK831X_AXIS_Y], obj->cali_sw[STK831X_AXIS_Z]);

	/*calculate the real offset expected by caller*/
	cali[STK831X_AXIS_X] += dat[STK831X_AXIS_X];
	cali[STK831X_AXIS_Y] += dat[STK831X_AXIS_Y];
	cali[STK831X_AXIS_Z] += dat[STK831X_AXIS_Z];

	GSE_LOG("UPDATE: (%+3d %+3d %+3d)\n", 
		dat[STK831X_AXIS_X], dat[STK831X_AXIS_Y], dat[STK831X_AXIS_Z]);

	obj->offset[STK831X_AXIS_X] = (s8)(obj->cvt.sign[STK831X_AXIS_X]*(cali[obj->cvt.map[STK831X_AXIS_X]])*(divisor));
	obj->offset[STK831X_AXIS_Y] = (s8)(obj->cvt.sign[STK831X_AXIS_Y]*(cali[obj->cvt.map[STK831X_AXIS_Y]])*(divisor));
	obj->offset[STK831X_AXIS_Z] = (s8)(obj->cvt.sign[STK831X_AXIS_Z]*(cali[obj->cvt.map[STK831X_AXIS_Z]])*(divisor));

	/*convert software calibration using standard calibration*/
	obj->cali_sw[STK831X_AXIS_X] =0; //obj->cvt.sign[MMA8452Q_AXIS_X]*(cali[obj->cvt.map[MMA8452Q_AXIS_X]])%(divisor);
	obj->cali_sw[STK831X_AXIS_Y] =0; //obj->cvt.sign[MMA8452Q_AXIS_Y]*(cali[obj->cvt.map[MMA8452Q_AXIS_Y]])%(divisor);
	obj->cali_sw[STK831X_AXIS_Z] =0;// obj->cvt.sign[MMA8452Q_AXIS_Z]*(cali[obj->cvt.map[MMA8452Q_AXIS_Z]])%(divisor);

	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		obj->offset[STK831X_AXIS_X] + obj->cali_sw[STK831X_AXIS_X], 
		obj->offset[STK831X_AXIS_Y] + obj->cali_sw[STK831X_AXIS_Y], 
		obj->offset[STK831X_AXIS_Z] + obj->cali_sw[STK831X_AXIS_Z], 
		obj->offset[STK831X_AXIS_X], obj->offset[STK831X_AXIS_Y], obj->offset[STK831X_AXIS_Z],
		obj->cali_sw[STK831X_AXIS_X], obj->cali_sw[STK831X_AXIS_Y], obj->cali_sw[STK831X_AXIS_Z]);
	//
	//go to standby mode to set cali
    STK831X_SetPowerModeToWrite(obj->client,false);
	if((err = hwmsen_write_block(obj->client, STK831X_REG_OFSX, obj->offset, STK831X_AXES_NUM)))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
	STK831X_SetPowerModeToWrite(obj->client,true);
	
	//
	/*
	STK831X_SetPowerModeToWrite(obj->client,false);
	msleep(20);
	if(err = hwmsen_write_byte(obj->client, STK831X_REG_OFSX, obj->offset[STK831X_AXIS_X]))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
    msleep(20);
	hwmsen_read_byte_sr(obj->client,STK831X_REG_OFSX,&testdata);
	GSE_LOG("write offsetX: %x\n", testdata);
	
	if(err = hwmsen_write_byte(obj->client, STK831X_REG_OFSY, obj->offset[STK831X_AXIS_Y]))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
	msleep(20);
	hwmsen_read_byte_sr(obj->client,STK831X_REG_OFSY,&testdata);
	GSE_LOG("write offsetY: %x\n", testdata);
	
	if(err = hwmsen_write_byte(obj->client, STK831X_REG_OFSZ, obj->offset[STK831X_AXIS_Z]))
	{
		GSE_ERR("write offset fail: %d\n", err);
		return err;
	}
	msleep(20);
	hwmsen_read_byte_sr(obj->client,STK831X_REG_OFSZ,&testdata);
	GSE_LOG("write offsetZ: %x\n", testdata);
	STK831X_SetPowerModeToWrite(obj->client,true);
*/
	return err;
}
/*----------------------------------------------------------------------------*/

static int STK831X_SetReset(struct i2c_client *client)
{
	u8 databuf[2];    
	int res = 0;
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);	
	GSE_FUN();
	//STK831X_SBtoWriteReg(client, true);	
	
	databuf[1] = 0x00;
	databuf[0] = STK831X_REG_RESET;
	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		GSE_LOG("fwq set reset failed!\n");
		return STK831X_ERR_I2C;
	}
	else if(atomic_read(&obj->trace) & ADX_TRC_INFO)
	{
		GSE_LOG("fwq set reset ok %d!\n", databuf[1]);
	}
	//STK831X_SBtoWriteReg(client, false);	
	
	return STK831X_SUCCESS;    
}

/*----------------------------------------------------------------------------*/
//set detect range

static int STK831X_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
    
	//struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	u8 databuf[10];    
	int res = 0;
	
//	STK831X_SBtoWriteReg(client, true);	
	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = STK831X_REG_XYZ_DATA_CFG;    
	databuf[1] = (dataformat<<6)|0x02;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return STK831X_ERR_I2C;
	}
//	STK831X_SBtoWriteReg(client, false);	
	return 0;
	//return MMA8452Q_SetDataResolution(obj,dataformat);    
}

/*
static int STK831X_SetIntEnable(struct i2c_client *client, u8 intenable)
{
	u8 databuf[10];    
	int res = 0;
	
	STK831X_SBtoWriteReg(client, true);	
	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = STK831X_REG_INT;    
	databuf[1] = intenable?0x10:0x00;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return STK831X_ERR_I2C;
	}
	STK831X_SBtoWriteReg(client, false);		
	return STK831X_SUCCESS;    
}
*/
/*----------------------------------------------------------------------------*/
static int STK831X_Init(struct i2c_client *client, int reset_cali)
{
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;
    GSE_LOG("fwq stk831x addr %x!\n",client->addr);

	res = STK831X_SetReset(client);
	if(res != STK831X_SUCCESS)
	{
	    GSE_LOG("fwq stk831x set reset error\n");
		return res;
	}

#ifdef SENSOR_STK8312
	res = STK831X_SetDataFormat(client, STK831X_RANGE_6G);
	if(res != STK831X_SUCCESS)
	{
	    GSE_LOG("fwq stk831x set data format error\n");
		return res;
	}
	res = STK831X_SetDataResolution(client, STK831X_RANGE_6G);
#elif defined SENSOR_STK8313
	res = STK831X_SetDataFormat(client, STK831X_RANGE_8G);
	if(res != STK831X_SUCCESS)
	{
	    GSE_LOG("fwq stk831x set data format error\n");
		return res;
	}
	res = STK831X_SetDataResolution(client, STK831X_RANGE_8G);
#endif	
	if(res != STK831X_SUCCESS) 
	{
	    GSE_LOG("fwq stk831x set data reslution error\n");
		return res;
	}
	
	res = STK831X_SetDelay(client, STK831X_INIT_ODR);		
	if(res != STK831X_SUCCESS)
	{
	    GSE_LOG("fwq stk831x set delay error\n");
		return res;		
	}
	
	res = STK831X_SetPowerMode(client, false);
	if(res != STK831X_SUCCESS)
	{
	    GSE_LOG("fwq stk831x set power error\n");
		return res;
	}
	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = obj->reso->sensitivity;
/*//we do not use interrupt
	res = STK831X_SetIntEnable(client, STK831X_DATA_READY);        
	if(res != STK831X_SUCCESS)//0x2E->0x80
	{
		return res;
	}
*/
    
	//if(NULL != reset_cali)
	if(0 != reset_cali)
	{ 
		/*reset calibration only in power on*/
		GSE_LOG("fwq stk831x  set cali\n");
		res = STK831X_ResetCalibration(client);
		if(res != STK831X_SUCCESS)
		{
		    GSE_LOG("fwq stk831x set cali error\n");
			return res;
		}
	}

#ifdef STK_TUNE	
	stk_tune_offset[0] = 0;
	stk_tune_offset[1] = 0;
	stk_tune_offset[2] = 0;	
	stk_tune_done = 0;
	first_enable = true;
#endif	

#ifdef CONFIG_STK831X_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif

    GSE_LOG("fwq stk831x Init OK\n");
	return STK831X_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int STK831X_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];    

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "STK831X Chip");
	return 0;
}
/*----------------------------------------------------------------------------*/
static int STK831X_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
	struct stk831x_i2c_data *obj = (struct stk831x_i2c_data*)i2c_get_clientdata(client);
	u8 databuf[20];
	int acc[STK831X_AXES_NUM];
	int res = 0;
	memset(databuf, 0, sizeof(u8)*10);

	if(NULL == buf)
	{
		return -1;
	}
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	if(sensor_power == false)
	{
		res = STK831X_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on stk831x error %d!\n", res);
		}
	}

	if((res = STK831X_ReadData(client, obj->data)))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
		obj->data[STK831X_AXIS_X] += obj->cali_sw[STK831X_AXIS_X];
		obj->data[STK831X_AXIS_Y] += obj->cali_sw[STK831X_AXIS_Y];
		obj->data[STK831X_AXIS_Z] += obj->cali_sw[STK831X_AXIS_Z];
		
		/*remap coordinate*/
		acc[obj->cvt.map[STK831X_AXIS_X]] = obj->cvt.sign[STK831X_AXIS_X]*obj->data[STK831X_AXIS_X];
		acc[obj->cvt.map[STK831X_AXIS_Y]] = obj->cvt.sign[STK831X_AXIS_Y]*obj->data[STK831X_AXIS_Y];
		acc[obj->cvt.map[STK831X_AXIS_Z]] = obj->cvt.sign[STK831X_AXIS_Z]*obj->data[STK831X_AXIS_Z];

		//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[MMA8452Q_AXIS_X], acc[MMA8452Q_AXIS_Y], acc[MMA8452Q_AXIS_Z]);

		//Out put the mg
		acc[STK831X_AXIS_X] = acc[STK831X_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[STK831X_AXIS_Y] = acc[STK831X_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[STK831X_AXIS_Z] = acc[STK831X_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;		

#ifdef STK_DEBUG_PRINT
	    GSE_LOG("ReadSensorData acc[STK831X_AXIS_X] = %d\n",acc[STK831X_AXIS_X]);
	    GSE_LOG("ReadSensorData acc[STK831X_AXIS_Y] = %d\n",acc[STK831X_AXIS_Y]);
	    GSE_LOG("ReadSensorData acc[STK831X_AXIS_Z] = %d\n",acc[STK831X_AXIS_Z]);
#endif		

		sprintf(buf, "%04x %04x %04x", acc[STK831X_AXIS_X], acc[STK831X_AXIS_Y], acc[STK831X_AXIS_Z]);
		if(atomic_read(&obj->trace) & ADX_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
			GSE_LOG("gsensor data:  sensitivity x=%d \n",gsensor_gain.z);
			 
		}
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int STK831X_ReadRawData(struct i2c_client *client, char *buf)
{
	struct stk831x_i2c_data *obj = (struct stk831x_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return EINVAL;
	}

	if(sensor_power == false)
	{
		res = STK831X_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on stk831x error %d!\n", res);
		}
	}
	
	if((res = STK831X_ReadData(client, obj->data)))
	{        
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{
		sprintf(buf, "%04x %04x %04x", obj->data[STK831X_AXIS_X], 
			obj->data[STK831X_AXIS_Y], obj->data[STK831X_AXIS_Z]);
	
	}
	GSE_LOG("gsensor data: %s!\n", buf);
	return 0;
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
#if 0
static int STK831X_InitSelfTest(struct i2c_client *client)
{
	/*	
	int res = 0;
	u8  data;
	u8 databuf[10]; 
    GSE_LOG("fwq init self test\n");

	res = STK831X_SetPowerMode(client,false);
	if(res != STK831X_SUCCESS ) //
	{
		return res;
	}
	
#ifdef SENSOR_STK8312
	res = STK831X_SetDataFormat(client, STK831X_RANGE_6G);
	if(res != STK831X_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
	res = STK831X_SetDataResolution(client, STK831X_RANGE_6G);
#elif defined SENSOR_STK8313
	res = STK831X_SetDataFormat(client, STK831X_RANGE_8G);
	if(res != STK831X_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
	res = STK831X_SetDataResolution(client, STK831X_RANGE_8G);
#endif	
	if(res != STK831X_SUCCESS) 
	{
	    GSE_LOG("fwq stk831x set data reslution error\n");
		return res;
	}


	//set self test reg
	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = STK831X_REG_MODE;//set self test    
	if(hwmsen_read_byte_sr(client, STK831X_REG_MODE, databuf))
	{
		GSE_ERR("read power ctl register err!\n");
		return STK831X_ERR_I2C;
	}

	databuf[0] &=~0x07;//clear original    	
	databuf[0] |= 0x02; //set self test
	
	databuf[1]= databuf[0];
	databuf[0]= STK831X_REG_MODE;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
	    GSE_LOG("fwq set selftest error\n");
		return STK831X_ERR_I2C;
	}
	
	GSE_LOG("fwq init self test OK\n");
	*/
	return STK831X_SUCCESS;
}
#endif
/*----------------------------------------------------------------------------*/
#if 0
static int STK831X_JudgeTestResult(struct i2c_client *client, s32 prv[STK831X_AXES_NUM], s32 nxt[STK831X_AXES_NUM])
{
    struct criteria {
        int min;
        int max;
    };
	
	// TODO
    struct criteria self[6][3] = {
        {{-2, 11}, {-2, 16}, {-20, 105}},
        {{-10, 89}, {0, 125}, {0, 819}},
        {{12, 135}, {-135, -12}, {19, 219}},            
        {{ 6,  67}, {-67,  -6},  {10, 110}},
        {{ 6,  67}, {-67,  -6},  {10, 110}},
        {{ 50,  540}, {-540,  -50},  {75, 875}},
    };
    struct criteria (*ptr)[3] = NULL;
    u8 detectRage;
	u8 tmp_resolution;
    int res;
	GSE_LOG("fwq judge test result\n");
    if(res = hwmsen_read_byte_sr(client, STK831X_REG_XYZ_DATA_CFG, &detectRage))
        return res;

#ifdef SENSOR_STK8312
	switch((detectRage&0xc0)>>6)
	{
		case 0x00:
		tmp_resolution = STK831X_6BIT_RES ;
		break;
		case 0x01:
		tmp_resolution = STK831X_8BIT_RES ;		
		break;
		case 0x02:
		tmp_resolution = STK831X_8BIT_RES ;		
		break;
		default:
		tmp_resolution = STK831X_8BIT_RES ;
		GSE_LOG("fwq judge test detectRage read error !!! \n");
		break;
	}
	GSE_LOG("fwq tmp_resolution=%x , detectRage=%x\n",tmp_resolution,detectRage);
	detectRage = detectRage >>6;

	if(detectRage==STK831X_6BIT_RES)//8 bit resolution
	ptr = &self[0];
	else if(detectRage==STK831X_8BIT_RES)//8 bit resolution
	ptr = &self[1];

	
#elif defined SENSOR_STK8313
	switch((detectRage&0xc0)>>6)
	{
		case 0x00:
		tmp_resolution = STK831X_10BIT_RES ;
		break;
		case 0x01:
		tmp_resolution = STK831X_11BIT_RES ;		
		break;
		case 0x02:
		tmp_resolution = STK831X_12BIT_RES ;		
		break;
		case 0x03:
		tmp_resolution = STK831X_12BIT_RES ;		
		break;
		default:
		tmp_resolution = STK831X_10BIT_RES ;
		GSE_LOG("fwq judge test detectRage read error !!! \n");
		break;
	}
	GSE_LOG("fwq tmp_resolution=%x , detectRage=%x\n",tmp_resolution,detectRage);
	detectRage = detectRage >>6;
	if((tmp_resolution&STK831X_12BIT_RES) && (detectRage==0x00))
	ptr = &self[0];
	else if((tmp_resolution&STK831X_12BIT_RES) && (detectRage==STK831X_RANGE_4G))
	{
		ptr = &self[1];
		GSE_LOG("fwq self test choose ptr1\n");
	}
	else if((tmp_resolution&STK831X_12BIT_RES) && (detectRage==STK831X_RANGE_8G))
	ptr = &self[2];
	else if(detectRage==STK831X_RANGE_2G)//8 bit resolution
	ptr = &self[3];
	else if(detectRage==STK831X_RANGE_4G)//8 bit resolution
	ptr = &self[4];
	else if(detectRage==STK831X_RANGE_8G)//8 bit resolution
	ptr = &self[5];
	
#endif

    if (!ptr) {
        GSE_ERR("null pointer\n");
		GSE_LOG("fwq ptr null\n");
        return -EINVAL;
    }

	/*
    if (((nxt[STK831X_AXIS_X] - prv[STK831X_AXIS_X]) > (*ptr)[STK831X_AXIS_X].max) ||
        ((nxt[STK831X_AXIS_X] - prv[STK831X_AXIS_X]) < (*ptr)[STK831X_AXIS_X].min)) {
        GSE_ERR("X is over range\n");
        res = -EINVAL;
    }
    if (((nxt[STK831X_AXIS_Y] - prv[STK831X_AXIS_Y]) > (*ptr)[STK831X_AXIS_Y].max) ||
        ((nxt[STK831X_AXIS_Y] - prv[STK831X_AXIS_Y]) < (*ptr)[STK831X_AXIS_Y].min)) {
        GSE_ERR("Y is over range\n");
        res = -EINVAL;
    }
    if (((nxt[STK831X_AXIS_Z] - prv[STK831X_AXIS_Z]) > (*ptr)[STK831X_AXIS_Z].max) ||
        ((nxt[STK831X_AXIS_Z] - prv[STK831X_AXIS_Z]) < (*ptr)[STK831X_AXIS_Z].min)) {
        GSE_ERR("Z is over range\n");
        res = -EINVAL;
    }
	*/
    return res;
}
#endif
/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = stk831x_i2c_client;
	char strbuf[STK831X_BUFSIZE];
    GSE_LOG("fwq show_chipinfo_value \n");
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	
	STK831X_ReadChipInfo(client, strbuf, STK831X_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}
/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = stk831x_i2c_client;
	char strbuf[STK831X_BUFSIZE];
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	STK831X_ReadSensorData(client, strbuf, STK831X_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);            
}
/*----------------------------------------------------------------------------*/
static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = stk831x_i2c_client;
	struct stk831x_i2c_data *obj;
	int err, len = 0, mul;
	int tmp[STK831X_AXES_NUM];
	int status = atomic_read(&cali_status);
    GSE_LOG("fwq show_cali_value \n");

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);

	len += snprintf(buf+len, PAGE_SIZE-len, "%02x\n", status);

	if((err = STK831X_ReadOffset(client, obj->offset)))
	{
	//	return -EINVAL;
		len = -EINVAL;
	}
	else if((err = STK831X_ReadCalibration(client, tmp)))
	{
	//	return -EINVAL;
		len = -EINVAL;
	}
	else
	{    
		mul = obj->reso->sensitivity/stk831x_offset_resolution.sensitivity;
		len += snprintf(buf+len, PAGE_SIZE-len, "[HW ][%d] (%+3d, %+3d, %+3d) : (0x%02X, 0x%02X, 0x%02X)\n", mul,                        
			obj->offset[STK831X_AXIS_X], obj->offset[STK831X_AXIS_Y], obj->offset[STK831X_AXIS_Z],
			obj->offset[STK831X_AXIS_X], obj->offset[STK831X_AXIS_Y], obj->offset[STK831X_AXIS_Z]);
		len += snprintf(buf+len, PAGE_SIZE-len, "[SW ][%d] (%+3d, %+3d, %+3d)\n", 1, 
			obj->cali_sw[STK831X_AXIS_X], obj->cali_sw[STK831X_AXIS_Y], obj->cali_sw[STK831X_AXIS_Z]);

		len += snprintf(buf+len, PAGE_SIZE-len, "[ALL]    (%+3d, %+3d, %+3d) : (%+3d, %+3d, %+3d)\n", 
			obj->offset[STK831X_AXIS_X]*mul + obj->cali_sw[STK831X_AXIS_X],
			obj->offset[STK831X_AXIS_Y]*mul + obj->cali_sw[STK831X_AXIS_Y],
			obj->offset[STK831X_AXIS_Z]*mul + obj->cali_sw[STK831X_AXIS_Z],
			tmp[STK831X_AXIS_X], tmp[STK831X_AXIS_Y], tmp[STK831X_AXIS_Z]);
		
	//	return len;
    }
	
	if(status != STK_K_RUNNING)
		STK831x_GetCali(client);
	
	return len;
}
/*----------------------------------------------------------------------------*/
static ssize_t store_cali_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct i2c_client *client = stk831x_i2c_client;  
	int err, x, y, z, sstate;
	int dat[STK831X_AXES_NUM];

	if(!strncmp(buf, "rst", 3))
	{
		if((err = STK831X_ResetCalibration(client)))
		{
			GSE_ERR("reset offset err = %d\n", err);
		}	
	}
	else if(3 == sscanf(buf, "0x%02X 0x%02X 0x%02X", &x, &y, &z))
	{
		dat[STK831X_AXIS_X] = x;
		dat[STK831X_AXIS_Y] = y;
		dat[STK831X_AXIS_Z] = z;
		if((err = STK831X_WriteCalibration(client, dat)))
		{
			GSE_ERR("write calibration err = %d\n", err);
		}		
	}
	else if(1 == sscanf(buf, "%d", &sstate))
	{
		printk(KERN_INFO "%s: sstate = %d\n", __func__, sstate);
		STK831x_SetCali(client, sstate);
	}
	else
	{
		GSE_ERR("invalid format\n");
	}
	
	return count;
}
/*----------------------------------------------------------------------------*/
#if 0
static ssize_t show_selftest_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = stk831x_i2c_client;
	struct stk831x_i2c_data *obj;
	int result =0;
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	GSE_LOG("fwq  selftestRes value =%s\n",selftestRes); 
	return snprintf(buf, 10, "%s\n", selftestRes);
}

/*----------------------------------------------------------------------------*/

static ssize_t store_selftest_value(struct device_driver *ddri, char *buf, size_t count)
{   /*write anything to this register will trigger the process*/
	struct item{
	s16 raw[STK831X_AXES_NUM];
	};
	
	struct i2c_client *client = stk831x_i2c_client;  
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int idx, res, num;
	struct item *prv = NULL, *nxt = NULL;
	s32 avg_prv[STK831X_AXES_NUM] = {0, 0, 0};
	s32 avg_nxt[STK831X_AXES_NUM] = {0, 0, 0};
    u8 databuf[10];

	if(1 != sscanf(buf, "%d", &num))
	{
		GSE_ERR("parse number fail\n");
		return count;
	}
	else if(num == 0)
	{
		GSE_ERR("invalid data count\n");
		return count;
	}

	prv = kzalloc(sizeof(*prv) * num, GFP_KERNEL);
	nxt = kzalloc(sizeof(*nxt) * num, GFP_KERNEL);
	if (!prv || !nxt)
	{
		goto exit;
	}

	res = STK831X_SetPowerMode(client,true);
	if(res != STK831X_SUCCESS ) //
	{
		return res;
	}

	GSE_LOG("NORMAL:\n");
	for(idx = 0; idx < num; idx++)
	{
		if(res = STK831X_ReadData(client, prv[idx].raw))
		{            
			GSE_ERR("read data fail: %d\n", res);
			goto exit;
		}
		
		avg_prv[STK831X_AXIS_X] += prv[idx].raw[STK831X_AXIS_X];
		avg_prv[STK831X_AXIS_Y] += prv[idx].raw[STK831X_AXIS_Y];
		avg_prv[STK831X_AXIS_Z] += prv[idx].raw[STK831X_AXIS_Z];        
		GSE_LOG("[%5d %5d %5d]\n", prv[idx].raw[STK831X_AXIS_X], prv[idx].raw[STK831X_AXIS_Y], prv[idx].raw[STK831X_AXIS_Z]);
	}
	
	avg_prv[STK831X_AXIS_X] /= num;
	avg_prv[STK831X_AXIS_Y] /= num;
	avg_prv[STK831X_AXIS_Z] /= num; 

	res = STK831X_SetPowerMode(client,false);
	if(res != STK831X_SUCCESS ) //
	{
		return res;
	}

	/*initial setting for self test*/
	STK831X_InitSelfTest(client);
	GSE_LOG("SELFTEST:\n");  
/*
	STK831X_ReadData(client, nxt[0].raw);
	GSE_LOG("nxt[0].raw[STK831X_AXIS_X]: %d\n", nxt[0].raw[STK831X_AXIS_X]);
	GSE_LOG("nxt[0].raw[STK831X_AXIS_Y]: %d\n", nxt[0].raw[STK831X_AXIS_Y]);
	GSE_LOG("nxt[0].raw[STK831X_AXIS_Z]: %d\n", nxt[0].raw[STK831X_AXIS_Z]);
	*/
	for(idx = 0; idx < num; idx++)
	{
		if(res = STK831X_ReadData(client, nxt[idx].raw))
		{            
			GSE_ERR("read data fail: %d\n", res);
			goto exit;
		}
		avg_nxt[STK831X_AXIS_X] += nxt[idx].raw[STK831X_AXIS_X];
		avg_nxt[STK831X_AXIS_Y] += nxt[idx].raw[STK831X_AXIS_Y];
		avg_nxt[STK831X_AXIS_Z] += nxt[idx].raw[STK831X_AXIS_Z];        
		GSE_LOG("[%5d %5d %5d]\n", nxt[idx].raw[STK831X_AXIS_X], nxt[idx].raw[STK831X_AXIS_Y], nxt[idx].raw[STK831X_AXIS_Z]);
	}

	//softrestet

	
	// 
	res = STK831X_Init(client, 0);

	avg_nxt[STK831X_AXIS_X] /= num;
	avg_nxt[STK831X_AXIS_Y] /= num;
	avg_nxt[STK831X_AXIS_Z] /= num;    

	GSE_LOG("X: %5d - %5d = %5d \n", avg_nxt[STK831X_AXIS_X], avg_prv[STK831X_AXIS_X], avg_nxt[STK831X_AXIS_X] - avg_prv[STK831X_AXIS_X]);
	GSE_LOG("Y: %5d - %5d = %5d \n", avg_nxt[STK831X_AXIS_Y], avg_prv[STK831X_AXIS_Y], avg_nxt[STK831X_AXIS_Y] - avg_prv[STK831X_AXIS_Y]);
	GSE_LOG("Z: %5d - %5d = %5d \n", avg_nxt[STK831X_AXIS_Z], avg_prv[STK831X_AXIS_Z], avg_nxt[STK831X_AXIS_Z] - avg_prv[STK831X_AXIS_Z]); 

	if(!STK831X_JudgeTestResult(client, avg_prv, avg_nxt))
	{
		GSE_LOG("SELFTEST : PASS\n");
		atomic_set(&obj->selftest, 1); 
		strcpy(selftestRes,"y");
		
	}	
	else
	{
		GSE_LOG("SELFTEST : FAIL\n");
		atomic_set(&obj->selftest, 0);
		strcpy(selftestRes,"n");
	}
	
	exit:
	/*restore the setting*/    
	res = STK831X_Init(client, 0);	
	kfree(prv);
	kfree(nxt);
	return count;
}
#else
/*----------------------------------------------------------------------------*/
static ssize_t show_selftest_value(struct device_driver *ddri, char *buf)
{

	return 0;
}
/*----------------------------------------------------------------------------*/
static ssize_t store_selftest_value(struct device_driver *ddri, const char *buf, size_t count)
{   /*write anything to this register will trigger the process*/
	return 0;
}
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static ssize_t show_firlen_value(struct device_driver *ddri, char *buf)
{
#ifdef CONFIG_STK831X_LOWPASS
	struct i2c_client *client = stk831x_i2c_client;
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	if(atomic_read(&obj->firlen))
	{
		int idx, len = atomic_read(&obj->firlen);
		GSE_LOG("len = %2d, idx = %2d\n", obj->fir.num, obj->fir.idx);
		
		for(idx = 0; idx < len; idx++)
		{
			GSE_LOG("[%5d %5d %5d]\n", obj->fir.raw[idx][STK831X_AXIS_X], obj->fir.raw[idx][STK831X_AXIS_Y], obj->fir.raw[idx][STK831X_AXIS_Z]);
		}
		
		GSE_LOG("sum = [%5d %5d %5d]\n", obj->fir.sum[STK831X_AXIS_X], obj->fir.sum[STK831X_AXIS_Y], obj->fir.sum[STK831X_AXIS_Z]);
		GSE_LOG("avg = [%5d %5d %5d]\n", obj->fir.sum[STK831X_AXIS_X]/len, obj->fir.sum[STK831X_AXIS_Y]/len, obj->fir.sum[STK831X_AXIS_Z]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->firlen));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}
/*----------------------------------------------------------------------------*/
static ssize_t store_firlen_value(struct device_driver *ddri, const char *buf, size_t count)
{
#ifdef CONFIG_STK831X_LOWPASS
	struct i2c_client *client = stk831x_i2c_client;  
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int firlen = 0;
	
	if(1 != sscanf(buf, "%d", &firlen))
	{
		GSE_ERR("invallid format\n");
	}
	else if(firlen > C_MAX_FIR_LENGTH)
	{
		GSE_ERR("exceeds maximum filter length\n");
	}
	else
	{ 
		atomic_set(&obj->firlen, firlen);
		if(0 == firlen)
		{
			atomic_set(&obj->fir_en, 0);
		}
		else
		{
			memset(&obj->fir, 0x00, sizeof(obj->fir));
			atomic_set(&obj->fir_en, 1);
		}
	}
#endif    
return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct stk831x_i2c_data *obj = obj_i2c_data;
    GSE_LOG("fwq show_trace_value \n");
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct stk831x_i2c_data *obj = obj_i2c_data;
	int trace;
    GSE_LOG("fwq store_trace_value \n");
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}	
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;    
	struct stk831x_i2c_data *obj = obj_i2c_data;
    GSE_LOG("fwq show_status_value \n");
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	
	
	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n", 
	            obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol); 
		len += snprintf(buf+len, PAGE_SIZE-len, "VER.: %s\n", STK831X_DRIVER_VERSION);
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	return len;    
}
/*----------------------------------------------------------------------------*/
static ssize_t stk831x_show_allreg(struct device_driver *ddri, char *buf)
{
	int error;	
	char buffer[16];
	char show_buffer[14];
	int aa,bb, no, show_no = 0;
	struct stk831x_i2c_data *obj = obj_i2c_data;
	
	memset(buffer, 0, sizeof(buffer));
	memset(show_buffer, 0, sizeof(show_buffer));
	
	printk(KERN_INFO "%s in\n", __func__);
	
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	
	
	for(bb=0;bb<4;bb++)
	{
		buffer[0] = bb * 0x10;
		printk(KERN_INFO "%s read (%d) buffer[0] = 0x%x\n", __func__, buffer, buffer[0]);
		error = STK_i2c_Rx(buffer, 8);	
		if (error < 0) 
		{
			printk(KERN_ERR "%s:failed\n", __func__);
			return error;
		}
		
		buffer[8] = bb * 0x10 + 8;
		printk(KERN_INFO "%s read (%d) buffer[8] = 0x%x\n", __func__, (buffer+8), *(buffer+8));
		error = STK_i2c_Rx( (buffer+8), 8);	
		if (error < 0) 
		{
			GSE_LOG(KERN_ERR "%s:failed\n", __func__);
			return error;
		}
		for(aa=0;aa<16;aa++)
		{
			no = bb*0x10+aa;
			printk(KERN_INFO "stk reg[0x%x]=0x%x\n", no, buffer[aa]);
			switch(no)
			{
			case 0x0:	
			case 0x1:	
			case 0x2:	
			case 0x3:	
			case 0x4:	
			case 0x5:	
			case STK831X_REG_INT:	
			case STK831X_REG_MODE:	
			case STK831X_REG_SR:	
			case STK831X_REG_OFSX:	
			case STK831X_REG_OFSY:	
			case STK831X_REG_OFSZ:	
			case STK831X_REG_XYZ_DATA_CFG:	
			case 0x24:	
				show_buffer[show_no] = buffer[aa];
				show_no++;
				break;
			default:
				break;
			}
		}
	}	
	return scnprintf(buf, PAGE_SIZE,  "0x0=%02x,0x1=%02x,0x2=%02x,0x3=%02x,0x4=%02x,0x5=%02x,INTSU=%02x,MODE=%02x,SR=%02x,OFSX=%02x,OFSY=%02x,OFSZ=%02x,STH=%02x,0x24=%02x\n", 
		show_buffer[0], show_buffer[1], show_buffer[2], show_buffer[3], show_buffer[4], 
		show_buffer[5], show_buffer[6], show_buffer[7], show_buffer[8], show_buffer[9], 
		show_buffer[10], show_buffer[11], show_buffer[12], show_buffer[13]);
}
/*----------------------------------------------------------------------------*/
static ssize_t stk831x_show_send(struct device_driver *ddri, char *buf)
{
    return 0;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk831x_store_send(struct device_driver *ddri, const char *buf, size_t count)
{
	struct i2c_client *client = stk831x_i2c_client;  
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int addr, cmd;
	u8 dat;
	u8 databuf[2];  

	if (obj == NULL)
	{
		GSE_LOG("i2c_data obj is null!!\n");
		return 0;
	}	
	else if(2 != sscanf(buf, "%x %x", &addr, &cmd))
	{
		GSE_LOG("invalid format: '%s'\n", buf);
		return 0;
	}

	dat = (u8)cmd;
	databuf[0] = addr;
	databuf[1] = dat;
	GSE_LOG("send(%02X, %02X) = %d\n", addr, cmd, 
	i2c_master_send(client, databuf, 0x2));
	
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t stk831x_show_recv(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = stk831x_i2c_client;  
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	if (obj == NULL)
	{
		GSE_LOG("i2c_data obj is null!!\n");
		return 0;
	}	
	return scnprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->recv_reg));     	
}
/*----------------------------------------------------------------------------*/
static ssize_t stk831x_store_recv(struct device_driver *ddri, const char *buf, size_t count)
{
	struct i2c_client *client = stk831x_i2c_client;  
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);
	int addr;
	u8 dat;
	if (obj == NULL)
	{
		GSE_LOG("i2c_data obj is null!!\n");
		return 0;
	}	
	else if(1 != sscanf(buf, "%x", &addr))
	{
		GSE_LOG("invalid format: '%s'\n", buf);
		return 0;
	}

	GSE_LOG("recv(%02X) = %d, 0x%02X\n", addr, 
	hwmsen_read_byte_sr(client, (u8)addr, &dat), dat);
	atomic_set(&obj->recv_reg, dat);	
	return count;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,             S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata,           S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(cali,       S_IWUSR | S_IRUGO, show_cali_value,          store_cali_value);
static DRIVER_ATTR(selftest,       S_IWUSR | S_IRUGO, show_selftest_value,          store_selftest_value);
static DRIVER_ATTR(firlen,     S_IWUSR | S_IRUGO, show_firlen_value,        store_firlen_value);
static DRIVER_ATTR(trace,      S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,               S_IRUGO, show_status_value,        NULL);
static DRIVER_ATTR(allreg,               S_IRUGO, stk831x_show_allreg,        NULL);
static DRIVER_ATTR(send,     S_IWUSR | S_IRUGO, stk831x_show_send,        stk831x_store_send);
static DRIVER_ATTR(recv,     S_IWUSR | S_IRUGO, stk831x_show_recv,        stk831x_store_recv);
/*----------------------------------------------------------------------------*/
static struct driver_attribute *stk831x_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
	&driver_attr_selftest,         /*self test demo*/
	&driver_attr_firlen,       /*filter length: 0: disable, others: enable*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,        
	&driver_attr_allreg,        
	&driver_attr_send,        
	&driver_attr_recv,  
};
/*----------------------------------------------------------------------------*/
static int stk831x_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(stk831x_attr_list)/sizeof(stk831x_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, stk831x_attr_list[idx])))
		{            
			GSE_ERR("driver_create_file (%s) = %d\n", stk831x_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int stk831x_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(stk831x_attr_list)/sizeof(stk831x_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, stk831x_attr_list[idx]);
	}
	

	return err;
}

/*----------------------------------------------------------------------------*/
static int gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	u8 sample_delay;	
	struct stk831x_i2c_data *priv = (struct stk831x_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[STK831X_BUFSIZE];
	
	//GSE_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
		
		#ifdef STK831X_HOLD_ODR
			printk(KERN_INFO "%s HOLD ODR = %d\n", __func__, STK831X_INIT_ODR);
			break;
		#endif
		
			//GSE_LOG("fwq set delay\n");
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = (*(int *)buff_in) * 1000;
				/*
				if(value <= STK831X_SAMPLE_TIME[0])
				{
					sample_delay = 0x0;
				}
				else if(value <= STK831X_SAMPLE_TIME[1])
				{
					sample_delay = 0x1;
				}
				*/
				if(value <= STK831X_SAMPLE_TIME[2])
				{
					sample_delay = 0x2;
				}
				else if(value <= STK831X_SAMPLE_TIME[3])
				{
					sample_delay = 0x3;
				}
				else if(value <= STK831X_SAMPLE_TIME[4])
				{
					sample_delay = 0x4;
				}				
				else
				{
					sample_delay = 0x5;
				}
				
				STK831X_SetDelay(priv->client, sample_delay);
				
				//err = MMA8452Q_SetBWRate(priv->client, MMA8452Q_BW_100HZ); //err = MMA8452Q_SetBWRate(priv->client, sample_delay);
				if(err != STK831X_SUCCESS ) //0x2C->BW=100Hz
				{
					GSE_ERR("Set delay parameter error!\n");
				}

				if(value >= 50000)
				{
					atomic_set(&priv->filter, 0);
				}
				else
				{					
#if defined(CONFIG_STK831X_LOWPASS)
					priv->fir.num = 0;
					priv->fir.idx = 0;
					priv->fir.sum[STK831X_AXIS_X] = 0;
					priv->fir.sum[STK831X_AXIS_Y] = 0;
					priv->fir.sum[STK831X_AXIS_Z] = 0;
					atomic_set(&priv->filter, 1);
#endif				
				}
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				GSE_LOG("fwq sensor enable gsensor, enable=%d\n", value);
				if(((value == 0) && (sensor_power == false)) ||((value == 1) && (sensor_power == true)))
				{
					GSE_LOG("Gsensor device have updated!\n");
				}
				else
				{
					err = STK831X_SetPowerMode( priv->client, !sensor_power);
				}
			}
			break;

		case SENSOR_GET_DATA:
			//GSE_LOG("fwq sensor operate get data\n");
			err = STK831X_ReadSensorData(priv->client, buff, STK831X_BUFSIZE);
			
			if(atomic_read(&priv->event_since_en) < 1200)
				atomic_add(1, &priv->event_since_en);	
			
			if(atomic_read(&priv->event_since_en) < atomic_read(&priv->event_since_en_limit))
				return 0;	

			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				gsensor_data = (hwm_sensor_data *)buff_out;
				if (0 == err)
				{
				sscanf(buff, "%x %x %x", &gsensor_data->values[0], 
					&gsensor_data->values[1], &gsensor_data->values[2]);				
				gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
				gsensor_data->value_divide = 1000;
					
				//GSE_LOG("X :%d,Y: %d, Z: %d\n",gsensor_data->values[0],gsensor_data->values[1],gsensor_data->values[2]);
			}
				else
					return STK831X_ERR_STATUS;
			}
			break;
		default:
			GSE_ERR("gsensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int stk831x_open(struct inode *inode, struct file *file)
{
	file->private_data = stk831x_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int stk831x_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,36))	
static long stk831x_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int stk831x_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif	   
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct stk831x_i2c_data *obj = (struct stk831x_i2c_data*)i2c_get_clientdata(client);	
	char strbuf[STK831X_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,36))	
	long err = 0;
#else
	int err = 0;
#endif
	int cali[3];

	//GSE_FUN(f);
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case GSENSOR_IOCTL_INIT:
			//GSE_LOG("fwq GSENSOR_IOCTL_INIT\n");
			err = STK831X_Init(client, 0);			
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			//GSE_LOG("fwq GSENSOR_IOCTL_READ_CHIPINFO\n");
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			STK831X_ReadChipInfo(client, strbuf, STK831X_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}				 
			break;	  

		case GSENSOR_IOCTL_READ_SENSORDATA:
			//GSE_LOG("fwq GSENSOR_IOCTL_READ_SENSORDATA\n");
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			STK831X_ReadSensorData(client, strbuf, STK831X_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}				 
			break;

		case GSENSOR_IOCTL_READ_GAIN:
			//GSE_LOG("fwq GSENSOR_IOCTL_READ_GAIN\n");
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &gsensor_gain, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

		case GSENSOR_IOCTL_READ_OFFSET:
			//GSE_LOG("fwq GSENSOR_IOCTL_READ_OFFSET\n");
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			if(copy_to_user(data, &gsensor_offset, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

		case GSENSOR_IOCTL_READ_RAW_DATA:
			//GSE_LOG("fwq GSENSOR_IOCTL_READ_RAW_DATA\n");
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			STK831X_ReadRawData(client, strbuf);
			if(copy_to_user(data, &strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}
			break;	  

		case GSENSOR_IOCTL_SET_CALI:
			//GSE_LOG("fwq GSENSOR_IOCTL_SET_CALI!!\n");
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;	  
			}
			if(atomic_read(&obj->suspend))
			{
				GSE_ERR("Perform calibration in suspend state!!\n");
				err = -EINVAL;
			}
			else
			{   
			    GSE_LOG("fwq going to set cali\n");
				cali[STK831X_AXIS_X] = sensor_data.x * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[STK831X_AXIS_Y] = sensor_data.y * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[STK831X_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;			  
				err = STK831X_WriteCalibration(client, cali);			 
			}
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			//GSE_LOG("fwq GSENSOR_IOCTL_CLR_CALI!!\n");
			err = STK831X_ResetCalibration(client);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			//GSE_LOG("fwq GSENSOR_IOCTL_GET_CALI\n");
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if((err = STK831X_ReadCalibration(client, cali)))
			{
				break;
			}
			
			sensor_data.x = cali[STK831X_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.y = cali[STK831X_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.z = cali[STK831X_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}		
			break;

		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}
/*----------------------------------------------------------------------------*/
#ifdef STK_PERMISSION_THREAD
static struct task_struct *STKPermissionThread = NULL;

static int stk_permission_thread(void *data)
{
	int ret = 0;
	int retry = 0;
	mm_segment_t fs = get_fs();
	set_fs(KERNEL_DS);	
	msleep(10000);
	do{
		msleep(5000);
		ret = sys_fchmodat(AT_FDCWD, "/sys/devices/platform/gsensor/driver/cali" , 0666);
		ret = sys_fchmodat(AT_FDCWD, "/sys/devices/platform/gsensor/driver/recv" , 0666);
		ret = sys_fchmodat(AT_FDCWD, "/sys/devices/platform/gsensor/driver/send" , 0666);
		ret = sys_chmod(STK_ACC_CALI_FILE , 0666);	
		ret = sys_fchmodat(AT_FDCWD, STK_ACC_CALI_FILE , 0666);
		//if(ret < 0)
		//	printk("fail to execute sys_fchmodat, ret = %d\n", ret);
		if(retry++ > 10)
			break;
	}while(ret == -ENOENT);
	set_fs(fs);
	printk(KERN_INFO "%s exit, retry=%d\n", __func__, retry);
	return 0;
}
#endif	/*	#ifdef STK_PERMISSION_THREAD	*/

static int stk_store_in_file(char offset[], char mode)
{
	struct file  *cali_file;
	char r_buf[STK_ACC_CALI_FILE_SIZE] = {0};
	char w_buf[STK_ACC_CALI_FILE_SIZE] = {0};	
	mm_segment_t fs;	
	ssize_t ret;
	int8_t i;
	
	w_buf[0] = STK_ACC_CALI_VER0;
	w_buf[1] = STK_ACC_CALI_VER1;
	w_buf[2] = offset[0];
	w_buf[3] = offset[1];
	w_buf[4] = offset[2];
	w_buf[5] = mode;
	
    cali_file = filp_open(STK_ACC_CALI_FILE, O_CREAT | O_RDWR,0666);
	
    if(IS_ERR(cali_file))
	{
        printk(KERN_ERR "%s: filp_open error!\n", __func__);
        return -STK_K_FAIL_OPEN_FILE;
	}
	else
	{
		fs = get_fs();
		set_fs(get_ds());
		
		ret = cali_file->f_op->write(cali_file,w_buf,STK_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret != STK_ACC_CALI_FILE_SIZE)
		{
			printk(KERN_ERR "%s: write error!\n", __func__);
			filp_close(cali_file,NULL);
			return -STK_K_FAIL_W_FILE;
		}
		cali_file->f_pos=0x00;
		ret = cali_file->f_op->read(cali_file,r_buf, STK_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret < 0)
		{
			printk(KERN_ERR "%s: read error!\n", __func__);
			filp_close(cali_file,NULL);
			return -STK_K_FAIL_R_BACK;
		}		
		set_fs(fs);
		
		//printk(KERN_INFO "%s: read ret=%d!\n", __func__, ret);
		for(i=0;i<STK_ACC_CALI_FILE_SIZE;i++)
		{
			if(r_buf[i] != w_buf[i])
			{
				printk(KERN_ERR "%s: read back error, r_buf[%x](0x%x) != w_buf[%x](0x%x)\n", 
					__func__, i, r_buf[i], i, w_buf[i]);				
				filp_close(cali_file,NULL);
				return -STK_K_FAIL_R_BACK_COMP;
			}
		}
    }
    filp_close(cali_file,NULL);	
	
#ifdef STK_PERMISSION_THREAD
	fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sys_chmod(STK_ACC_CALI_FILE , 0666);
	ret = sys_fchmodat(AT_FDCWD, STK_ACC_CALI_FILE , 0666);
	set_fs(fs);		
#endif
	//printk(KERN_INFO "%s successfully\n", __func__);
	return 0;		
}
/*----------------------------------------------------------------------------*/
static struct file_operations stk831x_fops = {
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))		
	.owner = THIS_MODULE,
#endif	
	.open = stk831x_open,
	.release = stk831x_release,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,36))	
	.unlocked_ioctl = stk831x_unlocked_ioctl,
#else
	.ioctl = stk831x_ioctl,
#endif	
};
/*----------------------------------------------------------------------------*/
static struct miscdevice stk831x_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &stk831x_fops,
};
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int stk831x_suspend(struct i2c_client *client, pm_message_t msg) 
{
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);    
	int err = 0;
	u8  dat=0;
	GSE_FUN();    

	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(obj == NULL)
		{
			GSE_ERR("null pointer!!\n");
			return -EINVAL;
		}		
		atomic_set(&obj->suspend, 1); 
		if(err = STK831X_SetPowerMode(obj->client, false))
		{
			GSE_ERR("write power control fail!!\n");
			return;
		}
		
		sensor_power = false;		
		STK831X_power(obj->hw, 0);				
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int stk831x_resume(struct i2c_client *client)
{
	struct stk831x_i2c_data *obj = i2c_get_clientdata(client);        
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	STK831X_power(obj->hw, 1);
	if(err = STK831X_Init(client, 0))
	{
		GSE_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->suspend, 0);

	return 0;
}
/*----------------------------------------------------------------------------*/
#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void stk831x_early_suspend(struct early_suspend *h) 
{
	struct stk831x_i2c_data *obj = container_of(h, struct stk831x_i2c_data, early_drv);   
	int err;
	GSE_FUN();    

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	atomic_set(&obj->suspend, 1); 
	/*
	if(err = hwmsen_write_byte(obj->client, STK831X_REG_POWER_CTL, 0x00))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}  
	*/
	if((err = STK831X_SetPowerMode(obj->client, false)))
	{
		GSE_ERR("write power control fail!!\n");
		return;
	}

	sensor_power = false;	
	STK831X_power(obj->hw, 0);
}
/*----------------------------------------------------------------------------*/
static void stk831x_late_resume(struct early_suspend *h)
{
	struct stk831x_i2c_data *obj = container_of(h, struct stk831x_i2c_data, early_drv);         
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}

	STK831X_power(obj->hw, 1);
	if((err = STK831X_Init(obj->client, 0)))
	{
		GSE_ERR("initialize client fail!!\n");
		return;        
	}
	atomic_set(&obj->suspend, 0);    
}
/*----------------------------------------------------------------------------*/
#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))	
static int stk831x_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
	strcpy(info->type, STK831X_DEV_NAME);
	return 0;
}
#endif
/*----------------------------------------------------------------------------*/
static int stk831x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct stk831x_i2c_data *obj;
	struct hwmsen_object sobj;
	int err = 0;
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	
	memset(obj, 0, sizeof(struct stk831x_i2c_data));

	obj->hw = get_cust_acc_hw();
	
	if((err = hwmsen_get_convert(obj->hw->direction, &obj->cvt)))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
	
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);
	atomic_set(&obj->event_since_en, 0);
	atomic_set(&obj->event_since_en_limit, 20);
	
#ifdef CONFIG_STK831X_LOWPASS
	if(obj->hw->firlen > C_MAX_FIR_LENGTH)
	{
		atomic_set(&obj->firlen, C_MAX_FIR_LENGTH);
	}	
	else
	{
		atomic_set(&obj->firlen, obj->hw->firlen);
	}
	
	if(atomic_read(&obj->firlen) > 0)
	{
		atomic_set(&obj->fir_en, 1);
	}	
#endif
	

	stk831x_i2c_client = new_client;	

	if((err = STK831X_Init(new_client, 1)))
	{
		goto exit_init_failed;
	}
	

	if((err = misc_register(&stk831x_device)))
	{
		GSE_ERR("stk831x_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	if((err = stk831x_create_attr(&stk831x_gsensor_driver.driver)))
	{
		GSE_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

	sobj.self = obj;
    sobj.polling = 1;
    sobj.sensor_operate = gsensor_operate;
	if((err = hwmsen_attach(ID_ACCELEROMETER, &sobj)))
	{
		GSE_ERR("attach fail = %d\n", err);
		goto exit_kfree;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = stk831x_early_suspend,
	obj->early_drv.resume   = stk831x_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif 

	GSE_LOG("%s: OK\n", __func__);    
	return 0;

	exit_create_attr_failed:
	misc_deregister(&stk831x_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);        
	return err;
}

/*----------------------------------------------------------------------------*/
static int stk831x_i2c_remove(struct i2c_client *client)
{
	int err = 0;	
	
	if((err = STK831X_SetPowerMode(client, false)))
	{
		GSE_ERR("write power control fail!!\n");
		return err;
	}
		
	if((err = stk831x_delete_attr(&stk831x_gsensor_driver.driver)))
	{
		GSE_ERR("stk831x_delete_attr fail: %d\n", err);
	}
	
	if((err = misc_deregister(&stk831x_device)))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}

	if((err = hwmsen_detach(ID_ACCELEROMETER)))
	    

	stk831x_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int stk831x_probe(struct platform_device *pdev) 
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();
	printk("%s: driver version=%s\n", __func__, STK831X_DRIVER_VERSION);
	
	STK831X_power(hw, 1);
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))		
	#ifdef SENSOR_STK8312
		stk8312_force[0] = hw->i2c_num;
	#elif defined SENSOR_STK8313
		stk8313_force[0] = hw->i2c_num;
	#endif
#endif	/* #if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0)) */
	if(i2c_add_driver(&stk831x_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int stk831x_remove(struct platform_device *pdev)
{
    struct acc_hw *hw = get_cust_acc_hw();

    GSE_FUN();    
    STK831X_power(hw, 0);    
    i2c_del_driver(&stk831x_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver stk831x_gsensor_driver = {
	.probe      = stk831x_probe,
	.remove     = stk831x_remove,    
	.driver     = {
		.name  = "gsensor",
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))			
		.owner = THIS_MODULE,
#endif		
	}
};

/*----------------------------------------------------------------------------*/
static int __init stk831x_init(void)
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();
	printk("%s: i2c_number=%d\n", __func__, hw->i2c_num);
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,0,0)) 		
	i2c_register_board_info( hw->i2c_num, &i2c_stk831x, 1);
#endif
	if(platform_driver_register(&stk831x_gsensor_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
	
#ifdef STK_PERMISSION_THREAD
	STKPermissionThread = kthread_run(stk_permission_thread,"stk","Permissionthread");
	if(IS_ERR(STKPermissionThread))
		STKPermissionThread = NULL;
#endif // STK_PERMISSION_THREAD	
	printk("======stk831x_init END======\n");
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
static void __exit stk831x_exit(void)
{
	GSE_FUN();
	platform_driver_unregister(&stk831x_gsensor_driver);
#ifdef STK_PERMISSION_THREAD
	if(STKPermissionThread)
		STKPermissionThread = NULL;
#endif // STK_PERMISSION_THREAD	
}
/*----------------------------------------------------------------------------*/
module_init(stk831x_init);
module_exit(stk831x_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("STK831X I2C driver");
MODULE_AUTHOR("Zhilin.Chen@mediatek.com");
