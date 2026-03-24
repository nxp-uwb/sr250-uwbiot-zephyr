/*
 *
 * Copyright 2018-2020,2022-2023, 2026 NXP.
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

#ifndef UWA_DM_INT_H
#define UWA_DM_INT_H

#include "uwa_api.h"
#include "uwa_sys.h"

/**
 * Constants and data types
 */

/* UWA_DM flags */
/* DM is enabled                                                        */
#define UWA_DM_FLAGS_DM_IS_ACTIVE      0x00000001
#define SES_ID_AND_NO_OF_PARAMS_OFFSET 0x05

/* DM events */
typedef enum Command_Event
{
    /* device manager local device API events */
    UWA_DM_API_ENABLE_EVT = UWA_SYS_EVT_START(UWA_ID_DM),
    UWA_DM_API_DISABLE_EVT,
    UWA_DM_API_REGISTER_EXT_CB_EVT,
    UWA_DM_API_SEND_RAW_EVT,
    UWA_DM_INTERNAL_NUM_ACTIONS,
    UWA_DM_API_CORE_GET_DEVICE_INFO_EVT,
    UWA_DM_API_CORE_SET_CONFIG_EVT,
    UWA_DM_API_CORE_GET_CONFIG_EVT,
    UWA_DM_API_CORE_DEVICE_RESET_EVT,
    UWA_DM_API_SESSION_INIT_EVT,
    UWA_DM_API_SESSION_DEINIT_EVT,
    UWA_DM_API_SESSION_GET_COUNT_EVT,
    UWA_DM_API_SESSION_SET_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_GET_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_START_EVT,
    UWA_DM_API_SESSION_STOP_EVT,
    UWA_DM_API_SESSION_GET_STATE_EVT,
    UWA_DM_API_CORE_GET_CAPS_INFO_EVT,
    UWA_DM_API_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_EVT,
#if !(UWBIOT_UWBD_SR04X)
    UWA_DM_API_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_EVT,
    UWA_DM_API_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_EVT,
    UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_EVT,
    UWA_DM_API_SESSION_SET_HUS_CONTROLLER_CONFIG_EVT,
    UWA_DM_API_SESSION_SET_HUS_CONTROLEE_CONFIG_EVT,
    UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_EVT,
    /*    UWB RF Test API events   */
    UWA_DM_API_TEST_SET_CONFIG_EVT,
    UWA_DM_API_TEST_GET_CONFIG_EVT,
    UWA_DM_API_TEST_PERIODIC_TX_EVT,
    UWA_DM_API_TEST_PER_RX_EVT,
    UWA_DM_API_TEST_UWB_LOOPBACK_EVT,
    UWA_DM_API_TEST_RX_EVT,
    UWA_DM_API_TEST_SR_RX_EVT,
    UWA_DM_API_TEST_STOP_SESSION_EVT,
    /* Proprietary event */
    UWA_DM_API_PROP_CALIB_INTEGRITY_PROTECTION,
    UWA_DM_API_PROP_GENERATE_TAG,
    UWA_DM_API_PROP_VERIFY_CALIB_DATA,
    UWA_DM_API_PROP_DO_BIND,
    UWA_DM_API_PROP_GET_BINDING_COUNT,
    UWA_DM_API_PROP_TEST_CONNECTIVITY,
    UWA_DM_API_PROP_TEST_SE_LOOP,
    UWA_DM_API_PROP_GET_BINDING_STATUS,
    /* Vendor event */
    UWA_DM_API_VENDOR_DO_CHIP_CALIBRATION,
    UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION,
    UWA_DM_API_VENDOR_GET_DEVICE_CALIBRATION,
#endif /* !(UWBIOT_UWBD_SR04X) */
    UWA_DM_API_VENDOR_GET_ALL_UWB_SESSIONS,
#if !(UWBIOT_UWBD_SR04X)
    UWA_DM_API_PROP_QUERY_TEMP,
    UWA_DM_API_PROP_CONFIG_AUTH_TAG_OPTIONS,
    UWA_DM_API_PROP_CONFIG_AUTH_TAG_VERSIONS,
    UWA_DM_API_PROP_URSK_DELETION_REQUEST,
    UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT,
    UWA_DM_API_SESSION_SET_LOCALIZATION_ZONE_EVT,
#endif /* !(UWBIOT_UWBD_SR04X) */
    UWA_DM_API_CORE_QUERY_UWBS_TIMESTAMP,
#if !(UWBIOT_UWBD_SR04X)
    UWA_DM_API_PROP_WRITE_OTP_CALIB_DATA,
    UWA_DM_API_PROP_READ_OTP_CALIB_DATA,
#endif // !(UWBIOT_UWBD_SR04X)
    UWA_DM_API_TRNG_EVENT,
#if (UWBIOT_UWBD_SR04X)
    UWA_DM_API_SUSPEND_DEVICE_EVENT,
    UWA_DM_API_SESSION_NVM_EVENT,
    UWA_DM_START_TEST_MODE_EVENT,
    UWA_DM_STOP_TEST_MODE_EVENT,
    UWA_DM_SET_CALIB_TRIM_EVENT,
    UWA_DM_GET_CALIB_TRIM_EVENT,
    UWA_MHR_IN_CCM,
    UWA_BYPASS_CURRENT_LIMITER,
#endif /* UWBIOT_UWBD_SR04X */
#if (UWBFTR_BlobParser)
    UWA_DM_API_PROFILE_PARAM_EVENT,
#endif // (UWBFTR_BlobParser)
    UWA_DM_API_SEND_DATA_EVENT,
    UWA_DM_API_LOGICAL_LINK_SEND_DATA_EVENT,
#if UWBFTR_DataTransfer
    UWA_DM_SESSION_LOGICAL_LINK_CREATE_EVT,
    UWA_DM_SESSION_LOGICAL_LINK_CLOSE_EVT,
    UWA_DM_SESSION_LOGICAL_LINK_GET_PARAM_EVT,
#endif // UWBFTR_DataTransfer

#if UWBIOT_UWBD_SR2XXT
    UWA_DM_API_VENDOR_SET_SECURE_CALIBRATION,
#endif // UWBIOT_UWBD_SR2XXT

#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250
    UWA_DM_API_WRITE_MODULE_MAKER_EVT,
    UWA_DM_API_READ_MODULE_MAKER_EVT,
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250

    UWA_DM_API_GET_ESE_SESSION_ID_LIST_EVT,
    UWA_DM_MAX_EVT,
} eCommand_Event_t;

