/*
 * Copyright (c) 2020 SixOctets Systems
 * Copyright (c) 2019 Aaron Tsui <aaron.tsui@outlook.com>
 * Copyright 2021-2022, 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#ifndef __ZEPHYR__
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>
#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#endif // #ifndef __ZEPHYR__
#include <uwb_nearby_service.h>
#include <phNxpLogApis_App.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bt_gatt_uwb_config_t s_Uwbcallback;

static bt_gatt_uwb_state_t s_UwbState;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static ssize_t bt_gatt_uwb_write_stream(struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags);

static void bt_gatt_uwb_read_stream_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);

static void bt_gatt_uwb_write_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params);

static uint8_t bt_gatt_uwb_discovery_service(
    struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* UWB Service Declaration */
BT_GATT_SERVICE_DEFINE(uwb_service,
    BT_GATT_PRIMARY_SERVICE(UWB_SERVICE),
#if CONFIG_BT_PAIRING
    BT_GATT_CHARACTERISTIC(UWB_WRITE_STREAM,
        BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_WRITE | BT_GATT_PERM_WRITE_ENCRYPT,
        NULL,
        bt_gatt_uwb_write_stream,
        NULL),
#else
    BT_GATT_CHARACTERISTIC(
        UWB_WRITE_STREAM, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL, bt_gatt_uwb_write_stream, NULL),
#endif //
    BT_GATT_CHARACTERISTIC(UWB_READ_STREAM, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CCC(bt_gatt_uwb_read_stream_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), );

/* UWB nearby Service Declaration */
BT_GATT_SERVICE_DEFINE(nearby_service,
    BT_GATT_PRIMARY_SERVICE(UWB_NEARBY_SERVICE),
    BT_GATT_CHARACTERISTIC(UWB_NEARBY_ACCESSORY_DATA,
        BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ | BT_GATT_PERM_READ_ENCRYPT,
        NULL,
        NULL,
        NULL), );

/*******************************************************************************
 * Code
 ******************************************************************************/
static void bt_gatt_uwb_start_service_discovery(struct bt_conn *conn)
{
    int err;
    uint8_t i;
    for (i = 0; i < CONFIG_BT_MAX_CONN; i++) {
        if (conn == s_UwbState.peer[i].conn) {
            break;
        }
    }
    if (i >= CONFIG_BT_MAX_CONN) {
        return;
    }
    memcpy(&s_UwbState.peer[i].serviceDiscoveryUuid, UWB_SERVICE, sizeof(s_UwbState.peer[i].serviceDiscoveryUuid));
    s_UwbState.peer[i].discoverParams.uuid         = &s_UwbState.peer[i].serviceDiscoveryUuid.uuid;
    s_UwbState.peer[i].discoverParams.func         = bt_gatt_uwb_discovery_service;
    s_UwbState.peer[i].discoverParams.start_handle = 0x0001;
    s_UwbState.peer[i].discoverParams.end_handle   = 0xffff;
    s_UwbState.peer[i].discoverParams.type         = BT_GATT_DISCOVER_PRIMARY;

    err = bt_gatt_discover(conn, &s_UwbState.peer[i].discoverParams);
    if (err) {
        LOG_E("Discover failed(err %d)", err);
        return;
    }
}

static void bt_gatt_uwb_get_mtu_callback(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params)
{
    if (err) {
        LOG_E("GATT MTU exchange response failed (err %u)", err);
    }
    else {
        LOG_D("GATT MTU exchanged: %d", bt_gatt_get_mtu(conn));
    }

    bt_gatt_uwb_start_service_discovery(conn);
}

void bt_gatt_uwb_connected(struct bt_conn *conn)
{
    int err;
    uint32_t i;
    int index = -1;

    for (i = 0; i < CONFIG_BT_MAX_CONN; i++) {
        if (conn == s_UwbState.peer[i].conn) {
            LOG_I("the connection 0x08X is attached", conn);
            return;
        }
        else if (NULL == s_UwbState.peer[i].conn) {
            if (-1 == index) {
                index = (int)i;
            }
        }
        else {
        }
    }

    if (-1 == index) {
        LOG_W("All connection slots are used");
        return;
    }
    i = (uint32_t)index;

    s_UwbState.peer[i].conn = conn;
    s_UwbState.connectCount++;

    s_UwbState.peer[i].exchangeParams.func = bt_gatt_uwb_get_mtu_callback;

    err = bt_gatt_exchange_mtu(conn, &s_UwbState.peer[i].exchangeParams);
    if (err) {
        bt_gatt_uwb_start_service_discovery(conn);
        LOG_E("Exchange GATT mtu failed (err %d)", err);
    }
    else {
    }
}

void bt_gatt_uwb_disconnected(struct bt_conn *conn)
{
    uint32_t i;

    for (i = 0; i < CONFIG_BT_MAX_CONN; i++) {
        if (conn == s_UwbState.peer[i].conn) {
            break;
        }
        else {
        }
    }
    if (i >= CONFIG_BT_MAX_CONN) {
        LOG_E("The connection 0x%08X is not found.", conn);
        return;
    }

    s_UwbState.peer[i].discoverWriteHandle = 0;
    s_UwbState.peer[i].discoverPermission  = 0;
    s_UwbState.peer[i].conn                = NULL;
    s_UwbState.connectCount--;

    if (0 == s_UwbState.connectCount) {
        s_UwbState.readStreamCccCfg = 0;
    }
}

int bt_gatt_uwb_init(bt_gatt_uwb_config_t *config)
{
    if (config == NULL) {
        return -1;
    }
    s_Uwbcallback               = *config;
    s_UwbState.readStreamCccCfg = 0;
    return 0;
}

static void bt_gatt_uwb_read_stream_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    s_UwbState.readStreamCccCfg = (value & BT_GATT_CCC_NOTIFY);
}

