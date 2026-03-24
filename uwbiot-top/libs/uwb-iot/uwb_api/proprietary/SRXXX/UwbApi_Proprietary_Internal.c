/*
 *
 * Copyright 2018-2026 NXP.
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

#include "UwbApi_Proprietary_Internal.h"
#include "UwbApi_Internal.h"
#include "phNxpLogApis_UwbApi.h"
#include "uwa_api.h"
#include "phOsalUwb.h"
#include "uci_ext_defs.h"
#include "UwbAdaptation.h"
#include "AppConfigParams.h"
#include "PrintUtility.h"
#include "phNxpUwbConfig.h"
#if UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71)
#include "UwbSeApi.h"
#endif // UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71)
#include "uwa_dm_int.h"
#include "uwb_int.h"
#include "UwbApi_Utility.h"
#include "uwbiot_ver.h"


/** Extended config header length (2 byte param id + 1 byte param len) */
#define UWBD_EXT_CONFIG_HEADER_LEN 0x03
/** DPD wakeup src GPIO1 */
#define DPD_WAKEUP_SRC_GPIO_1 2
/** DPD wakeup src GPIO3 */
#define DPD_WAKEUP_SRC_GPIO_3 4
/** WTX count min */
#define WTX_COUNT_MIN 20
/** WTX count max */
#define WTX_COUNT_MAX 180

/* skip sem posting op*/
BOOLEAN skip_sem_post = FALSE;

/**Local functions prototypes */
static void handle_schedstatus_ntf(uint8_t *p, uint16_t len);
#if UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250
static void handle_bindingStatus_ntf(uint8_t *p, uint16_t len);
#endif /** UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250 */
#if UWBIOT_SESN_SNXXX
static void handle_eseGetSessionIdList(uint8_t *p, uint16_t len);
#endif // UWBIOT_SESN_SNXXX
static void handle_do_chip_calibration_ntf(uint8_t *p, uint16_t len);
#if UWBIOT_UWBD_SR1XXT
static void handle_read_calibration_data_ntf(uint8_t *p, uint16_t len);
#endif // UWBIOT_UWBD_SR1XXT
#if UWBFTR_UWBS_DEBUG_Dump
static void handle_debug_ntf(eResponse_Ext_Ntf_Event event, uint8_t *p, uint16_t len);
#endif // UWBFTR_UWBS_DEBUG_Dump
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)
static void handle_se_com_err_ntf(uint8_t *p, uint16_t len);
#endif // #if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)
#if (UWBIOT_SESN_SNXXX)
static void handle_se_binding_ntf(uint8_t *p, uint16_t len);
#endif //(UWBIOT_SESN_SNXXX)

#if UWBIOT_UWBD_SR1XXT
#if (UWBFTR_AoA_FoV)
#if UWBIOT_UWBD_SR150
static const unsigned char aoa_ch5_config_block_names[] = {
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH5,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH5,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH5,
};

static const unsigned char aoa_ch9_config_block_names[] = {
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH9,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH9,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH9,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH9,
};

#else
static const unsigned char aoa_config_block_names[] = {
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH9,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH9,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH5,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH9,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH5,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH9,
};
#endif // UWBIOT_UWBD_SR150
#endif // (UWBFTR_AoA_FoV)
#endif // UWBIOT_UWBD_SR1XXT
/** Block Ids for Vendor Configurations */
static const unsigned char nxp_config_block_names[] = {UWB_NXP_CORE_CONFIG_BLOCK_1,
    UWB_NXP_CORE_CONFIG_BLOCK_2,
    UWB_NXP_CORE_CONFIG_BLOCK_3,
    UWB_NXP_CORE_CONFIG_BLOCK_4,
    UWB_NXP_CORE_CONFIG_BLOCK_5,
    UWB_NXP_CORE_CONFIG_BLOCK_6,
    UWB_NXP_CORE_CONFIG_BLOCK_7,
    UWB_NXP_CORE_CONFIG_BLOCK_8,
    UWB_NXP_CORE_CONFIG_BLOCK_9,
    UWB_NXP_CORE_CONFIG_BLOCK_10};

#if UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71)

void reset_se_on_error(void)
{
    uint16_t RespSize     = sizeof(uwbContext.rsp_data);
    uint8_t eSeRsetBuf[3] = {0x2F, 0x1E, 0x00}; // command to reset the eSE

    if (UwbSeApi_NciRawCmdSend(sizeof(eSeRsetBuf), eSeRsetBuf, &RespSize, uwbContext.rsp_data) != 0) {
        NXPLOG_UWBAPI_E("eSE reset failure");
    }
    else {
        NXPLOG_UWBAPI_D("eSE reset success");
    }
}
#endif /*UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71)*/

tUWBAPI_STATUS DebugConfig_TlvParser(
    const UWB_DebugParams_List_t *pDebugParams_List, UWB_Debug_Params_value_t *pOutput_param_value)
{
    tUWBAPI_STATUS status;
    uint8_t *param_value = pDebugParams_List->param_value.param.param_value;
    uint32_t index = 0;
    switch (pDebugParams_List->param_type) {
    case kUWB_DEBUGPARAMS_Type_u8:
        pOutput_param_value->param_len = sizeof(uint8_t);
        UWB_UINT8_TO_FIELD(pOutput_param_value->param_value, pDebugParams_List->param_value.vu8);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_DEBUGPARAMS_Type_u16:
        pOutput_param_value->param_len = sizeof(uint16_t);
        UWB_UINT16_TO_FIELD(pOutput_param_value->param_value, pDebugParams_List->param_value.vu16);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_DEBUGPARAMS_Type_u32:
        pOutput_param_value->param_len = sizeof(uint32_t);
        UWB_UINT32_TO_FIELD(pOutput_param_value->param_value, pDebugParams_List->param_value.vu32);
        status = UWBAPI_STATUS_OK;
        break;
    case kUWB_DEBUGPARAMS_Type_au8:
        pOutput_param_value->param_len = pDebugParams_List->param_value.param.param_len;
        UWB_STREAM_TO_ARRAY(pOutput_param_value->param_value, param_value, pOutput_param_value->param_len, index);
        status = UWBAPI_STATUS_OK;
        break;
    default:
        status = UWBAPI_STATUS_FAILED;
        break;
    }
    return status;
}

