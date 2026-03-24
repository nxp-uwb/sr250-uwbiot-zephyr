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

#include "UwbApi.h"
#include "AppConfigParams.h"
#include "uci_ext_defs.h"
#include "uwa_api.h"
#include "phNxpLogApis_UwbApi.h"
#include "phOsalUwb.h"
#include "uwa_api.h"
#include "phNxpUwbConfig.h"
#include "UwbAdaptation.h"
#include "uwa_dm_int.h"

#if (UWBFTR_SE_SE051W)
#include "SE_Wrapper.h"
#endif

#include "UwbApi_Utility.h"
#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary_Internal.h"
#include "uwb_int.h"

#if UWBIOT_UWBD_SR1XXT
#include <Mainline_Firmware.h>
#endif // UWBIOT_UWBD_SR1XXT
#if UWBIOT_UWBD_SR1XXT_SR2XXT
#include "uwb_fwdl_provider.h"
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

#define MAX_SUPPORTED_TDOA_REPORT_FREQ 22
#define MIN_TRNG_SIZE                  0x01
#define MAX_TRNG_SIZE                  0x10
#define CHANNEL_5                      5
#define CHANNEL_6                      6
#define CHANNEL_8                      8
#define CHANNEL_9                      9
#define SEND_DATA_HEADER_LEN           16 // 4(Session Handle) + 8(mac address) + 1(dst endpoint) + 1(seq no) + 2(data size)

/* Minimum number of controlees for time based ranging is 1*/
/*In case of contention based ranging, this could be 0*/
#define MIN_NUM_OF_CONTROLEES 1

EXTERNC tUWBAPI_STATUS UwbApi_Initialize(phUwbappContext_t *pAppCtx)
{
    tUWBAPI_STATUS status                 = UWBAPI_STATUS_INVALID_PARAM;
    tUwbApi_AppCallback *pCallbackGeneric = NULL;
    /*By Defaulut Operating mode Should be Default*/
    Uwb_operation_mode_t eOperatingMode = kOPERATION_MODE_default;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if (pAppCtx == NULL) {
        NXPLOG_UWBAPI_E("pAppCtx is Null");
        goto exit;
    }

    if ((pAppCtx->pCallback == NULL) && (pAppCtx->pTmlCallback == NULL)) {
        NXPLOG_UWBAPI_E("Atleast 1 appcallback should be set");
        goto exit;
    }
    if (pAppCtx->pCallback && pAppCtx->pTmlCallback) {
        LOG_E("Atmost one callback should be registered");
        goto exit;
    }

    uwbContext.fwMode = pAppCtx->fwImageCtx.fwMode;
    /* This check is needed, to allow FW download from external-flash, where in the fwImageCtx is NULL */
#if UWBIOT_UWBD_SR1XXT && UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    if (pAppCtx->fwImageCtx.fwImage == NULL || pAppCtx->fwImageCtx.fwImgSize == 0) {
        NXPLOG_UWBAPI_E("Cannot download the firmware without passing the firmware context");
        goto exit;
    }
#endif // UWBIOT_UWBD_SR1XXT && UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST

#if UWBIOT_UWBD_SR1XXT_SR2XXT
    if (uwb_fwdl_setFwImage(&pAppCtx->fwImageCtx) != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWBAPI_E("uwb_fwdl_setFwImage failed");
        return UWBAPI_STATUS_FAILED;
    }
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT

    if (pAppCtx->pCallback != NULL) {
        /*Register the Genric callback*/
        pCallbackGeneric = pAppCtx->pCallback;
        eOperatingMode   = kOPERATION_MODE_default;
    }
    else if (pAppCtx->pTmlCallback != NULL) {
        eOperatingMode = kOPERATION_MODE_mctt;
    }

    status = uwbInit(pCallbackGeneric, eOperatingMode);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("uwbInit failed");
        goto exit;
    }

    /* Set the default notification timeout */
    uwbContext.ntfTimeout = DEFAULT_NTF_TIMEOUT_MS;

    // Fira Test mode is enabled
    if (pAppCtx->pTmlCallback != NULL) {
        /* Do HAL call back register only after Uwb Init */
        HalRegisterAppCallback(pAppCtx->pTmlCallback);
    }

