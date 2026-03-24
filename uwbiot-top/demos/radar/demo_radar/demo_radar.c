/* Copyright 2022-2025 NXP
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

#ifndef UWBIOT_APP_BUILD__DEMO_RADAR
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_RADAR

#if (PRESENCE_DETECTION)
#error "Disable PRESENCE_DETECTION "
#endif

/*
 * Below list contains the application configs which are only related to default
 * configuration.
 */

/********************************************************************************/
/*               Radar APP configuration setting here */
/********************************************************************************/

#if UWBIOT_OS_ZEPHYR
/**
 * @brief Macro to define task stack size in bytes.
 */
#define DEMO_RADAR_TASK_STACK_SIZE 1800
#else
#define DEMO_RADAR_TASK_STACK_SIZE 400
#endif // UWBIOT_OS_ZEPHYR

#define DEMO_RADAR_APP_SESSION_ID 0x11223344

#define DEMO_RADAR_TASK_SIZE DEMO_RADAR_TASK_STACK_SIZE
#define DEMO_RADAR_TASK_NAME "DemoRadar"
#define DEMO_RADAR_TASK_PRIO 4

/*Radar ANTENNAE_CONFIGURATION_RX setting*/
/* Single RX on RXB  = ANT3_FLEX_FR (Horizontal Antenna Patch) */

#define DEMO_RADAR_MODE_RX          0x02
#define DEMO_RADAR_LENGTH_RX        0x03
#define DEMO_SET_APP_CONFIG_ANT_RX1 0x00 // RX1=RXC Ant ID
#define DEMO_SET_APP_CONFIG_ANT_RX2 0x02 // RX2=RXB Ant ID
#define DEMO_SET_APP_CONFIG_ANT_RX3 0x00 // RX3=RXA Ant ID

/*Radar ANTENNAE_CONFIGURATION_TX setting*/
/*TX on TRA1 (External Taoglas Antenna)*/
#define DEMO_SET_APP_CONFIG_LENGTH_TX 0x01
#define DEMO_SET_APP_CONFIG_TX_ANT_ID 0x01

/* Octet[3:0]: RANGING_INTERVAL*/
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_0 0x64
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_1 0x00
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_2 0x00
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_3 0x00
/*Octet[5:4]: SLOT_DURATION*/
#define DEMO_SET_VENDOR_APP_RFRI_SLOT_DURATION_OCTET_0 0xE0
#define DEMO_SET_VENDOR_APP_RFRI_SLOT_DURATION_OCTET_1 0x2E
/*Octet[6]: SLOTS_PER_RR*/
#define DEMO_SET_VENDOR_APP_RFRI_DEMO_SLOTS_PER_RR 0x02

/********************************************************************************/

AppContext_t appContext;

#pragma message("set USE_BARE_BOARD to 0 in respective Host Specific UWB_DeviceConfig.h ")

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    phUwbappContext_t appCtx = {0};
    phUwbDevInfo_t devInfo   = {0};
    uint32_t sessionHandle   = 0;
    uint32_t delay;

    PRINT_APP_NAME("Demo Radar");

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

    status = UwbApi_SessionInit(DEMO_RADAR_APP_SESSION_ID, UWBD_RADAR_TRANSFER, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    const uint8_t antennaeConfigurationRx[] = {/* */
        DEMO_RADAR_MODE_RX,
        DEMO_RADAR_LENGTH_RX,
        DEMO_SET_APP_CONFIG_ANT_RX1,
        DEMO_SET_APP_CONFIG_ANT_RX2,
        DEMO_SET_APP_CONFIG_ANT_RX3};

    const uint8_t antennaeConfigurationTx[] = {DEMO_SET_APP_CONFIG_LENGTH_TX, DEMO_SET_APP_CONFIG_TX_ANT_ID};

    const uint8_t rfri[] = {DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_0,
        DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_1,
        DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_2,
        DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_3,
        DEMO_SET_VENDOR_APP_RFRI_SLOT_DURATION_OCTET_0,
        DEMO_SET_VENDOR_APP_RFRI_SLOT_DURATION_OCTET_1,
        DEMO_SET_VENDOR_APP_RFRI_DEMO_SLOTS_PER_RR};

    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, 26),
    };
    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            ANTENNAE_CONFIGURATION_TX, &antennaeConfigurationTx[0], sizeof(antennaeConfigurationTx)),
        UWB_SET_VENDOR_APP_PARAM_VALUE(RADAR_MODE, kUWB_RadarMode_Medium_Distance),
        UWB_SET_VENDOR_APP_PARAM_VALUE(RADAR_SINGLE_FRAME_NTF, 1),
        UWB_SET_VENDOR_APP_PARAM_ARRAY(RADAR_RFRI, &rfri[0], sizeof(rfri)),
    };

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfigMultipleParams() Failed");
    }

    status = UwbApi_SetVendorAppConfigs(
        sessionHandle, sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]), &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    delay = 5 * 60 * 1000; /*Waiting for 5 mins*/

    /* When Radar session is terminated due to FCC, session will stop and this semaphore will
     * be signaled, otherwise ranging will be performed for the time specified */
    if (UWBSTATUS_SUCCESS == phOsalUwb_ConsumeSemaphore_WithTimeout(inBandterminationSem, delay)) {
        status = UWBAPI_STATUS_OK;
        NXPLOG_APP_I(
            "\n-------------------------------------------\n UWB SESSION RADAR FCC LIMIT REACHED "
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
    UWBIOT_STACK_DEFINE(DemoRadar_stack, DEMO_RADAR_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_RADAR_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_RADAR_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack     = (k_thread_stack_t *)&DemoRadar_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(DemoRadar_stack);
#else
    threadparams.stackdepth = DEMO_RADAR_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_RADAR
