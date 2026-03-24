/*
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

#include "UwbApi_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "PrintUtility.h"
#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "phOsalUwb.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "uci_test_defs.h"
#include "UwbAdaptation.h"
#include "UwbApi_Proprietary_Internal.h"
#include "AppConfigParams.h"
#include <UwbApi_Types.h>

#if UWBIOT_UWBD_SR1XXT
#include "UwbApi_RfTest.h"
#endif
// #include "uwb_api.h"
#include "uwb_int.h"
#include "UwbApi_Utility.h"
/* Context variable */

phUwbApiContext_t uwbContext;

/** Local functions prototypes */
static void rawCommandResponse_Cb(uint8_t gid, uint8_t event, uint16_t param_len, uint8_t *p_param);
static eResponse_Rsp_Event processInternalRsp(uint8_t oid, uint16_t len, uint8_t *eventData);
static eResponse_Rsp_Event processCoreRsp(uint8_t oid, uint16_t len, uint8_t *eventData);
static eResponse_Rsp_Event processSessionManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData);
static eResponse_Rsp_Event processRangeManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData);
#if !(UWBIOT_UWBD_SR04X)
static eResponse_Rsp_Event processTestManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData);
#endif //!(UWBIOT_UWBD_SR04X)
static eResponse_Rsp_Event processProprietaryRsp(uint8_t oid, uint16_t len, uint8_t *eventData);
static eResponse_Ntf_Event processCoreManagementNtf(uint8_t oid, uint8_t *eventData);
static eResponse_Ntf_Event processSessionManagementNtf(uint8_t oid, uint8_t *eventData, BOOLEAN *skip_sem_post);
static eResponse_Ntf_Event processRangeManagementNtf(uint8_t oid, uint16_t len, uint8_t *eventData);
#if !(UWBIOT_UWBD_SR04X)
static eResponse_Ntf_Event processTestManagementNtf(uint8_t oid, uint16_t len, uint8_t *eventData);
#endif // !(UWBIOT_UWBD_SR04X)
#if !(UWBIOT_UWBD_SR04X)
tUWBAPI_STATUS recoverUWBS(void)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    /*Resetting Device State to Device Init */
    uwbContext.dev_state = UWBD_STATUS_INIT;
    sep_SetWaitEvent(UWA_DM_DEVICE_STATUS_NTF_EVT);
    status = (uint8_t)UwbDeviceInit(TRUE);
    NXPLOG_UWBAPI_D("%s: uwb device init status: %d", __FUNCTION__, status);
    if (status == UWBAPI_STATUS_OK) {
        status = setDefaultCoreConfigs();
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: setDefaultCoreConfigs is failed:", __FUNCTION__);
            return status;
        }

        // Update UWBC device info
        status = getDeviceInfo();
    }
    else {
        sep_SetWaitEvent(DEFAULT_EVENT_TYPE);
        NXPLOG_UWBAPI_E("%s: DownloadFirmware is failed:", __FUNCTION__);
        return status;
    }
    return status;
}
#endif // !(UWBIOT_UWBD_SR04X)

void cleanUp()
{
    phOsalUwb_DeleteSemaphore(&uwbContext.devMgmtSem);
    Finalize(); // disable GKI, UCI task, UWB task
    phOsalUwb_SetMemory(&uwbContext, 0x00, sizeof(phUwbApiContext_t));
}

tUWBAPI_STATUS uwbInit(tUwbApi_AppCallback *pCallback, Uwb_operation_mode_t mode)
{
    tUWBAPI_STATUS status                = UWBAPI_STATUS_FAILED;
    const tHAL_UWB_ENTRY *halFuncEntries = NULL;
    phUwb_LogInit();
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == TRUE) {
        return UWBAPI_STATUS_OK;
    }
    uwbContext.sessionInfo.state = UWBAPI_SESSION_ERROR;
    uwbContext.pAppCallback      = pCallback;
    uwbContext.receivedEventId   = DEFAULT_EVENT_TYPE;
    if (phOsalUwb_CreateSemaphore(&uwbContext.devMgmtSem, 0) != UWBSTATUS_SUCCESS) {
        return status;
    }
#if UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250
    if (phOsalUwb_CreateSemaphore(&uwbContext.uwb_binding_status_ntf_wait, 0) != UWBSTATUS_SUCCESS) {
        LOG_E("%s : CreateSemaphore Failed for binding_status_ntf ", __FUNCTION__);
        return status;
    }
#endif // UWBIOT_SESN_SNXXX
    NXPLOG_UWBAPI_D("Initialize()");
    if (Initialize() != UCI_STATUS_OK) {
        LOG_E("%s : Initialize() Failed", __FUNCTION__);
        goto Error;
    }
    halFuncEntries = GetHalEntryFuncs();
    NXPLOG_UWBAPI_D("UWA_Init");
    UWA_Init(halFuncEntries);

    NXPLOG_UWBAPI_D("UfaEnable");
    sep_SetWaitEvent(UWA_DM_ENABLE_EVT);
#if UWBFTR_DataTransfer
    status =
        UWA_Enable(&ufaDeviceManagementRspCallback, &ufaDeviceManagementNtfCallback, &ufaDeviceManagementDataCallback);
#else
    status = UWA_Enable(&ufaDeviceManagementRspCallback, &ufaDeviceManagementNtfCallback);
#endif // UWBFTR_DataTransfer
    if (status == UWBAPI_STATUS_OK) {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT) !=
            UWBSTATUS_SUCCESS) {
            LOG_E("%s : UWA_DM_ENABLE_EVT timedout", __FUNCTION__);
            status = UWBAPI_STATUS_TIMEOUT;
            goto Error;
        }
        status = uwbContext.wstatus;
        if (status == UWBAPI_STATUS_OK) {
            sep_SetWaitEvent(UWA_DM_REGISTER_EXT_CB_EVT);
            status = UWA_RegisterExtCallback(&extDeviceManagementCallback);
            if (status == UWBAPI_STATUS_OK) {
                if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT) !=
                    UWBSTATUS_SUCCESS) {
                    LOG_E("%s : UWA_DM_REGISTER_EXT_CB_EVT timedout", __FUNCTION__);
                    status = UWBAPI_STATUS_TIMEOUT;
                    goto Error;
                }
                uwbContext.isUfaEnabled = TRUE;
                sep_SetWaitEvent(UWA_DM_DEVICE_STATUS_NTF_EVT);

                status = (uint8_t)UwbDeviceInit(FALSE);
                NXPLOG_UWBAPI_D("%s: DownloadFirmware status: %d", __FUNCTION__, status);
                /* Set operating mode */
                Hal_setOperationMode(mode);
                if (status == UWBAPI_STATUS_OK) {
                    status = setDefaultCoreConfigs();
                    if (status != UWBAPI_STATUS_OK) {
                        goto Error;
                    }

#if !(UWBIOT_UWBD_SR04X)
                    if (setVendorConfigs() != UWBAPI_STATUS_OK) {
                        NXPLOG_UWBAPI_E("%s : setVendorConfigs Failed", __FUNCTION__);
                        goto Error;
                    }
#endif // !(UWBIOT_UWBD_SR04X)

#if UWBFTR_DataTransfer
                    // Update UWBS capability info
                    phUwbCapInfo_t devCap;
                    status = getCapsInfo();
                    if (status == UWBAPI_STATUS_OK) {
                        if (parseCapabilityInfo(&devCap) == FALSE) {
                            NXPLOG_UWBAPI_E("%s: Parsing Capability Information Failed", __FUNCTION__);
                            status = UWBAPI_STATUS_FAILED;
                        }
                    }
                    else {
                        goto Error;
                    }
#endif // UWBFTR_DataTransfer
                }
                else {
                    sep_SetWaitEvent(DEFAULT_EVENT_TYPE);
                    goto Error;
                }
#if UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250
                if (uwbContext.fwMode == MAINLINE_FW){
                    /** Wait for the BINDING_STATUS_NTF */
                    if (phOsalUwb_ConsumeSemaphore_WithTimeout(
                            uwbContext.uwb_binding_status_ntf_wait, UWBD_BINDING_STATUS_NTF_TIMEOUT) == UWBSTATUS_SUCCESS) {
                        NXPLOG_UWBAPI_I("BINDING_STATUS_NTF received");
                        // Delete the Semaphore on it's completion.
                        phOsalUwb_DeleteSemaphore(&uwbContext.uwb_binding_status_ntf_wait);
                    }
                    else {
                        NXPLOG_UWBAPI_W("%s:BINDING_STATUS_NTF timed out", __FUNCTION__);
                        status = UWBAPI_STATUS_TIMEOUT;
                        goto Error;
                    }
                }
#endif // UWBIOT_SESN_SNXXX
            }
            else {
                NXPLOG_UWBAPI_D("%s: UWA_Enable status: %d", __FUNCTION__, status);
                return status;
            }
        }
        else {
            return status;
        }
        return status;
    }
Error:
    uwbContext.isUfaEnabled = FALSE;
    sep_SetWaitEvent(UWA_DM_DISABLE_EVT);
    if (UWA_Disable(FALSE) == UWBAPI_STATUS_OK) {
        if (UWBSTATUS_SUCCESS !=
            phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT)) {
            LOG_E("%s : phOsalUwb_ConsumeSemaphore_WithTimeout failed", __FUNCTION__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: UFA Disable is failed:", __FUNCTION__);
    }
#if UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250
    // Delete the Semaphore on it's completion.
    phOsalUwb_DeleteSemaphore(&uwbContext.uwb_binding_status_ntf_wait);
#endif // UWBIOT_SESN_SNXXX
    cleanUp();
    NXPLOG_UWBAPI_D("%s: exit with status %d", __FUNCTION__, status);
    return status;
}

void parseRangingParams(uint8_t *rspPtr, uint8_t noOfParams, phRangingParams_t *pRangingParams)
{
    uint8_t paramId;
    uint8_t length;
    uint32_t index = 0;

    for (int i = 0; i < noOfParams; i++) {
        UWB_STREAM_TO_UINT8(paramId, rspPtr, index);
        UWB_STREAM_TO_UINT8(length, rspPtr, index);
        switch (paramId) {
        case UCI_PARAM_ID_DEVICE_ROLE:
            /*  Device Role */
            UWB_STREAM_TO_UINT8(pRangingParams->deviceRole, rspPtr, index);
            break;
        case UCI_PARAM_ID_MULTI_NODE_MODE:
            /*  Multi Node Mode */
            UWB_STREAM_TO_UINT8(pRangingParams->multiNodeMode, rspPtr, index);
            break;
        case UCI_PARAM_ID_MAC_ADDRESS_MODE:
            /*  Mac addr mode */
            UWB_STREAM_TO_UINT8(pRangingParams->macAddrMode, rspPtr, index);
            break;
#if !(UWBIOT_UWBD_SR040)
        case UCI_PARAM_ID_SCHEDULED_MODE:
            /*  Scheduled Mode */
            UWB_STREAM_TO_UINT8(pRangingParams->scheduledMode, rspPtr, index);
            break;
#endif // !(UWBIOT_UWBD_SR040)
        case UCI_PARAM_ID_RANGING_ROUND_USAGE:
            /* Ranging Round Usage */
            UWB_STREAM_TO_UINT8(pRangingParams->rangingRoundUsage, rspPtr, index);
            break;
        case UCI_PARAM_ID_DEVICE_MAC_ADDRESS:
            /*  Device Mac Address */
            UWB_STREAM_TO_ARRAY(&pRangingParams->deviceMacAddr[0], rspPtr, length, index);
            break;
        case UCI_PARAM_ID_DEVICE_TYPE:
            /*  Device Type */
            UWB_STREAM_TO_UINT8(pRangingParams->deviceType, rspPtr, index);
            break;
        default:
            break;
        }
    }
}

/**
 **
 ** Function         processInternalRsp
 **
 ** Description      Process UCI responses in the internal group
 **
 ** Returns          void
 **
 */
