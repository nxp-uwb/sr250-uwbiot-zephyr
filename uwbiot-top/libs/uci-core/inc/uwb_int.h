/*
 *
 * Copyright 2018-2020,2022-2026 NXP.
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

#ifndef UWB_INT_H_
#define UWB_INT_H_

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "phUwb_BuildConfig.h"
#include "uwb_target.h"
#include "UwbAdaptation.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "uwb_types.h"
#include "uwb_hal_api.h"
#include "uwa_api.h"
#include "uci_test_defs.h"

#define UWB_SEGMENT_PKT_SENT 0xFE
typedef uint8_t tUWB_RAW_EVT; /* proprietary events */

/**
 *  RESPONSE Callback Functions
 */
typedef void(tUWB_RAW_CBACK)(uint8_t gid, tUWB_RAW_EVT event, uint16_t data_len, uint8_t *p_data);

/**
** UWB Internal definitions
*/
/**GID: Proprietary group SHIFT */
#define UCI_GID_GROUP_SHIFT 0x08

/** Helper Macro to fetch Particular GID and OID for Core Group - (0x00) */
#define GET_CORE_GROUP_GID_OID(CORE_OID) ((UCI_GID_CORE << UCI_GID_GROUP_SHIFT) | (CORE_OID))

/** Helper Macro to fetch Particular GID and OID for Session Config Group - (0x01) */
#define GET_SESSION_CONFIG_GROUP_GID_OID(SESSION_CONFIG_OID) \
    ((UCI_GID_SESSION_MANAGE << UCI_GID_GROUP_SHIFT) | (SESSION_CONFIG_OID))

/** Helper Macro to fetch Particular GID and OID for Session Control Group - (0x02)  */
#define GET_SESSION_CONTROL_GROUP_GID_OID(SESSION_CONTROL_OID) \
    ((UCI_GID_RANGE_MANAGE << UCI_GID_GROUP_SHIFT) | (SESSION_CONTROL_OID))

/** Helper Macro to fetch Particular GID and OID for Proprietary SE Group  - (0x0A) */
#define GET_PROPRIETARY_SE_GROUP_GID_OID(PROPRIETARY_SE_OID) \
    ((UCI_GID_PROPRIETARY << UCI_GID_GROUP_SHIFT) | (PROPRIETARY_SE_OID))

// UCI_GID_INTERNAL_GROUP

/** Helper Macro to fetch Particular GID and OID for NXP Internal Group  - (0x0B) */
#define GET_INTERNAL_GROUP_GID_OID(INTERNAL_OID) ((UCI_GID_INTERNAL_GROUP << UCI_GID_GROUP_SHIFT) | (INTERNAL_OID))

/** Helper Macro to fetch Particular GID and OID for Test Group  - (0x0D) */
#define GET_TEST_GROUP_GID_OID(TEST_OID) ((UCI_GID_TEST << UCI_GID_GROUP_SHIFT) | (TEST_OID))

/** Helper Macro to fetch Particular GID and OID for Proprietary Group  - (0x0E)*/
#define GET_PROP_GROUP_GID_OID(PROP_OID) ((UCI_GID_PROPRIETARY_CUSTOM_1 << UCI_GID_GROUP_SHIFT) | (PROP_OID))

/** Helper Macro to fetch Particular GID and OID for Vendor Group - (0x0F)*/
#define GET_VENDOR_GROUP_GID_OID(VENDOR_OID) ((UCI_GID_PROPRIETARY_CUSTOM_2 << UCI_GID_GROUP_SHIFT) | (VENDOR_OID))

/** Helper Macro to fetch Particular GID and OID for MW Internal DM Group - (0x1F)*/
#define GET_INTERNAL_DM_GROUP_GID_OID(INTERNAL_DM_OID) ((UCI_GID_INTERNAL << UCI_GID_GROUP_SHIFT) | (INTERNAL_DM_OID))


/****************************************************************************
** UWB_TASK definitions
****************************************************************************/

/* UWB_TASK event masks */
#define UWB_TASK_EVT_TRANSPORT_READY UWB_EVENT_MASK(APPL_EVT_0)

