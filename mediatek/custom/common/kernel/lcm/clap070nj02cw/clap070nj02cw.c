/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


#ifndef BUILD_LK
#include <linux/string.h>
#endif
#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>
#include <platform/upmu_common.h>
#include <debug.h>
#elif (defined BUILD_UBOOT)
#include <asm/arch/mt6577_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/xlog.h>
#include "mt8193_iic.h"
#include <mach/mt_pm_ldo.h>
#include <mach/upmu_common.h>
#endif
#include "lcm_drv.h"
#include "mt8193_lvds.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (1024)
#define FRAME_HEIGHT (600)

#define GPIO_LCD_RST_EN      GPIO119
#define GPIO_LCD_PWR_EN      GPIO115//GPIO8
#define GPIO_LCD_EN      GPIO12//GPIO13

#define HSYNC_PULSE_WIDTH 20 
#define HSYNC_BACK_PORCH  140
#define HSYNC_FRONT_PORCH 160
#define VSYNC_PULSE_WIDTH 3
#define VSYNC_BACK_PORCH  20
#define VSYNC_FRONT_PORCH 12

#define LCD_DATA_FORMAT LCD_DATA_FORMAT_VESA8BIT

#define V_TOTAL (FRAME_HEIGHT + VSYNC_PULSE_WIDTH + VSYNC_BACK_PORCH + VSYNC_FRONT_PORCH)
#define H_TOTAL (FRAME_WIDTH + HSYNC_PULSE_WIDTH + HSYNC_BACK_PORCH + HSYNC_FRONT_PORCH)

#define H_START (HSYNC_PULSE_WIDTH + HSYNC_BACK_PORCH)
#define H_END (H_START + FRAME_WIDTH - 1)

#define V_START (VSYNC_PULSE_WIDTH + VSYNC_BACK_PORCH)
#define V_END (V_START + FRAME_HEIGHT - 1)

#define V_DELAY  0x0002  //Fixed Value
#define H_DELAY  (FRAME_WIDTH + HSYNC_PULSE_WIDTH + HSYNC_BACK_PORCH + HSYNC_FRONT_PORCH - V_DELAY)

#ifdef BUILD_LK
#define MT8193_REG_WRITE(add, data) mt8193_reg_i2c_write(add, data)
#define MT8193_REG_READ(add) mt8193_reg_i2c_read(add)
#elif (defined BUILD_UBOOT)
    // do nothing in uboot
#else
extern int mt8193_i2c_write(u16 addr, u32 data);
extern int mt8193_i2c_read(u16 addr, u32 *data);

#define MT8193_REG_WRITE(add, data) mt8193_i2c_write(add, data)
#define MT8193_REG_READ(add) lcm_mt8193_i2c_read(add)
#endif
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (mt_set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

static __inline void send_ctrl_cmd(unsigned int cmd)
{

}

static __inline void send_data_cmd(unsigned int data)
{

}

static __inline void set_lcm_register(unsigned int regIndex,
                                      unsigned int regData)
{

}

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
    mt_set_gpio_mode(GPIO, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO, (output>0)? GPIO_OUT_ONE: GPIO_OUT_ZERO);
}
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

#ifdef BUILD_LK
#define I2C_CH                0
#define MT8193_I2C_ADDR       0x3A
int mt8193_reg_i2c_write(u16 addr, u32 data)
{
    u8 buffer[8];
    u8 lens;
	U32 ret_code = 0;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = (data >> 24) & 0xFF;
        buffer[2] = (data >> 16) & 0xFF;
        buffer[3] = (data >> 8) & 0xFF;
        buffer[4] = data & 0xFF;
        lens = 5;
    }
    else // 16 bit : noraml mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        buffer[1] = addr & 0xFF;
        buffer[2] = (data >> 24) & 0xFF;
        buffer[3] = (data >> 16) & 0xFF;
        buffer[4] = (data >> 8) & 0xFF;
        buffer[5] = data & 0xFF;        
        lens = 6;
    }

	ret_code = mt_i2c_write(I2C_CH, MT8193_I2C_ADDR, buffer, lens, 0); // 0:I2C_PATH_NORMAL
    if (ret_code != 0)
    {
        printf("[LK/LCM] mt_i2c_write reg[0x%X] fail, Error code [0x%X] \n", addr, ret_code);
        return ret_code;
    }
	
	return 0;
}

u32 mt8193_reg_i2c_read(u16 addr)
{
    
    u8 buffer[8] = {0};
    u8 lens;
    u32 ret_code = 0;
    u32 data;

    if(((addr >> 8) & 0xFF) >= 0x80) // 8 bit : fast mode
    {
        buffer[0] = (addr >> 8) & 0xFF;
        lens = 1;
    }
    else // 16 bit : noraml mode
    {
        buffer[0] = ( addr >> 8 ) & 0xFF;
        buffer[1] = addr & 0xFF;     
        lens = 2;
    }

    ret_code = mt_i2c_write(I2C_CH, MT8193_I2C_ADDR, buffer, lens, 0);    // set register command
    if (ret_code != 0)
        return ret_code;

    lens = 4;
    ret_code = mt_i2c_read(I2C_CH, MT8193_I2C_ADDR, buffer, lens, 0);
    if (ret_code != 0)
    {
        return ret_code;
    }
    
    data = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | (buffer[0]); //LSB fisrt

    return data;
    
}
#elif (defined BUILD_UBOOT)
    // do nothing in uboot
#else
u32 lcm_mt8193_i2c_read(u16 addr)
{
   u32 u4Reg = 0;
   u32 ret_code = 0;
	
   ret_code = mt8193_i2c_read(addr, &u4Reg);
   if (ret_code != 0)
   {
	   return ret_code;
   }

   return u4Reg;
}
#endif

static void lcm_mt8193_TTL_func1_init(void)
{
    MT8193_REG_WRITE(0x1220, 0x00009249);
    MT8193_REG_WRITE(0x1204, 0x09200000);
    MT8193_REG_WRITE(0x1208, 0x00249249);
    MT8193_REG_WRITE(0x120c, 0x00000000);	
    MT8193_REG_WRITE(0x124c, 0x807e07ff);
    MT8193_REG_WRITE(0x1224, 0x00000000);	
/*	MT8193_REG_WRITE(0x122c, 0x00000002);
	MT8193_REG_WRITE(0x1230, 0x00000002); 
	MT8193_REG_WRITE(0x1260, 0x00000000);	 */ //xmlq test1 base on FY7018D01H.c
	MDELAY(10);
}///FOR LCM use NAND PIN

static void lcm_mt8193_TTL_func2_init(void)
{
    MT8193_REG_WRITE(0x1220, 0x09240000);
    MT8193_REG_WRITE(0x1204, 0x12400000);
    MT8193_REG_WRITE(0x1208, 0x09490492);
    MT8193_REG_WRITE(0x120c, 0x00001249);	
    MT8193_REG_WRITE(0x124c, 0x8081ff7f);
    MT8193_REG_WRITE(0x1224, 0x00049249);	
    MT8193_REG_WRITE(0x122c, 0x00000040);
    MT8193_REG_WRITE(0x1230, 0x00000040);	
//    MT8193_REG_WRITE(0x1258, 0x00000000);	//xmlq test1 base on FY7018D01H.c
	MDELAY(10);
}///FOR LCM use HDMI PIN

static void lcm_mt8193_set_ckgen(void)
{
    u32 u4Reg = 0;
	
    u4Reg = MT8193_REG_READ(REG_PLL_GPANACFG0);
	u4Reg&=(~(0x7c000000)); //clear bit 26~30
    u4Reg |= (RG_PLL1_FBDIV | RG_PLL1_PREDIV | RG_PLL1_RST_DLY | RG_PLL1_LF | RG_PLL1_MONCKEN | RG_PLL1_VODEN | RG_NFIPLL_EN);
    MT8193_REG_WRITE(REG_PLL_GPANACFG0, u4Reg);
	
	u4Reg = MT8193_REG_READ(RG_LVDSWRAP_CTRL1);
	u4Reg&=(~(0x0000000f)); //clear bit 0~3
	u4Reg |= (RG_DCXO_POR_MON_EN | RG_PLL1_DIV);
	MT8193_REG_WRITE(RG_LVDSWRAP_CTRL1, u4Reg);
	UDELAY(200);

    MT8193_REG_WRITE(REG_LVDS_ANACFG2, (RG_VPLL_BC | RG_VPLL_BIC | RG_VPLL_BIR | RG_VPLL_BP | RG_VPLL_BR));
	
	u4Reg = 0;
	u4Reg |= (RG_VPLL_DIV | RG_VPLL_DPIX_CKSEL | RG_VPLL_MKVCO |RG_VPLL_POSTDIV_EN);
	MT8193_REG_WRITE(REG_LVDS_ANACFG3, u4Reg);
	UDELAY(200);
	u4Reg &= (~(RG_VPLL_POSTDIV_EN));
	MT8193_REG_WRITE(REG_LVDS_ANACFG3, u4Reg);	
}

static void lcm_mt8193_set_lvds_analog(void)
{
    MT8193_REG_WRITE(REG_LVDS_ANACFG4, (RG_T2TTLO_EN | RG_BYPASS | RG_LVDS_BYPASS));
    MT8193_REG_WRITE(REG_LVDS_ANACFG0, 0xfc70c3ff);
    MT8193_REG_WRITE(REG_LVDS_ANACFG1, 0x93ff0000);
}

static void lcm_mt8193_ckgen_power_on(void)
{
    u32 u4Reg = 0;

	u4Reg = MT8193_REG_READ(REG_PLL_GPANACFG0);
	u4Reg |= RG_PLL1_EN;
	MT8193_REG_WRITE(REG_PLL_GPANACFG0, u4Reg);	
    MDELAY(1);	
	
	u4Reg = MT8193_REG_READ(REG_LVDS_ANACFG2);
	u4Reg &= (~(RG_VPLL_BG_PD | RG_VPLL_BIAS_PD));
	MT8193_REG_WRITE(REG_LVDS_ANACFG2, u4Reg);	
    MDELAY(1);

	u4Reg = MT8193_REG_READ(REG_LVDS_ANACFG4);
	u4Reg &= (~(RG_VPLL_PD));
	MT8193_REG_WRITE(REG_LVDS_ANACFG4, u4Reg);
	MDELAY(1);
	u4Reg &= (~(RG_VPLL_RST));
	MT8193_REG_WRITE(REG_LVDS_ANACFG4, u4Reg);
}

static void lcm_mt8193_ckgen_power_off(void)
{
    u32 u4Reg = 0;

	u4Reg = MT8193_REG_READ(REG_LVDS_ANACFG4);
	u4Reg |= (RG_VPLL_RST);
	MT8193_REG_WRITE(REG_LVDS_ANACFG4, u4Reg);
	MDELAY(1);
	u4Reg |= (RG_VPLL_PD);
	MT8193_REG_WRITE(REG_LVDS_ANACFG4, u4Reg);
	UDELAY(200);
	
	u4Reg = MT8193_REG_READ(REG_LVDS_ANACFG2);
	u4Reg |= (RG_VPLL_BG_PD | RG_VPLL_BIAS_PD);
	MT8193_REG_WRITE(REG_LVDS_ANACFG2, u4Reg);	
	MDELAY(1);
	
	u4Reg = MT8193_REG_READ(REG_PLL_GPANACFG0);
	u4Reg &= (~(RG_PLL1_EN));
	MT8193_REG_WRITE(REG_PLL_GPANACFG0, u4Reg);
}

static void lcm_mt8193_lvds_analog_power_on(void)
{
    u32 u4Reg = 0;

	u4Reg = MT8193_REG_READ(REG_LVDS_ANACFG0);		
	u4Reg &= (~(RG_LVDS_APD | RG_LVDS_BIASA_PD));
	MT8193_REG_WRITE(REG_LVDS_ANACFG0, u4Reg);
	UDELAY(200);
}

static void lcm_mt8193_lvds_analog_power_off(void)
{
    u32 u4Reg = 0;

	u4Reg = MT8193_REG_READ(REG_LVDS_ANACFG0);
	u4Reg |= (RG_LVDS_APD | RG_LVDS_BIASA_PD);
	MT8193_REG_WRITE(REG_LVDS_ANACFG0, u4Reg);
	UDELAY(200);
}

static void lcm_mt8193_set_lvds_digital(void)
{
    MT8193_REG_WRITE(LVDS_CLK_CTRL, (RG_TEST_CK_EN | RG_RX_CK_EN | RG_TX_CK_EN));
	MT8193_REG_WRITE(LVDS_CH_SWAP, RG_SWAP_SEL);
	MT8193_REG_WRITE(LVDS_CLK_RESET, (RG_CTSCLK_RESET_B | RG_PCLK_RESET_B));
   MT8193_REG_WRITE(LVDS_FMT_CTRL, RG_6BIT_FORMAT);//Added by Evan_zeng @2013-12-3 for adjusting the 6bit mode
}

static void lcm_mt8193_set_dgi0(void)
{	
	MT8193_REG_WRITE(DGI0_DATA_OUT_CTRL, DATA_OUT_SWAP);
	MT8193_REG_WRITE(DGI0_TG_CTRL00, PRGS_OUT);
	MT8193_REG_WRITE(DGI0_TG_CTRL02, ((V_TOTAL<<16) | H_TOTAL));
	MT8193_REG_WRITE(DGI0_TG_CTRL03, ((VSYNC_PULSE_WIDTH<<16) | HSYNC_PULSE_WIDTH));
	MT8193_REG_WRITE(DGI0_TG_CTRL05, ((H_START<<16) | H_END));	
	MT8193_REG_WRITE(DGI0_TG_CTRL06, ((V_START<<16) | V_END));
	MT8193_REG_WRITE(DGI0_TG_CTRL07, ((V_START<<16) | V_END));	
	MT8193_REG_WRITE(DGI0_TG_CTRL01, ((V_DELAY<<16) | H_DELAY));	
	MT8193_REG_WRITE(DGI0_TTL_ANAIF_CTRL, TTL_CLK_INV_ENABLE);
	//MT8193_REG_WRITE(DGI0_DITHER_CTRL0, (FRC_EN | FCNT_DIF_EN | SYNC_SEL | OUT_FMT));//for 18bit LCM
}

static void lcm_mt8193_set_dgi0_func1(void)
{
	//MT8193_REG_WRITE(DGI0_DATA_OUT_CTRL, (DATA_OUT_SWAP | TTL_TIM_SWAP));  //HSYNC -> DE
	MT8193_REG_WRITE(DGI0_DATA_OUT_CTRL, (DATA_OUT_SWAP | TTL_TIM_SWAP2)); //VSYNC -> DE	
	MT8193_REG_WRITE(DGI0_TTL_ANAIF_CTRL1, PAD_TTL_EN_PP);
}///FOR LCM use NAND PIN

static void lcm_mt8193_set_dgi0_func2(void)
{
	MT8193_REG_WRITE(DGI0_TTL_ANAIF_CTRL1, (PAD_TTL_EN_PP | PAD_TTL_EN_FUN_SEL));
}///FOR LCM use HDMI PIN

static void lcm_mt8193_anaif_clock_enable(void)
{
    MT8193_REG_WRITE(DGI0_ANAIF_CTRL0, DGI0_PAD_CLK_ENABLE);
}

static void lcm_mt8193_anaif_clock_disable(void)
{
    MT8193_REG_WRITE(DGI0_ANAIF_CTRL0, DGI0_PAD_CLK_DISABLE);
}

static void lcm_mt8193_dgi0_clock_enable(void)
{
    MT8193_REG_WRITE(DGI0_CLK_RST_CTRL, (DGI0_CLK_OUT_ENABLE | DGI0_CLK_IN_INV_ENABLE | DGI0_CLK_IN_ENABLE));
}

static void lcm_mt8193_dgi0_clock_disable(void)
{
    MT8193_REG_WRITE(DGI0_CLK_RST_CTRL, DGI0_CLK_OUT_DISABLE);
}

static void lcm_mt8193_dgi0_fifo_write_disable(void)
{
    MT8193_REG_WRITE(DGI0_DEC_CTRL, 0x0);
}

static void lcm_mt8193_dgi0_fifo_write_enable(void)
{
    MT8193_REG_WRITE(DGI0_DEC_CTRL, FIFO_WRITE_EN);
}

static void lcm_mt8193_sw_reset(void)
{
    MT8193_REG_WRITE(DGI0_FIFO_CTRL, (SW_RST | FIFO_RESET_ON | RD_START));
	MDELAY(1);
	MT8193_REG_WRITE(DGI0_FIFO_CTRL, (FIFO_RESET_ON | RD_START));
}

static void lcm_mt8193_lvds_power_off(void)
{
    MT8193_REG_WRITE(REG_LVDS_PWR_CTRL, 0x00000006);
	MDELAY(1);
	MT8193_REG_WRITE(REG_LVDS_PWR_CTRL, 0x00000007);
	MDELAY(1);
    MT8193_REG_WRITE(REG_LVDS_PWR_RST_B, 0x00000000);
	MDELAY(1);
	MT8193_REG_WRITE(REG_LVDS_PWR_CTRL, 0x00000005);
}

static void lcm_mt8193_lvds_power_on(void)
{
    MT8193_REG_WRITE(REG_LVDS_PWR_CTRL, 0x00000007);
	MDELAY(1);
	MT8193_REG_WRITE(REG_LVDS_PWR_RST_B, 0x00000001);
	MDELAY(1);
    MT8193_REG_WRITE(REG_LVDS_PWR_CTRL, 0x00000006);
	MDELAY(1);
	MT8193_REG_WRITE(REG_LVDS_PWR_CTRL, 0x00000002);
}

static void lcm_mt8193_lvds_top_clock_disable(void)
{
    MT8193_REG_WRITE(LVDS_CLK_CTRL, 0x0);
}

static void lcm_mt8193_lvds_top_clock_enable(void)
{
    MT8193_REG_WRITE(LVDS_CLK_CTRL, (RG_TEST_CK_EN | RG_RX_CK_EN | RG_TX_CK_EN));
}

static void lcm_mt8193_lvds_out_disable(void)
{
    MT8193_REG_WRITE(LVDS_OUTPUT_CTRL, 0x0);
}

static void lcm_mt8193_lvds_out_enable(void)
{
	MT8193_REG_WRITE(LVDS_OUTPUT_CTRL, (RG_LVDSRX_FIFO_EN | RG_SYNC_TRIG_MODE | RG_OUT_FIFO_EN | RG_LVDS_E));
}

static void lcm_mt8193_TTL_func1_disable(void)
{ 
    MT8193_REG_WRITE(0x1208, 0x249); 
//    MT8193_REG_WRITE(0x1260, 0xA8);//xmlq test1 base on FY7018D01H.c
}///FOR LCM use NAND PIN

static void lcm_mt8193_TTL_func2_disable(void)
{
    MT8193_REG_WRITE(0x120c, 0x1);
//    MT8193_REG_WRITE(0x1258, 0x100800);//xmlq test1 base on FY7018D01H.c
}///FOR LCM use HDMI PIN

//Added by EvanZeng @2013-01-08 for adjust the lcd's timing
static void lcm_mt8193_enable_output1(void)
{
   lcm_mt8193_TTL_func1_init();
	lcm_mt8193_lvds_power_on();		
	lcm_mt8193_anaif_clock_enable();
	lcm_mt8193_set_ckgen();
	lcm_mt8193_ckgen_power_on();
	lcm_mt8193_dgi0_clock_enable();
	lcm_mt8193_dgi0_fifo_write_disable();
	lcm_mt8193_set_dgi0();
	lcm_mt8193_set_dgi0_func1();
//        lcm_mt8193_set_dgi0_func2();
	lcm_mt8193_dgi0_fifo_write_enable();
	lcm_mt8193_lvds_top_clock_enable();
	lcm_mt8193_lvds_out_disable();
	lcm_mt8193_set_lvds_digital();
	lcm_mt8193_lvds_analog_power_on();
	lcm_mt8193_set_lvds_analog();
}
//Modified by EvanZeng @2013-01-08 for adjust the lcd's timing
static void lcm_mt8193_enable_output(void)
{
	//MDELAY(1);
	lcm_mt8193_lvds_out_enable();	
	lcm_mt8193_sw_reset();  
}

