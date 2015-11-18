/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "OV5694AF.h"
#include "../camera/kd_camera_hw.h"

#define LENS_I2C_BUSNUM 0		/* The same with 8127 camera i2c bus # */
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("OV5694AF", 0x20)};  // COMPALE CAMERA


#define OV5694AF_DRVNAME "OV5694AF"
#define OV5694AF_VCM_WRITE_ID           0x18

#define E1003_OV5693_AF

#define OV5694AF_DEBUG
#ifdef OV5694AF_DEBUG
#define OV5694AFDB printk
#else
#define OV5694AFDB(x,...)
#endif

static spinlock_t g_OV5694AF_SpinLock;

static struct i2c_client * g_pstOV5694AF_I2Cclient = NULL;

static dev_t g_OV5694AF_devno;
static struct cdev * g_pOV5694AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4OV5694AF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static unsigned long g_u4OV5694AF_INF = 0;
static unsigned long g_u4OV5694AF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;

static int g_sr = 3;

#if 0
extern s32 mt_set_gpio_mode(u32 u4Pin, u32 u4Mode);
extern s32 mt_set_gpio_out(u32 u4Pin, u32 u4PinOut);
extern s32 mt_set_gpio_dir(u32 u4Pin, u32 u4Dir);
#endif

static int s4OV5694AF_ReadReg(unsigned short * a_pu2Result)
{
    int  i4RetValue = 0;
    char pBuff[2];

    i4RetValue = i2c_master_recv(g_pstOV5694AF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0) 
    {
        OV5694AFDB("[OV5694AF] I2C read failed!! \n");
        return -1;
    }

    *a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

    return 0;
}

static int s4OV5694AF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[2] = {(char)(a_u2Data >> 4) , (char)(((a_u2Data & 0xF) << 4)+g_sr)};

    //OV5694AFDB("[OV5694AF] g_sr %d, write %d \n", g_sr, a_u2Data);
    g_pstOV5694AF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstOV5694AF_I2Cclient, puSendCmd, 2);
	
    if (i4RetValue < 0) 
    {
        OV5694AFDB("[OV5694AF] I2C send failed!! \n");
        return -1;
    }

    return 0;
}

inline static int getOV5694AFInfo(__user stOV5694AF_MotorInfo * pstMotorInfo)
{
    stOV5694AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4OV5694AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4OV5694AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR      = TRUE;

	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = 1;}
	else						{stMotorInfo.bIsMotorMoving = 0;}

	if (g_s4OV5694AF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = 1;}
	else						{stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stOV5694AF_MotorInfo)))
    {
        OV5694AFDB("[OV5694AF] copy to user failed when getting motor information \n");
    }

    return 0;
}

#ifdef LensdrvCM3
inline static int getOV5694AFMETA(__user stOV5694AF_MotorMETAInfo * pstMotorMETAInfo)
{
    stOV5694AF_MotorMETAInfo stMotorMETAInfo;
    stMotorMETAInfo.Aperture=2.8;      //fn
	stMotorMETAInfo.Facing=1;   
	stMotorMETAInfo.FilterDensity=1;   //X
	stMotorMETAInfo.FocalDistance=1.0;  //diopters
	stMotorMETAInfo.FocalLength=34.0;  //mm
	stMotorMETAInfo.FocusRange=1.0;    //diopters
	stMotorMETAInfo.InfoAvalibleApertures=2.8;
	stMotorMETAInfo.InfoAvalibleFilterDensity=1;
	stMotorMETAInfo.InfoAvalibleFocalLength=34.0;
	stMotorMETAInfo.InfoAvalibleHypeDistance=1.0;
	stMotorMETAInfo.InfoAvalibleMinFocusDistance=1.0;
	stMotorMETAInfo.InfoAvalibleOptStabilization=0;
	stMotorMETAInfo.OpticalAxisAng[0]=0.0;
	stMotorMETAInfo.OpticalAxisAng[1]=0.0;
	stMotorMETAInfo.Position[0]=0.0;
	stMotorMETAInfo.Position[1]=0.0;
	stMotorMETAInfo.Position[2]=0.0;
	stMotorMETAInfo.State=0;
	stMotorMETAInfo.u4OIS_Mode=0;
	
	if(copy_to_user(pstMotorMETAInfo , &stMotorMETAInfo , sizeof(stOV5694AF_MotorMETAInfo)))
	{
		OV5694AFDB("[OV5694AF] copy to user failed when getting motor information \n");
	}

    return 0;
}
#endif

