/*
 * Copyright (C) 2019-2024,2026 NXP Semiconductors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "phNxpUciHal.h"
#include "UwbCoreSDK_Internal.h"
#include "phNxpUciHal_Adaptation.h"
#include "phNxpUciHal_ext.h"
#include "phTmlUwb.h"
#include "phTmlUwb_transport.h"
#include "phUwbTypes.h"
#include "phNxpLogApis_HalUci.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "uwb_types.h"
#include "phOsalUwb.h"
#include "phNxpUwbConfig.h"
#include <phTmlUwb_transport.h>

#if !(UWBIOT_UWBD_SR04X)
#include <UwbApi_Types_Proprietary.h>
#endif //!(UWBIOT_UWBD_SR04X)

#include "phUwbStatus.h"

#define UCI_CREDIT_NTF_STATUS_OFFSET 0x08

/**  Device State - IDLE */
#define UWB_UCI_DEVICE_INIT 0x00
/**  Device State - READY */
#define UWB_UCI_DEVICE_READY 0x01
/** Device State - ERROR */
#define UWB_UCI_DEVICE_ERROR 0xFF

#define NORMAL_MODE_LENGTH_OFFSET       0x03
#define EXTENDED_MODE_LEN_OFFSET        0x02
#define EXTENDED_MODE_LEN_SHIFT         0x08
#define EXTND_LEN_INDICATOR_OFFSET      0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80

bool uwb_device_initialized = FALSE;

#define SCALING_FACTOR(X) 200 + X / 4

/** Global Variables */
/* UCI HAL Control structure */
phNxpUciHal_Control_t nxpucihal_ctrl;

uint32_t uwbTimeoutTimerId = PH_OSALUWB_TIMER_ID_INVALID;

static uint8_t Rx_data[UCI_MAX_DATA_LEN];
/* Static function declarations */

static void phNxpUciHal_open_complete(UWBSTATUS status);
static void phNxpUciHal_read_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo);
static void phNxpUciHal_close_complete(UWBSTATUS status);
#if UWBFTR_UWBS_DEBUG_Dump
static void phNxpUciHal_dump_log(uint8_t gid, uint8_t oid);
#endif // UWBFTR_UWBS_DEBUG_Dump
extern int phNxpUciHal_fw_download(void);
static tHAL_UWB_STATUS phNxpUciHal_uwb_reset(void);

int phNxpUciHal_open(uwb_stack_callback_t *p_cback, uwb_stack_data_callback_t *p_data_cback)
{
    phTmlUwb_Config_t tTmlConfig;
    UWBSTATUS wConfigStatus = UWBSTATUS_SUCCESS;

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_OPEN) {
        NXPLOG_UCIHAL_E("phNxpUciHal_open already open");
        return UWBSTATUS_SUCCESS;
    }

    /*Create the timer for extns write response*/
    uwbTimeoutTimerId = phOsalUwb_Timer_Create(FALSE);
    if (uwbTimeoutTimerId == PH_OSALUWB_TIMER_ID_INVALID) {
        NXPLOG_UCIHAL_E("%s : Timer Create failed with timer ID %d", __FUNCTION__, uwbTimeoutTimerId);
        goto exit;
    }

    if (phNxpUciHal_init_monitor() == NULL) {
        NXPLOG_UCIHAL_E("Init monitor failed");
        goto exit;
    }

    phOsalUwb_SetMemory(&nxpucihal_ctrl, 0x00, sizeof(nxpucihal_ctrl));
    phOsalUwb_SetMemory(&tTmlConfig, 0x00, sizeof(tTmlConfig));

    /* By default HAL status is HAL_STATUS_OPEN */
    nxpucihal_ctrl.halStatus = HAL_STATUS_OPEN;

    nxpucihal_ctrl.p_uwb_stack_cback      = p_cback;
    nxpucihal_ctrl.p_uwb_stack_data_cback = p_data_cback;

    nxpucihal_ctrl.IsDev_suspend_enabled  = FALSE;
    nxpucihal_ctrl.IsFwDebugDump_enabled  = FALSE;
    nxpucihal_ctrl.IsCIRDebugDump_enabled = FALSE;
    nxpucihal_ctrl.fw_dwnld_mode          = FALSE;

    /* Initialize TML layer */
    wConfigStatus = phTmlUwb_Init(&tTmlConfig);
    if (wConfigStatus != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("phTmlUwb_Init Failed %d", wConfigStatus);
        goto clean_and_return;
    }

    /* Call open complete */
    phNxpUciHal_open_complete(wConfigStatus);
    return wConfigStatus;