uint8_t getVendorAppConfigTLVBuffer(uint8_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer)
{
    uint32_t length     = 0;
    tlvBuffer[length++] = paramId;
    if (paramValueLen > MAX_UCI_PACKET_SIZE) {
        LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
        length = 0;
        return (uint8_t)length;
    }
    switch (paramId) {
        /* Length 1 Byte */
#if (UWBFTR_UL_TDoA_Tag)
    case UCI_VENDOR_PARAM_ID_ULTDOA_MAC_FRAME_FORMAT:
#endif // (UWBFTR_UL_TDoA_Tag)
    case UCI_VENDOR_PARAM_ID_MAC_PAYLOAD_ENCRYPTION:
    case UCI_VENDOR_PARAM_ID_SESSION_SYNC_ATTEMPTS:
    case UCI_VENDOR_PARAM_ID_SESSION_SCHED_ATTEMPTS:
    case UCI_VENDOR_PARAM_ID_SCHED_STATUS_NTF:
    case UCI_VENDOR_PARAM_ID_TX_POWER_DELTA_FCC:
    case UCI_VENDOR_PARAM_ID_TEST_KDF_FEATURE:
    case UCI_VENDOR_PARAM_ID_ADAPTIVE_HOPPING_THRESHOLD:
    case UCI_VENDOR_PARAM_ID_RAN_MULTIPLIER:
    case UCI_VENDOR_PARAM_ID_STS_LAST_INDEX_USED:
    case UCI_VENDOR_PARAM_ID_DATA_TRANSFER_TX_STATUS_CONFIG:
    case UCI_VENDOR_PARAM_ID_WIFI_COEX_MAX_TOLERANCE_COUNT:
    case UCI_VENDOR_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER:
#if UWBFTR_CSA
    case UCI_VENDOR_PARAM_ID_RESYNC_ENABLE:
    case UCI_VENDOR_PARAM_ID_CSA_FINAL_DATA2_CONFIG:
#endif // UWBFTR_CSA
    case UCI_VENDOR_PARAM_ID_AUTHENTICITY_TAG:
    case UCI_VENDOR_PARAM_ID_TX_POWER_TEMP_COMPENSATION:
    case UCI_VENDOR_PARAM_ID_MAC_CFG:
    case UCI_VENDOR_PARAM_ID_CIR_LOG_NTF:
    case UCI_VENDOR_PARAM_ID_PSDU_LOG_NTF:
    case UCI_VENDOR_PARAM_ID_SESSION_INBAND_DATA_TX_BLOCKS:
    case UCI_VENDOR_PARAM_ID_SESSION_INBAND_DATA_RX_BLOCKS:
    case UCI_VENDOR_PARAM_ID_RFRAME_LOG_NTF:
#if (UWBFTR_AoA_FoV)
#if UWBIOT_UWBD_SR150
    case UCI_VENDOR_PARAM_ID_SWAP_ANTENNA_PAIR_3D_AOA:
#endif // UWBIOT_UWBD_SR150
    case UCI_VENDOR_PARAM_ID_FOV_ENABLE:
#endif // UWBFTR_AoA_FoV
#if UWBIOT_UWBD_SR150
    case UCI_VENDOR_PARAM_ID_ALIRO_CONTROLEE_EXTENSIONS:
#endif //UWBIOT_UWBD_SR150
    case UCI_VENDOR_PARAM_ID_ENABLE_FOM:
#if UWBFTR_Radar
    case UCI_VENDOR_PARAM_ID_RADAR_MODE:
    case UCI_VENDOR_PARAM_ID_RADAR_SINGLE_FRAME_NTF:
    case UCI_VENDOR_PARAM_ID_RADAR_CIR_NUM_SAMPLES:
    case UCI_VENDOR_PARAM_ID_RADAR_FCC_TEST_MODE:
    case UCI_VENDOR_PARAM_ID_RADAR_DC_FREEZE:
    case UCI_VENDOR_PARAM_ID_RADAR_PULSE_SHAPE:
#endif // UWBFTR_Radar
    case UCI_VENDOR_PARAM_ID_SESSION_INFO_NTF_FILTER_NUM :
    {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;

    /* Length 2 Bytes */
    case UCI_VENDOR_PARAM_ID_CIR_CAPTURE_MODE:
    case UCI_VENDOR_PARAM_ID_RX_NBIC_CONFIG:
    case UCI_VENDOR_PARAM_ID_RML_PROXIMITY_CONFIG:
#if UWBFTR_Radar
    case UCI_VENDOR_PARAM_ID_RADAR_DRIFT_COMPENSATION:
#endif //  UWBFTR_Radar
    {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;

    /* Length 4 Bytes */
    case UCI_VENDOR_PARAM_ID_RSSI_AVG_FILT_CNT:
#if UWBFTR_Radar
    case UCI_VENDOR_PARAM_ID_RRADAR_RX_GAIN:
#endif // UWBFTR_Radar
    {
        tlvBuffer[length++] = sizeof(uint32_t); // Param len
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;

    /* uint8_t array */
#if (UWBFTR_AoA_FoV)
    case UCI_VENDOR_PARAM_ID_AZIMUTH_FIELD_OF_VIEW:
#endif // UWBFTR_AoA_FoV
    case UCI_VENDOR_PARAM_ID_SET_USECASE_ID:
#if UWBFTR_CSA
    case UCI_VENDOR_PARAM_ID_INITIAL_SYNC_RX_WINDOW_CONFIG:
#endif // UWBFTR_CSA
#if UWBFTR_Radar
    case UCI_VENDOR_PARAM_ID_RADAR_CIR_START_OFFSET:
    case UCI_VENDOR_PARAM_ID_RADAR_RFRI:
    case UCI_VENDOR_PARAM_ID_RADAR_CFG:
    case UCI_VENDOR_PARAM_ID_RADAR_PRESENCE_DET_CFG:
#endif // UWBFTR_Radar
    case UCI_VENDOR_PARAM_ID_ANTENNAE_CONFIGURATION_TX:
    case UCI_VENDOR_PARAM_ID_ANTENNAE_CONFIGURATION_RX:
    {
        if ((paramValueLen + length) <= MAX_UCI_PACKET_SIZE) {
            tlvBuffer[length++] = (uint8_t)paramValueLen; // Param len
            phOsalUwb_MemCopy(&tlvBuffer[length], (uint8_t *)paramValue, paramValueLen);
            length += paramValueLen;
        }
        else {
            LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
            length = 0;
        }
    } break;
    default:
        LOG_E("UCI Extended parameter '0x%04X' not supported", paramId);
        length = 0;
        break;
    }
    if (length > MAX_UCI_PACKET_SIZE) {
        LOG_E("Max UCI Packet size supported for the Session App/EXT configs is %d bytes and received %d bytes",
            MAX_UCI_PACKET_SIZE,
            length);
        length = 0;
    }
    return (uint8_t)length;
}

uint8_t getExtTestConfigTLVBuffer(uint16_t paramId, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length       = 0;
    uint8_t uci_param_id = 0;
    uci_param_id         = (uint8_t)(paramId & 0x00FF);

    tlvBuffer[length++] = EXTENDED_TEST_CONFIG_ID;
    tlvBuffer[length++] = uci_param_id;

    switch (uci_param_id) {
    case UCI_EXT_TEST_PARAM_ID_RSSI_CALIBRATION_OPTION:
    case UCI_EXT_TEST_SESSION_STS_KEY_OPTION: {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    case UCI_EXT_TEST_PARAM_ID_AGC_GAIN_VAL_RX: {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;
    default:
        length = 0;
        break;
    }
    return length;
}

uint8_t getExtDeviceConfigTLVBuffer(uint8_t paramId, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = 0;

    tlvBuffer[length++] = EXTENDED_DEVICE_CONFIG_ID;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT: {
        tlvBuffer[length++] = sizeof(uint16_t); // Param len
        uint16_t value      = *((uint16_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
    } break;
#if UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_GPIO_SELECTION_FOR_DUAL_AOA:
#endif // UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC:
#if !(UWBIOT_UWBD_SR2XXT)
    case UCI_EXT_PARAM_ID_WTX_COUNT:
#endif //!(UWBIOT_UWBD_SR2XXT)
    case UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG: {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    default:
        length = 0;
        break;
    }
    return length;
}

uint8_t getVendorDebugConfigTLVBuffer(uint16_t paramId, void *paramValue, uint16_t paramValueLen, uint8_t *tlvBuffer)
{
    uint32_t length = 0;

    tlvBuffer[length++] = paramId;
    if (paramValueLen > MAX_UCI_PACKET_SIZE) {
        LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
        length = 0;
        return (uint8_t)length;
    }
    switch (paramId) {
        /* Length 1 Byte */
    case UCI_EXT_PARAM_ID_TEST_CONTENTION_RANGING_FEATURE:
    case UCI_EXT_PARAM_ID_RANGING_TIMESTAMP_NTF: {
        tlvBuffer[length++] = sizeof(uint8_t); // Param len
        uint8_t value       = *((uint8_t *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    /* Length 4 Byte */
    case UCI_EXT_PARAM_ID_CIR_WINDOW: {
        tlvBuffer[length++] = sizeof(uint32_t); // Param len 4
        uint32_t value      = *((uint32_t *)paramValue);
        tlvBuffer[length++] = (uint8_t)(value);
        tlvBuffer[length++] = (uint8_t)(value >> 8);
        tlvBuffer[length++] = (uint8_t)(value >> 16);
        tlvBuffer[length++] = (uint8_t)(value >> 24);
    } break;
#if UWBIOT_UWBD_SR2XXT
    case UCI_EXT_PARAM_ID_DATA_LOGGER_NTF: {
        if ((paramValueLen + length) <= MAX_UCI_PACKET_SIZE) {
            tlvBuffer[length++] = (uint8_t)paramValueLen; // Param len
            phOsalUwb_MemCopy(&tlvBuffer[length], (uint8_t *)paramValue, paramValueLen);
            length += paramValueLen;
        }
        else {
            LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
            length = 0;
        }

    } break;
#endif // UWBIOT_UWBD_SR2XXT
    default:
        LOG_E("UCI Extended parameter '0x%04X' not supported", paramId);
        length = 0;
        break;
    }
    if (length > MAX_UCI_PACKET_SIZE) {
        LOG_E("Max UCI packet size is %d bytes", MAX_UCI_PACKET_SIZE);
        length = 0;
    }
    return (uint8_t)length;
}

#if UWBFTR_CCC

BOOLEAN parseCapabilityCCCParams(
    phUwbCapInfo_t *pDevCap, uint8_t paramId, uint16_t *index, uint8_t length, uint8_t *capsInfoData)
{
    NXPLOG_UWBAPI_D("Param Id = %X\n", paramId);
    switch (paramId) {
    case SLOT_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        pDevCap->slotBitmask = capsInfoData[(*index)];
        *index               = (uint8_t)(*index + length);
    } break;
    case SYNC_CODE_INDEX_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_4) {
            return FALSE;
        }
        phOsalUwb_MemCopy(&pDevCap->syncCodeIndexBitmask, &capsInfoData[*index], length);
        *index = (uint8_t)(*index + length);
    } break;
    case HOPPING_CONFIG_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        pDevCap->hoppingConfigBitmask = capsInfoData[(*index)];
        *index                        = (uint8_t)(*index + length);
    } break;
    case CHANNEL_BITMASK: {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        pDevCap->channelBitmask = capsInfoData[(*index)];
        *index                  = (uint8_t)(*index + length);
    } break;
    case SUPPORTED_PROTOCOL_VERSION: {
        pDevCap->numSupportedProtocolVersions = length / DEVICE_CAPABILITY_LEN_2;
        if ((length == 0) || (pDevCap->numSupportedProtocolVersions > NUM_SUPPORTED_PROTOCOL_VERSIONS) || ((length % DEVICE_CAPABILITY_LEN_2) != 0)) {
            LOG_E("Invalid length or exceeding NUM_SUPPORTED_PROTOCOL_VERSIONS");
            return FALSE;
        }
        for (uint8_t iter = 0; iter < pDevCap->numSupportedProtocolVersions; iter++) {
            UWB_STREAM_TO_UINT16(pDevCap->supportedProtocolVersions[iter], capsInfoData, *index);
        }
    } break;
    case SUPPORTED_UWB_CONFIG_ID: {
        pDevCap->numSupportedUWBConfigIDs = length / DEVICE_CAPABILITY_LEN_2;
        if ((length == 0) || (pDevCap->numSupportedUWBConfigIDs > NUM_SUPPORTED_UWB_CONFIG_ID) || ((length % DEVICE_CAPABILITY_LEN_2) != 0)) {
            LOG_E("Invalid length or exceeding NUM_SUPPORTED_UWB_CONFIG_ID");
            return FALSE;
        }
        for (uint8_t iter = 0; iter < pDevCap->numSupportedUWBConfigIDs; iter++) {
            UWB_STREAM_TO_UINT16(pDevCap->supportedUWBConfigIDs[iter], capsInfoData, *index);
        }
    } break;
    case SUPPORTED_PULSESHAPE_COMBO: {
        if (length == 0 || length > DEVICE_CAPABILITY_LEN_9) {
            return FALSE;
        }
        phOsalUwb_MemCopy(&pDevCap->supportedPulseShapeCombo, &capsInfoData[*index], length);
        *index = (uint8_t)(*index + length);
    } break;
    case CCC_MINIMUM_RAN_MULTIPLIER : {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        pDevCap->minRanMultiplier = capsInfoData[(*index)];
        *index                    = (uint8_t)(*index + length);
    } break;
#if UWBFTR_CSA
    case ALIRO_SUPPORTED_MAC_MODES: {
        if (length != DEVICE_CAPABILITY_LEN_1) {
            return FALSE;
        }
        UWB_STREAM_TO_UINT8(pDevCap->aliroSupportedMacMode, capsInfoData, *index);
    } break;
    case ALIRO_SUPPORTED_PROTOCOL_VERSION: {
        pDevCap->numAliroSupportedProtocolVersions = length / DEVICE_CAPABILITY_LEN_2;
        if ((length == 0) || (pDevCap->numAliroSupportedProtocolVersions > NUM_SUPPORTED_ALIRO_PROTOCOL_VERSIONS) || ((length % DEVICE_CAPABILITY_LEN_2) != 0)) {
            LOG_E("Invalid length or exceeding NUM_SUPPORTED_ALIRO_PROTOCOL_VERSIONS");
            return FALSE;
        }
        for (uint8_t iter = 0; iter < pDevCap->numAliroSupportedProtocolVersions; iter++) {
            UWB_STREAM_TO_UINT16(pDevCap->aliroSupportedProtocolVersion[iter], capsInfoData, *index);
        }
    } break;
#endif /* UWBFTR_CSA */
    default:
        NXPLOG_UWBAPI_W("%s: unknown ccc param Id : 0x%X", __FUNCTION__, paramId);
        *index = (uint8_t)(*index + length);
    }
    return TRUE;
}
#endif // UWBFTR_CCC

BOOLEAN parseDeviceInfo(phUwbDevInfo_t *pdevInfo, uint8_t *deviceInfoData, uint16_t deviceInfoLength)
{
    uint16_t index = 0;
    uint8_t paramId;
    uint8_t length;
    uint16_t manufacturerLength = deviceInfoLength;
    uint8_t *manufacturerData   = deviceInfoData;

    if (manufacturerLength == 0) {
        NXPLOG_UWBAPI_E("%s: manufacturerLength is zero", __FUNCTION__);
        return FALSE;
    }

    if (manufacturerData == NULL) {
        NXPLOG_UWBAPI_E("%s: manufacturerData is NULL", __FUNCTION__);
        return FALSE;
    }

    if (pdevInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pdevInfo is NULL", __FUNCTION__);
        return FALSE;
    }
    UWB_STREAM_TO_UINT8(pdevInfo->uciGenericMajor, manufacturerData, index);
    UWB_STREAM_TO_UINT8(pdevInfo->uciGenericMinorMaintenanceVersion, manufacturerData, index);
    UWB_STREAM_TO_UINT8(pdevInfo->macMajorVersion, manufacturerData, index);
    UWB_STREAM_TO_UINT8(pdevInfo->macMinorMaintenanceVersion, manufacturerData, index);
    UWB_STREAM_TO_UINT8(pdevInfo->phyMajorVersion, manufacturerData, index);
    UWB_STREAM_TO_UINT8(pdevInfo->phyMinorMaintenanceVersion, manufacturerData, index);
    pdevInfo->mwMajor                           = UWBIOTVER_STR_VER_MAJOR;
    pdevInfo->mwMinor                           = UWBIOTVER_STR_VER_MINOR;
    pdevInfo->mwRc                              = UWBIOTVER_STR_VER_DEV;

    index = index + 2; // skip test version
    index++;           // skip extended params length

    while (index < manufacturerLength) {
        UWB_STREAM_TO_UINT8(paramId, manufacturerData, index);
        UWB_STREAM_TO_UINT8(length, manufacturerData, index);
        NXPLOG_UWBAPI_D("Extended Device Param Id = %d", paramId);
        switch (paramId) {
        case UCI_EXT_PARAM_ID_DEVICE_NAME:
            if (length > sizeof(pdevInfo->devName)) {
                NXPLOG_UWBAPI_E("%s : device name data size is more than response buffer", __FUNCTION__);
                return FALSE;
            }
            pdevInfo->devNameLen = length;
            if (length != 0) {
                UWB_STREAM_TO_ARRAY(&pdevInfo->devName[0], manufacturerData, length, index)
                if (length < sizeof(pdevInfo->devName)) { /*check is added to avoid the coverity warning*/
                    pdevInfo->devName[length] = '\0';
                }
            }
            break;
        case UCI_EXT_PARAM_ID_FW_VERSION:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            UWB_STREAM_TO_UINT8(pdevInfo->fwMajor, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->fwMinor, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->fwRc, manufacturerData, index);
            break;
        case UCI_EXT_PARAM_ID_VENDOR_UCI_VER:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            UWB_STREAM_TO_UINT8(pdevInfo->vendorUciMajor, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->vendorUciMinor, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->vendorUciPatch, manufacturerData, index);
            break;
        case UCI_EXT_PARAM_ID_UWB_CHIP_ID:
            if (length != MAX_UWB_CHIP_ID_LEN) {
                NXPLOG_UWBAPI_E("%s: UWB chip id Length %d is not equal to expected %d bytes",
                    __FUNCTION__,
                    length,
                    MAX_UWB_CHIP_ID_LEN);
                return FALSE;
            }
            UWB_STREAM_TO_ARRAY(&pdevInfo->uwbChipId[0], manufacturerData, length, index)
            break;
        case UCI_EXT_PARAM_ID_UWBS_MAX_PPM_VALUE:
            if (length != MAX_PPM_VALUE_LEN) {
                NXPLOG_UWBAPI_E("%s: PPM Value Length %d is not equal to expected %d bytes",
                    __FUNCTION__,
                    length,
                    MAX_PPM_VALUE_LEN);
                return FALSE;
            }
            UWB_STREAM_TO_UINT8(pdevInfo->maxPpmValue, manufacturerData, index);
            break;
        case UCI_EXT_PARAM_ID_TX_POWER:
            if (length != MAX_TX_POWER_LEN) {
                NXPLOG_UWBAPI_E("%s: TX Power Length %d is not equal to expected %d bytes ",
                    __FUNCTION__,
                    length,
                    MAX_TX_POWER_LEN);
                return FALSE;
            }
            UWB_STREAM_TO_UINT16(pdevInfo->txPowerValue, manufacturerData, index);
            break;
        case UCI_EXT_PARAM_UWBS_CAL_MODE: {
            if (length != MAX_CAL_MODE_LEN) {
                return FALSE;
            }
            UWB_STREAM_TO_UINT32(pdevInfo->lifecycle, manufacturerData, index);
        } break;
        case UCI_EXT_PARAM_ID_FIRA_EXT_UCI_GENERIC_VER:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            UWB_STREAM_TO_UINT8(pdevInfo->uciGenericMajor, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->uciGenericMinorMaintenanceVersion, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->uciGenericPatch, manufacturerData, index);
            break;
        case UCI_EXT_PARAM_ID_FIRA_EXT_TEST_VER:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return FALSE;
            }
            UWB_STREAM_TO_UINT8(pdevInfo->uciTestMajor, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->uciTestMinor, manufacturerData, index);
            UWB_STREAM_TO_UINT8(pdevInfo->uciTestPatch, manufacturerData, index);
            break;
        case UCI_EXT_PARAM_ID_UWB_FW_GIT_HASH: {
            if (length != FW_GIT_HASH_LEN) {
                NXPLOG_UWBAPI_E("%s: fwgitHash size %d bytes, is not equal to expected %d bytes",
                    __FUNCTION__,
                    length,
                    FW_GIT_HASH_LEN);
                return FALSE;
            }
            if (length != 0) {
                UWB_STREAM_TO_ARRAY(&pdevInfo->fwGitHash[0], manufacturerData, length, index);
            }
            break;
        }
        case UCI_EXT_PARAM_ID_FW_BOOT_MODE:
            if (length != FW_BOOT_MODE_LEN) {
                NXPLOG_UWBAPI_E("%s: fwBootMode id size %d is not equal to expected %d bytes ",
                    __FUNCTION__,
                    length,
                    FW_BOOT_MODE_LEN);
                return FALSE;
            }
            UWB_STREAM_TO_UINT8(pdevInfo->fwBootMode, manufacturerData, index);
            break;
#if UWBFTR_CCC
        case UCI_EXT_PARAM_ID_UCI_CCC_VERSION: {
            if (length > sizeof(pdevInfo->uciCccVersion)) {
                NXPLOG_UWBAPI_E("%s: UCI CCC version size %d is more than response buffer", __FUNCTION__, length);
                return FALSE;
            }
            if (length != 0) {
                UWB_STREAM_TO_ARRAY(&pdevInfo->uciCccVersion[0], manufacturerData, length, index);
            }
        } break;

        case UCI_EXT_PARAM_ID_CCC_VERSION: {
            if (length > sizeof(pdevInfo->cccVersion)) {
                NXPLOG_UWBAPI_E("%s: CCC version size %d is more than response buffer", __FUNCTION__, length);
                return FALSE;
            }
            if (length != 0) {
                UWB_STREAM_TO_ARRAY(&pdevInfo->cccVersion[0], manufacturerData, length, index);
            }
        } break;
#endif // UWBFTR_CCC
#if UWBFTR_CSA
        case UCI_EXT_PARAM_ID_ALIRO_SPEC_VERSION: {
            if (length > sizeof(pdevInfo->aliroSpecVersion)) {
                NXPLOG_UWBAPI_E("%s: Aliro Spec Version size %d is more than response buffer", __FUNCTION__, length);
                return FALSE;
            }
            if (length != 0) {
                UWB_STREAM_TO_ARRAY(&pdevInfo->aliroSpecVersion[0], manufacturerData, length, index);
            }
        } break;
#endif // UWBFTR_CSA
        default:
            NXPLOG_UWBAPI_W("%s: unknown param Id 0x%X", __FUNCTION__, paramId);
            // Ignore the Unknown Param.
            index            = index + length;
        }
    }
    return TRUE;
}

void parseDebugParams(uint8_t *rspPtr, uint8_t noOfParams, UWB_DebugParams_List_t *DebugParams_List)
{
    uint8_t paramId;
    uint32_t paramOffset = 0;
#if !(UWBIOT_UWBD_SR1XXT)
    uint8_t length;
#endif
    for (int i = 0; i < noOfParams; i++) {
        UWB_STREAM_TO_UINT8(paramId, rspPtr, paramOffset);
#if !(UWBIOT_UWBD_SR1XXT)
        UWB_STREAM_TO_UINT8(length, rspPtr, paramOffset);
#else
        ++paramOffset; // Skip the length field
#endif
        switch (paramId) {
#if UWBIOT_UWBD_SR2XXT
        case UCI_EXT_PARAM_ID_DATA_LOGGER_NTF:
            UWB_STREAM_TO_ARRAY(DebugParams_List[i].param_value.param.param_value, rspPtr, length, paramOffset);
            break;
#endif // UWBIOT_UWBD_SR2XXT
        case UCI_EXT_PARAM_ID_TEST_CONTENTION_RANGING_FEATURE:
            UWB_STREAM_TO_UINT8(DebugParams_List[i].param_value.vu8, rspPtr, paramOffset);
            break;
        case UCI_EXT_PARAM_ID_CIR_WINDOW:
            UWB_STREAM_TO_UINT32(DebugParams_List[i].param_value.vu32, rspPtr, paramOffset);
            break;
        case UCI_EXT_PARAM_ID_RANGING_TIMESTAMP_NTF:
            UWB_STREAM_TO_UINT8(DebugParams_List[i].param_value.vu8, rspPtr, paramOffset);
            break;
        default:
            LOG_E("UCI Extended parameter '0x%04X' not supported", paramId);
            break;
        }
    }
}

uint8_t getExtCoreDeviceConfigTLVBuffer(uint16_t paramId, uint8_t paramLen, void *paramValue, uint8_t *tlvBuffer)
{
    uint8_t length = (uint8_t)(paramLen + UWBD_EXT_CONFIG_HEADER_LEN);
    phDdfsToneConfig_t *ddfsToneConfig;
    uint8_t uci_param_id = 0;
    uint32_t bufferOffset = 0;

    if (paramValue == NULL || tlvBuffer == NULL) {
        NXPLOG_UWBAPI_E("%s: Buffer is NULL", __FUNCTION__);
        return 0;
    }
    uci_param_id = (uint8_t)(paramId & 0x00FF);
    UWB_UINT8_TO_STREAM(tlvBuffer, EXTENDED_DEVICE_CONFIG_ID, bufferOffset);
    UWB_UINT8_TO_STREAM(tlvBuffer, uci_param_id, bufferOffset);

    switch (uci_param_id) {
    /* 1 byte len */
    case UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value != DPD_WAKEUP_SRC_GPIO_1 && value != DPD_WAKEUP_SRC_GPIO_3) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;
    #if !(UWBIOT_UWBD_SR2XXT)
    case UCI_EXT_PARAM_ID_WTX_COUNT: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value < WTX_COUNT_MIN || value > WTX_COUNT_MAX) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;
    #endif //!(UWBIOT_UWBD_SR2XXT)
    case UCI_EXT_PARAM_ID_TX_BASE_BAND_CONFIG: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value != ENABLED && value != DISABLED) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;
#if UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_GPIO_SELECTION_FOR_DUAL_AOA: {
        uint8_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;
#endif // UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG: {
        uint8_t value = *((uint8_t *)paramValue);
        if (value != ENABLED && value != DISABLED) {
            return 0;
        }
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;

    /* Length 2 byte */
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT: {
        uint16_t value = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t), bufferOffset);
        UWB_UINT16_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;

    case UCI_EXT_PARAM_ID_CLOCK_PRESENT_WAITING_TIME: {
        uint16_t clockPresentWaitingTime = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(clockPresentWaitingTime), bufferOffset);
        UWB_UINT16_TO_STREAM(tlvBuffer, clockPresentWaitingTime, bufferOffset);
    } break;

    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_ABS: {
        uint16_t value = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t), bufferOffset);
        UWB_UINT16_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;

    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_REL: {
        uint16_t value = *((uint16_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t), bufferOffset);
        UWB_UINT16_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;

    /* Length 1 byte  or 2 byte*/
    case UCI_EXT_PARAM_ID_CLK_CONFIG_CTRL: {
        phClkConfigSrc_t *value = ((phClkConfigSrc_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(phClkConfigSrc_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value->clk_src_opt, bufferOffset);
#if UWBIOT_UWBD_SR1XXT
        /**
         * This UWBD macro is required
         */
        UWB_UINT8_TO_STREAM(tlvBuffer, value->xtal_opt, bufferOffset);
#elif UWBIOT_UWBD_SR2XXT
        UWB_UINT16_TO_STREAM(tlvBuffer, value->slow_clk_wait, bufferOffset);
        UWB_UINT16_TO_STREAM(tlvBuffer, value->rf_clk_wait, bufferOffset);
#endif // UWBIOT_UWBD_SR1XXT
    } break;

    /* Length 4 byte */
    case UCI_EXT_PARAM_ID_TX_PULSE_SHAPE_CONFIG: {
        uint32_t value = *((uint32_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint32_t), bufferOffset);
        UWB_UINT32_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;

    /* Length depending upon Structure */
    case UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG: {
        ddfsToneConfig = (phDdfsToneConfig_t *)paramValue;
        UWB_UINT8_TO_STREAM(tlvBuffer, paramLen, bufferOffset);
        for (int i = 0; i < NO_OF_BLOCKS; i++) {
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].channel_no, bufferOffset);
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].tx_antenna_selection, bufferOffset);
            UWB_UINT32_TO_STREAM(tlvBuffer, ddfsToneConfig[i].tx_ddfs_tone_0, bufferOffset);
            UWB_UINT32_TO_STREAM(tlvBuffer, ddfsToneConfig[i].tx_ddfs_tone_1, bufferOffset);
            UWB_UINT32_TO_STREAM(tlvBuffer, ddfsToneConfig[i].spur_duration, bufferOffset);
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].gainval_set, bufferOffset);
            UWB_UINT8_TO_STREAM(tlvBuffer, ddfsToneConfig[i].ddfsgainbypass_enbl, bufferOffset);
            UWB_UINT16_TO_STREAM(tlvBuffer, ddfsToneConfig[i].periodicity, bufferOffset);
        }
    } break;