/* UWB Timer events */
#define UWB_TTYPE_UCI_WAIT_RSP 0x00
#define UWB_WAIT_RSP_RAW_CMD   0x01

#define UWB_SAVED_HDR_SIZE 2

/* UWB Task event messages */
enum
{
    UWB_STATE_NONE,         /* not start up yet                         */
    UWB_STATE_W4_HAL_OPEN,  /* waiting for HAL_UWB_OPEN_CPLT_EVT   */
    UWB_STATE_IDLE,         /* normal operation( device is in idle state) */
    UWB_STATE_ACTIVE,       /* UWB device is in active                    */
    UWB_STATE_W4_HAL_CLOSE, /* waiting for HAL_UWB_CLOSE_CPLT_EVT  */
    UWB_STATE_CLOSING
};
typedef uint8_t tUWB_STATE;

/* This data type is for UWB task to send a UCI VS command to UCIT task */
typedef struct
{
    UWB_HDR bt_hdr;          /* the UCI command          */
    tUWB_RAW_CBACK *p_cback; /* the callback function to receive RSP   */
} tUWB_UCI_RAW_MSG;

/* This data type is for HAL event */
typedef struct
{
    UWB_HDR hdr;
    uint8_t hal_evt; /* HAL event code  */
    uint8_t status;  /* tHAL_UWB_STATUS */
} tUWB_HAL_EVT_MSG;

/* callback function pointer(8; use 8 to be safe + UWB_SAVED_CMD_SIZE(2) */
#define UWB_RECEIVE_MSGS_OFFSET (10)

/* UWB control blocks */
typedef struct
{
    tUWB_RAW_CBACK *p_ext_resp_cback;

    tUWB_STATE uwb_state;

    uint8_t last_hdr[UWB_SAVED_HDR_SIZE]; /* part of last UCI command header */
    uint8_t last_cmd[UWB_SAVED_HDR_SIZE]; /* part of last UCI command payload */

    tUWB_RAW_CBACK *p_raw_cmd_cback; /* the callback function for last raw command */

    uint16_t uci_wait_rsp_tout;   /* UCI command timeout (in ms) */
    uint16_t retry_rsp_timeout;   /* UCI command timeout during retry */
    uint8_t uci_cmd_window;       /* Number of commands the controller can accecpt
                             without waiting for response. */
    bool is_resp_pending;         /* response is pending from UWBS */
    bool is_recovery_in_progress; /* recovery in progresss  */

    const tHAL_UWB_ENTRY *p_hal;
    uint8_t rawCmdCbflag;
    uint8_t device_state;
    uint16_t cmd_retry_count;
    UWB_HDR *pLast_cmd_buf;
    uint8_t UwbOperatinMode;
    bool isCreditNtfReceived;
} tUWB_CB;

/* UWB Task Control structure */
typedef struct Uwbtask_Control
{
    UWBOSAL_TASK_HANDLE task_handle;
    intptr_t pMsgQHandle;
    void *uwb_task_sem;
} phUwbtask_Control_t;

/** \addtogroup uwb_status
 * @{ */
/**
 * \brief Session status error reason code
 * 0x06 - 0x1C : RFU
 * 0x80 - 0xFF : Vendor Specific Codes
 * 0xA0 - 0xAF : CCC Specific Codes
 */
