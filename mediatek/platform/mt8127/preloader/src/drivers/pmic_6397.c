#include <typedefs.h>
#include <platform.h>
#include <pmic_wrap_init.h>
#include <pmic.h>
#include <i2c.h>
#include "timer.h"


//#define PMIC_DEBUG
#ifdef PMIC_DEBUG
#define PMIC_PRINT   print
#else
#define PMIC_PRINT
#endif

//flag to indicate ca15 related power is ready
volatile int g_ca15_ready = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC access API
//////////////////////////////////////////////////////////////////////////////////////////
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6320_reg = 0;
    U32 rdata;    

    //mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;
    if(return_value!=0)
    {   
        PMIC_PRINT("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    PMIC_PRINT("[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
    
    pmic6320_reg &= (MASK << SHIFT);
    *val = (pmic6320_reg >> SHIFT);    
    PMIC_PRINT("[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6320_reg = 0;
    U32 rdata;

    //1. mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;    
    if(return_value!=0)
    {   
        PMIC_PRINT("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    PMIC_PRINT("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
    
    pmic6320_reg &= ~(MASK << SHIFT);
    pmic6320_reg |= (val << SHIFT);

    //2. mt6320_write_byte(RegNum, pmic6320_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic6320_reg, &rdata);
    if(return_value!=0)
    {   
        PMIC_PRINT("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    PMIC_PRINT("[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic6320_reg);    

#if 0
    //3. Double Check    
    //mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;    
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
#endif    

    return return_value;
}

#if 0
U32 upmu_get_reg_value(U32 reg)
{
    U32 ret=0;
    U32 reg_val=0;

    //printf("[upmu_get_reg_value] \n");
    ret=pmic_read_interface(reg, &reg_val, 0xFFFF, 0x0);
    
    return reg_val;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC-Charger Type Detection
//////////////////////////////////////////////////////////////////////////////////////////
CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE ret                 = CHARGER_UNKNOWN;

#if 0    
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
    //printf("mt_charger_type_detection : start!\r\n");

/********* Step 0.0 : enable USB memory and clock *********/
    //enable_pll(MT65XX_UPLL,"USB_PLL");
    //hwPowerOn(MT65XX_POWER_LDO_VUSB,VOL_DEFAULT,"VUSB_LDO");
    //printf("[hw_charger_type_detection] enable VUSB and UPLL before connect\n");

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
    //printf("Reg[0x%x]=%x, ", CHR_CON18, reg_val);
    //ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);        
    //printf("Reg[0x%x]=%x \n", CHR_CON19, reg_val);

/********* Step A *************************************/
    //printf("mt_charger_type_detection : step A\r\n");
    
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
    //printf("mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
    
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
        //printf("mt_charger_type_detection : step B\r\n");

        //RG_BC11_IPU_EN[1:0]=10
        ret_val=pmic_config_interface(CHR_CON19,0x2,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);        

        mdelay(80);
        
        bLineState_B = INREG16(USBPHYRegs+0x76);
        //printf("mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
        if(bLineState_B & 0x80)
        {
            ret = STANDARD_CHARGER;
            printf("[PL] mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
        }
        else
        {
            ret = CHARGING_HOST;
            printf("[PL] mt_charger_type_detection : step B : Charging Host!\r\n");
        }
    }
    else
    {
/********* Step C *************************************/
        //printf("mt_charger_type_detection : step C\r\n");

        //RG_BC11_IPU_EN[1:0]=01
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
        //RG_BC11_CMP_EN[1.0] = 01
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
        
        //ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);        
        //printf("mt_charger_type_detection : step C : Reg[0x%x]=%x\r\n", CHR_CON19, reg_val);        
        
        mdelay(80);
                
        ret_val=pmic_read_interface(CHR_CON18,&bLineState_C,0xFFFF,0);
        //printf("mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
        if(bLineState_C & 0x0080)
        {
            ret = NONSTANDARD_CHARGER;
            printf("[PL] mt_charger_type_detection : step C : UNSTANDARD CHARGER!!!\r\n");
            
            //RG_BC11_IPU_EN[1:0]=10
            ret_val=pmic_config_interface(CHR_CON19,0x2,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
            
            mdelay(80);
        }
        else
        {
            ret = STANDARD_HOST;
            printf("[PL] mt_charger_type_detection : step C : Standard USB Host!!\r\n");
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
    //printf("[hw_charger_type_detection] disable VUSB and UPLL before disconnect\n");

    if( (ret==STANDARD_HOST) || (ret==CHARGING_HOST) )
    {
        printf("[PL] mt_charger_type_detection : SW workaround for USB\r\n");
        //RG_BC11_BB_CTRL=1
        ret_val=pmic_config_interface(CHR_CON18,0x1,PMIC_RG_BC11_BB_CTRL_MASK,PMIC_RG_BC11_BB_CTRL_SHIFT);
        //RG_BC11_BIAS_EN=1
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_BIAS_EN_MASK,PMIC_RG_BC11_BIAS_EN_SHIFT); 
        //RG_BC11_VSRC_EN[1.0] = 11        
        ret_val=pmic_config_interface(CHR_CON18,0x3,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT);
        //check
        ret_val=pmic_read_interface(CHR_CON18,&reg_val,0xFFFF,0);
        printf("Reg[0x%x]=0x%x\n", CHR_CON18, reg_val);
        ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);
        printf("Reg[0x%x]=0x%x\n", CHR_CON19, reg_val);
    }        
#endif

    //step4:done, ret the type    
    return ret;    
}

#ifdef MTK_BQ24297_SUPPORT
int bq24297_read_byte(unsigned char reg, unsigned char *val)
{
    int ret = -1;
    //unsigned char reg = 0xA;
    //unsigned char val = 0;
    
    ret = mt_i2c_write(I2C4, 0xD6, &reg, 1, 1);	 // set register command
	if (ret != I2C_OK)
	{
	    printf("@@@ ret = %d\n", ret);
		return ret;
	}
	//ret = mt_i2c_read(I2C4, 0xD7, &val, 1, 1);
	ret = mt_i2c_read(I2C4, 0xD7, val, 1, 1);
    if (ret != I2C_OK)
	{
	    printf("@@@ ret = %d\n", ret);
		return ret;
	}
    else
    {
        printf("@@@@@@@@@@@@@@@@@@@@@@\r\n");
        printf("@@@@@@@@@@@@@@@@@@@@@@ val = %d\r\n", *val);
        return ret;
    }
}

CHARGER_TYPE bq24297_charger_type_detection(void)
{   
    unsigned int USB_U2PHYACR6_2     = 0x1121081A;
    unsigned int USBPHYRegs          = 0x11210800; //U2B20_Base+0x800
    CHARGER_TYPE charger_type = CHARGER_UNKNOWN;
    unsigned char val = 0;
    unsigned int m = 0;
    int ret = -1;
    unsigned char temp[2];
    
    /*
    if (bq24297_read_byte(0xA, &val) == I2C_OK)
    {
        printf("@@@@@@@@@@@@@@@@@@@@ PN = %x\n", val);
    }
    */
    
    while ((m < 1000) && (charger_type == CHARGER_UNKNOWN))
    {
        if (bq24297_read_byte(0x8, &val) == I2C_OK)
        {
            //printf("@@@@@@@@@@@@@@@@@@@@ REG08 = %x\n", val);
            switch(val & 0xC0)
            {
                case 0x40:
                    charger_type = STANDARD_HOST;
                    break;
                case 0x80:
                    charger_type = STANDARD_CHARGER;
                    break;
                default:
                    charger_type = CHARGER_UNKNOWN;
                    break;
            }
        }
        m++;
    }
    printf("m=%d type = %d\n", m, charger_type);
    charger_type = CHARGER_UNKNOWN;
        
    //SETREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 1 : switch to PMIC
    CLRREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 0 : switch to USB    

    //mdelay(100);
            
    //1. force charger type detection    
    if (bq24297_read_byte(0x7, &val) == I2C_OK)
    {        
        temp[0] = 0x7;
        temp[1] = (val | 0x80);
        ret = mt_i2c_write(I2C4, 0xD6, &temp, 2, 1);	 // set register command
    	if (ret != I2C_OK)
    	{
    	    printf("@@@@@@@@ DPDM ret = %d\n", ret);
    		return ret;
    	}        
    }    
        
    m = 0;
    while (m < 1000)
    {
        if (bq24297_read_byte(0x7, &val) == I2C_OK)
        {
            if (!(val & 0x80))
            {
                break;
            }
            m++;
            mdelay(1);
        }
    }
    //printf("m=%d m=%d m=%d m=%d m=%d m=%d m=%d\n", m, m, m, m, m, m, m);
    
    //CLRREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 0 : switch to USB
    
    if (bq24297_read_byte(0x8, &val) == I2C_OK)
    {
        //printf("@@@@@@@@@@@@@@@@@@@@ REG08 = %x\n", val);
        switch(val & 0xC0)
        {
            case 0x40:
                //charger_type = STANDARD_HOST;
                charger_type = hw_charger_type_detection();
                break;
            case 0x80:
                charger_type = STANDARD_CHARGER;
                break;
            default:
                charger_type = CHARGER_UNKNOWN;
                break;
        }
    }
    printf("m=%d type = %d\n", m, charger_type);
    
    if (charger_type == STANDARD_CHARGER)
    {
        if (bq24297_read_byte(0x0, &val) == I2C_OK)
        {
            if ((val & 0x7) < 0x4) //0x4:1A
            {
                charger_type = NONSTANDARD_CHARGER;            
            }
        }
    }
    
    return charger_type;
}
#endif

CHARGER_TYPE mt_charger_type_detection(void)
{
    if( g_first_check == 0 )
    {
        g_first_check = 1;
#ifndef MTK_BQ24297_SUPPORT
        g_ret = hw_charger_type_detection();
#else
        g_ret = bq24297_charger_type_detection();        
#endif
    }
    else
    {
        printf("[mt_charger_type_detection] Got data !!, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
}

//==============================================================================
// PMIC63297 Usage APIs
//==============================================================================
U32 get_pmic6397_chip_version (void)
{
    U32 ret=0;
    U32 eco_version = 0;
    
    ret=pmic_read_interface( (U32)(CID),
                             (&eco_version),
                             (U32)(PMIC_CID_MASK),
                             (U32)(PMIC_CID_SHIFT)
                             );

    return eco_version;
}

U32 pmic_IsUsbCableIn (void) 
{    
    U32 ret=0;
    U32 val=0;
    
    ret=pmic_read_interface( (U32)(CHR_CON0),
                             (&val),
                             (U32)(PMIC_RGS_CHRDET_MASK),
                             (U32)(PMIC_RGS_CHRDET_SHIFT)
                             );


    if(val)
        return PMIC_CHRDET_EXIST;
    else
        return PMIC_CHRDET_NOT_EXIST;
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
    if (val==1){     
        printf("[pmic_detect_powerkey_PL] Release\n");
        return 0;
    }else{
        printf("[pmic_detect_powerkey_PL] Press\n");
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

    if (val==1){     
        printf("[pmic_detect_homekey_PL] Release\n");
        return 0;
    }else{
        printf("[pmic_detect_homekey_PL] Press\n");
        return 1;
    }
}

void hw_set_cc(int cc_val)
{
    U32 ret_val=0;
    U32 reg_val=0;    
    U32 i=0;
    U32 hw_charger_ov_flag=0;

    printf("hw_set_cc: %d\r\n", cc_val);
    
    //VCDT_HV_VTH, 7V
    ret_val=pmic_config_interface(CHR_CON1, 0x0B, PMIC_RG_VCDT_HV_VTH_MASK, PMIC_RG_VCDT_HV_VTH_SHIFT); 
    //VCDT_HV_EN=1
    ret_val=pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_VCDT_HV_EN_MASK, PMIC_RG_VCDT_HV_EN_SHIFT); 
    //CS_EN=1
    ret_val=pmic_config_interface(CHR_CON2, 0x01, PMIC_RG_CS_EN_MASK, PMIC_RG_CS_EN_SHIFT);
    //CSDAC_MODE=1
    ret_val=pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_CSDAC_MODE_MASK, PMIC_RG_CSDAC_MODE_SHIFT);

    ret_val=pmic_read_interface(CHR_CON0, &hw_charger_ov_flag, PMIC_RGS_VCDT_HV_DET_MASK, PMIC_RGS_VCDT_HV_DET_SHIFT);
    if(hw_charger_ov_flag == 1)
    {
        ret_val=pmic_config_interface(CHR_CON0, 0x00, PMIC_RG_CHR_EN_MASK, PMIC_RG_CHR_EN_SHIFT);
        printf("[PreLoader_charger_ov] turn off charging \n"); 
        return;
    }

    // CS_VTH
    switch(cc_val){
        case 1600: ret_val=pmic_config_interface(CHR_CON4, 0x00, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1500: ret_val=pmic_config_interface(CHR_CON4, 0x01, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;       
        case 1400: ret_val=pmic_config_interface(CHR_CON4, 0x02, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1300: ret_val=pmic_config_interface(CHR_CON4, 0x03, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1200: ret_val=pmic_config_interface(CHR_CON4, 0x04, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1100: ret_val=pmic_config_interface(CHR_CON4, 0x05, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1000: ret_val=pmic_config_interface(CHR_CON4, 0x06, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 900:  ret_val=pmic_config_interface(CHR_CON4, 0x07, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;            
        case 800:  ret_val=pmic_config_interface(CHR_CON4, 0x08, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 700:  ret_val=pmic_config_interface(CHR_CON4, 0x09, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;       
        case 650:  ret_val=pmic_config_interface(CHR_CON4, 0x0A, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 550:  ret_val=pmic_config_interface(CHR_CON4, 0x0B, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 450:  ret_val=pmic_config_interface(CHR_CON4, 0x0C, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 400:  ret_val=pmic_config_interface(CHR_CON4, 0x0D, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 200:  ret_val=pmic_config_interface(CHR_CON4, 0x0E, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 70:   ret_val=pmic_config_interface(CHR_CON4, 0x0F, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;            
        default:
            dbg_print("hw_set_cc: argument invalid!!\r\n");
            break;
    }

    //upmu_chr_csdac_dly(0x4);                // CSDAC_DLY
    ret_val=pmic_config_interface(CHR_CON21, 0x04, PMIC_RG_CSDAC_DLY_MASK, PMIC_RG_CSDAC_DLY_SHIFT);
    //upmu_chr_csdac_stp(0x1);                // CSDAC_STP
    ret_val=pmic_config_interface(CHR_CON21, 0x01, PMIC_RG_CSDAC_STP_MASK, PMIC_RG_CSDAC_STP_SHIFT);
    //upmu_chr_csdac_stp_inc(0x1);            // CSDAC_STP_INC
    ret_val=pmic_config_interface(CHR_CON20, 0x01, PMIC_RG_CSDAC_STP_INC_MASK, PMIC_RG_CSDAC_STP_INC_SHIFT);
    //upmu_chr_csdac_stp_dec(0x2);            // CSDAC_STP_DEC
    ret_val=pmic_config_interface(CHR_CON20, 0x02, PMIC_RG_CSDAC_STP_DEC_MASK, PMIC_RG_CSDAC_STP_DEC_SHIFT);
    //upmu_chr_chrwdt_td(0x0);                // CHRWDT_TD, 4s
    ret_val=pmic_config_interface(CHR_CON13, 0x00, PMIC_RG_CHRWDT_TD_MASK, PMIC_RG_CHRWDT_TD_SHIFT);
    //upmu_chr_chrwdt_int_en(1);              // CHRWDT_INT_EN
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_INT_EN_MASK, PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //upmu_chr_chrwdt_en(1);                  // CHRWDT_EN
    ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);
    //upmu_chr_chrwdt_flag_wr(1);             // CHRWDT_FLAG
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_FLAG_WR_MASK, PMIC_RG_CHRWDT_FLAG_WR_SHIFT);
    //upmu_chr_csdac_enable(1);               // CSDAC_EN
    ret_val=pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_CSDAC_EN_MASK, PMIC_RG_CSDAC_EN_SHIFT);
    //upmu_set_rg_hwcv_en(1);                 // HWCV_EN
    ret_val=pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_HWCV_EN_MASK, PMIC_RG_HWCV_EN_SHIFT);
    //upmu_chr_enable(1);                     // CHR_EN
    ret_val=pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_CHR_EN_MASK, PMIC_RG_CHR_EN_SHIFT);

    for(i=CHR_CON0 ; i<=CHR_CON29 ; i++)    
    {        
        ret_val=pmic_read_interface(i,&reg_val,0xFFFF,0x0);        
        print("[PreLoader] Bank0[0x%x]=0x%x\n", i, reg_val);    
    }

    printf("hw_set_cc: done\r\n");    
}

void pl_hw_ulc_det(void)
{
    U32 ret_val=0;
    
    //upmu_chr_ulc_det_en(1);            // RG_ULC_DET_EN=1
    ret_val=pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_ULC_DET_EN_MASK, PMIC_RG_ULC_DET_EN_SHIFT);
    //upmu_chr_low_ich_db(1);            // RG_LOW_ICH_DB=000001'b
    ret_val=pmic_config_interface(CHR_CON22, 0x01, PMIC_RG_LOW_ICH_DB_MASK, PMIC_RG_LOW_ICH_DB_SHIFT);
}

int hw_check_battery(void)
{
#ifndef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
    U32 ret_val=0;
    U32 reg_val=0;

    ret_val=pmic_config_interface(CHR_CON7,    0x01, PMIC_RG_BATON_EN_MASK, PMIC_RG_BATON_EN_SHIFT);      //BATON_EN=1
    ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_BATON_TDET_EN_MASK, PMIC_BATON_TDET_EN_SHIFT);  //BATON_TDET_EN=0
    ret_val=pmic_config_interface(AUXADC_CON0, 0x00, PMIC_RG_BUF_PWD_B_MASK, PMIC_RG_BUF_PWD_B_SHIFT);    //RG_BUF_PWD_B=0
    //dump to check
    ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFFFF,0x0);    print("[hw_check_battery+] [0x%x]=0x%x\n",CHR_CON7,reg_val);
    ret_val=pmic_read_interface(AUXADC_CON0,&reg_val,0xFFFF,0x0); print("[hw_check_battery+] [0x%x]=0x%x\n",AUXADC_CON0,reg_val);

    ret_val=pmic_read_interface(CHR_CON7, &reg_val, PMIC_RGS_BATON_UNDET_MASK, PMIC_RGS_BATON_UNDET_SHIFT);

    if (reg_val == 1)
    {                     
        printf("[pl_hw_check_battery] No Battery!!\n");

        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_RG_BATON_EN_MASK, PMIC_RG_BATON_EN_SHIFT);      //BATON_EN=0
        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_BATON_TDET_EN_MASK, PMIC_BATON_TDET_EN_SHIFT);  //BATON_TDET_EN=0
        //ret_val=pmic_config_interface(AUXADC_CON0, 0x00, PMIC_RG_BUF_PWD_B_MASK, PMIC_RG_BUF_PWD_B_SHIFT);    //RG_BUF_PWD_B=0
        //dump to check
        ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFFFF,0x0);    print("[hw_check_battery-] [0x%x]=0x%x\n",CHR_CON7,reg_val);
        ret_val=pmic_read_interface(AUXADC_CON0,&reg_val,0xFFFF,0x0); print("[hw_check_battery-] [0x%x]=0x%x\n",AUXADC_CON0,reg_val);                
        
        return 0;        
    }
    else
    {
        printf("[pl_hw_check_battery] Battery exist!!\n");

        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_RG_BATON_EN_MASK, PMIC_RG_BATON_EN_SHIFT);      //BATON_EN=0
        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_BATON_TDET_EN_MASK, PMIC_BATON_TDET_EN_SHIFT);  //BATON_TDET_EN=0
        //ret_val=pmic_config_interface(AUXADC_CON0, 0x00, PMIC_RG_BUF_PWD_B_MASK, PMIC_RG_BUF_PWD_B_SHIFT);    //RG_BUF_PWD_B=0
        //dump to check
        ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFF,0x0);    print("[hw_check_battery-] [0x%x]=0x%x\n",CHR_CON7,reg_val);
        ret_val=pmic_read_interface(AUXADC_CON0,&reg_val,0xFF,0x0); print("[hw_check_battery-] [0x%x]=0x%x\n",AUXADC_CON0,reg_val);
    
        pl_hw_ulc_det();
        
        return 1;
    }
#else
	    return 1;
#endif
}

void pl_charging(int en_chr)
{
    U32 ret_val=0;
    U32 reg_val=0;
    U32 i=0;
    
    if(en_chr == 1)
    {
        printf("[pl_charging] enable\n");
    
        hw_set_cc(450);

        //USBDL set 1
        ret_val=pmic_config_interface(CHR_CON16, 0x01, PMIC_RG_USBDL_SET_MASK, PMIC_RG_USBDL_SET_SHIFT);        
    }
    else
    {
        printf("[pl_charging] disable\n");
    
        //USBDL set 0
        ret_val=pmic_config_interface(CHR_CON16, 0x00, PMIC_RG_USBDL_SET_MASK, PMIC_RG_USBDL_SET_SHIFT);

        //upmu_set_rg_hwcv_en(0); // HWCV_EN
        ret_val=pmic_config_interface(CHR_CON23, 0x00, PMIC_RG_HWCV_EN_MASK, PMIC_RG_HWCV_EN_SHIFT);
        //upmu_chr_enable(0); // CHR_EN
        ret_val=pmic_config_interface(CHR_CON0, 0x00, PMIC_RG_CHR_EN_MASK, PMIC_RG_CHR_EN_SHIFT);        
    }

    for(i=CHR_CON0 ; i<=CHR_CON29 ; i++)    
    {        
        ret_val=pmic_read_interface(i,&reg_val,0xFFFF,0x0);        
        print("[pl_charging] Bank0[0x%x]=0x%x\n", i, reg_val);    
    }
}

void pl_kick_chr_wdt(void)
{
    int ret_val=0;

    //upmu_chr_chrwdt_td(0x0);                // CHRWDT_TD
    ret_val=pmic_config_interface(CHR_CON13, 0x03, PMIC_RG_CHRWDT_TD_MASK, PMIC_RG_CHRWDT_TD_SHIFT);
    //upmu_chr_chrwdt_int_en(1);             // CHRWDT_INT_EN
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_INT_EN_MASK, PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //upmu_chr_chrwdt_en(1);                   // CHRWDT_EN
    ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);
    //upmu_chr_chrwdt_flag_wr(1);            // CHRWDT_FLAG
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_FLAG_WR_MASK, PMIC_RG_CHRWDT_FLAG_WR_SHIFT);

    //printf("[pl_kick_chr_wdt] done\n");
}

void pl_close_pre_chr_led(void)
{
    U32 ret_val=0;    

    ret_val=pmic_config_interface(CHR_CON22, 0x00, PMIC_RG_CHRIND_ON_MASK, PMIC_RG_CHRIND_ON_SHIFT);
    
    printf("[pmic6397_init] Close pre-chr LED\n");
}

//==============================================================================
// PMIC6320 Init Code
//==============================================================================
void PMIC_INIT_SETTING_V1(void)
{
    U32 chip_version = 0;
    U32 ret = 0;
    U32 reg_val=0;
    ret = pmic_read_interface( (U32)(CID),&chip_version,(U32)(PMIC_CID_MASK),(U32)(PMIC_CID_SHIFT));

    if(chip_version >= PMIC6397_E1_CID_CODE)
    {
        print("[pmic6397_init][Preloader_PMIC_INIT_SETTING_V1] PMIC Chip = %x\n",chip_version);
        
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
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x002, upmu_get_reg_value(0x002));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x00C, upmu_get_reg_value(0x00C));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x01A, upmu_get_reg_value(0x01A));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x024, upmu_get_reg_value(0x024));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x02A, upmu_get_reg_value(0x02A));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x02E, upmu_get_reg_value(0x02E));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x102, upmu_get_reg_value(0x102));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x128, upmu_get_reg_value(0x128));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x130, upmu_get_reg_value(0x130));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x134, upmu_get_reg_value(0x134)); 
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x178, upmu_get_reg_value(0x178));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x17E, upmu_get_reg_value(0x17E));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x206, upmu_get_reg_value(0x206));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x210, upmu_get_reg_value(0x210));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x216, upmu_get_reg_value(0x216));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x21E, upmu_get_reg_value(0x21E));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x224, upmu_get_reg_value(0x224));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x238, upmu_get_reg_value(0x238));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x244, upmu_get_reg_value(0x244));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x24A, upmu_get_reg_value(0x24A));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x25E, upmu_get_reg_value(0x25E));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x260, upmu_get_reg_value(0x260));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x262, upmu_get_reg_value(0x262));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x264, upmu_get_reg_value(0x264));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x270, upmu_get_reg_value(0x270));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x276, upmu_get_reg_value(0x276));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x27C, upmu_get_reg_value(0x27C));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x28A, upmu_get_reg_value(0x28A));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x28E, upmu_get_reg_value(0x28E));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x29C, upmu_get_reg_value(0x29C));
        print("[pmic_init_setting] eg[0x%x]=0x%x\n", 0x2B0, upmu_get_reg_value(0x2B0));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x330, upmu_get_reg_value(0x330));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x332, upmu_get_reg_value(0x332));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x336, upmu_get_reg_value(0x336));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x33C, upmu_get_reg_value(0x33C));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x34A, upmu_get_reg_value(0x34A));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x356, upmu_get_reg_value(0x356));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x358, upmu_get_reg_value(0x358));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x35C, upmu_get_reg_value(0x35C));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x362, upmu_get_reg_value(0x362));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x370, upmu_get_reg_value(0x370));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x372, upmu_get_reg_value(0x372));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x374, upmu_get_reg_value(0x374));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x376, upmu_get_reg_value(0x376));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x39C, upmu_get_reg_value(0x39C));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x440, upmu_get_reg_value(0x440));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x500, upmu_get_reg_value(0x500));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x502, upmu_get_reg_value(0x502));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x508, upmu_get_reg_value(0x508));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x50C, upmu_get_reg_value(0x50C));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x512, upmu_get_reg_value(0x512));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x55E, upmu_get_reg_value(0x55E));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x560, upmu_get_reg_value(0x560));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x566, upmu_get_reg_value(0x566));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x600, upmu_get_reg_value(0x600));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x604, upmu_get_reg_value(0x604));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x606, upmu_get_reg_value(0x606));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x60A, upmu_get_reg_value(0x60A));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x612, upmu_get_reg_value(0x612));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x632, upmu_get_reg_value(0x632));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x638, upmu_get_reg_value(0x638));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x63A, upmu_get_reg_value(0x63A));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x63C, upmu_get_reg_value(0x63C));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x714, upmu_get_reg_value(0x714));
        print("[pmic_init_setting] Reg[0x%x]=0x%x\n", 0x71A, upmu_get_reg_value(0x71A));        
        #endif
    }
    else
    {
        print("[pmic6397_init][Preloader_PMIC_INIT_SETTING_V1] Unknown PMIC Chip = %x\n",chip_version);

    }
}

