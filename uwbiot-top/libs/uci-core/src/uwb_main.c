/*
 *
 * Copyright 2018-2023 NXP.
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

#include <stdlib.h>
#include "UwbAdaptation.h"
#include "uwb_target.h"
#include "uwa_sys.h"
#include "uwb_hal_api.h"
#include "uwb_hal_int.h"
#include "uwb_int.h"
#include "uci_hmsgs.h"
#include "uwa_dm_int.h"
#include "phNxpLogApis_UciCore.h"
#include "uwa_api.h"

/** Declarations */

tUWB_CB uwb_cb;
/**
**
** Function         uwb_state_name
**
** Description      This function returns the state name.
**
** NOTE             conditionally compiled to save memory.
**
** Returns          pointer to the name
**
*/
#if (ENABLE_UCI_MODULE_TRACES == TRUE)
static const char *uwb_state_name(uint8_t state)
{
    switch (state) {
    case UWB_STATE_NONE:
        return "NONE";
    case UWB_STATE_W4_HAL_OPEN:
        return "W4_HAL_OPEN";
    case UWB_STATE_IDLE:
        return "IDLE";
    case UWB_STATE_ACTIVE:
        return "ACTIVE";
    case UWB_STATE_CLOSING:
        return "CLOSING";
    case UWB_STATE_W4_HAL_CLOSE:
        return "W4_HAL_CLOSE";
    default:
        return "???? UNKNOWN STATE";
    }
}
#endif

/**
**
** Function         uwb_hal_event_name
**
** Description      This function returns the HAL event name.
**
** NOTE             conditionally compiled to save memory.
**
** Returns          pointer to the name
**
*/
#if (ENABLE_UCI_MODULE_TRACES == TRUE)
static const char *uwb_hal_event_name(uint8_t event)
{
    switch (event) {
    case HAL_UWB_OPEN_CPLT_EVT:
        return "HAL_UWB_OPEN_CPLT_EVT";

    case HAL_UWB_CLOSE_CPLT_EVT:
        return "HAL_UWB_CLOSE_CPLT_EVT";

    case HAL_UWB_ERROR_EVT:
        return "HAL_UWB_ERROR_EVT";

    default:
        return "???? UNKNOWN EVENT";
    }
}
#endif

void uwb_set_state(tUWB_STATE uwb_state)
{
    UCI_TRACE_D("uwb_set_state %d (%s)->%d (%s)",
        uwb_cb.uwb_state,
        uwb_state_name(uwb_cb.uwb_state),
        uwb_state,
        uwb_state_name(uwb_state));
    uwb_cb.uwb_state = uwb_state;
}

void uwb_gen_cleanup(void)
{
    /* clear any pending CMD/RSP */
    uwb_main_flush_cmd_queue();
}

void uwb_main_handle_hal_evt(tUWB_HAL_EVT_MSG *p_msg)
{
    UCI_TRACE_D("HAL event=0x%x", p_msg->hal_evt);
    tUCI_STATUS status = UCI_STATUS_FAILED;
    switch (p_msg->hal_evt) {
    case HAL_UWB_OPEN_CPLT_EVT: /* only for failure case */
        (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_ENABLE, sizeof(tUCI_STATUS), &status);
        break;

    case HAL_UWB_CLOSE_CPLT_EVT:
        if (uwa_dm_cb.p_dm_rsp_cback) {
            if (uwb_cb.uwb_state == UWB_STATE_W4_HAL_CLOSE) {
                uwb_set_state(UWB_STATE_NONE);
                (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_DISABLE, 0, NULL);
                uwa_dm_cb.p_dm_rsp_cback = NULL;
            }
            else {
                /* found error during initialization */
                uwb_set_state(UWB_STATE_NONE);
                (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_ENABLE, sizeof(tUCI_STATUS), &status);
            }
        }
        break;

    case HAL_UWB_ERROR_EVT:
        switch (p_msg->status) {
        case HAL_UWB_STATUS_ERR_TRANSPORT:
            /* if enabling UWB, notify upper layer of failure after closing HAL
             */
            if (uwb_cb.uwb_state < UWB_STATE_IDLE) {
                (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_ENABLE, sizeof(tUCI_STATUS), &status);
            }
            break;
        default:
            break;
        }
        break;
    default:
        UCI_TRACE_E("unhandled event (0x%x).", p_msg->hal_evt);
        break;
    }
}

