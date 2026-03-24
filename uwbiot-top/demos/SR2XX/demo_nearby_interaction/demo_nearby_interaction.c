/* Copyright 2021-2024,2026 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"

#include "UwbApi.h"
#include <AppInternal.h>
#include "phOsalUwb.h"

#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION

#ifndef __ZEPHYR__
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
#include "PWR_Interface.h"
#include "board_lp.h"
#endif // (APP_LOWPOWER_ENABLED)
#include <porting.h>
#endif
#include "TLV_Types_i.h"
#include "uwb_nearby_service.h"

#ifndef __ZEPHYR__
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
extern status_t CRYPTO_InitHardware(void);
#endif
#endif

/********************************************************************************/
#define DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE 1024
#define DEMO_NEARBY_INTERACTION_SR250_TASK_NAME "DemoNearbyInteraction"
#define DEMO_NEARBY_INTERACTION_SR250_TASK_PRIO 4

#if DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE > 1024
#pragma message("DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE > 1024 : BLE demos will not work")
#endif // DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE

#define BLE_CONN_CB_MAX_REENTRANCE_TIMEOUT (0x2000)

/* UWB operation status */
struct bt_conn *default_conn;
uwb_ble_state_t g_UwbBleState;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static int uwb_service_data_received(struct bt_conn *conn, uint8_t *buffer, ssize_t length);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const struct bt_data ad[] = {
    // Advertise a custom 128-bit service UUID so the client app can discover the service
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x6E400001, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E)),
    // Include the complete device name in the advertising payload.
    // This ensures the BLE scanner app can display the correct device name during scanning.
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, (sizeof(CONFIG_BT_DEVICE_NAME) - 1)),
};

static const struct bt_data as[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_I("Security changed: %s level %u (error %d)", addr, level, err);
}

#if CONFIG_BT_PAIRING
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_D("Passkey for %s: %06u\r\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_E("Pairing cancelled: %s\r\n", addr);
    /* clear connection reference for sec mode 3 pairing */
    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
}
#endif // CONFIG_BT_PAIRING
#endif // CONFIG_BT_SMP

static struct bt_conn_cb conn_callbacks = {
    .connected    = connected,
    .disconnected = disconnected,
#if CONFIG_BT_SMP
    .security_changed = security_changed,
#endif
};

#if (CONFIG_BT_SMP && CONFIG_BT_PAIRING)
static struct bt_conn_auth_cb auth_cb_display = {
    .passkey_display = auth_passkey_display,
    .cancel          = auth_cancel,
    .passkey_entry   = NULL,
};
#endif // (CONFIG_BT_SMP && CONFIG_BT_PAIRING)

/*******************************************************************************
 * Code
 ******************************************************************************/
static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    struct bt_conn_info info;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
        LOG_E("Failed to connect %s (err %u)\n", addr, err);
    }
    else {
        uint32_t i;
        int index = -1;

        default_conn = bt_conn_ref(conn);

        err = bt_conn_get_info(conn, &info);
        if (err) {
            LOG_E("Failed to get info");
            return;
        }

        if (info.role == BT_HCI_ROLE_PERIPHERAL) {
            for (i = 0; i < CONFIG_BT_MAX_CONN; i++) {
                if (conn == g_UwbBleState.peerCentral[i].conn) {
                    LOG_E("the connection 0x08X is attached\r\n", conn);
                    return;
                }
                else if (NULL == g_UwbBleState.peerCentral[i].conn) {
                    if (-1 == index) {
                        index                                 = (int)i;
                        g_UwbBleState.peerCentral[index].conn = conn;
                        g_UwbBleState.peerCentralConnCount++;
                    }
                }
                else {
                }
            }
        }
        else {
            LOG_E("Invalid hci role..Exiting!!");
            return;
        }

        if (-1 == index) {
            LOG_E("All connection slots are used\r\n");
            return;
        }
        bt_gatt_uwb_connected(conn);
#if (CONFIG_BT_SMP && CONFIG_BT_PAIRING)
        if (BT_HCI_ROLE_PERIPHERAL == info.role) {
            if (bt_conn_set_security(conn, BT_SECURITY_L3)) {
                LOG_E("Failed to set security\n");
            }
        }
#endif // (CONFIG_BT_SMP && CONFIG_BT_PAIRING)

        /* Initialize the UWB */
        if (!handleDeviceInit()) {
            LOG_E("Device init failed");
        }

        LOG_I("BLE Connected to peer: %s, %d peers connected", addr, g_UwbBleState.peerCentralConnCount);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    uint32_t deviceId;
    uint8_t found = 0;
    int err       = -1;

    for (deviceId = 0; deviceId < CONFIG_BT_MAX_CONN; deviceId++) {
        if (conn == g_UwbBleState.peerCentral[deviceId].conn) {
            struct bt_conn_info info;
            bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
            err = bt_conn_get_info(conn, &info);
            if (err) {
                LOG_E("Failed to get info");
                return;
            }
            if (!handleStopSession((uint8_t)deviceId)) {
                LOG_E("Stopping session failed");
                return;
            }
            g_UwbBleState.peerCentral[deviceId].conn = NULL;
            g_UwbBleState.peerCentralConnCount--;
            found = 1;
            LOG_I("BLE Disconnected peer %s, %d peers connected\r\n", addr, g_UwbBleState.peerCentralConnCount);
            break;
        }
        else {
        }
    }
    if (0 == found) {
        LOG_E("The connection 0x%08X is not found.\r\n", conn);
        return;
    }
    /* if not more peer connected, shutdown UWB */
    if (g_UwbBleState.peerCentralConnCount == 0) {
        if (!handleShutDown()) {
            LOG_E("Stack Deinit failed");
        }
    }
    bt_gatt_uwb_disconnected(conn);

    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }

    LOG_I("BLE Disconnected (reason %u)\n", reason);
}

static int uwb_service_data_received(struct bt_conn *conn, uint8_t *buffer, ssize_t length)
{
    char addr[BT_ADDR_LE_STR_LEN];
    uint8_t i;
    int bleDeviceId = -1;

    if (NULL == conn) {
        LOG_E("bt_conn is null");
        return -1;
    }
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    for (i = 0; i < CONFIG_BT_MAX_CONN; i++) {
        if (conn == g_UwbBleState.peerCentral[i].conn) {
            bleDeviceId = (int)i;
            break;
        }
    }
    if (-1 == bleDeviceId) {
        LOG_E("Unknown ble device");
        return -1;
    }

    tlvRecv((uint8_t)bleDeviceId, UWB_HIF_BLE, buffer, length);
    return 0;
}

static void bt_ready(int err)
{
    if (err) {
        LOG_E("Bluetooth init failed (err %d)", err);
        return;
    }
    #if ((defined(CONFIG_BT_SETTINGS)) && (CONFIG_BT_SETTINGS))
    if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
        settings_load();
    }
    #endif // ((defined(CONFIG_BT_SETTINGS)) && (CONFIG_BT_SETTINGS))

    LOG_I("Bluetooth initialized");

    bt_conn_cb_register(&conn_callbacks);
#if (CONFIG_BT_SMP && CONFIG_BT_PAIRING)
    bt_conn_auth_cb_register(&auth_cb_display);
#if CONFIG_BT_FIXED_PASSKEY
    err = bt_passkey_set(FIXED_PASSKEY_VALUE);
    if (err) {
        LOG_E("Setting fixed passkey failed (err %d)", err);
        return;
    }
#endif // CONFIG_BT_FIXED_PASSKEY
#endif // (CONFIG_BT_SMP && CONFIG_BT_PAIRING)

    bt_gatt_uwb_config_t uwbConfig;
    uwbConfig.data_received = uwb_service_data_received;
    err                     = bt_gatt_uwb_init(&uwbConfig);
    if (err) {
        LOG_E("bt_gatt_uwb_init failed (err %d)", err);
        return;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, as, ARRAY_SIZE(as), ad, ARRAY_SIZE(ad));
    if (err) {
        LOG_E("Advertising failed to start (err %d)", err);
        return;
    }
    else{
        LOG_I("Bluetooth advertising started successfully");
    }
    /*
     * Set the Bluetooth device name explicitly using bt_set_name(). and it depends on CONFIG_BT_DEVICE_NAME_DYNAMIC
     * Note: BT_LE_ADV_CONN_NAME macro is deprecated in recent Zephyr versions.
     * So, we manually set the name and configure advertising data accordingly.
     * Note: CONFIG_BT_DEVICE_NAME is defined in the prj.conf
     */
    err = bt_set_name(CONFIG_BT_DEVICE_NAME);
    if (err) {
        LOG_E("bt_set_name() failed. (err %d)\n", err);
        return;
    }

    err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), as, ARRAY_SIZE(as));
    if (err) {
        LOG_E("Abt_le_adv_update_data (err %d)", err);
        return;
    }
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
    /* Release the WFI constraint, and allow the device to go to DeepSleep to allow for better power saving */
    PWR_ReleaseLowPowerModeConstraint(PWR_WFI);
    PWR_SetLowPowerModeConstraint(PWR_DeepSleep);
#endif
}

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
static void APP_ServiceInitLowpower(void)
{
    PWR_ReturnStatus_t status = PWR_Success;

    /* It is required to initialize PWR module so the application
     * can call PWR API during its init (wake up sources...) */
    PWR_Init();

    /* Initialize board_lp module, likely to register the enter/exit
     * low power callback to Power Manager */
    BOARD_LowPowerInit();

    /* Set WFI constraint by default (works for All application)
     * Application will be allowed to release the WFI constraint and set a deepest lowpower mode constraint such as
     * DeepSleep or PowerDown if it needs more optimization */
    status = PWR_SetLowPowerModeConstraint(PWR_WFI);
    assert(status == PWR_Success);
    (void)status;

    /* Register PWR functions into SerialManager module in order to disable device lowpower
        during SerialManager processing. Typically, allow only WFI instruction when
        uart data are processed by serail manager  */
    // SerialManager_SetLowpowerCriticalCb(&gSerMgr_LowpowerCriticalCBs);
}
#endif /* APP_LOWPOWER_ENABLED */

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    bool abortIdle = false;
    uint64_t expectedIdleTimeUs, actualIdleTimeUs;
    uint32_t irqMask = DisableGlobalIRQ();

    /* Disable and prepare systicks for low power */
    abortIdle = PWR_SysticksPreProcess((uint32_t)xExpectedIdleTime, &expectedIdleTimeUs);

    if (abortIdle == false) {
        /* Enter low power with a maximal timeout */
        actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

        /* Re enable systicks and compensate systick timebase */
        PWR_SysticksPostProcess(expectedIdleTimeUs, actualIdleTimeUs);
    }

    /* Exit from critical section */
    EnableGlobalIRQ(irqMask);
}
#endif /* APP_LOWPOWER_ENABLED */

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
void APP_InitServices(void)
{
    APP_ServiceInitLowpower();

    // #if defined(APP_USE_SENSORS) && (APP_USE_SENSORS > 0)
    //     SENSORS_InitAdc();
    // #endif /* APP_USE_SENSORS */
}
#endif

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Demo Nearby Interaction (BLE tracker)");
    phUwbappContext_t appCtx = {0};
    tUWBAPI_STATUS status    = UWBAPI_STATUS_FAILED;
#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
    appCtx.fwImageCtx.fwImage   = (uint8_t *)&heliosEncryptedMainlineFwImage[0];
    appCtx.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
    appCtx.fwImageCtx.fwMode = MAINLINE_FW;
    appCtx.pCallback         = AppCallback;
    appCtx.pTmlCallback      = NULL;

    /* Initialize the UWB */
    if (UwbApi_Initialize(&appCtx) != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_Initialize failed");
        goto exit;
    }

#ifndef __ZEPHYR__
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
    CRYPTO_InitHardware();
#endif /* CONFIG_BT_SMP */
#endif

#if DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE > 1024
    status = UWBAPI_STATUS_OK;
    LOG_W("Skipping demo as DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE > 1024");
#else

    if (!tlvBuilderInit()) {
        LOG_E("Failed to initialize TLV builder");
        goto exit;
    }

    if (!tlvMngInit()) {
        LOG_E("Failed to initialize manager");
        goto exit;
    }
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
    APP_InitServices();
#endif
    int err;
    err = bt_enable(bt_ready);
    if (err) {
        LOG_E("Bluetooth init failed (err %d)\n", err);
        goto exit;
    }
    while (1) {
#ifdef __ZEPHYR__
        k_msleep(1000);
#else
        vTaskDelay(1000);
#endif
    }

exit:
#endif // DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE
    UWBIOT_EXAMPLE_END(status);
}

/*
 * Interface which will be called from Main to create the required task with its own parameters.
 */
UWBOSAL_TASK_HANDLE uwb_demo_start(void)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBOSAL_TASK_HANDLE taskHandle;
    int pthread_create_status = 0;
    UWBIOT_STACK_DEFINE(DemoTrackerSR250_stack, DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE);
    PHOSALUWB_SET_TASKNAME(threadparams, DEMO_NEARBY_INTERACTION_SR250_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = DEMO_NEARBY_INTERACTION_SR250_TASK_PRIO;
#if UWBIOT_OS_ZEPHYR
    threadparams.pStack     = DemoTrackerSR250_stack;
    threadparams.stackdepth = UWBIOT_THREAD_STACK_SIZE(DemoTrackerSR250_stack);
#else
    threadparams.stackdepth = DEMO_NEARBY_INTERACTION_SR250_TASK_SIZE;
#endif // UWBIOT_OS_ZEPHYR
    pthread_create_status = phOsalUwb_Thread_Create((void **)&taskHandle, &StandaloneTask, &threadparams);
    if (0 != pthread_create_status) {
        NXPLOG_APP_E("Failed to create task %s", threadparams.taskname);
    }
    return taskHandle;
}

#endif // (UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
