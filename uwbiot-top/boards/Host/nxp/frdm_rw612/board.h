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

#include <stdint.h>

void hardware_init(void);
void BOARD_MCUReset();
void BOARD_GetMCUUid(uint8_t *aOutUid16B, uint8_t *pOutLen);
void BOARD_InitCDCDebugConsole(void);
void BOARD_DeInitCDCDebugConsole(void);
