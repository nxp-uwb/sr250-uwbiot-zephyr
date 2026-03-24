/*
 * Copyright 2019-2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*${header:start}*/
#include "uwb_board.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <boards.h>
#include <UWB_GPIOExtender.h>
#include "uwb_board.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

BUILD_ASSERT(
    DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart), "Console device is not ACM CDC UART device");

#define UWB1_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(nxp_uwb_device)

static struct device const *dev     = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

void BOARD_InitCDCDebugConsole(void)
{
    if (usb_enable(NULL)) {
        return;
    }
    uint32_t dtr = 0;
    /* Poll if the DTR flag was set */
    while (!dtr) {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        /* Give CPU resources to low priority threads. */
        k_sleep(K_MSEC(100));
    }
}

void BOARD_DeInitCDCDebugConsole(void)
{
    int ret;
    ret = usb_disable();
    if (ret) {
        PRINTF("Failed to disable USB (%d)", ret);
    }
}

/*${header:end}*/

/**
 **
 ** Function         board_init
 **
 ** Description      Board initialization
 **
 ** Returns          void
 **
 */
void board_init(void)
{
    BOARD_InitCDCDebugConsole();
}

void BOARD_GetMCUUid(uint8_t *aOutUid16B, uint8_t *pOutLen)
{
    const uint8_t uid[] = {"EMBED-LINUX"};
    if (*pOutLen >= sizeof(uid)) {
        memcpy(aOutUid16B, (uint8_t *)uid, sizeof(uid));
        *pOutLen = sizeof(uid);
    }
    else {
        *pOutLen = 0;
    }
}

/* Reboot MCU */
void BOARD_MCUReset()
{
    /* Not doing here anything currently */
}