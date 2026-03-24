/* Copyright 2022-2024 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef __TLV_DEFS_I__
#define __TLV_DEFS_I__

/* System includes */
#include <stdbool.h>
#include <stdint.h>
#include "phNxpLogApis_App.h"
#include "demo_device_config_i.h"

#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION

#define TLV_HEADER_SIZE 3 /* Tag(1) + Length(2) */

typedef enum
{
    notCreated,
    notStarted,
    Started
} UwbHandlerState;

/* Define for App developer */
/* Specification number must be filled as mention in the developer specification */
#define SPEC_VERSION_MAJOR \
    {                      \
        0x01, 0x00         \
    } // Spec major 00.01
#define SPEC_VERSION_MINOR \
    {                      \
        0x00, 0x00         \
    } // Spec minor 01.00

#define SHAREABLE_DATA_LENGTH_OFFSET 5
#define SHAREABLE_DATA_HEADER_LENGTH 5

typedef struct
{
    uint32_t version;
    uint8_t config_data_length;
    char country_code[2];
    uint32_t session_id;
    uint8_t preamble_id;
    uint8_t channel_number;
    uint16_t num_slots_per_rround;
    uint16_t slot_duration;
    uint16_t ranging_duration;
    uint8_t ranging_round_control;
    uint8_t sts_init_iv[6];
    uint16_t dest_address;
} shareable_data_t;

typedef enum
{
    kDev_unknown_c,
    kDev_iPhone_c,
    kDev_android_c,
} platform_t;

typedef enum
{
    kMsg_Initialize_iOS     = 0x0A,
    kMsg_Initialize_Android = 0xA5,
    kMsg_ConfigureAndStart  = 0x0B,
    kMsg_Stop               = 0x0C
} MessageId_t;

typedef enum
{
    kRsp_AccessoryConfigurationData = 0x01,
    kRsp_AccessoryUwbDidStart       = 0x02,
    kRsp_AccessoryUwbDidStop        = 0x03,
} ResponseId_t;

typedef enum
{
    UWB_HIF_BLE,
    UWB_HIF_UART,
} UWB_Hif_t;

typedef struct _uwb_ble_peer_state
{
    struct bt_conn *conn;
} uwb_ble_peer_state_t;

typedef struct _uwb_ble_state
{
    uwb_ble_peer_state_t peerCentral[CONFIG_BT_MAX_CONN];
    uint8_t peerCentralConnCount;
} uwb_ble_state_t;

bool tlvMngInit(void);
bool tlvBuilderInit(void);
bool tlvSendRaw(uint8_t deviceId, uint8_t *buf, uint16_t size);
void tlvSendDoneCb(void);
void tlvRecv(uint8_t deviceId, UWB_Hif_t interface, uint8_t *tlv, uint8_t tlvSize);
bool handleShutDown(void);
bool handleDeviceInit(void);
bool handleStopSession(uint8_t deviceId);
void handleDisconnection(uint8_t deviceId);

#endif /* UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION */

#endif /* __TLV_DEFS_I__ */