#if UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_PDOA_CALIB_TABLE_DEFINE: {
        phPdoaTableDef_t *value = ((phPdoaTableDef_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint16_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value->calibStepSize, bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value->noSteps, bufferOffset);
    } break;
#endif // UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_ANTENNA_RX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNA_TX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNAE_RX_PAIR_DEFINE: {
        phAntennaDefines_t *antennaDefines = (phAntennaDefines_t *)paramValue;
        if (antennaDefines->antennaDefsLen <= MAX_UCI_PACKET_SIZE - MAX_UCI_HEADER_SIZE) {
            UWB_UINT8_TO_STREAM(tlvBuffer, antennaDefines->antennaDefsLen, bufferOffset);
            UWB_ARRAY_TO_STREAM(tlvBuffer, antennaDefines->antennaDefs, antennaDefines->antennaDefsLen, bufferOffset);
        }
        else {
            LOG_E("%s : invalid antenna defines ", __FUNCTION__);
            return 0;
        }
    } break;

#if UWBFTR_AoA_FoV
    /* Length 1 byte */
    case UCI_EXT_PARAM_ID_AOA_MODE: {
        uint8_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;
#endif // UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_FREQ_OF_UWB_REQ_WLAN_CHANNEL_INFO: {
        uint8_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint8_t), bufferOffset);
        UWB_UINT8_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;

    /* Length 4 byte */
    case UCI_EXT_PARAM_ID_DDFS_CONFIG_PER_PULSE_SHAPE: {
        uint32_t value = *((uint8_t *)paramValue);
        UWB_UINT8_TO_STREAM(tlvBuffer, sizeof(uint32_t), bufferOffset);
        UWB_UINT32_TO_STREAM(tlvBuffer, value, bufferOffset);
    } break;

    default:
        length = 0;
        break;
    }
    return length;
}

