/* Copyright 2019-2020,2022-2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phOsalUwb.h"
#include "AppInternal.h"
#include "Utilities.h"
#if !(UWBIOT_UWBD_SR04X)
#include "PrintUtility_RfTest.h"
#endif // !(UWBIOT_UWBD_SR04X)
#include "UwbApi_Types.h"
#include "UwbApi_Utility.h"

#if UWBIOT_UWBD_SR1XXT
#include "UwbApi_Proprietary_Fm.h"
#endif

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif
#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if !defined(UWBIOT_APP_BUILD__DEMO_CDC)

void *perSem;
void *rangingDataSem;
void *inBandterminationSem;
void *testLoopBackNtfSem;
void *datatransferNtfSemRx;
void *datatransferNtfSemTx;
void *dataRcvNtfSemTx;
void *RadarTstAntIsoNtfSemTx;
void *llCreateStatusSem;
void *llUwbsCloseNtf;

#if !(UWBIOT_UWBD_SR04X)
uint8_t dataToSend[UWBS_MAX_UCI_PACKET_SIZE];
uint8_t dataReceived[UWBS_MAX_UCI_PACKET_SIZE];
uint16_t rcvDataSz = 0;
#endif

intptr_t demoNtfMngQueue;

#if UWBIOT_OS_NATIVE
uint8_t isRecovery = false;
#endif

extern UWBOSAL_TASK_HANDLE uwb_demo_start(void);
static UWBOSAL_TASK_HANDLE demoNtfHandlingTaskHandle;
static void demo_recovery_handler(eNotificationType opType);
extern UWBOSAL_TASK_HANDLE testTaskHandle;
extern void sendPresenceDetectionData(uint8_t *cirdata, uint32_t cir_len);

/*
 * Function to check whether sent data and received data are same or not
 * It compares data received with sent data.
 * If it matches, then only it returns success
 */
#if !(UWBIOT_UWBD_SR04X)
tUWBAPI_STATUS validateReceivedData()
{
    /* Generate the Data to be compared with */
    GENERATE_SEND_DATA(dataToSend, rcvDataSz)

    if ((phOsalUwb_MemCompare(dataReceived, dataToSend, sizeof(dataToSend)) == UWBAPI_STATUS_OK)) {
        NXPLOG_APP_I("Data received successfully!!!");
        return UWBAPI_STATUS_OK;
    }
    else {
        NXPLOG_APP_E("Data NOT received successfully");
        return UWBAPI_STATUS_FAILED;
    }
}
#endif
void UWBDemo_Init()
{
    phOsalUwb_CreateSemaphore(&perSem, 0);
    phOsalUwb_CreateSemaphore(&rangingDataSem, 0);
    phOsalUwb_CreateSemaphore(&inBandterminationSem, 0);
    phOsalUwb_CreateSemaphore(&testLoopBackNtfSem, 0);
    phOsalUwb_CreateSemaphore(&datatransferNtfSemRx, 0);
    phOsalUwb_CreateSemaphore(&datatransferNtfSemTx, 0);
    phOsalUwb_CreateSemaphore(&dataRcvNtfSemTx, 0);
    phOsalUwb_CreateSemaphore(&RadarTstAntIsoNtfSemTx, 0);
    phOsalUwb_CreateSemaphore(&llCreateStatusSem, 0);
    phOsalUwb_CreateSemaphore(&llUwbsCloseNtf, 0);
}

void UWBDemo_DeInit()
{
    phOsalUwb_DeleteSemaphore(&perSem);
    phOsalUwb_DeleteSemaphore(&rangingDataSem);
    phOsalUwb_DeleteSemaphore(&inBandterminationSem);
    phOsalUwb_DeleteSemaphore(&testLoopBackNtfSem);
    phOsalUwb_DeleteSemaphore(&datatransferNtfSemRx);
    phOsalUwb_DeleteSemaphore(&datatransferNtfSemTx);
    phOsalUwb_DeleteSemaphore(&RadarTstAntIsoNtfSemTx);
    phOsalUwb_DeleteSemaphore(&llCreateStatusSem);
    phOsalUwb_DeleteSemaphore(&llUwbsCloseNtf);
}

