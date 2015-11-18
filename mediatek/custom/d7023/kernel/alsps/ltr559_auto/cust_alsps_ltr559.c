/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode = 1,
//Gionee yanggy 2012-07-17 modify for interrupt mode begin
#if 1//def GN_MTK_BSP_ALSPS_INTERRUPT_MODE 
    .polling_mode_ps = 0,
#else
    .polling_mode_ps = 1,
#endif
//Gionee yanggy 2012-07-17 modify for interrupt mode end
    .polling_mode_als = 1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
//Gionee: mali 2012-06-14 modify the CR00623845 from the old data to the new data for mmi test begin
    //.als_level  = {5, 7,  9,  13,  16,  21,  25,  40,  120,  400, 1600,  3000, 6000,  10000, 65535},
    //.als_value  = { 1, 2,  4,   9,  16,  39,  80, 150, 300,  650,  1500,  3500,  8500, 15000,  45000, 100000},      //------the old data
    //.als_level  = { 4, 7,   10,   25,  50,  100,  200, 1600,  10000,  65535},
    //.als_value  = { 1, 60,   90,  225, 640, 1280,  2600, 6400, 60000,  100000},
    //.als_level  = {  4, 10,  100, 1000, 2000,  6000},
    //.als_value  = { 10, 100, 400, 800,  1500,  4000,  10240},
    
    .als_level  = {   4,   7,   20,   30,   40,   70,  100,   150,  300,  400,  600 , 900, 1200,  1400,  2000},
    .als_value  = {0, 30,  70,  120,  120,  200,  200,  600,  1000,  2000,  2000,  3000,  3000, 3500,  10240, 10240},

#if defined (GN_MTK_BSP)
	.ps_threshold =PS_THRESHOLD,
#else
	.ps_threshold_high = 1000,//xmxl@20140520
	.ps_threshold_low = 950,
	.ps_threshold =10,
#endif
//gionee sxt 2011-8-19 add for threshold end
};

struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
