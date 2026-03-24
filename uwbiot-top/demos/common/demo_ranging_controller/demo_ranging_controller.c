/* Copyright 2021-2025 NXP
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

#include "UwbApi.h"
#include <AppInternal.h>
#ifndef UWBIOT_APP_BUILD__DEMO_RANGING_CONTROLLER
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_RANGING_CONTROLLER

/*
 * Below list contains the application configs which are only related to default
 * configuration.
 */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#if UWBIOT_OS_ZEPHYR
/**
 * @brief Macro to define task stack size in multiples of 256 bytes.
 *
 * @param x Multiplier value. The final stack size will be (256 * x) bytes.
 *          For example, TASK_STACK_SIZE(4) = 1024 bytes.
 */
#define DEMO_RNG_CONTROLLER_TASK_STACK_SIZE 1200
#else
#define DEMO_RNG_CONTROLLER_TASK_STACK_SIZE 400
#endif // UWBIOT_OS_ZEPHYR

#define DEMO_RANGING_APP_SESSION_ID 0x11223344

#define DEMO_RANGING_APP_NO_OF_ANCHORS_P2P 1
#define DEMO_RNG_CONTROLLER_TASK_SIZE      DEMO_RNG_CONTROLLER_TASK_STACK_SIZE
#define DEMO_RNG_CONTROLLER_TASK_NAME      "DemoRngController"
#define DEMO_RNG_CONTROLLER_TASK_PRIO      4

#include "app_Ranging_Cfg.h"
#define DEMO_MAC_ADDR_MODE SHORT_MAC_ADDRESS_MODE
#define DEMO_MAC_ADDR_LEN  SHORT_MAC_ADDR_LEN

static const uint8_t gkDeviceMacAddr[] = {RANGING_APP_MAC_ADDR_CTRL};
static const uint8_t gkDstMacAddr[]    = {RANGING_APP_MAC_ADDR_CLEE};

/********************************************************************************/

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status     = UWBAPI_STATUS_FAILED;
    const uint32_t session_id = DEMO_RANGING_APP_SESSION_ID;
    uint32_t sessionHandle    = 0;
    phUwbDevInfo_t devInfo;
    phRangingParams_t inRangingParams = {0};
    uint32_t delay;

    const UWB_AppParams_List_t SetAppParamsList[] = {

        UWB_SET_APP_PARAM_VALUE(NO_OF_CONTROLEES, DEMO_RANGING_APP_NO_OF_ANCHORS_P2P),
        UWB_SET_APP_PARAM_ARRAY(DST_MAC_ADDRESS, gkDstMacAddr, DEMO_MAC_ADDR_LEN),
    };

    PRINT_APP_NAME("Demo Ranging Controller");

    phUwbappContext_t appCtx = {0};
#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = MAINLINE_FW;
    appCtx.pCallback         = AppCallback;
    appCtx.pTmlCallback      = NULL;

    status = UwbApi_Initialize(&appCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Initialize Failed");
        goto exit;
    }

    status = UwbApi_GetDeviceInfo(&devInfo);
    printDeviceInfo(&devInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetDeviceInfo() Failed");
        goto exit;
    }

    /** OTP read write is enabled for SR150 and SR100S ROW
    Not Required for SR100T Mobile FW */

#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S
    status = demo_configure_otp_calibration(CHANNEL_5);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("demo_configure_otp_calibration(CHANNEL_5) Failed");
        goto exit;
    }

    status = demo_configure_otp_calibration(CHANNEL_9);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("demo_configure_otp_calibration(CHANNEL_9) Failed");
        goto exit;
    }
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S

    status = UwbApi_SessionInit(session_id, UWBD_RANGING_SESSION, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    inRangingParams.deviceRole        = kUWB_DeviceRole_Initiator;
    inRangingParams.multiNodeMode     = kUWB_MultiNodeMode_UniCast;
    inRangingParams.macAddrMode       = DEMO_MAC_ADDR_MODE;
    inRangingParams.deviceType        = kUWB_DeviceType_Controller;
    inRangingParams.deviceMacAddr[0]  = gkDeviceMacAddr[0];
    inRangingParams.deviceMacAddr[1]  = gkDeviceMacAddr[1];
    inRangingParams.scheduledMode     = kUWB_ScheduledMode_TimeScheduled;
    inRangingParams.rangingRoundUsage = kUWB_RangingRoundUsage_DS_TWR;

    status = UwbApi_SetRangingParams(sessionHandle, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    status = demo_set_common_app_config(sessionHandle, kUWB_StsConfig_StaticSts);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("demo_set_common_app_config() Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    /* Delay 5 Mins for Ranging MILLISECONDS = MINUTES * 60 * 1000 */
    delay = 5 * 60 * 1000; /*Waiting for 5 mins*/

    /* When Ranging is terminated due to inband termination this semaphore will
     * be signaled, otherwise ranging will be performed for the time specified */
    if (UWBSTATUS_SUCCESS == phOsalUwb_ConsumeSemaphore_WithTimeout(inBandterminationSem, delay)) {
        status = UWBAPI_STATUS_OK;
        NXPLOG_APP_I(
            "\n-------------------------------------------\n in band termination "
            "is done  \n-------------------------------------------\n");
        UwbApi_SessionDeinit(sessionHandle);
        goto exit;
    }

    status = UwbApi_StopRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopRangingSession() Failed");
        goto exit;
    }

    status = UwbApi_SessionDeinit(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionDeinit() Failed");
        goto exit;
    }

exit:
    if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ShutDown Failed");
    }

    if (status == UWBAPI_STATUS_TIMEOUT) {
        demo_handle_error_scenario(UWBD_SESSION_ERROR_TIMEOUT_NTF);
    }

    UWBIOT_EXAMPLE_END(status);
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    UWBIOT_STACK_DEFINE(DemoRngCntlr_stack, DEMO_RNG_CONTROLLER_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_RNG_CONTROLLER_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_RNG_CONTROLLER_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack     = (k_thread_stack_t *)&DemoRngCntlr_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(DemoRngCntlr_stack);
#else
    threadparams.stackdepth = DEMO_RNG_CONTROLLER_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_RANGING_CONTROLLER