void parseExtGetDeviceConfigResponse(uint8_t *tlvBuffer, phDeviceConfigData_t *devConfig)
{
    uint8_t paramId;
    uint32_t bufferOffset = 0;
    uint8_t totalLen;
    bufferOffset++; // skipping the extended device config ID
    UWB_STREAM_TO_UINT8(paramId, tlvBuffer, bufferOffset);
    UWB_STREAM_TO_UINT8(totalLen, tlvBuffer, bufferOffset);

    switch (paramId) {
        /* 1 byte len */
    case UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC: {
        UWB_STREAM_TO_UINT8(devConfig->dpdWakeupSrc, tlvBuffer, bufferOffset);
    } break;

#if !(UWBIOT_UWBD_SR2XXT)
    case UCI_EXT_PARAM_ID_WTX_COUNT: {
        UWB_STREAM_TO_UINT8(devConfig->wtxCountConfig, tlvBuffer, bufferOffset);
    } break;
#endif //!(UWBIOT_UWBD_SR2XXT)

    case UCI_EXT_PARAM_ID_TX_BASE_BAND_CONFIG: {
        UWB_STREAM_TO_UINT8(devConfig->txBaseBandConfig, tlvBuffer, bufferOffset);
    } break;

#if UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_GPIO_SELECTION_FOR_DUAL_AOA: {
        UWB_STREAM_TO_UINT8(devConfig->rxAntennaSelectionConfig, tlvBuffer, bufferOffset);
    } break;
#endif // UWBFTR_AoA_FoV

    case UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG: {
        UWB_STREAM_TO_UINT8(devConfig->nxpExtendedNtfConfig, tlvBuffer, bufferOffset);
    } break;

    /* Length 2 byte */
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT: {
        UWB_STREAM_TO_UINT16(devConfig->dpdEntryTimeout, tlvBuffer, bufferOffset);
    } break;

    case UCI_EXT_PARAM_ID_TX_PULSE_SHAPE_CONFIG: {
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.shape_id, tlvBuffer, bufferOffset);
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.payload_tx_shape_id, tlvBuffer, bufferOffset);
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.sts_shape_id, tlvBuffer, bufferOffset);
        UWB_STREAM_TO_UINT8(devConfig->txPulseShapeConfig.dac_stage_cofig, tlvBuffer, bufferOffset);
    } break;

    case UCI_EXT_PARAM_ID_CLK_CONFIG_CTRL: {
        UWB_STREAM_TO_UINT8(devConfig->clockConfigCtrl.clk_src_opt, tlvBuffer, bufferOffset);
#if UWBIOT_UWBD_SR1XXT
        UWB_STREAM_TO_UINT8(devConfig->clockConfigCtrl.xtal_opt, tlvBuffer, bufferOffset);
#elif UWBIOT_UWBD_SR2XXT
        UWB_STREAM_TO_UINT16(devConfig->clockConfigCtrl.slow_clk_wait, tlvBuffer, bufferOffset);
        UWB_STREAM_TO_UINT16(devConfig->clockConfigCtrl.rf_clk_wait, tlvBuffer, bufferOffset);
#endif // UWBIOT_UWBD_SR1XXT
    } break;

    case UCI_EXT_PARAM_ID_CLOCK_PRESENT_WAITING_TIME: {
        UWB_STREAM_TO_UINT16(devConfig->clockPresentWaitingTime, tlvBuffer, bufferOffset);
    } break;

    case UCI_EXT_PARAM_ID_HOST_MAX_UCI_PAYLOAD_LENGTH: {
        UWB_STREAM_TO_UINT16(devConfig->hostMaxUCIPayloadLen, tlvBuffer, bufferOffset);
    } break;

    case UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG: {
        for (int i = 0; i < NO_OF_BLOCKS; i++) {
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].channel_no, tlvBuffer, bufferOffset);
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].tx_antenna_selection, tlvBuffer, bufferOffset);
            UWB_STREAM_TO_UINT32(devConfig->ddfsToneConfig[i].tx_ddfs_tone_0, tlvBuffer, bufferOffset);
            UWB_STREAM_TO_UINT32(devConfig->ddfsToneConfig[i].tx_ddfs_tone_1, tlvBuffer, bufferOffset);
            UWB_STREAM_TO_UINT32(devConfig->ddfsToneConfig[i].spur_duration, tlvBuffer, bufferOffset);
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].gainval_set, tlvBuffer, bufferOffset);
            UWB_STREAM_TO_UINT8(devConfig->ddfsToneConfig[i].ddfsgainbypass_enbl, tlvBuffer, bufferOffset);
            UWB_STREAM_TO_UINT16(devConfig->ddfsToneConfig[i].periodicity, tlvBuffer, bufferOffset);
        }
    } break;
