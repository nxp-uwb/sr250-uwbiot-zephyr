/* Copyright 2025,2026 NXP
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

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)
#include <stdio.h>
#include "board.h"
#include "UWB_Spi_Driver_Interface.h"
#include "Uwb_Read_task.h"
#include "UwbPnpInternal.h"
#include "phOsalUwb_Thread.h"
#include "phOsalUwb_Queue.h"
#include "phUwbTypes.h"
#include "UWB_DRIVER.h"
#include "phNxpUciHal_fwd.h"
#include "uwbiot_ver.h"
#include "UwbApi_Utility.h"
#include "app_config.h"

#define HIF_TASK_PRIO        1
#define PNP_APP_TASK_PRIO    1
#define UCI_READER_TASK_PRIO 1
#define HIF_WRITER_TASK_PRIO 1

/* Response Bytes to the Reset Command */
#define RESET_RSP_BYTE_1 0x01
#define RESET_RSP_BYTE_2 0x02
#define RESET_RSP_BYTE_3 0x03
#define RESET_RSP_BYTE_4 0x04

UWBOSAL_TASK_HANDLE mHifTask;
UWBOSAL_TASK_HANDLE mPnpAppTask;
UWBOSAL_TASK_HANDLE mUciReaderTask;
UWBOSAL_TASK_HANDLE mHifWriterTask;
void *mHifWriteMutex = NULL;
void *mHifSyncMutex  = NULL;
intptr_t mHifWriteQueue;
void *mHifIsr_Sem = NULL;
static uint8_t mRxData[UWB_MAX_HELIOS_RSP_SIZE];
static uint8_t mTlvBuf[TLV_RESP_SIZE];
static uint8_t uci_data[HIF_MAX_PKT_SIZE];

static bool fwdlIsSuccessful = false;
volatile bool mError         = false;
extern void uwb_board_virgo_enter_bootloader_mode(void);

/*USB header is handled here as per designed doc*/
void UWB_Handle_SR2XXT_TLV(tlv_t *tlv)
{
    size_t rspLen = UWB_MAX_HELIOS_RSP_SIZE;
    DEBUGOUT("\n0x%x\n", tlv->type);
    phOsalUwb_MemCopy(&uci_data[UCI_CMD_INDEX], tlv->value, tlv->size);
    switch (tlv->type) {
    case UCI_CMD: {
#if (ENABLE_UCI_CMD_LOGGING == ENABLED)
        DEBUGOUT("\r\n[UCI CMD] ");
        for (int i = 0; i < tlv->size; i++) {
            DEBUGOUT("%02x", tlv->value[i]);
        }
        DEBUGOUT("\r\n");
#endif

        if (UWB_SpiUciWrite(uci_data, tlv->size) == kUWBSTATUS_SUCCESS) {
        }
        else {
            DEBUGOUT("ERROR: error processing UCI command\n");
        }

    } break;

    case HDLL_GET_NTF:
        DEBUGOUT("HDLL Get NTF cmd Received\n");
        if (kUWBSTATUS_SUCCESS == UWB_Tml_Hdll_Read(mRxData, &rspLen)) {
            DEBUGOUT("UWB_Tml_Hdll_Read : Success\n");
            UWB_Hif_SendRsp(mRxData, rspLen);
        }
        else {
            /* Retry once again if we were early */
            phOsalUwb_Delay(1);
            if (kUWBSTATUS_SUCCESS == UWB_Tml_Hdll_Read(mRxData, &rspLen)) {
                DEBUGOUT("UWB_Tml_Hdll_Read : Success\n");
                UWB_Hif_SendRsp(mRxData, rspLen);
            }
        }
        if (fwdlIsSuccessful == true) {
            UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_UCI);
            phOsalUwb_Delay(30);
            UCI_ReaderTaskResume();
            phOsalUwb_TaskResume(mHifWriterTask);
            UWB_Uwbs_Enable_Interrupt();
            fwdlIsSuccessful = false;
        }
        DEBUGOUT("HDLL Get NTF cmd Received : EXIT\n");
        break;

    case HDLL_CMD:
        DEBUGOUT("HDLL cmd Received\n");

        if (UWB_Tml_Hdll_WriteRead(uci_data, tlv->size, mRxData, &rspLen) == kUWBSTATUS_SUCCESS) {
            UWB_Hif_SendRsp(mRxData, rspLen);
        }
        else {
            DEBUGOUT("ERROR: error processing UCI command\n");
        }
        break;

    case HDLL_EDL_LAST_WRITE: {
        DEBUGOUT("EDL Last Write Cmd Received\n");
        if (UWB_Tml_Hdll_WriteRead(uci_data, tlv->size, mRxData, &rspLen) == kUWBSTATUS_SUCCESS) {
            UWB_Hif_SendRsp(mRxData, rspLen);
        }
        else {
            DEBUGOUT("ERROR: error processing UCI command\n");
        }
        phOsalUwb_Delay(10);
        UWB_Tml_Chip_Reset();
        fwdlIsSuccessful = true;

    } break;

    case HELIOS2_RESET: {
        DEBUGOUT("Chip Enable Started\n");
        UWB_Tml_Chip_Reset();
        phOsalUwb_Delay(5);
        if (tlv->value[0] == true) {
            fwdlIsSuccessful = true;
        }
    } break;

    case HDLL_RESET_TO_UCI: {
        UWB_Tml_Chip_Reset();
        phOsalUwb_Delay(10);
        UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_HDLL);
        UWB_Tml_GetHdllReadyNtf();
        DEBUGOUT("Chip Enable Completed\n");
        phOsalUwb_Delay(150);

        UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_UCI);
        UCI_ReaderTaskResume();
        phOsalUwb_TaskResume(mHifWriterTask);
        UWB_Uwbs_Enable_Interrupt();
    } break;

    case RESET: {
        DEBUGOUT("Reset Received\n");
        (void)phOsalUwb_LockMutex(mHifSyncMutex);
        PRINTF_WITH_TIME("After mUsbSyncMutex\n");
        UCI_ReaderTaskSuspend();
        phOsalUwb_TaskSuspend(mHifWriterTask);

        /*Delete all the elements from the USB Write Queue before going in to Bootrom mode*/
        int itemsInQueue = (WRITER_QUEUE_SIZE - phOsalUwb_queueSpacesAvailable(mHifWriteQueue));
        PRINTF_WITH_TIME("items in Queue : %d\n", itemsInQueue);
        for (int i = 0; i < itemsInQueue; i++) {
            phLibUwb_Message_t tlv;
            if (phOsalUwb_msgrcv(mHifWriteQueue, &tlv, NO_DELAY) == UWBSTATUS_FAILED) {
                DEBUGOUT("Failed to Receive an item\n");
                continue;
            }
            if (tlv.pMsgData != NULL) {
                phOsalUwb_FreeMemory(tlv.pMsgData);
                tlv.pMsgData = NULL;
                DEBUGOUT("FREE\r\n");
            }
        }
        DEBUGOUT("Chip Enable Started\n");
        fwdlIsSuccessful = false;
        UWB_Tml_Chip_Reset();
        UWB_Tml_Set_Mode(kUWB_UWBS_TML_MODE_HDLL);
        // UWB_Tml_GetHdllReadyNtf();
        DEBUGOUT("Chip Enable Completed\n");
        (void)phOsalUwb_UnlockMutex(mHifSyncMutex);
        mTlvBuf[0] = RESET_RSP_BYTE_1;
        mTlvBuf[1] = RESET_RSP_BYTE_2;
        mTlvBuf[2] = RESET_RSP_BYTE_3;
        mTlvBuf[3] = RESET_RSP_BYTE_4;

        // Send response over USB to HOST upper layer
        UWB_Hif_internal_SendRsp(mTlvBuf, RESET_SOFTWARE_VERSION_SIZE);
    } break;
    case GET_SOFTWARE_VERSION: {
        mTlvBuf[0] = GET_SOFTWARE_VERSION;
        mTlvBuf[1] = 0x02;
        mTlvBuf[2] = UWBIOTVER_STR_VER_MAJOR;
        mTlvBuf[3] = UWBIOTVER_STR_VER_MINOR;
        UWB_Hif_internal_SendRsp(mTlvBuf, RESET_SOFTWARE_VERSION_SIZE);
    } break;
    case GET_BOARD_ID: {
        uint8_t len = 16;
        mTlvBuf[0]  = GET_BOARD_ID;
        mTlvBuf[1]  = 0x10;
        /*BLE is not supported for LPC55s69*/
        BOARD_GetMCUUid(&mTlvBuf[2], &len);
        UWB_Hif_internal_SendRsp(mTlvBuf, (uint16_t)(len + 2));
    } break;
    case MCU_RESET: {
        DEBUGOUT("Entering bootloader!\n");
#if UWBIOT_OS_FREERTOS
        uwb_board_virgo_enter_bootloader_mode();
#endif //#if UWBIOT_OS_FREERTOS
    } break;
    case USB_LOOPBACK: { // echo back to usb
        UWB_Hif_internal_SendRsp(tlv->value - 3, (uint16_t)(tlv->size + 3));
    } break;
    case GET_BOARD_VERSION: {
        DEBUGOUT("\nGET_BOARD_VERSION\n");
        mTlvBuf[0] = UWB_BOARD_VERSION;
        UWB_Hif_internal_SendRsp(mTlvBuf, 1);
    } break;
    case GET_VERISON_INFO: {
        size_t rspSize = UWBPnP_GetVersionInfo(mTlvBuf, sizeof(mTlvBuf), kUBWPnPBoardIdentifier_SR2XX_Crete);
        UWB_Hif_internal_SendRsp(mTlvBuf, rspSize);
    } break;
    default:
        DEBUGOUT("ERROR: invalid TLV type %02x\n", tlv->type);
    }
}

