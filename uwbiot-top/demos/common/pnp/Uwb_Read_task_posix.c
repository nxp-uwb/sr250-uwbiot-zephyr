/*
 * Copyright 2022-2023 NXP
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

/* System includes */
#include "phUwb_BuildConfig.h"

#if UWBIOT_OS_NATIVE
#include "phNxpLogApis_App.h"
#include "phUwbTypes.h"
#include <stdint.h>
#if UWBIOT_UWBD_SR1XXT
#include "UWB_Hbci.h"
#endif // UWBIOT_UWBD_SR1XXT

/* UWB includes */
#include "UWB_Spi_Driver_Interface.h"
#include "UWB_Evt_Pnp.h"
#include "UwbPnpInternal.h"
#include "phOsalUwb.h"

extern void *mReadTaskSyncMutex;
extern volatile int isUCI_State;
extern void *mReadTaskSync_Sem;

void UCI_ReaderTask_Enable(void)
{
    isUCI_State = 1;
    phOsalUwb_ProduceSemaphore(mReadTaskSync_Sem);
}

void UCI_ReaderTask_Disable(void)
{
    isUCI_State = 0;
}

OSAL_TASK_RETURN_TYPE UCI_ReaderTask(void *args)
{
    static uint8_t Buffer[HIF_MAX_PKT_SIZE];
    size_t numRead = HIF_MAX_PKT_SIZE;
    uint32_t error = 0;
    LOG_D("Reader Task Created! \n");
    while (1) {
        if (!isUCI_State) {
            if ((phOsalUwb_ConsumeSemaphore_WithTimeout(mReadTaskSync_Sem, 0xFFFFFF)) != UWBSTATUS_SUCCESS) {
                // LOG_I("reader task timeout");
                continue;
            }
        }
        /* Block UCI Read operation after reset and start after successful FW download */
        (void)phOsalUwb_LockMutex(mReadTaskSyncMutex);
        numRead = HIF_MAX_PKT_SIZE;
        UWB_SpiUciRead(Buffer, &numRead);
        (void)phOsalUwb_UnlockMutex(mReadTaskSyncMutex);
        if (numRead == 0) {
            LOG_E("Spi Read Error, Zero bytes read\n");
            continue;
        }
        LOG_I("received uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
#if ((defined(ENABLE_UWB_RESPONSE)) && (ENABLE_UWB_RESPONSE == ENABLED))
        LOG_I("read returned count is %d\n", numRead);
        LOG_MAU8_I("UCI rsp:", Buffer, numRead);
#endif
        if (Uwb_Is_Hif_Active()) {
            if (MT_UCI_RSP == UCI_MTS_CHECK(Buffer[ACTUAL_PACKET_START])) {
                /* Send response packet */
                LOG_D("sending rsp: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
                error = UWB_Hif_SendUCIRsp(Buffer, numRead);
                if (error != 0) {
                    LOG_E("UCI_READER: error sending RSP over HIF [%d]\n", error);
                }
            }
            else {
                /* Sending NTF and DPF packets to queue */
                LOG_D("sending Ntf : 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
                error = UWB_Hif_UciSendNtfn(Buffer, numRead);
                if (error != 0) {
                    LOG_E("UCI_READER: error sending NTF over HIF [%d]\n", error);
                }
            }
        }
        else {
            LOG_E("USB detached, USB channel reset.\n");
            LOG_E("missed uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
        }
    }
}

#endif // UWBIOT_OS_NATIVE
