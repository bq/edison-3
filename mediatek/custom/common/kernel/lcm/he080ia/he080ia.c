#ifndef BUILD_LK
#include <linux/string.h>
#else
#include <string.h>   //for fixed warning issue
#endif

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/upmu_common.h>
#endif
#include "lcm_drv.h"
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (800)
#define FRAME_HEIGHT (1280)


#define LCM_DSI_CMD_MODE    0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {
    .set_gpio_out = NULL,
};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(para_tbl,size,force_update)        lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

/*[M]For different lcd poweron logic,ArthurLin,20130809*/
#define GPIO_LCD_PWR_EN		GPIO115
#define GPIO_LCD_RST      GPIO119
#define GPIO_LCD_BL_EN		GPIO_UNSUPPORTED
#define LCD_RST_VALUE			GPIO_OUT_ZERO
/*END,ArthurLin,20130809*/

static LCM_setting_table_V3 lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/


	//must use 0x39 for init setting for all register.
	//======Internal setting======
	{0x39, 0xFF, 4, {0xAA, 0x55, 0xA5, 0x80}},

	//======MIPI ralated timing setting======
	{0x39, 0x6F, 2, {0x11, 0x00}},
	{0x39, 0xF7, 2, {0x20, 0x00}},
	//======Improve ESD option======
	{0x15, 0x6F, 1, {0x06}},
	{0x15, 0xF7, 1, {0xA0}},
	{0x15, 0x6F, 1, {0x19}},
	{0x15, 0xF7, 1, {0x12}},
	{0x15, 0xF4, 1, {0x03}},

	//======Vcom floating======
	{0x15, 0x6F, 1, {0x08}},
	{0x15, 0xFA, 1, {0x40}},
	{0x15, 0x6F, 1, {0x11}},
	{0x15, 0xF3, 1, {0x01}},

	//======Page0 relative======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x00}},
	{0x15, 0xC8, 1, {0x80}},

	//======Set WXGA resolution 0x6C======
	/* 
	The last parameter of below command, 
	original value is 0x07, 
	set to 0x01 to rotate 180 degress,
	the other option value is 0x03 & 0x05.
	*/
	{0x39, 0xB1, 2, {0x6C, 0x01}},

	//======Set source output hold time======
	{0x15, 0xB6, 1, {0x08}},

	//======EQ control function======
	{0x15, 0x6F, 1, {0x02}},
	{0x15, 0xB8, 1, {0x08}},

	//======Set bias current for GOP and SOP======
	{0x39, 0xBB, 2, {0x74, 0x44}},

	//======Inversion setting======
	{0x39, 0xBC, 2, {0x00, 0x00}},

	//======DSP timing Settings update for BIST======
	{0x39, 0xBD, 5, {0x02, 0xB0, 0x0C, 0x0A, 0x00}},

	//======Page1 relative======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x01}},

	//======Setting AVDD, AVEE clamp======
	{0x39, 0xB0, 2, {0x05, 0x05}},
	{0x39, 0xB1, 2, {0x05, 0x05}},

	//======VGMP, VGMN, VGSP, VGSN setting======
	{0x39, 0xBC, 2, {0x90, 0x01}},
	{0x39, 0xBD, 2, {0x90, 0x01}},

	//======Gate signal control======
	{0x15, 0xCA, 1, {0x00}},

	//======Power IC control======
	{0x15, 0xC0, 1, {0x04}},

	//======VCOM -1.88V======
	{0x15, 0xBE, 1, {0x29}},

	//======Setting VGH=15V, VGL=-11V======
	{0x39, 0xB3, 2, {0x37, 0x37}},
	{0x39, 0xB4, 2, {0x19, 0x19}},

	//======Power control for VGH, VGL======
	{0x39, 0xB9, 2, {0x44, 0x44}},
	{0x39, 0xBA, 2, {0x24, 0x24}},

	//======Page2 relative======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x02}},

	//======Gamma control register control======
	{0x15, 0xEE, 1, {0x01}},
	//======Gradient control for Gamma voltage======
	{0x39, 0xEF, 4, {0x09, 0x06, 0x15, 0x18}},

	{0x39, 0xB0, 6, {0x00, 0x00, 0x00, 0x25, 0x00, 0x43}},
	{0x15, 0x6F, 1, {0x06}},
	{0x39, 0xB0, 6, {0x00, 0x54, 0x00, 0x68, 0x00, 0xA0}},
	{0x15, 0x6F, 1, {0x0C}},
	{0x39, 0xB0, 4, {0x00, 0xC0, 0x01, 0x00}},
	{0x39, 0xB1, 6, {0x01, 0x30, 0x01, 0x78, 0x01, 0xAE}},
	{0x15, 0x6F, 1, {0x06}},
	{0x39, 0xB1, 6, {0x02, 0x08, 0x02, 0x52, 0x02, 0x54}},
	{0x15, 0x6F, 1, {0x0C}},
	{0x39, 0xB1, 4, {0x02, 0x99, 0x02, 0xF0}},
	{0x39, 0xB2, 6, {0x03, 0x20, 0x03, 0x56, 0x03, 0x76}},
	{0x15, 0x6F, 1, {0x06}},
	{0x39, 0xB2, 6, {0x03, 0x93, 0x03, 0xA4, 0x03, 0xB9}},
	{0x15, 0x6F, 1, {0x0C}},
	{0x39, 0xB2, 4, {0x03, 0xC9, 0x03, 0xE3}},
	{0x39, 0xB3, 4, {0x03, 0xFC, 0x03, 0xFF}},

	//======GOA relative======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x06}},
	{0x39, 0xB0, 2, {0x00, 0x10}},
	{0x39, 0xB1, 2, {0x12, 0x14}},
	{0x39, 0xB2, 2, {0x16, 0x18}},
	{0x39, 0xB3, 2, {0x1A, 0x29}},
	{0x39, 0xB4, 2, {0x2A, 0x08}},
	{0x39, 0xB5, 2, {0x31, 0x31}},
	{0x39, 0xB6, 2, {0x31, 0x31}},
	{0x39, 0xB7, 2, {0x31, 0x31}},
	{0x39, 0xB8, 2, {0x31, 0x0A}},
	{0x39, 0xB9, 2, {0x31, 0x31}},
	{0x39, 0xBA, 2, {0x31, 0x31}},
	{0x39, 0xBB, 2, {0x0B, 0x31}},
	{0x39, 0xBC, 2, {0x31, 0x31}},
	{0x39, 0xBD, 2, {0x31, 0x31}},
	{0x39, 0xBE, 2, {0x31, 0x31}},
	{0x39, 0xBF, 2, {0x09, 0x2A}},
	{0x39, 0xC0, 2, {0x29, 0x1B}},
	{0x39, 0xC1, 2, {0x19, 0x17}},
	{0x39, 0xC2, 2, {0x15, 0x13}},
	{0x39, 0xC3, 2, {0x11, 0x01}},
	{0x39, 0xE5, 2, {0x31, 0x31}},
	{0x39, 0xC4, 2, {0x09, 0x1B}},
	{0x39, 0xC5, 2, {0x19, 0x17}},
	{0x39, 0xC6, 2, {0x15, 0x13}},
	{0x39, 0xC7, 2, {0x11, 0x29}},
	{0x39, 0xC8, 2, {0x2A, 0x01}},
	{0x39, 0xC9, 2, {0x31, 0x31}},
	{0x39, 0xCA, 2, {0x31, 0x31}},
	{0x39, 0xCB, 2, {0x31, 0x31}},
	{0x39, 0xCC, 2, {0x31, 0x0B}},
	{0x39, 0xCD, 2, {0x31, 0x31}},
	{0x39, 0xCE, 2, {0x31, 0x31}},
	{0x39, 0xCF, 2, {0x0A, 0x31}},
	{0x39, 0xD0, 2, {0x31, 0x31}},
	{0x39, 0xD1, 2, {0x31, 0x31}},
	{0x39, 0xD2, 2, {0x31, 0x31}},
	{0x39, 0xD3, 2, {0x00, 0x2A}},
	{0x39, 0xD4, 2, {0x29, 0x10}},
	{0x39, 0xD5, 2, {0x12, 0x14}},
	{0x39, 0xD6, 2, {0x16, 0x18}},
	{0x39, 0xD7, 2, {0x1A, 0x08}},
	{0x39, 0xE6, 2, {0x31, 0x31}},
	{0x39, 0xD8, 5, {0x00, 0x00, 0x00, 0x54, 0x00}},
	{0x39, 0xD9, 5, {0x00, 0x15, 0x00, 0x00, 0x00}},
	{0x15, 0xE7, 1, {0x00}},

	//======Page3, gate timing control======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x03}},
	{0x39, 0xB0, 2, {0x20, 0x00}},
	{0x39, 0xB1, 2, {0x20, 0x00}},
	{0x39, 0xB2, 5, {0x05, 0x00, 0x00, 0x00, 0x00}},

	{0x39, 0xB6, 5, {0x05, 0x00, 0x00, 0x00, 0x00}},
	{0x39, 0xB7, 5, {0x05, 0x00, 0x00, 0x00, 0x00}},

	{0x39, 0xBA, 5, {0x57, 0x00, 0x00, 0x00, 0x00}},
	{0x39, 0xBB, 5, {0x57, 0x00, 0x00, 0x00, 0x00}},

	{0x39, 0xC0, 4, {0x00, 0x00, 0x00, 0x00}},
	{0x39, 0xC1, 4, {0x00, 0x00, 0x00, 0x00}},

	{0x15, 0xC4, 1, {0x60}},
	{0x15, 0xC5, 1, {0x40}},

	//======Page5======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x05}},
	{0x39, 0xBD, 5, {0x03, 0x01, 0x03, 0x03, 0x03}},
	{0x39, 0xB0, 2, {0x17, 0x06}},
	{0x39, 0xB1, 2, {0x17, 0x06}},
	{0x39, 0xB2, 2, {0x17, 0x06}},
	{0x39, 0xB3, 2, {0x17, 0x06}},
	{0x39, 0xB4, 2, {0x17, 0x06}},
	{0x39, 0xB5, 2, {0x17, 0x06}},

	{0x15, 0xB8, 1, {0x00}},
	{0x15, 0xB9, 1, {0x00}},
	{0x15, 0xBA, 1, {0x00}},
	{0x15, 0xBB, 1, {0x02}},
	{0x15, 0xBC, 1, {0x00}},

	{0x15, 0xC0, 1, {0x07}},

	{0x15, 0xC4, 1, {0x80}},
	{0x15, 0xC5, 1, {0xA4}},

	{0x39, 0xC8, 2, {0x05, 0x30}},
	{0x39, 0xC9, 2, {0x01, 0x31}},

	{0x39, 0xCC, 3, {0x00, 0x00, 0x3C}},
	{0x39, 0xCD, 3, {0x00, 0x00, 0x3C}},

	{0x39, 0xD1, 5, {0x00, 0x04, 0xFD, 0x07, 0x10}},
	{0x39, 0xD2, 5, {0x00, 0x05, 0x02, 0x07, 0x10}},

	{0x15, 0xE5, 1, {0x06}},
	{0x15, 0xE6, 1, {0x06}},
	{0x15, 0xE7, 1, {0x06}},
	{0x15, 0xE8, 1, {0x06}},
	{0x15, 0xE9, 1, {0x06}},
	{0x15, 0xEA, 1, {0x06}},

	{0x15, 0xED, 1, {0x30}},

	//======Reload setting======
	{0x15, 0x6F, 1, {0x11}},
	{0x15, 0xF3, 1, {0x01}},

	//======Normal Display======
	{0x05, 0x35, 0, {}},
	{0x05, 0x11, 0, {}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 120, {}},
	{0x05, 0x29, 0, {}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 20, {}},
};

