/**
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2019-2023,2026 NXP
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
 *  This is the main implementation file for the UWA system manager.
 *
 */

#include "uwa_sys.h"
#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "phNxpLogApis_UciCore.h"

const tUWA_SYS_CFG uwa_sys_cfg = {
    UWA_MBOX_EVT_MASK, /* GKI mailbox event */
    UWA_MBOX_ID,       /* GKI mailbox id */
    UWA_TIMER_ID,      /* GKI timer id */
};

tUWA_SYS_CFG *p_uwa_sys_cfg = (tUWA_SYS_CFG *)&uwa_sys_cfg;

/* system manager control block definition */
tUWA_SYS_CB uwa_sys_cb = {0}; /* uwa_sys control block. statically initialize 'flags' field to 0 */

void uwa_sys_init(void)
{
    phOsalUwb_SetMemory(&uwa_sys_cb, 0, sizeof(tUWA_SYS_CB));
}

void uwa_sys_event(UWB_HDR *p_msg)
{
    uint8_t id;
    bool freebuf = TRUE;

    UCI_TRACE_D("UWA got event 0x%04X", p_msg->event);

    /* get subsystem id from event */
    id = (uint8_t)(p_msg->event >> 8);

    /* verify id and call subsystem event handler */
    if ((id < UWA_ID_MAX) && (uwa_sys_cb.is_reg[id])) {
        freebuf = (*uwa_sys_cb.reg[id]->evt_hdlr)(p_msg);
    }
    else {
        UCI_TRACE_W("UWA got unregistered event id %d", id);
    }

    if (freebuf) {
        phOsalUwb_FreeMemory(p_msg);
    }
}

void uwa_sys_register(UWA_SYS_ID_t id, const tUWA_SYS_REG *p_reg)
{
    uwa_sys_cb.reg[id]    = (tUWA_SYS_REG *)p_reg;
    uwa_sys_cb.is_reg[id] = TRUE;

    if ((id != UWA_ID_DM) && (id != UWA_ID_SYS))
        uwa_sys_cb.enable_cplt_mask |= (uint16_t)(0x0001 << id);

    UCI_TRACE_D("id=%i, enable_cplt_mask=0x%x", id, uwa_sys_cb.enable_cplt_mask);
}

void uwa_sys_disable_subsystems(bool graceful)
{
    UCI_TRACE_D("uwa_sys: disabling subsystems:%d", graceful);
    uwa_sys_cb.graceful_disable = graceful;

    /* Disable DM */
    if (uwa_sys_cb.is_reg[UWA_ID_DM]) {
        (*uwa_sys_cb.reg[UWA_ID_DM]->disable)();
    }
}

void uwa_sys_sendmsg(void *p_msg)
{
    phUwb_OSAL_send_msg(UWB_TASK, p_uwa_sys_cfg->mbox, p_msg);
}
