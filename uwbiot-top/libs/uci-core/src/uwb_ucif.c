/*
 *
 * Copyright 2018-2023,2025,2026 NXP.
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
#include "uwb_hal_api.h"
#include "phNxpUciHal_ext.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "uci_test_defs.h"
#include "uci_hmsgs.h"
#include "uwb_int.h"
#include "uwa_sys.h"
#include "uwa_dm_int.h"
#include "uwb_hal_int.h"
#include "phNxpLogApis_UciCore.h"
#include "uwb_target.h"

#define NORMAL_MODE_LENGTH_OFFSET            0x03
#define EXTENDED_MODE_LEN_OFFSET             0x02
#define EXTENDED_MODE_LEN_SHIFT              0x08
#define EXTND_LEN_INDICATOR_OFFSET           0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK      0x80
#define CREDIT_AVAILABILITY_INDICATOR_OFFSET 0x04

#if !(UWBIOT_UWBD_SR04X)
/*
 * This buffer is used to store the command for chaining scenario.
 * Following format is used to store the data in sequence.
 * UCI Header [of size `UCI_MSG_HDR_SIZE`]
 * UCI Payload [Maximum of `UCI_MAX_PAYLOAD_SIZE_CHAINING` bytes]
 */
#define MAX_CHAINED_LEN UCI_MSG_HDR_SIZE + UCI_MAX_DATA_PACKET_SIZE
#elif (UWBIOT_UWBD_SR04X)
/*
 * This buffer is used to store the command for chaining scenario.
 * Following format is used to store the data in sequence.
 * UCI Header [of size `UCI_MSG_HDR_SIZE`]
 * UCI Payload [Maximum of `UCI_MAX_HPRF_PAYLOAD_SIZE_SR040` bytes]
 */
#define MAX_CHAINED_LEN UCI_MSG_HDR_SIZE + UCI_MAX_HPRF_PAYLOAD_SIZE_SR040
#else
/*
 * This buffer is used to store the command for chaining scenario.
 * Following format is used to store the data in sequence.
 * UCI Header [of size `UCI_MSG_HDR_SIZE`]
 * UCI Payload [Maximum of `UCI_MAX_PAYLOAD_SIZE` bytes]
 * In Multicast Scneario for max no of (8) resopnder devices,
 * range notification will come with pbf bit enabled.
 */
#define MAX_CHAINED_LEN UCI_MSG_HDR_SIZE + UCI_MAX_PAYLOAD_SIZE + SR040_MAX_MULTICAST_NTF_PAYLOAD_SIZE
#endif

#if !(UWBIOT_UWBD_SR04X)
#define MAX_CHAINED_PACKET_BUFFER_LEN (4110)
#else
#define MAX_CHAINED_PACKET_BUFFER_LEN (1024)
#endif //!(UWBIOT_UWBD_SR04X)
/*
 * This buffer is used to store the command for retransmit scenarios.
 * Following format is used to store the data in sequence.
 * UWB_HDR    [of size `UWB_HDR_SIZE`]
 * UCI Header [of size `UCI_MSG_HDR_SIZE`]
 * UCI Payload [Maximum of `UCI_MAX_PAYLOAD_SIZE` bytes]
 */
uint8_t last_cmd_buff[UCI_MAX_CMD_BUF_LEN];

typedef struct
{
// Chaining is enabled
#if UWBFTR_ChainedUCI
    uint8_t buffer[MAX_CHAINED_LEN];
#else
    // Chaining is disabled
    /*
     * This buffer is used to store the command for chaining scenario.
     * Following format is used to store the data in sequence.
     * UCI Header [of size `UCI_MSG_HDR_SIZE`]
     * UCI Payload [Maximum of `UCI_MAX_PAYLOAD_SIZE` bytes]
     */
    uint8_t buffer[UCI_MSG_HDR_SIZE + UCI_MAX_PAYLOAD_SIZE];
#endif // UWBFTR_ChainedUCI
    uint8_t oid;
    uint8_t gid;
    uint16_t offset;
    uint8_t is_first_frgmnt_done;
} chained_uci_packet;

