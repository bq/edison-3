#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/earlysuspend.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>

#include <mach/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/mt_gpio.h>
#include <mach/mtk_rtc.h>
#include <mach/mt_spm_mtcmos.h>

#include <mach/battery_common.h>
#include <linux/time.h>

// ============================================================ //
//extern function
// ============================================================ //
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);

static void hw_bc11_dump_register(void)
{
	kal_uint32 reg_val = 0;
	kal_uint32 reg_num = CHR_CON18;
	kal_uint32 i = 0;

	for(i=reg_num ; i<=CHR_CON19 ; i+=2)
	{
		reg_val = upmu_get_reg_value(i);
		battery_xlog_printk(BAT_LOG_FULL, "Chr Reg[0x%x]=0x%x \r\n", i, reg_val);
	}
}

static void hw_bc11_init(void)
{
	 Charger_Detect_Init();
		 
	 //RG_BC11_BIAS_EN=1	
	 upmu_set_rg_bc11_bias_en(0x1);
	 //RG_BC11_VSRC_EN[1:0]=00
	 upmu_set_rg_bc11_vsrc_en(0x0);
	 //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //BC11_RST=1
	 upmu_set_rg_bc11_rst(0x1);
	 //BC11_BB_CTRL=1
	 upmu_set_rg_bc11_bb_ctrl(0x1);
 
 	 //msleep(10);
 	 mdelay(50);
}

static U32 hw_bc11_DCD(void)
{
	 U32 wChargerAvail = 0;
 
	 //RG_BC11_IPU_EN[1.0] = 10
	 upmu_set_rg_bc11_ipu_en(0x2);
	 //RG_BC11_IPD_EN[1.0] = 01
	 upmu_set_rg_bc11_ipd_en(0x1);
	 //RG_BC11_VREF_VTH = [1:0]=01
	 upmu_set_rg_bc11_vref_vth(0x1);
	 //RG_BC11_CMP_EN[1.0] = 10
	 upmu_set_rg_bc11_cmp_en(0x2);
 
	 //msleep(20);
	 mdelay(80);

 	 wChargerAvail = upmu_get_rgs_bc11_cmp_out();
	 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	 //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
 
	 return wChargerAvail;
}

#ifndef MTK_PMIC_MT6397 //only for mt6323 use
static U32 hw_bc11_stepA1(void)
{
	U32 wChargerAvail = 0;
	  
	//RG_BC11_IPU_EN[1.0] = 10
	upmu_set_rg_bc11_ipu_en(0x2);
	//RG_BC11_VREF_VTH = [1:0]=10
	upmu_set_rg_bc11_vref_vth(0x2);
	//RG_BC11_CMP_EN[1.0] = 10
	upmu_set_rg_bc11_cmp_en(0x2);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
 
	return  wChargerAvail;
}
#endif

static U32 hw_bc11_stepB1(void)
{
   U32 wChargerAvail = 0;

   //RG_BC11_IPU_EN[1.0] = 01
   //upmu_set_rg_bc11_ipu_en(0x1);
   upmu_set_rg_bc11_ipd_en(0x1);
   //RG_BC11_VREF_VTH = [1:0]=10
   //upmu_set_rg_bc11_vref_vth(0x2);
   upmu_set_rg_bc11_vref_vth(0x0);
   //RG_BC11_CMP_EN[1.0] = 01
   upmu_set_rg_bc11_cmp_en(0x1);

   //msleep(80);
   mdelay(80);

   wChargerAvail = upmu_get_rgs_bc11_cmp_out();

   if(Enable_BATDRV_LOG == BAT_LOG_FULL)
   {
	   battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepB1() \r\n");
	   hw_bc11_dump_register();
   }

   //RG_BC11_IPU_EN[1.0] = 00
   upmu_set_rg_bc11_ipu_en(0x0);
   //RG_BC11_CMP_EN[1.0] = 00
   upmu_set_rg_bc11_cmp_en(0x0);
   //RG_BC11_VREF_VTH = [1:0]=00
   upmu_set_rg_bc11_vref_vth(0x0);

   return  wChargerAvail;
}


static U32 hw_bc11_stepC1(void)
{
   U32 wChargerAvail = 0;

   //RG_BC11_IPU_EN[1.0] = 01
   upmu_set_rg_bc11_ipu_en(0x1);
   //RG_BC11_VREF_VTH = [1:0]=10
   upmu_set_rg_bc11_vref_vth(0x2);
   //RG_BC11_CMP_EN[1.0] = 01
   upmu_set_rg_bc11_cmp_en(0x1);

   //msleep(80);
   mdelay(80);

   wChargerAvail = upmu_get_rgs_bc11_cmp_out();

   if(Enable_BATDRV_LOG == BAT_LOG_FULL)
   {
	   battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepC1() \r\n");
	   hw_bc11_dump_register();
   }

   //RG_BC11_IPU_EN[1.0] = 00
   upmu_set_rg_bc11_ipu_en(0x0);
   //RG_BC11_CMP_EN[1.0] = 00
   upmu_set_rg_bc11_cmp_en(0x0);
   //RG_BC11_VREF_VTH = [1:0]=00
   upmu_set_rg_bc11_vref_vth(0x0);

   return  wChargerAvail;
}

