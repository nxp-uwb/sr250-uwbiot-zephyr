/*
 * Copyright 2012-2020,2022-2024 NXP.
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
 * @brief OSAL header files related to thread functions.
 */

#ifndef PHOSALUWB_THREAD_H
#define PHOSALUWB_THREAD_H

/** Include Files */

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "phOsalUwb.h"
#include "phOsalUwb_Internal.h"

/** @addtogroup grp_osal_thread
 *
 * @{
 */

/**
 * \brief Thread Function Pointer Declaration.
 *
 * This points to the function to be invoked upon creation of thread.
 * It is not the immediate thread procedure since the API of this function
 * depends on the OS.
 *
 * This function shall be called within the body of the thread procedure
 * defined by the underlying, OS-depending OSAL implementation.
 *
 */
#if UWBIOT_OS_FREERTOS || UWBIOT_OS_ZEPHYR
typedef void (*pphOsalUwb_ThreadFunction_t)(void *);
#elif UWBIOT_OS_NATIVE
typedef void *(*pphOsalUwb_ThreadFunction_t)(void *);
#else
#error "Invalid OS Type"
#endif

#if UWBIOT_OS_ZEPHYR
#define UWBIOT_STACK_DEFINE(OBJECT,SIZE) static K_THREAD_STACK_DEFINE(OBJECT, SIZE);
#define UWBIOT_THREAD_STACK_SIZE(OBJECT) K_THREAD_STACK_SIZEOF(OBJECT);
#else
#define UWBIOT_STACK_DEFINE(...)
#endif

/**
 * Thread handle Declaration.
 *
 */
#if UWBIOT_OS_NATIVE

/** OSAL Handle structure containing details of a thread. */
typedef struct phOsalUwb_sOsalThreadHandle
{
    pphOsalUwb_ThreadFunction_t ThreadFunction; /**<Thread function to be invoked */
    void *Params;                               /**<Parameters to the thread function */
    pthread_t ObjectHandle;                     /**<Handle of the thread object */
} phOsalUwb_sOsalThreadHandle_t;

#elif UWBIOT_OS_ZEPHYR

typedef struct k_thread k_thread_t;

typedef struct phOsalUwb_sOsalThreadHandle
{
    /*Pointer for stroing the Thread Handle*/
    k_thread_t *zthread_Handle;
    /*Pointer for stroing the Stack Thread Handle*/
    k_thread_stack_t *zthread_Stack_Handle;
    size_t zthread_Stack_Size;
    /*ID of new thread */
    k_tid_t zthread_Tid;
} phOsalUwb_sOsalThreadHandle_t;
#else
/** OSAL Handle structure containing details of a thread.*/
typedef struct phOsalUwb_sOsalThreadHandle
{
    pphOsalUwb_ThreadFunction_t ThreadFunction; /**<Thread function to be invoked */
    void *Params;                               /**<Parameters to the thread function */
    TaskHandle_t ObjectHandle;                  /**<Handle of the Task object */
} phOsalUwb_sOsalThreadHandle_t;

#endif // defined(ANDROID_MW) || defined(COMPANION_DEVICE) || UWBIOT_OS_NATIVE

/**
 * @brief Parameters to create a thread/task OS Independent way
 *
 */
typedef struct phOsalUwb_ThreadCreationParams
{
    /** stackdepth in stackwidth units */
    uint16_t stackdepth;
    /** Name of task */
    char taskname[TASK_NAME_MAX_LENGTH];
    /** Context passed to thread at creation */
    void *pContext;
    /** Priority of this thread */
    uint16_t priority;
#if UWBIOT_OS_ZEPHYR
    /** Stack object */
    k_thread_stack_t *pStack;
#endif // UWBIOT_OS_ZEPHYR
} phOsalUwb_ThreadCreationParams_t;

/** @} */

/** @addtogroup grp_osal_thread
 *
 * Macro Definitions
 *
 * @{
 */

/**
 * A new mutex could not be created due to a system error. */
#define PH_OSALUWB_THREAD_CREATION_ERROR (0x00F8)

/**
 * The given thread could not be suspended due to a system error. */
#define PH_OSALUWB_THREAD_SUSPEND_ERROR (0x00F9)

/**
 * The given thread could not be resumed due to a system error. */
#define PH_OSALUWB_THREAD_RESUME_ERROR (0x00FA)

/**
 * The given thread could not be deleted due to a system error. */
#define PH_OSALUWB_THREAD_DELETE_ERROR (0x00FB)

/**
 * The given thread could not be deleted due to a system error. */
#define PH_OSALUWB_THREAD_SETPRIORITY_ERROR (0x00FC)

/**
 * The given thread could not be deleted due to a system error. */
#define PH_OSALUWB_THREAD_JOIN_ERROR (0x00FD)

/**
 * Invalid timer ID type.This ID used indicate timer creation is failed */
#define PH_OSALUWB_THREAD_ID_INVALID (0XFFFF)

/**
 * The maximum number of threads allowed in system. */