static void lcm_mt8193_disable_output(void)
{
	lcm_mt8193_TTL_func1_disable();
  //lcm_mt8193_TTL_func2_disable();
	lcm_mt8193_lvds_analog_power_off();		
	lcm_mt8193_lvds_out_disable();
	lcm_mt8193_lvds_top_clock_disable();
	lcm_mt8193_dgi0_fifo_write_disable();
    lcm_mt8193_dgi0_clock_disable();
	lcm_mt8193_ckgen_power_off();	
    lcm_mt8193_anaif_clock_disable();
    lcm_mt8193_lvds_power_off(); 
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DPI;
    params->ctrl   = LCM_CTRL_SERIAL_DBI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    params->io_select_mode = 0;	

// div0_real = div0==0 ? 1:
//          div0==1 ? 2: 4;
// div1_real = div1==0 ? 1:
//          div1==1 ? 2: 4;
// freq = 26*mipi_pll_clk_ref/2^24/div0_real/div1_real/8

    /* RGB interface configurations */
    params->dpi.mipi_pll_clk_ref  = 528611359;
    params->dpi.mipi_pll_clk_div1 = 0;  // div0=0,1,2,3;div0_real=1,2,4,4
    params->dpi.mipi_pll_clk_div2 = 0;  // div1=0,1,2,3;div1_real=1,2,4,4
    params->dpi.dpi_clk_div       = 4;           //{4,2}, pll/4=51.025M
    params->dpi.dpi_clk_duty      = 2;

    params->dpi.clk_pol           = LCM_POLARITY_RISING;
    params->dpi.de_pol            = LCM_POLARITY_RISING;
//xmlq test1 base on FY7018D01H.c
    params->dpi.vsync_pol = LCM_POLARITY_FALLING;
    params->dpi.hsync_pol = LCM_POLARITY_FALLING;
/*    params->dpi.vsync_pol         = LCM_POLARITY_RISING;//LCM_POLARITY_FALLING;
    params->dpi.hsync_pol         = LCM_POLARITY_RISING;//LCM_POLARITY_FALLING;*/

    params->dpi.hsync_pulse_width = HSYNC_PULSE_WIDTH;
    params->dpi.hsync_back_porch  = HSYNC_BACK_PORCH;
    params->dpi.hsync_front_porch = HSYNC_FRONT_PORCH;
    params->dpi.vsync_pulse_width = VSYNC_PULSE_WIDTH;
    params->dpi.vsync_back_porch  = VSYNC_BACK_PORCH;
    params->dpi.vsync_front_porch = VSYNC_FRONT_PORCH;
    
    params->dpi.i2x_en = 1;
    
    params->dpi.format            = LCM_DPI_FORMAT_RGB666;   // Modified by Evan_zeng @2013-12-3 for adjusting the 6bit mode
    params->dpi.rgb_order         = LCM_COLOR_ORDER_RGB;
    params->dpi.is_serial_output  = 0;

    params->dpi.intermediat_buffer_num = 0;

    params->dpi.io_driving_current = LCM_DRIVING_CURRENT_6575_12MA;
    //params->dpi.io_driving_current = LCM_DRIVING_CURRENT_2MA;
}

static void lcm_init(void)
{
#ifdef BUILD_LK
	printf("===xmlq_v01:[LK/LCM] lcm_init() enter++++ \n");
	//VGP6 3.3V
	// pmic_config_interface(DIGLDO_CON12, 0x1, PMIC_RG_VGP6_EN_MASK, PMIC_RG_VGP6_EN_SHIFT); 
	//  pmic_config_interface(DIGLDO_CON33, 0x07, PMIC_RG_VGP6_VOSEL_MASK, PMIC_RG_VGP6_VOSEL_SHIFT);
	upmu_set_rg_vgp1_vosel(0x7);
	upmu_set_rg_vgp1_en(0x1);
   //xmlq test1 base on FY7018D01H.c
   MDELAY(1);
   MT8193_REG_WRITE(LVDS_OUTPUT_CTRL, (RG_LVDSRX_FIFO_EN | RG_SYNC_TRIG_MODE | RG_OUT_FIFO_EN | RG_LVDS_E));
	MDELAY(30);

   //Added by EvanZeng @2013-01-08 for adjust the lcd's timing
   lcm_mt8193_enable_output1();

	//lcm_set_gpio_output(GPIO_LCD_PWR_EN, 1);
   /*mt_set_gpio_mode(GPIO_LCD_PWR_EN, GPIO_MODE_00);
     mt_set_gpio_dir(GPIO_LCD_PWR_EN, GPIO_DIR_OUT);
     mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);*/
   mt_set_gpio_mode(GPIO_LCM_PWR, GPIO_MODE_00);
   mt_set_gpio_dir(GPIO_LCM_PWR, GPIO_DIR_OUT);
   mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ONE);
