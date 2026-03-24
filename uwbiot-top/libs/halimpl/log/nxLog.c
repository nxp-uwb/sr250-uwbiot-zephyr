/* Copyright 2020,2022-2024,2026  NXP
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#include "phNxpLogApis_HalUci.h"
#include "phOsalUwb.h"
#include <stdarg.h>
#include <stdio.h>
#include "phUwbTypes.h"
#include <inttypes.h>
#if defined(__clang__) && !defined(__apple_build_version__) && !defined(__SES_ARM) && !defined(__arm__) && !defined(__ZEPHYR__)
#include <io.h> /* for isatty */
#endif

#if defined(_MSC_VER)
#include <windows.h>
#include <time.h>
#endif

#if (defined(__APPLE__)) || (defined(CPU_LPC54628))
#define USE_COLORED_LOGS 0
#else
#define USE_COLORED_LOGS 1
#endif

#define COLOR_RED    "\033[0;31m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE   "\033[0;34m"
#define COLOR_RESET  "\033[0m"

#define szCRLF "\r\n"
#define szLF   "\n"

#if defined(_MSC_VER)
#define SHOW_TIMESTAMP_IN_LOGS 1
static HANDLE sStdOutConsoleHandle = INVALID_HANDLE_VALUE;
#define szEOL szLF

#if USE_COLORED_LOGS
#define USE_COLORED_LOGS_MSVC 1
static void msvc_setColor(int level);
static void msvc_reSetColor(void);
#else // USE_COLORED_LOGS
#define USE_COLORED_LOGS_MSVC 0
#endif // USE_COLORED_LOGS

#endif // _MSC_VER

#if defined(__GNUC__) && !defined(__ARMCC_VERSION) && !defined(__SES_ARM)
#include <unistd.h>
#include <time.h>
#endif /* __GNUC__ && !defined(__ARMCC_VERSION) && !defined(__SES_ARM)*/

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#if ((defined(AX_EMBEDDED)) && (AX_EMBEDDED != 0))
#define szEOL szCRLF
#else
#define szEOL szLF
#endif

#if USE_COLORED_LOGS
#define USE_COLORED_LOGS_ANSI 1
static void ansi_setColor(int level);
static void ansi_reSetColor(void);
#else // USE_COLORED_LOGS
#define USE_COLORED_LOGS_ANSI 0
#endif // USE_COLORED_LOGS

#endif /* __GNUC__ && !defined(__ARMCC_VERSION) */

#ifndef szEOL
#define szEOL szCRLF
#endif

static const char *szLevel[] = {"ERROR", "WARN ", "INFO ", "DEBUG", "TX > ", "RX < "};

#if ((defined(AX_EMBEDDED)) && (AX_EMBEDDED != 0))
#define TAB_SEPRATOR "\t"
#else
#define TAB_SEPRATOR "   "
#endif

/* MaximumxSize required initial info of Log 36 bytes.
 * For example TMLUWB  :RX < :RECV                :*/
#define LOGPRINT_FORMAT_STRING 36

/*!
 * \brief Max size of CLOG buffer is calculated based on value of RX_LOG_MAX_NUMBER_OF_BYTES
 *        CLOG buffer includes the initial info, two times of number of bytes for printing
 *        In addition to that 1 byte for spaces after every 4 bytes and few extra bytes for buffer spaces
 *        Example : RX_LOG_MAX_NUMBER_OF_BYTES 259 bytes then the Buffer required for
 *        logging will consume 36 + 259 * 2 + ( (259 * 2) / 4 ) * 10 = 694 bytes
 */
#define CLOG_BUFFER_SIZE (LOGPRINT_FORMAT_STRING + (RX_LOG_MAX_NUMBER_OF_BYTES * 2) + ((RX_LOG_MAX_NUMBER_OF_BYTES * 2) / 4) + 10)

char cLogBuffer[CLOG_BUFFER_SIZE];

void *mLogMutex = NULL;

/* Call back pointer to give log data to other layers,
 * e.g. test framework to log data to common file */
fpUwb_LogPrintCb_t gfpLogPrintCb;

void phUwb_LogInit(void)
{
    if ((mLogMutex == NULL) && phOsalUwb_CreateMutex(&mLogMutex) != UWBSTATUS_SUCCESS) {
        LOG_E("Error: phUwb_LogInit(), could not create mutex mLogMutex\n");
    }
}

#if (UWBFTR_SE_SE051W)
/* FIXME: Not yet tested on board */
uint8_t nLog_Init()
{
    phUwb_LogInit();
    return 0;
}

/* FIXME: Not yet tested on board */
uint8_t nLog_DeInit()
{
    phUwb_LogDeInit();
    return 0;
}
#endif //( UWBFTR_SE_SE051W )

void phUwb_LogDeInit()
{
    (void)phOsalUwb_DeleteMutex(&mLogMutex);
    mLogMutex = NULL;
}

