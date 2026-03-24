/*
 * Copyright 2012-2020,2022-2024 NXP.
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

#ifndef PHUWBTYPES_H
#define PHUWBTYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#ifdef __SES_ARM
/* does not have memory.h */
#else
#include <memory.h>
#endif

//#include <phOsalUwb.h>

#if UWBIOT_OS_NATIVE
typedef long BaseType_t;
#endif

typedef unsigned char BOOLEAN;

#define EXTERNC extern

/** API Parameters */
#ifdef __GNUC__
#define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define ATTRIBUTE_UNUSED
#endif

#define ENABLED  1
#define DISABLED 0

#ifndef TRUE
#define TRUE (true) /* Logical True Value */
#endif

#ifndef FALSE
#define FALSE (false) /* Logical False Value */
#endif
typedef uint8_t utf8_t;     /* UTF8 Character String */
typedef uint8_t bool_t;     /* boolean data type */
typedef uint16_t UWBSTATUS; /* Return values */
#define UWB_STATIC static

/**
* Platform Specific Buffer Length:
*    - Macro: `#define UCI_CMD_INDEX (1)`
*    - Description: Represents the length of the platform-specific buffer. This byte is used to handle commands
                    and manage platform-related information within the communication buffer.
* Bidirectional Specific Buffer Length:
*   - Macro: `#define DIRECTION_BYTE_OFFSET (UCI_CMD_INDEX - 1)`
*   - Description: Refers to the bidirectional buffer length. This byte is primarily used for bidirectional read/write
*                  operations, particularly at the first index of the buffer.
* UCI Payload:
*   - Macro: `#define ACTUAL_PACKET_START (UCI_CMD_INDEX + N)`
*   - Description: Refers to the section of the buffer where the actual UCI (UWB Command Interface) data will be stored,
*                  starting after the platform-specific and bidirectional bytes.
*
* -Platform-Specific Buffer Usage:
*   1. Command Storage:
*   - For both SR1xx and SR2xx platforms, the platform-specific byte(s) are used to copy the UCI command, but the starting
*       index differs:
*           - SR1xx: The UCI command starts directly from the platform-specific index (index 0).
*               -Example : For SR1xx, the UCI command is directly sent from the platform-specific byte at index 0, so the same packet would look like
*                           - 21000005 44332211 00
*           - SR2xx: The zeroth index is filled by the driver with a bidirectional byte, and the UCI command starts from the first index (index 1)
*               -Example : The "zeroth index" (index 0) will be filled by the driver with the bidirectional byte, and the UCI command starts from the first index
*                           -[00]21000005 44332211 00
*
*  2. UCI Type and Extended Payload Check:
*     - During read operations, this byte is also used for checking the UCI type and for any extended payload and Data transfer checking.
*   3. Middleware (MW) and Plug-and-Play (PnP) Usage:
*     - This byte is utilized for both MW and PnP systems for managing platform-specific interactions during read and write operations.
*- Bidirectional Byte Usage:
*  1. Bidirectional Data Transmission:
*    - The bidirectional byte (located at the Zeorth index) is used for handling bidirectional read/write operations.
        It enables communication in both directions (e.g., sending commands and receiving responses).
*   - UCI Payload:
*  - The UCI payload represents the actual UCI data, including its header and payload. After processing the platform byte
 *   and updating the buffer index, the data pointer will point to the UCI payload.
 *      -  Example:
 *          - After receiving and reading the complete UCI packet:
 *              -   "FF60010001 FF01"
 *          - By applying the following macro: SHIFT_AND_OVERRIDE_HEADER The packet is transformed to:
 *              -   "FF606001000101"
 *          - The UCI payload starts from this point. This macro manipulates the buffer so that the actual UCI data is
 *            positioned after adjusting for platform-specific and bidirectional bytes.
 *
 */
#if UWBIOT_UWBD_SR2XXT && UWBIOT_TML_SPI
#define UCI_CMD_INDEX         (1)
#define DIRECTION_BYTE_OFFSET 0
#define ACTUAL_PACKET_START   (UCI_CMD_INDEX + 1)
#else
#define UCI_CMD_INDEX    (0)
#define DIRECTION_BYTE_OFFSET 0
#define ACTUAL_PACKET_START   (0)
#endif // UWBIOT_UWBD_SR2XXT && UWBIOT_TML_SPI
/* Message type Mask */
#define HDR_MT_MASK 0xE0
/* Message Type for DPF */
#define HDR_MT_DPF 0x00
/* Data Packet Format Mask */
#define HDR_DPF_MASK 0x0F
/* Data Packet Format for send/receive data with message type 0 */
#define UCI_DPF_SND    0x01 /* DATA_MESSAGE_SND: Host sends Application Data to UWBS using Bypass LL Mode */
#define UCI_DPF_RCV    0x02 /* DATA_MESSAGE_RCV: Host receives Application Data from UWBS using Bypass LL Mode */
#define UCI_DPF_LL_SND 0x03 /* LL_DATA_MESSAGE_SND: Host sends Application Data to UWBS using Logical Link Mode*/
#define UCI_DPF_LL_RCV 0x04 /* LL_DATA_MESSAGE_RCV: Host receives Application Data from UWBS using Logical Link Mode*/
#define UCI_LENGTH_OFFSET 0x03

