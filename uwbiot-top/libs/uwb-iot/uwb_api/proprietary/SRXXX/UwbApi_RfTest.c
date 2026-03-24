/*
 *
 * Copyright 2018-2020,2022,2023,2026 NXP.
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

#include "UwbApi_RfTest.h"
#include "phNxpLogApis_UwbApi.h"

#include "uci_defs.h"
#include "uci_test_defs.h"
#include "uci_ext_defs.h"
#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary.h"
#include "UwbApi_Proprietary_Internal.h"
#include "AppConfigParams.h"
#include "UwbAdaptation.h"
#include "UwbApi_Utility.h"
#include "uwa_dm_int.h"
#include "uwb_int.h"

/** Local functions prototypes */
static void parsePerTestParams(uint8_t *rspPtr, uint8_t noOfParams, phRfTestParams_t *pRfTestParams);

/**
 **
 ** Function:        parsePerTestParams
 **
 ** Description:     Extracts PER Test config Params from the given byte array
 *and updates the structure
 **
 ** Returns:         None
 **
 */
static void parsePerTestParams(uint8_t *rspPtr, uint8_t noOfParams, phRfTestParams_t *pRfTestParams)
{
    uint8_t paramId;
    uint32_t index = 0;

    for (int i = 0; i < noOfParams; i++) {
        UWB_STREAM_TO_UINT8(paramId, rspPtr, index);
        ++index; // Skip the length field
        switch (paramId) {
        case UCI_TEST_PARAM_ID_NUM_PACKETS:
            UWB_STREAM_TO_UINT32(pRfTestParams->numOfPckts, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_T_GAP:
            UWB_STREAM_TO_UINT32(pRfTestParams->tGap, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_T_START:
            UWB_STREAM_TO_UINT32(pRfTestParams->tStart, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_T_WIN:
            UWB_STREAM_TO_UINT32(pRfTestParams->tWin, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_RANDOMIZE_PSDU:
            UWB_STREAM_TO_UINT8(pRfTestParams->randomizedSize, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_PHR_RANGING_BIT:
            UWB_STREAM_TO_UINT8(pRfTestParams->phrRangingBit, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_RMARKER_RX_START:
            UWB_STREAM_TO_UINT32(pRfTestParams->rmarkerRxStart, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_RMARKER_TX_START:
            UWB_STREAM_TO_UINT32(pRfTestParams->rmarkerTxStart, rspPtr, index);
            break;
        case UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR:
            UWB_STREAM_TO_UINT8(pRfTestParams->stsIndexAutoIncr, rspPtr, index);
            break;
        default:
            break;
        }
    }
}

EXTERNC tUWBAPI_STATUS UwbApi_SetRfTestParams(uint32_t sessionHandle, const phRfTestParams_t *pRfTestParams)
{
    tUWBAPI_STATUS status;
    uint8_t noOfPerParams = 0;
    uint8_t paramLen      = 0;
    uint16_t cmdLen       = 0;
    size_t appConfigDataLen   = 0;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    uint8_t noOfControlees   = 1;
    uint8_t deviceMacAddr[2] = {0x11, 0x11};
    uint8_t dstMacAddr[2]    = {0x22, 0x22};

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pRfTestParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pPerParam is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_NUM_PACKETS,
                                        sizeof(pRfTestParams->numOfPckts),
                                        (void *)&pRfTestParams->numOfPckts,
                                        &uwbContext.snd_data[payloadOffset]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_T_GAP,
                                        sizeof(pRfTestParams->tGap),
                                        (void *)&pRfTestParams->tGap,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_T_START,
                                        sizeof(pRfTestParams->tStart),
                                        (void *)&pRfTestParams->tStart,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_T_WIN,
                                        sizeof(pRfTestParams->tWin),
                                        (void *)&pRfTestParams->tWin,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_RANDOMIZE_PSDU,
                                        sizeof(pRfTestParams->randomizedSize),
                                        (void *)&pRfTestParams->randomizedSize,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_PHR_RANGING_BIT,
                                        sizeof(pRfTestParams->phrRangingBit),
                                        (void *)&pRfTestParams->phrRangingBit,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_RMARKER_TX_START,
                                        sizeof(pRfTestParams->rmarkerTxStart),
                                        (void *)&pRfTestParams->rmarkerTxStart,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_RMARKER_RX_START,
                                        sizeof(pRfTestParams->rmarkerRxStart),
                                        (void *)&pRfTestParams->rmarkerRxStart,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR,
                                        sizeof(pRfTestParams->stsIndexAutoIncr),
                                        (void *)&pRfTestParams->stsIndexAutoIncr,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_STS_DETECT_BITMAP_EN,
                                        sizeof(pRfTestParams->sts_detect_bitmap_en),
                                        (void *)&pRfTestParams->sts_detect_bitmap_en,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    sep_SetWaitEvent(UWA_DM_TEST_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfPerParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_TEST_SET_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status != UCI_STATUS_OK) {
        return status;
    }

    noOfPerParams = 0;
    payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;
    paramLen      = 0;
    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_NO_OF_CONTROLEES,
                                        UCI_PARAM_LEN_NO_OF_CONTROLEES,
                                        (void *)&noOfControlees,
                                        &uwbContext.snd_data[payloadOffset]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_MAC_ADDRESS,
                                        UCI_PARAM_LEN_DEVICE_MAC_ADDRESS,
                                        (void *)&deviceMacAddr,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    appConfigDataLen = getAppConfigTLVBuffer(UCI_PARAM_ID_DST_MAC_ADDRESS,
                                        (uint8_t)(UCI_PARAM_LEN_DEST_MAC_ADDRESS * noOfControlees),
                                        (void *)&dstMacAddr,
                                        &uwbContext.snd_data[payloadOffset + paramLen]);
    if (paramLen < (UINT8_MAX - appConfigDataLen)) {
        paramLen = (uint8_t)(paramLen + appConfigDataLen);
    } else {
        NXPLOG_UWBAPI_E("%s: Parameter length overflow. paramLen=%u",
            __FUNCTION__, paramLen);
        return UWBAPI_STATUS_FAILED;
    }
    ++noOfPerParams; // Increment the number of debug params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfPerParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_SESSION_SET_APP_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if ((status == UWBAPI_STATUS_OK) && (uwbContext.sessionInfo.state != UWB_SESSION_IDLE)) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.sessionHandle != sessionHandle || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_IDLE notification", __FUNCTION__);
            status = UWBAPI_STATUS_FAILED;
            return status;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetRfTestParams(uint32_t sessionHandle, phRfTestParams_t *pRfTestParams)
{
    tUWBAPI_STATUS status;
    uint8_t *pGetRfParamsCommand = NULL;
    uint16_t index               = 0;
    uint16_t paramId             = 0;
    uint8_t noOfParams;
    uint16_t cmdLen       = 0;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    NXPLOG_UWBAPI_D("%s: Enter", __FUNCTION__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRfTestParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pPerParams is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    /* Get Test Config */
    noOfParams          = (uint8_t)(uciRfTest_TestParamIds_len / sizeof(uint8_t));
    pGetRfParamsCommand = &uwbContext.snd_data[0];
    for (index = 0; index < noOfParams; index++) {
        paramId = uciRfTest_TestParamIds[index];
        UWB_UINT8_TO_STREAM(pGetRfParamsCommand, paramId, payloadOffset);
        NXPLOG_UWBAPI_D("%s: App ID: %02X", __FUNCTION__, paramId);
    }
    sep_SetWaitEvent(UWA_DM_TEST_GET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, noOfParams, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_TEST_GET_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        parsePerTestParams(rspPtr, noOfParams, pRfTestParams);
    }
    else {
        NXPLOG_UWBAPI_D("%s: Failed UWA_TestGetConfig", __FUNCTION__);
        return status;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __FUNCTION__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_SetTestConfig(uint32_t sessionHandle, eTestConfig param_id, uint32_t param_value)
{
    uint8_t paramLen      = 0;
    uint8_t noOfParams    = 1;
    uint16_t cmdLen       = 0;
    uint8_t payloadOffset = SES_ID_AND_NO_OF_PARAMS_OFFSET;

    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((param_id >> 8) == EXTENDED_TEST_CONFIG_ID) {
        paramLen = getExtTestConfigTLVBuffer(param_id, (void *)&param_value, &uwbContext.snd_data[payloadOffset]);
    }
    else {
        paramLen = getTestConfigTLVBuffer(param_id, 0, (void *)&param_value, &uwbContext.snd_data[payloadOffset]);
    }

    if (paramLen == 0) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_TEST_SET_CONFIG_RSP_EVT);
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_TEST_SET_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_GetTestConfig(uint32_t sessionHandle, eTestConfig param_id, uint32_t *param_value)
{
    uint8_t len                    = 0;
    uint8_t offset                 = 0;
    uint8_t noOfParams             = 1;
    uint8_t paramLen               = 1;
    uint16_t cmdLen                = 0;
    uint8_t *pGetTestConfigCommand = NULL;
    uint8_t payloadOffset          = SES_ID_AND_NO_OF_PARAMS_OFFSET;

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

    sep_SetWaitEvent(UWA_DM_TEST_GET_CONFIG_RSP_EVT);
    pGetTestConfigCommand = &uwbContext.snd_data[0];
    if ((param_id >> 8) == EXTENDED_TEST_CONFIG_ID) {
        paramLen++;
        UWB_UINT8_TO_STREAM(pGetTestConfigCommand, param_id >> 8, payloadOffset);
        UWB_UINT8_TO_STREAM(pGetTestConfigCommand, param_id, payloadOffset);
    }
    else {
        UWB_UINT8_TO_STREAM(pGetTestConfigCommand, param_id, payloadOffset);
    }
    cmdLen = serializeAppConfigPayload(sessionHandle, noOfParams, paramLen, uwbContext.snd_data);
    status = sendUciCommandAndWait(UWA_DM_API_TEST_GET_CONFIG_EVT, cmdLen, uwbContext.snd_data);

    if (status == UWBAPI_STATUS_OK) {
        uint8_t *rspPtr = &uwbContext.rsp_data[0];
        offset++;
        if ((param_id >> 8) == EXTENDED_TEST_CONFIG_ID) {
            offset++;
        }
        UWB_STREAM_TO_UINT8(len, rspPtr, offset);

        if (len == sizeof(uint8_t)) {
            UWB_STREAM_TO_UINT8(*param_value, rspPtr, offset);
        }
        else if (len == sizeof(uint16_t)) {
            UWB_STREAM_TO_UINT16(*param_value, rspPtr, offset);
        }
        else if (len == sizeof(uint32_t)) {
            UWB_STREAM_TO_UINT32(*param_value, rspPtr, offset);
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_StartRfTest(eStartRfParam paramId, phRfStartData_t *pStartData)
{
    tUWBAPI_STATUS status;
    uint16_t cmdLen = 0;
    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((paramId != RF_TEST_RX) && pStartData == NULL) { // for Single Rx RF test psdu data is NUL
        NXPLOG_UWBAPI_E("%s: Input data is NULL", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    switch (paramId) {
    case RF_START_PER_TX:
        if (((phStartPerTxData_t *)pStartData)->txData == NULL) {
            NXPLOG_UWBAPI_E("%s: Input data is NULL for RF_START_PER_TX", __FUNCTION__);
            status = UWBAPI_STATUS_INVALID_PARAM;
            break;
        }
        sep_SetWaitEvent(UWA_DM_TEST_PERIODIC_TX_RSP_EVT);
        cmdLen = serializeTestDataPayload(((phStartPerTxData_t *)pStartData)->txDataLength,
            ((phStartPerTxData_t *)pStartData)->txData,
            uwbContext.snd_data);
        status = sendUciCommandAndWait(UWA_DM_API_TEST_PERIODIC_TX_EVT, cmdLen, uwbContext.snd_data);
        break;
    case RF_START_PER_RX:
        if (((phStartPerRxData_t *)pStartData)->rxData == NULL) {
            NXPLOG_UWBAPI_E("%s: Input data is NULL for RF_START_PER_RX", __FUNCTION__);
            status = UWBAPI_STATUS_INVALID_PARAM;
            break;
        }
        sep_SetWaitEvent(UWA_DM_TEST_PER_RX_RSP_EVT);
        cmdLen = serializeTestDataPayload(((phStartPerRxData_t *)pStartData)->rxDataLength,
            ((phStartPerRxData_t *)pStartData)->rxData,
            uwbContext.snd_data);
        status = sendUciCommandAndWait(UWA_DM_API_TEST_PER_RX_EVT, cmdLen, uwbContext.snd_data);
        break;
    case RF_LOOPBACK_TEST:
        if (((phLoopbackTestData_t *)pStartData)->loopbackData == NULL) {
            NXPLOG_UWBAPI_E("%s: Input data is NULL for RF_LOOPBACK_TEST", __FUNCTION__);
            status = UWBAPI_STATUS_INVALID_PARAM;
            break;
        }
        sep_SetWaitEvent(UWA_DM_TEST_LOOPBACK_RSP_EVT);
        cmdLen = serializeTestDataPayload(((phLoopbackTestData_t *)pStartData)->loopbackDataLength,
            ((phLoopbackTestData_t *)pStartData)->loopbackData,
            uwbContext.snd_data);
        status = sendUciCommandAndWait(UWA_DM_API_TEST_UWB_LOOPBACK_EVT, cmdLen, uwbContext.snd_data);
        break;
    case RF_TEST_RX:
        sep_SetWaitEvent(UWA_DM_TEST_RX_RSP_EVT);
        status = sendUciCommandAndWait(UWA_DM_API_TEST_RX_EVT, 0, NULL);
        break;
    case RF_TEST_SR_RX:
        sep_SetWaitEvent(UWA_DM_TEST_SR_RX_RSP_EVT);
        status = sendUciCommandAndWait(UWA_DM_API_TEST_SR_RX_EVT, 0, NULL);
        break;
    default:
        NXPLOG_UWBAPI_E("%s:    Invalid Param ", __FUNCTION__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_Stop_RfTest(void)
{
    tUWBAPI_STATUS status;

    NXPLOG_UWBAPI_D("%s: enter", __FUNCTION__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __FUNCTION__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_TEST_STOP_SESSION_RSP_EVT);
    status = sendUciCommandAndWait(UWA_DM_API_TEST_STOP_SESSION_EVT, 0, NULL);

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            status = UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __FUNCTION__, status);
    return status;
}