typedef enum
{
    UWB_SESSION_STATE_CHANGED                                               = 0x00,
    UWB_SESSION_MAX_RR_RETRY_COUNT_REACHED                                  = 0x01,
    UWB_SESSION_MAX_RANGING_BLOCKS_REACHED                                  = 0x02,
    UWB_SESSION_SUSPENDED_DUE_TO_INBAND_SIGNAL                              = 0x03,
    UWB_SESSION_RESUMED_DUE_TO_INBAND_SIGNAL                                = 0x04,
    UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL                                = 0x05,
    UWB_SESSION_INVALID_UL_TDOA_RANDOM_WINDOW                               = 0x1D,
    UWB_SESSION_MIN_RFRAMES_PER_RR_NOT_SUPPORTED                            = 0x1E,
    UWB_SESSION_INTER_FRAME_INTERVAL_NOT_SUPPORTED                          = 0x1F,
    UWB_SESSION_SLOT_LENTGH_NOT_SUPPORTED                                   = 0x20,
    UWB_SESSION_SLOTS_PER_RR_NOT_SUFFICIENT                                 = 0x21,
    UWB_SESSION_MAC_ADDRESS_MODE_NOT_SUPPORTED                              = 0x22,
    UWB_SESSION_INVALID_RANGING_DURATION                                    = 0x23,
    UWB_SESSION_INVALID_STS_CONFIG                                          = 0x24,
    UWB_SESSION_HUS_INVALID_RFRAME_CONFIG                                   = 0x25,
    UWB_SESSION_HUS_NOT_ENOUGH_SLOTS                                        = 0x26,
    UWB_SESSION_HUS_CFP_PHASE_TOO_SHORT                                     = 0x27,
    UWB_SESSION_HUS_CAP_PHASE_TOO_SHORT                                     = 0x28,
    UWB_SESSION_HUS_OTHERS                                                  = 0x29,
    UWB_SESSION_STATUS_SESSION_KEY_NOT_FOUND                                = 0x2A,
    UWB_SESSION_STATUS_SUB_SESSION_KEY_NOT_FOUND                            = 0x2B,
    UWB_SESSION_INVALID_PREAMBLE_CODE_INDEX                                 = 0x2C,
    UWB_SESSION_INVALID_SFD_ID                                              = 0x2D,
    UWB_SESSION_INVALID_PSDU_DATA_RATE                                      = 0x2E,
    UWB_SESSION_INVALID_PHR_DATA_RATE                                       = 0x2F,
    UWB_SESSION_INVALID_PREAMBLE_DURATION                                   = 0x30,
    UWB_SESSION_INVALID_STS_LENGTH                                          = 0x31,
    UWB_SESSION_INVALID_NUM_OF_STS_SEGMENTS                                 = 0x32,
    UWB_SESSION_INVALID_NUM_OF_CONTROLEES                                   = 0x33,
    UWB_SESSION_MAX_RANGING_REPLY_TIME_EXCEEDED                             = 0x34,
    UWB_SESSION_INVALID_DST_ADDRESS_LIST                                    = 0x35,
    UWB_SESSION_INVALID_OR_NOT_FOUND_SUB_SESSION_ID                         = 0x36,
    UWB_SESSION_INVALID_RESULT_REPORT_CONFIG                                = 0x37,
    UWB_SESSION_INVALID_RANGING_ROUND_CONTROL_CONFIG                        = 0x38,
    UWB_SESSION_INVALID_RANGING_ROUND_USAGE                                 = 0x39,
    UWB_SESSION_INVALID_MULTI_NODE_MODE                                     = 0x3A,
    UWB_SESSION_RDS_FETCH_FAILURE                                           = 0x3B,
    UWB_SESSION_DOES_NOT_EXIST                                              = 0x3C,
    UWB_SESSION_RANGING_DURATION_MISMATCH                                   = 0x3D,
    UWB_SESSION_INVALID_OFFSET_TIME                                         = 0x3E,
    UWB_SESSION_LOST                                                        = 0x3F,
    UWB_SESSION_DT_ANCHOR_RANGING_ROUNDS_NOT_CONFIGURED                     = 0x40,
    UWB_SESSION_DT_TAG_RANGING_ROUNDS_NOT_CONFIGURED                        = 0x41,
    UWB_SESSION_ERROR_HUS_INVALID_SLOT_DURATION                             = 0x42,
    UWB_SESSION_ERROR_INVALID_ANTENNA_CFG                                   = 0x80,
    UWB_SESSION_ERROR_INVALID_RX_MODE                                       = 0x81,
    UWB_SESSION_ERROR_FAIL_DYNAMIC_STS_NOT_ALLOWED                          = 0x82,
    UWB_SESSION_ERROR_FEATURE_NOT_SUPPORTED_FOR_MODEL                       = 0x83,
    UWB_SESSION_ERROR_RX_TOA_MODE_MISMATCH                                  = 0x84,
    UWB_SESSION_ERROR_INSUFFICIANT_MEMORY_FOR_INBAND_DATA                   = 0x85,
    UWB_SESSION_ERROR_INVALID_DATA_TRANSFER_MODE                            = 0x86,
    UWB_SESSION_ERROR_INVALID_MAC_CFG                                       = 0x87,
    UWB_SESSION_ERROR_ANTENNA_DEFINES_NOT_CONFIGURED                        = 0x88,
    UWB_SESSION_ERROR_INVALID_MAX_TDOA_SESSION_COUNT_REACHED                = 0x89,
    UWB_SESSION_ERROR_LOOPBACK_TX_POWER_TOO_HIGH                            = 0x8A,
    UWB_SESSION_ERROR_WRONG_SESSION_TYPE_FOR_INBAND_DATA                    = 0x8B,
    UWB_SESSION_ERROR_AOA_NOT_SUPPORTED_IN_SINGLE_RX                        = 0x8C,
    UWB_SESSION_ERROR_DUPLICATE_DST_MAC_ADDRESS_DETECTED                    = 0x8D,
    UWB_SESSION_ERROR_INVALID_ADAPTIVE_HOPPING_THRESHOLD                    = 0x8E,
    UWB_SESSION_ERROR_UNSUPPORTED_RANGING_LIMIT                             = 0x8F,
    UWB_SESSION_ERROR_RNG_INVALID_DEVICE_ROLE                               = 0x91,
    UWB_SESSION_ERROR_KEY_ROTATION_NOT_SUPPORTED                            = 0x92,
    UWB_SESSION_ERROR_TEST_KDF_NOT_SUPPORTED                                = 0x93,
    UWB_SESSION_ERROR_INVALID_ANTENNA_PAIR_SWAP_CONFIGURATION               = 0x94,
    UWB_SESSION_ERROR_INVALID_CHANNEL_ID                                    = 0x98,
    UWB_SESSION_STOPPED_DUE_TO_QOS_DECISION                                 = 0x9F,
    UWB_SESSION_URSK_EXPIRED                                                = 0xA0,
    UWB_SESSION_ERROR_URSK_TTL_MAX_VALUE_REACHED                            = 0xA1,
    UWB_SESSION_ERROR_CCC_TERMINATION_ON_ON_MAX_STS_INDEX                   = 0xA2,
    UWB_SESSION_ERROR_RADAR_CIR_MAX_TAP_IDX_EXCEEDED                        = 0xB0,
    UWB_SESSION_ERROR_RADAR_ANTENNA_CONFIG_RX_NOT_OK                        = 0xB1,
    UWB_SESSION_ERROR_RADAR_PRESENCE_DETECTION_RANGE_EXCEEDED               = 0xB2,
    UWB_SESSION_ERROR_RADAR_RX_GAIN_INDEX_NOT_OK                            = 0xB3,
    UWB_SESSION_ERROR_RADAR_DRIFTCOMP_ANTENNA_CONFIG_NOT_OK                 = 0xB4,
    UWB_SESSION_ERROR_DATA_NOT_PRESENT                                      = 0xB5,
    UWB_SESSION_RADAR_FCC_LIMIT_REACHED                                     = 0xB7,
    UWB_SESSION_ERROR_TEST_MMS_FRAME_NOT_SUPPORTED                          = 0xB8,
    UWB_SESSION_ERROR_RADAR_INVALID_INTERLEAVE_MODE                         = 0xB9,
    UWB_SESSION_ERROR_CSA_INVALID_CFG                                       = 0xC0,
    UWB_SESSION_ERROR_INVALID_RESPONDER_SLOT_INDEX_CONFIGURED               = 0xC1,
    UWB_SESSION_ERROR_INVALID_INITIAL_SYNC_RX_WINDOW_CONFIG_CONFIGURED      = 0xC9,
    UWB_SESSION_ERROR_INITIAL_SYNC_RX_WINDOW_INITIATION_TIME_NOT_CONFIGURED = 0xCA,
} eSESSION_STATUS_REASON_CODES_t;
/** @} */ /* @addtogroup uwb_status */

