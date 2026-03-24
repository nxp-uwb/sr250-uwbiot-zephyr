/*
 * Copyright (C) 2026 NXP Semiconductors
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

/* System includes */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <uwb_logging.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <boards.h>
#include <board.h>
#include <zephyr/drivers/i2c.h>

/* UWB includes */
#include "driver_config.h"
#include "phUwbTypes.h"

#include "uwb_bus_interface.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phOsalUwb.h"
#include "phUwb_BuildConfig.h"
#include "phUwbErrorCodes.h"

#define I2C_DEV_NODE DT_ALIAS(i2c_node)
eEvkRevision gSr2xx_EvkBoard = CRETE_REV_B;

uwb_bus_status_t BOARD_I2C_Init(i2c_bus_board_ctx_t *pCtx)
{
    uint32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER;

    if (pCtx == NULL) {
        LOG_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }

    pCtx->i2cMasterHandle = DEVICE_DT_GET(I2C_DEV_NODE);
    if (!pCtx->i2cMasterHandle) {
        LOG_E("Error in i2c device_get_binding \n");
        return kUWB_bus_Status_FAILED;
    }

    /* Test i2c_configure() */
    if (i2c_configure(pCtx->i2cMasterHandle, i2c_cfg)) {
        LOG_E("Error in i2c_configure \n");
        return kUWB_bus_Status_FAILED;
    }

    return kUWB_bus_Status_OK;
}

uwb_bus_status_t BOARD_I2C_Send(i2c_bus_board_ctx_t *pCtx, unsigned char slave_addr, uint8_t *pBuf, size_t bufLen)
{
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;
    int status                  = -1;

    if (pCtx == NULL) {
        NXPLOG_UWB_TML_E("uwbs bus context is NULL");
        goto exit;
    }

    if (pBuf == NULL || bufLen == 0) {
        NXPLOG_UWB_TML_E("uwbs data buffer invalid");
        goto exit;
    }

    status = i2c_write(pCtx->i2cMasterHandle, pBuf, bufLen, (uint16_t)slave_addr);
    if (status < 0) {
        NXPLOG_UWB_TML_D("i2c write failed");
        goto exit;
    }
    bus_status = kUWB_bus_Status_OK;
exit:
    return bus_status;
}

uwb_bus_status_t BOARD_I2C_Receive(i2c_bus_board_ctx_t *pCtx, unsigned char slave_addr, uint8_t *pBuf, size_t pBufLen)
{
    uwb_bus_status_t bus_status = kUWB_bus_Status_FAILED;
    int status                  = -1;

    if (pCtx == NULL) {
        NXPLOG_UWB_TML_E("uwbs bus context is NULL");
        goto exit;
    }

    if (pBuf == NULL || pBufLen == 0) {
        NXPLOG_UWB_TML_E("uwbs bus Rx Buffer is NULL");
        goto exit;
    }

    status = i2c_read(pCtx->i2cMasterHandle, pBuf, pBufLen, (uint16_t)slave_addr);
    if (status < 0) {
        NXPLOG_UWB_TML_E("i2c read failed");
        goto exit;
    }

    bus_status = kUWB_bus_Status_OK;
exit:
    return bus_status;
}

uwb_bus_status_t BOARD_I2C_Deinit(i2c_bus_board_ctx_t *pCtx)
{
    if (pCtx == NULL) {
        NXPLOG_UWB_TML_E("uwbs bus context is NULL");
        return kUWB_bus_Status_FAILED;
    }
    pCtx->i2cMasterHandle = NULL;
    return kUWB_bus_Status_OK;
}

uwb_bus_status_t BOARD_Get_Sr2xxEvkVersion(void)
{
    /* get the Rhodes 4 Board version */
    uint8_t dummyBuff[2] = {0};

    if (kUWB_bus_Status_OK != BOARD_I2C_Init(&i2cCtx)) {
        NXPLOG_UWB_TML_E("%s failed..", __FUNCTION__);
        return kUWB_bus_Status_FAILED;
    }

    if (kUWB_bus_Status_OK == BOARD_I2C_Send(&i2cCtx, VIRGO_GPIO_EXTENDER_SLAVE_ADDR, dummyBuff, sizeof(dummyBuff))) {
        /* Rev B board Detected */
        gSr2xx_EvkBoard = VIRGO_REV_A;
    }
    else {
        if (kUWB_bus_Status_OK ==
            BOARD_I2C_Send(&i2cCtx, CRETE_GPIO_EXTENDER_SLAVE_ADDR, dummyBuff, sizeof(dummyBuff))) {
            /* Rev C board Detected */
            gSr2xx_EvkBoard = CRETE_REV_B;
        }
        else {
            gSr2xx_EvkBoard = NO_IO_EXPANDER;
            NXPLOG_UWB_TML_E("EVK board not detected");
            return kUWB_bus_Status_FAILED;
        }
    }
    return kUWB_bus_Status_OK;
}
void AddDelayInMicroSec(int delay)
{
    k_sleep(K_USEC(delay));
}
