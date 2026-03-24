/* Copyright 2025,2026 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"
#if !defined(UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON)
#include "UWBIOT_APP_BUILD.h"
#endif
#if !defined(UWBIOT_APP_BUILD__DEMO_PNP)
#if UWBIOT_OS_ZEPHYR
#if defined(CPU_RW612ETA2I)
#include "board.h"
#include <stdbool.h>
#include "phOsalUwb.h"
#include <phNxpUciHal.h>
#include "Uwb_usb.h"
#include "UwbHif.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_ctrl.h>

#define MAX_UWBS_SPI_TRANSFER_TIMEOUT (1000)

#define UART_DEV_NODE DT_CHOSEN(zephyr_console) // USB CDC ACM is chosen as console
// static const struct device *cdc_acm_dev = DEVICE_DT_GET(UART_DEV_NODE);
const struct device *const cdc_acm_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);

static void (*usartRcvCb)(uint8_t *, uint32_t *);
void *mHifRspSem = NULL;

static uint16_t payload_len = 0;
static uint32_t rcv_sz      = 0;
#define UART_RX_BUF_SIZE 512 // HW limit per read

static uint8_t rx_buffer[5000]; // Buffer for incoming data for HDLL commands
uint16_t data_size;

/* mutex to protect usb write from multiple threads, release after taking
 * from ISR */
void *mUsbWriteMutex;

/* This semaphore is signaled in the USB CDC ISR context when any response is
 * read by Host*/
void *mHIfNtfnSem;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
static void usb_user_isr(const struct device *dev, void *user_data)
{
    /* Handle RX (Receiving from PC) */
    while (uart_irq_rx_ready(dev)) { // Keep reading until FIFO is empty
        rcv_sz += uart_fifo_read(dev, &rx_buffer[rcv_sz], UART_RX_BUF_SIZE);

        if (rcv_sz <= 0) {
            break; // No more data available, exit ISR
        }

        if (rcv_sz > 0) {
            // Extract Payload Length from Header
            payload_len = (rx_buffer[2] << 8) | rx_buffer[3];

            if (payload_len > UART_RX_BUF_SIZE - HEADER_SIZE_MCTT) {
                rcv_sz = 0;
                return;
            }
        }
        if (rcv_sz >= HEADER_SIZE_MCTT && payload_len > 0) {
            // Read Remaining Payload
            uint32_t bytes_to_read = payload_len - (rcv_sz - HEADER_SIZE_MCTT);
            rcv_sz += uart_fifo_read(dev, &rx_buffer[rcv_sz], bytes_to_read);
        }

        // Full message received
        if (rcv_sz == (HEADER_SIZE_MCTT + payload_len)) {
            if (usartRcvCb) {
                usartRcvCb(rx_buffer, &rcv_sz);
            }
            //ED_OFF(led_yellow);
            rcv_sz      = 0; // Reset for next message
            payload_len = 0;
        }
    }
}

uint32_t transmitToUsb(uint8_t *buffer, uint16_t size)
{
    uint32_t error = 0;

    int bytes_sent = uart_fifo_fill(cdc_acm_dev, buffer, size);
    if (bytes_sent < size) {
            error = 1;
    }
    return (uint32_t)error;
}

void Uwb_USB_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
    usartRcvCb = rcvCb;

    if (!device_is_ready(cdc_acm_dev)) {
        return;
    }

    if (usb_enable(NULL) != 0) {
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
        while (1)
            ;
    }
    uart_irq_callback_user_data_set(cdc_acm_dev, usb_user_isr, NULL);
    uart_irq_rx_enable(cdc_acm_dev); // Enable RX interrupt
}

#endif // UWBIOT_OS_ZEPHYR
#endif // CPU_RW612ETA2I
#endif // UWBIOT_APP_BUILD__DEMO_STANDALONE_COMMON
