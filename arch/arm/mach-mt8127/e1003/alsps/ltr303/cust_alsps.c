/*
 * (c) MediaTek Inc. 2010
 */

#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
//	.polling_mode = 1,
// yanggy 2012-07-17 modify for interrupt mode begin
#ifdef GN_MTK_BSP_ALSPS_INTERRUPT_MODE 
    .polling_mode_ps = 0,
#else
    .polling_mode_ps = 1,
#endif
// yanggy 2012-07-17 modify for interrupt mode end
    .polling_mode_als = 1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
    //.als_level  = {5, 7,  9,  13,  16,  21,  25,  40,  120,  400, 1600,  3000, 6000,  10000, 65535},
    //.als_value  = { 1, 2,  4,   9,  16,  39,  80, 150, 300,  650,  1500,  3500,  8500, 15000,  45000, 100000},      //------the old data
    //.als_level  = { 4, 7,   10,   25,  50,  100,  200, 1600,  10000,  65535},
    //.als_value  = { 1, 60,   90,  225, 640, 1280,  2600, 6400, 60000,  100000},
    .als_level  = { 4, 10,  50,   200,  500, 800,  1000, 1200, 1400,  1600, 3000, 6000, 8000, 10000, 65535},
    .als_value  = {0, 60,  90,  120, 160, 225,  640,  1280, 2600,  6400,  10240,  10240,  10240, 60000,  80000, 100000},
    //.als_value  = { 1, 40,   90,  160,  225,  320, 640, 1280,  2600, 10240,  20000,  40000, 60000,  80000, 100000}, //-----the new data
//sxt 2011-8-19 add for threshold begin
#if defined (GN_MTK_BSP)
	.ps_threshold =PS_THRESHOLD,
#else
	.ps_threshold =60,
#endif
//gionee sxt 2011-8-19 add for threshold end
};

struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}