clean_and_return:

    if(phOsalUwb_Timer_Delete(uwbTimeoutTimerId) != UWBSTATUS_SUCCESS){
        NXPLOG_UCIHAL_E("%s : Timer Delete Failed with timer ID %d", __FUNCTION__, uwbTimeoutTimerId);
    }
    /* Report error status */
    (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_OPEN_CPLT_EVT, HAL_UWB_ERROR_EVT);

    nxpucihal_ctrl.p_uwb_stack_cback      = NULL;
    nxpucihal_ctrl.p_uwb_stack_data_cback = NULL;
    phNxpUciHal_cleanup_monitor();
    nxpucihal_ctrl.halStatus = HAL_STATUS_CLOSE;
exit:
    return UWBSTATUS_FAILED;
}

/**
 * Function         phNxpUciHal_open_complete
 *
 * Description      This function inform the status of phNxpUciHal_open
 *                  function to libuwb-uci.
 *
 * Returns          void.
 *
 */
static void phNxpUciHal_open_complete(UWBSTATUS status)
{
    static phLibUwb_Message_t msg;

    if (status == UWBSTATUS_SUCCESS) {
        msg.eMsgType             = UCI_HAL_OPEN_CPLT_MSG;
        nxpucihal_ctrl.halStatus = HAL_STATUS_OPEN;
    }
    else {
        msg.eMsgType = UCI_HAL_ERROR_MSG;
    }

    msg.pMsgData = NULL;
    msg.Size     = 0;

    // Call to integration thread
    if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
        /* Send the event */
        if (msg.eMsgType == UCI_HAL_ERROR_MSG) {
            (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_ERROR_EVT, HAL_UWB_ERROR_EVT);
        }
        else {
            (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_OPEN_CPLT_EVT, HAL_UWB_STATUS_OK);
        }
    }

    return;
}

void phNxpUciHal_register_appdata_callback(phHalAppDataCb *appDataCb)
{
    if (NULL != appDataCb) {
        gpphTmlUwb_Context->appDataCallback = (pphTmlUwb_AppDataCb_t)appDataCb;
    }
}

int phNxpUciHal_write(uint16_t data_len, const uint8_t *p_data)
{
    uint16_t len;

    if (nxpucihal_ctrl.halStatus != HAL_STATUS_OPEN) {
        return UWBSTATUS_FAILED;
    }

    len = phNxpUciHal_write_unlocked(data_len, p_data);

    /* No data written */
    return len;
}

uint16_t phNxpUciHal_write_unlocked(uint16_t data_len, const uint8_t *p_data)
{
    UWBSTATUS status;

    /* Create local copy of cmd_data */
    if (data_len <= UCI_MAX_CMD_BUF_LEN) {
        phOsalUwb_MemCopy(&nxpucihal_ctrl.p_cmd_data[UCI_CMD_INDEX], p_data, data_len);
        nxpucihal_ctrl.cmd_len = data_len;

        data_len = nxpucihal_ctrl.cmd_len;
        status   = phTmlUwb_Write((uint8_t *)nxpucihal_ctrl.p_cmd_data, (uint16_t)nxpucihal_ctrl.cmd_len);
        if (UWBSTATUS_SUCCESS == status) {
            NXPLOG_UCIHAL_D("%s phTmlUwb_Write success", __FUNCTION__);
        }
        else {
            NXPLOG_UCIHAL_W("%s phTmlUwb_Write Failed", __FUNCTION__);
            data_len = 0;
        }
    }
    else {
        NXPLOG_UCIHAL_E("write_unlocked buffer overflow");
        data_len = 0;
    }

    return data_len;
}