static void pmic_default_buck_voltage(void)
{
	int reg_val=0;
	int buck_val=0;
	pmic_read_interface(EFUSE_DOUT_288_303, &reg_val, 0xFFFF, 0);
	if ((reg_val &0x01) == 0x01) {
		print("[EFUSE_DOUT_288_303] FUSE 288=0x%x\n", reg_val);

		/* VCORE */
		pmic_read_interface(EFUSE_DOUT_256_271, &reg_val, 0xF, 12);		
		pmic_read_interface(VCORE_CON9, &buck_val, PMIC_VCORE_VOSEL_MASK, PMIC_VCORE_VOSEL_SHIFT);
		buck_val = (buck_val&0x03)|(reg_val<<3);
		pmic_config_interface(VCORE_CON9, buck_val, PMIC_VCORE_VOSEL_MASK, PMIC_VCORE_VOSEL_SHIFT);
		pmic_config_interface(VCORE_CON10, buck_val, PMIC_VCORE_VOSEL_ON_MASK, PMIC_VCORE_VOSEL_ON_SHIFT);

		pmic_read_interface(EFUSE_DOUT_272_287, &reg_val, 0xFFFF, 0);
		/* VCA15 */
		buck_val = 0;
		pmic_read_interface(VCA15_CON9, &buck_val, PMIC_VCA15_VOSEL_MASK, PMIC_VCA15_VOSEL_SHIFT);
		buck_val = (buck_val&0x03)|((reg_val&0x0F)<<3);
		pmic_config_interface(VCA15_CON9, buck_val, PMIC_VCA15_VOSEL_MASK, PMIC_VCA15_VOSEL_SHIFT);
		pmic_config_interface(VCA15_CON10, buck_val, PMIC_VCA15_VOSEL_ON_MASK, PMIC_VCA15_VOSEL_ON_SHIFT);

		/* VSAMRCA15 */
		buck_val = 0;
		pmic_read_interface(VSRMCA15_CON9, &buck_val, PMIC_VSRMCA15_VOSEL_MASK, PMIC_VSRMCA15_VOSEL_SHIFT);
		buck_val = (buck_val&0x03)|((reg_val&0xF0)>>1);
		pmic_config_interface(VSRMCA15_CON9, buck_val, PMIC_VSRMCA15_VOSEL_MASK, PMIC_VSRMCA15_VOSEL_SHIFT);
		pmic_config_interface(VSRMCA15_CON10, buck_val, PMIC_VSRMCA15_VOSEL_ON_MASK, PMIC_VSRMCA15_VOSEL_ON_SHIFT);

		/* VCA7 */
		buck_val = 0;
		pmic_read_interface(VPCA7_CON9, &buck_val, PMIC_VPCA7_VOSEL_MASK, PMIC_VPCA7_VOSEL_SHIFT);
		buck_val = (buck_val&0x03)|((reg_val&0xF00)>>5);
		pmic_config_interface(VPCA7_CON9, buck_val, PMIC_VPCA7_VOSEL_MASK, PMIC_VPCA7_VOSEL_SHIFT);
		pmic_config_interface(VPCA7_CON10, buck_val, PMIC_VPCA7_VOSEL_ON_MASK, PMIC_VPCA7_VOSEL_ON_SHIFT);

		/* VSAMRCA7 */
		buck_val = 0;
		pmic_read_interface(VSRMCA7_CON9, &buck_val, PMIC_VPCA7_VOSEL_MASK, PMIC_VPCA7_VOSEL_SHIFT);
		buck_val = (buck_val&0x03)|((reg_val&0xF000)>>9);
		pmic_config_interface(VSRMCA7_CON9, buck_val, PMIC_VSRMCA7_VOSEL_MASK, PMIC_VSRMCA7_VOSEL_SHIFT); 
		pmic_config_interface(VSRMCA7_CON10, buck_val, PMIC_VSRMCA7_VOSEL_ON_MASK, PMIC_VSRMCA7_VOSEL_ON_SHIFT);

		pmic_config_interface(BUCK_CON3,0x1,0x1,12);
	}
}