static void lcd_power_en(unsigned char enabled)
{
    if (enabled)
    {      
        //mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);
        mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ONE);
        upmu_set_rg_vgp1_vosel(0x3);
        upmu_set_rg_vgp1_en(0x1);
	 upmu_set_rg_vgp3_vosel(0x3);
        upmu_set_rg_vgp3_en(0x1);
 
    }
    else
    {      
        //mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ZERO);
         mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ZERO);
	  upmu_set_rg_vgp1_en(0x0);
	  upmu_set_rg_vgp3_en(0x0);
    }
}


static void lcd_reset(unsigned char enabled)
{
    if(enabled)
    {
    	 mt_set_gpio_mode(GPIO_LCD_RST, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_RST, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_RST, LCD_RST_VALUE); //[M]For different reset gpio level,ArthurLin,20130809
    }
    else
    {	
        mt_set_gpio_mode(GPIO_LCD_RST, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_RST, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_RST, !LCD_RST_VALUE); //[M]For different reset gpio level,ArthurLin,20130809
    }
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type     = LCM_TYPE_DSI;
    params->width    = FRAME_WIDTH;
    params->height   = FRAME_HEIGHT;
    params->dsi.mode = BURST_VDO_MODE;

    params->physical_width = 108;  // 107.640 mm
    params->physical_height = 172; // 172.224 mm

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;

    // Video mode setting		
    params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 5;
    params->dsi.vertical_backporch					= 3;
    params->dsi.vertical_frontporch 				= 8;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 5;
    params->dsi.horizontal_backporch				= 59;
    params->dsi.horizontal_frontporch				= 16;
    params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;

    // Bit rate calculation
    //params->dsi.pll_div1=0;
    //params->dsi.pll_div2=1;
    //params->dsi.fbk_div =22;

    //params->dsi.cont_clock = 1;
    //params->dsi.horizontal_bllp = 0x115; //word count=0x340
    params->dsi.PLL_CLOCK = 234;

    params->dsi.ssc_disable= 1;
}

