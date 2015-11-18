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

/* drivers/hwmon/mt6516/amit/ltr559.c - LTR559 ALS/PS driver
 *
 * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
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

/*
 * Change log
 *
 * Date		Author		Description
 * ----------------------------------------------------------------------------
 * 11/17/2011	chenqy		Initial modification from ltr502.
 * 01/03/2012	chenqy		Fix logical error in sensor enable function.
 *
 */

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
#include <linux/wakelock.h>

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <mach/eint.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_eint_md1.h>
#include <cust_alsps.h>
#include "ltr559.h"
#include <linux/hwmsen_helper.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#define POWER_NONE_MACRO -1

#ifdef CONFIG_GN_DEVICE_CHECK
#include <linux/gn_device_check.h>
#endif

#define POWER_NONE_MACRO MT65XX_POWER_NONE



/******************************************************************************
 * configuration
*******************************************************************************/
#define I2C_DRIVERID_LTR559 559
/*----------------------------------------------------------------------------*/
#define LTR559_I2C_ADDR_RAR	0 /*!< the index in obj->hw->i2c_addr: alert response address */
#define LTR559_I2C_ADDR_ALS	1 /*!< the index in obj->hw->i2c_addr: ALS address */
#define LTR559_I2C_ADDR_PS	2 /*!< the index in obj->hw->i2c_addr: PS address */
#define LTR559_DEV_NAME		"LTR559"
/*----------------------------------------------------------------------------*/
#define APS_TAG	"[ALS/PS] "
#define APS_DEBUG
#if defined(APS_DEBUG)
#define APS_FUN(f)		printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)	printk(KERN_ERR APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)	printk(KERN_ERR APS_TAG "%s(%d):" fmt, __FUNCTION__, __LINE__, ##args)
#define APS_DBG(fmt, args...)	printk(KERN_INFO APS_TAG fmt, ##args)
#else
#define APS_FUN(f)
#define APS_ERR(fmt, args...)
#define APS_LOG(fmt, args...)
#define APS_DBG(fmt, args...)
#endif


#define GN_MTK_BSP_ALSPS_INTERRUPT_MODE
/******************************************************************************
 * extern functions
*******************************************************************************/
static struct i2c_client *ltr559_i2c_client = NULL;

//static struct wake_lock ps_wake_lock;
//static int ps_wakeup_timeout = 3;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id ltr559_i2c_id[] = {{LTR559_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_ltr559={ I2C_BOARD_INFO(LTR559_DEV_NAME, (LTR559_I2C_SLAVE_ADDR>>1))};

/*----------------------------------------------------------------------------*/
static int ltr559_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int ltr559_i2c_remove(struct i2c_client *client);
static int ltr559_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
static int ltr559_i2c_suspend(struct i2c_client *client, pm_message_t msg);
static int ltr559_i2c_resume(struct i2c_client *client);

static int ltr559_init_flag = -1;
static int  ltr559_local_init(void);
static int  ltr559_remove(void);

static struct sensor_init_info ltr559_init_info = {
		.name = "ltr559",
		.init = ltr559_local_init,
		.uninit = ltr559_remove,
};


static struct ltr559_priv *g_ltr559_ptr = NULL;

/*----------------------------------------------------------------------------*/
typedef enum {
	CMC_TRC_APS_DATA	= 0x0002,
	CMC_TRC_EINT		= 0x0004,
	CMC_TRC_IOCTL		= 0x0008,
	CMC_TRC_I2C		= 0x0010,
	CMC_TRC_CVT_ALS		= 0x0020,
	CMC_TRC_CVT_PS		= 0x0040,
	CMC_TRC_DEBUG		= 0x8000,
} CMC_TRC;
/*----------------------------------------------------------------------------*/
typedef enum {
	CMC_BIT_ALS		= 1,
	CMC_BIT_PS		= 2,
} CMC_BIT;
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
struct ltr559_priv {
	struct alsps_hw *hw;
	struct i2c_client *client;
#ifdef GN_MTK_BSP_ALSPS_INTERRUPT_MODE
	struct delayed_work eint_work;
#endif /* GN_MTK_BSP_ALSPS_INTERRUPT_MODE */
	//struct timer_list first_read_ps_timer;
	//struct timer_list first_read_als_timer;

	/*misc*/
	atomic_t	trace;
	atomic_t	i2c_retry;
	atomic_t	als_suspend;
	atomic_t	als_debounce;	/*debounce time after enabling als*/
	atomic_t	als_deb_on;	/*indicates if the debounce is on*/
	atomic_t	als_deb_end;	/*the jiffies representing the end of debounce*/
	atomic_t	ps_mask;	/*mask ps: always return far away*/
	atomic_t	ps_debounce;	/*debounce time after enabling ps*/
	atomic_t	ps_deb_on;	/*indicates if the debounce is on*/
	atomic_t	ps_deb_end;	/*the jiffies representing the end of debounce*/
	atomic_t	ps_suspend;


	/*data*/
	// u8		als;
	// u8		ps;
	int		als;
	int		ps;
	u8		_align;
	u16		als_level_num;
	u16		als_value_num;
	u32		als_level[C_CUST_ALS_LEVEL-1];
	u32		als_value[C_CUST_ALS_LEVEL];

	bool		als_enable;	/*record current als status*/
	unsigned int	als_widow_loss;

	bool		ps_enable;	 /*record current ps status*/
	bool		ps_cali_valid;  //xmxl@20140526
	unsigned int	ps_thd_val;	 /*the cmd value can't be read, stored in ram*/
	ulong		enable;		 /*record HAL enalbe status*/
	ulong		pending_intr;	/*pending interrupt*/
	//ulong		first_read;	// record first read ps and als
	unsigned int	polling;
	/*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend	early_drv;
#endif
};
//static struct platform_driver ltr559_alsps_driver;
static DEFINE_SEMAPHORE(ltr559_enabling_ps_mutex);//xmxl@20140526
/*----------------------------------------------------------------------------*/
static struct i2c_driver ltr559_i2c_driver = {
	.probe		= ltr559_i2c_probe,
	.remove		= ltr559_i2c_remove,
	.detect		= ltr559_i2c_detect,
	.suspend	= ltr559_i2c_suspend,
	.resume		= ltr559_i2c_resume,
	.id_table	= ltr559_i2c_id,
//	.address_data	= &ltr559_addr_data,
	.driver = {
//		.owner	= THIS_MODULE,
		.name	= LTR559_DEV_NAME,
	},
};

static struct ltr559_priv *ltr559_obj = NULL;

static int ps_gainrange;
static int als_gainrange;

static int final_prox_val;
static int final_lux_val;

/*
 * The ps_trigger_xxx_table
 * controls the interrupt trigger range
 * bigger value means close to p-sensor
 * smaller value means far away from p-sensor
 */
static int ps_trigger_high = 300;				//yaoyaoqin: trigger up threshold
static int ps_trigger_low = 250;				//yaoyaoqin: trigger low threshold
static int ps_trigger_delta = 0x0f;				//yaoyaoqin: delta for adjust threshold

static int ltr559_get_ps_value(struct ltr559_priv *obj, int ps);
static int ltr559_get_als_value(struct ltr559_priv *obj, int als);

/*----------------------------------------------------------------------------*/
static int hwmsen_read_byte_sr(struct i2c_client *client, u8 addr, u8 *data)
{
	u8 buf;
	int ret = 0;
	struct i2c_client client_read = *client;

	client_read.addr = (client->addr & I2C_MASK_FLAG) | I2C_WR_FLAG |I2C_RS_FLAG;
	buf = addr;
	ret = i2c_master_send(&client_read, (const char*)&buf, 1<<8 | 1);
	if (ret < 0) {
		APS_ERR("send command error!!\n");
		return -EFAULT;
	}

	*data = buf;
	client->addr = client->addr& I2C_MASK_FLAG;
	return 0;
}

/*----------------------------------------------------------------------------*/
int ltr559_read_data_als(struct i2c_client *client, int *data)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int ret = 0;
	int alsval_ch1_lo = 0;
	int alsval_ch1_hi = 0;
	int alsval_ch0_lo = 0;
	int alsval_ch0_hi = 0;
	int luxdata_int;
	int luxdata_flt;
	int ratio;
	int alsval_ch0;
	int alsval_ch1;