U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;
    int ret_val=0;
    int reg_val=0;
#ifdef MTK_BQ24297_SUPPORT    
    unsigned int USB_U2PHYACR6_2     = 0x1121081A;
#endif    

    print("[pmic6397_init] Start..................\n");

    /* Adjust default BUCK voltage */
    pmic_default_buck_voltage();

    //Enable PMIC RST function (depends on main chip RST function)
    ret_val=pmic_config_interface(TOP_RST_MISC,  0x1, PMIC_RG_SYSRSTB_EN_MASK, PMIC_RG_SYSRSTB_EN_SHIFT);
    //ret_val=pmic_config_interface(TOP_RST_MISC,  0x1, PMIC_RG_STRUP_MAN_RST_EN_MASK, PMIC_RG_STRUP_MAN_RST_EN_SHIFT);
    ret_val=pmic_read_interface(TOP_RST_MISC, &reg_val, 0xFFFF, 0);
    print("[pmic6397_init] Enable PMIC RST function (depends on main chip RST function) Reg[0x%x]=0x%x\n", TOP_RST_MISC, reg_val);
    
    //Enable CA15 by default for different PMIC behavior
    pmic_config_interface(VCA15_CON7, 0x1, PMIC_VCA15_EN_MASK, PMIC_VCA15_EN_SHIFT);
    pmic_config_interface(VSRMCA15_CON7, 0x1, PMIC_VSRMCA15_EN_MASK, PMIC_VSRMCA15_EN_SHIFT);
    gpt_busy_wait_us(200);
    g_ca15_ready = 1;
        
    ret_val=pmic_read_interface(VCA15_CON7, &reg_val, 0xFFFF, 0);
    print("Reg[0x%x]=0x%x\n", VCA15_CON7, reg_val);
    ret_val=pmic_read_interface(VSRMCA15_CON7, &reg_val, 0xFFFF, 0);
    print("Reg[0x%x]=0x%x\n", VSRMCA15_CON7, reg_val);
    

