#ifndef BUILD_LK
#include <linux/string.h>
#else
#include <string.h>
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

#define FRAME_WIDTH  (600)
#define FRAME_HEIGHT (1024)


#define LCM_DSI_CMD_MODE    0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {
    .set_gpio_out = NULL,
};

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

/*****************Mipi Init code*****************/
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

 #if 0 
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
	{0x39, 0xBB, 2, {0x54, 0x54}},//{0x74, 0x44}},

	//======Inversion setting======
	{0x39, 0xBC, 2, {0x05, 0x05}},//{0x00, 0x00}},

	//======Zigzag setting======
	{0x15, 0xC7, 1, {0x01}},

	//======DSP timing Settings update for BIST======
	{0x39, 0xBD, 5, {0x02, 0xB0, 0x0C, 0x0A, 0x00}},

	//======Page1 relative======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x01}},

	//======Setting AVDD, AVEE clamp======
	{0x39, 0xB0, 2, {0x05, 0x05}},
	{0x39, 0xB1, 2, {0x05, 0x05}},

	//======VGMP, VGMN, VGSP, VGSN setting======
	{0x39, 0xBC, 2, {0x3A, 0x01}},//{0x90, 0x01}},
	{0x39, 0xBD, 2, {0x3E, 0x01}},//{0x90, 0x01}},

	//======Gate signal control======
	{0x15, 0xCA, 1, {0x00}},

	//======Power IC control======
	{0x15, 0xC0, 1, {0x04}},

	//======VCL set -2.5V======
	{0x15, 0xB2, 1, {0x00}},

	//======VCOM -1.88V======
	{0x15, 0xBE, 1, {0x80}},//{0x29}},

	//======Setting VGH=15V, VGL=-11V======
	{0x39, 0xB3, 2, {0x19, 0x19}},//{0x37, 0x37}},
	{0x39, 0xB4, 2, {0x12, 0x12}},//{0x19, 0x19}},

	//======Power control for VGH, VGL======
	{0x39, 0xB9, 2, {0x24, 0x24}},//{0x44, 0x44}},
	{0x39, 0xBA, 2, {0x14, 0x14}},//{0x24, 0x24}},

	//======Page2 relative======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x02}},

	//======Gamma control register control======
	{0x15, 0xEE, 1, {0x01}},
	//======Gradient control for Gamma voltage======
	{0x39, 0xEF, 4, {0x09, 0x06, 0x15, 0x18}},

	{0x39, 0xB0, 6, {0x00, 0x00, 0x00, 0x08, 0x00, 0x17}},
	{0x15, 0x6F, 1, {0x06}},
	{0x39, 0xB0, 6, {0x00, 0x25, 0x00, 0x30, 0x00, 0x45}},
	{0x15, 0x6F, 1, {0x0C}},
	{0x39, 0xB0, 4, {0x00, 0x56, 0x00, 0x7A}},
	{0x39, 0xB1, 6, {0x00, 0xA3, 0x00, 0xE7, 0x01, 0x20}},
	{0x15, 0x6F, 1, {0x06}},
	{0x39, 0xB1, 6, {0x01, 0x7A, 0x01, 0xC2, 0x01, 0xC5}},
	{0x15, 0x6F, 1, {0x0C}},
	{0x39, 0xB1, 4, {0x02, 0x06, 0x02, 0x5F}},
	{0x39, 0xB2, 6, {0x02, 0x92, 0x02, 0xD0, 0x02, 0xFC}},
	{0x15, 0x6F, 1, {0x06}},
	{0x39, 0xB2, 6, {0x03, 0x35, 0x03, 0x5D, 0x03, 0x8B}},
	{0x15, 0x6F, 1, {0x0C}},
	{0x39, 0xB2, 4, {0x03, 0xA2, 0x03, 0xBF}},
	{0x39, 0xB3, 4, {0x03, 0xD2, 0x03, 0xFF}},

	//======GOA relative======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x06}},
	{0x39, 0xB0, 2, {0x00, 0x17}},
	{0x39, 0xB1, 2, {0x16, 0x15}},
	{0x39, 0xB2, 2, {0x14, 0x13}},
	{0x39, 0xB3, 2, {0x12, 0x11}},
	{0x39, 0xB4, 2, {0x10, 0x2D}},
	{0x39, 0xB5, 2, {0x01, 0x08}},
	{0x39, 0xB6, 2, {0x09, 0x31}},
	{0x39, 0xB7, 2, {0x31, 0x31}},
	{0x39, 0xB8, 2, {0x31, 0x31}},
	{0x39, 0xB9, 2, {0x31, 0x31}},
	{0x39, 0xBA, 2, {0x31, 0x31}},
	{0x39, 0xBB, 2, {0x31, 0x31}},
	{0x39, 0xBC, 2, {0x31, 0x31}},
	{0x39, 0xBD, 2, {0x31, 0x09}},
	{0x39, 0xBE, 2, {0x08, 0x01}},
	{0x39, 0xBF, 2, {0x2D, 0x10}},
	{0x39, 0xC0, 2, {0x11, 0x12}},
	{0x39, 0xC1, 2, {0x13, 0x14}},
	{0x39, 0xC2, 2, {0x15, 0x16}},
	{0x39, 0xC3, 2, {0x17, 0x00}},
	{0x39, 0xE5, 2, {0x31, 0x31}},
	{0x39, 0xC4, 2, {0x00, 0x17}},
	{0x39, 0xC5, 2, {0x16, 0x15}},
	{0x39, 0xC6, 2, {0x14, 0x13}},
	{0x39, 0xC7, 2, {0x12, 0x11}},
	{0x39, 0xC8, 2, {0x10, 0x2D}},
	{0x39, 0xC9, 2, {0x01, 0x08}},
	{0x39, 0xCA, 2, {0x09, 0x31}},
	{0x39, 0xCB, 2, {0x31, 0x31}},
	{0x39, 0xCC, 2, {0x31, 0x31}},
	{0x39, 0xCD, 2, {0x31, 0x31}},
	{0x39, 0xCE, 2, {0x31, 0x31}},
	{0x39, 0xCF, 2, {0x31, 0x31}},
	{0x39, 0xD0, 2, {0x31, 0x31}},
	{0x39, 0xD1, 2, {0x31, 0x09}},
	{0x39, 0xD2, 2, {0x08, 0x01}},
	{0x39, 0xD3, 2, {0x2D, 0x10}},
	{0x39, 0xD4, 2, {0x11, 0x12}},
	{0x39, 0xD5, 2, {0x13, 0x14}},
	{0x39, 0xD6, 2, {0x15, 0x16}},
	{0x39, 0xD7, 2, {0x17, 0x00}},
	{0x39, 0xE6, 2, {0x31, 0x31}},
	{0x39, 0xD8, 5, {0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x39, 0xD9, 5, {0x00, 0x00, 0x00, 0x00, 0x00}},
	{0x15, 0xE7, 1, {0x00}},

	//======Page3, gate timing control======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x03}},
	{0x39, 0xB0, 2, {0x20, 0x00}},
	{0x39, 0xB1, 2, {0x20, 0x00}},
	{0x39, 0xB2, 5, {0x05, 0x00, 0x42, 0x00, 0x00}},

	{0x39, 0xB6, 5, {0x05, 0x00, 0x42, 0x00, 0x00}},

	{0x39, 0xBA, 5, {0x53, 0x00, 0x42, 0x00, 0x00}},
	{0x39, 0xBB, 5, {0x53, 0x00, 0x42, 0x00, 0x00}},

	{0x15, 0xC4, 1, {0x40}},

	//======Page5======
	{0x39, 0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x05}},
	{0x39, 0xB0, 2, {0x17, 0x06}},
	{0x15, 0xB8, 1, {0x00}},
	{0x39, 0xBD, 5, {0x03, 0x01, 0x01, 0x00, 0x01}},
	{0x39, 0xB1, 2, {0x17, 0x06}},
	{0x39, 0xB9, 2, {0x00, 0x01}},
	{0x39, 0xB2, 2, {0x17, 0x06}},
	{0x39, 0xBA, 2, {0x00, 0x01}},
	{0x39, 0xB3, 2, {0x17, 0x06}},
	{0x39, 0xBB, 2, {0x0A, 0x00}},
	{0x39, 0xB4, 2, {0x17, 0x06}},
	{0x39, 0xB5, 2, {0x17, 0x06}},
	{0x39, 0xB6, 2, {0x14, 0x03}},
	{0x39, 0xB7, 2, {0x00, 0x00}},
	{0x39, 0xBC, 2, {0x02, 0x01}},

	{0x15, 0xC0, 1, {0x05}},
	{0x15, 0xC4, 1, {0xA5}},

	{0x39, 0xC8, 2, {0x03, 0x30}},
	{0x39, 0xC9, 2, {0x03, 0x51}},

	{0x39, 0xD1, 5, {0x00, 0x05, 0x03, 0x00, 0x00}},
	{0x39, 0xD2, 5, {0x00, 0x05, 0x09, 0x00, 0x00}},

	{0x15, 0xE5, 1, {0x02}},
	{0x15, 0xE6, 1, {0x02}},
	{0x15, 0xE7, 1, {0x02}},
	{0x15, 0xE9, 1, {0x02}},
	{0x15, 0xED, 1, {0x33}},

	//======Normal Display======
	{0x05, 0x11, 0, {}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 120, {}},
	{0x05, 0x29, 0, {}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 20, {}},
#else
       {0x15, 0x8c , 1, {0x80}},
	{0x15, 0xC5 , 1, {0x23}},
	{0x15, 0xC7 , 1, {0x23}},
#endif
};
/*****************Mipi Init END************************/

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

#define GPIO_LCD_PWR_EN   GPIO83 //GPIO115//GPIO142
#define GPIO_LCD_RST      GPIO90 //GPIO119
#define GPIO_LCD_STB      GPIO89 //GPIO118
#define GPIO_LCD_BL_EN       GPIO_UNSUPPORTED
#define LCD_RST_VALUE        GPIO_OUT_ZERO

static void lcd_power_en(unsigned char enabled)
{
    if (enabled)
    {      
        mt_set_gpio_out(GPIO_LCM_PWR_EN, GPIO_OUT_ONE);
    }
    else
    {      
        mt_set_gpio_out(GPIO_LCM_PWR_EN, GPIO_OUT_ZERO);
    }
}


static void lcd_stanby(unsigned char enabled)
{
    if (enabled)
    {
        mt_set_gpio_out(GPIO_LCD_STB, GPIO_OUT_ONE);
    }
    else
    {	
        mt_set_gpio_out(GPIO_LCD_STB, GPIO_OUT_ZERO);    	
    }
}

static void lcd_reset(unsigned char enabled)
{
    #ifdef BUILD_LK
    printf("%s, LK RESET=%d\n", __func__);
    #else
    printk("%s, kernel RESET=%d\n", __func__);
    #endif

    if (enabled)
    {
        mt_set_gpio_out(GPIO_LCD_RST, GPIO_OUT_ONE);
    }
    else
    {	
        mt_set_gpio_out(GPIO_LCD_RST, GPIO_OUT_ZERO);    	
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

    params->physical_width=90;
    params->physical_height=152;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;

    // Video mode setting		
    params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 5;
    params->dsi.vertical_backporch					= 25;
    params->dsi.vertical_frontporch 				= 35;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 5;
    params->dsi.horizontal_backporch				= 69;
    params->dsi.horizontal_frontporch				= 80;
    params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;

#if 1
    // Bit rate calculation
    params->dsi.pll_div1 = 0;		
    params->dsi.pll_div2 = 1; 		
    params->dsi.fbk_div  = 21;

    params->dsi.cont_clock = 1;
    params->dsi.horizontal_bllp = 0x115; //word count=0x340
#else
    params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_234;
#endif
}

extern void DSI_clk_HS_mode(unsigned char enter);
static void init_lcm_registers(void)
{
    #ifdef BUILD_LK
    printf("%s, LK \n", __func__);
    #else
    printk("%s, kernel \n", __func__);
    #endif

    dsi_set_cmdq_V3(lcm_initialization_setting,sizeof(lcm_initialization_setting)/sizeof(lcm_initialization_setting[0]),1);
}

static void lcm_init(void)
{
#ifdef BUILD_LK
    #ifdef BUILD_LK
    printf("%s, LK \n", __func__);
    #else
    printk("%s, kernel \n", __func__);
    #endif
   //xmlq debug
    //upmu_set_rg_vgp1_vosel(0x7);
    //upmu_set_rg_vgp1_en(0x1);
    lcd_power_en(0);
    lcd_stanby(0);
    lcd_reset(0);
    upmu_set_rg_vgp1_vosel(0x5);
    upmu_set_rg_vgp1_en(0x1);
    MDELAY(1);
    //lcd_power_en(1);
    lcd_stanby(1);
    MDELAY(1);
    lcd_reset(1);
    init_lcm_registers();
    MDELAY(1);
    lcd_power_en(1);
    MDELAY(40);
    lcd_reset(0);
    MDELAY(1);
    lcd_reset(1);
    #else
    printk("%s, kernel \n", __func__);
    #endif
 
}


static void lcm_suspend(void)
{
    unsigned int data_array[16];

    #ifdef BUILD_LK
    printf("%s, LK \n", __func__);
    #else
    printk("%s, kernel \n", __func__);
    #endif

    data_array[0] = 0x00280500;  //display off
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(1);
    data_array[0] = 0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    lcd_stanby(0);
    MDELAY(150);
    upmu_set_rg_vgp1_en(0);
    upmu_set_rg_vgp1_vosel(0);
    lcd_reset(0);
    lcd_power_en(0);
    DSI_clk_HS_mode(0);
    MDELAY(10);
}


static void lcm_resume(void)
{
    #ifdef BUILD_LK
    printf("%s, LK \n", __func__);
    #else
    printk("%s, kernel \n", __func__);
    #endif
    lcd_power_en(0);
    lcd_stanby(0);
    lcd_reset(0);
    upmu_set_rg_vgp1_vosel(0x5);
    upmu_set_rg_vgp1_en(0x1);
    MDELAY(1);
    //lcd_power_en(1);
    lcd_stanby(1);
    MDELAY(1);
    lcd_reset(1);
    init_lcm_registers();
    MDELAY(37);
    lcd_power_en(1);
    MDELAY(40);
    lcd_reset(0);
    MDELAY(1);
    lcd_reset(1);
    
}

LCM_DRIVER p070acz_3z1_wsvga_dsi_lcm_drv = 
{
       .name		  = "p070acz_3z1_wsvga_dsi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params    = lcm_get_params,
	.init                = lcm_init,
	.suspend         = lcm_suspend,
	.resume           = lcm_resume,
	//.compare_id    = lcm_compare_id,
};

