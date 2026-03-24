/*
 *
 * Copyright 2018-2020,2022-2024, 2026 NXP.
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

/**
 *
 *  This file contains function of the UCI unit to format and send UCI
 *  commands (for DH).
 *
 */

#include "uwb_target.h"
#include "UwbAdaptation.h"
#include "uci_defs.h"
#include "uci_test_defs.h"
#include "uci_hmsgs.h"
#include "uwb_int.h"
#include "uwa_sys.h"
#include "uwb_hal_int.h"
#include "phNxpLogApis_UciCore.h"
#include "uwa_dm_int.h"
#include "uci_ext_defs.h"

/** Local function declarations */
static uint16_t getGidOid(uint16_t eventId);

/**
**
** Function         getGidOid
**
** Description      To get the gid & oid based on the event
**
** Returns         gid & oid based on the event
**
*/
static uint16_t getGidOid(uint16_t eventId)
{
    uint16_t gidOid = kGidOid_InvalidGidOid;
    switch (eventId) {
    case UWA_DM_API_SESSION_INIT_EVT:
        gidOid = (uint16_t)kGidOid_SessionInit;
        break;
    case UWA_DM_API_CORE_SET_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_CoreSetConfig;
        break;
    case UWA_DM_API_CORE_GET_DEVICE_INFO_EVT:
        gidOid = (uint16_t)kGidOid_CoreGetDeviceInfo;
        break;
    case UWA_DM_API_CORE_GET_CAPS_INFO_EVT:
        gidOid = (uint16_t)kGidOid_CoreGetCapsInfo;
        break;
    case UWA_DM_API_CORE_DEVICE_RESET_EVT:
        gidOid = (uint16_t)kGidOid_CoreDeviceReset;
        break;
    case UWA_DM_API_CORE_GET_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_CoreGetConfig;
        break;
    case UWA_DM_API_SESSION_DEINIT_EVT:
        gidOid = (uint16_t)kGidOid_SessionDeinit;
        break;
    case UWA_DM_API_SESSION_SET_APP_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_SessionSetAppConfig;
        break;
    case UWA_DM_API_SESSION_GET_APP_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_SessionGetAppConfig;
        break;
    case UWA_DM_API_SESSION_STOP_EVT:
        gidOid = (uint16_t)kGidOid_SessionStop;
        break;
    case UWA_DM_API_SESSION_START_EVT:
        gidOid = (uint16_t)kGidOid_SessionStart;
        break;
    case UWA_DM_API_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST_EVT:
        gidOid = (uint16_t)kGidOid_SessionUpdateControllerMulticastList;
        break;
#if !(UWBIOT_UWBD_SR04X)
    case UWA_DM_API_TEST_SET_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_TestSetConfig;
        break;
    case UWA_DM_API_TEST_GET_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_TestGetConfig;
        break;
    case UWA_DM_API_TEST_PERIODIC_TX_EVT:
        gidOid = (uint16_t)kGidOid_TestPeriodicTx;
        break;
    case UWA_DM_API_TEST_PER_RX_EVT:
        gidOid = (uint16_t)kGidOid_TestPerRx;
        break;
    case UWA_DM_API_TEST_UWB_LOOPBACK_EVT:
        gidOid = (uint16_t)kGidOid_TestUwbLoopback;
        break;
    case UWA_DM_API_TEST_RX_EVT:
        gidOid = (uint16_t)kGidOid_TestRx;
        break;
    case UWA_DM_API_TEST_SR_RX_EVT:
        gidOid = (uint16_t)kGidOid_TestSrRx;
        break;
    case UWA_DM_API_TEST_STOP_SESSION_EVT:
        gidOid = (uint16_t)kGidOid_TestStopSession;
        break;
#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250
    case UWA_DM_API_WRITE_MODULE_MAKER_EVT:
        gidOid = (uint16_t)kGidOid_WriteModuleMaker;
        break;
    case UWA_DM_API_READ_MODULE_MAKER_EVT:
        gidOid = (uint16_t)kGidOid_ReadModuleMaker;
        break;
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250

#endif //!(UWBIOT_UWBD_SR04X)
    case UWA_DM_API_SESSION_GET_STATE_EVT:
        gidOid = (uint16_t)kGidOid_SessionGetState;
        break;
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)
    case UWA_DM_API_PROP_GENERATE_TAG:
        gidOid = (uint16_t)kGidOid_PropGenrateTag;
        break;
    case UWA_DM_API_PROP_CALIB_INTEGRITY_PROTECTION:
        gidOid = (uint16_t)kGidOid_PropCalibIntegrityProtection;
        break;
    case UWA_DM_API_PROP_VERIFY_CALIB_DATA:
        gidOid = (uint16_t)kGidOid_PropVerifyCalibData;
        break;
    case UWA_DM_API_PROP_DO_BIND:
        gidOid = (uint16_t)kGidOid_VendorDoBind;
        break;
    case UWA_DM_API_PROP_TEST_CONNECTIVITY:
        gidOid = (uint16_t)kGidOid_VendorTestConnectivity;
        break;
    case UWA_DM_API_PROP_TEST_SE_LOOP:
        gidOid = (uint16_t)kGidOid_PropTestSeLoop;
        break;
    case UWA_DM_API_PROP_GET_BINDING_STATUS:
        gidOid = (uint16_t)kGidOid_VendorGetBindingStatus;
        break;
#if (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S)
    case UWA_DM_API_PROP_CONFIG_AUTH_TAG_OPTIONS:
        gidOid = (uint16_t)kGidOid_PropConfigAuthTagOptions;
        break;
    case UWA_DM_API_PROP_CONFIG_AUTH_TAG_VERSIONS:
        gidOid = (uint16_t)kGidOid_PropConfigAuthTagVersions;
        break;
    case UWA_DM_API_PROP_URSK_DELETION_REQUEST:
        gidOid = (uint16_t)kGidOid_VendorUrskDeletionRequest;
        break;
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S)
#endif // (UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S || UWBIOT_UWBD_SR200S)