#if UWBFTR_UWBS_DEBUG_Dump
/**
 * Function         phNxpUciHal_dump_log
 *
 * Description      This function is called whenever there is an debug logs
 *                  needs to be collected
 *
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_dump_log(uint8_t gid, uint8_t oid)
{
    if ((gid == UCI_GID_PROPRIETARY_CUSTOM_1) && (oid == EXT_UCI_MSG_DBG_DATA_LOGGER_NTF)) {
        NXPLOG_UCIHAL_D("debug data logger ntf samples received");
        nxpucihal_ctrl.isSkipPacket = 0;
    }
    else if ((gid == UCI_GID_PROPRIETARY_CUSTOM_2) && (oid == VENDOR_UCI_MSG_CIR_LOG_NTF)) {
        NXPLOG_UCIHAL_D("CIR samples received");
        nxpucihal_ctrl.isSkipPacket = 0;
    }
    else if ((gid == UCI_GID_PROPRIETARY_CUSTOM_1) && (oid == EXT_UCI_MSG_DBG_GET_ERROR_LOG)) {
        NXPLOG_UCIHAL_D(" error log received. ntf received");
        nxpucihal_ctrl.isSkipPacket = 1;
    }
    else {
        if ((nxpucihal_ctrl.IsCIRDebugDump_enabled) && (gid == UCI_GID_RANGE_MANAGE) &&
            (oid == UCI_MSG_SESSION_INFO_NTF)) {
            NXPLOG_UCIHAL_D(" session info ntf received");
        }
    }
    return;
}
#endif // (UWBFTR_UWBS_DEBUG_Dump)

/**
 * Function         phNxpUciHal_read_complete
 *
 * Description      This function is called whenever there is an UCI packet
 *                  received from UWBC. It could be RSP or NTF packet. This
 *                  function provide the received UCI packet to libuwb-uci
 *                  using data callback of libuwb-uci.
 *                  There is a pending read called from each
 *                  phNxpUciHal_read_complete so each a packet received from
 *                  UWBC can be provide to libuwb-uci.
 *
 * Returns          void.
 *
 */