static eResponse_Rsp_Event processInternalRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    /* process the message based on the opcode and message type */
    switch (oid) {
    case UCI_ENABLE: /* enable */
        dmEvent            = UWA_DM_ENABLE_EVT;
        uwbContext.wstatus = *eventData;
        uwa_dm_cb.flags |= UWA_DM_FLAGS_DM_IS_ACTIVE;
        break;
    case UCI_DISABLE: /* disable */
        dmEvent            = UWA_DM_DISABLE_EVT;
        uwbContext.wstatus = UWBAPI_STATUS_OK;
        uwa_dm_cb.flags &= (uint32_t)(~UWA_DM_FLAGS_DM_IS_ACTIVE);
        break;
    case UCI_REG_EXT_CB: /* register external CB */
        dmEvent            = UWA_DM_REGISTER_EXT_CB_EVT;
        uwbContext.wstatus = UWBAPI_STATUS_OK;
        break;
    case UCI_TIMEOUT: /* response timeout event */
        dmEvent            = UWA_DM_UWBD_RESP_TIMEOUT_EVT;
        uwbContext.wstatus = UWBAPI_STATUS_TIMEOUT;
        break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/**
 **
 ** Function         processCoreRsp
 **
 ** Description      Process UCI responses in the CORE group
 **
 ** Returns          void
 **
 */
static eResponse_Rsp_Event processCoreRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    uint32_t index              = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL)) {
        uint16_t timestampLen;

        /* process the message based on the opcode and message type */
        switch (oid) {
        case UCI_MSG_CORE_DEVICE_RESET:
            dmEvent            = UWA_DM_DEVICE_RESET_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_CORE_DEVICE_INFO: {
            dmEvent                  = UWA_DM_CORE_GET_DEVICE_INFO_RSP_EVT;
            uint16_t device_info_len = (uint16_t)(len - sizeof(uint8_t)); // exclude status length
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if ((uwbContext.wstatus == UCI_STATUS_OK) && (device_info_len <= sizeof(uwbContext.rsp_data))) {
                uwbContext.rsp_len = device_info_len;
                UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, eventData, device_info_len, index);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_CORE_DEVICE_CAP failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UCI_MSG_CORE_GET_CAPS_INFO: {
            dmEvent               = UWA_DM_GET_CORE_DEVICE_CAP_RSP_EVT;
            uint16_t cap_info_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if ((uwbContext.wstatus == UCI_STATUS_OK) && (cap_info_len <= sizeof(uwbContext.rsp_data))) {
                index++; // skip no of TLVs
                uwbContext.rsp_len = cap_info_len;
                UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, eventData, cap_info_len, index);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_CORE_DEVICE_CAP failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UCI_MSG_CORE_GET_CONFIG: {
            /* Result of UWA_GetCoreConfig */
            dmEvent                = UWA_DM_CORE_GET_CONFIG_RSP_EVT;
            uint16_t core_info_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && core_info_len <= sizeof(uwbContext.rsp_data)) {
                index++; // skip no of TLVs
                uwbContext.rsp_len = core_info_len;
                UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, eventData, core_info_len, index);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_CORE_CONFIG failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UCI_MSG_CORE_SET_CONFIG:
            /* Result of UWA_SetCoreConfig */
            dmEvent            = UWA_DM_CORE_SET_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_CORE_QUERY_UWBS_TIMESTAMP:
            dmEvent            = UWA_DM_PROP_QUERY_TIMESTAMP_RESP_EVT;
            uwbContext.wstatus = *eventData;
            timestampLen       = len - sizeof(uint8_t); // Exclude the status
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                if ((timestampLen == UCI_MSG_CORE_UWBS_TIMESTAMP_LEN) && (len <= sizeof(uwbContext.rsp_data))) {
                    uwbContext.rsp_len = len;
                    UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, eventData, uwbContext.rsp_len, index);
                }
                else {
                    NXPLOG_UWBAPI_E("%s: Invalid response", __FUNCTION__);
                    uwbContext.rsp_len = 0;
                }
            }
            else {
                NXPLOG_UWBAPI_E("%s: UCI_MSG_CORE_QUERY_UWBS_TIMESTAMP failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/**
 **
 ** Function         processSessionManagementRsp
 **
 ** Description      Process UCI responses in the Session Management group
 **
 ** Returns          void
 **
 */
static eResponse_Rsp_Event processSessionManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL) && (len < (MAX_UCI_HEADER_SIZE + MAX_API_PACKET_SIZE))) {
        switch (oid) {
        case UCI_MSG_SESSION_INIT:
            dmEvent            = UWA_DM_SESSION_INIT_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && len <= sizeof(uwbContext.rsp_data)) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
            else {
                uwbContext.rsp_len = 0;
                NXPLOG_UWBAPI_E("%s: UWA_DM_SESSION_INIT failed", __FUNCTION__);
            }
            break;
        case UCI_MSG_SESSION_DEINIT:
            dmEvent            = UWA_DM_SESSION_DEINIT_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_SESSION_GET_APP_CONFIG: {
            dmEvent                 = UWA_DM_SESSION_GET_CONFIG_RSP_EVT;
            uint16_t get_config_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && get_config_len <= sizeof(uwbContext.rsp_data)) {
                index++; // skip no of TLVs
                uwbContext.rsp_len = get_config_len;
                UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, eventData, get_config_len, index);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_APP_CONFIG failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UCI_MSG_SESSION_SET_APP_CONFIG:
            dmEvent            = UWA_DM_SESSION_SET_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_SESSION_GET_STATE:
            dmEvent            = UWA_DM_SESSION_GET_STATE_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                UWB_STREAM_TO_UINT8(uwbContext.rsp_data[0], eventData, index);
            }
            else {
                NXPLOG_UWBAPI_E("%s: get session state Request is failed", __FUNCTION__);
            }
            break;
        case UCI_MSG_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST:
            dmEvent            = UWA_DM_SESSION_MC_LIST_UPDATE_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            NXPLOG_UWBAPI_D("%s: Received Multicast List Status.\n", __FUNCTION__);
            if ((UWBAPI_STATUS_FAILED == uwbContext.wstatus) && (len <= sizeof(uwbContext.rsp_data))) {
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, (uint32_t)(len));
                uwbContext.rsp_len = len;
            }
            else {
                uwbContext.rsp_len = 0;
            }
            break;
#if UWBFTR_DL_TDoA_Anchor
        case UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_ANCHOR_DEVICE:
            dmEvent            = UWA_DM_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED) {
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, (uint32_t)(len - 1));
                uwbContext.rsp_len = len;
            }
            break;
#endif // UWBFTR_DL_TDoA_Anchor
#if UWBFTR_DL_TDoA_Tag
        case UCI_MSG_UPDATE_ACTIVE_ROUNDS_OF_RECEIVER_DEVICE:
            dmEvent            = UWA_DM_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED) {
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, (uint32_t)(len - 1));
                uwbContext.rsp_len = len;
            }
            break;
#endif // UWBFTR_DL_TDoA_Tag
        case UCI_MSG_SESSION_SET_HUS_CONTROLLER_CONFIG_CMD:
            dmEvent = UWA_DM_SESSION_SET_HUS_CONTROLLER_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E(
                    "%s: Received HUS Controller Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
            }
            break;
        case UCI_MSG_SESSION_SET_HUS_CONTROLEE_CONFIG_CMD:
            dmEvent = UWA_DM_SESSION_SET_HUS_CONTROLEE_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E(
                    "%s: Received HUS Controlee Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
            }
            break;
        case UCI_MSG_SESSION_DATA_TRANSFER_PHASE_CONFIG: {
            dmEvent = UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E("%s: Received DTPCM Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
            }
        } break;
#if !(UWBIOT_UWBD_SR04X)
        case UCI_MSG_SESSION_QUERY_DATA_SIZE_IN_RANGING:
            dmEvent = UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_RSP_EVT;
            /**
             * Response for Query data size command update with 3 fields
             * 1: Session Handle --> 4 bytes.
             * 2: Status -->1 Byte .
             * 3: data size --> 2 bytes
             */
            /** Increment  the pointer with 4 hence it will point to the status */
            uwbContext.wstatus = *(eventData + 4);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                if (len <= sizeof(uwbContext.rsp_data)) {
                    phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
                    uwbContext.rsp_len = len;
                }
                else {
                    LOG_E("%s : Not enough buffer to store %d bytes", __FUNCTION__, len);
                    uwbContext.rsp_len = 0;
                }
            }
            else {
                NXPLOG_UWBAPI_E(
                    "%s: Received Query status  Config Response Error: %d\n", __FUNCTION__, uwbContext.wstatus);
                uwbContext.rsp_len = 0;
            }
            break;
#endif // !(UWBIOT_UWBD_SR04X)
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/**
 **
 ** Function         processRangeManagementRsp
 **
 ** Description      Process UCI responses in the Ranging Management group
 **
 ** Returns          void
 **
 */
static eResponse_Rsp_Event processRangeManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL)) {
        switch (oid) {
        case UCI_MSG_RANGE_START:
            dmEvent            = UWA_DM_RANGE_START_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_RANGE_STOP:
            dmEvent            = UWA_DM_RANGE_STOP_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
        case UCI_MSG_RANGE_BLINK_DATA_TX:
            dmEvent            = UWA_DM_SEND_BLINK_DATA_RSP_EVT;
            uwbContext.wstatus = *eventData;
            break;
#if UWBFTR_DataTransfer
        case UCI_MSG_LOGICAL_LINK_CREATE: {
            dmEvent            = UWA_DM_SESSION_CREATE_LOGICAL_LINK_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data))) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_SESSION_CREATE_LOGICAL_LINK_RSP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UCI_MSG_LOGICAL_LINK_CLOSE: {
            dmEvent            = UWA_DM_SESSION_LOGICAL_LINK_CLOSE_RSP_EVT;
            uwbContext.wstatus = *eventData;
        } break;
        case UCI_MSG_LOGICAL_LINK_GET_PARAM: {
            dmEvent            = UWA_DM_SESSION_LOGICAL_LINK_GET_PARAM_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data))) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_SESSION_LOGICAL_LINK_GET_PARAM_RSP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
#endif // UWBFTR_DataTransfer
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/**
 **
 ** Function         processTestManagementRsp
 **
 ** Description      Process UCI responses in the test Management group
 **
 ** Returns          void
 **
 */
#if !(UWBIOT_UWBD_SR04X)
static eResponse_Rsp_Event processTestManagementRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    uint32_t index              = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL)) {
        switch (oid) {
        case UCI_MSG_TEST_GET_CONFIG: {
            dmEvent                  = UWA_DM_TEST_GET_CONFIG_RSP_EVT;
            uint16_t test_config_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && test_config_len <= sizeof(uwbContext.rsp_data)) {
                index++; // skip no of TLVs
                uwbContext.rsp_len = test_config_len;
                UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, eventData, test_config_len, index);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_TEST_GET_CONFIG_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UCI_MSG_TEST_SET_CONFIG:
            dmEvent            = UWA_DM_TEST_SET_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_TEST_PERIODIC_TX:
            dmEvent            = UWA_DM_TEST_PERIODIC_TX_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_TEST_PER_RX:
            dmEvent            = UWA_DM_TEST_PER_RX_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_TEST_LOOPBACK:
            dmEvent            = UWA_DM_TEST_LOOPBACK_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_TEST_RX:
            dmEvent            = UWA_DM_TEST_RX_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_TEST_SR_RX:
            dmEvent            = UWA_DM_TEST_SR_RX_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        case UCI_MSG_TEST_STOP_SESSION:
            dmEvent            = UWA_DM_TEST_STOP_SESSION_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}
#endif //!(UWBIOT_UWBD_SR04X)
/**
 **
 ** Function         processProprietaryRsp
 **
 ** Description      Process UCI responses in the propriotory group
 **
 ** Returns          void
 **
 */
static eResponse_Rsp_Event processProprietaryRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL)) {
        switch (oid) {
#if !(UWBIOT_UWBD_SR04X)
        case EXT_UCI_MSG_SE_GET_BINDING_COUNT: {
            dmEvent            = UWA_DM_PROP_GET_BINDING_COUNT_RESP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_GET_BINDING_COUNT_RESP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;

        case EXT_UCI_MSG_QUERY_TEMPERATURE: {
            dmEvent            = UWA_DM_PROP_QUERY_TEMPERATURE_RESP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_QUERY_TEMPERATURE_RESP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;

#endif // !(UWBIOT_UWBD_SR04X)
#if !(UWBIOT_UWBD_SR04X)
        case EXT_UCI_MSG_GENERATE_TAG: {
            dmEvent            = UWA_DM_PROP_GENERATE_TAG_RESP_EVT;
            uwbContext.wstatus = *eventData;
        } break;
#endif // !(UWBIOT_UWBD_SR04X)
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S)

        case EXT_UCI_MSG_CALIBRATION_INTEGRITY_PROTECTION: {
            dmEvent            = UWA_DM_PROP_CALIB_INTEGRITY_PROTECTION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        } break;

        case EXT_UCI_MSG_VERIFY_CALIB_DATA: {
            dmEvent            = UWA_DM_PROP_VERIFY_CALIB_DATA_RESP_EVT;
            uwbContext.wstatus = *eventData;
        } break;

        case EXT_UCI_MSG_CONFIGURE_AUTH_TAG_OPTIONS_CMD: {
            dmEvent            = UWA_DM_PROP_CONFIGURE_AUTH_TAG_OPTION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        } break;

        case EXT_UCI_MSG_CONFIGURE_AUTH_TAG_VERSION_CMD: {
            dmEvent            = UWA_DM_PROP_CONFIGURE_AUTH_TAG_VERSION_RESP_EVT;
            uwbContext.wstatus = *eventData;
        } break;
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S)

#if UWBIOT_SESN_SNXXX
        case EXT_UCI_MSG_SE_DO_TEST_LOOP: {
            dmEvent            = UWA_DM_PROP_SE_TEST_LOOP_RESP_EVT;
            uwbContext.wstatus = *eventData;
        } break;
        case EXT_UCI_MSG_ESE_GET_SESSION_ID_LIST: {
            dmEvent            = UWA_DM_PROP_GET_ESE_SESSION_ID_LIST_RSP_EVT;
            uwbContext.wstatus = *eventData;
        } break;
#endif // UWBIOT_SESN_SNXXX

#if (UWBIOT_UWBD_SR04X)
        case EXT_UCI_MSG_GET_ALL_UWB_SESSIONS:
            dmEvent            = UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
        case EXT_UCI_MSG_GET_TRNG:
            dmEvent            = UWA_DM_PROP_TRNG_RESP_EVENT;
            uwbContext.wstatus = *eventData;
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_TRNG_RESP_EVENT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
        case EXT_UCI_MSG_DEVICE_SUSPEND_CMD: {
            dmEvent            = UWA_DM_PROP_SUSPEND_DEVICE_RSP_ENVT;
            uwbContext.wstatus = *eventData;
        } break;
        case EXT_UCI_MSG_SESSION_NVM_MANAGE_CMD: {
            dmEvent            = UWA_DM_SESSION_NVM_PAYLOAD_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        } break;
        case EXT_UCI_MSG_TEST_START_CMD: {
            dmEvent            = UWA_DM_START_TEST_MODE_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        } break;
        case EXT_UCI_MSG_TEST_STOP_CMD: {
            dmEvent            = UWA_DM_STOP_TEST_MODE_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        } break;
        case EXT_UCI_MSG_SET_TRIM_VALUES_CMD: {
            dmEvent            = UWA_DM_SET_CALIB_TRIM_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        } break;
        case EXT_UCI_MSG_GET_TRIM_VALUES_CMD: {
            dmEvent            = UWA_DM_GET_CALIB_TRIM_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        } break;
#if (UWBFTR_BlobParser)
        case EXT_UCI_MSG_SET_PROFILE: {
            dmEvent            = UWA_DM_PROP_PROFILE_BLOB_RSP_EVENT;
            uwbContext.wstatus = *eventData;
            /* catch the length in the response of the PROP_SET_PROFILE_CMD*/
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data))) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
        } break;
