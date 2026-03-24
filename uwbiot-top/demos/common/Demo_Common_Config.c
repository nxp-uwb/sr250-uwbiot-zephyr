/* Copyright 2022,2023,2025,2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/**
 * @file Demo_Common_Config.c
 * @author nxf18919
 * @brief This file is used for common application configs setting and common api calls in multiple demos
 *
 */
#include "Demo_Common_Config.h"
#include "phNxpLogApis_App.h"
#include "uwb_board.h"

#if (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))
#include "SeApi.h"
#include "wearable_platform_int.h"
#include "phTmlUwb_transport.h"

#define SUS_TEST_AID                                                                                   \
    {                                                                                                  \
        0xA0, 0x00, 0x00, 0x03, 0x96, 0x54, 0x53, 0x00, 0x00, 0x00, 0x01, 0x04, 0xF2, 0x00, 0x00, 0x00 \
    }
#define SUS_AID                                                                                        \
    {                                                                                                  \
        0xA0, 0x00, 0x00, 0x03, 0x96, 0x54, 0x53, 0x00, 0x00, 0x00, 0x01, 0x04, 0x02, 0x00, 0x00, 0x00 \
    }
#define ROOT_SESSION_KEY                                                                                            \
    {                                                                                                               \
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x01, 0x02, \
            0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10                      \
    }

#define SELECT_APPLET_CMD            \
    {                                \
        0x00, 0xA4, 0x04, 0x00, 0x00 \
    }

#define SET_SESSION_ID_CMD           \
    {                                \
        0x80, 0xA0, 0x00, 0x00, 0x00 \
    }

#define GET_SESSION_ID_CMD     \
    {                          \
        0x80, 0xCA, 0x00, 0x47 \
    }

extern void Enable_GPIO0_IRQ();

#endif // (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))

/* Currently not used for SR040 */
#if !(UWBIOT_UWBD_SR04X)

/* Helper macro for ToA Mode */
#define TOA_ID_X(H, V) kUWBAntCfgRxMode_AoA_Mode, 0x02, (H), (V)
#define TOA_ID_Y(Y)    kUWBAntCfgRxMode_AoA_Mode, 0x01, (Y)
/* Helper macro for ToA Mode */
#define TOF_ID_X(X) kUWBAntCfgRxMode_ToA_Mode, 0x01, (X)

/* Helper macro to select 2D AoA Azimuth Antenna */
#define ANT_ID_AOA_2D_AZIMUTH TOA_ID_Y(1)

/* Helper macro to select 3D AoA Azimuth and Elevation
 * configuration of the antenna.
 *
 * When using this, ensure for SR1XXT
 * - ``USE_NAKED_BOARD`` is set to ``0`` in ``boards/Host/Rhodes4/UWB_DeviceConfig_SR1XX.h``
 * When using this, ensure for SR2XXT
 * - ``USE_BARE_BOARD`` is set to ``0`` in ``boards/Host/Virgo/UWB_DeviceConfig.h``
 * - set ``USE_FRONT_ANT`` or ``USE_BACK_ANT``to ``1`` in ``boards/Host/Virgo/UWB_DeviceConfig.h``
 */
#define ANT_ID_AOA_3D_AoA TOA_ID_X(1, 2)

#if (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_2DAOA)
#define SET_ANTENNA_CONFIG (1)
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_3DAOA)
#define SET_ANTENNA_CONFIG (1)
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_TOF)
#define SET_ANTENNA_CONFIG (0)
#else
#error "Select TOF, 2D AOA or 3D AoA"
#endif

tUWBAPI_STATUS demo_set_common_app_config(uint32_t sessionHandle, UWB_StsConfig_t sts_config)
{
    tUWBAPI_STATUS status;

#if SET_ANTENNA_CONFIG
#if (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_2DAOA)
    uint8_t antennaeConfigurationRx[] = {ANT_ID_AOA_2D_AZIMUTH};
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_3DAOA)
    uint8_t antennaeConfigurationRx[] = {ANT_ID_AOA_3D_AoA};
