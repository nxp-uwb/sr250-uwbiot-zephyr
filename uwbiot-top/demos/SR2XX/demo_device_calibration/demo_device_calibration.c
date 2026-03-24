/* Copyright 2024-2026 NXP
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

#include <AppInternal.h>

#ifndef UWBIOT_APP_BUILD__DEMO_DEVICE_CALIBRATION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_DEVICE_CALIBRATION
#include "UWB_DeviceCalib_values.h"

/* Function Prototypes */
static UWBStatus_t demo_GetSetDeviceCalib(hostDeviceCalib_t *pDemoDevCalib);
static UWBStatus_t demo_InitializeDevice(phUwbappContext_t *appCtx, phUwbDevInfo_t *devInfo);
static UWBStatus_t demo_CheckAndPerformChipCalibration(
    uint8_t channel, const hostDeviceCalib_t *calibArray, size_t arraySize);

static uint8_t getCalibVal[HOST_CALIB_VALUE_SZ];

/**
 * @brief Channel calibration configuration table
 */
static const channelCalibConfig_t channelConfigs[] = {GET_CHANNEL_CONFIGS};


/*
 * Below list contains the application configs which are only related to default
 * configuration.
 */

/********************************************************************************/
#if UWBIOT_OS_ZEPHYR
/**
 * @brief Macro to define task stack size in bytes.
 */
#define DEMO_DEVICE_CALIBRATION_TASK_SIZE  1800
#else
#define DEMO_DEVICE_CALIBRATION_TASK_SIZE  512
#endif // UWBIOT_OS_ZEPHYR
#define DEMO_DEVICE_CALIBRATION_TASK_NAME "DemoDeviceCalib"
#define DEMO_DEVICE_CALIBRATION_TASK_PRIO 4

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status    = UWBAPI_STATUS_FAILED;
    phUwbappContext_t appCtx = {0};
    phUwbDevInfo_t devInfo   = {0};

    PRINT_APP_NAME("Demo Device Calibration");

    // Initialize UWB device
    status = demo_InitializeDevice(&appCtx, &devInfo);
    if (status != UWBAPI_STATUS_OK) {
        goto exit;
    }

    // Process calibration for all configured channels
    for (size_t i = 0; i < GET_ARRAY_SIZE(channelConfigs); i++) {
        const channelCalibConfig_t *config = &channelConfigs[i];

        LOG_I("Processing calibration for %s", config->channelName);

        status = demo_CheckAndPerformChipCalibration(config->channel, config->calibArray, config->arraySize);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Failed to process calibration for %s", config->channelName);
            goto exit;
        }
    }
    LOG_I("Device calibration demo completed successfully");

exit:
    if (UwbApi_ShutDown() != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ShutDown Failed");
    }

    if (status == UWBAPI_STATUS_TIMEOUT) {
        demo_handle_error_scenario(UWBD_SESSION_ERROR_TIMEOUT_NTF);
    }

    UWBIOT_EXAMPLE_END(status);
}


/**
 * @brief Initialize UWB device and get device information
 *
 * @param appCtx Application context to initialize
 * @param devInfo Device information structure to populate
 * @return UWBStatus_t Status of initialization
 */
static UWBStatus_t demo_InitializeDevice(phUwbappContext_t *appCtx, phUwbDevInfo_t *devInfo)
{
    UWBStatus_t status;

#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx->fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx->fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx->fwImageCtx.fwMode = MAINLINE_FW;
    appCtx->pCallback         = AppCallback;
    appCtx->pTmlCallback      = NULL;

    status = UwbApi_Initialize(appCtx);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Initialize Failed");
        return status;
    }

    status = UwbApi_GetDeviceInfo(devInfo);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetDeviceInfo() Failed");
        return status;
    }

    printDeviceInfo(devInfo);
    LOG_W("LifeCycle :0x%4X\n", devInfo->lifecycle);

    return UWBAPI_STATUS_OK;
}

/**
 * @brief Check calibration state and perform chip calibration if needed
 *
 * @param channel Channel to check calibration for
 * @return UWBStatus_t Status of calibration check/operation
 */
