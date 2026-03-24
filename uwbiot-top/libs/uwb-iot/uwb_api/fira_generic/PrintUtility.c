/*
 *
 * Copyright 2018-2020,2023-2026 NXP.
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

#include "PrintUtility.h"
#include "uwb_types.h"
#include "phNxpLogApis_App.h"
#include "phNxpLogApis_App.h"

EXTERNC void printGenericErrorStatus(const phGenericError_t *pGenericError)
{
    if (pGenericError != NULL) {
        NXPLOG_APP_I("Status                        : %hu", pGenericError->status);
    }
}

EXTERNC void printSessionStatusData(const phUwbSessionInfo_t *pSessionInfo)
{
    if (pSessionInfo != NULL) {
        NXPLOG_APP_I("pSessionInfo->sessionHandle          : %" PRIu32 "", pSessionInfo->sessionHandle);
        NXPLOG_APP_I("pSessionInfo->state               : %hu", pSessionInfo->state);
        NXPLOG_APP_I("pSessionInfo->reason_code         : %hu", pSessionInfo->reason_code);
    }
    else {
        NXPLOG_APP_E("pSessionInfo is NULL");
    }
}

EXTERNC void printUwbSessionData(const phUwbSessionsContext_t *pUwbSessionsContext)
{
    if (pUwbSessionsContext != NULL) {
        NXPLOG_APP_I("Status                        : %" PRIu32 " ", pUwbSessionsContext->status);
        NXPLOG_APP_I("Session Counter               : %d ", pUwbSessionsContext->sessioncnt);
        for (uint8_t i = 0; i < pUwbSessionsContext->sessioncnt; i++) {
            NXPLOG_APP_I(
                "Session %d ID             : %" PRIu32 " ", i, pUwbSessionsContext->pUwbSessionData[i].sessionHandle);
            NXPLOG_APP_I("Session %d Type           : %hu", i, pUwbSessionsContext->pUwbSessionData[i].session_type);
            NXPLOG_APP_I("Session %d State          : %hu", i, pUwbSessionsContext->pUwbSessionData[i].session_state);
        }
    }
    else {
        NXPLOG_APP_E("pUwbSessionsContext is NULL");
    }
}

EXTERNC void printMulticastListStatus(const phMulticastControleeListNtfContext_t *pControleeNtfContext)
{
    if (pControleeNtfContext != NULL) {
        NXPLOG_APP_I(
            "pControleeNtfContext->sessionHandle          : %" PRIu32 " ", pControleeNtfContext->sessionHandle);
        NXPLOG_APP_I("pControleeNtfContext->no_of_controlees    : %hu", pControleeNtfContext->no_of_controlees);

        for (uint8_t i = 0; i < pControleeNtfContext->no_of_controlees; i++) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
            NXPLOG_APP_I("pControleeNtfContext->controleeStatusList[%hu].controlee_mac_address  : %" PRIu16 "",
                i,
                pControleeNtfContext->controleeStatusList[i].controlee_mac_address);
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
            NXPLOG_APP_I("pControleeNtfContext->controleeStatusList[%hu].status                 : %hu",
                i,
                pControleeNtfContext->controleeStatusList[i].status);
        }
    }
    else {
        LOG_E("phControleeNtfContext_t is NULL");
    }
}

EXTERNC void printRangingParams(const phRangingParams_t *pRangingParams)
{
    if (pRangingParams != NULL) {
        NXPLOG_APP_D("pRangingParams->deviceType                    : %hu", pRangingParams->deviceType);
        NXPLOG_APP_D("pRangingParams->deviceRole                    : %hu", pRangingParams->deviceRole);
        NXPLOG_APP_D("pRangingParams->multiNodeMode                 : %hu", pRangingParams->multiNodeMode);
        NXPLOG_APP_D("pRangingParams->macAddrMode                   : %hu", pRangingParams->macAddrMode);
        NXPLOG_APP_D("pRangingParams->scheduledMode                 : %hu", pRangingParams->scheduledMode);
        NXPLOG_APP_D("pRangingParams->rangingRoundUsage             : %hu", pRangingParams->rangingRoundUsage);

#if (APP_LOG_LEVEL > UWB_LOG_INFO_LEVEL)
        uint8_t addrLen = SHORT_MAC_ADDR_LEN;
        if (pRangingParams->macAddrMode != SHORT_MAC_ADDRESS_MODE) { // mac addr is of 2 or 8 bytes.
            addrLen = EXT_MAC_ADDR_LEN;
        }
#endif
        LOG_MAU8_D("pRangingParams->deviceMacAddr                 : ", pRangingParams->deviceMacAddr, addrLen);
    }
    else {
        NXPLOG_APP_E("pRangingParams is NULL");
    }
}
/* clang-format off */
EXTERNC void printRangingData(const phRangingData_t *pRangingData)
{
    if (pRangingData != NULL) {
        NXPLOG_APP_D("--------------Received Range Data--------------");
        NXPLOG_APP_D("pRangingData->seq_ctr                             : %" PRIu32 " ", pRangingData->seq_ctr);
        NXPLOG_APP_D("pRangingData->sessionHandle                       : 0x%08X ", pRangingData->sessionHandle);
        NXPLOG_APP_D("pRangingData->rcr_indication                      : %hu", pRangingData->rcr_indication);
        NXPLOG_APP_D("pRangingData->curr_range_interval                 : %" PRIu32 " ", pRangingData->curr_range_interval);
        NXPLOG_APP_D("pRangingData->ranging_measure_type                : %hu", pRangingData->ranging_measure_type);
        NXPLOG_APP_D("pRangingData->mac_addr_mode_indicator             : %hu", pRangingData->mac_addr_mode_indicator);
        NXPLOG_APP_D("pRangingData->sessionHandle_of_primary_session    : 0x%x", pRangingData->sessionHandle_of_primary_session);
        NXPLOG_APP_D("pRangingData->no_of_measurements                  : %hu", pRangingData->no_of_measurements);
#if (UWBIOT_UWBD_SR04X)
        NXPLOG_APP_D("pRangingData->antenna_info                        : %hu", pRangingData->antenna_info);
#endif //UWBIOT_UWBD_SR04X

#if UWBFTR_TWR // support only for DSTWR
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_TWOWAY) {
            for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
                LOG_MAU8_D("pRangingData->range_meas.mac_addr           ", pRangingData->ranging_meas.range_meas_twr[i].mac_addr,(pRangingData->mac_addr_mode_indicator == SHORT_MAC_ADDRESS_MODE ? SHORT_MAC_ADDR_LEN : EXT_MAC_ADDR_LEN));
                NXPLOG_APP_D("TWR[%d].status                            : 0x%x ", i, pRangingData->ranging_meas.range_meas_twr[i].status);
                if ((pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK) || (pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT)) {
                    NXPLOG_APP_D("TWR[%d].nLos                              : %hu", i, pRangingData->ranging_meas.range_meas_twr[i].nLos);

                    /* This is a good thing to report... so keep it under INFO Tag. */
                    NXPLOG_APP_I("TWR[%" PRIu16 "].distance                          : %" PRIu16 "", i, pRangingData->ranging_meas.range_meas_twr[i].distance);
                    if (pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK_NEGATIVE_DISTANCE_REPORT) {
                        NXPLOG_APP_I("TWR[%" PRIu16 "]  Negative Distance Reported", i);
                    }

                    NXPLOG_APP_D("TWR[%d].aoa_azimuth                       : %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_azimuth));
                    NXPLOG_APP_D("TWR[%d].aoa_azimuth_FOM                   : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_azimuth_FOM);
                    NXPLOG_APP_D("TWR[%d].aoa_elevation                     : %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_elevation));
                    NXPLOG_APP_D("TWR[%d].aoa_elevation_FOM                 : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_elevation_FOM);
                    NXPLOG_APP_D("TWR[%d].aoa_dest_azimuth                  : %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_azimuth));
                    NXPLOG_APP_D("TWR[%d].aoa_dest_azimuth_FOM              : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_azimuth_FOM);
                    NXPLOG_APP_D("TWR[%d].aoa_dest_elevation                : %d.%d", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_elevation));
                    NXPLOG_APP_D("TWR[%d].aoa_dest_elevation_FOM            : %d", i, pRangingData->ranging_meas.range_meas_twr[i].aoa_dest_elevation_FOM);
                    NXPLOG_APP_D("TWR[%d].slot_index                        : %d", i, pRangingData->ranging_meas.range_meas_twr[i].slot_index);
                    NXPLOG_APP_D("TWR[%d].rssi                              : %d.%d", i, TO_Q_7_1(pRangingData->ranging_meas.range_meas_twr[i].rssi));
                }
            }
#if !(UWBIOT_UWBD_SR04X)
            if (pRangingData->vs_length != FALSE) {
                NXPLOG_APP_D("TWR.vs_length                            : %d ", pRangingData->vs_length);
                NXPLOG_APP_D("TWR.vs_data_type                         : %x ", pRangingData->vs_data_type);
                if(pRangingData->vs_data_type == NXP_SPECIFIC_DATA_TYPE_WITH_MSG_CNTRL){
                    NXPLOG_APP_D("TWR.Message control                   : 0x%08x",pRangingData->message_control);
                }
                NXPLOG_APP_D("TWR.wifiCoExStatus                       : %d", pRangingData->vs_data.twr.wifiCoExStatus);
                if(pRangingData->vs_data.twr.isTxAntInfoPreset){
                    NXPLOG_APP_D("TWR.txAntennaInfo                    : %d", pRangingData->vs_data.twr.txAntennaInfo);
                }
                NXPLOG_APP_D("TWR.rxMode                               : %d", pRangingData->vs_data.twr.rxInfoMesr_twr.rxMode);
                NXPLOG_APP_D("TWR.num_of_rx_antennaRxInfo              : %d", pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo);
                if(pRangingData->vs_data.twr.isRxAntDebugInfoPresent){
                    NXPLOG_APP_D("TWR.numOfRframes                         : %d",pRangingData->vs_data.twr.rxInfoDebugNtf_twr.numOfRframes);
                }
                else{
                    NXPLOG_APP_D("TWR.rxModeDebugNtf                   : %d",pRangingData->vs_data.twr.rxInfoDebugNtf_twr.rxModeDebugNtf);
                }
                NXPLOG_APP_D("TWR.num_of_rx_antennaDebugNtf            : %d",pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf);
                if (pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo != FALSE) {
                    for (int j = 0; j < pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo; j++) {
                        NXPLOG_APP_D("TWR.rx_antennaIdRxInfo                   : %d", pRangingData->vs_data.twr.rxInfoMesr_twr.rx_antennaIdRxInfo[j]);
                    }
                }
                if (pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf != FALSE) {
                    for (int k = 0; k < pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf; k++) {
                        NXPLOG_APP_D("TWR.rx_antennaIdDebugNtf[%d]              : %d", k, pRangingData->vs_data.twr.rxInfoDebugNtf_twr.rx_antennaIdDebugNtf[k]);
                    }
                }
                for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
                    LOG_MAU8_D("Vendor Specific info for responder.mac_addr ", pRangingData->ranging_meas.range_meas_twr[i].mac_addr,(pRangingData->mac_addr_mode_indicator == SHORT_MAC_ADDRESS_MODE ? SHORT_MAC_ADDR_LEN : EXT_MAC_ADDR_LEN));
                    if (pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf != FALSE) {
                        for (int j = 0; j < pRangingData->vs_data.twr.rxInfoMesr_twr.num_of_rx_antennaRxInfo; j++) {
                            NXPLOG_APP_D("TWR[%d].angleOfArrival[%d]                 : %d.%d", i, j, TO_Q_9_7(pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].angleOfArrival));
                            NXPLOG_APP_D("TWR[%d].pdoa[%d]                           : %d.%d", i, j, TO_Q_9_7(pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].pdoa));
                            NXPLOG_APP_D("TWR[%d].pdoaIndex[%d]                      : %d",i, j, pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].pdoaIndex);
#if UWBFTR_AoA_FoV
                            if(pRangingData->vs_data_type == FOV_SPECIFIC_DATA_TYPE)
                            {
                                NXPLOG_APP_D("TWR[%d].aoaFovFlag[%d]                    : %d ",i, j, pRangingData->vs_data.twr.vsMesr[i].aoaPdoaMesr_twr[j].aoaFovFlag);
                            }
#endif // UWBFTR_AoA_FoV
                        }
                    }
                    if (pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf != FALSE) {
                        for (int k = 0; k < pRangingData->vs_data.twr.rxInfoDebugNtf_twr.num_of_rx_antennaDebugNtf; k++) {
                            NXPLOG_APP_D("TWR[%d].rxSnrFirstPath[%d]                 : %d", i, k, pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rxSnrFirstPath);
                            NXPLOG_APP_D("TWR[%d].rxSnrMainPath[%d]                  : %d", i, k, pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rxSnrMainPath);
                            NXPLOG_APP_D("TWR[%d].rx_FirstPathIndex[%d]              : %d.%d", i, k, TO_Q_6_10(pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rx_FirstPathIndex));
                            NXPLOG_APP_D("TWR[%d].rx_MainPathIndex[%d]               : %d.%d", i, k, TO_Q_6_10(pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rx_MainPathIndex));
                        if(pRangingData->vs_data.twr.isRxAntDebugInfoPresent){
                            NXPLOG_APP_D("TWR[%d].snrTotal[%d]                       : %d.%d", i, k, TO_Q_8_8(pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].snrTotal));
                            NXPLOG_APP_D("TWR[%d].rssi[%d]                           : %d.%d", i, k, TO_Q_8_8(pRangingData->vs_data.twr.vsMesr[i].snrPathIndexMesr_twr[k].rssi));
                        }
                        }
                    }
                    if ((pRangingData->vs_data.twr.rxInfoMesr_twr.rxMode == kUWBAntCfgRxMode_ToA_Rfm_Mode) || (pRangingData->vs_data.twr.rxInfoMesr_twr.rxMode == kUWBAntCfgRxMode_AoA_Rfm_Mode)) {
                        NXPLOG_APP_D("TWR[%d].distance_2                        : %d", i, pRangingData->vs_data.twr.vsMesr[i].distance_2);
                    }
                    if(pRangingData->vs_data.twr.isDistanceMmPresent){
                        NXPLOG_APP_D("TWR[%d].distance_mm                        : %d", i, pRangingData->vs_data.twr.vsMesr[i].distance_mm);
                    }
                }
            }
#endif //!(UWBIOT_UWBD_SR04X)
        }
#endif //UWBFTR_TWR

#if UWBFTR_UL_TDoA_Anchor
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_ONEWAY) {
            for (int i = 0; i < pRangingData->no_of_measurements; i++) {
                LOG_MAU8_D("pRangingData->range_meas.mac_addr       ",
                    pRangingData->ranging_meas.range_meas_tdoa[i].mac_addr, (pRangingData->mac_addr_mode_indicator == SHORT_MAC_ADDRESS_MODE ? SHORT_MAC_ADDR_LEN : EXT_MAC_ADDR_LEN));
                NXPLOG_APP_D("TDoA[%d].status                       : %d", i, pRangingData->ranging_meas.range_meas_tdoa[i].status);
                NXPLOG_APP_D("TDoA[%d].message_control              : %d",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].message_control);
                NXPLOG_APP_D(
                    "TDoA[%d].frame_type                   : %d", i, pRangingData->ranging_meas.range_meas_tdoa[i].frame_type);
                NXPLOG_APP_D(
                    "TDoA[%d].nLos                         : %d", i, pRangingData->ranging_meas.range_meas_tdoa[i].nLos);
                NXPLOG_APP_D("TDoA[%" PRIi16 "].aoa_azimuth         : %d.%d",
                    i,
                    TO_Q_9_7(pRangingData->ranging_meas.range_meas_tdoa[i].aoa_azimuth));
                NXPLOG_APP_D("TDoA[%d].aoa_azimuth_FOM              : %d",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].aoa_azimuth_FOM);
                NXPLOG_APP_D("TDoA[%" PRIi16 "].aoa_elevation       : %d.%d",
                    i,
                    TO_Q_9_7(pRangingData->ranging_meas.range_meas_tdoa[i].aoa_elevation));
                NXPLOG_APP_D("TDoA[%d].aoa_elevation_FOM            : %d",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].aoa_elevation_FOM);
                NXPLOG_APP_D("TDoA[%d].frame_number                 : %" PRIu32 " ",
                    i,
                    pRangingData->ranging_meas.range_meas_tdoa[i].frame_number);
                LOG_MAU8_D("TDoA.rx_timestamp                       :",
                    pRangingData->ranging_meas.range_meas_tdoa[i].rx_timestamp, ULTDOA_64BIT_IN_BYTES);
                LOG_MAU8_D("TDoA.ul_tdoa_device_id                  :",
                    pRangingData->ranging_meas.range_meas_tdoa[i].ul_tdoa_device_id, ULTDOA_64BIT_IN_BYTES);
                LOG_MAU8_D("TDoA.tx_timestamp                       :",
                    pRangingData->ranging_meas.range_meas_tdoa[i].tx_timestamp, ULTDOA_64BIT_IN_BYTES);
            }