#elif (UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL == UWB_BOARD_RX_ANTENNA_CONFIG_MODE_TOF)
    /* No Need to define antennaeConfigurationsRX */
#else
#error "Select TOF, 2D AOA or 3D AoA"
#endif
#endif // SET_ANTENNA_CONFIG

#if (UWBIOT_UWBD_SR1XXT)
    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 2),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 25),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 25 * 8),
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, sts_config),
        UWB_SET_APP_PARAM_VALUE(NUMBER_OF_STS_SEGMENTS, 1),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, 3),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
    };

#endif // UWBIOT_UWBD_SR1XXT

#if (UWBIOT_UWBD_SR2XXT)
    const UWB_AppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(RANGING_ROUND_USAGE, kUWB_RangingRoundUsage_DS_TWR),
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 2),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 25),
        UWB_SET_APP_PARAM_VALUE(RANGING_DURATION, 25 * 8),
        /*static STS*/
        UWB_SET_APP_PARAM_VALUE(STS_CONFIG, 0),
        UWB_SET_APP_PARAM_VALUE(NUMBER_OF_STS_SEGMENTS, 1),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, kUWB_RfFrameConfig_SP3),
        UWB_SET_APP_PARAM_VALUE(CHANNEL_NUMBER, 9),
        UWB_SET_APP_PARAM_VALUE(AOA_RESULT_REQ, 0),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_CODE_INDEX, 10),
        UWB_SET_APP_PARAM_VALUE(PREAMBLE_DURATION, 1),
        UWB_SET_APP_PARAM_VALUE(PSDU_DATA_RATE, 0),
        UWB_SET_APP_PARAM_VALUE(PRF_MODE, 0),
        UWB_SET_APP_PARAM_VALUE(BPRF_PHR_DATA_RATE, 0),
        UWB_SET_APP_PARAM_VALUE(STS_LENGTH, 1),
    };
#endif

    status = UwbApi_SetAppConfigMultipleParams(
        sessionHandle, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

#if SET_ANTENNA_CONFIG
    const UWB_VendorAppParams_List_t SetVendorAppParamsList[] = {
        UWB_SET_VENDOR_APP_PARAM_ARRAY(
            ANTENNAE_CONFIGURATION_RX, &antennaeConfigurationRx[0], sizeof(antennaeConfigurationRx)),
    };

    status = UwbApi_SetVendorAppConfigs(
        sessionHandle, sizeof(SetVendorAppParamsList) / sizeof(SetVendorAppParamsList[0]), &SetVendorAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetVendorAppConfigs() Failed");
        goto exit;
    }
#endif // SET_ANTENNA_CONFIG
exit:
    return status;
}

#if (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))