#define PH_UWB_MAX_THREAD (4)

/** @} */

#if UWBIOT_OS_FREERTOS
#define OSAL_TASK_RETURN_TYPE void
#define OSAL_TASK_RETURN_VALUE
#define UWBOSAL_TASK_HANDLE xTaskHandle

#elif UWBIOT_OS_NATIVE
#define OSAL_TASK_RETURN_TYPE  void *
#define OSAL_TASK_RETURN_VALUE NULL
#define UWBOSAL_TASK_HANDLE    pthread_t

#elif UWBIOT_OS_ZEPHYR
#define OSAL_TASK_RETURN_TYPE  void
#define OSAL_TASK_RETURN_VALUE NULL
#define UWBOSAL_TASK_HANDLE    phOsalUwb_sOsalThreadHandle_t *

#define STACK_WORD_SIZE 0x4

#else
#error "Invalid OS Type"
#endif

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

#define PHOSALUWB_SET_TASKNAME(CREATION_PARAMS, TASK_NAME)                                                    \
    do {                                                                                                      \
        strncpy((CREATION_PARAMS).taskname, (TASK_NAME), MIN(sizeof((TASK_NAME)), TASK_NAME_MAX_LENGTH - 1)); \
        (CREATION_PARAMS).taskname[TASK_NAME_MAX_LENGTH - 1] = '\0';                                          \
    } while (0)

/** @addtogroup grp_osal_thread
 *
 * @{
 *
 */

/**
 * Thread Creation.
 *
 * This function creates a thread in the underlying system. To delete the
 * created thread use the phOsalUwb_Thread_Delete function.
 *
 *
 * \param[in,out] hThread    The Thread handle: The caller has to prepare a void
 * pointer that need not to be initialized. The value (content) of the pointer
 * is set by the function.
 *
 * \param[in] pThreadFunction Pointer to a function within the
 *                           implementation that shall be called by the Thread
 *                           procedure. This represents the Thread main
 *                           function. When this function exits the thread exits.
 * \param[in] pParam A pointer to a user-defined location the thread function receives.
 *
 * \retval #UWBSTATUS_SUCCESS                    The operation was successful.
 * \retval #UWBSTATUS_INSUFFICIENT_RESOURCES     At least one parameter value is
 *                                               invalid.
 * \retval #PH_OSALUWB_THREAD_CREATION_ERROR     A new Thread could not
 *                                               be created due to system error.
 * \retval #UWBSTATUS_NOT_INITIALISED Osal Module is not Initialized.
 *
 */
UWBSTATUS phOsalUwb_Thread_Create(void **hThread, pphOsalUwb_ThreadFunction_t pThreadFunction, void *pParam);

/**
 * Terminates the thread.
 *
 * This function Terminates the thread passed as a handle.
 *
 * \param[in] hThread The handle of the system object.
 *
 * \retval #UWBSTATUS_SUCCESS                The operation was successful.
 * \retval #UWBSTATUS_INVALID_PARAMETER      At least one parameter value is
 *                                           invalid.
 * \retval #PH_OSALUWB_THREAD_DELETE_ERROR   Thread could not be
 *                                           deleted due to system error.
 * \retval #UWBSTATUS_NOT_INITIALISED        Osal Module is not Initialized.
 *
 */
UWBSTATUS phOsalUwb_Thread_Delete(UWBOSAL_TASK_HANDLE hThread);

/**
 * Create Event.
 *
 * \retval void  Task handle as per underlying implementation.
 *
 */
UWBOSAL_TASK_HANDLE phOsalUwb_GetTaskHandle(void);

/**
 * This API allows to resume the current thread execution.
 * \note This function executes successfully without OSAL module Initialization.
 *
 * \param[in] xTaskToResume  Task to resume.
 *
 * \retval None
 */
void phOsalUwb_TaskResume(UWBOSAL_TASK_HANDLE xTaskToResume);

/**
 * This API allows to suspend the current thread execution.
 * \note This function executes successfully without OSAL module Initialization.
 *
 * \param[in] xTaskToSuspend  Task to suspend.
 *
 * \retval None
 */
void phOsalUwb_TaskSuspend(UWBOSAL_TASK_HANDLE xTaskToSuspend);

/**
 * This API allows to start the scheduler.
 * \note This function executes successfully without OSAL module Initialization.
 *
 * \retval None
 */
void phOsalUwb_TaskStartScheduler(void);

/**
 * Context switch task
 *
 * This function calls freertos TaskYIELD() to request context switch to another task.
 *
 *  \retval None
 */
void phOsalUwb_Thread_Context_Switch(void);

/**
 * To join the thread
 * This Function suspends execution of the calling thread until the target thread is terminated, in
 * native system with pthread_t, does nothing on RTOS base system.
 *
 * \param[in] hThread The handle of thread.
 *
 * \retval None
 */
void phOsalUwb_Thread_Join(UWBOSAL_TASK_HANDLE hThread);

/** @} */
#endif /*  PHOSALUWB_THREAD_H  */
