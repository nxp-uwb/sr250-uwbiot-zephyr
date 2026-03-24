/**
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2020,2022,2023 NXP
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
 *  This is the public interface file for UWA, NXP's UWB application
 *  layer for mobile phones/IOT devices.
 *
 */
#ifndef UWA_API_H
#define UWA_API_H
#include <stdint.h>

typedef uint8_t tUCI_STATUS;

#include "uci_defs.h"
#include "uwb_hal_api.h"
#include "uwb_target.h"

/**
 *  Constants and data types
 */

/** UCI Parameter IDs */
typedef uint8_t tUWA_PMID;

/** UWA_DM callback */
typedef void(tUWA_DM_RSP_CBACK)(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData);
typedef void(tUWA_DM_NTF_CBACK)(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData);
#if UWBFTR_DataTransfer
typedef void(tUWA_DM_DATA_CBACK)(uint8_t dpf, uint16_t len, uint8_t *eventData);
#endif // UWBFTR_DataTransfer
/** UWA_RAW_CMD callback */
typedef void(tUWA_RAW_CMD_CBACK)(uint8_t gid, uint8_t event, uint16_t param_len, uint8_t *p_param);

/**
 *  External Function Declarations
 */

/**
 *
 * Function         UWA_Init
 *
 * Description      This function initializes control blocks for UWA
 *
 *                  p_hal_entry_tbl points to a table of HAL entry points
 *
 *                  NOTE: the buffer that p_hal_entry_tbl points must be
 *                  persistent until UWA is disabled.
 *
 *
 * Returns          none
 *
 */
void UWA_Init(const tHAL_UWB_ENTRY *p_hal_entry_tbl);

/**
 *
 * Function         UWA_Enable
 *
 * Description      This function enables UWB. Prior to calling UWA_Enable,
 *                  the UWBC must be powered up, and ready to receive commands.
 *                  This function enables the tasks needed by UWB, opens the UCI
 *                  transport, resets the UWB controller, downloads patches to
 *                  the UWBC (if necessary), and initializes the UWB subsystems.
 *l
 *
 * Returns          UCI_STATUS_OK if successfully initiated
 *                  UCI_STATUS_FAILED otherwise
 *
 */
#if UWBFTR_DataTransfer
tUCI_STATUS UWA_Enable(
    tUWA_DM_RSP_CBACK *p_dm_rsp_cback, tUWA_DM_NTF_CBACK *p_dm_ntf_cback, tUWA_DM_DATA_CBACK *p_dm_data_cback);
#else
tUCI_STATUS UWA_Enable(tUWA_DM_RSP_CBACK *p_dm_rsp_cback, tUWA_DM_NTF_CBACK *p_dm_ntf_cback);
#endif // UWBFTR_DataTransfer

/**
 *
 * Function         UWA_Disable
 *
 * Description      This function is called to shutdown UWB. The tasks for UWB
 *                  are terminated, and clean up routines are performed. This
 *                  function is typically called during platform shut-down, or
 *                  when UWB is disabled from a settings UI. When the UWB
 *                  shutdown procedure is completed, an UWA_DM_DISABLE_EVT is
 *                  returned to the application using the tUWA_DM_CBACK.
 *
 * Returns          UCI_STATUS_OK if successfully initiated
 *                  UCI_STATUS_FAILED otherwise
 *
 */
tUCI_STATUS UWA_Disable(bool graceful);

/**
 *
 * Function         UWA_RegisterExtCallback
 *
 * Description      This function enables proprietary callbacks
 *l
 *
 * Returns          UCI_STATUS_OK if successfully initiated
 *                  UCI_STATUS_FAILED otherwise
 *
 */
tUCI_STATUS UWA_RegisterExtCallback(tUWA_RAW_CMD_CBACK *p_dm_cback);

/**
 * Function         UWA_SendUciCommand
 *
 * Description      Send Uci command
 *
 * Returns          UCI_STATUS_OK on success
 *                  UCI_STATUS_FAILED otherwise
 *
 */
tUCI_STATUS UWA_SendUciCommand(uint16_t event, uint16_t cmdLen, uint8_t *pCmd, uint8_t pbf);
/**
 *
 * Function         UWA_SendRawCommand
 *
 * Description      This function is called to send raw vendor specific
 *                  command to UWBS.
 *
 *                  cmd_params_len  - The command parameter len
 *                  p_cmd_params    - The command parameter
 *                  p_cback         - The callback function to receive the
 *                                    command
 *
 * Returns          UCI_STATUS_OK if successfully initiated
 *                  UCI_STATUS_FAILED otherwise
 *
 */
tUCI_STATUS UWA_SendRawCommand(uint16_t cmd_params_len, uint8_t *p_cmd_params, tUWA_RAW_CMD_CBACK *p_cback);

#endif /** UWA_API_H */