/* As same value is used to indicate Session Stopped due to MAX STS */
#define SESSION_STOPPED_DUE_TO_MAX_STS UWB_SESSION_ERROR_CCC_TERMINATION_ON_ON_MAX_STS_INDEX

// TODO: eSESSION_STATUS_REASON_CODES_t needs to be updated.
#define UWB_SESSION_HUS_NOT_ENOUGH_SLOTS UWB_SESSION_HUS_CFP_PHASE_TOO_SHORT

typedef enum Gid_Oid_Type
{
    /* UCI Core Group - (0x00)*/
    kGidOid_CoreDeviceReset        = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_DEVICE_RESET),
    kGidOid_CoreGetDeviceStatus    = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_DEVICE_STATUS_NTF),
    kGidOid_CoreGetDeviceInfo      = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_DEVICE_INFO),
    kGidOid_CoreGetCapsInfo        = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_GET_CAPS_INFO),
    kGidOid_CoreSetConfig          = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_SET_CONFIG),
    kGidOid_CoreGetConfig          = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_GET_CONFIG),
    kGidOid_CoreDeviceSuspend      = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_DEVICE_SUSPEND),
    kGidOid_CoreGenricErrorNtf     = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_GENERIC_ERROR_NTF),
    kGidOid_CoreQueryUwbsTimestamp = GET_CORE_GROUP_GID_OID(UCI_MSG_CORE_QUERY_UWBS_TIMESTAMP),

    /* UWB Session Config Group - (0x01)*/
    kGidOid_SessionInit         = GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_INIT),
    kGidOid_SessionDeinit       = GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_DEINIT),
    kGidOid_SessionStatus       = GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_STATUS_NTF),
    kGidOid_SessionSetAppConfig = GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_SET_APP_CONFIG),
    kGidOid_SessionGetAppConfig = GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_GET_APP_CONFIG),
    kGidOid_SessionGetCount     = GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_GET_COUNT),
    kGidOid_SessionGetState     = GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_GET_STATE),
    kGidOid_SessionUpdateControllerMulticastList =
        GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST),
    kGidOid_SessionQueryDataSizeInRanging =
        GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_QUERY_DATA_SIZE_IN_RANGING),
    kGidOid_SessionSetHusControllerConfig =
        GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_SET_HUS_CONTROLLER_CONFIG_CMD),
    kGidOid_SessionSetHusControleeConfig =
        GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_SET_HUS_CONTROLEE_CONFIG_CMD),
    kGidOid_SessionDataTransferPhaseConfig =
        GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_SESSION_DATA_TRANSFER_PHASE_CONFIG),

    /* UWB Session Control Group - (0x02)*/
    kGidOid_SessionStart             = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_RANGE_START),
    kGidOid_SessionStop              = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_RANGE_STOP),
    kGidOid_SessionCtrlReq           = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_RANGE_CTRL_REQ),
    kGidOid_SessionGetRangingCount   = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_RANGE_GET_RANGING_COUNT),
    kGidOid_SessionBlinkDataTx       = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_RANGE_BLINK_DATA_TX),
    kGidOid_SessionInfoNtf           = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_SESSION_INFO_NTF),
    kGidOid_SessionDataCreditNtf     = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_DATA_CREDIT_NTF),
    kGidOid_SessionTransmitStatusNtf = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_DATA_TRANSMIT_STATUS_NTF),
    kGidOid_SessionCccDataNtf        = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_RANGE_CCC_DATA_NTF),
    kGidOid_SessionLlCreate          = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_LOGICAL_LINK_CREATE),
    kGidOid_SessionLlClose           = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_LOGICAL_LINK_CLOSE),
    kGidOid_SessionLlUwbsClose       = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_LOGICAL_LINK_UWBS_CLOSE),
    kGidOid_SessionLlUwbsCreate      = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_LOGICAL_LINK_UWBS_CREATE),
    kGidOid_SessionLlGetParams       = GET_SESSION_CONTROL_GROUP_GID_OID(UCI_MSG_LOGICAL_LINK_GET_PARAM),

    /* Test Group - (0x0D)*/
    kGidOid_TestSetConfig   = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_SET_CONFIG),
    kGidOid_TestGetConfig   = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_GET_CONFIG),
    kGidOid_TestPeriodicTx  = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_PERIODIC_TX),
    kGidOid_TestPerRx       = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_PER_RX),
    kGidOid_TestRx          = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_RX),
    kGidOid_TestSrRx        = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_SR_RX),
    kGidOid_TestUwbLoopback = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_LOOPBACK),
    kGidOid_TestStopSession = GET_TEST_GROUP_GID_OID(UCI_MSG_TEST_STOP_SESSION),

