/*
 *
 * Copyright 2021 NXP.
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */
#include "phUwb_BuildConfig.h"

#ifndef UWB_DRIVER
#define UWB_DRIVER

OSAL_TASK_RETURN_TYPE UWB_WriterTask(void *args);
OSAL_TASK_RETURN_TYPE UWB_Hif_Handler_Task(void *args);
OSAL_TASK_RETURN_TYPE UCI_ReaderTask(void *args);
OSAL_TASK_RETURN_TYPE UWB_Pnp_App_Task(void *args);
#endif // UWB_DRIVER
