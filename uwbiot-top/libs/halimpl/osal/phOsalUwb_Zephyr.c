/*
 * Copyright 2024 NXP.
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


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "phNxpUciHal_utils.h"
#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"
#include "phUwb_BuildConfig.h"
#include "uwb_board.h"

#define ZEPHYR_STATUS_SUCCESS    0

/** Function Definitions */

void *phOsalUwb_GetMemory(uint32_t dwSize)
{
    return (void *)malloc(dwSize);
}

void phOsalUwb_FreeMemory(void *pMem)
{
    /* Check whether a null pointer is passed */
    if (NULL != pMem) {
        free(pMem);
    }
}

int32_t phOsalUwb_MemCompare(const void *pDest, const void *pSrc, uint32_t dwSize)
{
    return memcmp(pDest, pSrc, dwSize);
}

void phOsalUwb_SetMemory(void *pMem, uint8_t bVal, uint32_t dwSize)
{
    memset(pMem, bVal, dwSize);
}

void phOsalUwb_MemCopy(void *pDest, const void *pSrc, uint32_t dwSize)
{
    memcpy(pDest, pSrc, dwSize);
}

UWBSTATUS phOsalUwb_CreateSemaphore(void **hSemaphore, uint8_t bInitialValue)
{
    UWBSTATUS wCreateStatus                      = UWBSTATUS_FAILED;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = NULL;

    if (k_is_in_isr()) {
        printf("\nk_sem_init() is not allowed to use inside ISR\n");
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
        return wCreateStatus;
    }

    /* Check whether user passed valid input parameters */
    if (NULL == hSemaphore) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Allocate memory for Semaphore handle */
        pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalSemaphore_t));
        /* Check whether memory is valid*/
        if (pSemaphoreHandle == NULL) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
        }
        else {
            /*initialize Semaphore object Handle*/
            pSemaphoreHandle->ObjectHandle =  NULL;
            /*Allocate memory for Semaphore object Handle*/
            pSemaphoreHandle->ObjectHandle = (k_sem_t *)phOsalUwb_GetMemory(sizeof(k_sem_t));
            /* Check whether memory is valid*/
            if(pSemaphoreHandle->ObjectHandle == NULL){
                /*Update the status*/
                wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
                /*Free Semaphore Handle*/
                phOsalUwb_FreeMemory(pSemaphoreHandle);
            }
            else
            {
                /* k_sem_init() parameters as follows
                 * Address of the semaphore
                 * Initial semaphore count
                 * Maximum permitted semaphore count.
                 * It returns 0 on sucess and Invalid on failures 
                 * binary semaphore*/
                if (k_sem_init(pSemaphoreHandle->ObjectHandle, bInitialValue, 1) !=
                    ZEPHYR_STATUS_SUCCESS) {
                        /* Free allocated*/
                    phOsalUwb_FreeMemory(pSemaphoreHandle->ObjectHandle);
                    phOsalUwb_FreeMemory(pSemaphoreHandle);
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
                }
                else {
                    /* Return the semaphore handle to the caller function */
                    *hSemaphore = (void *)pSemaphoreHandle;
                    /*Update the return status*/
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
                }
            }
        }
    }
    return wCreateStatus;
}

UWBSTATUS phOsalUwb_ProduceSemaphore(void *hSemaphore)
{
    UWBSTATUS wReleaseStatus                     = UWBSTATUS_FAILED;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)hSemaphore;

    /* Check whether user passed valid parameter */
    if (pSemaphoreHandle == NULL || pSemaphoreHandle->ObjectHandle == NULL) {
        wReleaseStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* This routine gives sem, unless the semaphore is already at its maximum permitted count.
            * Parameter
            * Address of the semaphore.*/
        k_sem_give(pSemaphoreHandle->ObjectHandle);
        /*Update the return status*/
        wReleaseStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
    }

    return wReleaseStatus;
}