	if (hwmsen_read_byte_sr(client, APS_RO_ALS_DATA_CH1_0, &alsval_ch1_lo)) {
		APS_ERR("reads als data error (ch1 lo) = %d\n", alsval_ch1_lo);
		return -EFAULT;
	}
	if (hwmsen_read_byte_sr(client, APS_RO_ALS_DATA_CH1_1, &alsval_ch1_hi)) {
		APS_ERR("reads aps data error (ch1 hi) = %d\n", alsval_ch1_hi);
		return -EFAULT;
	}
	alsval_ch1 = (alsval_ch1_hi * 256) + alsval_ch1_lo;
	//APS_DBG("alsval_ch1_hi=%x alsval_ch1_lo=%x\n",alsval_ch1_hi,alsval_ch1_lo);


	if (hwmsen_read_byte_sr(client, APS_RO_ALS_DATA_CH0_0, &alsval_ch0_lo)) {
		APS_ERR("reads als data error (ch0 lo) = %d\n", alsval_ch0_lo);
		return -EFAULT;
	}
	if (hwmsen_read_byte_sr(client, APS_RO_ALS_DATA_CH0_1, &alsval_ch0_hi)) {
		APS_ERR("reads als data error (ch0 hi) = %d\n", alsval_ch0_hi);
		return -EFAULT;
	}
	alsval_ch0 = (alsval_ch0_hi * 256) + alsval_ch0_lo;

	if ((alsval_ch0 + alsval_ch1) == 0) {
		APS_ERR("Both CH0 and CH1 are zero\n");
		ratio = 1000;
	} else {
		ratio = (alsval_ch1 * 1000) / (alsval_ch0 + alsval_ch1);
	}

	if (ratio < 450){
		luxdata_flt = (17743 * alsval_ch0) - (-11059 * alsval_ch1);
		luxdata_flt = luxdata_flt / 10000;
	} else if ((ratio >= 450) && (ratio < 640)){
		luxdata_flt = (37725 * alsval_ch0) - (13363 * alsval_ch1);
		luxdata_flt = luxdata_flt / 10000;
	} else if ((ratio >= 640) && (ratio < 850)){
		luxdata_flt = (16900 * alsval_ch0) - (1690 * alsval_ch1);
		luxdata_flt = luxdata_flt / 10000;
	} else {
		luxdata_flt = 0;
	}

/*
	// For Range1
	if (als_gainrange == ALS_RANGE1_320)
		luxdata_flt = luxdata_flt / 150;
*/
	// convert float to integer;
	luxdata_int = luxdata_flt;
	if ((luxdata_flt - luxdata_int) > 0.5){
		luxdata_int = luxdata_int + 1;
	} else {
		luxdata_int = luxdata_flt;
	}

	if (atomic_read(&obj->trace) & CMC_TRC_APS_DATA) {
		APS_ERR("ALS (CH0): 0x%04X\n", alsval_ch0);
		APS_ERR("ALS (CH1): 0x%04X\n", alsval_ch1);
		APS_ERR("ALS (Ratio): %d\n", ratio);
		APS_ERR("ALS: %d\n", luxdata_int);
	}

	*data = luxdata_int;
	
	//APS_DBG("luxdata_int=%x \n",luxdata_int);
	final_lux_val = luxdata_int;

	return 0;
}

int ltr559_read_data_ps(struct i2c_client *client, int *data)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int ret = 0;
	int psval_lo = 0;
	int psval_hi = 0;
	int psdata = 0;

	if (hwmsen_read_byte_sr(client, APS_RO_PS_DATA_0, &psval_lo)) {
		APS_ERR("reads aps data = %d\n", psval_lo);
		return -EFAULT;
	}

	if (hwmsen_read_byte_sr(client, APS_RO_PS_DATA_1, &psval_hi)) {
		APS_ERR("reads aps hi data = %d\n", psval_hi);
		return -EFAULT;
	}

	psdata = ((psval_hi & 7) * 256) + psval_lo;
	APS_ERR("psensor rawdata is:%d\n", psdata);

	*data = psdata;
	final_prox_val = psdata;
	return 0;
}

/*----------------------------------------------------------------------------*/