void uwb_ucif_update_cmd_window(void)
{
    UCI_TRACE_D("%s: enter",__FUNCTION__);
    /* Sanity check - see if we were expecting a update_window */
    if (uwb_cb.uci_cmd_window == UCI_MAX_CMD_WINDOW) {
        if (uwb_cb.uwb_state != UWB_STATE_W4_HAL_CLOSE) {
            UCI_TRACE_D(
                "uwb_ucif_update_window:This warning may come due to : 1 : UWBD_STATUS_HDP_WAKEUP 2:UWBD_STATUS_ERROR "
                "3: UWB_STATUS_TIMEOUT 4: data transfer with DATA_REPETATION_COUNT");
        }
        return;
    }
    /* Stop command-pending timer */
    uwb_stop_quick_timer();

    uwb_cb.p_raw_cmd_cback = NULL;
    uwb_cb.uci_cmd_window++;
    uwb_cb.is_resp_pending = FALSE;
    uwb_cb.cmd_retry_count = 0; /* reset the retry count as response is received*/

    uwb_ucif_check_cmd_queue(NULL);
    UCI_TRACE_D("%s: exit", __FUNCTION__);
}

void uwb_ucif_cmd_timeout(void)
{
    UCI_TRACE_D("uwb_ucif_cmd_timeout");
    /**
     * Command timeout handling for MCTT (Conformance Test) mode
     *
     * In MCTT mode, command timeout handling differs from normal operation:
     * - Command timeout is NOT FiRA-specific, so MW (Middleware) does not retry
     * - If no response is received, notify upper layer immediately with UCI_TIMEOUT.
     *
     * Rationale for no retry in MCTT mode:
     * - There is a possibility that after sending a command, we may receive a
     *   retry notification (UCI_MSG_CORE_GENERIC_ERROR_NTF with UCI_STATUS_MESSAGE_RETRY)
     *   instead of the expected response
     * - If we perform a retry on timeout AND also handle the retry notification,
     *   this creates a double retry scenario
     * - Double retry can cause session duplication and other unintended side effects
     * - Therefore, in MCTT mode, we rely on the device's retry notification mechanism
     *   rather than implementing timeout-based retries at the middleware layer
     */
    if (uwb_cb.UwbOperatinMode == kOPERATION_MODE_mctt) {
        (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_TIMEOUT, 0, NULL);
        uwb_ucif_uwb_recovery();
    }

    else {
        if (uwb_cb.is_resp_pending && (uwb_cb.cmd_retry_count <= UCI_CMD_MAX_RETRY_COUNT)) {
            uwb_stop_quick_timer(); /*stop the pending timer */
            UCI_TRACE_W("Retrying last failed command");
            uwb_cb.cmd_retry_count++;
            uwb_ucif_retransmit_cmd(uwb_cb.pLast_cmd_buf);
        }
        else {
            UCI_TRACE_E("Command timeout failed for last retry command");
            (*uwa_dm_cb.p_dm_rsp_cback)(UCI_GID_INTERNAL, UCI_TIMEOUT, 0, NULL);
            uwb_ucif_uwb_recovery();
        }
    }
}

void uwb_ucif_retransmit_cmd(UWB_HDR *p_buf)
{
    UCI_TRACE_D("uwb_ucif_retransmit_cmd");
    if (p_buf == NULL) {
        UCI_TRACE_E("uwb_ucif_retransmit_cmd: p_data is NULL");
        return;
    }
    HAL_RE_WRITE(p_buf);
    /** Start UWB command-timeout timer
     * \note The timer is incremented by RETRY_RSP_TIMEOUT_OFFSET_MS for each retry.
     */
    uwb_start_quick_timer(uwb_cb.retry_rsp_timeout + (RETRY_RSP_TIMEOUT_OFFSET_MS * uwb_cb.cmd_retry_count));
    LOG_D("Retry Timeout : %d", uwb_cb.retry_rsp_timeout + (RETRY_RSP_TIMEOUT_OFFSET_MS * uwb_cb.cmd_retry_count));
}