#if UWBIOT_UWBD_SR2XXT
    case UCI_EXT_PARAM_ID_DDFS_CONFIG_PER_PULSE_SHAPE: {
        UWB_STREAM_TO_UINT32(devConfig->ddfsCfgPerPulseShape, tlvBuffer, bufferOffset);
    } break;
#endif // UWBIOT_UWBD_SR2XXT
#if UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_AOA_MODE: {
        UWB_STREAM_TO_UINT8(devConfig->aoaMode, tlvBuffer, bufferOffset);
    } break;
#endif // UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_FREQ_OF_UWB_REQ_WLAN_CHANNEL_INFO: {
        UWB_STREAM_TO_UINT8(devConfig->wlanChannelINfo, tlvBuffer, bufferOffset);
    } break;
    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_ABS: {
        UWB_STREAM_TO_UINT16(devConfig->initialRxOnOffsetAbs, tlvBuffer, bufferOffset);
    } break;
    case UCI_EXT_PARAM_ID_INITIAL_RX_ON_OFFSET_REL: {
        UWB_STREAM_TO_UINT16(devConfig->initialRxOnOffsetRel, tlvBuffer, bufferOffset);
    } break;
#if UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_PDOA_CALIB_TABLE_DEFINE: {
        UWB_STREAM_TO_UINT8(devConfig->pdoaCalibTableDef.calibStepSize, tlvBuffer, bufferOffset);
        UWB_STREAM_TO_UINT8(devConfig->pdoaCalibTableDef.noSteps, tlvBuffer, bufferOffset);
    } break;
#endif // UWBFTR_AoA_FoV
    case UCI_EXT_PARAM_ID_ANTENNA_RX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNA_TX_IDX_DEFINE:
    case UCI_EXT_PARAM_ID_ANTENNAE_RX_PAIR_DEFINE: {
        UWB_STREAM_TO_ARRAY(devConfig->antennaDefines.antennaDefs, tlvBuffer, totalLen, bufferOffset);
        devConfig->antennaDefines.antennaDefsLen = totalLen;
    } break;
    default:
        break;
    }
}

/*******************************************************************************
 **
 ** Function         handle_schedstatus_ntf
 **
 ** Description      This function is called to process Scheduler Status
 **                  notification
 **
 ** Returns          void
 **
 */
static void handle_schedstatus_ntf(uint8_t *p, uint16_t len)
{
    if (len > MAX_UCI_PACKET_SIZE) {
        LOG_E("%s Not enough buffer to store %d bytes", __FUNCTION__, len);
        return;
    }
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_SCHEDULER_STATUS_NTF, (void *)p, len);
    }
}
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)
/**
 **
 ** Function         handle_se_com_err_ntf
 **
 ** Description      This function is called to notify se comm err
 **
 ** Returns          void
 **
 */
static void handle_se_com_err_ntf(uint8_t *p, uint16_t len)
{
    if (len != 0) {
        NXPLOG_UWBAPI_W("%s: SE_COMM_ERR, status %d", __FUNCTION__, (uint8_t)(*(p)));
    }
    else {
        NXPLOG_UWBAPI_E("%s Invalid length %d", __FUNCTION__, len);
    }
}
/**
 **
 ** Function         handle_generate_tag_ntf
 **
 ** Description      This function is called to process CMAC Tag Generation notifications
 **
 ** Returns          void
 **
 */
static void handle_generate_tag_ntf(uint8_t *p, uint16_t len)
{
    uint32_t index = 0;
    phOsalUwb_SetMemory(&uwbContext.rsp_data, 0x00, MAX_UCI_PACKET_SIZE);
    if ((len > 0) && (len <= sizeof(uwbContext.rsp_data))) {
        uwbContext.rsp_len = len;
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, len, index);
    }
    else {
        LOG_E("%s Invalid length %d", __FUNCTION__, len);
    }
}
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)

