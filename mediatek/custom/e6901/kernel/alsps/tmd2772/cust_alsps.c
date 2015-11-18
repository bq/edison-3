#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
//#include <mach/mt6575_pm_ldo.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    /*Lenovo-sw chenlj2 add 2011-06-03,modify parameter below two lines*/
    .als_level  = { 1,  20  ,200,300,400,550,  650,750,1000,1200,1500,   2000,  5000, 10000, 65535},
    .als_value  = {50,200,200,200,200,200,  800,800,800  ,800,  800,    10240,10240,10240, 10240, 10240},
    //.als_level  = { 1,20  ,200,300,400,550,  650,750,1000,1200,1500,   2000,5000,10000, 65535},
    //.als_value  = {0,200,250,300,350,400,  500,600,750  ,2000,3000,   4000,6000,9000,  10240, 10240},
    .ps_threshold_high = 980,
    .ps_threshold_low = 930,
    .ps_threshold = 980,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

int TMD2771_CMM_PPCOUNT_VALUE = 0x04;//0x20;//FAE:0x0D,
int ZOOM_TIME = 11;
int TMD2771_CMM_CONTROL_VALUE = 0x20;//0xE0-12.5mA,0xA0-25mA,0x60-50mA,   FAE:0x60;
int TMD2771_CMM_OFFSET_VALUE = (0x80|0x0D);