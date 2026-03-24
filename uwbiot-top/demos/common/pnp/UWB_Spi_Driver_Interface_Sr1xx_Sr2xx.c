/* Copyright 2022-2023,2025,2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"

#if !(UWBIOT_UWBD_SR04X)
#include <string.h>
#include <stdint.h>

#include "UWB_Spi_Driver_Interface.h"
#include <uwb_uwbs_tml_interface.h>
#include "uwb_uwbs_tml_io.h"

#include "phUwbTypes.h"
#include "phUwbErrorCodes.h"
#include "phNxpLogApis_App.h"

uwb_uwbs_tml_ctx_t gUwbsPnpTmlCtx;

UWBStatus_t UWB_HeliosSpiInit(void)
{
    UWBStatus_t retStatus = uwb_uwbs_tml_init(&gUwbsPnpTmlCtx);
    if (retStatus != kUWBSTATUS_SUCCESS) {
        LOG_E("UWB_HeliosSpiInit failed");
    }
    return retStatus;
}

UWBStatus_t UWB_SpiHbciXfer(uint8_t *data, uint16_t len, uint8_t *rsp, size_t *rspLen)
{
    UWBStatus_t status = kUWBSTATUS_FAILED;
    status             = uwb_uwbs_tml_data_tx(&gUwbsPnpTmlCtx, data, len);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hbci_data_trx write data failed");
        goto end;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HBCI Tx >", data, len);
#endif // (FWDNLD_LOG_LEVEL)
    status = uwb_uwbs_tml_data_rx(&gUwbsPnpTmlCtx, rsp, rspLen);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hbci_data_trx read data failed");
        goto end;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HBCI Rx >", rsp, *rspLen);
#endif // (FWDNLD_LOG_LEVEL)
end:
    AddDelayInMicroSec(50);
    return status;
}

UWBStatus_t UWB_SpiHbciXferWithLen(uint8_t *data, uint16_t len, uint8_t *rsp, size_t rspLen)
{
    UWBStatus_t status = kUWBSTATUS_FAILED;
    uwb_bus_status_t bus_status;
    status = uwb_uwbs_tml_data_tx(&gUwbsPnpTmlCtx, data, len);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hbci_data_trx write data failed");
        goto end;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HBCI Tx >", data, len);
#endif //(FWDNLD_LOG_LEVEL)

#if (UWBIOT_TML_I2C || UWBIOT_TML_SPI)
    hbci_GPIOwait_ready();
#endif //(UWBIOT_TML_I2C || UWBIOT_TML_SPI)
    bus_status = uwb_bus_data_rx(&gUwbsPnpTmlCtx.busCtx, rsp, rspLen);
    if (bus_status != kUWB_bus_Status_OK) {
        LOG_E("uwb_bus_data_rx : reading from helios failed");
        status = kUWBSTATUS_FAILED;
        goto end;
    }
#if FWDNLD_LOG_LEVEL
    LOG_MAU8_D("HBCI Rx >", rsp, rspLen);
#endif //(FWDNLD_LOG_LEVEL)
    status = kUWBSTATUS_SUCCESS;
end:
    AddDelayInMicroSec(50);
    return status;
}

UWBStatus_t UWB_SpiUciRead(uint8_t *rsp, size_t *rspLen)
{
    UWBStatus_t status = uwb_uwbs_tml_data_rx(&gUwbsPnpTmlCtx, rsp, rspLen);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("UWB_SpiUciRead failed");
        *rspLen = 0;
    }
    return status;
}

UWBStatus_t UWB_SpiUciWrite(uint8_t *data, uint16_t len)
{
    return uwb_uwbs_tml_data_tx(&gUwbsPnpTmlCtx, data, len);
}

#if (UWBIOT_TML_I2C || UWBIOT_TML_SPI)
void hbci_GPIOwait_ready(void)
{
    uwbs_io_state_t gpioValue = kUWBS_IO_State_NA;
    uwb_bus_io_val_get(&gUwbsPnpTmlCtx.busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue);
    while (!gpioValue) {
        uwb_bus_io_val_get(&gUwbsPnpTmlCtx.busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue);
    }
}

bool UWB_Uwbs_Interupt_Status()
{
    uwbs_io_state_t gpioValue = kUWBS_IO_State_NA;
    if (uwb_bus_io_val_get(&gUwbsPnpTmlCtx.busCtx, kUWBS_IO_I_UWBS_IRQ, &gpioValue) != kUWB_bus_Status_OK) {
        LOG_E("UWB_Uwbs_Interupt_Status : uwb_bus_io_val_get failed");
    }
    return (bool)gpioValue;
}
#endif //(UWBIOT_TML_I2C || UWBIOT_TML_SPI)

#if UWBIOT_OS_FREERTOS || UWBIOT_OS_ZEPHYR
void UWB_Uwbs_Enable_Interrupt()
{
    if (uwb_bus_io_uwbs_irq_enable(&gUwbsPnpTmlCtx.busCtx) != kUWB_bus_Status_OK) {
        LOG_E("uwb_bus_io_uwbs_irq_enable failied");
    }
}

bool UWB_Uwbs_ConsumeIRQ_Interrupt()
{
    /** Acquire IRQ if present */
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(gUwbsPnpTmlCtx.busCtx.mIrqWaitSem, 1) != UWBSTATUS_SUCCESS) {
        LOG_D("phOsalUwb_ConsumeSemaphore_WithTimeout failed");
        return false;
    }
    return true;
}
#endif