exit:
    NXPLOG_UWBAPI_D("%s: exit ", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetRegisteredAppCallback(tUwbApi_AppCallback **const pAppCallback)
{
    if (NULL == pAppCallback) {
        return UWBAPI_STATUS_FAILED;
    }
    if (0 == uwbContext.isUfaEnabled) {
        /** UWB is not initialized */
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    /** Assign the function pointer for application callback */
    *pAppCallback = uwbContext.pAppCallback;

    return UWBAPI_STATUS_OK;
}

EXTERNC tUWBAPI_STATUS UwbApi_RegisterAppCallback(tUwbApi_AppCallback *pAppCallback)
{
    if (NULL == pAppCallback) {
        return UWBAPI_STATUS_FAILED;
    }
    if (0 == uwbContext.isUfaEnabled) {
        /** UWB is not initialized */
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    /** Assign the function pointer for application callback */
    uwbContext.pAppCallback = pAppCallback;

    return UWBAPI_STATUS_OK;
}

EXTERNC tUWBAPI_STATUS UwbApi_SwitchToMCTTMode(tUwbApi_TMLDataCallback *pAppCtx)
{
    // Fira Test mode is enabled
    if (pAppCtx != NULL) {
        /* Do HAL call back register only after Uwb Init */
        HalRegisterAppCallback(pAppCtx);
    }
    else {
        return UWBAPI_STATUS_FAILED;
    }

    /* Set operating mode */
    Hal_setOperationMode(kOPERATION_MODE_mctt);

    return UWBAPI_STATUS_OK;
}

EXTERNC tUWBAPI_STATUS UwbApi_ShutDown()
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_NOT_INITIALIZED;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_D("%s: UWB device is already  deinitialized", __FUNCTION__);
        return status;
    }
    sep_SetWaitEvent(UWA_DM_DISABLE_EVT);
    status = UWA_Disable(TRUE); /* gracefull exit */
    if (status == UWBAPI_STATUS_OK) {
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(uwbContext.devMgmtSem, UWB_MAX_DEV_MGMT_RSP_TIMEOUT) !=
            UWBSTATUS_SUCCESS) {
            LOG_E("%s : UWA_DM_DISABLE_EVT timedout", __FUNCTION__);
            status = UWBAPI_STATUS_TIMEOUT;
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: De-Init is failed:", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }
    /*Cleanup done only once */
    cleanUp();
    phUwb_LogDeInit();
    /* clear the Se handle in SE wrapper*/
#if (UWBFTR_SE_SE051W)
    Se_API_ResetHandle();
#endif
    return status;
}

#if !(UWBIOT_UWBD_SR04X)
EXTERNC tUWBAPI_STATUS UwbApi_RecoverUWBS()
{
#if UWBIOT_UWBD_SR1XXT
    phUwbFWImageContext_t fwImageCtx;
    fwImageCtx.fwImage   = (uint8_t *)heliosEncryptedMainlineFwImage;
    fwImageCtx.fwImgSize = sizeof(heliosEncryptedMainlineFwImage);
    fwImageCtx.fwMode    = MAINLINE_FW;
    if (uwb_fwdl_setFwImage(&fwImageCtx) != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWBAPI_E("uwb_fwdl_setFwImage failed");
        return UWBAPI_STATUS_FAILED;
    }
#endif // UWBIOT_UWBD_SR1XXT
    uwbContext.fwMode = MAINLINE_FW;

    return recoverUWBS();
}

#endif //!(UWBIOT_UWBD_SR04X)

EXTERNC tUWBAPI_STATUS UwbApi_UwbdReset(uint8_t resetConfig)
{
    tUWBAPI_STATUS status;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_DEVICE_RESET_RSP_EVT);
    uwbContext.dev_state   = UWBAPI_UCI_DEV_ERROR;
    uwbContext.snd_data[0] = resetConfig;
    status = sendUciCommandAndWait(UWA_DM_API_CORE_DEVICE_RESET_EVT, sizeof(resetConfig), uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_DEVICE_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.dev_state != UWBAPI_UCI_DEV_READY) {
            status = UWBAPI_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetUwbDevState(uint8_t *pDeviceState)
{
    tUWBAPI_STATUS status;
    tUWA_PMID configParam[] = {UCI_PARAM_ID_DEVICE_STATE};
    uint16_t cmdLen         = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (pDeviceState == NULL) {
        NXPLOG_UWBAPI_E("%s: pDeviceState is NULL\n", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB Device is not initialized\n", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_CORE_GET_CONFIG_RSP_EVT);
    cmdLen = serializeGetCoreConfigPayload(1, sizeof(configParam), configParam, uwbContext.snd_data);

    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK && uwbContext.rsp_data[0] == UCI_PARAM_ID_DEVICE_STATE) {
        *pDeviceState = uwbContext.rsp_data[2];
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get UWB DEV state is failed\n", __FUNCTION__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SessionInit(uint32_t sessionId, eSessionType sessionType, uint32_t *sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    uint32_t index = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (sessionType == UWBD_RFTEST) {
        if (sessionId != 0x00) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
    }
    if (sessionHandle == NULL) {
        NXPLOG_UWBAPI_E("%s: SessionHandle is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_INIT_RSP_EVT);
    cmdLen = serializeSessionInitPayload(sessionId, sessionType, &uwbContext.snd_data[0]);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_INIT_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK && uwbContext.rsp_len > SESSION_HANDLE_OFFSET) {
        uint8_t *rspPtr = uwbContext.rsp_data;
        // skip the status from response.
        index++;
        // copy the Session Handle received through response.
        UWB_STREAM_TO_UINT32(*sessionHandle, rspPtr, index);
    }
    else {
        *sessionHandle = sessionId;
    }

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != *sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_INITIALIZED) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_INITIALIZED notification", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);

    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_ConfigureNtfTimeout(uint32_t timeoutMs)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_OK;
    // Check if UWB stack is initialized
    if (!uwbContext.isUfaEnabled)
    {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if(timeoutMs == 0 || timeoutMs == 0xFFFFFFFF){
        NXPLOG_UWBAPI_E("%s: Invalid timeout value", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    uwbContext.ntfTimeout = timeoutMs;
    return status;
}


EXTERNC tUWBAPI_STATUS UwbApi_SessionDeinit(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_SESSION_DEINIT_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_DEINIT_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_DEINITIALIZED) {
            return UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SetRangingParams(uint32_t sessionHandle, const phRangingParams_t *pRangingParam)
{
    tUWBAPI_STATUS status;
    uint8_t paramLen          = 0;
    uint8_t addrLen           = 0;
    uint8_t noOfRangingParams = 0;
    uint16_t cmdLen           = 0;
    size_t appConfigDataLen   = 0;
    uint8_t payloadOffset     = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRangingParam == NULL) {
        NXPLOG_UWBAPI_E("%s: pRangingParam is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_ROLE,
                                        sizeof(pRangingParam->deviceRole),
                                        (void *)&pRangingParam->deviceRole,
                                        &uwbContext.snd_data[payloadOffset]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_MULTI_NODE_MODE,
                                        sizeof(pRangingParam->multiNodeMode),
                                        (void *)&pRangingParam->multiNodeMode,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_MAC_ADDRESS_MODE,
                                        sizeof(pRangingParam->macAddrMode),
                                        (void *)&pRangingParam->macAddrMode,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

#if !(UWBIOT_UWBD_SR040)
    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_SCHEDULED_MODE,
                                        sizeof(pRangingParam->scheduledMode),
                                        (void *)&pRangingParam->scheduledMode,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count
#endif /* !(UWBIOT_UWBD_SR040) */

    if (pRangingParam->macAddrMode == SHORT_MAC_ADDRESS_MODE) {
        addrLen = (uint8_t)SHORT_MAC_ADDR_LEN;
    }
    else if (pRangingParam->macAddrMode == EXTENDED_MAC_ADDRESS_MODE ||
             pRangingParam->macAddrMode == EXTENDED_MAC_ADDRESS_MODE_WITH_HEADER) {
#if (UWBIOT_UWBD_SR040) /* As SR048 follows FiRa 2.0 */
        addrLen = (uint8_t)EXT_MAC_ADDR_LEN;
#else
        LOG_E("Extended MAC Mode is not supported");
        return UWBAPI_STATUS_INVALID_PARAM;

#endif /* (UWBIOT_UWBD_SR040) */
    }

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_MAC_ADDRESS,
                                        addrLen,
                                        (void *)&pRangingParam->deviceMacAddr,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_RANGING_ROUND_USAGE,
                                        sizeof(pRangingParam->rangingRoundUsage),
                                        (void *)&pRangingParam->rangingRoundUsage,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_TYPE,
                                        sizeof(pRangingParam->deviceType),
                                        (void *)&pRangingParam->deviceType,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfRangingParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);
#if UWBIOT_OS_NATIVE
    phOsalUwb_Delay(100);
#endif

    if ((status == UWBAPI_STATUS_OK) && (uwbContext.sessionInfo.state != UWB_SESSION_IDLE)) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_IDLE notification", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetRangingParams(uint32_t sessionHandle, phRangingParams_t *pRangingParams)
{
    tUWBAPI_STATUS status;
    uint8_t *pGetRangingCommand = NULL;
    uint16_t index              = 0;
    uint8_t paramId             = 0;
    uint8_t noOfParams;
    uint16_t cmdLen      = 0;
    uint8_t payloadOffet = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRangingParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pRangingParams is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    noOfParams         = uciRangingParamIds_len / sizeof(uint8_t);
    pGetRangingCommand = &uwbContext.snd_data[0];
    for (index = 0; index < noOfParams; index++) {
        paramId = uciRangingParamIds[index];
        UWB_UINT8_TO_STREAM(pGetRangingCommand, paramId, payloadOffet);
        NXPLOG_UWBAPI_D("%s: App ID: %02X", __FUNCTION__, paramId);
    }
    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);

    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, noOfParams, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        parseRangingParams(rspPtr, noOfParams, pRangingParams);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#if UWBFTR_CCC
EXTERNC tUWBAPI_STATUS UwbApi_SetCccRangingParams(uint32_t sessionHandle, const phCccRangingParams_t *pCccRangingParam)
{
    tUWBAPI_STATUS status;
    uint8_t paramLen          = 0;
    uint8_t noOfRangingParams = 0;
    uint16_t cmdLen           = 0;
    size_t appConfigDataLen   = 0;
    uint8_t payloadOffset     = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCccRangingParam == NULL) {
        NXPLOG_UWBAPI_E("%s: pCccRangingParam is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_TYPE,
                                        sizeof(pCccRangingParam->deviceType),
                                        (void *)&pCccRangingParam->deviceType,
                                        &uwbContext.snd_data[payloadOffset]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_NO_OF_CONTROLEES,
                                        sizeof(pCccRangingParam->noOfControlees),
                                        (void *)&pCccRangingParam->noOfControlees,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_RESPONDER_SLOT_INDEX,
                                        sizeof(pCccRangingParam->responderSlotIndex),
                                        (void *)&pCccRangingParam->responderSlotIndex,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_SLOT_DURATION,
                                        sizeof(pCccRangingParam->slotDuration),
                                        (void *)&pCccRangingParam->slotDuration,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_HOPPING_MODE,
                                        sizeof(pCccRangingParam->hoppingMode),
                                        (void *)&pCccRangingParam->hoppingMode,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfRangingParams; // Increment the number of ranging params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfRangingParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if ((status == UWBAPI_STATUS_OK) && (uwbContext.sessionInfo.state != UWB_SESSION_IDLE)) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_IDLE notification", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif // UWBFTR_CCC

EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfig(uint32_t sessionHandle, eAppConfig param_id, uint32_t param_value)
{
    tUWBAPI_STATUS status;
    uint8_t noOfParams    = 1;
    uint8_t paramLen      = 0;
    uint16_t cmdLen       = 0;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
#if !(UWBIOT_UWBD_SR04X)
    if (!(param_id < END_OF_SUPPORTED_APP_CONFIGS)) {
        NXPLOG_UWBAPI_W("Parameter can not be set using UwbApi_SetAppConfig. Use UwbApi_SetAppConfigMultipleParams");
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    else if ((param_id == STATIC_STS_IV) || (param_id == UWB_INITIATION_TIME)) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#elif UWBIOT_UWBD_SR04X
    if (!((param_id >= RANGING_ROUND_USAGE && param_id < END_OF_SUPPORTED_APP_CONFIGS) ||
            ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID && param_id < END_OF_SUPPORTED_EXT_CONFIGS))) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    else if (param_id == STATIC_STS_IV) {
        NXPLOG_UWBAPI_W(
            "STATIC_STS_IV can not be set using UwbApi_SetAppConfig. Use UwbApi_SetAppConfigMultipleParams");
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#endif /* (UWBIOT_UWBD_SR04X) */

    /* TODO: to be removed */
    if (param_id == CHANNEL_NUMBER) {
        uint8_t channel = param_value & 0xff;
        if ((channel != CHANNEL_5) && (channel != CHANNEL_6) && (channel != CHANNEL_8) && (channel != CHANNEL_9)) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
    }
#if !(UWBIOT_UWBD_SR04X)
    if (param_id == MAC_ADDRESS_MODE &&
        (param_value == EXTENDED_MAC_ADDRESS_MODE || param_value == EXTENDED_MAC_ADDRESS_MODE_WITH_HEADER)) {
        LOG_E("Extended MAC Mode is not supported");
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#endif // !(UWBIOT_UWBD_SR04X)
#if (UWBIOT_UWBD_SR04X)
    if ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID) {
        paramLen = getExtTLVBuffer(param_id, (void *)&param_value, &uwbContext.snd_data[payloadOffset]);
    }
    else
#endif /* (UWBIOT_UWBD_SR04X) */
    {
        paramLen = getAppConfigTLVBuffer(param_id, 0, (void *)&param_value, &uwbContext.snd_data[payloadOffset]);
    }

    if (paramLen == 0) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

tUWBAPI_STATUS UwbApi_SetAppConfigMultipleParams(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_AppParams_List_t *AppParams_List)
{
    uint16_t paramLen = 0;
    uint16_t cmdLen   = 0;
    eAppConfig paramId;
    tUWBAPI_STATUS status;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    UWB_AppParams_value_au8_t output_param_value;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((AppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Assign the buffer for storing all the configs */
    output_param_value.param_value = uwbContext.snd_data;

    for (uint32_t LoopCnt = 0; LoopCnt < noOfparams; ++LoopCnt) {
        paramId = AppParams_List[LoopCnt].param_id;
#if !(UWBIOT_UWBD_SR04X)
        if (!(paramId < END_OF_SUPPORTED_APP_CONFIGS)) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#elif UWBIOT_UWBD_SR04X
        if (!((paramId >= RANGING_ROUND_USAGE && paramId < END_OF_SUPPORTED_APP_CONFIGS) ||
                ((AppParams_List[LoopCnt].param_id >> 4) >= EXTENDED_APP_CONFIG_ID &&
                    paramId < END_OF_SUPPORTED_EXT_CONFIGS))) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#endif /* (UWBIOT_UWBD_SR04X) */
        /* parse and get input length and pointer */
        if (AppConfig_TlvParser(&AppParams_List[LoopCnt], &output_param_value) != UWBAPI_STATUS_OK) {
            return UWBAPI_STATUS_FAILED;
        }
#if !(UWBIOT_UWBD_SR04X)
        if (paramId == MAC_ADDRESS_MODE &&
            (*output_param_value.param_value == EXTENDED_MAC_ADDRESS_MODE ||
                *output_param_value.param_value == EXTENDED_MAC_ADDRESS_MODE_WITH_HEADER)) {
            LOG_E("Extended MAC Mode is not supported");
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#endif // !(UWBIOT_UWBD_SR04X)

#if (UWBIOT_UWBD_SR04X)
        if ((AppParams_List[LoopCnt].param_id >> 4) >= EXTENDED_APP_CONFIG_ID) {
            /* Safe check for array indexing. Coverity issue fix. */
            paramLen = (uint16_t)(paramLen + getExtTLVBuffer(paramId,
                                                 (void *)(output_param_value.param_value),
                                                 &uwbContext.rsp_data[payloadOffset + paramLen]));
        }
        else
#endif /* (UWBIOT_UWBD_SR04X) */
        {
            paramLen = (uint16_t)(paramLen + getAppConfigTLVBuffer(paramId,
                                                 (uint8_t)(output_param_value.param_len),
                                                 output_param_value.param_value,
                                                 &uwbContext.rsp_data[payloadOffset + paramLen]));
        }
    }

    if (paramLen == 0) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, paramLen, uwbContext.rsp_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.rsp_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

#if !(UWBIOT_UWBD_SR04X)
tUWBAPI_STATUS UwbApi_SetVendorAppConfigs(
    uint32_t sessionHandle, uint8_t noOfparams, const UWB_VendorAppParams_List_t *vendorAppParams_List)
{
    uint16_t paramLen = 0;
    uint16_t cmdLen   = 0;
    eVendorAppConfig paramId;
    tUWBAPI_STATUS status;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    UWB_AppParams_value_au8_t output_param_value;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((vendorAppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Assign the buffer for storing all the configs */
    output_param_value.param_value = uwbContext.snd_data;

    for (uint32_t LoopCnt = 0; LoopCnt < noOfparams; ++LoopCnt) {
        paramId = vendorAppParams_List[LoopCnt].param_id;
        /* parse and get input length and pointer */
        if (VendorAppConfig_TlvParser(&vendorAppParams_List[LoopCnt], &output_param_value) != UWBAPI_STATUS_OK) {
            return UWBAPI_STATUS_FAILED;
        }
#if (UWBFTR_SE_SE051W)
        if (paramId == WRAPPED_RDS) {
            uwbContext.rsp_data[paramLen++] = UCI_VENDOR_PARAM_ID_WRAPPED_RDS;
            uwbContext.rsp_data[paramLen++] = output_param_value.param_len;
            phOsalUwb_MemCopy(
                &uwbContext.rsp_data[paramLen], (void *)(output_param_value.param_value), output_param_value.param_len);
            paramLen += output_param_value.param_len;
        }
        else
#endif // UWBFTR_SE_SE051W
        {
            paramLen = (uint16_t)(paramLen + getVendorAppConfigTLVBuffer(paramId,
                                                 (void *)(output_param_value.param_value),
                                                 output_param_value.param_len,
                                                 &uwbContext.rsp_data[payloadOffset + paramLen]));
        }
    }

    if (paramLen == 0) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, paramLen, uwbContext.rsp_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.rsp_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif //!(UWBIOT_UWBD_SR04X)

EXTERNC tUWBAPI_STATUS UwbApi_GetAppConfig(uint32_t sessionHandle, eAppConfig param_id, uint32_t *param_value)
{
    uint8_t len             = 0;
    uint8_t offset          = 0;
    uint8_t noOfParams      = 1;
    uint8_t paramLen        = 1;
    uint16_t cmdLen         = 0;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (param_value == NULL) {
        NXPLOG_UWBAPI_E("%s: param_value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

#if !(UWBIOT_UWBD_SR04X)
    if (!(param_id < END_OF_SUPPORTED_APP_CONFIGS)) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#elif UWBIOT_UWBD_SR04X
    if (!((param_id >= RANGING_ROUND_USAGE && param_id < END_OF_SUPPORTED_APP_CONFIGS) ||
            ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID && param_id < END_OF_SUPPORTED_EXT_CONFIGS))) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#endif /* (UWBIOT_UWBD_SR04X) */

    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);
    pConfigCommand = &uwbContext.snd_data[0];

#if (UWBIOT_UWBD_SR04X)
    if ((param_id >> 4) >= EXTENDED_APP_CONFIG_ID) {
        UWB_UINT8_TO_STREAM(pConfigCommand, param_id, payloadOffset);
    }
    else
#endif /* (UWBIOT_UWBD_SR04X) */
    {
        UWB_UINT8_TO_STREAM(pConfigCommand, param_id, payloadOffset);
    }
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        /* rsp_data contains rsp, starting from the paramId, so skip paramId field*/
        offset++;
        UWB_STREAM_TO_UINT8(len, rspPtr, offset);
        if (len == sizeof(uint8_t)) {
            UWB_STREAM_TO_UINT8(*param_value, rspPtr, offset);
        }
        else if (len == sizeof(uint16_t)) {
            UWB_STREAM_TO_UINT16(*param_value, rspPtr, offset);
        }
        else if (len == (sizeof(uint32_t) - 1)) {
            UWB_STREAM_TO_UINT24(*param_value, rspPtr, offset);
        }
        else if (len == sizeof(uint32_t)) {
            UWB_STREAM_TO_UINT32(*param_value, rspPtr, offset);
        }
        else {
            NXPLOG_UWBAPI_W(
                "%s: API limitation, API Shall not be called for parameters having length more than 4 bytes",
                __FUNCTION__);
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetAppConfigMultipleParams(
    uint32_t sessionHandle, uint8_t noOfparams, UWB_AppParams_List_t *AppParams_List)
{
    tUWBAPI_STATUS status;
    uint8_t i = 0;
    eAppConfig paramId;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint16_t cmdLen         = 0;
    uint8_t offset          = 0;
    uint8_t *rspPtr         = NULL;
    uint16_t len            = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((AppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pConfigCommand = &uwbContext.snd_data[payloadOffset];
    for (i = 0; i < noOfparams; i++) {
        paramId = AppParams_List[i].param_id;
#if !(UWBIOT_UWBD_SR04X)
        if (!(paramId >= RANGING_ROUND_USAGE && paramId < END_OF_SUPPORTED_APP_CONFIGS)) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#endif
#if (UWBIOT_UWBD_SR04X)
        if (!((paramId >= RANGING_ROUND_USAGE && paramId < END_OF_SUPPORTED_APP_CONFIGS) ||
                ((AppParams_List[i].param_id >> 4) >= EXTENDED_APP_CONFIG_ID &&
                    paramId < END_OF_SUPPORTED_EXT_CONFIGS))) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
#endif

#if (UWBIOT_UWBD_SR04X)
        if ((paramId >> 4) >= EXTENDED_APP_CONFIG_ID) {
            pConfigCommand[cmdLen++] = (paramId & 0xFF);
        }
        else
#endif /* (UWBIOT_UWBD_SR04X) */
        {
            pConfigCommand[cmdLen++] = (uint8_t)(paramId & 0xFF);
        }
    }
    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, cmdLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        rspPtr = &uwbContext.rsp_data[0];
        for (i = 0; i < noOfparams; i++) {
            /* rsp_data contains rsp, starting from the paramId, so skip paramId field*/
            offset++;
            UWB_STREAM_TO_UINT8(len, rspPtr, offset);
            if (AppParams_List[i].param_type == kUWB_APPPARAMS_Type_u32) {
                if (len == sizeof(uint8_t)) {
                    UWB_STREAM_TO_UINT8(AppParams_List[i].param_value.vu32, rspPtr, offset);
                }
                else if (len == sizeof(uint16_t)) {
                    UWB_STREAM_TO_UINT16(AppParams_List[i].param_value.vu32, rspPtr, offset);
                }
                else if (len == (sizeof(uint32_t) - 1)) {
                    UWB_STREAM_TO_UINT24(AppParams_List[i].param_value.vu32, rspPtr, offset);
                }
                else if (len == sizeof(uint32_t)) {
                    UWB_STREAM_TO_UINT32(AppParams_List[i].param_value.vu32, rspPtr, offset);
                }
            }
            else if (AppParams_List[i].param_type == kUWB_APPPARAMS_Type_au8) {
                if (AppParams_List[i].param_value.au8.param_len >= len) {
                    UWB_STREAM_TO_ARRAY(AppParams_List[i].param_value.au8.param_value, rspPtr, len, offset);
                }
                else {
                    NXPLOG_UWBAPI_E("%s: Not enough buffer to store app config value", __FUNCTION__);
                    status = UWBAPI_STATUS_BUFFER_OVERFLOW;
                }
                AppParams_List[i].param_value.au8.param_len = len;
            }
        }
    }
    return status;
}

#if !(UWBIOT_UWBD_SR04X)

tUWBAPI_STATUS UwbApi_GetVendorAppConfigs(
    uint32_t sessionHandle, uint8_t noOfparams, UWB_VendorAppParams_List_t *vendorAppParams_List)
{
    tUWBAPI_STATUS status;
    uint8_t i = 0;
    eVendorAppConfig paramId;
    uint8_t *pConfigCommand = NULL;
    uint8_t payloadOffset   = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint16_t cmdLen         = 0;
    uint16_t offset          = 0;
    uint8_t *rspPtr         = NULL;
    uint16_t len            = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((vendorAppParams_List == NULL) || (noOfparams == 0)) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pConfigCommand = &uwbContext.snd_data[payloadOffset];
    for (i = 0; i < noOfparams; i++) {
        paramId = vendorAppParams_List[i].param_id;

        // TODO: Add check to validate given vendor param is in list or not
        pConfigCommand[cmdLen++] = (uint8_t)(paramId & 0xFF);
    }
    // TODO: wait for GET_VENDOR_APP_CONFIG_RSP
    sep_SetWaitEvent(UWA_DM_SESSION_GET_VENDOR_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfparams, cmdLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        rspPtr = &uwbContext.rsp_data[0];
        for (i = 0; i < noOfparams; i++) {
            if (offset < (UINT8_MAX - 1)) {
                offset++;
            } else {
                NXPLOG_UWBAPI_E("%s: Offset overflow. offset=%u", __FUNCTION__, offset);
                return UWBAPI_STATUS_FAILED;
            }

            /* Validate we can read length field */
            if (offset >= sizeof(uwbContext.rsp_data)) {
                NXPLOG_UWBAPI_E("%s: Cannot read length at offset %d", __FUNCTION__, offset);
                status = UWBAPI_STATUS_BUFFER_OVERFLOW;
                break;
            }

            UWB_STREAM_TO_UINT8(len, rspPtr, offset);

            /* Validate length won't cause overrun */
            if ((len > sizeof(uwbContext.rsp_data)) || ((offset + len) > sizeof(uwbContext.rsp_data))) {
                NXPLOG_UWBAPI_E("%s: Invalid len=%d at offset=%d", __FUNCTION__, len, offset);
                status = UWBAPI_STATUS_BUFFER_OVERFLOW;
                break;
            }
            if (vendorAppParams_List[i].param_type == kUWB_APPPARAMS_Type_u32) {
                if (len == sizeof(uint8_t)) {
                    UWB_STREAM_TO_UINT8(vendorAppParams_List[i].param_value.vu32, rspPtr, offset);
                }
                else if (len == sizeof(uint16_t)) {
                    UWB_STREAM_TO_UINT16(vendorAppParams_List[i].param_value.vu32, rspPtr, offset);
                }
                else if (len == sizeof(uint32_t)) {
                    UWB_STREAM_TO_UINT32(vendorAppParams_List[i].param_value.vu32, rspPtr, offset);
                }
            }
            else if (vendorAppParams_List[i].param_type == kUWB_APPPARAMS_Type_au8) {
                if (vendorAppParams_List[i].param_value.au8.param_len >= len) {
                    UWB_STREAM_TO_ARRAY(vendorAppParams_List[i].param_value.au8.param_value, rspPtr, len, offset);
                }
                else {
                    NXPLOG_UWBAPI_E("%s: Not enough buffer to store app config value", __FUNCTION__);
                    status = UWBAPI_STATUS_BUFFER_OVERFLOW;
                    break;
                }
                vendorAppParams_List[i].param_value.au8.param_len = len;
            }
        }
    }
    return status;
}
#endif //!(UWBIOT_UWBD_SR04X)

EXTERNC tUWBAPI_STATUS UwbApi_SetStaticSts(uint32_t sessionHandle, uint16_t vendorId, uint8_t const *const staticStsIv)
{
    tUWBAPI_STATUS status;
    uint8_t noOfParams    = 0;
    uint8_t paramLen      = 0;
    uint16_t cmdLen       = 0;
    size_t appConfigDataLen   = 0;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (staticStsIv == NULL) {
        NXPLOG_UWBAPI_E("%s: Static Sts Iv is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    paramLen = getAppConfigTLVBuffer(
        UCI_PARAM_ID_VENDOR_ID, UCI_PARAM_LEN_VENDOR_ID, (void *)&vendorId, &uwbContext.snd_data[payloadOffset]);

    ++noOfParams; // Increment the number of params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_STATIC_STS_IV,
                                        UCI_PARAM_LEN_STATIC_STS_IV,
                                        (void *)staticStsIv,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfParams; // Increment the number of params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SetDeviceConfig(
    eDeviceConfig param_id, uint8_t param_len, phDeviceConfigData_t *param_value)
{
    uint8_t offset = 1;
    tUWBAPI_STATUS status;
#if (UWBIOT_UWBD_SR04X)
    uint8_t ext_param_id = 0;
#endif

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (param_value == NULL || param_len == 0) {
        NXPLOG_UWBAPI_E("%s: Parameter value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

#if !(UWBIOT_UWBD_SR04X)
    if (((param_id >> 8) & 0xFF) == EXTENDED_DEVICE_CONFIG_ID) {
        offset = getExtCoreDeviceConfigTLVBuffer(param_id, param_len, param_value, &uwbContext.snd_data[offset]);
    }
    else {
        offset = getCoreDeviceConfigTLVBuffer(param_id, param_len, param_value, &uwbContext.snd_data[offset]);
    }
#endif // !(UWBIOT_UWBD_SR04X)

#if (UWBIOT_UWBD_SR04X)
    ext_param_id = (uint8_t)(param_id >> 4);
    /* Checking for the second Nibble */
    if (ext_param_id >= EXTENDED_DEVICE_CONFIG_ID) {
        /*No Mapping need */
        offset =
            getExtCoreDeviceConfigTLVBuffer(param_id, param_len, (void *)param_value, &uwbContext.snd_data[offset]);
    }
    else {
        /*No Mapping need */
        offset = getCoreDeviceConfigTLVBuffer(param_id, param_len, param_value, &uwbContext.snd_data[offset]);
    }
#endif /* (UWBIOT_UWBD_SR04X) */

    if (offset == 0) {
        NXPLOG_UWBAPI_E("%s: offset is zero for getCoreDeviceConfigTLVBuffer", __FUNCTION__);
        return UWBAPI_STATUS_FAILED;
    }
    sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
    uwbContext.snd_data[0] = 1;
    status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetDeviceConfig(eDeviceConfig param_id, phDeviceConfigData_t *devConfig)
{
    uint8_t offset     = 0;
    uint8_t noOfParams = 1;
    uint8_t paramLen   = 1;
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (devConfig == NULL) {
        NXPLOG_UWBAPI_E("%s: param_value is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_CORE_GET_CONFIG_RSP_EVT);
#if !(UWBIOT_UWBD_SR04X)
    if ((uint8_t)(param_id >> 8) == EXTENDED_DEVICE_CONFIG_ID) {
        paramLen++;
        param_id = (eDeviceConfig)((param_id >> 8) | (param_id << 8));
    }
#endif // !(UWBIOT_UWBD_SR04X)

    cmdLen = serializeGetCoreConfigPayload(noOfParams, paramLen, (uint8_t *)&param_id, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_CORE_GET_CONFIG_EVT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
#if !(UWBIOT_UWBD_SR04X)
        if ((uint8_t)param_id == EXTENDED_DEVICE_CONFIG_ID) {
            parseExtGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
        else {
            parseCoreGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
#endif // !(UWBIOT_UWBD_SR04X)

#if (UWBIOT_UWBD_SR04X)
        if ((uint8_t)(param_id >> 4) >= EXTENDED_DEVICE_CONFIG_ID) {
            parseExtGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
        else {
            parseCoreGetDeviceConfigResponse(&uwbContext.rsp_data[offset], devConfig);
        }
#endif /* (UWBIOT_UWBD_SR04X) */
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_StartRangingSession(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
#if UWBIOT_SESN_SNXXX
    uint8_t KeyFetchErrorRetryCnt = 0U;
#endif
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
#if UWBIOT_SESN_SNXXX
RetryUponKeyFetchError:
#endif // UWBIOT_SESN_SNXXX
    sep_SetWaitEvent(UWA_DM_RANGE_START_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_START_EVT, cmdLen, uwbContext.snd_data);
#if UWBIOT_SESN_SNXXX
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWBD_SE_RANGING_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE) {
            /*
             * Hanlde the Key Fetch Error Handling. Only till Key Fetch error retry
             * count.
             */
            if (uwbContext.sessionInfo.state == UCI_SESSION_FAILED_WITH_KEY_FETCH_ERROR) {
                status = UCI_STATUS_ESE_RECOVERY_FAILURE;
                while (KeyFetchErrorRetryCnt < UWB_KEY_FETCH_ERROR_RETRY_COUNT) {
                    /*
                     * Increment the Key Fetch Error Retry Count.
                     */
                    ++KeyFetchErrorRetryCnt;
                    /*
                     * Wait for SE COMM ERROR NTF.
                     */
                    status = waitforNotification(UWA_DM_PROP_SE_COM_ERROR_NTF_EVT, UWB_NTF_TIMEOUT);
                    if (status == UWBAPI_STATUS_OK) {
                        /*
                         * If the Recovery is successful during se_comm_error notification
                         * then Resend start ranging command.
                         */
                        if (uwbContext.wstatus == UCI_STATUS_ESE_RECOVERY_SUCCESS) {
                            goto RetryUponKeyFetchError;
                        }
                        else if (uwbContext.wstatus == UCI_STATUS_ESE_RECOVERY_FAILURE) {
#if UWBIOT_SESN_P71
                            status = UWBAPI_STATUS_ESE_ERROR;
#else
                            /*
                             * Reset the eSE.[Do the cold reset of eSE]
                             */
                            reset_se_on_error();
                            status = UWBAPI_STATUS_ESE_RESET;
#endif // UWBIOT_SESN_P71
                            break;
                        }
                    }
                }
            }
            else if (uwbContext.sessionInfo.state == UCI_SESSION_FAILED_WITH_NO_RNGDATA_IN_SE) {
                status = UWBAPI_STATUS_SESSION_NOT_EXIST;
            }
            else {
                status = UWBAPI_STATUS_FAILED;
            }
        }
    }
#else  /* UWBIOT_SESN_SNXXX */
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (status != UWBAPI_STATUS_OK || uwbContext.sessionInfo.sessionHandle != sessionHandle ||
            uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE) {
            NXPLOG_UWBAPI_E("%s: waitforNotification for event %d Failed", __FUNCTION__, UWA_DM_SESSION_STATUS_NTF_EVT);
            status = UWBAPI_STATUS_FAILED;
        }
    }
#endif /* UWBIOT_SESN_SNXXX */

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_StopRangingSession(uint32_t sessionHandle)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_RANGE_STOP_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_STOP_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, uwbContext.ntfTimeout);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            status = UWBAPI_STATUS_FAILED;
        }
    }
#if (UWBIOT_UWBD_SR04X)
    /* Wait for 1 more NTF, 6001000101.
     * Else there's Writer thread takes mutex of IO interface and read will always be pending, and write will always
     * fail. Cleaner handling would be to ensure writer thread is able to unblock reader thread.
     */
    phOsalUwb_Delay(10);
#endif

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_EnableRangingDataNtf(
    uint32_t sessionHandle, uint8_t enableRangingDataNtf, uint16_t proximityNear, uint16_t proximityFar)
{
    uint8_t noOfParam     = 1;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    uint8_t paramLen      = 0;
    size_t appConfigDataLen    = 0;
    uint16_t cmdLen       = 0;
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (enableRangingDataNtf > 2) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_SESSION_INFO_NTF,
                                        UCI_PARAM_LEN_SESSION_INFO_NTF,
                                        &enableRangingDataNtf,
                                        &uwbContext.snd_data[payloadOffset]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }

    if (enableRangingDataNtf == 2) {
        noOfParam = (uint8_t)(noOfParam + 2);
        appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_NEAR_PROXIMITY_CONFIG,
                                            UCI_PARAM_LEN_NEAR_PROXIMITY_CONFIG,
                                            &proximityNear,
                                            &uwbContext.snd_data[payloadOffset + paramLen]);
        if (paramLen < (UINT8_MAX - appConfigDataLen)) {
            paramLen = (uint8_t)(paramLen + appConfigDataLen);
        } else {
            NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
                __FUNCTION__, paramLen);
            return UWBAPI_STATUS_FAILED;
        }

        appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_FAR_PROXIMITY_CONFIG,
                                            UCI_PARAM_LEN_FAR_PROXIMITY_CONFIG,
                                            &proximityFar,
                                            &uwbContext.snd_data[payloadOffset + paramLen]);
        if (paramLen < (UINT8_MAX - appConfigDataLen)) {
            paramLen = (uint8_t)(paramLen + appConfigDataLen);
        } else {
            NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
                __FUNCTION__, paramLen);
            return UWBAPI_STATUS_FAILED;
        }
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParam, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SendRawCommand(uint8_t data[], uint16_t data_len, uint8_t *pResp, uint16_t *pRespLen)
{
    tUWBAPI_STATUS status;
    uint8_t pbf;
    bool datapacketFlag = false;
    NXPLOG_UWBAPI_D("%s: enter; ", __FUNCTION__);

    if (data == NULL || data_len <= 0 || pResp == NULL || pRespLen == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        *pRespLen = 0;
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    datapacketFlag = IS_DATA_SEND_PACKET(data[0]);

    if (*pRespLen == 0) {
        NXPLOG_UWBAPI_E("%s: pRespLen is Zero", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = sendRawUci(data, data_len);
    if (status == UWBAPI_STATUS_PBF_PKT_SENT) {
        *pRespLen = 0;
        return status;
    }

    if (false == datapacketFlag) {
        if (uwbContext.rsp_len > *pRespLen) {
            UCI_MSG_PRS_PBF(&uwbContext.rsp_data[0], pbf)
            if (pbf) {
                NXPLOG_UWBAPI_E(
                    "%s: Response data size is more than response buffer due to chaining rsp expected for the command ",
                    __FUNCTION__);
                NXPLOG_UWBAPI_W("Max Response size of respone expecting is pResp[%d] bytes:", uwbContext.rsp_len);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __FUNCTION__);
            }
            *pRespLen = 0;
            status    = UWBAPI_STATUS_BUFFER_OVERFLOW;
        }
        else {
            *pRespLen = uwbContext.rsp_len;
            phOsalUwb_MemCopy(pResp, uwbContext.rsp_data, uwbContext.rsp_len);
        }
    }
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetSessionState(uint32_t sessionHandle, uint8_t *sessionState)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: enter; ", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (sessionState == NULL) {
        NXPLOG_UWBAPI_E("%s: sessionState is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_GET_STATE_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(sessionHandle, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_GET_STATE_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        *sessionState = uwbContext.rsp_data[0];
    }
    else {
        *sessionState = UWBAPI_SESSION_ERROR;
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_UpdateControllerMulticastList(phMulticastControleeListContext_t *pControleeContext,
    phMulticastControleeListRspContext_t *pControleeListRspContext)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((pControleeContext == NULL) || (NULL == pControleeListRspContext)) {
        NXPLOG_UWBAPI_E("%s: pControleeContext is 0x%p and pControleeListRspContext is 0x%p",
            __FUNCTION__, pControleeContext, pControleeListRspContext);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pControleeContext->action == MULTICAST_LIST_DEL_CONTROLEE) {
        for (uint8_t i = 0; i < pControleeContext->no_of_controlees; i++) {
            if (pControleeContext->controlee_list[i].subsession_id != 0x0) {
                return UWBAPI_STATUS_INVALID_PARAM; // for deletion, sub Session Handle must be zero
            }
        }
    }

    sep_SetWaitEvent(UWA_DM_SESSION_MC_LIST_UPDATE_RSP_EVT);
    cmdLen = serializeUpdateControllerMulticastListPayload(pControleeContext, uwbContext.snd_data);
    status =
        sendUciCommandAndWait(UWA_DM_API_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_EVT, cmdLen, uwbContext.snd_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("%s: SESSION_MC_LIST_UPDATE command returned 0x%x ", __FUNCTION__, status);
        /*
         * Whether the action is to Add or Delete the controlees from the list, if the response code is STATUS_FAILURE,
         * there would be response data with the controlee MAC addresses and respective status for the operations.
         * that needs to be reported to the caller.
         */
        if (UWBAPI_STATUS_FAILED == status) {
            deserializeUpdateControllerMulticastListResp(pControleeListRspContext, uwbContext.rsp_data);
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetTrng(uint8_t trng_size, uint8_t *ptrng)
{
    tUWBAPI_STATUS status;
    uint8_t *pResponse = NULL;
    uint16_t cmdLen;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (ptrng == NULL) {
        NXPLOG_UWBAPI_E("%s: trng data is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (trng_size > MAX_TRNG_SIZE || trng_size < MIN_TRNG_SIZE) {
        NXPLOG_UWBAPI_E("%s: Trng size  is invalid it should be Between 0x01-0x10 ", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_RANGE;
    }
    sep_SetWaitEvent(UWA_DM_PROP_TRNG_RESP_EVENT);
    cmdLen = serializeTrngtPayload(trng_size, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_TRNG_EVENT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
        pResponse = &uwbContext.rsp_data[0]; // Response
        UWB_STREAM_TO_UINT8(status, pResponse, index);
        if (status == UWBAPI_STATUS_OK) {
            /* Fetch the TRNG bytes */
        	UWB_STREAM_TO_ARRAY(ptrng, pResponse, trng_size, index);
        }
        else {
            NXPLOG_UWBAPI_E("%s: UwbApi_GetTrng failed", __FUNCTION__);
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

#if (UWBFTR_BlobParser)
EXTERNC tUWBAPI_STATUS UwbApi_SetProfileParams(
    uint8_t *pProfileBlob, uint16_t blobSize, phUwbProfileInfo_t *pProfileInfo)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pProfileBlob == NULL) {
        NXPLOG_UWBAPI_E("%s: profile blob buffer is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (pProfileInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: profile info is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if ((blobSize != TOTAL_PROFILE_BLOB_SIZE_v1_1) && (blobSize != TOTAL_PROFILE_BLOB_SIZE_v1_0)) {
        NXPLOG_UWBAPI_E("%s: profile blob size should be %d or %d bytes",
            __FUNCTION__,
            TOTAL_PROFILE_BLOB_SIZE_v1_0,
            TOTAL_PROFILE_BLOB_SIZE_v1_1);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_PROP_PROFILE_BLOB_RSP_EVENT);
    cmdLen = serializeSetProfileParamsPayload(pProfileInfo, blobSize, pProfileBlob, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_PROFILE_PARAM_EVENT, cmdLen, uwbContext.snd_data);

#if !(UWBIOT_UWBD_SR04X)
    /* set the sessionHandle received from the PROP_SET_PROFILE_CMD */
    if (status == UWBAPI_STATUS_OK &&
        (uwbContext.rsp_len > SESSION_HANDLE_OFFSET && uwbContext.rsp_len <= SESSION_HANDLE_OFFSET_LEN)) {
        uint8_t *rspPtr = uwbContext.rsp_data;
        uint32_t index = 0; // Declaration has to be here as the code is under feature macro
        // skip the status from response.
        index++;
        // copy the Session Handle received through response.
        UWB_STREAM_TO_UINT32(pProfileInfo->sessionHandle, rspPtr, index);
    }
#endif // !(UWBIOT_UWBD_SR04X)

    if (status == UWBAPI_STATUS_OK) {
        status = uwbContext.wstatus;
        if (status == UWBAPI_STATUS_OK) {
            status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
            if (status == UWBAPI_STATUS_OK && uwbContext.sessionInfo.state == UWB_SESSION_IDLE) {
                /** In case of SessionHandle is Disabled */
                if (uwbContext.rsp_len == SESSION_HANDLE_OFFSET) {
                    pProfileInfo->sessionHandle = uwbContext.sessionInfo.sessionHandle;
                }

                if (pProfileInfo->sessionHandle != uwbContext.sessionInfo.sessionHandle) {
                    status = UWBAPI_STATUS_FAILED;
                }
            }
            else {
                status = UWBAPI_STATUS_FAILED;
            }
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

tUWBAPI_STATUS UwbApi_GetUwbConfigData_iOS(
    UWB_DeviceRole_t device_role, AccessoryUwbConfigDataContent_t *uwb_data_content)
{
    tUWBAPI_STATUS status;
    static phUwbDevInfo_t devInfo    = {0};
    uint8_t uwb_spec_version_major[] = UWB_IOS_SPEC_VERSION_MAJOR;
    uint8_t uwb_spec_version_minor[] = UWB_IOS_SPEC_VERSION_MINOR;
    uint8_t manufacturer_id[]        = MANUFACTURER_ID;
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    uint16_t readMMId;
#endif //(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)

    phUwbSessionData_t sessionData[MAXIMUM_SESSION_COUNT] = {0};
    phUwbSessionsContext_t uwbSessionsContext             = {0};
    uwbSessionsContext.sessioncnt                         = 5;
    uwbSessionsContext.pUwbSessionData                    = sessionData;
    uwbSessionsContext.status                             = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (uwb_data_content == NULL) {
        NXPLOG_UWBAPI_E("%s: uwb_data_content is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if ((uwb_spec_version_minor[0] == 0x00) && (uwb_spec_version_minor[1] == 0x00)) {
        NXPLOG_UWBAPI_D(" Following spec 1.0");
        uwb_data_content->length =
            sizeof(AccessoryUwbConfigDataContent_t) - 1 - 2 /* clock drift not sent in spec 1.0 */;
    }
    else if ((uwb_spec_version_minor[0] == 0x01) && (uwb_spec_version_minor[1] == 0x00)) {
        NXPLOG_UWBAPI_D(" Following spec 1.1");
        uwb_data_content->length = sizeof(AccessoryUwbConfigDataContent_t) - 1;
    }
    else {
        NXPLOG_UWBAPI_D(" Unknown Spec");
        uwb_data_content->length = 0;
    }

    /* Generate mac address */
    status = UwbApi_GetTrng(SHORT_MAC_ADDR_LEN, uwb_data_content->device_mac_addr);

    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_GetTrng() Failed");
        goto exit;
    }

    /* Fill in UWB spec version */
    phOsalUwb_MemCopy(uwb_data_content->uwb_spec_ver_major, uwb_spec_version_major, sizeof(uwb_spec_version_major));
    phOsalUwb_MemCopy(uwb_data_content->uwb_spec_ver_minor, uwb_spec_version_minor, sizeof(uwb_spec_version_minor));

    /* Get info of all UWBsessions */
    status = UwbApi_GetAllUwbSessions(&uwbSessionsContext);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_GetAllUwbSessions Failed");
        goto exit;
    }
    /* check if there is no session present, then only call UwbApi_GetDeviceInfo() */
    if (uwbSessionsContext.sessioncnt == 0) {
        status = UwbApi_GetDeviceInfo(&devInfo);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_GetDeviceInfo failed");
            goto exit;
        }
    }
    /* Fill in the MW version*/
    uwb_data_content->mw_version[3] = devInfo.mwMajor;
    uwb_data_content->mw_version[2] = devInfo.mwMinor;
    /* First two bytes are zero default */
    uwb_data_content->mw_version[1] = 0;
    uwb_data_content->mw_version[0] = 0;
    /* Fill in the Device Model ID */
    uwb_data_content->model_id[0] = devInfo.fwMajor;

#if UWBIOT_UWBD_SR150
    /* Check the chip type */
    if (phOsalUwb_MemCompare(devInfo.devName, "SR150_PROD_IOT_ROW", devInfo.devNameLen) != 0) {
        uwb_data_content->model_id[1] = 0xFF;
    }
    else {
        uwb_data_content->model_id[1] = MODELID_CHIP_TYPE;
    }
#elif UWBIOT_UWBD_SR040 /* TODO: SR048 - Needs an update? */
    /* Check the chip type */
    if (phOsalUwb_MemCompare(devInfo.devName, "SR040", 5) != 0) {
        uwb_data_content->model_id[1] = 0xFF;
    }
    else {
        uwb_data_content->model_id[1] = MODELID_CHIP_TYPE;
    }
#elif UWBIOT_UWBD_SR250
    /* Check the chip type */
    if (phOsalUwb_MemCompare(devInfo.devName, "SR250_A1V2_PROD", devInfo.devNameLen) != 0) {
        uwb_data_content->model_id[1] = 0xFF;
    }
    else {
        uwb_data_content->model_id[1] = MODELID_CHIP_TYPE;
    }
#endif // UWBIOT_UWBD_SR250
    /* Fill in the Manufacturer Version */
    phOsalUwb_MemCopy(uwb_data_content->manufacturer_id, manufacturer_id, sizeof(manufacturer_id));

    /* Fill in the device role */
    uwb_data_content->ranging_role = device_role;

    if ((uwb_spec_version_minor[0] == 0x01) && (uwb_spec_version_minor[1] == 0x00)) {
        /* Fill in the clock drift value */
        uwb_data_content->clock_drift[0] = CLOCK_DRIFT;
        uwb_data_content->clock_drift[1] = (CLOCK_DRIFT >> 8);
    }

#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    status = UwbApi_ReadModuleMakerInfo(&readMMId);
    UWB_UINT16_TO_FIELD(&uwb_data_content->model_id[2], readMMId);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_ReadModuleMakerInfo Failed");
        goto exit;
    }
#elif UWBIOT_UWBD_SR04X
    uwb_data_content->model_id[2] = MODELID_BOARD_TYPE;
    uwb_data_content->model_id[3] = MODELID_RFU;
#endif /* (UWBIOT_UWBD_SR04X) */

exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

tUWBAPI_STATUS UwbApi_GetUwbConfigData_Android(UwbDeviceConfigData_t *uwb_data_content)
{
    tUWBAPI_STATUS status;
    static phUwbDevInfo_t devInfo    = {0};
    phUwbSessionData_t sessionData[MAXIMUM_SESSION_COUNT] = {0};
    phUwbSessionsContext_t uwbSessionsContext             = {0};
    uwbSessionsContext.sessioncnt                         = 5;
    uwbSessionsContext.pUwbSessionData                    = sessionData;
    uwbSessionsContext.status                             = 0;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (uwb_data_content == NULL) {
        NXPLOG_UWBAPI_E("%s: uwb_data_content is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
#if (UWBIOT_UWBD_SR04X)
    uint32_t sessionHandle = 0;
    if(uwb_data_content->isFindhubEnabled  == false){
        uint8_t uwb_spec_version_major[] = UWB_ANDROID_SPEC_VERSION_MAJOR;
        uint8_t uwb_spec_version_minor[] = UWB_ANDROID_SPEC_VERSION_MINOR;

        /* Fill in UWB spec version */
        phOsalUwb_MemCopy(uwb_data_content->spec_ver_major, uwb_spec_version_major, sizeof(uwb_spec_version_major));
        phOsalUwb_MemCopy(uwb_data_content->spec_ver_minor, uwb_spec_version_minor, sizeof(uwb_spec_version_minor));

        /* Get info of all UWBsessions */
        status = UwbApi_GetAllUwbSessions(&uwbSessionsContext);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_GetAllUwbSessions Failed");
            goto exit;
        }
        /* check if there is no session present, then only call UwbApi_GetDeviceInfo() */
        if (uwbSessionsContext.sessioncnt == 0) {
            status = UwbApi_GetDeviceInfo(&devInfo);
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E("UwbApi_GetDeviceInfo failed");
                goto exit;
            }
        }
        /* Fill in UWB chip FW version */
        uwb_data_content->chip_fw_version[0] = devInfo.fwMajor;
        uwb_data_content->chip_fw_version[1] = devInfo.fwMinor;

        /* Fill in the MW version */
        uwb_data_content->mw_version[0] = devInfo.mwMajor;
        uwb_data_content->mw_version[1] = devInfo.mwMinor;
        uwb_data_content->mw_version[2] = 0x00;

    #if UWBIOT_UWBD_SR150
        /* Check the chip type */
        if (phOsalUwb_MemCompare(devInfo.devName, "SR150_PROD_IOT_ROW", devInfo.devNameLen) != 0) {
            uwb_data_content->chip_id[1] = 0xFF;
        }
        else {
            uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
        }
    #elif UWBIOT_UWBD_SR040 /* TODO: SR048 - Needs an update? */
        /* Check the chip type */
        if (phOsalUwb_MemCompare(devInfo.devName, "SR040", 5) != 0) {
            uwb_data_content->chip_id[1] = 0xFF;
        }
        else {
            uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
        }
    #elif UWBIOT_UWBD_SR250
        /* Check the chip type */
        if (phOsalUwb_MemCompare(devInfo.devName, "SR250_A1V2_PROD", devInfo.devNameLen) != 0) {
            uwb_data_content->chip_id[1] = 0xFF;
        }
        else {
            uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
        }
    #endif // UWBIOT_UWBD_SR250
        /* Fill in supported UWB profile Ids */
        uwb_data_content->supported_config_ids = UWB_SUPPORTED_CONFIG_IDS;
    }
    else{
        /* Fill in supported UWB profile Ids */
        uwb_data_content->supported_config_ids = UWB_SUPPORTED_CONFIG_IDS_FIND_HUB;
        uwb_data_content->ranging_technology_role = RANGING_TECHNOLOGY_UWB;
        status = UwbApi_SessionInit(0x11223344, UWBD_RANGING_SESSION, &sessionHandle);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SessionInit() Failed");
            goto exit;
        }
        UWB_AppParams_List_t GetAppParamsList[] = {
            UWB_SET_GETAPP_PARAM(RANGING_DURATION),
            UWB_SET_GETAPP_PARAM(SLOT_DURATION),
            UWB_SET_GETAPP_PARAM(CHANNEL_NUMBER),
            UWB_SET_GETAPP_PARAM(PREAMBLE_CODE_INDEX),
        };

        status = UwbApi_GetAppConfigMultipleParams(sessionHandle,
            sizeof(GetAppParamsList) / sizeof(GetAppParamsList[0]),
            &GetAppParamsList[0]);
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E("UwbApi_SessionInit() Failed");
                goto exit;
            }
        uwb_data_content->ranging_interval = (uint16_t)GetAppParamsList[0].param_value.vu32;
        uwb_data_content->slot_duration = GetAppParamsList[1].param_value.vu32/1200;
        uwb_data_content->supported_channels = SET_SUPPORTED_CHANNEL(GetAppParamsList[2].param_value.vu32);
        uwb_data_content->preamble_index     = SET_PREAMBLE_INDEX(GetAppParamsList[3].param_value.vu32);
        uwb_data_content->payload_size = RANGING_CAP_RESPONSE_LENGTH_ANDROID;
        status = UwbApi_SessionDeinit(sessionHandle);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_SessionInit() Failed");
            goto exit;
        }
    }
#else
        uint8_t uwb_spec_version_major[] = UWB_ANDROID_SPEC_VERSION_MAJOR;
        uint8_t uwb_spec_version_minor[] = UWB_ANDROID_SPEC_VERSION_MINOR;

        /* Fill in UWB spec version */
        phOsalUwb_MemCopy(uwb_data_content->spec_ver_major, uwb_spec_version_major, sizeof(uwb_spec_version_major));
        phOsalUwb_MemCopy(uwb_data_content->spec_ver_minor, uwb_spec_version_minor, sizeof(uwb_spec_version_minor));

        /* Get info of all UWBsessions */
        status = UwbApi_GetAllUwbSessions(&uwbSessionsContext);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("UwbApi_GetAllUwbSessions Failed");
            goto exit;
        }
        /* check if there is no session present, then only call UwbApi_GetDeviceInfo() */
        if (uwbSessionsContext.sessioncnt == 0) {
            status = UwbApi_GetDeviceInfo(&devInfo);
            if (status != UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_E("UwbApi_GetDeviceInfo failed");
                goto exit;
            }
        }
        /* Fill in UWB chip FW version */
        uwb_data_content->chip_fw_version[0] = devInfo.fwMajor;
        uwb_data_content->chip_fw_version[1] = devInfo.fwMinor;

        /* Fill in the MW version */
        uwb_data_content->mw_version[0] = devInfo.mwMajor;
        uwb_data_content->mw_version[1] = devInfo.mwMinor;
        uwb_data_content->mw_version[2] = 0x00;

    #if UWBIOT_UWBD_SR150
        /* Check the chip type */
        if (phOsalUwb_MemCompare(devInfo.devName, "SR150_PROD_IOT_ROW", devInfo.devNameLen) != 0) {
            uwb_data_content->chip_id[1] = 0xFF;
        }
        else {
            uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
        }
    #elif UWBIOT_UWBD_SR040 /* TODO: SR048 - Needs an update? */
        /* Check the chip type */
        if (phOsalUwb_MemCompare(devInfo.devName, "SR040", 5) != 0) {
            uwb_data_content->chip_id[1] = 0xFF;
        }
        else {
            uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
        }
    #elif UWBIOT_UWBD_SR250
        /* Check the chip type */
        if (phOsalUwb_MemCompare(devInfo.devName, "SR250_A1V2_PROD", devInfo.devNameLen) != 0) {
            uwb_data_content->chip_id[1] = 0xFF;
        }
        else {
            uwb_data_content->chip_id[1] = MODELID_CHIP_TYPE;
        }
    #endif // UWBIOT_UWBD_SR250
        /* Fill in supported UWB profile Ids */
        uwb_data_content->supported_config_ids = UWB_SUPPORTED_CONFIG_IDS;
#endif/* (UWBIOT_UWBD_SR04X) */
    /* Fill in the supported uwb profile ids */
    uwb_data_content->ranging_role = UWB_SUPPORTED_DEVICE_RANGING_ROLES;

    /* Generate mac address */
    status = UwbApi_GetTrng(SHORT_MAC_ADDR_LEN, uwb_data_content->device_mac_addr);

    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("UwbApi_GetTrng() Failed");
        goto exit;
    }

exit:
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

#endif // (UWBFTR_BlobParser)

tUWBAPI_STATUS UwbApi_GetDeviceCapability(phUwbCapInfo_t *pDevCap)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pDevCap == NULL) {
        NXPLOG_UWBAPI_E("%s: pDevCap is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = getCapsInfo();

    if (status == UWBAPI_STATUS_OK) {
        if (parseCapabilityInfo(pDevCap) == FALSE) {
            NXPLOG_UWBAPI_E("%s: Parsing Capability Information Failed", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Parsing Capability Information Timed Out", __FUNCTION__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get Capability Information failed", __FUNCTION__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

#if UWBFTR_DataTransfer
EXTERNC tUWBAPI_STATUS UwbApi_SendData(phUwbDataPkt_t *pSendData)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    uint16_t cmdLen       = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pSendData == NULL || pSendData->data == NULL) {
        NXPLOG_UWBAPI_E("%s: pSendData is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (pSendData->data_size + SEND_DATA_HEADER_LEN > uwbContext.maxDataPacketPayloadSize) {
        NXPLOG_UWBAPI_E(
            "%s:%d: pDataPack length is more than %d", __FUNCTION__, __LINE__, uwbContext.maxDataPacketPayloadSize);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    cmdLen = serializeSendDataPayload(pSendData, uwbContext.snd_data);
    status = sendData(uwbContext.snd_data, cmdLen, Link_Layer_Mode_Bypass);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("%s: failed", __FUNCTION__);
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_LogicalLinkSendData(phLogicalLinkDataPkt_t *pllSendData)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    uint16_t cmdLen       = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pllSendData == NULL || pllSendData->data == NULL) {
        NXPLOG_UWBAPI_E("%s: pllSendData is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (pllSendData->data_size + SEND_DATA_HEADER_LEN > uwbContext.maxDataPacketPayloadSize) {
        NXPLOG_UWBAPI_E(
            "%s:%d: pDataPack length is more than %d", __FUNCTION__, __LINE__, uwbContext.maxDataPacketPayloadSize);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    cmdLen = serializeLogicalSendDataPayload(pllSendData, uwbContext.snd_data);
    status = sendData(uwbContext.snd_data, cmdLen, Link_Layer_Mode_LogicalLink);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("%s: failed", __FUNCTION__);
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif // UWBFTR_DataTransfer

#if !(UWBIOT_UWBD_SR04X)
tUWBAPI_STATUS UwbApi_SessionQueryDataSize(phUwbQueryDataSize_t *pQueryDataSize)
{
    tUWBAPI_STATUS status;
    uint32_t rspSessionId;
    uint8_t *pResponse = NULL;
    uint16_t cmdLen    = 0;
    uint32_t index = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pQueryDataSize == NULL) {
        NXPLOG_UWBAPI_E("%s: pDataSize is invalid", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_RSP_EVT);
    cmdLen = serializeSessionHandlePayload(pQueryDataSize->connectionId, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        /* Catch the Response of 6 bytes */
        pResponse = &uwbContext.rsp_data[0];
        /* Copy the sessionHandle Received */
        UWB_STREAM_TO_UINT32(rspSessionId, pResponse, index);
        /** Skip the status code */
        index++;
        /* Compare the Receved Session Handle with Sent Session Handle */
        if (pQueryDataSize->connectionId == rspSessionId) {
            /* Copy the DataSize of 2 bytes*/
            UWB_STREAM_TO_UINT16(pQueryDataSize->dataSize, pResponse, index);
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Session Handle", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Timed Out with status :%d", __FUNCTION__, status);
    }
    else if (status == UWBAPI_STATUS_UNKNOWN) {
        NXPLOG_UWBAPI_E("%s: Unknown with status :%d", __FUNCTION__, status);
    }
    else {
        NXPLOG_UWBAPI_E("%s: failed with status :%d", __FUNCTION__, status);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif // !(UWBIOT_UWBD_SR04X)

#if !(UWBIOT_UWBD_SR04X)
EXTERNC tUWBAPI_STATUS UwbApi_SetControllerHusSession(phControllerHusSessionConfig_t *pHusSessionCfg)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pHusSessionCfg == NULL) {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if((NULL == pHusSessionCfg->phase_list) || (0 == pHusSessionCfg->phase_count))
    {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg->phase_list is 0x%p and phase count is %d",
        __FUNCTION__, pHusSessionCfg->phase_list, pHusSessionCfg->phase_count);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_HUS_CONTROLLER_CONFIG_RSP_EVT);
    cmdLen = serializeControllerHusSessionPayload(pHusSessionCfg, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_HUS_CONTROLLER_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SetControleeHusSession(phControleeHusSessionConfig_t *pHusSessionCfg)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pHusSessionCfg == NULL) {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if((NULL == pHusSessionCfg->phase_list) || (0 == pHusSessionCfg->phase_count))
    {
        NXPLOG_UWBAPI_E("%s: pHusSessionCfg->phase_list is 0x%p and phase count is %d",
        __FUNCTION__, pHusSessionCfg->phase_list, pHusSessionCfg->phase_count);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_HUS_CONTROLEE_CONFIG_RSP_EVT);
    cmdLen = serializeControleeHusSessionPayload(pHusSessionCfg, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_HUS_CONTROLEE_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SessionDtpcmConfig(phDataTxPhaseConfig_t *phDataTxPhaseCfg)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;
    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (phDataTxPhaseCfg == NULL || phDataTxPhaseCfg->dtpml == NULL) {
        NXPLOG_UWBAPI_E(
            "%s: phDataTxPhaseCfg (0x%p) or phDataTxPhaseCfg->dtpml is NULL", __FUNCTION__, phDataTxPhaseCfg);
        NXPLOG_UWBAPI_E("%s: phDataTxPhaseCfg is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_RSP_EVT);
    cmdLen = serializeDtpcmPayload(phDataTxPhaseCfg, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_EVT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
        /* DTPCM Notification will only be received in active state */
        if (uwbContext.sessionInfo.state == UWB_SESSION_ACTIVE) {
            status = waitforNotification(UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_NTF_EVT, UWB_NTF_TIMEOUT);
            if (status != UWBAPI_STATUS_OK) {
                return UWBAPI_STATUS_FAILED;
            }
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
#endif // !(UWBIOT_UWBD_SR04X)

#if UWBFTR_DataTransfer
tUWBAPI_STATUS UwbApi_CreateLogicalLink(
    phLogicalLinkCreateCmd_t *phLogicalLinkCreateCmd, uint32_t *pLogicalLinkConnectId)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;
    uint32_t index = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        status = UWBAPI_STATUS_NOT_INITIALIZED;
        goto exit;
    }

    if (phLogicalLinkCreateCmd == NULL || pLogicalLinkConnectId == NULL) {
        NXPLOG_UWBAPI_E("%s: phLogicalLinkCreateCmd or pLogicalLinkConnectId is NULL", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_CREATE_LOGICAL_LINK_RSP_EVT);
    cmdLen = serializeCreateLogicalLinkCmd(phLogicalLinkCreateCmd, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_SESSION_LOGICAL_LINK_CREATE_EVT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
        if (uwbContext.rsp_len > 0) {
            uint8_t *rspPtr = uwbContext.rsp_data;
            // skip the status from response.
            index++;
            // copy the Logical Link Id received through response.
            UWB_STREAM_TO_UINT32(*pLogicalLinkConnectId, rspPtr, index);
        }
        else {
            LOG_E("%s: Incorrect LinkId", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
            goto exit;
        }
    }
    else {
        LOG_E("%s: sendUciCommandAndWait failed", __FUNCTION__);
        goto exit;
    }

exit:
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

tUWBAPI_STATUS UwbApi_CloseLogicalLink(uint32_t LogicalLinkConnectId)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        status = UWBAPI_STATUS_NOT_INITIALIZED;
        goto exit;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_LOGICAL_LINK_CLOSE_RSP_EVT);
    cmdLen = serializeLinkConnectIdPayload(LogicalLinkConnectId, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_SESSION_LOGICAL_LINK_CLOSE_EVT, cmdLen, uwbContext.snd_data);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("%s: sendUciCommandAndWait failed", __FUNCTION__);
        goto exit;
    }
    /** Once the UWBS has closed the logical link it shall send the LOGICAL_LINK_UWBS_CLOSE_NTF,
     * The UWBS may take some time to actually perfrom this activity,
     * e.g.in CO mode to inform the remote device.
     * The Notification data will be passed to the Application through AppCallback.
     */
exit:
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

tUWBAPI_STATUS UwbApi_LogicalLinkGetParams(
    uint32_t ConnectionIdentifier, phLogicalLinkGetParamsRsp_t *phLogicalLinkGetParamsRsp)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        status = UWBAPI_STATUS_NOT_INITIALIZED;
        goto exit;
    }

    if (phLogicalLinkGetParamsRsp == NULL) {
        NXPLOG_UWBAPI_E("%s: phLogicalLinkGetParamsRsp is NULL", __FUNCTION__);
        status = UWBAPI_STATUS_INVALID_PARAM;
        goto exit;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_LOGICAL_LINK_GET_PARAM_RSP_EVT);
    cmdLen = serializeLinkConnectIdPayload(ConnectionIdentifier, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_SESSION_LOGICAL_LINK_GET_PARAM_EVT, cmdLen, uwbContext.snd_data);
    if (status == UWBAPI_STATUS_OK) {
        if (uwbContext.rsp_len > 0) {
            uint8_t *rspPtr = uwbContext.rsp_data;
            deserializeLinkGetParamsPayload(rspPtr, phLogicalLinkGetParamsRsp);
        }
        else {
            LOG_E("%s: Incorrect response length", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
            goto exit;
        }
    }
    else {
        LOG_E("%s: sendUciCommandAndWait failed", __FUNCTION__);
        goto exit;
    }

exit:
    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

#endif // UWBFTR_DataTransfer