inline static int moveOV5694AF(unsigned long a_u4Position)
{
    int ret = 0;
    
    if((a_u4Position > g_u4OV5694AF_MACRO) || (a_u4Position < g_u4OV5694AF_INF))
    {
        OV5694AFDB("[OV5694AF] out of range \n");
        return -EINVAL;
    }

    if (g_s4OV5694AF_Opened == 1)
    {
        unsigned short InitPos;
        ret = s4OV5694AF_ReadReg(&InitPos);
	    
        if(ret == 0)
        {
            OV5694AFDB("[OV5694AF] Init Pos %6d \n", InitPos);
			
			spin_lock(&g_OV5694AF_SpinLock);
            g_u4CurrPosition = (unsigned long)InitPos;
			spin_unlock(&g_OV5694AF_SpinLock);
			
        }
        else
        {	
			spin_lock(&g_OV5694AF_SpinLock);
            g_u4CurrPosition = 0;
			spin_unlock(&g_OV5694AF_SpinLock);
        }

		spin_lock(&g_OV5694AF_SpinLock);
        g_s4OV5694AF_Opened = 2;
        spin_unlock(&g_OV5694AF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(&g_OV5694AF_SpinLock);	
        g_i4Dir = 1;
        spin_unlock(&g_OV5694AF_SpinLock);	
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(&g_OV5694AF_SpinLock);	
        g_i4Dir = -1;
        spin_unlock(&g_OV5694AF_SpinLock);			
    }
    else										{return 0;}

    spin_lock(&g_OV5694AF_SpinLock);    
    g_u4TargetPosition = a_u4Position;
    spin_unlock(&g_OV5694AF_SpinLock);	

    //OV5694AFDB("[OV5694AF] move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

            spin_lock(&g_OV5694AF_SpinLock);
            g_sr = 3;
            g_i4MotorStatus = 0;
            spin_unlock(&g_OV5694AF_SpinLock);	
		
            if(s4OV5694AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
            {
                spin_lock(&g_OV5694AF_SpinLock);		
                g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
                spin_unlock(&g_OV5694AF_SpinLock);				
            }
            else
            {
                OV5694AFDB("[OV5694AF] set I2C failed when moving the motor \n");			
                spin_lock(&g_OV5694AF_SpinLock);
                g_i4MotorStatus = -1;
                spin_unlock(&g_OV5694AF_SpinLock);				
            }

    return 0;
}

inline static int setOV5694AFInf(unsigned long a_u4Position)
{
    spin_lock(&g_OV5694AF_SpinLock);
    g_u4OV5694AF_INF = a_u4Position;
    spin_unlock(&g_OV5694AF_SpinLock);	
    return 0;
}

inline static int setOV5694AFMacro(unsigned long a_u4Position)
{
    spin_lock(&g_OV5694AF_SpinLock);
    g_u4OV5694AF_MACRO = a_u4Position;
    spin_unlock(&g_OV5694AF_SpinLock);	
    return 0;	
}

////////////////////////////////////////////////////////////////
static long OV5694AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case OV5694AFIOC_G_MOTORINFO :
            i4RetValue = getOV5694AFInfo((__user stOV5694AF_MotorInfo *)(a_u4Param));
        break;
		#ifdef LensdrvCM3
        case OV5694AFIOC_G_MOTORMETAINFO :
            i4RetValue = getOV5694AFMETA((__user stOV5694AF_MotorMETAInfo *)(a_u4Param));
        break;
		#endif
        case OV5694AFIOC_T_MOVETO :
            i4RetValue = moveOV5694AF(a_u4Param);
        break;
 
        case OV5694AFIOC_T_SETINFPOS :
            i4RetValue = setOV5694AFInf(a_u4Param);
        break;

        case OV5694AFIOC_T_SETMACROPOS :
            i4RetValue = setOV5694AFMacro(a_u4Param);
        break;
		
        default :
      	    OV5694AFDB("[OV5694AF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int OV5694AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    OV5694AFDB("[OV5694AF] OV5694AF_Open - Start\n");

#ifdef E1003_OV5693_AF
    char puSendCmd[2];
    long i4RetValue = 0;

        OV5694AFDB("[OV5694AF] s4OV5694AF_WriteReg truly module init!\n");
        puSendCmd[0] = (char)(0xEC);
        puSendCmd[1] = (char)(0xA3);
i4RetValue = i2c_master_send(g_pstOV5694AF_I2Cclient, puSendCmd, 2);
if (i4RetValue < 0) 
{
        OV5694AFDB("[OV5694AF] I2C send failed!! \n");
        return -1;
}
 
        puSendCmd[0] = (char)(0xA1);
        puSendCmd[1] = (char)(0x0E);  
i4RetValue = i2c_master_send(g_pstOV5694AF_I2Cclient, puSendCmd, 2);
if (i4RetValue < 0) 
{
        OV5694AFDB("[OV5694AF] I2C send failed!! \n");
        return -1;
}
 
        puSendCmd[0] = (char)(0xF2);
        puSendCmd[1] = (char)(0x90);
i4RetValue = i2c_master_send(g_pstOV5694AF_I2Cclient, puSendCmd, 2);
if (i4RetValue < 0) 
{
        OV5694AFDB("[OV5694AF] I2C send failed!! \n");
        return -1;
}
 
        puSendCmd[0] = (char)(0xDC);
        puSendCmd[1] = (char)(0x51);
i4RetValue = i2c_master_send(g_pstOV5694AF_I2Cclient, puSendCmd, 2);
if (i4RetValue < 0) 
{
        OV5694AFDB("[OV5694AF] I2C send failed!! \n");
        return -1;
}

#endif

    if(g_s4OV5694AF_Opened)
    {
        OV5694AFDB("[OV5694AF] the device is opened \n");
        return -EBUSY;
    }
	
    spin_lock(&g_OV5694AF_SpinLock);
    g_s4OV5694AF_Opened = 1;
    spin_unlock(&g_OV5694AF_SpinLock);

    OV5694AFDB("[OV5694AF] OV5694AF_Open - End\n");

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int OV5694AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    OV5694AFDB("[OV5694AF] OV5694AF_Release - Start\n");

    if (g_s4OV5694AF_Opened)
    {
        OV5694AFDB("[OV5694AF] feee \n");
        g_sr = 5;
	    s4OV5694AF_WriteReg(200);
        msleep(10);
	    s4OV5694AF_WriteReg(100);
        msleep(10);
            	            	    	    
        spin_lock(&g_OV5694AF_SpinLock);
        g_s4OV5694AF_Opened = 0;
        spin_unlock(&g_OV5694AF_SpinLock);

    }

    OV5694AFDB("[OV5694AF] OV5694AF_Release - End\n");

    return 0;
}

static const struct file_operations g_stOV5694AF_fops = 
{
    .owner = THIS_MODULE,
    .open = OV5694AF_Open,
    .release = OV5694AF_Release,
    .unlocked_ioctl = OV5694AF_Ioctl
};

inline static int Register_OV5694AF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    OV5694AFDB("[OV5694AF] Register_OV5694AF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_OV5694AF_devno, 0, 1,OV5694AF_DRVNAME) )
    {
        OV5694AFDB("[OV5694AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pOV5694AF_CharDrv = cdev_alloc();

    if(NULL == g_pOV5694AF_CharDrv)
    {
        unregister_chrdev_region(g_OV5694AF_devno, 1);

        OV5694AFDB("[OV5694AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }
    //Attatch file operation.
    cdev_init(g_pOV5694AF_CharDrv, &g_stOV5694AF_fops);

    g_pOV5694AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pOV5694AF_CharDrv, g_OV5694AF_devno, 1))
    {
        OV5694AFDB("[OV5694AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_OV5694AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv1");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        OV5694AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_OV5694AF_devno, NULL, OV5694AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    OV5694AFDB("[OV5694AF] Register_OV5694AF_CharDrv - End\n");    
    return 0;
}

inline static void Unregister_OV5694AF_CharDrv(void)
{
    OV5694AFDB("[OV5694AF] Unregister_OV5694AF_CharDrv - Start\n");

    //Release char driver
    cdev_del(g_pOV5694AF_CharDrv);

    unregister_chrdev_region(g_OV5694AF_devno, 1);
    
    device_destroy(actuator_class, g_OV5694AF_devno);

    class_destroy(actuator_class);

    OV5694AFDB("[OV5694AF] Unregister_OV5694AF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////

static int OV5694AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int OV5694AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id OV5694AF_i2c_id[] = {{OV5694AF_DRVNAME,0},{}};   
struct i2c_driver OV5694AF_i2c_driver = {                       
    .probe = OV5694AF_i2c_probe,                                   
    .remove = OV5694AF_i2c_remove,                           
    .driver.name = OV5694AF_DRVNAME,                 
    .id_table = OV5694AF_i2c_id,                             
};  

#if 0 
static int OV5694AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, OV5694AF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int OV5694AF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int OV5694AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    OV5694AFDB("[OV5694AF] OV5694AF_i2c_probe\n");

    /* Kirby: add new-style driver { */
    g_pstOV5694AF_I2Cclient = client;
    
    g_pstOV5694AF_I2Cclient->addr = 0x18 >> 1;
    
    //Register char driver
    i4RetValue = Register_OV5694AF_CharDrv();

    if(i4RetValue){

        OV5694AFDB("[OV5694AF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_OV5694AF_SpinLock);

    OV5694AFDB("[OV5694AF] Attached!! \n");

    return 0;
}

static int OV5694AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&OV5694AF_i2c_driver);
}

static int OV5694AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&OV5694AF_i2c_driver);
    return 0;
}

static int OV5694AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int OV5694AF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stOV5694AF_Driver = {
    .probe		= OV5694AF_probe,
    .remove	= OV5694AF_remove,
    .suspend	= OV5694AF_suspend,
    .resume	= OV5694AF_resume,
    .driver		= {
        .name	= "lens_actuator1",
        .owner	= THIS_MODULE,
    }
};

static int __init OV5694AF_i2C_init(void)
{
    i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
	
    if(platform_driver_register(&g_stOV5694AF_Driver)){
        OV5694AFDB("failed to register OV5694AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit OV5694AF_i2C_exit(void)
{
	platform_driver_unregister(&g_stOV5694AF_Driver);
}

module_init(OV5694AF_i2C_init);
module_exit(OV5694AF_i2C_exit);

MODULE_DESCRIPTION("OV5694AF lens module driver");
MODULE_AUTHOR("KY Chen <ky.chen@Mediatek.com>");
MODULE_LICENSE("GPL");