static U32 hw_bc11_stepA2(void)
{
	U32 wChargerAvail = 0;
	  
	//RG_BC11_VSRC_EN[1.0] = 10 
	upmu_set_rg_bc11_vsrc_en(0x2);
	//RG_BC11_IPD_EN[1:0] = 01
	upmu_set_rg_bc11_ipd_en(0x1);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(0x0);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
 
	return  wChargerAvail;
}

static U32 hw_bc11_stepB2(void)
{
	U32 wChargerAvail = 0;
 
	//RG_BC11_IPU_EN[1:0]=10
	upmu_set_rg_bc11_ipu_en(0x2);
	//RG_BC11_VREF_VTH = [1:0]=10
	upmu_set_rg_bc11_vref_vth(0x1);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
 
	return  wChargerAvail;
 }

static void hw_bc11_done(void)
{
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=0
	upmu_set_rg_bc11_vref_vth(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(0x0);
	//RG_BC11_BIAS_EN=0
	upmu_set_rg_bc11_bias_en(0x0); 
 
	Charger_Detect_Release();
}
#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)

CHARGER_TYPE hw_charger_type_detection(void)
{
    return STANDARD_HOST;
}

#else

#ifdef MTK_PMIC_MT6397 //for mt6397 detect flow

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE ret = CHARGER_UNKNOWN;
	hw_bc11_init();
 
	if(1 == hw_bc11_DCD())
	{
		 ret = NONSTANDARD_CHARGER;
	} else {
		 if(1 == hw_bc11_stepA2())
		 {
			 if(1 == hw_bc11_stepB2())
			 {
				 ret = STANDARD_CHARGER;
			 } else {
				 ret = CHARGING_HOST;
			 }
		 } else {
             ret = STANDARD_HOST;
		 }
	}
	hw_bc11_done();

	return ret;
}
#else //for mt6323 detect flow

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE ret = CHARGER_UNKNOWN;

#if 1
	/********* Step initial  ***************/
	hw_bc11_init();

	/********* Step DCD ***************/
	if(1 == hw_bc11_DCD())
	{
		/********* Step A1 ***************/
		if(1 == hw_bc11_stepA1())
		{
			/********* Step B1 ***************/
			if(1 == hw_bc11_stepB1())
			{
				ret = APPLE_2_1A_CHARGER;
				battery_xlog_printk(BAT_LOG_CRTI, "step B1 : Apple 2.1A CHARGER!\r\n");
			}
			else
			{
				ret = NONSTANDARD_CHARGER;
				battery_xlog_printk(BAT_LOG_CRTI, "step B1 : Non STANDARD CHARGER!\r\n");
			}
		}
		else
		{
			/********* Step C1 ***************/
			if(1 == hw_bc11_stepC1())
			{
				ret = APPLE_1_0A_CHARGER;
				battery_xlog_printk(BAT_LOG_CRTI, "step C1 : Apple 1A CHARGER!\r\n");
			}
			else
			{
				ret = APPLE_0_5A_CHARGER;
				battery_xlog_printk(BAT_LOG_CRTI, "step C1 : Apple 0.5A CHARGER!\r\n");
			}
		}
	}
	else
	{
		/********* Step A2 ***************/
		if(1 == hw_bc11_stepA2())
		{
			/********* Step B2 ***************/
			if(1 == hw_bc11_stepB2())
			{
				ret = STANDARD_CHARGER;
				battery_xlog_printk(BAT_LOG_CRTI, "step B2 : STANDARD CHARGER!\r\n");
			}
			else
			{
				ret = CHARGING_HOST;
				battery_xlog_printk(BAT_LOG_CRTI, "step B2 :  Charging Host!\r\n");
			}
		}
		else
		{
			ret = STANDARD_HOST;
			battery_xlog_printk(BAT_LOG_CRTI, "step A2 : Standard USB Host!\r\n");
		}
	}

	/********* Finally setting *******************************/
	hw_bc11_done();
#else
	hw_bc11_init();
 
	if(1 == hw_bc11_DCD())
	{
		if(1 == hw_bc11_stepA1())
		{
			ret = APPLE_2_1A_CHARGER;
		} else {
			ret = NONSTANDARD_CHARGER;
		}
	} else {
		 if(1 == hw_bc11_stepA2())
		 {
			 if(1 == hw_bc11_stepB2())
			 {
				 ret = STANDARD_CHARGER;
			 } else {
				 ret = CHARGING_HOST;
			 }
		 } else {
			 ret = STANDARD_HOST;
		 }
	}
	hw_bc11_done();
#endif

	return ret;
}
#endif
#endif
