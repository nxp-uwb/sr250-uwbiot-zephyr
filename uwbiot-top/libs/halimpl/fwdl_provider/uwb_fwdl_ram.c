/* Copyright 2022-2023 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "uwb_board.h"

#ifndef UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD
#error UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD must be defined in uwb_board.h
#endif

#if (0 == UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD)
/* RAM Based FW Download */
#include "phNxpLogApis_FwDnld.h"
#include <phTmlUwb_transport.h>
#include "UwbAdaptation.h"

/** Local Function prototypes */
static void setFwImage(const uint8_t *fwImgPtr, uint32_t fwSize);

#include "uwb_fwdl_provider.h"

extern uwb_fwdl_provider_t fwdlCtx;

static void setFwImage(const uint8_t *fwImgPtr, uint32_t fwSize)
{
    fwdlCtx.fwImgPtr = fwImgPtr;
    fwdlCtx.fwSize   = fwSize;
}

UWBStatus_t uwb_fwdl_setFwImage(const phUwbFWImageContext_t *const pAppfwImageCtx)
{
    NXPLOG_UWB_FWDNLD_I("FWDL Directly from host");
    if (pAppfwImageCtx->fwImage == NULL || pAppfwImageCtx->fwImgSize <= 0) {
        return kUWBSTATUS_FAILED;
    }

    setFwImage(pAppfwImageCtx->fwImage, pAppfwImageCtx->fwImgSize);
    NXPLOG_UWB_FWDNLD_D("Firmware size: %d", pAppfwImageCtx->fwImgSize);

    return kUWBSTATUS_SUCCESS;
}

UWBStatus_t uwb_fwdl_downloadFw(uwb_fwdl_provider_t *pCtx)
{
    NXPLOG_UWB_FWDNLD_D("uwb_fwdl_downloadFw Enter");
    uint8_t ackCls, ackIns;
    uint16_t lrc;
    uint32_t dataSz, payloadSz;
    size_t rcvLen = 0;

    ackCls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Ack);
    ackIns = (uint8_t)phHbci_Valid_APDU;

    do {
        if (pCtx->fwSize > PHHBCI_MAX_LEN_DATA_MOSI) {
            dataSz    = PHHBCI_MAX_LEN_DATA_MOSI;
            payloadSz = PHHBCI_APDU_SEG_FLAG;
        }
        else {
            lrc       = pCtx->fwSize ? PHHBCI_LEN_LRC : 0;
            dataSz    = pCtx->fwSize;
            payloadSz = (uint32_t)(dataSz + lrc);
        }
        pCtx->uwb_fwdl_MosiApdu.len = (uint16_t)payloadSz;
        rcvLen                      = PHHBCI_MAX_LEN_DATA_MISO;
        (void)phTmlUwb_hbci_transceive(
            (uint8_t *)&pCtx->uwb_fwdl_MosiApdu, PHHBCI_LEN_HDR, (uint8_t *)&pCtx->uwb_fwdl_MisoApdu, &rcvLen);
        pCtx->uwb_fwdl_MisoApdu.len = (uint16_t)rcvLen;
        if ((pCtx->uwb_fwdl_MisoApdu.cls != ackCls) || (pCtx->uwb_fwdl_MisoApdu.ins != ackIns)) {
            NXPLOG_UWB_FWDNLD_E(
                "ERROR: NACK (CLS = 0x%02x, INS = 0x%02x)", pCtx->uwb_fwdl_MisoApdu.cls, pCtx->uwb_fwdl_MisoApdu.ins);
            return kUWBSTATUS_FAILED;
        }

        if (dataSz) {
            pCtx->uwb_fwdl_MosiApdu.payload[0] = pCtx->uwb_fwdl_MosiApdu.cls;
            pCtx->uwb_fwdl_MosiApdu.payload[1] = pCtx->uwb_fwdl_MosiApdu.ins;
            pCtx->uwb_fwdl_MosiApdu.payload[2] = pCtx->uwb_fwdl_MosiApdu.len & 0x00FF;
            pCtx->uwb_fwdl_MosiApdu.payload[3] = (pCtx->uwb_fwdl_MosiApdu.len & 0xFF00) >> 8;

            phOsalUwb_MemCopy(&pCtx->uwb_fwdl_MosiApdu.payload[PHHBCI_LEN_HDR], pCtx->fwImgPtr, dataSz);

            pCtx->uwb_fwdl_MosiApdu.payload[PHHBCI_LEN_HDR + dataSz] =
                phHbci_CalcLrc(pCtx->uwb_fwdl_MosiApdu.payload, (uint16_t)(PHHBCI_LEN_HDR + dataSz));

            pCtx->fwImgPtr += dataSz;
            pCtx->fwSize -= dataSz;

            payloadSz = (uint16_t)(dataSz + PHHBCI_LEN_LRC);
            rcvLen    = PHHBCI_MAX_LEN_DATA_MISO;
            (void)phTmlUwb_hbci_transceive((uint8_t *)&pCtx->uwb_fwdl_MosiApdu.payload[PHHBCI_LEN_HDR],
                (uint16_t)payloadSz,
                (uint8_t *)&pCtx->uwb_fwdl_MisoApdu,
                &rcvLen);
            pCtx->uwb_fwdl_MisoApdu.len = (uint16_t)rcvLen;
            if ((pCtx->uwb_fwdl_MisoApdu.cls != ackCls) || (pCtx->uwb_fwdl_MisoApdu.ins != ackIns)) {
                NXPLOG_UWB_FWDNLD_E("ERROR: NACK (CLS = 0x%02x, INS = 0x%02x)",
                    pCtx->uwb_fwdl_MisoApdu.cls,
                    pCtx->uwb_fwdl_MisoApdu.ins);

                return kUWBSTATUS_FAILED;
            }
        }
    } while (pCtx->fwSize);

    return kUWBSTATUS_SUCCESS;
}
#endif // UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
