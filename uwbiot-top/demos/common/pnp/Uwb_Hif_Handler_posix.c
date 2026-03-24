/*
 * Copyright 2022-2025 NXP
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

#if UWBIOT_OS_NATIVE
#include "unistd.h"
#include "phNxpLogApis_App.h"
#include "UwbPnpInternal.h"
#include "UWB_Evt_Pnp.h"
#include "phOsalUwb.h"
#include "UWB_Spi_Driver_Interface.h"

#include "Uwb_Srv_Pnp.h"

#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
#include "Uwb_Read_task.h"
#if UWBIOT_UWBD_SR1XXT
#include "UWB_Hbci.h"
#endif // UWBIOT_UWBD_SR1XXT
#endif // INTERNAL_FIRMWARE_DOWNLOAD

// extern bool mError;
volatile uint16_t mRcvTlvSize    = 0;
volatile uint16_t mRcvTlvSizeExp = 0;
static uint8_t mRcvTlvBuf[HIF_MAX_PKT_SIZE];
intptr_t mHifCommandQueue;

bool Uwb_Is_Hif_Active()
{
    return true;
}

void Uwb_Reset_Hif_State(bool state)
{
    // mError = state;
}

uint32_t UWB_Hif_UciSendNtfn(uint8_t *pData, uint16_t size)
{
    return UWB_Srv_UciSendNtfn(pData, size);
}

uint32_t UWB_Hif_SendUCIRsp(uint8_t *pData, uint16_t size)
{
    return UWB_Srv_SendUCIRsp(pData, size);
}

uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size)
{
    return UWB_Srv_SendRsp(pData, size);
}
void Uwb_Hif_ReadDataCb(uint8_t *pData, uint32_t *pLen)
{
    while (*pLen > 0) {
        uint32_t cpSize;
        if (mRcvTlvSize < 3) {
            cpSize = 1;
        }
        else if (mRcvTlvSize + *pLen <= mRcvTlvSizeExp) {
            cpSize = *pLen;
        }
        else {
            cpSize = (uint32_t)(mRcvTlvSizeExp - mRcvTlvSize);
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

    if (mRcvTlvSize == mRcvTlvSizeExp) {
        mRcvTlvSize = mRcvTlvSizeExp = 0;
        // mError                       = false;
        (void)phOsalUwb_ProduceSemaphore(mHifIsr_Sem);
    }
}

void UWB_Hif_Init()
{
}

OSAL_TASK_RETURN_TYPE UWB_Hif_Handler_Task(void *args)
{
    phLibUwb_Message_t evt;
    tlv_t tlv;
    evt.eMsgType         = USB_TLV_EVT;
    evt.pMsgData         = &tlv;
    uint16_t tlv_run_len = 0;
    int receive_bytes    = -1;
    LOG_D("HIF Handler Task Created! \n");
    Uwb_Srv_Init();
    while (1) {
        if (!Uwb_Srv_Connect()) {
            LOG_I("TCP/IP client Connected! \n");
            receive_bytes = -1;
        }

        memset(mRcvTlvBuf, 0, 2060);
        while (0 != receive_bytes) {
            receive_bytes = Uwb_Srv_Sock_Recv(&mRcvTlvBuf[0], 3 /*HIF_MAX_PKT_SIZE*/);
            if (!receive_bytes) {
                continue;
            }

            /* Wait to receive TLV over USB */
            tlv.type  = mRcvTlvBuf[0];
            tlv.size  = (uint16_t)(mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]);
            tlv.value = &mRcvTlvBuf[3];

            LOG_D("TLV Size %d\n", tlv.size);

            if (tlv.size > 0) {
                tlv_run_len = Uwb_Srv_Sock_Recv(&mRcvTlvBuf[3], tlv.size);
                LOG_D("tlv_run_len before 0x%x\n", tlv_run_len);

                if (0 <= tlv_run_len) {
                    while (tlv_run_len < tlv.size) {
                        receive_bytes = Uwb_Srv_Sock_Recv(&mRcvTlvBuf[3 + tlv_run_len], tlv.size - tlv_run_len);
                        if (receive_bytes) {
                            tlv_run_len += receive_bytes;
                        }
                        else {
                            break;
                        }
                        LOG_D("tlv_run_len after 0x%x\n", tlv_run_len);
                        usleep(50000); // 50ms delay before next read.
                    };
                }
                else {
                    LOG_E("Read from Network Error !!!\n");
                }
                LOG_D("SRVRCV_TLV 0x%X 0x%X 0x%X\n", mRcvTlvBuf[3], mRcvTlvBuf[4], mRcvTlvBuf[5], mRcvTlvBuf[6]);
            }
            else {
                LOG_D("SRVRCV 0x%X 0x%X 0x%X\n", mRcvTlvBuf[0], mRcvTlvBuf[1], mRcvTlvBuf[2]);
            }
            // LOG_I("[TLV %02x]", tlv.type);
            // LOG_MAU8_I("TLV DATA", tlv.value, tlv.size);
            (void)phOsalUwb_msgsnd(mHifCommandQueue, &evt, MAX_DELAY);
        };
    };
}

void UWB_HandleEvt(UWB_EvtType_t ev, void *args)
{
    switch (ev) {
    case USB_TLV_EVT:
#if UWBIOT_UWBD_SR1XXT
        UWB_Handle_SR1XXT_TLV((tlv_t *)args);
#elif UWBIOT_UWBD_SR2XXT
        UWB_Handle_SR2XXT_TLV((tlv_t *)args);
#endif
        break;
    default:
        LOG_E("ERROR: Unknown event type %02x", ev);
        break;
    }
}

OSAL_TASK_RETURN_TYPE UWB_Pnp_App_Task(void *args)
{
    /* This may go somehwere else.. */
    phLibUwb_Message_t evt = {0};

    LOG_D("UWB_HeliosTask(): suspending communication interfaces\n");

#if (UWBFTR_SE_SE051W || UWBIOT_SESN_SNXXX)
    Se_Comm_Init();
#endif

#if UWBIOT_UWBD_SR1XXT
#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
    UCI_ReaderTask_Disable();
    UWB_Tml_Io_Set(kUWBS_IO_O_ENABLE_HELIOS, 1);
    UWB_Tml_Io_Set(kUWBS_IO_O_HELIOS_RTC_SYNC, 1);
    phOsalUwb_Delay(100);
    if (UWB_HbciEncryptedFwDownload()) {
        LOG_D("UWB_HeliosTask(): HELIOS FW download completed\n");
        phOsalUwb_Delay(100);
        UCI_ReaderTask_Enable();
    }
    else {
        LOG_E("UWB_HeliosTask(): CRITICAL ERROR downloading Helios image\n");
    }
#endif // INTERNAL_FIRMWARE_DOWNLOAD
#endif

    LOG_D("UWB_HeliosTask(): resuming communication interfaces\n");
    while (1) {
        if (phOsalUwb_msgrcv(mHifCommandQueue, &evt, MAX_DELAY) == UWBSTATUS_FAILED) {
            continue;
        }
        UWB_HandleEvt(evt.eMsgType, evt.pMsgData);
    }
}

#endif // UWBIOT_OS_NATIVE