UWBSTATUS phOsalUwb_ConsumeSemaphore_WithTimeout(void *hSemaphore, uint32_t delayms)
{
    int ret;
    UWBSTATUS wConsumeStatus                     = UWBSTATUS_FAILED;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)hSemaphore;

    /* Check whether user passed valid parameter */
    if (pSemaphoreHandle == NULL || pSemaphoreHandle->ObjectHandle == NULL) {
        wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Wait till the semaphore object is released
         * Take a semaphore*/
        if (k_is_in_isr()) {
            ret = k_sem_take(pSemaphoreHandle->ObjectHandle, K_NO_WAIT);
        }
        else {
            ret = k_sem_take(pSemaphoreHandle->ObjectHandle, K_MSEC(delayms));
        }
        if (ret != ZEPHYR_STATUS_SUCCESS) {
            wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CONSUME_ERROR);
        }
        else {
            /*Update the return status*/
            wConsumeStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
        }
    }
    return wConsumeStatus;
}

UWBSTATUS phOsalUwb_DeleteSemaphore(void **hSemaphore)
{
    UWBSTATUS wDeletionStatus                    = UWBSTATUS_FAILED;
    phOsalUwb_sOsalSemaphore_t *pSemaphoreHandle = (phOsalUwb_sOsalSemaphore_t *)*hSemaphore;

    if (k_is_in_isr()) {
        printf("\n k_sem_reset() is not allowed to use inside ISR\n");
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_DELETE_ERROR);
        return wDeletionStatus;
    }

    /* Check whether OSAL is initialized and user has passed a valid pointer */
    if (pSemaphoreHandle == NULL) {
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Check whether function was successful */
        k_sem_reset(pSemaphoreHandle->ObjectHandle);
        /*Free allocated memory*/
        phOsalUwb_FreeMemory(pSemaphoreHandle->ObjectHandle);
        phOsalUwb_FreeMemory(pSemaphoreHandle);
        *hSemaphore = NULL;
        /*Update the return status*/
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
    }

    return wDeletionStatus;
}

UWBSTATUS phOsalUwb_CreateBinSem(void **hBinSem)
{
    UWBSTATUS wCreateStatus                   = UWBSTATUS_FAILED;
    phOsalUwb_sOsalSemaphore_t *pBinSemHandle = NULL;

    if (k_is_in_isr()) {
        printf("\nk_sem_init() is not allowed to use inside ISR\n");
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
        return wCreateStatus;
    }

    /* Check whether user passed valid input parameters */
    if (NULL == hBinSem) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Allocate memory for Semaphore handle */
        pBinSemHandle = (phOsalUwb_sOsalSemaphore_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalSemaphore_t));
        /* Check whether memory is valid*/
        if(pBinSemHandle == NULL) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
        }
        else{
            /*initialize Semaphore object Handle*/
            pBinSemHandle->ObjectHandle = NULL;
            /*Allocate memory for Semaphore object Handle*/
            pBinSemHandle->ObjectHandle = (k_sem_t *)phOsalUwb_GetMemory(sizeof(k_sem_t));
            /* Check whether memory is valid*/
            if (pBinSemHandle->ObjectHandle == NULL) {
                wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
                /*Free Semaphore Handle*/
                phOsalUwb_FreeMemory(pBinSemHandle);
            }
            else {
                /* k_sem_init() parameters as follows
                * Address of the semaphore
                * Initial semaphore count
                * Maximum permitted semaphore count.
                * It returns 0 on sucess and Invalid on failures */
                if (k_sem_init(pBinSemHandle->ObjectHandle, 0, 1) != ZEPHYR_STATUS_SUCCESS) {
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_CREATION_ERROR);
                    phOsalUwb_FreeMemory(pBinSemHandle->ObjectHandle);
                    phOsalUwb_FreeMemory(pBinSemHandle);
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_SEMAPHORE_CREATION_ERROR);
                }
                else {
                    *hBinSem = (void *)pBinSemHandle;
                    /*Update the return status*/
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
                }
            }

        }
    }

    return wCreateStatus;
}