void uwb_ucif_check_cmd_queue(UWB_HDR *p_buf)
{
    uint8_t *ps;
    uint8_t *pTemp;
// Chaining is enabled
#if UWBFTR_ChainedUCI
    uint8_t pbf;
#endif // UWBFTR_ChainedUCI
    // tUWB_CONN_CB* p_cb = NULL;
    UCI_TRACE_D("uwb_ucif_check_cmd_queue()");
    /* If Helios can accept another command, then send the next command */
    if (uwb_cb.uci_cmd_window > 0) {
        if (p_buf) {
            /* save the message header to double check the response */
            ps = (uint8_t *)(p_buf + 1) + p_buf->offset;
// Chaining is enabled
#if UWBFTR_ChainedUCI
            pbf = (*(ps)&UCI_PBF_MASK) >> UCI_PBF_SHIFT;
#endif // UWBFTR_ChainedUCI
            phOsalUwb_MemCopy(uwb_cb.last_hdr, ps, UWB_SAVED_HDR_SIZE);
            /* Copy the last command if the command has any payload */
            if (!(IS_DATA_SEND_PACKET(ps[0])) && ps[UCI_LENGTH_OFFSET] != 0x00) {
                phOsalUwb_MemCopy(uwb_cb.last_cmd, ps + UCI_MSG_HDR_SIZE, UWB_SAVED_HDR_SIZE);
            }
            /* copying command to temp buff for retransmission */
            uwb_cb.pLast_cmd_buf                 = (UWB_HDR *)last_cmd_buff;
            uwb_cb.pLast_cmd_buf->offset         = 0;
            uwb_cb.pLast_cmd_buf->layer_specific = p_buf->layer_specific;
            pTemp                                = (uint8_t *)(uwb_cb.pLast_cmd_buf + 1);
            uwb_cb.pLast_cmd_buf->len            = p_buf->len;
            phOsalUwb_MemCopy(pTemp, ps, p_buf->len);
            if (p_buf->layer_specific == UWB_WAIT_RSP_RAW_CMD) {
                /* save the callback for RAW VS */
                uwb_cb.p_raw_cmd_cback = ((tUWB_UCI_RAW_MSG *)p_buf)->p_cback;
                uwb_cb.rawCmdCbflag    = TRUE;
            }

            /* Indicate command is pending */
            uwb_cb.uci_cmd_window--;
            uwb_cb.is_resp_pending = TRUE;
            uwb_cb.cmd_retry_count = 0;

            /* send to HAL */
            HAL_RE_WRITE(p_buf);
// Chaining is enabled
#if UWBFTR_ChainedUCI
            /* start UWB command-timeout timer */
            if (pbf) { // if pbf bit is set for conformance test skip timer start.
                if (p_buf->layer_specific == UWB_WAIT_RSP_RAW_CMD) {
                    tUWB_RAW_CBACK *p_rsp_cback = uwb_cb.p_raw_cmd_cback;
                    if (p_rsp_cback == NULL) {
                        UCI_TRACE_E("p_raw_cmd_cback is null");
                    }
                    else {
                        (*p_rsp_cback)(0, (uint8_t)UWB_SEGMENT_PKT_SENT, 0, NULL);
                        uwb_cb.p_raw_cmd_cback = NULL;
                    }
                }
                uwb_cb.rawCmdCbflag = FALSE;
                uwb_cb.uci_cmd_window++;
                uwb_cb.is_resp_pending = FALSE;
                uwb_cb.cmd_retry_count = 0;
            }
            else
#endif // UWBFTR_ChainedUCI
            {
                /* start UWB command-timeout timer */
                /**
                 * No response for the Data transfer Command .
                 * timer will be used for the Response not for ntf
                 * hence disabling the timer in case of data transfer use cases.
                 *
                 * This applies to Conformance mode as well.
                 */
                if (IS_DATA_SEND_PACKET(ps[0]) && (uwb_cb.UwbOperatinMode == kOPERATION_MODE_mctt)) {
                    /* Do Not Start timer for MCTT mode with data transfer */
                    // For a Raw Command, update the cmd window to unblock the next command.
                    uwb_cb.uci_cmd_window++;
                }
                else {
                    uwb_cb.isCreditNtfReceived = false;
                    uwb_start_quick_timer(uwb_cb.uci_wait_rsp_tout);
                }
            }
            /* Free the memory alloted for command, once HAL_WRITE is done */
            phOsalUwb_FreeMemory(p_buf);
            p_buf = NULL;
        }
    }
}