#if !(UWBIOT_UWBD_SR04X)
            if (pRangingData->vs_length != FALSE) {
                NXPLOG_APP_D("TDoA.vs_length                    : %d ", pRangingData->vs_length);
                NXPLOG_APP_D("TDoA.vendorExtLength              : %d ", pRangingData->vs_data.tdoa.vendorExtLength);
                NXPLOG_APP_D("TDoA.rssi_rx1                     : %d.%d", TO_Q_8_8(pRangingData->vs_data.tdoa.rssi_rx1));
                NXPLOG_APP_D("TDoA.rssi_rx2                     : %d.%d", TO_Q_8_8(pRangingData->vs_data.tdoa.rssi_rx2));
                NXPLOG_APP_D("TDoA.noOfPdoaMeasures             : %d", pRangingData->vs_data.tdoa.noOfPdoaMeasures);
                if (pRangingData->vs_data.tdoa.noOfPdoaMeasures != FALSE) {
                    for (uint8_t j = 0; j < pRangingData->vs_data.tdoa.noOfPdoaMeasures; j++) {
                        NXPLOG_APP_D("TDoA.pdoaFirst[%d]                : %d.%d", j, TO_Q_9_7(pRangingData->vs_data.tdoa.pdoa[j]));
                    }
                }
                if (pRangingData->vs_data.tdoa.noOfPdoaMeasures != FALSE) {
                    for (int j = 0; j < pRangingData->vs_data.tdoa.noOfPdoaMeasures; j++) {
                        NXPLOG_APP_D("TDoA.pdoaFirstIndex[%d]           : %d", j, pRangingData->vs_data.tdoa.pdoaIndex[j]);
                    }
                }
#if UWBIOT_UWBD_SR2XXT
                NXPLOG_APP_D("TDoA.message_control_extension    : %d ", pRangingData->vs_data.tdoa.message_control_extension);
                NXPLOG_APP_D("TDoA.rx3_ext_info_len             : %d", pRangingData->vs_data.tdoa.rx3_ext_info_len);
                NXPLOG_APP_D("TDoA.rssi_rx3                     : %d.%d", TO_Q_8_8(pRangingData->vs_data.tdoa.rssi_rx3));
#endif //UWBIOT_UWBD_SR2XXT
            NXPLOG_APP_D("pRangingData->antenna_pairInfo    : 0x%X ", pRangingData->antenna_pairInfo);
            NXPLOG_APP_D("pRangingData->wifiCoExStatusCode  : %d", pRangingData->wifiCoExStatusCode);
            }
#endif //!(UWBIOT_UWBD_SR04X)
        }

#endif //UWBFTR_UL_TDoA_Anchor
#if UWBFTR_DL_TDoA_Tag
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_DLTDOA) {
            for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
                if (pRangingData->ranging_meas.range_meas_dltdoa[i].status == UCI_STATUS_OK) {
                    LOG_MAU8_D("pRangingData->range_meas.mac_addr        :",
                        pRangingData->ranging_meas.range_meas_dltdoa[i].mac_addr,
                        (pRangingData->mac_addr_mode_indicator == SHORT_MAC_ADDRESS_MODE ? SHORT_MAC_ADDR_LEN : EXT_MAC_ADDR_LEN));

                    NXPLOG_APP_D("DLTDoA[%d].status              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].status);

                    NXPLOG_APP_D("DLTDoA[%d].message_type              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].message_type);

                    NXPLOG_APP_D("DLTDoA[%d].message_control              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].message_control);

                    NXPLOG_APP_D("DLTDoA[%d].block_index              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].block_index);

                    NXPLOG_APP_D("DLTDoA[%d].round_index              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].round_index);

                    NXPLOG_APP_D(
                        "DLTDoA[%d].nLoS              : 0x%x\n", i, pRangingData->ranging_meas.range_meas_dltdoa[i].nLoS);

                    NXPLOG_APP_D("DLTDoA[%d].aoa_azimuth             : %d.%d\n",
                        i,
                        TO_Q_9_7(pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_azimuth));

                    NXPLOG_APP_D("DLTDoA[%d].aoa_azimuth_fom             : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_azimuth_fom);

                    NXPLOG_APP_D("DLTDoA[%d].aoa_elevation            : %d.%d\n",
                        i,
                        TO_Q_9_7(pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_elevation));

                    NXPLOG_APP_D("DLTDoA[%d].aoa_elevation_fom            : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].aoa_elevation_fom);

                    NXPLOG_APP_D(
                        "DLTDoA[%d].rssi              : 0x%d.%d\n", i, TO_Q_7_1(pRangingData->ranging_meas.range_meas_dltdoa[i].rssi));

                    LOG_MAU8_D("pRangingData->range_meas.tx_timestamp        :",
                        pRangingData->ranging_meas.range_meas_dltdoa[i].tx_timestamp,
                        MAX_RX_TX_TIMESTAMP);

                    LOG_MAU8_D("pRangingData->range_meas.rx_timestamp        :",
                        pRangingData->ranging_meas.range_meas_dltdoa[i].rx_timestamp,
                        MAX_RX_TX_TIMESTAMP);

                    NXPLOG_APP_D("DLTDoA[%d].cfo_anchor              : %d.%d\n",
                        i,
                        TO_Q_6_10(pRangingData->ranging_meas.range_meas_dltdoa[i].cfo_anchor));

                    NXPLOG_APP_D("DLTDoA[%d].cfo              : %d.%d\n",
                        i,
                        TO_Q_6_10(pRangingData->ranging_meas.range_meas_dltdoa[i].cfo));

                    NXPLOG_APP_D("DLTDoA[%d].reply_time_initiator              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].reply_time_initiator);

                    NXPLOG_APP_D("DLTDoA[%d].reply_time_responder              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].reply_time_responder);

                    NXPLOG_APP_D("DLTDoA[%d].initiator_responder_tof              : 0x%x\n",
                        i,
                        pRangingData->ranging_meas.range_meas_dltdoa[i].initiator_responder_tof);
                }
            }
            NXPLOG_APP_D("pRangingData->wifiCoExStatusCode                    : %d ", pRangingData->wifiCoExStatusCode);
            NXPLOG_APP_D("pRangingData->antenna_pairInfo                    : %" PRIu32 " ", pRangingData->antenna_pairInfo);
        }
#endif //UWBFTR_DL_TDoA_Tag
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
        /**
         * Below fields are applicable for Observer side .
        */
        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_OWR_WITH_AOA) {
            for (int i = 0; i < pRangingData->no_of_measurements; i++) {
                LOG_MAU8_D("OWR.mac_addr                          ", pRangingData->ranging_meas.range_meas_owr_aoa[i].mac_addr, (pRangingData->mac_addr_mode_indicator == SHORT_MAC_ADDRESS_MODE ? SHORT_MAC_ADDR_LEN : EXT_MAC_ADDR_LEN));

                NXPLOG_APP_D("OWR[%d].status                            : 0x%x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].status);
                NXPLOG_APP_D("OWR[%d].nLos                              : 0x%x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].nLos);
                NXPLOG_APP_D("OWR[%d].frame_seq_num                     : 0x%x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].frame_seq_num);
                NXPLOG_APP_D("OWR[%d].block_index                       : %d\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].block_index);
                NXPLOG_APP_D("OWR[%d].aoa_azimuth                       : %d.%d\n", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_azimuth));
                NXPLOG_APP_D("OWR[%d].aoa_azimuth_FOM                   : 0x%x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_azimuth_FOM);
                NXPLOG_APP_D("OWR[%d].aoa_elevation                     : %d.%d\n", i, TO_Q_9_7(pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_elevation));
                NXPLOG_APP_D("OWR[%d].aoa_elevation_FOM                 : 0x%x\n", i, pRangingData->ranging_meas.range_meas_owr_aoa[i].aoa_elevation_FOM);
            }
            if (pRangingData->vs_length != FALSE) {
                NXPLOG_APP_D("OWR.vs_length                             : %d \n", pRangingData->vs_length);
                NXPLOG_APP_D("OWR.vs_data_type                          : %d \n", pRangingData->vs_data_type);
                NXPLOG_APP_D("OWR.rxMode                                : %d \n", pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.rxMode);
                NXPLOG_APP_D("OWR.num_of_rx_antennaRxInfo               : %d \n", pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo);
                for (int i = 0; i < pRangingData->no_of_measurements; i++) {
                    if (pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo != FALSE) {
                        for (int m = 0; m < pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.num_of_rx_antennaRxInfo; m++) {
                            NXPLOG_APP_D("OWR[%d].rxantennaIdRxInfo[%d]             : %d \n", i, m, pRangingData->vs_data.owr_aoa.rxInfoMesr_owr.rx_antennaIdRxInfo[m]);
                            NXPLOG_APP_D("OWR[%d].angleOfArrival[%d]                : %d.%d \n", i, m, TO_Q_9_7(pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].angleOfArrival));
                            NXPLOG_APP_D("OWR[%d].pdoa[%d]                          : %d.%d \n", i, m, TO_Q_9_7(pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].pdoa));
                            NXPLOG_APP_D("OWR[%d].pdoaIndex[%d]                     : %d \n", i, m, pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].pdoaIndex);
#if UWBIOT_UWBD_SR150
                            if(pRangingData->vs_data_type == FOV_SPECIFIC_DATA_TYPE)
                            {
                                NXPLOG_APP_D("OWR[%d].aoaFovFlag[%d]                    : %d ", i, m, pRangingData->vs_data.owr_aoa.vsMesr[i].aoaPdoaMesr_owr[m].aoaFovFlag);
                            }
#endif // UWBIOT_UWBD_SR150
                        }
                    }
                    NXPLOG_APP_D("OWR[%d].rssi                              : %d.%d \n", i, TO_Q_8_8(pRangingData->vs_data.owr_aoa.vsMesr[i].rssi));
                }
            }
        }
#endif //(UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR200T)
#if !(UWBIOT_UWBD_SR04X)
    NXPLOG_APP_D("pRangingData->authInfoPrsen      : %d\n ", pRangingData->authInfoPrsen);
        if (pRangingData->authInfoPrsen != FALSE) {
                    LOG_MAU8_D("pRangingData->authenticationTag   : ", pRangingData->authenticationTag, AUTH_TAG_IN_16BYTES);
        }
#endif //!(UWBIOT_UWBD_SR04X)
    }
    else {
        NXPLOG_APP_D("pRangingData is NULL");
    }
}
/* clang-format on */

#if UWBFTR_DataTransfer
EXTERNC void printTransmitStatus(const phUwbDataTransmit_t *pTransmitNtfContext)
{
    if (pTransmitNtfContext != NULL) {
        NXPLOG_APP_I("pTransmitNtfContext->transmitNtf_connectionId       : 0%x\n",
            pTransmitNtfContext->transmitNtf_connectionId);
        NXPLOG_APP_I("pTransmitNtfContext->transmitNtf_sequence_number     : %d\n",
            pTransmitNtfContext->transmitNtf_sequence_number);
        NXPLOG_APP_I(
            "pTransmitNtfContext->transmitNtf_status              : %d\n", pTransmitNtfContext->transmitNtf_status);
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        NXPLOG_APP_I(
            "pTransmitNtfContext->transmitNtf_txcount             : %d\n", pTransmitNtfContext->transmitNtf_txcount);
#endif /* (UWBIOT_UWBD_SR04X) */
    }
    else {
        NXPLOG_APP_E("pTransmitNtfContext is NULL");
    }
}

EXTERNC void printCreditStatus(const phUwbDataCredit_t *pCreditNtfContext)
{
    if (pCreditNtfContext != NULL) {
        NXPLOG_APP_I("pCreditNtfContext->connectionId               : 0%x\n", pCreditNtfContext->connectionId);
        NXPLOG_APP_I("pCreditNtfContext->credit_availability        : %d\n", pCreditNtfContext->credit_availability);
    }
    else {
        NXPLOG_APP_E("pCreditNtfContext is NULL");
    }
}

EXTERNC void printRcvDataStatus(const phUwbRcvDataPkt_t *pRcvDataPkt)
{
    if (pRcvDataPkt != NULL) {
        NXPLOG_APP_I("pRcvDataPkt->connection_identifier            : 0x%x", pRcvDataPkt->connection_identifier);
        NXPLOG_APP_I("pRcvDataPkt->status                   : 0x%x", pRcvDataPkt->status);
        LOG_MAU8_D("SrcMacAddr                              : ", pRcvDataPkt->src_address, EXT_MAC_ADDR_LEN);
        NXPLOG_APP_I("pRcvDataPkt->sequence_number          : %d", pRcvDataPkt->sequence_number);
        if (pRcvDataPkt->status == UWBAPI_STATUS_OK) {
            NXPLOG_APP_I("data_size                             : %d", pRcvDataPkt->data_size);
            LOG_MAU8_I("DataRcv                                 : ", pRcvDataPkt->data, pRcvDataPkt->data_size);
        }
    }
    else {
        NXPLOG_APP_E("pRcvDataPkt is NULL");
    }
}
#endif // UWBFTR_DataTransfer

#if UWBFTR_Radar

EXTERNC void printRadarRecvNtf(const phUwbRadarNtf_t *pRcvRadaNtfPkt)
{
    if (pRcvRadaNtfPkt != NULL) {
        NXPLOG_APP_D("pRcvRadaNtfPkt->sessionHandle          : 0x%x\n", pRcvRadaNtfPkt->sessionHandle);
        NXPLOG_APP_D("pRcvRadaNtfPkt->radar_status          : 0x%x\n", pRcvRadaNtfPkt->radar_status);
        NXPLOG_APP_D("pRcvRadaNtfPkt->radar_type          : 0x%x\n", pRcvRadaNtfPkt->radar_type);
        NXPLOG_APP_D("pRcvRadaNtfPkt->num_cirs          : 0x%x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.num_cirs);
        NXPLOG_APP_D("pRcvRadaNtfPkt->cir_taps          : 0x%x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_taps);
        NXPLOG_APP_D("pRcvRadaNtfPkt->rfu          : 0x%x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.rfu);
        NXPLOG_APP_D("pRcvRadaNtfPkt->len          : 0x%x\n", pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_len);
        LOG_MAU8_D("pRcvRadaNtfPkt->CIRDATA",
            pRcvRadaNtfPkt->radar_ntf.radr_cir.cirdata,
            pRcvRadaNtfPkt->radar_ntf.radr_cir.cir_len);
    }
    else {
        NXPLOG_APP_E("pRcvRadaNtfPkt is NULL");
    }
}

EXTERNC void printRadarTestIsoNtf(const phUwbRadarNtf_t *pRcvRadaTstNtfPkt)
{
    if (pRcvRadaTstNtfPkt != NULL) {
        NXPLOG_APP_I("sessionHandle          : 0x%x\n", pRcvRadaTstNtfPkt->sessionHandle);
        NXPLOG_APP_I("radar_status        : 0x%x\n", pRcvRadaTstNtfPkt->radar_status);
        NXPLOG_APP_I("radar_type          : 0x%x\n", pRcvRadaTstNtfPkt->radar_type);
        NXPLOG_APP_I("antenna_tx          : 0x%x\n", pRcvRadaTstNtfPkt->radar_ntf.radar_tst_ntf.antenna_tx);
        NXPLOG_APP_I("antenna_rx          : 0x%x\n", pRcvRadaTstNtfPkt->radar_ntf.radar_tst_ntf.antenna_rx);
        NXPLOG_APP_I("anteena_isolation   : 0x%x\n", pRcvRadaTstNtfPkt->radar_ntf.radar_tst_ntf.anteena_isolation);
    }
    else {
        NXPLOG_APP_E("pRcvRadaNtfPkt is NULL");
    }
}

EXTERNC void printRadarPresenceDetctionNtf(const phUwbRadarNtf_t *pRcvRadarPresenceNtfPkt)
{
    if (pRcvRadarPresenceNtfPkt != NULL) {
        NXPLOG_APP_I("sessionHandle           : 0x%x\n", pRcvRadarPresenceNtfPkt->sessionHandle);
        NXPLOG_APP_I("radar_status            : 0x%x\n", pRcvRadarPresenceNtfPkt->radar_status);
        NXPLOG_APP_I("radar_type              : 0x%x\n", pRcvRadarPresenceNtfPkt->radar_type);
        NXPLOG_APP_I("presence_detected: %s:%d\n",
            ((pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.presence_detected) & 0x01) ? "Yes" : "No",
            pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.presence_detected);
        NXPLOG_APP_I("presence_detection_mode : 0x%x\n",
            pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.presence_detection_mode);
        NXPLOG_APP_I("number_of_detections : 0x%x\n",
            pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.number_of_detections);
        for (uint8_t i = 0; i < pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.number_of_detections;
             i++) {
            NXPLOG_APP_I("Target[%d]:detection_distance      : %d cm\n",
                i,
                pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.detection_distance[i]);
            NXPLOG_APP_I("Target[%d]:detection_aoa           : %d degrees\n",
                i,
                pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.detection_aoa[i]);
            NXPLOG_APP_I("Target[%d]:detection_track_id      : 0x%x\n",
                i,
                pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.detection_track_id[i]);
            NXPLOG_APP_I("Target[%d]:detection_value         : 0x%x\n",
                i,
                pRcvRadarPresenceNtfPkt->radar_ntf.radar_presence_detect_ntf.detection_value[i]);
        }
    }
    else {
        NXPLOG_APP_E("pRcvRadaNtfPkt is NULL\n");
    }
}

#endif // UWBFTR_Radar

#if UWBFTR_CCC && !(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
EXTERNC void printCccRangingData(const phCccRangingData_t *pCccRangingData)
{
    if (pCccRangingData != NULL) {
        NXPLOG_APP_D("------------------Received Ccc Range Data------------------");
        NXPLOG_APP_D("CCC.sessionHandle                             : 0x%x ", pCccRangingData->sessionHandle);
        NXPLOG_APP_D("CCC.rangingStatus                             : 0x%.2X", pCccRangingData->rangingStatus);
        NXPLOG_APP_D("CCC.stsIndex                                  : %" PRIu32 "", pCccRangingData->stsIndex);
        NXPLOG_APP_D("CCC.rangingRoundIndex                         : %" PRIu16 "", pCccRangingData->rangingRoundIndex);
        NXPLOG_APP_I("CCC.distance                                  : %" PRIu16 "", pCccRangingData->distance);
        NXPLOG_APP_D("CCC.uncertanityAnchorFom                      : %hu", pCccRangingData->uncertanityAnchorFom);
        NXPLOG_APP_D("CCC.uncertanityInitiatorFom                   : %hu", pCccRangingData->uncertanityInitiatorFom);
        LOG_MAU8_D("CCC.ccmTag                                    ", pCccRangingData->ccmTag, MAX_CCM_TAG_SIZE);
        NXPLOG_APP_D("CCC.aoa_azimuth                               : %d.%d", TO_Q_9_7(pCccRangingData->aoa_azimuth));
        NXPLOG_APP_D("CCC.aoa_azimuth_fom                           : %d", pCccRangingData->aoa_azimuth_FOM);
        NXPLOG_APP_D("CCC.aoa_elevation                             : %d.%d", TO_Q_9_7(pCccRangingData->aoa_elevation));
        NXPLOG_APP_D("CCC.aoa_elevation_fom                         : %d", pCccRangingData->aoa_elevation_FOM);

        NXPLOG_APP_D(
            "CCC.antenna_pair_info.configMode              : %d", pCccRangingData->antenna_pair_info.configMode);
        NXPLOG_APP_D(
            "CCC.antenna_pair_info.antPairId1              : %d", pCccRangingData->antenna_pair_info.antPairId1);
        NXPLOG_APP_D(
            "CCC.antenna_pair_info.antPairId2              : %d", pCccRangingData->antenna_pair_info.antPairId2);

        NXPLOG_APP_D("CCC.noOfPdoaMeasures                          : %d", pCccRangingData->noOfPdoaMeasures);
        for (size_t i = 0; i < pCccRangingData->noOfPdoaMeasures; i++) {
            NXPLOG_APP_D("CCC.pdoaMeasurements[%d].pdoa                     : %d.%d",
                i,
                TO_Q_9_7(pCccRangingData->pdoaMeasurements[i].pdoa));
            NXPLOG_APP_D("CCC.pdoaMeasurements[%d].pdoaIndex                : %d",
                i,
                pCccRangingData->pdoaMeasurements[i].pdoaIndex);
        }
        NXPLOG_APP_D("CCC.noOfRssiMeasurements                      : %d", pCccRangingData->noOfRssiMeasurements);
        for (size_t i = 0; i < pCccRangingData->noOfRssiMeasurements; i++) {
            NXPLOG_APP_D("CCC.rssiMeasurements[%d].rssi_rx1              : %d.%d",
                i,
                TO_Q_8_8(pCccRangingData->rssiMeasurements[i].rssi_rx1));

            NXPLOG_APP_D("CCC.rssiMeasurements[%d].rssi_rx2              : %d.%d",
                i,
                TO_Q_8_8(pCccRangingData->rssiMeasurements[i].rssi_rx2));
        }
        NXPLOG_APP_D("CCC.snrMeasurements                        : %d", pCccRangingData->noOfSnrMeasurements);
        for (size_t i = 0; i < pCccRangingData->noOfSnrMeasurements; i++) {
            NXPLOG_APP_D("CCC.snrMeasurements[%d].slotIndexAndAntennaMap    : %hu",
                i,
                pCccRangingData->snrMeasurements[i].slotIndexAndAntennaMap);
            NXPLOG_APP_D("CCC.snrMeasurements[%d].snrMainPath               : %hu",
                i,
                pCccRangingData->snrMeasurements[i].snrMainPath);
            NXPLOG_APP_D("CCC.snrMeasurements[%d].snrFirstPath              : %hu",
                i,
                pCccRangingData->snrMeasurements[i].snrFirstPath);
            NXPLOG_APP_D("CCC.snrMeasurements[%d].snrTotal                  : %d.%d",
                i,
                TO_Q_8_8(pCccRangingData->snrMeasurements[i].snrTotal));
        }
    }
}

#endif // UWBFTR_CCC && !(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)

#if UWBFTR_CSA

EXTERNC void printCsaRangingData(const phCsaRangingData_t *pCsaRangingData)
{
    if (pCsaRangingData != NULL) {
        NXPLOG_APP_D("------------------Received Ccc Range Data------------------");
        NXPLOG_APP_D("CSA.sessionHandle                             : 0x%x ", pCsaRangingData->sessionHandle);
        NXPLOG_APP_D("CSA.rangingStatus                             : 0x%.2X", pCsaRangingData->rangingStatus);
        NXPLOG_APP_D("CSA.stsIndex                                  : %" PRIu32 "", pCsaRangingData->stsIndex);
        NXPLOG_APP_D("CSA.rangingRoundIndex                         : %" PRIu16 "", pCsaRangingData->rangingRoundIndex);
        NXPLOG_APP_D("CSA.blockIndex                                : %" PRIu16 "", pCsaRangingData->blockIndex);
        NXPLOG_APP_I("CSA.distance                                  : %" PRIu16 "", pCsaRangingData->distance);
        NXPLOG_APP_D("CSA.uncertanityAnchorFom                      : %hu", pCsaRangingData->uncertanityAnchorFom);
        NXPLOG_APP_D("CSA.uncertanityInitiatorFom                   : %hu", pCsaRangingData->uncertanityInitiatorFom);
        LOG_MAU8_D("CSA.ccmTag                                    ", pCsaRangingData->ccmTag, MAX_CCM_TAG_SIZE);
        NXPLOG_APP_D("CSA.aoa_azimuth                               : %d.%d", TO_Q_9_7(pCsaRangingData->aoa_azimuth));
        NXPLOG_APP_D("CSA.aoa_azimuth_fom                           : %d", pCsaRangingData->aoa_azimuth_FOM);
        NXPLOG_APP_D("CSA.aoa_elevation                             : %d.%d", TO_Q_9_7(pCsaRangingData->aoa_elevation));
        NXPLOG_APP_D("CSA.aoa_elevation_fom                         : %d", pCsaRangingData->aoa_elevation_FOM);

        NXPLOG_APP_D(
            "CSA.antenna_pair_info.configMode              : %d", pCsaRangingData->antenna_pair_info.configMode);
        NXPLOG_APP_D(
            "CSA.antenna_pair_info.antPairId1              : %d", pCsaRangingData->antenna_pair_info.antPairId1);
        NXPLOG_APP_D(
            "CSA.antenna_pair_info.antPairId2              : %d", pCsaRangingData->antenna_pair_info.antPairId2);

        NXPLOG_APP_D("CSA.noOfPdoaMeasures                          : %d", pCsaRangingData->noOfPdoaMeasures);
        for (size_t i = 0; i < pCsaRangingData->noOfPdoaMeasures; i++) {
            NXPLOG_APP_D("CSA.pdoaMeasurements[%d].pdoa                     : %d.%d",
                i,
                TO_Q_9_7(pCsaRangingData->pdoaMeasurements[i].pdoa));
            NXPLOG_APP_D("CSA.pdoaMeasurements[%d].pdoaIndex                : %d",
                i,
                pCsaRangingData->pdoaMeasurements[i].pdoaIndex);
        }
        NXPLOG_APP_D("CSA.noOfRssiMeasurements                      : %d", pCsaRangingData->noOfRssiMeasurements);
        for (size_t i = 0; i < pCsaRangingData->noOfRssiMeasurements; i++) {
            NXPLOG_APP_D("CSA.rssiMeasurements[%d].rssi_rx1              : %d.%d",
                i,
                TO_Q_8_8(pCsaRangingData->rssiMeasurements[i].rssi_rx1));

            NXPLOG_APP_D("CSA.rssiMeasurements[%d].rssi_rx2              : %d.%d",
                i,
                TO_Q_8_8(pCsaRangingData->rssiMeasurements[i].rssi_rx2));
#if UWBIOT_UWBD_SR250
            NXPLOG_APP_D("CSA.rssiMeasurements[%d].rssi_rx3              : %d.%d",
                i,
                TO_Q_8_8(pCsaRangingData->rssiMeasurements[i].rssi_rx3));
#endif //UWBIOT_UWBD_SR250
        }
        NXPLOG_APP_D("CSA.snrMeasurements                        : %d", pCsaRangingData->noOfSnrMeasurements);
        for (size_t i = 0; i < pCsaRangingData->noOfSnrMeasurements; i++) {
            NXPLOG_APP_D("CSA.snrMeasurements[%d].slotIndexAndAntennaMap    : %hu",
                i,
                pCsaRangingData->snrMeasurements[i].slotIndexAndAntennaMap);
            NXPLOG_APP_D("CSA.snrMeasurements[%d].snrMainPath               : %hu",
                i,
                pCsaRangingData->snrMeasurements[i].snrMainPath);
            NXPLOG_APP_D("CSA.snrMeasurements[%d].snrFirstPath              : %hu",
                i,
                pCsaRangingData->snrMeasurements[i].snrFirstPath);
            NXPLOG_APP_D("CSA.snrMeasurements[%d].snrTotal                  : %d.%d",
                i,
                TO_Q_8_8(pCsaRangingData->snrMeasurements[i].snrTotal));
        }
    }
}

#endif // UWBFTR_CSA
#if !(UWBIOT_UWBD_SR04X)
EXTERNC void printDataTxPhaseCfgNtf(const phDataTxPhaseCfgNtf_t *pDataTxPhCfgNtf)
{
    if (pDataTxPhCfgNtf != NULL) {
        NXPLOG_APP_I("pDataTxPhCfgNtf->sessionHandle                : 0x%x\n", pDataTxPhCfgNtf->sessionHandle);
        NXPLOG_APP_I("pDataTxPhCfgNtf->status                       : 0x%x\n", pDataTxPhCfgNtf->status);
    }
    else {
        NXPLOG_APP_E("pRcvRadaNtfPkt is NULL");
    }
}
#endif //! UWBIOT_UWBD_SR04X
#if UWBFTR_DataTransfer
EXTERNC void printLogicalLinkCreateNtf(const phLogicalLinkCreateNtf_t *pLLCreateNtf)
{
    if (pLLCreateNtf != NULL) {
        NXPLOG_APP_D("pLLCreateNtf->ll_connect_id                       : %x\n", pLLCreateNtf->ll_connect_id);
        NXPLOG_APP_D("pLLCreateNtf->status                              : %x\n", pLLCreateNtf->status);
    }
    else {
        NXPLOG_APP_E("pLLCreateNtf is NULL");
    }
}

EXTERNC void printLogicalLinkUwbsCreateNtf(const phLogicalLinkUwbsCreateNtf_t *pLLUwbsCreateNtf)
{
    if (pLLUwbsCreateNtf != NULL) {
        NXPLOG_APP_D(
            "pLLUwbsCreateNtf->sessionHandle                           : %x\n", pLLUwbsCreateNtf->sessionHandle);
        NXPLOG_APP_D(
            "pLLUwbsCreateNtf->ll_connect_id                           : %x\n", pLLUwbsCreateNtf->ll_connect_id);
        NXPLOG_APP_D(
            "pLLUwbsCreateNtf->llm_selector                            : %x\n", pLLUwbsCreateNtf->llm_selector);
        LOG_MAU8_D("pLLUwbsCreateNtf->src_address                             : ",
            pLLUwbsCreateNtf->src_address,
            EXT_MAC_ADDR_LEN);
    }
    else {
        NXPLOG_APP_E("pLLUwbsCreateNtf is NULL");
    }
}

EXTERNC void printLogicalLinkUwbsCloseNtf(phLogicalLinkUwbsCloseNtf_t *pLLCloseNtf)
{
    if (pLLCloseNtf != NULL) {
        NXPLOG_APP_D("pLLCloseNtf->ll_connect_id                        : %x\n", pLLCloseNtf->ll_connect_id);
        NXPLOG_APP_D("pLLCloseNtf->status                               : %x\n", pLLCloseNtf->status);
    }
    else {
        NXPLOG_APP_E("pLLCloseNtf is NULL");
    }
}

EXTERNC void printRcvLogicalDataStatus(const phLogicalLinkDataPkt_t *pRcvDataPkt)
{
    if (pRcvDataPkt != NULL) {
        NXPLOG_APP_D("pRcvDataPkt->llConnectId              : 0x%X", pRcvDataPkt->llConnectId);
        NXPLOG_APP_D("pRcvDataPkt->sequence_number          : %d", pRcvDataPkt->sequence_number);
        NXPLOG_APP_I("pRcvDataPkt->data_size                : %d", pRcvDataPkt->data_size);
        LOG_MAU8_D("DataRcv                                 : ", pRcvDataPkt->data, pRcvDataPkt->data_size);
    }
    else {
        NXPLOG_APP_E("pRcvDataPkt is NULL");
    }
}

EXTERNC void printGetLogicalLinkParams(const phLogicalLinkGetParamsRsp_t *pLogicalLinkGetParamsRsp)
{
    if (pLogicalLinkGetParamsRsp != NULL) {
        NXPLOG_APP_I("pLogicalLinkGetParamsRsp->status                : 0x%X", pLogicalLinkGetParamsRsp->status);
        NXPLOG_APP_I("pLogicalLinkGetParamsRsp->control_field         : 0x%X", pLogicalLinkGetParamsRsp->control_field);
        NXPLOG_APP_I("pLogicalLinkGetParamsRsp->ll_sdu_size           : 0x%X", pLogicalLinkGetParamsRsp->ll_sdu_size);
        NXPLOG_APP_I("pLogicalLinkGetParamsRsp->ll_pdu_size           : 0x%X", pLogicalLinkGetParamsRsp->ll_pdu_size);
        NXPLOG_APP_I(
            "pLogicalLinkGetParamsRsp->Tx_window_size        : 0x%X", pLogicalLinkGetParamsRsp->Tx_window_size);
        NXPLOG_APP_I(
            "pLogicalLinkGetParamsRsp->Rx_window_size        : 0x%X", pLogicalLinkGetParamsRsp->Rx_window_size);
        NXPLOG_APP_I(
            "pLogicalLinkGetParamsRsp->repetition_count      : 0x%X", pLogicalLinkGetParamsRsp->repetition_count);
        NXPLOG_APP_I("pLogicalLinkGetParamsRsp->link_to               : 0x%X", pLogicalLinkGetParamsRsp->link_to);
        NXPLOG_APP_I("pLogicalLinkGetParamsRsp->port                  : 0x%X", pLogicalLinkGetParamsRsp->port);
    }
    else {
        NXPLOG_APP_E("pLogicalLinkGetParamsRsp is NULL");
    }
}
#endif // UWBFTR_DataTransfer
#if !(UWBIOT_UWBD_SR04X)
EXTERNC void printNewRoleChangeRcvNtf(const phNewRoleChangeNtf_t *pNewRole)
{
    if (pNewRole != NULL) {
        NXPLOG_APP_D("pNewRole->sessionHandle              : 0x%X", pNewRole->sessionHandle);
        NXPLOG_APP_D("pNewRole->new_role          : %d", pNewRole->new_role);
    }
    else {
        NXPLOG_APP_E("pNewRole is NULL");
    }
}
#endif // #if !(UWBIOT_UWBD_SR04X)