int ltr559_init_device(struct i2c_client *client)
{
	//struct ltr559_priv *obj = i2c_get_clientdata(client);
	APS_LOG("ltr559_init_device.........\r\n");
	u8 buf =0;
	int i = 0;
	int ret = 0;

	ret=hwmsen_write_byte(client, 0x82, 0x7F);				//100mA,100%,60kHz		//yaoyaoqin
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
    ret=hwmsen_write_byte(client, 0x83, 0x08);				//4 pulse
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
    ret=hwmsen_write_byte(client, 0x84, 0x00);				//50 ms
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
    ret=hwmsen_write_byte(client, 0x9e, 0x20);				//2 consecutive data outside range to interrupt
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
//#ifdef GN_MTK_BSP_ALSPS_INTERRUPT_MODE
	ret=hwmsen_write_byte(client, 0x8f, 0x01);				//enable ps for interrupt mode
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
    ret=hwmsen_write_byte(client, 0x90, ps_trigger_high & 0xff);
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
    ret=hwmsen_write_byte(client, 0x91, (ps_trigger_high>>8) & 0X07);
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
	ret=hwmsen_write_byte(client, 0x92, 0x00);
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
    ret=hwmsen_write_byte(client, 0x93, 0x00);
    if(ret<0)
    {
        ltr559_init_flag=-1;
        return ltr559_init_flag;
    }
//#endif /* GN_MTK_BSP_ALSPS_INTERRUPT_MODE */

//	hwmsen_write_byte(client, 0x80, 0x03);
//	hwmsen_write_byte(client, 0x81, 0x03);
	
	mdelay(WAKEUP_DELAY);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int ltr559_power(struct alsps_hw *hw, unsigned int on)
{
	static unsigned int power_on = 0;
	int status = 0; 

	APS_LOG("power %s\n", on ? "on" : "off");
	APS_LOG("power id:%d POWER_NONE_MACRO:%d\n", hw->power_id, POWER_NONE_MACRO);

	if(hw->power_id != POWER_NONE_MACRO)
	{
		if(power_on == on)
		{
			APS_LOG("ignore power control: %d\n", on);
			status = 0;
		}
		else if(on)
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "LTR559"))
			{
				APS_ERR("power on fails!!\n");
			}
			status =  1;
		}
		else
		{
			if(!hwPowerDown(hw->power_id, "LTR559"))
			{
				APS_ERR("power off fail!!\n");
			}
			status =  -1;
		}
	}
	power_on = on;

	return status;
}
/*----------------------------------------------------------------------------*/
static int ltr559_enable_als(struct i2c_client *client, bool enable)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int err=0;
	int trc = atomic_read(&obj->trace);
	u8 regdata=0;
	u8 regint=0;

	if(enable == obj->als_enable)
	{
		return 0;
	}
	else if(enable == true)
	{
		if (hwmsen_write_byte(client, APS_RW_ALS_CONTR, 0x01)) {
			APS_LOG("ltr559_enable_als enable failed!\n");
			return -1;
		}
		hwmsen_read_byte_sr(client, APS_RW_ALS_CONTR, &regdata);			//yaoyaoqin: need delay at least 50ms
		APS_LOG("ltr559_enable_als, regdata: 0x%x!\n", regdata);
	}
	else if(enable == false)
	{
		if (hwmsen_write_byte(client, APS_RW_ALS_CONTR, 0x00)) {
			APS_LOG("ltr559_enable_als disable failed!\n");
			return -1;
		}
		hwmsen_read_byte_sr(client, APS_RW_ALS_CONTR, &regdata);
		APS_LOG("ltr559_enable_als, regdata: 0x%x!\n", regdata);
	}

	obj->als_enable = enable;

    /*
	if(trc & CMC_TRC_DEBUG)
	{
		APS_LOG("enable als (%d)\n", enable);
	}
    */
	APS_LOG("enable als (%d)\n", enable);
	return err;
}
static int ltr559_dynamic_calibrate(void)//xmxl@20140506
{
	//int ret = 0;
	int i = 0;
	//int j = 0;
	int data = 0;
	int dataX[5];
	//int noise = 0;
	//int len = 0;
	//int err = 0;
	int max = 0;
	//int min=0;
	//int idx_table = 0;
	//unsigned long data_total = 0;
	struct ltr559_priv *obj = g_ltr559_ptr;


	//APS_FUN(f);
	if (!obj) goto ERROR;

	mdelay(10);
	for (i = 0; i < 4; i++) {
		mdelay(40);
		data=0;
		ltr559_read_data_ps(obj->client,&data);//max data is 0x7FF(11bit)=2047
		if(data>max)
			max=data;
		dataX[i]=data;
	}

	if( (obj->hw->ps_threshold_low) < max)//if max value is more than ps_threshold_low, set threshold to be 1500,1600.
		goto ERROR;

	ps_trigger_high = max+150;//obj->hw->ps_threshold_high;//use defualt threshold
	ps_trigger_low = max+100;//obj->hw->ps_threshold_low;
	obj->ps_cali_valid=true;//xmxl@20140526
	APS_ERR(" ltr559_dynamic_calibrate end: obj->ps_thd_val_low= %d , obj->ps_thd_val_high = %d\n", ps_trigger_low, ps_trigger_high);
	return 0;
ERROR:
	ps_trigger_high = 1600;
	ps_trigger_low =  1500;
	obj->ps_cali_valid=false;//xmxl@20140526
	APS_ERR("ltr559_dynamic_calibrate fail ,use low %d ,high %d!\n",ps_trigger_low,ps_trigger_high);
	return -1;
}
static int ltr559_ps_set_thres_dynamic(void)//xmxl@20140506
{
	u8 databuf[2];
	int res;
	struct ltr559_priv *obj = g_ltr559_ptr;

	databuf[0] = APS_RW_PS_THRES_LOW_0;
	databuf[1] = (u8)(ps_trigger_low & 0x00FF);
	res = i2c_master_send(obj->client, databuf, 0x2);
	if(res <= 0){
		//return -1;
		goto ERROR;
	}

	databuf[0] = APS_RW_PS_THRES_LOW_1;
	databuf[1] = (u8)((ps_trigger_low & 0xFF00) >> 8);
	res = i2c_master_send(obj->client, databuf, 0x2);
	if(res <= 0){
		//return -1;
		goto ERROR;
	}

	databuf[0] = APS_RW_PS_THRES_UP_0;
	databuf[1] = (u8)(ps_trigger_high & 0x00FF);
	res = i2c_master_send(obj->client, databuf, 0x2);
	if(res <= 0){
		//return -1;
		goto ERROR;
	}

	databuf[0] = APS_RW_PS_THRES_UP_1;
	databuf[1] = (u8)((ps_trigger_high & 0xFF00) >> 8);
	res = i2c_master_send(obj->client, databuf, 0x2);
	if(res <= 0){
		//return -1;
		goto ERROR;
	}
	
	APS_ERR("ltr559_ps_set_thres_dynamic, ok !\n");
	return 0;
	
ERROR:
	APS_ERR("ltr559_ps_set_thres_dynamic, fail !\n");
	return res;
}
/*----------------------------------------------------------------------------*/
static int ltr559_enable_ps(struct i2c_client *client, bool enable)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int err=0;
	int trc = atomic_read(&obj->trace);
	u8 regdata = 0;
	u8 regint = 0;

	APS_LOG(" ltr559_enable_ps: enable:  %d, obj->ps_enable: %d\n",enable, obj->ps_enable);
	if (enable == obj->ps_enable)
	{
		return 0;
	}
	else if (enable == true)
	{
		mt_eint_mask(CUST_EINT_ALS_NUM);//xmxl@20140506
		if (hwmsen_write_byte(client, APS_RW_PS_CONTR, 0x03)) {
			APS_LOG("ltr559_enable_ps enable failed!\n");
			return -1;
		}
		hwmsen_read_byte_sr(client, APS_RW_PS_CONTR, &regdata);
		APS_ERR("ltr559_enable_ps, regdata: 0x%x !\n", regdata);
		if(0 == obj->hw->polling_mode_ps){//xmxl@20140506
			ltr559_dynamic_calibrate();
			ltr559_ps_set_thres_dynamic();
			mt_eint_unmask(CUST_EINT_ALS_NUM);
		}
	}
	else if(enable == false)
	{
		if (hwmsen_write_byte(client, APS_RW_PS_CONTR, 0x00)) {
			APS_LOG("ltr559_enable_ps disable failed!\n");
			return -1;
		}
		hwmsen_read_byte_sr(client, APS_RW_PS_CONTR, &regdata);
		APS_ERR("ltr559_enable_ps, regdata: 0x%x!\n", regdata);
	}
	obj->ps_enable = enable;
	if(trc & CMC_TRC_DEBUG)
	{
		APS_LOG("enable ps (%d)\n", enable);
	}

	return err;
}
/*----------------------------------------------------------------------------*/