void uwb_ucif_send_cmd(UWB_HDR *p_buf)
{
    UCI_TRACE_D("uwb_ucif_send_cmd.");
    if (p_buf == NULL) {
        UCI_TRACE_E("p_buf is NULL.");
        return;
    }
    /* post the p_buf to UCIT task */
    p_buf->event          = BT_EVT_TO_UWB_UCI;
    p_buf->layer_specific = 0;
    uwb_ucif_check_cmd_queue(p_buf);
}
#if (UWBFTR_DataTransfer && !(UWBIOT_UWBD_SR04X))

void chain_data_packet(UWB_HDR *p_msg)
{
    uint16_t payload_length = 0;
    static chained_uci_packet chained_packet;
    uint8_t pbf, dpf = 0;
    uint8_t *p, *pp;
    uint32_t pp_index = 0;

    p  = (uint8_t *)(p_msg + 1) + p_msg->offset;
    pp = p;
    UCI_MSG_PRS_PBF_GID(pp, pbf, dpf, pp_index);
    pp_index       = pp_index + 3;
    payload_length = p[NORMAL_MODE_LENGTH_OFFSET];
    payload_length = (uint16_t)((payload_length << EXTENDED_MODE_LEN_SHIFT) | p[EXTENDED_MODE_LEN_OFFSET]);

    if ((uwb_cb.UwbOperatinMode != kOPERATION_MODE_mctt) && pbf) { // No chaining required for conformance test mode
        if (!chained_packet.is_first_frgmnt_done) {
            chained_packet.gid = dpf;
            if (p_msg->len <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                phOsalUwb_MemCopy(&chained_packet.buffer[chained_packet.offset],
                    p,
                    p_msg->len); // Copy first fragment(uci packet with header)(p)
                chained_packet.offset               = p_msg->len;
                chained_packet.is_first_frgmnt_done = TRUE;
            }
            else {
                LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, p_msg->len);
                phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
            }
        }
        else {
            // if first fragment is copied, then copy only uci payload(pp) for subsequent fragments
            if (payload_length <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                UWB_STREAM_TO_ARRAY(&chained_packet.buffer[chained_packet.offset], pp, payload_length, pp_index);
                chained_packet.offset = (uint16_t)(chained_packet.offset + payload_length);
            }
            else {
                LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, payload_length);
                phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
            }
        }
    }
    else {
        if (chained_packet.is_first_frgmnt_done) {
            if (payload_length <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                // Append only payload to chained packet
                UWB_STREAM_TO_ARRAY(&chained_packet.buffer[chained_packet.offset], pp, payload_length, pp_index);
                chained_packet.offset = (uint16_t)(chained_packet.offset + payload_length);
            }
            else {
                LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, payload_length);
                phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
            }
        }
        else {
            // Copy first fragment(uci packet with header)(p)
            if (p_msg->len <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                phOsalUwb_MemCopy(&chained_packet.buffer[chained_packet.offset], p, p_msg->len);
                chained_packet.offset = p_msg->len;
            }
            else {
                LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, p_msg->len);
                phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
            }
        }

        // Update P & PP
        p              = &chained_packet.buffer[0]; // p -> points to complete UCI packet
        payload_length = (uint16_t)(chained_packet.offset - UCI_MSG_HDR_SIZE);
        pp             = p + 2;
        pp_index = 0;
        // p+2 length feild in hdr: Update overall payload length into the chained packet if payload > 255
        if (payload_length > 0xFF)
        {
            UWB_UINT16_TO_STREAM(pp, payload_length, pp_index);
        }

        pp = p + 4; // pp points to payload
        // Clear flags
        chained_packet.offset               = 0;
        chained_packet.is_first_frgmnt_done = FALSE;
        chained_packet.oid                  = 0xFF;
        chained_packet.gid                  = 0xFF;
    }
    (*uwa_dm_cb.p_dm_data_cback)(dpf, payload_length, pp);
}
#endif // (UWBFTR_DataTransfer && !(UWBIOT_UWBD_SR04X))

bool uwb_ucif_process_event(UWB_HDR *p_msg)
{
    uint8_t mt, pbf, gid, old_gid, old_oid = 0;
    uint8_t oid = 0;
    uint8_t *p, *pp, *p_old;
    bool free               = TRUE;
    uint16_t payload_length = 0;
    static chained_uci_packet chained_packet;
    uint8_t is_extended_length = 0;
    uint32_t pp_index             = 0;
    uint32_t index               = 0;

    if (p_msg != NULL) {
        p  = (uint8_t *)(p_msg + 1) + p_msg->offset;
        pp = p;
        UCI_MSG_PRS_HDR0(pp, mt, pbf, gid, pp_index);
#if UWBFTR_DataTransfer
#if !(UWBIOT_UWBD_SR04X)
        if (mt == UCI_MT_DATA) {
            chain_data_packet(p_msg);
            return (free);
        }
        else
#endif //!(UWBIOT_UWBD_SR04X)
#endif // UWBFTR_DataTransfer
        {
            UCI_MSG_PRS_HDR1(pp, oid, pp_index);
            pp_index           = pp_index + 2; // Skip payload fields
            is_extended_length = p[EXTND_LEN_INDICATOR_OFFSET] & EXTND_LEN_INDICATOR_OFFSET_MASK;
            payload_length     = p[NORMAL_MODE_LENGTH_OFFSET];
            if (is_extended_length) {
                payload_length = (uint16_t)((payload_length << EXTENDED_MODE_LEN_SHIFT) | p[EXTENDED_MODE_LEN_OFFSET]);
            }
// If chaining is disabled in MW but PBF bit is set, then throw error
#if !UWBFTR_ChainedUCI
            if ((pbf == 1) || (is_extended_length == 1)) {
                UCI_TRACE_E("uwb_ucif_process_event unexpected rsp: pbf:0x%x, ext len:0x%x", pbf, is_extended_length);
            }
#endif // UWBFTR_ChainedUCI

// Chaining is enabled
#if UWBFTR_ChainedUCI

            /**
             * @brief   Below logic will form the chaining NTF and Rsp(only for rawApi).
             *        ex:
             *           1: We will receive the notifications as chunks and pbf bit enabled(see UCI_MSG_PRS_PBF for
             * more). 2: Chunk the data till pbf bit disabled. 3: PBF bit Enable is indication of continuous NTF/Rsp of
             * same GID and OID. 4: PBF is Disable is indication of either last NTF/Rsp of same GID and OID or Rsp/NTF
             * is less than or equal to 0xFF.
             */
            if ((uwb_cb.UwbOperatinMode != kOPERATION_MODE_mctt) &&
                pbf) { // No chaining required for conformance test mode
                if (!chained_packet.is_first_frgmnt_done) {
                    chained_packet.oid = oid;
                    chained_packet.gid = gid;
                    if (p_msg->len <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                        phOsalUwb_MemCopy(&chained_packet.buffer[chained_packet.offset],
                            p,
                            p_msg->len); // Copy first fragment(uci packet with header)(p)
                        chained_packet.offset               = p_msg->len;
                        chained_packet.is_first_frgmnt_done = TRUE;
                    }
                    else {
                        LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, p_msg->len);
                        phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
                        return (free);
                    }
                }
                else {
                    // if first fragment is copied, then copy only uci payload(pp) for subsequent fragments
                    if ((chained_packet.oid == oid) && (chained_packet.gid == gid)) {
                        if (payload_length <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                            if ((uwb_cb.rawCmdCbflag == TRUE) && (mt != UCI_MT_NTF)) {
                                /**
                                 * @brief Copy buffer as is for raw callback.
                                 *        Update the offset as total message length +Header.
                                 */
                                phOsalUwb_MemCopy(&chained_packet.buffer[chained_packet.offset], p, p_msg->len);
                                chained_packet.offset = (uint16_t)(chained_packet.offset + p_msg->len);
                            }
                            else {
                                UWB_STREAM_TO_ARRAY(
                                    &chained_packet.buffer[chained_packet.offset], pp, payload_length, pp_index);
                                chained_packet.offset = (uint16_t)(chained_packet.offset + payload_length);
                            }
                        }
                        else {
                            LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, payload_length);
                            phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
                            return (free);
                        }
                    }
                    else {
                        UCI_TRACE_E(
                            "uwb_ucif_process_event: unexpected chain packet: chained_packed_gid: 0x%x, "
                            "chained_packet_oid=0x%x, received packet gid:0x%x, recived packet oid:0x%x",
                            chained_packet.gid,
                            chained_packet.oid,
                            gid,
                            oid);
                    }
                }
                return (free);
            }
            else
#endif // UWBFTR_ChainedUCI
            {
                if (chained_packet.is_first_frgmnt_done) {
                    if ((chained_packet.oid == oid) && (chained_packet.gid == gid)) {
                        if (payload_length <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                            if ((uwb_cb.rawCmdCbflag == TRUE) && (mt != UCI_MT_NTF)) {
                                /**
                                 * @brief Copy buffer as is for raw callback
                                 *        Update the offset as total message length +Header
                                 */
                                phOsalUwb_MemCopy(&chained_packet.buffer[chained_packet.offset], p, p_msg->len);
                                chained_packet.offset = (uint16_t)(chained_packet.offset + p_msg->len);
                            }
                            else {
                                UWB_STREAM_TO_ARRAY(
                                    &chained_packet.buffer[chained_packet.offset], pp, payload_length, pp_index);
                                chained_packet.offset = (uint16_t)(chained_packet.offset + payload_length);
                            }
                        }
                        else {
                            LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, payload_length);
                            phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
                            return (free);
                        }
                    }
                    else {
                        /* clean the buffer as chain packet is not completely received  */
                        LOG_W("Incomplete chained packet received");
                        phOsalUwb_SetMemory(chained_packet.buffer, 0x00, sizeof(chained_packet.buffer));
                        if (p_msg->len <= sizeof(chained_packet.buffer)) {
                            phOsalUwb_MemCopy(&chained_packet.buffer[0], p, p_msg->len);
                            chained_packet.offset = p_msg->len;
                        }
                    }
                }
                else {
                    // Copy first fragment(uci packet with header)(p)
                    if (p_msg->len <= (sizeof(chained_packet.buffer) - chained_packet.offset)) {
                        phOsalUwb_MemCopy(&chained_packet.buffer[chained_packet.offset], p, p_msg->len);
                        chained_packet.offset = p_msg->len;
                    }
                    else {
                        LOG_E("%s:%d Not enough buffer to store %d bytes", __FUNCTION__, __LINE__, p_msg->len);
                        phOsalUwb_SetMemory(&chained_packet, 0, sizeof(chained_packet));
                        return (free);
                    }
                }
                // Update P & PP
                p              = &chained_packet.buffer[0]; // p -> points to complete UCI packet
                payload_length = (uint16_t)(chained_packet.offset - UCI_MSG_HDR_SIZE);
                pp             = p + 2;
                pp_index = 0;
                // p+2 length feild in hdr: Update overall payload length into the chained packet if payload > 255
                if ((payload_length > 0xFF) && (mt != UCI_MT_RSP))
                {
                    /**
                     * For Raw command response dont update the length field.
                     * chunk the response as is to upper layers.
                     */
                    UWB_UINT16_TO_STREAM(pp, payload_length, pp_index);
                }
                pp = p + 4; // pp points to payload
                // Clear flags
                chained_packet.offset               = 0;
                chained_packet.is_first_frgmnt_done = FALSE;
                chained_packet.oid                  = 0xFF;
                chained_packet.gid                  = 0xFF;
            }
        }

        if ((uwb_cb.rawCmdCbflag == TRUE) && (mt != UCI_MT_NTF)) {
            LOG_MAU8_D("UCI-DEBUG ->", p, (uint16_t)(payload_length + UCI_MSG_HDR_SIZE));
            uci_proc_raw_cmd_rsp(p, (uint16_t)(payload_length + UCI_MSG_HDR_SIZE));
            uwb_cb.rawCmdCbflag = FALSE;
            return (free);
        }

        switch (mt) {
        case UCI_MT_RSP:
            UCI_TRACE_D("uwb_ucif_process_event: UWB received rsp gid:%d", gid);
            p_old = uwb_cb.last_hdr;
            UCI_MSG_PRS_PBF_GID(p_old, pbf, old_gid, index);
            UCI_MSG_PRS_HDR1(p_old, old_oid, index);
            /* make sure this is the RSP we are waiting for before updating the
             * command window */
            if ((old_gid != gid) || (old_oid != oid)) {
                UCI_TRACE_E("uwb_ucif_process_event unexpected rsp: gid:0x%x, oid:0x%x", gid, oid);
                return TRUE;
            }
            /**
             * This will notify upper layers via "ufaDeviceManagementRspCallback api"
             */
            (*uwa_dm_cb.p_dm_rsp_cback)(gid, oid, payload_length, pp);

            uwb_ucif_update_cmd_window();
            break;

        case UCI_MT_NTF:
            UCI_TRACE_D("uwb_ucif_process_event: UWB received ntf gid:%d", gid);
            if ((gid == UCI_GID_PROPRIETARY_CUSTOM_1) || (gid == UCI_GID_PROPRIETARY) ||
                (gid == UCI_GID_INTERNAL_GROUP) || (gid == UCI_GID_PROPRIETARY_CUSTOM_2)) {
                uci_proc_proprietary_ntf(gid, oid, p, (uint16_t)(payload_length + UCI_MSG_HDR_SIZE));
            }
            else {
#if (UWBIOT_UWBD_SR04X)
                if ((gid == UCI_GID_SESSION_MANAGE)
#else
                if ((gid == UCI_GID_RANGE_MANAGE )
#endif /* (UWBIOT_UWBD_SR04X) */
                && (oid == UCI_MSG_DATA_CREDIT_NTF)) {
                    if (pp[CREDIT_AVAILABILITY_INDICATOR_OFFSET]) {
                        /* In Case of Data transfer, Currently first a credit notification with status credit not
                         * available comes and then data transmit done notification, followed by a credit notification
                         * with credit available, update cmd window only if credit is available
                         */
                        UCI_TRACE_D("uwb_ucif_process_event: Credit is available");
                        uwb_ucif_update_cmd_window();
                        uwb_cb.isCreditNtfReceived = false;
                        (*uwa_dm_cb.p_dm_ntf_cback)(gid, oid, payload_length, pp);
                    }
                    else {
                        /* In case of credit notification with status credit not available
                         * Stop the retransmit command counter as response is received.
                         */
                        UCI_TRACE_D("uwb_ucif_process_event: Credit is not available");
                        uwb_stop_quick_timer();
                        uwb_cb.p_raw_cmd_cback     = NULL;
                        uwb_cb.is_resp_pending     = FALSE;
                        uwb_cb.cmd_retry_count     = 0; /* reset the retry count as response is received*/
                        uwb_cb.isCreditNtfReceived = true;
                        /**
                         * for Raw Api need to indicate the Credit ntf to application.
                         */
                        if (uwb_cb.rawCmdCbflag == TRUE) {
                            (*uwa_dm_cb.p_dm_ntf_cback)(gid, oid, payload_length, pp);
                        }
                    }
                }
                else if (gid == UCI_GID_CORE) {
                    if ((oid == UCI_MSG_CORE_DEVICE_STATUS_NTF) &&
                        (p[UCI_RESPONSE_STATUS_OFFSET] == UWBD_STATUS_HDP_WAKEUP ||
                            p[UCI_RESPONSE_STATUS_OFFSET] == UWBD_STATUS_ERROR)) {
                        /*After receiving FC or FF retry will happen only once to get the DEVICE_STATUS
                         * After that it will clear the retry & res_pending.
                         */
                        uwb_cb.rawCmdCbflag = FALSE;
                        uwb_ucif_uwb_recovery();
                        (*uwa_dm_cb.p_dm_ntf_cback)(gid, oid, payload_length, pp);
                    }
                    else if ((oid == UCI_MSG_CORE_GENERIC_ERROR_NTF) &&
                             (p[UCI_RESPONSE_STATUS_OFFSET] == UCI_STATUS_MESSAGE_RETRY)) {
                        uwb_stop_quick_timer(); /*stop the pending timer */
                        UCI_TRACE_W("Retrying last sent command");
                        uint8_t *pLast_buf;
                        pLast_buf = (uint8_t *)(uwb_cb.pLast_cmd_buf + 1) + uwb_cb.pLast_cmd_buf->offset;
                        if (IS_DATA_SEND_PACKET(*pLast_buf)) {
                            /* As part of data send if the host receive retry ntf then start timer to get
                             * credit ntf. If retry ntf comes after credit ntf. Skip retransmit the last data
                             * command If timer timeout retramsmit previous data msg.
                             */
                            if (uwb_cb.isCreditNtfReceived == false) {
                                UCI_TRACE_D("start timer to get credit ntf");
                                uwb_start_quick_timer(UWB_DATA_CREDIT_NTF_TIMEOUT);
                            }
                            else {
                                LOG_I("credit ntf already received, skip retransmit");
                            }
                        }
                        else {
                            uwb_ucif_retransmit_cmd(uwb_cb.pLast_cmd_buf);
                        }
                    }
                    else {
                        /** Except Device status ntf FC rest of the reason codes should be sent it to upper layers*/
                        (*uwa_dm_cb.p_dm_ntf_cback)(gid, oid, payload_length, pp);
                    }
                }
                else {
                    (*uwa_dm_cb.p_dm_ntf_cback)(gid, oid, payload_length, pp);
                }
            }
            break;
        default:
            UCI_TRACE_E("uwb_ucif_process_event: UWB received unknown mt:0x%x, gid:%d", mt, gid);
            break;
        }
    }
    else {
        UCI_TRACE_E("uwb_ucif_process_event: NULL pointer");
    }
    return (free);
}

void uwb_ucif_uwb_recovery(void)
{
    UCI_TRACE_D("uwb_ucif_uwb_recovery");
    if (uwb_cb.is_recovery_in_progress) {
        UCI_TRACE_D("uwb_ucif_uwb_recovery: recovery is already in progreess");
        return;
    }
    uwb_cb.cmd_retry_count         = 0;
    uwb_cb.is_resp_pending         = FALSE;
    uwb_cb.is_recovery_in_progress = TRUE;

    uwb_main_flush_cmd_queue();
    uwb_cb.is_recovery_in_progress = FALSE;
}
