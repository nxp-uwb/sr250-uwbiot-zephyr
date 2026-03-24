/* Copyright 2022,2025 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef __DEMO_COMMON_CONFIG__
#define __DEMO_COMMON_CONFIG__

#include "phUwb_BuildConfig.h"
#include "UwbApi.h"

tUWBAPI_STATUS demo_set_common_app_config(uint32_t sessionHandle, UWB_StsConfig_t sts_config);

#if (UWBIOT_SESN_SNXXX)
#define LEN_OFFSET              4
#define PAYLOAD_OFFSET          5
#define ROOT_SESSION_KEY_TAG_ID 0XC0
#define ROOT_SESSION_KEY_OFFSET 7
#define SESSION_ID_TAG_ID       0xCF
#endif // UWBIOT_SESN_SNXXX

#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S
#define MAX_CALIB_VALUE             16
#define RF_CLK_ACCURACY_CALIB_LEN   0x07

/**
 * 1B noOfEntries
 * 5B calibrationValue
 * LEN =( noOfEntries + (numberofEntries * calibrationValue))
 */
#define TX_POWER_PER_ANT_LEN(entries) (1 + (entries * 5))
#define CHANNEL_5                     0x05
#define CHANNEL_9                     0x09
#endif //UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S

tUWBAPI_STATUS demo_setFixedSessionKey(uint32_t sessionId);
#if (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S)
tUWBAPI_STATUS demo_configure_otp_calibration(uint8_t channel);
#endif // (UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S)

#endif // __DEMO_COMMON_CONFIG__