static void phNxpUciHal_read_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo)
{
    UWBSTATUS status;
    uint8_t gid = 0, oid = 0, mt = 0;
    PHUWB_UNUSED(pContext);
    if (nxpucihal_ctrl.read_retry_cnt == 1) {
        nxpucihal_ctrl.read_retry_cnt = 0;
    }
    int32_t totalLength = pInfo->wLength;
    int32_t length      = 0;
    int32_t index       = 0;
    while (totalLength > index) {
        uint8_t extBitSet = (pInfo->pBuff[index + EXTND_LEN_INDICATOR_OFFSET] & EXTND_LEN_INDICATOR_OFFSET_MASK);
        length            = pInfo->pBuff[index + NORMAL_MODE_LENGTH_OFFSET];
        if (extBitSet || ((pInfo->pBuff[index] & UCI_MT_MASK) == 0x00)) {
            length = (length << EXTENDED_MODE_LEN_SHIFT) | pInfo->pBuff[index + EXTENDED_MODE_LEN_OFFSET];
        }
        if(length < (INT32_MAX - UCI_MSG_HDR_SIZE)) {
            length += UCI_MSG_HDR_SIZE;
        } else {
            NXPLOG_UCIHAL_E("Invalid length value. length=%d exceeds safe range", length);
            return;
        }
        NXPLOG_UCIHAL_D("read successful length = 0x%x", length);
        if (pInfo->wStatus == UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_D("read successful status = 0x%x", pInfo->wStatus);
            if((index + length) > totalLength){
                NXPLOG_UCIHAL_E("Length exceeds buffer size!!!");
                return;
            }
            nxpucihal_ctrl.p_rx_data   = &pInfo->pBuff[index];
            nxpucihal_ctrl.rx_data_len = length;
            if (nxpucihal_ctrl.operationMode == kOPERATION_MODE_mctt) {
                // Print complete log for mctt
                LOG_RX("RECV ", nxpucihal_ctrl.p_rx_data, nxpucihal_ctrl.rx_data_len);
            }
            else {
                if (nxpucihal_ctrl.rx_data_len > RX_LOG_MAX_NUMBER_OF_BYTES) {
                    LOG_RX("RECV ", nxpucihal_ctrl.p_rx_data, RX_LOG_MAX_NUMBER_OF_BYTES);
                }
                else {
                    LOG_RX("RECV ", nxpucihal_ctrl.p_rx_data, nxpucihal_ctrl.rx_data_len);
                }
            }

            mt                          = nxpucihal_ctrl.p_rx_data[0] & UCI_MT_MASK;
            gid                         = nxpucihal_ctrl.p_rx_data[0] & UCI_GID_MASK;
            oid                         = nxpucihal_ctrl.p_rx_data[1] & UCI_OID_MASK;
            nxpucihal_ctrl.isSkipPacket = 0;
#if UWBFTR_UWBS_DEBUG_Dump
            if (mt == UCI_MTS_NTF) {
                phNxpUciHal_dump_log(gid, oid);
            }
#endif // UWBFTR_UWBS_DEBUG_Dump
            if ((gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_DEVICE_STATUS_NTF)) {
                nxpucihal_ctrl.uwb_dev_status = nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET];
                if (!uwb_device_initialized) {
                    if (nxpucihal_ctrl.uwb_dev_status == UWB_UCI_DEVICE_INIT ||
                        nxpucihal_ctrl.uwb_dev_status == UWB_UCI_DEVICE_READY) {
                        nxpucihal_ctrl.isSkipPacket = 1;
                        (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.dev_status_ntf_wait.sem);
                    }
                }
            }

            if (gid == UCI_GID_CORE && oid == UCI_MSG_CORE_GENERIC_ERROR_NTF &&
                nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] == UCI_STATUS_MESSAGE_RETRY) {
                nxpucihal_ctrl.ext_cb_data.status = UCI_STATUS_MESSAGE_RETRY;
                nxpucihal_ctrl.isSkipPacket       = 0;
                (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.ext_cb_data.sem);
            }

            if ((gid == UCI_GID_RANGE_MANAGE) && (oid == UCI_MSG_DATA_CREDIT_NTF)) {
                nxpucihal_ctrl.ext_cb_data.status = nxpucihal_ctrl.p_rx_data[UCI_CREDIT_NTF_STATUS_OFFSET];
                (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.ext_cb_data.sem);
            }

            if (nxpucihal_ctrl.hal_ext_enabled == 1) {
                nxpucihal_ctrl.isSkipPacket = 1;
                if (mt == UCI_MT_RSP << UCI_MT_SHIFT) {
                    if (nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] == UWBSTATUS_SUCCESS) {
                        nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_SUCCESS;
                    }
                    else if ((gid == UCI_GID_PROPRIETARY_CUSTOM_1) && (oid == UCI_DBG_GET_ERROR_LOG_CMD)) {
                        nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_SUCCESS;
                    }
                    else {
                        nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_FAILED;
                        NXPLOG_UCIHAL_E(
                            "Response failed status = 0x%x", nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET]);
                    }
                    (void)phOsalUwb_ProduceSemaphore(nxpucihal_ctrl.ext_cb_data.sem);
                }
            }
            /* if Debug Notification, then skip sending to application */
            if (nxpucihal_ctrl.isSkipPacket == 0) {
                /* Read successful, send the event to higher layer */
                if ((nxpucihal_ctrl.p_uwb_stack_data_cback != NULL)
#if (UWBIOT_UWBD_SR04X)
                    && (nxpucihal_ctrl.rx_data_len <= UCI_MAX_PACKET_LEN)
#endif /* (UWBIOT_UWBD_SR04X) */
                ) {
                    (*nxpucihal_ctrl.p_uwb_stack_data_cback)(nxpucihal_ctrl.rx_data_len, nxpucihal_ctrl.p_rx_data);
                }
            }
        }
        else {
            NXPLOG_UCIHAL_E("read error status = 0x%x", pInfo->wStatus);
        }

        if (nxpucihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
            return;
        }
        /* Disable junk data check for each UCI packet*/
        if (nxpucihal_ctrl.fw_dwnld_mode) {
            if ((gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_DEVICE_STATUS_NTF)) {
                nxpucihal_ctrl.fw_dwnld_mode = FALSE;
            }
        }
        if(index < (INT32_MAX - length)) {
            index += length;
        } else {
            NXPLOG_UCIHAL_E("Invalid index value. index=%d exceeds safe range", index);
            return;
        }
    }
    /* Read again because read must be pending always.*/
    status =
        phTmlUwb_Read(Rx_data, UCI_MAX_DATA_LEN, (pphTmlUwb_TransactCompletionCb_t)&phNxpUciHal_read_complete, NULL);
    if (status != UWBSTATUS_PENDING) {
        NXPLOG_UCIHAL_E("read status error status = %x", status);
        /* TODO: Not sure how to handle this ? */
    }

    return;
}

