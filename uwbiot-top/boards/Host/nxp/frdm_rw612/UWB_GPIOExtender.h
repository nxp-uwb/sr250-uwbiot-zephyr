/* Copyright  2022, 2024-2025 NXP
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

#ifndef PHNXPUWB_GPIO_EXTENDER_H_
#define PHNXPUWB_GPIO_EXTENDER_H_

#include <stdbool.h>
#include <stdint.h>
#include <board.h>
#include "phUwb_BuildConfig.h"

#define GPIO_EXTENDER_SLAVE_ADDR       0x20
#define VIRGO_GPIO_EXTENDER_SLAVE_ADDR 0x34

#define MASK_BIT_POS 0x5

#define GPIO_DIRECTION_OUT 0x01
#define GPIO_DIRECTION_IN  0x00

#define GENERAL_CFG_CORE_FREQ(x) (((x)&0x03) << 5)
#define GENERAL_CFG_OSC_EN       (1 << 7)

/*Outlogic of pin*/
#define GPIO_PIN_HIGH 0x01
#define GPIO_PIN_LOW  0x00

typedef enum bitPos
{
    /* Enable VDD (1.8V) to RF */
    UWB_RF_ENABLE = 1,
    /* Used to power cycle the SN220 (Cold Reset) */
    NFC_GPIO0, // => Where why
    /* Enable external oscillator for UWB */
    EXT_OSC_ENABLE = 4,
    /* Clock request 1 input from UWB */
    UWB_CLK_REQ1,
    /* Clock request 2 input from UWB */
    UWB_CLK_REQ2,
    /* Enable for SE051 secure element */
    SE051_ENA,
    /* Reset signal for SN220 (active low) */
    NFC_RSTN, // => Should remain low
    /* Reset signal for UWB (active low) */
    UWB_RSTN,
} eBitPos;
typedef enum ioExpnReg
{
    /*Register Mapping*/
    /***************************************Vorgo EVK**************************************************/

    GPO_DATA_OUT_A = 0x23,
    /*Set the data for the GIPO B
     *  0 = sets output low.
     *  1 = sets output high.
     */
    GPO_DATA_OUT_B = 0x24,
    /*Set the Mode for the GIPO A
     *  0 = push/pull.
     *  1 = open drain.
     */
    GPO_OUT_MODE_A = 0x25,
    /*Set the Mode for the GIPO B
     *  0 = push/pull.
     *  1 = open drain.
     */
    GPO_OUT_MODE_B = 0x26,
    /*Set the Direction for the GIPO A
     *  0 = GPIO  is an input.
     *  1 = GPIO  is an output.
     */
    GPIO_DIRECTION_A = 0x27,
    /*Set the Direction for the GIPO B
     *  0 = GPIO  is an input.
     *  1 = GPIO  is an output.
     */
    GPIO_DIRECTION_B = 0x28,
    /***************************************Crete EVK**************************************************/

    /*Register Mapping*/

    GPIO_REGISTER_WRITE = 0x03,

    GPIO_REGISTER_CONFIG = 0x07
} eGPIOExtndrReg;

typedef struct
{
    eGPIOExtndrReg reg;
    eBitPos bitPos;
    uint8_t expanderAddr;
    bool state;
} IoExpnParamInfo_t;

void GPIOExtenderInit();
void GPIOExtenderSetRSTPin();
void GPIOExtenderResetRSTPin();
void GPIOExtenderSetPinConfig(IoExpnParamInfo_t *IoExpnParam);
uint32_t GPIOExtenderGetPinConfig(IoExpnParamInfo_t *IoExpnParam);
void GPIOExtenderSetMode(eBitPos Pin, bool direction);
void GPIOExtenderSetPin(eBitPos Pin, bool state);
bool GPIOExtenderGetPin(eBitPos Pin, uint32_t *val);
#endif // PHNXPUWB_GPIO_EXTENDER_H_
