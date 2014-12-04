
#include <linux/device.h>

#include <linux/kernel.h>

#include <linux/i2c.h>

#include "ntrig_i2c_low.h"
#include "cust_gpio_usage.h"
#include <cust_eint.h>
#include <mach/mt_pm_ldo.h>
#include <linux/delay.h>

#define DRIVER_NAME	"ntrig_i2c"

#define NTRIG_I2C_ID

#ifdef CONFIG_PM
static ntrig_i2c_suspend_callback suspend_callback;
static ntrig_i2c_resume_callback resume_callback;

void ntrig_i2c_register_pwr_mgmt_callbacks(
	ntrig_i2c_suspend_callback s, ntrig_i2c_resume_callback r)
{
	suspend_callback = s;
	resume_callback = r;
}
EXPORT_SYMBOL_GPL(ntrig_i2c_register_pwr_mgmt_callbacks);

static struct i2c_device_data s_i2c_device_data;

struct i2c_device_data *get_ntrig_i2c_device_data(void)
{
	return &s_i2c_device_data;
}
EXPORT_SYMBOL_GPL(get_ntrig_i2c_device_data);

static int ntrig_power_on(struct i2c_client *client)
{
    //power on, need confirm with SA
    hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_3300, "TP");
    msleep(10);
    hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_1800, "TP");
    return 0;
}

extern int ntrig_i2c_mod_init(void);
static int ntrig_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	printk(KERN_DEBUG "in %s\n", __func__);
	ntrig_power_on(client);
	s_i2c_device_data.m_i2c_client = client;
	s_i2c_device_data.m_platform_data =
		*(struct ntrig_i2c_platform_data *)client->dev.platform_data;
	ntrig_i2c_mod_init();

	return 0;
}

static int ntrig_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	printk(KERN_DEBUG "in %s\n", __func__);
	if (suspend_callback)
		return suspend_callback(client, mesg);
	return 0;
}

static int ntrig_i2c_resume(struct i2c_client *client)
{
	printk(KERN_DEBUG "in %s\n", __func__);
	if (resume_callback)
		return resume_callback(client);
	return 0;
}

#endif	/* CONFIG_PM */

static int ntrig_i2c_remove(struct i2c_client *client)
{
	printk(KERN_DEBUG "in %s\n", __func__);
	return 0;
}

static struct i2c_device_id ntrig_i2c_idtable[] = {
		{ DRIVER_NAME, NTRIG_I2C_ID },
		{ }
};

MODULE_DEVICE_TABLE(i2c, ntrig_i2c_idtable);

static struct i2c_driver ntrig_i2c_driver = {
		.driver = {
				.name	= DRIVER_NAME,
				.owner	= THIS_MODULE,
		},
		.class = I2C_CLASS_HWMON,
		.id_table = ntrig_i2c_idtable,
		.suspend = ntrig_i2c_suspend,
		.resume = ntrig_i2c_resume,
		.probe		= ntrig_i2c_probe,
		.remove		= __devexit_p(ntrig_i2c_remove),
};

#define NTRIG_I2C_OUTPUT_ENABLE		-1//TEGRA_GPIO_PQ7
#define NTRIG_I2C_INT			CUST_EINT_TOUCH_PANEL_NUM
#define NTRIG_I2C_PWR			-1
#define NTRIG_I2C_SLAVE_ADDRESS		0x70

static struct ntrig_i2c_platform_data tegra_ntrig_i2c_pdata __initdata = {
		.oe_gpio = NTRIG_I2C_OUTPUT_ENABLE,
		.oe_inverted = 1,
		.pwr_gpio = NTRIG_I2C_PWR,
		.irq_flags = 0 // use default flags in the driver 
};
static struct i2c_board_info tegra_ntrig_i2c_devices[] __initdata = {
	{
		.type = "ntrig_i2c",
		.flags = 0,
		.addr = NTRIG_I2C_SLAVE_ADDRESS,
		.platform_data = &tegra_ntrig_i2c_pdata,
		.archdata = 0,
#ifdef CONFIG_OF
		.of_node = 0,
#endif	
		.irq = 0, // will be calculated later
		
	},		
};

static void __init register_ntrig_i2c_devices(void)
{
	printk(KERN_WARNING "in %s!!!\n", __FUNCTION__);
	// map irq from our allocated gpio line 
	tegra_ntrig_i2c_devices[0].irq = NTRIG_I2C_INT;
	i2c_register_board_info(1,tegra_ntrig_i2c_devices,ARRAY_SIZE(tegra_ntrig_i2c_devices));	
}

static int __init ntrig_i2c_init(void)
{
	printk(KERN_DEBUG "in %s\n", __func__);
	register_ntrig_i2c_devices();
	return i2c_add_driver(&ntrig_i2c_driver);
}

static void __exit ntrig_i2c_exit(void)
{
	i2c_del_driver(&ntrig_i2c_driver);
}

module_init(ntrig_i2c_init);
module_exit(ntrig_i2c_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("N-Trig I2C driver");
