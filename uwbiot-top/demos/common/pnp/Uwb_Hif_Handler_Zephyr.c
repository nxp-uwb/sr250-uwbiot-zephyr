/*
 * Copyright 2025,2026 NXP
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

#include "phUwb_BuildConfig.h"

#if UWBIOT_OS_ZEPHYR

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)
#include "app_config.h"
#include "uwb_board.h"
#include "UWB_Spi_Driver_Interface.h"
#include "UwbPnpInternal.h"
#include "UwbApi_Utility.h"
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_ctrl.h>
#include "Uwb_Vcom_Pnp.h"
#include "UWB_Evt_Pnp.h"

#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"

#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
#include "UWB_Hbci.h"
#endif //  (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)

#if !defined ENABLED
#error ENABLED must be defined
#endif

extern bool mError;
volatile uint16_t mRcvTlvSize    = 0;
volatile uint16_t mRcvTlvSizeExp = 0;
static uint8_t mRcvTlvBuf[HIF_MAX_PKT_SIZE];
intptr_t mHifCommandQueue;
#define MAX_HIF_TASK_TLV_WAIT_TIMEOUT (1000)

bool Uwb_Is_Hif_Active()
{
    return mError;
}

void Uwb_Reset_Hif_State(bool state)
{
    mError = state;
}

/**
 * * UCI Payload:
 *   - Macro: `#define ACTUAL_PACKET_START (UCI_CMD_INDEX + N)`
 *   - Description: Refers to the section of the buffer where the actual UCI (UWB Command Interface)
 *                : data will be stored, starting after the platform-specific and bidirectional
 * bytes.
 *   - For any Uwbs all Bidirectional and Platform specific handling done on the Transport layer.
 *   - And actual Payload for the Read start from the "ACTUAL_PACKET_START"
 *
 */

uint32_t UWB_Hif_UciSendNtfn(uint8_t *pData, uint16_t size)
{
#if (UWBIOT_UWBD_SR04X)
    return UWB_Vcom_SendInternalRsp(pData, size);
#else
    return UWB_Vcom_UciSendNtfn(&pData[ACTUAL_PACKET_START], size);
#endif //
}

uint32_t UWB_Hif_SendUCIRsp(uint8_t *pData, uint16_t size)
{
#if (UWBIOT_UWBD_SR04X)
    return UWB_Vcom_SendInternalRsp(pData, size);
#else
    return UWB_Vcom_SendUCIRsp(&pData[ACTUAL_PACKET_START], size);
#endif //
}

uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size)
{
#if (UWBIOT_UWBD_SR04X)
    return UWB_Vcom_SendInternalRsp(pData, size);
#else
    return UWB_Vcom_SendRsp(&pData[ACTUAL_PACKET_START], size);
#endif //
}

uint32_t UWB_Hif_internal_SendRsp(uint8_t *pData, uint16_t size)
{
    return UWB_Vcom_SendInternalRsp(pData, size);
}

void Uwb_Hif_ReadDataCb(uint8_t *pData, uint32_t *pLen)
{
    while (*pLen > 0) {
        int32_t cpSize;
        if (mRcvTlvSize < 3) {
            cpSize = 1;
        }
        else if (mRcvTlvSize + *pLen <= mRcvTlvSizeExp) {
            cpSize = *pLen;
        }
        else {
            cpSize = (mRcvTlvSizeExp - mRcvTlvSize);
        }
        if (cpSize <= 0) {
            /* Error case */
            break;
        }
        if ((mRcvTlvSize + (uint32_t)cpSize) > sizeof(mRcvTlvBuf)) {
            /* Error case */
            break;
        }

        phOsalUwb_MemCopy(&mRcvTlvBuf[mRcvTlvSize], pData, cpSize);
        *pLen -= cpSize;
        pData += cpSize;
        mRcvTlvSize += (uint16_t)cpSize;

        if (mRcvTlvSize == 3) {
            /* TLV size received */
            mRcvTlvSizeExp = (uint16_t)(3 + (mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]));
        }
    }
    if (mRcvTlvSize && mRcvTlvSize == mRcvTlvSizeExp) {
        mRcvTlvSize = mRcvTlvSizeExp = 0;
        mError                       = FALSE;
        (void)phOsalUwb_ProduceSemaphore(mHifIsr_Sem);
    }
}

void UWB_Hif_Init()
{
    Uwb_Vcom_Init(&Uwb_Hif_ReadDataCb);
}

OSAL_TASK_RETURN_TYPE UWB_Hif_Handler_Task(void *args)
{
    phLibUwb_Message_t evt;
    tlv_t tlv;

    evt.eMsgType = USB_TLV_EVT;
    evt.pMsgData = &tlv;
    UWB_Hif_Init();

    while (1) {
        /* Wait to receive TLV over USB */
        if (!mError &&
            phOsalUwb_ConsumeSemaphore_WithTimeout(mHifIsr_Sem, MAX_HIF_TASK_TLV_WAIT_TIMEOUT) != UWBSTATUS_SUCCESS) {
            continue;
        }
        if (!mError) {
            tlv.type  = mRcvTlvBuf[0];
            tlv.size  = (uint16_t)(mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]);
            tlv.value = &mRcvTlvBuf[3];
            if (tlv.size > 3) {
                DEBUGOUT("USB 0x%X 0x%X 0x%X 0x%X\n", mRcvTlvBuf[0], mRcvTlvBuf[1], mRcvTlvBuf[2], mRcvTlvBuf[3]);
            }
            else {
                DEBUGOUT("USB 0x%X 0x%X 0x%X\n", mRcvTlvBuf[0], mRcvTlvBuf[1], mRcvTlvBuf[2]);
            }
            DEBUGOUT("[TLV %02x]\n", tlv.type);
            (void)phOsalUwb_msgsnd(mHifCommandQueue, &evt, MAX_DELAY);
        }
    }
}

