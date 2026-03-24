/* Copyright 2022 NXP
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

#if (UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD == 1)

/* Flash based FW download */
#include "phNxpLogApis_FwDnld.h"
#include <phTmlUwb_transport.h>
#include "UwbAdaptation.h"

#include "uwb_fwdl_provider.h"
#include "uwb_extfl_provider_interface.h"

#define MIN_FW_SIZE (100 * 1024) /* Minimum firmware size 100kb */
#define MAX_FW_SIZE (400 * 1024) /* Maximum firmware size 400kb */

extern uwb_fwdl_provider_t fwdlCtx;
uwb_fwdl_provider_ctx_t fwdl_ctx;

UWBStatus_t uwb_fwdl_setFwImage(const phUwbFWImageContext_t *const pAppfwImageCtx)
{
    NXPLOG_UWB_FWDNLD_I("FWDL from External Flash");
    bool_t ret;
    uint32_t fwImgSz = 0;

    if (uwb_fwdl_provider_init(&fwdl_ctx) != kUWBSTATUS_SUCCESS) {
        NXPLOG_UWB_FWDNLD_E("Firmware Download provider Failed");
        return kUWBSTATUS_FAILED;
    }

    ret = uwb_fwdl_provider_setmode(&fwdl_ctx, (eFirmwareMode)pAppfwImageCtx->fwMode);
    if (ret != TRUE) {
        NXPLOG_UWB_FWDNLD_E("Firmware Download provider SetMode Failed");
        return kUWBSTATUS_FAILED;
    }

    if (uwb_fwdl_provider_get_fwLength(&fwdl_ctx, &fwImgSz) != kUWBSTATUS_SUCCESS) {
        return kUWBSTATUS_FAILED;
    }

    if (fwImgSz <= MIN_FW_SIZE || fwImgSz > MAX_FW_SIZE) {
        NXPLOG_UWB_FWDNLD_E("No Firmware Found In the External Flash");
        return kUWBSTATUS_FAILED;
    }

    NXPLOG_UWB_FWDNLD_I("Firmware size: %d", fwImgSz);
    fwdlCtx.fwSize = fwImgSz;

    return kUWBSTATUS_SUCCESS;
}

UWBStatus_t uwb_fwdl_downloadFw(uwb_fwdl_provider_t *pCtx)
{
    NXPLOG_UWB_FWDNLD_D("uwb_fwdl_downloadFw Enter");
    uint8_t ackCls, ackIns;
    uint16_t lrc;
    uint32_t dataSz, payloadSz;
    size_t rcvLen = 0;

    uint32_t AddrIncr = 0;

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

            fwdl_ctx.flashStartAddr = fwdl_ctx.fwStartAddr + AddrIncr;
            (void)uwb_fwdl_provider_data_read(&fwdl_ctx, &pCtx->uwb_fwdl_MosiApdu.payload[PHHBCI_LEN_HDR], dataSz);

            pCtx->uwb_fwdl_MosiApdu.payload[PHHBCI_LEN_HDR + dataSz] =
                phHbci_CalcLrc(pCtx->uwb_fwdl_MosiApdu.payload, (uint16_t)(PHHBCI_LEN_HDR + dataSz));

            AddrIncr += dataSz;
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

                (void)uwb_fwdl_provider_deinit(&fwdl_ctx);

                return kUWBSTATUS_FAILED;
            }
        }
    } while (pCtx->fwSize);

    return kUWBSTATUS_SUCCESS;
}
#endif // FLASH_BASED_FWDOWNLOAD