/** demo ntf handling function*/
void demo_handle_ntf(phLibUwb_Message_t *evt)
{
    size_t len               = evt->Size;
    eNotificationType opType = evt->eMsgType;
    uint8_t *pData           = (uint8_t *)evt->pMsgData;

    switch (opType) {
    case UWBD_RANGING_DATA: {
        phRangingData_t pRangingData = {0};
        parseRangingNtf(pData, (uint16_t)len, &pRangingData);
        printRangingData(&pRangingData);
#if !(UWBIOT_UWBD_SR04X)
        printDistance_Aoa(&pRangingData);
#endif //!(UWBIOT_UWBD_SR04X)
        break;
    }
    case UWBD_TEST_MODE_LOOP_BACK_NTF: {
#if (UWBIOT_UWBD_SR04X)
        uint8_t *p                              = (uint8_t *)pData;
        phTestLoopbackData_t testLoopbackStatus = {0};
        parseTestLoopbackData(p, (uint16_t)len, &testLoopbackStatus);
        Log("Group Delay  : %d\n", testLoopbackStatus.groupDelay);
        phOsalUwb_ProduceSemaphore(testLoopBackNtfSem);
#else
        // Handling for SR1xx/SR2xx
        phRfTestData_t rftestdata = {0};
        deserializeRfTestDataNtf(&rftestdata, pData, (uint16_t)len);

        phTest_Loopback_Ntf_t loopbackTestData = {0};
        /* Allocate the memory according to the PRF mode */
        uint8_t *psdu           = dataReceived;
        loopbackTestData.status = rftestdata.status;
        if (UWBAPI_STATUS_OK == rftestdata.status) {
            deserializeDataFromLoopbackNtf(&loopbackTestData, rftestdata.data, psdu);
            printLoopbackRecvData(&loopbackTestData);
        }
        phOsalUwb_ProduceSemaphore(testLoopBackNtfSem);
#endif // UWBIOT_UWBD_SR04X
        break;
    }
    case UWBD_RFRAME_DATA:
    case UWBD_DBG_DPD_INFO_NTF:
    case UWBD_CIR_PULL_DATA_NTF:
    case UWBD_COMMAND_TIMESTAMP_NTF:
    case UWBD_SCHEDULER_STATUS_NTF: {
        // Nothing to be done
    } break;
#if !(UWBIOT_UWBD_SR04X)
    case UWBD_PER_SEND: {
        Log("pPerTxData->status  : %d\n", ((phPerTxData_t *)pData)->status);
        phOsalUwb_ProduceSemaphore(perSem);
    } break;
#endif // !(UWBIOT_UWBD_SR04X)
#if !(UWBIOT_UWBD_SR04X)
    case UWBD_PER_RCV: {
        phRfTestData_t rftestdata = {0};
        deserializeRfTestDataNtf(&rftestdata, pData, (uint16_t)len);

        phTestPer_Rx_Ntf_t testrecvdata = {0};
        testrecvdata.status             = rftestdata.status;
        if (UWBAPI_STATUS_OK == rftestdata.status) {
            deserializeDataFromRxPerNtf(&testrecvdata, rftestdata.data);
            printPerRecvData(&testrecvdata);
        }
        phOsalUwb_ProduceSemaphore(perSem);
    } break;
    case UWBD_SR_RX_RCV: {
        phRfTestData_t rftestdata = {0};
        deserializeRfTestDataNtf(&rftestdata, pData, (uint16_t)len);

        phTest_Test_Sr_Ntf_t testSrRxData = {0};
        testSrRxData.Test_Sr_Ntf_status   = rftestdata.status;
        uint8_t *pPsdu                    = dataReceived;
        if (UWBAPI_STATUS_OK == rftestdata.status) {
            deserializeDataFromSrRxNtf(&testSrRxData, rftestdata.data, pPsdu, rftestdata.dataLength);
            printTestSrRecvData(&testSrRxData);
        }
    } break;
#endif // !(UWBIOT_UWBD_SR04X)
#if ((!(UWBIOT_UWBD_SR04X)) && UWBFTR_UWBS_DEBUG_Dump)
    case UWBD_CIR_DATA_NTF:
    case UWBD_DATA_LOGGER_NTF:
    case UWBD_RANGING_TIMESTAMP_NTF:
    case UWBD_PSDU_DATA_NTF: {
        if (opType == UWBD_CIR_DATA_NTF) {
            PRINTF("CIR data length :%zu\n", len);
            LOG_MAU8_D("CIR: ", (uint8_t *)pData, ((len >= 256) ? (256) : (len)));
        }
        else if (opType == UWBD_PSDU_DATA_NTF) {
            PRINTF("PSDU data length :%zu\n", len);
        }
        else if (opType == UWBD_RANGING_TIMESTAMP_NTF) {
            PRINTF("Ranging Timestamp data length :%zu\n", len);
        }
        else {
            /* do nothing */
        }
    } break;
#endif // ((!(UWBIOT_UWBD_SR04X)) && UWBFTR_UWBS_DEBUG_Dump)
    case UWBD_TEST_RX_RCV: {
#if !(UWBIOT_UWBD_SR04X)
        phRfTestData_t rftestdata = {0};
        deserializeRfTestDataNtf(&rftestdata, pData, (uint16_t)len);

        phTest_Rx_Ntf_t testrecvdata = {0};
        /* Allocate the memory according to the PRF mode */
        uint8_t *psdu       = dataReceived;
        testrecvdata.status = rftestdata.status;
        if (UWBAPI_STATUS_OK == rftestdata.status) {
            deserializeDataFromRxNtf(&testrecvdata, rftestdata.data, psdu);
            printrxRecvData(&testrecvdata);
        }
        phOsalUwb_ProduceSemaphore(perSem);

#endif // !(UWBIOT_UWBD_SR04X)
    } break;
#if !(UWBIOT_UWBD_SR04X)
    case UWBD_GENERIC_ERROR_NTF: {
        printGenericErrorStatus((phGenericError_t *)pData);
    } break;
#endif // UWBIOT_UWBD_SR04X

    case UWBD_DEVICE_RESET:  // Error Recovery: cleanup all states and end all states.
        cleanUpAppContext(); // This would have called while 1. SeComError 2. other reasons
        break;

    case UWBD_RECOVERY_NTF: {
        // Error Recovery: do uwbd cleanup, fw download, move to ready state
#if !(UWBIOT_UWBD_SR04X)
        phFwCrashLogInfo_t LogInfo;
        LogInfo.logLen = 255;
        LogInfo.pLog   = (uint8_t *)phOsalUwb_GetMemory((uint32_t)LogInfo.logLen * sizeof(uint8_t));
        UwbApi_GetFwCrashLog(&LogInfo);
        LOG_MAU8_I("Crash Log: ", LogInfo.pLog, LogInfo.logLen);
        phOsalUwb_FreeMemory(LogInfo.pLog);
#endif // !(UWBIOT_UWBD_SR04X)
        demo_recovery_handler(UWBD_RECOVERY_NTF);
    } break;

    case UWBD_SESSION_DATA: {
        phUwbSessionInfo_t *pSessionInfo = (phUwbSessionInfo_t *)pData;
        printSessionStatusData(pSessionInfo);
        if ((pSessionInfo->reason_code == UWB_SESSION_STOPPED_DUE_TO_INBAND_SIGNAL) ||
            pSessionInfo->reason_code == SESSION_STOPPED_DUE_TO_MAX_STS) {
            phOsalUwb_ProduceSemaphore(inBandterminationSem);
        }
#if UWBIOT_UWBD_SR100S
        else if ((pSessionInfo->state == UWB_SESSION_IDLE) &&
                 (pSessionInfo->reason_code == UWB_SESSION_STOPPED_DUE_TO_QOS_DECISION)) {
            /** The SESSION_STATUS_NTF with UWB_SESSION_IDLE following with the reason code
             * SESSION_STOPPED_DUE_TO_QOS_DECISION indicates that this session is not of it's transaction's intrest. Now
             * the Application can Deinit the corresponding session.
             */
            LOG_W("SESSION_STOPPED_DUE_TO_QOS_DECISION Received De-initializing Session:0x%X",
                pSessionInfo->sessionHandle);
            if (UwbApi_SessionDeinit(pSessionInfo->sessionHandle) != UWBAPI_STATUS_OK) {
                NXPLOG_APP_E("%s: UwbApi_SessionDeinit() Failed", __FUNCTION__);
            }
        }
#endif // UWBIOT_UWBD_SR100S
    } break;
    case UWBD_MULTICAST_LIST_NTF: {
#if !(UWBIOT_UWBD_SR04X)
        phMulticastControleeListNtfContext_t *pControleeNtfContext = (phMulticastControleeListNtfContext_t *)pData;
        printMulticastListStatus(pControleeNtfContext);
#endif // #if !(UWBIOT_UWBD_SR04X)
    } break;
    case UWBD_ACTION_APP_CLEANUP: {
        demo_recovery_handler(UWBD_ACTION_APP_CLEANUP);
    } break;
#if UWBFTR_DataTransfer
    case UWBD_DATA_TRANSMIT_NTF: {
        phUwbDataTransmit_t *pTransmitNtfContext = (phUwbDataTransmit_t *)pData;
        printTransmitStatus(pTransmitNtfContext);
        phOsalUwb_ProduceSemaphore(datatransferNtfSemTx);
    } break;
    case UWBD_CREDIT_RCV_NTF: {
        phUwbDataCredit_t *pCreditNtfContext = (phUwbDataCredit_t *)pData;
        printCreditStatus(pCreditNtfContext);
    } break;
    case UWBD_DATA_RCV_NTF: {
#if !(UWBIOT_UWBD_SR04X)
        phUwbRcvDataPkt_t pRcvDataPkt = {0};
        parseDataRcvNtf(pData, (uint16_t)len, &pRcvDataPkt);
        printRcvDataStatus(&pRcvDataPkt);
#endif
        (void)phOsalUwb_ProduceSemaphore(dataRcvNtfSemTx);
#if !(UWBIOT_UWBD_SR04X)
        if ((pRcvDataPkt.data_size != 0) && (pRcvDataPkt.data_size <= sizeof(dataReceived))) {
            phOsalUwb_MemCopy(dataReceived, pRcvDataPkt.data, pRcvDataPkt.data_size);
            rcvDataSz = pRcvDataPkt.data_size;
        }
        else {
            LOG_E("%s : Exceeding dataReceived buffer bounds", __FUNCTION__);
        }
#endif

    } break;
#endif // UWBFTR_DataTransfer
#if UWBFTR_Radar
    case UWBD_RADAR_RCV_NTF: {
        phUwbRadarNtf_t radar_notification = {0};
        /*This buffer later used for Pd/Bd Algo*/
        static uint8_t RadarNtfBuff[MAX_RADAR_LEN];
        parseRadarNtf(pData, (uint16_t)len, &radar_notification, &RadarNtfBuff[0]);
        /* UWBD_RADAR_RCV_NTF */
        /* Application/Demo needs to allocate the memory for CIR Data*/
        // TODO: to be fixed, as demos with limited Stack size can fail.
        printRadarRecvNtf(&radar_notification);
#if (PRESENCE_DETECTION)
        sendPresenceDetectionData(RadarNtfBuff, (uint32_t)radar_notification.radar_ntf.radr_cir.cir_len);
#endif // PRESNECE_DETECTION

    } break;
    case UWBD_TEST_RADAR_ISO_NTF: {
        phUwbRadarNtf_t radar_notification = {0};
        parseRadarNtf(pData, (uint16_t)len, &radar_notification, NULL);
        /* UWBD_TEST_RADAR_ISO_NTF */
        printRadarTestIsoNtf(&radar_notification);
        (void)phOsalUwb_ProduceSemaphore(RadarTstAntIsoNtfSemTx);
    } break;
    case UWBD_PRESENCE_DETECTION_NTF: {
        phUwbRadarNtf_t radar_notification = {0};
        parseRadarNtf(pData, (uint16_t)len, &radar_notification, NULL);
        /* UWBD_PRESENCE_DETECTION_NTF */
        printRadarPresenceDetctionNtf(&radar_notification);
    } break;
#endif // UWBFTR_Radar

#if !(UWBIOT_UWBD_SR04X)
    case UWBD_WIFI_WLAN_IND_NTF: {
        UWB_Wlan_IndNtf_t *pUWB_Wlan_IndNtf = (UWB_Wlan_IndNtf_t *)pData;
        printUwbWlanIndNtf(pUWB_Wlan_IndNtf);
    } break;
    case UWB_WLAN_COEX_MAX_GRANT_DURATION_EXCEEDED_WRN_NTF: {
        LOG_D("Max Active Grant Duration Status : %X", *(uint8_t *)pData);
    } break;
#if !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)
    case WLAN_UWB_IND_NTF: {
        Wlan_Uwb_IndNtf_t *pWlan_Uwb_IndNtf = (Wlan_Uwb_IndNtf_t *)pData;
        printWlanUwbIndNtf(pWlan_Uwb_IndNtf);
    } break;
#endif // !(UWBIOT_UWBD_SR250 || UWBIOT_UWBD_SR150)
#endif // !(UWBIOT_UWBD_SR04X)

#if UWBFTR_CCC || UWBFTR_CSA
    case UWBD_RANGING_CCC_DATA: {
#if UWBFTR_CSA
        if (IS_CSA_SESSION(pData)) {
            phCsaRangingData_t csaRangingData = {0};
            parseCsaRangingNtf(pData, (uint16_t)len, &csaRangingData);
            printCsaRangingData(&csaRangingData);
        }
#endif // UWBFTR_CSA
#if UWBFTR_CCC && !(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
        if (IS_CCC_SESSION(pData)) {
            phCccRangingData_t cccRangingData = {0};
            parseCccRangingNtf(pData, (uint16_t)len, &cccRangingData);
            printCccRangingData(&cccRangingData);
        }
#endif // UWBFTR_CCC && !(UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR250)
    } break;
#endif // UWBFTR_CCC || UWBFTR_CSA

#if !(UWBIOT_UWBD_SR04X)
    case UWBD_DATA_TRANSFER_PHASE_CONFIG_NTF: {
        phDataTxPhaseCfgNtf_t *pDataTxPhCfgNtf = (phDataTxPhaseCfgNtf_t *)pData;
        printDataTxPhaseCfgNtf(pDataTxPhCfgNtf);
    } break;
#endif // !(UWBIOT_UWBD_SR04X)
#if UWBFTR_DataTransfer
    case UWBD_LL_CREATE_NTF: {
        phLogicalLinkCreateNtf_t *pLLCreateNtf = (phLogicalLinkCreateNtf_t *)pData;
        printLogicalLinkCreateNtf(pLLCreateNtf);
        /** Application can use this semaphore to indicate that the Link has been created with the Remote device,
         *  and now the Data can be transfered.
         **/
        if (pLLCreateNtf->status == LOGICAL_CO_LINK_CONNECTED) {
            (void)phOsalUwb_ProduceSemaphore(llCreateStatusSem);
        }

    } break;
    case UWBD_LL_UWBS_CLOSE_NTF: {
        phLogicalLinkUwbsCloseNtf_t *pLLCloseNtf = (phLogicalLinkUwbsCloseNtf_t *)pData;
        printLogicalLinkUwbsCloseNtf(pLLCloseNtf);
        phOsalUwb_ProduceSemaphore(llUwbsCloseNtf);
    } break;
    case UWBD_LL_UWBS_CREATE_NTF: {
        phLogicalLinkUwbsCreateNtf_t *pLLUwbsCreateNtf = (phLogicalLinkUwbsCreateNtf_t *)pData;
        printLogicalLinkUwbsCreateNtf(pLLUwbsCreateNtf);
        LOG_D("UWBD_LL_UWBS_CREATE_NTF Update Link Layer Status");

    } break;
#if !(UWBIOT_UWBD_SR04X)
    case UWBD_LOGICAL_LINK_DATA_RCV_NTF: {
        phLogicalLinkDataPkt_t pRcvDataPkt = {0};
        pRcvDataPkt.data                   = dataReceived;
        parseLogicalDataRcvNtf(pData, (uint16_t)len, &pRcvDataPkt);
        printRcvLogicalDataStatus(&pRcvDataPkt);
        rcvDataSz = pRcvDataPkt.data_size;
        (void)phOsalUwb_ProduceSemaphore(dataRcvNtfSemTx);
    } break;
#endif // !(UWBIOT_UWBD_SR04X)
#endif // UWBFTR_DataTransfer
    case UWBD_SESSION_ROLE_CHANGE_RCV_NTF: {
#if !(UWBIOT_UWBD_SR04X)
        phNewRoleChangeNtf_t pNewRole = {0};
        parseNewRoleChangeRcvNtf(pData, (uint16_t)len, &pNewRole);
        printNewRoleChangeRcvNtf(&pNewRole);
#endif // UWBIOT_UWBD_SR04X
    } break;
    case UWBD_SESSION_ERROR_TIMEOUT_NTF: {
        demo_recovery_handler(UWBD_SESSION_ERROR_TIMEOUT_NTF);
    } break;

    default:
        LOG_W("%s : Unregistered Event : 0x%X ", __FUNCTION__, opType);
        break;
    }
    /* Free pData Memory*/
    phOsalUwb_FreeMemory(pData);
}

