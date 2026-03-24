/* Copyright 2021-2024,2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if !defined(UWBIOT_APP_BUILD__DEMO_PNP) && !defined(UWBIOT_APP_BUILD__SE_VCOM)

#include "UwbHif.h"
#include "uwb_int.h"
#include "UwbApi.h"
#include "phOsalUwb.h"

void *mHIFIsr_Sem                = NULL;
volatile uint16_t mRcvTlvSize    = 0;
volatile uint16_t mRcvTlvSizeExp = 0;
volatile bool merror             = FALSE;

#if defined(CPU_S32K144)
uint8_t __attribute__((section(".customSection"))) mRcvTlvBuf[HIF_MAX_PKT_SIZE];
#else
uint8_t mRcvTlvBuf[HIF_MAX_PKT_SIZE];
#endif
UWB_Mode_t usermode;
UWB_Hif_Comm_Mode_t gCommMode;

#if !defined(__linux__)
uint8_t UWB_Socket_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
    return 0;
}
void UWB_Socket_DeInit()
{
}
uint32_t UWB_Socket_SendRsp(uint8_t *pData, size_t size)
{
    return 0;
}
#endif

void HifSetMode(UWB_MODES_t state)
{
    /* store user mode here */
    usermode.uwbmode = state;
}

UWB_MODES_t HifGetMode()
{
    return (UWB_MODES_t)usermode.uwbmode;
}

uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size)
{
    if (gCommMode == kUWB_COMM_Serial) {
        return UWB_Serial_Com_SendRsp(pData, size);
    }
    else {
        return UWB_Socket_SendRsp(pData, size);
    }
}

uint16_t getUciPktSize(uint8_t *hdr)
{
    uint16_t payload_sz;
    if ((hdr[GID_INDEX] == VCOM_SE_GID_HEARDER)) {
        payload_sz = (uint16_t)(UCI_HEADAER + hdr[DATA_PAYLOD_LENGTH_INDEX]);
    }
    else if ((IS_DATA_SEND_PACKET(hdr[0]))) {
        // For data packets, payload length is always encoded in byte 3 and4 (index 2 and 3), the UCI payload extension.
        payload_sz =
            (uint16_t)(UCI_HEADAER + (hdr[DATA_PAYLOD_LENGTH_INDEX] + (hdr[DATA_PAYLOD_LENGTH_INDEX + 1] << 8)));
    }
    else {
        payload_sz = (uint16_t)(UCI_HEADAER + hdr[UCI_PAYLOAD_LENGTH_INDEX]);
    }
    if (hdr[1] & 0x80)
        payload_sz = (uint16_t)((payload_sz << 8) | hdr[2]);
    return payload_sz;
}

// USB functions
OSAL_TASK_RETURN_TYPE UWB_HIFTask(void *args)
{
    phLibUwb_Message_t tlv;

    while (TRUE) {
        // Wait to receive TLV over USB
        if (!merror &&
            phOsalUwb_ConsumeSemaphore_WithTimeout(mHIFIsr_Sem, MAX_HIF_TASK_TLV_WAIT_TIMEOUT) != UWBSTATUS_SUCCESS) {
            continue;
        }
        if (!merror) {
            if (usermode.uwbmode == kUWB_MODE_MCTT) {
                tlv.eMsgType = (uint16_t)UWB_MCTT_UCI_READY;
                tlv.Size     = getUciPktSize(&mRcvTlvBuf[0]);
                tlv.pMsgData = &mRcvTlvBuf[0]; // uci
            }
            else { // generic mode
                tlv.eMsgType = mRcvTlvBuf[CMD_TYPE_OFFSET];
                tlv.Size = (uint16_t)(mRcvTlvBuf[MSB_LENGTH_OFFSET] << MSB_LENGTH_MASK | mRcvTlvBuf[LSB_LENGTH_OFFSET]);
                tlv.pMsgData = &mRcvTlvBuf[CMD_SUB_TYPE_OFFSET];
            }
            phOsalUwb_msgsnd(mHifCommandQueue, &tlv, MAX_DELAY);
        }
    }
}

void Uwb_Hif_ReadDataCb(uint8_t *pData, uint32_t *pLen)
{
    uint8_t headerSize = HEADER_SIZE_GENERIC;
    if (HifGetMode() == kUWB_MODE_MCTT) {
        headerSize = HEADER_SIZE_MCTT;
    }
    // phLibUwb_Message_t tlv;
    while (*pLen > 0) {
        uint16_t cpSize;
        if (mRcvTlvSize < headerSize) {
            cpSize = headerSize;
        }
        else if (mRcvTlvSize + *pLen <= mRcvTlvSizeExp) {
            cpSize = (uint16_t)*pLen;
        }
        else {
            cpSize = (uint16_t)(mRcvTlvSizeExp - mRcvTlvSize);
        }
        /* copy the header*/
        memcpy(&mRcvTlvBuf[mRcvTlvSize], pData, cpSize);
        *pLen -= cpSize;
        pData += cpSize;
        mRcvTlvSize += cpSize;

        if (mRcvTlvSize == headerSize) {
            // TLV size received
            if (HifGetMode() == kUWB_MODE_MCTT) {
                mRcvTlvSizeExp = getUciPktSize(&mRcvTlvBuf[0]);
            }
            else { // generic mode
                mRcvTlvSizeExp = (uint16_t)(3 + (mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]));
            }
        }
    }

    if (mRcvTlvSize == mRcvTlvSizeExp) {
        mRcvTlvSize = mRcvTlvSizeExp = 0;
        merror                       = FALSE;
        phOsalUwb_ProduceSemaphore(mHIFIsr_Sem);
    }
}

void HifInit(UWB_Hif_Comm_Mode_t comm_mode)
{
    UWBSTATUS wCreateStatus = UWBSTATUS_SUCCESS;
    /* This Queue is used to store the commands received from Host
     * Currently it can store MAX 40 elements at a time*/
    mHifCommandQueue = phOsalUwb_msgget(40);
    if (mHifCommandQueue == (intptr_t)NULL) {
        PRINTF("Error: main, could not create queue mHifCommandQueue\n");
        while (TRUE)
            ;
    }

    /* This semaphore is signaled in the USB CDC ISR context when any command is
     * received from Host*/
    wCreateStatus = phOsalUwb_CreateBinSem(&mHIFIsr_Sem);
    if (wCreateStatus != UWBSTATUS_SUCCESS) {
        PRINTF("Error: main, could not create semaphore mHIFIsr_Sem\n");
        while (TRUE)
            ;
    }

    gCommMode = comm_mode;
    if (comm_mode == kUWB_COMM_Serial) {
        UWB_Serial_Com_Init(&Uwb_Hif_ReadDataCb);
    }
    else if (comm_mode == kUWB_COMM_Socket) {
        UWB_Socket_Init(&Uwb_Hif_ReadDataCb);
    }
    else {
        PRINTF("Invalid communication mode");
        while (TRUE)
            ;
    }
}

void HifDeInit()
{
    phOsalUwb_msgrelease(mHifCommandQueue);
    phOsalUwb_DeleteSemaphore(&mHIFIsr_Sem);

    if (gCommMode == kUWB_COMM_Serial) {
        UWB_Serial_Com_DeInit();
    }
    else {
        UWB_Socket_DeInit();
    }
    HifSetMode(kUWB_MODE_UNKNOWN);
}

#endif /* !defined(UWBIOT_APP_BUILD__DEMO_PNP) && !defined(UWBIOT_APP_BUILD__SE_VCOM) */
