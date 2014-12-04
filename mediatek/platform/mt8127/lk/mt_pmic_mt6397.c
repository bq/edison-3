#include <debug.h>
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/mt_gpt.h>
#include <platform/mt_pmic_wrap_init.h>
//#include <printf.h>

//==============================================================================
// Global variable
//==============================================================================
int Enable_PMIC_LOG = 1;

CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;
int g_pmic_cid=0;

extern int g_R_BAT_SENSE;
extern int g_R_I_SENSE;
extern int g_R_CHARGER_1;
extern int g_R_CHARGER_2;

//==============================================================================
// PMIC access API
//==============================================================================
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;    

    //mt6323_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;
    if(return_value!=0)
    {   
        dprintf(CRITICAL, "[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    dprintf(INFO, "[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= (MASK << SHIFT);
    *val = (pmic_reg >> SHIFT);    
    dprintf(INFO, "[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;

    //1. mt6323_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        dprintf(CRITICAL, "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    dprintf(INFO, "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= ~(MASK << SHIFT);
    pmic_reg |= (val << SHIFT);

    //2. mt6323_write_byte(RegNum, pmic_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic_reg, &rdata);
    if(return_value!=0)
    {   
        dprintf(CRITICAL, "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    dprintf(INFO, "[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic_reg);    

#if 0
    //3. Double Check    
    //mt6323_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        dprintf(CRITICAL, "[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    dprintf(INFO, "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
#endif    

    return return_value;
}

//==============================================================================
// PMIC APIs
//==============================================================================


//==============================================================================
// PMIC6397 Usage APIs
//==============================================================================
U32 pmic_IsUsbCableIn (void) 
{
    U32 ret=0;
    U32 val=0;
    
    ret=pmic_read_interface( (U32)(CHR_CON0),
                             (&val),
                             (U32)(PMIC_RGS_CHRDET_MASK),
                             (U32)(PMIC_RGS_CHRDET_SHIFT)
                             );
    if(Enable_PMIC_LOG>1) 
      dprintf(CRITICAL, "%d", ret); 

    return val;
}

kal_bool upmu_is_chr_det(void)
{
	U32 tmp32;
	tmp32=upmu_get_rgs_chrdet();
	if(tmp32 == 0)
	{
		//printk("[upmu_is_chr_det] No charger\n");
		return KAL_FALSE;
	}
	else
	{
		//printk("[upmu_is_chr_det] Charger exist\n");
		return KAL_TRUE;
	}
}

kal_bool pmic_chrdet_status(void)
{
    if( upmu_is_chr_det() == KAL_TRUE )    
    {
        #ifndef USER_BUILD
        dprintf(INFO, "[pmic_chrdet_status] Charger exist\r\n");
        #endif
        
        return KAL_TRUE;
    }
    else
    {
        #ifndef USER_BUILD
        dprintf(INFO, "[pmic_chrdet_status] No charger\r\n");
        #endif
        
        return KAL_FALSE;
    }
}

int pmic_detect_powerkey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(CHRSTATUS),
                             (&val),
                             (U32)(PMIC_PWRKEY_DEB_MASK),
                             (U32)(PMIC_PWRKEY_DEB_SHIFT)
                             );
    
    if(Enable_PMIC_LOG>1) 
      dprintf(CRITICAL, "%d", ret); 

    if (val==1){     
        dprintf(CRITICAL, "[pmic_detect_powerkey] Release\n");
    return 0;
    }else{
        dprintf(CRITICAL, "[pmic_detect_powerkey] Press\n");
        return 1;
    }
}

int pmic_detect_homekey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(OCSTATUS2),
                             (&val),
                             (U32)(PMIC_HOMEKEY_DEB_MASK),
                             (U32)(PMIC_HOMEKEY_DEB_SHIFT)
                             );
    
    if(Enable_PMIC_LOG>1) 
      dprintf(CRITICAL, "%d", ret);
    
    return val;
}

kal_uint32 upmu_get_reg_value(kal_uint32 reg)
{
	U32 ret=0;
	U32 temp_val=0;

	ret=pmic_read_interface(reg, &temp_val, 0xFFFF, 0x0);

    if(Enable_PMIC_LOG>1) 
      dprintf(CRITICAL, "%d", ret);

	return temp_val;
}

void PMIC_DUMP_ALL_Register(void)
{
    U32 i=0;
    U32 ret=0;
    U32 reg_val=0;

    for (i=0;i<0xFFFF;i++)
    {
        ret=pmic_read_interface(i,&reg_val,0xFFFF,0);
        dprintf(CRITICAL, "Reg[0x%x]=0x%x, %d\n", i, reg_val, ret);
    }
}

//==============================================================================
// PMIC6397 Init Code
//==============================================================================
void PMIC_INIT_SETTING_V1(void)
{
    U32 chip_version = 0;
    U32 ret = 0;

    chip_version = upmu_get_cid();

    if(chip_version >= PMIC6397_E1_CID_CODE)
    {
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] PMIC Chip = 0x%x\n",chip_version);

        //put init setting from DE/SA        
        ret = pmic_config_interface(0x2,0xB,0xF,4); // [7:4]: RG_VCDT_HV_VTH; 7V OVP
        ret = pmic_config_interface(0xC,0x1,0x7,1); // [3:1]: RG_VBAT_OV_VTH; VBAT_OV=4.3V
        ret = pmic_config_interface(0x1A,0x3,0xF,0); // [3:0]: RG_CHRWDT_TD; align to 6250's
        ret = pmic_config_interface(0x24,0x1,0x1,1); // [1:1]: RG_BC11_RST; 
        ret = pmic_config_interface(0x2A,0x0,0x7,4); // [6:4]: RG_CSDAC_STP; align to 6250's setting
        ret = pmic_config_interface(0x2E,0x1,0x1,7); // [7:7]: RG_ULC_DET_EN; 
        ret = pmic_config_interface(0x2E,0x1,0x1,6); // [6:6]: RG_HWCV_EN; 
        ret = pmic_config_interface(0x2E,0x1,0x1,2); // [2:2]: RG_CSDAC_MODE; 
        ret = pmic_config_interface(0x102,0x0,0x1,3); // [3:3]: RG_PWMOC_CK_PDN; For OC protection
        ret = pmic_config_interface(0x128,0x1,0x1,9); // [9:9]: RG_SRCVOLT_HW_AUTO_EN; 
        ret = pmic_config_interface(0x128,0x1,0x1,8); // [8:8]: RG_OSC_SEL_AUTO; 
        ret = pmic_config_interface(0x128,0x1,0x1,6); // [6:6]: RG_SMPS_DIV2_SRC_AUTOFF_DIS; 
        ret = pmic_config_interface(0x128,0x1,0x1,5); // [5:5]: RG_SMPS_AUTOFF_DIS; 
        ret = pmic_config_interface(0x130,0x1,0x1,7); // [7:7]: VDRM_DEG_EN; 
        ret = pmic_config_interface(0x130,0x1,0x1,6); // [6:6]: VSRMCA7_DEG_EN; 
        ret = pmic_config_interface(0x130,0x1,0x1,5); // [5:5]: VPCA7_DEG_EN; 
        ret = pmic_config_interface(0x130,0x1,0x1,4); // [4:4]: VIO18_DEG_EN; 
        ret = pmic_config_interface(0x130,0x1,0x1,3); // [3:3]: VGPU_DEG_EN; For OC protection
        ret = pmic_config_interface(0x130,0x1,0x1,2); // [2:2]: VCORE_DEG_EN; 
        ret = pmic_config_interface(0x130,0x1,0x1,1); // [1:1]: VSRMCA15_DEG_EN; 
        ret = pmic_config_interface(0x130,0x1,0x1,0); // [0:0]: VCA15_DEG_EN; 
        ret = pmic_config_interface(0x178,0x1,0x1,11); // [11:11]: RG_INT_EN_THR_H; 
        ret = pmic_config_interface(0x178,0x1,0x1,10); // [10:10]: RG_INT_EN_THR_L; 
        ret = pmic_config_interface(0x178,0x1,0x1,4); // [4:4]: RG_INT_EN_BAT_L; 
        ret = pmic_config_interface(0x17E,0x1,0x1,11); // [11:11]: RG_INT_EN_VGPU; OC protection
        ret = pmic_config_interface(0x17E,0x1,0x1,8); // [8:8]: RG_INT_EN_VCA15; OC protection
        ret = pmic_config_interface(0x206,0x600,0x0FFF,0); // [12:0]: BUCK_RSV; for OC protection
        ret = pmic_config_interface(0x210,0x0,0x3,10); // [11:10]: QI_VCORE_VSLEEP; sleep mode only (0.85V)
        ret = pmic_config_interface(0x210,0x0,0x3,6); // [7:6]: QI_VSRMCA7_VSLEEP; sleep mode only (0.85V)
        ret = pmic_config_interface(0x210,0x0,0x3,2); // [3:2]: QI_VPCA7_VSLEEP; sleep mode only (0.85V)
        ret = pmic_config_interface(0x216,0x0,0x3,12); // [13:12]: RG_VCA15_CSL2; for OC protection
        ret = pmic_config_interface(0x216,0x0,0x3,10); // [11:10]: RG_VCA15_CSL1; for OC protection
        ret = pmic_config_interface(0x224,0x1,0x1,15); // [15:15]: VCA15_SFCHG_REN; soft change rising enable
        ret = pmic_config_interface(0x224,0x5,0x7F,8); // [14:8]: VCA15_SFCHG_RRATE; soft change rising step=0.5us
        ret = pmic_config_interface(0x224,0x1,0x1,7); // [7:7]: VCA15_SFCHG_FEN; soft change falling enable
        ret = pmic_config_interface(0x224,0x17,0x7F,0); // [6:0]: VCA15_SFCHG_FRATE; soft change falling step=2us
        ret = pmic_config_interface(0x238,0x1,0x1,8); // [8:8]: VCA15_VSLEEP_EN; set sleep mode reference voltage from R2R to V2V
        ret = pmic_config_interface(0x238,0x3,0x3,4); // [5:4]: VCA15_VOSEL_TRANS_EN; rising & falling enable
        ret = pmic_config_interface(0x24A,0x1,0x1,15); // [15:15]: VSRMCA15_SFCHG_REN; 
        ret = pmic_config_interface(0x24A,0x5,0x7F,8); // [14:8]: VSRMCA15_SFCHG_RRATE; 
        ret = pmic_config_interface(0x24A,0x1,0x1,7); // [7:7]: VSRMCA15_SFCHG_FEN; 
        ret = pmic_config_interface(0x24A,0x17,0x7F,0); // [6:0]: VSRMCA15_SFCHG_FRATE; 
        ret = pmic_config_interface(0x25E,0x1,0x1,8); // [8:8]: VSRMCA15_VSLEEP_EN; set sleep mode reference voltage from R2R to V2V
        ret = pmic_config_interface(0x25E,0x3,0x3,4); // [5:4]: VSRMCA15_VOSEL_TRANS_EN; rising & falling enable
        ret = pmic_config_interface(0x270,0x1,0x1,1); // [1:1]: VCORE_VOSEL_CTRL; sleep mode voltage control follow SRCLKEN
        ret = pmic_config_interface(0x276,0x1,0x1,15); // [15:15]: VCORE_SFCHG_REN; 
        ret = pmic_config_interface(0x276,0x5,0x7F,8); // [14:8]: VCORE_SFCHG_RRATE; 
        ret = pmic_config_interface(0x276,0x17,0x7F,0); // [6:0]: VCORE_SFCHG_FRATE; 
        ret = pmic_config_interface(0x27C,0x18,0x7F,0); // [6:0]: VCORE_VOSEL_SLEEP; Sleep mode setting only (0.85V)
        ret = pmic_config_interface(0x28A,0x1,0x1,8); // [8:8]: VCORE_VSLEEP_EN; Sleep mode HW control  R2R to VtoV
        ret = pmic_config_interface(0x28A,0x0,0x3,4); // [5:4]: VCORE_VOSEL_TRANS_EN; Follows MT6320 VCORE setting.
        ret = pmic_config_interface(0x28A,0x3,0x3,0); // [1:0]: VCORE_TRANSTD; 
        ret = pmic_config_interface(0x28E,0x1,0x3,8); // [9:8]: RG_VGPU_CSL; for OC protection
        ret = pmic_config_interface(0x29C,0x1,0x1,15); // [15:15]: VGPU_SFCHG_REN; 
        ret = pmic_config_interface(0x29C,0x5,0x7F,8); // [14:8]: VGPU_SFCHG_RRATE; 
        ret = pmic_config_interface(0x29C,0x17,0x7F,0); // [6:0]: VGPU_SFCHG_FRATE; 
        ret = pmic_config_interface(0x2B0,0x0,0x3,4); // [5:4]: VGPU_VOSEL_TRANS_EN; 
        ret = pmic_config_interface(0x2B0,0x3,0x3,0); // [1:0]: VGPU_TRANSTD; 
        ret = pmic_config_interface(0x332,0x0,0x3,4); // [5:4]: VPCA7_VOSEL_SEL; 
        ret = pmic_config_interface(0x336,0x1,0x1,15); // [15:15]: VPCA7_SFCHG_REN; 
        ret = pmic_config_interface(0x336,0x5,0x7F,8); // [14:8]: VPCA7_SFCHG_RRATE; 
        ret = pmic_config_interface(0x336,0x1,0x1,7); // [7:7]: VPCA7_SFCHG_FEN; 
        ret = pmic_config_interface(0x336,0x17,0x7F,0); // [6:0]: VPCA7_SFCHG_FRATE; 
        ret = pmic_config_interface(0x33C,0x18,0x7F,0); // [6:0]: VPCA7_VOSEL_SLEEP; 
        ret = pmic_config_interface(0x34A,0x1,0x1,8); // [8:8]: VPCA7_VSLEEP_EN; 
        ret = pmic_config_interface(0x34A,0x3,0x3,4); // [5:4]: VPCA7_VOSEL_TRANS_EN; 
        ret = pmic_config_interface(0x356,0x1,0x1,5); // [5:5]: VSRMCA7_TRACK_SLEEP_CTRL; 
        ret = pmic_config_interface(0x358,0x0,0x3,4); // [5:4]: VSRMCA7_VOSEL_SEL; 
        ret = pmic_config_interface(0x35C,0x1,0x1,15); // [15:15]: VSRMCA7_SFCHG_REN; 
        ret = pmic_config_interface(0x35C,0x5,0x7F,8); // [14:8]: VSRMCA7_SFCHG_RRATE; 
        ret = pmic_config_interface(0x35C,0x1,0x1,7); // [7:7]: VSRMCA7_SFCHG_FEN; 
        ret = pmic_config_interface(0x35C,0x17,0x7F,0); // [6:0]: VSRMCA7_SFCHG_FRATE; 
        ret = pmic_config_interface(0x362,0x18,0x7F,0); // [6:0]: VSRMCA7_VOSEL_SLEEP; 
        ret = pmic_config_interface(0x370,0x1,0x1,8); // [8:8]: VSRMCA7_VSLEEP_EN; 
        ret = pmic_config_interface(0x370,0x3,0x3,4); // [5:4]: VSRMCA7_VOSEL_TRANS_EN; 
        ret = pmic_config_interface(0x39C,0x1,0x1,8); // [8:8]: VDRM_VSLEEP_EN; 
        ret = pmic_config_interface(0x440,0x1,0x1,2); // [2:2]: VIBR_THER_SHEN_EN; 
        ret = pmic_config_interface(0x500,0x1,0x1,5); // [5:5]: THR_HWPDN_EN; 
        ret = pmic_config_interface(0x502,0x1,0x1,3); // [3:3]: RG_RST_DRVSEL; 
        ret = pmic_config_interface(0x502,0x1,0x1,2); // [2:2]: RG_EN_DRVSEL; 
        ret = pmic_config_interface(0x508,0x1,0x1,1); // [1:1]: PWRBB_DEB_EN; 
        ret = pmic_config_interface(0x50C,0x1,0x1,12); // [12:12]: VSRMCA15_PG_H2L_EN; 
        ret = pmic_config_interface(0x50C,0x1,0x1,11); // [11:11]: VPCA15_PG_H2L_EN; 
        ret = pmic_config_interface(0x50C,0x1,0x1,10); // [10:10]: VCORE_PG_H2L_EN; 
        ret = pmic_config_interface(0x50C,0x1,0x1,9); // [9:9]: VSRMCA7_PG_H2L_EN; 
        ret = pmic_config_interface(0x50C,0x1,0x1,8); // [8:8]: VPCA7_PG_H2L_EN; 
        ret = pmic_config_interface(0x512,0x1,0x1,1); // [1:1]: STRUP_PWROFF_PREOFF_EN; 
        ret = pmic_config_interface(0x512,0x1,0x1,0); // [0:0]: STRUP_PWROFF_SEQ_EN; 
        ret = pmic_config_interface(0x55E,0xFC,0xFF,8); // [15:8]: RG_ADC_TRIM_CH_SEL; 
        ret = pmic_config_interface(0x560,0x1,0x1,1); // [1:1]: FLASH_THER_SHDN_EN; 
        ret = pmic_config_interface(0x566,0x1,0x1,1); // [1:1]: KPLED_THER_SHDN_EN; 
        ret = pmic_config_interface(0x600,0x1,0x1,9); // [9:9]: SPK_THER_SHDN_L_EN; 
        ret = pmic_config_interface(0x604,0x1,0x1,0); // [0:0]: RG_SPK_INTG_RST_L; 
        ret = pmic_config_interface(0x606,0x1,0x1,9); // [9:9]: SPK_THER_SHDN_R_EN; 
        ret = pmic_config_interface(0x60A,0x1,0xF,11); // [14:11]: RG_SPKPGA_GAINR; 
        ret = pmic_config_interface(0x612,0x1,0xF,8); // [11:8]: RG_SPKPGA_GAINL; 
        ret = pmic_config_interface(0x632,0x1,0x1,8); // [8:8]: FG_SLP_EN; 
        ret = pmic_config_interface(0x638,0xFFC2,0xFFFF,0); // [15:0]: FG_SLP_CUR_TH; 
        ret = pmic_config_interface(0x63A,0x14,0xFF,0); // [7:0]: FG_SLP_TIME; 
        ret = pmic_config_interface(0x63C,0xFF,0xFF,8); // [15:8]: FG_DET_TIME; 
        ret = pmic_config_interface(0x714,0x1,0x1,7); // [7:7]: RG_LCLDO_ENC_REMOTE_SENSE_VA28; 
        ret = pmic_config_interface(0x714,0x1,0x1,4); // [4:4]: RG_LCLDO_REMOTE_SENSE_VA33; 
        ret = pmic_config_interface(0x714,0x1,0x1,1); // [1:1]: RG_HCLDO_REMOTE_SENSE_VA33; 
        ret = pmic_config_interface(0x71A,0x1,0x1,15); // [15:15]: RG_NCP_REMOTE_SENSE_VA18; 
        ret = pmic_config_interface(0x260,0x4,0x7F,8); // [14:8]: VSRMCA15_VOSEL_OFFSET; set offset=25mV
        ret = pmic_config_interface(0x260,0x0,0x7F,0); // [6:0]: VSRMCA15_VOSEL_DELTA; set delta=0mV
        ret = pmic_config_interface(0x262,0x5C,0x7F,8); // [14:8]: VSRMCA15_VOSEL_ON_HB; set HB=1.275V
        ret = pmic_config_interface(0x262,0x38,0x7F,0); // [6:0]: VSRMCA15_VOSEL_ON_LB; set LB=1.05000V
        ret = pmic_config_interface(0x264,0x18,0x7F,0); // [6:0]: VSRMCA15_VOSEL_SLEEP_LB; set sleep LB=0.85000V
        ret = pmic_config_interface(0x372,0x4,0x7F,8); // [14:8]: VSRMCA7_VOSEL_OFFSET; set offset=25mV
        ret = pmic_config_interface(0x372,0x0,0x7F,0); // [6:0]: VSRMCA7_VOSEL_DELTA; set delta=0mV
        ret = pmic_config_interface(0x374,0x5C,0x7F,8); // [14:8]: VSRMCA7_VOSEL_ON_HB; set HB=1.275V
        ret = pmic_config_interface(0x374,0x38,0x7F,0); // [6:0]: VSRMCA7_VOSEL_ON_LB; set LB=1.05000V
        ret = pmic_config_interface(0x376,0x18,0x7F,0); // [6:0]: VSRMCA7_VOSEL_SLEEP_LB; set sleep LB=0.85000V
        ret = pmic_config_interface(0x21E,0x1,0x1,1); // [1:1]: VCA15_VOSEL_CTRL; DVS HW control by SRCLKEN
        ret = pmic_config_interface(0x244,0x1,0x1,1); // [1:1]: VSRMCA15_VOSEL_CTRL; 
        ret = pmic_config_interface(0x330,0x1,0x1,1); // [1:1]: VPCA7_VOSEL_CTRL;
        ret = pmic_config_interface(0x356,0x1,0x1,1); // [1:1]: VSRMCA7_VOSEL_CTRL;
        ret = pmic_config_interface(0x21E,0x0,0x1,4); // [4:4]: VCA15_TRACK_ON_CTRL; DVFS tracking enable
        ret = pmic_config_interface(0x244,0x0,0x1,4); // [4:4]: VSRMCA15_TRACK_ON_CTRL; 
        ret = pmic_config_interface(0x330,0x0,0x1,4); // [4:4]: VPCA7_TRACK_ON_CTRL; 
        ret = pmic_config_interface(0x356,0x0,0x1,4); // [4:4]: VSRMCA7_TRACK_ON_CTRL;
        ret = pmic_config_interface(0x134,0x3,0x3,14); // [15:14]: VGPU OC; 
        ret = pmic_config_interface(0x134,0x3,0x3,2); // [3:2]: VCA15 OC;

        #if 0
        //dump register
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x002, upmu_get_reg_value(0x002));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x00C, upmu_get_reg_value(0x00C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x01A, upmu_get_reg_value(0x01A));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x024, upmu_get_reg_value(0x024));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x02A, upmu_get_reg_value(0x02A));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x02E, upmu_get_reg_value(0x02E));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x102, upmu_get_reg_value(0x102));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x128, upmu_get_reg_value(0x128));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x130, upmu_get_reg_value(0x130));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x134, upmu_get_reg_value(0x134));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x178, upmu_get_reg_value(0x178));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x17E, upmu_get_reg_value(0x17E));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x206, upmu_get_reg_value(0x206));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x210, upmu_get_reg_value(0x210));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x216, upmu_get_reg_value(0x216));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x21E, upmu_get_reg_value(0x21E));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x224, upmu_get_reg_value(0x224));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x238, upmu_get_reg_value(0x238));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x244, upmu_get_reg_value(0x244));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x24A, upmu_get_reg_value(0x24A));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x25E, upmu_get_reg_value(0x25E));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x260, upmu_get_reg_value(0x260));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x262, upmu_get_reg_value(0x262));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x264, upmu_get_reg_value(0x264));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x270, upmu_get_reg_value(0x270));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x276, upmu_get_reg_value(0x276));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x27C, upmu_get_reg_value(0x27C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x28A, upmu_get_reg_value(0x28A));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x28E, upmu_get_reg_value(0x28E));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x29C, upmu_get_reg_value(0x29C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x2B0, upmu_get_reg_value(0x2B0));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x330, upmu_get_reg_value(0x330));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x332, upmu_get_reg_value(0x332));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x336, upmu_get_reg_value(0x336));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x33C, upmu_get_reg_value(0x33C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x34A, upmu_get_reg_value(0x34A));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x356, upmu_get_reg_value(0x356));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x358, upmu_get_reg_value(0x358));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x35C, upmu_get_reg_value(0x35C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x362, upmu_get_reg_value(0x362));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x370, upmu_get_reg_value(0x370));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x372, upmu_get_reg_value(0x372));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x374, upmu_get_reg_value(0x374));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x376, upmu_get_reg_value(0x376));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x39C, upmu_get_reg_value(0x39C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x440, upmu_get_reg_value(0x440));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x500, upmu_get_reg_value(0x500));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x502, upmu_get_reg_value(0x502));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x508, upmu_get_reg_value(0x508));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x50C, upmu_get_reg_value(0x50C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x512, upmu_get_reg_value(0x512));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x55E, upmu_get_reg_value(0x55E));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x560, upmu_get_reg_value(0x560));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x566, upmu_get_reg_value(0x566));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x600, upmu_get_reg_value(0x600));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x604, upmu_get_reg_value(0x604));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x606, upmu_get_reg_value(0x606));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x60A, upmu_get_reg_value(0x60A));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x612, upmu_get_reg_value(0x612));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x632, upmu_get_reg_value(0x632));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x638, upmu_get_reg_value(0x638));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x63A, upmu_get_reg_value(0x63A));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x63C, upmu_get_reg_value(0x63C));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x714, upmu_get_reg_value(0x714));
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Reg[0x%x]=0x%x\n", 0x71A, upmu_get_reg_value(0x71A));        
        #endif
    }
    else
    {
        dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] Unknown PMIC Chip (%x)\n",chip_version);
    }
    NOUSE(ret);
}

