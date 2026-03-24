/*
 *
 * Copyright 2018-2020, 2022, 2026 NXP.
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

#include "PrintUtility_RfTest.h"
#include "phNxpLogApis_App.h"

EXTERNC void printPerParams(const phRfTestParams_t *pRfTestParams)
{
    if (pRfTestParams != NULL) {
        NXPLOG_APP_I("TestRF : numOfPckts : %" PRIu32, pRfTestParams->numOfPckts);
        NXPLOG_APP_I("TestRF : tGap : %" PRIu32, pRfTestParams->tGap);
        NXPLOG_APP_I("TestRF : tStart : %" PRIu32, pRfTestParams->tStart);
        NXPLOG_APP_I("TestRF : tWin : %" PRIu32, pRfTestParams->tWin);
        NXPLOG_APP_I("TestRF : randomizedSize : %hu", pRfTestParams->randomizedSize);
        NXPLOG_APP_I("TestRF : phrRangingBit : %" PRIu16, pRfTestParams->phrRangingBit);
        NXPLOG_APP_I("TestRF : rmarkerRxStart : %" PRIu32, pRfTestParams->rmarkerRxStart);
        NXPLOG_APP_I("TestRF : rmarkerTxStart : %" PRIu32, pRfTestParams->rmarkerTxStart);
        NXPLOG_APP_I("TestRF : stsIndexAutoIncr : %hu", pRfTestParams->stsIndexAutoIncr);
        NXPLOG_APP_I("TestRF : sts_detect_bitmap_en : %hu", pRfTestParams->sts_detect_bitmap_en);
    }
    else {
        NXPLOG_APP_I("pRfTestParams is NULL");
    }
}

EXTERNC void printLoopbackRecvData(const phTest_Loopback_Ntf_t *pRfTestRecvData)
{
    if (pRfTestRecvData != NULL) {
        NXPLOG_APP_I("Loopback : status : %hu", pRfTestRecvData->status);
        NXPLOG_APP_I(
            "Loopback : tx_ts : %" PRIu32 ".%" PRIu16, pRfTestRecvData->tx_ts_int, pRfTestRecvData->tx_ts_frac);
        NXPLOG_APP_I(
            "Loopback : rx_ts : %" PRIu32 ".%" PRIu16, pRfTestRecvData->rx_ts_int, pRfTestRecvData->rx_ts_frac);
        NXPLOG_APP_I("Loopback : aoa_azimuth :  %d.%d", TO_Q_9_7(pRfTestRecvData->aoa_azimuth));
        NXPLOG_APP_I("Loopback : aoa_elevation : %d.%d", TO_Q_9_7(pRfTestRecvData->aoa_elevation));
        NXPLOG_APP_I("Loopback : phr : %" PRIu16, pRfTestRecvData->phr);
        if (pRfTestRecvData->vs_data_len > 0) {
            NXPLOG_APP_I("PER : vs_data_len : %hu", pRfTestRecvData->vs_data_len);
            NXPLOG_APP_I("PER : vs_data_type : %hu", pRfTestRecvData->vs_data_type);
            NXPLOG_APP_I("PER : rx_mode : %hu", pRfTestRecvData->vs_data.rx_mode);
            NXPLOG_APP_I("PER : no_of_rx_antenna : %hu", pRfTestRecvData->vs_data.no_of_rx_antenna);
            for (int i = 0; i < pRfTestRecvData->vs_data.no_of_rx_antenna; i++) {
                NXPLOG_APP_I("PER : rx_antenna_id : %hu", pRfTestRecvData->vs_data.rx_antenna_id[i]);
            }
            for (int j = 0; j < pRfTestRecvData->vs_data.no_of_rx_antenna; j++) {
                NXPLOG_APP_I("PER : rssi_rx : %d.%d", TO_Q_8_8(pRfTestRecvData->vs_data.rssi_rx[j]));
            }
            for (int k = 0; k < pRfTestRecvData->vs_data.no_of_rx_antenna; k++) {
                NXPLOG_APP_I("PER : snr_rx : %d.%d", TO_Q_8_8(pRfTestRecvData->vs_data.snr_rx[k]));
            }
            NXPLOG_APP_I("PER : rx_cfo_est : %d.%d", TO_Q_5_11(pRfTestRecvData->rx_cfo_est));
        }
    }
    else {
        NXPLOG_APP_I("TestRecvData is NULL");
    }
}

EXTERNC void printTestSrRecvData(const phTest_Test_Sr_Ntf_t *pRfTestSrRecvData)
{
    if (pRfTestSrRecvData != NULL) {
        NXPLOG_APP_I("PER : status : %hu", pRfTestSrRecvData->Test_Sr_Ntf_status);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_attempts : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_attempts);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_acq_Detect : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_acq_Detect);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_acq_Reject : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_acq_Reject);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_rxfail : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_rxfail);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_sync_cir_ready : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_sync_cir_ready);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_sfd_fail : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_sfd_fail);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_sfd_found : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_sfd_found);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_sts_found : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_sts_found);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_eof : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_eof);
        NXPLOG_APP_I("PER : Test_Sr_Ntf_bitmap_len : %" PRIu32, pRfTestSrRecvData->Test_Sr_Ntf_bitmap_len);
        if (pRfTestSrRecvData->Test_Sr_Ntf_bitmap_len !=0){
            LOG_MAU8_I(
                "Test_Sr_Ntf_bitmap", pRfTestSrRecvData->Test_Sr_Ntf_bitmap, pRfTestSrRecvData->Test_Sr_Ntf_bitmap_len);
        }
    }
    else {
        NXPLOG_APP_I("TestRecvData is NULL");
    }
}

EXTERNC void printPerRecvData(const phTestPer_Rx_Ntf_t *pRfTestRecvData)
{
    if (pRfTestRecvData != NULL) {
        NXPLOG_APP_I("PER : status : %hu", pRfTestRecvData->status);
        NXPLOG_APP_I("PER : attempts : %" PRIu32, pRfTestRecvData->attempts);
        NXPLOG_APP_I("PER : acq_Detect : %" PRIu32, pRfTestRecvData->acq_Detect);
        NXPLOG_APP_I("PER : acq_Reject : %" PRIu32, pRfTestRecvData->acq_Reject);
        NXPLOG_APP_I("PER : rxfail : %" PRIu32, pRfTestRecvData->rxfail);
        NXPLOG_APP_I("PER : sync_cir_ready : %" PRIu32, pRfTestRecvData->sync_cir_ready);
        NXPLOG_APP_I("PER : sfd_fail : %" PRIu32, pRfTestRecvData->sfd_fail);
        NXPLOG_APP_I("PER : sfd_found : %" PRIu32, pRfTestRecvData->sfd_found);
        NXPLOG_APP_I("PER : phr_dec_error : %" PRIu32, pRfTestRecvData->phr_dec_error);
        NXPLOG_APP_I("PER : phr_bit_error : %" PRIu32, pRfTestRecvData->phr_bit_error);
        NXPLOG_APP_I("PER : psdu_dec_error : %" PRIu32, pRfTestRecvData->psdu_dec_error);
        NXPLOG_APP_I("PER : psdu_bit_error : %" PRIu32, pRfTestRecvData->psdu_bit_error);
        NXPLOG_APP_I("PER : sts_found : %" PRIu32, pRfTestRecvData->sts_found);
        NXPLOG_APP_I("PER : eof : %" PRIu32, pRfTestRecvData->eof);
        if (pRfTestRecvData->vs_data_len > 0) {
            NXPLOG_APP_I("PER : vs_data_len : %hu", pRfTestRecvData->vs_data_len);
            NXPLOG_APP_I("PER : vs_data_type : %hu", pRfTestRecvData->vs_data_type);
            NXPLOG_APP_I("PER : rx_mode : %hu", pRfTestRecvData->vs_data.rx_mode);
            NXPLOG_APP_I("PER : no_of_rx_antenna : %hu", pRfTestRecvData->vs_data.no_of_rx_antenna);
            for (int i = 0; i < pRfTestRecvData->vs_data.no_of_rx_antenna; i++) {
                NXPLOG_APP_I("PER : rx_antenna_id : %hu", pRfTestRecvData->vs_data.rx_antenna_id[i]);
            }
            for (int j = 0; j < pRfTestRecvData->vs_data.no_of_rx_antenna; j++) {
                NXPLOG_APP_I("PER : rssi_rx : %d.%d", TO_Q_8_8(pRfTestRecvData->vs_data.rssi_rx[j]));
            }
            for (int k = 0; k < pRfTestRecvData->vs_data.no_of_rx_antenna; k++) {
                NXPLOG_APP_I("PER : snr_rx : %d.%d", TO_Q_8_8(pRfTestRecvData->vs_data.snr_rx[k]));
            }
            NXPLOG_APP_I("PER : rx_cfo_est : %d.%d", TO_Q_5_11(pRfTestRecvData->rx_cfo_est));
        }
    }
    else {
        NXPLOG_APP_I("TestRecvData is NULL");
    }
}

EXTERNC void printrxRecvData(const phTest_Rx_Ntf_t *pRfTestRecvData)
{
    if (pRfTestRecvData != NULL) {
        NXPLOG_APP_I("TestRX : status : %hu", pRfTestRecvData->status);
        NXPLOG_APP_I("TestRX : rx_done_ts_int : %" PRIu32 ".%" PRIu16,
            pRfTestRecvData->rx_done_ts_int,
            pRfTestRecvData->rx_done_ts_frac);
        NXPLOG_APP_I("TestRX : aoa_azimuth : %d.%d", TO_Q_9_7(pRfTestRecvData->aoa_azimuth));
        NXPLOG_APP_I("TestRX : aoa_elevation : %d.%d", TO_Q_9_7(pRfTestRecvData->aoa_elevation));
        NXPLOG_APP_I("TestRX : toa_gap : %hu", pRfTestRecvData->toa_gap);
        NXPLOG_APP_I("TestRX : phr : %" PRIu16, pRfTestRecvData->phr);
        if (pRfTestRecvData->vs_data_len > 0) {
            NXPLOG_APP_I("PER : vs_data_len : %hu", pRfTestRecvData->vs_data_len);
            NXPLOG_APP_I("PER : vs_data_type : %hu", pRfTestRecvData->vs_data_type);
            NXPLOG_APP_I("PER : rx_mode : %hu", pRfTestRecvData->vs_data.rx_mode);
            NXPLOG_APP_I("PER : no_of_rx_antenna : %hu", pRfTestRecvData->vs_data.no_of_rx_antenna);
            for (int i = 0; i < pRfTestRecvData->vs_data.no_of_rx_antenna; i++) {
                NXPLOG_APP_I("PER : rx_antenna_id : %hu", pRfTestRecvData->vs_data.rx_antenna_id[i]);
            }
            for (int j = 0; j < pRfTestRecvData->vs_data.no_of_rx_antenna; j++) {
                NXPLOG_APP_I("PER : rssi_rx : %d.%d", TO_Q_8_8(pRfTestRecvData->vs_data.rssi_rx[j]));
            }
            for (int k = 0; k < pRfTestRecvData->vs_data.no_of_rx_antenna; k++) {
                NXPLOG_APP_I("PER : snr_rx : %d.%d", TO_Q_8_8(pRfTestRecvData->vs_data.snr_rx[k]));
            }
        }
    }
    else {
        NXPLOG_APP_I("TestRecvData is NULL");
    }
}
