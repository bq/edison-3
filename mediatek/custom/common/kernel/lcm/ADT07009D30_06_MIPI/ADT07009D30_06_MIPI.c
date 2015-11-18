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

#define FRAME_WIDTH  (1024)
#define FRAME_HEIGHT (600)

#define LCM_DSI_CMD_MODE    0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {
    .set_gpio_out = NULL,
};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n)           (lcm_util.udelay(n))
#define MDELAY(n)           (lcm_util.mdelay(n))

/*[M]For different lcd poweron logic,ArthurLin,20130809*/
#define GPIO_LCD_PWR_EN		GPIO115
#define GPIO_LCD_STB      GPIO118
#define GPIO_LCD_RST      GPIO119
#define GPIO_LCD_BL_EN		GPIO_UNSUPPORTED
#define LCD_STB_VALUE			GPIO_OUT_ONE
#define LCD_RST_VALUE			GPIO_OUT_ONE
/*END,ArthurLin,20130809*/

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

	//must use 0x39 for init setting for all register.
	//======Internal setting======
	{0x05, 0x01, 0, {}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 30, {}},

	{0x15, 0x80, 1, {0x47}},
	{0x15, 0x81, 1, {0x40}},
	{0x15, 0x82, 1, {0x04}},
	{0x15, 0x83, 1, {0x77}},
	{0x15, 0x84, 1, {0x0F}},
	{0x15, 0x85, 1, {0x70}},
	{0x15, 0x86, 1, {0x70}},
};
/*****************Mipi Init END************************/

static void lcd_vcc3v3_en(unsigned char enabled)
{
    if (enabled)
    {      
	      upmu_set_rg_vgp1_vosel(0x3);//(0x7);
	      upmu_set_rg_vgp1_en(0x1);
    }
    else
    {      
	      upmu_set_rg_vgp1_en(0x0);
	      upmu_set_rg_vgp3_en(0x0);
    }
}

static void lcd_power_en(unsigned char enabled)
{
    if (enabled)
    {      
	      mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);
    }
    else
    {      
	      mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ZERO);
    }
}

static void lcd_stanby(unsigned char enabled)
{
    if(enabled)
    {
    	  mt_set_gpio_mode(GPIO_LCD_STB, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_STB, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_STB, LCD_STB_VALUE); //[M]For different reset gpio level,ArthurLin,20130809
    }
    else
    {	
        mt_set_gpio_mode(GPIO_LCD_STB, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCD_STB, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCD_STB, !LCD_STB_VALUE); //[M]For different reset gpio level,ArthurLin,20130809
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

    params->physical_width = 154; // 154 mm
    params->physical_height = 86; // 86 mm

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;

    // Video mode setting		
    params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 1;
    params->dsi.vertical_backporch					= 23;
    params->dsi.vertical_frontporch 				= 12;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 10;
    params->dsi.horizontal_backporch				= 160;
    params->dsi.horizontal_frontporch				= 160;
    params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;

    // Bit rate calculation
    params->dsi.pll_div1=0;
    params->dsi.pll_div2=1;
    params->dsi.fbk_div  = 15;//21;
    //params->dsi.fbk_sel =0;

    params->dsi.cont_clock = 1;
    params->dsi.horizontal_bllp = 0x115; //word count=0x340
    //params->dsi.PLL_CLOCK = 234;
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
    printf("%s, LK \n", __func__);

    lcd_power_en(0);
    lcd_vcc3v3_en(0);
    lcd_stanby(0);
    lcd_reset(0);
    MDELAY(1);

    lcd_vcc3v3_en(1);
    MDELAY(5);
    lcd_stanby(1);
    MDELAY(5);
    lcd_reset(1);
    MDELAY(1);
    lcd_power_en(1);
    MDELAY(10);
    lcd_reset(0);
    MDELAY(10);
    lcd_reset(1);
    init_lcm_registers();
    MDELAY(40);
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

    lcd_stanby(1);
    MDELAY(80);
    lcd_power_en(0);
    MDELAY(40);
    lcd_vcc3v3_en(0);
    DSI_clk_HS_mode(0);
    MDELAY(5);
}

static void lcm_resume(void)
{
#ifdef BUILD_LK
    printf("%s, LK \n", __func__);
#else
    printk("%s, kernel \n", __func__);
#endif
    lcd_power_en(0);
    lcd_vcc3v3_en(0);
    lcd_stanby(0);
    lcd_reset(0);
    MDELAY(1);
    lcd_vcc3v3_en(1);
    MDELAY(5);
    lcd_stanby(1);
    MDELAY(5);
    lcd_reset(1);
    MDELAY(1);
    lcd_power_en(1);
    MDELAY(10);
    lcd_reset(0);
    MDELAY(10);
    lcd_reset(1);
    init_lcm_registers();
    MDELAY(40);
}

LCM_DRIVER adt07009d30_06_mipi_lcm_drv = 
{
	.name           = "adt07009d30_06_mipi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	//.compare_id    = lcm_compare_id,
};