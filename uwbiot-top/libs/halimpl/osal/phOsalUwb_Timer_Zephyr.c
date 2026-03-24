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

/*
 * OSAL Implementation for Timers.
 */
#ifndef COMPANION_DEVICE
#include "phOsalUwb.h"
#include "phUwbCommon.h"
#include "phUwbTypes.h"
#include "phNxpLogApis_TmlUwb.h"

static phOsalUwb_TimerHandle_t apTimerInfo[PH_UWB_MAX_TIMER];

/* Forward declarations */
static void phOsalUwb_Timer_Expired(struct k_timer *timer);

uint32_t phOsalUwb_Timer_Create(bool bAutoReload)
{
    phOsalUwb_TimerHandle_t *pTimerHandle;
    /* Zephyr Timer Object Pointer*/
    struct k_timer *ztimer;

    if (k_is_in_isr()) {
        return PH_OSALUWB_TIMER_CREATE_ERROR;
    }

    /* dwTimerId is also used as an index at which timer object can be stored */
    /* Timer needs to be initialized for timer usage */
    uint32_t dwTimerId = phUtilUwb_CheckForAvailableTimer();

    /* Check whether timers are available, if yes create a timer handle structure
     */
    if (dwTimerId < PH_UWB_MAX_TIMER) {
        pTimerHandle = (phOsalUwb_TimerHandle_t *)&apTimerInfo[dwTimerId];

        /* Allocating memory for timer object*/
        ztimer = (struct k_timer *)phOsalUwb_GetMemory(sizeof(struct k_timer));
        if(ztimer == NULL){
            return PH_OSALUWB_TIMER_START_ERROR;
        }

        /* This routine initializes a timer, prior to its first use.
         * Parameters
         * Address of timer
         * Function to invoke each time the timer expires
         * Function to invoke if the timer is stopped while running */
        k_timer_init(ztimer, &phOsalUwb_Timer_Expired, NULL);
        /*Storing timer handle*/
        pTimerHandle->hTimerHandle = (void *)ztimer;
        /* Set the state to indicate timer is ready */
        pTimerHandle->eState = eTimerIdle;
        /* Store the Timer Id which shall act as flag during check for timer
            * availability */
        pTimerHandle->TimerId = dwTimerId;
    }
    else {
        dwTimerId = PH_OSALUWB_TIMER_ID_INVALID;
    }

    /* Timer ID invalid can be due to Uninitialized state,Non availability of
     * Timer */
    return dwTimerId;
}

