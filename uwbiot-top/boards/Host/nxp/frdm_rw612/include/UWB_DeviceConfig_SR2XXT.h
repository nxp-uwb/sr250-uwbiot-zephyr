/*
 * Copyright 2026 NXP
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

#ifndef _UWB_DEVICECONFIG_VIRGO_H_
#define _UWB_DEVICECONFIG_VIRGO_H_

#include <stdint.h>
#include <uwb_board.h>
#include <phNxpUwbConfig.h>
#include <nxAntennaDefine.h>

/* Set to 0 in case you are using Casing/Type2HQ Murata Board,
 *
 * else set to 1 */
#define USE_BARE_BOARD 0

#if (USE_BARE_BOARD)
#define TX_ANTENNA_ENTRIES 0x02
#define RX_ANTENNA_ENTRIES 0x02
#define RX_PAIR_ENTRIES    0x01
#else
#define TX_ANTENNA_ENTRIES 0x02
#define RX_ANTENNA_ENTRIES 0x04
#define RX_PAIR_ENTRIES    0x02
#endif
/*
 * 0xE4 0x02 : DPD wakeup source : default value : 0x00
 * 0xE4 0x04 : DPD entry timeout : default value : 500ms (0x01F4)
 * 0xE4 0x60 : RX antenna Identifier
 * 0xE4 0x62 : Identifiers for each RX antenna pairs.
 * 0xE4 0x63 : TX antenna Identifiers
 * 0xE4 0x3A : Clock Config
 * */

/* clang-format off */
const uint8_t phNxpUciHal_core_configs[] =
{
   0x16, 0x20, 0x04, 0x00, 0x12, 0x03,
      0xE4, 0x02, 0x01, 0x00,
      0xE4, 0x04, 0x02, 0xF4, 0x01,
      0xE4, 0x3A, 0x05, 0x00, 0xE8, 0x03, 0xE8, 0x03
};

