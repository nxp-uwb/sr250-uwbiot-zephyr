/*
 * Copyright 2019,2020 NXP
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
#include "phUwb_BuildConfig.h"

#ifndef UWB_EVT_H
#define UWB_EVT_H

typedef enum
{
    USB_TLV_EVT,
} UWB_EvtType_t;

typedef struct
{
    uint8_t type;
    uint16_t size;
    uint8_t *value;
} tlv_t;

typedef struct
{
    UWB_EvtType_t type;
    void *data;
} UWB_Evt_t;

/* TLV types */

typedef enum
{
    kUWB_PNP_TYPE_NONE           = 0,
    kUWB_PNP_TYPE_UCI_CMD        = 0x01,
    kUWB_PNP_TYPE_HBCI_CMD       = 0x02,
    kUWB_PNP_TYPE_HBCI_LAST_CMD  = 0x03,
    kUWB_PNP_TYPE_RESET          = 0x04,
    kUWB_PNP_TYPE_HBCI_QUERY_CMD = 0x05,
    kUWB_PNP_TYPE_MCU_RESET      = 0x06, // MCU nvReset will be called.
} UWP_PNP_TYPE_t;

/* What will be type of next command */
void phNxpUwb_SetNextPNPTYPE(UWP_PNP_TYPE_t uwb_pnp_type);

#endif
