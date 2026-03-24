/**
 *
 *  Copyright (C) 1999-2014 Broadcom Corporation
 *  Copyright 2018-2019,2023 NXP
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
 *  this file contains the UCI transport internal definitions and functions.
 *
 */

#ifndef UWB_HAL_INT_H
#define UWB_HAL_INT_H

#include "uci_defs.h"
#include "uwb_hal_api.h"

enum
{
    HAL_UWB_OPEN_CPLT_EVT  = 0x00,
    HAL_UWB_CLOSE_CPLT_EVT = 0x01,
    HAL_UWB_ERROR_EVT      = 0x02
};
enum
{
    HAL_UWB_STATUS_OK              = 0x00,
    HAL_UWB_STATUS_ERR_TRANSPORT   = 0x01,
    HAL_UWB_STATUS_ERR_CMD_TIMEOUT = 0x02
};
enum
{
    HAL_UWB_IOCTL_FW_DWNLD = 0,
    HAL_UWB_IOCTL_SET_SUSPEND_STATE,
    HAL_UWB_IOCTL_DUMP_FW_LOG,
    HAL_UWB_IOCTL_DUMP_FW_CRASH_LOG,
    HAL_UWB_IOCTL_SET_BOARD_CONFIG,
    HAL_UWB_IOCTL_ESE_RESET
};

#endif /* UWB_HAL_INT_H */