/*
 * @brief   Application entry point.
 */
int main(void)
{
    phOsalUwb_ThreadCreationParams_t threadParams;
#if UWBIOT_OS_FREERTOS
    /* Init board hardware. */
    hardware_init();
    /* Uncomment to enable debug logs over UART */
    // BOARD_InitDebugConsole();
    BOARD_InitRW61xIRQ();
#endif // UWBIOT_OS_FREERTOS

    PRINT_APP_NAME("Demo PNP FRDM_RW612");

    /* Init Helios subsystem */
    if (kUWBSTATUS_SUCCESS == UWB_HeliosSpiInit()) {
        PRINTF_WITH_TIME("main(): Helios initialized\n");
    }
    else {
        PRINTF_WITH_TIME("CRITICAL: error initializing Helios\n");
        while (1)
            ;
    }
    if (kUWBSTATUS_SUCCESS == UWB_Tml_Io_Init()) {
        PRINTF_WITH_TIME("main(): Helios initialized\n");
    }
    else {
        PRINTF_WITH_TIME("CRITICAL: error initializing Helios\n");
        while (1)
            ;
    }

    PRINTF_WITH_TIME("main(): GPIO/IRQ module initialized\n");

    /* This mutex is used to make USB write operations(Bulkin) from FRDM_RW612 mutually exclusive . */
    if (phOsalUwb_CreateMutex(&mHifWriteMutex) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: UWB_HeliosInit(), could not create mutex mUsbWriteMutex\n");
        while (1)
            ;
    }
    /* This mutex is used to make Reset operation and USB write operation from UWB_WriterTask mutually exclusive
     * anytime Host can send a Reset command when ranging is ongoing*/
    if (phOsalUwb_CreateMutex(&mHifSyncMutex) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: UWB_HeliosInit(), could not create mutex mUsbSyncMutex\n");
        while (1)
            ;
    }
    /* This semaphore is signaled in the USB CDC ISR context when any command is received from Host*/
    if (phOsalUwb_CreateBinSem(&mHifIsr_Sem) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mSem\n");
        while (1)
            ;
    }
    /* This Queue is used to store the notifications received from helios
     * Currently it can store WRITER_QUEUE_SIZE elements*/
    mHifWriteQueue = phOsalUwb_msgget(WRITER_QUEUE_SIZE);
    if (!mHifWriteQueue) {
        PRINTF_WITH_TIME("Error: main, could not create queue mUsbWriteQueue\n");
        while (1)
            ;
    }
    /* This Queue is used to store the commands received from Host
     * Currently it can store MAX 1 element at a time*/
    mHifCommandQueue = phOsalUwb_msgget(1);
    if (!mHifCommandQueue) {
        PRINTF_WITH_TIME("Error: main, could not create queue mHifCommandQueue\n");
        while (1)
            ;
    }

    UWBIOT_STACK_DEFINE(UWB_Pnp_App_Task_stack, PNP_APP_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadParams, "UWB_Pnp_App_Task");
    threadParams.pContext = NULL;
    threadParams.priority = PNP_APP_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadParams.pStack     = (k_thread_stack_t *)&UWB_Pnp_App_Task_stack;
    threadParams.stackdepth = UWBIOT_THREAD_STACK_SIZE(UWB_Pnp_App_Task_stack);
#else
    threadParams.stackdepth = PNP_APP_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    /*This is the PNP Rhodes Application task which receives all the commands sent by Host*/
    phOsalUwb_Thread_Create((void **)&mPnpAppTask, &UWB_Pnp_App_Task, &threadParams);

    UWBIOT_STACK_DEFINE(UWB_HIF_Task_stack, HIF_TASK_SIZE);

    PHOSALUWB_SET_TASKNAME(threadParams, "UWB_HIF_Task");
    threadParams.pContext = NULL;
    threadParams.priority = HIF_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadParams.pStack     = (k_thread_stack_t *)&UWB_HIF_Task_stack;
    threadParams.stackdepth = UWBIOT_THREAD_STACK_SIZE(UWB_HIF_Task_stack);
#else
    threadParams.stackdepth = HIF_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    /*This task is waiting on a CDC USB interrupt. Once USB command is received, it is forwarded to UWB_HeliosTask queue
     *for the further processing */
    phOsalUwb_Thread_Create((void **)&mHifTask, &UWB_Hif_Handler_Task, &threadParams);

    UWBIOT_STACK_DEFINE(UCI_ReaderTask_stack, UCI_READER_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadParams, "UCI_ReaderTask");
    threadParams.pContext = NULL;
    threadParams.priority = UCI_READER_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadParams.pStack     = (k_thread_stack_t *)&UCI_ReaderTask_stack;
    threadParams.stackdepth = UWBIOT_THREAD_STACK_SIZE(UCI_ReaderTask_stack);
#else
    threadParams.stackdepth = UCI_READER_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    /*This task is used for reading UCI resp and notifications from Helios. its blocked on a helios IRQ interrupt*/
    phOsalUwb_Thread_Create((void **)&mUciReaderTask, &UCI_ReaderTask, &threadParams);

    UWBIOT_STACK_DEFINE(UWB_WriterTask_stack, HIF_WRITER_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadParams, "UWB_WriterTask");
    threadParams.pContext = NULL;
    threadParams.priority = HIF_WRITER_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadParams.pStack     = (k_thread_stack_t *)&UWB_WriterTask_stack;
    threadParams.stackdepth = UWBIOT_THREAD_STACK_SIZE(UWB_WriterTask_stack);
#else
    threadParams.stackdepth = HIF_WRITER_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR

    /*This task is used for sending all notifications received from helios to Host via CDC USB interface.*/
    phOsalUwb_Thread_Create((void **)&mHifWriterTask, &UWB_WriterTask, &threadParams);
#if UWBIOT_OS_ZEPHYR
    phOsalUwb_Thread_Join(mPnpAppTask);
    phOsalUwb_Thread_Join(mHifTask);
    phOsalUwb_Thread_Join(mUciReaderTask);
    phOsalUwb_Thread_Join(mHifWriterTask);
#else
    phOsalUwb_TaskStartScheduler();
#endif // UWBIOT_OS_ZEPHYR
    return 0;
}

void uwb_pnp_board_protocol_error_handler()
{
    // Nothing special to do here (yet).
}

#endif // UWBIOT_APP_BUILD__DEMO_PNP