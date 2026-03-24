/*
 * Copyright 2012-2020 NXP.
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

#ifndef _PHNXPUCIHAL_ADAPTATION_H_
#define _PHNXPUCIHAL_ADAPTATION_H_

typedef uint8_t uwb_event_t;
typedef uint8_t uwb_status_t;
#include "uwb_types.h"
#include "uwb_hal_api.h"
/*`
 * The callback passed in from the UWB stack that the HAL
 * can use to pass events back to the stack.
 */
typedef void(uwb_stack_callback_t)(uwb_event_t event, uwb_status_t event_status);

/*
 * The callback passed in from the UWB stack that the HAL
 * can use to pass incoming data to the stack.
 */
typedef void(uwb_stack_data_callback_t)(uint16_t data_len, uint8_t *p_data);

/*
 * The callback is passed in from the application
 * callback to be invoked on data packet reception
 */
typedef void(phHalAppDataCb)(uint8_t *recvBuf, uint16_t dataLen);

/* NXP HAL functions */

/**
 * Function         phNxpUciHal_open
 *
 * Description      This function is called by libuwb-uci during the
 *                  initialization of the UWBC. It opens the physical connection
 *                  with UWBC and creates required client thread for
 *                  operation.
 *                  After open is complete, status is informed to libuwb-uci
 *                  through callback function.
 *
 * Returns          This function return UWBSTATUS_SUCCES (0) in case of success
 *                  In case of failure returns other failure value.
 *
 */
int phNxpUciHal_open(uwb_stack_callback_t *p_cback, uwb_stack_data_callback_t *p_data_cback);

/**
 * Function         phNxpUciHal_write
 *
 * Description      This function write the data to UWBC through physical
 *                  interface (e.g. SPI) using the SR100 driver interface.
 *
 * Returns          It returns number of bytes successfully written to UWBC.
 *
 */
int phNxpUciHal_write(uint16_t data_len, const uint8_t *p_data);

/**
 * Function         phNxpUciHal_register_data_callback
 *
 * Description      This function register data packet callback
 *
 * Returns          void
 *
 */
void phNxpUciHal_register_appdata_callback(phHalAppDataCb *appDataCb);

/**
 *  Function         phNxpUciHal_close
 *
 * Description      This function close the UWBC interface and free all
 *                  resources.This is called by libuwb-uci on UWB service stop.
 *
 * Returns          Always return UWBSTATUS_SUCCESS (0).
 *
 */
int phNxpUciHal_close();

/**
 * Function         phNxpUciHal_ioctl
 *
 * Description      This function is called by jni when wired mode is
 *                  performed.First SR100 driver will give the access
 *                  permission whether wired mode is allowed or not
 *                  arg (0):
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the actual state of operation in arg pointer
 *
 */
int phNxpUciHal_ioctl(long arg, tHAL_UWB_IOCTL *p_data);

/**
 * Function         phNxpUciHal_uwbDeviceInit
 *
 * Description      This function is called to initialize UWB device. It performs
 *                  firmware download and set device configuration
 * Returns          return status
 *
 */
int phNxpUciHal_uwbDeviceInit(BOOLEAN recovery);

/**
 * Function         phNxpUciHal_SetOperatingMode
 *
 * Description      Register the UWB Operating Mode
 *
 * Returns          None
 *
 */
void phNxpUciHal_SetOperatingMode(Uwb_operation_mode_t mode);
#endif /* _PHNXPUCIHAL_ADAPTATION_H_ */
