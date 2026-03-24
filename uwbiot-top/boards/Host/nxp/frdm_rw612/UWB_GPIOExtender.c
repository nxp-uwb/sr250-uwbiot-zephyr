/* Copyright 2022,2024-2025 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 *
 */

#include "UWB_GPIOExtender.h"
#include "phNxpLogApis_App.h"
#include "phUwb_BuildConfig.h"
#include "phUwbErrorCodes.h"
#include "UWB_GPIOExtender.h"
#include "boards.h"
#include "uwb_bus_board.h"

extern eEvkRevision gSr2xx_EvkBoard;
i2c_bus_board_ctx_t i2cCtx;

void GPIOExtenderSetRSTPin()
{
    GPIOExtenderSetPin(UWB_RSTN, GPIO_PIN_HIGH);
}

void GPIOExtenderResetRSTPin()
{
    GPIOExtenderSetPin(UWB_RSTN, GPIO_PIN_LOW);
}

void GPIOExtenderInit()
{
    if (gSr2xx_EvkBoard == VIRGO_REV_A) {
        GPIOExtenderSetMode(UWB_RF_ENABLE, GPIO_DIRECTION_OUT);
        GPIOExtenderSetPin(UWB_RF_ENABLE, GPIO_PIN_HIGH);
        GPIOExtenderSetMode(NFC_GPIO0, GPIO_DIRECTION_OUT);
        GPIOExtenderSetPin(NFC_GPIO0, GPIO_PIN_HIGH);

        // Set Direction
        GPIOExtenderSetMode(UWB_RSTN, GPIO_DIRECTION_OUT);
        GPIOExtenderSetMode(NFC_RSTN, GPIO_DIRECTION_OUT);
    }
    else if (gSr2xx_EvkBoard == CRETE_REV_B) {
        GPIOExtenderSetMode(UWB_RSTN, GPIO_DIRECTION_OUT);
    }
    else {
        return;
    }
}

void GPIOExtenderSetPinConfig(IoExpnParamInfo_t *IoExpnParam)
{
    uint8_t buff[2] = {0};

    buff[0] = IoExpnParam->reg;
    /*check which pins are set*/
    BOARD_I2C_Send(&i2cCtx, IoExpnParam->expanderAddr, &buff[0], 01);
    BOARD_I2C_Receive(&i2cCtx, IoExpnParam->expanderAddr, &buff[1], 01);
    /*set bitpos*/
    if (IoExpnParam->state == GPIO_PIN_HIGH) {
        buff[1] = buff[1] | (1 << IoExpnParam->bitPos);
    }
    /*reset bitpos*/
    else if (IoExpnParam->state == GPIO_PIN_LOW) {
        buff[1] = buff[1] & ~(1 << IoExpnParam->bitPos);
    }

    BOARD_I2C_Send(&i2cCtx, IoExpnParam->expanderAddr, &buff[0], 02);
}

uint32_t GPIOExtenderGetPinConfig(IoExpnParamInfo_t *IoExpnParam)
{
    uint8_t buff[2]     = {0};
    uint32_t gpio_value = 0;

    buff[0] = IoExpnParam->reg;
    /*check which pins are set*/
    BOARD_I2C_Send(&i2cCtx, IoExpnParam->expanderAddr, &buff[0], 01);
    BOARD_I2C_Receive(&i2cCtx, IoExpnParam->expanderAddr, &buff[1], 01);
    /*get bitpos value*/
    gpio_value = 1 & (buff[1] >> IoExpnParam->bitPos);
    return gpio_value;
}