extern void DSI_clk_HS_mode(unsigned char enter);
static void init_lcm_registers(void)
{
    unsigned int data_array[16];

    #ifdef BUILD_LK
    printf("%s, LK \n", __func__);
    #else
    printk("%s, kernel", __func__);
    #endif

    dsi_set_cmdq_V3(lcm_initialization_setting,sizeof(lcm_initialization_setting)/sizeof(lcm_initialization_setting[0]),1);
}


static void lcm_init(void)
{
   #ifdef BUILD_LK
    printf("%s, LK \n", __func__);
    // Set parameter to 0x3 to set VGP1_PMU to 1.8V
    //upmu_set_rg_vgp1_vosel(0x3);
   // upmu_set_rg_vgp1_en(0x1);
    if(GPIO_LCD_BL_EN != GPIO_UNSUPPORTED){
        //lcd_bl_en(0);
    }

    lcd_reset(0);
    lcd_power_en(0);
    MDELAY(1);
    lcd_power_en(1);
    MDELAY(50);
    lcd_reset(1);
    MDELAY(1);
    lcd_reset(0);
    MDELAY(15);
    lcd_reset(1);
    MDELAY(130);//Must > 120ms
    init_lcm_registers();
   #else
    printk("%s, kernel \n", __func__);
   #endif
}


static void lcm_suspend(void)
{
    unsigned int data_array[16];
    
    data_array[0] = 0x00280500;  //display off                        
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(5);
    lcd_reset(0);
    lcd_power_en(0);
    DSI_clk_HS_mode(0);
    MDELAY(10);
}


static void lcm_resume(void)
{
    //upmu_set_rg_vgp1_vosel(0x3);
   // upmu_set_rg_vgp1_en(0x1);
    if(GPIO_LCD_BL_EN != GPIO_UNSUPPORTED){
        //lcd_bl_en(0);
    }
    lcd_reset(0);
    lcd_power_en(0);
    MDELAY(1);
    lcd_power_en(1);
    MDELAY(50);
    lcd_reset(1);
    MDELAY(1);
    lcd_reset(0);
    MDELAY(15);
    lcd_reset(1);
    MDELAY(130);//Must > 120ms
    init_lcm_registers();
}

LCM_DRIVER he080ia_lcm_drv = 
{
	.name           = "he080ia",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	//.compare_id    = lcm_compare_id,
};

