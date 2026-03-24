/**
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2020,2022-2023, 2026 NXP
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

#ifndef UWB_TARGET_H
#define UWB_TARGET_H

#include "phUwbTypes.h"

#ifndef USERIAL_DEBUG
#define USERIAL_DEBUG FALSE
#endif

#include "phUwb_BuildConfig.h"

/** UCI Command, Notification or Data*/
#define BT_EVT_TO_UWB_UCI 0x4000
/** messages between UWB and UCI task */
#define BT_EVT_TO_UWB_MSGS 0x4300

/** start timer */
#define BT_EVT_TO_START_TIMER 0x3c00

/** start quick timer */
#define BT_EVT_TO_START_QUICK_TIMER 0x3e00

/**
 *
 * GKI Mail Box and Timer
 *
 */

/** Mailbox event mask for UWB stack */
#ifndef UWB_MBOX_EVT_MASK
#define UWB_MBOX_EVT_MASK (TASK_MBOX_0_EVT_MASK)
#endif

/** Mailbox ID for UWB stack */
#ifndef UWB_MBOX_ID
#define UWB_MBOX_ID (TASK_MBOX_0)
#endif

/** Mailbox event mask for UFA */
#ifndef UWA_MBOX_EVT_MASK
#define UWA_MBOX_EVT_MASK (TASK_MBOX_2_EVT_MASK)
#endif

/** Mailbox ID for UFA */
#ifndef UWA_MBOX_ID
#define UWA_MBOX_ID (TASK_MBOX_2)
#endif

/** timer event mask used for quick timer in UWB stack */
#ifndef UWB_QUICK_TIMER_EVT_MASK
#define UWB_QUICK_TIMER_EVT_MASK (TIMER_1_EVT_MASK)
#endif

/** GKI timer id used for protocol timer in UFA */
#ifndef UWA_TIMER_ID
#define UWA_TIMER_ID (TIMER_1)
#endif

/** GKI timer event mask used for protocol timer in UFA */
#ifndef UWA_TIMER_EVT_MASK
#define UWA_TIMER_EVT_MASK (TIMER_2_EVT_MASK)
#endif

/** Quick Timer */
#ifndef QUICK_TIMER_TICKS_PER_SEC
#define QUICK_TIMER_TICKS_PER_SEC 100 /** 10ms timer */
#endif

/**
 *
 * UCI Transport definitions
 *
 */
/** offset of the first UCI packet in buffer for outgoing */
#ifndef UCI_MSG_OFFSET_SIZE
#define UCI_MSG_OFFSET_SIZE 1
#endif

/**
 *
 * UWB Retry Timeout details
 *
 */
/** Timeout for receiving response to UCI command */
#ifndef UWB_CMD_CMPL_TIMEOUT
#if (UWBIOT_UWBD_SR04X)
// Timeout required for the Test Commands.
#define UWB_CMD_CMPL_TIMEOUT 250 // 250ms
#else
#define UWB_CMD_CMPL_TIMEOUT 150 // 150ms
#endif /* (UWBIOT_UWBD_SR04X) */
#endif //UWB_CMD_CMPL_TIMEOUT


/** Timeout for receiving response to UCI command in case of retry */
#ifndef UWB_CMD_RETRY_TIMEOUT
#define UWB_CMD_RETRY_TIMEOUT UWB_CMD_CMPL_TIMEOUT
#endif //UWB_CMD_RETRY_TIMEOUT

/** Timeout offset for incremental delay in milliseconds to avoid blind spots when UWBD wakes up form DPD*/
#define RETRY_RSP_TIMEOUT_OFFSET_MS 10

/** Timeout for receiving response to UCI command in case of retry */
#ifndef UWB_CMD_RETRY_TIMEOUT
#define UWB_CMD_RETRY_TIMEOUT UWB_CMD_CMPL_TIMEOUT // 150ms
#endif

/** Timeout offset for incremental delay in milliseconds to avoid blind spots when UWBD wakes up form DPD*/
#define RETRY_RSP_TIMEOUT_OFFSET_MS 10

/** Maximum number of UCI commands that the UWBC accepts without needing to wait
 * for response */
#ifndef UCI_MAX_CMD_WINDOW
#define UCI_MAX_CMD_WINDOW 1
#endif

#ifndef UCI_CMD_MAX_RETRY_COUNT
#if (UWBIOT_UWBD_SR04X)
/** if there's more delay, PCTT Demo fails, hence needs smaller
 *  retry count
 */
#define UCI_CMD_MAX_RETRY_COUNT 2
#else
#define UCI_CMD_MAX_RETRY_COUNT 10
#endif
#endif

/** Timeout for receiving Credit Ntf in case of retry ntf received after data send */
#ifndef UWB_DATA_CREDIT_NTF_TIMEOUT
#define UWB_DATA_CREDIT_NTF_TIMEOUT 5 // 5ms
#endif

/**
 * Define HAL_WRITE depending on whether HAL is using shared GKI resources
 * as the UWB stack.
 */
#ifndef HAL_WRITE
#define HAL_WRITE(p)                                                 \
    {                                                                \
        uwb_cb.p_hal->write(p->len, (uint8_t *)(p + 1) + p->offset); \
        phOsalUwb_FreeMemory(p);                                     \
    }
#define HAL_RE_WRITE(p)                                              \
    {                                                                \
        uwb_cb.p_hal->write(p->len, (uint8_t *)(p + 1) + p->offset); \
    }

#define HAL_UCI_CMD_WRITE(len, buf)               \
    {                                             \
        uwb_cb.p_hal->write(len, (uint8_t *)buf); \
    }

/** Mem allocation with 8 byte alignment */
#define HAL_MALLOC(x) malloc(((x - 1) | 7) + 1)
#endif /** HAL_WRITE */

#endif /** UWB_TARGET_H */
