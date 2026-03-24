/* Copyright 2024,2025 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef __UWB_VCOM_PNP_H_
#define __UWB_VCOM_PNP_H_
#include <stdint.h>
#if defined(CPU_LPC55S69JBD100_cm33) || defined(CPU_LPC54628J512ET180) || defined(CPU_MIMXRT1176DVMAA) || \
    defined(CPU_RW612ETA2I)

#define USB_TX_RETRY_COUNT 3

/** used in all boards that support USB*/
uint32_t UWB_Usb_UciSendNtfn(uint8_t *pData, uint16_t size);
uint32_t UWB_Usb_SendUCIRsp(uint8_t *pData, uint16_t size);
uint32_t UWB_Usb_SendRsp(uint8_t *pData, uint16_t size);
uint32_t UWB_Vcom_SendInternalRsp(uint8_t *pData, uint16_t size);
void Uwb_Usb_Init(void (*rcvCb)(uint8_t *, uint32_t *));

#define Uwb_Vcom_Init Uwb_Usb_Init
#if UWBIOT_OS_FREERTOS
#define UWB_Vcom_UciSendNtfn UWB_Usb_UciSendNtfn
#define UWB_Vcom_SendUCIRsp  UWB_Usb_SendUCIRsp
#else
#define UWB_Vcom_UciSendNtfn UWB_Usb_SendRsp
#define UWB_Vcom_SendUCIRsp  UWB_Usb_SendRsp
#endif // UWBIOT_OS_FREERTOS
#define UWB_Vcom_SendRsp         UWB_Usb_SendRsp
#define UWB_Vcom_SendInternalRsp UWB_Usb_SendRsp
#endif // defined(CPU_LPC55S69JBD100_cm33)

#define MAX_UWBS_SPI_TRANSFER_TIMEOUT (1000)

#if defined(NRF52_SERIES)

/** used in NRF52 Series*/
void Uwb_UART_Init(void (*rcvCb)(uint8_t *, uint32_t *));
uint32_t transmitToUart(uint8_t *pData, size_t size);

#define HEADER_SIZE_USB_PNP 3
#define SET_LED_RED()
#define SET_LED_BLUE()
#define SET_LED_GREEN()
#define CLEAR_LED_RED()
#define CLEAR_LED_BLUE()
#define CLEAR_LED_GREEN()

#define Uwb_Vcom_Init            Uwb_UART_Init
#define Uwb_Vcom_Init            Uwb_UART_Init
#define UWB_Vcom_UciSendNtfn     transmitToUart
#define UWB_Vcom_SendUCIRsp      transmitToUart
#define UWB_Vcom_SendRsp         transmitToUart
#define UWB_Vcom_SendInternalRsp transmitToUart
#endif // defined(NRF52_SERIES)

#if defined(QN9090_SERIES)

/** common for Qn9090 Series*/
uint32_t transmitToUsart(uint8_t *pData, size_t size);
void Uwb_USART_Init(void (*rcvCb)(uint8_t *, uint32_t *));

#define Uwb_Vcom_Init            Uwb_USART_Init
#define UWB_Vcom_UciSendNtfn     transmitToUsart
#define UWB_Vcom_SendUCIRsp      transmitToUsart
#define UWB_Vcom_SendRsp         transmitToUsart
#define UWB_Vcom_SendInternalRsp transmitToUsart

#define USART_DEVICE_INTERRUPT_PRIORITY (3U)
#endif // defined(QN9090_SERIES)

#endif /* __UWB_VCOM_PNP_H_ */
