/*
 * Copyright 2019-2022 NXP
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

#ifndef PHUWB_CONFIGURATION_H
#define PHUWB_CONFIGURATION_H

#include "phUwbTypes.h"

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif // UWBIOT_USE_FTR_FILE

/* Build UWB Middle-ware for a specific Mode */
#define UWB_BUILD_STANDALONE_DEFAULT  1
#define UWB_BUILD_STANDALONE_WITH_BLE 4

#define UWBCORE_SDK_BUILDCONFIG UWB_BUILD_STANDALONE_DEFAULT

/* doc:start:enable-int-fw-download */

/* Internal Firmware Download is DISABLED by Default.
 *
 * Enable it if required by setting INTERNAL_FIRMWARE_DOWNLOAD to ENABLED
 */

#define INTERNAL_FIRMWARE_DOWNLOAD DISABLED
/* doc:end:enable-int-fw-download */

/* Check for compilation */
#ifndef ENABLED
#error ENABLED must be defined
#endif
#ifndef DISABLED
#error DISABLED must be defined
#endif

/* doc:start:enable-Presence-detection */

/*
 * PRESENCE_DETECTION flag is used to either enable or disable presence detection
 * Presence Detection Flag is DISABLED by Default.
 * When enabled, the presence detection algorithm is enabled and used.
 * Note: The presence detection algorithm requires a powerful controller
 *       Hence is must only be enabled for PC or RPi demos
 *       or when the Microcontroller is powerful enough.
 *
 * Enable for Presence detection demo by setting PRESENCE_DETECTION to ENABLED
 */
#define PRESENCE_DETECTION DISABLED
/* doc:end:enable-Presence-detection */

/* doc:start:enable-Breathing-detection */

/* Breathing Detection Flag is DISABLED by Default.
 *
 * Enable for Breathing detection demo by setting BREATHING_DETECTION to ENABLED
 */

#define BREATHING_DETECTION DISABLED
/* doc:end:enable-Breathing-detection */

#endif // PHUWB_CONFIGURATION_H
