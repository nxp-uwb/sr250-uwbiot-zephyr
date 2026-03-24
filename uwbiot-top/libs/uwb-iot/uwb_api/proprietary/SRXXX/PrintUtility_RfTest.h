/*
 *
 * Copyright 2018-2020 NXP.
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

#ifndef _PRINT_UTILITY_RF_TEST_H
#define _PRINT_UTILITY_RF_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "PrintUtility.h"
#include "UwbApi_Types_RfTest.h"

EXTERNC void printPerParams(const phRfTestParams_t *pRfTestParams);
EXTERNC void printPerRecvData(const phTestPer_Rx_Ntf_t *pRfTestRecvData);
EXTERNC void printrxRecvData(const phTest_Rx_Ntf_t *pRfTestRecvData);
EXTERNC void printLoopbackRecvData(const phTest_Loopback_Ntf_t *pRfTestRecvData);
EXTERNC void printTestSrRecvData(const phTest_Test_Sr_Ntf_t *pRfTestSrRecvData);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif
