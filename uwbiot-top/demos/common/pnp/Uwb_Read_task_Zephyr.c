/*
 * Copyright 2025 NXP
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

#include "phUwbTypes.h"
#include <stdint.h>
#if UWBIOT_OS_ZEPHYR

/* Freescale includes */

#if UWBIOT_UWBD_SR1XXT
#if ((defined(BOARD_VERSION)) && (defined(SHIELD)) && (defined(RHODES_V4))) && \
    (((BOARD_VERSION != SHIELD) && (BOARD_VERSION != RHODES_V4)))
#include "fsl_port.h"
#include "fsl_lpspi.h"
#endif //((BOARD_VERSION != SHIELD) && (BOARD_VERSION != RHODES_V4))
#include "UWB_Hbci.h"
#endif
//#include "fsl_gpio.h"
/* UWB includes */
#include "UWB_Spi_Driver_Interface.h"

#include "UWB_Evt_Pnp.h"
#include "UwbPnpInternal.h"
#include "phOsalUwb.h"
#include "app_config.h"

extern UWBOSAL_TASK_HANDLE mUciReaderTask;

void UCI_ReaderTaskSuspend(void)
{
    // Suspend task
    UWB_Uwbs_ConsumeIRQ_Interrupt();
    phOsalUwb_TaskSuspend(mUciReaderTask);
    // Consume IRQ semaphore
    UWB_Uwbs_ConsumeIRQ_Interrupt();
}

void UCI_ReaderTaskResume(void)
{
    // Resume task
    phOsalUwb_TaskResume(mUciReaderTask);
}

OSAL_TASK_RETURN_TYPE UCI_ReaderTask(void *args)
{
    static uint8_t Buffer[HIF_MAX_PKT_SIZE + ACTUAL_PACKET_START];
    size_t numRead = HIF_MAX_PKT_SIZE;
    while (1) {
        numRead = HIF_MAX_PKT_SIZE;
        UWB_SpiUciRead(Buffer, &numRead);
        if (numRead == 0) {
            DEBUGOUT("Spi Read Error, Zero bytes read\n");
            continue;
        }
        DEBUGOUT("received uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
#if ((defined(ENABLE_UWB_RESPONSE)) && (ENABLE_UWB_RESPONSE == ENABLED))
        DEBUGOUT("read returned count is %d\n", numRead);
        DEBUGOUT("UCI rsp:\n");
        for (int i = 0; i < numRead; i++) {
            DEBUGOUT(" %02x", Buffer[i]);
        }
        DEBUGOUT("\n");
#endif
        if (!Uwb_Is_Hif_Active()) {
            /* Send only response packet here. Otherwise send over queue to HifWriterTask*/
            /**
             * * UCI Payload:
             *   - Macro: `#define ACTUAL_PACKET_START (UCI_CMD_INDEX
             *   - Description: Refers to the section of the buffer where the actual UCI (UWB Command Interface)
             *                : data will be stored, starting after the platform-specific and bidirectional
             * bytes.
             *   - For any Uwbs all Bidirectional and Platform specific handling done on the Transport layer.
             *   - And actual Payload for the Read start from the "ACTUAL_PACKET_START"
             *
             */
            if (MT_UCI_RSP == UCI_MTS_CHECK(Buffer[ACTUAL_PACKET_START])) {
                /* Send response packet */
                uint32_t error;
                (void)phOsalUwb_LockMutex(mHifWriteMutex);
                DEBUGOUT("sending rsp: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
                error = UWB_Hif_SendUCIRsp(Buffer, numRead);
                if (error != 0) {
                    DEBUGOUT("UCI_READER: error sending over HIF [%d]\n", error);
                }
                (void)phOsalUwb_UnlockMutex(mHifWriteMutex);
            }
            else {
                /* Send to HifWriterTask
                 * Sending NTF and DPF packets to queue
                 */
                if (phOsalUwb_queueSpacesAvailable(mHifWriteQueue) > 0) {
                    phLibUwb_Message_t tlv;
                    if (numRead <= HIF_MAX_PKT_SIZE) {
                        tlv.Size     = (uint16_t)numRead;
                        tlv.pMsgData = (void *)phOsalUwb_GetMemory(tlv.Size * sizeof(uint8_t));
                        if (tlv.pMsgData != NULL) {
                            /**
                             * * UCI Payload:
                             *   - Macro: `#define ACTUAL_PACKET_START (UCI_CMD_INDEX
                             *   - Description: Refers to the section of the buffer where the actual UCI (UWB Command
                             * Interface) : data will be stored, starting after the platform-specific and bidirectional
                             * bytes.
                             *   - For any Uwbs all Bidirectional and Platform specific handling done on the Transport
                             * layer.
                             *   - And actual Payload for the Read start from the "ACTUAL_PACKET_START"
                             *   - Copy the whole data including the Platform and bidirectional for both SR1XX and SR2XX
                             *   - Driver will take care of the buffer allocation and maintaince .
                             */
                            phOsalUwb_MemCopy((uint8_t *)tlv.pMsgData, Buffer, tlv.Size + ACTUAL_PACKET_START);
                            phOsalUwb_msgsnd(mHifWriteQueue, &tlv, NO_DELAY);
                        }
                        else {
                            DEBUGOUT("UCI_ReaderTask: Unable to Allocate Memory of %d, Memory Full:\n", tlv.Size);
                        }
                    }
                    else {
                        DEBUGOUT("UCI_ReaderTask: Invalid number of bytes read %d:\n", numRead);
                    }
                }
                else {
                    DEBUGOUT("Queue is FULL, ignoring notification\n");
                }
            }
        }
        else {
            DEBUGOUT("USB detached, USB channel reset.\n");
            DEBUGOUT("missed uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
            Uwb_Reset_Hif_State(FALSE);
        }
    }
}
#endif // UWBIOT_OS_FREERTOS
