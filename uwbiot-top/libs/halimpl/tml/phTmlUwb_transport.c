/*
 * Copyright 2012-2020,2022-2024, 2026 NXP.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>
#include <limits.h>

#include "phNxpUciHal.h"
#include "phNxpUciHal_utils.h"

#include "phTmlUwb_transport.h"
#include "phUwbStatus.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwb_BuildConfig.h"
#include <uwb_uwbs_tml_interface.h>
#include "uwb_uwbs_tml_io.h"

/** Local Function prototypes */
#if (UWBIOT_UWBD_SR04X) && UWBIOT_TML_SPI
static void helios_irq_cb(void *args);
#endif /* (UWBIOT_UWBD_SR04X) && UWBIOT_TML_SPI */

/** Global Variables */
/* UCI HAL Control structure */
static int gInitDone = 0xFF;
/** UWB Subsystem tml interface context */
uwb_uwbs_tml_ctx_t gUwbsTmlCtx;

#if (UWBIOT_UWBD_SR04X) && UWBIOT_TML_SPI
static void helios_irq_cb(void *args)
{
    // Disable interrupt
    uwb_uwbs_tml_ctx_t *pCtx = (uwb_uwbs_tml_ctx_t *)args;
    uwb_bus_io_irq_dis(&pCtx->busCtx, kUWBS_IO_I_UWBS_IRQ);
}
#endif /* (UWBIOT_UWBD_SR04X) && UWBIOT_TML_SPI */

UWBSTATUS phTmlUwb_open_and_configure(pphTmlUwb_Config_t pConfig, void **pLinkHandle)
{
    UWBSTATUS retStatus = UWBSTATUS_INVALID_DEVICE;

    if (uwb_uwbs_tml_init(&gUwbsTmlCtx) == kUWBSTATUS_SUCCESS) {
        gInitDone = 0;
        retStatus = UWBSTATUS_SUCCESS;
    }
    if (pLinkHandle != NULL) {
        /*
         *  Bus io init should be done only once.
         *  Null check added because for pnp and mctt and SR040(SWUP) we are calling this api with NUll.
         *  once we call from the params NULL we dont want to re initialize the IO configs.
         */
        retStatus = phTmlUwb_io_init();
        *pLinkHandle = (void *)&gInitDone;
    }
    return retStatus;
}

UWBSTATUS phTmlUwb_io_init()
{
    UWBSTATUS retStatus = UWBSTATUS_INVALID_DEVICE;
    if (uwb_bus_io_init(&gUwbsTmlCtx.busCtx) == kUWB_bus_Status_OK) {
        retStatus = UWBSTATUS_SUCCESS;
    }
    return retStatus;
}

UWBSTATUS phTmlUwb_io_deinit()
{
    UWBSTATUS retStatus = UWBSTATUS_INVALID_DEVICE;
    if (uwb_bus_io_deinit(&gUwbsTmlCtx.busCtx) == kUWB_bus_Status_OK) {
        retStatus = UWBSTATUS_SUCCESS;
    }
    return retStatus;
}

UWBSTATUS phTmlUwb_io_set(uwbs_io_t ioPin, bool_t value)
{
    if ((uwb_bus_io_val_set(&gUwbsTmlCtx.busCtx, ioPin, (uwbs_io_state_t)value)) == kUWB_bus_Status_OK) {
        return UWBSTATUS_SUCCESS;
    }
    return UWBSTATUS_INVALID_DEVICE;
}

// TODO: Temporary for sn110 and sr1xx simultaneous irq enablement
#if UWBIOT_SESN_SNXXX
void phTmlUwb_io_enable_uwb_irq()
{
    uwb_bus_io_uwbs_irq_enable(&gUwbsTmlCtx.busCtx);
}
#endif

void phTmlUwb_close()
{
    if (kUWBSTATUS_SUCCESS != uwb_uwbs_tml_deinit(&gUwbsTmlCtx)) {
        LOG_E("phTmlUwb_close : uwb_uwbs_tml_deinit failed");
    }
    gInitDone = 0xFF;
}

int phTmlUwb_uci_read(uint8_t *pBuffer, int nNbBytesToRead)
{
    size_t bufLen      = nNbBytesToRead;
    UWBStatus_t status = uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pBuffer, &bufLen);
    if (status != kUWBSTATUS_SUCCESS) {
#if UWBIOT_UWBD_SR1XXT_SR2XXT
        if (status == kUWBSTATUS_RESPONSE_TIMEOUT) {
            LOG_D("phTmlUwb_uci_read : Read IRQ Timedout");
            return UWBSTATUS_IRQ_READ_TIMEOUT;
        }
        else
#endif // UWBIOT_UWBD_SR1XXT_SR2XXT
        {
            LOG_D("phTmlUwb_uci_read : uwb_uwbs_tml_data_rx failed");
            return 0;
        }
    }
    if (bufLen > INT_MAX) {
        LOG_D("%s : Data Length exceeds INT_MAX",__FUNCTION__);
        return 0;
    }
    return (int)bufLen;
}

#if (UWBIOT_UWBD_SR04X)

int phTmlUwb_rci_read(uint8_t *pBuffer, int nNbBytesToRead)
{
    size_t bufLen = (size_t)nNbBytesToRead;
    if(phTmlUwb_set_mode_fwdld() != kUWBSTATUS_SUCCESS) {
        LOG_E("phTmlUwb_rci_read : phTmlUwb_set_mode_fwdld failed");
        return -1;
    }
    if (uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pBuffer, &bufLen) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_rci_read : uwb_uwbs_tml_data_rx failed");
    }
    return (int)bufLen;
}
#endif /* (UWBIOT_UWBD_SR04X) */

int phTmlUwb_uci_write(uint8_t *pBuffer, uint16_t nNbBytesToWrite)
{
    int numWrote                  = 0;
    const UWBStatus_t writeStatus = uwb_uwbs_tml_data_tx(&gUwbsTmlCtx, pBuffer, nNbBytesToWrite);

    if (writeStatus == kUWBSTATUS_SUCCESS) {
        numWrote = gUwbsTmlCtx.noOfBytesWritten;
    }
    else if (writeStatus == kUWBSTATUS_BUSY) {
        LOG_D("phTmlUwb_uci_write : uwb_uwbs_tml_data_tx failed");
        numWrote = -2;
    }
    else {
        LOG_D("phTmlUwb_uci_write : uwb_uwbs_tml_data_tx failed");
        numWrote = -1;
    }

    return numWrote;
}

#if (UWBIOT_UWBD_SR04X)

int phTmlUwb_rci_write(uint8_t *pBuffer, uint16_t nNbBytesToWrite)
{
    int numWrote = 0;
    if(phTmlUwb_set_mode_fwdld() != kUWBSTATUS_SUCCESS){
        LOG_E("phTmlUwb_rci_write : phTmlUwb_set_mode_fwdld failed");
        return -1;
    }
    if (uwb_uwbs_tml_data_tx(&gUwbsTmlCtx, pBuffer, nNbBytesToWrite) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_rci_write : uwb_uwbs_tml_data_tx failed");
        return -1;
    }
    numWrote = gUwbsTmlCtx.noOfBytesWritten;

    if (numWrote <= 0) {
        return -1;
    }
    else {
        return 0;
    }
}
#endif /* (UWBIOT_UWBD_SR04X) */

#if UWBIOT_UWBD_SR1XXT

UWBSTATUS phTmlUwb_hbci_transceive(uint8_t *pWriteBuf, size_t writeBufLen, uint8_t *pRespBuf, size_t *pRspBufLen)
{
    UWBSTATUS status;
    status           = uwb_uwbs_tml_data_tx(&gUwbsTmlCtx, pWriteBuf, writeBufLen);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_data_tx write data failed");
        goto end;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HBCI Tx >", pWriteBuf, writeBufLen);
#endif //(FWDNLD_LOG_LEVEL)
    status = uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pRespBuf, pRspBufLen);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hbci_data_trx read data failed");
        goto end;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HBCI Rx >", pRespBuf, *pRspBufLen);
