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
#include "UwbApi.h"
#include <AppInternal.h>

#ifndef UWBIOT_APP_BUILD__DEMO_OCPD
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_OCPD

#if (PRESENCE_DETECTION)
#error "Disable PRESENCE_DETECTION "
#endif

/*
 * Below list contains the application configs which are only related to default
 * configuration.
 */

/********************************************************************************/
/*               Radar OCPD APP configuration setting here */
/********************************************************************************/

#if UWBIOT_OS_ZEPHYR
/**
 * @brief Macro to define task stack size in bytes.
 */
#define DEMO_RADAR_OCPD_RANGE_TASK_STACK_SIZE 1800
#else
#define DEMO_RADAR_OCPD_RANGE_TASK_STACK_SIZE 400
#endif // UWBIOT_OS_ZEPHYR

#define DEMO_OCPD_APP_SESSION_ID 0x11223344

#define DEMO_OCPD_TASK_SIZE DEMO_RADAR_OCPD_RANGE_TASK_STACK_SIZE
#define DEMO_OCPD_TASK_NAME "DemoOcpd"
#define DEMO_OCPD_TASK_PRIO 4

/*Radar ANTENNAE_CONFIGURATION_RX setting*/
/* Single RX on RXB  = ANT3_FLEX_FR (Horizontal Antenna Patch) */

#define DEMO_RADAR_MODE_RX          0x02
#define DEMO_RADAR_LENGTH_RX        0x03
#define DEMO_SET_APP_CONFIG_ANT_RX1 0x01 // RX1=RXC Ant ID
#define DEMO_SET_APP_CONFIG_ANT_RX2 0x02 // RX2=RXB Ant ID
#define DEMO_SET_APP_CONFIG_ANT_RX3 0x00 // RX3=RXA Ant ID

/*Radar ANTENNAE_CONFIGURATION_TX setting*/
/*TX on TRA1 (External Taoglas Antenna)*/
#define DEMO_SET_APP_CONFIG_LENGTH_TX 0x01
#define DEMO_SET_APP_CONFIG_TX_ANT_ID 0x01

/* Octet[3:0]: RANGING_INTERVAL*/
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_0 0x32
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_1 0x00
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_2 0x00
#define DEMO_SET_VENDOR_APP_RFRI_RANGING_INTERVAL_OCTET_3 0x00
/*Octet[5:4]: SLOT_DURATION*/
#define DEMO_SET_VENDOR_APP_RFRI_SLOT_DURATION_OCTET_0 0xE0
#define DEMO_SET_VENDOR_APP_RFRI_SLOT_DURATION_OCTET_1 0x2E
/*Octet[6]: SLOTS_PER_RR*/
#define DEMO_SET_VENDOR_APP_RFRI_DEMO_SLOTS_PER_RR 0x01

/**
 * UTILITY MACROS
 */

#define DEC_TO_Q4_4(dec)           ((int)((dec)*16))
#define DEC_TO_HEX_OCTETS_LSB(dec) (((dec) >> 8) & 0xFF)
#define DEC_TO_HEX_OCTETS_MSB(dec) ((dec)&0xFF)
/**
 * @brief Converts a floating-point value to Q1.15 fixed-point format.
 *
 * Q1.15 format uses 1 sign bit and 15 fractional bits.
 * The conversion multiplies the float by 32768 (2^15) and rounds to the nearest integer.
 * This macro is useful for setting values like drift compensation in fixed-point format.
 *
 * Example:
 *   Input:  0.1
 *   Step 1: 0.1 * 32768 = 3276.8
 *   Step 2: Round to nearest integer → 3277
 *   Step 3: Convert to hex → 0x0CCD
 *
 * @param val Floating-point value (e.g., 0.1)
 * @return uint16_t Hex representation in Q1.15 format
 */
#define TO_Q15_HEX(val) ((uint16_t)((val) * 32768.0f + ((val) >= 0 ? 0.5f : -0.5f)))
/**
 * @brief
 * 1-Convert to binary: Absolute value of 90 is `1011010`.
 * 2-Inverting each bit: Inverting each bit results in `0100101`.
 * 3-Add 1 : Adding 1 to `0100101` yields `0100110`.
 * 4-Hexadecimal conversion: `0100110` in binary is `26` in hexadecimal. Since it's negative, the leading bit is 1, so
 *   the hexadecimal representation becomes `0xA6`.
 *
 * Ex: 256-90 = 166(0xA6)
 */

#define DEC_TO_HEX(n) ((n < 0) ? (256 + n) : n)

/* Radar Presence Detection config values.
 * Edit Config Values here.
 *
*/
#define PRESENCE_DETECTION_MODE_ENABLE 0x03
/**
 * @brief Octet[1]: Periodic data reporting
            -Bit 0: sending raw CIRs to the host
            -Bit 2-1:  00: no periodic presence reporting
              - 01:  presence reporting every 50ms
              - 10: presence reporting every 400ms
              - 11: presence reporting every 1600ms
            (Default: 0:Disabled)
 */
#define PRESENCE_DETECTION_PERIODIC_DATA_REPORT 0x04
/**
 * @brief Octet[3] - Set to enable GPIO to notify presence
 *          - Bit 3-0: GPIO line to be used, 0 disabled
 *          - 1 : GPIO1       - 9 : GPIO9
 *          - 2 : GPIO2       -10 : GPIO10
 *          - 3 : GPIO3       -11 : GPIO11
 *          - 4 : GPIO4       -12 : GPIO12
 *          - 5 : GPIO5       -13 : GPIO13
 *          - 6 : GPIO6       -14 : GPIO14
 *          - 7 : GPIO7       -15 : GPIO15
 *          - 8 : GPIO8
 *          - Bit 6-4: RFU
 *          - Bit 7: 1 to disable RADAR_RX_NTF(Presence Detection)
 *          - (Default:0)
 *          - @note: Both GPIO and PD notification should not be disabled at one time.
 *                   FW will return invalid range error.
 *
 */
#define PRESENCE_DETECTION_ENABLE_IRQ        0x00
#define PRESENCE_DETECTION_SENSITIVITY_VALUE 3.75
#define PRESENCE_DETECTION_DISTANCE_MIN      30
#define MIN_PRESENCE_DETECTION_DISTANCE_MAX  200
#define MIN_PRESENCE_DETECTION_HOLD_DELAY    1600
#define MIN_PRESENCE_DETECTION_ANGLE         -90
#define MAX_PRESENCE_DETECTION_ANGLE         +90

/********************************************************************************/
#define PRESENCE_DETECTION_MODE        PRESENCE_DETECTION_MODE_ENABLE
#define DETECTION_PERIODIC_DATA_REPORT PRESENCE_DETECTION_PERIODIC_DATA_REPORT
#define SENSITIVITY_VALUE              PRESENCE_DETECTION_SENSITIVITY_VALUE
#define ENABLE_IRQ                     PRESENCE_DETECTION_ENABLE_IRQ
#define MIN_DISTANCE_MSB               DEC_TO_HEX_OCTETS_MSB(PRESENCE_DETECTION_DISTANCE_MIN)
#define MIN_DISTANCE_LSB               DEC_TO_HEX_OCTETS_LSB(PRESENCE_DETECTION_DISTANCE_MIN)
#define MAX_DISTANCE_MSB               DEC_TO_HEX_OCTETS_MSB(MIN_PRESENCE_DETECTION_DISTANCE_MAX)
#define MAX_DISTANCE_LSB               DEC_TO_HEX_OCTETS_LSB(MIN_PRESENCE_DETECTION_DISTANCE_MAX)
#define HOLD_DELAY_MSB                 DEC_TO_HEX_OCTETS_MSB(MIN_PRESENCE_DETECTION_HOLD_DELAY)
#define HOLD_DELAY_LSB                 DEC_TO_HEX_OCTETS_LSB(MIN_PRESENCE_DETECTION_HOLD_DELAY)
#define MINIMUM_ANGLE                  DEC_TO_HEX(MIN_PRESENCE_DETECTION_ANGLE)
#define MAXIMUM_ANGLE                  DEC_TO_HEX(MAX_PRESENCE_DETECTION_ANGLE)

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

    PRINT_APP_NAME("Demo OCPD (On Chip Presence Detection)");

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

    status = UwbApi_SessionInit(DEMO_OCPD_APP_SESSION_ID, UWBD_RADAR_TRANSFER, &sessionHandle);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    const uint8_t antennaeConfigurationRx[] = {DEMO_RADAR_MODE_RX,
        DEMO_RADAR_LENGTH_RX,
        DEMO_SET_APP_CONFIG_ANT_RX1,
        DEMO_SET_APP_CONFIG_ANT_RX2,
        DEMO_SET_APP_CONFIG_ANT_RX3};

    const uint8_t antennaeConfigurationTx[] = {DEMO_SET_APP_CONFIG_LENGTH_TX, DEMO_SET_APP_CONFIG_TX_ANT_ID};

    const uint8_t radarPresencedetectioncfg[] = {PRESENCE_DETECTION_MODE,
        DETECTION_PERIODIC_DATA_REPORT,
        DEC_TO_Q4_4(SENSITIVITY_VALUE),
        ENABLE_IRQ,
        MIN_DISTANCE_MSB,
        MIN_DISTANCE_LSB,
        MAX_DISTANCE_MSB,
        MAX_DISTANCE_LSB,
        HOLD_DELAY_MSB,
        HOLD_DELAY_LSB,
        MINIMUM_ANGLE,
        MAXIMUM_ANGLE};

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
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            RADAR_PRESENCE_DET_CFG, &radarPresencedetectioncfg[0], sizeof(radarPresencedetectioncfg)),
        UWB_SET_VENDOR_APP_PARAM_ARRAY(RADAR_RFRI, &rfri[0], sizeof(rfri)),
        UWB_SET_VENDOR_APP_PARAM_VALUE(RADAR_DRIFT_COMPENSATION, TO_Q15_HEX(0.1f)),
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
    UWBIOT_STACK_DEFINE(DemoRadarOcpd_stack, DEMO_OCPD_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_OCPD_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_OCPD_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack     = (k_thread_stack_t *)&DemoRadarOcpd_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(DemoRadarOcpd_stack);
#else
    threadparams.stackdepth = DEMO_OCPD_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // UWBIOT_APP_BUILD__DEMO_OCPD
