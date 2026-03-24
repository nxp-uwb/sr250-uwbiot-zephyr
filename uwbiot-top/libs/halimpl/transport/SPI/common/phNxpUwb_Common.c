/*
 * Copyright (C) 2012-2020,2022, 2026 NXP
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

#include <stdio.h>
#include "phOsalUwb.h"
#include "phNxpUwb_Common.h"
#if UWBIOT_OS_FREERTOS
#include "cmsis_gcc.h"
#endif

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#if (UWBIOT_UWBD_SR04X)

/*
 *  Backoff Delay helps in improving the performance of complete system
 *          +
 *  Retry   |
 *  count   |                                         _________________
 *          |                                        |
 *          |                                   _____|
 *          |                                  |
 *          |                             _____|
 *          |                            |
 *          |                      ______|
 *          |                     /
 *          |                    /
 *          |                   /
 *          |                  /
 *          |                 /
 *          |  ______________/
 *          | /
 *          +----------------------------------------------------------------------+
 *            |<--- NOP ---->|<-------------ms---------------->|<-Timeout->|
 *
 */

void phNxpUwb_BackoffDelayReset(uint16_t *stepDelay)
{
    *stepDelay = 0;
}

int phNxpUwb_BackoffDelay(uint16_t *stepDelay)
{
    int timed_out = 0;
    int msDelay   = 0;
    if (*stepDelay <= 2) {
        __NOP();
        __NOP();
    }
    else if (*stepDelay <= 5) {
        /* Wait for millisec in incremental way till 10ms */
        msDelay = ((*stepDelay));
        phOsalUwb_Delay(msDelay);
    }
    else if (*stepDelay > 5 && *stepDelay <= 20) {
        /* Wait for millisec in steps till 50ms
         * +-------------------+---------------------+
         * | stepDelay         | Backoff             |
         * +-------------------+---------------------+
         * |  6 - 9            | 5ms                 |
         * | 10 - 19           | 6ms                 |
         * | 20                | 7ms                 |
         * +-------------------+---------------------+
         *
         */
        msDelay = 5 + ((*stepDelay / 10));
        phOsalUwb_Delay(msDelay);
    }
    else {
        /* Wait for max time before backoff timeout */
        phOsalUwb_Delay(10 * 2);
        if ((*stepDelay) > BACKOFF_TIMEOUT_VALUE) {
            phOsalUwb_Delay(100);
            timed_out = 1;
        }
    }
    (*stepDelay)++;
    return timed_out;
}
#endif /* UWBIOT_UWBD_SR04X */