static int ltr559_check_intr(struct i2c_client *client)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int err;
	u8 data=0;

	// err = hwmsen_read_byte_sr(client,APS_INT_STATUS,&data);
	err = hwmsen_read_byte_sr(client,APS_RO_ALS_PS_STATUS,&data);
	APS_ERR("INT flage: = %x\n", data);

	if (err) {
		APS_ERR("WARNING: read int status: %d\n", err);
		return 0;
	}

	if (data & 0x08) {
		set_bit(CMC_BIT_ALS, &obj->pending_intr);
	} else {
		clear_bit(CMC_BIT_ALS, &obj->pending_intr);
	}

	if (data & 0x02) {
		set_bit(CMC_BIT_PS, &obj->pending_intr);
	} else {
		clear_bit(CMC_BIT_PS, &obj->pending_intr);
	}

	if (atomic_read(&obj->trace) & CMC_TRC_DEBUG) {
		APS_LOG("check intr: 0x%08X\n", obj->pending_intr);
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
#ifdef GN_MTK_BSP_ALSPS_INTERRUPT_MODE
void ltr559_eint_func(void)
{
	struct ltr559_priv *obj = g_ltr559_ptr;
	APS_LOG("fwq interrupt fuc\n");
	if(!obj)
	{
		return;
	}

	schedule_delayed_work(&obj->eint_work,0);
	if(atomic_read(&obj->trace) & CMC_TRC_EINT)
	{
		APS_LOG("eint: als/ps intrs\n");
	}
}
/*----------------------------------------------------------------------------*/
static void ltr559_eint_work(struct work_struct *work)	
{
	struct ltr559_priv *obj = (struct ltr559_priv *)container_of(work, struct ltr559_priv, eint_work);
	int err;
	hwm_sensor_data sensor_data;
	u8 buf;

	APS_ERR("interrupt........");
	memset(&sensor_data, 0, sizeof(sensor_data));

	if (0 == atomic_read(&obj->ps_deb_on)) {
		// first enable do not check interrupt
		err = ltr559_check_intr(obj->client);
	}

	if (err) {
		APS_ERR("check intrs: %d\n", err);
	}

	APS_ERR("ltr559_eint_work obj->pending_intr =%d, obj:%p\n",obj->pending_intr,obj);

	if ((1<<CMC_BIT_ALS) & obj->pending_intr) {
		// get raw data
		APS_ERR("fwq als INT\n");
		if (err = ltr559_read_data_als(obj->client, &obj->als)) {
			APS_ERR("ltr559 read als data: %d\n", err);;
		}
		//map and store data to hwm_sensor_data
		while(-1 == ltr559_get_als_value(obj, obj->als)){
			ltr559_read_data_als(obj->client, &obj->als);
			msleep(50);
		}
 		sensor_data.values[0] = ltr559_get_als_value(obj, obj->als);
		APS_LOG("ltr559_eint_work sensor_data.values[0] =%d\n",sensor_data.values[0]);
		sensor_data.value_divide = 1;
		sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
		// let up layer to know
		if (err = hwmsen_get_interrupt_data(ID_LIGHT, &sensor_data)) {
			APS_ERR("call hwmsen_get_interrupt_data light fail = %d\n", err);
		}
	}
	
	if ((1 << CMC_BIT_PS) & obj->pending_intr) {
		// get raw data
		APS_ERR("fwq ps INT\n");
		if (err = ltr559_read_data_ps(obj->client, &obj->ps)) {
			APS_ERR("ltr559 read ps data: %d\n", err);;
		}
		//map and store data to hwm_sensor_data
		while(-1 == ltr559_get_ps_value(obj, obj->ps)) {
			ltr559_read_data_ps(obj->client, &obj->ps);
			msleep(50);
			APS_ERR("ltr559 read ps data delay\n");;
		}
		sensor_data.values[0] = ltr559_get_ps_value(obj, obj->ps);
		
		if(sensor_data.values[0] == 0)
		{
			hwmsen_write_byte(obj->client,0x90,0xff);
			hwmsen_write_byte(obj->client,0x91,0x07);
			
			hwmsen_write_byte(obj->client,0x92,ps_trigger_low & 0xff);				//yaoyaoqin:low threshold for faraway interrupt only
			hwmsen_write_byte(obj->client,0x93,(ps_trigger_low>>8) & 0X07);		
			//wake_unlock(&ps_wake_lock);
		}
		else if(sensor_data.values[0] == 1)
		{
			//wake_lock_timeout(&ps_wake_lock,ps_wakeup_timeout*HZ);
			hwmsen_write_byte(obj->client,0x90,ps_trigger_high & 0xff);				//yaoyaoqin:high threshold for close interrupt only
			hwmsen_write_byte(obj->client,0x91,(ps_trigger_high>>8) & 0X07);
			
			hwmsen_write_byte(obj->client,0x92,0x00);
			hwmsen_write_byte(obj->client,0x93,0x00);	
		}

		sensor_data.value_divide = 1;
		sensor_data.status = (obj->ps_cali_valid ? SENSOR_STATUS_ACCURACY_MEDIUM : SENSOR_STATUS_ACCURACY_LOW);//xmxl@20140526
		//let up layer to know
		APS_ERR("ltr559 read ps data = %d,status=%d \n",sensor_data.values[0],sensor_data.status);
		if(err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)) {
			APS_ERR("call hwmsen_get_interrupt_data proximity fail = %d\n", err);
		}
	}

	mt_eint_unmask(CUST_EINT_ALS_NUM);
}

int ltr559_setup_eint(struct i2c_client *client)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);

	APS_FUN();
	g_ltr559_ptr = obj;

	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

	//mt_eint_set_sens(CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE);
	//mt_eint_set_polarity(CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY);
	mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, ltr559_eint_func, 0);//CUST_EINT_ALS_POLARITY
	mt_eint_mask(CUST_EINT_ALS_NUM);	//xmxl@20140506


	return 0;
}
#endif /* GN_MTK_BSP_ALSPS_INTERRUPT_MODE */
/*----------------------------------------------------------------------------*/
static int ltr559_init_client(struct i2c_client *client)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int err=0;
	APS_LOG("ltr559_init_client.........\r\n");
	
#ifdef GN_MTK_BSP_ALSPS_INTERRUPT_MODE
	if((err = ltr559_setup_eint(client)))
	{
		APS_ERR("setup eint: %d\n", err);
		return err;
	}
#endif

	if((err = ltr559_init_device(client)))
	{
		APS_ERR("init dev: %d\n", err);
		return err;
	}
	return err;
}
/******************************************************************************
 * Sysfs attributes
*******************************************************************************/
static ssize_t ltr559_show_config(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "(%d %d %d %d %d)\n",
		atomic_read(&ltr559_obj->i2c_retry), atomic_read(&ltr559_obj->als_debounce),
		atomic_read(&ltr559_obj->ps_mask), ltr559_obj->ps_thd_val, atomic_read(&ltr559_obj->ps_debounce));
	return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_store_config(struct device_driver *ddri, char *buf, size_t count)
{
	int retry, als_deb, ps_deb, mask, thres;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	if(5 == sscanf(buf, "%d %d %d %d %d", &retry, &als_deb, &mask, &thres, &ps_deb))
	{
		atomic_set(&ltr559_obj->i2c_retry, retry);
		atomic_set(&ltr559_obj->als_debounce, als_deb);
		atomic_set(&ltr559_obj->ps_mask, mask);
		ltr559_obj->ps_thd_val= thres;
		atomic_set(&ltr559_obj->ps_debounce, ps_deb);
	}
	else
	{
		APS_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_show_trace(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&ltr559_obj->trace));
	return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_store_trace(struct device_driver *ddri, char *buf, size_t count)
{
	int trace;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&ltr559_obj->trace, trace);
	}
	else
	{
		APS_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_show_als(struct device_driver *ddri, char *buf)
{
	int res;
	u8 dat = 0;

	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}
	// if(res = ltr559_read_data(ltr559_obj->client, &ltr559_obj->als))
	if(res = ltr559_read_data_als(ltr559_obj->client, &ltr559_obj->als))
	{
		return snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res);
	}
