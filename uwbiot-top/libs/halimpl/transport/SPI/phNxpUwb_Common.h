/*
 * Copyright 2019,2020,2022 NXP
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

#ifndef UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPICOMMON_H_
#define UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPICOMMON_H_

#include "phUwbTypes.h"

#define BACKOFF_TIMEOUT_VALUE 30

#define SHIFT_AND_OVERRIDE_HEADER(buff, hdr_len) \
    buff = buff + UCI_CMD_INDEX;     \
    for (int i = hdr_len; i > 0; i--) {          \
        (buff)[i] = (buff)[i - 1];               \
    }                                            \
    buff = buff + 1;

EXTERNC void phNxpUwb_BackoffDelayReset(uint16_t *stepDelay);

/* Returns 1 if timed out beyond reasonable limit
 *
 * Increments stepDelay by 1.
 */
EXTERNC int phNxpUwb_BackoffDelay(uint16_t *stepDelay);

#define ENSURE_BACKOFF_TIMEOUT_OR_RET(COUNT, RETVAL)      \
    timeout = phNxpUwb_BackoffDelay(&COUNT);              \
    if (timeout == 1) {                                   \
        LOG_D("SPI Backoff timeout. Line: %d", __LINE__); \
        return RETVAL;                                    \
    }

#define ENSURE_BACKOFF_TIMEOUT_OR_CLEANUP(COUNT)          \
    timeout = phNxpUwb_BackoffDelay(&COUNT);              \
    if (timeout == 1) {                                   \
        LOG_D("SPI Backoff timeout. Line: %d", __LINE__); \
        goto cleanup;                                     \
    }

#define ENSURE_BACKOFF_TIMEOUT_OR_UNLOCKMUTEX(COUNT)      \
    timeout = phNxpUwb_BackoffDelay(&COUNT);              \
    if (timeout == 1) {                                   \
        LOG_D("SPI Backoff timeout. Line: %d", __LINE__); \
        goto unlockmutex;                                 \
    }

#endif // UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPICOMMON_H_
