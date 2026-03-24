/**
 *  Copyright (C) 2011-2012 Broadcom Corporation
 *  Copyright 2018-2022,2023, 2026 NXP.
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

#include <stdio.h>
#include <inttypes.h>
#include <phNxpUwbConfig.h>
#include <UWB_DeviceConfig.h>

#include "phOsalUwb.h"
#include "phNxpLogApis_HalUci.h"

#define DATA_HEADER_LENGTH 4

/* Local functions declarations */
static const NxpParam_t *phNxpUciHal_NxpParamFind(const unsigned char key);

/**
**
** Function:    phNxpUciHal_NxpParamFind
**
** Description: search if a setting exist in the setting array
**
** Returns:     pointer to the setting object
**
*/
static const NxpParam_t *phNxpUciHal_NxpParamFind(const unsigned char key)
{
    int i;
    int listSize;

    listSize = (sizeof(phNxpUciHal_NXPConfig) / sizeof(NxpParam_t));

    if (listSize == 0)
        return NULL;

    for (i = 0; i < listSize; ++i) {
        if (phNxpUciHal_NXPConfig[i].key == key) {
            if (phNxpUciHal_NXPConfig[i].type == TYPE_DATA) {
                NXPLOG_UCIHAL_D("%s found key %d, data len = %d\n",
                    __FUNCTION__,
                    key,
                    *((unsigned char *)(phNxpUciHal_NXPConfig[i].val)));
            }
            else {
                NXPLOG_UCIHAL_D("%s found key %d = (0x% " PRIxPTR ")\n",
                    __FUNCTION__,
                    key,
                    (uintptr_t)phNxpUciHal_NXPConfig[i].val);
            }
            return &(phNxpUciHal_NXPConfig[i]);
        }
    }
    return NULL;
}

#if !(UWBIOT_UWBD_SR04X)
extern int phNxpUciHal_GetNxpByteArrayValue(unsigned char key, void **pValue, long *readlen)
{
    long ucilen;
    if (!pValue) {
        return FALSE;
    }

    const NxpParam_t *pParam = phNxpUciHal_NxpParamFind(key);

    if (pParam == NULL) {
        return FALSE;
    }
    if ((pParam->type == TYPE_DATA) && (pParam->val != 0)) {
        *readlen = (long)((unsigned char *)(pParam->val))[0];
        /* If Over All Length defined in  pParam is greater than 0 then only fetch other componnet values */
        if (*readlen !=  0)
        {
            *pValue = &(((unsigned char *)pParam->val)[1]);
            ucilen   = (long)((unsigned char *)(pParam->val))[4];
            if (((ucilen + DATA_HEADER_LENGTH) != (*readlen)) && (*readlen != 0x00)) {
                NXPLOG_UCIHAL_W("%s, found key %d, Core Config Length Error = %d\n", __FUNCTION__, key, ucilen);
            }
            return TRUE;
        }
    }
    else if ((pParam->type == TYPE_EXTENDED_DATA) && (pParam->val != 0)) {
        unsigned char *pBuffer = &(((unsigned char *)pParam->val)[0]);
        *readlen               = (long)(pBuffer[0]);
        /* If Over All Length defined in  pParam is greater than 0 then only fetch other componnet values */
        if(*readlen !=0)
        {
            *pValue                = &(pBuffer[2]);
            *readlen |= (long)((pBuffer[1] << 8) & 0xFFFF);
            return TRUE;
        }
    }
    return FALSE;
}
#endif /* (UWBIOT_UWBD_SR04X) */
extern int phNxpUciHal_GetNxpNumValue(unsigned char key, void *pValue, unsigned long len)
{
    if (!pValue) {
        return FALSE;
    }

    const NxpParam_t *pParam = phNxpUciHal_NxpParamFind(key);

    if (pParam == NULL) {
        return FALSE;
    }

    size_t v = (size_t)pParam->val;

    switch (len) {
    case sizeof(unsigned long):
        *((unsigned long *)(pValue)) = (unsigned long)v;
        break;
    case sizeof(unsigned short):
        *((unsigned short *)(pValue)) = (unsigned short)v;
        break;
    case sizeof(unsigned char):
        *((unsigned char *)(pValue)) = (unsigned char)v;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