int phNxpUciHal_close()
{
    UWBSTATUS status = UWBSTATUS_FAILED;

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
        NXPLOG_UCIHAL_E("phNxpUciHal_close is already closed, ignoring close");
        return UWBSTATUS_FAILED;
    }

    nxpucihal_ctrl.IsFwDebugDump_enabled  = FALSE;
    nxpucihal_ctrl.IsCIRDebugDump_enabled = FALSE;

    nxpucihal_ctrl.halStatus = HAL_STATUS_CLOSE;

    if (NULL != gpphTmlUwb_Context->pDevHandle) {
        phNxpUciHal_close_complete(UWBSTATUS_SUCCESS);
        /* Abort any pending read and write */
        phTmlUwb_ReadAbort();

        if (phOsalUwb_Timer_Stop(uwbTimeoutTimerId) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s : Timer Stop Failed with Timer ID %d", __FUNCTION__, uwbTimeoutTimerId);
        }

        if (phOsalUwb_Timer_Delete(uwbTimeoutTimerId) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s : Timer Delete Failed with Timer ID %d", __FUNCTION__, uwbTimeoutTimerId);
        }
        /* Resetting the timeout timer ID
         * Since 0 is also valid timer ID, resetting it to invalid value*/
        uwbTimeoutTimerId = PH_OSALUWB_TIMER_ID_INVALID;

        status = phTmlUwb_Shutdown();

        phOsalUwb_SetMemory(&nxpucihal_ctrl, 0x00, sizeof(nxpucihal_ctrl));

        NXPLOG_UCIHAL_D("%s : phOsalUwb_DeInit completed", __FUNCTION__);
    }

    phNxpUciHal_cleanup_monitor();

    /* Return success always */
    return status;
}
/**
 *  Function         phNxpUciHal_close_complete
 *
 * Description      This function inform libuwb-uci about result of
 *                  phNxpUciHal_close.
 *
 * Returns          void.
 *
 */
static void phNxpUciHal_close_complete(UWBSTATUS status)
{
    static phLibUwb_Message_t msg;

    if (status == UWBSTATUS_SUCCESS) {
        msg.eMsgType = UCI_HAL_CLOSE_CPLT_MSG;
    }
    else {
        msg.eMsgType = UCI_HAL_ERROR_MSG;
    }
    msg.pMsgData = NULL;
    msg.Size     = 0;

    // Call to integration thread
    if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
        /* Send the event */
        if (msg.eMsgType == UCI_HAL_ERROR_MSG) {
            (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_ERROR_EVT, HAL_UWB_ERROR_EVT);
        }
        else {
            (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_CLOSE_CPLT_EVT, HAL_UWB_STATUS_OK);
        }
    }

    return;
}

