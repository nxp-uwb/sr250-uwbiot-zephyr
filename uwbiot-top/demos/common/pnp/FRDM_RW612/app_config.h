/* Copyright 2025 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#define DEBUGOUT(...)

#if UWBIOT_OS_FREERTOS
#include "pin_mux.h"
#include "clock_config.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_descriptor.h"
#include "fsl_common.h"
#include "fsl_component_serial_manager.h"
#include "fsl_component_serial_port_internal.h"
#include "fsl_component_serial_port_usb.h"
#include "virtual_com.h"
#endif //UWBIOT_OS_FREERTOS

#endif // __APP_CONFIG_H__
