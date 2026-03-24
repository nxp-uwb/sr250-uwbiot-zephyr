/*
 *
 * Copyright 2018-2024, 2026 NXP.
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

/*
 *
 *  This is the main implementation file for the UWA device manager.
 *
 */

#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "uci_hmsgs.h"
#include "phNxpLogApis_UciCore.h"

/** Constants and types */
static const tUWA_SYS_REG uwa_dm_sys_reg = {&uwa_dm_evt_hdlr, &uwa_dm_sys_disable};
tUWA_DM_CB uwa_dm_cb;

#define UWA_DM_NUM_ACTIONS (UWA_DM_MAX_EVT & 0x00ff)

/* type for action functions */
typedef bool (*tUWA_DM_ACTION)(tUWA_DM_MSG *p_data);

/* action function list */
const tUWA_DM_ACTION uwa_dm_action[] = {
    /* device manager local device API events */
    &uwa_dm_enable,          /* UWA_DM_API_ENABLE_EVT            */
    &uwa_dm_disable,         /* UWA_DM_API_DISABLE_EVT           */
    &uwa_dm_register_ext_cb, /* UWA_DM_API_REGISTER_EXT_CB_EVT   */
    &uwa_dm_act_send_raw_cmd /* UWA_DM_API_SEND_RAW_EVT          */
};

/** Local function prototypes */
#if (ENABLE_UCI_MODULE_TRACES == TRUE)
static const char *uwa_dm_evt_2_str(uint16_t event);
#endif

void uwa_dm_init(void)
{
    UCI_TRACE_D(__FUNCTION__);
    /* register message handler on UWA SYS */
    phOsalUwb_SetMemory(&uwa_dm_cb, 0, sizeof(tUWA_DM_CB));
    uwa_sys_register(UWA_ID_DM, &uwa_dm_sys_reg);
}

bool uwa_dm_evt_hdlr(UWB_HDR *p_msg)
{
    UCI_TRACE_D(__FUNCTION__);
    bool freebuf   = TRUE;
    uint16_t event = p_msg->event & 0x00ff;
    tUCI_CMD *uci_cmd;
    UCI_TRACE_D("event: %s (0x%02x)", uwa_dm_evt_2_str(event), event);
    /* execute action functions */
    if (event < (UWA_DM_INTERNAL_NUM_ACTIONS & 0x00FF)) {
        freebuf = (*uwa_dm_action[event])((tUWA_DM_MSG *)p_msg);
    }
    else if (event < UWA_DM_NUM_ACTIONS) {
        uci_cmd = (tUCI_CMD *)p_msg;
        freebuf = uci_snd_cmd_interface(p_msg->event, uci_cmd->length, uci_cmd->p_data, uci_cmd->pbf);
    }
    return freebuf;
}

void uwa_dm_sys_disable(void)
{
    /* Disable the DM sub-system */
    uwa_dm_disable_complete();
}

/**
**
** Function         uwa_dm_uwb_revt_2_str
**
** Description      convert uwb revt to string
**
*/
#if (ENABLE_UCI_MODULE_TRACES == TRUE)
static const char *uwa_dm_evt_2_str(uint16_t event)
{
    switch (UWA_SYS_EVT_START(UWA_ID_DM) | event) {
    case UWA_DM_API_ENABLE_EVT:
        return "UWA_DM_API_ENABLE_EVT";

    case UWA_DM_API_DISABLE_EVT:
        return "UWA_DM_API_DISABLE_EVT";

    case UWA_DM_API_REGISTER_EXT_CB_EVT:
        return "UWA_DM_API_REGISTER_EXT_CB_EVT";

    case UWA_DM_API_CORE_GET_DEVICE_INFO_EVT:
        return "UWA_DM_API_CORE_GET_DEVICE_INFO_EVT";

    case UWA_DM_API_CORE_SET_CONFIG_EVT:
        return "UWA_DM_API_CORE_SET_CONFIG_EVT";

    case UWA_DM_API_CORE_GET_CONFIG_EVT:
        return "UWA_DM_API_CORE_GET_CONFIG_EVT";

    case UWA_DM_API_CORE_DEVICE_RESET_EVT:
        return "UWA_DM_API_CORE_DEVICE_RESET_EVT";

    case UWA_DM_API_SESSION_INIT_EVT:
        return "UWA_DM_API_SESSION_INIT_EVT";

    case UWA_DM_API_SESSION_DEINIT_EVT:
        return "UWA_DM_API_SESSION_DEINIT_EVT";

    case UWA_DM_API_SESSION_GET_COUNT_EVT:
        return "UWA_DM_API_SESSION_GET_COUNT_EVT";

    case UWA_DM_API_SESSION_SET_APP_CONFIG_EVT:
        return "UWA_DM_API_SESSION_SET_APP_CONFIG_EVT";

    case UWA_DM_API_SESSION_GET_APP_CONFIG_EVT:
        return "UWA_DM_API_SESSION_GET_APP_CONFIG_EVT";

    case UWA_DM_API_SESSION_START_EVT:
        return "UWA_DM_API_SESSION_START_EVT";

    case UWA_DM_API_SESSION_STOP_EVT:
        return "UWA_DM_API_SESSION_STOP_EVT";

    case UWA_DM_API_SEND_RAW_EVT:
        return "UWA_DM_API_SEND_RAW_EVT";

    case UWA_DM_API_SESSION_GET_STATE_EVT:
        return "UWA_DM_API_SESSION_GET_STATE_EVT";

#if !(UWBIOT_UWBD_SR04X)
    case UWA_DM_API_TEST_SET_CONFIG_EVT:
        return "UWA_DM_API_TEST_SET_CONFIG_EVT";

    case UWA_DM_API_TEST_GET_CONFIG_EVT:
        return "UWA_DM_API_TEST_GET_CONFIG_EVT";

    case UWA_DM_API_TEST_PERIODIC_TX_EVT:
        return "UWA_DM_API_TEST_PERIODIC_TX_EVT";

    case UWA_DM_API_TEST_PER_RX_EVT:
        return "UWA_DM_API_TEST_PER_RX_EVT";

    case UWA_DM_API_TEST_STOP_SESSION_EVT:
        return "UWA_DM_API_TEST_STOP_SESSION_EVT";

    case UWA_DM_API_TEST_RX_EVT:
        return "UWA_DM_API_TEST_RX_EVT";

    case UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT:
        return "UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT";

    case UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT:
        return "UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT";
#endif //!(UWBIOT_UWBD_SR04X)
    }

    return "Unknown or Vendor Specific";
}
#endif /* ENABLE_UCI_MODULE_TRACES */