UWBSTATUS phOsalUwb_Timer_Start(
    uint32_t dwTimerId, uint32_t dwRegTimeCnt, pphOsalUwb_TimerCallbck_t pApplication_callback, void *pContext)
{
    UWBSTATUS wStartStatus = UWBSTATUS_SUCCESS;

    phOsalUwb_TimerHandle_t *pTimerHandle;

    /*Determine if code is running at interrupt level*/
    if (k_is_in_isr()) {
        return PH_OSALUWB_TIMER_START_ERROR;
    }

    if (dwTimerId < PH_UWB_MAX_TIMER) {
        /* Retrieve the index at which the timer handle structure is stored */
        pTimerHandle = (phOsalUwb_TimerHandle_t *)&apTimerInfo[dwTimerId];

        if ((NULL != pApplication_callback) && (NULL != pTimerHandle->hTimerHandle)) {
            /* OSAL Module needs to be initialized for timer usage */
            /* Check whether the handle provided by user is valid */

            pTimerHandle->Application_callback = pApplication_callback;
            pTimerHandle->pContext             = pContext;
            pTimerHandle->eState               = eTimerRunning;

            /* This routine starts a timer, and resets its status to zero. The timer
             * begins counting down using the specified duration and period values.
             * Parameters as follows
             * timer     Address of timer.
             * duration  Initial timer duration.
             * period    Timer period.
             * */

            if (pTimerHandle->bAutoReload == TRUE) {
                k_timer_start(pTimerHandle->hTimerHandle, K_MSEC(dwRegTimeCnt), K_MSEC(dwRegTimeCnt));
            }
            else {
                k_timer_start(pTimerHandle->hTimerHandle, K_MSEC(dwRegTimeCnt), K_NO_WAIT);
            }
        }
        else {
            wStartStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
        }
    }
    else {
        wStartStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wStartStatus;
}

UWBSTATUS phOsalUwb_Timer_Stop(uint32_t dwTimerId)
{
    UWBSTATUS wStopStatus = UWBSTATUS_SUCCESS;

    phOsalUwb_TimerHandle_t *pTimerHandle;

    if (dwTimerId < PH_UWB_MAX_TIMER) {
        /* OSAL Module and Timer needs to be initialized for timer usage */
        pTimerHandle = (phOsalUwb_TimerHandle_t *)&apTimerInfo[dwTimerId];
        /* Check whether the TimerId provided by user is valid */

        if((NULL != pTimerHandle->hTimerHandle) && (pTimerHandle->eState == eTimerStopped)){
            /*timer is already stopped: no action required*/
        }
        else if ((NULL != pTimerHandle->hTimerHandle) && (pTimerHandle->eState == eTimerRunning)) {
            /* Stop the timer only if the callback has not been invoked */
            k_timer_stop(pTimerHandle->hTimerHandle);
            /* Change the state of timer to Stopped */
            pTimerHandle->eState = eTimerStopped;
        }
        else {
            wStopStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
        }
    }
    else {
        wStopStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wStopStatus;
}

UWBSTATUS phOsalUwb_Timer_Delete(uint32_t dwTimerId)
{
    UWBSTATUS wDeleteStatus = UWBSTATUS_SUCCESS;

    phOsalUwb_TimerHandle_t *pTimerHandle;

    if (dwTimerId < PH_UWB_MAX_TIMER) {
        /* OSAL Module and Timer needs to be initialized for timer usage */
        pTimerHandle = (phOsalUwb_TimerHandle_t *)&apTimerInfo[dwTimerId];
        /* Check whether the TimerId passed by user is valid and Deregistering of
         * timer is successful */
        if(pTimerHandle->hTimerHandle == NULL){
            /*timer is already delted*/
            return wDeleteStatus;
        }
        else if (UWBSTATUS_SUCCESS == phOsalUwb_CheckTimerPresence(pTimerHandle)) {
            /*Check if timer is stopped*/
            if(pTimerHandle->eState != eTimerStopped){
                /* Cancel the timer before deleting */
                k_timer_stop(pTimerHandle->hTimerHandle);
            }

            /*Clear the memory allocated for timer*/
            phOsalUwb_FreeMemory(pTimerHandle->hTimerHandle);

            /* Clear Timer structure used to store timer related data */
            phOsalUwb_SetMemory(pTimerHandle, (uint8_t)0x00, sizeof(phOsalUwb_TimerHandle_t));
        }
        else {
            wDeleteStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
        }
    }
    else {
        wDeleteStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }

    return wDeleteStatus;
}

/**
** Function         phOsalUwb_Timer_Expired
**
** Description      posts message upon expiration of timer
**                  Shall be invoked when any one timer is expired
**                  Shall post message on user thread to invoke respective
**                  callback function provided by the caller of Timer function
**
** Returns          None
**
*/
static void phOsalUwb_Timer_Expired(struct k_timer *timer)
{
    uint32_t dwIndex                      = 0;
    phOsalUwb_TimerHandle_t *pTimerHandle = NULL;

    for (dwIndex = 0x00; (dwIndex < PH_UWB_MAX_TIMER); dwIndex++) {
        /* For Timer, check whether the requested handle is present or not */
        if ((apTimerInfo[dwIndex].hTimerHandle) == ((void *)timer)) {
            pTimerHandle = (phOsalUwb_TimerHandle_t *)&apTimerInfo[dwIndex];
            break;
        }
    }
    if (pTimerHandle != NULL) {
        /* Timer is stopped when callback function is invoked */
        pTimerHandle->eState = eTimerStopped;
        if (pTimerHandle->Application_callback) {
            /* Post a message on the queue to invoke the function */
            pTimerHandle->Application_callback(pTimerHandle->TimerId, pTimerHandle->pContext);
        }
    }
    return;
}

uint32_t phUtilUwb_CheckForAvailableTimer(void)
{
    /* Variable used to store the index at which the object structure details
     can be stored. Initialize it as not available. */
    uint32_t dwRetval = PH_OSALUWB_TIMER_ID_INVALID;

    /* Check whether Timer object can be created
     * Maximum 2 Timers supported
     * Timer 0 & Timer 1
     * By default hTimerHandle is null, value gets assigned in Timer creation API */
    for (dwRetval = 0x00; (dwRetval < PH_UWB_MAX_TIMER); dwRetval++) {
        if ((apTimerInfo[dwRetval].hTimerHandle) == NULL) {
            break;
        }
    }

    if (dwRetval >= PH_UWB_MAX_TIMER){
        dwRetval = PH_OSALUWB_TIMER_ID_INVALID;
    }

    return (dwRetval);
}

UWBSTATUS phOsalUwb_CheckTimerPresence(void *pObjectHandle)
{
    uint32_t dwIndex;
    UWBSTATUS wRegisterStatus = UWBSTATUS_INVALID_PARAMETER;

    for (dwIndex = 0x00; (dwIndex < PH_UWB_MAX_TIMER); dwIndex++) {
        /* For Timer, check whether the requested handle is present or not */
        if (((&apTimerInfo[dwIndex]) == (phOsalUwb_TimerHandle_t *)pObjectHandle)) {
            wRegisterStatus = UWBSTATUS_SUCCESS;
            break;
        }
    }
    return wRegisterStatus;
}

#endif

