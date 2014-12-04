#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>

#include "tpd_custom_ft5x06.h"

#include <mach/mt_pm_ldo.h>
#include <mach/upmu_common.h>
#include "cust_gpio_usage.h"

#include <linux/mount.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>

#ifdef MT8127
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#include <mach/eint.h>
#endif


#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
extern int ft_rw_iic_drv_init(struct i2c_client *client);
extern void  ft_rw_iic_drv_exit(void);
#endif

extern struct tpd_device *tpd;

struct i2c_client *i2c_client = NULL;
struct task_struct *thread = NULL;

static DECLARE_WAIT_QUEUE_HEAD(waiter);


static void tpd_eint_interrupt_handler(void);


extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                kal_bool auto_umask);

static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);


static int tpd_flag = 0;
static int point_num = 0;
static int p_point_num = 0;

//#define TPD_CLOSE_POWER_IN_SLEEP

#define TPD_OK 0
//register define

#define DEVICE_MODE 0x00
#define GEST_ID 0x01
#define TD_STATUS 0x02

#define TOUCH1_XH 0x03
#define TOUCH1_XL 0x04
#define TOUCH1_YH 0x05
#define TOUCH1_YL 0x06

#define TOUCH2_XH 0x09
#define TOUCH2_XL 0x0A
#define TOUCH2_YH 0x0B
#define TOUCH2_YL 0x0C

#define TOUCH3_XH 0x0F
#define TOUCH3_XL 0x10
#define TOUCH3_YH 0x11
#define TOUCH3_YL 0x12
//register define

#define TPD_RESET_ISSUE_WORKAROUND

#define TPD_MAX_RESET_COUNT 3
#if TPD_TEN_POINTS
#define FW_VERSION_REG  62
#else
#define FW_VERSION_REG  40
#endif

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static int ft5x0x_read_reg(u8 addr, u8 *pdata);
static int ft5x0x_write_reg(u8 addr, u8 para);

//#define VELOCITY_CUSTOM_FT5406
#ifdef VELOCITY_CUSTOM_FT5406
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

// for magnify velocity********************************************

#ifndef TPD_VELOCITY_CUSTOM_X
#define TPD_VELOCITY_CUSTOM_X 10
#endif
#ifndef TPD_VELOCITY_CUSTOM_Y
#define TPD_VELOCITY_CUSTOM_Y 10
#endif

#define TOUCH_IOC_MAGIC 'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)

int g_v_magnify_x =TPD_VELOCITY_CUSTOM_X;
int g_v_magnify_y =TPD_VELOCITY_CUSTOM_Y;
static int tpd_misc_open(struct inode *inode, struct file *file)
{
/*
    file->private_data = adxl345_i2c_client;

    if(file->private_data == NULL)
    {
        printk("tpd: null pointer!!\n");
        return -EINVAL;
    }
    */
    return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int tpd_misc_release(struct inode *inode, struct file *file)
{
    //file->private_data = NULL;
    return 0;
}
/*----------------------------------------------------------------------------*/
//static int adxl345_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
    //struct i2c_client *client = (struct i2c_client*)file->private_data;
    //struct adxl345_i2c_data *obj = (struct adxl345_i2c_data*)i2c_get_clientdata(client);
    //char strbuf[256];
    void __user *data;

    long err = 0;

    if(_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if(_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }

    if(err)
    {
        printk("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
        return -EFAULT;
    }

    switch(cmd)
    {
        case TPD_GET_VELOCITY_CUSTOM_X:
            data = (void __user *) arg;
            if(data == NULL)
            {
                err = -EINVAL;
                break;
            }

            if(copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))
            {
                err = -EFAULT;
                break;
            }
            break;

       case TPD_GET_VELOCITY_CUSTOM_Y:
            data = (void __user *) arg;
            if(data == NULL)
            {
                err = -EINVAL;
                break;
            }

            if(copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))
            {
                err = -EFAULT;
                break;
            }
            break;


        default:
            printk("tpd: unknown IOCTL: 0x%08x\n", cmd);
            err = -ENOIOCTLCMD;
            break;

    }

    return err;
}