#if !(UWBIOT_UWBD_SR04X)
    case UWA_DM_API_VENDOR_DO_CHIP_CALIBRATION:
        gidOid = (uint16_t)kGidOid_VendorDoChipCalibration;
        break;
    case UWA_DM_API_PROP_GET_BINDING_COUNT:
        gidOid = (uint16_t)kGidOid_PropGetBindingCount;
        break;
    case UWA_DM_API_SESSION_UPDATE_DT_ANCHOR_RANGING_ROUNDS_EVT:
        gidOid = (uint16_t)kGidOid_SessionUpdateDtAnchorRangingRound;
        break;
    case UWA_DM_API_SESSION_UPDATE_DT_TAG_RANGING_ROUNDS_EVT:
        gidOid = (uint16_t)kGidOid_SessionUpdateDtTagRangingRound;
        break;
    case UWA_DM_SESSION_QUERY_DATA_SIZE_IN_RANGING_EVT:
        gidOid = (uint16_t)kGidOid_SessionQueryDataSizeInRanging;
        break;
    case UWA_DM_API_PROP_WRITE_OTP_CALIB_DATA:
        gidOid = (uint16_t)kGidOid_WriteOtpCalibData;
        break;
    case UWA_DM_API_PROP_READ_OTP_CALIB_DATA:
        gidOid = (uint16_t)kGidOid_ReadOtpCalibData;
        break;
    case UWA_DM_API_SESSION_SET_HUS_CONTROLLER_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_SessionSetHusControllerConfig;
        break;
    case UWA_DM_API_SESSION_SET_HUS_CONTROLEE_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_SessionSetHusControleeConfig;
        break;
    case UWA_DM_SESSION_DATA_TRANSFER_PHASE_CONFIG_EVT:
        gidOid = kGidOid_SessionDataTransferPhaseConfig;
        break;
    case UWA_DM_API_PROP_QUERY_TEMP:
        gidOid = (uint16_t)kGidOid_PropQueryTemp;
        break;
    case UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION:
        gidOid = (uint16_t)kGidOid_VendorSetDeviceCalibration;
        break;
    case UWA_DM_API_VENDOR_GET_DEVICE_CALIBRATION:
        gidOid = (uint16_t)kGidOid_VendorGetDeviceCalibration;
        break;