OSAL_TASK_RETURN_TYPE UWB_WriterTask(void *args)
{
    phLibUwb_Message_t tlv = {0};
    while (1) {
        if (phOsalUwb_msgrcv(mHifWriteQueue, &tlv, MAX_DELAY) == UWBSTATUS_FAILED) {
            phOsalUwb_Thread_Context_Switch();
            continue;
        }
        (void)phOsalUwb_LockMutex(mHifWriteMutex);
        // uint8_t *Buffer = (uint8_t *)tlv.pMsgData;
        // DEBUGOUT("Sending ntf: 0x%X 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4]);
        UWB_Hif_UciSendNtfn(tlv.pMsgData, tlv.Size);
        DEBUGOUT("After Sending ntf\n");
        if ((uint8_t *)tlv.pMsgData != NULL) {
            phOsalUwb_FreeMemory((uint8_t *)tlv.pMsgData);
            tlv.pMsgData = NULL;
            /* DEBUGOUT("FREE\r\n"); */
        }
        (void)phOsalUwb_UnlockMutex(mHifWriteMutex);
        // phOsalUwb_Delay(2); /* Give some time for Reader Task to Read the Data.*/
    }
}

void UWB_HandleEvt(UWB_EvtType_t ev, void *args)
{
    switch (ev) {
    case USB_TLV_EVT:
#if UWBIOT_UWBD_SR1XXT
        UWB_Handle_SR1XXT_TLV((tlv_t *)args);
#elif UWBIOT_UWBD_SR2XXT
        UWB_Handle_SR2XXT_TLV((tlv_t *)args);
#elif UWBIOT_UWBD_SR040
        UWB_Handle_SR040_TLV((tlv_t *)args);
#endif
        break;
    default:
        DEBUGOUT("ERROR: Unknown event type %02x\n", ev);
        break;
    }
}

OSAL_TASK_RETURN_TYPE UWB_Pnp_App_Task(void *args)
{
    /* This may go somehwere else.. */
    phLibUwb_Message_t evt = {0};

    DEBUGOUT("UWB_HeliosTask(): suspending communication interfaces\n");
    /*Suspend USB task which gets USB messages from the Host. During HBCI internal download Rhodes can not accept
     * any commands from Host*/
    phOsalUwb_TaskSuspend(mHifTask);
    /*Suspend ALL tasks which are used for UCI operations. During HBCI mode UCI tasks need to be disabled.*/
    UCI_ReaderTaskSuspend();
    phOsalUwb_TaskSuspend(mHifWriterTask);

#if (UWBFTR_SE_SE051W || UWBFTR_SE_SN110)
    Se_Comm_Init();
#endif
#if UWBIOT_UWBD_SR1XXT
#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
    UWB_Tml_Io_Set(kUWBS_IO_O_ENABLE_HELIOS, 1);
    UWB_Tml_Io_Set(kUWBS_IO_O_HELIOS_RTC_SYNC, 1);
    phOsalUwb_Delay(100);
    if (UWB_HbciEncryptedFwDownload()) {
        DEBUGOUT("UWB_HeliosTask(): HELIOS FW download completed\n");
        phOsalUwb_Delay(100);
        UCI_ReaderTaskResume();
    }
    else {
        DEBUGOUT("UWB_HeliosTask(): CRITICAL ERROR downloading Helios image\n");
    }
#endif // INTERNAL_FIRMWARE_DOWNLOAD
#endif // UWBIOT_UWBD_SR1XXT

    DEBUGOUT("UWB_HeliosTask(): resuming communication interfaces\n");
    /* After HBCI DND resume USB task. Now Host can send command over USB CDC interface.*/
    phOsalUwb_TaskResume(mHifTask);
    /*Resume ALL tasks which are used for UCI operations. After HBCI mode UCI tasks need to be enabled.*/
#if (UWBIOT_UWBD_SR04X)
    UCI_ReaderTaskResume();
#endif
    phOsalUwb_TaskResume(mHifWriterTask);
    while (1) {
        if (phOsalUwb_msgrcv(mHifCommandQueue, &evt, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        if (evt.pMsgData != NULL) {
            UWB_HandleEvt((UWB_EvtType_t)evt.eMsgType, evt.pMsgData);
        }
        else {
            DEBUGOUT("UWB_Pnp_App_Task(): Queue is empty\n");
        }
    }
}

#endif /* defined(UWBIOT_APP_BUILD__DEMO_PNP) */

#endif // UWBIOT_OS_ZEPHYR