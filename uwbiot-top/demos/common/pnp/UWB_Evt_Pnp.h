/*
 * Copyright 2019,2020,2024 NXP
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
#ifndef UWB_EVT_H
#define UWB_EVT_H

#include "phUwb_BuildConfig.h"

typedef enum
{
    USB_TLV_EVT,
} UWB_EvtType_t;

typedef struct
{
    uint16_t type;
    uint8_t *value;
    uint16_t size;

} tlv_t;

/* TLV types */
#define UCI_CMD        0x01
#define HBCI_CMD       0x02
#define HBCI_LAST_CMD  0x03
#define RESET          0x04
#define HBCI_QUERY_CMD 0x05

/** Deprecated. Use GET_VERISON_INFO instead */
#define GET_SOFTWARE_VERSION 0x06
#define GET_BOARD_ID         0x07
#define USB_LOOPBACK         0x08
#define MCU_RESET            0x09 // MCU nvReset will be called.
#define RCI_CMD              0x0A
#define SWITCH_PROTOCOL_CMD  0x0B
/** Deprecated. Use GET_VERISON_INFO instead */
#define GET_BOARD_VERSION 0x0C
#define GET_VERISON_INFO  0x0D

#define HDLL_CMD            0x0E
#define HDLL_EDL_LAST_WRITE 0x0F
#define HDLL_GET_NTF        0x10
#define HELIOS2_RESET       0x11

#define HDLL_RESET_TO_UCI 0x12

#define TLV_RESP_SIZE                      0xFF
#define RESET_SOFTWARE_VERSION_SIZE        0x04
#define RESET_RESPONSE_SIZE                4
#define GET_SOFTWARE_VERSION_RESPONSE_SIZE 4
#define GET_VERISON_INFO_RESPONSE_SIZE                       \
    (00 + 1 /*< [0]  Tag   : GET_VERISON_INFO */             \
        + 1 /*< [1] SW VER : SW Major */                     \
        + 1 /*< [2] SW VER : SW Minor */                     \
        + 1 /*< [3] SW VER : SW Dev   */                     \
        + 1 /*< [4] SW VER : RFU */                          \
        + 1 /*< [5]   IC   : @ref UBWPnPUWBSIdentifier_t */  \
        + 1 /*< [6] Board  : @ref UBWPnPBoardIdentifier_t */ \
    )

#define UCI_MT_MASK     0xE0
#define UCI_MT_SHIFT    0x05
#define MT_ESE_CTRL_CMD 4
#define MT_ESE_CTRL_RSP 5
#define UCI_HEADER_SIZE 4

typedef enum UBWPnPUWBSIdentifier
{
    kUBWPnPUWBSIdentifier_SR100T = 00 + 1,
    kUBWPnPUWBSIdentifier_SR110T = 00 + 2,
    kUBWPnPUWBSIdentifier_SR150  = 00 + 3,
    kUBWPnPUWBSIdentifier_SR160  = 00 + 4,
    kUBWPnPUWBSIdentifier_SR100S = 00 + 5,
    kUBWPnPUWBSIdentifier_SR040  = 10 + 1,
    kUBWPnPUWBSIdentifier_SR048M = 10 + 2,
    kUBWPnPUWBSIdentifier_SR200T = 20 + 1,
    kUBWPnPUWBSIdentifier_SR250  = 20 + 3,
    kUBWPnPUWBSIdentifier_SR200S = 20 + 5,
} UBWPnPUWBSIdentifier_t;

typedef enum UBWPnPBoardIdentifier
{
    /* Rhodes V4 */
    kUBWPnPBoardIdentifier_SR1XX_RV4 /*             */ = 00 + 2,
    /* QN9090 + MK Shield */
    kUBWPnPBoardIdentifier_SR1XX_QN9090_ShieldV4 /* */ = 00 + 3,
    /* LPC55S + MK Shield */
    kUBWPnPBoardIdentifier_SR1XX_LPC55S_ShieldV4 /* */ = 00 + 4,
    /* SR040 Finder V3 */
    kUBWPnPBoardIdentifier_SR040_FV3 /*             */ = 10 + 1,
    /* SR048 Finder V3 */
    kUBWPnPBoardIdentifier_SR048M_FV3 /*            */ = 10 + 2,
    /* SR2XX with Crete */
    kUBWPnPBoardIdentifier_SR2XX_Crete /*           */ = 20 + 2,
    /* SR2XX with Virgo */
    kUBWPnPBoardIdentifier_SR2XX_Virgo /*           */ = 20 + 3,
    /* SR2XX  with FRDM_RW612 */
    kUBWPnPBoardIdentifier_SR2XX_FRDM_RW612 /*      */ = 20 + 4,
} UBWPnPBoardIdentifier_t;

#define MCTT_UCI_CMD       0
#define MCTT_ESE_CMD       1
#define UCI_HDR_LEN        4
#define UCI_EXT_LEN_IND    1
#define UCI_LEN_OFFSET     3
#define UCI_EXT_LEN_OFFSET 2
#define UCI_EXT_LEN_MASK   0x80

void UWB_HandleEvt(UWB_EvtType_t ev, void *args);

size_t UWBPnP_GetVersionInfo(uint8_t *tlvBuf, size_t tlvBufLen, UBWPnPBoardIdentifier_t board);

#endif // UWB_EVT_H
