/*
 *
 * Copyright 2018-2020,2022 NXP.
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

/* Select the values of log */

#ifndef _PHNXPLOG_DEFAULT_H
#define _PHNXPLOG_DEFAULT_H

/* Ensure uwb_logging.h is included before this file */
#ifndef UWB_LOGGING_H
#error "Ensure uwb_logging.h is included before this file"
#endif

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#if (UWBIOT_LOG_VERBOSE == 1)

/* FOR VERBOSE
 *
 * Define Global Log level. Depending on this, sub module levels are enabled/disabled.
 *
 * To switch off all the logging part then set the macro to 0 value.
 */
#define UWB_GLOBAL_LOG_LEVEL UWB_LOG_DEBUG_LEVEL

#define APP_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#ifndef BOARD_LOG_LEVEL
#define BOARD_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#endif
#define FWDNLD_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define HALUCI_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define HALUTILS_LOG_LEVEL   UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define SWUP_LOG_LEVEL       UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define TMLUWB_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define UCICORE_LOG_LEVEL    UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define UWBAPI_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define DELU_LOG_LEVEL       UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */
#define SE_WRAPPER_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL /* Over-ride if needed */

#elif (UWBIOT_LOG_SILENT == 1)

/* FOR SILENT
 *
 * Define Global Log level. Depending on this, sub module levels are enabled/disabled.
 * To switch off all the logging part then set the macro to 0 value.
 */
#define UWB_GLOBAL_LOG_LEVEL UWB_LOG_SILENT_LEVEL

#define APP_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL
#ifndef BOARD_LOG_LEVEL
#define BOARD_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL
#endif
#define FWDNLD_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define HALUCI_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define HALUTILS_LOG_LEVEL   UWB_GLOBAL_LOG_LEVEL
#define SWUP_LOG_LEVEL       UWB_GLOBAL_LOG_LEVEL
#define TMLUWB_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define UCICORE_LOG_LEVEL    UWB_GLOBAL_LOG_LEVEL
#define UWBAPI_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define DELU_LOG_LEVEL       UWB_GLOBAL_LOG_LEVEL
#define SE_WRAPPER_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL

#else

/* FOR DEFAULT
 *
 * Define Global Log level. Depending on this, sub module levels are enabled/disabled.
 * To switch off all the logging part then set the macro to 0 value.
 */
#define UWB_GLOBAL_LOG_LEVEL UWB_LOG_INFO_LEVEL

#define APP_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL

#ifndef BOARD_LOG_LEVEL
#define BOARD_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL
#endif

#define FWDNLD_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define HALUCI_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define HALUTILS_LOG_LEVEL   UWB_GLOBAL_LOG_LEVEL
#define SWUP_LOG_LEVEL       UWB_GLOBAL_LOG_LEVEL
#define TMLUWB_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define UCICORE_LOG_LEVEL    UWB_GLOBAL_LOG_LEVEL
#define UWBAPI_LOG_LEVEL     UWB_GLOBAL_LOG_LEVEL
#define DELU_LOG_LEVEL       UWB_GLOBAL_LOG_LEVEL
#define SE_WRAPPER_LOG_LEVEL UWB_GLOBAL_LOG_LEVEL

#endif // UWBIOT_LOG_VERBOSE

#endif // _PHNXPLOG_DEFAULT_H
