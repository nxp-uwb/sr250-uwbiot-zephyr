/*
 *
 * Copyright 2019-2020,2023 NXP.
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
/**
 *
 *  This file contains the definition from UCI specification
 *
 */

#ifndef UWB_UCI_TEST_DEFS_H
#define UWB_UCI_TEST_DEFS_H

#include <stdint.h>

/**
 **GID: UCI test group - 0x0D : Opcodes
 */
#define UCI_MSG_TEST_SET_CONFIG  0
#define UCI_MSG_TEST_GET_CONFIG  1
#define UCI_MSG_TEST_PERIODIC_TX 2
#define UCI_MSG_TEST_PER_RX      3
/** RFU 4 */
#define UCI_MSG_TEST_RX           5
#define UCI_MSG_TEST_LOOPBACK     6
#define UCI_MSG_TEST_STOP_SESSION 7
#define UCI_MSG_TEST_SR_RX        9

#define UCI_MSG_TEST_PERIODIC_TX_CMD_SIZE  0
#define UCI_MSG_TEST_PER_RX_CMD_SIZE       0
#define UCI_MSG_TEST_STOP_SESSION_CMD_SIZE 0
#define UCI_MSG_TEST_RX_CMD_SIZE           0

/**
 * UCI test Parameter IDs : RF Test Configurations
 */
#define UCI_TEST_PARAM_ID_NUM_PACKETS          0x00
#define UCI_TEST_PARAM_ID_T_GAP                0x01
#define UCI_TEST_PARAM_ID_T_START              0x02
#define UCI_TEST_PARAM_ID_T_WIN                0x03
#define UCI_TEST_PARAM_ID_RANDOMIZE_PSDU       0x04
#define UCI_TEST_PARAM_ID_PHR_RANGING_BIT      0x05
#define UCI_TEST_PARAM_ID_RMARKER_TX_START     0x06
#define UCI_TEST_PARAM_ID_RMARKER_RX_START     0x07
#define UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR  0x08
#define UCI_TEST_PARAM_ID_STS_DETECT_BITMAP_EN 0x09
#endif