static void demo_recovery_handler(eNotificationType opType)
{
    switch (opType) {
    case UWBD_SESSION_ERROR_TIMEOUT_NTF:
    case UWBD_ACTION_APP_CLEANUP:
    case UWBD_RECOVERY_NTF:
        Log("Recovery Started for Scenario : %d\n", opType);
        if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
            LOG_E("recovery handler : UwbApi_ShutDown failed");
        }
#if UWBIOT_OS_NATIVE
        isRecovery = true;
        pthread_cancel(testTaskHandle);
#elif UWBIOT_OS_FREERTOS
        phOsalUwb_Thread_Delete(testTaskHandle);
        testTaskHandle = uwb_demo_start();
#endif
        break;
    default:
        Log("Recovery Started for Default Scenario. \n");
        break;
    }
}

void demo_handle_error_scenario(eNotificationType opType)
{
    phLibUwb_Message_t evt = {0};
    evt.eMsgType           = opType;
    evt.pMsgData           = NULL;
    evt.Size               = 0;
    (void)phOsalUwb_msgsnd(demoNtfMngQueue, &evt, NO_DELAY);
}

static OSAL_TASK_RETURN_TYPE demo_ntf_handling_task(void *args)
{
    LOG_D("Started demo_ntf_handling_task");
    /* main loop */
    while (1) {
        phLibUwb_Message_t evt = {0};

        if (phOsalUwb_msgrcv(demoNtfMngQueue, &evt, MAX_DELAY) == UWBSTATUS_FAILED) {
            NXPLOG_APP_D("%s : msgrcv timeout!!!", __FUNCTION__);
            continue;
        }
        demo_handle_ntf(&evt);
    }

    (void)demo_ntf_handling_task_stop();
}