static UWBStatus_t demo_CheckAndPerformChipCalibration(
    uint8_t channel, const hostDeviceCalib_t *calibArray, size_t arraySize)
{
    UWBStatus_t status;
    phGetCalibInputParams_t getDevCalibState = {0x00};
    phCalibRespStatus_t calibStateResp       = {0x00};
    uint8_t calibState                       = 0x00;

    // Check current calibration state
    getDevCalibState.paramId         = CHIP_CALIBRATION_STATE;
    getDevCalibState.channel         = channel;
    calibStateResp.pCalibrationValue = &calibState;

    status = UwbApi_GetCalibration(&getDevCalibState, &calibStateResp);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetCalibration for CHIP_CALIBRATION_STATE Failed");
        return status;
    }

    if (calibState == eCalibState_DEVICE_NOT_CALIBRATED) {
        LOG_I("Device not calibrated for channel %d, performing chip calibration", channel);

        phDoCalibNtfStatus_t doCalibNtfStatus = {0x00};
        status                                = UwbApi_DoChipCalibration(channel, &doCalibNtfStatus);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_DoChipCalibration Failed for channel %d", channel);
            return status;
        }

        LOG_I("Chip calibration completed for channel %d\n", channel);
    }
    else {
        LOG_I("Device is already calibrated for channel %d\n", channel);
    }

    // Always apply all calibration parameters for this channel
    LOG_I("Applying all calibration parameters for channel %d", channel);
    for (size_t i = 0; i < arraySize; i++) {
        status = demo_GetSetDeviceCalib((hostDeviceCalib_t *)&calibArray[i]);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("demo_GetSetDeviceCalib() to set %s Failed", calibArray[i].paramName);
            return status;
        }
    }

    return UWBAPI_STATUS_OK;
}
static UWBStatus_t demo_GetSetDeviceCalib(hostDeviceCalib_t *pDemoDevCalib)
{
    UWBStatus_t status;
    phCalibRespStatus_t calibResp      = {0x00};
    phGetCalibInputParams_t getCalibIn = {0};

    getCalibIn.channel = pDemoDevCalib->channelId;
    getCalibIn.paramId = pDemoDevCalib->paramId;
#if UWBFTR_AoA_FoV
    if (pDemoDevCalib->paramId == AOA_ANTENNAS_PDOA_CALIB ||
        pDemoDevCalib->paramId == AOA_ANTENNAS_PDOA_CALIB_EXTENDED_SUPPORT) {
        getCalibIn.rxAntennaPairID = pDemoDevCalib->pCalibValue[HOST_RX_PAIR_OFFSET];
        if (getCalibIn.rxAntennaPairID != AD_RX_ID(1) && getCalibIn.rxAntennaPairID != AD_RX_ID(2)) {
            NXPLOG_APP_E("AntennaPairId :%d cannot be stored in flash", getCalibIn.rxAntennaPairID);
            status = UWBAPI_STATUS_INVALID_PARAM;
            goto exit;
        }
    }
#endif // UWBFTR_AoA_FoV

    /** Calibrations not matching, hence Set*/
    LOG_W("Setting : %s", pDemoDevCalib->paramName);
    status = UwbApi_SetCalibration(pDemoDevCalib->channelId,
        pDemoDevCalib->paramId,
        (uint8_t *)pDemoDevCalib->pCalibValue,
        pDemoDevCalib->calibLen);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetCalibration for %s Failed", pDemoDevCalib->paramName);
        goto exit;
    }

    /** Get the Calibrations set and validate */
    LOG_W("Getting : %s", pDemoDevCalib->paramName);
    calibResp.pCalibrationValue = &getCalibVal[0];
    status                      = UwbApi_GetCalibration(&getCalibIn, &calibResp);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_GetCalibration for %s Failed", pDemoDevCalib->paramName);
        goto exit;
    }
    if (phOsalUwb_MemCompare(&pDemoDevCalib->pCalibValue[0],
            calibResp.pCalibrationValue,
            calibResp.calibrationValuelength) != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Validation of %s Failed", pDemoDevCalib->paramName);

        /* Calibration which was set */
        LOG_MAU8_W("Set calib", pDemoDevCalib->pCalibValue, pDemoDevCalib->calibLen);
        /* Calibration which was received */
        LOG_MAU8_E("Get calib", calibResp.pCalibrationValue, calibResp.calibrationValuelength);
        goto exit;
    }

exit:
    return status;
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    UWBIOT_STACK_DEFINE(DemoDevice_Calib_stack, DEMO_DEVICE_CALIBRATION_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_DEVICE_CALIBRATION_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_DEVICE_CALIBRATION_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack     = (k_thread_stack_t *)&DemoDevice_Calib_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(DemoDevice_Calib_stack);
#else
    threadparams.stackdepth = DEMO_DEVICE_CALIBRATION_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_DEVICE_CALIBRATION