void uwb_main_flush_cmd_queue(void)
{
    UCI_TRACE_D(__FUNCTION__);

    /* initialize command window */
    uwb_cb.uci_cmd_window = UCI_MAX_CMD_WINDOW;

    /* Stop command-pending timer */
    uwb_stop_quick_timer();
    uwb_cb.is_resp_pending = FALSE;
    uwb_cb.cmd_retry_count = 0;
}

void uwb_main_post_hal_evt(uint8_t hal_evt, tUCI_STATUS status)
{
    tUWB_HAL_EVT_MSG *p_msg;

    p_msg = (tUWB_HAL_EVT_MSG *)phOsalUwb_GetMemory(sizeof(tUWB_HAL_EVT_MSG));
    if (p_msg != NULL) {
        /* Initialize UWB_HDR */
        p_msg->hdr.len            = 0;
        p_msg->hdr.event          = BT_EVT_TO_UWB_MSGS;
        p_msg->hdr.offset         = 0;
        p_msg->hdr.layer_specific = 0;
        p_msg->hal_evt            = hal_evt;
        p_msg->status             = status;
        phUwb_OSAL_send_msg(UWB_TASK, UWB_MBOX_ID, p_msg);
    }
    else {
        UCI_TRACE_E("No buffer");
    }
}

void uwb_main_hal_cback(uint8_t event, tUCI_STATUS status)
{
    UCI_TRACE_D("uwb_main_hal_cback event: %s(0x%x), status=%d", uwb_hal_event_name(event), event, status);
    switch (event) {
    case HAL_UWB_OPEN_CPLT_EVT:
        /*
         ** if UWB_Disable() is called before receiving HAL_UWB_OPEN_CPLT_EVT,
         ** then wait for HAL_UWB_CLOSE_CPLT_EVT.
         */
        if (uwb_cb.uwb_state == UWB_STATE_W4_HAL_OPEN) {
            if (status == HAL_UWB_STATUS_OK) {
                /* Notify UWB_TASK that UCI transport is initialized */
                phUwb_OSAL_send_msg(UWB_TASK, UWB_TASK_EVT_TRANSPORT_READY, NULL);
            }
            else {
                uwb_main_post_hal_evt(event, status);
            }
        }
        break;

    case HAL_UWB_CLOSE_CPLT_EVT:
    case HAL_UWB_ERROR_EVT:
        uwb_main_post_hal_evt(event, status);
        break;

    default:
        UCI_TRACE_E("uwb_main_hal_cback unhandled event %x", event);
        break;
    }
}

void uwb_main_hal_data_cback(uint16_t data_len, uint8_t *p_data)
{
    UWB_HDR *p_msg;
    uint16_t size;

    /* ignore all data while shutting down UWB */
    if (uwb_cb.uwb_state == UWB_STATE_W4_HAL_CLOSE || uwb_cb.uwb_state == UWB_STATE_W4_HAL_OPEN) {
        return;
    }
    if (p_data) {
        size  = (uint16_t)(UWB_HDR_SIZE + UWB_RECEIVE_MSGS_OFFSET + data_len);
        p_msg = (UWB_HDR *)phOsalUwb_GetMemory(size);
        if (p_msg != NULL) {
            /* Initialize UWB_HDR */
            p_msg->len    = data_len;
            p_msg->event  = BT_EVT_TO_UWB_UCI;
            p_msg->offset = UWB_RECEIVE_MSGS_OFFSET;

            /* no need to check length, it always less than pool size */
            phOsalUwb_MemCopy((uint8_t *)(p_msg + 1) + p_msg->offset, p_data, p_msg->len);

            phUwb_OSAL_send_msg(UWB_TASK, UWB_MBOX_ID, p_msg);
        }
        else {
            UCI_TRACE_E("No buffer");
        }
    }
}