static struct file_operations tpd_fops = {
//    .owner = THIS_MODULE,
    .open = tpd_misc_open,
    .release = tpd_misc_release,
    .unlocked_ioctl = tpd_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice tpd_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "touch",
    .fops = &tpd_fops,
};

//**********************************************
#endif

struct touch_info {
    int y[TPD_MAX_POINTS];
    int x[TPD_MAX_POINTS];
    int p[TPD_MAX_POINTS];
    int id[TPD_MAX_POINTS];
    int count;
};

 static const struct i2c_device_id ft5x06_tpd_id[] = {{"ft5x06",0},{}};
 //unsigned short force[] = {0,0x70,I2C_CLIENT_END,I2C_CLIENT_END};
 //static const unsigned short * const forces[] = { force, NULL };
 //static struct i2c_client_address_data addr_data = { .forces = forces, };
 static struct i2c_board_info __initdata ft5x06_i2c_tpd={ I2C_BOARD_INFO("ft5x06", (TPD_IIC_ADDRESS>>1))};


 static struct i2c_driver tpd_i2c_driver = {
  .driver = {
    .name = "ft5x06",//.name = TPD_DEVICE,
//    .owner = THIS_MODULE,
    },
  .probe = tpd_probe,
  .remove = __devexit_p(tpd_remove),
  .id_table = ft5x06_tpd_id,
  .detect = tpd_detect,
//  .address_data = &addr_data,
 };

#ifdef CHANGE_THRESHOLD_IN_CHARGE
int get_charger_status(void)
{
    return upmu_get_rgs_chrdet();
}
#endif

static  void tpd_down(int x, int y, int p) {

    #ifdef MALATA_D1019
        x=1280*x/1024;
        y=800*y/768;
    #endif

    //input_report_abs(tpd->dev, ABS_PRESSURE, p);
    if(x > TPD_RES_X)
    {
        TPD_DEBUG("warning: IC have sampled wrong value.\n");;
        return;
    }
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    //printk("D[%4d %4d %4d] ", x, y, p);
    /* track id Start 0 */
    input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p);
    input_mt_sync(tpd->dev);
    /*
    if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
    {
        tpd_button(x, y, 1);
    }
    */
    if(y > TPD_RES_Y) //virtual key debounce to avoid android ANR issue
    {
        msleep(5);
        printk("D virtual key \n");
    }
    TPD_EM_PRINT(x, y, x, y, p-1, 1);
 }

static  void tpd_up(int x, int y,int *count) {

        #ifdef MALATA_D1019
            x=1280*x/1024;
            y=800*y/768;
        #endif

     //if(*count>0) {
         //input_report_abs(tpd->dev, ABS_PRESSURE, 0);
         input_report_key(tpd->dev, BTN_TOUCH, 0);
         //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
         input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
         input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
         //printk("U[%4d %4d %4d] ", x, y, 0);
         input_mt_sync(tpd->dev);
         TPD_EM_PRINT(x, y, x, y, 0, 0);
    //     (*count)--;
    /*
    if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
    {
        tpd_button(x, y, 0);
    }
    */

 }

 static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
 {

    int i = 0;
    char data[FW_VERSION_REG] = {0};

    u16 high_byte,low_byte;
    u8 report_rate =0;
    int temp;

    #ifdef CHANGE_THRESHOLD_IN_CHARGE
    unsigned char threshold_value;
    int ret;
    #endif

    p_point_num = point_num;

    i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(data[0]));
    i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(data[8]));
    i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(data[16]));
    i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(data[24]));
    i2c_smbus_read_i2c_block_data(i2c_client, 0x20, 8, &(data[32]));
    #if TPD_TEN_POINTS
    i2c_smbus_read_i2c_block_data(i2c_client, 0x28, 8, &(data[40]));
    i2c_smbus_read_i2c_block_data(i2c_client, 0x30, 8, &(data[48]));
    i2c_smbus_read_i2c_block_data(i2c_client, 0x38, 6, &(data[56]));
    i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &(data[62]));
    #else
    i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &(data[40]));
    #endif
    i2c_smbus_read_i2c_block_data(i2c_client, 0x88, 1, &report_rate);
    TPD_DEBUG("FW version=%x]\n",data[FW_VERSION_REG]);

    //TPD_DEBUG("received raw data from touch panel as following:\n");
    //printk("[data[0]=%x,data[1]= %x ,data[2]=%x ,data[3]=%x ,data[4]=%x ,data[5]=%x]\n",data[0],data[1],data[2],data[3],data[4],data[5]);
    //TPD_DEBUG("[data[9]=%x,data[10]= %x ,data[11]=%x ,data[12]=%x]\n",data[9],data[10],data[11],data[12]);
    //TPD_DEBUG("[data[15]=%x,data[16]= %x ,data[17]=%x ,data[18]=%x]\n",data[15],data[16],data[17],data[18]);
    printk("raw data: 0x%x,0x%x,0x%x,0x%x \n",data[3],data[4],data[5],data[6]);


    //we have  to re update report rate
    // TPD_DMESG("report rate =%x\n",report_rate);
    if(report_rate < 8)
    {
        report_rate = 0x8;
        if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
        {
            TPD_DMESG("I2C read report rate error, line: %d\n", __LINE__);
        }
    }


    /* Device Mode[2:0] == 0 :Normal operating Mode*/
    if((data[0] & 0x70) != 0) return false;

    /*get the number of the touch points*/
    point_num= data[2] & 0x0f;

    //TPD_DEBUG("point_num =%d\n",point_num);

//    if(point_num == 0) return false;

    //TPD_DEBUG("Procss raw data...\n");


        for(i = 0; i < point_num; i++)
        {
            cinfo->p[i] = data[3+6*i] >> 6; //event flag 
            cinfo->id[i] = data[3+6*i+2]>>4; //touch id
            /*get the X coordinate, 2 bytes*/
            high_byte = data[3+6*i];
            high_byte <<= 8;
            high_byte &= 0x0f00;
            low_byte = data[3+6*i + 1];
            cinfo->x[i] = high_byte |low_byte;

            //cinfo->x[i] =  cinfo->x[i] * 480 >> 11; //calibra

            /*get the Y coordinate, 2 bytes*/

            high_byte = data[3+6*i+2];
            high_byte <<= 8;
            high_byte &= 0x0f00;
            low_byte = data[3+6*i+3];
            cinfo->y[i] = high_byte |low_byte;

            //cinfo->y[i]=  cinfo->y[i] * 800 >> 11;

            cinfo->count++;

#if defined(WIDTH_SCALE)
#if defined(TARGET_WIDTH) && defined(ORI_WIDTH) 
        cinfo->x[i] = cinfo->x[i]*TARGET_WIDTH/ORI_WIDTH;
#endif
#if defined(TARGET_HEIGHT) && defined(ORI_HEIGHT)
        cinfo->y[i] = cinfo->y[i]*TARGET_HEIGHT/ORI_HEIGHT;
#endif
        cinfo->x[i] = TARGET_WIDTH - cinfo->x[i];
        cinfo->y[i] = TARGET_HEIGHT - cinfo->y[i];
#endif

            #if WAP_Y
                cinfo->y[i] = TPD_RES_Y - cinfo->y[i];
            #endif
            #if WAP_X
                cinfo->x[i] = TPD_RES_X - cinfo->x[i];
            #endif
            #if CHANGE_X2Y
                temp  = cinfo->x[i];
                cinfo->x[i] = cinfo->y[i];
                cinfo->y[i] = temp;
                #ifdef MALATA_D1019
                    cinfo->x[i] = cinfo->x[i] + 224;//224=1024-800,触摸屏IC的输出为1024，当调整TPD_RES_Y = 800，则相应把差额进行调整
                #else
                    cinfo->x[i] = cinfo->x[i] + 256;//256=1024-768
                #endif   //MALATA_D1019
            #endif   //CHANGE_X2Y

        }
        //printk(" cinfo->x[0] = %d, cinfo->y[0] = %d, cinfo->p[0] = %d\n", cinfo->x[0], cinfo->y[0], cinfo->p[0]);
        //printk(" cinfo->x[1] = %d, cinfo->y[1] = %d, cinfo->p[1] = %d\n", cinfo->x[1], cinfo->y[1], cinfo->p[1]);
        //printk(" cinfo->x[2]= %d, cinfo->y[2]= %d, cinfo->p[2] = %d\n", cinfo->x[2], cinfo->y[2], cinfo->p[2]);
        //printk(" cinfo->x[3]= %d, cinfo->y[3]= %d, cinfo->p[3] = %d\n", cinfo->x[3], cinfo->y[3], cinfo->p[3]);
        //printk(" cinfo->x[4]= %d, cinfo->y[4]= %d, cinfo->p[4] = %d\n", cinfo->x[4], cinfo->y[4], cinfo->p[4]);
	printk("x and y: %d,%d,  %d,%d,  %d,%d,  %d,%d,  %d,%d \n", cinfo->x[0],cinfo->y[0],
		cinfo->x[1],cinfo->y[1],cinfo->x[2],cinfo->y[2],cinfo->x[3],cinfo->y[3],cinfo->x[4],cinfo->y[4]);

	#ifdef CHANGE_THRESHOLD_IN_CHARGE
	ret = ft5x0x_read_reg(0x80, &threshold_value);
	if ( ret < 0)
	{
		return true;
	}
	else
	{
		/* DC IN status */
		if(1 == get_charger_status())
		{
			/* change sensitivity , threshold value is 35 x 4 , peak value is 100*/
			if(threshold_value != 30)
			{
				ft5x0x_write_reg(0x80,REG80_THRESHOLD_IN_CHARGE);
				ft5x0x_write_reg(0x81,REG81_PEAK_IN_CHARGE);
			}
		}
		else  /*Not DC in */
		{
			/* change sensitivity , threshold value is 20 x 4 , peak value is 60 */
			if(threshold_value != 20)
			{
				ft5x0x_write_reg(0x80,REG80_THRESHOLD_UN_CHARGE);
				ft5x0x_write_reg(0x81,REG81_PEAK_UN_CHARGE);
			}
		}
	}
	#endif

     return true;

 };

 static int touch_event_handler(void *unused)
 {

    struct touch_info cinfo, pinfo;
    int i=0;

    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
    sched_setscheduler(current, SCHED_RR, &param);

    do
    {
        #ifdef MT8127
        mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
        #else
        mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
        #endif

        set_current_state(TASK_INTERRUPTIBLE); 
        wait_event_interruptible(waiter,tpd_flag!=0);

        tpd_flag = 0;

        set_current_state(TASK_RUNNING);

        if (tpd_touchinfo(&cinfo, &pinfo))
        {
            //printk("point_num = %d\n",point_num);
            TPD_DEBUG_SET_TIME;
            if(point_num >0) 
            {
                for(i =0; i<point_num && i<TPD_MAX_POINTS; i++)//only support 3 point
                {
                    tpd_down(cinfo.x[i], cinfo.y[i], cinfo.id[i]);
                }
                input_sync(tpd->dev);
            }
            else  
            {
                tpd_up(cinfo.x[0], cinfo.y[0], 0);
                //TPD_DEBUG("release --->\n"); 
                //input_mt_sync(tpd->dev);
                input_sync(tpd->dev);
            }
        }
/*
        if(tpd_mode==12)
        {
            //power down for desence debug
            //power off, need confirm with SA
            hwPowerDown(MT65XX_POWER_LDO_VGP2,  "TP");
            hwPowerDown(MT65XX_POWER_LDO_VGP,  "TP");
            msleep(20);
        }
*/
 }while(!kthread_should_stop());

     return 0;
 }

 static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info)
 {
    strcpy(info->type, TPD_DEVICE);
    return 0;
 }
 
 static void tpd_eint_interrupt_handler(void)
 {
    //TPD_DEBUG("TPD interrupt has been triggered\n");
    TPD_DEBUG_PRINT_INT;
    tpd_flag = 1;
    wake_up_interruptible(&waiter);

 }


#ifdef __UPDATE_FOCALTECH_TP_FRAMEWARE__

typedef enum
{
    ERR_OK,
    ERR_MODE,
    ERR_READID,
    ERR_ERASE,
    ERR_STATUS,
    ERR_ECC,
    ERR_DL_ERASE_FAIL,
    ERR_DL_PROGRAM_FAIL,
    ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       0x70

static u8 *gpDMABuf_va = NULL;
static u32 gpDMABuf_pa = NULL;

/***********************************************************************************************
Name   :        ft5x0x_read_reg

Input  :        addr
                pdata

Output

function:       read register of ft5x0x

***********************************************************************************************/
static int ft5x0x_read_reg(u8 addr, u8 *pdata)
{
    int ret;
    u8 buf[2] = {0};

    buf[0] = addr;
    struct i2c_msg msgs[] = {
        {
            .addr    = i2c_client->addr,
            .flags    = 0,
            .len    = 1,
            .buf    = buf,
        },
        {
            .addr    = i2c_client->addr,
            .flags    = I2C_M_RD,
            .len    = 1,
            .buf    = buf,
        },
    };

    //msleep(1);
    ret = i2c_transfer(i2c_client->adapter, msgs, 2);
    if (ret < 0)
        pr_err("msg %s i2c read error: %d\n", __func__, ret);

    *pdata = buf[0];
    return ret;

}

void delay_qt_ms(unsigned long  w_ms)
{
    udelay(w_ms*1000);
}


/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;

    ret=i2c_master_recv(i2c_client, pbt_buf, dw_lenth);

    if(ret<=0)
    {
        printk("[TSP]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

/*
[function]:
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;

    ret=i2c_master_send(i2c_client, pbt_buf, dw_lenth);
    if(ret<=0)
    {
        printk("[TSP]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

/*
[function]:
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;
    btPara2[in]    :parameter 2;
    btPara3[in]    :parameter 3;
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]:
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{

    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]:
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}


/*
[function]:
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/


#define    FTS_PACKET_LENGTH        128
/*
static unsigned char CTPM_FW[]=
{
///#include "app.i"
#include "ft_app_ba.i" //tianzhiyu
};

static unsigned char CTPM_FW1[]=
{
///#include "app.i"
#include "ft_app_92.i" //beitai
};
*/
/***********************************************************************************************
Name    :

Input    :

Output    :

function    :

***********************************************************************************************/
static int ft5x0x_i2c_txdata(char *txdata, int length)
{
    int ret;

    struct i2c_msg msg[] = {
        {
            .addr    = i2c_client->addr,
            .flags    = 0,
            .len    = length,
            .buf    = txdata,
        },
    };

    //msleep(1);
    ret = i2c_transfer(i2c_client->adapter, msg, 1);
    if (ret < 0)
        pr_err("%s i2c write error: %d\n", __func__, ret);

    return ret;
}

/*write data by i2c*/
int ft5x0x_dma_i2c_Write(struct i2c_client *client, u32 writebuf, int writelen)
{
    int ret;

    struct i2c_msg msg[] = {
        {
         .addr = client->addr,
         .ext_flag = (client->ext_flag | I2C_ENEXT_FLAG | I2C_DMA_FLAG),
         .flags = 0,
         .len = writelen,
         .buf = writebuf,
         },
    };

    ret = i2c_transfer(client->adapter, msg, 1);
    if (ret < 0)
        dev_err(&client->dev, "%s i2c write error.\n", __func__);

    return ret;
}

/***********************************************************************************************
Name    :     ft5x0x_write_reg

Input    :    addr -- address
                     para -- parameter

Output    :

function    :    write register of ft5x0x

***********************************************************************************************/
static int ft5x0x_write_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = ft5x0x_i2c_txdata(buf, 2);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }

    return 0;
}

E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;

    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_ret;

    gpDMABuf_va = (u8 *)dma_alloc_coherent(NULL, 256, &gpDMABuf_pa, GFP_KERNEL);
    if(!gpDMABuf_va){
            printk("[Error] Allocate DMA I2C Buffer failed!\n");
    }

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    //ft5x0x_write_reg(0xfc,0xaa);
    //mdelay(20);
     /*write 0x55 to register 0xfc*/
    //ft5x0x_write_reg(0xfc,0x55);
    printk("[TSP] Step 1: Reset CTPM test\n");

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(UPGRADE_AA_DELAY);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

    mdelay(UPGRADE_55_DELAY);


    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        //i_ret  = i2c_smbus_write_block_data(i2c_client,0x55,1,&auc_i2c_write_buf[1]);
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        mdelay(5);
    }while(i_ret <= 0 && i < 5 );
    printk("-----test zhjg i=%u,  ret = %d\n",i,i_ret);


    /*********Step 3:check READ-ID***********************/
    mdelay(100);
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    /* if IC is ft5216 , id is 0x79 0x07 ;*/
    /* if ic is ft5606 ,ic is 0x79 0x06 ;*/
    /* if IC if ft5406 , id is 0x79 0x03 */
    if (reg_val[0] == 0x79 && reg_val[1] == FT5X06_IC_ID2)
    {
        printk("[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        printk("[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x  222222\n",reg_val[0],reg_val[1]);

        return ERR_READID;
        //i_is_new_protocol = 1;
    }

     /*********Step 4:erase app*******************************/
    cmd_write(0x61,0x00,0x00,0x00,1);

    mdelay(1500);
    printk("[TSP] Step 4: erase. \n");

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
    printk("[TSP] Step 5: start upgrade. \n");
    dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    //dw_lenth = 28666
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i];
            bt_ecc ^= packet_buf[6+i];
        }
        for (i = 0; i < FTS_PACKET_LENGTH+6; i++) {
            *(gpDMABuf_va+i)=packet_buf[i];
            //DBG("*(gpDMABuf_va+%d)=0x%02x,packet_buf[%d]=0x%02x \n",i, *(gpDMABuf_va+i),i, packet_buf[i]);
        }
        ft5x0x_dma_i2c_Write(i2c_client,gpDMABuf_pa , FTS_PACKET_LENGTH + 6);
        //printk("----test zhjg before byte_write!\n");
        //byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);

        delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              printk("[TSP] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i];
            bt_ecc ^= packet_buf[6+i];
        }
        for (i = 0; i < temp+6; i++) {
            *(gpDMABuf_va+i)=packet_buf[i];
        }
        ft5x0x_dma_i2c_Write(i2c_client,gpDMABuf_pa , temp + 6);
        //byte_write(&packet_buf[0],temp+6);
        delay_qt_ms(20);
    }

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i];
        bt_ecc ^= packet_buf[6];
        for (j = 0; j < 7; j++) {
            *(gpDMABuf_va+j)=packet_buf[j];
        }

        ft5x0x_dma_i2c_Write(i2c_client,gpDMABuf_pa , 7);
        //byte_write(&packet_buf[0],7);
        delay_qt_ms(20);
    }

    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        if(gpDMABuf_va){
           dma_free_coherent(NULL, 256, gpDMABuf_va, gpDMABuf_pa);
           gpDMABuf_va = NULL;
           gpDMABuf_pa = NULL;
        }
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
        msleep(50);
        mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
        msleep(40);
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    //cmd_write(0x07,0x00,0x00,0x00,1);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(50);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    msleep(300);
        if(gpDMABuf_va){
            dma_free_coherent(NULL, 256, gpDMABuf_va, gpDMABuf_pa);
            gpDMABuf_va = NULL;
            gpDMABuf_pa = NULL;
        }

    return ERR_OK;
}