UWBSTATUS phOsalUwb_CreateMutex(void **hMutex)
{
    UWBSTATUS wCreateStatus              = UWBSTATUS_FAILED;
    phOsalUwb_sOsalMutex_t *pMutexHandle = NULL;

    if (k_is_in_isr()) {
        printf("\n k_mutex_init() is not allowed to use inside ISR\n");
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_CREATION_ERROR);
        return wCreateStatus;
    }

    if (NULL == hMutex) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Allocate memory for Mutex handle & Mutex struct */
        pMutexHandle               = (phOsalUwb_sOsalMutex_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalMutex_t));
        /* Check whether memory is valid*/
        if (pMutexHandle == NULL){
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_CREATION_ERROR);
        }
        else{
            /*initialize Mutex object Handle*/
            pMutexHandle->ObjectHandle = NULL;
            pMutexHandle->ObjectHandle = (k_mutex_t *)phOsalUwb_GetMemory(sizeof(k_mutex_t));
            /* Check whether memory is valid*/
            if (pMutexHandle->ObjectHandle == NULL) {
                wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_CREATION_ERROR);
                phOsalUwb_FreeMemory(pMutexHandle);
            }
            else {
                /* This routine initializes a mutex object, prior to its first use.
                 * Upon completion, the mutex is available and does not have an owner.*/
                if (k_mutex_init(pMutexHandle->ObjectHandle) != ZEPHYR_STATUS_SUCCESS) {
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_CREATION_ERROR);
                    phOsalUwb_FreeMemory(pMutexHandle->ObjectHandle);
                    phOsalUwb_FreeMemory(pMutexHandle);
                }
                else {
                    *hMutex = (void *)pMutexHandle;
                    /*Update the return status*/
                    wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
                }
            }
        }
    }

    return wCreateStatus;
}

UWBSTATUS phOsalUwb_LockMutex(void *hMutex)
{
    UWBSTATUS wLockStatus                = UWBSTATUS_FAILED;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)hMutex;

    /* Check whether handle provided by user is valid */
    if (pMutexHandle == NULL || pMutexHandle->ObjectHandle == NULL) {
        wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        if (k_is_in_isr()) {
            /*In zephyr Mutex Lock is not allowed in ISR*/
            printf("Mutex locking is not allowed inside ISR");
            wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_LOCK_ERROR);
        }
        else {
            /* This routine locks mutex
             * Parameters
             * Address of the mutex
             * Waiting period to lock the mutex*/
            if ((k_mutex_lock(pMutexHandle->ObjectHandle, K_FOREVER)) != ZEPHYR_STATUS_SUCCESS) {
                wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_LOCK_ERROR);
            }
            else {
                /*Update the return status*/
                wLockStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
            }
        }
    }
    return wLockStatus;
}

UWBSTATUS phOsalUwb_UnlockMutex(void *hMutex)
{
    UWBSTATUS wUnlockStatus              = UWBSTATUS_FAILED;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)hMutex;

    /* Check whether the handle provided by user is valid */
    if (pMutexHandle == NULL || pMutexHandle->ObjectHandle == NULL) {
        wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        if (k_is_in_isr()) {
           /*In zephyr Mutex unlock is not allowed in ISR*/
            printf("Mutex unlocking is not allowed inside ISR");
            wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
        }
        else {
            /* This routine unlocks mutex
             * Parameters
             * Address of the mutex */
            if ((k_mutex_unlock(pMutexHandle->ObjectHandle)) != ZEPHYR_STATUS_SUCCESS) {
                wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_MUTEX_UNLOCK_ERROR);
            }
            else {
                /*Update the return status*/
                wUnlockStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
            }
        }
    }

    return wUnlockStatus;
}

UWBSTATUS phOsalUwb_DeleteMutex(void **hMutex)
{
    UWBSTATUS wDeletionStatus            = UWBSTATUS_FAILED;
    phOsalUwb_sOsalMutex_t *pMutexHandle = (phOsalUwb_sOsalMutex_t *)*hMutex;

    /* Check whether the handle provided by user is valid */
    if (pMutexHandle == NULL || pMutexHandle->ObjectHandle == NULL) {
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        phOsalUwb_FreeMemory(pMutexHandle->ObjectHandle);
        phOsalUwb_FreeMemory(pMutexHandle);
        *hMutex = NULL;
        /*Update the return status*/
        wDeletionStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_SUCCESS);
    }

    return wDeletionStatus;
}
/* Delay  in Milli second */
void phOsalUwb_Delay(uint32_t dwDelay)
{
    k_msleep(dwDelay);
}

void phOsalUwb_GetTickCount(unsigned long *pTickCount)
{
    /*Return the current system tick count*/
    *pTickCount = sys_clock_tick_get();
}