#if defined(SHOW_TIMESTAMP_IN_LOGS)
void phUwb_ConsoleTimeStamp(void)
{
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    PRINTF("%02u:%02u:%02u:%03u : ", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
}
#endif //  SHOW_TIMESTAMP_IN_LOGS

static size_t phUwb_LogPrint_FormatBuffer_String(char *cLogbufferptr, size_t buffersize, const char *message, size_t totalSize)
{
    size_t alignmentSize = 0;
    size_t i;
    size_t totalCount = 0;
    size_t messageLen = strlen(message);
    if (messageLen <= totalSize) {
        alignmentSize = (totalSize - messageLen);
    }
    else {
        messageLen = totalSize;
        alignmentSize = 0;
    }

    for (i = 0; i < messageLen && buffersize > 3 && buffersize < CLOG_BUFFER_SIZE; i++) {
        cLogbufferptr[totalCount] = message[i];
        buffersize -= 1;
        if (totalCount < (SIZE_MAX - 1)) {
            totalCount += 1;
        } else {
            return SIZE_MAX;
        }
    }

    for (i = 0; i < alignmentSize && buffersize > 3 && buffersize < CLOG_BUFFER_SIZE; i++) {
        cLogbufferptr[totalCount] = ' ';
        buffersize -= 1;
        if (totalCount < (SIZE_MAX - 1)) {
            totalCount += 1;
        } else {
            return SIZE_MAX;
        }
    }

    return totalCount;
}

size_t phUwb_LogPrint_FormatBuffer_Hex(
    char *cLogbufferptr, size_t buffersize, const unsigned char *array, size_t array_len)
{
    const char hex[] = "0123456789ABCDEF";
    size_t i;
    size_t totalCount = 0;
    for (i = 0; i < array_len && buffersize > 3 && buffersize < CLOG_BUFFER_SIZE; ++i) {
        if (totalCount < (SIZE_MAX - 2)) {
            cLogbufferptr[totalCount++] = hex[(array[i] >> 4)];
            cLogbufferptr[totalCount++] = hex[array[i] & 0xF];
        } else {
            return SIZE_MAX;
        }
        buffersize -= 2;

        if (0 == ((i + 1) % 4)) {
            buffersize -= 1;
            if (totalCount < (SIZE_MAX - 1)) {
                cLogbufferptr[totalCount++] = ' ';
            } else {
                return SIZE_MAX;
            }
            cLogbufferptr[totalCount]   = '\0';
        }
    }

    if (totalCount != 0) {
        cLogbufferptr[totalCount] = '\0';
        if (totalCount < (SIZE_MAX - 1)) {
            totalCount += 1;
            } else {
                return SIZE_MAX;
            }
    }
    return totalCount;
}

void nLog(const char *tag, int level, const char *fmt, ...)
{
    size_t buffersize = sizeof(cLogBuffer) - 1;
    size_t offset = 0;
    size_t tempOffset = 0;
    int charsWritten = 0;
    char *cLogbufferptr    = cLogBuffer;
    const char separator[] = {':', '\0'};

    phOsalUwb_LockMutex(mLogMutex);
    phUwb_LogSetColor(level);

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, tag, 8);
    if (tempOffset == SIZE_MAX) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset = tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, separator, 1);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, szLevel[level - 1], 5);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, separator, 1);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        charsWritten = vsnprintf(&cLogbufferptr[offset], buffersize, fmt, args);
        va_end(args);
        if (charsWritten < 0) {
            goto exit;
        }
    }
#if defined(SHOW_TIMESTAMP_IN_LOGS)
    phUwb_ConsoleTimeStamp();
#endif
    LOG("%s", cLogBuffer);
    if (gfpLogPrintCb != NULL) {
#if defined(SHOW_TIMESTAMP_IN_LOGS)
        /* 16 is timestamp header */
        char timestamp_and_logmsg[16 + CLOG_BUFFER_SIZE];
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        snprintf(timestamp_and_logmsg,
            sizeof(timestamp_and_logmsg),
            "%02d:%02d:%02d:%03d : %s",
            lt.wHour,
            lt.wMinute,
            lt.wSecond,
            lt.wMilliseconds,
            cLogBuffer);
        gfpLogPrintCb(timestamp_and_logmsg);
#else
        gfpLogPrintCb(cLogBuffer);
#endif
        gfpLogPrintCb("\n");
    }

exit:
    phUwb_LogReSetColor();
    phOsalUwb_UnlockMutex(mLogMutex);
    return;
}

void nLog_au8(const char *tag, int level, const char *message, const unsigned char *array, size_t array_len)
{
    size_t buffersize = sizeof(cLogBuffer) - 1;
    size_t offset = 0;
    size_t tempOffset = 0;
    char *cLogbufferptr    = cLogBuffer;
    const char separator[] = {':', '\0'};

    phOsalUwb_LockMutex(mLogMutex);
    phUwb_LogSetColor(level);

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, tag, 8);
    if (tempOffset == SIZE_MAX) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset = tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, separator, 1);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, szLevel[level - 1], 5);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, separator, 1);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, message, 20);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_String(&cLogbufferptr[offset], buffersize, separator, 1);
    if ((tempOffset == SIZE_MAX) || (SIZE_MAX - tempOffset <= offset)) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }
    offset += tempOffset;
    buffersize -= offset;

    tempOffset = phUwb_LogPrint_FormatBuffer_Hex(&cLogbufferptr[offset], buffersize, array, array_len);
    if (tempOffset == SIZE_MAX) {
        PRINTF("Invalid buffer size. buffersize=%zu, offset=%zu",
            buffersize, offset);
        goto exit;
    }