static ssize_t bt_gatt_uwb_write_stream(struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
    /**
     * @brief This call back hit from the Client to Server
     *
     */
    s_Uwbcallback.data_received(conn, (uint8_t *)buf, len);
    return (ssize_t)len;
}

static void bt_gatt_uwb_write_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params)
{
}

static uint8_t bt_gatt_uwb_discovery_service(
    struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params)
{
    int err;
    uint32_t i;

    for (i = 0; i < CONFIG_BT_MAX_CONN; i++) {
        if (conn == s_UwbState.peer[i].conn) {
            break;
        }
        else {
        }
    }
    if (i >= CONFIG_BT_MAX_CONN) {
        memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }

    if (!attr) {
        LOG_D("Discover complete");
        memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }

    LOG_I("[ATTRIBUTE] handle %u", attr->handle);

    if (!bt_uuid_cmp(s_UwbState.peer[i].discoverParams.uuid, UWB_SERVICE)) {
        memcpy(&s_UwbState.peer[i].serviceDiscoveryUuid,
            UWB_WRITE_STREAM,
            sizeof(s_UwbState.peer[i].serviceDiscoveryUuid));
        s_UwbState.peer[i].discoverParams.uuid         = &s_UwbState.peer[i].serviceDiscoveryUuid.uuid;
        s_UwbState.peer[i].discoverParams.start_handle = attr->handle + 1;
        s_UwbState.peer[i].discoverParams.type         = BT_GATT_DISCOVER_CHARACTERISTIC;

        err = bt_gatt_discover(conn, &s_UwbState.peer[i].discoverParams);
        if (err) {
            LOG_E("Discover stream write service failed (err %d)", err);
        }
    }
    else if (!bt_uuid_cmp(s_UwbState.peer[i].discoverParams.uuid, UWB_WRITE_STREAM)) {
        struct bt_gatt_chrc *gattChrc = (struct bt_gatt_chrc *)attr->user_data;

        s_UwbState.peer[i].discoverPermission  = gattChrc->properties;
        s_UwbState.peer[i].discoverWriteHandle = attr->handle + 1;
    }
    else {
    }

    return BT_GATT_ITER_STOP;
}

