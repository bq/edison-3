#include <linux/xlog.h>
#include <linux/delay.h> 

#include <mach/upmu_common.h>
#include <mach/upmu_hw.h>

#include <mach/battery_meter_hal.h>
#include <cust_battery_meter.h>
     
#ifdef SOC_BY_EXT_HW_FG
#include <cust_ext_fg_defs.h>

#ifdef MTK_BQ27541_SUPPORT
#include "bq27541.h"
#endif
#endif
//============================================================ //
//define
//============================================================ //
#define STATUS_OK    0
#define STATUS_UNSUPPORTED    -1
#define VOLTAGE_FULL_RANGE     1800
#define ADC_PRECISE           32768  // 15 bits

//============================================================ //
//global variable
//============================================================ //
kal_int32 chip_diff_trim_value_4_0 = 0;
kal_int32 chip_diff_trim_value = 0; // unit = 0.1

kal_int32 g_hw_ocv_tune_value = 8;

kal_bool g_fg_is_charging = 0;

kal_bool ext_fg_init_done = KAL_FALSE;
//============================================================ //
//function prototype
//============================================================ //

//============================================================ //
//extern variable
//============================================================ //
 
//============================================================ //
//extern function
//============================================================ //
extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
extern int IMM_IsAdcInitReady(void);

//============================================================ // 
void get_hw_chip_diff_trim_value(void)
{
#if defined(CONFIG_POWER_EXT)
#else
    kal_int32 reg_val_1 = 0;
    kal_int32 reg_val_2 = 0;
    
    #if 1
    reg_val_1 = upmu_get_reg_value(0x01C4);
    reg_val_1 = (reg_val_1 & 0xE000) >> 13;
    
    reg_val_2 = upmu_get_reg_value(0x01C6);
    reg_val_2 = (reg_val_2 & 0x0003);

    chip_diff_trim_value_4_0 = reg_val_1 | (reg_val_2 << 3);
    
    bm_print(BM_LOG_FULL, "[Chip_Trim] Reg[0x%x]=0x%x, Reg[0x%x]=0x%x, chip_diff_trim_value_4_0=%d\n", 
        0x01C4, upmu_get_reg_value(0x01C4), 0x01C6, upmu_get_reg_value(0x01C6), chip_diff_trim_value_4_0);   
    
    #else
    bm_print(BM_LOG_FULL,, "[Chip_Trim] need check reg number\n");
    #endif

    switch(chip_diff_trim_value_4_0){       
        case 0:    
            chip_diff_trim_value = 1000; 
            bm_print(BM_LOG_FULL, "[Chip_Trim] chip_diff_trim_value = 1000\n");
            break;
        case 1:    chip_diff_trim_value = 1005; break;
        case 2:    chip_diff_trim_value = 1010; break;
        case 3:    chip_diff_trim_value = 1015; break;
        case 4:    chip_diff_trim_value = 1020; break;
        case 5:    chip_diff_trim_value = 1025; break;
        case 6:    chip_diff_trim_value = 1030; break;
        case 7:    chip_diff_trim_value = 1035; break;
        case 8:    chip_diff_trim_value = 1040; break;
        case 9:    chip_diff_trim_value = 1045; break;
        case 10:   chip_diff_trim_value = 1050; break;
        case 11:   chip_diff_trim_value = 1055; break;
        case 12:   chip_diff_trim_value = 1060; break;
        case 13:   chip_diff_trim_value = 1065; break;
        case 14:   chip_diff_trim_value = 1070; break;
        case 15:   chip_diff_trim_value = 1075; break;
        case 31:   chip_diff_trim_value = 995; break; 
        case 30:   chip_diff_trim_value = 990; break; 
        case 29:   chip_diff_trim_value = 985; break; 
        case 28:   chip_diff_trim_value = 980; break; 
        case 27:   chip_diff_trim_value = 975; break; 
        case 26:   chip_diff_trim_value = 970; break; 
        case 25:   chip_diff_trim_value = 965; break; 
        case 24:   chip_diff_trim_value = 960; break; 
        case 23:   chip_diff_trim_value = 955; break; 
        case 22:   chip_diff_trim_value = 950; break; 
        case 21:   chip_diff_trim_value = 945; break; 
        case 20:   chip_diff_trim_value = 940; break; 
        case 19:   chip_diff_trim_value = 935; break; 
        case 18:   chip_diff_trim_value = 930; break; 
        case 17:   chip_diff_trim_value = 925; break; 
        default:
            bm_print(BM_LOG_FULL, "[Chip_Trim] Invalid value(%d)\n", chip_diff_trim_value_4_0);
            break;
    }

    bm_print(BM_LOG_FULL, "[Chip_Trim] %d,%d\n", 
        chip_diff_trim_value_4_0, chip_diff_trim_value);
#endif
}

kal_int32 use_chip_trim_value(kal_int32 not_trim_val)
{
#if defined(CONFIG_POWER_EXT)
    return not_trim_val;
#else
    kal_int32 ret_val=0;

    ret_val=((not_trim_val*chip_diff_trim_value)/1000);

    bm_print(BM_LOG_FULL, "[use_chip_trim_value] %d -> %d\n", not_trim_val, ret_val);
    
    return ret_val;
#endif    
}

int get_hw_ocv(void)
{
#if defined(CONFIG_POWER_EXT)
    return 4001;    
#else
    kal_int32 adc_result_reg=0;
    kal_int32 adc_result=0;
    kal_int32 r_val_temp=4;    

    #if defined(SWCHR_POWER_PATH)
    adc_result_reg = upmu_get_rg_adc_out_wakeup_swchr();
    adc_result = (adc_result_reg*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
    bm_print(BM_LOG_CRTI, "[oam] get_hw_ocv (swchr) : adc_result_reg=%d, adc_result=%d\n", 
        adc_result_reg, adc_result);
    #else
    adc_result_reg = upmu_get_rg_adc_out_wakeup_pchr();
    adc_result = (adc_result_reg*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;        
    bm_print(BM_LOG_CRTI, "[oam] get_hw_ocv (pchr) : adc_result_reg=%d, adc_result=%d\n", 
        adc_result_reg, adc_result);
    #endif

    adc_result += g_hw_ocv_tune_value;
    return adc_result;
#endif    
}


//============================================================//

static kal_int32 fgauge_initialization(void *data)
{
#ifdef SOC_BY_EXT_HW_FG
    //wait I2C init done
    bm_print(BM_LOG_CRTI, "[fgauge_initialization] wait ext fg init done...\n");
    int m = 0;
    while (1)
    {        
        if (ext_fg_init_done == KAL_TRUE)
        {
            bm_print(BM_LOG_CRTI, "[fgauge_initialization] ext fg init done!\n");
            break;
        }
        else
        {
            m++;
            msleep(1);
            if (m>1000)
            {            
                bm_print(BM_LOG_CRTI, "[fgauge_initialization] timeout!\r\n");
                break;
            }
        }
    }
    
    return STATUS_OK;
#endif
    return STATUS_OK;
}

static kal_int32 fgauge_read_current(void *data)
{
    return STATUS_OK;
}

static kal_int32 fgauge_read_current_sign(void *data)
{
    return STATUS_OK;
}

static kal_int32 fgauge_read_columb(void *data)
{
    return STATUS_OK;
}


static kal_int32 fgauge_hw_reset(void *data)
{
    return STATUS_OK;
}


static kal_int32 read_adc_v_bat_sense(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 4201;
#else
    *(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(VBAT_CHANNEL_NUMBER,*(kal_int32*)(data),1);
#endif

    return STATUS_OK;
}



static kal_int32 read_adc_v_i_sense(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 4202;
#else
    *(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(ISENSE_CHANNEL_NUMBER,*(kal_int32*)(data),1);
#endif

    return STATUS_OK;
}

static kal_int32 read_adc_v_bat_temp(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 0;
#else
    #if defined(MTK_PCB_TBAT_FEATURE)

        int ret = 0, data[4], i, ret_value = 0, ret_temp = 0;
        int Channel=1;
        
        if( IMM_IsAdcInitReady() == 0 )
        {
            bm_print(BM_LOG_CRTI, "[get_tbat_volt] AUXADC is not ready");
            return 0;
        }
    
        i = times;
        while (i--)
        {
            ret_value = IMM_GetOneChannelValue(Channel, data, &ret_temp);
            ret += ret_temp;
            bm_print(BM_LOG_FULL, "[get_tbat_volt] ret_temp=%d\n",ret_temp);
        }
        
        ret = ret*1500/4096 ;
        ret = ret/times;
        bm_print(BM_LOG_CRTI, "[get_tbat_volt] Battery output mV = %d\n",ret);

        *(kal_int32*)(data) = ret;

    #else
        bm_print(BM_LOG_FULL, "[read_adc_v_charger] return PMIC_IMM_GetOneChannelValue(4,times,1);\n");
        *(kal_int32*)(data) = PMIC_IMM_GetOneChannelValue(VBATTEMP_CHANNEL_NUMBER,*(kal_int32*)(data),1);
    #endif
#endif

    return STATUS_OK;
}

static kal_int32 read_adc_v_charger(void *data)
{    
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 5001;
#else
    kal_int32 val;
    val = PMIC_IMM_GetOneChannelValue(VCHARGER_CHANNEL_NUMBER,*(kal_int32*)(data),1);
    val = (((R_CHARGER_1+R_CHARGER_2)*100*val)/R_CHARGER_2)/100;
	
    *(kal_int32*)(data) = val;
#endif

    return STATUS_OK;
}

static kal_int32 read_hw_ocv(void *data)
{
#if defined(CONFIG_POWER_EXT)
    *(kal_int32*)(data) = 3999;
#else
    *(kal_int32*)(data) = get_hw_ocv();
#endif

    return STATUS_OK;
}

static kal_int32 dump_register_fgadc(void *data)
{
    return STATUS_OK;
}
#ifdef EXT_FG_HAS_SOC
static kal_int32 ext_fg_get_soc(void *data)
{
#ifdef MTK_BQ27541_SUPPORT
    int ret = 0;
    int returnData = 0;
    
    ret = bq27541_set_cmd_read(BQ27541_CMD_StateOfCharge, &returnData);
    if(ret == 1)    
    {
        bm_print(BM_LOG_CRTI, "[ext_fg_get_soc] bq27541 read raw data = %d\n", returnData);
        *(kal_int32 *)(data) = returnData;
        return STATUS_OK;
    }
    else
    {
        return STATUS_UNSUPPORTED;
    }        
#endif

#ifdef MTK_BQ27741_SUPPORT
#endif

    return STATUS_OK;
}
#endif

#ifdef EXT_FG_HAS_TEMPERATURE
static kal_int32 ext_fg_get_temperature(void *data)
{
#ifdef MTK_BQ27541_SUPPORT
    int ret = 0;
    int returnData = 0;
    
    ret = bq27541_set_cmd_read(BQ27541_CMD_Temperature, &returnData);
    if(ret == 1)    
    {
        bm_print(BM_LOG_CRTI, "[ext_fg_get_temperature] bq27541 read raw data = %d\n", returnData);
        *(kal_int32 *)(data) = ((returnData/10) - 273);
        return STATUS_OK;
    }
    else
    {
        return STATUS_UNSUPPORTED;
    }        
#endif

#ifdef MTK_BQ27741_SUPPORT
#endif

    return STATUS_OK;
}
#endif

#ifdef EXT_FG_HAS_VOLTAGE
static kal_int32 ext_fg_get_voltage(void *data)
{
#ifdef MTK_BQ27541_SUPPORT
    int ret = 0;
    int returnData = 0;
    
    ret = bq27541_set_cmd_read(BQ27541_CMD_Voltage, &returnData);
    if(ret == 1)
    {
        bm_print(BM_LOG_CRTI, "[ext_fg_get_voltage] bq27541 read voltage data = %d\n", returnData);
	 if(returnData <= 0)
		return STATUS_UNSUPPORTED;
	 else
		*(kal_int32 *)(data) = returnData;
        return STATUS_OK;
    }
    else
    {
        return STATUS_UNSUPPORTED;
    }        
#endif

#ifdef MTK_BQ27741_SUPPORT
#endif

    return STATUS_OK;
}
#endif

#ifdef EXT_FG_HAS_CURRENT
static kal_int32 ext_fg_get_current(void *data)
{
#ifdef MTK_BQ27541_SUPPORT
    int ret = 0;
    int returnData = 0;

    ret = bq27541_set_cmd_read(BQ27541_CMD_AverageCurrent, &returnData);
    if(ret == 1)
    {
        bm_print(BM_LOG_CRTI, "[ext_fg_get_current] bq27541 read current raw data = %d\n", returnData);
		if(returnData > 0x8000){
			returnData = (0xFFFF^(returnData - 1))*(-1);
		}
		bm_print(BM_LOG_CRTI, "[ext_fg_get_current] current data = %dmA\n", returnData);

        *(kal_int32 *)(data) = returnData * 10;

        return STATUS_OK;
    }
    else
    {
        return STATUS_UNSUPPORTED;
    }
#endif

    return STATUS_OK;
}
#endif

static kal_int32 ext_hw_fg(void *data)
{
#ifdef SOC_BY_EXT_HW_FG
    kal_int32 cmd = EXT_FG_CMD_NUMBER;

    //make ext fg I2C init done
    if (ext_fg_init_done == KAL_FALSE)
    {
        bm_print(BM_LOG_CRTI, "[ext_hw_fg] ext fg is not ready\n");
        return STATUS_UNSUPPORTED;
    }
    
    //translate ext fg cmd
    if (data)
    {
        cmd = *(kal_int32 *)(data);
    }
    else
    {
        bm_print(BM_LOG_CRTI, "[ext_hw_fg] NULL cmd!\n");
        return STATUS_UNSUPPORTED;
    }
    
    //switch case to get battery information
    switch (cmd)
    {
#ifdef EXT_FG_HAS_SOC
        case EXT_FG_CMD_SOC:
            return ext_fg_get_soc(data);
            break;
#endif           

#ifdef EXT_FG_HAS_TEMPERATURE
        case EXT_FG_CMD_TEMPERATURE:
            return ext_fg_get_temperature(data);
            break;
#endif

#ifdef EXT_FG_HAS_VOLTAGE
	case EXT_FG_CMD_VOLTAGE:
		return ext_fg_get_voltage(data);
		break;
#endif

#ifdef EXT_FG_HAS_CURRENT
	case EXT_FG_CMD_CURRENT:
		return ext_fg_get_current(data);
		break;
#endif

        default:
            bm_print(BM_LOG_CRTI, "[ext_hw_fg] unsupported cmd(%d)!\n", cmd);
            return STATUS_UNSUPPORTED;
    }        
#endif
    
    return STATUS_UNSUPPORTED;
}

static kal_int32 (* const bm_func[BATTERY_METER_CMD_NUMBER])(void *data)=
{
    fgauge_initialization		//hw fuel gague used only
    
    ,fgauge_read_current		//hw fuel gague used only
    ,fgauge_read_current_sign	//hw fuel gague used only
    ,fgauge_read_columb			//hw fuel gague used only

    ,fgauge_hw_reset			//hw fuel gague used only
    
    ,read_adc_v_bat_sense
    ,read_adc_v_i_sense
    ,read_adc_v_bat_temp
    ,read_adc_v_charger

    ,read_hw_ocv
    ,dump_register_fgadc		//hw fuel gague used only
    ,ext_hw_fg                  //ext hw fuel gauge used only
};

kal_int32 bm_ctrl_cmd(BATTERY_METER_CTRL_CMD cmd, void *data)
{
    kal_int32 status;

    if(cmd < BATTERY_METER_CMD_NUMBER)
        status = bm_func[cmd](data);
    else
        return STATUS_UNSUPPORTED;
    
    return status;
}