void PMIC_CUSTOM_SETTING_V1(void)
{
    dprintf(INFO, "[LK_PMIC_INIT_SETTING_V1] \n");
}

U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;

	dprintf(CRITICAL, "[LK_pmic6397_init] Start...................\n");

	g_pmic_cid=upmu_get_cid();
    dprintf(CRITICAL, "[LK_PMIC_INIT] PMIC CID=0x%x\n", g_pmic_cid);

	//upmu_set_rg_chrind_on(0);    
	//dprintf(CRITICAL, "[LK_PMIC_INIT] Turn Off chrind\n");

    PMIC_INIT_SETTING_V1();
    dprintf(CRITICAL, "[LK_PMIC_INIT_SETTING_V1] Done\n");	
    
    PMIC_CUSTOM_SETTING_V1();
    dprintf(CRITICAL, "[LK_PMIC_CUSTOM_SETTING_V1] Done\n");
    
    //PMIC_DUMP_ALL_Register();
    
    return ret_code;
}

//==============================================================================
// PMIC6397 API for LK : AUXADC
//==============================================================================
int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd)
{
    kal_int32 u4Sample_times = 0;
    kal_int32 u4channel[8] = {0,0,0,0,0,0,0,0};
    kal_int32 adc_result=0;
    kal_int32 adc_result_temp=0;
    kal_int32 r_val_temp=0;    
    kal_int32 count=0;
    kal_int32 count_time_out=1000;
    kal_int32 ret_data=0;

    if(dwChannel==1)
    {
        pmic_config_interface(0x0020, 0x0801, 0xFFFF, 0);
        upmu_set_rg_source_ch0_norm_sel(1);
        upmu_set_rg_source_ch0_lbat_sel(1);
        dwChannel=0;
    }

    /*
        0 : V_BAT
        1 : V_I_Sense
        2 : V_Charger
        3 : V_TBAT
        4~7 : reserved    
    */
    upmu_set_rg_auxadc_chsel(dwChannel);

    //upmu_set_rg_avg_num(0x3);

    if(dwChannel==3)
    {
        upmu_set_rg_buf_pwd_on(1);
        upmu_set_rg_buf_pwd_b(1);
        upmu_set_baton_tdet_en(1);
        mdelay(20);
    }

    if(dwChannel==4)
    {
        upmu_set_rg_vbuf_en(1);
        upmu_set_rg_vbuf_byp(0);
        upmu_set_rg_vbuf_calen(1);
    }

    u4Sample_times=0;
    
    do
    {
        upmu_set_rg_auxadc_start(0);
        upmu_set_rg_auxadc_start(1);

        //Duo to HW limitation
        mdelay(10);

        count=0;
        ret_data=0;

        switch(dwChannel){         
            case 0:    
                while( upmu_get_rg_adc_rdy_c0() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c0_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c0();
                }
                break;
            case 1:    
                while( upmu_get_rg_adc_rdy_c1() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c1_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c1();
                }
                break;
            case 2:    
                while( upmu_get_rg_adc_rdy_c2() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c2_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c2();
                }
                break;
            case 3:    
                while( upmu_get_rg_adc_rdy_c3() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c3_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c3();
                }
                break;
            case 4:    
                while( upmu_get_rg_adc_rdy_c4() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c4_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c4();
                }
                break;
            case 5:    
                while( upmu_get_rg_adc_rdy_c5() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c5_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c5();
                }
                break;
            case 6:    
                while( upmu_get_rg_adc_rdy_c6() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c6_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c6();
                }
                break;
            case 7:    
                while( upmu_get_rg_adc_rdy_c7() != 1 )
                {    
                    if( (count++) > count_time_out)
                    {
                        dprintf(CRITICAL, "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
                        break;
                    }
                }
                if(trimd == 1) {
                    ret_data = upmu_get_rg_adc_out_c7_trim();
                } else {            
                    ret_data = upmu_get_rg_adc_out_c7();
                }
                break;    
            default:
                dprintf(CRITICAL, "[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
                return -1;
                break;
        }

        u4channel[dwChannel] += ret_data;

        u4Sample_times++;

        //debug
        dprintf(INFO, "[AUXADC] u4Sample_times=%d, ret_data=%d, u4channel[%d]=%d\n", 
            u4Sample_times, ret_data, dwChannel, u4channel[dwChannel]);
        
    }while (u4Sample_times < deCount);

    /* Value averaging  */ 
    u4channel[dwChannel] = u4channel[dwChannel] / deCount;
    adc_result_temp = u4channel[dwChannel];

    switch(dwChannel){         
        case 0:                
            r_val_temp = g_R_BAT_SENSE;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 1:    
            r_val_temp = g_R_I_SENSE;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 2:    
            r_val_temp = (((g_R_CHARGER_1+g_R_CHARGER_2)*100)/g_R_CHARGER_2);
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 3:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 4:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 5:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 6:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 7:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;    
        default:
            dprintf(CRITICAL, "[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
            return -1;
            break;
    }

    //debug
    dprintf(CRITICAL, "[AUXADC] adc_result_temp=%d, adc_result=%d, r_val_temp=%d\n", 
            adc_result_temp, adc_result, r_val_temp);

    count=0;

    if(dwChannel==0)
    {
        upmu_set_rg_source_ch0_norm_sel(0);
        upmu_set_rg_source_ch0_lbat_sel(0);
    }

    if(dwChannel==3)
    {
        upmu_set_baton_tdet_en(0);     
        upmu_set_rg_buf_pwd_b(0);
        upmu_set_rg_buf_pwd_on(0);
    }

    if(dwChannel==4)
    {
        //upmu_set_rg_vbuf_en(0);
        //upmu_set_rg_vbuf_byp(0);
        upmu_set_rg_vbuf_calen(0);
    }

    return adc_result;
    
}

int get_bat_sense_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(0,times,1);
}

int get_i_sense_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(1,times,1);
}

int get_charger_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(2,times,1);
}