int phNxpUciHal_ioctl(long arg, tHAL_UWB_IOCTL *p_data)
{
    NXPLOG_UCIHAL_D("%s : enter - arg = %ld", __FUNCTION__, arg);

    int status = UWBSTATUS_FAILED;
    switch (arg) {
    case HAL_UWB_IOCTL_DUMP_FW_CRASH_LOG: {
#if !(UWBIOT_UWBD_SR04X)
        status = phNxpUciHal_dump_fw_crash_log();
        if (status == UWBSTATUS_SUCCESS) {
            if (p_data == NULL) {
                NXPLOG_UCIHAL_E("%s : p_data is NULL", __FUNCTION__);
                return UWBSTATUS_INVALID_PARAMETER;
            }
            phFwCrashLogInfo_t *fwLogInfo = (phFwCrashLogInfo_t *)p_data->pCrashInfo;
            if (fwLogInfo->logLen >= (size_t)(nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_LEN_OFFSET])) {
                fwLogInfo->logLen = nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_LEN_OFFSET];
                phOsalUwb_MemCopy(fwLogInfo->pLog,
                    &nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET],
                    (uint32_t)fwLogInfo->logLen);
                return UWBSTATUS_SUCCESS;
            }
            else {
                fwLogInfo->logLen = (size_t)nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_LEN_OFFSET] - 1;
                NXPLOG_UCIHAL_E("%s : Not Enough buffer to copy FW crash log required buffer size is %d",
                    __FUNCTION__,
                    fwLogInfo->logLen);
                return UWBSTATUS_INVALID_PARAMETER;
            }
        }
        NXPLOG_UCIHAL_E("%s : phNxpUciHal_dump_fw_crash_log failed", __FUNCTION__);
#endif //!(UWBIOT_UWBD_SR04X)
    } break;

    case HAL_UWB_IOCTL_SET_SUSPEND_STATE:
        nxpucihal_ctrl.IsDev_suspend_enabled = TRUE;
        break;
#if !(UWBIOT_UWBD_SR04X)
    case HAL_UWB_IOCTL_DUMP_FW_LOG: {
        if (p_data == NULL) {
            NXPLOG_UCIHAL_E("%s : p_data is NULL", __FUNCTION__);
            return UWBSTATUS_INVALID_PARAMETER;
        }
        InputOutputData_t *ioData            = (InputOutputData_t *)(p_data)->pIoData;
        nxpucihal_ctrl.IsFwDebugDump_enabled = ioData->enableFwDump;
        NXPLOG_UCIHAL_I("%s : Fw Dump is enabled status is %d", __FUNCTION__, ioData->enableFwDump);
        nxpucihal_ctrl.IsCIRDebugDump_enabled = ioData->enableCirDump;
        NXPLOG_UCIHAL_I("%s : Cir Dump is enabled status is %d", __FUNCTION__, ioData->enableCirDump);
        status = UWBSTATUS_SUCCESS;
    } break;
#endif //!(UWBIOT_UWBD_SR04X)
    default:
        NXPLOG_UCIHAL_E("%s : Wrong arg = %ld", __FUNCTION__, arg);
        break;
    }
    return status;
}

int phNxpUciHal_uwbDeviceInit(BOOLEAN recovery)
{
    int status;
    NXPLOG_UCIHAL_D(" Start FW download");
    nxpucihal_ctrl.fw_dwnld_mode  = TRUE; /* system in FW download mode*/
    nxpucihal_ctrl.uwb_dev_status = UWB_UCI_DEVICE_ERROR;
    uwb_device_initialized        = FALSE;
    /* Initiate semaphore */
    if (phOsalUwb_CreateSemaphore(&nxpucihal_ctrl.dev_status_ntf_wait.sem, 0) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("Semaphore creation failed");
        return UWBSTATUS_FAILED;
    }
    if (recovery == TRUE) {
        (void)phTmlUwb_reset(0);
        phTmlUwb_suspendReader();
#if (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
        phTmlUwb_set_mode_fwdld();
        phTmlUwb_reset_uwbs();
#endif // (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
    }

#if (UWBIOT_UWBD_SR04X)
    status = UWBSTATUS_SUCCESS;
#if UWBIOT_TML_SPI
    phTmlUwb_io_set(kUWBS_IO_O_RSTN, 0);
    phOsalUwb_Delay(10);
    phTmlUwb_io_set(kUWBS_IO_O_RSTN, 1);
#endif /* (UWBIOT_TML_SPI) */
#endif /* (UWBIOT_UWBD_SR04X) */

#if UWBIOT_UWBD_SR2XXT
    status = phNxpUciHal_fw_download();
#endif // UWBIOT_UWBD_SR2XXT

#if UWBIOT_UWBD_SR1XXT
    LOG_I("Starting FW download");
    status = phNxpUciHal_fw_download();
#if UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
    if (status != UWBSTATUS_SUCCESS) {
        /* Retry, just once more...
         * This failure is seen in PNP PC Windows mode, where if there was no clean
         * shut down, above call seems to fail, so sending again.
         */
        status = phNxpUciHal_fw_download();
    }
#endif // UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
#endif // UWBIOT_UWBD_SR1XXT
    if (status == UWBSTATUS_SUCCESS) {
#if UWBIOT_UWBD_SR1XXT
        LOG_I("FW Download done.");
#endif // UWBIOT_UWBD_SR1XXT

        if (recovery == TRUE) {
            phTmlUwb_resumeReader();
        }

        status = phTmlUwb_Read(
            Rx_data, UCI_MAX_DATA_LEN, (pphTmlUwb_TransactCompletionCb_t)&phNxpUciHal_read_complete, NULL);
        if (status != UWBSTATUS_PENDING) {
            NXPLOG_UCIHAL_E("read status error status = %x", status);
        }
        else {
            status = UWBSTATUS_SUCCESS; // Reader thread started successfully
        }
    }
#if UWBIOT_UWBD_SR1XXT
    else {
        NXPLOG_UCIHAL_E("FW download is failed: status= %x", status);
        status = UWBSTATUS_FAILED;
        goto clean_and_return;
    }
#endif // UWBIOT_UWBD_SR1XXT
    if (status == UWBSTATUS_SUCCESS) {
        // Wait for device init ntf
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(
                nxpucihal_ctrl.dev_status_ntf_wait.sem, HAL_MAX_DEVICE_ST_NTF_TIMEOUT) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s: Sem Timed out", __FUNCTION__);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#if UWBIOT_UWBD_SR1XXT
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_INIT) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#else
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_READY) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#endif /* UWBIOT_UWBD_SR1XXT */