/*
 * .
 */
void demo_ntf_handling_task_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;
    UWBIOT_STACK_DEFINE(demo_ntf_handling_task_stack, DEMO_NTF_HANDLE_TASK_SIZE);

    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_NTF_HANDLE_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_NTF_HANDLE_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack     = (k_thread_stack_t *)&demo_ntf_handling_task_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(demo_ntf_handling_task_stack);
#else
    threadparams.stackdepth = DEMO_NTF_HANDLE_TASK_SIZE;
#endif
    demoNtfMngQueue = phOsalUwb_msgget(20);

    if (!demoNtfMngQueue) {
        NXPLOG_APP_E("Failed to create demoNtfMngQueue %s", threadparams.taskname);
    }
    pthread_create_status =
        phOsalUwb_Thread_Create((void **)&demoNtfHandlingTaskHandle, &demo_ntf_handling_task, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
}

void demo_ntf_handling_task_stop(void)
{
    (void)phOsalUwb_Thread_Delete(demoNtfHandlingTaskHandle);
    /* demoNtfMngQueue needs to release after deleting demoNtfHandlingTaskHandle
     *  otherwise outside the task resources are not accessible.
     */
    phOsalUwb_msgrelease(demoNtfMngQueue);
}

void AppCallback(eNotificationType opType, void *pData, size_t len)
{
    static phLibUwb_Message_t pSessionNtfInfo = {0};

    pSessionNtfInfo.eMsgType = opType;
    pSessionNtfInfo.Size     = (uint16_t)len;
    pSessionNtfInfo.pMsgData = (void *)phOsalUwb_GetMemory(pSessionNtfInfo.Size * sizeof(uint8_t));
    if (pSessionNtfInfo.pMsgData != NULL) {
        phOsalUwb_MemCopy((uint8_t *)pSessionNtfInfo.pMsgData, pData, pSessionNtfInfo.Size);
        (void)phOsalUwb_msgsnd(demoNtfMngQueue, &pSessionNtfInfo, MAX_DELAY);
    }
    else {
        NXPLOG_APP_E("AppCallback: Unable to Allocate Memory of %d, Memory Full:\n", pSessionNtfInfo.Size);
    }
}

#endif // #if !defined(UWBIOT_APP_BUILD__DEMO_CDC)