#if !(UWBIOT_UWBD_SR04X)
    kGidOid_SessionUpdateDtAnchorRangingRound =
        GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_ANCHOR_DEVICE),
    kGidOid_SessionUpdateDtTagRangingRound =
        GET_SESSION_CONFIG_GROUP_GID_OID(UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_RECEIVER_DEVICE),

    /* Proprietary SE Group*/
    kGidOid_WriteOtpCalibData = GET_PROPRIETARY_SE_GROUP_GID_OID(EXT_UCI_MSG_WRITE_CALIB_DATA_CMD),
    kGidOid_ReadOtpCalibData  = GET_PROPRIETARY_SE_GROUP_GID_OID(EXT_UCI_MSG_READ_CALIB_DATA_CMD),
#if UWBFTR_CSA
    kGidOid_SetLocZone = GET_PROPRIETARY_SE_GROUP_GID_OID(EXT_UCI_MSG_SESSION_SET_LOCALIZATION_ZONE_CMD),
#endif // UWBFTR_CSA
    kGidOid_RangingTimestampNtf = GET_INTERNAL_GROUP_GID_OID(VENDOR_UCI_MSG_RANGING_TIMESTAMP_NTF),
    kGidOid_RframeLogNtf        = GET_INTERNAL_GROUP_GID_OID(VENDOR_UCI_MSG_DBG_RFRAME_LOG_NTF),
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    kGidOid_WriteModuleMaker = GET_PROPRIETARY_SE_GROUP_GID_OID(EXT_UCI_MSG_WRITE_MODULE_MAKER_ID),
    kGidOid_ReadModuleMaker  = GET_PROPRIETARY_SE_GROUP_GID_OID(EXT_UCI_MSG_READ_MODULE_MAKER_ID),
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)

    /* Proprietary Specific Group */
    kGidOid_PropGetBindingCount          = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_SE_GET_BINDING_COUNT),
    kGidOid_PropTestSeLoop               = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_SE_DO_TEST_LOOP),
    kGidOid_PropCalibIntegrityProtection = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_CALIBRATION_INTEGRITY_PROTECTION),
    kGidOid_PropGenrateTag               = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_GENERATE_TAG),
    kGidOid_PropVerifyCalibData          = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_VERIFY_CALIB_DATA),
    kGidOid_EseGetSessionIdList          = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_ESE_GET_SESSION_ID_LIST),
    /* Vendor Specific Group */
    kGidOid_VendorSetVendorAppConfig   = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_SET_VENDOR_APP_CONFIG_CMD),
    kGidOid_VendorGetAllUwbSessions    = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_GET_ALL_UWB_SESSIONS),
    kGidOid_VendorGetVendorAppConfig   = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_GET_VENDOR_APP_CONFIG_CMD),
    kGidOid_VendorDoChipCalibration    = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_DO_CHIP_CALIBRATION),
    kGidOid_VendorSetDeviceCalibration = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_SET_DEVICE_CALIBRATION),
    kGidOid_VendorGetDeviceCalibration = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_GET_DEVICE_CALIBRATION),
#if UWBIOT_UWBD_SR2XXT
    kGidOid_VendorSetSecureCalibration = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_SET_SECURE_CALIBRATION),
#endif // UWBIOT_UWBD_SR2XXT
    kGidOid_VendorTestConnectivity = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_SE_DO_TEST_CONNECTIVITY),
    kGidOid_VendorDoBind           = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_SE_DO_BIND),
    kGidOid_VendorGetBindingStatus = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_ESE_BINDING_CHECK_CMD),
    kGidOid_VendorPsduLogNtf       = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_PSDU_LOG_NTF),
    kGidOid_VendorCirLogNtf        = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_CIR_LOG_NTF),
    kGidOid_PropQueryTemp          = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_QUERY_TEMPERATURE),
#endif //!(UWBIOT_UWBD_SR04X)
    /* Proprietary Specific Group */
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S)
    kGidOid_VendorUrskDeletionRequest = GET_VENDOR_GROUP_GID_OID(VENDOR_UCI_MSG_URSK_DELETION_REQ),
    kGidOid_PropConfigAuthTagOptions  = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_CONFIGURE_AUTH_TAG_OPTIONS_CMD),
    kGidOid_PropConfigAuthTagVersions = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_CONFIGURE_AUTH_TAG_VERSION_CMD),
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S)
#if (UWBIOT_UWBD_SR04X)
    kGidOid_GetStartTestMode        = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_TEST_START),
    kGidOid_GetStopTestMode         = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_TEST_STOP),
    kGidOid_GetSuspendDevice        = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_DEVICE_SUSPEND),
    kGidOid_SetCalibTrimMode        = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_SET_TRIM_VALUES),
    kGidOid_VendorGetAllUwbSessions = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_GET_ALL_UWB_SESSIONS),
    kGidOid_GetCalibTrimMode        = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_GET_TRIM_VALUES),
    kGidOid_GetSessionNvm           = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_SESSION_NVM_MANAGE),
    kGidOid_GetTrng                 = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_GEN_TRUE_RANDOM_NUM),
    kGidOid_BypassCurrentLimiter    = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_BYPASS_CURRENT_LIMITER),
#endif /* (UWBIOT_UWBD_SR04X) */

#if !(UWBIOT_UWBD_SR04X)
    kGidOid_GetTrng = GET_PROPRIETARY_SE_GROUP_GID_OID(EXT_UCI_MSG_GET_TRNG),
#endif // !(UWBIOT_UWBD_SR04X)

#if UWBFTR_BlobParser
#if (UWBIOT_UWBD_SR04X)
    kGidOid_ProfileParamMode = GET_PROP_GROUP_GID_OID(EXT_UCI_MSG_SET_PROFILE),
#elif (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)
    kGidOid_ProfileParamMode =
        GET_PROPRIETARY_SE_GROUP_GID_OID(EXT_UCI_MSG_SET_PROFILE), /*Todo : Check with FW once available*/
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)
#endif // UWBFTR_BlobParser
    /* MW Internal DM group */
    kGidOid_InternalUciEnable   = GET_INTERNAL_DM_GROUP_GID_OID(UCI_ENABLE),
    kGidOid_InternalUciDisable  = GET_INTERNAL_DM_GROUP_GID_OID(UCI_DISABLE),
    kGidOid_InternalUciRegExtCb = GET_INTERNAL_DM_GROUP_GID_OID(UCI_REG_EXT_CB),
    kGidOid_InternalUciTimeout  = GET_INTERNAL_DM_GROUP_GID_OID(UCI_TIMEOUT),
    kGidOid_InvalidGidOid       = 0xFFFF
} eGidOid_t;

/*****************************************************************************
 **  EXTERNAL FUNCTION DECLARATIONS
 *****************************************************************************/

/* Global UWB data */
extern tUWB_CB uwb_cb;

/**
 * Internal uwb functions
 */

/**
 **
 ** Function         uwb_ucif_process_event
 **
 ** Description      This function is called to process the
 **                  data/response/notification from UWBC
 **
 ** Returns          TRUE if need to free buffer
 **
 */
extern bool uwb_ucif_process_event(UWB_HDR *p_msg);

/**
 **
 ** Function         uwb_ucif_check_cmd_queue
 **
 ** Description      Send UCI command to the transport
 **
 ** Returns          void
 **
 */