int bt_gatt_uwb_notify(uint8_t deviceId, const uint8_t *buf, size_t len)
{
    int ret = -EINVAL;
    if (s_UwbState.peer[deviceId].conn != NULL) {
        if (s_UwbState.readStreamCccCfg) {
            ret = bt_gatt_notify(s_UwbState.peer[deviceId].conn, &uwb_service.attrs[3], buf, (uint16_t)len);
            if (ret) {
                LOG_E("bt_gatt_notify error %d", ret);
                return -EINVAL;
            }
        }
        else if (s_UwbState.peer[deviceId].discoverWriteHandle) {
            ret = 0;
            if (s_UwbState.peer[deviceId].discoverPermission & BT_GATT_CHRC_WRITE_WITHOUT_RESP) {
                ret = bt_gatt_write_without_response(
                    s_UwbState.peer[deviceId].conn, s_UwbState.peer[deviceId].discoverWriteHandle, buf, len, 0);
            }
            else if (s_UwbState.peer[deviceId].discoverPermission & BT_GATT_CHRC_WRITE) {
                s_UwbState.peer[deviceId].writeParams.data   = (const void *)buf;
                s_UwbState.peer[deviceId].writeParams.length = (uint8_t)len;
                s_UwbState.peer[deviceId].writeParams.handle = s_UwbState.peer[deviceId].discoverWriteHandle;
                s_UwbState.peer[deviceId].writeParams.offset = 0;
                s_UwbState.peer[deviceId].writeParams.func   = bt_gatt_uwb_write_func;
                ret = bt_gatt_write(s_UwbState.peer[deviceId].conn, &s_UwbState.peer[deviceId].writeParams);
            }
            else {
            }
            if (ret) {
                LOG_E("bt_gatt_write error %d", ret);
                return -EINVAL;
            }
        }
        else {
        }
    }
    else {
        LOG_E("bt_gatt_uwb_notify error %d", ret);
        return -EINVAL;
    }
    return ret;
}

int Update_Nearby(uint8_t deviceId, uint16_t size, uint8_t *testData)
{
    uint16_t handle;
    int ret;

    handle = bt_gatt_attr_get_handle(&nearby_service.attrs[2]);
    if (!handle) {
        LOG_E("bt_gatt_attr_get_handle ");
        return -EINVAL;
    }

    s_UwbState.peer[deviceId].writeParams.data   = (const void *)testData;
    s_UwbState.peer[deviceId].writeParams.length = (uint8_t)size;
    s_UwbState.peer[deviceId].writeParams.handle = handle;
    s_UwbState.peer[deviceId].writeParams.offset = 0;
    s_UwbState.peer[deviceId].writeParams.func   = bt_gatt_uwb_write_func;
    ret = bt_gatt_write(s_UwbState.peer[deviceId].conn, &s_UwbState.peer[deviceId].writeParams);
    if (ret) {
        LOG_E("bt_gatt_write failed ret%d", ret);
        return -EINVAL;
    }

    return 0;
}

int Erase_Nearby(uint8_t deviceId)
{
    uint16_t handle;
    int ret;
    uint8_t blank_data[48] = {0x00};

    handle = bt_gatt_attr_get_handle(&nearby_service.attrs[2]);
    if (!handle) {
        LOG_E("bt_gatt_attr_get_handle ");
        return -EINVAL;
    }

    s_UwbState.peer[deviceId].writeParams.data   = (const void *)blank_data;
    s_UwbState.peer[deviceId].writeParams.length = sizeof(blank_data);
    s_UwbState.peer[deviceId].writeParams.handle = handle;
    s_UwbState.peer[deviceId].writeParams.offset = 0;
    s_UwbState.peer[deviceId].writeParams.func   = bt_gatt_uwb_write_func;
    ret = bt_gatt_write(s_UwbState.peer[deviceId].conn, &s_UwbState.peer[deviceId].writeParams);
    if (ret) {
        LOG_E("bt_gatt_write failed ret%d", ret);
        return -EINVAL;
    }
    return 0;
}
#endif // UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