/** Check if the Message type is Data RCV Packet */
#define IS_DATA_RCV_PACKET(HDR0)            \
    (((HDR0)&HDR_MT_MASK) == HDR_MT_DPF) && \
        ((((HDR0)&HDR_DPF_MASK) == UCI_DPF_RCV) || (((HDR0)&HDR_DPF_MASK) == UCI_DPF_LL_RCV))

/** Check if the Message type is Send Data Packet */
#define IS_DATA_SEND_PACKET(HDR0)           \
    (((HDR0)&HDR_MT_MASK) == HDR_MT_DPF) && \
        ((((HDR0)&HDR_DPF_MASK) == UCI_DPF_SND) || (((HDR0)&HDR_DPF_MASK) == UCI_DPF_LL_SND))

#define PHUWB_UNUSED(X) (void)(X);
/**
 * Possible Hardware Configuration exposed to upper layer.
 * Typically this should be port name (Ex:"COM1","COM2") to which SR100 is
 * connected.
 */
typedef enum
{
    ENUM_LINK_TYPE_COM1,
    ENUM_LINK_TYPE_COM2,
    ENUM_LINK_TYPE_COM3,
    ENUM_LINK_TYPE_COM4,
    ENUM_LINK_TYPE_COM5,
    ENUM_LINK_TYPE_COM6,
    ENUM_LINK_TYPE_COM7,
    ENUM_LINK_TYPE_COM8,
    ENUM_LINK_TYPE_SPI,
    ENUM_LINK_TYPE_USB,
    ENUM_LINK_TYPE_TCP,
    ENUM_LINK_TYPE_NB
} phLibUwb_eConfigLinkType;

/**
 * Deferred message. This message type will be posted to the client application
 * thread
 * to notify that a deferred call must be invoked.
 */
#define PH_LIBUWB_DEFERREDCALL_MSG (0x311)

/**
 * Deferred call declaration.
 * This type of API is called from ClientApplication ( main thread) to notify
 * specific callback.
 */
typedef void (*pphLibUwb_DeferredCallback_t)(void *);

/**
 * Deferred parameter declaration.
 * This type of data is passed as parameter from ClientApplication (main thread)
 * to the
 * callback.
 */
typedef void *pphLibUwb_DeferredParameter_t;

/**
 * Possible Hardware Configuration exposed to upper layer.
 * Typically this should be at least the communication link (Ex:"COM1","COM2")
 * the controller is connected to.
 */
typedef struct phLibUwb_sConfig
{
    uint8_t *pLogFile; /* Log File Name*/
    /* Hardware communication link to the controller */
    phLibUwb_eConfigLinkType nLinkType;
    /* The client ID (thread ID or message queue ID) */
    uintptr_t nClientId;
} phLibUwb_sConfig_t, *pphLibUwb_sConfig_t;

/**
 * UWB Message structure contains message specific details like
 * message type, message specific data block details, etc.
 */
typedef struct phLibUwb_Message
{
#if UWBIOT_OS_ZEPHYR
    uint32_t reservedForKernel; /*first word of the item is reserved for the kernels use*/
#endif
    uint16_t eMsgType; /* Type of the message to be posted*/
    void *pMsgData;    /* Pointer to message specific data block in case any*/
    uint16_t Size;     /* Size of the data block*/
} phLibUwb_Message_t, *pphLibUwb_Message_t;

/**
 * @brief  UWBD  Firmware Modes.
 */
typedef enum sdkMode
{
    /** Factory Firmware */
    FACTORY_FW,
    /** Mainline Firmware */
    MAINLINE_FW
} eFirmwareMode;
/**
 * @brief  Structure lists out the Firmware Image Context
 */
typedef struct phUwbFWImageContext
{
    /** pointer to the FW image to be used*/
    const uint8_t *fwImage;
    /** size of fw image */
    uint32_t fwImgSize;
    /** fw type */
    eFirmwareMode fwMode;
#if UWBIOT_UWBD_SR2XXT
    /** force firmware type */
    bool forceFwUpdate;
    /* FW Image  Patch version */
    uint8_t patchVerion;
#endif // UWBIOT_UWBD_SR2XXT
} phUwbFWImageContext_t;

/**
 * Deferred message specific info declaration.
 * This type of information is packed as message data when
 * PH_LIBUWB_DEFERREDCALL_MSG
 * type message is posted to message handler thread.
 */
typedef struct phLibUwb_DeferredCall
{
    pphLibUwb_DeferredCallback_t pCallback;   /* pointer to Deferred callback */
    pphLibUwb_DeferredParameter_t pParameter; /* pointer to Deferred parameter */
} phLibUwb_DeferredCall_t;

/**
 * Definitions for supported protocol
 */

#ifdef __GNUC__
#define UWB_API_DEPCREATED __attribute__((deprecated))
#else
#define UWB_API_DEPCREATED
#endif

#endif /* PHUWBTYPES_H */
