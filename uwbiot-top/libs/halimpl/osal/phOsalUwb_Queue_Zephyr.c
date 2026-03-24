/*
 * Copyright 2024-2025 NXP.
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

/**
 * \file  phOsalUwb_Queue_Zephyr.c
 * @brief OSAL Implementation.
 */

/** Header Files */
#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"
#include "uwb_board.h"
#include <stdio.h>
#include <zephyr/kernel.h>

typedef struct {
    struct k_msgq msgq;
    uint8_t *buffer;  // Buffer for k_msgq
} phOsalUwb_Queue_t;

/**
* @brief Create a new message queue.
*/
intptr_t phOsalUwb_msgget(uint32_t queueLength)
{
    if (queueLength == 0) {
        return 0;  // Invalid queue size
    }

    phOsalUwb_Queue_t *pQueue = (phOsalUwb_Queue_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_Queue_t));
    if (pQueue == NULL) {
        return 0; // Memory allocation failed
    }
    // Allocate memory for the queue buffer dynamically
    pQueue->buffer = (uint8_t *)phOsalUwb_GetMemory(queueLength * sizeof(phLibUwb_Message_t));
    if (pQueue->buffer == NULL) {
        phOsalUwb_FreeMemory(pQueue);
        return 0; // Memory allocation for buffer failed
    }

    // Initialize the queue
    k_msgq_init(&pQueue->msgq, pQueue->buffer, sizeof(phLibUwb_Message_t), queueLength);
    return (intptr_t)pQueue;
}

/**
* @brief Release a message queue.
*/
void phOsalUwb_msgrelease(intptr_t msqid) {
    phOsalUwb_Queue_t *pQueue = (phOsalUwb_Queue_t *)msqid;
    if (pQueue != NULL) {
        if (pQueue->buffer != NULL) {
            phOsalUwb_FreeMemory(pQueue->buffer); // Free the dynamically allocated buffer
        }
        phOsalUwb_FreeMemory(pQueue); // Free the queue structure
    }
}

/**
* @brief Send a message to the queue (supports ISR context).
*/
UWBSTATUS phOsalUwb_msgsnd(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    int ret;
    phOsalUwb_Queue_t *pQueue = (phOsalUwb_Queue_t *)msqid;

    if (pQueue == NULL || msg == NULL) {
        return UWBSTATUS_FAILED;
    }

    /* Use K_NO_WAIT inside ISR, otherwise, use waittimeout */
    if (k_is_in_isr()) {
        ret = k_msgq_put(&pQueue->msgq, msg, K_NO_WAIT);
    } else {
        ret = k_msgq_put(&pQueue->msgq, msg, K_MSEC(waittimeout));
    }

    return (ret == 0) ? UWBSTATUS_SUCCESS : UWBSTATUS_FAILED;
}

/**
* @brief Receive a message from the queue.
*/
UWBSTATUS phOsalUwb_msgrcv(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout)
{
    phOsalUwb_Queue_t *pQueue = (phOsalUwb_Queue_t *)msqid;

    if (pQueue == NULL || msg == NULL) {
        return UWBSTATUS_FAILED;
    }

    int ret = k_msgq_get(&pQueue->msgq, msg, K_MSEC(waittimeout));

    return (ret == 0) ? UWBSTATUS_SUCCESS : UWBSTATUS_FAILED;
}

/**
* @brief Check if there is space available in the queue.
*/
int phOsalUwb_queueSpacesAvailable(intptr_t msqid)
{
    phOsalUwb_Queue_t *pQueue = (phOsalUwb_Queue_t *)msqid;

    if (pQueue == NULL) {
        return 0;
    }

    return k_msgq_num_free_get(&pQueue->msgq);
}

UWBSTATUS phOsalUwb_msgreset(intptr_t msqid)
{
    phOsalUwb_Queue_t *pMsgQ = (phOsalUwb_Queue_t *)msqid;

    if (pMsgQ == NULL || pMsgQ->buffer == NULL) {
        return UWBSTATUS_FAILED;
    }

    k_msgq_purge((struct k_msgq *)pMsgQ->buffer);

    return UWBSTATUS_SUCCESS;
}