extern void uwb_ucif_check_cmd_queue(UWB_HDR *p_buf);

/**
 **
 ** Function         uwb_ucif_retransmit_cmd
 **
 ** Description      Retransmission of last packet
 **
 ** Returns          void
 **
 */
extern void uwb_ucif_retransmit_cmd(UWB_HDR *p_buf);

/**
 **
 ** Function         uwb_ucif_send_cmd
 **
 ** Description      Send UCI command to the UCIT task
 **
 ** Returns          void
 **
 */
extern void uwb_ucif_send_cmd(UWB_HDR *p_buf);

/**
 **
 ** Function         uwb_ucif_update_cmd_window
 **
 ** Description      Update tx cmd window to indicate that UWBC can received
 **
 ** Returns          void
 **
 */
extern void uwb_ucif_update_cmd_window(void);

/**
 **
 ** Function         uwb_ucif_cmd_timeout
 **
 ** Description      Handle a command timeout
 **
 ** Returns          void
 **
 */
extern void uwb_ucif_cmd_timeout(void);

/**
 **
 ** Function         uwb_ucif_uwb_recovery
 **
 ** Description      uwb recovery
 **                  1) spi reset
 **                  2) FW download
 **
 ** Returns          void
 **
 */
extern void uwb_ucif_uwb_recovery(void);
void uwb_ucif_dump_fw_crash_log();

/* From uwb_task.c */

/**
**
** Function         uwb_task
**
** Description      UWB event processing task
**
** Returns          nothing
**
*/
extern OSAL_TASK_RETURN_TYPE uwb_task(void *args);

/**
**
** Function         uwb_task_shutdown_uwbc
**
** Description      Handle UWB shutdown
**
** Returns          nothing
**
*/
void uwb_task_shutdown_uwbc(void);

/* From uwb_main.c */

/**
**
** Function         uwb_set_state
**
** Description      Set the state of UWB stack
**
** Returns          void
**
*/
void uwb_set_state(tUWB_STATE uwb_state);

/**
**
** Function         uwb_main_flush_cmd_queue
**
** Description      This function is called when setting power off sleep state.
**
** Returns          void
**
*/
void uwb_main_flush_cmd_queue(void);

/**
**
** Function         uwb_main_handle_hal_evt
**
** Description      Handle BT_EVT_TO_UWB_MSGS
**
*/
void uwb_main_handle_hal_evt(tUWB_HAL_EVT_MSG *p_msg);

/**
**
** Function         uwb_main_post_hal_evt
**
** Description      This function posts HAL event to UWB_TASK
**
** Returns          void
**
*/
void uwb_main_post_hal_evt(uint8_t hal_evt, tUCI_STATUS status);

/**
**
** Function         uwb_gen_cleanup
**
** Description      Clean up for both going into low power mode and disabling
**                  UWB
**
*/
void uwb_gen_cleanup(void);

/**
**
** Function         uwb_start_quick_timer
**
** Description      Start a timer for the specified amount of time.
**                  NOTE: The timeout resolution depends on including modules.
**                  QUICK_TIMER_TICKS_PER_SEC should be used to convert from
**                  time to ticks.
**
**
** Returns          void
**
*/
void uwb_start_quick_timer(uint32_t timeout);

/**
**
** Function         uwb_stop_quick_timer
**
** Description      Stop a timer.
**
** Returns          void
**
*/
void uwb_stop_quick_timer();

/**
**
** Function         uwb_process_quick_timer_evt
**
** Description      Process quick timer event
**
** Returns          void
**
*/
void uwb_process_quick_timer_evt(uint16_t event);

/**
**
** Function         uwb_main_hal_cback
**
** Description      HAL event handler
**
** Returns          void
**
*/
void uwb_main_hal_cback(uint8_t event, tUCI_STATUS status);

/**
**
** Function         uwb_main_hal_data_cback
**
** Description      HAL data event handler
**
** Returns          void
**
*/
void uwb_main_hal_data_cback(uint16_t data_len, uint8_t *p_data);
#endif /* UWB_INT_H_ */