static int fts_ctpm_auto_clb(void)
{
    unsigned char uc_temp;
    unsigned char i ;

    printk("[FTS] start auto CLB.\n");
    msleep(200);
    ft5x0x_write_reg(0, 0x40);
    delay_qt_ms(100);                       //make sure already enter factory mode
    ft5x0x_write_reg(2, 0x4);               //write command to start calibration
    delay_qt_ms(300);
    for(i=0;i<100;i++)
    {
        ft5x0x_read_reg(0,&uc_temp);
        if ( ((uc_temp&0x70)>>4) == 0x0)    //return to normal mode, calibration finish
        {
            break;
        }
        delay_qt_ms(200);
        printk("[FTS] waiting calibration %d\n",i);
    }

    printk("[FTS] calibration OK.\n");

    msleep(300);
    ft5x0x_write_reg(0, 0x40);              //goto factory mode
    delay_qt_ms(100);                       //make sure already enter factory mode
    ft5x0x_write_reg(2, 0x5);               //store CLB result
    delay_qt_ms(300);
    ft5x0x_write_reg(0, 0x0);               //return to normal mode
    msleep(300);
    printk("[FTS] store CLB result OK.\n");
    return 0;
}
int fts_ctpm_fw_upgrade_with_i_file(void)
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret;

    //=========FW upgrade========================*/
   pbt_buf = CTPM_FW;
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW));
   if (i_ret != 0)
   {
       //error handling ...
       //TBD
   }
   else
   {
       printk("[FTS] upgrade successfully.\n");
       fts_ctpm_auto_clb();  //start auto CLB
   }

   return i_ret;
}

#ifdef FT5606
int fts_ctpm_fw_upgrade_with_i_file1(void)
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret;

    //=========FW upgrade========================*/
   pbt_buf = CTPM_FW1;
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW1));
   if (i_ret != 0)
   {
       //error handling ...
       //TBD
   }
   else
   {
       printk("[FTS] upgrade successfully.\n");
       fts_ctpm_auto_clb();  //start auto CLB
   }

   return i_ret;
}
#endif

unsigned char fts_ctpm_get_upg_ver(void)
{
    unsigned int ui_sz;
    ui_sz = sizeof(CTPM_FW);
    if (ui_sz > 2)
    {
        return CTPM_FW[ui_sz - 2];
    }
    else
    {
        //TBD, error handling?
        return 0xff; //default value
    }
}

#endif

 static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {
    int retval = TPD_OK;
    char data;
    u8 report_rate=0;
    int err=0;
    int reset_count = 0;
    u8 buf[4];
    char new_version;
    unsigned char ver;

reset_proc:
    i2c_client = client;

    #ifdef TPD_CLOSE_POWER_IN_SLEEP     
    hwPowerDown(TPD_POWER_SOURCE,"TP");
    hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");
    msleep(100);
    #else
    #ifdef MT6573
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
    msleep(100);
    #endif
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    mdelay(5);
    #endif

#ifdef MT6575
    //power on, need confirm with SA
    hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
    hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");
#endif

#ifdef MT6577
#ifdef TPD_POWER_SOURCE_CUSTOM
    //power on, need confirm with SA
    hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#else
    hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
    hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_1800, "TP");
