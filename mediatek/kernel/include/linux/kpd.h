/* alps/ALPS_SW/TRUNK/MAIN/alps/kernel/include/linux/kpd.h
 *
 * (C) Copyright 2009 
 * MediaTek <www.MediaTek.com>
 *
 * MT6516 Sensor IOCTL & data structure
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __KPD_H__
#define __KPD_H__

#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/aee.h>

#ifdef MTK_SMARTBOOK_SUPPORT
#include <linux/sbsuspend.h> //smartbook
#endif

#include <asm/atomic.h>
#include <asm/uaccess.h>

#include <mach/hal_pub_kpd.h>

#define KPD_AUTOTEST	KPD_YES
#define KPD_DEBUG	KPD_YES

#if KPD_AUTOTEST
#define PRESS_OK_KEY		_IO('k', 1)
#define RELEASE_OK_KEY		_IO('k', 2)
#define PRESS_MENU_KEY		_IO('k', 3)
#define RELEASE_MENU_KEY	_IO('k', 4)
#define PRESS_UP_KEY		_IO('k', 5)
#define RELEASE_UP_KEY		_IO('k', 6)
#define PRESS_DOWN_KEY		_IO('k', 7)
#define RELEASE_DOWN_KEY	_IO('k', 8)
#define PRESS_LEFT_KEY		_IO('k', 9)
#define RELEASE_LEFT_KEY	_IO('k', 10)
#define PRESS_RIGHT_KEY		_IO('k', 11)
#define RELEASE_RIGHT_KEY	_IO('k', 12)
#define PRESS_HOME_KEY		_IO('k', 13)
#define RELEASE_HOME_KEY	_IO('k', 14)
#define PRESS_BACK_KEY		_IO('k', 15)
#define RELEASE_BACK_KEY	_IO('k', 16)
#define PRESS_CALL_KEY		_IO('k', 17)
#define RELEASE_CALL_KEY	_IO('k', 18)
#define PRESS_ENDCALL_KEY	_IO('k', 19)
#define RELEASE_ENDCALL_KEY	_IO('k', 20)
#define PRESS_VLUP_KEY		_IO('k', 21)
#define RELEASE_VLUP_KEY	_IO('k', 22)
#define PRESS_VLDOWN_KEY	_IO('k', 23)
#define RELEASE_VLDOWN_KEY	_IO('k', 24)
#define PRESS_FOCUS_KEY		_IO('k', 25)
#define RELEASE_FOCUS_KEY	_IO('k', 26)
#define PRESS_CAMERA_KEY	_IO('k', 27)
#define RELEASE_CAMERA_KEY	_IO('k', 28)
#define PRESS_POWER_KEY		_IO('k', 30)
#define RELEASE_POWER_KEY	_IO('k', 31)
#endif
#define SET_KPD_KCOL		_IO('k', 29)


#define KPD_SAY		"kpd: "
#if KPD_DEBUG
#define kpd_print(fmt, arg...)	printk(KPD_SAY fmt, ##arg)
#else
#define kpd_print(fmt, arg...)	do {} while (0)
#endif

#define KPD_PWRKEY_MAP KEY_POWER
#define KPD_PWRKEY_USE_EINT KPD_NO
#define KPD_PWRKEY_USE_PMIC KPD_YES
#define KPD_DRV_CTRL_BACKLIGHT	KPD_NO	/* retired, move to Lights framework */
#define KPD_BACKLIGHT_TIME	8	/* sec */

/*[A] add Hall-Switch function,xmxkp,20140526*/
#define KPD_HALLKEY_USE_EINT
#ifdef KPD_HALLKEY_USE_EINT
#define CUST_EINT_POLARITY_HIGH             1

#define KPD_HALLKEY_SLEEP                  KEY_HALL_SLEEP
#define KPD_HALLKEY_WAKE                   KEY_HALL_WAKE

#define KPD_HALLKEY_EINT_PIN               GPIO_MHALL_EINT_PIN
#define KPD_HALLKEY_EINT_PIN_M_EINT        GPIO_MHALL_EINT_PIN_M_EINT


#define KPD_HALLKEY_EINT_NUM               CUST_EINT_MHALL_NUM
#define KPD_HALLKEY_EINT_DEBOUNCE_CN       CUST_EINT_MHALL_DEBOUNCE_CN
#define KPD_HALLKEY_EINT_DEBOUNCE_EN       CUST_EINT_MHALL_DEBOUNCE_EN
#define KPD_HALLKEY_EINT_TYPE              CUST_EINT_MHALL_TYPE 

#define KEY_HALL_SLEEP 249
#define KEY_HALL_WAKE  250
#endif
/*[END] xmlinyl,20140526*/

#endif // __KPD_H__
