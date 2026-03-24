/* Copyright 2021-2024 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION

#include "phNxpLogApis_App.h"
#include "phUwbTypes.h"
#include "TLV_Types_i.h"

#ifndef __ZEPHYR__
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif // __ZEPHYR__

/* Standard library*/
#include <string.h>
#include <stdbool.h>

/* Interface includes */
#include "uwb_types.h"

/* Log */

#include "phOsalUwb.h"
#include "UwbApi_Types_Proprietary.h"
#include "UwbApi_Proprietary.h"
#include "uwb_nearby_service.h"

extern intptr_t tlvMngQueue;
static void *mHifSem = NULL;
extern uwb_ble_state_t g_UwbBleState;

#define MAX_HIF_TLV_WAIT (10)
static volatile UWB_Hif_t mInterface = UWB_HIF_BLE;

bool tlvSendRaw(uint8_t deviceId, uint8_t *buf, uint16_t size)
{
    /* Send TLV */
    bool ok = TRUE;

    if (mInterface == UWB_HIF_BLE) {
        bool bleResult = bt_gatt_uwb_notify(deviceId, buf, size);
        tlvSendDoneCb();
        if (bleResult != 0) {
            LOG_E("%s(): Failed to send over BLE: %X", __FUNCTION__, bleResult);
            ok = FALSE;
            goto end;
        }
    }
    else {
        LOG_E("%s(): Invalid interface %X\n", __FUNCTION__, mInterface);
        ok = FALSE;
        goto end;
    }

    if (phOsalUwb_ConsumeSemaphore_WithTimeout(mHifSem, MAX_HIF_TLV_WAIT) != UWBSTATUS_SUCCESS) {
        LOG_E("%s(): failed to wait HIF semaphore\n", __FUNCTION__);
        ok = FALSE;
    }

end:
    return ok;
}

void tlvSendDoneCb(void)
{
    (void)phOsalUwb_ProduceSemaphore(mHifSem);
}

void tlvRecv(uint8_t deviceId, UWB_Hif_t interface, uint8_t *tlv, uint8_t tlvSize)
{
    mInterface                                     = interface;
    static phLibUwb_Message_t shareable_config_buf = {0};
    shareable_config_buf.eMsgType                  = deviceId;
    shareable_config_buf.Size                      = tlvSize;
    shareable_config_buf.pMsgData                  = &tlv[0];

    (void)phOsalUwb_msgsnd(tlvMngQueue, &shareable_config_buf, NO_DELAY);
}

bool tlvBuilderInit(void)
{
    if (phOsalUwb_CreateSemaphore(&mHifSem, 0) != UWBSTATUS_SUCCESS) {
        LOG_E("Could not create TLV mutex");
        return FALSE;
    }
    return TRUE;
}
#endif // UWBIOT_APP_BUILD__DEMO_NEARBY_INTERACTION
