/* Copyright 2025 NXP
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
#ifndef UWBIOT_APP_BUILD__DEMO_SR2XX_FW_UPDATE
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_SR2XX_FW_UPDATE

#if UWBIOT_OS_ZEPHYR
/**
 * @brief Macro to define task stack size in bytes.
 */
#define DEMO_FW_UPDATE_TASK_STACK_SIZE 1800
#else
#define DEMO_FW_UPDATE_TASK_STACK_SIZE 1024
#endif // UWBIOT_OS_ZEPHYR

/*
 * Below list contains the application configs which are only related to default
 * configuration.
 */

/********************************************************************************/
/*               Ranging APP configuration setting here */
/********************************************************************************/

#define DEMO_SR2XX_FW_DOWNLOAD_TASK_SIZE DEMO_FW_UPDATE_TASK_STACK_SIZE
#define DEMO_SR2XX_FW_DOWNLOAD_TASK_NAME "demoFirmwareDownload"
#define DEMO_SR2XX_FW_DOWNLOAD_TASK_PRIO 4
/********************************************************************************/

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    phUwbDevInfo_t devInfo;

    PRINT_APP_NAME("demo_sr2xx_fw_download");

    phUwbappContext_t appCtx        = {0};
    appCtx.fwImageCtx.fwImage       = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize     = heliosEncryptedMainlineFwImageLen;
    appCtx.fwImageCtx.patchVerion   = heliosEncryptedMainlineFwImagePatchVer;
    appCtx.fwImageCtx.fwMode        = MAINLINE_FW;
    appCtx.fwImageCtx.forceFwUpdate = FALSE;
    appCtx.pCallback                = AppCallback;
    appCtx.pTmlCallback             = NULL;
    status                          = UwbApi_Initialize(&appCtx);
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
    UWBIOT_STACK_DEFINE(DemofwUpdate_stack, DEMO_SR2XX_FW_DOWNLOAD_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_SR2XX_FW_DOWNLOAD_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_SR2XX_FW_DOWNLOAD_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack     = (k_thread_stack_t *)&DemofwUpdate_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(DemofwUpdate_stack);
#else
    threadparams.stackdepth = DEMO_SR2XX_FW_DOWNLOAD_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_SR2XX_FW_UPDATE