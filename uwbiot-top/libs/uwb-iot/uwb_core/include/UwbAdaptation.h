/**
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2019,2022-2025 NXP
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
#ifndef UWBADAPT_H
#define UWBADAPT_H

#include "phNxpUciHal.h"
#include "phUwb_BuildConfig.h"
#include "uwb_hal_api.h"
#include "uwb_target.h"
#include "uwa_api.h"

#define TASK_MBOX_0_EVT_MASK 0x0001
#define TASK_MBOX_1_EVT_MASK 0x0002 // Not used at present
#define TASK_MBOX_2_EVT_MASK 0x0004
#define TASK_MBOX_3_EVT_MASK 0x0008 // Not used at present

/* Only one timer is used for command response timeout */
#define TIMER_1 1

#define TIMER_0_EVT_MASK 0x0010
#define TIMER_1_EVT_MASK 0x0020
#define TIMER_2_EVT_MASK 0x0040
#define TIMER_3_EVT_MASK 0x0080

#ifndef UWB_TASK
#define UWB_TASK 3
#endif

/**
** Mailbox definitions. Each task has 4 mailboxes that are used to
** send buffers to the task.
*/
#define TASK_MBOX_0 0
#define TASK_MBOX_2 2

#define NUM_TASK_MBOX 4

#define APPL_EVT_0 8
#define APPL_EVT_7 15

#define UWB_EVENT_MASK(evt) ((uint16_t)(0x0001u << (evt)))

#ifndef UWB_SHUTDOWN_EVT
#define UWB_SHUTDOWN_EVT APPL_EVT_7
#endif

#ifndef UWB_SHUTDOWN_EVT_MASK
#define UWB_SHUTDOWN_EVT_MASK UWB_EVENT_MASK(UWB_SHUTDOWN_EVT)
#endif

#ifndef UWB_TASK
#define UWB_TASK 3
#endif

/**
**
** Function:    Initialize()
**
**
** Returns:     The status.
**              #UCI_STATUS_OK              - On success.
**              #UCI_STATUS_FAILED          - otherwise
**
*/
tUCI_STATUS Initialize(void);

/**
**
** Function:    UwbDeviceInit
**
** Description: Download firmware patch files and apply device configs.
**
** Returns:     None.
**
*/
tUCI_STATUS UwbDeviceInit(bool recovery);

/**
**
** Function:    Finalize()
**
** Returns:     none
**
*/
void Finalize();

/**
**
** Function:    GetHalEntryFuncs()
**
** Description: Get the set of HAL entry points.
**
** Returns:     Functions pointers for HAL entry points.
**
*/
const tHAL_UWB_ENTRY *GetHalEntryFuncs();

/**
**
** Function:    HalRegisterAppCallback
**
** Description: registers app data call back in tml context.
**
** Returns:     None.
**
*/
void HalRegisterAppCallback(phHalAppDataCb *recvDataCb);

/**
**
** Function:    isCmdRespPending
**
** Description: This function is get the Response status for the current command sent to fw
**
** Returns:     TRUE if response is pending, FALSE otherwise.
**
*/
bool isCmdRespPending();

/**
**
** Function:    Hal_setOperationMode
**
** Description: This function is get the Register the Operation Mode as follows
                1: MCTT
                2: STANDALONE(DEFAULT)
** Returns:     None.
**
*/
void Hal_setOperationMode(Uwb_operation_mode_t state);
void phUwb_OSAL_send_msg(uint8_t task_id, uint16_t mbox, void *pmsg);

#endif