#endif // UWBFTR_BlobParser
        case EXT_UCI_MSG_BYPASS_CURRENT_LIMITER_CMD: {
            dmEvent            = UWA_DM_GET_BYPASS_CURRENT_LIMITER;
            uwbContext.wstatus = *eventData;
        } break;
#endif /* (UWBIOT_UWBD_SR04X) */
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}
#if !(UWBIOT_UWBD_SR04X)
/**
 **
 ** Function         processVendorRsp
 **
 ** Description      Process UCI responses in the proprietary group
 **
 ** Returns          eResponse_Rsp_Event
 **
 */
static eResponse_Rsp_Event processVendorRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    uint32_t index              = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL) && (len < (MAX_UCI_HEADER_SIZE + MAX_API_PACKET_SIZE))) {
        switch (oid) {
        case VENDOR_UCI_MSG_DO_CHIP_CALIBRATION: {
            dmEvent            = UWA_DM_VENDOR_DO_CHIP_CALIBRATION_RESP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
        } break;
        case UCI_MSG_SESSION_VENDOR_GET_APP_CONFIG: {
            dmEvent                 = UWA_DM_SESSION_GET_VENDOR_CONFIG_RSP_EVT;
            uint16_t get_config_len = (uint16_t)(len - TLV_BUFFER_OFFSET);
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK && get_config_len <= sizeof(uwbContext.rsp_data)) {
                index++; // skip no of TLVs
                uwbContext.rsp_len = get_config_len;
                UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, eventData, get_config_len, index);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_APP_CONFIG failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UCI_MSG_SESSION_VENDOR_SET_APP_CONFIG:
            dmEvent            = UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            break;

        case VENDOR_UCI_MSG_SET_DEVICE_CALIBRATION: {
            dmEvent            = UWA_DM_VENDOR_SET_DEVICE_CALIBRATION_RESP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
        } break;

#if UWBIOT_UWBD_SR2XXT
        case VENDOR_UCI_MSG_SET_SECURE_CALIBRATION: {
            dmEvent            = UWA_DM_VENDOR_SET_SECURE_CALIB_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
        } break;
#endif // UWBIOT_UWBD_SR2XXT

        case VENDOR_UCI_MSG_GET_DEVICE_CALIBRATION: {
            dmEvent            = UWA_DM_VENDOR_GET_DEVICE_CALIBRATION_RESP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                if (sizeof(uwbContext.rsp_data) >= (size_t)len) {
                    uwbContext.rsp_len = len;
                    phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
                }
                else {
                    LOG_E("%s : Not enough buffer to store %d bytes", __FUNCTION__, len);
                    uwbContext.rsp_len = 0;
                }
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_VENDOR_GET_DEVICE_CALIBRATION_RESP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;

        case VENDOR_UCI_MSG_GET_ALL_UWB_SESSIONS: {
            dmEvent            = UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
            if (uwbContext.wstatus == UWBAPI_STATUS_OK) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: VENDOR_GET_ALL_UWB_SESSION failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }

        } break;

#if (UWBIOT_SESN_SNXXX)
        case VENDOR_UCI_MSG_SE_DO_TEST_CONNECTIVITY: {
            dmEvent            = UWA_DM_PROP_TEST_CONNECTIVITY_RESP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
        } break;

        case VENDOR_UCI_MSG_ESE_BINDING_CHECK_CMD: {
            dmEvent            = UWA_DM_PROP_GET_BINDING_STATUS_RESP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
        } break;

        case VENDOR_UCI_MSG_URSK_DELETION_REQ: {
            dmEvent            = UWA_DM_PROP_URSK_DELETION_REQUEST_RESP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
        } break;

        case VENDOR_UCI_MSG_SE_DO_BIND: {
            dmEvent            = UWA_DM_PROP_DO_BIND_RESP_EVT;
            UWB_STREAM_TO_UINT8(uwbContext.wstatus, eventData, index);
        } break;

#endif // (UWBIOT_SESN_SNXXX)
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}
#endif // !(UWBIOT_UWBD_SR04X)

#if !(UWBIOT_UWBD_SR04X)
/**
 **
 ** Function         processProprietarySERsp
 **
 ** Description      Process UCI responses in the propriotorySE group
 **
 ** Returns          void
 **
 */
static eResponse_Rsp_Event processProprietarySeRsp(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if ((len != 0) && (eventData != NULL)) {
        switch (oid) {
#if (UWBFTR_BlobParser)
        case EXT_UCI_MSG_SET_PROFILE: {
            dmEvent            = UWA_DM_PROP_PROFILE_BLOB_RSP_EVENT;
            uwbContext.wstatus = *eventData;
            /* catch the sessionHandle in the response of the PROP_SET_PROFILE_CMD*/
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data))) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, len);
            }
        } break;
#endif // (UWBFTR_BlobParser)
        case EXT_UCI_MSG_GET_TRNG:
            dmEvent            = UWA_DM_PROP_TRNG_RESP_EVENT;
            uwbContext.wstatus = *eventData;
            if ((uwbContext.wstatus == UWBAPI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data))) {
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_PROP_TRNG_RESP_EVENT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
            break;
#if !(UWBIOT_UWBD_SR2XXT)
        case EXT_UCI_MSG_WRITE_CALIB_DATA_CMD: {
            dmEvent            = UWA_DM_PROP_WRITE_OTP_CALIB_DATA_RSP_EVT;
            uwbContext.wstatus = *eventData;
        } break;

        case EXT_UCI_MSG_READ_CALIB_DATA_CMD: {
            dmEvent            = UWA_DM_PROP_READ_OTP_CALIB_DATA_RSP_EVT;
            uwbContext.wstatus = *eventData;
        } break;
#endif // UWBIOT_UWBD_SR2XXT
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
        case EXT_UCI_MSG_WRITE_MODULE_MAKER_ID: {
            dmEvent            = UWA_DM_WRITE_MODULE_MAKER_RSP_EVT;
            uwbContext.wstatus = *eventData;
        } break;
        case EXT_UCI_MSG_READ_MODULE_MAKER_ID: {
            dmEvent            = UWA_DM_READ_MODULE_MAKER_RSP_EVT;
            uwbContext.wstatus = *eventData;
            if ((uwbContext.wstatus == UCI_STATUS_OK) && (len <= sizeof(uwbContext.rsp_data))){
                uwbContext.rsp_len = len;
                phOsalUwb_MemCopy(uwbContext.rsp_data, eventData, uwbContext.rsp_len);
            }
            else{
                NXPLOG_UWBAPI_E("%s: UWA_DM_READ_MODULE_MAKER_RSP_EVT failed", __FUNCTION__);
                uwbContext.rsp_len = 0;
            }
        } break;
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
#if UWBFTR_CSA
        case EXT_UCI_MSG_SESSION_SET_LOCALIZATION_ZONE_CMD: {
            dmEvent            = UWA_DM_SESSION_SET_LOCALIZATION_ZONE_RSP_EVENT;
            uwbContext.wstatus = *eventData;
        } break;
#endif // UWBFTR_CSA
        default:
            NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
            break;
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}
#endif // !(UWBIOT_UWBD_SR04X)
/**
 **
 ** Function         processCoreManagementNtf
 **
 ** Description      Process UCI responses in the core Management group
 **
 ** Returns          void
 **
 */
static eResponse_Ntf_Event processCoreManagementNtf(uint8_t oid, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    switch (oid) {
    case UCI_MSG_CORE_GENERIC_ERROR_NTF: {
        dmEvent = UWA_DM_CORE_GEN_ERR_STATUS_EVT;
        if (UCI_STATUS_MESSAGE_RETRY == (*eventData))
        {
            /* Don't pass error notification to application */
        }
#if (UWBIOT_UWBD_SR1XXT)
        /*
         * Notify application if STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY is received.
         */
        else if (UCI_STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY == *eventData) {
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_OVER_TEMP_REACHED, NULL, 0);
            }
        }
#endif
        else {
            phGenericError_t generic_error = {0};
            generic_error.status = *eventData;
            NXPLOG_UWBAPI_E("%s: UWA_DM_CORE_GEN_ERR_STATUS_EVT status %d", __FUNCTION__, *eventData);
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_GENERIC_ERROR_NTF, &generic_error, sizeof(generic_error));
            }
        }
    } break;
    case UCI_MSG_CORE_DEVICE_STATUS_NTF: {
        dmEvent = UWA_DM_DEVICE_STATUS_NTF_EVT;
        // uwb_ucif_proc_core_device_status(eventData, len);
        uwbContext.dev_state = (eUWBD_DEVICE_STATUS_t)*eventData;
        if (uwbContext.dev_state == UWBD_STATUS_ERROR) {
            if (isCmdRespPending()) {
                uwbContext.wstatus = UWBAPI_STATUS_TIMEOUT;
            }
            else {
                // in case of uwb device err firmware crash, send recovery signal to app
                if (uwbContext.pAppCallback) {
                    uwbContext.pAppCallback(UWBD_RECOVERY_NTF, &uwbContext.wstatus, sizeof(uwbContext.wstatus));
                }
            }
        }
        else if (uwbContext.dev_state == UWBAPI_STATUS_HPD_WAKEUP) {
            uwbContext.wstatus = UWBAPI_STATUS_HPD_WAKEUP;
            /* Keeping below code for future use in case we want to change the handling of HPD wakeup*/
            /* If Device Status Notification is 0xFC, then inform application to perform clean up */
            // if (uwbContext.pAppCallback) {
            //      uwbContext.pAppCallback(UWBD_ACTION_APP_CLEANUP, NULL, 0);
            // }
        }
#if (UWBIOT_UWBD_SR04X)
        else if (uwbContext.dev_state == UWBAPI_STATUS_LOW_POWER_ERROR) {
            uwbContext.wstatus = UWBAPI_STATUS_LOW_POWER_ERROR;
        }
#endif /* (UWBIOT_UWBD_SR04X) */
    } break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/**
 **
 ** Function         processSessionManagementNtf
 **
 ** Description      Process UCI responses in the session Management group
 **
 ** Returns          void
 **
 */