#if 1
    //Adjust VCORE voltage: Set VCORE to 1.15V
    pmic_config_interface(VCORE_CON9, 0x48, PMIC_VCORE_VOSEL_MASK, PMIC_VCORE_VOSEL_SHIFT);
    pmic_config_interface(VCORE_CON10, 0x48, PMIC_VCORE_VOSEL_ON_MASK, PMIC_VCORE_VOSEL_ON_SHIFT );
#endif

#if 0 //SS: VPROC 1.25; VCORE: 1.15
    //set VPROC to 1.25V
    pmic_config_interface(VCA15_CON9, 0x58, PMIC_VCA15_VOSEL_MASK, PMIC_VCA15_VOSEL_SHIFT);
    pmic_config_interface(VCA15_CON10, 0x58, PMIC_VCA15_VOSEL_ON_MASK, PMIC_VCA15_VOSEL_ON_SHIFT);
    //set VCORE to 1.15V, already set. No need to set again.     
    //get settings
    pmic_read_interface(VCA15_CON12,&reg_val,PMIC_NI_VCA15_VOSEL_MASK,PMIC_NI_VCA15_VOSEL_SHIFT);
    print("[SS]VPROC setting 0x%x, should be 0x58\n", reg_val);
    pmic_read_interface(VCORE_CON12,&reg_val,PMIC_NI_VCORE_VOSEL_MASK, PMIC_NI_VCORE_VOSEL_SHIFT);
    print("[SS]VCORE setting 0x%x, should be 0x48\n", reg_val);
