/* Copyright 2020,2021, 2023 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef APP_R_CFG_H
#define APP_R_CFG_H

#ifndef RANGING_APP_SESSION_ID
#define RANGING_APP_SESSION_ID 0x11223344
#endif // RANGING_APP_SESSION_ID

#ifndef RANGING_APP_MULTI_NODE_MODE_MANY
#define RANGING_APP_MULTI_NODE_MODE_MANY 1
#endif // RANGING_APP_MULTI_NODE_MODE_MANY

#ifndef RANGING_APP_NO_OF_ANCHORS_P2P
#define RANGING_APP_NO_OF_ANCHORS_P2P 1
#endif // RANGING_APP_NO_OF_ANCHORS_P2P

/* clang-format off */

/* Short MAC addresses for Controller */
#define RANGING_APP_MAC_ADDR_CTRL                       \
    0x11, 0x22                                          \

/* Short MAC addresses for Controlee */
#define RANGING_APP_MAC_ADDR_CLEE                       \
    0x22, 0x11                                          \

/* Extended MAC addresses for Controller */
#define RANGING_APP_EXT_MAC_ADDR_CTRL                   \
    0x11, 0x22, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22,     \

/* Extended MAC addresses for Controlee */
#define RANGING_APP_EXT_MAC_ADDR_CLEE                   \
    0x22, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22, 0x11,     \

/* Broadcast MAC addresses */
#define RANGING_APP_BROADCAST_MAC_ADDR                   \
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

/* clang-format on */

#endif /* APP_R_CFG_H */