const uint8_t phNxpUciHal_NXPCoreConfig1[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig2[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig3[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig4[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig5[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig6[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig7[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig8[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig9[] = {0x00};

const uint8_t phNxpUciHal_NXPCoreConfig10[] = {0x00};

const uint8_t phNxpUciHal_core_antennadefs[] =
{
 /* Full length */ 4 + (1 + 3 + 3 + 3 +  ((1 + 6*AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES)) + (1 + 6*AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES)) + (1 + 6*AD_N_PAIR_ENTRIES(RX_PAIR_ENTRIES)))) ,
 /* Core set config */ 0x20, 0x04,
 /* Length */ 0x00, 1 + 3 + 3 + 3 +  ((1 + 6*AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES)) + (1 + 6*AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES)) + (1 + 6*AD_N_PAIR_ENTRIES(RX_PAIR_ENTRIES))) ,
 /* Num Configs */ 3,
AD_ANTENNA_RX_IDX_DEFINE_GPIO,
 (1 + 6*AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES)) , AD_N_RX_ENTRIES(RX_ANTENNA_ENTRIES),
#if (USE_BARE_BOARD)
    // Azimuth on the Naked Virgo/Tyep2HQ Murata board
    // Used for PCTT as well. EF1 Low.
    AD_RX_ID(1), AD_DEF_RX_PORT(SR2XX_RXC_PORT), AD_DEF_MASK(kAD_GPIO_EF1), AD_DEF_VAL(0x0000),
    /* On chip Antenna TX/RXA2  */
    AD_RX_ID(2), AD_DEF_RX_PORT(SR2XX_RXA2_PORT), AD_DEF_MASK(kAD_GPIO_EF1), AD_DEF_VAL(0x0000),
#else
   // H AoA for Front Patch Array on Casing board
    AD_RX_ID(1), AD_DEF_RX_PORT(SR2XX_RXC_PORT), AD_DEF_MASK(kAD_GPIO_EF1|kAD_GPIO_EF2), AD_DEF_VAL(kAD_GPIO_EF1|kAD_GPIO_EF2),
    // V AoA for Front Patch Array on Casing board.
    // NA for Naked Virgo/Tyep2HQ Murata board
    AD_RX_ID(2), AD_DEF_RX_PORT(SR2XX_RXB_PORT), AD_DEF_MASK(kAD_GPIO_EF1|kAD_GPIO_EF2), AD_DEF_VAL(kAD_GPIO_EF1|kAD_GPIO_EF2),
    /* Common RX Pin for both H and V. Goes to TX/RXA2 port of Front Patch Antenna Array  */
    AD_RX_ID(3), AD_DEF_RX_PORT(SR2XX_RXA2_PORT), AD_DEF_MASK(kAD_GPIO_EF1|kAD_GPIO_EF2), AD_DEF_VAL(kAD_GPIO_EF1|kAD_GPIO_EF2),
    /* TX/RXA1 port */
    AD_RX_ID(4), AD_DEF_RX_PORT(SR2XX_RXA1_PORT), AD_DEF_MASK(0), AD_DEF_VAL(0),
#endif // (USE_BARE_BOARD)

AD_ANTENNA_TX_IDX_DEFINE,
 (1 + 6*AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES)) , AD_N_TX_ENTRIES(TX_ANTENNA_ENTRIES),
    /* TRA1 Radar ant*/
    AD_TX_ID(1), AD_DEF_TX_PORT(SR2XX_TRA1_PORT), AD_DEF_MASK(0), AD_DEF_VAL(0),
#if (USE_BARE_BOARD)
    /* TRA2 On chip antenna */
    AD_TX_ID(2), AD_DEF_TX_PORT(SR2XX_TRA2_PORT), AD_DEF_MASK(kAD_GPIO_EF1), AD_DEF_VAL(0x0000),
#else
    /* TRA2 Front Patch Array*/
    AD_TX_ID(2), AD_DEF_TX_PORT(SR2XX_TRA2_PORT), AD_DEF_MASK(kAD_GPIO_EF1|kAD_GPIO_EF2), AD_DEF_VAL(kAD_GPIO_EF1|kAD_GPIO_EF2),
#endif // (USE_BARE_BOARD)

AD_ANTENNAS_RX_PAIR_DEFINE,
 (1 + 6*AD_N_PAIR_ENTRIES(RX_PAIR_ENTRIES)) , AD_N_PAIR_ENTRIES(RX_PAIR_ENTRIES),

/* 2D-AoA */
#if (USE_BARE_BOARD)
    /* RX Pair: H Naked */
    AD_AP_ID(1), AD_AP_RXC(1), AD_AP_RXB(0), AD_AP_RXA(2), AD_AP_FOV(0x0000),
#else
    /* RX Pair: H Virgo/Tyep2HQ Murata Front Patch Array */
    AD_AP_ID(1), AD_AP_RXC(1), AD_AP_RXB(2), AD_AP_RXA(0), AD_AP_FOV(0x0000),
    /* RX Pair: V Virgo/Tyep2HQ Murata Front Patch Array */
    AD_AP_ID(2), AD_AP_RXC(0), AD_AP_RXB(2), AD_AP_RXA(3), AD_AP_FOV(0x0000),
#endif // (USE_BARE_BOARD)
};

/* clang-format on */
const NxpParam_t phNxpUciHal_NXPConfig[] = {
    /*
     *  Ranging session period: which means, it is the duration of one 1:N
     session and the interval before start of next 1:N session. (1:N ranging
     period + interval between two consecutive  1:N cycles)
        UWB_RANG_SESSION_INTERVAL= (UWB_RANG_CYCLE_INTERVAL * No_of_anchors) +
     IDLE_BEFORE_START_OF_NEXT_1_N Value is in milliseconds This config is valid
     only in 1:N ranging session
     * */
    {UWB_RANG_SESSION_INTERVAL, TYPE_VAL, CONFIG_VAL 2000},
    /*
     *  Application session timeout: How log ranging shall continue.
        value is in milliseconds
        0 value: MW shall configure the value which is passed from application
        Non zero value: MW shall configure timeout with this config value
     provided here
     * */
    {UWB_APP_SESSION_TIMEOUT, TYPE_VAL, CONFIG_VAL 3600000},
    /*
     *  Ranging cycle interval: intreval between two consecutive SS/DS-TWR
     ranging cycle value is in milliseconds 0 value: MW shall configure the
     value which is passed from application Non zero value: MW shall configure
     interval with this config value provided here
     * */
    {UWB_RANG_CYCLE_INTERVAL, TYPE_VAL, CONFIG_VAL 200},
    /*
     *
     *  Timeout value in milliseconds for UWB standby mode.
        The range is between 5000 msec to 20000 msec and zero is to disable
     * */
    {UWB_STANDBY_TIMEOUT_VALUE, TYPE_VAL, CONFIG_VAL 0x00},
    /*
    *FW log level for each above module
     Logging Level Error            0x0001
     Logging Level Warning          0x0002
     Logging Level Timestamp        0x0004
     Logging Level Sequence Number  0x0008
     Logging Level Info-1           0x0010
     Logging Level Info-2           0x0020
     Logging Level Info-3           0x0040
     * */
    {UWB_SET_FW_LOG_LEVEL, TYPE_VAL, CONFIG_VAL 0x003},
    /*
     * Enable/disable to dump FW binary log for different Modules as below
       0x00 for disable the binary log
       Secure Thread      0x01
       Secure ISR         0x02
       Non-Secure ISR     0x04
       Shell Thread       0x08
       PHY Thread         0x10
       Ranging Thread     0x20
     * */
    {UWB_FW_LOG_THREAD_ID, TYPE_VAL, CONFIG_VAL 0x00},
    /*
     * Ranging feature:  Single Sided Two Way Ranging or Double Sided Two Way
     Ranging SS-TWR =0x00 DS-TWR =0x01
     */
    {UWB_MW_RANGING_FEATURE, TYPE_VAL, CONFIG_VAL 0x01},

    {UWB_CORE_CONFIG_PARAM, TYPE_DATA, phNxpUciHal_core_configs},

    /* Core config antennae defines*/
    {UWB_CORE_ANTENNAE_DEFINES, TYPE_DATA, phNxpUciHal_core_antennadefs},
    /* Timeout for Firmware to enter DPD mode
     * Note: value set for UWB_DPD_ENTRY_TIMEOUT shall be in MilliSeconds.
     * Min : 100ms
     * Max : 2000ms */
    {UWB_DPD_ENTRY_TIMEOUT, TYPE_VAL, CONFIG_VAL 500},
    /* 0x00 = FIRA generic notifications (Default)
     * 0x01 = Vendor extended notifications
     * UWBS shall send any proprietary information in any response/notification
     * if NXP_EXTENDED_NTF_CONFIG is set 0x01 */
    {UWB_NXP_EXTENDED_NTF_CONFIG, TYPE_VAL, CONFIG_VAL 0x01},
    /* Firmware Low Power Mode
     * if UWB_LOW_POWER_MODE is 0, Firmware is Configured in non Low Power Mode
     * if UWB_LOW_POWER_MODE in 1, Firmware is Configured with Low Power Mode */
    {UWB_LOW_POWER_MODE, TYPE_VAL, CONFIG_VAL 0x01},

    {UWB_NXP_CORE_CONFIG_BLOCK_1, TYPE_DATA, phNxpUciHal_NXPCoreConfig1},
    {UWB_NXP_CORE_CONFIG_BLOCK_2, TYPE_DATA, phNxpUciHal_NXPCoreConfig2},
    {UWB_NXP_CORE_CONFIG_BLOCK_3, TYPE_DATA, phNxpUciHal_NXPCoreConfig3},
    {UWB_NXP_CORE_CONFIG_BLOCK_4, TYPE_DATA, phNxpUciHal_NXPCoreConfig4},
    {UWB_NXP_CORE_CONFIG_BLOCK_5, TYPE_DATA, phNxpUciHal_NXPCoreConfig5},
    {UWB_NXP_CORE_CONFIG_BLOCK_6, TYPE_DATA, phNxpUciHal_NXPCoreConfig6},
    {UWB_NXP_CORE_CONFIG_BLOCK_7, TYPE_DATA, phNxpUciHal_NXPCoreConfig7},
    {UWB_NXP_CORE_CONFIG_BLOCK_8, TYPE_DATA, phNxpUciHal_NXPCoreConfig8},
    {UWB_NXP_CORE_CONFIG_BLOCK_9, TYPE_DATA, phNxpUciHal_NXPCoreConfig9},
    {UWB_NXP_CORE_CONFIG_BLOCK_10, TYPE_DATA, phNxpUciHal_NXPCoreConfig10},

    /* Number of UWB_NXP_CORE_CONFIG_BLOCKS available in the config file */
    {UWB_NXP_CORE_CONFIG_BLOCK_COUNT, TYPE_VAL, CONFIG_VAL 10}};

#endif //_UWB_DEVICECONFIG_VIRGO_H_