#if UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250
/*******************************************************************************
 **
 ** Function         handle_bindingStatus_ntf
 **
 ** Description      This function is called to notify Binding Status.
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_bindingStatus_ntf(uint8_t *p, uint16_t len)
{
    if (len != 0) {
        NXPLOG_UWBAPI_W("%s: Binding Status : %d", __FUNCTION__, (uint8_t)(*(p)));
    }
    else {
        NXPLOG_UWBAPI_E("%s Invalid length %d", __FUNCTION__, len);
    }
    /** If the Binding Status Notification is received, Produce the semaphore */
    (void)phOsalUwb_ProduceSemaphore(uwbContext.uwb_binding_status_ntf_wait);
}
#endif /** UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250 */

#if UWBIOT_SESN_SNXXX
static void handle_eseGetSessionIdList(uint8_t *p, uint16_t len)
{
    /* Clear the rsp_data */
    phOsalUwb_SetMemory(&uwbContext.rsp_data[0], 0x00, sizeof(uwbContext.rsp_data));
    if ((len > 0) && (len <= sizeof(uwbContext.rsp_data))) {
        /* Copy the GET_ESE_SESSION_ID_LIST_NTF fields */
        uwbContext.rsp_len = len;
        phOsalUwb_MemCopy(uwbContext.rsp_data, p, uwbContext.rsp_len);
    }
    else {
        LOG_E("%s Invalid length %d", __FUNCTION__, len);
        uwbContext.rsp_len = 0;
    }
}

#endif // UWBIOT_SESN_SNXXX
/**
 **
 ** Function         handle_do_chip_calibration_ntf
 **
 ** Description      This function is called to process do calibration
 *notification
 **
 ** Returns          void
 **
 */
static void handle_do_chip_calibration_ntf(uint8_t *p, uint16_t len)
{
    uint32_t index = 0;
    phOsalUwb_SetMemory(&uwbContext.rsp_data[0], 0x00, sizeof(uwbContext.rsp_data));
    if ((len > 0) && (len <= sizeof(uwbContext.rsp_data))) {
        uwbContext.rsp_len = len;
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, len, index);
    }
    else {
        LOG_E("%s Invalid length %d", __FUNCTION__, len);
        uwbContext.rsp_len = 0;
    }
}

#if UWBIOT_UWBD_SR1XXT
/**
 **
 ** Function         handle_read_calibration_data_ntf
 **
 ** Description      This function is called to process read calibration data
 **                  notification
 **
 ** Returns          void
 **
 */
static void handle_read_calibration_data_ntf(uint8_t *p, uint16_t len)
{
    uint32_t index = 0;
    phOsalUwb_SetMemory(&uwbContext.rsp_data, 0x00, MAX_UCI_PACKET_SIZE);
    if (len > 0) {
        UWB_STREAM_TO_UINT8(uwbContext.wstatus, p, index);
        UWB_STREAM_TO_UINT8(uwbContext.rsp_len, p, index);
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, uwbContext.rsp_len, index);
    }
}
#endif // UWBIOT_UWBD_SR1XXT

#if UWBFTR_UWBS_DEBUG_Dump
/**
 **
 ** Function         handle_debug_ntf
 **
 ** Description      This function is called to process debug notification.
 **
 ** Returns          void
 **
 */
static void handle_debug_ntf(eResponse_Ext_Ntf_Event event, uint8_t *p, uint16_t len)
{
    eNotificationType ntfType = UWBD_INVALID_NTF_EVT;
    if (len > MAX_DEBUG_NTF_SIZE) {
        LOG_E("%s Not enough buffer to store %d bytes", __FUNCTION__, len);
        return;
    }
    if (uwbContext.pAppCallback) {
        if (event == UWA_DM_VENDOR_CIR_LOG_NTF_EVT) {
            ntfType = UWBD_CIR_DATA_NTF;
        }
#if  UWBIOT_UWBD_SR2XXT
        else if (event == UWA_DM_PROP_DBG_DATA_LOGGER_NTF_EVT) {
            ntfType = UWBD_DATA_LOGGER_NTF;
        }
#endif // UWBIOT_UWBD_SR2XXT
        else if (event == UWA_DM_VENDOR_PSDU_LOG_NTF_EVT) {
            ntfType = UWBD_PSDU_DATA_NTF;
        }
        else if (event == UWA_DM_INTERNAL_RANGING_TIMESTAMP_NTF_EVT) {
            ntfType = UWBD_RANGING_TIMESTAMP_NTF;
        }

        if (ntfType != UWBD_INVALID_NTF_EVT) {
            uwbContext.pAppCallback(ntfType, (void *)p, len);
        }
    }
}
#endif // UWBFTR_UWBS_DEBUG_Dump

/**
 **
 ** Function         handle_UwbWlanInd_ntf
 **
 ** Description      This function is called to process WiFi CoEx Ind
 **                  notification
 **
 ** Returns          void
 **
 */
static void handle_UwbWlanInd_ntf(uint8_t *responsePayloadPtr)
{
    UWB_Wlan_IndNtf_t wifiWlanIndNtf;
    uint32_t index = 0;

    UWB_STREAM_TO_UINT8(wifiWlanIndNtf.UWB_Wlan_IndNtf_status, responsePayloadPtr, index);
    UWB_STREAM_TO_UINT32(wifiWlanIndNtf.UWB_Wlan_IndNtf_time_index, responsePayloadPtr, index);
    UWB_STREAM_TO_UINT32(wifiWlanIndNtf.UWB_Wlan_IndNtf_sessionHandle, responsePayloadPtr, index);

    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_WIFI_WLAN_IND_NTF, &wifiWlanIndNtf, sizeof(wifiWlanIndNtf));
    }
}

/**
 **
 ** Function         handle_WiFiCoEx_ActGrantDurationNtf
 **
 ** Description      This function is called to process WiFi CoEx
 **                  Max Active Grant Duration Status notification
 **
 ** Returns          void
 **
 */
static void handle_WiFiCoEx_ActGrantDurationNtf(uint8_t *responsePayloadPtr)
{
    uint8_t actGrantDuration = 0x00;
    uint32_t index = 0;
    UWB_STREAM_TO_UINT8(actGrantDuration, responsePayloadPtr, index);
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWB_WLAN_COEX_MAX_GRANT_DURATION_EXCEEDED_WRN_NTF, &actGrantDuration, sizeof(actGrantDuration));
    }
}

#if !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)
/**
 **
 ** Function         handle_WlanUwbInd_ntf
 **
 ** Description      This function is called to process WiFi CoEx
 **                  Ind Error Notification
 **
 ** Returns          void
 **
 */
static void handle_WlanUwbInd_ntf(uint8_t *responsePayloadPtr)
{
    Wlan_Uwb_IndNtf_t Wlan_Uwb_IndNtf = {0};
    uint32_t index                    = 0;
    UWB_STREAM_TO_UINT8(Wlan_Uwb_IndNtf.Wlan_Uwb_IndNtf_status, responsePayloadPtr, index);
    UWB_STREAM_TO_UINT32(Wlan_Uwb_IndNtf.Wlan_Uwb_IndNtf_time_index, responsePayloadPtr, index);
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(WLAN_UWB_IND_NTF, &Wlan_Uwb_IndNtf, sizeof(Wlan_Uwb_IndNtf));
    }
}
#endif // !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)

#if UWBIOT_UWBD_SR1XXT
#if (UWBFTR_AoA_FoV)
/**
 **
 ** Function         setDefaultAoaCalibration
 **
 ** Description      This function set the default AOA Calibration Parmeters
 **
 ** Returns          UWBAPI_STATUS_OK if successful, otherwise UWBAPI_STATUS_FAILED
 **
 */
static tUWBAPI_STATUS setDefaultAoaCalibration()
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_OK;
    void *p_cmd           = NULL;
    long cmd_len          = 0;
    uint8_t count         = 0;

    LOG_D("%s : Enter", __FUNCTION__);

#if UWBIOT_UWBD_SR150
    sAoACoreConfigs_t aoaConfigs[] = {
        {.key = UWB_AOA_CH5_CONFIG_BLOCK_COUNT, .pValue = aoa_ch5_config_block_names},
        {.key = UWB_AOA_CH9_CONFIG_BLOCK_COUNT, .pValue = aoa_ch9_config_block_names},
    };
#else
    sAoACoreConfigs_t aoaConfigs[] = {
        {.key = UWB_AOA_CONFIG_BLOCK_COUNT, .pValue = aoa_config_block_names},
    };
#endif // UWBIOT_UWBD_SR150

    for (uint8_t iter = 0; iter < GET_ARRAY_SIZE(aoaConfigs); iter++) {
        if (phNxpUciHal_GetNxpNumValue(aoaConfigs[iter].key, &count, sizeof(count))) {
            LOG_D("%s : Number of AOA calibration config count is %d", __FUNCTION__, count);
            for (int i = 0; i < count; i++) {
                if ((phNxpUciHal_GetNxpByteArrayValue(aoaConfigs[iter].pValue[i], &p_cmd, &cmd_len) == TRUE) &&
                    cmd_len > 0) {
                    status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
                    if (status != UWBAPI_STATUS_OK) {
                        LOG_E("%s : set aoa calibration for block %d failed", __FUNCTION__, count);
                        break;
                    }
                }
                else {
                    status = UWBAPI_STATUS_FAILED;
                    LOG_E("%s: calibration len for block %d is : %ld", __FUNCTION__, count, cmd_len);
                    break;
                }
            }
        }
    }
    LOG_D("%s: Exit ", __FUNCTION__);
    return status;
}
#endif // UWBFTR_AoA_FoV
#endif // UWBIOT_UWBD_SR1XXT

