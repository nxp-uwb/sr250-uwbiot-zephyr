/*
 * Copyright 2019,2020,2024 NXP
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

#ifndef UWBCORESDK_INTERNAL_H_
#define UWBCORESDK_INTERNAL_H_

/* Build configurations */

#ifndef CONFIG_NXP_UWB_STACK_SIZE
#define CONFIG_NXP_UWB_STACK_SIZE 1500
#endif

#ifndef CONFIG_NXP_UWB_READER_STACK_SIZE
#if defined(CPU_S32K144)
#define CONFIG_NXP_UWB_READER_STACK_SIZE 512
#else
#define CONFIG_NXP_UWB_READER_STACK_SIZE 256
#endif // CPU_S32K144
#endif // CONFIG_NXP_UWB_READER_STACK_SIZE

/* Stack size */
#define UWBTASK_STACK_SIZE   CONFIG_NXP_UWB_STACK_SIZE
#define TMLREADER_STACK_SIZE CONFIG_NXP_UWB_READER_STACK_SIZE

#define TMLREADER_PRIO 3

#endif
