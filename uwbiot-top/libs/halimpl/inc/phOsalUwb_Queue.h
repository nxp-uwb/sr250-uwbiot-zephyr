/*
 * Copyright 2012-2023,2024,2025 NXP.
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
 * \file
 * @brief OSAL header files related to queue functions.
 */

#ifndef PHOSALUWB_QUEUE_H
#define PHOSALUWB_QUEUE_H

/** Include Files */

#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"

#define configTML_QUEUE_LENGTH 50
#define NO_DELAY               ((TickType_t)0U)

/** \addtogroup grp_osal_queue
 *
 * @{
 */

/**
 * \brief Allocates message queue
 *
 * \param[in] queueLength Length of the queue ignored for linux
 *
 * \retval          (int) value of pQueue if successful
 *                  -1, if failed to allocate memory or to init mutex
 *
 */
intptr_t phOsalUwb_msgget(uint32_t queueLength);

/**
 * \brief Releases message queue
 *
 * \param[in] msqid message queue handle
 *
 * \retval          None
 *
 */
void phOsalUwb_msgrelease(intptr_t msqid);

/**
 * \brief Destroys message queue, needed only for posix
 *
 * \param[in] msqid message queue handle
 * \param[in] cmd  ignored, included only for Linux queue API
 *                  compatibility
 * \param[in] buf ignored, included only for Linux queue API
 *                  compatibility
 *
 * \retval 0  if successful
 * \retval -1 if invalid handle is passed
 *
 */
int phOsalUwb_msgctl(intptr_t msqid, int cmd, void *buf);

/**
 * \brief Sends a message to the queue. The message will be added at
 * the end of the queue as appropriate for FIFO policy
 *
 * \param[in] msqid  message queue handle
 * \param[in] msg    message to be sent
 * \param[in] waittimeout  timeout in ms
 *
 * \retval #UWBSTATUS_SUCCESS  if successful
 * \retval #UWBSTATUS_FAILED   if invalid parameter passed or failed to allocate memory
 *
 */
UWBSTATUS phOsalUwb_msgsnd(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout);

/**
 * \brief Gets the oldest message from the queue.
 * If the queue is empty the function waits (blocks on a mutex)
 * until a message is posted to the queue with phOsalUwb_msgsnd.
 *
 * \param[in] msqid   message queue handle
 * \param[in] msg     container for the message to be received
 * \param[in] waittimeout  max wait time for task in ms
 *
 * \retval #UWBSTATUS_SUCCESS if successful
 * \retval #UWBSTATUS_FAILED  if invalid parameter passed
 *
 */
UWBSTATUS phOsalUwb_msgrcv(intptr_t msqid, phLibUwb_Message_t *msg, unsigned long waittimeout);

/**
 * \brief Gets the space available in queue
 *
 * \param[in] msqid   message queue handle
 *
 * \retval -1 if invalid parameter passed
 * \retval others space available if successful
 *
 */
int phOsalUwb_queueSpacesAvailable(intptr_t msqid);


/**
 * \brief Required for PCTT Demo APP Which needs queue reset in case
 * interface is stuck
 *
 * \param[in] msqid message queue handle
 *
 * \retval #UWBSTATUS_SUCCESS if successful
 */
UWBSTATUS phOsalUwb_msgreset(intptr_t msqid);


/** @} */

#endif /*  PHOSALUWB_QUEUE_H  */