#if (UWBIOT_SESN_SNXXX)
/**
 **
 ** Function         handle_se_binding_ntf
 **
 ** Description      This function is called to process se binding realted notifications
 **
 ** Returns          void
 **
 */
static void handle_se_binding_ntf(uint8_t *p, uint16_t len)
{
    uint32_t index = 0;
    phOsalUwb_SetMemory(&uwbContext.rsp_data, 0x00, MAX_UCI_PACKET_SIZE);
    if ((len > 0) && (len <= sizeof(uwbContext.rsp_data))) {
        uwbContext.rsp_len = len;
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, len, index);
    }
    else {
        LOG_E("%s Invalid length %d", __FUNCTION__, len);
    }
}
#endif //(UWBIOT_SESN_SNXXX)

#if UWBIOT_SESN_SNXXX
/**
 **
 ** Function         handle_se_test_loop_ntf
 **
 ** Description      This function is called to process SE Test Loop notifications
 **
 ** Returns          void
 **
 */
static void handle_se_test_loop_ntf(uint8_t *p, uint16_t len)
{
    uint32_t index = 0;
    phOsalUwb_SetMemory(&uwbContext.rsp_data, 0x00, MAX_UCI_PACKET_SIZE);
    if ((len > 0) && (len <= sizeof(uwbContext.rsp_data))) {
        uwbContext.rsp_len = len;
        UWB_STREAM_TO_ARRAY(uwbContext.rsp_data, p, len, index);
    }
    else {
        LOG_E("%s Invalid length %d", __FUNCTION__, len);
    }
}
#endif // UWBIOT_SESN_SNXXX
eResponse_Ext_Ntf_Event processProprietaryNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;
    switch (oid) {
#if UWBIOT_SESN_SNXXX
    case EXT_UCI_MSG_SE_DO_TEST_LOOP: {
        dmExtEvent = UWA_DM_PROP_SE_TEST_LOOP_NTF_EVT;
        handle_se_test_loop_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif // UWBIOT_SESN_SNXXX
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S|| UWBIOT_UWBD_SR200S)
    case EXT_UCI_MSG_GENERATE_TAG: {
        dmExtEvent = UWA_DM_PROP_GENERATE_TAG_NTF_EVT;
        handle_generate_tag_ntf(responsePayloadPtr, responsePayloadLen);
    } break;

    case EXT_UCI_MSG_SE_COMM_ERROR_NTF: {
        dmExtEvent = UWA_DM_PROP_SE_COM_ERROR_NTF_EVT;
        handle_se_com_err_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S|| UWBIOT_UWBD_SR200S)
#if UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250
    case EXT_UCI_MSG_SE_GET_BINDING_STATUS: {
        dmExtEvent = UWA_DM_PROP_BINDING_STATUS_NTF_EVT;
        handle_bindingStatus_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif /** UWBIOT_SESN_SNXXX || UWBIOT_UWBD_SR250 */
#if UWBIOT_SESN_SNXXX
    case EXT_UCI_MSG_ESE_GET_SESSION_ID_LIST: {
        dmExtEvent = UWA_DM_PROP_GET_ESE_SESSION_ID_LIST_NTF_EVT;
        handle_eseGetSessionIdList(responsePayloadPtr, responsePayloadLen);
    } break;
#endif // UWBIOT_SESN_SNXXX
    case EXT_UCI_MSG_SCHEDULER_STATUS_NTF: {
        dmExtEvent = UWA_DM_PROP_SCHEDULER_STATUS_NTF_EVT;
        handle_schedstatus_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#if UWBFTR_UWBS_DEBUG_Dump && UWBIOT_UWBD_SR2XXT
    case EXT_UCI_MSG_DBG_DATA_LOGGER_NTF: {
        dmExtEvent = UWA_DM_PROP_DBG_DATA_LOGGER_NTF_EVT;
        handle_debug_ntf(dmExtEvent, responsePayloadPtr, responsePayloadLen);
    } break;
#endif // UWBFTR_UWBS_DEBUG_Dump && UWBIOT_UWBD_SR2XXT
    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        break;
    }

    return dmExtEvent;
}

eResponse_Ext_Ntf_Event processVendorNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;
    switch (oid) {
    case VENDOR_UCI_MSG_DO_CHIP_CALIBRATION: {
        dmExtEvent = UWA_DM_PROP_DO_CHIP_CALIBRATION_NTF_EVT;
        handle_do_chip_calibration_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#if UWBFTR_UWBS_DEBUG_Dump
    case VENDOR_UCI_MSG_CIR_LOG_NTF: {
        dmExtEvent = UWA_DM_VENDOR_CIR_LOG_NTF_EVT;
        handle_debug_ntf(dmExtEvent, responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_PSDU_LOG_NTF: {
        dmExtEvent = UWA_DM_VENDOR_PSDU_LOG_NTF_EVT;
        handle_debug_ntf(dmExtEvent, responsePayloadPtr, responsePayloadLen);
    } break;
#endif // UWBFTR_UWBS_DEBUG_Dump
#if (UWBIOT_SESN_SNXXX)
    case VENDOR_UCI_MSG_SE_DO_BIND: {
        dmExtEvent = UWA_DM_VENDOR_SE_DO_BIND_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_SE_DO_TEST_CONNECTIVITY: {
        dmExtEvent = UWA_DM_VENDOR_SE_DO_TEST_CONNECTIVITY_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_ESE_BINDING_CHECK_CMD: {
        dmExtEvent = UWA_DM_VENDOR_ESE_BINDING_CHECK_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    case VENDOR_UCI_MSG_URSK_DELETION_REQ: {
        dmExtEvent = UWA_DM_VENDOR_URSK_DELETION_REQ_NTF_EVT;
        handle_se_binding_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
#endif //(UWBIOT_SESN_SNXXX)

    case VENDOR_UCI_MSG_UWB_WLAN_IND_STATUS_NTF: {
        dmExtEvent = UWA_DM_PROP_UWB_WLAN_IND_NTF_EVT;
        handle_UwbWlanInd_ntf(responsePayloadPtr);
    } break;
#if !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)
    case VENDOR_UCI_MSG_WLAN_UWB_IND_STATUS_NTF: {
        dmExtEvent = UWA_DM_PROP_WLAN_UWB_IND_ERR_NTF_EVT;
        handle_WlanUwbInd_ntf(responsePayloadPtr);
    } break;
#endif // !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)
    case VENDOR_UCI_MSG_UWB_WLAN_COEX_MAX_GRANT_DURATION_EXCEEDED_WARN_NTF: {
        dmExtEvent = UWA_DM_PROP_UWB_WLAN_COEX_MAX_GRANT_DURATION_EXCEEDED_WARN_NTF;
        handle_WiFiCoEx_ActGrantDurationNtf(responsePayloadPtr);
    } break;
    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        break;
    }

    return dmExtEvent;
}

#if UWBIOT_UWBD_SR1XXT
eResponse_Ext_Ntf_Event processProprietarySeNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;
    switch (oid) {
    case EXT_UCI_MSG_WRITE_CALIB_DATA_CMD: {
        dmExtEvent         = UWA_DM_PROP_SE_WRITE_CALIB_DATA_NTF_EVT;
        uwbContext.wstatus = *responsePayloadPtr;
    } break;
    case EXT_UCI_MSG_READ_CALIB_DATA_CMD: {
        dmExtEvent = UWA_DM_PROP_SE_READ_CALIB_DATA_NTF_EVT;
        handle_read_calibration_data_ntf(responsePayloadPtr, responsePayloadLen);
    } break;
    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        break;
    }
    return dmExtEvent;
}
#endif // UWBIOT_UWBD_SR1XXT

eResponse_Ext_Ntf_Event processInternalNtf(uint8_t oid, uint16_t responsePayloadLen, uint8_t *responsePayloadPtr)
{
    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    eNotificationType ntfType          = UWBD_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;

    switch (oid) {
#if UWBFTR_UWBS_DEBUG_Dump
    case VENDOR_UCI_MSG_RANGING_TIMESTAMP_NTF: {
        dmExtEvent = UWA_DM_INTERNAL_RANGING_TIMESTAMP_NTF_EVT;
        ntfType    = UWBD_RANGING_TIMESTAMP_NTF;
    } break;
    case VENDOR_UCI_MSG_CMD_TIMESTAMP_NTF: {
        dmExtEvent = UWA_DM_INTERNAL_CMD_TIMESTAMP_NTF_EVT;
        ntfType    = UWBD_COMMAND_TIMESTAMP_NTF;
    } break;
    case VENDOR_UCI_MSG_DBG_RFRAME_LOG_NTF: {
        dmExtEvent = UWA_DM_INTERNAL_DBG_RFRAME_LOG_NTF_EVT;
        ntfType    = UWBD_RFRAME_DATA;
    } break;
    case VENDOR_UCI_MSG_DBG_DPD_INFO_NTF: {
        dmExtEvent = UWA_DM_INTERNAL_DBG_DPD_INFO_NTF_EVT;
        ntfType    = UWBD_DBG_DPD_INFO_NTF;
    } break;
    case VENDOR_UCI_MSG_CIR_PULL_DATA_NTF: {
        dmExtEvent = UWA_DM_INTERNAL_CIR_PULL_DATA_NTF_EVT;
        ntfType    = UWBD_CIR_PULL_DATA_NTF;
    } break;
#endif // UWBFTR_UWBS_DEBUG_Dump
    default:
        NXPLOG_UWBAPI_W("%s: unhandled oid 0x%x", __FUNCTION__, oid);
        skip_sem_post = TRUE;
        dmExtEvent    = UWA_EXT_DM_INVALID_NTF_EVT;
        ntfType       = UWBD_INVALID_NTF_EVT;
        break;
    }

    if (ntfType != UWBD_INVALID_NTF_EVT) {
        if (uwbContext.pAppCallback) {
            uwbContext.pAppCallback(ntfType, (void *)responsePayloadPtr, responsePayloadLen);
        }
    }

    return dmExtEvent;
}

void extDeviceManagementCallback(uint8_t gid, uint8_t oid, uint16_t paramLength, uint8_t *pResponseBuffer)
{
    uint16_t responsePayloadLen = 0;
    uint8_t *responsePayloadPtr = NULL;

    eResponse_Ext_Ntf_Event dmExtEvent = UWA_EXT_DM_INVALID_NTF_EVT;
    skip_sem_post                      = FALSE;

    if ((paramLength > UCI_RESPONSE_STATUS_OFFSET) && (pResponseBuffer != NULL)) {
        NXPLOG_UWBAPI_D(
            "extDeviceManagementCallback: Received length data = 0x%x "
            "status = 0x%x",
            paramLength,
            pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET]);

        responsePayloadLen = (uint16_t)(paramLength - UCI_RESPONSE_STATUS_OFFSET);
        responsePayloadPtr = &pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET];

        switch (gid) {
        case UCI_GID_PROPRIETARY_CUSTOM_1:
            dmExtEvent = processProprietaryNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
        case UCI_GID_PROPRIETARY_CUSTOM_2:
            dmExtEvent = processVendorNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
#if UWBIOT_UWBD_SR1XXT
        case UCI_GID_PROPRIETARY:
            dmExtEvent = processProprietarySeNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
#endif // UWBIOT_UWBD_SR1XXT
        case UCI_GID_INTERNAL_GROUP:
            dmExtEvent = processInternalNtf(oid, responsePayloadLen, responsePayloadPtr);
            break;
        default:
            NXPLOG_UWBAPI_W("%s: unhandled gid 0x%x", __FUNCTION__, gid);
            skip_sem_post = TRUE;
            break;
        }
        uwbContext.receivedEventId = dmExtEvent;
        if (uwbContext.currentEventId == dmExtEvent ||
            (isCmdRespPending() && (uwbContext.dev_state == UWBD_STATUS_ERROR))) {
            if (!skip_sem_post) {
                NXPLOG_UWBAPI_D("%s: posting devMgmtSem", __FUNCTION__);
                uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
                (void)phOsalUwb_ProduceSemaphore(uwbContext.devMgmtSem);
            }
        }
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: pResponseBuffer is NULL or paramLength is less than "
            "UCI_RESPONSE_STATUS_OFFSET",
            __FUNCTION__);
    }
}

tUWBAPI_STATUS setDefaultCoreConfigs(void)
{
    tUWBAPI_STATUS status  = UWBAPI_STATUS_FAILED;
    void *p_cmd            = NULL;
    long cmd_len           = 0;
    uint8_t config         = 0;
    uint16_t dpdTimeout    = 0;
    uint8_t extendedConfig = 0;
    uint8_t offset         = 1;

    NXPLOG_UWBAPI_D("%s: Enter ", __FUNCTION__);
    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_CORE_CONFIG_PARAM, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting UWB Core config failed", __FUNCTION__);
            goto exit;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_CORE_ANTENNAE_DEFINES, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Antennae Define failed", __FUNCTION__);
            goto exit;
        }
    }
#if UWBIOT_UWBD_SR1XXT
    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH5, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 5 failed", __FUNCTION__);
            goto exit;
        }
    }

    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_RX_ANTENNAE_DELAY_CALIB_CH9, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: Setting Rx Antennae delay calib for channel 9 failed", __FUNCTION__);
            goto exit;
        }
    }
#if (UWBFTR_AoA_FoV)
    status = setDefaultAoaCalibration();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_E("%s: Setting default AOA calibration values failed", __FUNCTION__);
        goto exit;
    }
#endif // UWBFTR_AoA_FoV
#endif // UWBIOT_UWBD_SR1XXT

    if (phNxpUciHal_GetNxpNumValue(UWB_LOW_POWER_MODE, &config, 0x01) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_LOW_POWER_MODE value %d ", __FUNCTION__, (uint8_t)config);

        offset = (uint8_t)getCoreDeviceConfigTLVBuffer(
            UCI_PARAM_ID_LOW_POWER_MODE, sizeof(config), (void *)&config, &uwbContext.snd_data[offset]);
        uwbContext.snd_data[0] = 1; // No of parameters
        sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
        status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);
        if (status == UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_D("%s: low power mode config is Sucess", __FUNCTION__);
        }
        else {
            NXPLOG_UWBAPI_E("%s: low power mode config is failed with status %s (0x%x)",
                __FUNCTION__,
                getStatusString(status),
                status);
            goto exit;
        }
    }

    if (phNxpUciHal_GetNxpNumValue(UWB_DPD_ENTRY_TIMEOUT, &dpdTimeout, 0x02) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_DPD_ENTRY_TIMEOUT value %d ", __FUNCTION__, dpdTimeout);

        offset = 1;
        if ((dpdTimeout >= UWBD_DPD_TIMEOUT_MIN) && (dpdTimeout <= UWBD_DPD_TIMEOUT_MAX)) {
            offset = (uint8_t)getExtDeviceConfigTLVBuffer(
                UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT, (void *)&dpdTimeout, &uwbContext.snd_data[offset]);
            uwbContext.snd_data[0] = 1; // No of parameters
            sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
            status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: UWBD_DPD_TIMEOUT_MIN is Success", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWBD_DPD_TIMEOUT_MIN is failed with status %s (0x%x)",
                    __FUNCTION__,
                    getStatusString(status),
                    status);
                goto exit;
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Range for DPD Entry Timeout in ConfigFile", __FUNCTION__);
            goto exit;
        }
    }

    /* During MCTT/PCTT execution, this setting needs to be disabled.
     * Disabling NXP Extended config to be taken care in the respective application.
     */
    if (phNxpUciHal_GetNxpNumValue(UWB_NXP_EXTENDED_NTF_CONFIG, &extendedConfig, 0x01) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_NXP_EXTENDED_NTF_CONFIG value %d ", __FUNCTION__, extendedConfig);
        offset = 1;
        if ((extendedConfig == DISABLED) || (extendedConfig == ENABLED)) {
            offset = (uint8_t)getExtDeviceConfigTLVBuffer(
                UCI_EXT_PARAM_ID_NXP_EXTENDED_NTF_CONFIG, (void *)&extendedConfig, &uwbContext.snd_data[offset]);
            uwbContext.snd_data[0] = 1; // No of parameters
            sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
            status = sendUciCommandAndWait(UWA_DM_API_CORE_SET_CONFIG_EVT, (uint16_t)(offset + 1), uwbContext.snd_data);
            if (status == UWBAPI_STATUS_OK) {
                NXPLOG_UWBAPI_D("%s: UWB_NXP_EXTENDED_NTF_CONFIG is Success", __FUNCTION__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWB_NXP_EXTENDED_NTF_CONFIG is failed with status %s (0x%x)",
                    __FUNCTION__,
                    getStatusString(status),
                    status);
                goto exit;
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Range for nxp extended ntf config in ConfigFile", __FUNCTION__);
            goto exit;
        }
    }

exit:
    NXPLOG_UWBAPI_D("%s: Exit ", __FUNCTION__);
    return status;
}

tUWBAPI_STATUS setVendorConfigs(void)
{
    NXPLOG_UWBAPI_D("setVendorConfigs Enter");
    void *p_cmd   = NULL;
    long cmd_len  = 0;
    int status    = UWBAPI_STATUS_OK;
    uint8_t count = 0;
    if (phNxpUciHal_GetNxpNumValue(UWB_NXP_CORE_CONFIG_BLOCK_COUNT, &count, sizeof(count))) {
        NXPLOG_UWBAPI_D("setVendorConfigs :: Value of count in %s is %x", __FUNCTION__, count);
    }
    for (int i = 0; i < count; i++) {
        if ((phNxpUciHal_GetNxpByteArrayValue(nxp_config_block_names[i], &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
            status = sendRawUci((uint8_t *)p_cmd, (uint16_t)cmd_len);
            if (status != UWBAPI_STATUS_OK) {
                LOG_E("%s : set vendor configs for block %d failed", __FUNCTION__, count);
            }
        }
        else {
            NXPLOG_UWBAPI_D("setVendorConfigs: cmd_len is %ld", cmd_len);
            break;
        }
    }
    NXPLOG_UWBAPI_D("setVendorConfigs Exit");

    return status;
}