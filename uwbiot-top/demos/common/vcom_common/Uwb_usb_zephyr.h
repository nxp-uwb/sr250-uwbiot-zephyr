/* Copyright 2025 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#ifndef _UWB_USB_H_
#define _UWB_USB_H_

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif
#if !defined(UWBIOT_APP_BUILD__DEMO_PNP)
#include "board.h"
#include <stdbool.h>
#include "phOsalUwb.h"
#if UWBIOT_OS_ZEPHYR
void Uwb_USB_Init(void (*rcvCb)(uint8_t *, uint32_t *));
uint32_t transmitToUsb(uint8_t *buffer, uint16_t size);
#define UWB_Serial_Com_Init Uwb_USB_Init
#define UWB_Serial_Com_DeInit()
#define UWB_Serial_Com_SendRsp transmitToUsb

#endif //_UWB_USB_H_
#endif // UWBIOT_OS_ZEPHYR
#endif // UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON
