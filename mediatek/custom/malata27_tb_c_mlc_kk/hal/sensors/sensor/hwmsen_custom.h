/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef __HWMSEN_CUSTOM_H__ 
#define __HWMSEN_CUSTOM_H__

#ifdef CUSTOM_KERNEL_ACCELEROMETER
    #define ACCELEROMETER           "MPU-6880 Six-Axis Accelerometer"
    #define ACCELEROMETER_VENDER    "INVENSENSE"
#endif

#ifdef CUSTOM_KERNEL_ALSPS
		#define PROXIMITY 			"APDS-9930 Proximity Sensor"
		#define PROXIMITY_VENDER 		"AVAGO"
		#define LIGHT 				"APDS-9930 Ambient Light Sensor"
		#define LIGHT_VENDER 			"AVAGO"
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
    #define MAGNETOMETER 			"AK09911 3-axis Electronic Compass"
		#define MAGNETOMETER_VENDER 		"AKM"
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
		#define GYROSCOPE 			"MPU-6880 Six-Axis Gyroscope"
		#define GYROSCOPE_VENDER 		"INVENSENSE"
#endif

#endif

