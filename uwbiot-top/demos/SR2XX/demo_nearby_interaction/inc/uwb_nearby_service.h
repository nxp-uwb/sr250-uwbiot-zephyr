/*
 * Copyright (c) 2019 Aaron Tsui <aaron.tsui@outlook.com>
 * Copyright 2021, 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION

#ifdef __ZEPHYR__
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/hrs.h>
#include <zephyr/bluetooth/services/ias.h>
#else
#include <bluetooth/gatt.h>
#endif
/**
 * @brief UWB Nearby Interaction Service
 * @defgroup UWB Nearby Interaction Service
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif

// /*******************************************************************************
// * Definitions
// ******************************************************************************/

#define UWB_SERVICE_UUID 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E
#define UWB_READ_CHAR    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E
#define UWB_WRITE_CHAR   0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E

#define UWB_NEARBY_DATA_SERVICE_UUID \
    0xba, 0x2d, 0x9c, 0x68, 0x73, 0x30, 0x33, 0x86, 0xb2, 0x4b, 0x17, 0x08, 0x40, 0x3e, 0xfe, 0x48
#define UWB_NEARBY_DATA_CHAR \
    0x28, 0x33, 0xf5, 0x75, 0x73, 0x80, 0x4e, 0x9a, 0x21, 0x47, 0xef, 0xd8, 0xd5, 0xd9, 0xe8, 0x95

#define UWB_SERVICE      BT_UUID_DECLARE_128(UWB_SERVICE_UUID)
#define UWB_READ_STREAM  BT_UUID_DECLARE_128(UWB_READ_CHAR)
#define UWB_WRITE_STREAM BT_UUID_DECLARE_128(UWB_WRITE_CHAR)

#define UWB_NEARBY_SERVICE        BT_UUID_DECLARE_128(UWB_NEARBY_DATA_SERVICE_UUID)
#define UWB_NEARBY_ACCESSORY_DATA BT_UUID_DECLARE_128(UWB_NEARBY_DATA_CHAR)

#if __ZEPHYR__
#if defined(CONFIG_BT_FIXED_PASSKEY)
#define FIXED_PASSKEY_VALUE 999999
#define CONFIG_BT_PAIRING   1
#endif // CONFIG_BT_FIXED_PASSKEY
#endif

// /*******************************************************************************
// * Prototypes
// ******************************************************************************/
typedef int (*bt_gatt_uwb_data_received_t)(struct bt_conn *conn, uint8_t *buffer, ssize_t length);

/*******************************************************************************
* Definitions
******************************************************************************/
typedef struct _bt_gatt_uwb_config
{
    bt_gatt_uwb_data_received_t data_received; // client to server
} bt_gatt_uwb_config_t;

typedef struct _bt_gatt_uwb_peer_state
{
    struct bt_conn *conn;
    struct bt_uuid_128 serviceDiscoveryUuid;
    struct bt_gatt_exchange_params exchangeParams;
    struct bt_gatt_discover_params discoverParams;
    struct bt_gatt_write_params writeParams;
    volatile uint8_t discoverWriteHandle;
    volatile uint8_t discoverPermission;
} bt_gatt_uwb_peer_state_t;

typedef struct _bt_gatt_uwb_state
{
    bt_gatt_uwb_config_t config;
    bt_gatt_uwb_peer_state_t peer[CONFIG_BT_MAX_CONN];
    volatile uint8_t readStreamCccCfg;
    volatile uint8_t connectCount;
} bt_gatt_uwb_state_t;

/*******************************************************************************
* Prototypes
******************************************************************************/
void bt_gatt_uwb_connected(struct bt_conn *conn);
void bt_gatt_uwb_disconnected(struct bt_conn *conn);
int bt_gatt_uwb_init(bt_gatt_uwb_config_t *config);
int bt_gatt_uwb_notify(uint8_t deviceId, const uint8_t *buf, size_t len);
int Update_Nearby(uint8_t deviceId, uint16_t size, uint8_t *testData);
int Erase_Nearby(uint8_t deviceId);
#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION */
