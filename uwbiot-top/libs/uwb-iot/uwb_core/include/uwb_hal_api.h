/**
 *
 *  Copyright (C) 1999-2014 Broadcom Corporation
 *  Copyright 2018-2019,2022,2023, 2026 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
/**
 *
 *  UWB Hardware Abstraction Layer API
 *
 */
#ifndef UWB_HAL_API_H
#define UWB_HAL_API_H

#include "phUwbTypes.h"
#if !(UWBIOT_UWBD_SR04X)
#include <UwbApi_Types_Proprietary.h>
#endif //!(UWBIOT_UWBD_SR04X)

#define MAX_IOCTL_TRANCEIVE_CMD_LEN 256

typedef struct
{
    uint16_t cmd_len;
    uint8_t p_cmd[MAX_IOCTL_TRANCEIVE_CMD_LEN];
} uwb_uci_ExtnCmd_t;

typedef struct
{
    uint8_t status;
    uwb_uci_ExtnCmd_t uciCmd;
    uint8_t enableCirDump;
    uint8_t enableFwDump;
} InputOutputData_t;

typedef InputOutputData_t uwb_uci_IoctlInOutData_t;

/** HAL IOCTL structure */
typedef union {
#if !(UWBIOT_UWBD_SR04X)
    /* FW Crash info */
    phFwCrashLogInfo_t *pCrashInfo;
#endif //!(UWBIOT_UWBD_SR04X)
    /* In Out data */
    InputOutputData_t *pIoData;
} tHAL_UWB_IOCTL;

typedef uint8_t tHAL_UWB_STATUS;
typedef void(tHAL_UWB_CBACK)(uint8_t event, tHAL_UWB_STATUS status);
typedef void(tHAL_UWB_DATA_CBACK)(uint16_t data_len, uint8_t *p_data);

/**
** tHAL_UWB_ENTRY HAL entry-point lookup table
*/

typedef void(tHAL_UWB_API_OPEN)(tHAL_UWB_CBACK *p_hal_cback, tHAL_UWB_DATA_CBACK *p_data_cback);
typedef void(tHAL_UWB_API_CLOSE)(void);
typedef void(tHAL_UWB_API_WRITE)(uint16_t data_len, uint8_t *p_data);
typedef uint8_t(tHAL_UWB_API_IOCTL)(long arg, tHAL_UWB_IOCTL *p_data);

typedef struct
{
    tHAL_UWB_API_OPEN *open;
    tHAL_UWB_API_CLOSE *close;
    tHAL_UWB_API_WRITE *write;
    tHAL_UWB_API_IOCTL *ioctl;
} tHAL_UWB_ENTRY;

typedef struct
{
    const tHAL_UWB_ENTRY *hal_entry_func;
} tHAL_UWB_CONTEXT;

#endif /* UWB_HAL_API_H  */