tUWBAPI_STATUS demo_setFixedSessionKey(uint32_t sessionId)
{
    tSEAPI_STATUS status       = 0;
    uint8_t txBuffer[255]      = {0};
    uint8_t rxBuffer[255]      = {0};
    uint16_t rxRecvBufferLen   = 0;
    uint8_t SUSTestApplet[]    = SUS_TEST_AID;
    uint8_t SUSApplet[]        = SUS_AID;
    uint8_t rootSessionKey[]   = ROOT_SESSION_KEY;
    uint8_t selectAppletCmd[]  = SELECT_APPLET_CMD;
    uint8_t sessionInjectCmd[] = SET_SESSION_ID_CMD;
    uint8_t getSessionId[]     = GET_SESSION_ID_CMD;
    uint32_t recvSessionId;
    uint8_t offset;

    /* Enable SN110 Irq */
    // TODO: This is the temporary fix for SR1XX and SN110 IRQ enablament
    Enable_GPIO0_IRQ();

    SeApi_WiredEnable(TRUE);

    /* Select SUS Test Applet */
    phOsalUwb_MemCopy(&txBuffer[0], &selectAppletCmd[0], sizeof(selectAppletCmd));
    txBuffer[LEN_OFFSET] = sizeof(SUSTestApplet);
    phOsalUwb_MemCopy(&txBuffer[PAYLOAD_OFFSET], &SUSTestApplet[0], sizeof(SUSTestApplet));

    LOG_MAU8_I("SE Tx >", txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet));
    status = SeApi_WiredTransceive(
        txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet), rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }

    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    /* Set Fixed Session Key  */
    phOsalUwb_MemCopy(&txBuffer[0], &sessionInjectCmd[0], sizeof(sessionInjectCmd));
    txBuffer[4] = sizeof(rootSessionKey) + 4 + SESSION_ID_LEN;
    txBuffer[5] = ROOT_SESSION_KEY_TAG_ID;
    txBuffer[6] = sizeof(rootSessionKey);
    phOsalUwb_MemCopy(&txBuffer[ROOT_SESSION_KEY_OFFSET], &rootSessionKey[0], sizeof(rootSessionKey));
    offset             = ROOT_SESSION_KEY_OFFSET + sizeof(rootSessionKey);
    txBuffer[offset++] = SESSION_ID_TAG_ID;
    txBuffer[offset++] = SESSION_ID_LEN;
    UWB_UINT32_TO_BE_FIELD(&txBuffer[offset], sessionId);

    LOG_MAU8_I("SE Tx >", txBuffer, txBuffer[4] + 5);

    status = SeApi_WiredTransceive(txBuffer, txBuffer[4] + 5, rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }
    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    /* select SUS Applet */
    phOsalUwb_MemCopy(&txBuffer[0], &selectAppletCmd[0], sizeof(selectAppletCmd));
    txBuffer[4] = sizeof(SUSApplet);
    phOsalUwb_MemCopy(&txBuffer[5], &SUSApplet[0], sizeof(SUSApplet));

    LOG_MAU8_I("SE Tx >", txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet));
    status = SeApi_WiredTransceive(
        txBuffer, sizeof(selectAppletCmd) + sizeof(SUSApplet), rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }

    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    /* Verify Session ID */
    phOsalUwb_MemCopy(&txBuffer[0], &getSessionId[0], sizeof(getSessionId));

    LOG_MAU8_I("SE Tx >", txBuffer, txBuffer[4] + 5);

    status = SeApi_WiredTransceive(txBuffer, 4, rxBuffer, 255, &rxRecvBufferLen, 10000);
    if (status != SEAPI_STATUS_OK) {
        LOG_E("Failed to select applet");
        return UWBAPI_STATUS_FAILED;
    }
    LOG_MAU8_I("SE Rx <", rxBuffer, rxRecvBufferLen);

    recvSessionId = (((uint32_t)(rxBuffer[5])) + (((uint32_t)(rxBuffer[4])) << 8) + (((uint32_t)(rxBuffer[3])) << 16) +
                     (((uint32_t)(rxBuffer[2])) << 24));

    if (recvSessionId != sessionId) {
        LOG_E("Session Key mismatch");
        return UWBAPI_STATUS_FAILED;
    }

    /* Enable SR1XX Irq */
    // TODO: This is the temporary fix for SR1XX and SN110 IRQ enablement
    phTmlUwb_io_enable_uwb_irq();

    return UWBAPI_STATUS_OK;
}
#endif // (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))

#if UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S
tUWBAPI_STATUS demo_configure_otp_calibration(uint8_t channel)
{
    PRINTF(" demo_configure_otp_calibration applying for channel %d\n", channel);
    tUWBAPI_STATUS status;
    uint16_t bitMask;
    phCalibPayload_t readCalibData = {0x00};

    uint8_t calibValues[MAX_CALIB_VALUE] = {0x00};
    uint8_t *pSetCalibValue;
    uint32_t index = 0;
    bitMask        = (CHIP_CALIBRATION_POS | PAPPPA_CALIB_CTRL_POS | TX_POWER_POS | XTAL_CAP_VALUES_POS);
    status         = UwbApi_ReadOtpCalibDataCmd(channel, bitMask, &readCalibData);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_ReadOtpCalibDataCmd Failed");
        return UWBAPI_STATUS_FAILED;
    }

    if (readCalibData.CHIP_CALIBRATION != 0) {
        pSetCalibValue = calibValues;
        UWB_UINT16_TO_STREAM(pSetCalibValue, readCalibData.CHIP_CALIBRATION, index);
        status = UwbApi_SetCalibration(channel, CHIP_CALIBRATION, calibValues, sizeof(readCalibData.CHIP_CALIBRATION));
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Set Calib param CHIP_CALIBRATION Failed");
            return UWBAPI_STATUS_FAILED;
        }
    }

    if (readCalibData.PA_PPA_CALIB_CTRL != 0) {
        pSetCalibValue = calibValues;
        index          = 0;
        UWB_UINT16_TO_STREAM(pSetCalibValue, readCalibData.PA_PPA_CALIB_CTRL, index);
        status =
            UwbApi_SetCalibration(channel, PA_PPA_CALIB_CTRL, calibValues, sizeof(readCalibData.PA_PPA_CALIB_CTRL));
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Set Calib param PA_PPA_CALIB_CTRL Failed");
            return UWBAPI_STATUS_FAILED;
        }
    }
    else {
        PRINTF("Set Calib param for PA_PPA_CALIB_CTRL SKIPPED due to no data from the OTP\n");
    }

    if (readCalibData.XTAL_CAP_VALUES[0] != 0) {
        phOsalUwb_SetMemory(calibValues, 0x00, sizeof(calibValues));

        calibValues[0] = 0x03; // Number of registers(must be 0x03)
        calibValues[1] = readCalibData.XTAL_CAP_VALUES[0];
        calibValues[3] = readCalibData.XTAL_CAP_VALUES[1];
        calibValues[5] = readCalibData.XTAL_CAP_VALUES[2];
        status         = UwbApi_SetCalibration(channel, RF_CLK_ACCURACY_CALIB, calibValues, RF_CLK_ACCURACY_CALIB_LEN);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("Set Calib param RF_CLK_ACCURACY_CALIB Failed");
            return UWBAPI_STATUS_FAILED;
        }
    }
    else {
        PRINTF("Set Calib param for RF_CLK_ACCURACY_CALIB SKIPPED due to no data from the OTP\n");
    }
    // Temp commented the  TX_POWER_ID to update from the APP evey time
    //   if (readCalibData.TX_POWER_ID[0] != 0) {
    PRINTF("Set Calib param for TX_POWER_PER_ANTENNA SET\n");
    phOsalUwb_SetMemory(calibValues, 0x00, sizeof(calibValues));
    uint8_t noOfEntries = 0x01;

    calibValues[0] = noOfEntries; // No. of Entries
    calibValues[1] = 0x01;        // Antenna Id 1
    calibValues[2] = 0x17;        // Tx Power Delta Peak
    calibValues[4] = 0x00;        // Tx Power Id RMS
    calibValues[6] = 0x02;        // Antenna Id 2
    calibValues[7] = 0X00;        // Tx Power Delta Peak
    calibValues[9] = 0X00;        // Tx Power Id RMS

    status = UwbApi_SetCalibration(channel, TX_POWER_PER_ANTENNA, calibValues, TX_POWER_PER_ANT_LEN(noOfEntries));
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Set Calib param TX_POWER_PER_ANTENNA Failed");
        return UWBAPI_STATUS_FAILED;
    }
    //}
    return UWBAPI_STATUS_OK;
}
#endif // UWBIOT_UWBD_SR150 || UWBIOT_UWBD_SR100S


#endif // !(UWBIOT_UWBD_SR04X)