int get_tbat_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(3,times,1);
}

//==============================================================================
// PMIC-Charger Type Detection 
//==============================================================================
CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE ret                 = CHARGER_UNKNOWN;
    
#if defined(CONFIG_POWER_EXT)    
    ret = STANDARD_HOST;
#else
    unsigned int USB_U2PHYACR6_2     = 0x1121081A;
    unsigned int USBPHYRegs          = 0x11210800; //U2B20_Base+0x800
    U16 bLineState_B                 = 0;
    U32 wChargerAvail                = 0;
    U32 bLineState_C                 = 0;
    U32 ret_val                      = 0;
    U32 reg_val                      = 0;

    //msleep(400);
    //dprintf(INFO, "mt_charger_type_detection : start!\r\n");

/********* Step 0.0 : enable USB memory and clock *********/
    //enable_pll(MT65XX_UPLL,"USB_PLL");
    //hwPowerOn(MT65XX_POWER_LDO_VUSB,VOL_DEFAULT,"VUSB_LDO");
    //dprintf(INFO, "[hw_charger_type_detection] enable VUSB and UPLL before connect\n");

/********* Step 1.0 : PMU_BC11_Detect_Init ***************/        
    SETREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 1 : switch to PMIC        
    
    //BC11_RST=1
    ret_val=pmic_config_interface(CHR_CON18,0x1,PMIC_RG_BC11_RST_MASK,PMIC_RG_BC11_RST_SHIFT); 
    //BC11_BB_CTRL=1
    ret_val=pmic_config_interface(CHR_CON18,0x1,PMIC_RG_BC11_BB_CTRL_MASK,PMIC_RG_BC11_BB_CTRL_SHIFT);
    
    //RG_BC11_BIAS_EN=1    
    ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_BIAS_EN_MASK,PMIC_RG_BC11_BIAS_EN_SHIFT); 
    //RG_BC11_VSRC_EN[1:0]=00
    ret_val=pmic_config_interface(CHR_CON18,0x0,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_VREF_VTH = 0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_VREF_VTH_MASK,PMIC_RG_BC11_VREF_VTH_SHIFT); 
    //RG_BC11_CMP_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
    //RG_BC11_IPU_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
    //RG_BC11_IPD_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);

    //ret_val=pmic_read_interface(CHR_CON18,&reg_val,0xFFFF,0);        
    //dprintf(INFO, "Reg[0x%x]=%x, ", CHR_CON18, reg_val);
    //ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);        
    //dprintf(INFO, "Reg[0x%x]=%x \n", CHR_CON19, reg_val);

/********* Step A *************************************/
    //dprintf(INFO, "mt_charger_type_detection : step A\r\n");
    
    //RG_BC11_IPU_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
    
    SETREG16(USBPHYRegs+0x1C,0x1000);//RG_PUPD_BIST_EN = 1    
    CLRREG16(USBPHYRegs+0x1C,0x0400);//RG_EN_PD_DM=0
    
    //RG_BC11_VSRC_EN[1.0] = 10 
    ret_val=pmic_config_interface(CHR_CON18,0x2,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_IPD_EN[1:0] = 01
    ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);
    //RG_BC11_VREF_VTH = 0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_VREF_VTH_MASK,PMIC_RG_BC11_VREF_VTH_SHIFT);
    //RG_BC11_CMP_EN[1.0] = 01
    ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);

    mdelay(100);
        
    ret_val=pmic_read_interface(CHR_CON18,&wChargerAvail,PMIC_RGS_BC11_CMP_OUT_MASK,PMIC_RGS_BC11_CMP_OUT_SHIFT); 
    //dprintf(INFO, "mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
    
    //RG_BC11_VSRC_EN[1:0]=00
    ret_val=pmic_config_interface(CHR_CON18,0x0,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_IPD_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);
    //RG_BC11_CMP_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
    
    mdelay(50);
    
    if(wChargerAvail==1)
    {
/********* Step B *************************************/
        //dprintf(INFO, "mt_charger_type_detection : step B\r\n");

        //RG_BC11_IPU_EN[1:0]=10
        ret_val=pmic_config_interface(CHR_CON19,0x2,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);        

        mdelay(80);
        
        bLineState_B = INREG16(USBPHYRegs+0x76);
        //dprintf(INFO, "mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
        if(bLineState_B & 0x80)
        {
            ret = STANDARD_CHARGER;
            dprintf(CRITICAL, "mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
        }
        else
        {
            ret = CHARGING_HOST;
            dprintf(CRITICAL, "mt_charger_type_detection : step B : Charging Host!\r\n");
        }
    }
    else
    {
/********* Step C *************************************/
        //dprintf(INFO, "mt_charger_type_detection : step C\r\n");

        //RG_BC11_IPU_EN[1:0]=01
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
        //RG_BC11_CMP_EN[1.0] = 01
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
        
        //ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);        
        //dprintf(INFO, "mt_charger_type_detection : step C : Reg[0x%x]=%x\r\n", CHR_CON19, reg_val);        
        
        mdelay(80);
                
        ret_val=pmic_read_interface(CHR_CON18,&bLineState_C,0xFFFF,0);
        //dprintf(INFO, "mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
        if(bLineState_C & 0x0080)
        {
            ret = NONSTANDARD_CHARGER;
            dprintf(CRITICAL, "mt_charger_type_detection : step C : UNSTANDARD CHARGER!!!\r\n");
            
            //RG_BC11_IPU_EN[1:0]=10
            ret_val=pmic_config_interface(CHR_CON19,0x2,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
            
            mdelay(80);
        }
        else
        {
            ret = STANDARD_HOST;
            dprintf(CRITICAL, "mt_charger_type_detection : step C : Standard USB Host!!\r\n");
        }
    }
/********* Finally setting *******************************/

    //RG_BC11_VSRC_EN[1:0]=00
    ret_val=pmic_config_interface(CHR_CON18,0x0,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_VREF_VTH = 0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_VREF_VTH_MASK,PMIC_RG_BC11_VREF_VTH_SHIFT);
    //RG_BC11_CMP_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
    //RG_BC11_IPU_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
    //RG_BC11_IPD_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);
    //RG_BC11_BIAS_EN=0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_BIAS_EN_MASK,PMIC_RG_BC11_BIAS_EN_SHIFT); 
    
    CLRREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 0 : switch to USB

    //hwPowerDown(MT65XX_POWER_LDO_VUSB,"VUSB_LDO");
    //disable_pll(MT65XX_UPLL,"USB_PLL");
    //dprintf(INFO, "[hw_charger_type_detection] disable VUSB and UPLL before disconnect\n");

    if( (ret==STANDARD_HOST) || (ret==CHARGING_HOST) )
    {
        dprintf(CRITICAL, "mt_charger_type_detection : SW workaround for USB\r\n");
        //RG_BC11_BB_CTRL=1
        ret_val=pmic_config_interface(CHR_CON18,0x1,PMIC_RG_BC11_BB_CTRL_MASK,PMIC_RG_BC11_BB_CTRL_SHIFT);
        //RG_BC11_BIAS_EN=1
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_BIAS_EN_MASK,PMIC_RG_BC11_BIAS_EN_SHIFT); 
        //RG_BC11_VSRC_EN[1.0] = 11        
        ret_val=pmic_config_interface(CHR_CON18,0x3,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT);
        //check
        ret_val=pmic_read_interface(CHR_CON18,&reg_val,0xFFFF,0);
        dprintf(INFO, "Reg[0x%x]=0x%x\n", CHR_CON18, reg_val);
        ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);
        dprintf(INFO, "Reg[0x%x]=0x%x\n", CHR_CON19, reg_val);
    }        
#endif

    if(Enable_PMIC_LOG>1) 
      dprintf(CRITICAL, "%d", ret_val);

    //step4:done, ret the type    
    return ret;
}

CHARGER_TYPE mt_charger_type_detection(void)
{
    if( g_first_check == 0 )
    {
        g_first_check = 1;
        g_ret = hw_charger_type_detection();
    }
    else
    {
        dprintf(CRITICAL, "[mt_charger_type_detection] Got data !!, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
}