#endif

#if 0 //TT-: VPROC 1.15; VCORE: 1.15
    //set VPROC to 1.15V, it is default value. No need to set again. 
    //set VCORE to 1.15V, already set. No need to set again.
    //get settings
    pmic_read_interface(VCA15_CON12,&reg_val,PMIC_NI_VCA15_VOSEL_MASK,PMIC_NI_VCA15_VOSEL_SHIFT);
    print("[TT-]VPROC setting 0x%x, should be 0x48\n", reg_val);
    pmic_read_interface(VCORE_CON12,&reg_val,PMIC_NI_VCORE_VOSEL_MASK, PMIC_NI_VCORE_VOSEL_SHIFT);
    print("[TT-]VCORE setting 0x%x, should be 0x48\n", reg_val);

#endif

#if 0 //TT+: VPROC 1.25; VCORE: 1.25
    //set VPROC to 1.25V
    pmic_config_interface(VCA15_CON9, 0x58, PMIC_VCA15_VOSEL_MASK, PMIC_VCA15_VOSEL_SHIFT);
    pmic_config_interface(VCA15_CON10, 0x58, PMIC_VCA15_VOSEL_ON_MASK, PMIC_VCA15_VOSEL_ON_SHIFT);
    //set VCORE to 1.25V
    pmic_config_interface(VCORE_CON9, 0x58, PMIC_VCORE_VOSEL_MASK, PMIC_VCORE_VOSEL_SHIFT);
    pmic_config_interface(VCORE_CON10, 0x58, PMIC_VCORE_VOSEL_ON_MASK, PMIC_VCORE_VOSEL_ON_SHIFT);
    //get settings
    pmic_read_interface(VCA15_CON12,&reg_val,PMIC_NI_VCA15_VOSEL_MASK,PMIC_NI_VCA15_VOSEL_SHIFT);
    print("[TT+]VPROC setting 0x%x, should be 0x58\n", reg_val);
    pmic_read_interface(VCORE_CON12,&reg_val,PMIC_NI_VCORE_VOSEL_MASK, PMIC_NI_VCORE_VOSEL_SHIFT);
    print("[TT+]VCORE setting 0x%x, should be 0x58\n", reg_val);