static eResponse_Ntf_Event processSessionManagementNtf(uint8_t oid, uint8_t *eventData, BOOLEAN *skip_sem_post)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    switch (oid) {
    case UCI_MSG_SESSION_STATUS_NTF: {
        dmEvent = UWA_DM_SESSION_STATUS_NTF_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.sessionInfo.sessionHandle, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.sessionInfo.state, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.sessionInfo.reason_code, eventData, index);

#if (defined(UWBFTR_Radar) && (UWBFTR_Radar != 0))
        if ((uwbContext.sessionInfo.reason_code == UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL) ||
            (uwbContext.sessionInfo.reason_code == UWB_SESSION_RADAR_FCC_LIMIT_REACHED)) {
#else
        if (uwbContext.sessionInfo.reason_code == UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL) {
#endif                             // (defined(UWBFTR_Radar) && (UWBFTR_Radar != 0))
            *skip_sem_post = TRUE; // Skip posting session status in-band termination
        }

        if (uwbContext.sessionInfo.reason_code != UWB_SESSION_STATE_CHANGED) {
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_SESSION_DATA, &uwbContext.sessionInfo, sizeof(uwbContext.sessionInfo));
            }
        }
    } break;
    case UCI_MSG_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST: {
        dmEvent = UWA_DM_SESSION_MC_LIST_UPDATE_NTF_EVT;
        phMulticastControleeListNtfContext_t pControleeNtfContext;
        UWB_STREAM_TO_UINT32(pControleeNtfContext.sessionHandle, eventData, index);
        UWB_STREAM_TO_UINT8(pControleeNtfContext.no_of_controlees, eventData, index);

        if (pControleeNtfContext.no_of_controlees > MAX_NUM_CONTROLLEES) {
            NXPLOG_UWBAPI_E("%s: wrong number of controless : %d", __FUNCTION__, pControleeNtfContext.no_of_controlees);
            break;
        }
        for (uint8_t i = 0; i < pControleeNtfContext.no_of_controlees; i++) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
            UWB_STREAM_TO_UINT16(pControleeNtfContext.controleeStatusList[i].controlee_mac_address, eventData, index);
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
            /*
             * If Action is 0x02 or 0x03 for STS_CONFIG values other than 0x04, the UWBS shall return
             * STATUS_ERROR_SUB_SESSION_KEY_NOT_APPLICABLE for each Controlee in the Controlee List
             */
            UWB_STREAM_TO_UINT8(pControleeNtfContext.controleeStatusList[i].status, eventData, index);
        }

        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_MULTICAST_LIST_NTF, &pControleeNtfContext, sizeof(pControleeNtfContext));
        }

        uwbContext.wstatus = *(eventData + index);
        NXPLOG_UWBAPI_D("%s: Received Multicast List data.\n", __FUNCTION__);
    } break;
    case UCI_MSG_SESSION_DATA_TRANSFER_PHASE_CONFIG: {
        dmEvent = UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_NTF_EVT;
        phDataTxPhaseCfgNtf_t pDataTxPhCfgNtf;
        UWB_STREAM_TO_UINT32(pDataTxPhCfgNtf.sessionHandle, eventData, index);
        UWB_STREAM_TO_UINT8(pDataTxPhCfgNtf.status, eventData, index);
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_DATA_TRANSFER_PHASE_CONFIG_NTF, &pDataTxPhCfgNtf, sizeof(pDataTxPhCfgNtf));
        }
    } break;
#if (UWBIOT_UWBD_SR04X) && UWBFTR_DataTransfer
    case UCI_MSG_DATA_TRANSMIT_STATUS_NTF: {
        dmEvent = UWA_DM_DATA_TRANSMIT_STATUS_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.dataTransmit.transmitNtf_connectionId, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.dataTransmit.transmitNtf_sequence_number, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.dataTransmit.transmitNtf_status, eventData, index);
    } break;
    case UCI_MSG_DATA_CREDIT_NTF: {
        dmEvent = UWA_DM_DATA_CREDIT_STATUS_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.dataCredit.connectionId, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.dataCredit.credit_availability, eventData, index);
    } break;
#endif // !(UWBIOT_UWBD_SR2XX) && UWBFTR_DataTransfer
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return dmEvent;
}

/**
 **
 ** Function         processRangeManagementNtf
 **
 ** Description      Process UCI responses in the range Management group
 **
 ** Returns          void
 **
 */
static eResponse_Ntf_Event processRangeManagementNtf(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    switch (oid) {
#if UWBFTR_DataTransfer && (UWBIOT_UWBD_SR1XXT_SR2XXT)
    case UCI_MSG_DATA_TRANSMIT_STATUS_NTF: {
        uint32_t index = 0; // Declaration has to be here as the code is under feature macro
        dmEvent = UWA_DM_DATA_TRANSMIT_STATUS_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.dataTransmit.transmitNtf_connectionId, eventData, index);
        UWB_STREAM_TO_UINT16(uwbContext.dataTransmit.transmitNtf_sequence_number, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.dataTransmit.transmitNtf_status, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.dataTransmit.transmitNtf_txcount, eventData, index);
        /**
         * data Send command using Raw Api , Notfication is handling Asynchronously
         * Hence indicating the UWBD_DATA_TRANSMIT_NTF using Callback .
         */
        if (uwb_cb.rawCmdCbflag == TRUE) {
            uwbContext.pAppCallback(
                UWBD_DATA_TRANSMIT_NTF, (void *)&uwbContext.dataTransmit, sizeof(uwbContext.dataTransmit));
        }
    } break;
    case UCI_MSG_DATA_CREDIT_NTF: {
        uint32_t index = 0; // Declaration has to be here as the code is under feature macro
        dmEvent = UWA_DM_DATA_CREDIT_STATUS_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.dataCredit.connectionId, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.dataCredit.credit_availability, eventData, index);
        /**
         * data Send command using Raw Api , Notfication is handling Asynchronously
         * Hence indicating the UWBD_CREDIT_RCV_NTF using Callback .
         * uwb_cb.rawCmdCbflag = false; explicitly disable the rawcbflag for last trasmission of the Data transfer.
         */
        if (uwb_cb.rawCmdCbflag == TRUE) {
            uwbContext.pAppCallback(UWBD_CREDIT_RCV_NTF, (void *)&uwbContext.dataCredit, sizeof(uwbContext.dataCredit));
            if (uwbContext.dataCredit.credit_availability) {
                uwb_cb.rawCmdCbflag = false;
            }
        }
    } break;
    case UCI_MSG_LOGICAL_LINK_CREATE: {
        uint32_t index = 0; // Declaration has to be here as the code is under feature macro
        dmEvent = UWA_DM_SESSION_LOGICAL_LINK_CREATE_NTF_EVT;
        UWB_STREAM_TO_UINT32(uwbContext.logicalLinkCreate.ll_connect_id, eventData, index);
        UWB_STREAM_TO_UINT8(uwbContext.logicalLinkCreate.status, eventData, index);
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(
                UWBD_LL_CREATE_NTF, (void *)&uwbContext.logicalLinkCreate, sizeof(uwbContext.logicalLinkCreate));
        }
    } break;
    case UCI_MSG_LOGICAL_LINK_UWBS_CLOSE: {
        dmEvent = UWA_DM_SESSION_LOGICAL_LINK_UWBS_CLOSE_NTF_EVT;
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_LL_UWBS_CLOSE_NTF, eventData, len);
        }
    } break;
    case UCI_MSG_LOGICAL_LINK_UWBS_CREATE: {
        dmEvent = UWA_DM_SESSION_LOGICAL_LINK_UWBS_CREATE_NTF_EVT;
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_LL_UWBS_CREATE_NTF, eventData, len);
        }
    } break;
#endif // UWBFTR_DataTransfer
    case UCI_MSG_SESSION_INFO_NTF:
        dmEvent = UWA_DM_SESSION_INFO_NTF_EVT;
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_RANGING_DATA, eventData, len);
        }
        break;
    case UCI_MSG_SESSION_ROLE_CHANGE_NTF: {
        dmEvent = UWA_DM_SESSION_ROLE_CHANGE_NTF_EVT;
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_SESSION_ROLE_CHANGE_RCV_NTF, eventData, len);
        }
        break;
    }
#if UWBFTR_CCC
    case UCI_MSG_RANGE_CCC_DATA_NTF: {
        dmEvent = UWA_DM_RANGE_CCC_DATA_NTF_EVT;
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_RANGING_CCC_DATA, eventData, len);
        }
    } break;
#endif // UWBFTR_CCC
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    return dmEvent;
}

#if UWBFTR_Radar

/**
 **
 ** Function         processDataControlNtf
 **
 ** Description      Process UCI responses in the data Management group
 **
 ** Returns          void
 **
 */
static eResponse_Ntf_Event processDataControlNtf(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    switch (oid) {
    case EXT_UCI_MSG_RADAR_NTF: {
        if (len < RADAR_CIR_NTF_HEADER) {
            NXPLOG_UWBAPI_E("%s: Invalid  Radar Notificaiton Length :0x%x", __FUNCTION__, len);
            break;
        }
        if (uwbContext.pAppCallback) {
            if (eventData[RADAR_NTF_RADAR_TYPE_OFFSET] == RADAR_MEASUREMENT_TYPE_CIR) {
                uwbContext.pAppCallback(UWBD_RADAR_RCV_NTF, eventData, len);
            }
            else if (eventData[RADAR_NTF_RADAR_TYPE_OFFSET] == RADAR_MEASUREMENT_TYPE_TEST_ISOLATION) {
                uwbContext.pAppCallback(UWBD_TEST_RADAR_ISO_NTF, eventData, len);
            }
            else if (eventData[RADAR_NTF_RADAR_TYPE_OFFSET] == RADAR_MEASUREMENT_TYPE_PRESENCE_DETECTION) {
                uwbContext.pAppCallback(UWBD_PRESENCE_DETECTION_NTF, eventData, len);
            }
        }
    } break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    return dmEvent;
}

#endif // UWBFTR_Radar

#if !(UWBIOT_UWBD_SR04X)
/**
 **
 ** Function         processTestManagementNtf
 **
 ** Description      Process UCI responses in the test Management group
 **
 ** Returns          void
 **
 */
static eResponse_Ntf_Event processTestManagementNtf(uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;
    uint32_t index = 0;
    switch (oid) {
    case UCI_MSG_TEST_PERIODIC_TX: {
        dmEvent = UWA_DM_TEST_PERIODIC_TX_NTF_EVT;
        phPerTxData_t pPerTxData;
        UWB_STREAM_TO_UINT8(pPerTxData.status, eventData, index);
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(UWBD_PER_SEND, &pPerTxData, sizeof(pPerTxData));
        }
    } break;
    case UCI_MSG_TEST_PER_RX:
    case UCI_MSG_TEST_RX:
    case UCI_MSG_TEST_LOOPBACK:
    case UCI_MSG_TEST_SR_RX: {
        if (uwbContext.pAppCallback) {
            switch (oid) {
            case UCI_MSG_TEST_PER_RX:
                dmEvent = UWA_DM_TEST_PER_RX_NTF_EVT;
                uwbContext.pAppCallback(UWBD_PER_RCV, eventData, len);
                break;
            case UCI_MSG_TEST_RX:
                dmEvent = UWA_DM_TEST_RX_NTF_EVT;
                uwbContext.pAppCallback(UWBD_TEST_RX_RCV, eventData, len);
                break;
            case UCI_MSG_TEST_LOOPBACK:
                dmEvent = UWA_DM_TEST_LOOPBACK_NTF_EVT;
                uwbContext.pAppCallback(UWBD_TEST_MODE_LOOP_BACK_NTF, eventData, len);
                break;
            case UCI_MSG_TEST_SR_RX:
                dmEvent = UWA_DM_TEST_TEST_SR_RX_NTF_EVT;
                uwbContext.pAppCallback(UWBD_SR_RX_RCV, eventData, len);
                break;
            }
        }
    } break;
    default:
        NXPLOG_UWBAPI_E("%s: unknown oid:0x%x", __FUNCTION__, oid);
        break;
    }
    return dmEvent;
}
#endif // !(UWBIOT_UWBD_SR04X)

void ufaDeviceManagementRspCallback(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Rsp_Event dmEvent = UWA_DM_INVALID_RSP_EVT;

    NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __FUNCTION__, dmEvent, *eventData);

    switch (gid) {
    case UCI_GID_INTERNAL:
        dmEvent = processInternalRsp(oid, len, eventData);
        break;
    case UCI_GID_CORE: /* 0000b UCI Core group */
        dmEvent = processCoreRsp(oid, len, eventData);
        break;
    case UCI_GID_SESSION_MANAGE: /* 0001b UCI Session Config group */
        dmEvent = processSessionManagementRsp(oid, len, eventData);
        break;
    case UCI_GID_RANGE_MANAGE: /* 0010b UCI Range group */
        dmEvent = processRangeManagementRsp(oid, len, eventData);
        break;
#if !(UWBIOT_UWBD_SR04X)
    case UCI_GID_TEST: /* 1101b test group */
        dmEvent = processTestManagementRsp(oid, len, eventData);
        break;
#endif // !(UWBIOT_UWBD_SR04X)
    case UCI_GID_PROPRIETARY_CUSTOM_1:
        dmEvent = processProprietaryRsp(oid, len, eventData);
        break;
#if !(UWBIOT_UWBD_SR04X)
    case UCI_GID_PROPRIETARY:
        dmEvent = processProprietarySeRsp(oid, len, eventData);
        break;
#endif // !(UWBIOT_UWBD_SR04X)
#if !(UWBIOT_UWBD_SR04X)
    case UCI_GID_PROPRIETARY_CUSTOM_2:
        dmEvent = processVendorRsp(oid, len, eventData);
        break;
#endif //!(UWBIOT_UWBD_SR04X)
    default:
        NXPLOG_UWBAPI_E("ufaDeviceManagementRspCallback: Unknown gid:%d", gid);
        break;
    }

    uwbContext.receivedEventId = dmEvent;

    if (uwbContext.currentEventId == dmEvent || dmEvent == UWA_DM_UWBD_RESP_TIMEOUT_EVT ||
        (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))) {
        NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __FUNCTION__);
        uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
        (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
    }
}