#if 0
	else
	{
		// dat = ltr559_obj->als & 0x3f;
		dat = ltr559_obj->als;
		return snprintf(buf, PAGE_SIZE, "0x%04X\n", dat);
	}
#endif
	while(-1 == ltr559_get_als_value(ltr559_obj,ltr559_obj->als))
	{
		ltr559_read_data_als(ltr559_obj->client,&ltr559_obj->als);
		msleep(50);
	}
	dat = ltr559_get_als_value(ltr559_obj,ltr559_obj->als);
	
	return snprintf(buf, PAGE_SIZE, "0x%04X\n", dat);
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_show_ps(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	u8 dat=0;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	// if(res = ltr559_read_data(ltr559_obj->client, &ltr559_obj->ps))
	if(res = ltr559_read_data_ps(ltr559_obj->client, &ltr559_obj->ps))
	{
		return snprintf(buf, PAGE_SIZE, "ERROR: %d\n", res);
	}
	else
	{
		// dat = ltr559_obj->ps & 0x80;
		dat = ltr559_get_ps_value(ltr559_obj, ltr559_obj->ps);
		return snprintf(buf, PAGE_SIZE, "0x%04X\n", dat);
	}
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_show_status(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;

	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	if(ltr559_obj->hw)
	{

		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d, (%d %d)\n",
			ltr559_obj->hw->i2c_num, ltr559_obj->hw->power_id, ltr559_obj->hw->power_vol);

	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}

	#ifdef MT6516
	len += snprintf(buf+len, PAGE_SIZE-len, "EINT: %d (%d %d %d %d)\n", mt_get_gpio_in(GPIO_ALS_EINT_PIN),
				CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY, CUST_EINT_ALS_DEBOUNCE_EN, CUST_EINT_ALS_DEBOUNCE_CN);

	len += snprintf(buf+len, PAGE_SIZE-len, "GPIO: %d (%d %d %d %d)\n",	GPIO_ALS_EINT_PIN,
				mt_get_gpio_dir(GPIO_ALS_EINT_PIN), mt_get_gpio_mode(GPIO_ALS_EINT_PIN),
				mt_get_gpio_pull_enable(GPIO_ALS_EINT_PIN), mt_get_gpio_pull_select(GPIO_ALS_EINT_PIN));
	#endif

	len += snprintf(buf+len, PAGE_SIZE-len, "MISC: %d %d\n", atomic_read(&ltr559_obj->als_suspend), atomic_read(&ltr559_obj->ps_suspend));

	return len;
}

#define IS_SPACE(CH) (((CH) == ' ') || ((CH) == '\n'))
/*----------------------------------------------------------------------------*/
static int read_int_from_buf(struct ltr559_priv *obj, const char* buf, size_t count,
							 u32 data[], int len)
{
	int idx = 0;
	char *cur = (char*)buf, *end = (char*)(buf+count);

	while(idx < len)
	{
		while((cur < end) && IS_SPACE(*cur))
		{
			cur++;
		}

		if(1 != sscanf(cur, "%d", &data[idx]))
		{
			break;
		}

		idx++;
		while((cur < end) && !IS_SPACE(*cur))
		{
			cur++;
		}
	}
	return idx;
}
/*----------------------------------------------------------------------------*/


static ssize_t ltr559_show_reg(struct device_driver *ddri, char *buf)
{
	int i = 0;
	u8 bufdata;
	int count  = 0;
	
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	for(i = 0;i < 31 ;i++)
	{
		hwmsen_read_byte_sr(ltr559_obj->client,0x80+i,&bufdata);
		count+= sprintf(buf+count,"[%x] = (%x)\n",0x80+i,bufdata);
	}

	return count;
}

static ssize_t ltr559_store_reg(struct device_driver *ddri,char *buf,ssize_t count)
{

	u32 data[2];
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null\n");
		return 0;
	}
	/*else if(2 != sscanf(buf,"%d %d",&addr,&data))*/
	else if(2 != read_int_from_buf(ltr559_obj,buf,count,data,2))
	{
		APS_ERR("invalid format:%s\n",buf);
		return 0;
	}

	hwmsen_write_byte(ltr559_obj->client,data[0],data[1]);

	return count;
}
/*----------------------------------------------------------------------------*/


static ssize_t ltr559_show_alslv(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	for(idx = 0; idx < ltr559_obj->als_level_num; idx++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "%d ", ltr559_obj->hw->als_level[idx]);
	}
	len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	return len;
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_store_alslv(struct device_driver *ddri, char *buf, size_t count)
{
	struct ltr559_priv *obj;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}
	else if(!strcmp(buf, "def"))
	{
		memcpy(ltr559_obj->als_level, ltr559_obj->hw->als_level, sizeof(ltr559_obj->als_level));
	}
	else if(ltr559_obj->als_level_num != read_int_from_buf(ltr559_obj, buf, count,
			ltr559_obj->hw->als_level, ltr559_obj->als_level_num))
	{
		APS_ERR("invalid format: '%s'\n", buf);
	}
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_show_alsval(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	for(idx = 0; idx < ltr559_obj->als_value_num; idx++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "%d ", ltr559_obj->hw->als_value[idx]);
	}
	len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	return len;
}
/*----------------------------------------------------------------------------*/
static ssize_t ltr559_store_alsval(struct device_driver *ddri, char *buf, size_t count)
{
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}
	else if(!strcmp(buf, "def"))
	{
		memcpy(ltr559_obj->als_value, ltr559_obj->hw->als_value, sizeof(ltr559_obj->als_value));
	}
	else if(ltr559_obj->als_value_num != read_int_from_buf(ltr559_obj, buf, count,
			ltr559_obj->hw->als_value, ltr559_obj->als_value_num))
	{
		APS_ERR("invalid format: '%s'\n", buf);
	}
	return count;
}

static ssize_t ltr559_show_enable_als(struct device_driver *ddrv,char *buf)
{
	ssize_t len =  0;
	int idx;
	if(!ltr559_obj)
	{
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}

	if(true == ltr559_obj->als_enable)
	{
		len = sprintf(buf,"%d\n",1);
	}
	else
	{
		len = sprintf(buf,"%d\n",0);
	}
	return len;

}
static  ssize_t ltr559_store_enable_als(struct device_driver *ddrv,char *buf, size_t count)
{
	int enable;
	if(!ltr559_obj)
	{	
		APS_ERR("ltr559_obj is null!!\n");
		return 0;
	}
	if(1 == sscanf(buf,"%d",&enable))
	{
		if(enable)
		{
			ltr559_enable_als(ltr559_obj->client,true);
		}
		else
		{
			ltr559_enable_als(ltr559_obj->client,false);
		}
	}
	else
	{
		APS_ERR("enable als fail\n");
	}
	return count;
}