#endif

#if 0 //FF: VPROC 1.15; VCORE: 1.25
    //set VPROC to 1.15V, it is default value. No need to set again. 
    //set VCORE to 1.25V
    pmic_config_interface(VCORE_CON9, 0x58, PMIC_VCORE_VOSEL_MASK, PMIC_VCORE_VOSEL_SHIFT);
    pmic_config_interface(VCORE_CON10, 0x58, PMIC_VCORE_VOSEL_ON_MASK, PMIC_VCORE_VOSEL_ON_SHIFT);
    //get settings
    pmic_read_interface(VCA15_CON12,&reg_val,PMIC_NI_VCA15_VOSEL_MASK,PMIC_NI_VCA15_VOSEL_SHIFT);
    print("[FF]VPROC setting 0x%x, should be 0x48\n", reg_val);
    pmic_read_interface(VCORE_CON12,&reg_val,PMIC_NI_VCORE_VOSEL_MASK, PMIC_NI_VCORE_VOSEL_SHIFT);
    print("[FF]VCORE setting 0x%x, should be 0x58\n", reg_val);
#endif

    //pmic initial setting
    PMIC_INIT_SETTING_V1();
    print("[PMIC_INIT_SETTING_V1] Done\n");

    /* Disable non-used buck: VPCA7, VSAMR7, VPVSRAM15*/
    pmic_config_interface(VPCA7_CON7, 0x0, PMIC_VPCA7_EN_MASK, PMIC_VPCA7_EN_SHIFT);
    pmic_config_interface(VSRMCA7_CON7, 0x0, PMIC_VSRMCA7_EN_MASK, PMIC_VSRMCA7_EN_SHIFT);
    pmic_config_interface(VSRMCA15_CON7, 0x0, PMIC_VSRMCA15_EN_MASK, PMIC_VSRMCA15_EN_SHIFT);

    //26M clock amplitute adjust
    pmic_config_interface(RG_DCXO_ANALOG_CON1, 0x0, PMIC_RG_DCXO_LDO_BB_V_MASK, PMIC_RG_DCXO_LDO_BB_V_SHIFT);
    pmic_config_interface(RG_DCXO_ANALOG_CON1, 0x1, PMIC_RG_DCXO_ATTEN_BB_MASK, PMIC_RG_DCXO_ATTEN_BB_SHIFT);

    //cal 12Mhz clock if no efuse
    //vWrite2BCbus(0x039E, 0x0041);
    //vWrite2BCbus(0x039E, 0x0040);	
	//vWrite2BCbus(0x039E, 0x0050);
	pmic_read_interface(EFUSE_DOUT_304_319, &reg_val, 0xFFFF, 0);
	if ((reg_val & 0x8000) == 0)
	{
	    pmic_config_interface(BUCK_K_CON0, 0x0041, 0xFFFF, 0);
	    pmic_config_interface(BUCK_K_CON0, 0x0040, 0xFFFF, 0);
	    pmic_config_interface(BUCK_K_CON0, 0x0050, 0xFFFF, 0);
    }

#ifdef MTK_BQ24297_SUPPORT
    CLRREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 0 : switch DPDM to USB
#endif    

#ifndef EVB_PLATFORM
    //Enable PMIC HW reset function
    //ret_val=pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
    //ret_val=pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
#endif    

    hw_check_battery();
    printf("[pmic6397_init] hw_check_battery\n");

    //if (PMIC_CHRDET_EXIST == pmic_IsUsbCableIn())
    //    mt_charger_type_detection();

    print("[pmic6397_init] Done...................\n");

    return ret_code;
}

