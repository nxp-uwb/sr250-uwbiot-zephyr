/*
 *
 * Copyright 2020 NXP.
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

/* GENERATED FILE, DO NOT MODIFY! */

#ifndef _PHNXPLOG_UCICORE_H
#define _PHNXPLOG_UCICORE_H

#include "uwb_logging.h"

/* Check if we are double defining these macros */
#if defined(LOG_E) || defined(LOG_W) || defined(LOG_I) || defined(LOG_D)
/* This should not happen.  The only reason this could happen is double inclusion of different log files. */
#error "LOG_ macro already defined"
#endif

/* Logging Level used by UCICORE module */
#define UCICORE_MODULE_NAME "UCICORE"

#if (UWB_GLOBAL_LOG_LEVEL >= UWB_LOG_INFO_LEVEL) && (UCICORE_LOG_LEVEL >= UWB_LOG_DEBUG_LEVEL)
#define ENABLE_UCI_MODULE_TRACES TRUE
#else
#define ENABLE_UCI_MODULE_TRACES FALSE
#endif

/*
 * Use the following macros.
 */

#if (UCICORE_LOG_LEVEL >= UWB_LOG_ERROR_LEVEL)
#define LOG_E(...)                      nLog(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, __VA_ARGS__)
#define LOG_X8_E(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_E(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X16_E(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_E(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X32_E(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_E(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_E(ARRAY, LEN)           nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_E(MESSAGE, ARRAY, LEN) nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_ERROR_LEVEL, MESSAGE, ARRAY, LEN)
#else
#define LOG_E(...)
#define LOG_X8_E(VALUE)
#define LOG_U8_E(VALUE)
#define LOG_X16_E(VALUE)
#define LOG_U16_E(VALUE)
#define LOG_X32_E(VALUE)
#define LOG_U32_E(VALUE)
#define LOG_AU8_E(ARRAY, LEN)
#define LOG_MAU8_E(MESSAGE, ARRAY, LEN)
#endif

#if (UCICORE_LOG_LEVEL >= UWB_LOG_WARN_LEVEL)
#define LOG_W(...)                      nLog(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, __VA_ARGS__)
#define LOG_X8_W(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_W(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X16_W(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_W(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X32_W(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_W(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_W(ARRAY, LEN)           nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_W(MESSAGE, ARRAY, LEN) nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_WARN_LEVEL, MESSAGE, ARRAY, LEN)
#else
#define LOG_W(...)
#define LOG_X8_W(VALUE)
#define LOG_U8_W(VALUE)
#define LOG_X16_W(VALUE)
#define LOG_U16_W(VALUE)
#define LOG_X32_W(VALUE)
#define LOG_U32_W(VALUE)
#define LOG_AU8_W(ARRAY, LEN)
#define LOG_MAU8_W(MESSAGE, ARRAY, LEN)
#endif

#if (UCICORE_LOG_LEVEL >= UWB_LOG_INFO_LEVEL)
#define LOG_I(...)                      nLog(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, __VA_ARGS__)
#define LOG_X8_I(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_I(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X16_I(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_I(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X32_I(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_I(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_I(ARRAY, LEN)           nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_I(MESSAGE, ARRAY, LEN) nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_INFO_LEVEL, MESSAGE, ARRAY, LEN)
#else
#define LOG_I(...)
#define LOG_X8_I(VALUE)
#define LOG_U8_I(VALUE)
#define LOG_X16_I(VALUE)
#define LOG_U16_I(VALUE)
#define LOG_X32_I(VALUE)
#define LOG_U32_I(VALUE)
#define LOG_AU8_I(ARRAY, LEN)
#define LOG_MAU8_I(MESSAGE, ARRAY, LEN)
#endif

#if (UCICORE_LOG_LEVEL >= UWB_LOG_DEBUG_LEVEL)
#define LOG_D(...)                      nLog(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, __VA_ARGS__)
#define LOG_X8_D(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_D(VALUE)                 nLog(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X16_D(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_D(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_X32_D(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_D(VALUE)                nLog(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_D(ARRAY, LEN)           nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_D(MESSAGE, ARRAY, LEN) nLog_au8(UCICORE_MODULE_NAME, UWB_LOG_DEBUG_LEVEL, MESSAGE, ARRAY, LEN)
#else
#define LOG_D(...)
#define LOG_X8_D(VALUE)
#define LOG_U8_D(VALUE)
#define LOG_X16_D(VALUE)
#define LOG_U16_D(VALUE)
#define LOG_X32_D(VALUE)
#define LOG_U32_D(VALUE)
#define LOG_AU8_D(ARRAY, LEN)
#define LOG_MAU8_D(MESSAGE, ARRAY, LEN)
#endif

/*
 * These are the Legacy macros used for logging.
 * Do not use further.
 */
#define UCI_TRACE_E LOG_E
#define UCI_TRACE_W LOG_W
#define UCI_TRACE_I LOG_I
#define UCI_TRACE_D LOG_D

#endif /* _PHNXPLOG_UCICORE_H */