#endif //(FWDNLD_LOG_LEVEL)
end:
    AddDelayInMicroSec(50);
    return status;
}

#endif // UWBIOT_UWBD_SR1XXT

#if UWBIOT_UWBD_SR2XXT

int phTmlUwb_hdll_read(uint8_t *pBuffer, uint16_t *pRspBufLen)
{
    size_t bufLen = 0;
    if(phTmlUwb_set_mode_fwdld() != kUWBSTATUS_SUCCESS){
        LOG_E("phTmlUwb_hdll_read : phTmlUwb_set_mode_fwdld failed");
        return -1;
    }
#if UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
    if (uwb_uwbs_tml_helios_get_hdll_edl_ntf(&gUwbsTmlCtx, pBuffer, &bufLen) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_hdll_read : uwb_uwbs_tml_data_rx failed");
        return -1;
    }
#else
    if (uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pBuffer, &bufLen) != kUWBSTATUS_SUCCESS) {
        LOG_D("phTmlUwb_hdll_read : uwb_uwbs_tml_data_rx failed");
        return -1;
    }
#endif

    if (bufLen <= UINT16_MAX) {
        *pRspBufLen = (uint16_t)bufLen;
    } else {
        LOG_E("Buffer length (%u) exceeds UINT16_MAX", bufLen);
        return -1;
    }

    return 0;
}

UWBSTATUS phTmlUwb_hdll_transceive(uint8_t *pWriteBuf, size_t writeBufLen, uint8_t *pRespBuf, size_t *pRspBufLen)
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    if(phTmlUwb_set_mode_fwdld() != kUWBSTATUS_SUCCESS){
        LOG_E("phTmlUwb_hdll_transceive : phTmlUwb_set_mode_fwdld failed");
        return status;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HDLL Tx >", &pWriteBuf[UCI_CMD_INDEX], writeBufLen);
#endif //FWDNLD_LOG_LEVEL

    *pRspBufLen = 0;
    status     = uwb_uwbs_tml_data_tx(&gUwbsTmlCtx, pWriteBuf, writeBufLen);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hdll_data_trx write data failed");
        goto end;
    }
#if UWBIOT_TML_PNP
#if UWBIOT_UWBD_SR2XXT
    // Delay increased to 30ms for SR200T as unexpected behaviour seen on USB port
    // USB port fails to open without any error notification if delay time is too low
    phOsalUwb_Delay(30);
#else
    phOsalUwb_Delay(10);
#endif
#elif UWBIOT_TML_SOCKET
    phOsalUwb_Delay(20);
#elif UWBIOT_UWBD_SR2XXT && UWBIOT_TML_SPI
    /* Add small delay before read operation to ensure hardware stability */
    phOsalUwb_Delay(10);
#endif
    status = uwb_uwbs_tml_data_rx(&gUwbsTmlCtx, pRespBuf, pRspBufLen);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hdll_data_trx read data failed");
        goto end;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HDLL Rx <", &pRespBuf[ACTUAL_PACKET_START], *pRspBufLen);
#endif //FWDNLD_LOG_LEVEL
end:
    AddDelayInMicroSec(50);
    return status;
}

#if UWBIOT_TML_PNP || UWBIOT_TML_SOCKET
UWBSTATUS phTmlUwb_hdll_reset(bool isFWDownloadDone)
{
    UWBSTATUS status = UWBSTATUS_FAILED;
    if(phTmlUwb_set_mode_fwdld() != kUWBSTATUS_SUCCESS){
        LOG_E("phTmlUwb_hdll_reset : phTmlUwb_set_mode_fwdld failed");
        return status;
    }
    if (uwb_uwbs_tml_helios_hdll_reset(&gUwbsTmlCtx, isFWDownloadDone) == kUWBSTATUS_SUCCESS) {
        status = UWBSTATUS_SUCCESS;
    }
    return status;
}
#endif // (UWBIOT_TML_PNP || UWBIOT_TML_SOCKET)
#endif // UWBIOT_UWBD_SR2XXT

UWBSTATUS phTmlUwb_reset_uwbs(void)
{
    UWBSTATUS status = UWBSTATUS_SUCCESS;
    if (uwb_uwbs_tml_reset(&gUwbsTmlCtx) != kUWBSTATUS_SUCCESS) {
        LOG_E("phTmlUwb_reset_uwbs: uwb_uwbs_tml_reset failed");
        status = UWBSTATUS_FAILED;
    }
    return status;
}

int phTmlUwb_reset(long level)
{
    NXPLOG_UWB_TML_D("phTmlUwb_reset(), VEN level %ld", level);
    phTmlUwb_ReadAbort();
#if ((UWBIOT_OS_NATIVE) && !((UWBIOT_TML_PNP) || (UWBIOT_TML_SOCKET)))
    /** Read abort for the Kernel mode */
    if (uwb_bus_io_uwbs_irq_disable(&gUwbsTmlCtx.busCtx) != kUWB_bus_Status_OK) {
        LOG_E("uwb_bus_io_uwbs_irq_disable failed");
    }
#endif // UWBIOT_OS_NATIVE
    return 1;
}

void phTmlUwb_set_mode_uci(void)
{
    if (uwb_uwbs_tml_setmode(&gUwbsTmlCtx, kUWB_UWBS_TML_MODE_UCI) != kUWBSTATUS_SUCCESS) {
        LOG_E("phTmlUwb_set_mode_uci : uwb_uwbs_tml_setmode failed");
    }
}

UWBSTATUS phTmlUwb_set_mode_fwdld(void)
{
#if (UWBIOT_UWBD_SR04X)
    uwb_uwbs_tml_mode_t tml_mode = kUWB_UWBS_TML_MODE_SWUP;
#elif UWBIOT_UWBD_SR1XXT
    uwb_uwbs_tml_mode_t tml_mode = kUWB_UWBS_TML_MODE_HBCI;
#elif UWBIOT_UWBD_SR2XXT
    uwb_uwbs_tml_mode_t tml_mode = kUWB_UWBS_TML_MODE_HDLL;
#else
    #error "Unsupported UWBD"
#endif /** (UWBIOT_UWBD_SR04X) */
    if (uwb_uwbs_tml_setmode(&gUwbsTmlCtx, tml_mode) != kUWBSTATUS_SUCCESS) {
        LOG_E("phTmlUwb_set_mode_fwdld : uwb_uwbs_tml_setmode failed");
        return kUWBSTATUS_FAILED;
    }
    return kUWBSTATUS_SUCCESS;
}

#if (UWBIOT_UWBD_SR04X)
#if UWBIOT_TML_SPI
void phTmlUwb_flush_read_buffer()
{
    uwb_uwbs_tml_flush_read_buffer(&gUwbsTmlCtx);
}
void phTmlUwb_helios_irq_enable()
{
    uwbs_io_callback callback;
    callback.fn   = &helios_irq_cb;
    callback.args = (void *)&gUwbsTmlCtx;
    if (uwb_bus_io_irq_en(&gUwbsTmlCtx.busCtx, kUWBS_IO_I_UWBS_IRQ, &callback) != kUWB_bus_Status_OK) {
        LOG_E("phTmlUwb_helios_irq_enable : uwb_bus_io_irq_en failed");
    }
}

bool phTmlUwb_rdy_read()
{
    uwbs_io_state_t gpioValue = kUWBS_IO_State_NA;
    if (uwb_bus_io_val_get(&gUwbsTmlCtx.busCtx, kUWBS_IO_I_UWBS_READY_IRQ, &gpioValue) != kUWB_bus_Status_OK) {
        LOG_E("phTmlUwb_rdy_read : uwb_bus_io_val_get failed");
    }
    return (bool)gpioValue;
}
#endif /* UWBIOT_TML_SPI */
#endif /* (UWBIOT_UWBD_SR04X) */

#if UWBIOT_TML_S32UART
void phTmlUwb_switch_protocol(uint8_t protocol)
{
    uwb_uwbs_tml_switch_protocol(&gUwbsTmlCtx, (uwb_uwbs_tml_mode_t)protocol);
}
#endif // UWBIOT_TML_S32UART
