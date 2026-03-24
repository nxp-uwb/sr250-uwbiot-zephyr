/*
 * Copyright 2012-2020,2023,2024 NXP.
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
 * OSAL header files related to Timer functions.
 */

#ifndef PHOSALUWB_TIMER_H
#define PHOSALUWB_TIMER_H

#include "phUwbTypes.h"
#include "phUwb_BuildConfig.h"

/*
 * Timer callback interface which will be called once registered timer
 * time out expires.
 *        TimerId  - Timer Id for which callback is called.
 *        pContext - Parameter to be passed to the callback function
 */
typedef void (*pphOsalUwb_TimerCallbck_t)(uint32_t TimerId, void *pContext);

/** \addtogroup grp_osal_err
 *
 * @{
 */

/**
 * The Timer could not be created due to a
 * system error */
#define PH_OSALUWB_TIMER_CREATE_ERROR (0X00E0)

/**
 * The Timer could not be started due to a
 * system error or invalid handle */
#define PH_OSALUWB_TIMER_START_ERROR (0X00E1)

/**
 * The Timer could not be stopped due to a
 * system error or invalid handle */
#define PH_OSALUWB_TIMER_STOP_ERROR (0X00E2)

/**
 * The Timer could not be deleted due to a
 * system error or invalid handle */
#define PH_OSALUWB_TIMER_DELETE_ERROR (0X00E3)

/**
 * Invalid timer ID type.This ID used indicate timer creation is failed */
#define PH_OSALUWB_TIMER_ID_INVALID (0xFFFF)

/**
 * OSAL timer message .This message type will be posted to
 * calling application thread.*/
#define PH_OSALUWB_TIMER_MSG (0x315)

/*
 * 2 Timers are used. One each by UciHal.
 */
#define PH_UWB_MAX_TIMER (2U)

/** @} */

/** Globals,Structure and Enumeration */

/** \addtogroup grp_osal_timer
 *
 * @{
 */

/**
 * @brief Creates a timer which shall call back the specified function
 * when the timer expires Fails if OSAL module is not initialized or timers are
 *  already occupied
 *
 * @param[in] bAutoReload (not used in Posix) If bAutoReload is set to TRUE then the timer will
 *                        expire repeatedly with a frequency set by the xTimerPeriodInTicks parameter.
 *                        Timer restarts automatically.
 *                        If bAutoReload is set to FALSE then the timer will be a one-shot timer and
 *                        enter the dormant state after it expires.
 *
 * @retval TimerId TimerId value of PH_OSALUWB_TIMER_ID_INVALID indicates that
 *         timer is not created                -
 *
 */
uint32_t phOsalUwb_Timer_Create(bool bAutoReload);

/**
 * @brief      Creates a timer which shall call back the specified function
 *             when the timer expires
 *             Starts the requested, already created, timer If the timer is already running,
 *             timer stops and restarts with the new timeout value and new callback function
 *             in case any.
 *
 *
 * @param[in] dwTimerId valid timer ID obtained during timer creation
 * @param[in] dwRegTimeCnt requested timeout in milliseconds
 * @param[in] pApplication_callback  application callback interface to be
 *                                   called when timer expires
 * @param[in] pContext caller context, to be passed to the application callback function
 *
 * @retval UWBSTATUS_SUCCESS the operation was successful
 * @retval UWBSTATUS_NOT_INITIALISED  OSAL Module is not initialized
 * @retval UWBSTATUS_INVALID_PARAMETER  invalid parameter passed to the function
 * @retval PH_OSALUWB_TIMER_START_ERROR timer could not be created due to system error
 *
 **/
UWBSTATUS phOsalUwb_Timer_Start(
    uint32_t dwTimerId, uint32_t dwRegTimeCnt, pphOsalUwb_TimerCallbck_t pApplication_callback, void *pContext);

/**
 * @brief Stops already started timer, allows to stop running timer. In case timer is stopped,
 *        timer callback will not be notified any more
 *
 * @param[in] dwTimerId  valid timer ID obtained during timer creation
 *
 * @retval UWBSTATUS_SUCCESS the operation was successful
 * @retval UWBSTATUS_NOT_INITIALISED  OSAL Module is not initialized
 * @retval UWBSTATUS_INVALID_PARAMETER invalid parameter passed to the function
 * @retval PH_OSALUWB_TIMER_STOP_ERROR  timer could not be stopped due to system error
 *
 */
UWBSTATUS phOsalUwb_Timer_Stop(uint32_t dwTimerId);

/**
 * @brief Deletes previously created timer
 *        Allows to delete previously created timer. In case timer is running,
 *        it is first stopped and then deleted
 *
 * @param[in] dwTimerId valid timer ID obtained during timer creation
 *
 * @retval UWBSTATUS_SUCCESS the operation was successful
 * @retval UWBSTATUS_NOT_INITIALISED OSAL Module is not initialized
 * @retval UWBSTATUS_INVALID_PARAMETER  invalid parameter passed to the function
 * @retval PH_OSALUWB_TIMER_DELETE_ERROR  timer could not be deleted
 *                                        due to system error
 *
 */
UWBSTATUS phOsalUwb_Timer_Delete(uint32_t dwTimerId);

/**
 * @brief Find an available timer id
 *
 *
 * @retval Available timer id
 *
 **/
uint32_t phUtilUwb_CheckForAvailableTimer(void);

/**
 * @brief Checks the requested timer is present or not
 *
 * @param[in] pObjectHandle  timer context
 *
 * @retval UWBSTATUS_SUCCESS if found
 *
 */
UWBSTATUS phOsalUwb_CheckTimerPresence(void *pObjectHandle);

/** @} */

#endif /* PHOSALUWB_TIMER_H */
