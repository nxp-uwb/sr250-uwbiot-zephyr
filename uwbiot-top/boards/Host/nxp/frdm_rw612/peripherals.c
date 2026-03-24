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

#include <stddef.h>
#include <string.h>

/**
 * @brief Set up and initialize all required blocks and functions related to the peripherals hardware.
 */
void BOARD_InitBootPeripherals(void)
{
    /* The user initialization should be placed here */
}

void board_SerialGetCOMPort(char *comPortName, size_t *pInOutcomPortNameLen)
{
    *pInOutcomPortNameLen = sizeof("SPI");
    memcpy(comPortName, "SPI", sizeof("SPI"));
}