static ssize_t ltr559_ps_adjust(struct device_driver *ddrv,char *buf)			
{																				
	int ret=0;
	int i=0;
	int data;
	int data_total=0;
	ssize_t len = 0;
	int noise = 0;
	int count = 20;
	int max = 0;
	int noise_high = 55;
	int noise_low = 45;
	int trigger_high = 0;
	int trigger_low = 0;

	if(!ltr559_obj)
	{	
		APS_ERR("ltr559_obj is null!!\n");
		len = sprintf(buf, "ltr559_obj is null\n");
		return 0;
	}

	hwmsen_write_byte(ltr559_obj->client, 0x8f, 0x00);
	if(ret = ltr559_enable_ps(ltr559_obj->client, true)) {
		APS_ERR("enable ps fail: %d\n", ret);
		len = sprintf(buf,"enable ps fail\n");
		return len;
	}

	// wait for register to be stable
	msleep(100);

	for (i = 0; i < count; i++) {
		// wait for ps value be stable
		msleep(50);

		ret=ltr559_read_data_ps(ltr559_obj->client,&data);
		if (ret < 0) {
			i--;
			continue;
		}
		data_total+=data;

		if (max++ > 100) {
			len = sprintf(buf,"adjust fail\n");
			return len;
		}
	}
	noise=data_total/count;
	trigger_high = noise + noise_high;
	trigger_low = noise + noise_low;

	if ((abs(trigger_high - ps_trigger_high) > ps_trigger_delta)) {
		len = sprintf(buf,"adjust larger than delta\n");
		return len;
	}
	ps_trigger_high = trigger_high;
	ps_trigger_low = trigger_low;

	hwmsen_write_byte(ltr559_obj->client, 0x90, ps_trigger_high & 0XFF);
	hwmsen_write_byte(ltr559_obj->client, 0x91, (ps_trigger_high>>8) & 0X07);
	hwmsen_write_byte(ltr559_obj->client, 0x92, 0x00);
	hwmsen_write_byte(ltr559_obj->client, 0x93, 0x00);

	hwmsen_write_byte(ltr559_obj->client, 0x8f, 0x01);
	len = sprintf(buf,"ps_trigger_high: %d, ps_trigger_low: %d\n",
		ps_trigger_high,ps_trigger_low);
	return len;
}

/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(als,	 S_IWUSR | S_IRUGO, ltr559_show_als,	NULL);
static DRIVER_ATTR(ps,	 S_IWUSR | S_IRUGO, ltr559_show_ps,	NULL);
static DRIVER_ATTR(alslv, S_IWUSR | S_IRUGO, ltr559_show_alslv, ltr559_store_alslv);
static DRIVER_ATTR(alsval, S_IWUSR | S_IRUGO, ltr559_show_alsval,ltr559_store_alsval);
static DRIVER_ATTR(trace, S_IWUSR | S_IRUGO, ltr559_show_trace, ltr559_store_trace);
static DRIVER_ATTR(status, S_IWUSR | S_IRUGO, ltr559_show_status, NULL);
static DRIVER_ATTR(reg,	 S_IWUSR | S_IRUGO, ltr559_show_reg, ltr559_store_reg);
static DRIVER_ATTR(enable_als,	 S_IWUGO | S_IRUGO, ltr559_show_enable_als, ltr559_store_enable_als);
static DRIVER_ATTR(adjust, S_IWUSR | S_IRUGO, ltr559_ps_adjust, NULL);
/*----------------------------------------------------------------------------*/
static struct device_attribute *ltr559_attr_list[] = {
	&driver_attr_als,
	&driver_attr_ps,
	&driver_attr_trace,		/*trace log*/
	&driver_attr_alslv,
	&driver_attr_alsval,
	&driver_attr_status,
	&driver_attr_reg,
	&driver_attr_enable_als,
	&driver_attr_adjust
};
/*----------------------------------------------------------------------------*/
static int ltr559_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(ltr559_attr_list)/sizeof(ltr559_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(err = driver_create_file(driver, ltr559_attr_list[idx]))
		{
			APS_ERR("driver_create_file (%s) = %d\n", ltr559_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}
/*----------------------------------------------------------------------------*/
	static int ltr559_delete_attr(struct device_driver *driver)
	{
	int idx ,err = 0;
	int num = (int)(sizeof(ltr559_attr_list)/sizeof(ltr559_attr_list[0]));

	if (!driver)
	return -EINVAL;

	for (idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, ltr559_attr_list[idx]);
	}

	return err;
}
/******************************************************************************
 * Function Configuration
******************************************************************************/
// static int ltr559_get_als_value(struct ltr559_priv *obj, u8 als)
static int ltr559_get_als_value(struct ltr559_priv *obj, int als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als < obj->hw->als_level[idx])
		{
			break;
		}
	}

	if(idx >= obj->als_value_num)
	{
		APS_ERR("exceed range\n");
		idx = obj->als_value_num - 1;
	}

	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}

		if(1 == atomic_read(&obj->als_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		if (atomic_read(&obj->trace) & CMC_TRC_CVT_ALS)
		{
			APS_DBG("ALS: %05d => %05d\n", als, obj->hw->als_value[idx]);
		}

		return obj->hw->als_value[idx];
	}
	else
	{
		if(atomic_read(&obj->trace) & CMC_TRC_CVT_ALS)
		{
			APS_DBG("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);
		}
		return -1;
	}
}
/*----------------------------------------------------------------------------*/

static int ltr559_get_ps_value(struct ltr559_priv *obj, int ps)
{
	int val= -1;
	int invalid = 0;
	

	if (ps > ps_trigger_high) {
		// bigger value, close
		val = 0;
	} else if (ps < ps_trigger_low) {
		// smaller value, far away
		val = 1;
	} else {
		val = 1;
	}

	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}

		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		if(unlikely(atomic_read(&obj->trace) & CMC_TRC_CVT_PS))
		{
			APS_DBG("PS: %05d => %05d\n", ps, val);
		}
		return val;

	}
	else
	{
		if(unlikely(atomic_read(&obj->trace) & CMC_TRC_CVT_PS))
		{
			APS_DBG("PS: %05d => %05d (-1)\n", ps, val);
		}
		return -1;
	}

}