void ufaDeviceManagementNtfCallback(uint8_t gid, uint8_t oid, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;

    BOOLEAN skip_sem_post = FALSE;
    if ((len != 0) && (eventData != NULL)) {
        NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __FUNCTION__, dmEvent, *eventData);

        switch (gid) {
        case UCI_GID_CORE:
            dmEvent = processCoreManagementNtf(oid, eventData);
            break;
        case UCI_GID_SESSION_MANAGE: /* 0001b UCI management group */
            dmEvent = processSessionManagementNtf(oid, eventData, &skip_sem_post);
            break;
        case UCI_GID_RANGE_MANAGE: /* 0010b UCI Range management group */
            dmEvent = processRangeManagementNtf(oid, len, eventData);
            break;
#if UWBFTR_Radar
        case UCI_GID_PROP_RADAR_CONTROL: /* 1001b UCI DATA control group */
            dmEvent = processDataControlNtf(oid, len, eventData);
            if (oid == EXT_UCI_MSG_RADAR_NTF) {
                skip_sem_post = TRUE;
            }
            break;
#endif // UWBFTR_Radar
#if !(UWBIOT_UWBD_SR04X)
        case UCI_GID_TEST: /* 1101b test group */
            dmEvent = processTestManagementNtf(oid, len, eventData);
            break;
#endif // !(UWBIOT_UWBD_SR04X)
        default:
            NXPLOG_UWBAPI_E("ufaDeviceManagementNtfCallback: UWB Unknown gid:%d", gid);
            break;
        }
    }
    uwbContext.receivedEventId = dmEvent;
    if (uwbContext.currentEventId == dmEvent || (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))
#if (UWBIOT_UWBD_SR04X)
        || (uwbContext.dev_state == UWBD_STATUS_HDP_WAKEUP)
#endif
    ) {
        if (!skip_sem_post) {
            NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __FUNCTION__);
            uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
            (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
        }
    }
}
#if UWBFTR_DataTransfer

void ufaDeviceManagementDataCallback(uint8_t dpf, uint16_t len, uint8_t *eventData)
{
    eResponse_Ntf_Event dmEvent = UWA_DM_INVALID_NTF_EVT;

    BOOLEAN skip_sem_post = FALSE;
    if ((len != 0) && (eventData != NULL)) {
        NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __FUNCTION__, dmEvent, *eventData);

        switch (dpf) {
        case UCI_DPF_RCV: /* Data Packet Format for receive data with message type 0 */
            dmEvent = UWA_DM_DATA_RCV_NTF_EVT;
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_DATA_RCV_NTF, eventData, len);
            }
            break;
        case UCI_DPF_LL_RCV: /* Logical Link Data Packet Format for receive data */
            dmEvent = UWA_DM_LOGICAL_DATA_RCV_NTF_EVT;
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_LOGICAL_LINK_DATA_RCV_NTF, eventData, len);
            }
            break;
        default:
            NXPLOG_UWBAPI_E("%s : UWB Unknown dpf:%d", __FUNCTION__, dpf);
            break;
        }
    }
    uwbContext.receivedEventId = dmEvent;
    if (uwbContext.currentEventId == dmEvent || (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))) {
        if (!skip_sem_post) {
            NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __FUNCTION__);
            uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
            (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
        }
    }
}
#endif // UWBFTR_DataTransfer

void sep_SetWaitEvent(uint16_t eventID)
{
    uwbContext.currentEventId = eventID;
}

/**
 **
 ** Function:        RawCommandResponse_Cb
 **
 ** Description:     Receive response from the stack for raw command sent from
 **                  UWB API.
 **                  gid: gid
 **                  event:  event ID.
 **                  param_len: length of the response
 **                  p_param: pointer to data
 **
 ** Returns:         None
 **
 */
static void rawCommandResponse_Cb(uint8_t gid, uint8_t event, uint16_t param_len, uint8_t *p_param)
{
    NXPLOG_UWBAPI_D(
        "NxpResponse_Cb Received length data = 0x%x status = 0x%x", param_len, p_param[UCI_RESPONSE_STATUS_OFFSET]);
    uwbContext.wstatus = UWBAPI_STATUS_FAILED;
    uwbContext.rsp_len = param_len;
    if (event == UWB_SEGMENT_PKT_SENT) {
        uwbContext.wstatus = UWBAPI_STATUS_PBF_PKT_SENT;
    }
    else if (param_len > 0 && p_param != NULL) {
        uwbContext.wstatus = p_param[UCI_RESPONSE_STATUS_OFFSET];
        phOsalUwb_MemCopy(uwbContext.rsp_data, p_param, param_len);
    }
    NXPLOG_UWBAPI_D("%s: posting devMgmtSem", __FUNCTION__);
    uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
    (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
}

tUWBAPI_STATUS sendRawUci(uint8_t *p_cmd_params, uint16_t cmd_params_len)
{
    uint8_t cmd_gid, cmd_oid, rsp_gid, rsp_oid;

    bool datapacketFlag   = false;
    datapacketFlag        = IS_DATA_SEND_PACKET(p_cmd_params[0]);
    cmd_gid               = p_cmd_params[0] & UCI_GID_MASK;
    cmd_oid               = p_cmd_params[1] & UCI_OID_MASK;
    tUWBAPI_STATUS status;

    sep_SetWaitEvent(UWA_DM_UWBD_RESP_TIMEOUT_EVT);
    status = UWA_SendRawCommand(cmd_params_len, p_cmd_params, &rawCommandResponse_Cb);
    if (status == UCI_STATUS_OK) {
        /** Data transfer command using the SendRaw
         * uwbContext.wstatus = UWBAPI_STATUS_OK; is required as  uwbContext.wstatus is global context .
         * Data transfer does not have any response hence make the status success and safe exit.
         */
        if (true == datapacketFlag) {
            NXPLOG_UWBAPI_W("%s:For data transfer command no need to wait for response", __FUNCTION__);
            uwbContext.wstatus = UWBAPI_STATUS_OK;
            status             = UWBAPI_STATUS_OK;
            goto exit;
        }
        status = phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, 3000); // Wait for only 3000ms or 3sec
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: semaphore wait timed out", __FUNCTION__);
            status = UWBAPI_STATUS_TIMEOUT;
            goto exit;
        }
        NXPLOG_UWBAPI_D("%s: Success UWA_SendRawCommand", __FUNCTION__);
        status = uwbContext.wstatus;
    }
    else {
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    if (uwbContext.wstatus == UWBAPI_STATUS_TIMEOUT) {
        status = UWBAPI_STATUS_TIMEOUT;
    }
    else if (uwbContext.wstatus == UWBAPI_STATUS_PBF_PKT_SENT) {
        status = UWBAPI_STATUS_PBF_PKT_SENT;
    }
    else if (uwbContext.wstatus == UWBAPI_STATUS_HPD_WAKEUP) {
        status = UWBAPI_STATUS_HPD_WAKEUP;
    }
#if (UWBIOT_UWBD_SR04X)
    else if (status == UWBAPI_STATUS_LOW_POWER_ERROR) {
        status = UWBAPI_STATUS_LOW_POWER_ERROR;
        NXPLOG_UWBAPI_W("%s: Low Power Mode Error : Status : %d", __FUNCTION__, status);
    }
#endif /* (UWBIOT_UWBD_SR04X) */
    else {
        rsp_gid = uwbContext.rsp_data[0] & UCI_GID_MASK;
        rsp_oid = uwbContext.rsp_data[1] & UCI_OID_MASK;
        if ((cmd_gid != rsp_gid) || (cmd_oid != rsp_oid)) {
            LOG_E(
                "Status = %d: Error, Received gid/oid other than what is sent, sent %x%x recv "
                "%x%x",
                status,
                cmd_gid,
                cmd_oid,
                rsp_gid,
                rsp_oid);
            status = UWBAPI_STATUS_FAILED;
        }
    }
exit:
    return status;
}

tUWBAPI_STATUS waitforNotification(uint16_t waitEventId, uint32_t waitEventNtftimeout)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    /*
     * Check the waiting notification is already received or not.
     */
    if (uwbContext.receivedEventId != waitEventId) {
        /*
         *  Wait for the event notification
         */
        sep_SetWaitEvent(waitEventId);

        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, waitEventNtftimeout) == UWBSTATUS_SUCCESS) {
            status = UWBAPI_STATUS_OK;
        }
        else {
            /*
             * A scenario can happen when waiting for session status notification.
             * Session status notification comes prior to device status notification.
             * In that case, status to be set to ok since the notification is
             * already received. In any case, session state checking is done in the
             * Session init/deinit related API's.
             */
            if (UWA_DM_SESSION_STATUS_NTF_EVT == waitEventId) {
                status = UWBAPI_STATUS_OK;
            }
            else {
                NXPLOG_UWBAPI_E("%s: Wait timed-out for EventId: %d", __FUNCTION__, waitEventId);
            }
        }
    }
    else {
        status = UWBAPI_STATUS_OK;
    }
    /*
     * Reset the received event id to default event.
     */
    uwbContext.receivedEventId = DEFAULT_EVENT_TYPE;

    return status;
}

