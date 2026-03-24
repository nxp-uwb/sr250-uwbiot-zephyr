/* Copyright 2020,2022,2024 NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef UWB_LOGGING_H
#define UWB_LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <phUwb_BuildConfig.h>

#include <uwb_board.h>

void phUwb_LogInit(void);
void phUwb_LogDeInit(void);

#ifndef PRINTF
#error PRINTF must be defined by now
#endif

#define LOG(...)         \
    PRINTF(__VA_ARGS__); \
    PRINTF("\n");

#define ALOGD(...)          LOG(__VA_ARGS__)
#define ALOGE(...)          LOG(__VA_ARGS__)
#define ALOGD_IF(cond, ...) LOG(__VA_ARGS__)
// Make Zero to disable
// Define Logging Levels
#define UWB_LOG_SILENT_LEVEL 0x00
#define UWB_LOG_ERROR_LEVEL  0x01
#define UWB_LOG_WARN_LEVEL   0x02
#define UWB_LOG_INFO_LEVEL   0x03
#define UWB_LOG_DEBUG_LEVEL  0x04
#define UWB_LOG_TX_LEVEL     0x05
#define UWB_LOG_RX_LEVEL     0x06

/*!
 * \brief Indicates the Maximum number of bytes to be printed while logging UCI Notifications or Responses
 *        when length of the received UCI frame is more.
 *        Maximum value supported is 2052 bytes for CIR frames.
 *        All length parameters indicated here include UCI header and payload.
 *        4 Bytes for UCI header and rest for UCI payload.
 *        Internal RAM Buffer required for logging will be increased as per the equation
 *        Buffer size = 36 + (2 * RX_LOG_MAX_NUMBER_OF_BYTES ) + ((2 * RX_LOG_MAX_NUMBER_OF_BYTES ) / 4) + 10.
 *        Example : RX_LOG_MAX_NUMBER_OF_BYTES 259 bytes then the Buffer required for
 *        logging will consume 36 + 259 * 2 + ( (259 * 2) / 4 ) + 10 = 694 bytes
 *        This macro is configurable, can be increased or decreased. Default value is 259 bytes.
 *
 * \warning Configuring the macro value beyond default value of 259 bytes, will have the following consequences.
 *          Internal RAM Buffer needed to Process will be increased as per the mentioned equation.
 *          Notifications may get missed during reception of UCI frames, since Logger is busy in printing more bytes
 *          as specified in the macro. Only for debugging purposes, the macro value to be altered.
 *          All the test results shared will always be with the default value 259 bytes.
 */
#define RX_LOG_MAX_NUMBER_OF_BYTES 259

#include "phNxpLogDefault.h"

#define LOG_PRINT nLog

EXTERNC void nLog(const char *tag, int level, const char *fmt, ...);
EXTERNC void nLog_au8(const char *tag, int level, const char *message, const unsigned char *array, size_t array_len);
#ifdef NDEBUG
/* show a spinnner that keeps rotating at one fixed place, just to show progress */
EXTERNC void phUwb_LogPrint_Spinner(void);
#endif //NDEBUG

/* for log call back from test frameworks
 * Not to be used in production */
typedef void (*fpUwb_LogPrintCb_t)(const char *szString);
EXTERNC void phUwb_LogPrintSetCb(fpUwb_LogPrintCb_t fpCbLogSzFn);

#define LOG_PRI nLog

#define phUwb_LogMsg(trace_set_mask, ...) LOG(__VA_ARGS__)

EXTERNC void phUwb_LogSetColor(int level);
EXTERNC void phUwb_LogReSetColor(void);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif // UWB_LOGGING_H
