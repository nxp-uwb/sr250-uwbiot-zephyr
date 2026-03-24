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
#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"

#ifndef UWB_READ_TASK_H
#define UWB_READ_TASK_H

OSAL_TASK_RETURN_TYPE UCI_ReaderTask(void *args);

#if UWBIOT_OS_NATIVE
void UCI_ReaderTask_Enable(void);
void UCI_ReaderTask_Disable(void);
#endif

#endif
