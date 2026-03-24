/**
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2019,2022,2023,2026 NXP
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

#ifndef UWA_SYS_H
#define UWA_SYS_H

#include "phUwb_BuildConfig.h"
#include "UwbAdaptation.h"
#include "uwa_api.h"
#include "uwb_target.h"
#include "uwb_types.h"

/**
 * Constants and data types
 */

/** SW sub-systems */
typedef enum
{
    UWA_ID_SYS, /** system manager                      */
    UWA_ID_DM,  /** device manager                      */
    UWA_ID_MAX
}UWA_SYS_ID_t;

/** event handler function type */
typedef bool(tUWA_SYS_EVT_HDLR)(UWB_HDR *p_msg);

/** disable function type */
typedef void(tUWA_SYS_DISABLE)(void);

typedef void(tUWA_SYS_ENABLE_CBACK)(void);

/** registration structure */
typedef struct
{
    tUWA_SYS_EVT_HDLR *evt_hdlr;
    tUWA_SYS_DISABLE *disable;
} tUWA_SYS_REG;

/** system manager configuration structure */
typedef struct
{
    uint16_t mbox_evt; /** GKI mailbox event */
    uint8_t mbox;      /** GKI mailbox id */
    uint8_t timer;     /** GKI timer id */
} tUWA_SYS_CFG;

/**
 *  state table
 */

/** system manager control block */
typedef struct
{
    uint32_t flags;                /** uwa_sys flags (must be first element of structure) */
    tUWA_SYS_REG *reg[UWA_ID_MAX]; /** registration structures */
    bool is_reg[UWA_ID_MAX];       /** registration structures */
    tUWA_SYS_ENABLE_CBACK *p_enable_cback;
    uint16_t enable_cplt_flags;
    uint16_t enable_cplt_mask;

    bool graceful_disable; /** TRUE if UWA_Disable () is called with TRUE */
    bool timers_disabled;  /** TRUE if sys timers disabled */
} tUWA_SYS_CB;

/**
 *  Global data
 */
/** system manager control block */
extern tUWA_SYS_CB uwa_sys_cb;

/** system manager configuration structure */
extern tUWA_SYS_CFG *p_uwa_sys_cfg;

/**
 *  Macros
 */

/** Calculate start of event enumeration; id is top 8 bits of event */
#define UWA_SYS_EVT_START(id) ((id) << 8)

/**
 *  Function declarations
 */

/**
**
** Function         uwa_sys_init
**
** Description      UWA initialization; called from task initialization.
**
**
** Returns          void
**
*/
extern void uwa_sys_init(void);

/**
**
** Function         uwa_sys_event
**
** Description      BTA event handler; called from task event handler.
**
**
** Returns          void
**
*/
extern void uwa_sys_event(UWB_HDR *p_msg);

/**
**
** Function         uwa_sys_register
**
** Description      Called by other subsystems to register their event
**                  handler.
**
** Returns          void
**
*/
extern void uwa_sys_register(UWA_SYS_ID_t id, const tUWA_SYS_REG *p_reg);

/**
**
** Function         uwa_sys_disable_subsystems
**
** Description      Call on UWA shutdown. Disable all subsystems above UWA_DM
**
** Returns          void
**
*/
extern void uwa_sys_disable_subsystems(bool graceful);

/**
**
** Function         uwa_sys_sendmsg
**
** Description      Sends the Message Event to Message Queue via Osal Layer
**
** Returns          void
**
*/
extern void uwa_sys_sendmsg(void *p_msg);

#endif /** UWA_SYS_H */