/* data type for UWA_DM_API_ENABLE_EVT */
typedef struct
{
    UWB_HDR hdr;
    tUWA_DM_RSP_CBACK *p_dm_rsp_cback;
    tUWA_DM_NTF_CBACK *p_dm_ntf_cback;
#if UWBFTR_DataTransfer
    tUWA_DM_DATA_CBACK *p_dm_data_cback;
#endif // UWBFTR_DataTransfer
} tUWA_DM_API_ENABLE;

/* data type for UWA_DM_API_DISABLE_EVT */
typedef struct
{
    UWB_HDR hdr;
    bool graceful;
} tUWA_DM_API_DISABLE;

/* data type for UWA_DM_API_SEND_RAW_EVT */
typedef struct
{
    UWB_HDR hdr;
    tUWA_RAW_CMD_CBACK *p_cback;
    uint8_t oid;
    uint16_t cmd_params_len;
    uint8_t *p_cmd_params;
} tUWA_DM_API_SEND_RAW;

typedef struct
{
    UWB_HDR hdr;
    tUWA_RAW_CMD_CBACK *p_dm_ext_cback;
} tUWA_DM_API_REGISTER_EXT_CB;

typedef struct
{
    UWB_HDR hdr;
    uint16_t length;
    uint8_t *p_data;
    uint8_t pbf;
} tUCI_CMD;

/* union of all data types */
typedef union {
    /* GKI event buffer header */
    UWB_HDR hdr;
    tUWA_DM_API_ENABLE enable;          /* UWA_DM_API_ENABLE_EVT           */
    tUWA_DM_API_DISABLE disable;        /* UWA_DM_API_DISABLE_EVT          */
    tUWA_DM_API_REGISTER_EXT_CB ext_cb; /* UWA_DM_API_REGISTER_EXT_CB_EVT  */
    tUWA_DM_API_SEND_RAW send_raw;      /* UWA_DM_API_SEND_RAW_EVT         */
    tUCI_CMD sUci_cmd;
} tUWA_DM_MSG;

typedef struct
{
    uint32_t flags;                    /* UWA_DM flags (see definitions for UWA_DM_FLAGS_*)    */
    tUWA_DM_RSP_CBACK *p_dm_rsp_cback; /* UWA DM callback for response */
    tUWA_DM_NTF_CBACK *p_dm_ntf_cback; /* UWA DM callback for ntf */
#if UWBFTR_DataTransfer
    tUWA_DM_DATA_CBACK *p_dm_data_cback; /* UWA DM callback for data */
#endif                                   // UWBFTR_DataTransfer
} tUWA_DM_CB;

/**
**
** Function         uwa_dm_disable_complete
**
** Description      Called when all UWA subsytems are disabled.
**
**                  UWB core stack can now be disabled.
**
** Returns          void
**
*/
void uwa_dm_disable_complete(void);

/* UWA device manager control block */
extern tUWA_DM_CB uwa_dm_cb;

/**
** Function         uwa_dm_init
**
** Description      Initialises the UWB device manager
**
** Returns          void
**
*/
void uwa_dm_init(void);

/* Action function prototypes */

/**
**
** Function         uwa_dm_enable
**
** Description      Initialises the UWB device manager
**
** Returns          TRUE (message buffer to be freed by caller)
**
*/
bool uwa_dm_enable(tUWA_DM_MSG *p_data);

/**
**
** Function         uwa_dm_disable
**
** Description      Disables the UWB device manager
**
** Returns          TRUE (message buffer to be freed by caller)
**
*/
bool uwa_dm_disable(tUWA_DM_MSG *p_data);

/**
**
** Function         uwa_dm_register_ext_cb
**
** Description      Initialises the UWB device manager
**
** Returns          TRUE (message buffer to be freed by caller)
**
*/
bool uwa_dm_register_ext_cb(tUWA_DM_MSG *p_data);

/**
**
** Function         uwa_dm_act_send_raw_cmd
**
** Description      Send raw command to Queue
**
** Returns          FALSE (message buffer is NOT freed by caller)
**
*/
bool uwa_dm_act_send_raw_cmd(tUWA_DM_MSG *p_data);

/* Main function prototypes */

/**
**
** Function         uwa_dm_evt_hdlr
**
** Description      Event handling function for DM
**
**
** Returns          void
**
*/
bool uwa_dm_evt_hdlr(UWB_HDR *p_msg);

/**
**
** Function         uwa_dm_sys_disable
**
** Description      This function is called after all subsystems have been
**                  disabled.
**
** Returns          void
**
*/
void uwa_dm_sys_disable(void);

#endif /* UWA_DM_INT_H */