void GPIOExtenderSetMode(eBitPos Pin, bool direction)
{
    IoExpnParamInfo_t IoExpnParam = {0};
    IoExpnParam.state             = direction;
    if (gSr2xx_EvkBoard == CRETE_REV_B) {
        // Direction are reversed in case of crete evk
        IoExpnParam.state = !(IoExpnParam.state);
    }
    switch (Pin) {
    case UWB_RF_ENABLE:
    case NFC_GPIO0:
    case EXT_OSC_ENABLE: {
        IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
        IoExpnParam.reg          = GPIO_DIRECTION_A;
        IoExpnParam.bitPos       = Pin;

    } break;
    case UWB_CLK_REQ1:
    case UWB_CLK_REQ2: {
        IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
        IoExpnParam.reg          = GPIO_DIRECTION_B;
        IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
    } break;
    case NFC_RSTN:
    case UWB_RSTN: {
        if (gSr2xx_EvkBoard == CRETE_REV_B) {
            IoExpnParam.expanderAddr = CRETE_GPIO_EXTENDER_SLAVE_ADDR;
            IoExpnParam.reg          = GPIO_REGISTER_CONFIG;
            IoExpnParam.bitPos       = (eBitPos)(Pin - 2);
        }
        if (gSr2xx_EvkBoard == VIRGO_REV_A) {
            IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
            IoExpnParam.reg          = GPIO_DIRECTION_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
    } break;
    default:
        LOG_E("Selected Pin is not correct ");
        return;
    }
    GPIOExtenderSetPinConfig(&IoExpnParam);
}

void GPIOExtenderSetPin(eBitPos Pin, bool state)
{
    IoExpnParamInfo_t IoExpnParam = {0};
    IoExpnParam.state             = state;
    switch (Pin) {
    case UWB_RF_ENABLE:
    case NFC_GPIO0:
    case EXT_OSC_ENABLE: {
        IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
        IoExpnParam.reg          = GPO_DATA_OUT_A;
        IoExpnParam.bitPos       = Pin;
    } break;
    case UWB_CLK_REQ1:
    case UWB_CLK_REQ2: {
        IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
        IoExpnParam.reg          = GPO_DATA_OUT_B;
        IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
    } break;
    case NFC_RSTN:
    case UWB_RSTN: {
        if (gSr2xx_EvkBoard == CRETE_REV_B) {
            IoExpnParam.expanderAddr = CRETE_GPIO_EXTENDER_SLAVE_ADDR;
            IoExpnParam.reg          = GPIO_REGISTER_WRITE;
            IoExpnParam.bitPos       = (eBitPos)(Pin - 2);
        }
        if (gSr2xx_EvkBoard == VIRGO_REV_A) {
            IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
    } break;
    default:
        LOG_E("Selected Pin is not correct ");
        return;
    }
    GPIOExtenderSetPinConfig(&IoExpnParam);
}

bool GPIOExtenderGetPin(eBitPos Pin, uint32_t *val)
{
    IoExpnParamInfo_t IoExpnParam = {0};

    switch (Pin) {
    case UWB_RF_ENABLE:
    case NFC_GPIO0:
    case EXT_OSC_ENABLE: {
        IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
        IoExpnParam.reg          = GPO_DATA_OUT_A;
        IoExpnParam.bitPos       = Pin;
    } break;
    case UWB_CLK_REQ1:
    case UWB_CLK_REQ2: {
        IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
        IoExpnParam.reg          = GPO_DATA_OUT_B;
        IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
    } break;
    case NFC_RSTN:
    case UWB_RSTN: {
        if (gSr2xx_EvkBoard == CRETE_REV_B) {
            IoExpnParam.expanderAddr = CRETE_GPIO_EXTENDER_SLAVE_ADDR;
            IoExpnParam.reg          = GPIO_REGISTER_WRITE;
            IoExpnParam.bitPos       = (eBitPos)(Pin - 1);
        }
        if (gSr2xx_EvkBoard == VIRGO_REV_A) {
            IoExpnParam.expanderAddr = VIRGO_GPIO_EXTENDER_SLAVE_ADDR;
            IoExpnParam.reg          = GPO_DATA_OUT_B;
            IoExpnParam.bitPos       = (eBitPos)(Pin - MASK_BIT_POS);
        }
    } break;
    default:
        LOG_E("Selected Pin is not correct ");
        return FALSE;
    }
    *val = GPIOExtenderGetPinConfig(&IoExpnParam);
    return TRUE;
}
