/* Copyright 2020 , 2023, 2025,2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef _UWB_SPI_PNP_H_
#define _UWB_SPI_PNP_H_

#include "phUwbTypes.h"
#include "phUwbStatus.h"
#include "phUwb_BuildConfig.h"
#include "uwb_uwbs_tml_interface.h"
#include "UwbPnpInternal.h"
#if UWBIOT_UWBD_SR2XXT
#include "phNxpUciHal_fwd.h"
#endif

UWBStatus_t UWB_HeliosSpiInit(void);
void UWB_HeliosSpiDeInit(void);
UWBStatus_t UWB_SpiHbciXfer(uint8_t *data, uint16_t len, uint8_t *rsp, size_t *rspLen);
UWBStatus_t UWB_SpiHbciXferWithLen(uint8_t *data, uint16_t len, uint8_t *rsp, size_t rspLen);
#if (UWBIOT_OS_FREERTOS || UWBIOT_OS_ZEPHYR)
void UWB_Uwbs_Enable_Interrupt();
extern bool UWB_Uwbs_ConsumeIRQ_Interrupt(void);
#endif //(UWBIOT_OS_FREERTOS || UWBIOT_OS_ZEPHYR)
#if UWBIOT_OS_NATIVE
void UWB_Uwbs_Disable_Interrupt();
#endif
UWBStatus_t UWB_SpiUciWrite(uint8_t *data, uint16_t len);
UWBStatus_t UWB_SpiUciRead(uint8_t *rsp, size_t *rspLen);
UWBStatus_t UWB_Tml_Io_Init();

#if (UWBIOT_TML_I2C || UWBIOT_TML_SPI)
void hbci_GPIOwait_ready(void);
bool UWB_Uwbs_Interupt_Status();
#endif //(UWBIOT_TML_I2C || UWBIOT_TML_SPI)
void UWB_Tml_Set_Mode(uwb_uwbs_tml_mode_t mode);
UWBStatus_t UWB_Tml_Io_Set(uwbs_io_t ioPin, bool_t value);

#if UWBIOT_UWBD_SR2XXT
void UWB_Tml_Chip_Reset(void);
phFWD_Status_t UWB_Tml_GetHdllReadyNtf();
UWBStatus_t UWB_Tml_Hdll_Read(uint8_t *pBuffer, size_t *pRspBufLen);
UWBStatus_t UWB_Tml_Hdll_WriteRead(uint8_t *pBuffer, uint16_t txBufLen, uint8_t *rsp_buf, size_t *rsp_buf_len);
#endif

#if (UWBIOT_UWBD_SR04X)
UWBStatus_t UWB_SpiRciTransceive(uint8_t *data, uint16_t len);
UWBStatus_t UWB_Tml_Reset_Uwbs();
void UWB_Tml_flush_read_buffer();

#define RCI_COMMAND_LENGTH 256
/*UCI/SWUP interface configuration*/
/** interface handler */
typedef enum
{
    /* UCI application mode */
    kInterfaceModeUci = 0x00u,
    /* RCI/SWUP application mode */
    kInterfaceModeSwup = 0x01u,
} interface_handler_t;
typedef struct
{
    interface_handler_t interface_mode;
} interface_config_t;

#endif // UWBIOT_UWBD_SR04X
#endif // _UWB_SPI_PNP_H_
