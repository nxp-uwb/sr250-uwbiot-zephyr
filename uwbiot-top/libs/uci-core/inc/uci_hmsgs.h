/**
 *
 * Copyright 2018-2020,2022,2023 NXP.
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

/**
 *
 *  defines UCI interface messages (for DH)
 *
 */
#ifndef UWB_UCI_HMSGS_H
#define UWB_UCI_HMSGS_H

#include "uwb_types.h"
#include "uci_defs.h"

#include <stdbool.h>

/**
 *
 * Function         uci_snd_cmd_interface
 *
 * Description     Interface that internally calls UCI SEND command
 *
 * Returns         True in order to free the allocated memory in event handler
 *
 */
bool uci_snd_cmd_interface(uint16_t eventId, uint16_t length, uint8_t *data, uint8_t pbf);

/**
 *
 * Function         uci_snd_cmd
 *
 * Description      compose and send command to command queue
 *
 * Returns          status
 *
 */
uint8_t uci_snd_cmd(uint16_t eventId, uint16_t length, uint8_t *data, uint8_t pbf);

/**
 *
 * Function         uci_proc_raw_cmd_rsp
 *
 * Description      Process the Responses for Raw command and Notify upper layers via (rawCommandResponse_Cb) callback
 *
 * Returns          void
 *
 */
extern void uci_proc_raw_cmd_rsp(uint8_t *p_buf, uint16_t len);

/**
 *
 * Function         uci_proc_proprietary_ntf
 *
 * Description      Process the UCI Prop Notification and Notify upper layers via (extDeviceManagementCallback) callback
 *
 * Returns          void
 *
 */
extern void uci_proc_proprietary_ntf(uint8_t gid, uint8_t op_code, uint8_t *p_buf, uint16_t len);
#endif /** UWB_UCI_MSGS_H */