#endif
#endif

#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerDown(TPD_POWER_SOURCE_CUSTOM,  "TP");
#else
    hwPowerDown(TPD_POWER_SOURCE,    "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerDown(TPD_POWER_SOURCE_1800,    "TP");
#endif  

#ifdef TPD_POWER_SOURCE_CUSTOM
    hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#else
    hwPowerOn(TPD_POWER_SOURCE, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
    hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif  

    mdelay(100);

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
 
#if 1//def mt8127
    mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_TYPE, tpd_eint_interrupt_handler, 1); 
    mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
#else
    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
#endif

    msleep(100);

    if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
    {
        tpd_load_status = 0;
        TPD_DMESG("I2C transfer error, line: %d\n", __LINE__);
#ifdef TPD_RESET_ISSUE_WORKAROUND
    if ( reset_count < TPD_MAX_RESET_COUNT )
    {
        reset_count++;
        goto reset_proc;
    }
#endif
        return -1; 
    }

    //set report rate 80Hz
    report_rate = 0x8; 
    if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
    {
        if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
        {
            TPD_DMESG("I2C read report rate error, line: %d\n", __LINE__);
        }

    }

    tpd_load_status = 1;

    #ifdef VELOCITY_CUSTOM_FT5406
    if((err = misc_register(&tpd_misc_device)))
    {
        printk("mtk_tpd: tpd_misc_device register failed\n");
    }
    #endif
#ifdef __UPDATE_FOCALTECH_TP_FRAMEWARE__
    printk("-----test zhjg for touchpannel\n");
    if (ft5x0x_read_reg(0xa6,&ver) < 0){
        ver = 0xff;
    }//can be deleted   read fw virson
    //ver = 0xb7;
    printk("################# TP FW version = %x \n", ver);

    //if(&buf[0] < fts_ctpm_get_upg_ver()){//error
#ifdef FT5606
    if(( ver < CURRENT_FW_VERSION) && ( ver > 0xa0)){
        mdelay(50);  //must add or "Step 3:check READ-ID" will return error 
        if(fts_ctpm_fw_upgrade_with_i_file() == 0)//tianzhiyu
        {
            printk("----test zhjg update TP  frameware ok!\n");
            mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
            msleep(1);
            mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
        }
    }
    if(( ver < CURRENT_FW_VERSION_1) && ( ver >= 0x90)){
        mdelay(50);  //must add or "Step 3:check READ-ID" will return error
        if(fts_ctpm_fw_upgrade_with_i_file1() == 0)//beitai
        {
            printk("----test zhjg update TP  frameware ok!\n");
            mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
            msleep(1);
            mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
        }
    }
#else
    if(( ver < CURRENT_FW_VERSION) || ( ver == 0xa6)){
        mdelay(50);  //must add or "Step 3:check READ-ID" will return error 
        if(fts_ctpm_fw_upgrade_with_i_file() == 0)//tianzhiyu
        {
            printk("----test zhjg update TP  frameware ok!\n");
            mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
            msleep(1);
            mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
        }
    }
#endif
#endif

    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
     if (IS_ERR(thread))
    {
        retval = PTR_ERR(thread);
        TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
    }
    #ifdef FTS_CTL_IIC
        if (ft_rw_iic_drv_init(client) < 0)
            dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
                    __func__);
    #endif

    TPD_DMESG("ft5x06 Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
    return 0;

 }

 static int __devexit tpd_remove(struct i2c_client *client)
 {
    #ifdef FTS_CTL_IIC
    ft_rw_iic_drv_exit();
    #endif 
    TPD_DEBUG("TPD removed\n");
    return 0;
 }

 static int tpd_local_init(void)
 {

    TPD_DMESG("Focaltech ft5x06 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);

    if(i2c_add_driver(&tpd_i2c_driver)!=0)
    {
        TPD_DMESG("ft5x06 unable to add i2c driver.\n");
        return -1;
    }
    if(tpd_load_status == 0) 
    {
        TPD_DMESG("ft5x06 add error touch panel driver.\n");
        i2c_del_driver(&tpd_i2c_driver);
        return -1;
    }

#ifdef TPD_HAVE_BUTTON
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);
#endif
    TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);
    tpd_type_cap = 1;

    TPD_RES_X = RES_X; //malata test
    TPD_RES_Y = RES_Y; //malata test

    return 0; 
 }

 static void tpd_resume( struct early_suspend *h )
 {
    //int retval = TPD_OK;
    char data;

    TPD_DMESG("TPD wake up\n");
#ifdef TPD_CLOSE_POWER_IN_SLEEP
    hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");
#else
#ifdef MT6573
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif
    msleep(100);

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(1);
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#endif

    #ifdef MT8127
    mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    #else
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    #endif

    msleep(20);
    if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
    {
        TPD_DMESG("resume I2C transfer error, line: %d\n", __LINE__);
    }
    tpd_up(0,0,0);
    input_sync(tpd->dev);
    TPD_DMESG("TPD wake up done\n");
    //return retval;
 }

 static void tpd_suspend( struct early_suspend *h )
 {
    // int retval = TPD_OK;
     static char data = 0x3;
 
     TPD_DMESG("TPD enter sleep\n");
#ifdef MT8127
     mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
#else
     mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
#endif
#ifdef TPD_CLOSE_POWER_IN_SLEEP    
    hwPowerDown(TPD_POWER_SOURCE,"TP");
#else
    i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
#ifdef MT6573
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif

#endif
    TPD_DMESG("TPD enter sleep done\n");
     //return retval;
 } 


 static struct tpd_driver_t tpd_device_driver = {
         .tpd_device_name = "FT5x06",
         .tpd_local_init = tpd_local_init,
         .suspend = tpd_suspend,
         .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
         .tpd_have_button = 1,
#else
         .tpd_have_button = 0,
#endif
 };
 /* called when loaded into kernel */
 static int __init tpd_driver_init(void) {
     printk("MediaTek ft5x06 touch panel driver init\n");
       i2c_register_board_info(1, &ft5x06_i2c_tpd, 1);
         if(tpd_driver_add(&tpd_device_driver) < 0)
             TPD_DMESG("add ft5x06 driver failed\n");
     return 0;
 }
 
 /* should never be called */
 static void __exit tpd_driver_exit(void) {
     TPD_DMESG("MediaTek ft5x06 touch panel driver exit\n");
     //input_unregister_device(tpd->dev);
     tpd_driver_remove(&tpd_device_driver);
 }
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);