/******************************************************************************
 * Function Configuration
******************************************************************************/
static int ltr559_open(struct inode *inode, struct file *file)
{
	file->private_data = ltr559_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int ltr559_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static long ltr559_unlocked_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int err = 0;
	void __user *ptr = (void __user*) arg;
	int dat;
	uint32_t enable;

	switch (cmd)
	{
		case ALSPS_SET_PS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if(err = ltr559_enable_ps(obj->client, true))
				{
					APS_ERR("enable ps fail: %d\n", err);
					goto err_out;
				}
				set_bit(CMC_BIT_PS, &obj->enable);
			}
			else
			{
				if(err = ltr559_enable_ps(obj->client, false))
				{
					APS_ERR("disable ps fail: %d\n", err);
					goto err_out;
				}
				clear_bit(CMC_BIT_PS, &obj->enable);
			}
			break;

		case ALSPS_GET_PS_MODE:
			enable = test_bit(CMC_BIT_PS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:
			if(err = ltr559_read_data_ps(obj->client, &obj->ps))
			{
				goto err_out;
			}
			dat = ltr559_get_ps_value(obj, obj->ps);
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_RAW_DATA:
			if(err = ltr559_read_data_ps(obj->client, &obj->ps))
			{
				goto err_out;
			}

			dat = obj->ps;// & 0x80; modified for ATA tool by xmxl@20140714
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_SET_ALS_MODE:
			if(copy_from_user(&enable, ptr, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(enable)
			{
				if(err = ltr559_enable_als(obj->client, true))
				{
					APS_ERR("enable als fail: %d\n", err);
					goto err_out;
				}
				set_bit(CMC_BIT_ALS, &obj->enable);
			}
			else
			{
				if(err = ltr559_enable_als(obj->client, false))
				{
					APS_ERR("disable als fail: %d\n", err);
					goto err_out;
				}
				clear_bit(CMC_BIT_ALS, &obj->enable);
			}
			break;

		case ALSPS_GET_ALS_MODE:
			enable = test_bit(CMC_BIT_ALS, &obj->enable) ? (1) : (0);
			if(copy_to_user(ptr, &enable, sizeof(enable)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_DATA:
			if(err = ltr559_read_data_als(obj->client, &obj->als))
			{
				goto err_out;
			}
			dat = obj->als;
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_ALS_RAW_DATA:
			if(err = ltr559_read_data_als(obj->client, &obj->als))
			{
				goto err_out;
			}

			dat = obj->als;	
			if(copy_to_user(ptr, &dat, sizeof(dat)))
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		default:
			APS_ERR("%s not supported = 0x%04x", __FUNCTION__, cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	err_out:
	return err;
}
/*----------------------------------------------------------------------------*/
static struct file_operations ltr559_fops = {
//	.owner = THIS_MODULE,
	.open = ltr559_open,
	.release = ltr559_release,
	.unlocked_ioctl = ltr559_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice ltr559_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &ltr559_fops,
};
/*----------------------------------------------------------------------------*/
static int ltr559_i2c_suspend(struct i2c_client *client, pm_message_t msg)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int err;
	APS_FUN();

	if(msg.event == PM_EVENT_SUSPEND)
	{
		if(!obj)
		{
			APS_ERR("null pointer!!\n");
			return -EINVAL;
		}

		atomic_set(&obj->als_suspend, 1);
		if(err = ltr559_enable_als(client, false))
		{
			APS_ERR("disable als: %d\n", err);
			return err;
		}
		if(!obj->ps_enable)
			ltr559_power(obj->hw, 0);
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int ltr559_i2c_resume(struct i2c_client *client)
{
	struct ltr559_priv *obj = i2c_get_clientdata(client);
	int err;
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	if(1 == ltr559_power(obj->hw, 1))
	{
		if(err = ltr559_init_device(client))
		{
			APS_ERR("initialize client fail!!\n");
			return err;
		}
		if(obj->ps_enable)
		{
			if(err = ltr559_enable_ps(client,true))
			{
				APS_ERR("enable ps fail: %d\n",err);
			}
			return err;
		}
	}
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))														//yaoyaoqin
	{
		if(err = ltr559_enable_als(client, true))
		{
			APS_ERR("enable als fail: %d\n", err);
		}
	}
	else
	{
		if(err = ltr559_enable_als(client, false))
		{
			APS_ERR("enable als fail: %d\n", err);
		}
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static void ltr559_early_suspend(struct early_suspend *h)
{
	/*early_suspend is only applied for ALS*/
	struct ltr559_priv *obj = container_of(h, struct ltr559_priv, early_drv);
	int err;
	//APS_FUN();
	APS_ERR("ltr559_early_suspend!\n");

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

	atomic_set(&obj->als_suspend, 1);
	if(err = ltr559_enable_als(obj->client, false))
	{
		APS_ERR("disable als fail: %d\n", err);
	}
}
/*----------------------------------------------------------------------------*/
static void ltr559_late_resume(struct early_suspend *h)
{
	/*early_suspend is only applied for ALS*/
	struct ltr559_priv *obj = container_of(h, struct ltr559_priv, early_drv);
	int err;
	//APS_FUN();
	APS_ERR("ltr559_late_resume!\n");

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}

	atomic_set(&obj->als_suspend, 0);
	if(test_bit(CMC_BIT_ALS, &obj->enable))
	{
		if(err = ltr559_enable_als(obj->client, true))
		{
			APS_ERR("enable als fail: %d\n", err);
		}
	}
	else
	{
		if(err = ltr559_enable_als(obj->client, false))
		{
			APS_ERR("enable als fail: %d\n", err);
		}
	}
}

int ltr559_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct ltr559_priv *obj = (struct ltr559_priv *)self;

	//APS_FUN(f);
	APS_LOG("ltr559_ps_operate command:%d\n",command);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				if(down_interruptible(&ltr559_enabling_ps_mutex)){//xmxl@20140526
					APS_ERR("PS SENSOR_ENABLE can't get semaphore\n"); 
					return -EFAULT;
				}
				value = *(int *)buff_in;
				if(value)
				{
					if(err = ltr559_enable_ps(obj->client, true))
					{
						APS_ERR("enable ps fail: %d\n", err);
						up(&ltr559_enabling_ps_mutex);//xmxl@20140526
						return -1;
					}
					set_bit(CMC_BIT_PS, &obj->enable);
				}
				else
				{
					if(err = ltr559_enable_ps(obj->client, false))
					{
						APS_ERR("disable ps fail: %d\n", err);
						up(&ltr559_enabling_ps_mutex);//xmxl@20140526
						return -1;
					}
					clear_bit(CMC_BIT_PS, &obj->enable);
				}
				up(&ltr559_enabling_ps_mutex);//xmxl@20140526
			}
			break;

		case SENSOR_GET_DATA:
			if ((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data))) {
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			} else {
				sensor_data = (hwm_sensor_data *)buff_out;
				if (err = ltr559_read_data_ps(obj->client, &obj->ps)) {
					err = -1;
					break;
				} else {
					while(-1 == ltr559_get_ps_value(obj, obj->ps)) {
						ltr559_read_data_ps(obj->client, &obj->ps);
						msleep(50);
					}
					sensor_data->values[0] = ltr559_get_ps_value(obj, obj->ps);
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
					APS_ERR("fwq get ps raw_data = %d, sensor_data =%d\n",obj->ps, sensor_data->values[0]);
				}
			}
			break;
		default:
			APS_ERR("proxmy sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}

	return err;
}

int ltr559_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct ltr559_priv *obj = (struct ltr559_priv *)self;

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			value = *(int *)buff_in;
			APS_ERR("ltr559_als_operate SENSOR_DELAY:%d\n",value);
			if(value<150)//xmxl@20140523: too fast
				err=-1;
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				APS_ERR("ltr559_als_operate SENSOR_ENABLE:%d\n",value);
				if(value)
				{
					if(err = ltr559_enable_als(obj->client, true))
					{
						APS_ERR("enable als fail: %d\n", err);
						return -1;
					}
					set_bit(CMC_BIT_ALS, &obj->enable);
				}
				else
				{
					if(err = ltr559_enable_als(obj->client, false))
					{
						APS_ERR("disable als fail: %d\n", err);
						return -1;
					}
					clear_bit(CMC_BIT_ALS, &obj->enable);
				}
			}
			break;

		case SENSOR_GET_DATA:
			//APS_LOG("fwq get als data !!!!!!\n");
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("ltr559_als_operate get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				if(down_interruptible(&ltr559_enabling_ps_mutex)){//xmxl@20140526
					APS_ERR("ALS SENSOR_GET_DATA can't get semaphore\n"); 
					err = -EINVAL;
					break;
				}
				sensor_data = (hwm_sensor_data *)buff_out;

				if(err = ltr559_read_data_als(obj->client, &obj->als))
				{
					err = -1;;
				}
				else
				{
					/*
					while(-1 == ltr559_get_als_value(obj, obj->als))
					{
						ltr559_read_data_als(obj->client, &obj->als);
						msleep(50);
					}
					*/
					sensor_data->values[0] = ltr559_get_als_value(obj, obj->als);
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
					//APS_DBG("ltr559_als_operate get obj->als = %d, sensor_data =%d\n",obj->als, sensor_data->values[0]);
					//obj->als=sensor_data->values[0];
				}
			
				up(&ltr559_enabling_ps_mutex);//xmxl@20140526
			}
			break;
		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}

	return err;
}


/*----------------------------------------------------------------------------*/
static int ltr559_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
	APS_FUN();
	strcpy(info->type, LTR559_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
static int ltr559_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ltr559_priv *obj;
	struct hwmsen_object obj_ps, obj_als;
	int err = 0;

	u8 buf = 0;
	int addr = 1;
	int ret = 0;
#ifdef CONFIG_GN_DEVICE_CHECK
	struct gn_device_info gn_dev_info_light = {0};
	struct gn_device_info gn_dev_info_proximity = {0};
#endif
		APS_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	ltr559_obj = obj;

	obj->hw = get_cust_alsps_hw();

#ifdef GN_MTK_BSP_ALSPS_INTERRUPT_MODE
	INIT_DELAYED_WORK(&obj->eint_work, ltr559_eint_work);
#endif
	obj->client = client;
	APS_LOG("addr = %x\n",obj->client->addr);
	i2c_set_clientdata(client, obj);
	atomic_set(&obj->als_debounce, 1000);
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 1000);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->trace, 0x00);
	atomic_set(&obj->als_suspend, 0);

	obj->ps_enable = 0;
	obj->ps_cali_valid=false;//xmxl@20140526
	obj->als_enable = 0;
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);

	//pre set ps threshold
	obj->ps_thd_val = obj->hw->ps_threshold;
	ps_trigger_high = obj->hw->ps_threshold_high;//xmxl@20140506
	ps_trigger_low = obj->hw->ps_threshold_low;
	//pre set window loss
	obj->als_widow_loss = obj->hw->als_window_loss;

	ltr559_i2c_client = client;

	if(err = ltr559_init_client(client))
	{
		goto exit_init_failed;
	}

	if(err = ltr559_enable_als(client, false))
	{
		APS_ERR("disable als fail: %d\n", err);
	}
	if(err = ltr559_enable_ps(client, false))
	{
		APS_ERR("disable ps fail: %d\n", err);
	}

	if(err = misc_register(&ltr559_device))
	{
		APS_ERR("ltr559_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	if(err = ltr559_create_attr(&(ltr559_init_info.platform_diver_addr->driver)))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

	obj_ps.self = ltr559_obj;
	APS_ERR("obj->hw->polling_mode:%d\n",obj->hw->polling_mode);
	if(1 == obj->hw->polling_mode_ps)
	{
		obj_ps.polling = 1;
	}
	else
	{
		obj_ps.polling = 0;//interrupt mode
	}
	obj_ps.sensor_operate = ltr559_ps_operate;
	if(err = hwmsen_attach(ID_PROXIMITY, &obj_ps))
	{
		APS_ERR("attach ID_PROXIMITY fail = %d\n", err);
		goto exit_create_attr_failed;
	}

	obj_als.self = ltr559_obj;
	ltr559_obj->polling = obj->hw->polling_mode;
	if(1 == obj->hw->polling_mode_als)
	{
		obj_als.polling = 1;
		APS_ERR("polling mode\n");
	}
	else
	{
		obj_als.polling = 0;//interrupt mode
		APS_ERR("interrupt mode\n");
	}
	obj_als.sensor_operate = ltr559_als_operate;
	if(err = hwmsen_attach(ID_LIGHT, &obj_als))
	{
		APS_ERR("attach ID_LIGHT fail = %d\n", err);
		goto exit_create_attr_failed;
	}
						
#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level	= EARLY_SUSPEND_LEVEL_DISABLE_FB,
	obj->early_drv.suspend = ltr559_early_suspend,
	obj->early_drv.resume = ltr559_late_resume,
	register_early_suspend(&obj->early_drv);
#endif

	
#ifdef CONFIG_GN_DEVICE_CHECK
	gn_dev_info_light.gn_dev_type = GN_DEVICE_TYPE_LIGHT;
	strcpy(gn_dev_info_light.name, LTR559_DEV_NAME);
	gn_set_device_info(gn_dev_info_light);

	gn_dev_info_proximity.gn_dev_type = GN_DEVICE_TYPE_PROXIMITY;
	strcpy(gn_dev_info_proximity.name, LTR559_DEV_NAME);
	gn_set_device_info(gn_dev_info_proximity);
#endif
	
	APS_LOG("%s: OK\n", __func__);
    ltr559_init_flag = 0;

	return 0;

	exit_create_attr_failed:
	misc_deregister(&ltr559_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	exit_kfree:
	kfree(obj);
	exit:
	ltr559_i2c_client = NULL;
	mt_eint_unmask(CUST_EINT_ALS_NUM);

	ltr559_init_flag = -1;
	APS_ERR("%s: err = %d\n", __func__, err);
	return err;
}
/*----------------------------------------------------------------------------*/
static int ltr559_i2c_remove(struct i2c_client *client)
{
	int err;

	if(err = ltr559_delete_attr(&(ltr559_init_info.platform_diver_addr->driver)))
	{
		APS_ERR("ltr559_delete_attr fail: %d\n", err);
	}

	if(err = misc_deregister(&ltr559_device))
	{
		APS_ERR("misc_deregister fail: %d\n", err);
	}

	ltr559_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static int ltr559_local_init(void)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();

	ltr559_power(hw,1);
	if(i2c_add_driver(&ltr559_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	}

	if(-1 == ltr559_init_flag)
	{
			return -1;
	}

	return 0;
}

static int ltr559_remove(void)
{
	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();
	ltr559_power(hw, 0);
	i2c_del_driver(&ltr559_i2c_driver);
	return 0;
}
/*
static struct platform_driver ltr559_alsps_driver = {
	.probe	= ltr559_probe,
	.remove	= ltr559_remove,
	.driver	= {
		.name	= "als_ps",
//		.owner	= THIS_MODULE,
	}
};
*/
static int __init ltr559_init(void)
{
	APS_FUN();
	//wake_lock_init(&ps_wake_lock,WAKE_LOCK_SUSPEND,"ps module");
    struct alsps_hw *hw = get_cust_alsps_hw();

	i2c_register_board_info(hw->i2c_num, &i2c_ltr559, 1);  //xiaoqian, 20120412, add for alsps
	hwmsen_alsps_sensor_add(&ltr559_init_info);
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit ltr559_exit(void)
{
	APS_FUN();
	//platform_driver_unregister(&ltr559_alsps_driver);
	//wake_lock_destroy(&ps_wake_lock);
}
/*----------------------------------------------------------------------------*/
module_init(ltr559_init);
module_exit(ltr559_exit);
/*----------------------------------------------------------------------------*/
MODULE_DESCRIPTION("LTR559 light sensor & p sensor driver");
MODULE_LICENSE("GPL");