#if UWBIOT_UWBD_SR2XXT
    case UWA_DM_API_VENDOR_SET_SECURE_CALIBRATION:
        gidOid = (uint16_t)kGidOid_VendorSetSecureCalibration;
        break;
#endif // UWBIOT_UWBD_SR2XXT
    case UWA_DM_API_SESSION_SET_VENDOR_APP_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_VendorSetVendorAppConfig;
        break;
    case UWA_DM_API_SESSION_GET_VENDOR_APP_CONFIG_EVT:
        gidOid = (uint16_t)kGidOid_VendorGetVendorAppConfig;
        break;
#if UWBFTR_CSA
    case UWA_DM_API_SESSION_SET_LOCALIZATION_ZONE_EVT:
        gidOid = (uint16_t)kGidOid_SetLocZone;
        break;
#endif // UWBFTR_CSA
    case UWA_DM_API_GET_ESE_SESSION_ID_LIST_EVT:
        gidOid = (uint16_t)kGidOid_EseGetSessionIdList;
        break;
#endif //!(UWBIOT_UWBD_SR04X)
    case UWA_DM_API_CORE_QUERY_UWBS_TIMESTAMP:
        gidOid = (uint16_t)kGidOid_CoreQueryUwbsTimestamp;
        break;
    case UWA_DM_API_VENDOR_GET_ALL_UWB_SESSIONS:
        gidOid = (uint16_t)kGidOid_VendorGetAllUwbSessions;
        break;
    case UWA_DM_API_TRNG_EVENT:
        gidOid = (uint16_t)kGidOid_GetTrng;
        break;
#if (UWBIOT_UWBD_SR04X)
    case UWA_DM_API_SUSPEND_DEVICE_EVENT:
        gidOid = (uint16_t)kGidOid_GetSuspendDevice;
        break;
    case UWA_DM_API_SESSION_NVM_EVENT:
        gidOid = (uint16_t)kGidOid_GetSessionNvm;
        break;
    case UWA_DM_START_TEST_MODE_EVENT:
        gidOid = (uint16_t)kGidOid_GetStartTestMode;
        break;
    case UWA_DM_STOP_TEST_MODE_EVENT:
        gidOid = (uint16_t)kGidOid_GetStopTestMode;
        break;
    case UWA_DM_SET_CALIB_TRIM_EVENT:
        gidOid = (uint16_t)kGidOid_SetCalibTrimMode;
        break;
    case UWA_DM_GET_CALIB_TRIM_EVENT:
        gidOid = (uint16_t)kGidOid_GetCalibTrimMode;
        break;
    case UWA_BYPASS_CURRENT_LIMITER:
        gidOid = (uint16_t)kGidOid_BypassCurrentLimiter;
        break;
#endif
#if (UWBFTR_BlobParser)
    case UWA_DM_API_PROFILE_PARAM_EVENT:
        gidOid = (uint16_t)kGidOid_ProfileParamMode;
        break;
#endif // UWBFTR_BlobParser
#if UWBFTR_DataTransfer
    case UWA_DM_SESSION_LOGICAL_LINK_CREATE_EVT:
        gidOid = (uint16_t)kGidOid_SessionLlCreate;
        break;
    case UWA_DM_SESSION_LOGICAL_LINK_CLOSE_EVT:
        gidOid = (uint16_t)kGidOid_SessionLlClose;
        break;
    case UWA_DM_SESSION_LOGICAL_LINK_GET_PARAM_EVT:
        gidOid = (uint16_t)kGidOid_SessionLlGetParams;
        break;
#endif // UWBFTR_DataTransfer
    default:
        break;
    }
    return gidOid;
}

