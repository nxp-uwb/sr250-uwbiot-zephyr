/*
 *
 * Copyright 2018-2020,2022-2024 NXP.
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
#include "phNxpLogApis_App.h"

EXTERNC void printDeviceInfo(const phUwbDevInfo_t *pdevInfo)
{
    if (pdevInfo != NULL) {
        NXPLOG_APP_D("UCI Generic Version            : %02X.%02X\n",
            pdevInfo->uciGenericMajor,
            pdevInfo->uciGenericMinorMaintenanceVersion);
        NXPLOG_APP_D("MAC Version                    : %02X.%02X\n",
            pdevInfo->macMajorVersion,
            pdevInfo->macMinorMaintenanceVersion);
        NXPLOG_APP_D("PHY Version                    : %02X.%02X\n",
            pdevInfo->phyMajorVersion,
            pdevInfo->phyMinorMaintenanceVersion);
        NXPLOG_APP_D("Device Name Length             : %d\n", pdevInfo->devNameLen);
        NXPLOG_APP_I("Device Name                    : %s\n", pdevInfo->devName);
        NXPLOG_APP_I(
            "Firmware Version               : %02X.%02X.%02X\n", pdevInfo->fwMajor, pdevInfo->fwMinor, pdevInfo->fwRc);
        NXPLOG_APP_D("Vendor UCI Version             : %02X.%02X.%02x\n",
            pdevInfo->vendorUciMajor,
            pdevInfo->vendorUciMinor,
            pdevInfo->vendorUciPatch);
        NXPLOG_APP_D("UWB Chip ID                    : %s\n", pdevInfo->uwbChipId);

        NXPLOG_APP_D(
            "Middleware Version             : %02X.%02X.%02X\n", pdevInfo->mwMajor, pdevInfo->mwMinor, pdevInfo->mwRc);

        NXPLOG_APP_D("Max PPM Value                  : %d\n", pdevInfo->maxPpmValue);

        NXPLOG_APP_D("TX Power Value                 : %d\n", pdevInfo->txPowerValue);

        NXPLOG_APP_D("FW Boot Mode                   : %d\n", pdevInfo->fwBootMode);

        NXPLOG_APP_D("Vendor UCI Test Version        : %02X.%02X.%02X\n",
            pdevInfo->uciTestMajor,
            pdevInfo->uciTestMinor,
            pdevInfo->uciTestPatch);

#if (UWBFTR_CCC)
        NXPLOG_APP_D("UCI CCC Version                    : %02x.%02x\n",
            pdevInfo->uciCccVersion[0],
            pdevInfo->uciCccVersion[1]);

        NXPLOG_APP_D("CCC Version                        : %s \n", pdevInfo->cccVersion);
#endif // (UWBFTR_CCC)
#if UWBFTR_CSA
        NXPLOG_APP_D("Aliro Spec Version                 : %s \n", pdevInfo->aliroSpecVersion);
#endif // UWBFTR_CSA

#if UWBIOT_UWBD_SR2XXT
        NXPLOG_APP_D("lifecycle                      : %x\n", pdevInfo->lifecycle);

        NXPLOG_APP_D("fwGitHash                      : %s\n", pdevInfo->fwGitHash);

#endif // UWBIOT_UWBD_SR2XXT
    }
}

#if (UWBIOT_SESN_SNXXX)
EXTERNC void printDoBindStatus(const phSeDoBindStatus_t *pDoBindStatus)
{
    if (pDoBindStatus != NULL) {
        NXPLOG_APP_D("pDoBindStatus->status                : 0x%0x \n", pDoBindStatus->status);
        NXPLOG_APP_D("pDoBindStatus->count_remaining       : 0x%0x \n", pDoBindStatus->count_remaining);
        NXPLOG_APP_D("pDoBindStatus->binding_state         : 0x%0x \n", pDoBindStatus->binding_state);
        NXPLOG_APP_D("pDoBindStatus->se_instruction_code   : 0x%0x \n", pDoBindStatus->se_instruction_code);
        NXPLOG_APP_D("pDoBindStatus->se_error_status       : 0x%0x \n", pDoBindStatus->se_error_status);
    }
    else {
        NXPLOG_APP_E("pDoBindStatus is NULL");
    }
}

EXTERNC void printGetBindingStatus(const phSeGetBindingStatus_t *pGetBindingStatus)
{
    if (pGetBindingStatus != NULL) {
        NXPLOG_APP_D("pGetBindingStatus->status                 : 0x%0x \n", pGetBindingStatus->status);
        NXPLOG_APP_D("pGetBindingStatus->se_binding_count       : 0x%0x \n", pGetBindingStatus->se_binding_count);
        NXPLOG_APP_D("pGetBindingStatus->uwbd_binding_count     : 0x%0x \n", pGetBindingStatus->uwbd_binding_count);
        NXPLOG_APP_D("pGetBindingStatus->se_instruction_code    : 0x%0x \n", pGetBindingStatus->se_instruction_code);
        NXPLOG_APP_D("pGetBindingStatus->se_error_status        : 0x%0x \n", pGetBindingStatus->se_error_status);
    }
    else {
        NXPLOG_APP_E("printGetBindingStatus is NULL");
    }
}

EXTERNC void printGetEseTestConnectivityStatus(const SeConnectivityStatus_t *pGetSeConnectivityStatus)
{
    if (pGetSeConnectivityStatus != NULL) {
        NXPLOG_APP_D("pGetSeConnectivityStatus->status                 : 0x%0x \n", pGetSeConnectivityStatus->status);
        NXPLOG_APP_D("pGetSeConnectivityStatus->se_instruction_code    : 0x%0x \n",
            pGetSeConnectivityStatus->se_instruction_code);
        NXPLOG_APP_D(
            "pGetSeConnectivityStatus->se_error_status        : 0x%0x \n", pGetSeConnectivityStatus->se_error_status);
    }
    else {
        NXPLOG_APP_E("printGetEseTestConnectivityStatus is NULL");
    }
}

#endif //(UWBIOT_SESN_SNXXX)

EXTERNC void printDistance_Aoa(const phRangingData_t *pRangingData)
{
    if (pRangingData != NULL) {
        NXPLOG_APP_D("--------------Received Range Data--------------\n");
        LOG_D("pRangingData->sessionHandle                     : %" PRIu32 " \n", pRangingData->sessionHandle);
        for (uint8_t i = 0; i < pRangingData->no_of_measurements; i++) {
#if UWBFTR_TWR // support only for DSTWR
            NXPLOG_APP_D("pRangingData->range_meas[%" PRIu8 "].status          : %" PRIu8 " \n",
                i,
                pRangingData->ranging_meas.range_meas_twr[i].status);
            if (pRangingData->ranging_meas.range_meas_twr[i].status == UWBAPI_STATUS_OK) {
                NXPLOG_APP_D("pRangingData->range_meas[%" PRIu16 "].distance        : %" PRIu16 " \n",
                    i,
                    pRangingData->ranging_meas.range_meas_twr[i].distance);
                NXPLOG_APP_D("pRangingData->range_meas[%" PRIu16 "].aoaFirst             : %" PRIu16 " \n",
                    i,
                    pRangingData->ranging_meas.range_meas_twr[i].aoa_elevation_FOM);
            }
#endif // UWBFTR_TWR // support only for DSTWR
        }
    }
    else {
        NXPLOG_APP_D("pRangingData is NULL");
    }
}

EXTERNC void printDebugParams(uint8_t noOfParams, const UWB_DebugParams_List_t *DebugParams_List)
{
    if (DebugParams_List != NULL) {
        for (uint8_t LoopCnt = 0; LoopCnt < noOfParams; LoopCnt++) {
            switch (DebugParams_List[LoopCnt].param_id) {
#if UWBIOT_UWBD_SR2XXT
            case kUWB_DBG_CFG_DATA_LOGGER_NTF:{
                LOG_MAU8_D("DebugParams_List->kUWB_DBG_CFG_DATA_LOGGER_NTF",
                    DebugParams_List[LoopCnt].param_value.param.param_value,
                    DebugParams_List[LoopCnt].param_value.param.param_len);
            } break;
#endif
            case kUWB_DBG_CFG_TEST_CONTENTION_RANGING_FEATURE:
                NXPLOG_APP_D("DebugParams_List->kUWB_DBG_CFG_TEST_CONTENTION_RANGING_FEATURE: 0x%" PRIx16 " \n ",
                    DebugParams_List[LoopCnt].param_value);
                break;
            case kUWB_DBG_CFG_CIR_CAPTURE_WINDOW:
                NXPLOG_APP_D("DebugParams_List->kUWB_DBG_CFG_CIR_CAPTURE_WINDOW: 0x%" PRIx32 "\n",
                    DebugParams_List[LoopCnt].param_value);
                break;
            case kUWB_DBG_CFG_RANGING_TIMESTAMP_NTF:
                NXPLOG_APP_D("DebugParams_List->kUWB_DBG_CFG_RANGING_TIMESTAMP_NTF: 0x%" PRIx16 " \n",
                    DebugParams_List[LoopCnt].param_value);
                break;
            default:
                break;
            }
        }
    }
    else {
        NXPLOG_APP_D("DebugParams_List is NULL");
    }
}

#if UWBIOT_SESN_SNXXX

EXTERNC void printTestLoopNtfData(const phTestLoopData_t *pTestLoopData)
{
    if (pTestLoopData != NULL) {
        NXPLOG_APP_D("Status                        : %" PRIu8 " \n", pTestLoopData->status);
        NXPLOG_APP_D("Loop Count                    : %" PRIu8 " \n", pTestLoopData->loop_cnt);
        NXPLOG_APP_D("Loop Pass Count               : %" PRIu8 " \n", pTestLoopData->loop_pass_count);
    }
    else {
        NXPLOG_APP_D("pTestLoopData is NULL");
    }
}

#endif // UWBIOT_SESN_SNXXX

EXTERNC void printUwbWlanIndNtf(const UWB_Wlan_IndNtf_t *UWB_Wlan_IndNtf)
{
    if (UWB_Wlan_IndNtf != NULL) {
        NXPLOG_APP_D(
            "UWB_Wlan_IndNtf->.UWB_Wlan_IndNtf_status                : %x\n", UWB_Wlan_IndNtf->UWB_Wlan_IndNtf_status);
        NXPLOG_APP_D("UWB_Wlan_IndNtf_time_index            : %x\n", UWB_Wlan_IndNtf->UWB_Wlan_IndNtf_time_index);
        NXPLOG_APP_D(
            "UWB_Wlan_IndNtf_sessionHandle             : %x\n", UWB_Wlan_IndNtf->UWB_Wlan_IndNtf_sessionHandle);
    }
}

#if !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)
EXTERNC void printWlanUwbIndNtf(const Wlan_Uwb_IndNtf_t *pWlan_Uwb_IndNtf)
{
    if (pWlan_Uwb_IndNtf != NULL) {
        LOG_D("pWlan_Uwb_IndNtf->status     : %X", pWlan_Uwb_IndNtf->Wlan_Uwb_IndNtf_status);
        LOG_D("pWlan_Uwb_IndNtf->Wlan_Uwb_IndNtf_time_index : %X", pWlan_Uwb_IndNtf->Wlan_Uwb_IndNtf_time_index);
    }
}
#endif // !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)