#if UWBIOT_OS_NATIVE
void UWB_Uwbs_Disable_Interrupt()
{
    if (uwb_bus_io_uwbs_irq_disable(&gUwbsPnpTmlCtx.busCtx) != kUWB_bus_Status_OK) {
        LOG_E("uwb_bus_io_uwbs_irq_disable failied");
    }
}
#endif

void UWB_Tml_Set_Mode(uwb_uwbs_tml_mode_t mode)
{
    if (uwb_uwbs_tml_setmode(&gUwbsPnpTmlCtx, mode) != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_setmode failed");
    }
}

UWBStatus_t UWB_Tml_Io_Init()
{
    UWBStatus_t retStatus = kUWBSTATUS_INVALID_DEVICE;
    if (uwb_bus_io_init(&gUwbsPnpTmlCtx.busCtx) == kUWB_bus_Status_OK) {
        retStatus = kUWBSTATUS_SUCCESS;
    }
    return retStatus;
}

UWBStatus_t UWB_Tml_Io_Set(uwbs_io_t ioPin, bool_t value)
{
    if ((uwb_bus_io_val_set(&gUwbsPnpTmlCtx.busCtx, ioPin, (uwbs_io_state_t)value)) == kUWB_bus_Status_OK) {
        return kUWBSTATUS_SUCCESS;
    }
    return kUWBSTATUS_INVALID_DEVICE;
}

#if UWBIOT_UWBD_SR2XXT
void UWB_Tml_Chip_Reset(void)
{
    UWB_Tml_Io_Set(kUWBS_IO_O_RSTN, 0);
    phOsalUwb_Delay(10);
    UWB_Tml_Io_Set(kUWBS_IO_O_RSTN, 1);
    // TODO: added for switching helios to UCI mode
    // phOsalUwb_Delay(10);
}

UWBStatus_t UWB_Tml_Hdll_Read(uint8_t *pBuffer, size_t *pRspBufLen)
{
    size_t bufLen = 0;
    if (uwb_uwbs_tml_data_rx(&gUwbsPnpTmlCtx, pBuffer, &bufLen) != kUWBSTATUS_SUCCESS) {
        LOG_D("UWB_Tml_Hdll_Read : uwb_uwbs_tml_data_rx failed");
        return kUWBSTATUS_FAILED;
    }
    *pRspBufLen = bufLen;
    return kUWBSTATUS_SUCCESS;
}

UWBStatus_t UWB_Tml_Hdll_WriteRead(uint8_t *pBuffer, uint16_t txBufLen, uint8_t *rsp_buf, size_t *rsp_buf_len)
{
    UWBStatus_t status = kUWBSTATUS_FAILED;
    *rsp_buf_len       = 0;
    status             = uwb_uwbs_tml_data_tx(&gUwbsPnpTmlCtx, pBuffer, txBufLen);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hdll_data_trx write data failed");
        goto end;
    }
    status = uwb_uwbs_tml_data_rx(&gUwbsPnpTmlCtx, rsp_buf, rsp_buf_len);
    if (status != kUWBSTATUS_SUCCESS) {
        LOG_E("uwb_uwbs_tml_hdll_data_trx read data failed");
        goto end;
    }
    LOG_MAU8_I("RECV: ", rsp_buf, *rsp_buf_len);
end:
    AddDelayInMicroSec(50);
    return status;
}

phFWD_Status_t Hdll_GetApdu(uint8_t *pApdu, uint16_t sz, size_t *rsp_buf_len)
{
    if (sz == 0 || sz > PHHDLL_MAX_LEN_PAYLOAD_MISO) {
        LOG_E(
            "ERROR: Hdll_GetApdu data len is 0 or greater than max "
            "palyload length supported\n");
        return FW_DNLD_FAILURE;
    }
    if (UWB_Tml_Hdll_Read(pApdu, rsp_buf_len) == kUWBSTATUS_SUCCESS) {
        LOG_MAU8_I("RECV:", pApdu, *rsp_buf_len);
        return FW_DNLD_SUCCESS;
    }

    LOG_E("ERROR: Get APDU %u bytes failed!\n", sz);
    return FW_DNLD_FAILURE;
}

phFWD_Status_t UWB_Tml_GetHdllReadyNtf()
{
    uint8_t rsp_buf[HDLL_READ_BUFF_SIZE] = {0};
    phFWD_Status_t ret                   = FW_DNLD_FAILURE;
    size_t rsp_buf_len                   = 0x0;

    LOG_D("Wait for HDL_READY notification\n");
    ret = Hdll_GetApdu((uint8_t *)&rsp_buf[0], HDLL_READ_BUFF_SIZE, &rsp_buf_len);

    if (!rsp_buf_len || ret == FW_DNLD_FAILURE) {
        LOG_D("Error in reading GET_HDL_READY notification\n");
        return ret;
    }

    ret = process_hdll_response(rsp_buf, rsp_buf_len, OP_PROTOCOL_HDLL, READY, HCP_TYPE_NOTIFICATION);
    return ret;
}

#endif // UWBIOT_UWBD_SR2XXT

#endif // !(UWBIOT_UWBD_SR04X)
