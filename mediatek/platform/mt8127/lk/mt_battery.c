#include <target/board.h>
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#define CFG_POWER_CHARGING
#endif
#ifdef CFG_POWER_CHARGING
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/boot_mode.h>
#include <platform/mt_gpt.h>
#include <platform/mt_rtc.h>
#include <platform/mt_disp_drv.h>
#include <platform/mtk_wdt.h>
#include <platform/mtk_key.h>
#include <platform/mt_logo.h>
#include <platform/mt_leds.h>
#include <platform/mt_gpio.h>
#include <printf.h>
#include <sys/types.h>
#include <target/cust_battery.h>
//#undef printf


/*****************************************************************************
 *  Type define
 ****************************************************************************/
#define BATTERY_LOWVOL_THRESOLD             3450


/*****************************************************************************
 *  Global Variable
 ****************************************************************************/
bool g_boot_reason_change = false;

int g_R_BAT_SENSE = R_BAT_SENSE;
int g_R_I_SENSE = R_I_SENSE;
int g_R_CHARGER_1 = R_CHARGER_1;
int g_R_CHARGER_2 = R_CHARGER_2;
/*****************************************************************************
 *  Externl Variable
 ****************************************************************************/
extern bool g_boot_menu;

#if defined(MTK_POWER_EXT_DETECT)

#define GPIO_PHONE_EVB_DETECT (GPIO143|0x80000000)

enum mt_board_type {    
	MT_BOARD_NONE = 0,
	MT_BOARD_EVB = 1,    
	MT_BOARD_PHONE = 2
};

int mt_get_board_type(void) 
{
#if 1
	 static int board_type = MT_BOARD_NONE;

	 if (board_type != MT_BOARD_NONE)
	 	return board_type;

	 /* Enable AUX_IN0 as GPI */
	 mt_set_gpio_ies(GPIO_PHONE_EVB_DETECT, GPIO_IES_ENABLE);
 
	 /* Set internal pull-down for AUX_IN0 */
	 mt_set_gpio_pull_select(GPIO_PHONE_EVB_DETECT, GPIO_PULL_DOWN);
	 mt_set_gpio_pull_enable(GPIO_PHONE_EVB_DETECT, GPIO_PULL_ENABLE);
 
	 /* Wait 20us */
	 udelay(20);
 
	 /* Read AUX_INO's GPI value*/
	 mt_set_gpio_mode(GPIO_PHONE_EVB_DETECT, GPIO_MODE_00);
	 mt_set_gpio_dir(GPIO_PHONE_EVB_DETECT, GPIO_DIR_IN);

	 if (mt_get_gpio_in(GPIO_PHONE_EVB_DETECT) == 1) {
		 /* Disable internal pull-down if external pull-up on PCB(leakage) */
		 mt_set_gpio_pull_enable(GPIO_PHONE_EVB_DETECT, GPIO_PULL_DISABLE);
		 board_type = MT_BOARD_EVB;
	 } else {
	 	 /* Disable internal pull-down if external pull-up on PCB(leakage) */
		 mt_set_gpio_pull_enable(GPIO_PHONE_EVB_DETECT, GPIO_PULL_DISABLE);
		 board_type = MT_BOARD_PHONE;
	 }
	 printf("[LK] Board type is %s\n", (board_type == MT_BOARD_EVB) ? "EVB" : "PHONE");
	 return board_type;
#else
	 return MT_BOARD_EVB;
#endif
 }
#endif //MTK_POWER_EXT_DETECT

#ifdef MTK_FAN5405_SUPPORT
extern void fan5405_hw_init(void);
extern void fan5405_turn_on_charging(void);
extern void fan5405_dump_register(void);
#endif

#ifdef MTK_BQ24297_SUPPORT
extern void bq24297_hw_init(void);
extern void bq24297_turn_on_charging();
extern void bq24297_dump_register(void);
#endif

#ifdef MTK_BQ24296_SUPPORT
extern void bq24296_hw_init(void);
extern void bq24296_turn_on_charging(void);
extern void bq24296_dump_register(void);
extern kal_uint32 bq24296_get_vsys_stat(void);
#endif

#ifdef MTK_BQ24196_SUPPORT
extern void bq24196_hw_init(void);
extern void bq24196_charging_enable(UINT8 bEnable);
#endif

#ifdef MTK_BQ24158_SUPPORT
extern void bq24158_hw_init(void);
extern void bq24158_turn_on_charging(void);
extern void bq24158_dump_register(void);
#endif

#ifdef MTK_NCP1851_SUPPORT
extern void ncp1851_hw_init(void);
extern void ncp1851_turn_on_charging(void);
extern void ncp1851_dump_register(void);
#endif

#ifdef MTK_NCP1854_SUPPORT
extern void ncp1854_hw_init(void);
extern void ncp1854_turn_on_charging(void);
extern void ncp1854_dump_register(void);
#endif

