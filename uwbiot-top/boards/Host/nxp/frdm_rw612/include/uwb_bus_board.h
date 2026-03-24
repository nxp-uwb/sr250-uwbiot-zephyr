/* Copyright 2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef __UWB_BUS_BOARD_H__
#define __UWB_BUS_BOARD_H__

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <uwb_uwbs_tml_io.h>
#include "phUwbTypes.h"

typedef enum
{
    WRITE_MODE = 0,
    READ_MODE
} op_mode_t;

typedef struct
{
    /*master handle */
    op_mode_t op_mode;
    /* SPI DT specification */
    struct spi_dt_spec *masterHandle;
    /*GPIO DT Specification*/
    struct gpio_dt_spec *irq_gpio;
    /*GPIO DT Specification*/
    struct gpio_dt_spec *rstn_gpio;
    /* This semaphore is use to wait for read interrupt from helios */
    void *mIrqWaitSem;
} uwb_bus_board_ctx_t;

typedef struct
{
    /* I2C device */
    const struct device *i2cMasterHandle;
    /* No of bytes read */
    int bytesRead;
} i2c_bus_board_ctx_t;

void AddDelayInMicroSec(int delay);
#endif /*__UWB_BUS_BOARD_H__*/
