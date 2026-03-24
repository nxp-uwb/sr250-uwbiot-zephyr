/*
 * Copyright 2012-2020, 2026 NXP.
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
#ifndef _PHNXPUCIHAL_EXT_H_
#define _PHNXPUCIHAL_EXT_H_

#include <phNxpUciHal.h>

/**
 * Function         phNxpUciHal_send_ext_cmd
 *
 * Description      This function send the extension command to UWBC. No
 *                  response is checked by this function but it waits for
 *                  the response to come.
 *
 * Returns          Returns UWBSTATUS_SUCCESS if sending cmd is successful and
 *                  response is received.
 *
 */
UWBSTATUS phNxpUciHal_send_ext_cmd(uint16_t cmd_len, uint8_t *p_cmd);
#if !(UWBIOT_UWBD_SR04X)

/**
 * Function         phNxpUciHal_dump_fw_crash_log
 *
 * Description      dump FW crash log when fw is crashed
 *
 * Returns          Returns UWBSTATUS_SUCCESS if fw crash log dumped
 *successfully
 *
 */
UWBSTATUS phNxpUciHal_dump_fw_crash_log();
#endif //!(UWBIOT_UWBD_SR04X)

/**
 * Function         phNxpUciHal_set_board_config
 *
 * Description      This function is called to set the board variant config
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the actual state of operation in arg pointer
 *
 */
UWBSTATUS phNxpUciHal_set_board_config();
#endif /* _PHNXPNICHAL_EXT_H_ */
