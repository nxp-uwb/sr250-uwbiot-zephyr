/**
 *
 *  Copyright (C) 2011-2012 Broadcom Corporation
 *  Copyright 2018-2021,2023, 2026 NXP.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef __NXPCONFIG_H
#define __NXPCONFIG_H

#include <uwb_board_values.h> /*<> For legacy reasons */
#include <phNxpUciHal_CoreConfig.h>

/**
**
** Function:    GetNumValue
**
** Description: API function for getting a numerical value of a setting
**
** Returns:     TRUE, if successful
**
*/
int phNxpUciHal_GetNxpNumValue(unsigned char key, void *p_value, unsigned long len);

#if !(UWBIOT_UWBD_SR04X)
/**
**
** Function:    GetByteArrayValue()
**
** Description: Read byte array value from the config file.
**
** Parameters:
**              name    - name of the config param to read.
**              pValue  - pointer to input buffer.
**              len     - input buffer length.
**              readlen - out parameter to return the number of bytes read from
**                        config file
**                        return -1 in case bufflen is not enough.
**
** Returns:     TRUE[1] if config param name is found in the config file, else
**              FALSE[0]
**
*/
int phNxpUciHal_GetNxpByteArrayValue(unsigned char key, void **pValue, long *readlen);
#endif // #if !(UWBIOT_UWBD_SR04X)
#endif // __NXPCONFIG_H