uint8_t getAppConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;

    tlvBuffer[length++] = paramId;

    if (paramLen > (MAX_UCI_PACKET_SIZE - 2 /* 1 byte of paramId + 1 byte of param_len */)) {
        LOG_E("%s, app config value len is greater than max uci payload size", __FUNCTION__);
        length = 0;
        return length;
    }

    switch (paramId) {
        /* Length 1 Byte */
    case UCI_PARAM_ID_DEVICE_ROLE:
    case UCI_PARAM_ID_RANGING_ROUND_USAGE:
    case UCI_PARAM_ID_STS_CONFIG:
    case UCI_PARAM_ID_MULTI_NODE_MODE:
    case UCI_PARAM_ID_CHANNEL_NUMBER:
    case UCI_PARAM_ID_NO_OF_CONTROLEES:
    case UCI_PARAM_ID_SESSION_INFO_NTF:
    case UCI_PARAM_ID_DEVICE_TYPE:
    case UCI_PARAM_ID_MAC_FCS_TYPE:
    case UCI_PARAM_ID_RANGING_ROUND_CONTROL:
    case UCI_PARAM_ID_AOA_RESULT_REQ:
    case UCI_PARAM_ID_RFRAME_CONFIG:
    case UCI_PARAM_ID_RSSI_REPORTING:
    case UCI_PARAM_ID_PREAMBLE_CODE_INDEX:
    case UCI_PARAM_ID_SFD_ID:
    case UCI_PARAM_ID_PSDU_DATA_RATE:
    case UCI_PARAM_ID_PREAMBLE_DURATION:
    case UCI_PARAM_ID_RANGING_TIME_STRUCT:
    case UCI_PARAM_ID_SLOTS_PER_RR:
#if (UWBIOT_UWBD_SR04X)
    case UCI_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER:
#endif /* (UWBIOT_UWBD_SR04X) */
    case UCI_PARAM_ID_PRF_MODE:
    case UCI_PARAM_ID_SCHEDULED_MODE:
    case UCI_PARAM_ID_KEY_ROTATION:
    case UCI_PARAM_ID_KEY_ROTATION_RATE:
    case UCI_PARAM_ID_SESSION_PRIORITY:
    case UCI_PARAM_ID_MAC_ADDRESS_MODE:
    case UCI_PARAM_ID_NUMBER_OF_STS_SEGMENTS:
    case UCI_PARAM_ID_HOPPING_MODE:
    case UCI_PARAM_ID_IN_BAND_TERMINATION_ATTEMPT_COUNT:
    case UCI_PARAM_ID_RESULT_REPORT_CONFIG:
    case UCI_PARAM_ID_STS_LENGTH:
    case UCI_PARAM_ID_UL_TDOA_TX_TIMESTAMP:
    case UCI_PARAM_ID_BPRF_PHR_DATA_RATE:
    case UCI_PARAM_ID_BLOCK_STRIDING:
    case UCI_PARAM_ID_DLTDOA_RANGING_METHOD:
    case UCI_PARAM_ID_DLTDOA_TX_TIMESTAMP_CONF:
    case UCI_PARAM_ID_DLTDOA_INTER_CLUSTER_SYNC_PERIOD:
    case UCI_PARAM_ID_DLTDOA_ANCHOR_CFO:
    case UCI_PARAM_ID_DLTDOA_TX_ACTIVE_RANGING_ROUNDS:
    case UCI_PARAM_ID_DL_TDOA_BLOCK_SKIPPING:
#if UWBFTR_CCC
    case UCI_PARAM_ID_PULSESHAPE_COMBO:
    case UCI_PARAM_ID_RESPONDER_LISTEN_ONLY:
    case UCI_PARAM_ID_RESPONDER_SLOT_INDEX:
#endif // UWBFTR_CCC
#if UWBFTR_CSA
    case UCI_PARAM_ID_ALIRO_MAC_MODE:
#endif // UWBFTR_CSA
    case UCI_PARAM_ID_SUSPEND_RANGING_ROUNDS:
    case UCI_PARAM_ID_DLTDOA_TIME_REF_ANCHOR:
    case UCI_PARAM_ID_APPLICATION_DATA_ENDPOINT:
    case UCI_PARAM_ID_DL_TDOA_RESPONDER_TOF:
    case UCI_PARAM_ID_DATA_TRANSFER_STATUS_NTF_CONFIG:
    case UCI_PARAM_ID_SECURE_RANGING_NEFA_LEVEL:
    case UCI_PARAM_ID_SECURE_RANGING_CSW_LENGTH: {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
#if UWBFTR_DataTransfer
        case UCI_PARAM_ID_LINK_LAYER_MODE:
    case UCI_PARAM_ID_DATA_REPETITION_COUNT:
#endif // UWBFTR_DataTransfer
    case UCI_PARAM_ID_MIN_FRAMES_PER_RR:
    case UCI_PARAM_ID_INTER_FRAME_INTERVAL: {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;

    /* Length 2 Bytes */
    case UCI_PARAM_ID_CAP_SIZE_RANGE: // Contention based ranging.
    case UCI_PARAM_ID_NEAR_PROXIMITY_CONFIG:
    case UCI_PARAM_ID_FAR_PROXIMITY_CONFIG:
    case UCI_PARAM_ID_SLOT_DURATION:
    case UCI_PARAM_ID_MAX_RR_RETRY:
    case UCI_PARAM_ID_VENDOR_ID:
#if UWBFTR_CCC
    case UCI_PARAM_ID_RANGING_PROTOCOL_VER:
    case UCI_PARAM_ID_UWB_CONFIG_ID:
    case UCI_PARAM_ID_URSK_TTL:
#endif // UWBFTR_CCC
    case UCI_PARAM_ID_MAX_NUMBER_OF_MEASUREMENTS:
    case UCI_PARAM_ID_MTU_SIZE: {
        tlvBuffer[length++] = 2; // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;

    /* Length 4 Byte */
    case UCI_PARAM_ID_UL_TDOA_TX_INTERVAL:
    case UCI_PARAM_ID_UL_TDOA_RANDOM_WINDOW:
    case UCI_PARAM_ID_STS_INDEX:
#if UWBFTR_CCC
    case UCI_PARAM_ID_LAST_STS_INDEX_USED:
    case UCI_PARAM_ID_HOP_MODE_KEY:
#endif // UWBFTR_CCC
    case UCI_PARAM_ID_SUB_SESSION_ID:
    case UCI_PARAM_ID_RANGING_DURATION: {
        tlvBuffer[length++] = 4; // Param len
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;

    /* Length Array of 1 Bytes */
#if !(UWBIOT_UWBD_SR04X)
    case UCI_PARAM_ID_SESSION_TIME_BASE:
#endif
    case UCI_PARAM_ID_UL_TDOA_NTF_REPORT_CONFIG:
    case UCI_PARAM_ID_UL_TDOA_DEVICE_ID:
    case UCI_PARAM_ID_STATIC_STS_IV:
    case UCI_PARAM_ID_DEVICE_MAC_ADDRESS:
    case UCI_PARAM_ID_DST_MAC_ADDRESS:
    case UCI_PARAM_ID_UWB_INITIATION_TIME:
    case UCI_PARAM_ID_DLTDOA_ANCHOR_LOCATION:
    case UCI_PARAM_ID_SESSION_KEY:
    case UCI_PARAM_ID_SUB_SESSION_KEY:
    case UCI_PARAM_ID_AOA_BOUND_CONFIG: {
        uint8_t *value      = (uint8_t *)paramValue;
        tlvBuffer[length++] = paramLen; // Param len
        for (uint8_t i = 0; i < (paramLen / sizeof(uint8_t)); i++) {
            tlvBuffer[length++] = value[i];
        }
    } break;
    default:
        length = 0;
        break;
    }

    return length;
}

#if !(UWBIOT_UWBD_SR04X)

uint8_t getTestConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;

    tlvBuffer[length++] = paramId;

    switch (paramId) {
    /* Length 1 Byte */
    case UCI_TEST_PARAM_ID_RANDOMIZE_PSDU:
    case UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR:
    case UCI_TEST_PARAM_ID_PHR_RANGING_BIT:
    case UCI_TEST_PARAM_ID_STS_DETECT_BITMAP_EN: {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;

    /* Length 4 Byte */
    case UCI_TEST_PARAM_ID_NUM_PACKETS:
    case UCI_TEST_PARAM_ID_T_GAP:
    case UCI_TEST_PARAM_ID_T_START:
    case UCI_TEST_PARAM_ID_T_WIN:
    case UCI_TEST_PARAM_ID_RMARKER_TX_START:
    case UCI_TEST_PARAM_ID_RMARKER_RX_START: {
        tlvBuffer[length++] = 4; // Param len
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;
    default:
        length = 0;
        break;
    }
    return length;
}

tUWBAPI_STATUS VendorAppConfig_TlvParser(
    const UWB_VendorAppParams_List_t *pAppParams_List, UWB_AppParams_value_au8_t *pOutput_param_value)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    uint8_t *param_value = pAppParams_List->param_value.au8.param_value;

    switch (pAppParams_List->param_type) {
    case kUWB_APPPARAMS_Type_u32:
        pOutput_param_value->param_len = 4;
        UWB_UINT32_TO_FIELD(pOutput_param_value->param_value, pAppParams_List->param_value.vu32);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_APPPARAMS_Type_au8:
        pOutput_param_value->param_len = pAppParams_List->param_value.au8.param_len;
        UWB_STREAM_TO_ARRAY(pOutput_param_value->param_value, param_value, pOutput_param_value->param_len, index);
        status = UWBAPI_STATUS_OK;
        break;
    default:
        status = UWBAPI_STATUS_FAILED;
        break;
    }
    return status;
}
#endif // !(UWBIOT_UWBD_SR04X)

uint8_t getCoreDeviceConfigTLVBuffer(uint8_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;
    if (paramValue == NULL || tlvBuffer == 0) {
        NXPLOG_UWBAPI_E("%s: Buffer is NULL", __FUNCTION__);
        return 0;
    }
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    case UCI_PARAM_ID_DEVICE_STATE:
    case UCI_PARAM_ID_LOW_POWER_MODE:
#if (UWBIOT_UWBD_SR04X)
    case UCI_EXT_PARAM_ID_MHR_IN_CCM:
    case UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_ENABLE:
#endif
    {
        tlvBuffer[length++] = 1; // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        if (value != ENABLED && value != DISABLED) {
            return 0;
        }
        tlvBuffer[length++] = value;
    } break;
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    case UCI_PARAM_ID_UCI_WIFI_COEX_FEATURE: {
        UWB_WiFiCoEx_Ftr_t *wifiCoExFtr = (UWB_WiFiCoEx_Ftr_t *)paramValue;
        tlvBuffer[length++] = paramLen; // Param len
        tlvBuffer[length++] = wifiCoExFtr->UWB_WiFiCoEx_Enable;
        tlvBuffer[length++] = wifiCoExFtr->UWB_WiFiCoEx_noOfChannels;

        for (uint8_t i = 0; i < wifiCoExFtr->UWB_WiFiCoEx_noOfChannels; i++) {
            tlvBuffer[length++] = wifiCoExFtr->wifiCoexFtrList[i].UWB_WiFiCoEx_channel_Id;
            tlvBuffer[length++] = wifiCoExFtr->wifiCoexFtrList[i].UWB_WiFiCoEx_MinGuardDuration;
            tlvBuffer[length++] = wifiCoExFtr->wifiCoexFtrList[i].UWB_WiFiCoEx_MaxGrantDuration;
            tlvBuffer[length++] = wifiCoExFtr->wifiCoexFtrList[i].UWB_WiFiCoEx_AdvacnedGrantDuration;
        }
    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    default:
        length = 0;
        break;
    }
    return length;
}

void parseCoreGetDeviceConfigResponse(uint8_t *tlvBuffer, phDeviceConfigData_t *devConfig)
{
    uint16_t paramId;
    uint32_t index = 0;
    UWB_STREAM_TO_UINT8(paramId, tlvBuffer, index);
    index++; // skipping the length
    switch (paramId) {
    /* 1 byte len */
    case UCI_PARAM_ID_LOW_POWER_MODE: {
        UWB_STREAM_TO_UINT8(devConfig->lowPowerMode, tlvBuffer, index);
    } break;
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    case UCI_PARAM_ID_UCI_WIFI_COEX_FEATURE: {
        UWB_STREAM_TO_UINT8(devConfig->wifiCoExFtr.UWB_WiFiCoEx_Enable, tlvBuffer, index);
        UWB_STREAM_TO_UINT8(devConfig->wifiCoExFtr.UWB_WiFiCoEx_noOfChannels, tlvBuffer, index);

        for (uint8_t i = 0; i < devConfig->wifiCoExFtr.UWB_WiFiCoEx_noOfChannels; i++) {
            UWB_STREAM_TO_UINT8(devConfig->wifiCoExFtr.wifiCoexFtrList[i].UWB_WiFiCoEx_channel_Id, tlvBuffer, index);
            UWB_STREAM_TO_UINT8(
                devConfig->wifiCoExFtr.wifiCoexFtrList[i].UWB_WiFiCoEx_MinGuardDuration, tlvBuffer, index);
            UWB_STREAM_TO_UINT8(
                devConfig->wifiCoExFtr.wifiCoexFtrList[i].UWB_WiFiCoEx_MaxGrantDuration, tlvBuffer, index);
            UWB_STREAM_TO_UINT8(
                devConfig->wifiCoExFtr.wifiCoexFtrList[i].UWB_WiFiCoEx_AdvacnedGrantDuration, tlvBuffer, index);
        }
    } break;
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    default :
        LOG_E("%s ParmId is invalid %d ", __FUNCTION__, paramId);
    }
}

tUWBAPI_STATUS getDeviceInfo(void)
{
    tUWBAPI_STATUS status;
    sep_SetWaitEvent(UWA_DM_CORE_GET_DEVICE_INFO_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_DEVICE_INFO_EVT, 0, NULL);

    return status;
}

tUWBAPI_STATUS getCapsInfo(void)
{
    tUWBAPI_STATUS status;
    sep_SetWaitEvent(UWA_DM_GET_CORE_DEVICE_CAP_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_CAPS_INFO_EVT, 0, NULL);
    return status;
}

tUWBAPI_STATUS AppConfig_TlvParser(
    const UWB_AppParams_List_t *pAppParams_List, UWB_AppParams_value_au8_t *pOutput_param_value)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    uint8_t *param_value = pAppParams_List->param_value.au8.param_value;

    switch (pAppParams_List->param_type) {
    case kUWB_APPPARAMS_Type_u32:
        pOutput_param_value->param_len = 4;
        UWB_UINT32_TO_FIELD(pOutput_param_value->param_value, pAppParams_List->param_value.vu32);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_APPPARAMS_Type_au8:
        pOutput_param_value->param_len = pAppParams_List->param_value.au8.param_len;
        UWB_STREAM_TO_ARRAY(pOutput_param_value->param_value, param_value, pOutput_param_value->param_len, index);
        status = UWBAPI_STATUS_OK;
        break;
    default:
        status = UWBAPI_STATUS_FAILED;
        break;
    }
    return status;
}

tUWBAPI_STATUS parseUwbSessionParams(uint8_t *rspPtr, phUwbSessionsContext_t *pUwbSessionsContext)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    // Validation of all the parameters needs to be added.
    const uint8_t maxAvailableCount = pUwbSessionsContext->sessioncnt;
    if (maxAvailableCount == 0) {
        LOG_W("pUwbSessionsContext->sessioncnt is not set");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    else if (maxAvailableCount > 10) {
        LOG_W("Seems pUwbSessionsContext->sessioncnt is garbage");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    else {
        UWB_STREAM_TO_UINT8(pUwbSessionsContext->sessioncnt, rspPtr, index);

        if (maxAvailableCount < pUwbSessionsContext->sessioncnt) {
            LOG_W("Param Error: Not all Values returned for session. ");
            pUwbSessionsContext->sessioncnt = maxAvailableCount;
            pUwbSessionsContext->status     = kUWBSTATUS_BUFFER_TOO_SMALL;
        }

        for (uint8_t i = 0; i < pUwbSessionsContext->sessioncnt; i++) {
            UWB_STREAM_TO_UINT32(pUwbSessionsContext->pUwbSessionData[i].sessionHandle, rspPtr, index);
            UWB_STREAM_TO_UINT8(pUwbSessionsContext->pUwbSessionData[i].session_type, rspPtr, index);
            UWB_STREAM_TO_UINT8(pUwbSessionsContext->pUwbSessionData[i].session_state, rspPtr, index);
        }
        printUwbSessionData(pUwbSessionsContext);
        status = UWBAPI_STATUS_OK;
    }
exit:
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetAllUwbSessions(phUwbSessionsContext_t *pUwbSessionsContext)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pUwbSessionsContext == NULL) {
        NXPLOG_UWBAPI_E("%s: UwbSessionsContext is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_VENDOR_GET_ALL_UWB_SESSION_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_GET_ALL_UWB_SESSIONS, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: GetAllUWBSessions successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(pUwbSessionsContext->status, rspPtr, index);

        if (pUwbSessionsContext->status == UWBAPI_STATUS_OK) {
            /*
             * Parse all the response parameters are correct or not.
             */
            status = parseUwbSessionParams((rspPtr + index), pUwbSessionsContext);
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E("%s: parseUwbSessionParams failed", __FUNCTION__);
            }
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: GetAllUWBSessions Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: GetAllUWBSessions failed", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

BOOLEAN parseCapabilityInfo(phUwbCapInfo_t *pDevCap)
{
    BOOLEAN status        = TRUE;
    uint16_t index        = 0;
    uint8_t extParamId    = 0;
    uint8_t length        = 0;
    uint8_t capsInfoLen   = uwbContext.rsp_len;
    uint8_t *capsInfoData = uwbContext.rsp_data;
    if ((capsInfoLen == 0) || (capsInfoData == NULL)) {
        NXPLOG_UWBAPI_E("%s: capsInfoLen is zero or capsInfoData is NULL", __FUNCTION__);
        return FALSE;
    }

    while (index < capsInfoLen) {
        // Store Ext Param Id in case of 0xE0, 0xE1,..or Param Id in case of 0xA0, 0xA1,..0x01, 0x02,...
        extParamId = capsInfoData[index++];

        if ((extParamId & EXTENDED_PARAM_ID_MASK) == CCC_INFO_ID) {
            length = capsInfoData[index++];
            if((length + index) > capsInfoLen) {
                NXPLOG_UWBAPI_E("%s: Invalid length for extended capability info", __FUNCTION__);
                status = FALSE;
                break;
            }

#if (UWBFTR_CCC && UWBIOT_UWBD_SR1XXT_SR2XXT)
            if (parseCapabilityCCCParams(pDevCap, extParamId, &index, length, capsInfoData) == FALSE) {
                NXPLOG_UWBAPI_W("Error Parsing CCC Params.");
                status = FALSE;
            }
#else
            NXPLOG_UWBAPI_W("%s: unknown param Id 0x%X", __FUNCTION__, extParamId);
            index = (uint8_t)(index + length); // Skip CCC Params
#endif // UWBFTR_CCC
        }
#if !(UWBIOT_UWBD_SR04X)
        else if (extParamId == EXTENDED_CAP_INFO_ID) {
            index++; // skip the param ID
            length = capsInfoData[index++];
            if((length + index) > capsInfoLen) {
                NXPLOG_UWBAPI_E("%s: Invalid length for extended capability info", __FUNCTION__);
                status = FALSE;
                break;
            }
            index = (uint8_t)(index + length);
        }
#endif // !(UWBIOT_UWBD_SR04X)
        /* Skipping E0 to E9 CCC Parameters */
        else if ((extParamId & EXTENDED_PARAM_ID_MASK) == CCC_EXT_PARAM_ID) {
            length = capsInfoData[index++];
            if((length + index) > capsInfoLen) {
                NXPLOG_UWBAPI_E("%s: Invalid length for extended capability info", __FUNCTION__);
                status = FALSE;
                break;
            }
            index  = (uint8_t)(index + length);
        }
        else {
            length = capsInfoData[index++];
            if((length + index) > capsInfoLen) {
                NXPLOG_UWBAPI_E("%s: Invalid length for extended capability info", __FUNCTION__);
                status = FALSE;
                break;
            }

            switch (extParamId) {
            case MAX_MESSAGE_SIZE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->maxMessageSize, &capsInfoData[index], length);
#if UWBFTR_DataTransfer
                uwbContext.maxMessageSize = pDevCap->maxMessageSize;
#endif // UWBFTR_DataTransfer
                index = (uint8_t)(index + length);
            } break;
            case MAX_DATA_PACKET_PAYLOAD_SIZE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->maxDataPacketPayloadSize, &capsInfoData[index], length);
#if UWBFTR_DataTransfer
                uwbContext.maxDataPacketPayloadSize = pDevCap->maxDataPacketPayloadSize;
#endif // UWBFTR_DataTransfer
                index = (uint8_t)(index + length);
            } break;
            case FIRA_PHY_VERSION_RANGE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_4) {
                    return FALSE;
                }
                pDevCap->firaPhyLowerRangeMajorVersion             = capsInfoData[index++];
                pDevCap->firaPhyLowerRangeMinorMaintenanceVersion  = capsInfoData[index++];
                pDevCap->firaPhyHigherRangeMajorVersion            = capsInfoData[index++];
                pDevCap->firaPhyHigherRangeMinorMaintenanceVersion = capsInfoData[index++];
            } break;
            case FIRA_MAC_VERSION_RANGE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_4) {
                    return FALSE;
                }
                pDevCap->firaMacLowerRangeMajorVersion             = capsInfoData[index++];
                pDevCap->firaMacLowerRangeMinorMaintenanceVersion  = capsInfoData[index++];
                pDevCap->firaMacHigherRangeMajorVersion            = capsInfoData[index++];
                pDevCap->firaMacHigherRangeMinorMaintenanceVersion = capsInfoData[index++];
            } break;

            case DEVICE_ROLES_ID: {
#if (UWBIOT_UWBD_SR040)
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->deviceRoles = capsInfoData[index++];
#else  // !(UWBIOT_UWBD_SR040)
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->deviceRoles, &capsInfoData[index], length);
                index += length;
