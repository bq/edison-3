#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
#define TPD_POWER_SOURCE         
#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

#define TPD_VELOCITY_CUSTOM_X 15
#define TPD_VELOCITY_CUSTOM_Y 20

#define TPD_POWER_SOURCE_CUSTOM         MT6323_POWER_LDO_VGP2

#define TPD_DELAY                (2*HZ/100)

#define TARGET_WIDTH                800
#define TARGET_HEIGHT                1280
#define ORI_WIDTH        768
#define ORI_HEIGHT       1024
#define TPD_CALIBRATION_MATRIX  {962,0,0,0,1600,0,0,0};

//#define TPD_HAVE_CALIBRATION
//#define TPD_HAVE_BUTTON
//#define TPD_HAVE_TREMBLE_ELIMINATION
#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (100)
#define TPD_KEY_COUNT           4
#define TPD_KEYS                { KEY_MENU, KEY_HOMEPAGE ,KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,850,120,TPD_BUTTON_HEIGH},{180,850,120,TPD_BUTTON_HEIGH},{300,850,120,TPD_BUTTON_HEIGH}, {420,850,120,TPD_BUTTON_HEIGH}}


#define FTS_CTL_IIC
#define __UPDATE_FOCALTECH_TP_FRAMEWARE__

// Added by xmylm, to replace the model name in C file, such as MALAAT_D8010
#define WIDTH_SCALE


#define WAP_Y       0
#define WAP_X       0
#define CHANGE_X2Y  0
#define RES_X                800
#define RES_Y                1280
#define TPD_MAX_POINTS      10
#define TPD_TEN_POINTS      1
#define TPD_IIC_ADDRESS      0x70
//#define FT5606

/* if IC is ft5216 , id is 0x79 0x07 ;*/
/* if ic is ft5606 , id is 0x79 0x06 ;*/
/* if IC if ft5406 , id is 0x79 0x03 */
#define FT5X06_IC_ID2    0x03

/* if IC is ft5216 , AA=50 55=40 ;*/
/* if ic is ft5606 , AA=50 55=10;*/
#define UPGRADE_AA_DELAY        50
#define UPGRADE_55_DELAY        30

#define CHANGE_THRESHOLD_IN_CHARGE
#define REG80_THRESHOLD_IN_CHARGE  30
#define REG81_PEAK_IN_CHARGE       100
#define REG80_THRESHOLD_UN_CHARGE  20
#define REG81_PEAK_UN_CHARGE       60

#define CURRENT_FW_VERSION      0x12
static unsigned char CTPM_FW[]={
    #include "ft5406_app_12.i"
};


#endif /* TOUCHPANEL_H__ */
