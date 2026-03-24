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

#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "uwa_api.h"
#include "phOsalUwb.h"
#include "uci_ext_defs.h"
#include "UwbAdaptation.h"
#include "AppConfigParams.h"
#include "PrintUtility.h"
#include "uwa_api.h"
#include "uwa_dm_int.h"
#include "UwbApi_Utility.h"
#include "UwbApi.h"
#include "uwbiot_ver.h"
#include "phNxpUwbConfig.h"
#if UWBIOT_UWBD_SR1XXT_SR2XXT
#include "phNxpUciHal_fwd.h"
#include "phTmlUwb_transport.h"
#include "uwb_fwdl_provider.h"
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#if (UWBFTR_SE_SE051W)
#include "StateMachine.h"
#endif

EXTERNC tUWBAPI_STATUS UwbApi_GetDeviceInfo(phUwbDevInfo_t *pdevInfo)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter; ", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pdevInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pdevInfo is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = getDeviceInfo();

    if (status == UWBAPI_STATUS_OK) {
        if (parseDeviceInfo(pdevInfo, uwbContext.rsp_data, uwbContext.rsp_len) == FALSE) {
            NXPLOG_UWBAPI_E("%s: Parsing Device Information Failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Parsing Device Information Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Parsing Device Information failed with status %s (0x%x)",
            __FUNCTION__,
            getStatusString(status),
            status);
        status = UWBAPI_STATUS_FAILED;
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_DoChipCalibration(uint8_t channel, phDoCalibNtfStatus_t *pDoCalibStatus)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    uint32_t index = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pDoCalibStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: pDoCalibStatus is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_VENDOR_DO_CHIP_CALIBRATION_RESP_EVT);
    cmdLen = serializeDoChipCalibPayload(channel, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_DO_CHIP_CALIBRATION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_PROP_DO_CHIP_CALIBRATION_NTF_EVT, UWBD_DO_CALIB_NTF_TIMEOUT);

        if (status == UWBAPI_STATUS_OK) {
            if (0 != uwbContext.rsp_len)
            {
                uint8_t *p = uwbContext.rsp_data;
                UWB_STREAM_TO_UINT8(pDoCalibStatus->calibNtfStatus, p, index);
                if (pDoCalibStatus->calibNtfStatus == UWBAPI_STATUS_OK) {
                    UWB_STREAM_TO_UINT16(pDoCalibStatus->calibNtfValue, p, index);
                    NXPLOG_UWBAPI_D("%s: Do Chip Calibration notification successful", __FUNCTION__);
                }
                else {
                    status = pDoCalibStatus->calibNtfStatus;
                    NXPLOG_UWBAPI_E("%s: Do Chip Calibration notification failed", __FUNCTION__);
                }
            }
            else{
                status = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_E("%s: Do Chip Calib ntf Invalid length", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Do Chip Calibration notification time out", __FUNCTION__);
        }
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Do Chip Calibration failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SetCalibration(
    uint8_t channel, eCalibParam paramId, uint8_t *calibrationValue, uint16_t length)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    switch (paramId) {
    case CHIP_CALIBRATION:
    case RF_CLK_ACCURACY_CALIB:
    case RX_ANT_DELAY_CALIB:
#if UWBFTR_AoA_FoV
    case PDOA_OFFSET_CALIB:
#endif // UWBFTR_AoA_FoV
    case TX_POWER_PER_ANTENNA:
    case MANUAL_TX_POW_CTRL:
#if UWBFTR_AoA_FoV
    case AOA_ANTENNAS_PDOA_CALIB:
    case PDOA_MANUFACT_ZERO_OFFSET_CALIB:
    case AOA_THRESHOLD_PDOA:
#endif // UWBFTR_AoA_FoV
    case TX_TEMPERATURE_COMP_PER_ANTENNA:
    case RSSI_CALIB_CONSTANT_PER_ANTENNA:
#if UWBIOT_UWBD_SR1XXT
    case PA_PPA_CALIB_CTRL:
#endif // UWBIOT_UWBD_SR1XXT
#if UWBFTR_AoA_FoV
#if UWBIOT_UWBD_SR1XXT
    case AOA_ANTENNAS_MULTIPOINT_CALIB:
#endif // UWBIOT_UWBD_SR1XXT
    case AOA_ANTENNAS_PDOA_CALIB_EXTENDED_SUPPORT:
#endif // UWBFTR_AoA_FoV
#if UWBIOT_UWBD_SR2XXT
    case AOA_PHASEFLIP_ANTSPACING:
    case PLATFORM_ID:
    case CONFIG_VERSION:
    case TX_ANT_DELAY_CALIB:
    case TRA2_LOFT_CALIB:
    case TRA1_LOFT_CALIB:
#endif // UWBIOT_UWBD_SR2XXT
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid calibration parameter %0X ", __FUNCTION__, paramId);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (calibrationValue == NULL || length == 0) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (length > (MAX_CMD_BUFFER_DATA_TRANSFER - 3)) { // channel +  T(ag)calibParam + L(ength) + V(alue)calib data
        NXPLOG_UWBAPI_E("%s: calibration data is more that MAX_CMD_BUFFER_TRANSFER", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_VENDOR_SET_DEVICE_CALIBRATION_RESP_EVT);

    cmdLen = serializeSetCalibPayload(channel, paramId, calibrationValue, length, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Set Calibration successful", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Set Calibration failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetCalibration(phGetCalibInputParams_t *pCalibInput, phCalibRespStatus_t *pCalibResp)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((NULL == pCalibInput) || (NULL == pCalibResp)) {
        NXPLOG_UWBAPI_E(
            "%s: Invalid parameter => pCalibInput: 0x%p pCalibResp : 0x%p", __FUNCTION__, pCalibInput, pCalibResp);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (NULL == pCalibResp->pCalibrationValue) {
        NXPLOG_UWBAPI_E("%s: Invalid parameter => pCalibrationValue is NULL ", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    switch (pCalibInput->paramId) {
    case CHIP_CALIBRATION:
    case RF_CLK_ACCURACY_CALIB:
    case RX_ANT_DELAY_CALIB:
#if UWBFTR_AoA_FoV
    case PDOA_OFFSET_CALIB:
#endif // UWBFTR_AoA_FoV
    case TX_POWER_PER_ANTENNA:
    case MANUAL_TX_POW_CTRL:
#if UWBFTR_AoA_FoV
    case AOA_ANTENNAS_PDOA_CALIB:
    case PDOA_MANUFACT_ZERO_OFFSET_CALIB:
    case AOA_THRESHOLD_PDOA:
#endif // UWBFTR_AoA_FoV
    case TX_TEMPERATURE_COMP_PER_ANTENNA:
    case RSSI_CALIB_CONSTANT_PER_ANTENNA:
#if UWBIOT_UWBD_SR1XXT
    case PA_PPA_CALIB_CTRL:
#endif // UWBIOT_UWBD_SR1XXT
#if UWBFTR_AoA_FoV
    case AOA_ANTENNAS_PDOA_CALIB_EXTENDED_SUPPORT:
#if UWBIOT_UWBD_SR1XXT
    case AOA_ANTENNAS_MULTIPOINT_CALIB:
#endif // UWBIOT_UWBD_SR1XXT
#endif // UWBFTR_AoA_FoV
#if UWBIOT_UWBD_SR2XXT
    case AOA_PHASEFLIP_ANTSPACING:
    case PLATFORM_ID:
    case CONFIG_VERSION:
    case TX_ANT_DELAY_CALIB:
    case TRA2_LOFT_CALIB:
    case TRA1_LOFT_CALIB:
    case CHIP_CALIBRATION_STATE:
#endif // UWBIOT_UWBD_SR2XXT
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid calibration parameter ", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_VENDOR_GET_DEVICE_CALIBRATION_RESP_EVT);
    cmdLen = serializeGetCalibPayload(pCalibInput, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_GET_DEVICE_CALIBRATION, cmdLen, uwbContext.snd_data);

    if ((status == UWBAPI_STATUS_OK) && (0 != uwbContext.rsp_len)) {
        deserializeGetCalibResp(pCalibResp, uwbContext.rsp_data);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Get Calibration value failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SetDebugParams(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_DebugParams_List_t *DebugParams_List)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen                  = 0;
    uint8_t paramLen                 = 0;
    uint8_t flag_for_data_logger_ntf = false;
    UWB_DBG_CFG_t paramId;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    UWB_Debug_Params_value_t output_param_value;

    /* Check if the device is initialized or not */
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    /* Check if the passed list is having parameters or not */
    if ((DebugParams_List == NULL) || ((noOfparams == 0))) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Assign the buffer for storing all the configs */
    output_param_value.param_value = uwbContext.snd_data;

    for (uint32_t LoopCnt = 0; LoopCnt < noOfparams; ++LoopCnt) {
        paramId = DebugParams_List[LoopCnt].param_id;

        if (paramId >= END_OF_SUPPORTED_EXT_DEBUG_CONFIGS) {
            NXPLOG_UWBAPI_E("%s: Invalid Parameter value", __FUNCTION__);
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#if UWBIOT_UWBD_SR2XXT
        if (paramId == kUWB_DBG_CFG_DATA_LOGGER_NTF) {
            flag_for_data_logger_ntf = true;
        }
#endif // UWBIOT_UWBD_SR2XXT

        /* parse and get input length and pointer */
        if (DebugConfig_TlvParser(&DebugParams_List[LoopCnt], &output_param_value) != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: DebugConfig_TlvParser() failed", __FUNCTION__);
            return UWBAPI_STATUS_FAILED;
        }

        // we have only extended debug configs
        paramLen = (uint16_t)(paramLen + getVendorDebugConfigTLVBuffer(paramId,
                                             (void *)(output_param_value.param_value),
                                             output_param_value.param_len,
                                             &uwbContext.rsp_data[payloadOffset + paramLen]));
    }

    if (paramLen == 0) {
        NXPLOG_UWBAPI_E("%s: getVendorDebugConfigTLVBuffer() failed", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, paramLen, uwbContext.rsp_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.rsp_data);

    if ((status == UWBAPI_STATUS_OK) && flag_for_data_logger_ntf) {
        InputOutputData_t ioData = {
            .enableFwDump = TRUE,
            .enableCirDump = FALSE
        };

        const tHAL_UWB_ENTRY *halFuncEntries = GetHalEntryFuncs();
        if (halFuncEntries) {
            tHAL_UWB_IOCTL ioCtl = {
                .pIoData = &ioData
            };
            halFuncEntries->ioctl(HAL_UWB_IOCTL_DUMP_FW_LOG, &ioCtl);
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetDebugParams(
    uint32_t sessionHandle, uint8_t noOfparams, UWB_DebugParams_List_t *DebugParams_List)
{
    tUWBAPI_STATUS status;
    uint8_t i = 0;
    UWB_DBG_CFG_t paramId;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint16_t cmdLen         = 0;
    uint8_t offset          = 0;
    uint8_t *rspPtr         = NULL;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((DebugParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pConfigCommand = &uwbContext.snd_data[payloadOffset];
    for (i = 0; i < noOfparams; i++) {
        paramId = DebugParams_List[i].param_id;

        if (paramId >= END_OF_SUPPORTED_EXT_DEBUG_CONFIGS) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }

        // we have only extended debug configs
        pConfigCommand[cmdLen++] = (uint8_t)(paramId & 0xFF);
    }

    sep_SetWaitEvent(UWA_DM_SESSION_GET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, cmdLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        rspPtr = &uwbContext.rsp_data[offset];
        parseDebugParams(rspPtr, noOfparams, DebugParams_List);
    }
    return status;
}

#if UWBIOT_SESN_SNXXX
EXTERNC tUWBAPI_STATUS UwbApi_PerformBinding(phSeDoBindStatus_t *doBindStatus)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (doBindStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: doBindStatus is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_PROP_DO_BIND_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_DO_BIND, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        sep_SetWaitEvent(UWA_DM_VENDOR_SE_DO_BIND_NTF_EVT);
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWBD_SE_TIMEOUT) == UWBSTATUS_SUCCESS) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(doBindStatus->status, p, index);
            if (doBindStatus->status != UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_W(
                    "%s: Get doBindStatus is not success, status is %0x", __FUNCTION__, doBindStatus->status);
            }
            UWB_STREAM_TO_UINT8(doBindStatus->count_remaining, p, index);
            UWB_STREAM_TO_UINT8(doBindStatus->binding_state, p, index);
            UWB_STREAM_TO_UINT16(doBindStatus->se_instruction_code, p, index);
            UWB_STREAM_TO_UINT16(doBindStatus->se_error_status, p, index);
        }
        else {
            NXPLOG_UWBAPI_E("%s: DoBind status is failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Do Binding Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Binding is failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //(UWBIOT_SESN_SNXXX)

#if (UWBFTR_SE_SE051W)
EXTERNC tUWBAPI_STATUS UwbApi_PerformBinding(void)
{
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    return Binding_Process();
}

EXTERNC tUWBAPI_STATUS UwbApi_PerformLocking(void)
{
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    return locking_Process();
}
#endif // UWBFTR_SE_SE051W

#if (UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)
EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfigWrappedRDS(uint32_t sessionHandle, uint8_t *pWrappedRds, size_t WrappedRdsLen)
{
    tUWBAPI_STATUS uwb_status = UWBAPI_STATUS_FAILED;
    uint16_t cmdLen           = 0;
    uint8_t payloadOffset     = 0;

    uint32_t WrappedRdsLenU32 = (uint32_t)WrappedRdsLen;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        uwb_status = UWBAPI_STATUS_NOT_INITIALIZED;
        goto exit;
    }

    if (pWrappedRds == NULL) {
        NXPLOG_UWBAPI_E("%s: pWrappedRds is NULL", __FUNCTION__);
        uwb_status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    cmdLen                        = serializeAppConfigPayload(sessionHandle, 1, payloadOffset, uwbContext.snd_data);
    uwbContext.snd_data[cmdLen++] = UCI_VENDOR_PARAM_ID_WRAPPED_RDS;
    uwbContext.snd_data[cmdLen++] = (uint8_t)WrappedRdsLen;
    phOsalUwb_MemCopy(&uwbContext.snd_data[cmdLen], (void *)pWrappedRds, WrappedRdsLenU32);
    cmdLen += (uint16_t)WrappedRdsLen;

    sep_SetWaitEvent(UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT);
    uwb_status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, uwb_status);
    return uwb_status;
}

EXTERNC tUWBAPI_STATUS UwbApi_ReadModuleMakerInfo(uint16_t *pModuleMakerInfo)
{
    tUWBAPI_STATUS status;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pModuleMakerInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: output params is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_READ_MODULE_MAKER_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_READ_MODULE_MAKER_EVT, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Read module maker info command successful", __FUNCTION__);
        if (uwbContext.rsp_len > 0) {
            *pModuleMakerInfo = uwbContext.rsp_data[1] << 0 | uwbContext.rsp_data[2] << 8;
        }
        else {
            NXPLOG_UWBAPI_E("%s: Received wrong data ", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Read module maker Command failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
    }
    return status;
}
#endif // (UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)


#if (UWBIOT_UWBD_SR150 && UWBFTR_FactoryMode) || (UWBIOT_UWBD_SR250)
EXTERNC tUWBAPI_STATUS UwbApi_WriteModuleMakerInfo(uint16_t ModuleMakerInfo)
{
    tUWBAPI_STATUS status;
    uint8_t offset = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    UWB_UINT16_TO_STREAM(uwbContext.snd_data, ModuleMakerInfo, offset);
    sep_SetWaitEvent(UWA_DM_WRITE_MODULE_MAKER_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_WRITE_MODULE_MAKER_EVT, offset, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Write module maker Command successful", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Write module maker Command failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
    }
    return status;
}
#endif // (UWBIOT_UWBD_SR150 && UWBFTR_FactoryMode) || (UWBIOT_UWBD_SR250)

EXTERNC tUWBAPI_STATUS UwbApi_GetBindingCount(phSeGetBindingCount_t *getBindingCount)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint32_t index = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (getBindingCount == NULL) {
        NXPLOG_UWBAPI_E("%s: getBindingCount is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_GET_BINDING_COUNT_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_GET_BINDING_COUNT, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Binding Count is successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(status, rspPtr, index);
        if (status == UWBAPI_STATUS_OK) {
            UWB_STREAM_TO_UINT8(getBindingCount->bindingStatus, rspPtr, index);
            UWB_STREAM_TO_UINT8(getBindingCount->uwbdBindingCount, rspPtr, index);
            UWB_STREAM_TO_UINT8(getBindingCount->seBindingCount, rspPtr, index);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get Binding Count Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Binding Count is failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#if (UWBIOT_SESN_SNXXX)
EXTERNC tUWBAPI_STATUS UwbApi_TestConnectivity(SeConnectivityStatus_t *ConnectivityStatus)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint32_t index = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (ConnectivityStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: ConnectivityStatus is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_TEST_CONNECTIVITY_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_TEST_CONNECTIVITY, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_VENDOR_SE_DO_TEST_CONNECTIVITY_NTF_EVT, UWB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(ConnectivityStatus->status, p, index);
            if (ConnectivityStatus->status != UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_W(
                    "%s: Get ConnectivityStatus is not success %0x", __FUNCTION__, ConnectivityStatus->status);
            }
            UWB_STREAM_TO_UINT16(ConnectivityStatus->se_instruction_code, p, index);
            UWB_STREAM_TO_UINT16(ConnectivityStatus->se_error_status, p, index);
            NXPLOG_UWBAPI_D("%s: Get ESE Test cmd passed", __FUNCTION__);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Get  ESE Test cmd failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Connectivity test Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Connectivity test response is failed with status %s (0x%x)",
            __FUNCTION__,
            getStatusString(status),
            status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SeTestLoop(uint16_t loopCnt, uint16_t timeInterval, phTestLoopData_t *testLoopData)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (testLoopData == NULL) {
        NXPLOG_UWBAPI_E("%s: testLoopData is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    sep_SetWaitEvent(UWA_DM_PROP_SE_TEST_LOOP_RESP_EVT);
    cmdLen = serializeSeLoopTestPayload(loopCnt, timeInterval, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_TEST_SE_LOOP, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(
            UWA_DM_PROP_SE_TEST_LOOP_NTF_EVT, (uint32_t)((uint32_t)(loopCnt * timeInterval) + UWBD_SE_TIMEOUT));
        /*
         * Increasing the delay as it is not sufficient to get the notification.
         */
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(testLoopData->status, p, index);
            if (testLoopData->status == UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_OK;
                UWB_STREAM_TO_UINT16(testLoopData->loop_cnt, p, index);
                UWB_STREAM_TO_UINT16(testLoopData->loop_pass_count, p, index);
                NXPLOG_UWBAPI_D("%s: Loop test is successful", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Loop test is failed", __FUNCTION__);
                status = UWBAPI_STATUS_FAILED;
            }
        }
        else {
            testLoopData->status = 0xFF;
            NXPLOG_UWBAPI_E("%s: Loop test is failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Loop test Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Loop test is failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetBindingStatus(phSeGetBindingStatus_t *getBindingStatus)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (getBindingStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: getBindingStatus is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_GET_BINDING_STATUS_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_GET_BINDING_STATUS, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_VENDOR_ESE_BINDING_CHECK_NTF_EVT, UWBD_SE_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(getBindingStatus->status, p, index);
            if (getBindingStatus->status != 0x02) {
                status = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_W(
                    "Get binding status is NOT bound and Unlocked , status is %0x", getBindingStatus->status);
            }
            UWB_STREAM_TO_UINT8(getBindingStatus->se_binding_count, p, index);
            UWB_STREAM_TO_UINT8(getBindingStatus->uwbd_binding_count, p, index);
            UWB_STREAM_TO_UINT16(getBindingStatus->se_instruction_code, p, index);
            UWB_STREAM_TO_UINT16(getBindingStatus->se_error_status, p, index);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Get binding status cmd failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get binding status Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Get binding status cmd failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_URSKdeletionRequest(
    uint8_t noOfSessionIds, uint32_t *pSessionIdList, phUrskDeletionRequestStatus_t *pUrskDeletionStatus)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (noOfSessionIds == 0) {
        NXPLOG_UWBAPI_E("%s: noOfSessionIds is 0", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (pSessionIdList == NULL || pUrskDeletionStatus == NULL) {
        NXPLOG_UWBAPI_E(
            "%s: pSessionIdList = 0x%p and pUrskDeletionStatus = 0x%p", __FUNCTION__, pSessionIdList, pUrskDeletionStatus);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_URSK_DELETION_REQUEST_RESP_EVT);
    cmdLen = serializeUrskDeletionRequestPayload(noOfSessionIds, pSessionIdList, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_URSK_DELETION_REQUEST, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_VENDOR_URSK_DELETION_REQ_NTF_EVT, UWB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *p = &uwbContext.rsp_data[0];
            UWB_STREAM_TO_UINT8(pUrskDeletionStatus->status, p, index);
            if (pUrskDeletionStatus->status == UWBAPI_STATUS_OK) {
                status = UWBAPI_STATUS_OK;
                UWB_STREAM_TO_UINT8(pUrskDeletionStatus->noOfSessionIds, p, index);
                for (int i = 0; i < pUrskDeletionStatus->noOfSessionIds; i++) {
                    UWB_STREAM_TO_UINT32(pUrskDeletionStatus->sessionIdList[i].sessionId, p, index);
                    UWB_STREAM_TO_UINT8(pUrskDeletionStatus->sessionIdList[i].status, p, index);
                }
            }
            else {
                pUrskDeletionStatus->status = 0xFF;
                NXPLOG_UWBAPI_E("%s: URSK deletion request ntf is failed", __FUNCTION__);
                status = UWBAPI_STATUS_FAILED;
            }
        }
        else {
            pUrskDeletionStatus->status = 0xFF;
            NXPLOG_UWBAPI_E("%s: URSK deletion request ntf not received", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: URSK deletion request Command Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: URSK deletion request is failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //(UWBIOT_SESN_SNXXX)

#if !(UWBIOT_UWBD_SR04X)
EXTERNC tUWBAPI_STATUS UwbApi_QueryTemperature(uint8_t *pTemperatureValue)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pTemperatureValue == NULL) {
        NXPLOG_UWBAPI_E("%s: pTemperatureValue is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_QUERY_TEMPERATURE_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_QUERY_TEMP, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Query temperature cmd successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(status, rspPtr, index);

        if (status == UWBAPI_STATUS_OK) {
            UWB_STREAM_TO_UINT8(*pTemperatureValue, rspPtr, index);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Query temperature cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Query temperature cmd failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif //!(UWBIOT_UWBD_SR04X)

#if (UWBIOT_UWBD_SR100T)
tUWBAPI_STATUS UwbApi_CalibrationIntegrityProtection(eCalibTagOption tagOption, uint16_t calibBitMask)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_PROP_CALIB_INTEGRITY_PROTECTION_RESP_EVT);
    cmdLen = serializecalibIntegrityProtectionPayload(tagOption, calibBitMask, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_CALIB_INTEGRITY_PROTECTION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Calibration Integrity Protection command successful", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Calibration Integrity Protection Command failed with status %s (0x%x)",
            __FUNCTION__,
            getStatusString(status),
            status);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

tUWBAPI_STATUS UwbApi_VerifyCalibData(uint8_t *pCmacTag, uint8_t tagOption, uint16_t tagVersion)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCmacTag == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_VERIFY_CALIB_DATA_RESP_EVT);
    cmdLen = serializeVerifyCalibDataPayload(pCmacTag, tagOption, tagVersion, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_PROP_VERIFY_CALIB_DATA, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        /** TODO: Need to handle in secure calibration implementation of SR250*/
        status = waitforNotification(EXT_UCI_MSG_VERIFY_CALIB_DATA, UWBD_GENERATE_TAG_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            status = uwbContext.wstatus;
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: Verify Calib Data successful", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Verify Calib Data failed", __FUNCTION__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Verify Calib Data notification time out", __FUNCTION__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: Verify Calib Data Command failed with status %s (0x%x)",
            __FUNCTION__,
            getStatusString(status),
            status);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif // UWBIOT_UWBD_SR100T
EXTERNC tUWBAPI_STATUS UwbApi_QueryUwbTimestamp(uint8_t len, uint8_t pTimestampValue[])
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((len < 8) || (pTimestampValue == NULL)) {
        NXPLOG_UWBAPI_E("%s: pTimestampValue is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_QUERY_TIMESTAMP_RESP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_QUERY_UWBS_TIMESTAMP, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Query timestamp cmd successful", __FUNCTION__);
        /* rsp_data contains complete rsp, we have to skip Header */
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        UWB_STREAM_TO_UINT8(status, rspPtr, index);

        if (status == UWBAPI_STATUS_OK) {
            if (uwbContext.rsp_len > (UCI_MSG_CORE_UWBS_TIMESTAMP_LEN + UCI_RESPONSE_PAYLOAD_OFFSET)) {
                NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __FUNCTION__);
                return UWBAPI_STATUS_BUFFER_OVERFLOW;
            }
            UWB_STREAM_TO_ARRAY(pTimestampValue, rspPtr, UCI_MSG_CORE_UWBS_TIMESTAMP_LEN, index);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Query timestamp cmd Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Query timestamp cmd failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#if UWBFTR_DL_TDoA_Anchor
tUWBAPI_STATUS UwbApi_UpdateActiveRoundsAnchor(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    UWB_MacAddressMode_t macAddressingMode,
    const phActiveRoundsConfig_t roundConfigList[],
    phNotActivatedRounds_t *pNotActivatedRound)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint16_t cmdLen = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((!nActiveRounds) || (roundConfigList == NULL)) {
        NXPLOG_UWBAPI_E("%s: nActiveRounds is 0 or roundConfigList is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pNotActivatedRound == NULL) {
        NXPLOG_UWBAPI_E("%s: pNotActivatedRound is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_RSP_EVT);
    cmdLen = serializeUpdateActiveRoundsAnchorPayload(
        sessionHandle, nActiveRounds, macAddressingMode, roundConfigList, &uwbContext.snd_data[0]);
    if (cmdLen != 0) {
        status =
            sendUciCommandAndWait(UWA_DM_API_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_EVT, cmdLen, uwbContext.snd_data);
    }
    else {
        NXPLOG_UWBAPI_E("%s: responderSlots or dstMacAddr is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Update active rounds successful!", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED) {
        NXPLOG_UWBAPI_D("%s: One or more rounds couldn't be activated", __FUNCTION__);
        uint8_t *rspPtr               = &uwbContext.rsp_data[0];
        pNotActivatedRound->noOfIndex = (uint8_t)(uwbContext.rsp_len - sizeof(status));
        for (int i = 0; i < pNotActivatedRound->noOfIndex; i++) {
            pNotActivatedRound->indexList[i] = *rspPtr++;
        }
    }
    else if (status == UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED) {
        NXPLOG_UWBAPI_E("%s: Number of active rounds exceeded", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Update active rounds cmd failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif // UWBFTR_DL_TDoA_Anchor

#if UWBFTR_DL_TDoA_Tag
tUWBAPI_STATUS UwbApi_UpdateActiveRoundsReceiver(uint32_t sessionHandle,
    uint8_t nActiveRounds,
    const uint8_t RangingroundIndexList[],
    phNotActivatedRounds_t *pNotActivatedRound)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint16_t cmdLen = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((!nActiveRounds) || (RangingroundIndexList == NULL)) {
        NXPLOG_UWBAPI_E("%s: nActiveRounds is 0 or roundConfigList is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pNotActivatedRound == NULL) {
        NXPLOG_UWBAPI_E("%s: pNotActivatedRound is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_RSP_EVT);
    cmdLen = serializeUpdateActiveRoundsReceiverPayload(
        sessionHandle, nActiveRounds, RangingroundIndexList, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Update active rounds successful!", __FUNCTION__);
    }
    else if (status == UWBAPI_STATUS_ERROR_ROUND_INDEX_NOT_ACTIVATED) {
        NXPLOG_UWBAPI_D("%s: One or more rounds couldn't be activated", __FUNCTION__);
        uint8_t *rspPtr               = &uwbContext.rsp_data[0];
        pNotActivatedRound->noOfIndex = (uint8_t)(uwbContext.rsp_len - sizeof(status));
        for (int i = 0; i < pNotActivatedRound->noOfIndex; i++) {
            pNotActivatedRound->indexList[i] = *rspPtr++;
        }
    }
    else if (status == UWBAPI_STATUS_ERROR_NUMBER_OF_ACTIVE_RANGING_ROUNDS_EXCEEDED) {
        NXPLOG_UWBAPI_E("%s: Number of active rounds exceeded", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Update active rounds cmd failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif // UWBFTR_DL_TDoA_Tag

tUWBAPI_STATUS UwbApi_GetFwCrashLog(phFwCrashLogInfo_t *pLogInfo)
{
    tHAL_UWB_IOCTL ioCtl;
    const tHAL_UWB_ENTRY *halFuncEntries = NULL;

    if (pLogInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pLogInfo is null ", __FUNCTION__);
        return UWBAPI_STATUS_FAILED;
    }
    if (pLogInfo->pLog == NULL) {
        NXPLOG_UWBAPI_E("%s: buffer is null ", __FUNCTION__);
        return UWBAPI_STATUS_FAILED;
    }

    halFuncEntries   = GetHalEntryFuncs();
    ioCtl.pCrashInfo = pLogInfo;
    if ((halFuncEntries->ioctl(HAL_UWB_IOCTL_DUMP_FW_CRASH_LOG, &ioCtl)) == UWBSTATUS_SUCCESS) {
        return UWBAPI_STATUS_OK;
    }
    else {
        return UWBAPI_STATUS_FAILED;
    }
}

tUWBAPI_STATUS UwbApi_SetDefaultCoreConfigs()
{
    return setDefaultCoreConfigs();
}


#if UWBFTR_BlobParser

tUWBAPI_STATUS UwbApi_ConfigureData_iOS(uint8_t *pShareableData,
    uint16_t ShareableDataLength,
    phUwbProfileInfo_t *pProfileInfo,
    uint8_t noOfVendorAppParams,
    const UWB_VendorAppParams_List_t *VendorAppParams_List,
    uint8_t noOfDebugParams,
    const UWB_DebugParams_List_t *DebugParams_List)
{
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    tUWBAPI_STATUS status;

    if (pProfileInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pProfileInfo is invalid", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }
    if (pShareableData == NULL) {
        NXPLOG_UWBAPI_E("%s: pShareableData is invalid", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    if ((ShareableDataLength != TOTAL_PROFILE_BLOB_SIZE_v1_1) &&
        (ShareableDataLength != TOTAL_PROFILE_BLOB_SIZE_v1_0)) {
        NXPLOG_UWBAPI_E("%s: profile blob size should be %d or %d bytes",
            __FUNCTION__,
            TOTAL_PROFILE_BLOB_SIZE_v1_0,
            TOTAL_PROFILE_BLOB_SIZE_v1_1);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    pProfileInfo->profileId = kUWB_Profile_1;

    status = UwbApi_SetProfileParams(pShareableData, ShareableDataLength, pProfileInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SetProfileParams failed");
        goto exit;
    }

    if ((noOfVendorAppParams != 0) && (VendorAppParams_List != NULL)) {
        status = UwbApi_SetVendorAppConfigs(pProfileInfo->sessionHandle, noOfVendorAppParams, VendorAppParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetVendorAppConfigs failed");
            goto exit;
        }
    }
    if ((noOfDebugParams != 0) && (DebugParams_List != NULL)) {
        status = UwbApi_SetDebugParams(pProfileInfo->sessionHandle, noOfDebugParams, DebugParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetDebugParams failed");
            goto exit;
        }
    }

    status = UwbApi_StartRangingSession(pProfileInfo->sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

tUWBAPI_STATUS UwbApi_ConfigureData_Android(uint8_t *pUwbPhoneConfigData,
    uint16_t UwbPhoneConfigDataLen,
    phUwbProfileInfo_t *pProfileInfo,
    uint8_t noOfVendorAppParams,
    const UWB_VendorAppParams_List_t *VendorAppParams_List,
    uint8_t noOfDebugParams,
    const UWB_DebugParams_List_t *DebugParams_List)
{
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    tUWBAPI_STATUS status;
    uint32_t sessionHandle = 0;

    const uint8_t stsStatic[] = UWB_ANDROID_STATIC_STS;
    const uint8_t vendorId[]  = UWB_ANDROID_VENDOR_ID;

    phRangingParams_t inRangingParams = {0};
    UwbPhoneConfigData_t UwbPhoneConfig;
    UWB_AppParams_List_t SetAppParamsList[MAX_CONFIG_ID_NUM_CONFIGS];

    if (pUwbPhoneConfigData == NULL || pProfileInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: Phone Config Data or profile info is invalid", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    if (UwbPhoneConfigDataLen == SHAREABLE_DATA_HEADER_LENGTH_ANDROID) {
        serializeUwbPhoneConfigData(&UwbPhoneConfig, pUwbPhoneConfigData);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Phone Config Data length is invalid : %d", __FUNCTION__, UwbPhoneConfigDataLen);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    status = UwbApi_SessionInit(UwbPhoneConfig.session_id, UWBD_RANGING_SESSION, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    pProfileInfo->sessionHandle = sessionHandle;
    LOG_W("CONFIG_ID_%d", UwbPhoneConfig.config_id);
    switch (UwbPhoneConfig.config_id) {
    case UWB_CONFIG_ID_1: {
        /**
         * +----------------------+------------------------------------+
         * | UWB Params           | CONFIG_ID_1 (0x01)                 |
         * +======================+====================================+
         * | Multinode Mode       | Unicast                            |
         * | STS                  | Static                             |
         * | Ranging Method       | DS-TWR Deferred mode               |
         * | Ranging Interval     | 240ms                              |
         * | AOA_RESULT_REQ       | 0x01 (Enabled)                     |
         * | Slot Duration        | 2400                               |
         * | Slots per RR         | 10                                 |
         * | STS IV               | 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 |
         * | Vendor ID            | 0x08, 0x07                         |
         * | Number of Controlees | 1                                  |
         * +----------------------+------------------------------------+
         */
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_UniCast;

        SetAppParamsList[0] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 240);
        SetAppParamsList[1] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, ENABLED);
        SetAppParamsList[2] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10);

    } break;

    case UWB_CONFIG_ID_2: {
        /**
         * +----------------------+------------------------------------+
         * | UWB Params           | CONFIG_ID_2 (0x02)                 |
         * +======================+====================================+
         * | Multinode Mode       | One to many                        |
         * | STS                  | Static                             |
         * | Ranging Method       | DS-TWR Deferred mode               |
         * | Ranging Interval     | 200ms                              |
         * | AOA_RESULT_REQ       | 0x01 (Enabled)                     |
         * | Slot Duration        | 2400                               |
         * | Slots per RR         | 10                                 |
         * | STS IV               | 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 |
         * | Vendor ID            | 0x08, 0x07                         |
         * | Number of Controlees | 1                                  |
         * +----------------------+------------------------------------+
         */
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_OnetoMany;

        SetAppParamsList[0] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 200);
        SetAppParamsList[1] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, ENABLED);
        SetAppParamsList[2] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10);

    } break;

    case UWB_CONFIG_ID_3: {
        /**
         * +----------------------+------------------------------------+
         * | UWB Params           | CONFIG_ID_3(0x03)                  |
         * +======================+====================================+
         * | Multinode Mode       | Unicast                            |
         * | STS                  | Static                             |
         * | Ranging Method       | DS-TWR Deferred mode               |
         * | Ranging Interval     | 240ms                              |
         * | AOA_RESULT_REQ       | 0x00 (Disabled)                    |
         * | Slot Duration        | 2400                               |
         * | Slots per RR         | 10                                 |
         * | STS IV               | 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 |
         * | Vendor ID            | 0x08, 0x07                         |
         * | Number of Controlees | 1                                  |
         * +----------------------+------------------------------------+
         */
        inRangingParams.multiNodeMode = kUWB_MultiNodeMode_UniCast;

        SetAppParamsList[0] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 240);
        SetAppParamsList[1] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, DISABLED);
        SetAppParamsList[2] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 10);

    } break;

    default: {
        NXPLOG_UWBAPI_E("Config ID %d not supported", UwbPhoneConfig.config_id);
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    } break;
    }

    /* Common configs across config IDs 1, 2 and 3*/
    SetAppParamsList[3] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(SLOT_DURATION, 2400);
    SetAppParamsList[4] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_ARRAY(STATIC_STS_IV, &stsStatic[0], sizeof(stsStatic)); // Android shows [1, 2, 3, 4, 5, 6]
    SetAppParamsList[5] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_ARRAY(VENDOR_ID, &vendorId[0], sizeof(vendorId)); // Android shows [7, 8]
    SetAppParamsList[6] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, UwbPhoneConfig.preamble_id);
    SetAppParamsList[7] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, UwbPhoneConfig.channel_number);
    SetAppParamsList[8] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, 1);
    SetAppParamsList[9] = (UWB_AppParams_List_t)UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, UwbPhoneConfig.phone_mac_address, SHORT_MAC_ADDR_LEN);


    /* Set the Config_Id Parameters */
    status = UwbApi_SetAppConfigMultipleParams(
        pProfileInfo->sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    switch (UwbPhoneConfig.device_ranging_role) {
    case UWB_DEVICE_CONTROLLER: {
        inRangingParams.deviceRole = kUWB_DeviceRole_Initiator;
        inRangingParams.deviceType = kUWB_DeviceType_Controller;
    } break;

    case UWB_DEVICE_CONTROLEE: {
        inRangingParams.deviceRole = kUWB_DeviceRole_Responder;
        inRangingParams.deviceType = kUWB_DeviceType_Controlee;
    } break;

    default: {
        NXPLOG_UWBAPI_E("Role not supported");
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    } break;
    }

    inRangingParams.deviceMacAddr[0] = pProfileInfo->mac_addr[0];
    inRangingParams.deviceMacAddr[1] = pProfileInfo->mac_addr[1];

    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    status = UwbApi_SetRangingParams(pProfileInfo->sessionHandle, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    if ((noOfVendorAppParams != 0) && (VendorAppParams_List != NULL)) {
        status = UwbApi_SetVendorAppConfigs(pProfileInfo->sessionHandle, noOfVendorAppParams, VendorAppParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetVendorAppConfigs failed");
            goto exit;
        }
    }

    if ((noOfDebugParams != 0) && (DebugParams_List != NULL)) {
        status = UwbApi_SetDebugParams(pProfileInfo->sessionHandle, noOfDebugParams, DebugParams_List);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SetDebugParams failed");
            goto exit;
        }
    }

    status = UwbApi_StartRangingSession(pProfileInfo->sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

#endif // UWBFTR_BlobParser

#if UWBIOT_UWBD_SR2XXT

EXTERNC tUWBAPI_STATUS UwbApi_SetSecureCalibration(phSecureCalibParams_t *pSecureCalibParams)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (NULL == pSecureCalibParams) {
        NXPLOG_UWBAPI_E("%s: pSecureCalibParams is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (FALSE == validateSecureCalibParamId(pSecureCalibParams->calibParamId)) {
        NXPLOG_UWBAPI_E("%s: Caliberation parameter is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pSecureCalibParams->calibrationValue == NULL || pSecureCalibParams->length == 0) {
        NXPLOG_UWBAPI_E("%s: calibrationValue is 0x%p and length is %d", __FUNCTION__, pSecureCalibParams->calibrationValue, pSecureCalibParams->length);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_VENDOR_SET_SECURE_CALIB_RSP_EVT);

    cmdLen = serializeSetSecureCalibPayload(pSecureCalibParams, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_VENDOR_SET_SECURE_CALIBRATION, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Set Calibration successful", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: Set Calibration failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}
#endif // UWBIOT_UWBD_SR2XXT

#if UWBFTR_CSA
tUWBAPI_STATUS UwbApi_SessionSetLocZone(phSessionSetLocZone_t *pSetLocZone)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    uint16_t cmdLen = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        status = UWBAPI_STATUS_NOT_INITIALIZED;
        goto exit;
    }

    if (NULL == pSetLocZone) {
        NXPLOG_UWBAPI_E("%s: pSetLocZone is NULL", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_LOCALIZATION_ZONE_RSP_EVENT);
    cmdLen = serializeSessionSetLocZoneCmd(pSetLocZone, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_LOCALIZATION_ZONE_EVT, cmdLen, uwbContext.snd_data);

    if (UWBAPI_STATUS_OK != status) {
        NXPLOG_UWBAPI_E("%s: Session Set Localization Zone Failed!", __FUNCTION__);
        goto exit;
    }

exit:
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#endif // UWBFTR_CSA

#if UWBIOT_SESN_SNXXX
tUWBAPI_STATUS UwbApi_Se_GetSessionIdList(pSeGetSessionIdList_t *pSeGetSessionIdList)
{
    tUWBAPI_STATUS status;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pSeGetSessionIdList == NULL) {
        NXPLOG_UWBAPI_E("%s: pSeGetSessionIdList is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_GET_ESE_SESSION_ID_LIST_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_GET_ESE_SESSION_ID_LIST_EVT, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_PROP_GET_ESE_SESSION_ID_LIST_NTF_EVT, UWB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            uint8_t *rspData = uwbContext.rsp_data;
            /* Skip the status field */
            UWB_STREAM_TO_UINT8(status, rspData, index);
            uwbContext.rsp_len = uwbContext.rsp_len - sizeof(uint8_t);

            if (status == UWBAPI_STATUS_OK) {
                /* Copy the number of Session Ids */
                UWB_STREAM_TO_UINT8(pSeGetSessionIdList->numberOfSessionIds, rspData, index)
                /* Check if the received number of session IDs can be accomodated the list of sessionIds */
                if (pSeGetSessionIdList->numberOfSessionIds <= MAX_SESSION_ID_LIST) {
                    /* Copy the List of SessionIds */
                    UWB_STREAM_TO_ARRAY(pSeGetSessionIdList->SessionIdList,
                        rspData,
                        (pSeGetSessionIdList->numberOfSessionIds * sizeof(uint32_t)),
                        index);
                }
                else {
                    LOG_E("%s Number of SessionIDs exceeds MAX_SESSION_ID_LIST : %d",
                        __FUNCTION__,
                        pSeGetSessionIdList->numberOfSessionIds);
                    status = UWBAPI_STATUS_FAILED;
                    goto exit;
                }
            }
            else {
                LOG_E("%s GET_ESE_SESSION_ID_LIST_NTF Failed with status : 0x%X", __FUNCTION__, status);
                goto exit;
            }
        }
        else {
            NXPLOG_UWBAPI_E(
                "%s: waitforNotification failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
            goto exit;
        }
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: sendUciCommandAndWait failed with status %s (0x%x)", __FUNCTION__, getStatusString(status), status);
        goto exit;
    }
exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif // UWBIOT_SESN_SNXXX