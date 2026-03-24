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

#ifndef UWB_BOARD_VIRGO_H
#define UWB_BOARD_VIRGO_H

#include <stdio.h>
#include <string.h>
#include <uwb_board_values.h>
#include "phUwb_BuildConfig.h"

//#define phPlatform_Is_Irq_Context() (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)
static inline BOOLEAN phPlatform_Is_Irq_Context(void)
{
    return 0;
}

#define BOARD_SetPinsForRunMode()
#define BOARD_InitBootClocks()
#define BOARD_InitBootPins()
#define BOARD_InitDebugConsole()

#define UWB_BOARD_ENABLE_EXT_FLASH_BASED_FW_DOWNLOAD 1

#define UWB_BOARD_ENABLE_FW_DOWNLOAD_ON_UWBINIT 1

#define UWB_BOARD_RX_ANTENNA_CONFIG_MODE_VAL UWB_BOARD_RX_ANTENNA_CONFIG_MODE_2DAOA

//#define __BKPT(X)

#define PRINTF(...) printf(__VA_ARGS__)

#define PUTCHAR(X) putchar(X)

/* No LEDs here */
#define UWB_BOARD_DEFINE_LED_APIS(THE_LED)                      \
    static inline void UWB_BOARD_GPIO_SET_##THE_LED##_ON(void)  \
    {                                                           \
    }                                                           \
    static inline void UWB_BOARD_GPIO_SET_##THE_LED##_OFF(void) \
    {                                                           \
    }

UWB_BOARD_DEFINE_LED_APIS(LED_R)
UWB_BOARD_DEFINE_LED_APIS(LED_O)
UWB_BOARD_DEFINE_LED_APIS(LED_B)
UWB_BOARD_DEFINE_LED_APIS(LED_G)

/**
 * @brief Board initialization
 *
 */
void board_init(void);

#endif // UWB_BOARD_VIRGO_H