#endif /* (UWBIOT_UWBD_SR040) */
            } break;

            case RANGING_METHOD_ID: {
#if (UWBIOT_UWBD_SR040)
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->rangingMethod = capsInfoData[index++];
#else  // !(UWBIOT_UWBD_SR040)
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->rangingMethod, &capsInfoData[index], length);
                index += length;
#endif /* (UWBIOT_UWBD_SR040) */
            } break;

#if !(UWBIOT_UWBD_SR040)
            case DEVICE_TYPE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->deviceTypes = capsInfoData[index++];
            } break;
#endif //  !(UWBIOT_UWBD_SR040)

            case STS_CONFIG_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->stsConfig = capsInfoData[index++];
            } break;
            case MULTI_NODE_MODE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->multiNodeMode = capsInfoData[index++];
            } break;
            case RANGING_TIME_STRUCT_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->rangingTimeStruct = capsInfoData[index++];
            } break;
            case SCHEDULED_MODE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->scheduledMode = capsInfoData[index++];
            } break;
            case HOPPING_MODE_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->hoppingMode = capsInfoData[index++];
            } break;
            case BLOCK_STRIDING_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->blockStriding = capsInfoData[index++];
            } break;
            case UWB_INITIATION_TIME_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->uwbInitiationTime = capsInfoData[index++];
            } break;
            case CHANNELS_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->channels = capsInfoData[index++];
            } break;
            case RFRAME_CONFIG_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->rframeConfig = capsInfoData[index++];
            } break;
            case CC_CONSTRAINT_LENGTH_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->ccConstraintLength = capsInfoData[index++];
            } break;
            case BPRF_PARAMETER_SETS_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->bprfParameterSets = capsInfoData[index++];
            } break;
            case HPRF_PARAMETER_SETS_ID: {
                if (length != DEVICE_CAPABILITY_LEN_5) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(pDevCap->hprfParameterSets, &capsInfoData[index], length);
                index = (uint8_t)(index + length);
            } break;
            case AOA_SUPPORT_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->aoaSupport = capsInfoData[index++];
            } break;
            case EXTENDED_MAC_ADDR_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->extendedMacAddress = capsInfoData[index++];
            } break;
#if !(UWBIOT_UWBD_SR040)
            case SUSPEND_RANGING_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->suspendRanging = capsInfoData[index++];
            } break;
            case SESSION_KEY_LEN_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->sessionKeyLen = capsInfoData[index++];
            } break;
            case DT_ANCHOR_MAX_ACTIVE_RR_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->ancorMaxRrActive = capsInfoData[index++];
            } break;
            case DT_TAG_MAX_ACTIVE_RR_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->tagMaxRrActive = capsInfoData[index++];
            } break;
            case DT_TAG_BLOCK_SKIPPING_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->tagBlockSkipping = capsInfoData[index++];
            } break;
            case PSDU_LENGTH_SUPPORT_ID: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->psduLengthSupport = capsInfoData[index++];
            } break;
#if !(UWBIOT_UWBD_SR048M)
            case LL_CAPABILITY_PARAM: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                pDevCap->llCapabilityParam = capsInfoData[index] | (capsInfoData[index + 1] << 8);
                index                      = (uint8_t)(index + length);
            } break;
            case BYPASS_MODE_SUPPORT: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->bypassModeSupport = capsInfoData[index++];
            } break;
            case UCI_EXT_PARAM_ID_UWBS_MAX_UCI_PAYLOAD_LENGTH: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                phOsalUwb_MemCopy(&pDevCap->maxUciPayloadLength, &capsInfoData[index], length);
                index = (uint8_t)(index + length);
            } break;
            case UCI_EXT_PARAM_ID_UWBS_INBAND_DATA_BUFFER_BLOCK_SIZE: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->inbandDataBlockSize = capsInfoData[index++];
            } break;
            case UCI_EXT_PARAM_ID_UWBS_INBAND_DATA_MAX_BLOCKS: {
                if (length != DEVICE_CAPABILITY_LEN_1) {
                    return FALSE;
                }
                pDevCap->inbandDataMaxBlock = capsInfoData[index++];
            } break;
            case MIN_SLOT_DURATION_SUPPORT: {
                if (length != DEVICE_CAPABILITY_LEN_2) {
                    return FALSE;
                }
                pDevCap->minSlotDurationSupport = capsInfoData[index] | (capsInfoData[index + 1] << 8);
                index                           = (uint8_t)(index + length);
            } break;
#endif /* !(UWBIOT_UWBD_SR048M) */
#endif /* !(UWBIOT_UWBD_SR040) */
            default:
                NXPLOG_UWBAPI_W("%s: unknown param Id 0x%X", __FUNCTION__, extParamId);
                index = (uint8_t)(index + length);
            }
        }
    } // End Of While

    return status;
}

