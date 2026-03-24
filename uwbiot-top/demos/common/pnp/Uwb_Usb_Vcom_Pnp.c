/*
 * The Clear BSD License

 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)

#if defined(CPU_LPC55S69JBD100_cm33) || defined(CPU_LPC54628J512ET180) || defined(CPU_MIMXRT1176DVMAA) || \
    defined(CPU_RW612ETA2I)

#include "board.h"
#include "usb.h"
#include "fsl_os_abstraction.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_config.h"
#include "usb_device_descriptor.h"
#include "usb_device_dci.h"
#include "usb_device_cdc_acm.h"

#if defined(CPU_RW612ETA2I)
#include "fsl_common.h"
#include "fsl_component_serial_manager.h"
#include "fsl_component_serial_port_internal.h"
#include "fsl_component_serial_port_usb.h"
#endif // defined(CPU_RW612ETA2I)
#include "virtual_com.h"
#include "Uwb_Vcom_Pnp.h"
#include "UwbPnpInternal.h"
#include "app_config.h"
#include "UwbPnpInternal.h"
#include "UwbApi_Utility.h"

static void UWB_Usb_SendDoneCb(uint8_t *pData, uint16_t size);

extern void AddDelayInMicroSec(int delay);
#if defined(CPU_RW612ETA2I)
extern serial_usb_cdc_state_t s_UsbCdcVcom;
static serial_usb_cdc_state_t *s_cdcVcom_pt;
#else
extern usb_cdc_vcom_struct_t *s_cdcVcom_pt;
#endif // defined(CPU_RW612ETA2I)

void *mHifNtfnSem = NULL;
void *mHifRspSem  = NULL;

#if defined(CPU_LPC54628J512ET180)
volatile bool mError = false;
#endif // defined(CPU_LPC54628J512ET180)

void Uwb_Usb_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
#if defined(CPU_RW612ETA2I)
    s_cdcVcom_pt = &s_UsbCdcVcom;
#endif // defined(CPU_RW612ETA2I)
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending Notifications
     * from Rhodes*/
    if (phOsalUwb_CreateBinSem(&mHifNtfnSem) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifNtfnSem\n");
        while (1)
            ;
    }
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending UCI resp from
     * Rhodes*/
    if (phOsalUwb_CreateBinSem(&mHifRspSem) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifRspSem\n");
        while (1)
            ;
    }

    USB_Init(&UWB_Usb_SendDoneCb, rcvCb);
}

void UWB_Usb_SendDoneCb(uint8_t *pData, uint16_t size)
{
    if (pData != NULL && size > 4) {
        if (MT_UCI_RSP == UCI_MTS_CHECK(pData[0])) {
            DEBUGOUT("Sent rsp: 0x%X 0x%X 0x%X 0x%X\n", pData[0], pData[1], pData[2], pData[3]);
            (void)phOsalUwb_ProduceSemaphore(mHifRspSem);
        }
        else {
            DEBUGOUT("Sent ntf: 0x%X 0x%X 0x%X 0x%X 0x%X\n", pData[0], pData[1], pData[2], pData[3], pData[4]);
            (void)phOsalUwb_ProduceSemaphore(mHifNtfnSem);
        }
    }
}

uint32_t UWB_Usb_UciSendNtfn(uint8_t *pData, uint16_t size)
{
    uint8_t retry_counter = 0;
#if defined(CPU_RW612ETA2I)
    serial_manager_status_t error = kStatus_SerialManager_Error;
retry:
    error = Serial_UsbCdcWrite(s_cdcVcom_pt, pData, size);
#else
    usb_status_t error = kStatus_USB_Error;
retry:
    error = USB_DeviceCdcAcmSend(s_cdcVcom_pt->cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT, pData, size);
#endif // defined(CPU_RW612ETA2I)
    if (error != 0) {
        DEBUGOUT("UWB_WRITER: error sending over USB [%d]\n", error);
#if defined(CPU_RW612ETA2I)
        if ((kStatus_SerialManager_Busy == error) && (retry_counter < USB_TX_RETRY_COUNT)) {
#else
        if ((kStatus_USB_Busy == error) && (retry_counter < USB_TX_RETRY_COUNT)) {
#endif
            /* USB was busy and could not send the packet. Wait for a while and retry again */
            AddDelayInMicroSec(100);
            retry_counter++;
            goto retry;
        }
    }
    else {
#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
        (void)phOsalUwb_ConsumeSemaphore_WithTimeout(mHifNtfnSem, 100); /* Wait for 100msec */
#else
        if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifNtfnSem, 10000) != UWBSTATUS_SUCCESS) {
            /* Wait for worst case 10 Sec, Since Windows Host is Slow */
            DEBUGOUT("Ntf Timed out after 10 sec of waiting\n");
        }
#endif
    }
    return (uint32_t)error;
}

uint32_t UWB_Usb_SendUCIRsp(uint8_t *pData, uint16_t size)
{
    uint8_t retry_counter = 0;
#if defined(CPU_RW612ETA2I)
    serial_manager_status_t error = kStatus_SerialManager_Error;
retry:
    error = Serial_UsbCdcWrite(s_cdcVcom_pt, pData, size);
#else
    usb_status_t error = kStatus_USB_Error;
retry:
    error = USB_DeviceCdcAcmSend(s_cdcVcom_pt->cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT, pData, size);
#endif // defined(CPU_RW612ETA2I)
    if (error != 0) {
        DEBUGOUT("UCI_READER: error sending over USB [%d]\n", error);
#if defined(CPU_RW612ETA2I)
        if ((kStatus_SerialManager_Busy == error) && (retry_counter < USB_TX_RETRY_COUNT)) {
#else
        if ((kStatus_USB_Busy == error) && (retry_counter < USB_TX_RETRY_COUNT)) {
#endif
            /* USB was busy and could not send the packet. Wait for a while and retry again */
            AddDelayInMicroSec(50);
            retry_counter++;
            goto retry;
        }
    }
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifRspSem, 5000) == UWBSTATUS_SUCCESS) {
        /* Wait for Max 5 sec to read the response by windows since it is slow.
           Waiting for Response */
    }
    else {
        DEBUGOUT("UCI_READER: Sem Timeout for Uci Rsp\n");
    }
    return (uint32_t)error;
}

uint32_t UWB_Usb_SendRsp(uint8_t *pData, uint16_t size)
{
#if defined(CPU_RW612ETA2I)
    serial_manager_status_t error = kStatus_SerialManager_Error;
    error                         = Serial_UsbCdcWrite(s_cdcVcom_pt, pData, size);
#else
    usb_status_t error = kStatus_USB_Error;
    error              = USB_DeviceCdcAcmSend(s_cdcVcom_pt->cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT, pData, size);
#endif // defined(CPU_RW612ETA2I)
    if (error != 0) {
        DEBUGOUT("UCI_READER: error sending over USB [%d]\n", error);
    }
    return (uint32_t)error;
}

#endif // defined(CPU_LPC55S69JBD100_cm33)

#endif // UWBIOT_APP_BUILD__DEMO_PNP