bool uci_snd_cmd_interface(uint16_t eventId, uint16_t length, uint8_t *data, uint8_t pbf)
{
    uint8_t status;

    status = uci_snd_cmd(eventId, length, data, pbf);
    if (status != UCI_STATUS_OK) {
        UCI_TRACE_E("uci_snd_cmd(): failed ,status=0x%X", status);
    }
    else {
        UCI_TRACE_D("uci_snd_cmd(): success ,status=0x%X", status);
    }
    return (TRUE);
}

uint8_t uci_snd_cmd(uint16_t eventId, uint16_t length, uint8_t *data, uint8_t pbf)
{
    UCI_TRACE_D(__FUNCTION__);
    UWB_HDR *p;
    uint8_t *pp;
    uint16_t gidOid     = kGidOid_InvalidGidOid;
    uint16_t payloadLen = length;
    uint16_t offset     = 0;
    uint32_t pp_index   = 0;

    do {
        // For Send Data, use pbf from api layer
#if (UWBIOT_UWBD_SR04X)
        if ((eventId != UWA_DM_API_SEND_DATA_EVENT) && (eventId != UWA_DM_START_TEST_MODE_EVENT)) {
#else
        if (eventId != (UWA_DM_API_SEND_DATA_EVENT) && eventId != (UWA_DM_API_LOGICAL_LINK_SEND_DATA_EVENT) &&
            (eventId != UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION)) {
#endif //UWBIOT_UWBD_SR04X
            if (length > UCI_MAX_PAYLOAD_SIZE) {
                payloadLen = UCI_MAX_PAYLOAD_SIZE;
                pbf        = 1;
            }
            else {
                payloadLen = length;
                pbf        = 0;
            }
        }

        if ((p = UCI_GET_CMD_BUF(payloadLen)) == NULL)
            return (UCI_STATUS_FAILED);

        p->event          = BT_EVT_TO_UWB_UCI;
        p->len            = (uint16_t)(UCI_MSG_HDR_SIZE + payloadLen);
        p->offset         = UCI_MSG_OFFSET_SIZE;
        p->layer_specific = 0;
        pp                = (uint8_t *)(p + 1) + p->offset;
        pp_index          = 0;

        if ((eventId == UWA_DM_API_SEND_DATA_EVENT) || (eventId == UWA_DM_API_LOGICAL_LINK_SEND_DATA_EVENT)) {
            if (eventId == UWA_DM_API_LOGICAL_LINK_SEND_DATA_EVENT) {
                // For Send Data, MT should be data type
                UCI_MSG_PBLD_HDR0(pp, UCI_MT_DATA, pbf, UCI_DPF_LL_SND, pp_index);
            }
            else {
                // For Send Data, MT should be data type
                UCI_MSG_PBLD_HDR0(pp, UCI_MT_DATA, pbf, UCI_DPF_SND, pp_index);
            }
            UCI_MSG_BLD_HDR1(pp, 0x00, pp_index); // there is no OID for send data
            UWB_UINT8_TO_STREAM(pp, payloadLen & 0xFF, pp_index);
            UWB_UINT8_TO_STREAM(pp, payloadLen >> 8, pp_index);
            UWB_ARRAY_TO_STREAM(pp, data, payloadLen, pp_index);
            uwb_ucif_send_cmd(p);
            break;
        }
#if !(UWBIOT_UWBD_SR04X)
        else if (eventId == UWA_DM_API_VENDOR_SET_DEVICE_CALIBRATION) {
            gidOid = getGidOid(eventId);
            UCI_MSG_PBLD_HDR0(pp, UCI_MT_CMD, pbf, (uint8_t)(gidOid >> 8), pp_index);
            if (length > UCI_MAX_PAYLOAD_SIZE) {
                UCI_MSG_BLD_HDR1_EXT(pp, (uint8_t)gidOid, pp_index);
                UWB_UINT8_TO_STREAM(pp, payloadLen & 0xFF, pp_index);
                UWB_UINT8_TO_STREAM(pp, payloadLen >> 8, pp_index);
            }
            else {
                UCI_MSG_BLD_HDR1(pp, (uint8_t)gidOid, pp_index)
                UWB_UINT8_TO_STREAM(pp, 0x00, pp_index);
                UWB_UINT8_TO_STREAM(pp, payloadLen, pp_index);
            };
            UWB_ARRAY_TO_STREAM(pp, data, payloadLen, pp_index);
            uwb_ucif_send_cmd(p);
            break;
        }
#endif // !(UWBIOT_UWBD_SR04X)
        else {
            gidOid = getGidOid(eventId);
            UCI_MSG_PBLD_HDR0(pp, UCI_MT_CMD, pbf, (uint8_t)(gidOid >> 8), pp_index);
            UCI_MSG_BLD_HDR1(pp, (uint8_t)gidOid, pp_index);
            UWB_UINT8_TO_STREAM(pp, 0x00, pp_index);
            UWB_UINT8_TO_STREAM(pp, payloadLen, pp_index);
            UWB_ARRAY_TO_STREAM(pp, (&data[offset]), payloadLen, pp_index);
        }

        if (offset < (UINT16_MAX - payloadLen)) {
            offset += payloadLen;
        } else {
            UCI_TRACE_E("Offset exceeded valid range. offset=%u, payloadLen=%u", offset, payloadLen);
            return UCI_STATUS_FAILED;
        }
        length -= payloadLen;

        uwb_ucif_send_cmd(p);
    } while (length != 0);

    return (UCI_STATUS_OK);
}

void uci_proc_raw_cmd_rsp(uint8_t *p_buf, uint16_t len)
{
    tUWB_RAW_CBACK *p_cback = uwb_cb.p_raw_cmd_cback;

    UCI_TRACE_D(" uci_proc_raw_cmd_rsp:"); // for debug

    /* If there's a pending/stored command, restore the associated address of the
     * callback function */
    if (p_cback == NULL) {
        UCI_TRACE_E("p_raw_cmd_cback is null");
    }
    else {
        /**
         *Invokes rawCommandResponse_Cb with Raw UCI Data in UCI.
         */
        (*p_cback)(0, 0 /*unused in this case*/, len, p_buf);
        uwb_cb.p_raw_cmd_cback = NULL;
    }
    uwb_cb.rawCmdCbflag = FALSE;
    uwb_ucif_update_cmd_window();
}

void uci_proc_proprietary_ntf(uint8_t gid, uint8_t op_code, uint8_t *p_buf, uint16_t len)
{
    if (len > 0) {
        UCI_TRACE_D(" uci_proc_raw_cmd_rsp:"); // for debug

        if (uwb_cb.p_ext_resp_cback == NULL) {
            UCI_TRACE_E("ext response callback is null");
        }
        else {
            switch (op_code) {
#if UWBIOT_UWBD_SR100T
                /* Perform Reset incase of SE Communication Failure*/

            case EXT_UCI_MSG_SE_COMM_ERROR_NTF: {
                uint8_t *status_ptr = p_buf + UCI_RESPONSE_STATUS_OFFSET;
                uint8_t status      = *status_ptr;
                if (status == UCI_STATUS_ESE_RECOVERY_FAILURE) {
                    tHAL_UWB_IOCTL ioCtl;
                    uwb_uci_IoctlInOutData_t inpOutData;
                    uint8_t stat;
                    ioCtl.pIoData = &inpOutData;
                    stat          = uwb_cb.p_hal->ioctl(HAL_UWB_IOCTL_ESE_RESET, &ioCtl);
                    if (stat == UCI_STATUS_OK) {
                        UCI_TRACE_D("%s: Set ESE RESET successful", __FUNCTION__);
                    }
                    else {
                        UCI_TRACE_E("%s: Set ESE RESET Failed", __FUNCTION__);
                    }
                }
            } break;
#endif // UWBIOT_UWBD_SR100T
            default:
                break;
            }

            /* Invokes extDeviceManagementCallBack with Raw UCI Data in JNI */
            (*uwb_cb.p_ext_resp_cback)(gid, (tUWB_RAW_EVT)(op_code), len, p_buf);
        }
    }
    else {
        UCI_TRACE_E("%s: len is zero", __FUNCTION__);
    }
}
