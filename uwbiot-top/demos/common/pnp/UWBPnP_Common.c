/*
 *
 * Copyright 2021,2024-2026 NXP.
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

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)

#include <UWB_Evt_Pnp.h>
#include <uwbiot_ver.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "UwbPnpInternal.h"
#include "UWB_Spi_Driver_Interface.h"

#if UWBFTR_SE_SE051W || UWBIOT_SESN_P71
#include <sm_types.h>
#include "phNxpEse_Internal.h"
#include "smComT1oI2C.h"
#endif //(UWBFTR_SE_SE051W || UWBIOT_SESN_P71)

#if (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))
#include "SeApi.h"
#include "wearable_platform_int.h"
#include "phTmlUwb_transport.h"
extern void Enable_GPIO0_IRQ();
int8_t GeneralState = 0;
#endif // (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))

size_t UWBPnP_GetVersionInfo(uint8_t *tlvBuf, size_t tlvBufLen, UBWPnPBoardIdentifier_t board)
{
    if (tlvBufLen < GET_VERISON_INFO_RESPONSE_SIZE) {
        return 0;
    }
    tlvBuf[0] = GET_VERISON_INFO;
    tlvBuf[1] = UWBIOTVER_STR_VER_MAJOR;
    tlvBuf[2] = UWBIOTVER_STR_VER_MINOR;
    tlvBuf[3] = UWBIOTVER_STR_VER_DEV;
    tlvBuf[4] = 0; /* RFU */

#if UWBIOT_UWBD_SR150
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR150;
#elif UWBIOT_UWBD_SR040
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR040;
#elif UWBIOT_UWBD_SR100T
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR100T;
#elif UWBIOT_UWBD_SR200T
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR200T;
#elif UWBIOT_UWBD_SR100S
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR100S;
#elif UWBIOT_UWBD_SR250
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR250;
#elif UWBIOT_UWBD_SR200S
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR200S;
#elif UWBIOT_UWBD_SR048M
    tlvBuf[5] = kUBWPnPUWBSIdentifier_SR048M;
#else
#error "Don't know the IC"
#endif
    tlvBuf[6] = board;

    return GET_VERISON_INFO_RESPONSE_SIZE;
}

#if (UWBFTR_SE_SE051W || UWBIOT_SESN_P71)
void Se_Comm_Init(void)
{
    uint16_t se_status = SMCOM_COM_FAILED;
    uint8_t Atr[64];
    uint16_t AtrLen = sizeof(Atr);
    se_status       = smComT1oI2C_Open(NULL, 0x00, 0x00, Atr, &AtrLen);
    if (se_status != SMCOM_OK) {
        PRINTF("ERROR: smComT1oI2C_Open Failed\n");
    }
}
#endif // (UWBFTR_SE_SE051W || UWBIOT_SESN_P71)

#if (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))
void Se_Comm_Init(void)
{
    tSEAPI_STATUS se_status = SEAPI_STATUS_FAILED;
    se_status               = SeApi_Init(NULL, NULL);
    if (se_status != SEAPI_STATUS_OK) {
        PRINTF("ERROR: SeApi_Init Failed\n");
    }
    Enable_GPIO0_IRQ();
    se_status = SeApi_WiredEnable(TRUE);
    UWB_Tml_Io_Init();
    if (se_status != SEAPI_STATUS_OK) {
        PRINTF("ERROR: SeApi_WiredEnable Failed\n");
    }
}
#endif // (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))

#if (UWBFTR_SE_SE051W || UWBIOT_SESN_SNXXX)
void Se_Transceive(uint8_t *cmd, uint16_t cmdLen, uint8_t *resp, size_t *respLen)
{
    uint32_t status = 0;
#if (UWBFTR_SE_SE051W || UWBIOT_SESN_P71)
    status = smCom_TransceiveRaw(NULL, &cmd[4], cmdLen - 4, &resp[4], (uint32_t *)respLen);
    if (status == SMCOM_OK) {
        resp[0]  = MT_ESE_CTRL_RSP << UCI_MT_SHIFT | 0;
        resp[1]  = 0x00;
        resp[2]  = *respLen;
        resp[3]  = 0x00;
        *respLen = *respLen + UCI_HEADER_SIZE;
    }
#endif // (UWBFTR_SE_SE051W || UWBIOT_SESN_P71)
#if (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))
    Enable_GPIO0_IRQ();
    status = SeApi_WiredTransceive(&cmd[4], cmdLen - 4, &resp[4], 255, (uint16_t *)respLen, 10000);
    UWB_Uwbs_Enable_Interrupt();
    if (status == SEAPI_STATUS_OK) {
        resp[0]  = MT_ESE_CTRL_RSP << UCI_MT_SHIFT | 0;
        resp[1]  = 0x00;
        resp[2]  = *respLen;
        resp[3]  = 0x00;
        *respLen = *respLen + UCI_HEADER_SIZE;
    }
#endif // (UWBIOT_SESN_SNXXX && !(UWBIOT_SESN_P71))
}
#endif

void PRINTF_WITH_TIME(const char *fmt, ...)
{
    // Nothing to do
}

#endif // UWBIOT_APP_BUILD__DEMO_PNP