//	MDELAY(6);

	lcm_mt8193_enable_output();
//	MDELAY(210);
	/*[A]fix lcd display not work when power on*/
	lcm_mt8193_set_ckgen();
	lcm_mt8193_ckgen_power_on();
	/*[A] END, bugid 15346, xmtzqh, 20131206*/

//modify accord hw engineer request to avoid white screen when bootup
#ifdef GPIO_LCM_RST
   mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
   mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
   mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
   MDELAY(5);
   mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
#endif
//modify end
	//lcm_set_gpio_output(GPIO_LCD_EN, 1);
	MDELAY(200);
#elif (defined BUILD_UBOOT)
    // do nothing in uboot
#else
    printk("[LCM] lcm_init() enter\n");

#endif
}


static void lcm_suspend(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_suspend() enter+++\n");
	//lcm_set_gpio_output(GPIO_LCM_PWR, 0);
	MDELAY(150);
	lcm_mt8193_disable_output();
	MDELAY(3);

	lcm_set_gpio_output(GPIO_LCM_PWR, 0);
	MDELAY(20);   

	//VGP6 3.3V
	////pmic_config_interface(DIGLDO_CON12, 0x0, PMIC_RG_VGP6_EN_MASK, PMIC_RG_VGP6_EN_SHIFT); 
	//// pmic_config_interface(DIGLDO_CON33, 0x0, PMIC_RG_VGP6_VOSEL_MASK, PMIC_RG_VGP6_VOSEL_SHIFT);

	upmu_set_rg_vgp1_vosel(0);
	upmu_set_rg_vgp1_en(0);
   MDELAY(5); 
#elif (defined BUILD_UBOOT)
		// do nothing in uboot
#else
	printk("[LCM] lcm_suspend() enter\n");
	//lcm_set_gpio_output(GPIO_LCM_PWR, 0);
	MDELAY(200);
	lcm_mt8193_disable_output();
	MDELAY(3);  

	lcm_set_gpio_output(GPIO_LCM_PWR, 0);
	MDELAY(20);  

	upmu_set_rg_vgp1_vosel(0x0);    
	upmu_set_rg_vgp1_en(0x0);
   MDELAY(5); 

#endif

}


static void lcm_resume(void)
{
#ifdef BUILD_LK
		// do nothing in LK
	printf("[LK/LCM] lcm_resume() enter\n");		
	upmu_set_rg_vgp1_vosel(0x7);
	upmu_set_rg_vgp1_en(0x1);
	MDELAY(30);

	//lcm_set_gpio_output(GPIO_LCD_PWR_EN, 1);
        mt_set_gpio_mode(GPIO_LCM_PWR, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCM_PWR, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ONE);
	MDELAY(30);

	lcm_mt8193_enable_output();
	MDELAY(210);

	//lcm_set_gpio_output(GPIO_LCD_EN, 1);
	//MDELAY(30);
#elif (defined BUILD_UBOOT)
		// do nothing in uboot
#else
	printk("[LCM] lcm_resume() enter\n");

	upmu_set_rg_vgp1_vosel(0x7);
	upmu_set_rg_vgp1_en(0x1);
//xmlq test1 base on FY7018D01H.c
MDELAY(1);  
MT8193_REG_WRITE(LVDS_OUTPUT_CTRL, (RG_LVDSRX_FIFO_EN | RG_SYNC_TRIG_MODE | RG_OUT_FIFO_EN | RG_LVDS_E)); 
	MDELAY(30);

   //Added by EvanZeng @2013-01-08 for adjust the lcd's timing
   lcm_mt8193_enable_output1();

	//lcm_set_gpio_output(GPIO_LCD_PWR_EN, 1);
        mt_set_gpio_mode(GPIO_LCM_PWR, GPIO_MODE_00);
        mt_set_gpio_dir(GPIO_LCM_PWR, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ONE);
	MDELAY(6);

	lcm_mt8193_enable_output();
	MDELAY(200);

	//lcm_set_gpio_output(GPIO_LCD_EN, 1);
	//MDELAY(30);
#endif

}

LCM_DRIVER clap070nj02cw_lcm_drv = 
{
    .name		= "clap070nj02cw",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
};