const char *getStatusString(uint8_t status)
{
    switch (status) {
    case UWBAPI_STATUS_REJECTED:
        return "UWBAPI_STATUS_REJECTED";
    case UWBAPI_STATUS_FAILED:
        return "UWBAPI_STATUS_FAILED";
    case UWBAPI_STATUS_NOT_INITIALIZED:
        return "UWBAPI_STATUS_NOT_INITIALIZED";
    case UWBAPI_STATUS_INVALID_PARAM:
        return "UWBAPI_STATUS_INVALID_PARAM";
    case UWBAPI_STATUS_INVALID_RANGE:
        return "UWBAPI_STATUS_INVALID_RANGE";
    case UWBAPI_STATUS_SESSION_NOT_EXIST:
        return "UWBAPI_STATUS_SESSION_NOT_EXIST";
    case UWBAPI_STATUS_SESSION_ACTIVE:
        return "UWBAPI_STATUS_SESSION_ACTIVE";
    case UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED:
        return "UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED";
    case UWBAPI_STATUS_SESSION_NOT_CONFIGURED:
        return "UWBAPI_STATUS_SESSION_NOT_CONFIGURED";
    case UWBAPI_STATUS_SESSIONS_ONGOING:
        return "UWBAPI_STATUS_SESSIONS_ONGOING";
    case UWBAPI_STATUS_MULTICAST_LIST_FULL:
        return "UWBAPI_STATUS_MULTICAST_LIST_FULL";
    case UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT:
        return "UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT";
    case UWBAPI_STATUS_ESE_RESET:
        return "UWBAPI_STATUS_ESE_RESET";
    case UWBAPI_STATUS_NO_CREDIT_AVAILABLE:
        return "UWBAPI_STATUS_NO_CREDIT_AVAILABLE";
    case UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED:
        return "UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED";
    case UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED:
        return "UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED";
    case UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_SET_AS_INITIATOR:
        return "UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_SET_AS_INITIATOR";
    case UWBAPI_STATUS_BUFFER_OVERFLOW:
        return "UWBAPI_STATUS_BUFFER_OVERFLOW";
    case UWBAPI_STATUS_PBF_PKT_SENT:
        return "UWBAPI_STATUS_PBF_PKT_SENT";
    case UWBAPI_STATUS_HPD_WAKEUP:
        return "UWBAPI_STATUS_HPD_WAKEUP";
    case UWBAPI_STATUS_TIMEOUT:
        return "UWBAPI_STATUS_TIMEOUT";
    case UWBAPI_STATUS_ESE_ERROR:
        return "UWBAPI_STATUS_ESE_ERROR";
    case UWBAPI_STATUS_SUSPEND:
        return "UWBAPI_STATUS_SUSPEND";
    case UWBAPI_STATUS_SESSION_RSN_RADAR_RFRI_INVALID:
        return "UWBAPI_STATUS_SESSION_RSN_RADAR_RFRI_INVALID";
    case UWBAPI_STATUS_UNKNOWN:
        return "UWBAPI_STATUS_UNKNOWN";
    default:
        return "STATUS NOT AWARE";
    }
}

#if UWBFTR_DataTransfer
EXTERNC tUWBAPI_STATUS sendData(uint8_t *pData, uint16_t len, uint8_t llMode)
{
    tUWBAPI_STATUS status    = UWBAPI_STATUS_FAILED;
    uint16_t packetLength    = 0;
    uint8_t offset           = 0;
    bool isFirstSegment      = true;
    uint8_t pbf              = 1;
    uint8_t hdrLen           = SEND_DATA_HEADER_LEN;
    uint8_t pkt3             = FALSE; // This flag is used to indicate 3rd packet
    bool dataTransferOngoing = false;
    uint16_t eventId         = 0;
    uint32_t connectionId    = 0;
    uint16_t sequenceNumber  = 0;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pData == NULL) {
        NXPLOG_UWBAPI_E("%s: pData is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (llMode == Link_Layer_Mode_Bypass) {
        eventId                   = UWA_DM_API_SEND_DATA_EVENT;
        phUwbDataPkt_t *pSendData = (phUwbDataPkt_t *)pData;

        connectionId   = pSendData->sessionHandle;
        sequenceNumber = pSendData->sequence_number;
    }
    else if (llMode == Link_Layer_Mode_LogicalLink) {
        eventId                             = UWA_DM_API_LOGICAL_LINK_SEND_DATA_EVENT;
        phLogicalLinkDataPkt_t *pllSendData = (phLogicalLinkDataPkt_t *)pData;

        connectionId   = pllSendData->llConnectId;
        sequenceNumber = pllSendData->sequence_number;
    }
    else {
        NXPLOG_UWBAPI_E("%s:%d: Invalid Link Layer Mode", __FUNCTION__, __LINE__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    while (len > 0) {
        // Only for the first packet we should send header data
        if (!isFirstSegment) {
            hdrLen = 0;
        }

        // For the first packet in case of fragmentation, we should send header data along with MAX_DATA_PACKET_PAYLOAD_SIZE
        if (len > uwbContext.maxDataPacketPayloadSize + hdrLen) {
            packetLength = uwbContext.maxDataPacketPayloadSize + hdrLen;
        }
        else {
            // Incase of single packet, we should send the entire length including header
            pbf          = 0;
            packetLength = len;
        }

        // For 1st packet offset should be 0, for 2nd packet offset should be MAX_DATA_PACKET_PAYLOAD_SIZE + 16
        if (pkt3 == FALSE) {
            if (!isFirstSegment) {
                offset += uwbContext.maxDataPacketPayloadSize + SEND_DATA_HEADER_LEN;
                pkt3 = TRUE;
            }
        }
        else {
            // From 3rd packet onwards, offset should be MAX_DATA_PACKET_PAYLOAD_SIZE
            offset += uwbContext.maxDataPacketPayloadSize;
        }

        status = sendUciCommand(eventId, packetLength, &uwbContext.snd_data[offset], pbf);
        if (status == UWBAPI_STATUS_OK) {
            /**
             * Normal flow handling is skipped
             */
            if (pbf) {
                status = waitforNotification(UWA_DM_DATA_CREDIT_STATUS_EVT, UWB_NTF_TIMEOUT);
                if ((status != UWBAPI_STATUS_OK) || (uwbContext.dataCredit.connectionId != connectionId) ||
                    (uwbContext.dataCredit.credit_availability != UCI_STATUS_CREDIT_AVAILABLE)) {
                    status = UWBAPI_STATUS_FAILED;
                    NXPLOG_UWBAPI_E("%s: status: %d", __FUNCTION__, status);
                    break;
                }
            }
        }
        else {
            status = UWBAPI_STATUS_FAILED;
            NXPLOG_UWBAPI_E("%s: status: %d", __FUNCTION__, status);
            break;
        }

        len -= packetLength;
        isFirstSegment = false;
    }
    if (status == UWBAPI_STATUS_OK) {
        /** As Per CR-490
         *  If SESSION_DATA_TRANSFER_STATUS_NTF is disabled, then the UWBS shall not send SESSION_DATA_TRANSFER_STATUS_NTF for every Application Data transmission except for last.
         *  SESSION_DATA_TRANSFER_STATUS_NTF is 'enabled' and 'DATA_REPETITION_COUNT > 0' it indicates that one Data transmission is completed in a RR.
         *  if above configs are enabled then UWBS shall send 'STATUS_REPETITION_OK' after each Application Data transmission completion except for the last Application Data transmission.
         * EX: SESSION_DATA_TRANSFER_STATUS_NTF = 1 and DATA_REPETITION_COUNT = 5
         *     We receive 5 data Transmit notifications , in which 4 are with status 'STATUS_REPETITION_OK' and last ntf with STATUS_OK.
         *     uwbContext.dataTransmit.txcount is 5 (DATA_REPETITION_COUNT = 5).
         *  by default 'dataTransferOngoing' is enabled to check at least one Transmit notification.
        */
        do {
            /* wait fot the  Data Transmit ntf (0x62(GID), 0x05(OID))*/
            status = waitforNotification(UWA_DM_DATA_TRANSMIT_STATUS_EVT, UWBD_TRANSMIT_NTF_TIMEOUT);
            if (status == UWBAPI_STATUS_OK) {
#if (UWBIOT_UWBD_SR04X)
                if ((uwbContext.dataTransmit.transmitNtf_status == UWBAPI_STATUS_OK) &&
                    (uwbContext.dataTransmit.transmitNtf_connectionId == connectionId) &&
                    (uwbContext.dataTransmit.transmitNtf_sequence_number == sequenceNumber)) {
                    status = UWBAPI_STATUS_OK;
                }
                else {
                    NXPLOG_UWBAPI_E("%s: status: %d", __FUNCTION__, uwbContext.dataTransmit.transmitNtf_status);
                    status = UWBAPI_STATUS_FAILED;
                }
                dataTransferOngoing = FALSE;
#else
                /**
                 * check for the  Data transmission status.
                 * status is UWBAPI_DATA_TRANSFER_STATUS_OK then Data transmission is completed.
                 */
                if ((uwbContext.dataTransmit.transmitNtf_status == UWBAPI_DATA_TRANSFER_STATUS_OK) &&
                    (uwbContext.dataTransmit.transmitNtf_connectionId == connectionId) &&
                    (uwbContext.dataTransmit.transmitNtf_sequence_number == sequenceNumber)) {
                    /* disable dataTransferOngoing flag it is last Data transmission". */
                    NXPLOG_UWBAPI_I("%s: Tansmit-status: %d txcount:%d",
                        __FUNCTION__,
                        uwbContext.dataTransmit.transmitNtf_status,
                        uwbContext.dataTransmit.transmitNtf_txcount);
                    dataTransferOngoing = FALSE;
                }
                else if (uwbContext.dataTransmit.transmitNtf_status == UWBAPI_DATA_TRANSFER_STATUS_REPETITION_OK) {
                    /* Transmission status UWBAPI_DATA_TRANSFER_STATUS_REPETITION_OK indicates Data transmission is ongoing.
                    *  Enable dataTransferOngoing flag  till last Data transmission with status code "UWBAPI_DATA_TRANSFER_STATUS_OK".
                    */
                    dataTransferOngoing = TRUE;
                    NXPLOG_UWBAPI_I("%s: Tansmit-status: %d txcount:%d",
                        __FUNCTION__,
                        uwbContext.dataTransmit.transmitNtf_status,
                        uwbContext.dataTransmit.transmitNtf_txcount);
                }
                else if (uwbContext.dataTransmit.transmitNtf_status == UWBAPI_DATA_TRANSFER_DATA_TRANSFER_IS_ONGOING) {
                    /* Application Data is being transmitted and the number of configured DATA_REPETITION_COUNT transmissions is not yet completed.*/
                    dataTransferOngoing = TRUE;
                    NXPLOG_UWBAPI_W("Transmit status is ongoing");
                    NXPLOG_UWBAPI_I("%s: Tansmit-status: %d txcount:%d",
                        __FUNCTION__,
                        uwbContext.dataTransmit.transmitNtf_status,
                        uwbContext.dataTransmit.transmitNtf_txcount);
                }
                else {
                    /* Apart from 'STATUS_REPETITION_OK','STATUS_REPETITION_OK'and 'UWBAPI_DATA_TRANSFER_DATA_TRANSFER_IS_ONGOING' rest of the status treated as failure .*/
                    NXPLOG_UWBAPI_E(
                        "%s: UWB TRANSMIT STATUS : %d", __FUNCTION__, uwbContext.dataTransmit.transmitNtf_status);
                    dataTransferOngoing = FALSE;
                }
#endif /* (UWBIOT_UWBD_SR04X) */
            }
            else {
                /**
                 * NTF timeout(UWBD_TRANSMIT_NTF_TIMEOUT) occured .
                 * Not able to receive the UWA_DM_DATA_TRANSMIT_STATUS_EVT 0x62(GID), 0x05(OID) notification.
                 *
                */
                NXPLOG_UWBAPI_E("%s: UWBD_TRANSMIT_NTF_TIMEOUT with staus: %d", __FUNCTION__, status);
                dataTransferOngoing = FALSE;
                status              = UWBAPI_STATUS_FAILED;
            }
        } while (dataTransferOngoing);

        /**
         * Check for the status success .
         * In case of failure do not exit ,wait for the Credit norification.
         * Even staus fail , still Host Receives the Credit Ntf.
         */
        if (UWBAPI_STATUS_OK != status) {
            LOG_W("Transmit notification fails still wait for the credit notificaiton");
        }

        status = waitforNotification(UWA_DM_DATA_CREDIT_STATUS_EVT, UWB_NTF_TIMEOUT);
        if ((status != UWBAPI_STATUS_OK) || (uwbContext.dataCredit.connectionId != connectionId)) {
            /**
             * After failure of the Credit notification Donot exit .
             * Transmit notification only treating as Api status .
             * irrespective of the credit ntf , Need to update the Transmit ntf .
            */
            NXPLOG_UWBAPI_E(
                "%s: UWB_NTF_TIMEOUT for UWA_DM_DATA_CREDIT_STATUS_EVT with status: %d", __FUNCTION__, status);
        }
        /**
         * Indicate the Api status fail , incase of the Trasmit notification failure .
         * Other than success,Repetation and Ongoing , return the status as is.
         */
        if (UWB_DATA_TRANSFER_STATUS != uwbContext.dataTransmit.transmitNtf_status) {
            NXPLOG_UWBAPI_E("%s: UWBD TRANSMIT NTF FAILED with status: %d",
                __FUNCTION__,
                uwbContext.dataTransmit.transmitNtf_status);
            status = uwbContext.dataTransmit.transmitNtf_status;
        }
        if (NULL != uwbContext.pAppCallback) {
            uwbContext.pAppCallback(
                UWBD_DATA_TRANSMIT_NTF, (void *)&uwbContext.dataTransmit, sizeof(uwbContext.dataTransmit));
        }
        else {
            NXPLOG_UWBAPI_E("%s: %d pAppCallback is NULL", __FUNCTION__, __LINE__);
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif // UWBFTR_DataTransfer