#if defined(SHOW_TIMESTAMP_IN_LOGS)
    phUwb_ConsoleTimeStamp();
#endif
    LOG("%s", cLogBuffer);
    if (gfpLogPrintCb != NULL) {
#if defined(SHOW_TIMESTAMP_IN_LOGS)
        /* 16 is timestamp header */
        char timestamp_and_log[16 + CLOG_BUFFER_SIZE];
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        snprintf(timestamp_and_log,
            sizeof(timestamp_and_log),
            "%02d:%02d:%02d:%03d : %s",
            lt.wHour,
            lt.wMinute,
            lt.wSecond,
            lt.wMilliseconds,
            cLogBuffer);
        gfpLogPrintCb(timestamp_and_log);
#else
        gfpLogPrintCb(cLogBuffer);
#endif
        gfpLogPrintCb("\n");
    }

exit:
    phUwb_LogReSetColor();
    phOsalUwb_UnlockMutex(mLogMutex);
    return;
}

#ifdef NDEBUG

void phUwb_LogPrint_Spinner(void)
{
    const char spin_pattern[] = {'[', '|', ']', '#', '[', '|', ']', '(', '+', ')'};
    static size_t spin_index  = 0;
    PUTCHAR(spin_pattern[spin_index++]);
    PUTCHAR('\r');
    if (spin_index >= sizeof(spin_pattern)) {
        spin_index = 0;
    }
}
#endif // NDEBUG

void phUwb_LogPrintSetCb(fpUwb_LogPrintCb_t fpCbLogSzFn)
{
    gfpLogPrintCb = fpCbLogSzFn;
}

void phUwb_LogSetColor(int level)
{
#if ((defined(USE_COLORED_LOGS_MSVC)) && (USE_COLORED_LOGS_MSVC != 0))
    msvc_setColor(level);
#endif
#if USE_COLORED_LOGS_ANSI
    ansi_setColor(level);
#endif
}

void phUwb_LogReSetColor(void)
{
#if ((defined(USE_COLORED_LOGS_MSVC)) && (USE_COLORED_LOGS_MSVC != 0))
    msvc_reSetColor();
#endif
#if USE_COLORED_LOGS_ANSI
    ansi_reSetColor();
#endif
}

#if ((defined(USE_COLORED_LOGS_MSVC)) && (USE_COLORED_LOGS_MSVC != 0))
static void msvc_setColor(int level)
{
    WORD wAttributes = 0;
    if (sStdOutConsoleHandle == INVALID_HANDLE_VALUE) {
        sStdOutConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    switch (level) {
    case UWB_LOG_ERROR_LEVEL:
        wAttributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
    case UWB_LOG_WARN_LEVEL:
        wAttributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
    case UWB_LOG_INFO_LEVEL:
        wAttributes = FOREGROUND_GREEN;
        break;
    case UWB_LOG_DEBUG_LEVEL:
        /* As of now put color here. All normal printfs would be in WHITE
         * Later, remove this color.
         */
        wAttributes = FOREGROUND_RED | FOREGROUND_GREEN;
        break;
    case UWB_LOG_TX_LEVEL:
        wAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN;
        break;
    case UWB_LOG_RX_LEVEL:
        wAttributes = FOREGROUND_BLUE;
        break;
    default:
        wAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    }
    SetConsoleTextAttribute(sStdOutConsoleHandle, wAttributes);
}

static void msvc_reSetColor()
{
    msvc_setColor(-1 /* default */);
}
#endif // USE_COLORED_LOGS_MSVC

#if USE_COLORED_LOGS_ANSI

int log_isatty()
{
#if !defined(__SES_ARM) && !defined(__ZEPHYR__)
    if (isatty(fileno(stdout))) {
        return true;
    }
#endif
    return false;
}

static void ansi_setColor(int level)
{
    if (log_isatty()) {
        return;
    }

    switch (level) {
    case UWB_LOG_ERROR_LEVEL:
        PRINTF(COLOR_RED);
        break;
    case UWB_LOG_WARN_LEVEL:
        PRINTF(COLOR_YELLOW);
        break;
    case UWB_LOG_INFO_LEVEL:
        PRINTF(COLOR_BLUE);
        break;
    case UWB_LOG_DEBUG_LEVEL:
        /* As of now put color here. All normal printfs would be in WHITE
         * Later, remove this color.
         */
        PRINTF(COLOR_GREEN);
        break;
    case UWB_LOG_TX_LEVEL:
        PRINTF(COLOR_BLUE);
        break;
    case UWB_LOG_RX_LEVEL:
        PRINTF(COLOR_GREEN);
        break;
    default:
        PRINTF(COLOR_RESET);
    }
}

static void ansi_reSetColor()
{
    if (log_isatty()) {
        return;
    }
    PRINTF(COLOR_RESET);
}
#endif // USE_COLORED_LOGS_ANSI