void kick_charger_wdt(void)
{
    upmu_set_rg_chrwdt_td(0x0);           // CHRWDT_TD, 4s
    upmu_set_rg_chrwdt_wr(1); 			  // CHRWDT_WR
    upmu_set_rg_chrwdt_int_en(1);         // CHRWDT_INT_EN
    upmu_set_rg_chrwdt_en(1);             // CHRWDT_EN
    upmu_set_rg_chrwdt_flag_wr(1);        // CHRWDT_WR
}

kal_bool is_low_battery(kal_int32  val)
{    
    static UINT8 g_bat_low = 0xFF;

    //low battery only justice once in lk
    if(0xFF != g_bat_low)
	{
        return g_bat_low;
	}
    else
        g_bat_low = FALSE;

	if(0 == val)
#if defined(MTK_BQ24296_SUPPORT)
	val = get_i_sense_volt(5);
#else
	val = get_bat_sense_volt(1);
#endif

    if (val < BATTERY_LOWVOL_THRESOLD)
    {
        printf("%s, TRUE\n", __FUNCTION__);
		g_bat_low = 0x1;
    }
#if 0
    else
    {
        #ifdef MTK_BQ24296_SUPPORT
        kal_uint32 bq24296_vsys_status = bq24296_get_vsys_stat();
        printf("bq24296_vsys_status = %d\n", bq24296_vsys_status);
    
        if(bq24296_vsys_status == 0x1) //Pre-charge
        {
            printf("%s, battery protect TRUE\n", __FUNCTION__);
			g_bat_low = 0x1;
        }  
        #endif
    }
#endif 
	if(FALSE == g_bat_low)
    printf("%s, FALSE\n", __FUNCTION__);
    return g_bat_low;
}

void pchr_turn_on_charging (void)
{
	upmu_set_rg_usbdl_rst(1);		//force leave USBDL mode
	
	kick_charger_wdt();
	
    upmu_set_rg_cs_vth(0xC);    	// CS_VTH, 450mA            
    upmu_set_rg_csdac_en(1);                // CSDAC_EN
    upmu_set_rg_chr_en(1);                  // CHR_EN  

#ifdef MTK_FAN5405_SUPPORT
    fan5405_hw_init();
    fan5405_turn_on_charging();
    fan5405_dump_register();
#endif

#ifdef MTK_BQ24297_SUPPORT
    bq24297_hw_init();
    bq24297_turn_on_charging();
    bq24297_dump_register();
#endif

#ifdef MTK_BQ24296_SUPPORT
    bq24296_hw_init();
    bq24296_turn_on_charging();
    bq24296_dump_register();
#endif

#ifdef MTK_BQ24196_SUPPORT
    bq24196_hw_init();
    bq24196_charging_enable(1);
#endif

#ifdef MTK_BQ24158_SUPPORT
    bq24158_hw_init();
    bq24158_turn_on_charging();
    bq24158_dump_register();
#endif
}


void mt65xx_bat_init(void)
{    
		kal_int32 bat_vol;
		
#if defined(MTK_POWER_EXT_DETECT)
    if(MT_BOARD_EVB == mt_get_board_type())
        return;
#endif		
		// Low Battery Safety Booting
		
		bat_vol = get_bat_sense_volt(1);
		printf("[mt65xx_bat_init] check VBAT=%d mV with %d mV\n", bat_vol, BATTERY_LOWVOL_THRESOLD);
		
		pchr_turn_on_charging();

		if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && (upmu_get_pwrkey_deb()==0) ) {
				printf("[mt65xx_bat_init] KPOC+PWRKEY => change boot mode\n");		
		
				g_boot_reason_change = true;
		}
		rtc_boot_check(false);

	#ifndef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
  //if (bat_vol < BATTERY_LOWVOL_THRESOLD)
    if (is_low_battery(bat_vol))
    {
        if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && upmu_is_chr_det() == KAL_TRUE)
        {
            printf("[%s] Kernel Low Battery Power Off Charging Mode\n", __func__);
            g_boot_mode = LOW_POWER_OFF_CHARGING_BOOT;
            return;
        }
        else
        {
            printf("[BATTERY] battery voltage(%dmV) <= CLV ! Can not Boot Linux Kernel !! \n\r",bat_vol);
#ifndef NO_POWER_OFF
            mt6575_power_off();
#endif			
            while(1)
            {
                printf("If you see the log, please check with RTC power off API\n\r");
            }
        }
    }
	#endif
    return;
}

#else

#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <printf.h>

void mt65xx_bat_init(void)
{
    printf("[BATTERY] Skip mt65xx_bat_init !!\n\r");
    printf("[BATTERY] If you want to enable power off charging, \n\r");
    printf("[BATTERY] Please #define CFG_POWER_CHARGING!!\n\r");
}

#endif
