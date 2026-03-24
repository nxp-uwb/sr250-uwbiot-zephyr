/*
 * Copyright 2025,2026 NXP
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

#include "phUwb_BuildConfig.h"

#if UWBIOT_OS_ZEPHYR

#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif

#if defined(UWBIOT_APP_BUILD__DEMO_PNP)

#if defined(CPU_RW612ETA2I)
#include "board.h"
#include "Uwb_Vcom_Pnp.h"
#include "UwbPnpInternal.h"
#include "app_config.h"
#include "UwbPnpInternal.h"
#include "UwbApi_Utility.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_ctrl.h>

#define UART_DEV_NODE DT_CHOSEN(zephyr_console) // USB CDC ACM is chosen as console
// static const struct device *cdc_acm_dev = DEVICE_DT_GET(UART_DEV_NODE);
const struct device *const cdc_acm_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);

static void (*usartRcvCb)(uint8_t *, uint32_t *);
void *mHifRspSem = NULL;
#define UART_HEADER_SIZE 3
#define USB_CHUNK_SIZE   512 // HW limit per read
static uint32_t payload_len    = 0;
static uint16_t bytes_received = 0;

static uint8_t rx_buf[5000]; // Buffer for incoming data for HDLL commands
uint16_t data_size;

static void usb_user_isr(const struct device *dev, void *user_data)
{
    /* Handle RX (Receiving from PC) */
    while (uart_irq_rx_ready(dev)) { // Keep reading until FIFO is empty
        int bytes_read = uart_fifo_read(dev, &rx_buf[bytes_received], USB_CHUNK_SIZE);

        if (bytes_read <= 0) {
            break; // No more data available, exit ISR
        }

        if (bytes_received == 0) {
            // Extract payload length on the first chunk
            payload_len = (rx_buf[UART_HEADER_SIZE - 2] << (8 * 1)) | (rx_buf[UART_HEADER_SIZE - 1] << (8 * 0));
            payload_len += UART_HEADER_SIZE;

            // Validate payload length (optional)
            if (payload_len > sizeof(rx_buf)) {
                payload_len    = 0; // Reset on invalid length
                bytes_received = 0;
                return;
            }
        }

        // Accumulate received data
        bytes_received += bytes_read;

        // If full payload is received, call the callback
        if (bytes_received >= payload_len) {
            if (usartRcvCb) {
                usartRcvCb(rx_buf, (uint32_t *)&payload_len);
            }
            bytes_received = 0; // Reset for next packet
            payload_len    = 0;
            break; // Stop further processing as we completed one message
        }
    }
}

void Uwb_Usb_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
    usartRcvCb = rcvCb;

    if (!device_is_ready(cdc_acm_dev)) {
        PRINTF_WITH_TIME("CDC ACM device not ready");
        return;
    }

    if (usb_enable(NULL) != 0) {
        PRINTF_WITH_TIME("Failed to enable USB");
        return;
    }

    while (true) {
        uint32_t dtr = 0U;

        uart_line_ctrl_get(cdc_acm_dev, UART_LINE_CTRL_DTR, &dtr);
        if (dtr) {
            break;
        }
        else {
            /* Give CPU resources to low priority threads. */
            k_sleep(K_MSEC(100));
        }
    }

    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending UCI resp from
     * Rhodes*/
    if (phOsalUwb_CreateBinSem(&mHifRspSem) != UWBSTATUS_SUCCESS) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifRspSem\n");
        while (1)
            ;
    }
    uart_irq_callback_user_data_set(cdc_acm_dev, usb_user_isr, NULL);
    uart_irq_rx_enable(cdc_acm_dev); // Enable RX interrupt
}

uint32_t UWB_Usb_SendRsp(uint8_t *pData, uint16_t size)
{
    uint32_t error = 0;

    int bytes_sent = uart_fifo_fill(cdc_acm_dev, pData, size);
    if (bytes_sent < size) {
            error = 1;
    }
    return (uint32_t)error;
}

#endif // defined(CPU_RW612ETA2I)
#endif // UWBIOT_OS_ZEPHYR
#endif // UWBIOT_APP_BUILD__DEMO_PNP