#if !(UWBIOT_UWBD_SR04X)
        /* set board variant */
        status = phNxpUciHal_set_board_config();
        if (status != UWBSTATUS_OK) {
            NXPLOG_UCIHAL_E("%s: set board config is failed with status %d", __FUNCTION__, status);
            goto clean_and_return;
        }

        // wait for dev ready ntf
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(
                nxpucihal_ctrl.dev_status_ntf_wait.sem, HAL_MAX_DEVICE_ST_NTF_TIMEOUT) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s: Sem Timed out", __FUNCTION__);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_READY) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
#endif /*!(UWBIOT_UWBD_SR04X)*/
        // reset device
        status = phNxpUciHal_uwb_reset();
        if (status != UWBSTATUS_OK) {
            NXPLOG_UCIHAL_E("%s: Device reset Failed", __FUNCTION__);
            goto clean_and_return;
        }

        // wait for dev ready ntf
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(
                nxpucihal_ctrl.dev_status_ntf_wait.sem, HAL_MAX_DEVICE_ST_NTF_TIMEOUT) != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIHAL_E("%s: Sem Timed out", __FUNCTION__);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
        if (nxpucihal_ctrl.uwb_dev_status != UWB_UCI_DEVICE_READY) {
            NXPLOG_UCIHAL_E("%s: device status is failed %d", __FUNCTION__, nxpucihal_ctrl.uwb_dev_status);
            status = UWBSTATUS_FAILED;
            goto clean_and_return;
        }
        uwb_device_initialized = TRUE;
    }

clean_and_return:
    phOsalUwb_DeleteSemaphore(&nxpucihal_ctrl.dev_status_ntf_wait.sem);

    return status;
}
/**
 * Function         phNxpUciHal_uwb_reset
 *
 * Description      This function is called to reset uwb device
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the actual state of operation in arg pointer
 *
 */
static tHAL_UWB_STATUS phNxpUciHal_uwb_reset()
{
    tHAL_UWB_STATUS status;
    uint8_t buffer[] = {0x20, 0x00, 0x00, 0x01, 0x00};
    status           = (tHAL_UWB_STATUS)phNxpUciHal_send_ext_cmd(sizeof(buffer), buffer);
    if (status != UWBSTATUS_SUCCESS) {
        return status;
    }
    return UWBSTATUS_SUCCESS;
}

void phNxpUciHal_SetOperatingMode(Uwb_operation_mode_t state)
{
    nxpucihal_ctrl.operationMode = state;
}
