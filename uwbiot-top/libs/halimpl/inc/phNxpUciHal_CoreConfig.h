/*
 *
 * Copyright 2018-2020,2022 NXP.
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

#ifndef _PHNXPUCIHAL_RHODESCONFIG_H_
#define _PHNXPUCIHAL_RHODESCONFIG_H_

#include "phUwb_BuildConfig.h"
#include "phUwbTypes.h"

#define TYPE_VAL           0
#define TYPE_DATA          1
#define TYPE_STR           2
#define TYPE_EXTENDED_DATA 3

typedef struct
{
    unsigned char key;
    unsigned char type;
    const void *val;
} NxpParam_t;

#define CONFIG_VAL (void *)

typedef enum
{
    UWB_MODEM_FRAME_CONFIG,
    UWB_MODEM_PREAMBLE_IDX,
    UWB_MODEM_SFD_IDX,
    UWB_MODEM_RF_CHAN_NO,
    UWB_MODEM_PREAMBLE_DURATION,
    UWB_RX_ANTENNAE_DELAY_CALIB_CH5 = 5,
    UWB_RX_ANTENNAE_DELAY_CALIB_CH9  = 9,
    UWB_RX_ANTENNAE_DELAY_CALIB_CH10 = 10,
    UWB_RX_ANTENNAE_DELAY_CALIB_CH12 = 12,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR1_CH9,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH5,
    UWB_AOA_CONFIG_PDOA_CALIB_RXPAIR2_CH9,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH5,
    UWB_AOA_CONFIG_PDOA_OFFSET_CH9,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH5,
    UWB_AOA_CONFIG_THRESHOLD_PDOA_CH9,
#if UWBIOT_UWBD_SR150
    UWB_AOA_CH5_CONFIG_BLOCK_COUNT,
    UWB_AOA_CH9_CONFIG_BLOCK_COUNT,
#else
    UWB_AOA_CONFIG_BLOCK_COUNT,
#endif
    UWB_CORE_CONFIG_PARAM,
    UWB_CORE_ANTENNAE_DEFINES,
    UWB_MW_RANGING_FEATURE,
    UWB_FW_LOG_THREAD_ID,
    UWB_SET_FW_LOG_LEVEL,
    UWB_STANDBY_TIMEOUT_VALUE,
    UWB_RANG_CYCLE_INTERVAL,
    UWB_APP_SESSION_TIMEOUT,
    UWB_RANG_SESSION_INTERVAL,
    UWB_DPD_ENTRY_TIMEOUT,
    UWB_NXP_EXTENDED_NTF_CONFIG,
    UWB_LOW_POWER_MODE,
    UWB_HPD_ENTRY_TIMEOUT,
    UWB_MHR_IN_CCM,
    UWB_DDFS_TONE_CONFIG_ENABLE,
    UWB_TELEC_CONFIG,
#if UWBIOT_UWBD_SR1XXT_SR2XXT
    UWB_NXP_CORE_CONFIG_BLOCK_1,
    UWB_NXP_CORE_CONFIG_BLOCK_2,
    UWB_NXP_CORE_CONFIG_BLOCK_3,
    UWB_NXP_CORE_CONFIG_BLOCK_4,
    UWB_NXP_CORE_CONFIG_BLOCK_5,
    UWB_NXP_CORE_CONFIG_BLOCK_6,
    UWB_NXP_CORE_CONFIG_BLOCK_7,
    UWB_NXP_CORE_CONFIG_BLOCK_8,
    UWB_NXP_CORE_CONFIG_BLOCK_9,
    UWB_NXP_CORE_CONFIG_BLOCK_10,
    UWB_NXP_CORE_CONFIG_BLOCK_COUNT
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
} NxpUwbConfig;

#endif //_PHNXPUCIHAL_RHODESCONFIG_H_
